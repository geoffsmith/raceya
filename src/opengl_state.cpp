#include "opengl_state.h"
#include "logger.h"
#include "texture.h"

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

    // Set up the multi texture stuff
    this->maxTextures = 5;

    this->multiTextures = new int[this->maxTextures];
    this->currentTextures = new int[this->maxTextures];
    this->texGenRs = new int[this->maxTextures];
    this->texGenSs = new int[this->maxTextures];
    this->texGenTs = new int[this->maxTextures];

    for (int i = 0; i < this->maxTextures; ++i)  {
        this->multiTextures[i] = GL_TEXTURE0_ARB;
        this->currentTextures[i] = 0;

        this->texGenRs[i] = GL_OBJECT_LINEAR;
        this->texGenSs[i] = GL_OBJECT_LINEAR;
        this->texGenTs[i] = GL_OBJECT_LINEAR;
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

void OpenGLState::setTexture(int texture) {
    if (this->currentTextures[0] != texture || this->lastUsedTextures > 1) {
        glClientActiveTexture(0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
        this->currentTextures[0] = texture;
    }
    // Now go through disabling anything thats left
    for (int i = 1; i < this->lastUsedTextures; ++i) {
            glClientActiveTexture(this->multiTextures[i]);
            glBindTexture(GL_TEXTURE_2D, 0);
    }
    this->lastUsedTextures = 1;
}

void OpenGLState::setTextures(list<ShaderLayer *> * layers) {
    Texture * texture;
    ShaderLayer * layer;
    int index = 0;
    list<ShaderLayer *>::iterator it;

    // Iterate through the textures to set
    for (it = layers->begin(); it != layers->end(); ++it) {
        layer = *it;
        texture = layer->texture;

        // Skip if the current texture is not valid, NOTE, this will skip ++index too
        if (texture->texture == 0) continue;

        // Only set the texture if it is different
        if (this->currentTextures[index] != texture->texture) {
            glClientActiveTexture(this->multiTextures[index]);
            glBindTexture(GL_TEXTURE_2D, texture->texture);
            this->currentTextures[index] = texture->texture;
        }

        // Set the texenv
        if (index > 0) {
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_ADD);
        } else {
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
        }

        // Set the texgen
        if (this->texGenRs[index] != layer->texGenR) {
            this->texGenRs[index] = layer->texGenR;
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, layer->texGenR);
        }
        if (this->texGenSs[index] != layer->texGenS) {
            this->texGenSs[index] = layer->texGenS;
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, layer->texGenS);
        }
        if (this->texGenTs[index] != layer->texGenT) {
            this->texGenTs[index] = layer->texGenT;
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, layer->texGenT);
        }

        ++index;
    }

    // Now go through disabling anything thats left
    for (int i = index; i < this->lastUsedTextures; ++i) {
            glClientActiveTexture(this->multiTextures[i]);
            glBindTexture(GL_TEXTURE_2D, 0);
    }

    this->lastUsedTextures = index;
}
