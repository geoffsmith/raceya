#include "hud.h"
#include <iostream>
#include <sstream>
#include <OpenGL/gl.h>

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
    SDL_Color yellow = { 1, 1, 0 };
    SDL_Surface * textSurface;
    stringstream text;
    GLuint texture;
    float width, height;
    float x, y , z;

    // Print out the RPM
    text << "RPM: ";
    text << this->_playersCar->getRPM();
    textSurface = TTF_RenderText_Solid(this->_font, text.str().c_str(), yellow);
    if (textSurface != NULL) {
        //Disable lighting
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 10, 0, 10, 0, 10);
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_MODELVIEW);

        // Now draw on to a polygon
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textSurface->w, textSurface->h, 0, GL_BGRA, 
                GL_UNSIGNED_BYTE, textSurface->pixels);

        width = textSurface->w / 100;
        height = textSurface->h / 100;
        z = 0;

        glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); 
        glVertex3d(0, 0, z);
        glTexCoord2d(1, 0); 
        glVertex3d(0 + width, 0, z);
        glTexCoord2d(1, 1); 
        glVertex3d(0 + width, 0 + height, z);
        glTexCoord2d(0, 1); 
        glVertex3d(0, 0 + height, z);
        glEnd();

        // Clean up
        glDeleteTextures(1, &texture);
        SDL_FreeSurface(textSurface);

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
