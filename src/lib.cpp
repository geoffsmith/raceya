#include "lib.h"

#include <math.h>
#include <iostream>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define PI 3.14159265

using namespace std;

void loadTexture(string name, unsigned int & texture, bool isMipmap, unsigned int wrapT) {
    // Try and load the image
    SDL_Surface * surface;
    SDL_Surface * alphaSurface;
    int nOfColours;
    GLenum textureFormat = 0;
    unsigned int error = glGetError();
    
    if ((surface = IMG_Load(name.c_str()))) {
        // Make it transparent
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
        if (isMipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

        // Write the texture data
        if ((error = glGetError()) != 0) {
            cout << "Error before loading texture: " << gluErrorString(error) << endl;
            texture = 0;
        }

        if (isMipmap) {
            gluBuild2DMipmaps(GL_TEXTURE_2D, nOfColours, surface->w, surface->h,
                    textureFormat, GL_UNSIGNED_BYTE, surface->pixels);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, nOfColours, surface->w, surface->h, 
                    0, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);
        }

        // TODO: check the flags for loading Mipmaps

        if ((error = glGetError()) != 0) {
            cout << "Error loading texture into OpenGL: " << gluErrorString(error) << endl;
            texture = 0;
        }


        // Free the surface
        SDL_FreeSurface(surface);
    } else {
        cout << "With: " << name << endl;
        cout << "Error loading texture: " << IMG_GetError() << "_" << endl;
        texture = 0;
    }
}

void read_obj(const char *filename, GLfloat (*vertices)[3], GLfloat (*textures)[2], GLfloat (*normals)[3]) {
}

void matrixMultiply(float* matrix, float* vector, float* result) {
    float sum;
    for (int i = 0; i < 4; ++i) {
        sum = 0;
        for (int j = 0; j < 4; ++j) {
            sum += matrix[j * 4 + i] * vector[j];
        }
        result[i] = sum;
    }
}

void buildYRotationMatrix(float *matrix, float angle) {
    matrix[0] = cos(angle);
    matrix[1] = 0;
    matrix[2] = -1 * sin(angle);
    matrix[3] = 0;

    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;

    matrix[8] = sin(angle);
    matrix[9] = 0;
    matrix[10] = cos(angle);
    matrix[11] = 0;

    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

float angleBetweenVectors(float * vector1, float * vector2) {
    // Calculate the dot product
    float dotProduct = vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];
    float absVector1 = sqrt(vector1[0] * vector1[0] + vector1[1] * vector1[1] + vector1[2] * vector1[2]);
    float absVector2 = sqrt(vector2[0] * vector2[0] + vector2[1] * vector2[1] + vector2[2] * vector2[2]);
    // make sure we don't divide by zero
    if (absVector1 == 0 || absVector2 == 0) {
        cout << "Error: Divide by zero in angleBetweenVectors-> " 
            << absVector1 << ", " << absVector2 << endl;
        return 0;
    }
    // Calculate angle and convert into degrees
    return (acos(dotProduct / (absVector1 * absVector2)) * 180.0) / PI;
}

float dotProduct(float * a, float * b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vertexSub(float * a, float * b, float * result) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

void vertexCopy(float * from, float * to) {
    to[0] = from[0];
    to[1] = from[1];
    to[2] = from[2];
}

void crossProduct(float * a, float * b, float * result) {
    result[0] =  a[1] * b[2] - b[1] * a[2];
    result[0] =  a[0] * b[2] - b[0] * a[2];
    result[0] =  a[0] * b[1] - b[0] * a[1];
}

void vertexMultiply(float a, float * b, float * result) {
    result[0] = a * b[0];
    result[1] = a * b[1];
    result[2] = a * b[2];
}

void vertexAdd(float * a, float * b, float * result) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
}

float vertexDistance(float * a, float * b) {
    float tmp[3];
    vertexSub(b, a, tmp);
    return sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
}

float vertexSquareDistance(float * a, float * b) {
    float tmp[3];
    vertexSub(b, a, tmp);
    return tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2];
}
