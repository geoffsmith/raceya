#include "dof.h"
#include "frustum_culler.h"
#include "logger.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <math.h>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Dof::Dof(string filePath) {
    // 4 characters used for tokens such as DOF1 and 32 bit integers
    char buffer[5];
    int dofLength;
    this->_filePath = filePath;
    buffer[4] = NULL;
    
    // Load the file
    ifstream file(this->_filePath.c_str());
    if (!file.is_open()) {
        cout << "Error opening file: " << this->_filePath << endl;
        return;
    }

    // Read the header: DOF1 and its length
    file.read(buffer, 4);

    // Check that we have a DOF1 object
    if (strcmp(buffer, "DOF1") != 0) {
        cout << "Warning: not DOF1 token at start (was " << buffer << ")" << endl;
    }
    
    // Read the length of the DOF object
    file.read((char *)&dofLength, sizeof(int));

    // Read the object components
    // .. first up should be the MATS
    file.read(buffer, 4);
    if (strcmp(buffer, "MATS") != 0) {
        cout << "Warning: MATS was expected but not found" << endl;
    }

    this->_parseMats(&file);

    // Now read the GEOB components
    file.read(buffer, 4);
    if (strcmp(buffer, "GEOB") != 0) {
        cout << "Warning: GEOB was expected but got " << buffer << endl;
    }

    // Parse the GEOBs
    this->_parseGeobs(&file);

    // close the file
    file.close();

    // Create the display lists
    this->_createDisplayLists();
    
    // Calculate bounding box
    this->_calculateBoundingBox();
}

Dof::~Dof() {
    // Delete the geobs
    if (this->_nGeobs != 0) delete[] this->_geobs;
}

void Dof::_parseMats(ifstream * file) {
    Mat * mat;
    Shader * shader;
    int length;
    char token[5];
    char fileString[255];
    int fileStringLength;
    token[4] = NULL;

    // Read the MAT0 chunk length
    file->read((char *)&length, sizeof(int));
    // .. and the number of MAT0s
    file->read((char *)&(this->_nMats), sizeof(int));

    // Create all the mats
    this->_mats = new Mat[this->_nMats];

    // Create all the mats
    for (int i = 0; i < this->_nMats; ++i) {
        mat = &(this->_mats[i]);

        // read the token
        file->read(token, 4);
        if (strcmp(token, "MAT0")) {
            cout << "Warning: expected MAT0 (" << i << "), got " << token << endl;
        }

        // read the chunk length
        file->read((char *)&length, sizeof(int));

        // Parse the object component attributes
        do {
            // Get the token
            file->read(token, 4);
            // .. and the length
            if (strcmp(token, "MEND") != 0) {
                file->read((char *)&length, sizeof(int));
            }

            if (strcmp(token, "MHDR") == 0) {
                // The header just contains a string, skip this for now TODO
                file->seekg(length, ios::cur);
            } else if (strcmp(token, "MCOL") == 0) {
                // Contains the various material colors
                parseVector<float>(file, mat->ambient, 4);
                parseVector<float>(file, mat->diffuse, 4);
                parseVector<float>(file, mat->specular, 4);
                parseVector<float>(file, mat->emission, 4);
                file->read((char *)&(mat->shininess), sizeof(float));
            } else if (strcmp(token, "MTEX") == 0) {
                // Load the textures
                file->read((char *)&(mat->nTextures), sizeof(int));
                mat->textures = new unsigned int[mat->nTextures];
                mat->shaders = new Shader *[mat->nTextures];

                for (int j = 0; j < mat->nTextures; ++j) {
                    fileStringLength = parseString(file, fileString);

                    // Check if there is a shader for this file
                    shader = Shader::getShader(fileString);
                    if (shader != NULL) {
                        mat->textures[j] = shader->textureMap;
                        mat->shaders[j] = shader;
                    } else {
                        this->_loadTexture(fileString, mat->textures[j]);
                        mat->shaders[j] = NULL;
                    }

                }
            } else if (strcmp(token, "MUVW") == 0) {
                file->read((char *)&(mat->uvwUoffset), sizeof(float));
                file->read((char *)&(mat->uvwVoffset), sizeof(float));
                file->read((char *)&(mat->uvwUtiling), sizeof(float));
                file->read((char *)&(mat->uvwVtiling), sizeof(float));
                file->read((char *)&(mat->uvwAngle), sizeof(float));
                file->read((char *)&(mat->uvwBlur), sizeof(float));
                file->read((char *)&(mat->uvwBlurOffset), sizeof(float));
            } else if (strcmp(token, "MTRA") == 0) {
                // The transparency float isn't used
                file->seekg(sizeof(float), ios::cur);

                // get the blendMode
                file->read((char *)&(mat->blendMode), sizeof(int));
            } else if (strcmp(token, "MEND") == 0) {
                // do nothing
            } else {
                file->seekg(length, ios::cur);
            }
        } while (strcmp(token, "MEND") != 0);
    }
}

