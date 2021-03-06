// DoxygenParse.cpp : Defines the entry point for the console application.

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
using namespace std;

// include smart pointer functions
using std::shared_ptr;
using std::make_shared;
using std::enable_shared_from_this;

#include <boost/exception/all.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "DoxygenParse.h"


const int maxDist = numeric_limits<int>::max();

struct fileinfo_t {
    string filename;
    int start;
    int end;

    bool operator<(const fileinfo_t &n) const {
        return this->start  * this->end < n.start * n.end;
    }
};

struct edge {
	int begin;
	int end;

	edge(int b, int e) : begin(b), end(e) { }
	bool operator <(const edge &n) const {
		return this->begin < n.begin;
	}
};

void printUsageAndExit(int exitCondition)
{
    cout << endl;
    cout << "Usage: DoxygenParse.exe <codeDir> <func> ..." << endl;
    cout << endl;
    cout << "  <func> = f     Generate a mapping of line numbers in all code files to " << endl;
    cout << "                   function names, outputs a file <codeDir>/functions.txt" << endl;
    cout << "             Ex: DoxygenParse U:\\classes\\cs706\\yukon\\TestProg f" << endl << endl;
    cout << "         = g     Generate a call graph for all *_cgraph.dot files in the" << endl;
    cout << "                   <codeDir>/doc/html/ directory, default output directory of" << endl;
    cout << "                   Doxygen HTML output.  Note that .dot files must be specified" << endl;
    cout << "                   to be retained in the Doxygen config file." << endl;
    cout << "             Ex: DoxygenParse U:\\classes\\cs706\\yukon\\TestProg\\ g" << endl << endl;
    cout << "         = s     Generate a shortest path mapping (if one exists) from all use" << endl;
    cout << "                   cases to edited function using Dijkstra's algorithm.  Looks" << endl;
    cout << "                   in <codeDir>/doc/html for call graph dot files, parses a" << endl;
    cout << "                   use case definition file (see code for format), and parses" << endl;
    cout << "                   a edited functions file (see code for format); outputs a" << endl;
    cout << "                   list of use cases and their shortest-path edited function" << endl;
    cout << "                   call graph." << endl;
    cout << "             Ex: U:\\classes\\cs706\\yukon\\TestProg\\ s " << endl;
    cout << "                   U:\\classes\\cs706\\yukon\\TestProg\\usecase.txt" << endl;
    cout << "                   U:\\classes\\cs706\\yukon\\TestProg\\edited.txt" << endl;
    cout << endl;
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

                // get rid of \l in names
                boost::replace_all(name, "\\l", "");

                // get rid of namespace declarations (to make things easier on ourselves)
                if (count(name.begin(), name.end(), ':') == 4)
                {
                    vector<string> funcParts;
                    boost::iter_split(funcParts, name, boost::algorithm::first_finder("::"));
                    
                    name = funcParts[1] + "::" + funcParts[2];
                }

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
            int from = boost::lexical_cast<int>(line[0].substr(4, 2)) - 1;
            int to   = boost::lexical_cast<int>(line[2].substr(4, 2)) - 1;
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
    boost::filesystem::recursive_directory_iterator end_itr;
    for (boost::filesystem::recursive_directory_iterator i(codeDir); i != end_itr; ++i)
    {
        // skip non-files (e.g. ".." or directories)
        /*if (!boost::filesystem::is_regular_file(i->status()))
            continue;*/

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

    //string funcRegex = "^\\s*[^\\s]+\\s+([^\\s]+)\\s*\\(\\s*([^\\s]+\\s+[^\\s,]+(,\\s*[^\\s]+\\s+[^\\s,]+)*)*\\s*\\)\\s*";
    string funcRegex = "^\\s*[^\\s]+\\s+([^\\s]+)\\s*\\(.*";
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

void parseAllGraphFiles(vector<string> files, vector<vector<edge> > &edges, map<string, int> &names) 
{
	int curIndex = 0;
	names.clear();
	edges.clear();

	for (int n = 0; n < (int)files.size(); n++)
	{
		vector<string> lnames;
		vector<vector<int> > graph = parseCallGraph(files[n], lnames);

		for (int i = 0; i < graph.size(); i++)
		{
			string iname = lnames[i];
			for (int j = 0; j < graph.size(); j++)
			{
				if (i == j) continue; // skip self-loops
				if (graph[i][j] == 1) // a edge exists from i to j
				{
					string jname = lnames[j];

					// add to name map if it doesn't exist
					if (names.count(iname) == 0)
					{
						names[iname] = curIndex++;
						edges.push_back(vector<edge>());
					} if (names.count(jname) == 0) {
						names[jname] = curIndex++;
						edges.push_back(vector<edge>());
					}

					edges[names[iname]].push_back(edge(names[iname], names[jname]));
				}
			}
		}
	}

	//funcs.clear();
	//for (map<string, int>::iterator it = names.begin(); it != names.end(); ++it)
	//	funcs.push_back(it->first);
}

void dijkstraPaths(int start, const vector<vector<edge> > edges, vector<int> &dists, vector<int> &prev)
{
	int n = edges.size();
	dists.clear();
	dists.resize(n, maxDist);
	dists[start] = 0;

	prev.clear();
	prev.resize(n, -1);
	set<pair<int, int> > vert_queue;
	vert_queue.insert(make_pair(0, start));

	while (!vert_queue.empty())
	{
		int dist = vert_queue.begin()->first;
		int u = vert_queue.begin()->second;
		vert_queue.erase(vert_queue.begin());

		// for all neighbors
		const vector<edge> &neighs = edges[u];
		for (vector<edge>::const_iterator it = neighs.begin(); it != neighs.end(); it++)
		{
			int v = it->end;
			int dist_thru_u = dist + 1; // constant weight
			if (dist_thru_u < dists[v])
			{
				vert_queue.erase(make_pair(dists[v], v));

				dists[v] = dist_thru_u;
				prev[v] = u;
				vert_queue.insert(make_pair(dists[v], v));
			}
		}
	}
}

list<int> dijkstraTrace(int vert, const vector<int> &prev)
{
	list<int> path;
	for (; vert != -1; vert = prev[vert])
		path.push_front(vert);
	
	return path;
}

void getDotFiles(boost::filesystem::path codeDir, vector<string> &files)
{
    // get all *_cgraph.dot files in html doc directory
    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator i(codeDir); i != end_itr; ++i)
    {
        // skip non-files (e.g. '..' or directories
        if (!boost::filesystem::is_regular_file(i->status())) continue;

        if (boost::ends_with(i->path().filename().string(), "_cgraph.dot"))
            files.push_back(i->path().string());
    }
}

int main(int argc, char** argv)
{
    if (argc < 3)
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
        boost::filesystem::path proj(fname + "\\doc\\html");

        if (!boost::filesystem::exists(proj))
        {
            cout << "Unable to find doc/html directory in the given code path." << endl;
            cerr << "ERROR: Directory " << proj.string() << " not found." << endl;
            printUsageAndExit(-1);
        }

        vector<string> cgraphFiles;
        getDotFiles(proj, cgraphFiles);

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
    
    // given files of (1) use cases and (2) edited functions, output the call graphs for each
    else if (func == "s")
    {
        if (argc != 5)
        {
            cout << "Requires four arguments: <codeDir> s <useCase.txt> <modifiedFuncs.txt>" << endl;
            cerr << "Missing required arguments; aborting." << endl;
            return -2;
        }

        //assume that the call graphs are in the /doc/html directory
        boost::filesystem::path proj(fname + "\\doc\\html");

        if (!boost::filesystem::exists(proj))
        {
            cout << "Unable to find doc/html directory in the given code path." << endl;
            cerr << "ERROR: Directory " << proj.string() << " not found." << endl;
            printUsageAndExit(-1);
        }

        if (!boost::filesystem::exists(argv[3]))
        {
            cout << "Unable to find use case definition file.  Aborting." << endl;
            cerr << "ERROR: Use Case file " << argv[3] << " not found." << endl;
            printUsageAndExit(-3);
        } 
        if (!boost::filesystem::exists(argv[4]))
        {
            cout << "Unable to find edited functions file.  Aborting." << endl;
            cerr << "ERROR: Edited functions file " << argv[4] << " not found." << endl;
            printUsageAndExit(-4);
        }

        // get the dot files
        vector<string> cgraphFiles;
        getDotFiles(proj, cgraphFiles);

        // get the list of all functions and their dependencies
        vector<vector<edge> > edges;
        map<string, int> funcs;
        parseAllGraphFiles(cgraphFiles, edges, funcs);

        // maintain a list of functions for easy referencing (int -> func)
        vector<string> funcNames(funcs.size());
	    for (map<string, int>::iterator it = funcs.begin(); it != funcs.end(); ++it)
	        funcNames[it->second] = it->first;

        // get the use cases
        ifstream fUseCase(argv[3]);
        if (!fUseCase.is_open())
            cerr << "Unable to open use case file: " << argv[3] << endl;

        string line;
        map<string, pair<string, int>> useCases;
        while (fUseCase.good())
        {
            getline(fUseCase, line);

            // skip invalid lines; expecting input of type: "useCase|fileName|methodName"
            if (count(line.begin(), line.end(), '|') != 2)
                continue;

            vector<string> parts;
            boost::split(parts, line, boost::is_any_of("|"));

            if (funcs.count(parts[2]) != 0)
                useCases[parts[0]] = make_pair(parts[2], funcs[parts[2]]);
            else
                cout << "WARNING: Unable to find specified function " << parts[2] << " (use case " << parts[0] << ") in the call graphs." << endl;

        }
        fUseCase.close();

        if (useCases.empty())
        {
            cout << "No use cases found, quitting." << endl;
            return 0;
        }

        // get the modified functions
        ifstream fModiFuncs(argv[4]);
        if (!fModiFuncs.is_open())
            cerr << "Unable to open modified functions file: " << argv[4] << endl;

        map<int, string> editedFuncs;
        while (fModiFuncs.good())
        {
            getline(fModiFuncs, line);

            // skip invalid lines; expecting input of type "fileName|methodName"
            if (count(line.begin(), line.end(), '|') != 1)
                continue;

            vector<string> parts;
            boost::split(parts, line, boost::is_any_of("|"));
            
            if (funcs.count(parts[1]) != 0)
                editedFuncs[funcs[parts[1]]] = parts[1];
            else
                cout << "WARNING: Unable to find the edited function " << parts[1] << " in the call graphs." << endl;
        }

        if (editedFuncs.empty())
        {
            cout << "No edited functions found, quitting." << endl;
            return 0;
        }

        // now that we have the directed graph, use cases, and modified functions, try to reconcile them.
        vector<string> outputs;
        // for each use case:
        for (map<string, pair<string, int> >::iterator it = useCases.begin(); it != useCases.end(); it++)
        {
            string useCaseName = it->first;
            string useCaseFunc = it->second.first;
            int useCaseFuncInt = it->second.second;

            // calculate the distance to all nodes via Dijkstra
            vector<int> dists;
            vector<int> prevF;
            dijkstraPaths(useCaseFuncInt, edges, dists, prevF);

            // for all the edited functions, see if there exists a path from this use case
            for (map<int, string>::iterator fit = editedFuncs.begin(); fit != editedFuncs.end(); fit++)
            {
                int funcID = fit->first;

                // if a path exists, add it to our output
                if (dists[funcID] != maxDist)
                {
                    // get the path to this object
                    list<int> path = dijkstraTrace(funcID, prevF);
                    string output = useCaseName;
                    
                    // iterate over each item in the path
                    for (list<int>::iterator pit = path.begin(); pit != path.end(); pit++)
                    {
                        output += "|" + funcNames[*pit];
                    }
                    outputs.push_back(output);
                }
            }
        }

        for (int i = 0; i < outputs.size(); i++)
            cout << outputs[i] << endl;
    }


    return 0;
}

