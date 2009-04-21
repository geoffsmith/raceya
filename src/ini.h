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

        // Get an integer
        int getInt(string key);
        float getFloat(string key);
        float getFloat(string key, int keyIndex, string restOfKey);

        // Query the data. Basically does a starts_with on each key in data and returns
        // a set of paths that match, but only at that path token length. i.e.
        // if query = /tracks/track would return /tracks/track0 and /tracks/track1 but not
        // /tracks/track0/file
        void query(string query, list<string> & results);

        // Similar to query, but returns only the top level token rather than the full
        // path
        void queryTokens(string query, list<string> & results);

    private:
        // Parse an ini file into the data structure
        void _parseFile();

        // Add all the keys under path + base to path + child
        void _inheritKeys(string base, string child, string path);

};
