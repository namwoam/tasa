

all: app1 app2 app3

test: test.c
	sparc-elf-gcc test.c -o test

stanford.exe: stanford.c
	sparc-rtems-gcc -O2 stanford.c -o stanford.exe -lm

app1: app1.c simple_io.c
	gcc -O2 -g app1.c simple_io.c -o app1 tsimleon3.o -lm -ldl -lsocket

app2: app2.c io.c
	gcc -O2 -g app2.c io.c -o app2 tsimleon3.o -lm -ldl -lsocket

app3: app3.c io.c
	gcc -O2 -g app3.c io.c -o app3 tsimleon3.o -lm -ldl -lsocket

clean:
	-rm app1 app2 app3
