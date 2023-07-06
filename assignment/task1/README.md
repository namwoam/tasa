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
4. Try to load the linux image with tsim emulator, have successful result, but the eval version only have 32bit simulation timestamp, thus is not useable. Check ./emulate.sh for more information.


