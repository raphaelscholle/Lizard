cmd_/home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/misc/.install := bash scripts/headers_install.sh /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/misc ./include/uapi/misc cxl.h; bash scripts/headers_install.sh /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/misc ./include/misc ; bash scripts/headers_install.sh /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/misc ./include/generated/uapi/misc ; for F in ; do echo "$(pound)include <asm-generic/$$F>" > /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/misc/$$F; done; touch /home/xuqiang/workspace/tina-r528/out/v853-perf1/compile_dir/toolchain/linux-dev//include/misc/.install