libsst_path := $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))
include ${libsst_path}/*/*.mk
