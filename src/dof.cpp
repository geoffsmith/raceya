#include "dof.h"
#include "frustum_culler.h"
#include "logger.h"
#include "lib.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <math.h>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Dof::Dof(string filePath, int flags, bool perGeobDisplayList) {
    // 4 characters used for tokens such as DOF1 and 32 bit integers
    char buffer[5];
    int dofLength;
    this->_filePath = filePath;
    this->_perGeobDisplayList = perGeobDisplayList;
    this->_flags = flags;
    buffer[4] = NULL;
    this->isValid = false;

    this->_geobs = NULL;
    
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

    // Generate VAOs
    for (int i = 0; i < this->_nGeobs; ++i) {
        this->_geobs[i].generateVAO();
    }
    
    // Calculate bounding box
    this->_calculateBoundingBox();

    // If we got here it is valid
    this->isValid = true;
}

Dof::~Dof() {
    // Delete the geobs
    if (this->_geobs != NULL) delete[] this->_geobs;
}

void Dof::_parseMats(ifstream * file) {
    Mat * mat;
    Shader * shader;
    int length;
    char token[5];
    char fileString[255];
    int fileStringLength;
    token[4] = NULL;
    path texturePath(this->_filePath);
    texturePath.remove_filename();
    string textureName;

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
                // Get the material name
                parseString(file, fileString);
                mat->name = fileString;
                // Ignore the material class
                parseString(file, fileString);
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
                mat->textures = new Texture *[mat->nTextures];
                mat->shaders = new Shader *[mat->nTextures];

                for (int j = 0; j < mat->nTextures; ++j) {
                    fileStringLength = parseString(file, fileString);

                    // Check if there is a shader for this file
                    shader = Shader::getShader(fileString);
                    if (shader != NULL) {
                        mat->textures[j] = shader->texture;
                        mat->shaders[j] = shader;
                    } else {
                        textureName = (texturePath / fileString).string();
                        mat->textures[j] = Texture::getOrMakeTexture(textureName);
                        mat->shaders[j] = 0;
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
        geob->dof = this;

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

void Dof::_renderGeob(Geob * geob, Mat * & previousMat) {
    Mat * mat;
    int burstCount, burstStart;
    int stop;

    if (geob->material < this->_nMats) {
        mat = &(this->_mats[geob->material]);
    } else {
        Logger::warn << "Error loading material, skipping..." << endl;
        return;
    }

    this->loadMaterial(mat);

    // Bind to VAO
    //glBindVertexArrayAPPLE(geob->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geob->vertexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geob->indexVBO);
    glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
    glNormalPointer(GL_FLOAT, 0, 
            (GLvoid*)((char*)NULL + geob->nVertices * 3 * sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, 0, 
            (GLvoid*)((char*)NULL + geob->nVertices * 3 * sizeof(float) + geob->nNormals * 3 * sizeof(float)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);


    for (int j = 0; j < geob->nBursts; ++j) {
        burstCount = geob->burstsCount[j] / 3;
        burstStart = geob->burstStarts[j] / 3;

        // NOTE: I would have expected the burst material to be valid here, but it 
        // doesn't seem to work. Not sure what it's for...

        // Some files have fewer indices than bursts, which is really odd.
        // Racer's source doesn't handle this case, something really odd is 
        // going on there... 
        // It only happens with some tracks, so it might be a bug in a track 
        // generator.
        stop = min(burstStart + burstCount, geob->nIndices) - burstStart;

        // Do some sanity checks
        if (burstStart > geob->nIndices) {
            cout << "Error start past indices" << endl;
            continue;
        }

        // Draw the elements
        //glLockArraysEXT(0, geob->nVertices);
        //glDrawElements(GL_TRIANGLES, stop, GL_UNSIGNED_SHORT, 
                //&(geob->indices[burstStart]));
        //glUnlockArraysEXT();
        glDrawElements(GL_TRIANGLES, stop, GL_UNSIGNED_SHORT, 
                (GLvoid*)((char *)(0)));
    }
}

void Dof::_initialiseMaterials() {
    // Set the state for a display list
    glDisable(GL_TEXTURE_2D);
    this->_renderState.texture2d = false;

    glDisable(GL_BLEND);
    this->_renderState.blend = false;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, 0);
    this->_renderState.texture = 0;

    glDisable(GL_ALPHA_TEST);
    this->_renderState.alphaTest = false;
}

void Dof::loadMaterial(Mat * mat) {
    // Set the defaults
    if (this->_renderState.alphaTest) {
        glDisable(GL_ALPHA_TEST);
        this->_renderState.alphaTest = false;

    }

    // Check if this material has a shader
    if (mat->blendMode > 0 || 
       (mat->nTextures > 0 && mat->shaders[0] != NULL && mat->shaders[0]->blend)) {
        if (!this->_renderState.blend) {
            glEnable(GL_BLEND);
            this->_renderState.blend = true;
        }
    } else {
        if (this->_renderState.blend) {
            glDisable(GL_BLEND);
            this->_renderState.blend = false;
        }
    }

    if (mat->nTextures > 0 && mat->textures[0]->texture) {
        if (!this->_renderState.texture2d) {
            glEnable(GL_TEXTURE_2D);
            this->_renderState.texture2d = true;
        }
        if (this->_renderState.texture != mat->textures[0]->texture) {
            Texture * texture = mat->textures[0];
            glBindTexture(GL_TEXTURE_2D, texture->texture);
            this->_renderState.texture = texture->texture;
        }
        
        // See if there is an alpha function to be set
        if (mat->shaders != NULL && mat->shaders[0]) {
            if (mat->shaders[0]->alphaFuncSet) {
                glEnable(GL_ALPHA_TEST);
                //glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
                this->_renderState.alphaTest = true;
                glAlphaFunc(GL_EQUAL, mat->shaders[0]->alphaFunc);
            }
        }
    } else {
        if (this->_renderState.texture2d) {
            glDisable(GL_TEXTURE_2D);
            glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            this->_renderState.texture2d = false;
        }
    }

    if (colorEquals4(this->_renderState.ambient, mat->ambient)) {
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
        colorCopy4(mat->ambient, this->_renderState.ambient);

    }

    // Check if we need to change the diffuse material
    if (colorEquals4(this->_renderState.diffuse, mat->diffuse)) {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
        colorCopy4(mat->diffuse, this->_renderState.diffuse);
    }

    if (colorEquals4(this->_renderState.specular, mat->specular)) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
        colorCopy4(mat->specular, this->_renderState.specular);
    }

    if (colorEquals4(this->_renderState.emission, mat->emission)) {
        glMaterialfv(GL_FRONT, GL_EMISSION, mat->emission);
        colorCopy4(mat->emission, this->_renderState.emission);
    }
}

int Dof::render(bool overrideFrustrumTest) {
    int count = 0;
    Geob * geob;
    Mat * mat;
    Mat * previousMat = NULL;

    //this->_calculateBoundingBox();

    this->_initialiseMaterials();

    // First we render the non-transparent geobs
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        mat = &(this->_mats[geob->material]);

        if (!mat->isTransparent()) {
            // Check if we need to render this geob
            if (overrideFrustrumTest || 
                    ViewFrustumCulling::culler->testObject(geob->boundingBox)) {
                // call the previously created display list
                this->_renderGeob(geob, previousMat);
                ++count;
            }
        }
    }

    // .. Now we render the transparent geobs
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        mat = &(this->_mats[geob->material]);

        if (mat->isTransparent()) {
            // Check if we need to render this geob
            if (overrideFrustrumTest ||
                    ViewFrustumCulling::culler->testObject(geob->boundingBox)) {
                // call the previously created display list
                this->_renderGeob(geob, previousMat);
                ++count;
            }
        }
    }
    return count;
}

bool Dof::isTransparent() {
    Mat * mat;
    Geob * geob;
    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        mat = &(this->_mats[geob->material]);

        if (mat->isTransparent()) {
            return true;
        }
    }
    return false;
}

