#ifndef GAME_H
#define GAME_H

#include "Time.h"
#include "TimeControl.h"
#include "player.h"
#include "tool.h"
#include <string>
#include <iostream>
#include <climits>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <unordered_map>
using namespace std;

struct Move {
    friend ostream& operator<<(ostream& os, const Move& m);
    unsigned int number;        
    string whiteMove;           
    string blackMove;           
    string whiteComment = "";   
    string blackComment = ""; 
    
    Move(unsigned int num,
         const string& white,
         const string& black,
         const string& whiteCom = "",
         const string& blackCom = "")
        : number(num), whiteMove(white), blackMove(black),
          whiteComment(whiteCom), blackComment(blackCom) {}
    
    Move() = default; 
};


class Game{
    friend ostream& operator<<(ostream& os, const Game& g);
    public:
        Game();
        Game(const string& filename);

        virtual ~Game();

        // METHODS
        //add a move at the end of the moves list 
        void addMove(const Move& move); 
        
        //convert a game from/to a stream corresponding to a pgn
        bool fromPgn(sqlite3* db, istream& in); 
        bool toPgn(sqlite3* db, ostream& out) const;

        // convert a game from/to a pgn file 
        bool toPgn(sqlite3* db, const string& filename) const;
        bool fromPgn(sqlite3* db, const string& filename);

        // convert a string of chess moves from/to the game's move list
        void parseMoves(const string& mooves);
        string serializeMoves() const;

        void print() const;    

        // GETTERS

        unsigned int getGameId() const;
        Player getPlayerWhite() const;
        Player getPlayerBlack() const;
        unsigned int getWhiteElo() const;
        unsigned int getBlackElo() const;
        unsigned int getTournamentId() const;
        Result getResult() const;
        string getLink() const;
        vector<Move> getMoves() const;

        // SETTERS

        void setGameId(const unsigned int id);
        void setPlayerWhite(const Player& w);
        void setPlayerBlack(const Player& b);
        void setWhiteElo(const unsigned int w);
        void setBlackElo(const unsigned int b);
        void setTournamentId(const unsigned int id);
        void setResult(const Result& r);
        void setLink(const string& l);
        void setMoves(const vector<Move>& m);

        // DB
        bool insertIntoDB(sqlite3* db);
        bool updateIntoDB(sqlite3* db);
        bool deleteFromDB(sqlite3* db) const;

        static Game getById(sqlite3* db, unsigned int id);

    protected:
        // ID
        unsigned int gameId;

        // TOURNAMENT
        unsigned int tournamentId;

        // PLAYERS
        Player white;
        Player black;
        unsigned int whiteElo;
        unsigned int blackElo;
        
        // GAME
        vector<Move> moves;
        string link;

        // RESULT
        Result result;
        
};

#endif
