#pragma once

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <string>

using namespace std;

void loadTexture(string name, unsigned int & texture, bool isMipmap = false, unsigned int wrapT = GL_REPEAT);
void read_obj(const char *filename, GLfloat (*vertices)[3], GLfloat (*textures)[2], GLfloat (*normals)[3]);


/******************************************************************************
 * OpenGL vector math helpers
 *****************************************************************************/
void matrixMultiply(float* matrix, float* vector, float* result);
void buildYRotationMatrix(float *matrix, float angle);
float angleBetweenVectors(float * vector1, float * vector2);
