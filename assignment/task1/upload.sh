# use grmon to conect to GR-UT699
#sudo ./grmon -nb -baud 460800 -uart /dev/ttyUSB0 from mentor
sudo ../../from_mentor/grmon-pro-3.3.2/linux/bin64/grmon -baud 460800 -uart /dev/ttyUSB0 -nosram -nb -u -e "load leon-buildroot/output/images/image.ram; run"
