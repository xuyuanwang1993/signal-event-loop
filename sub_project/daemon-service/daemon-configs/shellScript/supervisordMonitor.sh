#!/bin/bash
process_name=supervisord
if test -z ${process_name} ; then
    exit 1
fi
echo "Monitor process status"
PS_CHECK=`ps | wc -l`
if [ ${PS_CHECK} -gt 10 ]; then
PS_COMMAND="ps"
else
PS_COMMAND="ps -aux"
fi
pid_list=`${PS_COMMAND} | grep ${process_name} | grep -v grep | awk '{print $1}' | xargs`
if test -z "${pid_list}" ; then
    echo "${process_name} is not running"
    exit 1
fi
echo "------${process_name}------"
for pid in ${pid_list} 
do
    check_info=`cat /proc/${pid}/stat | grep "(${process_name})"`
    if test -z "${check_info}" ; then
        continue
    fi
    echo "pid:${pid}"
    echo "#  cat /proc/${pid}/status"
    cat /proc/${pid}/status
    echo ""
    echo "#  cat /proc/${pid}/stat"
    cat /proc/${pid}/stat
    echo ""
    echo "#  md5sum /proc/${pid}/exe"
    md5sum /proc/${pid}/exe
    echo ""
    echo "#  cat /proc/${pid}/cmdline"
    cat /proc/${pid}/cmdline
    echo ""
    echo "#  ls /proc/${pid}/fdinfo/ | wc -l"
    ls /proc/${pid}/fdinfo/ | wc -l
    echo ""
    echo "#  lsof | grep ${process_name}"
    lsof | grep ${process_name}
    echo ""
    echo "#  supervisord ctl status"
    supervisord ctl status
    echo -e "\n\n"
done


