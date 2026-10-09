CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra

PKG_CONFIG ?= pkg-config
RAYLIB_PKG_CONFIG_PATH = $(HOME)/.local/lib/pkgconfig:$(PKG_CONFIG_PATH)
RAYLIB_CFLAGS = $(patsubst -I%,-isystem %,$(shell PKG_CONFIG_PATH="$(RAYLIB_PKG_CONFIG_PATH)" $(PKG_CONFIG) --cflags raylib))
RAYLIB_LIBS = $(shell PKG_CONFIG_PATH="$(RAYLIB_PKG_CONFIG_PATH)" $(PKG_CONFIG) --libs raylib)

SOURCES = src/main.cpp src/Game.cpp src/UI.cpp src/utils.cpp src/entities/Player.cpp src/entities/Enemy.cpp src/entities/Tree.cpp src/entities/Coin.cpp src/entities/Animation.cpp src/entities/Collectable.cpp src/entities/NPC.cpp
HEADERS = $(wildcard src/*.hpp src/*.h src/entities/*.hpp src/entities/*.h)

.PHONY: run clean

main: $(SOURCES) $(HEADERS) Makefile
	$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) $(SOURCES) -o main $(RAYLIB_LIBS)

run: main
	./main

clean:
	rm -f main
