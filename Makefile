CXXFLAGS = -D TODOREMOVE
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -I inc/

SRC = $(wildcard src/*)

.PHONY: all
all:
	 $(CXX) $(CXXFLAGS) examples/basic.cpp $(SRC)

.PHONY: clean
clean:
	rm a.out
