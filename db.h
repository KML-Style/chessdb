#ifndef DB_H
#define DB_H

#include <string>
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

string readSQLFile(const string& filepath);
void executeSQLScript(sqlite3* db, const string& sqlScript);
void printAllFromTable(sqlite3* db, const string& tableName);

#endif 