#! /bin/bash

echo ""

dev=$1

# Check arguments
if [ -z $dev ]; then
  echo "A small shell script to unload the ftdi_sio kernel driver from an FTDI device."
  echo "Usage: $(basename $0) /dev/ttyUSB0"
  echo ""
  exit
fi

# Find and parse USB device
usb=$(udevadm info --name=$dev --attribute-walk | grep  'KERNELS==\"[0-9]-[0-9\.]\+:[0-9\.]\+\"')
if [ $PIPESTATUS -ne 0 ]; then
  echo ""
  exit
fi
usb=$(expr match $usb '[^"]*"\([^"]\+\)')

# Unbind driver from USB device
echo "Unbinding ftdi_sio from $dev ($usb)"
echo $usb > /sys/bus/usb/drivers/ftdi_sio/unbind
echo ""

