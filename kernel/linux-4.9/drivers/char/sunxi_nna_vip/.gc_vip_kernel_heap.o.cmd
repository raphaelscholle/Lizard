cmd_drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o := /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -Wp,-MD,drivers/char/sunxi_nna_vip/.gc_vip_kernel_heap.o.d  -nostdinc -isystem /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/5.3.1/include -I./arch/arm/include -I./arch/arm/include/generated/uapi -I./arch/arm/include/generated  -I./include -I./arch/arm/include/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-sunxi/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2048 -fstack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -Werror -Wno-unused-function -Wno-pointer-to-int-cast -Wno-unused-variable -Wno-format -Idrivers/char/sunxi_nna_vip/ -Idrivers/char/sunxi_nna_vip/linux -Idrivers/char/sunxi_nna_vip/linux/allwinner -I./include/npu -Wno-unused-variable -Wno-unused-function -Wno-pointer-to-int-cast -DLINUX -DDRIVER -DDBG=0 -DUSE_LINUX_PLATFORM_DEVICE -DvpmdAUTO_CORRECT_CONFLICTS=1    -DKBUILD_BASENAME='"gc_vip_kernel_heap"'  -DKBUILD_MODNAME='"vipcore"' -c -o drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.c

source_drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o := drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.c

deps_drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o := \
  drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.h \
  include/npu/gc_vip_common.h \
  include/npu/vip_lite_common.h \
  drivers/char/sunxi_nna_vip/gc_vip_kernel_port.h \
  include/npu/gc_vip_kernel_share.h \
  drivers/char/sunxi_nna_vip/gc_vip_kernel.h \
  drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.h \
  drivers/char/sunxi_nna_vip/gc_vip_kernel_util.h \
  include/npu/vip_lite.h \
  include/npu/vip_lite_common.h \

drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o: $(deps_drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o)

$(deps_drivers/char/sunxi_nna_vip/gc_vip_kernel_heap.o):