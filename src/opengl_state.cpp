#include "opengl_state.h"
#include "logger.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <iostream>

using namespace std;

OpenGLState OpenGLState::global;

void OpenGLState::reset() {
    // Set the state for a display list
    glDisable(GL_TEXTURE_2D);
    this->texture2d = false;

    glDisable(GL_BLEND);
    this->blend = false;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, 0);
    this->texture = 0;

    glDisable(GL_ALPHA_TEST);
    this->alphaTest = false;

    glEnable(GL_DEPTH_TEST);
    this->depthTest = true;

    glDisable(GL_CULL_FACE);
    this->culling = 0;

    // NOTE: It looks like it's quicker just to enable this from the start and not
    // keep enabling / disabling all the time. However, alpha testing seems to be
    // really slow
    glDisable(GL_ALPHA_TEST);
    //glEnable(GL_ALPHA_TEST);
    this->alphaFunction = 1;
}

void OpenGLState::setCulling(int culling) {
    if (this->culling != culling) {
        // enable or disable culling
        if (this->culling == 0 && (culling == 1 || culling == 2)) {
            glEnable(GL_CULL_FACE);
        } else if (culling == 0) {
            glDisable(GL_CULL_FACE);
        }

        // Set the face
        if (this->culling == 1) {
            glCullFace(GL_BACK);
        }

        if (this->culling == 2) {
            glCullFace(GL_FRONT);
        }

        // Save the new state
        this->culling = culling;
    }
}

void OpenGLState::setAlpha(int function, int value) {
    GLenum func;
    GLclampf ref;

    // Check if the function is different
    if (function != this->alphaFunction) {
        // Decide if we need to enable / disable alphafunc
        if (this->alphaFunction == 1) {
            glEnable(GL_ALPHA_TEST);
        } else if (function == 1) {
            glDisable(GL_ALPHA_TEST);
        }

        // Change the actual function
        switch (function) {
            case 0:
                func = GL_NEVER;
                break;
            case 1:
                func = GL_ALWAYS;
                break;
            case 2:
                func = GL_LESS;
                break;
            case 3:
                func = GL_LEQUAL;
                break;
            case 4:
                func = GL_EQUAL;
                break;
            case 5:
                func = GL_GEQUAL;
                break;
            case 6:
                func = GL_GREATER;
                break;
            case 7:
                func = GL_NOTEQUAL;
                break;
            default:
                func = GL_ALWAYS;
                Logger::warn << "Bad alphaFunction" << endl;
        }

        // Get the clamp
        ref = value;

        // Set up the new function
        glAlphaFunc(func, ref);

        // Reset the current function
        this->alphaFunction = function;
        this->alphaValue = value;
    }
}
