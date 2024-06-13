
LICHEE_DIR=${LICHEE_TOP_DIR}
LICHEE_BUILD_CONFIG=${LICHEE_DIR}/.buildconfig

if [ -f "${LICHEE_BUILD_CONFIG}" ]; then
    source ${LICHEE_BUILD_CONFIG}
    export KDIR=${LICHEE_TOP_DIR}/out/kernel/build/
    if [ "$LICHEE_ARCH" == "arm64" ]; then
        export CROSS_COMPILE=${LICHEE_TOOLCHAIN_PATH}/bin/aarch64-linux-gnu-
    else
        export CROSS_COMPILE=${LICHEE_TOOLCHAIN_PATH}/bin/arm-linux-gnueabi-
    fi
else
    echo "need config and build kernel first!"
    exit 0
fi

SRC_PATH=${LICHEE_BR_OUT}/build/vs-modules

if [ "$LICHEE_ARCH" == "arm64" ]; then
	cp -f ${SRC_PATH}/Install/Core/Makefile_AARCH64.Inc ${SRC_PATH}/Install/Core/Makefile.Inc
	cp -f ${SRC_PATH}/Install/Core/Rule_AARCH64.make ${SRC_PATH}/Install/Core/Rule.make
	echo "configured kernel 64bit!"
else
	cp -f ${SRC_PATH}/Install/Core/Makefile_AARCH32.Inc ${SRC_PATH}/Install/Core/Makefile.Inc
	cp -f ${SRC_PATH}/Install/Core/Rule_AARCH32.make ${SRC_PATH}/Install/Core/Rule.make
	echo "configured kernel 32bit!"
fi

LIBSST_SRC_PATH=${LICHEE_BR_OUT}/build/libsst/src
mkdir -p ${SRC_PATH}/Install/Core/Inc/Vendor_SecStorage;
cp -fva ${LIBSST_SRC_PATH}/include/*.h ${SRC_PATH}/Install/Core/Inc/Vendor_SecStorage;

SRC_DIR=Utility
cp -rf ${SRC_PATH}/Util_modules.makefile ${SRC_PATH}/Utility
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/Util_modules.makefile
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/Util_modules.makefile clean


SRC_DIR=HAL_SX6
cp -rf ${SRC_PATH}/kernel_modules.makefile ${SRC_PATH}/HAL_SX6
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/kernel_modules.makefile
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/kernel_modules.makefile clean

cp -rf ${SRC_PATH}/Hal_modules.makefile ${SRC_PATH}/HAL_SX6
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/Hal_modules.makefile
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/Hal_modules.makefile clean

SRC_DIR=Tools
cp -rf ${SRC_PATH}/Tools_modules.makefile ${SRC_PATH}/Tools
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/Tools_modules.makefile
make -C ${SRC_PATH}/${SRC_DIR} -f ${SRC_PATH}/Tools_modules.makefile clean


unset KDIR
unset CROSS_COMPILE
