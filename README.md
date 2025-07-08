# Chess Tournament Statistics Manager

This project is a C++ application for managing chess tournaments, games, and players, all stored in a SQLite database. 
It allows you to **import, add, and manage tournaments and games**, either manually or from PGN files, and provides various statistics and performance metrics for players.

→ The project is one-player oriented , which means that the additions of tournaments and games correspond to those **played by a player**, with the aim of obtaining a database of his games/tournaments and statistics... and not player/tournament management for a club

## Features

- **Database Management**  
  Store all your chess tournaments, games, and players in a robust SQLite database.

- **Flexible Data Entry**  
 Import games, tournaments or libraries (which brings together several tournaments) from PGN files or manually.

- **Statistical Analysis**  
  Get detailed statistics for any player (normally used for the player who stores his games in the DB), such as:
  - Win/draw/loss ratios (global, by color or by relative or absolute Elo-range)
  - Performance ratings and tournament performance breakdowns *(Coming soon)*
  - Results by round number *(Coming soon)*

- **Command-Line Interface**  
  Intuitive menu-driven console UI for entering and viewing data.

## How it works

- You can **add tournaments and games** interactively via the menu, or import large PGN files which are automatically split into per-tournament files and added to the database.
- **Players** are referenced by name and created on-the-fly if missing.
- **Stats** can be generated for any player, with flexible filtering.

## Build

- Requires a C++17 compiler.
- The project is built via a Makefile

**To compile:**
Config the database with
```sh
make config
```
And then the application with
```sh
make
```

## Use
```sh
./exe
```
to launch the application

## Planned Features

- Export DB to a PGN library
- Advanced stats (more detailled filtering, ratio by round number, and tournaments performance stats)
- Cross-platform GUI (Qt) in development

## License
MIT License.




