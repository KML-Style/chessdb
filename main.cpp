/*
MIT License

Copyright (c) 2025 KML_Style

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/



#include "tournament.h"
#include "db.h"
#include "player.h"
#include "Library.h"
#include "game.h"
#include "Stats.h"
#include <limits>
#include <filesystem>

Player selectPlayerByName(sqlite3* db, const string& partialName) {
    vector<Player> players = Player::getByName(db, partialName);

    if (players.empty()) {
        cout << "No players found for : " << partialName << endl;
        cout << "Do you want to add the player quickly ? (y/n) : ";
        char reponse;
        cin >> reponse;
        cin.ignore();

        if (reponse == 'y' || reponse == 'Y') {
            Player newPlayer;
            string prenom, nom;
            
            cout << "Name : ";
            getline(cin, prenom);
            newPlayer.setName(nom);
            
            cout << "Firstname : "; 
            getline(cin, prenom);
            newPlayer.setFirstname(prenom);

            if (newPlayer.insertIntoDB(db)) {
                cout << "Inserted player with success." << endl;
                return newPlayer;
            } else {
                cout << "Error : Cannot insert player." << endl;
                return Player();
            }
        } else {
            return Player(); 
        }

    } else if (players.size() == 1) {
        return players[0];
    } else {
        cout << "Several players found :" << endl;
        for (size_t i = 0; i < players.size(); ++i) {
            cout << i + 1 << ". " << players[i].getFirstname() << " " << players[i].getName() << endl;
        }

        cout << "Choose a player (1-" << players.size() << ") : ";
        int choix;
        cin >> choix;
        cin.ignore();

        if (choix < 1 || choix > (int)players.size()) {
            cout << "Invalid choice. 0 player selected." << endl;
            return Player();
        }

        return players[choix - 1];
    }
}


int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    sqlite3* db;
    int rc = sqlite3_open("chess.sqlite", &db);
    if (rc) {
        std::cerr << "Error : Cannot open database : " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    try {
        string sqlScript = readSQLFile("sql/create.sql");
        executeSQLScript(db, sqlScript);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // try {
    //     string sqlScript = readSQLFile("sql/delete.sql");
    //     executeSQLScript(db, sqlScript);
    // } catch (const exception& e) {
    //     cerr << e.what() << endl;
    // }

    string filename;
    int choix;
    do {
        cout << endl << "=== MENU ===" << endl;
        cout << "1. Add a tournament" << endl;
        cout << "2. Add a player" << endl;
        cout << "3. Add a library (PGN)" << endl;
        cout << "4. Print players table" << endl;
        cout << "5. Print tournaments table" << endl;
        cout << "6. Print games table" << endl;
        cout << "7. Show statistics" << endl;
        cout << "0. Leave the application" << endl;
        cout << "Your choice : ";
        cin >> choix;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        
        switch (choix) {
            case 1: {
                Tournament t;

                cout << "Add via a PGN file ? (y/n) : ";
                char reponsePgn;
                cin >> reponsePgn;
                cin.ignore();

                if (reponsePgn == 'y' || reponsePgn == 'Y') {
                    cout << "Name of the PGN file : ";
                    getline(cin, filename);

                    if (t.fromPgn(db, filename)) {
                        if (t.insertIntoDB(db)) {
                            cout << "Tournament inserted (ID : " << t.getId() << ")." << endl;
                        } else {
                            cout << "Error : Cannot insert tournament." << endl;
                        }
                    } else {
                        cout << "Error : Cannot read PGN" << endl;
                    }
                    break;
                }

                cout << "Tournament name : ";
                string tname; getline(cin, tname);
                t.setName(tname);

                cout << "City : ";
                string city; getline(cin, city);
                t.setCity(city);

                cout << "Date (YYYY-MM-DD HH:MM:SS +HH:mm) : ";
                string dateStr; getline(cin, dateStr);
                t.setTime(Time(dateStr));

                cout << "-- Time control --" << endl;
                unsigned int baseMinutes = 0, incrementSeconds = 0;
                cout << "Minutes (base time) : "; cin >> baseMinutes;
                cout << "Seconds of increment : "; cin >> incrementSeconds;
                cin.ignore();
                TimeControl tc;
                tc.setBaseMinutes(baseMinutes);
                tc.setIncrementSeconds(incrementSeconds);
                t.setTimeControl(tc);

                cout << "Number of rounds : ";
                unsigned int rounds; cin >> rounds; cin.ignore();
                t.setNumberofRounds(rounds);

                cout << "Current ELO of the main player : ";
                unsigned int elo; cin >> elo; cin.ignore();
                t.setCurrentElo(elo);

                cout << "Performance of the main player: ";
                unsigned int perf; cin >> perf; cin.ignore();
                t.setPerformance(perf);

                if (!t.insertIntoDB(db)) {
                    cout << "Error : Cannot insert tournament." << endl;
                    break;
                }

                cout << "Tournament inserted with success (ID : " << t.getId() << ")." << endl;

                unsigned int count = 1;
                unsigned int nbRounds = t.getNumberOfRounds();
                int choixPartie;

                do {
                    cout << "\nRound " << count << endl;
                    cout << "1. Add the game from a PGN\n";
                    cout << "2. Add the game quickly (players, ELOs, result)\n";
                    cout << "3. Add the game (with moves list)\n";
                    cout << "0. Finish\n";
                    cout << "Your choice : ";
                    cin >> choixPartie;
                    cin.ignore();

                    if (choixPartie == 0) break;

                    Game g;
                    g.setTournamentId(t.getId());

                    if (choixPartie == 1) {
                        cout << "PGN filename : ";
                        getline(cin, filename);

                        if (g.fromPgn(db, filename)) {
                            if (g.insertIntoDB(db)) {
                                t.addGame(g);
                                cout << "Game added from PGN" << endl;
                                count++;
                                cout << "Do you want to add a link to the game ? (y/n) : ";
                                char reponse; cin >> reponse; cin.ignore();
                                if (reponse == 'y' || reponse == 'Y') {
                                    cout << "Link : ";
                                    string link; getline(cin, link);
                                    g.setLink(link);
                                }
                            }
                        } else {
                            cout << "Error : Cannot read PGN." << endl;
                        }
                    }

                    else if (choixPartie == 2 || choixPartie == 3) {
                        cout << "Name/Firstname white player : ";
                        string whiteName; getline(cin, whiteName);
                        Player white = selectPlayerByName(db, whiteName);
                        if (white.getName().empty()) continue;
                        g.setPlayerWhite(white);

                        cout << "Name/Firstname black player : ";
                        string blackName; getline(cin, blackName);
                        Player black = selectPlayerByName(db, blackName);
                        if (black.getName().empty()) continue;
                        g.setPlayerBlack(black);

                        unsigned int eloW, eloB;
                        cout << "ELO white : "; cin >> eloW;
                        cout << "ELO black  : "; cin >> eloB;
                        cin.ignore();
                        g.setWhiteElo(eloW);
                        g.setBlackElo(eloB);

                        cout << "Result (W/D/B) : ";
                        char res; cin >> res; cin.ignore();
                        g.setResult(res == 'W' || res == 'w' ? Result::White :
                                    res == 'B' || res == 'b' ? Result::Black :
                                    res == 'D' || res == 'd' ? Result::Draw : Result::Unknown);

                        if (choixPartie == 3) {
                            cout << "Set the mooves one by one (white + black). 'End' to stop" << endl;
                            while (true) {
                                string wm, bm;
                                cout << "White : "; getline(cin, wm);
                                if (wm == "end") break;
                                cout << "Black : "; getline(cin, bm);
                                if (bm == "end") break;
                                Move m(g.getMoves().size() + 1, wm, bm, "", "");
                                g.addMove(m);
                            }

                            cout << "Add a link to the game ? (y/n) : ";
                            char reponse; cin >> reponse; cin.ignore();
                            if (reponse == 'y' || reponse == 'Y') {
                                cout << "Link : ";
                                string link; getline(cin, link);
                                g.setLink(link);
                            }
                        }

                        if (g.insertIntoDB(db)) {
                            t.addGame(g);
                            cout << "Game added with success." << endl;
                            count++;
                        }
                    }

                } while (count <= nbRounds);

                break;
            }

            case 2: {
                cout << "Add quickly (q) ou full (f) ? ";
                char typeAjout;
                cin >> typeAjout;
                cin.ignore();

                Player p;
                string nom, prenom;
                cout << "Name : "; getline(cin, nom);
                cout << "Firstname : "; getline(cin, prenom);
                p.setName(nom);
                p.setFirstname(prenom);

                if (typeAjout == 'f' || typeAjout == 'F') {
                    string borndateStr;
                    cout << "Borndate (YYYY-MM-DD HH:MM:SS +HH:mm) : ";
                    getline(cin, borndateStr);
                    p.setBornDate(Time(borndateStr));

                    cout << "Federation : ";
                    string fed; getline(cin, fed);
                    p.setFederation(fed);

                    unsigned int elos[3];
                    string typeElos[3];
                    const char* labels[3] = { "Standard", "Rapid", "Blitz" };

                    for (int i = 0; i < 3; ++i) {
                        cout << "ELO " << labels[i] << " : ";
                        cin >> elos[i]; cin.ignore();
                        cout << "Type ELO " << labels[i] << " (F/N/E) : ";
                        getline(cin, typeElos[i]);
                    }

                    p.setElo(elos);
                    p.setTypeElo(typeElos);
                }

                if (p.insertIntoDB(db)) {
                    cout << "Player inserted with success" << endl;
                } else {
                    cout << "Error : Cannot insert player." << endl;
                }

                break;
            }
            case 3:
                cout << "PGN File : ";
                getline(cin, filename);

                addLibrary(filename, db);
                break;

            case 4:
                printAllFromTable(db, "Players");
                break;

            case 5:
                printAllFromTable(db, "Tournaments");
                break;

            case 6:
                printAllFromTable(db, "Games");
                break;
            
            case 7: {
                cout << "Enter (part of) the name or firstname of the player for stats: ";
                string partialName;
                
                getline(cin, partialName);
                Player selected = selectPlayerByName(db, partialName);

                cout << "Player selected" << endl << selected << endl;

                if (selected.getName().empty()) {
                    cout << "No player selected. Returning to menu." << endl;
                    break;
                }

                Stats stats(db, selected);

                int statChoice = -1;
                do {
                    cout << endl << "=== STATISTICS MENU ===" << endl;
                    cout << "1. Win/Draw/Loss Ratio (all games)" << endl;
                    cout << "2. Win/Draw/Loss Ratio by color" << endl;
                    cout << "3. Win/Draw/Loss by Elo buckets" << endl;
                    cout << "4. Win/Draw/Loss by relative Elo buckets" << endl;
                    cout << "5. Performance ratio in tournaments" << endl;
                    cout << "6. List games" << endl;
                    cout << "7. List tournaments" << endl;
                    cout << "0. Return to main menu" << endl;
                    cout << "Your choice: ";
                    cin >> statChoice;
                    cin.ignore(numeric_limits<std::streamsize>::max(), '\n'); 

                    switch (statChoice) {
                        case 1: {
                            stats.winDrawLossRatio();
                            break;
                        }
                        case 2: {
                            stats.winDrawLossByColor();
                            break;
                        }
                        case 3: {
                            int size = 100;
                            cout << "Enter bucket size (default 100): ";
                            string input;
                            getline(cin, input); 
                            if (!input.empty()) size = stoi(input);
                            stats.winDrawLossRatioByBucket(size, false); 
                            break;
                        }
                        case 4: {
                            int margin = 50;
                            cout << "Enter relative Elo margin (default 50): ";
                            string input;
                            getline(cin, input); 
                            if (!input.empty()) margin = stoi(input);
                            stats.winDrawLossRatioByBucket(margin, true); 
                            break;
                        }
                         case 5: {
                        //     stats.perfRatioInTournaments();
                            cout << "Coming soon !" << endl;
                            break;
                        }
                        case 6: {
                            stats.printGames();
                            break;
                        }
                        case 7: {
                            stats.printTournaments();
                            break;
                        }
                        case 0:
                            break;
                        default:
                            cout << "Unknown choice." << endl;
                    }
                } while (statChoice != 0);

                break;
            }

            case 0:
                cout << "Bye !" << endl;
                break;

            default:
                cout << "Incorrect choice" << endl;
                break;
        }

    } while (choix != 0);

    sqlite3_close(db);
    return 0;
}