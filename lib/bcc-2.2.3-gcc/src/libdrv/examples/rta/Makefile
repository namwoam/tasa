###############################################################################
# RTA example
#
###############################################################################

CC=sparc-gaisler-elf-gcc
CFLAGS  += -qbsp=gr716b
CFLAGS  += -Os
CFLAGS  += -mcpu=leon3
CFLAGS  += -mflat
CFLAGS  += -qnano
CFLAGS  += -qsvt
CFLAGS  += -Wall -Wextra -pedantic

BINARIES=main_cpu.elf rta0.elf rta1.elf

all: $(BINARIES)

main_cpu.elf: main_cpu.c
	$(CC) $< -o $@ $(CFLAGS) -ldrv

rta0.elf: rta0.c
	$(CC) $< -o $@ $(CFLAGS) -T linkcmds-rta0 -ldrv

rta1.elf: rta1.c
	$(CC) $< -o $@ $(CFLAGS) -T linkcmds-rta1 -ldrv

clean:
	rm $(BINARIES)

include ../target.mk
