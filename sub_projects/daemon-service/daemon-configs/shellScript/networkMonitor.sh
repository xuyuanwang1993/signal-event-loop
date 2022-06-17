#!/bin/bash
echo "monitor network info"
echo ""
echo "#   ifconfig -a"
ifconfig -a
echo ""
echo "#   route -n"
route -n
echo ""
echo "#   ip addr show"
ip addr show
echo "#   dump net status"
network_list=`ls /sys/class/net`
for i in $network_list
do
    if [ "$i" == "lo" ];then
        continue
    fi
    echo ""
    echo "inteface:$i"
    echo ""
    echo "#   cat /sys/class/net/$i/carrier"
    cat /sys/class/net/$i/carrier 2>&1
    echo ""
    echo "#   cat /sys/class/net/$i/speed"
    cat /sys/class/net/$i/speed 2>&1
    echo ""
    echo "#   cat /sys/class/net/$i/address"
    cat /sys/class/net/$i/address
    echo ""
    echo "#   cat /sys/class/net/$i/operstate"
    cat /sys/class/net/$i/operstate
    echo ""
    echo "#   cat /sys/class/net/$i/link_mode"
    cat /sys/class/net/$i/link_mode
    echo ""
    if [ "$i" == "wlan0" ];then
        echo ""
        echo '#   iwlist wlan0 scanning | grep -E "ESSID|Signal level"'
        /sbin/iwlist wlan0 scanning | grep -E "ESSID|Signal level"
        echo ""
        echo "#   iwgetid"
        /sbin/iwgetid
    fi
done