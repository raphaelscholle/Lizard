cmd_drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o := /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -Wp,-MD,drivers/mmc/host/.sunxi-mmc-sun50iw1p1-2.o.d  -nostdinc -isystem /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/5.3.1/include -I./arch/arm/include -I./arch/arm/include/generated/uapi -I./arch/arm/include/generated  -I./include -I./arch/arm/include/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-sunxi/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2048 -fstack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types    -DKBUILD_BASENAME='"sunxi_mmc_sun50iw1p1_2"'  -DKBUILD_MODNAME='"sunxi_mmc_host"' -c -o drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.c

source_drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o := drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.c

deps_drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o := \
    $(wildcard include/config/arch/sun50iw1p1.h) \

drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o: $(deps_drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o)

$(deps_drivers/mmc/host/sunxi-mmc-sun50iw1p1-2.o):
