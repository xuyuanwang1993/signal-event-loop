#!/bin/bash
echo "monitor disk info"
echo ""
echo "#   df -h"
df -h
echo ""
echo "#   du -h -d 2 /userdata"
du -h -d 2 /userdata
echo ""
echo "#   mount"
mount