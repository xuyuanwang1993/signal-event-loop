#!/bin/bash
echo "monitor hdmi status"
echo "# ls -lat /sys/class/drm/"
ls -lat /sys/class/drm/
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/kernel/debug/dri/0/summary"
cat /sys/kernel/debug/dri/0/summary
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/kernel/debug/dri/0/clients"
cat /sys/kernel/debug/dri/0/clients
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/kernel/debug/dri/0/clients"
cat /sys/kernel/debug/dri/0/HDMI-A-1/force 
echo -e "-----------------------------------\n\n\n"




echo "# edid-decode < /sys/class/drm/card0-HDMI-A-1/edid"
edid-decode < /sys/class/drm/card0-HDMI-A-1/edid
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/class/drm/card0-HDMI-A-1/enabled"
cat /sys/class/drm/card0-HDMI-A-1/enabled
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/class/drm/card0-HDMI-A-1/status"
cat /sys/class/drm/card0-HDMI-A-1/status
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/class/drm/card0-HDMI-A-1/mode"
cat /sys/class/drm/card0-HDMI-A-1/mode
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/class/drm/card0-HDMI-A-1/modes"
cat /sys/class/drm/card0-HDMI-A-1/modes
echo -e "-----------------------------------\n\n\n"

echo "# cat /sys/class/drm/card0-HDMI-A-1/content_protection"
cat /sys/class/drm/card0-HDMI-A-1/content_protection
echo -e "-----------------------------------\n\n\n"






echo "# cat /sys/kernel/debug/dw-hdmi/status"
cat /sys/kernel/debug/dw-hdmi/status
echo -e "-----------------------------------\n\n\n"

#echo "# cat /sys/kernel/debug/dw-hdmi/phy"
#cat /sys/kernel/debug/dw-hdmi/phy
#echo -e "-----------------------------------\n\n\n"

#echo "# cat /sys/kernel/debug/dw-hdmi/ctrl"
#cat /sys/kernel/debug/dw-hdmi/ctrl
#echo -e "-----------------------------------\n\n\n"
