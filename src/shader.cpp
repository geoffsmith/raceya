#include "shader.h"
#include "lib.h"

#include <OpenGL/gl.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

// Initialise static members
map<string, Shader *> Shader::_shaders;

Shader::Shader() {
    this->texEnv = NULL;
    this->wrapT = GL_REPEAT;
    this->alphaFunc = 0;
    this->alphaFuncSet = false;
    this->blend = false;
    this->texture = NULL;

    // By default a shader is not sky
    this->isSky = false;
}

void Shader::parseShaderFile(string shaderPath) {
    list<string> matches;
    list<string>::iterator it;
    Ini ini(shaderPath);
    string name;
    string value;
    vector<string> parts;
    Shader * shader;

    // Go through each shader object
    ini.query("/", matches);
    for (it = matches.begin(); it != matches.end(); ++it) {
        // Parse out the shader's name
        split(parts, *it, is_any_of("/"));
        name = parts[1];
        trim(name);

        // Create a new shader
        shader = new Shader();
        Shader::_shaders[name] = shader;

        // Check if this shader is sky
        value = ini[*it + "/sky"];
        if (!value.empty() && value == "1") {
            shader->isSky = true;
        }


        // Load the shader layers
        Shader::_parseLayers(*it, ini, *shader);
    }
}

void Shader::_parseLayers(string iniPath, Ini & ini, Shader & shader) {
    list<string> matches;
    list<string>::iterator it;
    ShaderLayer * layer;
    int index = 0;
    string value;

    // Get the path of the shader ini file
    path iniFilePath(ini.path);
    iniFilePath.remove_filename();

    ini.queryTokens(iniPath + "/layer", matches);

    // Now we know how many layers this shader has
    shader.nLayers = matches.size();
    shader.layers = new ShaderLayer *[shader.nLayers];

    for (it = matches.begin(); it != matches.end(); ++it) {
        layer = new ShaderLayer();
        shader.layers[index] = layer;

        //  Attempt to load the mipmap setting for this layer
        value = ini[iniPath + "/" + *it + "/mipmap"];
        if (!value.empty() && value == "0") {
            layer->isMipmap = false;
        }

        // Check for a texture on this layer
        value = ini[iniPath + "/" + *it + "/map"];
        if (!value.empty()) {
            layer->texture = Texture::getOrMakeTexture((iniFilePath / value).string(), 
                    layer->isMipmap);
        }

        ++index;
    }
}

Shader * Shader::getShader(string name) {
    map<string, Shader * >::iterator it;
    vector<string> parts;
    string tmp;
    // Strip the file type from the file name
    trim(name);
    to_lower(name);
    split(parts, name, is_any_of("."));

    // Check if we have this shader
    for (it = Shader::_shaders.begin(); it != Shader::_shaders.end(); ++it) {
        // Strip the "shader_ part out"
        tmp = it->first.substr(7);
        if (tmp == parts[0]) {
            return Shader::_shaders[it->first];
        }
    }

    cout << "Shader NOT found: " << name << endl;
    return NULL;
}

/******************************************************************************
 * ShaderLayer stuff
 *****************************************************************************/
ShaderLayer::ShaderLayer() {
    this->isMipmap = true;
}
