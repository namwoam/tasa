# rpi-rt

To install linux with real-time support on raspberry pi (Raspberry Pi 3).

Reference: https://github.com/remusmp/rpi-rt-kernel, https://www.yoctoproject.org/

## Tasks

1. Build linux with minimal modules and include Preempt-RT patch.
2. Probably will use the Yocto project to include custom applications.
3. Build 27 tasks , priority ranges from 1 to 25. Idle->100, Reserved->255. Note: every tasks are single threaded.
4. Check uart functionality.
5. Lower latency.