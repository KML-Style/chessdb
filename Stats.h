#ifndef STATS_H
#define STATS_H

#include <vector>
#include <set>
#include <sqlite3.h>
#include "player.h"
#include "game.h"
#include "tournament.h"

class Stats {
public:
    struct EloFilter {
        bool enabled;
        int minElo;
        int maxElo;
        bool relativeToPlayer;
        int relativeMargin;

        EloFilter()
            : enabled(false), minElo(0), maxElo(3000),
              relativeToPlayer(false), relativeMargin(0)
        {}
    };

    Stats(sqlite3* db, const Player& player);

    void loadGames();
    void loadTournaments();

    void printGames() const;
    void printTournaments() const;

    void winDrawLossRatio(const EloFilter& filter = EloFilter()) const;
    void winDrawLossByColor(const EloFilter& filter = EloFilter()) const;
    //void perfRatioInTournaments() const;

    void winDrawLossRatioByBucket(int size, bool relativeToPlayer = false) const;

private:
    sqlite3* db;
    Player player;
    vector<Game> games;
    vector<Tournament> tournaments;

    bool gameMatchesElo(const Game& g, const EloFilter& filter) const;
};

#endif // STATS_H
