#!/bin/bash
#

ans=""
read -p "Please enter IP-Address (xxx.xxx.xxx.xxx): " ans
if [ -z $ans ]; then
	echo "Error! No ip-address entered."
	exit -1
fi
ip="$ans"

ans=""
read -p "Please TCP link (0-5): " ans
if [ -z $ans ]; then
	echo "Error! No TCP link entered."
	exit -1
else 
	valid=`expr match "$ans" '^[0-5]$'`
	if [ $valid -eq 0 ]; then
		echo "Error! Invalid TCP link"
		exit -1
	fi
fi
tcp="$ans"

ans=""
read -p "Please enter SPW link (0-2): " ans
if [ -z $ans ]; then
	echo "Error! No SPW link entered."
	exit -1
else 
	valid=`expr match "$ans" '^[0-2]$'`
	if [ $valid -eq 0 ]; then
		echo "Error! Invalid SPW link"
		exit -1
	fi
fi
spw="$ans"

# Setup host to target table
for (( node=0; node < 256 ; node++ )) {
	./set_route $ip $tcp $node $spw spw 0 1 0 $(($tcp + 16))
}

# Setup target to host
for (( node=0; node < 256 ; node++ )) {
	./set_route $ip $tcp $node $tcp tcp 0 1 0 $spw
}

./set_route $ip $tcp save
