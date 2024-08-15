CFLAGS = -D TODOREMOVE
CFLAGS += -Wall -Wextra
CFLAGS += -I inc/

SRC = $(wildcard src/*)

.PHONY: all
all:
	 $(CC) $(CFLAGS) examples/basic.cpp $(SRC)

.PHONY: clean
clean:
	rm a.out
