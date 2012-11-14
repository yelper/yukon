// DoxygenParse.cpp : Defines the entry point for the console application.

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// include smart pointer functions
using std::shared_ptr;
using std::make_shared;
using std::enable_shared_from_this;

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

void printUsageAndExit(int exitCondition)
{
    exit(exitCondition);
}

vector<vector<int> > parseCallGraph(string filename, map<string, string> &names)
{
    ifstream f(filename.c_str());
    if (!f.is_open())
        cerr << "Unable to open specified file: " << filename << endl;
    
    vector<vector<string> > lines;
    names.clear();
    string line;
    while (f.good())
    {
        getline(f, line);
        boost::trim(line);

        // only parse those lines that start with 'node'
        if (boost::starts_with(line, "Node"))
        {
            vector<string> parts;
            boost::split(parts, line, boost::is_any_of(" ,"));
            
            // parse the name if this is a node definition line
            if (parts[1] != "->")
            {
                string name = parts[1];
                name = name.substr(8, name.size() - 9);
                cout << name << endl;
                names[parts[0]] = name;
            }
            
            lines.push_back(parts);
        }
    }

    // now that we have all the labels, we can fill them in.
    vector<vector<int> > graph(names.size(), vector<int>(names.size(), 0));
    for (int i = 0; i < (int)lines.size(); i++)
    {
        vector<string> line = lines[i];
        if (line[1] == "->")
        {
            int from = boost::lexical_cast<int>(line[0].substr(4, 1)) - 1;
            int to   = boost::lexical_cast<int>(line[2].substr(4, 1)) - 1;
            graph[from][to] = 1;
        }
    }


    return graph;
}

int main(int argc, char** argv)
{
    if (argc != 2)
        printUsageAndExit(-1);
    
    string fname = argv[1];
    map<string, string> names;
	vector<vector<int> > graph = parseCallGraph(fname, names);

    for (int i = 0; i < graph.size(); i++)
    {
        for (int j = 0; j < graph.size(); j++)
        {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }

    return 0;
}

