# Exe name
TARGET = exe

# Compilator
CXX = g++

# Compilator options
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Object files
OBJS = time.o timecontrol.o player.o tournament.o game.o main.o db.o tool.o library.o stats.o

# Default rule
all: $(TARGET)

# Final link edition
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lsqlite3

# Cpp files compilation
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Config
CONFIG_SRC = config.cpp
CONFIG_BIN = config

config: $(CONFIG_BIN)

$(CONFIG_BIN): $(CONFIG_SRC) $(filter-out main.o, $(OBJS))
	$(CXX) $(CXXFLAGS) -o $(CONFIG_BIN) $(CONFIG_SRC) $(filter-out main.o, $(OBJS)) -lsqlite3

run_config: config
	./$(CONFIG_BIN)

# Clean
clean:
	rm -f *.o $(TARGET) $(CONFIG_BIN)

