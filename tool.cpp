#include "tool.h"

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return tolower(c); });
    return result;
}

double expected_score(const vector<float>& opponent_ratings, float own_rating) {
    double total = 0.0;
    for (float opponent : opponent_ratings) {
        total += 1.0 / (1.0 + pow(10.0, (opponent - own_rating) / 400.0));
    }
    return total;
}

int performance_rating(const vector<float>& opponent_ratings, float score) {
    double lo = 0.0, hi = 4000.0;

    while (hi - lo > 0.001) {
        double mid = (lo + hi) / 2.0;
        if (expected_score(opponent_ratings, mid) < score)
            lo = mid;
        else
            hi = mid;
    }

    return static_cast<int>((lo + hi) / 2.0 + 0.5);
}

vector<string> splitPgnBlocks(const string& fullText) {
    vector<string> blocks;
    istringstream stream(fullText);
    string line;
    string block;
    bool first = true;

    while (getline(stream, line)) {
        if (startsWith(line, "[Event")) {
            if (!first && !block.empty()) {
                blocks.push_back(block);
                block.clear();
            }
            first = false;
        }
        block += line + "\n";
    }

    if (!block.empty())
        blocks.push_back(block);

    return blocks;
}

bool startsWith(const string& str, const string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

Result stringToResult(const string& str) {
    if (str == "1-0") return Result::White;
    if (str == "0-1") return Result::Black;
    if (str == "1/2-1/2") return Result::Draw;
    return Result::Unknown;
}

string resultToString(Result r) {
    switch (r) {
        case Result::White: return "1-0";
        case Result::Black: return "0-1";
        case Result::Draw: return "1/2-1/2";
        case Result::Unknown: default: return "*";
    }
}

map<string, vector<string>> splitPgnByTournament(const string& pgnPath) {
    ifstream in(pgnPath);
    map<string, vector<string>> tournaments;
    string line, currentBlock;
    string currentEvent;

    while (getline(in, line)) {
        if (line.rfind("[Event ", 0) == 0) {
            if (!currentBlock.empty() && !currentEvent.empty()) {
                tournaments[currentEvent].push_back(currentBlock);
                currentBlock.clear();
            }

            size_t firstQuote = line.find('\"');
            size_t lastQuote = line.rfind('\"');
            currentEvent = (firstQuote != string::npos && lastQuote != string::npos) ?
                line.substr(firstQuote + 1, lastQuote - firstQuote - 1) : "Unknown";
        }
        currentBlock += line + "\n";
        if (line.empty() && !currentEvent.empty()) {
            tournaments[currentEvent].push_back(currentBlock);
            currentBlock.clear();
        }
    }
    if (!currentBlock.empty() && !currentEvent.empty()) {
        tournaments[currentEvent].push_back(currentBlock);
    }
    return tournaments;
}

pair<string, string> splitAnnotated(const string& moveToken) {
    string move, annot;
    size_t i = 0;

    while (i < moveToken.size() && 
          (isalnum(moveToken[i]) || moveToken[i] == '=' || moveToken[i] == '+' || moveToken[i] == '#' || moveToken[i] == '-' || moveToken[i] == 'O')) {
        move += moveToken[i];
        i++;
    }

    while (i < moveToken.size()) {
        if (moveToken[i] == '!' || moveToken[i] == '?') {
            while (i < moveToken.size() && (moveToken[i] == '!' || moveToken[i] == '?')) {
                annot += moveToken[i];
                i++;
            }
            annot += " ";
        }
        else if (moveToken[i] == '$') {
            annot += "$";
            i++;
            while (i < moveToken.size() && isdigit(moveToken[i])) {
                annot += moveToken[i];
                i++;
            }
            annot += " ";
        }
        else if (moveToken[i] == '{') {
            annot += "{";
            i++;
            while (i < moveToken.size() && moveToken[i] != '}') {
                annot += moveToken[i];
                i++;
            }
            if (i < moveToken.size() && moveToken[i] == '}') {
                annot += "}";
                i++;
            }
            annot += " ";
        }
        else {
            i++;
        }
    }
    while (!annot.empty() && isspace(annot.back())) annot.pop_back();
    return {move, annot};
}
