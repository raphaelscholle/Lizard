#!/bin/bash
# Martin <wuyan@allwinnertech.com>, 2021-07-20
# Deal with independent BSP repo stuff

BASE_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
BUILDCONFIG_FILE=${BASE_DIR}/../.buildconfig
if ! [ -f ${BUILDCONFIG_FILE} ]; then
	echo "File '${BUILDCONFIG_FILE}' does not exist"
	echo "Please run configuration first. eg: ./build.sh config"
	exit 1
fi
source ${BUILDCONFIG_FILE}

KER_DIR=${LICHEE_KERN_DIR}
BSP_DIR=${LICHEE_BSP_DIR}
SUPPORTED_KER=${BSP_DIR}/.supportedkernel
VERBOSE=""
FORCE=0
ERR=0

# Error code
ERR_UNSUPPORT=250

# Attention: Do not run this function! It's just for creating the bsp repo.
function bsp_repo_init()
{
	pushd ${KER_DIR} >/dev/null
	local target_dir=${BSP_DIR}
	mkdir -p ${target_dir}
	for file in `git log --author=**@allwinnertech.com --name-only --format='' . `;do
		cp -vf --parent $file ${target_dir}
	done
	popd >/dev/null
}

function setup_bsp()
{
	[ -n "$VERBOSE" ] && echo "${FUNCNAME[0]}() BEGIN"
	echo "Setup BSP files"
	pushd "${KER_DIR}" >/dev/null

	rm $VERBOSE -rf bsp
	ln $VERBOSE -sr ${BSP_DIR} bsp

	popd >/dev/null
	[ -n "$VERBOSE" ] && echo "${FUNCNAME[0]}() END"
}

function remove_bsp()
{
	[ -n "$VERBOSE" ] && echo "${FUNCNAME[0]}() BEGIN"
	echo "Remove BSP files"
	pushd ${KER_DIR} >/dev/null

	rm $VERBOSE -rf bsp

	popd >/dev/null
	[ -n "$VERBOSE" ] && echo "${FUNCNAME[0]}() END"
}

function merge_bsp()
{
	[ -n "$VERBOSE" ] && echo "${FUNCNAME[0]}() BEGIN"
	echo "Remove BSP files"
	pushd ${KER_DIR} >/dev/null

	rm $VERBOSE -rf bsp
	cp $VERBOSE -ar ${BSP_DIR} ./

	popd >/dev/null
	[ -n "$VERBOSE" ] && echo "${FUNCNAME[0]}() END"
}

# Check if independent BSP repo is supported
# return 0 if supported, otherwise ${ERR_UNSUPPORT}
function independent_bsp_supported()
{
	if [ ! -d "${BSP_DIR}" ]; then
		echo "BSP_DIR '${BSP_DIR}' does not exist!"
		return ${ERR_UNSUPPORT}
	fi

	if ! grep "^${LICHEE_KERN_VER}$" "${SUPPORTED_KER}" >/dev/null 2>&1; then
		echo "Unsupported kernel version '${LICHEE_KERN_VER}'"
		return ${ERR_UNSUPPORT}
	fi

	#echo "Independent BSP is supported"
	return 0
}

function show_usage()
{
	echo "Usage: $0 CMD [OPTIONS]"
	echo "CMD could be one of:"
	echo "  setup | remove"
	echo "  merge"
	echo "  supported"
	echo "OPTIONS could be:"
	echo "  -v      Verbose output"
	echo "  -f      Force execute"
}

# MAIN

independent_bsp_supported; ERR=$?
if [ $ERR -ne 0 ] || [ "$1" == "supported" ]; then
	exit $ERR
fi

if echo "$*" | grep -w "\-v" >/dev/null; then
	echo "VERBOSE ON"
	VERBOSE="--verbose"
fi
if echo "$*" | grep -w "\-f" >/dev/null; then
	[ -n "$VERBOSE" ] && echo "FORCE ON"
	FORCE=1
fi

[ -n "$VERBOSE" ] && echo "--- bsp.sh BEGIN"

if [ "$1" == "setup" ]; then
	setup_bsp
elif [ "$1" == "remove" ]; then
	remove_bsp
elif [ "$1" == "merge" ]; then
	merge_bsp
else
	echo "Syntax error!"
	show_usage
	exit 1
fi

[ -n "$VERBOSE" ] && echo "--- bsp.sh END"
exit $ERR