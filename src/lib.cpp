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
    float dotProduct = 
        vector1[0] * vector2[0] + 
        vector1[1] * vector2[1] + 
        vector1[2] * vector2[2];
    float absVector1 = sqrt(
        vector1[0] * vector1[0] + 
        vector1[1] * vector1[1] + 
        vector1[2] * vector1[2]);
    float absVector2 = sqrt(
        vector2[0] * vector2[0] + 
        vector2[1] * vector2[1] + 
        vector2[2] * vector2[2]);
    // make sure we don't divide by zero
    if (absVector1 == 0 || absVector2 == 0) {
        cout << "Error: Divide by zero in angleBetweenVectors-> " 
            << absVector1 << ", " << absVector2 << endl;
        return 0;
    }

    // Calculate angle and convert into degrees
    float tmp = dotProduct / (absVector1 * absVector2);
    if (fabs(tmp) >= 1) {
        return 0;
    } else {
        float result = (acos(tmp) * 180.0) / PI;
        if (isnan(result))
        {
            cout << "result: " << tmp << endl;
        }
        return result;
    }
}

float angleInZPlane(float * vector1, float * vector2) {
    // Project vectors onto x plane (saving the z-values)
    float result;
    float z1 = vector1[2];
    float z2 = vector2[2];

    // NOTE: I'm not convinced this is a good idea. It's quicker than copying the vectors
    // to something on the stack, but makes this function side effect in a strange way.
    vector1[2] = 0;
    vector2[2] = 0;

    result = angleBetweenVectors(vector1, vector2); 
    // Restore the z-values
    vector2[2] = z1;
    vector2[2] = z2;
    return result;
}

float angleInPlaneZ(float * vector1, float * vector2, float * planeVector) {
    float zVector[3];
    zVector[0] = 0;
    zVector[1] = 0;
    zVector[2] = 1;
    return angleInPlane(vector1, vector2, planeVector, zVector);
}

float angleInPlaneY(float * vector1, float * vector2, float * planeVector) {
    float yVector[3];
    yVector[0] = 0;
    yVector[1] = 1;
    yVector[2] = 0;
    return angleInPlane(vector1, vector2, planeVector, yVector);
}

float angleInPlane(float * vector1, float * vector2, float * planeVector, float * planeVector2) {
    // We need to project vector1 and vector2 onto the plane defined by 
    // planeVector x { 0, 1, 0 }
    float tmp1[3];
    float tmp2[3];
    float tmpn[3];
    float n[3];
    float t;
    float angle;

    // Calculate the plane
    crossProduct(planeVector, planeVector2, n);

    // Check that the normal vector isn't 0
    if (n[0] * n[0] + n[1] * n[1] + n[2] * n[2] == 0) {
        return 0;
    }
    normaliseVector(n);

    // Do the projection, d = 0 because the plane goes through the origin
    // R = Q - (n . Q) . n
    // .. first for vector1
    t = dotProduct(n, vector1);
    vertexMultiply(t, n, tmp1);
    vertexSub(vector1, tmp1, tmp1);

    // .. then for the escond one
    t = dotProduct(n, vector2);
    vertexMultiply(t, n, tmp2);
    vertexSub(vector2, tmp2, tmp2);

    // .. now we find the angle between those two
    angle = angleBetweenVectors(tmp1, tmp2);
    if (isnan(angle)) 
    {
        cout << "Angle is nan: " << angle << endl;
        cout << tmp1[0] << " : " << tmp2[0] << endl;
        return 0;
    }

    // We work out the sign on the angle by whether the cross product of the tmp1 and tmp2
    // matches the direction of n
    crossProduct(tmp1, tmp2, tmpn);
    // Avoid the chance of tmp1 = tmp2
    if (tmpn[0] * tmpn[0] + tmpn[1] * tmpn[1] + tmpn[2] * tmpn[2] != 0) {
        normaliseVector(tmpn);
        // Swap the sign of the angle if the normals dont' match
        if (vertexDistance(tmpn, n) < 0.000001) angle *= -1;
    }

    return angle;
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
    result[1] =  a[0] * b[2] - b[0] * a[2];
    result[2] =  a[0] * b[1] - b[0] * a[1];
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

float vectorLength(float * a) {
    return sqrt(a[0] * a[0] + a[1] * a[1]  + a[2] * a[2]);
}

bool normaliseVector(float * a) {
    float length = vectorLength(a);
    if (length == 0) return false;
    a[0] /= length;
    a[1] /= length;
    a[2] /= length;
    return true;
}

void printVector(float * a) {
    cout << "{ " << a[0] << ", " << a[1] << ", " << a[2] << "}" << endl;
}

bool vectorEquals(float * a, float * b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

void horizontalFlipSurface(SDL_Surface * surface) {
    // The documentation says to lock the surface, I think this is to make sure the 
    // pointer to pixels doesnt' change. Probably OK, as there shouldn't me > 1 thread
    // on this
    SDL_LockSurface(surface);

    // We use int as a 4 byte type, this is not that neat, should probably use char,
    // but not sure whether memcpy is the thing to use
    int tmp;
    int * pixels = (int *)surface->pixels;
    int width = surface->w;
    int height = surface->h;
    int a, b;

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height / 2; ++j) {
            a = j * width + i;
            b = (height - 1 - j) * width + i;
            tmp = pixels[a];
            pixels[a]  = pixels[b];
            pixels[b] = tmp;
        }
    }
    SDL_UnlockSurface(surface);
}
