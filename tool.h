#ifndef TOOL_H
#define TOOL_H

#include <string>
#include <algorithm>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

enum class Result {
    White,
    Draw,
    Black,
    Unknown
};

//USED TOOLS IN THE APPLICATION

string toLower(const string& str);
double expected_score(const std::vector<float>& opponent_ratings, float own_rating);
int performance_rating(const std::vector<float>& opponent_ratings, float score);
vector<string> splitPgnBlocks(const string& fullText);
bool startsWith(const string& str, const string& prefix);
Result stringToResult(const string& str);
string resultToString(Result r);
map<string, vector<string>> splitPgnByTournament(const string& pgnPath);
pair<string, string> splitAnnotated(const string& moveToken);

#endif
