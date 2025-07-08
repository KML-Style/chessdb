#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "tool.h"
#include "tournament.h"

using namespace std;

void addLibrary(const string& filename, sqlite3* db);

#endif // LIBRARY_H
