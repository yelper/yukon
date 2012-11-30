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
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "DoxygenParse.h"

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

void parseFunctionHeaders(string codeDir, vector<string> &files, map<pair<int, int>, string> &lines)
{
    // get a list of all code files in the codeDir, this'll involve looking at all *.cpp files
    files.clear();
    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator i(codeDir); i != end_itr; ++i)
    {
        // skip non-files (e.g. ".." or directories)
        if (!boost::filesystem::is_regular_file(i->status()))
            continue;
        
        if (i->path().extension() == ".cpp")
            files.push_back(i->path().string());
    }

    // TODO: handle double-templated+ datatypes (match "vector<vector<int> >" as a valid datatype)
    // TODO: handle const, static, etc. keywords :(
    // string plusParamRegex = "^[^\s]+\s+[^\s]+\s*\((\s*[^\s]+\s+[^\s]+)*\s*\)\s*\{";  // only handles zero or one parameters
                
    // explanation of below regex:
    // ^  head of string
    //  [^\s]+  match return data type
    //        \s+  space after data type
    //           [^\s]+ match function name
    //                 \s*  match any space between name and open parenthesis
    //                    \(  open paren
    //                      \s*  match any space between open paren and first parameter
    //                         (                                       )*  optional function parameters
    //                          [^\s]+  first parameter datatype
    //                                \s+  space between datatype and name
    //                                   [^\s,]+  first parameter name
    //                                          (                    )*  optional additional parameters
    //                                           ,                       match separating comma
    //                                            \s*                    match some space 
    //                                               [^\s]+              match parameter datatype
    //                                                     \s+           match space between datatype and name
    //                                                        [^\s,]+    match parameter name
    //                                                                   \s*  match any space before close paren
    //                                                                      \)  close paren
    //                                                                        \s* match any space after close paren
    //                                                                           \{  match opening bracket
    // ^[^\s]+\s+[^\s]+\s*\(\s*([^\s]+\s+[^\s,]+(,\s*[^\s]+\s+[^\s,]+)*)*\s*\)\s*\{

    string funcRegex = "^\\s*[^\\s]+\\s+[^\\s]+\\s*\\(\\s*([^\\s]+\\s+[^\\s,]+(,\\s*[^\\s]+\\s+[^\\s,]+)*)*\\s*\\)\\s*";
    boost::regex funcHeader(funcRegex);

    string funcStart = "^[^\\s]+\\s+[^\\s]+\\s*\\(";
    boost::regex funcPartial(funcStart);

    // iterate through the code files
    for (int i = 0; i < (int)files.size(); i++)
    {
        // read the file into a vector (each entry is a line)
        ifstream f(files[i].c_str());
        if (!f.is_open())
            cerr << "Unable to open found code file: " << files[i] << endl;
        
        string line;
        int lineNum = 0;
        int startFunc = -1;
        int numOpenBrackets = 0;
        bool inAFunction = false;
        bool inAFunctionHeader = false;
        
        string::iterator it;
        while (f.good())
        {
            getline(f, line);
            ++lineNum;

            // if in a function, iterate until we get back to net zero { } and record end number
            if (inAFunction) {
                numOpenBrackets += count(line.begin(), line.end(), '{');
                numOpenBrackets -= count(line.begin(), line.end(), '}');

                // if we're closing this function out, add the ranges here
                if (numOpenBrackets == 0)
                {
                    // TODO: add function information here
                    lines[pair<int, int>(startFunc, lineNum)] = "testfunc";
                    inAFunction = false;
                    startFunc = -1;
                }

            } else if (!inAFunctionHeader) { // test if this line looks like a function header
                if (boost::regex_match(line, funcHeader)) 
                {
                    inAFunction = true;
                    numOpenBrackets += count(line.begin(), line.end(), '{');
                    startFunc = lineNum;
                } 
                else if (boost::regex_match(line, funcPartial))
                    inAFunctionHeader = true;

            } else { // inAFunctionHeader
                
                // wait until a opening curly brace
                numOpenBrackets = count(line.begin(), line.end(), '{');
                if (numOpenBrackets > 0)
                {
                    startFunc = lineNum;
                    inAFunction = true;
                    inAFunctionHeader = false;
                }
            }

            if (numOpenBrackets < 0) // should never get here
            {
                cerr << "Got " << numOpenBrackets << " open brackets on line " << lineNum << "... aborting." << endl;
                break;
            }
        }
    }

}

int main(int argc, char** argv)
{
    if (argc != 2)
        printUsageAndExit(-1);
    
    string fname = argv[1];

    /* for testing the parseFunctionHeaders functionality
       Example: DoxygenParse U:\classes\cs706\yukon\TestProg
    */
    vector<string> files;
    map<pair<int, int>, string> lines;
    parseFunctionHeaders(fname, files, lines);

    map<pair<int, int>, string>::iterator it;
    for (it = lines.begin(); it != lines.end(); it++)
    {
        cout << "Lines " << it->first.first << "-" << it->first.second << ": " << it->second << endl;
    }
    
    /* for testing the parseCallGraph functionality

    Example: DoxygenParse U:\classes\cs706\yukon\TestProg\doc\html\_test_prog_8cpp_a3c04138a5bfe5d72780bb7e82a18e627_cgraph.dot
    
    map<string, string> names;
    vector<vector<int> > graph = parseCallGraph(fname, names);

    for (int i = 0; i < graph.size(); i++)
    {
        for (int j = 0; j < graph.size(); j++)
        {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    } */

    return 0;
}

