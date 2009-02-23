#include "obj.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

/**
  * Obj format definition: http://people.sc.fsu.edu/~burkardt/txt/obj_format.txt
  * usemtl
  * s: smoothing group, ignore for now
  * newmtl: ka, kd, ks, illum, ns
  * g
**/

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

list< Obj* > Obj::objectCache;
unsigned int Obj::_nextDisplayList = 1;

Obj* Obj::makeObj(const char* filename) {
    // First check if we already have an object that fits
    list< Obj* >::iterator it = Obj::objectCache.begin();
    for(; it != Obj::objectCache.end(); ++it) {
        if ((*it)->filename == filename) {
            return *it;
        }
    }
    // we didn't find one, so create one here
     Obj* result = new Obj(filename);
     Obj::objectCache.push_back(result);
     return result;
}

Obj::Obj(const char *filename) {
    // Keep track of the current material
    Material* currentMaterial = 0;
    vector<string> parts;
    string currentGroup;

    this->filename = filename;

    // Load this file in 
    ifstream file(filename);
    if (!file.is_open()) cout << "Error opening obj file\n";
    string line;


    while (!file.eof()) {
        getline(file, line);
        // parse this line and make either a vector, normal, texture coordinate or
        // face
        size_t position = line.find_first_of(' ');
        string command = line.substr(0, position);
        split(parts, line, is_any_of(" "));
        if (command == "v") {
            this->_addVertex(line);
        } else if (command == "vt") {
            this->_addTextureCoord(line);
        } else if (command == "vn") {
            this->_addNormal(line);
        } else if (command == "f") {
            this->_addFace(line, currentMaterial, currentGroup);
        } else if (command == "mtllib") {
            this->_addMTL(line);
        } else if (command == "usemtl") {
            // Try to find the material
            trim(parts[1]);
            currentMaterial = this->_findMaterial(parts[1]);
        } else if (command == "g") {
            cout << "Obj group: " << parts[1] << endl;
            // TODO: span this name over all parts > 0
            trim(parts[1]);
            currentGroup = parts[1];

        } else if (command == "s" || command == "g" || command == "#" || command == "") {
            // ignore for now
        } else {
            cout << "Ignore obj command: " << command << endl;
        }
    }
    file.close();

    // Create the display list
    this->_createDisplayLists();
}

void Obj::_addVertex(string line) {
    vector<string> tmp;
    vector<GLfloat>* vertex = new vector<GLfloat>;
    split(tmp, line, is_any_of(" "), token_compress_on);

    for (int i = 0; i < 3; ++i) {
        vertex->push_back(atof(tmp[i + 1].c_str()));
    }

    this->_vertices.push_back(vertex);
    this->_verticesList.push_back(vertex);
}

void Obj::_addTextureCoord(string line) {
    vector<string> tmp;
    vector<GLfloat>* texture = new vector<GLfloat>;
    split(tmp, line, is_any_of(" "), token_compress_on);

    for (int i = 0; i < 2; ++i) {
        texture->push_back(atof(tmp[i + 1].c_str()));
    }

    this->_textureCoords.push_back(texture);
}

void Obj::_addNormal(string line) {
    vector<string> tmp;
    vector<GLfloat>* normal = new vector<GLfloat>;
    split(tmp, line, is_any_of(" "), token_compress_on);

    for (int i = 0; i < 3; ++i) {
        normal->push_back(atof(tmp[i + 1].c_str()));
    }

    this->_normals.push_back(normal);
}

