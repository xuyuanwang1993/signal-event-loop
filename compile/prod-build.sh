#!/bin/sh
if [ $# -lt 4 ];then
    echo "usage: $0 arch:[3288|3328|3399|3568]  platform:[linux|android] install_path_prefix output_path_prefix"
    return 1
fi
project_path=$(realpath $(dirname $(realpath $0))/../)
version=`cat ${project_path}/VERSION | tr -d "\n" | tr -d "\r"`

if [ -z "${version}" ];then
    version="1.0.0"
    echo "version is empty!"
fi
arch=$1
platform=$2
intstall_path_prefix=$(realpath $3)
output_path_prefix=$(realpath $4)
echo "project_path:${project_path}"
echo "intstall_path_prefix:${intstall_path_prefix}"
echo "version:${version}"
echo "output_path_prefix:${output_path_prefix}"

config_file_name="prod-${arch}_${platform}.conf"
intstall_path=${intstall_path_prefix}/${platform}-${arch}/${version}
rm -rvf ${intstall_path}
mkdir -p ${intstall_path}
mkdir -p ${output_path_prefix}
echo "##### using config_file_name=${config_file_name} intstall_path=${intstall_path} output_path_prefix=${output_path_prefix}#####"
#relocated install
export INSTALL_PREFIX=${intstall_path}
#relocated output
export OUTPUT_PREFIX=${output_path_prefix}
#clean 
export CLEAN_BUILD_CACHE=true

if [ ${platform} == 'android' ];then
${project_path}/compile/build-android.sh ${project_path}/compile/${config_file_name}
else
${project_path}/compile/build.sh ${project_path}/compile/${config_file_name}
fi
if [ ! $? -eq 0 ];then
    return 1
fi
rm -rvf ${intstall_path_prefix}/${platform}-${arch}/latest
ln -srf ${intstall_path} ${intstall_path_prefix}/${platform}-${arch}/latest
