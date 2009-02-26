/**
 * Perform view frustrum culling using the MODELVIEW and PROJETION matrices. This is 
 * available as a singleton.
 *
 * References: 
 * 	* http://www.lighthouse3d.com/opengl/maths/index.php?planes
 * 	* http://racer.nl/reference/vfc_markmorley.htm
 */
#pragma once

#include "matrix.h"

class ViewFrustrumCulling {
    public:
        // Refresh the MODELVIEW and PROJECTION matrices
        void refreshMatrices();

        // Test if an object is in the frustrum, return TRUE if it is.
        bool testObject(float * boundingBox);

        static ViewFrustrumCulling * culler;

    private:
        float _frustrum[6][4];
        Matrix _matrix;

};
