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


struct fileinfo_t {
    string filename;
    int start;
    int end;

    bool operator<(const fileinfo_t &n) const {
        return this->start < n.start;
    }
};

void printUsageAndExit(int exitCondition)
{
    exit(exitCondition);
}

vector<vector<int> > parseCallGraph(string filename, vector<string> &names)
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
                names.push_back(name);
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

void parseFunctionHeaders(string codeDir, vector<string> &files, map<fileinfo_t, string> &lines)
{
    // get a list of all code files in the codeDir, this'll involve looking at all *.cpp files
    files.clear();
    lines.clear();
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

    string funcRegex = "^\\s*[^\\s]+\\s+([^\\s]+)\\s*\\(\\s*([^\\s]+\\s+[^\\s,]+(,\\s*[^\\s]+\\s+[^\\s,]+)*)*\\s*\\)\\s*";
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
        
        string curFunc;
        string curFile = files[i].substr(codeDir.length() + 1, files[i].length() - codeDir.length());

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
                    fileinfo_t info;
                    info.filename = curFile;
                    info.start = startFunc;
                    info.end = lineNum;

                    lines[info] = curFunc;
                    inAFunction = false;
                    startFunc = -1;
                }

            } else if (!inAFunctionHeader) { 
                // test if this line looks like a function header
                boost::cmatch matches;                
                
                if (boost::regex_match(line.c_str(), matches, funcHeader)) 
                {
                    /* some information about the matches
                    for (int k = 0; k < matches.size(); k++)
                        cout << "matches[" << k << "] = " << matches[k].str() << endl;
                    */

                    curFunc = matches[1].str();
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

void parseAllGraphFiles(vector<string> files) 
{
    for (int n = 0; n < (int)files.size(); n++)
    {
        vector<string> names;
        vector<vector<int> > graph = parseCallGraph(files[n], names);
        
        vector<vector<string > > callGraphs;
        for (int i = 0; i < graph.size(); i++)
        {
            string thisName = names[i];
            callGraphs.push_back(vector<string>());
            for (int j = 0; j < graph.size(); j++) 
            {
                if (i == j) continue; // ignore self-references/recursion for this problem
                if (graph[i][j] == 1)
                {

                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
        printUsageAndExit(-1);
    
    string fname = argv[1];
    string func = argv[2];

    // for testing the parseFunctionHeaders functionality
    // Example: DoxygenParse U:\classes\cs706\yukon\TestProg f
    
    if (func == "f") {
        vector<string> files;
        map<fileinfo_t, string> lines;
        parseFunctionHeaders(fname, files, lines);

        map<fileinfo_t, string>::iterator it;
        for (it = lines.begin(); it != lines.end(); it++)
        {
            cout << "File " << it->first.filename << "[" << it->first.start << ":" << it->first.end << "] " << it->second << endl;
        }

        ofstream f(fname + "/functions.txt");
        if (f.is_open())
        {
            for (it = lines.begin(); it != lines.end(); it++)
                f << it->first.filename << " " << it->first.start << "-" << it->first.end << " " << it->second << endl;
            f << endl;
        }
    }
    
    
    // for testing the parseCallGraph functionality
    // Example: DoxygenParse U:\classes\cs706\yukon\TestProg\ g
    
    else if (func == "g")    
    {
        // assume that the call graphs are in the /html/doc directory
        boost::filesystem::path proj(fname);
        proj /= "doc/html";

        if (!boost::filesystem::exists(proj))
        {
            cout << "Unable to find doc/html directory in the given code path." << endl;
            cerr << "ERROR: Directory " << proj.string() << " not found." << endl;
            return -1;
        }

        // get all *_cgraph.dot files in html doc directory
        vector<string> cgraphFiles;
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator i(proj); i != end_itr; ++i)
        {
            // skip non-files (e.g. '..' or directories
            if (!boost::filesystem::is_regular_file(i->status())) continue;

            if (boost::ends_with(i->path().filename().string(), "_cgraph.dot"))
                cgraphFiles.push_back(i->path().string());
        }

        ofstream f(fname + "/graphs.txt");
        if (f.is_open())
        {
            for (int n = 0; n < (int)cgraphFiles.size(); n++) 
            {
                vector<string> names;
                vector<vector<int> > graph = parseCallGraph(cgraphFiles[n], names);

                for (int i = 0; i < graph.size(); i++)
                {
                    f << names[i];
                    for (int j = 0; j < graph.size(); j++)
                    {
                        if (i == j) continue; // ignore self-references/recursion for this problem
                        if (graph[i][j] == 1)
                            f << " " << names[j];
                    }
                    f << endl;
                }
                f << endl;
            }
        }
    }
    
    // given files of (1) use cases and (2) edited functions, output the (scall graphs for each
    else if (func == "s")
    {

    }


    return 0;
}