Geob * Dof::getGeob(unsigned int index) {
    return &(this->_geobs[index]);
}

int Dof::getNGeobs() {
    return this->_nGeobs;
}

Mat * Dof::getMats() { return this->_mats; }
int Dof::getNMats() { return this->_nMats; }

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

bool Dof::isSurface() {
    return this->_flags & DOF_SURFACE;
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


Geob::Geob() {
    this->indices = NULL;
    this->vertices = NULL;
    this->normals = NULL;
    this->textureCoords = NULL;
}
Geob::~Geob() {
    // Delete the various arrays
    if (this->indices != NULL) delete[] this->indices;
    if (this->vertices != NULL) delete[] this->vertices;
    if (this->normals != NULL) delete[] this->normals;
    if (this->textureCoords != NULL) delete[] this->textureCoords;
}

void Geob::generateVAO() {
    Mat * mat;

    if (this->material < this->dof->getNMats()) {
        mat = &(this->dof->getMats()[this->material]);
    } else {
        Logger::warn << "Error loading material, skipping..." << endl;
        return;
    }

    //this->dof->loadMaterial(mat);

    // SEt up the VAO
    //glGenVertexArraysAPPLE(1, &(this->vao));
    //glBindVertexArrayAPPLE(this->vao);
    glGenBuffers(1, &(this->vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexVBO);
    const GLsizeiptr vSize = this->nVertices * 3 * sizeof(float);
    const GLsizeiptr nSize = this->nNormals * 3 * sizeof(float);
    const GLsizeiptr tSize = this->nTextureCoords * 2 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, vSize + nSize + tSize, NULL, GL_STATIC_DRAW);

    // Copy the data
    /*GLvoid * buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(buffer, this->vertices, vSize);
    buffer = (GLvoid *)((char *)buffer + vSize);
    memcpy(buffer, this->normals, nSize);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    */
    glBufferSubData(GL_ARRAY_BUFFER, 0, vSize, this->vertices);
    glBufferSubData(GL_ARRAY_BUFFER, vSize, nSize, this->normals);
    glBufferSubData(GL_ARRAY_BUFFER, vSize + nSize, tSize, this->textureCoords);

    glGenBuffers(1, &(this->indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->nIndices * sizeof(unsigned short), this->indices, GL_STATIC_DRAW);
}

Mat::Mat() {
    this->shaders = NULL;
}

bool Mat::isTransparent() {
    if (this->blendMode > 0) {
        return true;
    }
    // Check if this material has a shader
    if (this->nTextures > 0 && this->shaders[0] != NULL && this->shaders[0]->blend) {
        return true;
    }

    return false;
}
