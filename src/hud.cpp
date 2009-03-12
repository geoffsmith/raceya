#include "hud.h"
#include "lib.h"
#include "logger.h"
#include "frame_timer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <OpenGL/gl.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Hud::Hud(Car * playersCar, int width, int height) {
    string fontPath = "resources/digital_readout.ttf";
    this->_playersCar = playersCar;
    this->_width = width;
    this->_height = height;
    this->_font = TTF_OpenFont(fontPath.c_str(), 32);

    fontPath = "resources/VeraMono.ttf";
    this->_monoFont = TTF_OpenFont(fontPath.c_str(), 11);

    // Have opengl generate a texture object
    glGenTextures(1, &(this->_texture));
}

void Hud::_renderStats() {
    stringstream rpmText;
    stringstream fpsText;
    int skip = TTF_FontLineSkip(this->_font);

    // Print out the RPM
    rpmText << "RPM: ";
    rpmText << this->_playersCar->getRPM();

    this->_renderText(rpmText.str(), 10, this->_height - 50, this->_font, "rpm");

    fpsText << "FPS: ";
    fpsText << FrameTimer::timer.getCurrentFPS();
    this->_renderText(fpsText.str(), 10, this->_height - 50 - skip, this->_font, "fps");

    fpsText.str("");
    fpsText << "Average FPS: ";
    fpsText << FrameTimer::timer.getAverageFPS();
    this->_renderText(fpsText.str(), 10, this->_height - 50 - skip * 2, this->_font, "afps");
}

void Hud::_renderConsole() {
    deque<string>::iterator it = Logger::debugLines.begin();;
    int skip = TTF_FontLineSkip(this->_font);
    int i = 0;


    while (it != Logger::debugLines.end()) {
        this->_renderText(*it, 10, 10 + i * skip, this->_monoFont, "console" + i);
        ++i;
        ++it;
    }
}

void Hud::render() {
    // Only set the state once
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_MODELVIEW);

    this->_renderStats();
    this->_renderConsole();

    glEnable(GL_LIGHTING);
}

void Hud::_renderText(string text, float x, float y, TTF_Font * font, string textureKey) {
    HUDTexture * texture;

    // Check if this texture has a key
    if (this->_textures.count(textureKey) > 0) {
        texture = this->_textures[textureKey];
    } else {
        texture = new HUDTexture();
        glGenTextures(1, &(texture->texture));
        this->_textures[textureKey] = texture;
    }

    // Check if the text has changed
    if (this->_textures[textureKey]->text != text) {
        SDL_Color yellow = { 255, 255, 0 };
        SDL_Surface * textSurface = TTF_RenderText_Blended(font, text.c_str(), yellow);
        if (textSurface != NULL) {

            // Bind the texture object
            glBindTexture(GL_TEXTURE_2D, texture->texture);

            glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);

            // mix color with texture
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexImage2D(GL_TEXTURE_2D, 0, 4, textSurface->w, textSurface->h, 
                    0, GL_BGRA, GL_UNSIGNED_BYTE, textSurface->pixels);

            // Set the texture width and height
            texture->width = textSurface->w;
            texture->height = textSurface->h;

            SDL_FreeSurface(textSurface);
            this->_textures[textureKey]->text = text;
        } else {
            cout << "Error rending HUD: " << SDL_GetError() << endl;
            return;
        }
    } else {
        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, texture->texture);
    }


    // convert x and y into translations
    float yRatio = (float)this->_width / (float)this->_height;
    float translateX = -1 * yRatio + (x / (float)this->_width) * 2 * yRatio;
    float translateY = -1 + (y / (float)this->_height) * 2;
    float lineX = (texture->width / (float)this->_width) * 2 * yRatio;
    float lineY = (texture->height / (float)this->_height) * 2;

    glPushMatrix();
    glLoadIdentity();

    glTranslatef(translateX, translateY, -1.5);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex3f(0, 0, 0);

    glTexCoord2f(1, 1);
    glVertex3f(lineX, 0, 0);

    glTexCoord2f(1, 0);
    glVertex3f(lineX, lineY, 0);

    glTexCoord2f(0, 0);
    glVertex3f(0, lineY, 0);

    glEnd();

    glPopMatrix();
}