void Dof::_parseGeobs(ifstream * file) {
    Geob * geob;
    int geobChunkLength, geobLength;
    int length, chunkLength;
    char token[5];

    // We need to add a null to the token so that we can print and compare the string
    token[4] = NULL;

    file->read((char *)&geobChunkLength, sizeof(int));

    file->read((char *)&(this->_nGeobs), sizeof(int));

    // Create a number of geob instances
    this->_geobs = new Geob[this->_nGeobs];

    // Create all the geobs
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);

        // read the token, this should be GOB1
        file->read(token, 4);
        if (strcmp(token, "GOB1") != 0) {
            cout << "Warning: (" << i << ") expecting GOB1, got " << token << ". " << endl;
        }

        // Get this geob's length
        file->read((char *)&geobLength, sizeof(int));

        // Now we parse a number of object component attributes
        do {
            // Read the next token
            file->read(token, 4);
            // .. and its chunk length (if token is not GEND)
            if (strcmp(token, "GEND") != 0) {
                file->read((char *)&chunkLength, sizeof(int));
            }

            if (strcmp(token, "GHDR") == 0) {
                // skip the header
                // The header is made up of
                // 0: int flags (not defined in spec)
                // 1: int paintFlags (not defined)
                // 2: int materialRef, the material reference 

                // We ignore the first two
                file->seekg(2 * sizeof(int), ios::cur);
                file->read((char *)&(geob->material), sizeof(int));
            } else if (strcmp(token, "INDI") == 0) {
                // Parse the indices, not sure what these are for, an index which is global 
                // to DOF for the vertices?
                file->read((char *)&(geob->nIndices), sizeof(int));
                geob->indices = new unsigned short[geob->nIndices];
                for (int j = 0; j < geob->nIndices; ++j) {
                    file->read((char *)&(geob->indices[j]), sizeof(unsigned short));
                }

            } else if (strcmp(token, "VERT") == 0) {
                // These are the vertices
                file->read((char *)&length, sizeof(int));
                geob->nVertices = length;
                geob->vertices = new float[length][3];

                // Read the vertices
                for (int j = 0; j < length; ++j) {
                    parseVector<float>(file, geob->vertices[j], 3);
                }
            } else if (strcmp(token, "TVER") == 0) {
                // Read the texture coordinates
                file->read((char *)&length, sizeof(int));
                geob->nTextureCoords = length;
                geob->textureCoords = new float[length][2];

                // Read the vertices
                for (int j = 0; j < length; ++j) {
                    parseVector<float>(file, geob->textureCoords[j], 2);
                    // Flip the y
                    geob->textureCoords[j][1] *= -1;
                }
            } else if (strcmp(token, "NORM") == 0) {
                // These are the normals
                file->read((char *)&length, sizeof(int));
                geob->nNormals = length;
                geob->normals = new float[length][3];

                // Read the vertices
                for (int j = 0; j < length; ++j) {
                    parseVector<float>(file, geob->normals[j], 3);
                }
            } else if (strcmp(token, "VCOL") == 0) {
                // We're ignoring this for now
                file->read((char *)&length, sizeof(int));
                file->seekg(length * 3 * sizeof(float), ios::cur);
            } else if (strcmp(token, "BRST") == 0) {
                // We're ignoring this for now
                file->read((char *)&(geob->nBursts), sizeof(int));
                geob->burstStarts = new int[geob->nBursts];
                for (int j = 0; j < geob->nBursts; ++j) {
                    file->read((char *)&(geob->burstStarts[j]), sizeof(int));
                }
                
                geob->burstsCount = new int[geob->nBursts];
                for (int j = 0; j < geob->nBursts; ++j) {
                    file->read((char *)&(geob->burstsCount[j]), sizeof(int));
                }
                
                geob->burstsMaterials = new int[geob->nBursts];
                for (int j = 0; j < geob->nBursts; ++j) {
                    file->read((char *)&(geob->burstsMaterials[j]), sizeof(int));
                }
                //file->seekg(length * sizeof(int), ios::cur);
                file->seekg(geob->nBursts * sizeof(int), ios::cur);
            } else if (strcmp(token, "GEND") == 0) {
                // ignore this
            } else {
                cout << "Warning: unknown token in GOB1, " << token << endl;
                file->seekg(chunkLength, ios::cur);
                break;
            }
        } while (strcmp(token, "GEND") != 0);
    }
}

