cmd_arch/arm/boot/uImage := /bin/bash ./scripts/mkuboot.sh -A arm -O linux -C none  -T kernel -a 0x40008000 -e 0x40008000 -n 'Linux-4.9.191' -d arch/arm/boot/zImage arch/arm/boot/uImage
