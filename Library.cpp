#include "Library.h"

void addLibrary(const string& filename, sqlite3* db) {
    auto tournaments = splitPgnByTournament(filename);
    filesystem::create_directory("pgn_by_tournament");

    for (const auto& [eventName, games] : tournaments) {
        string fileName = "pgn_by_tournament/" + eventName + ".pgn";
        for (char& c : fileName) if (c == ' ' || c == '/' || c == '\\') c = '_';

        ofstream out(fileName);
        for (const string& g : games) out << g << endl;
        out.close();

        Tournament t;
        if (t.fromPgn(db, fileName)) {
            cout << "Tournament " << eventName << " imported." << endl;
        } else {
            cout << "Error : Cannot import " << eventName << "." << endl;
        }
    }
}