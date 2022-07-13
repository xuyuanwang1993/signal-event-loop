#!/bin/bash
echo "collect process list"
PS_CHECK=`ps | wc -l`
if [ ${PS_CHECK} -gt 10 ]; then
PS_COMMAND="ps"
else
PS_COMMAND="ps -aux"
fi
echo "#   ${PS_COMMAND} "
${PS_COMMAND}