void Obj::_addFace(string line, Material* material, string group) {

    vector<string> vertices;
    split(vertices, line, is_any_of(" "), token_compress_on);

    Face* face = new Face;
    vector<string> vertex;

    // Add the material to the face
    face->material = material;

    vector<GLfloat>* pointer;

    // Skip the first one, its the command
    for (int i = 1; i < 4; ++i) {
        //vertex = this->_splitLine(vertices[i], 3, '/');
        split(vertex, vertices[i], is_any_of("/"));

        // Look up the vertex and add the pointer
        pointer = this->_vertices[atoi(vertex[0].c_str()) - 1];

        face->vertices[i - 1] = pointer;

        if (vertex[1].size() > 0) {
            pointer = this->_textureCoords[atoi(vertex[1].c_str()) - 1];
            face->textures[i - 1] = pointer;
        }
        else {
            face->textures[i - 1] = 0;
        }

        pointer = this->_normals[atoi(vertex[2].c_str()) - 1];
        face->normals[i - 1] = pointer;
    }

    // Add to list of faces
    this->_faces.push_back(face);

    // Add to group's list of faces
    this->_groupFaces[group].push_back(face);
}

vector<GLfloat> Obj::_parseLine(string line, const unsigned int length, const char seperator) {
    // split up the parts and convert to  vector<GLfloat>
    vector<string> parts = this->_splitLine(line, length + 1, seperator);
    vector<GLfloat> result(length);
    // Skip the first one its the command
    for (unsigned int i = 1; i < length + 1; ++i) {
        result[i - 1] = atof(parts[i].c_str());
    }
    return result;
}

vector<string> Obj::_splitLine(string line, const unsigned int length, const char seperator) {
    string rest = line;
    size_t splitter;

    vector<string> result(length);
    for (unsigned int i = 0; i < length; ++i) {
        splitter = rest.find_first_of(seperator);
        result[i] = rest.substr(0, splitter);
        rest = rest.substr(splitter + 1, rest.length() - splitter);
    }

    return result;
}

void Obj::_createDisplayLists() {
    unsigned int displayList;

    // Generate display lists for each group
    map<string, list< Face * > >::iterator it = this->_groupFaces.begin();
    for (; it != this->_groupFaces.end(); ++it) {
        // Generate a display list and save the reference to that list
        displayList = this->_createDisplayListForGroup((*it).first);
        this->_groupDisplayLists[(*it).first] = displayList;
    }
}

unsigned int Obj::_createDisplayListForGroup(string group) {
    unsigned int displayList;
    Material * material = 0;
    Face* face;
    vector<GLfloat>* texture;
    vector<GLfloat>* vertex;
    vector<GLfloat>* normal;
    list< Face * > * faces = &(this->_groupFaces[group]);
    list< Face * >::iterator it = faces->begin();

    float defaultAmbient[] = { 0.2, 0.2, 0.2, 1 };
    float defaultDiffuse[] = { 0.8, 0.8, 0.8, 1 };
    float defaultSpecular[] = { 0, 0, 0, 1 };
    float defaultEmission[] = { 0, 0, 0, 1 };
    float defaultShininess = 0;

    glLoadIdentity();

    // We're making a display list
    // .. so first we set the one from the static member
    displayList = Obj::_nextDisplayList;
    // .. and increment it for the next one
    Obj::_nextDisplayList++;

    glNewList(displayList, GL_COMPILE);

    // Iterate through all the faces in this group
    while (it != faces->end() ) {
        face = *it;

        // Check if we have a new material, and set things up for this new material
        if (material != face->material) {
            // Check if there is a texture, if not reset
            if (face->material != 0 && face->material->textureMap != 0) {
                glBindTexture(GL_TEXTURE_2D, face->material->textureMap);
            } else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            // Check if we have ambient color, if not reset to white
            if (face->material != 0 && face->material->ambientSet) {
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, face->material->ambient);
            } else {
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultAmbient);
            }

            // Check if we have diffuse color, if not reset to white
            if (face->material != 0 && face->material->diffuseSet) {
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face->material->diffuse);
            } else {
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultDiffuse);
            }

            // Check if we have diffuse color, if not reset to white
            if (face->material != 0 && face->material->specularSet) {
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, face->material->specular);
            } else {
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultSpecular);
            }

            // Check if we have a shininess value, otherwise set as default
            if (face->material != 0 && face->material->shininessSet) {
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, face->material->shininess);
            } else {
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, defaultShininess);
            }

            // Set the default emission
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultEmission);


            material = face->material;
        }

        // Draw a triangle
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; ++j) {

            normal = face->normals[j];
            glNormal3f((*normal)[0], (*normal)[1], (*normal)[2]);

            texture = face->textures[j];
            if (texture != 0) {
                // We need to convert obj texture corrdinates into openGL so X is 1 - X
                glTexCoord2f((*texture)[0], 1 - (*texture)[1]);
            }

            vertex = face->vertices[j];
            glVertex3f((*vertex)[0], (*vertex)[1], (*vertex)[2]);
        }
        glEnd();
        ++it;
    }

    // Stop making the list
    glEndList();

    return displayList;
}

