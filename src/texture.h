/**
 * Class representing a texture. Each instance contains link to GLuint texture object
 * and there is a static map for looking up existing textures
 *
 * TODO:
 * 	* Get min / mag etc from settings
 */
#pragma once

#include <map>
#include <string>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;


class Texture {
    public:
        GLenum format;
        int nOfColours;
        int width;
        int height;
        unsigned int texture;
	bool isMipmap;
	string name;

	Texture(string name, bool isMipmap);

	// Keep track of all the textures loaded, so we don't load them multiple times
        static map<string, Texture * > textures;

	// Check Texture::textures for a texture matching this name, if one is found
	// return it, otherwise create one and save to textures
	static Texture * getOrMakeTexture(string name, bool isMipmap=true);

    private:
	// Load a texture into a texture object
	void _loadTexture(string name);
};
