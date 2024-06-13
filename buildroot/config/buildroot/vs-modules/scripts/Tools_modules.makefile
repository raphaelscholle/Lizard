PATH_TAG := Tools
TOP_DIR := $(shell pwd -P | sed "s@/$(PATH_TAG)*@@g")

SRC_PATH=${LICHEE_BR_OUT}/build/vs-modules
include ${SRC_PATH}/Build/RuleAll.mk
include ${SRC_PATH}/Install/Core/Rule.make
include ${SRC_PATH}/Install/Core/Makefile.Inc


INC_INSTALL_DIR := ${SRC_PATH}/Install

ifneq (,$(findstring -DTV303, $(DEF_PROJECT_RULE)))
SUBDIRS := DebugServer Regtest
endif

.PHONY: subdirs $(SUBDIRS)

all debug release: subdirs
#	install -d ${LICHEE_BR_OUT}/target/lib/vs/modules/
#	install ${SRC_PATH}/inc/*.h $(INC_INSTALL_DIR)/Core/Inc
#	find ${SRC_PATH}/ -name "*.ko" -exec cp {} ${LICHEE_BR_OUT}/target/lib/vs/modules/ \;
#	find ${SRC_PATH}/ -name "*.ko" -exec cp {} ${LICHEE_TOP_DIR}/platform/target/tv303/perf1/busybox-init-base-files/lib/vs/modules \;

subdirs:$(SUBDIRS)

$(SUBDIRS):
	make $(MAKECMDGOALS) -C $@ || exit 1;

clean: 
#	for dir in $(SUBDIRS); do ( make clean -C $$dir) ||exit 1; done
