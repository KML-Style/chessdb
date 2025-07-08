#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "Time.h"
#include "TimeControl.h"
#include "game.h"
#include <string>
#include <iostream>
#include <sqlite3.h>
#include <cmath>
#include <numeric>
#include <vector>
using namespace std;

class Tournament {
    friend ostream& operator<<(ostream& os, const Tournament& t);
    public:
        // CONSTRUCTORS / DESTRUCTORS
        Tournament();
        virtual ~Tournament();

        // GETTERS
        unsigned int getId() const;
        Time getTime() const;
        string getName() const;
        string getCity() const;
        TimeControl getTimeControl() const;
        unsigned int getNumberOfRounds() const;
        unsigned int getCurrentElo() const;
        unsigned int getPerformance() const;
        vector <Game> getGames() const;
        string getResult() const;
        
        // SETTERS
        void setId(unsigned int i);
        void setTime(const Time& t);
        void setName(const string& n);
        void setCity(const string& c);
        void setTimeControl(const TimeControl& tc);
        void setNumberofRounds(unsigned int n);
        void setCurrentElo(unsigned int e);
        void setPerformance(unsigned int p);
        void setGames(vector <Game>& g);
        void setResult(string& l);

        // METHODS

        void addGame(Game& g);
        bool toPgn(sqlite3* db, const string& filename) const;
        bool fromPgn(sqlite3* db, const string& filename);

         // DB
        bool insertIntoDB(sqlite3* db) const;
        bool updateIntoDB(sqlite3* db) const;
        bool deleteFromDB(sqlite3* db) const;

        static Tournament getById(sqlite3* db, unsigned int id);
    protected:
        // ID
        unsigned int id;

        // EVENT
        Time time;
        string name;
        string city;

        // TOURNAMENT INFORMATIONS
        TimeControl timecontrol;
        unsigned int number_of_rounds;

        // PLAYER INFORMATIONS
        unsigned int current_elo;

        // RESULTS
        unsigned int perf;
        vector <Game> games;
        string result;
};

#endif