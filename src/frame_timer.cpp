#include "frame_timer.h"

#include <SDL/SDL.h>
#include <iostream>

using namespace std;

FrameTimer FrameTimer::timer;

FrameTimer::FrameTimer() {
    // We are using SDL_GetTicks, which is in milliseconds
    this->_ticksPerSecond = 1000;
    this->_currentFrame = SDL_GetTicks();
    this->_lastFrame = SDL_GetTicks();
    this->_targetFPS = 20;
}

void FrameTimer::newFrame() {
    // Save the last frame and update the current frame
    this->_lastFrame = this->_currentFrame;
    this->_currentFrame = SDL_GetTicks();
}

float FrameTimer::getSeconds() {
    // Get the time elapsed since the last frame
    return (this->_currentFrame - this->_lastFrame) / (float)this->_ticksPerSecond;
}

float FrameTimer::getMinutes() {
    return this->getSeconds() / 60.0;
}

int FrameTimer::getTimeTillNextDraw() {
    // Get the time since the current frame
    unsigned int now = SDL_GetTicks();
    int result = 1000.0 / (float)this->_targetFPS - (now - this->_currentFrame);

    // Make sure the result is positive
    if (result < 0) result = 0;

    return result;
}
