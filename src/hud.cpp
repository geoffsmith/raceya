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
    this->_font = TTF_OpenFont(fontPath.c_str(), 16);

    fontPath = "resources/VeraMono.ttf";
    this->_monoFont = TTF_OpenFont(fontPath.c_str(), 10);
}

void Hud::_renderStats() {
    stringstream rpmText;
    stringstream fpsText;
    int skip = TTF_FontLineSkip(this->_font);

    // Print out the RPM
    rpmText << "RPM: ";
    rpmText << this->_playersCar->getRPM();

    this->_renderText(rpmText.str(), 10, this->_height - 50, this->_font);

    fpsText << "FPS: ";
    fpsText << FrameTimer::timer.getCurrentFPS();
    this->_renderText(fpsText.str(), 10, this->_height - 50 - skip, this->_font);
}

void Hud::_renderConsole() {
    deque<string>::iterator it = Logger::debugLines.begin();;
    int skip = TTF_FontLineSkip(this->_font);
    int i = 0;


    while (it != Logger::debugLines.end()) {
        this->_renderText(*it, 10, 10 + i * skip, this->_monoFont);
        ++i;
        ++it;
    }
}

void Hud::render() {
    this->_renderStats();
    this->_renderConsole();
}

void Hud::_renderText(string text, float x, float y, TTF_Font * font) {
    SDL_Color yellow = { 255, 255, 0 };
    SDL_Surface * textSurface = TTF_RenderText_Blended(font, text.c_str(), yellow);
    if (textSurface != NULL) {

        // Flip the surface
        horizontalFlipSurface(textSurface);

        glDisable(GL_LIGHTING);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, this->_width, 0, this->_height, 0, 10);

        glMatrixMode(GL_MODELVIEW);

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
