#include "texture.h"
#include "logger.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

map<string, Texture * > Texture::textures;

Texture::Texture(string name, bool isMipmap) {
    this->name = name;
    this->isMipmap = isMipmap;
    this->_loadTexture(name);
}

void Texture::_loadTexture(string name) {
    // Try and load the image
    SDL_Surface * surface;
    SDL_Surface * alphaSurface;
    int nOfColours;
    GLenum textureFormat = 0;
    unsigned int error = glGetError();
    
    std::string realFilename = Texture::findRealFileName(name);
    if ((surface = IMG_Load(realFilename.c_str()))) {
        SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 255, 0, 255));
        alphaSurface = SDL_DisplayFormatAlpha(surface);

        SDL_FreeSurface(surface);
        surface = alphaSurface;

        // Check that width and height are powers of 2
        if ((surface->w & (surface->w - 1)) != 0 ) {
            Logger::warn << "Warning: width not power of 2 " << name << endl;
            SDL_FreeSurface(surface);
            return;
        }
        if ((surface->h & (surface->h -1)) != 0) {
            Logger::warn << "Warning: height not power of 2 " << name << endl;
            SDL_FreeSurface(surface);
            return;
        }

        // Get the number of channels in the SDL surface
        nOfColours = surface->format->BytesPerPixel;
        if (nOfColours == 4) {
            if (surface->format->Rmask == 0x000000ff) {
                textureFormat = GL_RGBA;
            } else {
                textureFormat = GL_BGRA;
            }
        } else if (nOfColours == 3) {
            if (surface->format->Rmask == 0x000000ff) {
                textureFormat = GL_RGB;
            } else {
                textureFormat = GL_BGR;
            }
        }
        // Have opengl generate a texture object
        glGenTextures(1, &(this->texture));

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, this->texture);

        /* Not sure what this is for yet and whether its needed
        glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
        */

        // mix color with texture
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Write the texture data
        if ((error = glGetError()) != 0) {
            Logger::warn << "Error before loading texture: " << 
                gluErrorString(error) << endl;
            this->texture = 0;
        }

        if (this->isMipmap) {
            // Set the texture's stretching properties
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            gluBuild2DMipmaps(GL_TEXTURE_2D, nOfColours, surface->w, surface->h, 
                    textureFormat, GL_UNSIGNED_BYTE, surface->pixels);
        } else {
            // Set the texture's stretching properties
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, nOfColours, surface->w, surface->h, 
                    0, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);
        }

        if ((error = glGetError()) != 0) {
            Logger::warn << "Error loading texture into OpenGL: " << 
                gluErrorString(error) << endl;
            this->texture = 0;
        }

        // Set up the texture
        this->width = surface->w;
        this->height = surface->h;
        this->nOfColours = nOfColours;
        this->format = textureFormat;

        // Free the surface
        SDL_FreeSurface(surface);
    } else {
        std::cout << "Error loading texture (" << name << "): " << IMG_GetError() << endl;
        Logger::debug << "Error loading texture (" << name << "): " << IMG_GetError() << endl;
        texture = 0;
    }
}

Texture * Texture::getOrMakeTexture(string name, bool isMipmap) {
    Texture * texture;

	// Check if we have already loaded this name
    if (Texture::textures.count(name) > 0) {
        texture = Texture::textures[name];
    // Otherwise we need to create a new one
    } else {
        texture = new Texture(name, isMipmap);
        Texture::textures[name] = texture;
    }

    return texture;
}

std::string Texture::findRealFileName(const std::string originalFile) {
    // Check if the path exists straight off
    if (boost::filesystem::exists(originalFile)) {
        return originalFile;
    }

    // Get the path of the file and check it exists
    boost::filesystem::path originalPath(originalFile);
    originalPath.remove_filename();

    if (!boost::filesystem::exists(originalPath)) return originalFile;

    // Create a lower case version of the original path so that we can compare
    std::string originalLowerPath = originalFile;
    boost::to_lower(originalLowerPath);

    // We List the contents of the directory and check each file in lowercase
    boost::filesystem::directory_iterator endIt;
    for (boost::filesystem::directory_iterator it(originalPath); it != endIt; ++it) {
        // Skip if it is a subdirectory
        if (boost::filesystem::is_directory(it->status())) continue;

        // Get lowercase version of this file
        std::string path = it->path().string();
        boost::to_lower(path);

        //std::cout << path << std::endl;
        //std::cout << originalLowerPath << std::endl;

        if (originalLowerPath == path) {
            return it->path().string();
        }
    }

    // If we don't find anything, just return the original path and let 
    // SDL throw the error
    return originalFile;
}
