

all: app1.exe app2.exe app3.exe test stanford.exe

test: test.c
	sparc-rtems-gcc test.c -o test

stanford.exe: stanford.c
	sparc-rtems-gcc -O2 stanford.c -o stanford.exe -lm

app1.exe: app1.c simple_io.c
	gcc -O2 -g app1.c simple_io.c -o app1.exe tsimleon3.o -lm

app2.exe: app2.c io.c
	gcc -O2 -g app2.c io.c -o app2.exe tsimleon3.o -lm

app3.exe: app3.c io.c
	gcc -O2 -g app3.c io.c -o app3.exe tsimleon3.o -lm

clean:
	-rm app1.exe app2.exe app3.exe
