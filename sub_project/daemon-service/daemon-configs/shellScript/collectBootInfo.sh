#!/bin/bash
echo "collect boot info"
echo '#   dmesg |grep -E " 0\.[0-9]{6}'
dmesg |grep -E " 0\.[0-9]{6}"
echo ""
echo '#   dmesg |grep -E " [1-9]{1}[0-9]{0,1}\.[0-9]{6}| 1[0-5]{1}[0-9]{1}\.[0-9]{6}"'
dmesg |grep -E " [1-9]{1}[0-9]{0,1}\.[0-9]{6}| 1[0-5]{1}[0-9]{1}\.[0-9]{6}"
echo ""
echo "#   date"
date
echo ""
echo "#   hwclock"
hwclock
echo ""
echo "#   ip addr show"
ip addr show
echo ""
echo "#   cat /proc/cmdline"
cat /proc/cmdline
echo ""
echo "#   uname -a"
uname -a
echo ""
echo "#   cat /etc/issue"
cat /etc/issue
echo ""
echo "#   ls -lat /etc/init.d/"
ls -lat /etc/init.d/
echo ""
echo "#   lsof"
lsof
echo ""
echo "#   ls /sys/class/net/ -lat"
ls /sys/class/net/ -lat
echo ""
echo "#   fdisk -l"
fdisk -l
echo ""
echo "#   collect mem frep 3568->1560 : memfreq"
memfreq
echo ""
echo "#   collect gpu frep 3568->800 : gpufreq"
gpufreq
echo ""
echo "#   collect cpu frep 3568->1992 : cpufreq"
cpufreq