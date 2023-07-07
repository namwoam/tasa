

all: io.so  ahb.so

fp.so : end.h fp.c
	gcc -c -fPIC -g -O6 fp.c -o fp.o
	gcc -shared fp.o -o fp.so

io.so : end.h io.c
	gcc -c -fPIC -g -O6 io.c -o io.o
	gcc -shared io.o -o io.so

ahb.so : end.h ahb.c
	gcc -c -fPIC -g -O6 ahb.c -o ahb.o
	gcc -shared ahb.o -o ahb.so

io.dll:
	gcc -c -fPIC -g -O6 io.c -o io.o
	dllwrap io.o ../tlib/win32/libtsim.lib -o io.dll
ahb.dll:
	gcc -c -fPIC -g -O6 ahb.c -o ahb.o
	dllwrap ahb.o ../tlib/win32/libtsim.lib -o ahb.dll
clean:
	-rm io.so io.o  fp.o fp.so ahb.o ahb.so io.dll ahb.dll
