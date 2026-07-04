### project name -- edit this
TARGET=sunweb-master

### sources -- edit this
CSRC += main.c
CSRC += pll.c 
CSRC += ports.c
CSRC += timer.c
CSRC += uart1.c
CSRC += uart4.c
CSRC += spi.c
CSRC += i2c.c
CSRC += adc.c
CSRC += dac.c
CSRC += cli.c
CSRC += ble.c
CSRC += printf.c
CSRC += printf4.c
CSRC += dump.c
CSRC += ec.c
CSRC += ntc.c
CSRC += ph.c
CSRC += ds3234.c
CSRC += 25aa040.c
CSRC += scd30.c
CSRC += pump.c
CSRC += bus-handler.c
CSRC += sunweb-master.c
CSRC += commands-master.c
CSRC += commands-hotbed.c
CSRC += commands-simsun.c
CSRC += monitor.c
CSRC += eve.c
CSRC += tft.c
CSRC += tft-monitor.c
CSRC += tft-simsun.c
CSRC += tft-hotbed.c

CSRC += ./cmath/powten.c ./cmath/atof.c ./cmath/dtoa.c 

# GCC compiler
CCPATH  = d:/gcc/bin/
CC      = $(CCPATH)arm-none-eabi-gcc
OBJCOPY = $(CCPATH)arm-none-eabi-objcopy
OBJDUMP = $(CCPATH)arm-none-eabi-objdump
SIZE    = $(CCPATH)arm-none-eabi-size

CFLAGS  = -mcpu=cortex-m4
CFLAGS += -std=gnu99 -g -O2 -Wall -T$(LDSCRIPT)
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
CFLAGS += -fsingle-precision-constant 
#CFLAGS += -mfloat-abi=soft
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -fno-builtin-malloc

### linker-loader script -- edit this
LDSCRIPT = STM32F407VG_FLASH.ld

# dependencies
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

### own includes -- edit this
CFLAGS += -Iinc -Iinc/core  -Iinc/peripherals -Iown_inc

AFLAGS = -Iinc

### add startup file to build -- edit this
ASRC += core/startup_stm32f4xx.S


# rules
.PHONY: lib proj

HEX = $(TARGET).hex
ELF = $(TARGET).elf
DIS = $(TARGET).dis
OBJS = $(CSRC:.c=.o) $(ASRC:.S=.o)

all: proj

#proj: $(ELF) $(DIS) $(HEX)
proj: $(ELF) $(HEX)

$(DIS): $(ELF)
	$(OBJDUMP) -d $(ELF) > $(DIS)
#	$(OBJDUMP) -D $(ELF) > $(DIS).all

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex --set-start=0 $(ELF) $(HEX)

$(ELF): $(OBJS) $(LDSCRIPT)
	$(CC) $(GENDEPFLAGS) $(CFLAGS) $(OBJS) -o $@ -lm -Wl,-Map=$(TARGET).map

%.o: %.c
	@echo $(CC) $<
	@$(CC) $(CFLAGS) $(GENDEPFLAGS) -c $< -o $@ 

%.o: %.S
	@echo $(CC) $<
	@$(CC) $(AFLAGS) $(GENDEPFLAGS) -c $< -o $@ 

clean:
	rm -f *.o
	rm -f cmath/*.o
	rm -f $(ELF) 
	rm -f $(DIS)
	rm -f $(TARGET).bin
	rm -f $(TARGET).map
	rm -f $(TARGET).hex
	rm -f .dep/*

list:
	@echo
	@echo C sources:
	@echo $(CSRC)
	@echo
	@echo ASM sources:
	@echo $(ASRC)
	@echo
	@echo objects:
	@echo $(OBJS)

include $(wildcard .dep/*)
