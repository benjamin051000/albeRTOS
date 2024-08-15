CXXFLAGS = -D TODOREMOVE
CXXFLAGS += -Wall -Wextra -Wunused -Wshadow=compatible-local -Wpedantic -Wconversion
CXXFLAGS += -I inc/
CXXFLAGS += --std=c++20

SRC = $(wildcard src/*)

.PHONY: all
all:
	 $(CXX) $(CXXFLAGS) examples/basic.cpp $(SRC)

.PHONY: clean
clean:
	rm a.out
