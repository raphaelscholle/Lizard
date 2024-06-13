#!/bin/bash

function _load_config()
{
	local cfgkey=$1
	local cfgfile=$2
	local defval=$3
	local val=""

	[ -f "$cfgfile" ] && val="$(sed -n "/^\s*export\s\+$cfgkey\s*=/h;\${x;p}" $cfgfile | sed -e 's/^[^=]\+=//g' -e 's/^\s\+//g' -e 's/\s\+$//g')"
	eval echo "${val:-"$defval"}"
}

function croot()
{
	cd ${TINA_TOPDIR}
}

# same as croot
function cl()
{
	croot
}

# cd current kernel dir
function ckernel()
{
	local dkey="LICHEE_KERN_VER"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1

	cd ${TINA_TOPDIR}/kernel/$dval
}

# same as ck
function ck()
{
	ckernel
}

function ckernelout()
{
	# TODO different linux version
	cd ${TINA_TOPDIR}/out/kernel/build
}

# cd current dts dir
function cdts()
{
	local dkey1="LICHEE_KERN_VER"
	local dkey2="LICHEE_ARCH"
	local dkey3="LICHEE_CHIP_CONFIG_DIR"

	local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
	local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	local dval3=$(_load_config $dkey3 ${TINA_TOPDIR}/.buildconfig)

	[ -z "$dval1" ] && echo "ERROR: $dkey1 not set in .buildconfig" && return 1
	[ -z "$dval2" ] && echo "ERROR: $dkey2 not set in .buildconfig" && return 1

	local dval=${TINA_TOPDIR}/kernel/$dval1/arch/$dval2/boot/dts
	[ "$dval2" == "arm64" ] && dval=$dval/sunxi
	[ "$dval1" == "linux-5.10" ] && dval=${TINA_TOPDIR}/device/config/chips/r528/configs/default/linux-5.10
	cd $dval
}

