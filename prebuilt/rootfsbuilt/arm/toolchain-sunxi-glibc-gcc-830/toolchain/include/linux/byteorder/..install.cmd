cmd_/home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/linux/byteorder/.install := bash scripts/headers_install.sh /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/linux/byteorder ./include/uapi/linux/byteorder big_endian.h little_endian.h; bash scripts/headers_install.sh /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/linux/byteorder ./include/linux/byteorder ; bash scripts/headers_install.sh /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/linux/byteorder ./include/generated/uapi/linux/byteorder ; for F in ; do echo "$(pound)include <asm-generic/$$F>" > /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/linux/byteorder/$$F; done; touch /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/linux/byteorder/.install