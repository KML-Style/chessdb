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

#include <sqlite3.h>
#include <iostream>
#include "db.h"
using namespace std;

int main(){
    sqlite3* db;
    if (sqlite3_open("test/chess.sqlite", &db)) {
        cerr << "Erreur ouverture DB : " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    try {
        string sqlScript = readSQLFile("sql/create.sql");
        executeSQLScript(db, sqlScript);
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    try {
        string sqlScript = readSQLFile("sql/constraints.sql");
        executeSQLScript(db, sqlScript);
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    cout << "DB Config : OK" << endl;
    return 1;
}
