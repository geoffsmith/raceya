/**
 * The heads up display. For now this will mostly have development related stuff in it.
 *
 * TODO:
 *      * Get transparency working
 */
#pragma once

#include "car.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

struct HUDTexture {
    unsigned int texture;
    int width;
    int height;
    string text;
};

class Hud {
    public:
        // Most of the data for the HUD will come from the player's car
        Hud(Car * playersCar, int width, int height);

        // render the HUD
        void render();

    private:
        Car * _playersCar;
        TTF_Font * _font;
        TTF_Font * _monoFont;
        unsigned int _texture;
        map<string, HUDTexture * > _textures;

        // Render the various stats
        void _renderStats();

        // Render the debug console
        void _renderConsole();

        void _renderText(string text, float x, float y, TTF_Font * font, string textureKey);

        int _width;
        int _height;
};
