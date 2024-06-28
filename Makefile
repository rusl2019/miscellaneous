# Compiler dan flags
CC = gcc
CFLAGS  = -Wall -Wextra -std=c11
CFLAGS += -L./ncurses/lib -I./ncurses/include
LDFLAGS = -lncursesw

.PHONY: ncurses_systemctl
ncurses_systemctl:
	$(CC) $(CFLAGS) -c ncurses_systemctl.c -o ncurses_systemctl.o
	$(CC) $(CFLAGS) -o ncurses_systemctl ncurses_systemctl.o -lncursesw

.PHONY: clean
clean:
	$(RM) ncurses_systemctl *.o

.PHONY: clean-all
clean-all:
	$(RM) -r ncurses_systemctl *.o ncurses-6.5* ncurses libssh2*

.PHONY: download
download:
	aria2c -x16 "https://invisible-island.net/archives/ncurses/ncurses-6.5.tar.gz"
	aria2c -x16 "https://libssh2.org/download/libssh2-1.11.0.tar.gz"

.PHONY: format
format:
	clang-format -i *.c
