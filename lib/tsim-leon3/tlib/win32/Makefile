CC=gcc

all: app1.exe app2.exe app3.exe 

test: test.c
	sparc-elf-gcc test.c -o test.exe

stanford.exe: stanford.c
	sparc-elf-gcc -O2 stanford.c -o stanford.exe -lm

app1.exe: app1.c simple_io.c
	$(CC) -O2 -o app1.exe app1.c simple_io.c -g \
	-lm -luser32 -lkernel32 -L. -ltsim 

app2.exe: app2.c io.c
	$(CC) -O2 app2.c io.c -o app2.exe -g \
	-lm -luser32 -lkernel32 -L. -ltsim 

app3.exe: app3.c io.c
	$(CC) -O2 app3.c io.c -o app3.exe -g \
	-lm -luser32 -lkernel32 -L. -ltsim 

clean:
	-rm app1.exe app2.exe app3.exe
