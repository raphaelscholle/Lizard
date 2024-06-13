PLATFORM_PATH := $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))
include ${PLATFORM_PATH}/allwinner/utils/mtop/mtop.mk

include ${PLATFORM_PATH}/allwinner/usb/mtp/mtp.mk
include ${PLATFORM_PATH}/allwinner/usb/adbd/adbd.mk
