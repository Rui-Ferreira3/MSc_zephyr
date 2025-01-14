add_custom_target(devicetree_target)

set_target_properties(devicetree_target PROPERTIES "DT_CHOSEN|zephyr,console" "/soc/uart@80002000")
set_target_properties(devicetree_target PROPERTIES "DT_CHOSEN|zephyr,shell-uart" "/soc/uart@80002000")
set_target_properties(devicetree_target PROPERTIES "DT_CHOSEN|zephyr,sram" "/memory@0")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/|compatible" "ChipsAlliance,Veerwolf-Nexys;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/|SIZE" "")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/cpus" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_REG|/cpus|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/cpus|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/cpus|SIZE" "")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/cpus/cpu@0" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/cpus/cpu@0|compatible" "ChipsAlliance,VeeR-EH1-v1.2;riscv;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/cpus/cpu@0|reg" "0;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/cpus/cpu@0|status" "okay")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/cpus/cpu@0|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/cpus/cpu@0|ADDR" "0x0;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/cpus/cpu@0|SIZE" "NONE;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/soc" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc|compatible" "ChipsAlliance,VeeRwolf-soc;simple-bus;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc|SIZE" "")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/soc/interrupt-controller@f00c0000" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|pic" "/soc/interrupt-controller@f00c0000")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/interrupt-controller@f00c0000|reg" "4027318272;8192;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/interrupt-controller@f00c0000|riscv,max-priority" "15")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/interrupt-controller@f00c0000|interrupt-controller" "True")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/interrupt-controller@f00c0000|compatible" "swerv,pic;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/interrupt-controller@f00c0000|reg-names" "reg;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/interrupt-controller@f00c0000|wakeup-source" "False")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/interrupt-controller@f00c0000|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/interrupt-controller@f00c0000|ADDR" "0xf00c0000;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/interrupt-controller@f00c0000|SIZE" "0x2000;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/soc/uart@80002000" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|uart0" "/soc/uart@80002000")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|reg" "2147491840;4096;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|clock-frequency" "50000000")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|current-speed" "115200")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|label" "uart0")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|hw-flow-control" "False")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|status" "okay")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|compatible" "ns16550;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|reg-names" "control;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|interrupts" "12;15;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/uart@80002000|wakeup-source" "False")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/uart@80002000|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/uart@80002000|ADDR" "0x80002000;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/uart@80002000|SIZE" "0x1000;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/soc/spi@80001040" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|spi0" "/soc/spi@80001040")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040|reg" "2147487808;64;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040|label" "SPI0")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040|status" "okay")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040|compatible" "opencores,spi-simple;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040|reg-names" "control;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040|wakeup-source" "False")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/spi@80001040|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/spi@80001040|ADDR" "0x80001040;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/spi@80001040|SIZE" "0x40;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/soc/spi@80001040/flash@0" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|flash0" "/soc/spi@80001040/flash@0")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|reg" "0;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|spi-max-frequency" "2000000")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|label" "FLASH0")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|compatible" "jedec,spi-nor;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|wakeup-source" "False")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|requires-ulbpr" "False")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|has-dpd" "False")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|jedec-id" "1;32;24;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/spi@80001040/flash@0|size" "16777216")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/spi@80001040/flash@0|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/spi@80001040/flash@0|ADDR" "0x0;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/spi@80001040/flash@0|SIZE" "NONE;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/soc/gpio@80001010" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|gpio_led0" "/soc/gpio@80001010")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|reg" "2147487760;16;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|label" "LED0")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|ngpios" "32")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|gpio-controller" "True")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|status" "okay")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|compatible" "veerwolf,gpio;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/soc/gpio@80001010|wakeup-source" "False")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/gpio@80001010|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/gpio@80001010|ADDR" "0x80001010;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/soc/gpio@80001010|SIZE" "0x10;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/aliases" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_REG|/aliases|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/aliases|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/aliases|SIZE" "")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/chosen" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_REG|/chosen|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/chosen|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/chosen|SIZE" "")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/memory@0" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|sram" "/memory@0")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/memory@0|reg" "0;8388608;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/memory@0|compatible" "mmio-sram;")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/memory@0|wakeup-source" "False")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/memory@0|NUM" "1")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/memory@0|ADDR" "0x0;")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/memory@0|SIZE" "0x800000;")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/leds" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_REG|/leds|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/leds|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/leds|SIZE" "")
set_target_properties(devicetree_target PROPERTIES "DT_NODE|/leds/led" TRUE)
set_target_properties(devicetree_target PROPERTIES "DT_NODELABEL|led_0" "/leds/led")
set_target_properties(devicetree_target PROPERTIES "DT_PROP|/leds/led|label" "Red LED")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/leds/led|NUM" "0")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/leds/led|ADDR" "")
set_target_properties(devicetree_target PROPERTIES "DT_REG|/leds/led|SIZE" "")