void Dof::_loadTexture(string name, unsigned int & texture) {
    // Try and load the image
    SDL_Surface * surface;
    SDL_Surface * alphaSurface;
    int nOfColours;
    GLenum textureFormat = 0;
    path texturePath(this->_filePath);
    unsigned int error = glGetError();
    
    texturePath.remove_filename();
    texturePath = texturePath / name;
    
    if ((surface = IMG_Load(texturePath.string().c_str()))) {
        SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 0, 255));
        alphaSurface = SDL_DisplayFormatAlpha(surface);

        SDL_FreeSurface(surface);
        surface = alphaSurface;

        // Check that width and height are powers of 2
        if ((surface->w & (surface->w - 1)) != 0 ) {
            cout << "Warning: width not power of 2 " << name << endl;
            SDL_FreeSurface(surface);
            return;
        }
        if ((surface->h & (surface->h -1)) != 0) {
            cout << "Warning: height not power of 2 " << name << endl;
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


        glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);

        // mix color with texture
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // Set the texture's stretching properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Write the texture data
        if ((error = glGetError()) != 0) {
            cout << "Error before loading texture: " << gluErrorString(error) << endl;
            texture = 0;
        }

        // TODO: check the flags for loading Mipmaps
        gluBuild2DMipmaps(GL_TEXTURE_2D,nOfColours, surface->w, surface->h, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

        if ((error = glGetError()) != 0) {
            cout << "Error loading texture into OpenGL: " << gluErrorString(error) << endl;
            texture = 0;
        }


        // Free the surface
        SDL_FreeSurface(surface);
    } else {
        Logger::debug << "Error loading texture: " << IMG_GetError() << endl;
        texture = 0;
    }
}

void Dof::_createDisplayLists() {
    Geob * geob;
    Mat * mat;
    unsigned short index;
    int burstCount, burstStart;
    int stop;
    unsigned int error;


    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        mat = &(this->_mats[geob->material]);

        // Create a new display list
        geob->displayList = glGenLists(1);
        glNewList(geob->displayList, GL_COMPILE);

        // Set the defaults
        glDisable(GL_ALPHA_TEST);

        if (this->isTransparent()) {
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }

        if (mat->nTextures > 0 && mat->textures[0]) {
            error = glGetError();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, mat->textures[0]);
            if ((error = glGetError()) != 0) {
                cout << "Error binding textre: " << gluErrorString(error) << endl;
            }
            
            // See if there is an alpha function to be set
            if (mat->shaders[0]) {
                if (mat->shaders[0]->alphaFuncSet) {
                    glEnable(GL_ALPHA_TEST);
                    glAlphaFunc(GL_EQUAL, mat->shaders[0]->alphaFunc);
                    //glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
                }
            }

        } else {
            glDisable(GL_TEXTURE_2D);
        }

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat->emission);
        
        // Set up the vertex pointers
        glEnableClientState(GL_VERTEX_ARRAY);
        // this probably wont' work because its a pointer to pointer
        glVertexPointer(3, GL_FLOAT, 0, geob->vertices);

        // ... and the normals
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, geob->normals);

        // ... and the texture coords
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, geob->textureCoords);


        for (int j = 0; j < geob->nBursts; ++j) {
            burstCount = geob->burstsCount[j] / 3;
            burstStart = geob->burstStarts[j] / 3;

            // Some files have fewer indices than bursts, which is really odd.
            // Racer's source doesn't handle this case, something really odd is 
            // going on there... 
            // It only happens with some tracks, so it might be a bug in a track 
            // generator.
            // stop = min(burstStart + burstCount, geob->nIndices) - burstStart;

            // Draw the elements
            glDrawElements(GL_TRIANGLES, burstCount, GL_UNSIGNED_SHORT, 
                    &(geob->indices[burstStart]));

        }


        glEndList();
    }


}

