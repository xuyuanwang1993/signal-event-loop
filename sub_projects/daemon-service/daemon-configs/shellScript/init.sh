#!/bin/bash
disciver_process_name=device-discover
daemon_process_name=daemon-service
machine_type=`uname -m`
work_dir=/var/daemon-configs/bin
ln -sf /var/daemon-configs/bin/${machine_type}/${disciver_process_name} /usr/bin/${disciver_process_name}
ln -sf /var/daemon-configs/bin/${machine_type}/${daemon_process_name} /usr/bin/${daemon_process_name}