#include "Game.h"
#include "tournament.h"

// CONSTRUCTORS
Game::Game()
    : gameId(0), tournamentId(UINT_MAX), whiteElo(0), blackElo(0), result(Result::Unknown) {}

Game::Game(const string& filename)
    {

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open the file " << filename << endl;
        return;
    }

    string line;
    string movesSection;
    bool inMoves = false;
    string eventName, site, date;

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (line[0] == '[') {

            size_t firstQuote = line.find('"');
            size_t lastQuote = line.rfind('"');
            string value = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);

            if (line.find("White ") != string::npos)
                //search player in db 
                white.setName(value);
            else if (line.find("Black ") != string::npos)
                //search player in db 
                black.setName(value);
            else if (line.find("WhiteElo") != string::npos)
                whiteElo = stoi(value);
            else if (line.find("BlackElo") != string::npos)
                blackElo = stoi(value);
            else if (line.find("Result") != string::npos) {
                if (value == "1-0") result = Result::White;
                else if (value == "0-1") result = Result::Black;
                else if (value == "1/2-1/2") result = Result::Draw;
                else result = Result::Unknown;
            }
            else if (line.find("Event") != string::npos){
                eventName = value;
            }
            else if (line.find("Site") != string::npos){
                site = value;
            }
            else if (line.find("Date") != string::npos){
                date = value;
            }
        } else {
            inMoves = true;
            movesSection += line + " ";
        }
    }
    gameId = 0; 

    file.close();

    if (inMoves) {
        parseMoves(movesSection);
    }
}

// DESTRUCTOR
Game::~Game() {}

// GETTERS
unsigned int Game::getGameId() const {
    return gameId;
}

Player Game::getPlayerWhite() const {
    return white;
}

Player Game::getPlayerBlack() const {
    return black;
}

unsigned int Game::getWhiteElo() const {
    return whiteElo;
}

unsigned int Game::getBlackElo() const {
    return blackElo;
}

unsigned int Game::getTournamentId() const {
    return tournamentId;
}

Result Game::getResult() const {
    return result;
}

string Game::getLink() const {
    return link;
}

vector<Move> Game::getMoves() const {
    return moves;
}

// SETTERS
void Game::setGameId(const unsigned int id) {
    gameId = id;
}

void Game::setPlayerWhite(const Player& w) {
    white = w;
}

void Game::setPlayerBlack(const Player& b) {
    black = b;
}

void Game::setWhiteElo(const unsigned int w) {
    whiteElo = w;
}

void Game::setBlackElo(const unsigned int b) {
    blackElo = b;
}

void Game::setTournamentId(const unsigned int id) {
    tournamentId = id;
}

void Game::setResult(const Result& r) {
    result = r;
}

void Game::setLink(const string& l) {
    link = l;
}

void Game::setMoves(const vector<Move>& m) {
    moves = m;
}

// METHOD
void Game::addMove(const Move& move) {
    moves.push_back(move);
}

bool Game::toPgn(sqlite3* db, const string& filename) const{
    ofstream out(filename);
    if (!out) return false;

    Tournament t;
    try {
        t = Tournament::getById(db, tournamentId);
    } catch (const std::exception& e) {
        cerr << "Cannot get the tournament linked to the game: " << e.what() << endl;
        return false;
    }

    out << "[Event \"" << t.getName() << "\"]" << endl;
    out << "[Site \"" << t.getCity() << "\"]" << endl;
    out << "[Date \"" << t.getTime() << "\"]" << endl;
    out << "[Round \"" << t.getNumberOfRounds() << "\"]" << endl;
    out << "[White \"" << white.getFirstname() << " " << white.getName() << "\"]" << endl;
    out << "[WhiteElo \"" << whiteElo << "\"]" << endl;
    out << "[Black \"" << black.getFirstname() << " " << black.getName() << "\"]" << endl;
    out << "[BlackElo \"" << blackElo << "\"]" << endl;
    out << "[Result \"" << resultToString(result) << "\"]" << endl;
    out << "[TimeControl \"" << t.getTimeControl().toString() << "\"]" << endl;

    if (!link.empty()) out << "[Link \"" << link << "\"]" << endl;
    out << endl;

    for (const Move& m : moves) {
        out << m.number << ". " << m.whiteMove << " ";
        if (!m.whiteComment.empty()) out << "{" << m.whiteComment << "} ";
        out << m.blackMove << " ";
        if (!m.blackComment.empty()) out << "{" << m.blackComment << "} ";
    }

    out << resultToString(result) << endl;

    out.close();
    return true;
}

