#include "db.h"

string readSQLFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        throw runtime_error("Impossible d'ouvrir le fichier SQL : " + filepath);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void executeSQLScript(sqlite3* db, const string& sqlScript) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sqlScript.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "Erreur lors de l'exécution du script SQL : " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Script SQL exécuté avec succès." << endl;
    }
}

void printAllFromTable(sqlite3* db, const string& tableName) {
    string sql = "SELECT * FROM " + tableName + ";";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int colCount = sqlite3_column_count(stmt);

        for (int i = 0; i < colCount; ++i) {
            const char* colName = sqlite3_column_name(stmt, i);
            const char* colText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));

            cout << colName << ": " << (colText ? colText : "NULL");

            if (i < colCount - 1) cout << ", ";
        }

        cout << endl;
    }

    if (rc != SQLITE_DONE) {
        cerr << "Error reading from table: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
}