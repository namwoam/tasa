ifeq ($(OS),Windows_NT)
CC=sparc-gaisler-elf-gcc.exe
RM=del
else
CC=sparc-gaisler-elf-gcc
RM=rm -f
endif
CFLAGS=-g -O0 -Wall
all: hello.elf
clean:
$(RM) hello.elf
hello.elf: hello.c
$(CC) $(CFLAGS) -o $@ $<