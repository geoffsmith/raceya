#include "frustum_culler.h"
#include <math.h>
#include <iostream>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

using namespace std;

ViewFrustumCulling * ViewFrustumCulling::culler = new ViewFrustumCulling();

bool ViewFrustumCulling::testObject(float * boundingBox) {
    float center[3];
    float sizeVector[3];
    float size;

    // First we need the center of the box
    center[0] = (boundingBox[0] + boundingBox[1]) / 2.0;
    center[1] = (boundingBox[2] + boundingBox[3]) / 2.0;
    center[2] = (boundingBox[4] + boundingBox[5]) / 2.0;

    sizeVector[0] = boundingBox[0] - center[0];
    sizeVector[1] = boundingBox[2] - center[1];
    sizeVector[2] = boundingBox[4] - center[2];

    // And now get the size of the size vector
    size = sqrt(
            sizeVector[0] * sizeVector[0] + 
            sizeVector[1] * sizeVector[1] + 
            sizeVector[2] * sizeVector[2] );

    // Now we check if the sphere is outside of each frustum plane
    for (int i = 0; i < 6; ++i) {
        if (this->_frustum[i][0] * center[0] + 
                this->_frustum[i][1] * center[1] + 
                this->_frustum[i][2] * center[2] + 
                this->_frustum[i][3] <= -1 * size)
            return false;
    }
    return true;
}

void ViewFrustumCulling::refreshMatrices() {
    float modelView[16];
    float projection[16];
    float t;
    float (* frustum)[4] = this->_frustum;

    // Load the MODELVIEW and PROJETION matrices
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    // Combine the two matrices
    this->_matrix.reset();
    this->_matrix.multiplyMatrix(projection);
    this->_matrix.multiplyMatrix(modelView);

    // Calculate the frustum
    // ... for the right plane
    frustum[0][0] = this->_matrix[3] - this->_matrix[0];
    frustum[0][1] = this->_matrix[7] - this->_matrix[4];
    frustum[0][2] = this->_matrix[11] - this->_matrix[8];
    frustum[0][3] = this->_matrix[15] - this->_matrix[12];

    // and normalise
    t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] 
            + frustum[0][2] * frustum[0][2]);
    frustum[0][0] /= t;
    frustum[0][1] /= t;
    frustum[0][2] /= t;
    frustum[0][3] /= t;

    // ... for the left plane
    frustum[1][0] = this->_matrix[3] + this->_matrix[0];
    frustum[1][1] = this->_matrix[7] + this->_matrix[4];
    frustum[1][2] = this->_matrix[11] + this->_matrix[8];
    frustum[1][3] = this->_matrix[15] + this->_matrix[12];

    // and normalise
    t = sqrt(frustum[1][0] * frustum[1][0]
            + frustum[1][1] * frustum[1][1] 
            + frustum[1][2] * frustum[1][2]);
    frustum[1][0] /= t;
    frustum[1][1] /= t;
    frustum[1][2] /= t;
    frustum[1][3] /= t;

    // ... for the botton plane
    frustum[2][0] = this->_matrix[3] + this->_matrix[1];
    frustum[2][1] = this->_matrix[7] + this->_matrix[5];
    frustum[2][2] = this->_matrix[11] + this->_matrix[9];
    frustum[2][3] = this->_matrix[15] + this->_matrix[13];

    // and normalise
    t = sqrt(frustum[2][0] * frustum[2][0]
            + frustum[2][1] * frustum[2][1] 
            + frustum[2][2] * frustum[2][2]);
    frustum[2][0] /= t;
    frustum[2][1] /= t;
    frustum[2][2] /= t;
    frustum[2][3] /= t;

    // ... for the top plane
    frustum[3][0] = this->_matrix[3] - this->_matrix[1];
    frustum[3][1] = this->_matrix[7] - this->_matrix[5];
    frustum[3][2] = this->_matrix[11] - this->_matrix[9];
    frustum[3][3] = this->_matrix[15] - this->_matrix[13];

    // and normalise
    t = sqrt(frustum[3][0] * frustum[3][0] 
            + frustum[3][1] * frustum[3][1] 
            + frustum[3][2] * frustum[3][2]);
    frustum[3][0] /= t;
    frustum[3][1] /= t;
    frustum[3][2] /= t;
    frustum[3][3] /= t;

    // ... for the far plane
    frustum[4][0] = this->_matrix[3] - this->_matrix[2];
    frustum[4][1] = this->_matrix[7] - this->_matrix[6];
    frustum[4][2] = this->_matrix[11] - this->_matrix[10];
    frustum[4][3] = this->_matrix[15] - this->_matrix[14];

    // and normalise
    t = sqrt(frustum[4][0] * frustum[4][0] 
            + frustum[4][1] * frustum[4][1] 
            + frustum[4][2] * frustum[4][2]);
    frustum[4][0] /= t;
    frustum[4][1] /= t;
    frustum[4][2] /= t;
    frustum[4][3] /= t;

    // ... for the near plane
    frustum[5][0] = this->_matrix[3] + this->_matrix[2];
    frustum[5][1] = this->_matrix[7] + this->_matrix[6];
    frustum[5][2] = this->_matrix[11] + this->_matrix[10];
    frustum[5][3] = this->_matrix[15] + this->_matrix[14];

    // and normalise
    t = sqrt(frustum[5][0] * frustum[5][0] 
            + frustum[5][1] * frustum[5][1] 
            + frustum[5][2] * frustum[5][2]);
    frustum[5][0] /= t;
    frustum[5][1] /= t;
    frustum[5][2] /= t;
    frustum[5][3] /= t;
}