void Obj::renderGroup(string group) {
    glCallList(this->_groupDisplayLists[group]);
}

void Obj::render() { 
    map<string, unsigned int>::iterator it = this->_groupDisplayLists.begin();
    for (; it != this->_groupDisplayLists.end(); ++it) {
        glCallList((*it).second);
    }
}

vector<GLfloat> Obj::getVertex(const unsigned int index) {
    vector<GLfloat> result;
    if (index < this->_vertices.size()) {
        result = (*this->_vertices[index]);
    } else {
        result.push_back(0);
        result.push_back(0);
        result.push_back(0);
    }
    return result;
}

void Obj::_addMTL(string line) {
    string texPath;

    // Get the file path
    path mtlPath(this->filename);
    mtlPath.remove_filename();

    // Add on the mtl file name
    vector<string> parts;
    split(parts, line, is_any_of(" "));

    char *buf = new char[100];
    getcwd(buf, 100);

    path filePath = buf / mtlPath / parts[1];

    // The material file is stateful, we need the current material
    Material* material = 0;

    // This is dirty, there is a load of white space on the end of the string
    string spath = filePath.string();
    trim_right(spath);
    const char* cpath = spath.c_str();

    // parse this file
    ifstream file(cpath);
    if (!file.is_open()) {
        perror("Error opening MTL file: ");
        return;
    }

    string mtlLine;
    while (!file.eof()) {
        getline(file, mtlLine);
        parts.clear();
        split(parts, mtlLine, is_any_of(" "));
        string command = parts[0];
        if (command == "newmtl") {
            // If there was a previous material we push it 
            if (material) {
                this->_materials.push_back(material);
            }
            material = new Material;
            trim(parts[1]);
            Obj::_initMaterial(material, parts[1]);
        } else if (command == "map_Kd") {
            // Load up the texture
            texPath = "";
            // Glue the parts together to form the path
            for (unsigned int i = 1; i < parts.size(); ++i) texPath += parts[i] + " ";
            
            // If the path is valid load a texture
            trim(texPath);
            if (texPath.size() > 0) {
                // Add the path
                texPath = mtlPath.string() + texPath;
                Obj::_loadTexture(texPath, material->textureMap);
            }
        } else if (command == "Ka") {
            // The ambient color is split into 3 floats
            for (int i = 1; i < 4; ++i) material->ambient[i - 1] = atof(parts[i].c_str());
            material->ambientSet = true;
        } else if (command == "Kd") {
            // The diffuse color is split into 3 floats
            for (int i = 1; i < 4; ++i) material->diffuse[i - 1] = atof(parts[i].c_str());
            material->diffuseSet = true;
        } else if (command == "Ks") {
            // The specular color is split into 3 floats
            for (int i = 1; i < 4; ++i) material->specular[i - 1] = atof(parts[i].c_str());
            material->specularSet = true;
        } else if (command == "Ns") {
            // Shininess is just a single float
            material->shininess = atof(parts[1].c_str());
            material->shininessSet = true;
        }
    }
    file.close();
    if (material) {
        this->_materials.push_back(material);
    }
}

