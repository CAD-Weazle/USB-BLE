### project name -- edit this
TARGET = solar-counter

### sources -- edit this
CSRC += ./core/system_stm32l1xx.c
CSRC += ./cmath/powten.c ./cmath/atof.c ./cmath/dtoa.c 
CSRC += main.c 
CSRC += clock.c
CSRC += power.c
CSRC += ports.c
CSRC += ticker.c
CSRC += uart2.c
CSRC += i2c.c
CSRC += printf2.c
CSRC += dump.c
CSRC += cli.c
CSRC += dma.c
CSRC += adc.c
CSRC += ltc4150.c
CSRC += ltc2941.c

### add startup file to build -- edit this
ASRC += ./core/startup_stm32l151xc.S

### linker-loader script -- edit this
LDSCRIPT = STM32L151XC_FLASH.ld

# GCC compiler
CCPATH  = d:/gcc/bin/
CC      = $(CCPATH)arm-none-eabi-gcc
OBJCOPY = $(CCPATH)arm-none-eabi-objcopy
OBJDUMP = $(CCPATH)arm-none-eabi-objdump
SIZE    = $(CCPATH)arm-none-eabi-size

CFLAGS  = -mcpu=cortex-m3                   # -- edit his
CFLAGS += -DSTM32L151xC                     # -- edit his
CFLAGS += -std=gnu99 -g -O2 -Wall -T$(LDSCRIPT)
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
CFLAGS += -fsingle-precision-constant 
CFLAGS += -mfloat-abi=soft

### own flags -- edit this
CFLAGS += -Iinc -Iinc/core -Iown_inc

AFLAGS = -Iinc

# dependencies
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

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

$(ELF): $(OBJS) 
	$(CC) $(GENDEPFLAGS) $(CFLAGS) $(OBJS) -o $@ -lm -Wl,-Map=$(TARGET).map

%.o: %.c
	@echo $(CC) $<
	@$(CC) $(GENDEPFLAGS) $(CFLAGS) -c $< -o $@ 

%.o: %.S
	@echo $(CC) $<
	@$(CC) $(GENDEPFLAGS) $(AFLAGS) -c $< -o $@ 

clean:
	rm -f $(wildcard *.o)
	rm -f cmath/*.o
	rm -f $(ELF) 
	rm -f $(DIS)
	rm -f $(TARGET).bin
	rm -f $(TARGET).map
	rm -f $(TARGET).hex
	rm -f $(wildcard .dep/*)

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