bool Game::fromPgn(sqlite3* db, const std::string& filename) {
    ifstream in(filename);
    if (!in) return false;

    string line;
    unordered_map<string, string> tags;
    ostringstream movesStream;
    bool inTags = true;

    while (getline(in, line)) {
        if (line.empty()) {
            inTags = false;
            continue;
        }

        if (inTags && line[0] == '[') {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.rfind('"');
            if (firstQuote != string::npos && lastQuote != string::npos && lastQuote > firstQuote) {
                string tag = line.substr(1, line.find(' ') - 1);
                string value = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                tags[tag] = value;
            }
        } else {
            movesStream << line << " ";
        }
    }

    if (tags.count("White")) {
        Player w = Player::getByExactName(db, tags["White"]);
        setPlayerWhite(w);
    }

    if (tags.count("Black")) {
        Player b = Player::getByExactName(db, tags["Black"]);
        setPlayerBlack(b);
    }

    if (tags.count("WhiteElo")) setWhiteElo(stoi(tags["WhiteElo"]));
    if (tags.count("BlackElo")) setBlackElo(stoi(tags["BlackElo"]));
    if (tags.count("Result")) setResult(stringToResult(tags["Result"]));
    if (tags.count("Link")) setLink(tags["Link"]);

    string allMoves = movesStream.str();
    parseMoves(allMoves);
    
    return true;
}

string Game::serializeMoves() const {
    /**
     * Converts the game's list of moves into a single PGN-like string.
     *
     * @return A string representing all moves of the game in PGN style.
     */
    string m;
    for (const Move& move : moves) {
        m += std::to_string(move.number) + ". ";

        m += move.whiteMove + move.whiteComment + " ";
        m += move.blackMove + move.blackComment + " ";
    }
    return m;
}

void Game::parseMoves(const string& movesStr) {
    /**
     * Parses a string of chess moves in PGN-like format and populates the game's move list.
     *
     * This function reads a sequence of moves (including move numbers and annotations) from a string. 
     * For each move pair (white and black), it extracts the moves, any associated comments or annotations, 
     * and adds them as Move objects to the Game.
     *
     * - Both white and black moves are handled, and odd/even turns are respected.
     * - The function can handle incomplete move pairs at the end (i.e., if a black move is missing).
     */
    istringstream iss(movesStr);
    string token;
    unsigned int currentNumber = 0;
    string whiteMove, blackMove;
    string whiteComment, blackComment;
    bool expectingWhite = true;
    bool expectingBlack = false;

    auto splitAnnotated = [](const string& moveToken) -> pair<string, string> {
        smatch match;
        regex pattern(R"(([a-zA-Z0-9+=#]+)([!?]{1,2})?)");
        if (regex_match(moveToken, match, pattern)) {
            string move = match[1].str();
            string annot = match[2].matched ? match[2].str() : "";
            return {move, annot};
        }
        return {moveToken, ""}; 
    };

    while (iss >> token) {
        if (token[0] == '{') {
            string comment = token;
            while (comment.back() != '}' && iss >> token)
                comment += " " + token;
            comment = comment.substr(1, comment.size() - 2); // Trim {}
            if (expectingBlack)
                blackComment += (blackComment.empty() ? "" : " ") + comment;
            else
                whiteComment += (whiteComment.empty() ? "" : " ") + comment;
        }
        else if (token.back() == '.' && all_of(token.begin(), token.end() - 1, ::isdigit)) {
            currentNumber = stoi(token.substr(0, token.size() - 1));
            expectingWhite = true;
            expectingBlack = false;
        }
        else {
            auto [cleanMove, annot] = splitAnnotated(token);

            if (expectingWhite) {
                whiteMove = cleanMove;
                if (!annot.empty()) whiteComment = annot;
                expectingWhite = false;
                expectingBlack = true;
            } else if (expectingBlack) {
                blackMove = cleanMove;
                if (!annot.empty()) blackComment = annot;
                Move m(currentNumber, whiteMove, blackMove, whiteComment, blackComment);
                addMove(m);
                
                // Reset
                whiteMove.clear(); blackMove.clear();
                whiteComment.clear(); blackComment.clear();
                expectingBlack = false;
            }
        }
    }

    if (!whiteMove.empty() && !blackMove.empty()) {
        Move m(currentNumber, whiteMove, blackMove, whiteComment, blackComment);
        addMove(m);
        
    }
}

bool Game::fromPgn(sqlite3* db, std::istream& in) {
    string line;
    unordered_map<string, string> tags;
    ostringstream movesStream;
    bool inTags = true;

    while (getline(in, line)) {
        if (line.empty()) {
            inTags = false;
            continue;
        }

        if (inTags && line[0] == '[') {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.rfind('"');
            if (firstQuote != string::npos && lastQuote != string::npos && lastQuote > firstQuote) {
                string tag = line.substr(1, line.find(' ') - 1);
                string value = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                tags[tag] = value;
            }
        } else {
            movesStream << line << " ";
        }
    }

    if (tags.count("White")) {
        Player w = Player::getByExactName(db, tags["White"]);
        setPlayerWhite(w);
    }

    if (tags.count("Black")) {
        Player b = Player::getByExactName(db, tags["Black"]);
        setPlayerBlack(b);
    }

    if (tags.count("WhiteElo")) setWhiteElo(stoi(tags["WhiteElo"]));
    if (tags.count("BlackElo")) setBlackElo(stoi(tags["BlackElo"]));
    if (tags.count("Result")) setResult(stringToResult(tags["Result"]));
    if (tags.count("Link")) setLink(tags["Link"]);

    string allMoves = movesStream.str();
    parseMoves(allMoves);

    return true;
}

