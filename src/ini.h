/**
 * A racer style .ini file parser and encapsulation class. This allows hierarchical ini
 * files to be imported. The paths can be queried in an XPath style way with wildcards.
 */
#pragma once

#include <map>
#include <string>
#include <list>

using namespace std;

class Ini {
    public:
        Ini(string path);

        // The path of the ini file
        string path;
        
        // A map representing the ini file, the key is a path i.e. /body/model/file
        map<string, string> data;

        // Make an ini file path, convert stack of strings to / separated string
        static string makeIniPath(list<string> & nodes);

        // Return true if the ini has the path
        bool hasKey(string key);

        // Return the value for that key
        string operator[](string key);

    private:
        // Parse an ini file into the data structure
        void _parseFile();

};
