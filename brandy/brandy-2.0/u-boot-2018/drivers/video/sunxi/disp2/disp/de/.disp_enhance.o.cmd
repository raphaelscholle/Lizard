cmd_drivers/video/sunxi/disp2/disp/de/disp_enhance.o := ./../tools/toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -Wp,-MD,drivers/video/sunxi/disp2/disp/de/.disp_enhance.o.d  -nostdinc -isystem /home/book/tina-v853-open/brandy/brandy-2.0/tools/toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/7.2.1/include -Iinclude    -I./arch/arm/include -include ./include/linux/kconfig.h -I./include/openssl -D__KERNEL__ -D__UBOOT__ -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -Werror -Wno-packed-bitfield-compat -fshort-wchar -Os -fno-stack-protector -fno-delete-null-pointer-checks -g -fstack-usage -Wno-format-nonliteral -Werror=date-time -D__ARM__ -Wa,-mimplicit-it=always -mthumb -mthumb-interwork -mabi=aapcs-linux -mword-relocations -fno-pic -mno-unaligned-access -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -msoft-float -pipe -march=armv7-a -D__LINUX_ARM_ARCH__=7 -I./arch/arm/mach-sunxi/include    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(disp_enhance)"  -D"KBUILD_MODNAME=KBUILD_STR(disp)" -c -o drivers/video/sunxi/disp2/disp/de/disp_enhance.o drivers/video/sunxi/disp2/disp/de/disp_enhance.c

source_drivers/video/sunxi/disp2/disp/de/disp_enhance.o := drivers/video/sunxi/disp2/disp/de/disp_enhance.c

deps_drivers/video/sunxi/disp2/disp/de/disp_enhance.o := \
    $(wildcard include/config/disp2/sunxi/support/enahnce.h) \

drivers/video/sunxi/disp2/disp/de/disp_enhance.o: $(deps_drivers/video/sunxi/disp2/disp/de/disp_enhance.o)

$(deps_drivers/video/sunxi/disp2/disp/de/disp_enhance.o):