# cd current product out dir
function cout()
{
	local dkey1="LICHEE_IC"
	local dkey2="LICHEE_BOARD"
	local dkey3="LICHEE_LINUX_DEV"
	local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
	local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	local dval3=$(_load_config $dkey3 ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval1" ] && echo "ERROR: $dkey1 not set in .buildconfig" && return 1
	[ -z "$dval2" ] && echo "ERROR: $dkey2 not set in .buildconfig" && return 1
	[ -z "$dval3" ] && echo "ERROR: $dkey3 not set in .buildconfig" && return 1

	cd ${TINA_TOPDIR}/out/${dval1}/${dval2}/${dval3}
}

# cd brandy dir
function cbrandy()
{
	local dkey="LICHEE_BRANDY_DIR"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	cd $dval
}

function carisc()
{
	cd ${TINA_TOPDIR}/brandy/arisc
}

function cboot()
{
	local dkey="LICHEE_BRANDY_DIR"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	cd $dval/u-boot-2018
}

function cboot0()
{
	local dkey="LICHEE_BRANDY_DIR"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	cd $dval/spl
}

function copensbi()
{
	local dkey="LICHEE_BRANDY_DIR"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	cd $dval/opensbi
}

# cd current product dir
function cchips()
{
	local dkey="LICHEE_IC"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1

	cd ${TINA_TOPDIR}/device/config/chips/${dval}
}

# cd current board config dir
function cconfigs()
{
	local dkey="LICHEE_BOARD_CONFIG_DIR"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	cd $dval
}

# same as cconfigs
function cbd()
{
	cconfigs
}

function cbin()
{
	local dkey1="LICHEE_IC"
	local dkey2="LICHEE_BOARD"
	local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
	local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval1" ] && echo "ERROR: $dkey1 not set in .buildconfig" && return 1
	[ -z "$dval2" ] && echo "ERROR: $dkey2 not set in .buildconfig" && return 1

	local bins=
	local bin_index1="${TINA_TOPDIR}/device/config/chips/${dval1}/bin"
	local bin_index2="${TINA_TOPDIR}/device/config/chips/${dval1}/configs/${dval2}/bin"
	[ -d ${bin_index1} ] && bins=(${bins[@]} ${bin_index1})
	[ -d ${bin_index2} ] && bins=(${bins[@]} ${bin_index2})
	[ 0 -eq ${#bins[@]} ] && echo "ERROR: Not found bin." && return -1

	# if only one
	[ 1 -eq ${#bins[@]} ] && cd ${bins[0]} && return 0

	local index=
	# if more, and $1 is not null
	# $1 must be number and not eq 0, or not to drop
	echo "$1" | grep [^0-9] >/dev/null || index=$1
	if [ -n "$index" ] ; then
		[ 0 -eq "$index" ] && index= || index=$(($index-1))
		local bin=${bins[${index}]}
		[ -d ${bin} ] && cd ${bin}
		return 0
	fi

	# choose one
	local i=1
	local choice=
	for choice in ${bins[@]}
	do
		echo " $i. $choice"
		i=$(($i+1))
	done
	echo -n "Which would you like? : "
	read index
	index=$(($index-1))

	local bin=${bins[${index}]}
	[ -d ${bin} ] && cd ${bin}
	return 0;
}

# cd buildroot dir
function cbr()
{
	local dkey="LICHEE_BR_DIR"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	cd $dval
}

# cd bsp dir
function cbsp()
{
	cd ${TINA_TOPDIR}/bsp
}

# cd bsp dir
function cbuild()
{
	cd ${TINA_TOPDIR}/build
}

######## FUNCTION #########

function cgrep()
{
    find . -name .repo -prune -o -name .git -prune -o -name out -prune -o -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 grep --color -n "$@"
}

function mgrep()
{
    find . -name .repo -prune -o -name .git -prune -o -path ./out -prune -o \( -regextype posix-egrep -iregex '(.*\/Makefile|.*\/Makefile\..*|.*\.make|.*\.mak|.*\.mk|.*\.bp)' -o -regextype posix-extended -regex '(.*/)?soong/[^/]*.go' \) -type f \
        -exec grep --color -n "$@" {} +
}


# print current .buildconfig
function printconfig()
{
	cat ${TINA_TOPDIR}/.buildconfig
}

# save kernel .config to xxx_defconfig
function saveconfig()
{
	${TINA_TOPDIR}/build.sh saveconfig $@
}

# run kernel menuconfig
function menuconfig()
{
	${TINA_TOPDIR}/build.sh menuconfig $@
}

# load kernel xxx_defconfig to .config
function loadconfig()
{
	${TINA_TOPDIR}/build.sh loadconfig $@
}

# Build brandy(uboot,boot0,fes) if you want.
function build_boot()
{
    local T=${TINA_TOPDIR}
    local chip=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_CHIP=\(.*\)$/\1/g'p)
    local cmd=$1
    local o_option=$2
    local platform
    local bin_dir

    local lichee_ic=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_IC=\(.*\)$/\1/g'p)
    local lichee_board=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_BOARD=\(.*\)$/\1/g'p)
    local lichee_brandy_def=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_BRANDY_DEFCONF=\(.*\)$/\1/g'p)
    local lichee_brandy_ver=$(cat ${TINA_TOPDIR}/.buildconfig |	sed -n 's/^.*LICHEE_BRANDY_VER=\(.*\)$/\1/g'p)
    local lichee_brandy_spl=$(cat ${TINA_TOPDIR}/.buildconfig |	sed -n 's/^.*LICHEE_BRANDY_SPL=\(.*\)$/\1/g'p)

    #local f="$T/device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_PLAN}/sys_config.fex"
    #local B="$( awk -F"=" '/^storage_type/{print $2}' $f | sed 's/^[ \t]*//g' )"
    #if [ x"$B" = x"3" ]; then
    #    export LICHEE_FLASH="nor"
    #else
    #    export LICHEE_FLASH="default"
    #fi

    local TARGET_PLATFORM=${lichee_ic}
    local TARGET_BOARD=${lichee_ic}-${lichee_board}

    echo $TARGET_PLATFORM $TARGET_BOARD
    if [ -z "$TARGET_BOARD" -o -z "$TARGET_PLATFORM" ]; then
        echo "Please use lunch to select a target board before build boot."
        return 1
    fi

    if [ "x$chip" = "x" ]; then
        echo "platform($TARGET_PLATFORM) not support"
        return 1
    fi

    platform=${chip}

    bin_dir="device/config/chips/${TARGET_PLATFORM}/bin"
    #if [ "${TARGET_UBOOT}" = "u-boot-2018" ]; then
    if [ x"${lichee_brandy_ver}" = x"2.0" ]; then
        \cd $T/brandy/brandy-2.0/
        [ x"$o_option" = x"uboot" -a -f "u-boot-2018/configs/${chip}_tina_defconfig" ] && {
            platform=${chip}_tina
        }
        [ x"$o_option" = x"uboot" -a -f "u-boot-2018/configs/${TARGET_BOARD}_defconfig" ] && {
            platform=${TARGET_BOARD}
            bin_dir="device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_BOARD#*-}/bin"
            mkdir -p $T/${bin_dir}
        }
        [ x"$o_option" = x"uboot" -a -f "$T/device/config/chips/${TARGET_PLATFORM}/configs/${lichee_board}/BoardConfig.mk" ] && {
            if [ x"${lichee_brandy_def}" != "x" ]; then
                platform=${lichee_brandy_def%%_def*}
            fi
        }
        if [ x"$o_option" == "xboot0" ]; then
            if [ x"${lichee_brandy_spl}" != x ]; then
                o_option=${lichee_brandy_spl}
            else
                o_option=spl
            fi

            if [ ! -f "spl-pub/Makefile" ]; then
                o_option=spl
            fi

            if [ ! -f "spl/Makefile" ]; then
                o_option=spl-pub
            fi
        fi
    else
        \cd $T/brandy/brandy-1.0/brandy
    fi

	echo "build_boot platform:$platform o_option:$o_option"
    #if [ x"$o_option" != "x" ] && [ "${TARGET_UBOOT}" = "u-boot-2018" ]; then
    if [ x"$o_option" != "x" ] && [ x"${lichee_brandy_ver}" = x"2.0" ]; then
        TARGET_BIN_DIR=${bin_dir} ./build.sh -p $platform -o $o_option -b ${TARGET_PLATFORM}
    elif [ x"$o_option" != "x" ]; then
        TARGET_BIN_DIR=${bin_dir} ./build.sh -p $platform -o $o_option
    else
        TARGET_BIN_DIR=${bin_dir} ./build.sh -p $platform
    fi
    if [ $? -ne 0 ]; then
        echo "$cmd stop for build error in brandy, Please check!"
        \cd - 1>/dev/null
        return 1
    fi
    \cd - 1>/dev/null
    echo "$cmd success!"
    return 0
}

function mboot0
{
    (build_boot mboot0 boot0)
}

function muboot
{
    (build_boot muboot uboot)
}

function mboot
{
    (build_boot muboot uboot)
    (build_boot mboot0 boot0)
}

function mkernel
{
    ${TINA_TOPDIR}/build.sh kernel
}

function pack()
{
    check_tina_topdir || return -1
    [ ! -e ${TINA_TOPDIR}/.buildconfig ] && \
	echo  "Not found .buildconfig,  Please lunch." &&  \
	return -1;

    local chip=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_CHIP=\(.*\)$/\1/g'p)
    local platform=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_LINUX_DEV=\(.*\)$/\1/g'p)
    local lichee_ic=$(cat ${TINA_TOPDIR}/.buildconfig | \
	    sed -n 's/^.*LICHEE_IC=\(.*\)$/\1/g'p)

    local lichee_board=$(cat ${TINA_TOPDIR}/.buildconfig | \
	    sed -n 's/^.*LICHEE_BOARD=\(.*\)$/\1/g'p)

    local lichee_kernel_ver=$(cat ${TINA_TOPDIR}/.buildconfig | \
	    sed -n 's/^.*LICHEE_KERN_VER=\(.*\)$/\1/g'p)

    local flash=$(cat ${TINA_TOPDIR}/.buildconfig | sed -n 's/^.*LICHEE_FLASH=\(.*\)$/\1/g'p)

    local board=${lichee_ic}-${lichee_board}
    local debug=uart0
    local sigmode=none
    local verity=
    local mode=normal
    local programmer=none
    local tar_image=none
    unset OPTIND

    while getopts "dsvmwih" arg
    do
        case $arg in
            d)
                debug=card0
                ;;
            s)
                sigmode=secure
                ;;
            v)
                verity="--verity"
                ;;
            m)
                mode=dump
                ;;
            w)
                programmer=programmer
                ;;
            i)
                tar_image=tar_image
                ;;
            h)
                pack_usage
                return 0
                ;;
            ?)
            return 1
            ;;
        esac
    done

	echo "${TINA_TOPDIR}/build/pack -c $chip -i ${board%%-*} -p ${platform} -b ${board##*-} -k $lichee_kernel_ver -d $debug -v $sigmode -m $mode -w $programmer -n ${flash} ${verity}"
	cd ${TINA_TOPDIR}/build/
	./pack -c $chip -i ${board%%-*} -p ${platform} -b ${board##*-} -k $lichee_kernel_ver -d $debug -v $sigmode -m $mode -w $programmer -n ${flash} ${verity}
	cd ${TINA_TOPDIR}
}

### MAIN ###

# check top of SDK
if [ ! -f "${TINA_TOPDIR}/build/.tinatopdir" ]; then
	echo "ERROR: Not found .tinatopdir"
	return -1;
fi
