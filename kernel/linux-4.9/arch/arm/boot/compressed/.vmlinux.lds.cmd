cmd_arch/arm/boot/compressed/vmlinux.lds := /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -E -Wp,-MD,arch/arm/boot/compressed/.vmlinux.lds.d  -nostdinc -isystem /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/5.3.1/include -I./arch/arm/include -I./arch/arm/include/generated/uapi -I./arch/arm/include/generated  -I./include -I./arch/arm/include/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-sunxi/include     -DTEXT_START="0" -DBSS_START="ALIGN(8)" -P -C -Uarm -D__ASSEMBLY__ -DLINKER_SCRIPT -o arch/arm/boot/compressed/vmlinux.lds arch/arm/boot/compressed/vmlinux.lds.S

source_arch/arm/boot/compressed/vmlinux.lds := arch/arm/boot/compressed/vmlinux.lds.S

deps_arch/arm/boot/compressed/vmlinux.lds := \
    $(wildcard include/config/cpu/endian/be8.h) \

arch/arm/boot/compressed/vmlinux.lds: $(deps_arch/arm/boot/compressed/vmlinux.lds)

$(deps_arch/arm/boot/compressed/vmlinux.lds):
