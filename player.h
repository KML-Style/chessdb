#ifndef STRING_H
#define STRING_H

#include "Time.h"
#include <sqlite3.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "tool.h"
using namespace std;

class Player {
    friend ostream& operator<<(ostream& os, const Player& p);
    public:
        Player();
        virtual ~Player();

        // GETTERS
        unsigned int getPlayerId() const;
        string getName() const;
        string getFirstname() const;
        Time getBornDate() const;
        string getFederation() const;
        const unsigned int* getElo() const;
        const string* getTypeElo() const;

        // SETTERS
        void setPlayerId(const unsigned int id);
        void setName(const string& n);
        void setFirstname(const string& f);
        void setBornDate(const Time& d);
        void setFederation(const string& f);
        void setElo(const unsigned int* elo);
        void setTypeElo(const string* Telo);

        // DB
        bool insertIntoDB(sqlite3* db) const;
        bool updateIntoDB(sqlite3* db) const;
        bool deleteFromDB(sqlite3* db) const;

        static Player getById(sqlite3* db, unsigned int id);
        static vector<Player> getByName(sqlite3* db, const string& name);
        static Player getByExactName(sqlite3* db, const string& fullname);

        void printSummary() const;
    protected:
        // ID
        unsigned int playerId;

        // INFOS
        string name;
        string firstname;
        Time borndate;
        string federation;

        // ELO
        unsigned int elo[3]; // STANDARD - RAPID - BLITZ
        string typeElo[3]; // 'E', 'N', 'F' 
};



#endif