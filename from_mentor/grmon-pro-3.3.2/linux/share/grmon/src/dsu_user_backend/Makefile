CC=gcc
CFLAGS=-O2 -Wall
LDFLAGS=-shared

# Build platform variables
RM=rm -f
ifeq ($(OS),Windows_NT)
ifneq ($(MAKE_MODE),unix)
	RM=del /q
endif
endif

# Target platform variables
ifneq (, $(findstring mingw, $(shell $(CC) -dumpmachine)))
	EXT=.dll
else
	EXT=.so
	CFLAGS+=-fPIC
endif

all: my_io$(EXT)

my_io$(EXT): my_io.o
	$(CC) $(LDFLAGS) my_io.o -o $@

my_io.o: my_io.c
	$(CC) $(CFLAGS) -c my_io.c -o $@

clean:
	-$(RM) my_io$(EXT) my_io.o