bool Game::toPgn(sqlite3* db, ostream& out) const {
    Tournament t;
    try {
        t = Tournament::getById(db, tournamentId);
    } catch (const exception& e) {
        cerr << "Error : Cannot get the tournament linked to the game: " << e.what() << endl;
        return false;
    }

    out << "[Event \"" << t.getName() << "\"]" << endl;
    out << "[Site \"" << t.getCity() << "\"]" << endl;
    out << "[Date \"" << t.getTime() << "\"]" << endl;
    out << "[Round \"" << t.getNumberOfRounds() << "\"]" << endl;
    out << "[White \"" << white.getFirstname() << " " << white.getName() << "\"]" << endl;
    out << "[WhiteElo \"" << whiteElo << "\"]" << endl;
    out << "[Black \"" << black.getFirstname() << " " << black.getName() << "\"]" << endl;
    out << "[BlackElo \"" << blackElo << "\"]" << endl;
    out << "[Result \"" << resultToString(result) << "\"]" << endl;
    out << "[TimeControl \"" << t.getTimeControl().toString() << "\"]" << endl;

    if (!link.empty()) out << "[Link \"" << link << "\"]" << endl;
    out << endl;

    for (const Move& m : moves) {
        out << m.number << ". " << m.whiteMove;
        if (!m.whiteComment.empty()) out << " {" << m.whiteComment << "}";
        out << " " << m.blackMove;
        if (!m.blackComment.empty()) out << " {" << m.blackComment << "}";
        out << " ";
    }

    out << resultToString(result) << endl;

    return true;
}

// << OPERATOR
ostream& operator<<(ostream& os, const Game& g) {
    os << g.white.getName() << " (" << g.whiteElo << ") ";
    switch (g.result) {
        case Result::White: os << "1-0 "; break;
        case Result::Draw:  os << "1/2-1/2 "; break;
        case Result::Black: os << "0-1 "; break;
        case Result::Unknown: default: os << "?-? "; break;
    }
    os << g.black.getName() << " (" << g.blackElo << ")" << endl;
    return os;
}

ostream& operator<<(ostream& os, const Move& m) {
    os << m.number << ". " << m.whiteMove << m.whiteComment << " " << m.blackMove << m.blackComment << " ";
    return os;
}

// DB

bool Game::insertIntoDB(sqlite3* db) {
    const char* query = R"(
        INSERT INTO games (
            tournament_id, white_id, black_id,
            white_elo, black_elo, moves, link, result
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, tournamentId);
    sqlite3_bind_int(stmt, 2, white.getPlayerId()); 
    sqlite3_bind_int(stmt, 3, black.getPlayerId());
    sqlite3_bind_int(stmt, 4, whiteElo);
    sqlite3_bind_int(stmt, 5, blackElo);
    sqlite3_bind_text(stmt, 6, serializeMoves().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, link.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, resultToString(result).c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    gameId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return true;
}

bool Game::updateIntoDB(sqlite3* db) {
    const char* query = R"(
        UPDATE games SET
            tournament_id = ?, white_id = ?, black_id = ?,
            white_elo = ?, black_elo = ?, moves = ?, link = ?, result = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, tournamentId);
    sqlite3_bind_int(stmt, 2, white.getPlayerId());
    sqlite3_bind_int(stmt, 3, black.getPlayerId());
    sqlite3_bind_int(stmt, 4, whiteElo);
    sqlite3_bind_int(stmt, 5, blackElo);
    sqlite3_bind_text(stmt, 6, serializeMoves().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, link.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, resultToString(result).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 9, gameId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Game::deleteFromDB(sqlite3* db) const {
    const char* query = "DELETE FROM games WHERE id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, gameId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

Game Game::getById(sqlite3* db, unsigned int id) {
    const char* query = R"(
        SELECT id, tournament_id, white_id, black_id,
               white_elo, black_elo, moves, link, result
        FROM games WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Failed to prepare statement");

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("No game found");
    }

    Game g;
    g.gameId = sqlite3_column_int(stmt, 0);
    g.tournamentId = sqlite3_column_int(stmt, 1);

    unsigned int whiteId = sqlite3_column_int(stmt, 2);
    unsigned int blackId = sqlite3_column_int(stmt, 3);
    g.white = Player::getById(db, whiteId);
    g.black = Player::getById(db, blackId);

    g.whiteElo = sqlite3_column_int(stmt, 4);
    g.blackElo = sqlite3_column_int(stmt, 5);

    string movesStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    g.parseMoves(movesStr);

    g.link = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    g.result = stringToResult(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)));

    sqlite3_finalize(stmt);
    return g;
}