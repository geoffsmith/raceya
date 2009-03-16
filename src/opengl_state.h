/**
 * Keep track of the openGL state and manage the state changes. This is faster than
 * querying the API all the time and it means we can nicely encapsulate state checks 
 * and changes.
 */
#pragma once

class OpenGLState {
    public:
        bool alphaTest;
        bool blend;
        bool texture2d;
        bool depthTest;
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float emission[4];
        unsigned int texture;

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
};
