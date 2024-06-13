##!/bin/bash
#
# scripts/build.sh
# (c) Copyright 2013
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# James Deng <csjamesdeng@allwinnertech.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

set -e

if [ -f ../../.buildconfig ]; then
    source ../../.buildconfig
else
    echo "Please run ./build.sh config first"
    return
fi

LICHEE_BR_RAMFS_OUT=${LICHEE_PLAT_OUT}/ramfs

make_ramfs_menuconfig()
{
	local config=$1

    if [ ! -d ${LICHEE_BR_RAMFS_OUT} ]; then
        mkdir ${LICHEE_BR_RAMFS_OUT}
    fi

	if [ "x" != "x${config}" ]; then
		make O=${LICHEE_BR_RAMFS_OUT} ${config}
	fi

    if [ ! -f ${LICHEE_BR_RAMFS_OUT}/.config ] ; then
        printf "\nUsing default config ...${LICHEE_BR_RAMFS_CONF}\n\n"
        make O=${LICHEE_BR_RAMFS_OUT} ${LICHEE_BR_RAMFS_CONF}
    fi

    make O=${LICHEE_BR_RAMFS_OUT} menuconfig
}

ramfs_saveconfig()
{
    cp ${LICHEE_BR_RAMFS_OUT}/.config configs/${LICHEE_BR_RAMFS_CONF}
}

ramfs_savedefconfig()
{
    make O=${LICHEE_BR_RAMFS_OUT} savedefconfig
}

build_package()
{
	local package=$1

	if [ "x" = "x${package}" ]; then
        echo "package not found"
        return
    fi

	make O=${LICHEE_BR_OUT} ${package}-dirclean
	make O=${LICHEE_BR_OUT} ${package}-rebuild
}

build_ramfs()
{
    local config=$1

    if [ ! -d ${LICHEE_BR_RAMFS_OUT} ]; then
        mkdir ${LICHEE_BR_RAMFS_OUT}
    fi

    if [ "x" = "x${config}" ]; then
        if [ "x" != "x${LICHEE_BR_RAMFS_CONF}" ]; then
            config=${LICHEE_BR_RAMFS_CONF}
        else
            echo "ramfs config not found"
            return
        fi
    fi

    if [ ! -f ${LICHEE_BR_RAMFS_OUT}/.config ] ; then
        printf "\nUsing default config ...${config}\n\n"
        make O=${LICHEE_BR_RAMFS_OUT} ${config}
    fi

    make O=${LICHEE_BR_RAMFS_OUT}
}

build_buildroot()
{
    if [ ! -f ${LICHEE_BR_OUT}/.config ] ; then
        printf "\nUsing default config ...\n\n"
        echo ${LICHEE_BR_DEFCONF}
        make O=${LICHEE_BR_OUT} -C ${LICHEE_BR_DIR} ${LICHEE_BR_DEFCONF}
    fi

    make O=${LICHEE_BR_OUT} -C ${LICHEE_BR_DIR}
}

build_toolchain()
{
    local tooldir="${LICHEE_BR_OUT}/host/opt/ext-toolchain/"
    mkdir -p ${tooldir}
    if [ -f ${tooldir}/.installed ] ; then
        printf "external toolchain has been installed\n"
    else
        printf "installing external toolchain\n"
        printf "please wait for a few minutes ...\n"
        tar --strip-components=1 \
            -xJf ${LICHEE_BR_DIR}/dl/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi.tar.xz \
            -C ${tooldir}
        [ $? -eq 0 ] && touch ${tooldir}/.installed
    fi

    export PATH=${tooldir}/bin:${PATH}
}

EXTERNAL_DIR=${LICHEE_BR_DIR}/external-packages
DESTDIR=${LICHEE_BR_DIR}/images
STAGING_DIR=${LICHEE_BR_OUT}/staging
INCDIR=${STAGING_DIR}/usr/include
TARGET_DIR=${LICHEE_BR_OUT}/target
TARGET_SYSROOT_OPT="--sysroot=${STAGING_DIR}"

CROSS_COMPILE=arm-linux-gnueabi-

TARGET_AR=${CROSS_COMPILE}ar
TARGET_AS=${CROSS_COMPILE}as
TARGET_CC="${CROSS_COMPILE}gcc ${TARGET_SYSROOT_OPT}"
TARGET_CPP="${CROSS_COMPILE}cpp ${TARGET_SYSROOT_OPT}"
TARGET_CXX="${CROSS_COMPILE}g++ ${TARGET_SYSROOT_OPT}"
TARGET_FC="${CROSS_COMPILE}gfortran ${TARGET_SYSROOT_OPT}"
TARGET_LD="${CROSS_COMPILE}ld ${TARGET_SYSROOT_OPT}"
TARGET_NM="${CROSS_COMPILE}nm"
TARGET_RANLIB="${CROSS_COMPILE}ranlib"
TARGET_OBJCOPY="${CROSS_COMPILE}objcopy"
TARGET_OBJDUMP="${CROSS_COMPILE}objdump"
TARGET_CFLAGS="-pipe -Os  -mtune=cortex-a7 -march=armv7-a -mabi=aapcs-linux -msoft-float -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -I${INCDIR}"

BUILD_OPTIONS="DESTDIR=${DESTDIR} CROSS_COMPILE=${CROSS_COMPILE} \
    STAGING_DIR=${STAGING_DIR} TARGET_DIR=${TARGET_DIR} CC=\"${TARGET_CC}\" \
    AR=${TARGET_AR} AS=${TARGET_AS} CPP=\"${TARGET_CPP}\" CXX=\"${TARGET_CXX}\" \
    FC=\"${TARGET_FC}\" LD=\"${TARGET_LD}\" NM=${TARGET_NM} RANLIB=${TARGET_RANLIB} \
    OBJCOPY=${TARGET_OBJCOPY} OBJDUMP=${TARGET_OBJDUMP} CFLAGS=\"${TARGET_CFLAGS}\""

build_external()
{
    for dir in ${EXTERNAL_DIR}/* ; do
        if [ -f ${dir}/Makefile ]; then
            BUILD_COMMAND="make -C ${dir} ${BUILD_OPTIONS} all"
            eval $BUILD_COMMAND
            BUILD_COMMAND="make -C ${dir} ${BUILD_OPTIONS} install"
            eval $BUILD_COMMAND
        fi
    done
}

clean()
{
   if [ "x" != "x${LICHEE_BR_OUT}" ];then
       rm -rf ${LICHEE_BR_OUT}
   fi

   if [ "x" != "x${LICHEE_BR_RAMFS_OUT}" ];then
     rm -rf ${LICHEE_BR_RAMFS_OUT}
   fi
}

case "$1" in
    clean)
        clean
        ;;
    ramfs)
        build_ramfs $2
        ;;
	ramfs_menuconfig)
        make_ramfs_menuconfig $2
        ;;
	ramfs_saveconfig)
        ramfs_saveconfig
        ;;
	ramfs_savedefconfig)
        ramfs_savedefconfig
        ;;
	package)
		build_package $2
		;;
    *)
        if [ "x${LICHEE_PLATFORM}" = "xlinux" ] ; then
            build_buildroot
            export PATH=${LICHEE_BR_OUT}/host/opt/ext-toolchain/bin:$PATH
            build_external
        else
            build_toolchain
        fi
        ;;
esac
