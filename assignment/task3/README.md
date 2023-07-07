# TASA Internship-Task3
Write a custom c/c++ program to the UT699 development board.

## info

1. `./fib_benchmark.cpp` is the source code of our program.
2. `./compile.sh` or `Makefile` will compile our code to `.elf` format.

todo:

3. use `mklinuximg` to generate bootable image.
4. use tsim2 to emulate the result.
5. test POSIX threads capability.

## log

### 2023-07-06

1. Finish the main program `./fib_benchmark.cpp`.
2. Work around `bcc-2.2.3-gcc` library, examples at `bcc-2.2.3-gcc/src/examples`

### 2023-07-07

1. Add bcc to `$PATH` by editing `.bashrc`, append the line:
```
export PATH=$PATH:/home/fsw/rep/tasa/lib/bcc-2.2.3-gcc/bin
```

2. Build bcc examples for ut699
3. Install tsim2