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

class Hud {
    public:
        // Most of the data for the HUD will come from the player's car
        Hud(Car * playersCar);

        // render the HUD
        void render();

    private:
        Car * _playersCar;
        TTF_Font * _font;
        unsigned int _texture;
};
