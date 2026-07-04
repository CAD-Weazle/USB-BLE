### project name -- edit this
TARGET = ble-usb

### sources -- edit this
CSRC += main.c 
CSRC += pll.c 
CSRC += ports.c
CSRC += timer.c
CSRC += printf.c
CSRC += printf2.c
CSRC += uart1.c
CSRC += uart2.c
CSRC += cli.c
CSRC += dump.c
CSRC += ble.c

CSRC += cmath/atof.c

CSRC += ./core/system_stm32f4xx.c

# USB protocol stack 
CSRC += ./USB/usb_desc.c
CSRC += ./USB/usb_hw.c
CSRC += ./USB/usb_core.c
CSRC += ./USB/printu.c

### add startup file to build -- edit this
ASRC = ./core/startup_stm32f411xe.s

### linker-loader script -- edit this
LDSCRIPT = STM32F411CEUx_FLASH.ld

# GCC compiler
CCPATH  = d:/gcc/bin/
CC      = $(CCPATH)arm-none-eabi-gcc
OBJCOPY = $(CCPATH)arm-none-eabi-objcopy
OBJDUMP = $(CCPATH)arm-none-eabi-objdump
SIZE    = $(CCPATH)arm-none-eabi-size

-include Make.local

CFLAGS  = -DSTM32F411xE                     # -- edit his
CFLAGS += -mcpu=cortex-m4                   # -- edit his
CFLAGS += -std=gnu99 -g -O2 -Wall -T$(LDSCRIPT)
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
CFLAGS += -fsingle-precision-constant 
CFLAGS += -mfloat-abi=hard
CFLAGS += -mfpu=fpv4-sp-d16 -fshort-double 
CFLAGS += -fno-builtin-malloc 
CFLAGS += -fno-builtin-printf
CFLAGS += -specs=nano.specs 
CFLAGS += -specs=nosys.specs
CFLAGS += $(DEFINES)

### own flags -- edit this
CFLAGS += -Iinc -Iinc/core -Iown_inc

AFLAGS  = -Iinc -Iown_inc
AFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4
AFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard

GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

LDFLAGS = -Lplatform/MCU/STM32F4xx/GCC/

OBJS = $(CSRC:.c=.o) $(ASRC:.S=.o)

###################################################

VERSION = 1.0.1

IHEX = hextool/ihex.exe
MAIN = 0x08020000

.PHONY: lib proj tags

ELF = $(TARGET).elf
DIS = $(TARGET).dis
BIN = $(TARGET).bin
IMG = $(TARGET).img
HEX = $(TARGET).hex
IMGHEX = $(TARGET).imghex

all: proj

list:
	echo C sources
	@echo $(CSRC)
	echo
	echo ASM sources
	@echo $(ASRC)
	echo
	echo Objects
	@echo $(OBJS)

lib:
	$(MAKE) -C lib

#proj: $(ELF) $(DIS) $(BIN) $(IMG) $(HEX) - ao
proj: $(ELF) $(BIN) $(HEX)

prog: $(HEX)
	openocd -f oo.cfg -c "program $(HEX) reset exit"
	#stm -L prog $(IMG)  reset
	#modem term

ooprog: $(IMGHEX) $(IMG)
	openocd -f oo.cfg -c "program $(IMGHEX) reset exit"
	modem term

term:
	swd21 -t -a 0x08020200

info:
	swd21 -i -a 0x08020200

detach:
	usbreset 198c 5357

$(DIS): $(ELF)
	$(OBJDUMP) -d $(ELF) > $(DIS)

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex --set-start=0 $(ELF) $(HEX)

$(ELF): $(OBJS) $(LDSCRIPT)
	$(CC) $(LDFLAGS) $(GENDEPFLAGS) $(CFLAGS) $(OBJS) -o $@ -lm 

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(BIN) 

$(IMG): $(HEX) $(IHEX)
	$(IHEX) -i $(HEX) -o $(IMG) -b -n F411 -V $(VERSION)

$(IMGHEX): $(IMG)
	$(IHEX) -i $(HEX) -o $(IMGHEX) -n F411 -V $(VERSION)

%.o: %.c
	@echo $(CC) $<
	$(CC) $(CFLAGS) $(GENDEPFLAGS) -c $< -o $@ 

%.o: %.S
	@echo $(CC) $<
	$(CC) $(AFLAGS) -c $< -o $@ 

clean:
	rm -f $(wildcard *.o)
	rm -f $(wildcard net/*.o)
	rm -f $(wildcard peripherals/*.o)
	rm -f $(wildcard wwd/*.o)
	rm -f $(wildcard core/*.o)
	rm -f $(ELF) $(DIS)
	rm -f $(TARGET).bin
	rm -f $(TARGET).hex
	rm -f $(wildcard .dep/*)

-include $(wildcard .dep/*)

