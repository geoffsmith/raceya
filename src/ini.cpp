#include "ini.h"
#include "logger.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <list>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Ini::Ini(string path) {
    this->path = path;
    this->_parseFile();
}

bool Ini::hasKey(string key) {
    return this->data.count(key) > 0;
}

string Ini::operator[](string key) {
    if (this->hasKey(key)) {
        return this->data[key];
    } else {
        return NULL;
    }
}

/******************************************************************************
 * File parsing
 *****************************************************************************/
void Ini::_parseFile() {
    vector<string> parts;
    list<string> currentPath;
    string path;
    string line;
    string previous;

    // A car is mostly described in a car.ini file, so we check it exists and open it
    if (!exists(this->path)) {
        Logger::warn << "Ini file was not found: " << this->path << endl;
        return;
    }

    ifstream file(this->path.c_str());

    if (!file.is_open()) {
        Logger::warn << "Error opening file: " << this->path << endl;
        return;
    }

    // Keep track of brackets so we know when the next token is a path token rather
    // than a value
    while (!file.eof()) {
        getline(file, line);
        trim(line);

        // Skip the line if its empty or is a comment
        if (line.size() == 0 || line[0] == ';') continue;

        // Check if we need to pop a path token, NOTE due to bugs in inis we ignore
        // too many closing brackets
        if (line[0] == '}' && currentPath.size() > 0) {
            currentPath.pop_back();
            continue;
        }

        // Check if the next value is a token
        if (line[0] == '{') {
            currentPath.push_back(previous);
            continue;
        }

        // Check if we have a key/value pair
        split(parts, line, is_any_of("="));
        previous = parts[0];
        if (parts.size() >= 2) {
            // build the current path
            path = Ini::makeIniPath(currentPath) + parts[0];
            this->data[path] = parts[1];
            continue;
        }
    }
}

string Ini::makeIniPath(list<string> & nodes) {
    list<string>::iterator it;
    string result = "/";
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        result += *it;
        result += "/";
    }
    return result;
}
