CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -fopenmp -I. -MMD -MP
SDLFLAGS := $(shell pkg-config --cflags --libs sdl2 SDL2_image)

SRC := main.cpp src/bird.cpp src/cat.cpp src/tree.cpp src/leaf.cpp src/flower.cpp src/season.cpp src/flying_animal.cpp src/weather.cpp src/wildlife_update.cpp src/grass.cpp src/cloud.cpp src/tulip.cpp src/celestial_body.cpp src/star.cpp src/performance.cpp
OBJ := $(SRC:.cpp=.o)
DEP := $(OBJ:.o=.d)
BIN := screensaver

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SDLFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(shell pkg-config --cflags sdl2 SDL2_image)

clean:
	rm -f $(OBJ) $(DEP) $(BIN)

-include $(DEP)