int Dof::render() {
    int count = 0;
    Geob * geob;

    this->_calculateBoundingBox();
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        // Check if we need to render this geob
        if (ViewFrustumCulling::culler->testObject(geob->boundingBox)) {
            // call the previously created display list
            glCallList(geob->displayList);
            ++count;
        }
    }
    return count;
}

bool Dof::isTransparent() {
    Mat * mat;
    Geob * geob;
    bool result = false;
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        mat = &(this->_mats[geob->material]);
        if (mat->blendMode > 0) {
            result = true;
        }
        // Check if this material has a shader
        if (mat->shaders[0] && mat->shaders[0]->blend) {
            result = true;
        }

        if (result) break;
    }
    return result;
}

Geob * Dof::getGeob(unsigned int index) {
    return &(this->_geobs[index]);
}

int Dof::getNGeobs() {
    return this->_nGeobs;
}

/****************************************************************************************
 * Bounding box and collision detection
 ***************************************************************************************/
void Dof::_calculateBoundingBox() {
    Geob * geob;
    float * vertex;
    // The first vertex is always set, so we have a starting point
    bool firstVertex;

    // Go through each geob and each vertex to find the min / max points for each
    // axis
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        firstVertex = true;
        for (int j = 0; j < geob->nIndices; ++j) {
            vertex = geob->vertices[geob->indices[j]];

            // If this is the first vertex, set the bounds
            if (firstVertex) {
                geob->boundingBox[0] = vertex[0];
                geob->boundingBox[1] = vertex[0];
                geob->boundingBox[2] = vertex[1];
                geob->boundingBox[3] = vertex[1];
                geob->boundingBox[4] = vertex[2];
                geob->boundingBox[5] = vertex[2];
                firstVertex = false;
            } else {
                // Now check if the vertex expands any of the bounds
                if (geob->boundingBox[0] > vertex[0]) geob->boundingBox[0] = vertex[0];
                if (geob->boundingBox[1] < vertex[0]) geob->boundingBox[1] = vertex[0];
                if (geob->boundingBox[2] > vertex[1]) geob->boundingBox[2] = vertex[1];
                if (geob->boundingBox[3] < vertex[1]) geob->boundingBox[3] = vertex[1];
                if (geob->boundingBox[4] > vertex[2]) geob->boundingBox[4] = vertex[2];
                if (geob->boundingBox[5] < vertex[2]) geob->boundingBox[5] = vertex[2];
            }
        }
    }
}

/****************************************************************************************
 * Utilities
 ***************************************************************************************/

template <class T> void parseVector(ifstream * file, T * vector, int length) {
    for (int i = 0; i < length; ++i) {
        file->read((char *)&(vector[i]), sizeof(T));
    }
}

int parseString(ifstream * file, char * buffer) {
    short length;
    file->read((char *)&length, sizeof(short));
    file->read(buffer, length);
    buffer[length] = NULL;
    return length;
}


Geob::~Geob() {
    // Delete the various arrays
    if (this->nIndices != 0) delete[] this->indices;
    if (this->nVertices != 0) delete[] this->vertices;
    if (this->nNormals != 0) delete[] this->normals;
    if (this->nTextureCoords != 0) delete[] this->textureCoords;
}
