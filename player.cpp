#include "player.h"

// CONSTRUCTOR
Player::Player() {
    playerId = 0;
    name = "";
    firstname = "";
    federation = "";
    borndate = Time();
    for (int i = 0; i < 3; ++i) {
        elo[i] = 0;
        typeElo[i] = "F";
    }
}

// DESTRUCTOR
Player::~Player() {}

// GETTERS
unsigned int Player::getPlayerId() const {
    return playerId;
}

string Player::getName() const {
    return name;
}

string Player::getFirstname() const {
    return firstname;
}

Time Player::getBornDate() const {
    return borndate;
}

string Player::getFederation() const {
    return federation;
}

const unsigned int* Player::getElo() const {
    return elo;
}

const string* Player::getTypeElo() const {
    return typeElo;
}

// SETTERS
void Player::setPlayerId(const unsigned int id) {
    playerId = id;
}

void Player::setName(const string& n) {
    name = n;
}

void Player::setFirstname(const string& f) {
    firstname = f;
}

void Player::setBornDate(const Time& d) {
    borndate = d;
}

void Player::setFederation(const string& f) {
    federation = f;
}

void Player::setElo(const unsigned int* e) {
    elo[0] = e[0];
    elo[1] = e[1];
    elo[2] = e[2];
}

void Player::setTypeElo(const string* e) {
    typeElo[0] = e[0];
    typeElo[1] = e[1];
    typeElo[2] = e[2];
}


// << OVERLOAD
ostream& operator<<(ostream& os, const Player& p) {
    os << "ID: " << p.playerId << endl;
    os << "Name: " << p.firstname << " " << p.name << endl;
    os << "Born: " << p.borndate << endl;
    os << "Federation: " << p.federation << endl;
    os << "Elo (Standard): " << p.elo[0] << " (" << p.typeElo[0] << ")" << endl;
    os << "Elo (Rapid): " << p.elo[1] << " (" << p.typeElo[1] << ")" << endl;
    os << "Elo (Blitz): " << p.elo[2] << " (" << p.typeElo[2] << ")" << endl;
    return os;
}

void Player::printSummary() const {
    std::cout << "ID: " << getPlayerId() << " | " << getName() << " " << getFirstname() << std::endl;
}


// DB

bool Player::insertIntoDB(sqlite3* db) const {
    const char* query = R"(
        INSERT INTO players (
            name, firstname, borndate, federation,
            elo_std, elo_rapid, elo_blitz,
            type_elo_std, type_elo_rapid, type_elo_blitz
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, firstname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, borndate.toString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, federation.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, 5, elo[0]);
    sqlite3_bind_int(stmt, 6, elo[1]);
    sqlite3_bind_int(stmt, 7, elo[2]);

    sqlite3_bind_text(stmt, 8, typeElo[0].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, typeElo[1].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, typeElo[2].c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    ((Player*)this)->playerId = sqlite3_last_insert_rowid(db);
    return true;
}

bool Player::updateIntoDB(sqlite3* db) const {
    const char* query = R"(
        UPDATE players SET
            name = ?, firstname = ?, borndate = ?, federation = ?,
            elo_std = ?, elo_rapid = ?, elo_blitz = ?,
            type_elo_std = ?, type_elo_rapid = ?, type_elo_blitz = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, firstname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, borndate.toString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, federation.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, 5, elo[0]);
    sqlite3_bind_int(stmt, 6, elo[1]);
    sqlite3_bind_int(stmt, 7, elo[2]);

    sqlite3_bind_text(stmt, 8, typeElo[0].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, typeElo[1].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, typeElo[2].c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_int(stmt, 11, playerId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Player::deleteFromDB(sqlite3* db) const {
    if (playerId == 0) return false;

    const char* query = "DELETE FROM players WHERE id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, playerId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

Player Player::getById(sqlite3* db, unsigned int id) {
    const char* sql = "SELECT id, name, firstname, borndate, federation, elo_std, elo_rapid, elo_blitz, type_elo_std, type_elo_rapid, type_elo_blitz FROM Players WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw runtime_error(std::string("Failed to prepare SELECT statement: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error(std::string("Failed to bind playerId: ") + sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        Player p;
        p.playerId = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.firstname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string borndateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        p.borndate = Time(borndateStr);
        p.federation = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        p.elo[0] = static_cast<unsigned int>(sqlite3_column_int(stmt, 5));
        p.elo[1] = static_cast<unsigned int>(sqlite3_column_int(stmt, 6));
        p.elo[2] = static_cast<unsigned int>(sqlite3_column_int(stmt, 7));
        p.typeElo[0] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        p.typeElo[1] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        p.typeElo[2] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));

        sqlite3_finalize(stmt);
        return p;
    }
    else {
        sqlite3_finalize(stmt);
        throw std::runtime_error("No player found with id " + std::to_string(id));
    }
}

vector<Player> Player::getByName(sqlite3* db, const std::string& name) {
    vector<Player> players;
    sqlite3_stmt* stmt;
    string sql = R"(
    SELECT id, name, firstname, borndate, federation,
           elo_std, elo_rapid, elo_blitz,
           type_elo_std, type_elo_rapid, type_elo_blitz
    FROM Players
    WHERE name LIKE ? OR firstname LIKE ?
)";


    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Erreur préparation requête getByName : " << sqlite3_errmsg(db) << endl;
        return players;
    }

    string likeName = "%" + name + "%";
    sqlite3_bind_text(stmt, 1, likeName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, likeName.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
       Player p;
        p.setPlayerId(sqlite3_column_int(stmt, 0));
        p.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        p.setFirstname(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        p.setBornDate(Time(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))));
        p.setFederation(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));

        unsigned int elos[3] = {
            static_cast<unsigned int>(sqlite3_column_int(stmt, 5)),
            static_cast<unsigned int>(sqlite3_column_int(stmt, 6)),
            static_cast<unsigned int>(sqlite3_column_int(stmt, 7))
        };
        p.setElo(elos);

        string typeElos[3] = {
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10))
        };
        p.setTypeElo(typeElos);

        players.push_back(p);
    }

    sqlite3_finalize(stmt);
    return players;
}

Player Player::getByExactName(sqlite3* db, const string& fullName) {
    istringstream ss(fullName);
    string firstname, name;
    ss >> firstname >> name;

    vector<Player> results = Player::getByName(db, name);
    string targetFirst = toLower(firstname);
    string targetLast = toLower(name);

    for (const auto& p : results) {
        if (toLower(p.getFirstname()) == targetFirst && toLower(p.getName()) == targetLast)
            return p;
    }

    // player not found => create it 
    Player p;
    p.setFirstname(firstname);
    p.setName(name);
    p.insertIntoDB(db);
    return p;
}