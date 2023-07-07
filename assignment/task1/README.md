# TASA Internship-Task1
Load the official realtime linux repo onto the development board

## info

1. ./build.sh will build a RAM image with leon-buildroot
2. ./upload.sh will load the RAM image onto the development board

## log

### 2023-07-06

1. Initialize git repo & publish on github
2. Build the linux image with leon-buildroot
3. Encounter error when loading RAM image:
```
Image /home/fsw/rep/tasa/assignment/task1/leon-buildroot/output/images/image.ram loaded
  Loaded 26418 symbols
  Error mode  (tt = 0x02, illegal instruction)
  0x40000800: 91d02000  ta  0  <head_bad_trap+0>
```
use `make gaisler_ut700_defconfig` yield error:
```
Image /home/fsw/rep/tasa/assignment/task1/leon-buildroot/output/images/image.ram loaded
  Loaded 25141 symbols
  Error mode  (tt = 0x2b, data store error)
  0x00000000: 0 
```
Note: push RESET->BREAK on the board to load image. PWR, DSU and ERR LED should be on.

use flag: no-sram yield:
```
  Image /home/fsw/rep/tasa/assignment/task1/leon-buildroot/output/images/image.ram loaded
  Loaded 26418 symbols
  Error mode  (tt = 0x09, data access exception)
  0x00000000: 0  
```
4. Try to load the linux image with tsim emulator, have successful result, but the eval version only have 32bit simulation timestamp, thus is not useable. Check `./emulate.sh` for more information.

5. Discover linux 5.10 is not supported by the UT699 ref: https://www.gaisler.com/index.php/products/sw-overview.

## 2023-07-06

1. Install tsim2
2. Try to use tsim2 to emulate hardware that supports leon linux, we currently have tsim2 version 2.0.7, the active version is 2.0.66. The hardware support of tsim2 2.0.7 provided by the doc are: at697e, gr702rc and tsc691. Although whether gr702rc supports leon linux is not specified on the official gaisler website, by cross-checking the simulation result of tsim3-eval and tsim2 2.0.7, we can confirm that gr702rc does not support leon linux.
    1. `./tsim-eval/tsim/linux-x64/tsim-leon3 -nosram leon-buildroot/output/images/image.ram -sym  leon-buildroot/output/images/vmlinux` yield successful result.
    2. `./tsim-eval/tsim/linux-x64/tsim-leon3 -gr702rc -nosram leon-buildroot/output/images/image.ram -sym  leon-buildroot/output/images/vmlinux` yield:

    ```
    Initializing and starting from 0x40000000
    CPU 0 in error mode: tt=0x80, trap instruction
    Before trap:         tt=0x2b, data store error
        269  40000800  91d02000   ta        0                        arch/sparc/lib/locks.o + 0x40000800
    ```
    3. `tsim-leon3 -nosram -gr702rc leon-buildroot/output/images/image.ram -sym  leon-buildroot/output/images/vmlinux `(use `go 0x40000000` instead of `run` inside the emulator) yield:
    ```
    resuming at 0x40000000
    Memory exception at ffd00000

    Program exited normally.
    tsim> hist
     8715  40000de4  8620c004  sub      %g3, %g4, %g3
     8716  40000de8  8608fff8  and      %g3, 0xfffffff8, %g3
     8717  40000dec  8600e008  add      %g3, 8, %g3
     8718  40000df0  8600c004  add      %g3, %g4, %g3
     8719  40000df4  c0204000  clr      [%g1]
     8729  40000090  108001dc  ba       0x40000800
     8736  40000094  90102009  mov      9, %o0
     8737  40000800  91d02000  ta       0x0
    ```
    Thus conclude that we can't use tsim2 2.0.7 to emulate leon linux.
3. 



