# the compiler: gcc or what ever compiler is in CC env
CC = gcc

# compiler flags:
#  -g        adds debugging information to the executable file
#  -Wall     turns on most, but not all, compiler warnings
#  -std=c99  c99 standard
#  ncursesw args from pkg-config
CFLAGS  = -O3 -Wall -std=c99 $(shell pkg-config --cflags ncursesw)

all: executable

debug: CFLAGS += -DDEBUG -g
debug: executable

# linker flags
#  -lncurses link ncurses for the terminal interface
LDFLAGS  = -lncursesw

executable: pulopulo.c
	$(CC) $(CFLAGS) -o pulopulo pulopulo.c $(LDFLAGS)

clean:
	$(RM) pulopulo 
