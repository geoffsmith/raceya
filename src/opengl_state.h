/**
 * Keep track of the openGL state and manage the state changes. This is faster than
 * querying the API all the time and it means we can nicely encapsulate state checks 
 * and changes.
 */
#pragma once

#include "shader.h"

#include <list>

using namespace std;

class OpenGLState {
    public:
        bool alphaTest;
        bool texture2d;
        bool depthTest;
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float emission[4];
        unsigned int texture;

        // The blending src and dsts
        int * blendSrcs;
        int * blendDsts;
        bool blend;

        // The current texgen values
        int * texGenRs;
        int * texGenSs;
        int * texGenTs;

        // A global openGL state
        static OpenGLState global;

        // Reset the state to default
        void reset();

        // Culling
        // Same as shader: 0: none, 1: back, 2: front
        int culling;
        void setCulling(int culling);

        // Alpha blending function and value
        // 0: never, 1: always / none, 2: less [value], 3: lequal [value],
        // 4: equal [value], 5: gequal [value], 6: greater [value], 7: notequal [value]
        int alphaFunction;
        int alphaValue;
        void setAlpha(int function, int value);

        // Manage the state of the texture objects. We use multitexturing, so either a 
        // single texture can be set as current, or several can be and multitextuing will
        // be used
        // Keep track of which GL_TEXTUREi_ARB is being used
        int maxTextures;
        int * multiTextures;
        int * currentTextures;
        // So we know which textures we need to disable
        int lastUsedTextures;

        // Set an array of textures
        void setTextures(list<ShaderLayer *> * layers);
        void setTexture(int texture);
};
