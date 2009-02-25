#include "dof.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <OpenGL/gl.h>

using namespace std;

Dof::Dof(string filePath) {
    // 4 characters used for tokens such as DOF1 and 32 bit integers
    char buffer[5];
    int dofLength, matsLength, matsChunkLength;
    int matLength;
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
    cout << "Reading a DOF of length: " << dofLength << endl;

    // Read the object components
    // .. first up should be the MATS
    file.read(buffer, 4);
    if (strcmp(buffer, "MATS") != 0) {
        cout << "Warning: MATS was expected but not found" << endl;
    }
    file.read((char *)&matsChunkLength, sizeof(int));
    cout << "MATS Chunk length: " << matsChunkLength << endl;

    file.read((char *)&matsLength, sizeof(int));
    cout << "MATS Length: " << matsLength << endl;

    // For now we skip the materials
    for (int i = 0; i < matsLength; ++i) {
        // read the token
        file.read(buffer, 4);
        if (strcmp(buffer, "MAT0") != 0) {
            cout << "Warning: MAT0[" << i << "] expected, got " << buffer << endl;
        }

        // Read this length
        file.read((char *)&matLength, sizeof(int));
        cout << "MAT length: " << matLength << endl;

        // skip this mat
        file.seekg(matLength, ios::cur);
    }

    // Now read the GEOB components
    file.read(buffer, 4);
    if (strcmp(buffer, "GEOB") != 0) {
        cout << "Warning: GEOB was expected but got " << buffer << endl;
    }

    // Parse the GEOBs
    this->_parseGeobs(&file);

    // close the file
    file.close();
}

Dof::~Dof() {
    // Delete the geobs
    if (this->_nGeobs != 0) delete[] this->_geobs;
}

void Dof::_parseGeobs(ifstream * file) {
    Geob * geob;
    int geobChunkLength, geobLength;
    int length, chunkLength;
    float tmpFloat;
    char token[5];

    // We need to add a null to the token so that we can print and compare the string
    token[4] = NULL;

    file->read((char *)&geobChunkLength, sizeof(int));
    cout << "GEOB Chunk length: " << geobChunkLength << endl;

    file->read((char *)&(this->_nGeobs), sizeof(int));
    cout << "GEOB Length: " << this->_nGeobs << endl;

    // Create a number of geob instances
    this->_geobs = new Geob[this->_nGeobs];

    // Create all the geobs
    for (unsigned int i = 0; i < this->_nGeobs; ++i) {
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
                // 2: int materialRef, the material reference TODO
                file->seekg(3 * sizeof(int), ios::cur);
            } else if (strcmp(token, "INDI") == 0) {
                // Parse the indices, not sure what these are for, an index which is global 
                // to DOF for the vertices?
                file->read((char *)&length, sizeof(int));
                // ... we'll skip this for now TODO
                file->seekg(length * sizeof(short), ios::cur);
            } else if (strcmp(token, "VERT") == 0) {
                // These are the vertices
                file->read((char *)&length, sizeof(int));
                geob->nVertices = length;
                geob->vertices = new float[length][3];

                // Read the vertices
                for (int j = 0; j < length; ++j) {
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->vertices[j][0] = tmpFloat;
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->vertices[j][1] = tmpFloat;
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->vertices[j][2] = tmpFloat;
                }
            } else if (strcmp(token, "TVER") == 0) {
                // Read the texture coordinates
                file->read((char *)&length, sizeof(int));
                geob->nTextureCoords = length;
                geob->textureCoords = new float[length][2];

                // Read the vertices
                for (int j = 0; j < length; ++j) {
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->textureCoords[j][0] = tmpFloat;
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->textureCoords[j][1] = tmpFloat;
                }
            } else if (strcmp(token, "NORM") == 0) {
                // These are the normals
                file->read((char *)&length, sizeof(int));
                geob->nNormals = length;
                geob->normals = new float[length][3];

                // Read the vertices
                for (int j = 0; j < length; ++j) {
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->normals[j][0] = tmpFloat;
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->normals[j][1] = tmpFloat;
                    file->read((char *)&tmpFloat, sizeof(float));
                    geob->normals[j][2] = tmpFloat;
                }
            } else if (strcmp(token, "VCOL") == 0) {
                // We're ignoring this for now
                file->read((char *)&length, sizeof(int));
                file->seekg(length * 3 * sizeof(float), ios::cur);
            } else if (strcmp(token, "BRST") == 0) {
                // We're ignoring this for now
                file->read((char *)&length, sizeof(int));
                file->seekg(length * sizeof(int), ios::cur);
                file->seekg(length * sizeof(int), ios::cur);
                file->seekg(length * sizeof(int), ios::cur);
                file->seekg(length * sizeof(int), ios::cur);
            } else if (strcmp(token, "GEND") == 0) {
                // ignore this
            } else {
                cout << "Warning: unknown token in GOB1, " << token << endl;
                break;
            }
        } while (strcmp(token, "GEND") != 0);
    }
}

void Dof::render() {
    Geob * geob;
    glBegin(GL_TRIANGLES);

    for (int i = 0; i < this->_nGeobs; ++i) {
        geob = &(this->_geobs[i]);
        glVertex3f(geob->vertices[0][0], geob->vertices[0][1], geob->vertices[0][2]);
        glVertex3f(geob->vertices[1][0], geob->vertices[1][1], geob->vertices[1][2]);
        glVertex3f(geob->vertices[2][0], geob->vertices[2][1], geob->vertices[2][2]);
        cout << "V: " << geob->vertices[0][0] << endl;
    }

    glEnd();
}


Geob::~Geob() {
    // Delete the various arrays
    if (this->nIndices != 0) delete[] this->indices;
    if (this->nVertices != 0) delete[] this->vertices;
    if (this->nNormals != 0) delete[] this->normals;
    if (this->nTextureCoords != 0) delete[] this->textureCoords;
}
