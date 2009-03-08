/**
 * A simple logging utility that will pump text into the onscreen console
 */
#pragma once

#include <sstream>
#include <deque>

using namespace std;

class Logger {
    public:
        static stringstream debug;
        static deque<string> debugLines;
        static int size;

        // Utility function to clear out old statements from the logger, should be
        // called at each frame
        static void maintain();
};