void Obj::_initMaterial(Material * material, string name) {
    material->name = name;
    material->textureMap = 0;
    material->ambientSet = false;
    material->diffuseSet = false;
    material->specularSet = false;
    material->shininessSet = false;
    
    // Set alpha in material properties
    material->ambient[3] = 1;
    material->diffuse[3] = 1;
    material->specular[3] = 1;
}

Material* Obj::_findMaterial(string name) {
    // iterate through the materials looking for this name
    list< Material * >::iterator it = this->_materials.begin();
    for (; it != this->_materials.end(); ++it) {
        if ((*it)->name == name) {
            return *it;
        }
    }
    return 0;
}

void Obj::calculateBounds(Matrix *transformationMatrix, float *bounds) {
    // Transform all the vertices into the right space.
    // ... and get the max / mins for each dimension
    float* result = new float[4];
    float* vertex = new float[4];
    vector<GLfloat>* pointer;
    vertex[3] = 1;

    // We reset the bounds on the first iteration. This could be done by comparing
    // it to vertices->begin() but this way should be faster.
    bool resetBounds = true;

    list< vector<GLfloat>* >::iterator it = this->_verticesList.begin();
    for (; it != this->_verticesList.end(); ++it) {
        pointer = *it;
        
        // Transform the vertex
        vertex[0] = (*pointer)[0]; vertex[1] = (*pointer)[1]; vertex[2] = (*pointer)[2];
        //matrixMultiply(transformationMatrix, vertex, result);
        transformationMatrix->multiplyVector(vertex, result);

        // Reset the bounds on the first vertex
        if (resetBounds) {
            bounds[0] = result[0];
            bounds[1] = result[0];
            bounds[2] = result[1];
            bounds[3] = result[1];
            bounds[4] = result[2];
            bounds[5] = result[2];
            resetBounds = false;
            continue;
        }

        // Check if it is the largest / smallest of each dimension
        if (result[0] < bounds[0]) {
            bounds[0] = result[0];
        }
        if (result[0] > bounds[1]) {
            bounds[1] = result[0];
        }
        if (result[1] < bounds[2]) {
            bounds[2] = result[1];
        }
        if (result[1] > bounds[3]) {
            bounds[3] = result[1];
        }
        if (result[2] < bounds[4]) {
            bounds[4] = result[2];
        }
        if (result[2] > bounds[5]) {
            bounds[5] = result[2];
        }
    }

    // clean up
    delete vertex;
    delete result;
}

void Obj::_loadTexture(string texturePath, GLuint & texture) {
    // Try and load the image
    SDL_Surface * surface;
    int nOfColours;
    GLenum textureFormat = 0;
    if ((surface = IMG_Load(texturePath.c_str()))) {
        // Check that width and height are powers of 2
        if ((surface->w & (surface->w - 1)) != 0 ) {
            cout << "Warning: width not power of 2 " << texturePath << endl;
            SDL_FreeSurface(surface);
            return;
        }
        if ((surface->h & (surface->h -1)) != 0) {
            cout << "Warning: height not power of 2 " << texturePath << endl;
            SDL_FreeSurface(surface);
            return;
        }

        // Get the number of channels in the SDL surface
        nOfColours = surface->format->BytesPerPixel;
        if (nOfColours == 4) {
            if (surface->format->Rmask == 0x000000ff) {
                textureFormat = GL_RGBA;
            } else {
                textureFormat = GL_BGRA;
            }
        } else if (nOfColours == 3) {
            if (surface->format->Rmask == 0x000000ff) {
                textureFormat = GL_RGB;
            } else {
                textureFormat = GL_BGR;
            }
        }
        // Have opengl generate a texture object
        glGenTextures(1, &texture);

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, texture);

        // Set the texture's stretching properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Write the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, nOfColours, surface->w, surface->h, 0, 
                textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

        // Free the surface
        SDL_FreeSurface(surface);
    } else {
        cout << "With: " << texturePath << endl;
        cout << "Error loading texture: " << IMG_GetError() << "_" << endl;
    }
}
