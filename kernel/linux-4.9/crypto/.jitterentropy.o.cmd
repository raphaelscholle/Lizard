cmd_crypto/jitterentropy.o := /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -Wp,-MD,crypto/.jitterentropy.o.d  -nostdinc -isystem /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/../lib/gcc/arm-linux-gnueabi/5.3.1/include -I./arch/arm/include -I./arch/arm/include/generated/uapi -I./arch/arm/include/generated  -I./include -I./arch/arm/include/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-sunxi/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2048 -fstack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -O0    -DKBUILD_BASENAME='"jitterentropy"'  -DKBUILD_MODNAME='"jitterentropy_rng"' -c -o crypto/jitterentropy.o crypto/jitterentropy.c

source_crypto/jitterentropy.o := crypto/jitterentropy.c

deps_crypto/jitterentropy.o := \
    $(wildcard include/config/x86/tsc.h) \

crypto/jitterentropy.o: $(deps_crypto/jitterentropy.o)

$(deps_crypto/jitterentropy.o):
