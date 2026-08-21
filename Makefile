CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread -I.
SDLFLAGS := $(shell pkg-config --cflags --libs sdl2 SDL2_image)

SRC := main.cpp src/bird.cpp src/tree.cpp src/flower.cpp src/season.cpp src/flying_animal.cpp src/weather.cpp src/grass.cpp src/cloud.cpp src/tulip.cpp
OBJ := $(SRC:.cpp=.o)
BIN := screensaver

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SDLFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(shell pkg-config --cflags sdl2 SDL2_image)

clean:
	rm -f $(OBJ) $(BIN)
