/**
 * Parse racer shader files. For now it is overly simple and can only read a
 * single layer.
 *
 * Possible bugs / missing features:
 *  * If map is defined before mipmap, it won't be loaded as a mipmap
 *  * getShader assumes only a single '.' in a filename
 *  * getShader assumes the file casing is right
 *  * Shader assumes a gequal alpha function
 *  * Only have clamp to edge for wrapT
 *  * Enable / disable writing to the depth buffer
 */
#pragma once

#include "texture.h"
#include "ini.h"

#include <string>
#include <map>

using namespace std;

class ShaderLayer {
    public:
        ShaderLayer();
        Texture * texture;
        string textureMapPath;
        bool isMipmap;
};

class Shader {
    public:
        Shader(); 
        string name;

        // The shader layers
        ShaderLayer ** layers;
        int nLayers;

    Texture * texture;
    string textureMapPath;

        // Texture stuff
        unsigned int texEnv;

        unsigned int wrapT;

        // Alpha stuff
        // This assume a gequal function
        bool blend;
        unsigned int alphaFunc;
        bool alphaFuncSet;

        // If this is a sky shader
        bool isSky;

        // Static method and members so we have access to shaders from 
        // anywhere
        static Shader * getShader(string name);
        static void parseShaderFile(string file);
        static void _parseLayers(string path, Ini & ini, Shader & shader);



    private:
        static map<string, Shader *> _shaders;
};
