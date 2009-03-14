#pragma once

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <SDL/SDL.h>
#include <string>

using namespace std;

void read_obj(const char *filename, GLfloat (*vertices)[3], GLfloat (*textures)[2], GLfloat (*normals)[3]);

void printError();

/******************************************************************************
 * OpenGL vector math helpers
 *****************************************************************************/
void matrixMultiply(float* matrix, float* vector, float* result);
void buildYRotationMatrix(float *matrix, float angle);
float angleBetweenVectors(float * vector1, float * vector2);


// Calculate the angle between two vectors in the Z plane
float angleInZPlane(float * vector1, float * vector2);

// Calculate the angle between the two vectors in the plane defined by planeVector and the
// Y axis
float angleInPlaneY(float * vector1, float * vector2, float * planeVector);
float angleInPlaneZ(float * vector1, float * vector2, float * planeVector);
float angleInPlane(float * vector1, float * vector2, float * planeVector, float * planeVector2);

float dotProduct(float * a, float * b);
void vertexSub(float * a, float * b, float * result);
void vertexCopy(float * from, float * to);
void crossProduct(float * a, float * b, float * result);
void vertexMultiply(float a, float * b, float * result);
void vertexAdd(float * a, float * b, float * result);
float vertexSquareDistance(float * a, float * b);
float vectorLength(float * a);
bool normaliseVector(float * a);
bool vectorEquals(float * a, float * b);
float vertexDistance(float * a, float * b);
bool colorEquals4(float * a, float * b);
bool colorCopy4(float * from, float * to);

void printVector(float * a);

// Flip a SDL_Surface horizontally
void horizontalFlipSurface(SDL_Surface * surface);
