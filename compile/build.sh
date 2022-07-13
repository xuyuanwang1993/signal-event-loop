#!/bin/bash -e
shell_dir_name=`dirname $(realpath $0)`
config_file_name="${shell_dir_name}/.last.config"
project_dir_name=`basename $(realpath ${shell_dir_name}/..)`
if [ $# -lt 1 ];then
	echo "you can specify a config file"
else
	config_file_name=$1
fi
echo "you can override install prefix path with \"export INSTALL_PREFIX=\""
CMAKE_BUILD_TYPE=
COMPILER_PREFIX=
CMAKE_FIND_ROOT_PATH=
CMAKE_INSTALL_PREFIX=
CMAKE_SYSTEM_NAME=Linux
ARCH_PLATFORM=x86
if [ ! -f "$config_file_name" ];then
	echo "choose compile mode:1.debug 2.release"
	while true
	do
		read option
		case $option in
		1)
			CMAKE_BUILD_TYPE=Debug
			break
			;;
		2)
			CMAKE_BUILD_TYPE=Release
			break
			;;
		*)
			echo "choose compile mode:1.debug 2.release"
			;;
		esac
	done
	echo -e "enter [COMPILER_PREFIX]:"
	read COMPILER_PREFIX
	echo -e "enter [CMAKE_FIND_ROOT_PATH]:"
	read CMAKE_FIND_ROOT_PATH
	echo -e "enter [CMAKE_INSTALL_PREFIX -> default value is inttall to output_path/install]:"
	read CMAKE_INSTALL_PREFIX
	echo -e "enter [CMAKE_SYSTEM_NAME -> Linux Android or other]:"
	read CMAKE_SYSTEM_NAME
	echo -e "enter [ARCH_PLATFORM -> 3288 3568 3399 3566 or other]:"
	read ARCH_PLATFORM
	echo -e "enter [GCC_LIB_PATH]:"
	read GCC_LIB_PATH
else
	source $config_file_name
fi

if [ ! -z "${INSTALL_PREFIX}" ];then
	echo "override install prefix from ${CMAKE_INSTALL_PREFIX} to ${INSTALL_PREFIX}"
	CMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
fi
echo "config_file_name=${config_file_name}"
echo "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
echo "COMPILER_PREFIX=${COMPILER_PREFIX}"
echo "CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}"
echo "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
echo "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}"
echo "ARCH_PLATFORM=${ARCH_PLATFORM}"
echo "GCC_LIB_PATH=${GCC_LIB_PATH}"
rm -f ${shell_dir_name}/.last.config
echo "export CMAKE_BUILD_TYPE=\"${CMAKE_BUILD_TYPE}\"" >> ${shell_dir_name}/.last.config
echo "export COMPILER_PREFIX=\"${COMPILER_PREFIX}\"" >> ${shell_dir_name}/.last.config
echo "export CMAKE_FIND_ROOT_PATH=\"${CMAKE_FIND_ROOT_PATH}\"" >> ${shell_dir_name}/.last.config
echo "export CMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"" >> ${shell_dir_name}/.last.config
echo "export CMAKE_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\"" >> ${shell_dir_name}/.last.config
echo "export ARCH_PLATFORM=\"${ARCH_PLATFORM}\"" >> ${shell_dir_name}/.last.config
echo "export GCC_LIB_PATH=\"${GCC_LIB_PATH}\"" >> ${shell_dir_name}/.last.config
#

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${GCC_LIB_PATH}

output_dir_sufix=
if [ ${CMAKE_BUILD_TYPE} == 'Debug' ];then
	output_dir_sufix=d
fi
platform_dir=linux
if [ ${CMAKE_SYSTEM_NAME} == 'Android' ];then
	platform_dir=android
fi
arch_str=`basename "${COMPILER_PREFIX}"`
if [ ! -z `echo ${arch_str} | grep arm` ];then
	platform_dir=${ARCH_PLATFORM}-${platform_dir}-arm
elif [ ! -z `echo ${arch_str} | grep aarch64` ];then
	platform_dir=${ARCH_PLATFORM}-${platform_dir}-aarch64
fi
OUTPUT_PATH=`realpath ../${project_dir_name}-build`
OUTPUT_PATH=${OUTPUT_PATH}/${platform_dir}${output_dir_sufix}
mkdir -p ${OUTPUT_PATH}
if [ -z "${CMAKE_INSTALL_PREFIX}" ];then
	CMAKE_INSTALL_PREFIX=${OUTPUT_PATH}/install
fi
current_dir=`pwd`
cd ${OUTPUT_PATH}&&cmake -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -D CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} \
	-D CMAKE_CXX_COMPILER=${COMPILER_PREFIX}g++ -DCMAKE_C_COMPILER=${COMPILER_PREFIX}gcc \
	-D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
	-D CMAKE_TOOLCHAIN_FILE=${shell_dir_name}/cross_compile.cmake ${current_dir}
make -j4
