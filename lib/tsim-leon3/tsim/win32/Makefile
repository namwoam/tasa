
all: io.dll ahb.dll

io.dll: 
	make CFLAGS="-g -O2" \
	LINKTSIMLIB=tsim CC=mingw32-gcc io-dll-mingw
io-dll-mingw: io.o 
	$(dir $(shell which mingw32-gcc))/dllwrap --output-lib=io.lib --dllname=io.dll --driver-name=gcc \
	$^ -lm  -luser32 -lkernel32 -L. -l$(LINKTSIMLIB)

ahb.dll: 
	make CFLAGS="-g -O2 " \
	LINKTSIMLIB=tsim CC=mingw32-gcc ahb-dll-mingw

ahb-dll-mingw: ahb.o 
	$(dir $(shell which mingw32-gcc))/dllwrap --output-lib=ahb.lib --dllname=ahb.dll --driver-name=gcc \
	$^ -lm -lws2_32 -luser32 -lkernel32 -L. -l$(LINKTSIMLIB)


clean:
	rm -f io.dll ahb.dll *.o *.lib
