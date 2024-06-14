cmd_drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o := /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -Wp,-MD,drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/.de_feat.o.d  -nostdinc -isystem /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/5.3.1/include -I./arch/arm/include -I./arch/arm/include/generated/uapi -I./arch/arm/include/generated  -I./include -I./arch/arm/include/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-sunxi/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2048 -fstack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types    -DKBUILD_BASENAME='"de_feat"'  -DKBUILD_MODNAME='"disp"' -c -o drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.c

source_drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o := drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.c

deps_drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o := \
    $(wildcard include/config/arch/sun8iw17.h) \
    $(wildcard include/config/arch/sun8iw21.h) \
    $(wildcard include/config/arch/sun8iw21.h) \
    $(wildcard include/config/arch/sun8iw7.h) \
    $(wildcard include/config/arch/sun50iw8.h) \
    $(wildcard include/config/arch/sun8iw15.h) \
    $(wildcard include/config/arch/sun50iw10.h) \
    $(wildcard include/config/arch/sun50i10.h) \
    $(wildcard include/config/arch/sun8iw19.h) \
    $(wildcard include/config/arch/sun50iw2.h) \
    $(wildcard include/config/arch/sun50iw1.h) \
    $(wildcard include/config/arch/sun8iw11.h) \
    $(wildcard include/config/arch/sun8iw12.h) \
    $(wildcard include/config/arch/sun8iw16.h) \
    $(wildcard include/config/arch/sun8iw6.h) \
  drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.h \
    $(wildcard include/config/fpga/v4/platform.h) \
    $(wildcard include/config/fpga/v7/platform.h) \
    $(wildcard include/config/disp2/sunxi/support/smbl.h) \
    $(wildcard include/config/independent/de.h) \

drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o: $(deps_drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o)

$(deps_drivers/video/fbdev/sunxi/disp2/disp/de/lowlevel_v2x/de_feat.o):
