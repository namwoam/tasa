# rpi-rt

To install linux with real-time support on raspberry pi (Raspberry Pi 3).

Reference: https://github.com/remusmp/rpi-rt-kernel, https://www.yoctoproject.org/

## Tasks

1. Build linux with minimal modules and include Preempt-RT patch.
2. Probably will use the Yocto project to include custom applications.
3. Build 27 tasks , priority ranges from 1 to 25. Idle->100, Reserved->255. Note: every tasks are single threaded.
4. Check uart functionality.
5. Lower latency.

## Worklog

### 2023-07-10

1. Build rpi-rt-kernel and install it onto the raspberry pi sbc.
2. Tested the latency, the worst-case latency is roughly 2500us (2.5ms), which is significantly better than non-rt kernel (worst case 12000us), full report is at `/rpi-rt/utils/latency`.
3. Install and build the Yocto project.

### 2023-07-11

1. Setup ssh development environmnent for rpi.
2. Test gpio capability with python and c (`gpiod` library).
3. Write program to test real-time latency (not tested yet).\
4. Yecto doesn't support preempt-rt natively.

### 2023-07-12

1. Try the os scheduler with real-time features, check `./utils/scheduling`.
2. Read fsw code.