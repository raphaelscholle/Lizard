cmd_arch/arm/boot/dts/board.dtb := mkdir -p arch/arm/boot/dts/ ; /home/book/tina-v853-open/out/toolchain/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.board.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.board.dtb.dts.tmp arch/arm/boot/dts/board.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/board.dtb -b 0 -i arch/arm/boot/dts/ -Wno-unit_address_vs_reg -d arch/arm/boot/dts/.board.dtb.d.dtc.tmp arch/arm/boot/dts/.board.dtb.dts.tmp ; cat arch/arm/boot/dts/.board.dtb.d.pre.tmp arch/arm/boot/dts/.board.dtb.d.dtc.tmp > arch/arm/boot/dts/.board.dtb.d

source_arch/arm/boot/dts/board.dtb := arch/arm/boot/dts/board.dts

deps_arch/arm/boot/dts/board.dtb := \
  arch/arm/boot/dts/sun8iw21p1.dtsi \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/sun8iw21p1-clk.dtsi \
  arch/arm/boot/dts/sun8iw21p1-pinctrl.dtsi \
  arch/arm/boot/dts/include/dt-bindings/thermal/thermal.h \
  arch/arm/boot/dts/include/dt-bindings/power/v853-power.h \

arch/arm/boot/dts/board.dtb: $(deps_arch/arm/boot/dts/board.dtb)

$(deps_arch/arm/boot/dts/board.dtb):
