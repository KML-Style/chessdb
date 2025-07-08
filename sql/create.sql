CREATE TABLE players (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,
    firstname TEXT,
    borndate TEXT,          
    federation TEXT,
    elo_std INTEGER,
    elo_rapid INTEGER,
    elo_blitz INTEGER,
    type_elo_std TEXT,
    type_elo_rapid TEXT,
    type_elo_blitz TEXT
);


CREATE TABLE tournaments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    time TEXT,
    name TEXT,
    city TEXT,
    timecontrol TEXT,
    number_of_rounds INTEGER,
    current_elo INTEGER,
    perf INTEGER,
    result TEXT
);

CREATE TABLE games (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tournament_id INTEGER,
    white_id INTEGER,
    black_id INTEGER,
    white_elo INTEGER,
    black_elo INTEGER,
    moves TEXT,
    link TEXT,
    result TEXT
);
