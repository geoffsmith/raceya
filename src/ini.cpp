#include "ini.h"
#include "logger.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <list>
#include <vector>
#include <iostream>

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
        return "";
    }
}

void Ini::query(string query, list<string> & results) {
    string tmp;
    vector<string> parts;
    // For this we need to check each key. It's not very efficient, but this is only
    // used during a loading sequence, so it should be ok for now.
    map<string, string>::iterator it;
    for (it = this->data.begin(); it != this->data.end(); ++it) {
        if (starts_with(it->first, query)) {
            // Get the path including query until the next /
            tmp = it->first.substr(query.size());
            split(parts, tmp, is_any_of("/"));

            results.push_back(query + parts[0]);
        }
    }

    // Remove any duplicates
    results.unique();
}

void Ini::queryTokens(string query, list<string> & results) {
    list<string> tmp;
    list<string>::iterator it;
    vector<string> parts;

    // First do a normal query, then strip out the query part of the path
    this->query(query, tmp);

    // Now strip out everything except the last part of query
    for (it = tmp.begin(); it != tmp.end(); ++it) {
        split(parts, *it, is_any_of("/"));
        results.push_back(parts[parts.size() - 1]);
    }
}

/******************************************************************************
 * File parsing
 *****************************************************************************/
void Ini::_parseFile() {
    vector<string> parts;
    vector<string> inherit;
    string inheritPath;
    list<string> tokens;
    string token;
    list<string> currentPath;
    string path;
    string line;
    string previous;

    // A car is mostly described in a car.ini file, so we check it exists and open it
    if (!exists(this->path)) {
        Logger::warn << "Ini file was not found: " << this->path << endl;
        return;
    }

    cout << "Parsing ini file: " << this->path << endl;
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

        // If the line has an equal sign, then it is a key / value line. At the moment
        // we can't handle { or } here
        split(parts, line, is_any_of("="));
        if (parts.size() >= 2) {
            // build the current path
            path = Ini::makeIniPath(currentPath) + parts[0];
            this->data[path] = parts[1];
            previous = parts[0];
            continue;
        } 

        // Now we space-separated tokenise the line to check for path tokens and { }
        split(tokens, line, is_any_of(" "));
        while (tokens.size() > 0) {
            token = tokens.front();
            tokens.pop_front();

            // Ignore empty tokens
            if (token.size() == 0) continue;

            // If we hit a comment, we stop
            if (token[0] == ';') break;

            // Check if the token is a closing bracket. NOTE due to bugs in inis 
            // we ignore too many closing brackets
            if (token == "}") {
                if (currentPath.size() > 0) {
                    currentPath.pop_back();
                }
            } else if (token == "{") {
                // If there is a ~ in the token, we need to inherit from another key
                split(inherit, previous, is_any_of("~"));
                if (inherit.size() == 1) {
                    currentPath.push_back(previous);
                } else {
                    path = Ini::makeIniPath(currentPath);
                    this->_inheritKeys(inherit[1], inherit[0], path);
                    // Copy the settings from inherit[1]
                    currentPath.push_back(inherit[0]);
                }
            } else {
                previous = token;
            }
        }
    }
}

void Ini::_inheritKeys(string base, string child, string path) {
    string test = path + base;
    string tmp;
    map<string, string>::iterator it;
    for (it = this->data.begin(); it != this->data.end(); ++it) {
        // Check if this starts with path + base
        if (starts_with(it->first, path + base)) {
            // replace base with child
            tmp = it->first;
            replace_first(tmp, base, child);
            this->data[tmp] = it->second;
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

/******************************************************************************
 * Getters for various types. They will try and convert a string into a type
 *****************************************************************************/
int Ini::getInt(string key) {
    string value = (*this)[key];
    return atoi(value.c_str());
}

float Ini::getFloat(string key) {
    string value = (*this)[key];
    return atof(value.c_str());
}
