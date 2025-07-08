#include "tournament.h"

// CONSTRUCTOR

Tournament::Tournament()
    : id(0), name(""), city(""), number_of_rounds(0), current_elo(0), perf(0), result("") {}


// DESTRUCTROR

Tournament::~Tournament() = default;

// GETTERS

unsigned int Tournament::getId() const {
    return id;
}

Time Tournament::getTime() const {
    return time;
}

string Tournament::getName() const {
    return name;
}

string Tournament::getCity() const {
    return city;
}

TimeControl Tournament::getTimeControl() const {
    return timecontrol;
}

unsigned int Tournament::getNumberOfRounds() const {
    return number_of_rounds;
}

unsigned int Tournament::getCurrentElo() const {
    return current_elo;
}

unsigned int Tournament::getPerformance() const {
    return perf;
}

vector<Game> Tournament::getGames() const {
    return games;
}

string Tournament::getResult() const{
    return result;
}

// SETTERS

void Tournament::setId(unsigned int i) {
    id = i;
}

void Tournament::setTime(const Time& t) {
    time = t;
}

void Tournament::setName(const string& n) {
    name = n;
}

void Tournament::setCity(const string& c) {
    city = c;
}

void Tournament::setTimeControl(const TimeControl& tc) {
    timecontrol = tc;
}

void Tournament::setNumberofRounds(unsigned int n) {
    number_of_rounds = n;
}

void Tournament::setCurrentElo(unsigned int e) {
    current_elo = e;
}

void Tournament::setPerformance(unsigned int p) {
    perf = p;
}

void Tournament::setGames(vector<Game>& g) {
    games = g;
}

void Tournament::setResult(string& l){
    result = l;
}

// METHODS

void Tournament::addGame(Game& g) {
    games.push_back(g);
}

bool Tournament::toPgn(sqlite3* db, const string& filename) const {
    ofstream out(filename);
    if (!out) return false;

    bool first = true;
    for (const Game& g : games) {

        ostringstream oss;
        g.toPgn(db, oss);
        if (!first) out << endl; 
        out << oss.str();
        first = false;
    }

    out.close();
    return true;
}


bool Tournament::fromPgn(sqlite3* db, const string& filename) {
    ifstream in(filename);
    if (!in) return false;

    ostringstream oss;
    oss << in.rdbuf();
    string fullText = oss.str();
    vector<string> blocks = splitPgnBlocks(fullText); // a block = one game

    if (blocks.empty()) return false;

    // Tournaments infos
    string event, site, date, timeControlStr;
    istringstream tags(blocks[0]);
    string tagLine;
    while (getline(tags, tagLine)) {
        if (startsWith(tagLine, "[Event ")) {
            size_t pos1 = tagLine.find("\""), pos2 = tagLine.rfind("\"");
            if (pos1 != string::npos && pos2 != string::npos && pos2 > pos1)
                event = tagLine.substr(pos1 + 1, pos2 - pos1 - 1);
        } else if (startsWith(tagLine, "[Site ")) {
            size_t pos1 = tagLine.find("\""), pos2 = tagLine.rfind("\"");
            if (pos1 != string::npos && pos2 != string::npos && pos2 > pos1)
                site = tagLine.substr(pos1 + 1, pos2 - pos1 - 1);
        } else if (startsWith(tagLine, "[Date ")) {
            size_t pos1 = tagLine.find("\""), pos2 = tagLine.rfind("\"");
            if (pos1 != string::npos && pos2 != string::npos && pos2 > pos1)
                date = tagLine.substr(pos1 + 1, pos2 - pos1 - 1);
        } else if (startsWith(tagLine, "[TimeControl ")) {
            size_t pos1 = tagLine.find("\""), pos2 = tagLine.rfind("\"");
            if (pos1 != string::npos && pos2 != string::npos && pos2 > pos1)
                timeControlStr = tagLine.substr(pos1 + 1, pos2 - pos1 - 1);
        }
    }

    // Load the games
    vector<Game> loadedGames;
    for (const string& pgn : blocks) {
        istringstream iss(pgn);
        Game g;
        if (g.fromPgn(db, iss))
            loadedGames.push_back(g);
    }

    if (loadedGames.empty()) return false;

    setName(event.empty() ? "Imported Tournament" : event);
    setCity(site.empty() ? "Unknown" : site);
    setTime(date.empty() ? Time("2025.01.01 00:00:00 +00:00") : Time(date + " 00:00:00 +00:00"));
    try {
        setTimeControl(TimeControl(timeControlStr));
    } catch (...) {
        cerr << "Erreur parsing TimeControl : " << timeControlStr << endl;
        return false;
    }
    setNumberofRounds(loadedGames.size());

    // Find the main player (player who have played all the games from the PGN file)
    unordered_map<int, int> playerCounts;
    for (const Game& g : loadedGames) {
        playerCounts[g.getPlayerWhite().getPlayerId()]++;
        playerCounts[g.getPlayerBlack().getPlayerId()]++;
    }

    unsigned int mainPlayerId = 0;
    for (const auto& it : playerCounts) {
        if (it.second == (int)loadedGames.size()) {
            mainPlayerId = it.first;
            break;
        }
    }

    if (mainPlayerId != 0) {
        unsigned int mainElo = 0;
        for (const Game& g : loadedGames) {
            if (g.getPlayerWhite().getPlayerId() == mainPlayerId) {
                mainElo = g.getWhiteElo();
                break;
            }
            if (g.getPlayerBlack().getPlayerId() == mainPlayerId) {
                mainElo = g.getBlackElo();
                break;
            }
        }
        setCurrentElo(mainElo);

        // Compute performance of the main player during the tournament
        vector<float> oppElos;
        float score = 0;
        for (const Game& g : loadedGames) {
            bool isWhite = g.getPlayerWhite().getPlayerId() == mainPlayerId;
            int oppElo = isWhite ? g.getBlackElo() : g.getWhiteElo();
            oppElos.push_back(static_cast<float>(oppElo));

            Result r = g.getResult();
            if ((r == Result::White && isWhite) || (r == Result::Black && !isWhite))
                score += 1;
            else if (r == Result::Draw)
                score += 0.5;
        }
        setPerformance(performance_rating(oppElos, score));
    } else {
        setCurrentElo(0);
        setPerformance(0);
    }

    // === DB === //
    if (!insertIntoDB(db)) {
        cerr << "Error : Cannot insert the tournament" << endl;
        return false;
    }

    for (Game& g : loadedGames) {
        g.setTournamentId(getId());
        if (g.getPlayerWhite().getPlayerId() == 0 || g.getPlayerBlack().getPlayerId() == 0) {
            cerr << "Skipped a game (players not found)" << endl;
            continue;
        }
        if (!g.insertIntoDB(db)) {
            cerr << "Error : Cannot insert a game" << endl;
            return false;
        }
        addGame(g);
    }

    return true;
}

