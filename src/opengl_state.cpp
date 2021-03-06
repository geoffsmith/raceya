#include "opengl_state.h"
#include "logger.h"
#include "texture.h"
#include "lib.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <boost/foreach.hpp>

using namespace std;

OpenGLState OpenGLState::global;

void OpenGLState::reset() {
    // Set the state for a display list
    glDisable(GL_TEXTURE_2D);
    this->texture2d = false;


    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, 0);
    this->texture = 0;

    glDisable(GL_ALPHA_TEST);
    this->alphaTest = false;

    glEnable(GL_DEPTH_TEST);
    this->depthTest = true;

    glDisable(GL_CULL_FACE);
    this->culling = 0;

    glDisable(GL_BLEND);
    //glEnable(GL_BLEND);
    this->blend = false;

    // NOTE: It looks like it's quicker just to enable this from the start and not
    // keep enabling / disabling all the time. However, alpha testing seems to be
    // really slow
    //glDisable(GL_ALPHA_TEST);
    glEnable(GL_ALPHA_TEST);
    this->alphaFunction = 1;

    // Set up the multi texture stuff
    this->maxTextures = 5;

    this->multiTextures = new int[this->maxTextures];
    this->currentTextures = new int[this->maxTextures];
    this->texGenRs = new int[this->maxTextures];
    this->texGenSs = new int[this->maxTextures];
    this->texGenTs = new int[this->maxTextures];
    this->blendSrcs = new int[this->maxTextures];
    this->blendDsts = new int[this->maxTextures];

    for (int i = 0; i < this->maxTextures; ++i)  {
        this->multiTextures[i] = GL_TEXTURE0 + i;
        this->currentTextures[i] = 0;

        this->texGenRs[i] = GL_OBJECT_LINEAR;
        this->texGenSs[i] = GL_OBJECT_LINEAR;
        this->texGenTs[i] = GL_OBJECT_LINEAR;

        this->blendSrcs[i] = GL_SRC_ALPHA;
        this->blendDsts[i] = GL_ONE_MINUS_SRC_ALPHA;
    }
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

    // Check if the function is different
    if (function != this->alphaFunction) {
        // Decide if we need to enable / disable alphafunc
        if (this->alphaFunction == 1) {
            glEnable(GL_ALPHA_TEST);
        } else if (function == 1) {
            glDisable(GL_ALPHA_TEST);
        }

        // Change the actual function
        GLenum func;
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
        GLclampf ref = value / 255.0;
        std::cout << "clamp: " << ref << std::endl;

        // Set up the new function
        glAlphaFunc(func, ref);

        // Reset the current function
        this->alphaFunction = function;
        this->alphaValue = value;
    }
}

void OpenGLState::setTexture(int texture) {
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Now go through disabling anything thats left
    for (int i = 1; i < this->lastUsedTextures; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
    }
    this->lastUsedTextures = 1;
}

void OpenGLState::setTextures(const list<ShaderLayer *> & layers) {
    int index = 0;

    // Iterate through the textures to set
    BOOST_FOREACH (ShaderLayer * layer, layers) {
        Texture * texture = layer->texture;

        // Skip if the current texture is not valid, NOTE, this will skip ++index too
        if (texture == NULL) {
            continue;
        }

        glActiveTexture(GL_TEXTURE0 + index);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, texture->texture);
        this->currentTextures[index] = texture->texture;

        // Set the texture environment
        // This should only really be used when we multi-pass
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, layer->texEnv);

        // Set the texgen
        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, layer->texGenR);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, layer->texGenS);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, layer->texGenT);

        // Set the wrapping
        // This doesn't seem to help at all
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, layer->wrapT);

        // If the bottom layer has a blend function, we use it
        if (index == 0 && layer->blend) {
            glEnable(GL_BLEND);
            glBlendFunc(layer->blendSrc, layer->blendDst);
        } else {
            glDisable(GL_BLEND);
        }

        ++index;
    }

    // Now go through disabling anything thats left
    for (int i = index; i < this->lastUsedTextures; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
    }

    this->lastUsedTextures = index;
}
