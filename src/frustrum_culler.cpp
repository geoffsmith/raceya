#include "frustrum_culler.h"
#include <math.h>
#include <iostream>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

using namespace std;

ViewFrustrumCulling * ViewFrustrumCulling::culler = new ViewFrustrumCulling();

bool ViewFrustrumCulling::testObject(float * boundingBox) {
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

    // Now we check if the sphere is outside of each frustrum plane
    for (int i = 0; i < 6; ++i) {
        if (this->_frustrum[i][0] * center[0] + 
                this->_frustrum[i][1] * center[1] + 
                this->_frustrum[i][2] * center[2] + 
                this->_frustrum[i][3] <= -1 * size)
            return false;
    }
    return true;
}

void ViewFrustrumCulling::refreshMatrices() {
    float modelView[16];
    float projection[16];
    float t;
    float (* frustrum)[4] = this->_frustrum;

    // Load the MODELVIEW and PROJETION matrices
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    // Combine the two matrices
    this->_matrix.reset();
    this->_matrix.multiplyMatrix(projection);
    this->_matrix.multiplyMatrix(modelView);

    // Calculate the frustrum
    // ... for the right plane
    frustrum[0][0] = this->_matrix[3] - this->_matrix[0];
    frustrum[0][1] = this->_matrix[7] - this->_matrix[4];
    frustrum[0][2] = this->_matrix[11] - this->_matrix[8];
    frustrum[0][3] = this->_matrix[15] - this->_matrix[12];

    // and normalise
    t = sqrt(frustrum[0][0] * frustrum[0][0] + frustrum[0][1] * frustrum[0][1] 
            + frustrum[0][2] * frustrum[0][2]);
    frustrum[0][0] /= t;
    frustrum[0][1] /= t;
    frustrum[0][2] /= t;
    frustrum[0][3] /= t;

    // ... for the left plane
    frustrum[1][0] = this->_matrix[3] + this->_matrix[0];
    frustrum[1][1] = this->_matrix[7] + this->_matrix[4];
    frustrum[1][2] = this->_matrix[11] + this->_matrix[8];
    frustrum[1][3] = this->_matrix[15] + this->_matrix[12];

    // and normalise
    t = sqrt(frustrum[1][0] * frustrum[1][0]
            + frustrum[1][1] * frustrum[1][1] 
            + frustrum[1][2] * frustrum[1][2]);
    frustrum[1][0] /= t;
    frustrum[1][1] /= t;
    frustrum[1][2] /= t;
    frustrum[1][3] /= t;

    // ... for the botton plane
    frustrum[2][0] = this->_matrix[3] + this->_matrix[1];
    frustrum[2][1] = this->_matrix[7] + this->_matrix[5];
    frustrum[2][2] = this->_matrix[11] + this->_matrix[9];
    frustrum[2][3] = this->_matrix[15] + this->_matrix[13];

    // and normalise
    t = sqrt(frustrum[2][0] * frustrum[2][0]
            + frustrum[2][1] * frustrum[2][1] 
            + frustrum[2][2] * frustrum[2][2]);
    frustrum[2][0] /= t;
    frustrum[2][1] /= t;
    frustrum[2][2] /= t;
    frustrum[2][3] /= t;

    // ... for the top plane
    frustrum[3][0] = this->_matrix[3] - this->_matrix[1];
    frustrum[3][1] = this->_matrix[7] - this->_matrix[5];
    frustrum[3][2] = this->_matrix[11] - this->_matrix[9];
    frustrum[3][3] = this->_matrix[15] - this->_matrix[13];

    // and normalise
    t = sqrt(frustrum[3][0] * frustrum[3][0] 
            + frustrum[3][1] * frustrum[3][1] 
            + frustrum[3][2] * frustrum[3][2]);
    frustrum[3][0] /= t;
    frustrum[3][1] /= t;
    frustrum[3][2] /= t;
    frustrum[3][3] /= t;

    // ... for the far plane
    frustrum[4][0] = this->_matrix[3] - this->_matrix[2];
    frustrum[4][1] = this->_matrix[7] - this->_matrix[6];
    frustrum[4][2] = this->_matrix[11] - this->_matrix[10];
    frustrum[4][3] = this->_matrix[15] - this->_matrix[14];

    // and normalise
    t = sqrt(frustrum[4][0] * frustrum[4][0] 
            + frustrum[4][1] * frustrum[4][1] 
            + frustrum[4][2] * frustrum[4][2]);
    frustrum[4][0] /= t;
    frustrum[4][1] /= t;
    frustrum[4][2] /= t;
    frustrum[4][3] /= t;

    // ... for the near plane
    frustrum[5][0] = this->_matrix[3] + this->_matrix[2];
    frustrum[5][1] = this->_matrix[7] + this->_matrix[6];
    frustrum[5][2] = this->_matrix[11] + this->_matrix[10];
    frustrum[5][3] = this->_matrix[15] + this->_matrix[14];

    // and normalise
    t = sqrt(frustrum[5][0] * frustrum[5][0] 
            + frustrum[5][1] * frustrum[5][1] 
            + frustrum[5][2] * frustrum[5][2]);
    frustrum[5][0] /= t;
    frustrum[5][1] /= t;
    frustrum[5][2] /= t;
    frustrum[5][3] /= t;
}
