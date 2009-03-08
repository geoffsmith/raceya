#include "hud.h"
#include <iostream>
#include <sstream>
#include <OpenGL/gl.h>
#include "lib.h"

using namespace std;

Hud::Hud(Car * playersCar) {
    string fontPath = "resources/digital_readout.ttf";
    this->_playersCar = playersCar;
    this->_font = TTF_OpenFont(fontPath.c_str(), 36);
    if (this->_font == NULL) {
        cout << "Error loading font (" << fontPath << "): " << SDL_GetError() << endl;
    }
}

void Hud::render() {
    SDL_Color yellow = { 255, 255, 0 };
    SDL_Surface * textSurface;
    stringstream text;
    float width, height;
    float x, y , z;
    unsigned int error = glGetError();

    // Print out the RPM
    text << "RPM: ";
    text << this->_playersCar->getRPM();
    textSurface = TTF_RenderText_Blended(this->_font, text.str().c_str(), yellow);
    if (textSurface != NULL) {

        // Flip the surface
        horizontalFlipSurface(textSurface);

        error = glGetError();

        glDisable(GL_LIGHTING);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 10, 0, 10, 0, 10);

        glMatrixMode(GL_MODELVIEW);

        glRasterPos3f(0.5, 0.5, 0);

        glDrawPixels(textSurface->w, textSurface->h, 
                GL_BGRA, GL_UNSIGNED_BYTE, textSurface->pixels);
        error = glGetError();
        if (error) {
            cout << "Error: " << gluErrorString(error) << endl;
        }

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
