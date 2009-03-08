#include "logger.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

stringstream Logger::debug;
deque<string> Logger::debugLines;
int Logger::size = 7;
bool Logger::outputToConsole = true;

void Logger::maintain() {
    // Clear the older lines if there are too many
    list<string> parts;
    list<string>::iterator it;
    string tmp = Logger::debug.str();

    // If we want to output to console, do so
    if (Logger::outputToConsole) {
        cout << tmp;
    }

    // Split into lines
    split(parts, tmp, is_any_of("\n"));

    // Clear the buffer
    Logger::debug.str("");

    // Add to the queue
    for (it = parts.begin(); it != parts.end(); ++it) {
        if (it->size() > 0) {
            Logger::debugLines.push_back(*it);
        }
    }

    // Clean up the old stuff
    for (int i = Logger::debugLines.size(); i > Logger::size; --i) {
        Logger::debugLines.pop_front();
    }
}
