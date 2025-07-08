#include "stats.h"
#define ELO_MIN 800
#define ELO_MAX 3000

using namespace std;

Stats::Stats(sqlite3* db, const Player& player)
    : db(db), player(player)
{
    loadGames();
    loadTournaments();
}

void Stats::loadGames() {
    games.clear();
    string sql = "SELECT id FROM games WHERE white_id = ? OR black_id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int(stmt, 1, player.getPlayerId());
    sqlite3_bind_int(stmt, 2, player.getPlayerId());

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        unsigned int gameId = sqlite3_column_int(stmt, 0);
        games.push_back(Game::getById(db, gameId));
    }
    sqlite3_finalize(stmt);
}

void Stats::loadTournaments() {
    set<unsigned int> tournamentIds;
    for (const Game& g : games)
        tournamentIds.insert(g.getTournamentId());

    tournaments.clear();
    for (unsigned int tid : tournamentIds) {
        tournaments.push_back(Tournament::getById(db, tid)); 
    }
}

bool Stats::gameMatchesElo(const Game& g, const EloFilter& filter) const {
    if (!filter.enabled) return true;
    bool isWhite = g.getPlayerWhite().getPlayerId() == player.getPlayerId();
    int playerElo = isWhite ? g.getWhiteElo() : g.getBlackElo();
    int oppElo    = isWhite ? g.getBlackElo() : g.getWhiteElo();
    int minElo = filter.minElo;
    int maxElo = filter.maxElo;
    if (filter.relativeToPlayer) {
        minElo = playerElo - filter.relativeMargin;
        maxElo = playerElo + filter.relativeMargin;
    }
    return oppElo >= minElo && oppElo <= maxElo;
}

void Stats::winDrawLossRatio(const EloFilter& filter) const {
    int win = 0, draw = 0, loss = 0, total = 0;
    for (const Game& g : games) {
        if (!gameMatchesElo(g, filter)) continue;
        Result r = g.getResult();
        bool isWhite = g.getPlayerWhite().getPlayerId() == player.getPlayerId();
        if ((isWhite && r == Result::White) || (!isWhite && r == Result::Black)) win++;
        else if (r == Result::Draw) draw++;
        else loss++;
        total++;
    }
    if (total == 0) return;
    cout << "Win: " << win << ", Draw: " << draw << ", Loss: " << loss << ", Total: " << total << endl;
    cout << "Win%: " << 100.0 * win / total << "% | Draw%: " << 100.0 * draw / total << "% | Loss%: " << 100.0 * loss / total << "%" << endl;
}

void Stats::winDrawLossByColor(const EloFilter& filter) const {
    int winW=0, drawW=0, lossW=0, totalW=0;
    int winB=0, drawB=0, lossB=0, totalB=0;

    for (const Game& g : games) {
        if (!gameMatchesElo(g, filter)) continue;
        bool isWhite = g.getPlayerWhite().getPlayerId() == player.getPlayerId();
        Result r = g.getResult();
        if (isWhite) {
            if (r == Result::White) winW++;
            else if (r == Result::Draw) drawW++;
            else lossW++;
            totalW++;
        } else {
            if (r == Result::Black) winB++;
            else if (r == Result::Draw) drawB++;
            else lossB++;
            totalB++;
        }
    }

    cout << "-- White: W " << winW << " / D " << drawW << " / L " << lossW << " / Tot " << totalW << endl;
    if (totalW)
        cout << "     (W%) " << 100.0*winW/totalW << "  (D%) " << 100.0*drawW/totalW << "  (L%) " << 100.0*lossW/totalW << endl;
    cout << "-- Black: W " << winB << " / D " << drawB << " / L " << lossB << " / Tot " << totalB << endl;
    if (totalB)
        cout << "     (W%) " << 100.0*winB/totalB << "  (D%) " << 100.0*drawB/totalB << "  (L%) " << 100.0*lossB/totalB << endl;
}

// void Stats::perfRatioInTournaments() const {
// }

void Stats::printGames() const {
    cout << "=== Games played ===" << endl;
    for (const Game& g : games) {
        cout << g << endl;
    }
}

void Stats::printTournaments() const {
    cout << "=== Tournaments played ===" << endl;
    for (const Tournament& t : tournaments) {
        cout << t << endl;
    }
}

void Stats::winDrawLossRatioByBucket(int bucketSize, bool relative) const {
    if (!relative) {
        for (int minElo = 900; minElo <= 2800; minElo += bucketSize) {
            int maxElo = minElo + bucketSize - 1;
            int win = 0, draw = 0, loss = 0, total = 0;
            for (const Game& g : games) {
                bool isWhite = g.getPlayerWhite().getPlayerId() == player.getPlayerId();
                int oppElo    = isWhite ? g.getBlackElo() : g.getWhiteElo();
                if (oppElo >= minElo && oppElo <= maxElo) {
                    Result r = g.getResult();
                    if ((isWhite && r == Result::White) || (!isWhite && r == Result::Black)) win++;
                    else if (r == Result::Draw) draw++;
                    else loss++;
                    total++;
                }
            }
            if (total > 0) {
                cout << "Elo " << minElo << " - " << maxElo << " : ";
                cout << "Win: " << win << ", Draw: " << draw << ", Loss: " << loss << ", Total: " << total << endl;
                cout << "Win%: " << 100.0 * win / total << "% | "
                    << "Draw%: " << 100.0 * draw / total << "% | "
                    << "Loss%: " << 100.0 * loss / total << "%" << endl;
            }
        }
    } else {
        for (int diff = -500; diff <= 500; diff += bucketSize) {
            int lower = diff;
            int upper = diff + bucketSize - 1;
            int win = 0, draw = 0, loss = 0, total = 0;

            for (const Game& g : games) {
                bool isWhite = g.getPlayerWhite().getPlayerId() == player.getPlayerId();
                int playerElo = isWhite ? g.getWhiteElo() : g.getBlackElo();
                int oppElo    = isWhite ? g.getBlackElo() : g.getWhiteElo();
                int delta = oppElo - playerElo;
                if (delta >= lower && delta <= upper) {
                    Result r = g.getResult();
                    if ((isWhite && r == Result::White) || (!isWhite && r == Result::Black)) win++;
                    else if (r == Result::Draw) draw++;
                    else loss++;
                    total++;
                }
            }
            if (total > 0) {
                cout << "Elo diff " << lower << " to " << upper << " : ";
                cout << "Win: " << win << ", Draw: " << draw << ", Loss: " << loss << ", Total: " << total << endl;
                cout << "Win%: " << 100.0 * win / total << "% | "
                    << "Draw%: " << 100.0 * draw / total << "% | "
                    << "Loss%: " << 100.0 * loss / total << "%" << endl;
            }
        }
    }
}
