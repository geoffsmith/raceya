#include "frame_timer.h"
#include "logger.h"

#include <SDL/SDL.h>
#include <iostream>

using namespace std;

FrameTimer FrameTimer::timer;

FrameTimer::FrameTimer(int targetFPS) {
    // We are using SDL_GetTicks, which is in milliseconds
    this->_ticksPerSecond = 1000;
    this->_lastFrame = SDL_GetTicks();
    this->_currentFrame = SDL_GetTicks();
    this->_targetFPS = targetFPS;
    this->_currentFPS = 0;
}

void FrameTimer::newFrame() {
    // Save the last frame and update the current frame
    this->_lastFrame = this->_currentFrame;
    this->_currentFrame = SDL_GetTicks();

    // Calculate how many frames we are doing 
    if (this->_currentFrame - this->_lastFrame == 0) {
        this->_currentFPS = 0;
    } else {
        this->_currentFPS = this->_ticksPerSecond 
            / (this->_currentFrame - this->_lastFrame);
    }

    // Add this to the history
    this->_fpsHistory.push_back(this->_currentFPS);
    if (this->_fpsHistory.size() > 10) {
        this->_fpsHistory.pop_front();
    }
}

float FrameTimer::getSeconds() {
    // Get the time elapsed since the last frame
    return (this->_currentFrame - this->_lastFrame) / (float)this->_ticksPerSecond;
}

float FrameTimer::getTargetSeconds() {
    return 1.0 / this->_targetFPS;
}

float FrameTimer::getMinutes() {
    return this->getSeconds() / 60.0;
}

int FrameTimer::getTimeTillNext() {
    // Get the time since the current frame
    unsigned int now = SDL_GetTicks();
    int result = (float)this->_ticksPerSecond / (float)this->_targetFPS 
        - (now - this->_currentFrame);

    // Make sure the result is positive
    if (result < 0) result = 0;

    return result;
}

int FrameTimer::getCurrentFPS() {
    return this->_currentFPS;
}

int FrameTimer::getAverageFPS() {
    int sum = 0;
    int count = 0;
    for (list<unsigned int>::iterator it = this->_fpsHistory.begin(); it != this->_fpsHistory.end(); ++it) {
        sum += *it;
        ++count;
    }
    if (count == 0) return 0;
    else {
        return sum / count;
    }
     
}