// << OVERLOAD

ostream& operator<<(ostream& os, const Tournament& t) {
    os << "Tournament: " << t.getName() << " (" << t.getId() << ") in " << t.getCity() << endl;
    os << "Date: " << t.getTime() << endl;
    os << "Time Control: " << t.getTimeControl() << endl;
    os << "Rounds: " << t.getNumberOfRounds() << endl;
    os << "Results link: " << t.getResult() << endl;
    os << "Elo: " << t.getCurrentElo() << ", Performance: " << t.getPerformance() << endl;
    if (t.getGames().size() > 0){
        for (const auto& game : t.getGames()) {
        os << game << endl;
        }
    }
    return os;
}

// DB

bool Tournament::insertIntoDB(sqlite3* db) const {
    const char* sql = R"(
        INSERT INTO tournaments (time, name, city, timecontrol, number_of_rounds, current_elo, perf, result)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, time.toString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timecontrol.toString().c_str(), -1, SQLITE_TRANSIENT); 
    sqlite3_bind_int(stmt, 5, number_of_rounds);
    sqlite3_bind_int(stmt, 6, current_elo);
    sqlite3_bind_int(stmt, 7, perf);
    sqlite3_bind_text(stmt, 8, result.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (success) {
        ((Tournament*)this)->id = static_cast<unsigned int>(sqlite3_last_insert_rowid(db));
    }

    sqlite3_finalize(stmt);
    return success;
};

bool Tournament::updateIntoDB(sqlite3* db) const {
    if (id == 0) return false;

    const char* sql = R"(
        UPDATE tournaments SET
            time = ?, name = ?, city = ?, timecontrol = ?, number_of_rounds = ?, 
            current_elo = ?, perf = ?, result = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, time.toString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timecontrol.toString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, number_of_rounds);
    sqlite3_bind_int(stmt, 6, current_elo);
    sqlite3_bind_int(stmt, 7, perf);
    sqlite3_bind_text(stmt, 8, result.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 9, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
};

bool Tournament::deleteFromDB(sqlite3* db) const {
    if (id == 0) return false;

    const char* sql = "DELETE FROM tournaments WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
};

Tournament Tournament::getById(sqlite3* db, unsigned int id) {
    const char* sql = R"(
        SELECT id, time, name, city, timecontrol, number_of_rounds, current_elo, perf, result
        FROM tournaments WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw runtime_error("Failed to prepare SELECT statement.");

    if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind id.");
    }

    Tournament t;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << endl;
        t.id = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
        t.time = Time(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        t.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        t.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* timeControlText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        string tc_str = timeControlText ? timeControlText : "";
        tc_str.erase(remove_if(tc_str.begin(), tc_str.end(), ::isspace), tc_str.end());

        size_t plusPos = tc_str.find('+');
        if (plusPos != string::npos) {
            string baseStr = tc_str.substr(0, plusPos);
            string incStr = tc_str.substr(plusPos + 1);

            int base = stoi(baseStr);
            int inc = stoi(incStr);

            t.timecontrol.setBaseMinutes(base);
            t.timecontrol.setIncrementSeconds(inc);
        }
        t.number_of_rounds = static_cast<unsigned int>(sqlite3_column_int(stmt, 5));
        t.current_elo = static_cast<unsigned int>(sqlite3_column_int(stmt, 6));
        t.perf = static_cast<unsigned int>(sqlite3_column_int(stmt, 7));
        t.result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    } else {
        sqlite3_finalize(stmt);
        throw runtime_error("No tournament found with id " + to_string(id));
    }

    sqlite3_finalize(stmt);
    return t;
};
