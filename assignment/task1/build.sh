# extract buildroot
tar xf ./leon-buildroot-2022.02-1.2.tar.bz2
rm -r -f ./leon-buildroot
mv ./leon-buildroot-2022.02-1.2 ./leon-buildroot
cd ./leon-buildroot
# config the build image, remember to install qt, ref:https://stackoverflow.com/questions/48147356/install-qt-on-ubuntu
make gaisler_leon_defconfig
make