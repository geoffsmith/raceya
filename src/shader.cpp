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
    // TODO: find out if enabled mipmap is actually the default
    this->isMipmap = true;
    this->textureMap = 0;
    this->wrapT = GL_REPEAT;
    this->alphaFunc = 0;
    this->alphaFuncSet = false;
}

void Shader::parseShaderFile(string shaderPath) {
    string line;
    string name;
    vector<string> parts;
    Shader * shader = NULL;
    path shaderDirectory;

    shaderDirectory = shaderPath;
    shaderDirectory.remove_filename();

    // Check the file exists
    if (!exists(shaderPath)) {
        cout << "File doesn't exist: " << shaderPath << endl;
    }

    // Read the file
    ifstream file(shaderPath.c_str());
    if (!file.is_open()) {
        cout << "Couldn't open file: " << shaderPath << endl;
    }

    while (!file.eof()) {
        getline(file, line);
        trim(line);

        // If the line starts with a ;, ignore
        if (line[0] == ';') continue;

        // Check if this is a shader, in which case we create a new shader
        if (starts_with(line, "shader_")) {
            // First we load the textureMap now that we have everything
            if (shader != NULL && shader->textureMapPath.size() > 0) {
                loadTexture(shader->textureMapPath, 
                        shader->textureMap, 
                        shader->isMipmap);
            }

            // Parse out the shader's name
            split(parts, line, is_any_of("_"));
            name = parts[1];

            shader = new Shader();
            Shader::_shaders[name] = shader;
            continue;
        }

        // If we dont have a shader here, we need to skip the line
        if (shader == NULL) continue;

        if (starts_with(line, "texenv")) {
            // set the texenv on this shader
            split(parts, line, is_any_of("="));
            if (parts[1] == "replace") {
                shader->texEnv = GL_REPLACE;
            }
        } else if (starts_with(line, "map")) {
            // Load a texture map
            split(parts, line, is_any_of("="));
            shader->textureMapPath = (shaderDirectory / parts[1]).string();
        } else if (starts_with(line, "mipmap")) {
            split(parts, line, is_any_of("="));
            if (parts[1] == "1") {
                shader->isMipmap = true;
            }
        } else if (starts_with(line, "wrap_t")) {
            // Set the texture wrapping in the T direction
            split(parts, line, is_any_of("="));
            if (parts[1] == "clamp_to_edge") {
                shader->wrapT = GL_CLAMP_TO_EDGE;
            }
        } else if (starts_with(line, "alphafunc")) {
            split(parts, line, is_any_of(" "));
            shader->alphaFunc = atoi(parts[1].c_str());
            shader->alphaFuncSet = true;
        }
    }
}

Shader * Shader::getShader(string name) {
    vector<string> parts;
    // Strip the file type from the file name
    trim(name);
    split(parts, name, is_any_of("."));

    // Check if we have this shader
    if (Shader::_shaders.count(parts[0]) == 1) {
        return Shader::_shaders[parts[0]];
    } else {
        return NULL;
    }
}
