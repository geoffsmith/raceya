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
}

void Hud::_renderStats() {
    stringstream rpmText;
    stringstream fpsText;
    int skip = TTF_FontLineSkip(this->_font);

    // Print out the RPM
    rpmText << "RPM: ";
    rpmText << this->_playersCar->getRPM();

    this->_renderText(rpmText.str(), 10, this->_height - 50, skip,this->_font);

    fpsText << "FPS: ";
    fpsText << FrameTimer::timer.getCurrentFPS();
    this->_renderText(fpsText.str(), 10, this->_height - 50 - skip, skip, this->_font);
}

void Hud::_renderConsole() {
    deque<string>::iterator it = Logger::debugLines.begin();;
    int skip = TTF_FontLineSkip(this->_font);
    int i = 0;


    while (it != Logger::debugLines.end()) {
        this->_renderText(*it, 10, 10 + i * skip, skip, this->_monoFont);
        ++i;
        ++it;
    }
}

void Hud::render() {
    this->_renderStats();
    this->_renderConsole();
}

void Hud::_renderText(string text, float x, float y, float lineHeight, TTF_Font * font) {
    unsigned int texture;
    SDL_Color yellow = { 255, 255, 0 };
    SDL_Surface * textSurface = TTF_RenderText_Blended(font, text.c_str(), yellow);
    if (textSurface != NULL) {

        // Flip the surface
        horizontalFlipSurface(textSurface);

        // Have opengl generate a texture object
        glGenTextures(1, &texture);

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, texture);


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

        glDisable(GL_LIGHTING);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // convert x and y into translations
        float yRatio = (float)this->_width / (float)this->_height;
        float translateX = -1 * yRatio + (x / (float)this->_width) * 2 * yRatio;
        float translateY = -1 + (y / (float)this->_height) * 2;
        float lineX = (textSurface->w / (float)this->_width) * 2 * yRatio;
        float lineY = (textSurface->h / (float)this->_height) * 2;

        glTranslatef(translateX, translateY, -1.5);

        //glColor4f(1, 1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(0, 0, 0);

        glTexCoord2f(1, 0);
        glVertex3f(lineX, 0, 0);

        glTexCoord2f(1, 1);
        glVertex3f(lineX, lineY, 0);

        glTexCoord2f(0, 1);
        glVertex3f(0, lineY, 0);

        glEnd();


        glEnable(GL_LIGHTING);
        glPopMatrix();
        SDL_FreeSurface(textSurface);
        glDeleteTextures(1, &(texture));
    } else {
        cout << "Error rending HUD: " << SDL_GetError() << endl;
    }
}

/*
void _renderText2(string text, float x, float y, TTF_Font * font) {
    SDL_Color yellow = { 255, 255, 0 };
    SDL_Surface * textSurface = TTF_RenderText_Blended(font, text.c_str(), yellow);
    if (textSurface != NULL) {

        // Flip the surface
        horizontalFlipSurface(textSurface);

        char * test = (char *)textSurface->pixels;
        //for (int i = 3; i < textSurface->w * textSurface->h; i += 4) test[i] = (char)255;
        Logger::debug << "Testing: " << (unsigned short)test[3] << endl;

        glDisable(GL_LIGHTING);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, this->_width, 0, this->_height, 0, 10);

        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_ZERO, GL_SRC_COLOR);

        glRasterPos3f(x, y, 0);

        glDrawPixels(textSurface->w, textSurface->h, 
                GL_BGRA, GL_UNSIGNED_BYTE, textSurface->pixels);

        SDL_FreeSurface(textSurface);


        glEnable(GL_BLEND);
        glMatrixMode(GL_PROJECTION);
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    } else {
        cout << "Error rending HUD: " << SDL_GetError() << endl;
    }
}
*/
