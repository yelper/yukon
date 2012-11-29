#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

class DoxygenParse {
public:
    vector<vector<int> > parseCallGraph(string filename, map<string, string> &names);
	void parseFunctionHeaders(string codeDir, vector<string> &files, map<pair<int, int>, string> &lines);
private:
	void printUsageAndExit(int exitCondition);
}