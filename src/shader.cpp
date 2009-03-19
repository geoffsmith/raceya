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
        shader->name = name;
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
    vector<string> parts;
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
            layer->textureMapPath = (iniFilePath / value).string();
            // Check if this is a special texture and starts with $
            if (!starts_with(value, "$")) {
                layer->texture = Texture::getOrMakeTexture(
                        layer->textureMapPath, 
                        layer->isMipmap);
            }
        }

        // Get the culling value if there is one
        value = ini[iniPath + "/" + *it + "/cull"];
        if (value == "front") {
            layer->culling = 2;
        } else if (value == "back") {
            layer->culling = 1;
        }

        // See if there is some alpha function
        value = ini[iniPath + "/" + *it + "/alphafunc"];
        if (!value.empty()) {
            split(parts, value, is_any_of(" "));
            if (parts[0] == "never") layer->alphaFunction = 0;
            else if (parts[0] == "always") layer->alphaFunction = 1;
            else if (parts[0] == "none") layer->alphaFunction = 1;
            else if (parts[0] == "less") layer->alphaFunction = 2;
            else if (parts[0] == "lequal") layer->alphaFunction = 3;
            else if (parts[0] == "equal") layer->alphaFunction = 4;
            else if (parts[0] == "gequal") layer->alphaFunction = 5;
            else if (parts[0] == "greater") layer->alphaFunction = 6;
            else if (parts[0] == "notequal") layer->alphaFunction = 7;

            // Save the value if there is one
            if (parts.size() > 1) {
                layer->alphaValue = atoi(parts[1].c_str());
            }
        }

        // See if there is a texgen_{s,t,r}
        Shader::_checkForTexGen(ini, iniPath + "/" + *it + "/texgen_r", layer->texGenR);
        Shader::_checkForTexGen(ini, iniPath + "/" + *it + "/texgen_s", layer->texGenS);
        Shader::_checkForTexGen(ini, iniPath + "/" + *it + "/texgen_t", layer->texGenT);

        // Get the blending function
        Shader::_checkForBlendFunc(ini[iniPath + "/" + *it + "/blendfunc"], layer);

        // Get the texture wrapping
        Shader::_checkForTextureWrap(ini[iniPath + "/" + *it + "/wrap_t"], layer->wrapT);

        // Get texture environment
        Shader::_checkForTextureEnv(ini[iniPath + "/" + *it + "/texenv"], layer->texEnv);

        ++index;
    }
}

void Shader::_checkForTextureEnv(string value, int & result) {
    if (value.empty()) return;

    if (value == "replace") result = GL_REPLACE;
    else if (value == "disable") result = GL_REPLACE;
}

void Shader::_checkForTextureWrap(string value, int & result) {
    if (value.empty()) return;

    if (value == "clamp_to_edge") {
        result = GL_CLAMP_TO_EDGE;
    }
}

void Shader::_checkForBlendFunc(string value, ShaderLayer * layer) {
    vector<string> parts;
    int result;
    split(parts, value, is_any_of(" "));

    // Enable if value is not empty
    if (!value.empty()) {
        layer->blend = true;
    } else {
        return;
    }

    // If there is only one part, this is an abbreviation, otherwise it specifies both
    // src and dst
    if (parts.size() == 1) {
        if (parts[0] == "add") {
            layer->blendSrc = GL_ONE;
            layer->blendDst = GL_ONE;
        } else if (parts[0] == "filter") {
            layer->blendSrc = GL_ZERO;
            layer->blendDst = GL_SRC_COLOR;
        } else if (parts[0] == "blend") {
            layer->blendSrc = GL_SRC_ALPHA;
            layer->blendDst = GL_ONE_MINUS_SRC_ALPHA;
        }
    } else {
        for (int i = 0; i < 2; ++i) {
            if (parts[i] == "zero") result = GL_ZERO;
            else if (parts[i] == "one") result = GL_ONE;
            else if (parts[i] == "dst_color") result = GL_DST_COLOR;
            else if (parts[i] == "one_minus_dst_color") result = GL_ONE_MINUS_DST_COLOR;
            else if (parts[i] == "src_color") result = GL_SRC_COLOR;
            else if (parts[i] == "one_minus_src_color") result = GL_ONE_MINUS_SRC_COLOR;
            else if (parts[i] == "src_alpha") result = GL_SRC_ALPHA;
            else if (parts[i] == "one_minus_src_alpha") result = GL_ONE_MINUS_SRC_ALPHA;
            else if (parts[i] == "dst_alpha") result = GL_DST_ALPHA;
            else if (parts[i] == "one_minus_dst_alpha") result = GL_ONE_MINUS_DST_ALPHA;
            else if (parts[i] == "src_alpha_saturate") result = GL_SRC_ALPHA_SATURATE;

            if (i == 0) {
                layer->blendSrc = result;
            }
            else {
                layer->blendDst = result;
            }
        } 
    }
}

void Shader::_checkForTexGen(Ini & ini, string type, int & result) {
    string value = ini[type];
    if (value == "object_linear") result = GL_OBJECT_LINEAR;
    else if (value == "reflection_map") result = GL_REFLECTION_MAP;
    else if (value == "sphere_map") result = GL_SPHERE_MAP;
}

Shader * Shader::getShader(string name) {
    map<string, Shader * >::iterator it;
    vector<string> parts;
    string tmp;
    // Strip the file type from the file name
    trim(name);
    to_lower(name);
    split(parts, name, is_any_of("."));

    // Check if we have this shader, based on the material name
    for (it = Shader::_shaders.begin(); it != Shader::_shaders.end(); ++it) {
        // Strip the "shader_ part out"
        if (it->first.size() < 7) {
            //cout << "out of range shader: " << it->first << endl;
            continue;
        }
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
    this->texture = NULL;
    this->isMipmap = true;

    // Default culling to none
    this->culling = 0;

    // Default alpha function is none
    this->alphaFunction = 1;
    this->alphaValue = 0;

    this->texGenR = GL_OBJECT_LINEAR;
    this->texGenS = GL_OBJECT_LINEAR;
    this->texGenT = GL_OBJECT_LINEAR;

    //this->blendSrc = GL_SRC_ALPHA;
    //this->blendDst = GL_ONE_MINUS_SRC_ALPHA;
    this->blendSrc = GL_ONE;
    this->blendDst = GL_ZERO;
    this->blend = false;

    this->wrapS = GL_REPEAT;
    this->wrapT = GL_REPEAT;

    this->texEnv = GL_MODULATE;
}
