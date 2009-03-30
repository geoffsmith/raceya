/**
 * Class to calculate the time between frames for use by various parts of the game to 
 * calculate time-based algorithms. I.e. Velocity, wheel rotation etc.
 */
#pragma once

#include <list>

using namespace std;

class FrameTimer {
    public:
        // The constructor queries the timer to set up the ticks per seconds
        FrameTimer(int targetFPS=40);

        // Update the frame
        void newFrame();
        
        // Get the number of seconds since the last frame
        float getSeconds();
        // ... and minutes
        float getMinutes();

        // Singleton class
        static FrameTimer timer;

        // Get the time to pause until it's time to draw the next frame. We use 
        // the target FPS to calculate the time until we need to draw the next
        // frame. Result in milliseconds.
        int getTimeTillNext();
        
        //  Get the current FPS
        int getCurrentFPS();
        int getAverageFPS();


    private:
        unsigned int _currentFrame;
        unsigned int _lastFrame;
        unsigned int _ticksPerSecond;
        unsigned int _targetFPS;
        unsigned int _currentFPS;

        // We keep a history of the FPS so that we can work out the average for a smooth
        // FPS display
        list<unsigned int> _fpsHistory;
};
