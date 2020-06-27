# Hey Emacs, this is a -*- makefile -*-

# AVR-GCC Makefile
# Fubared from:
#   http://www.sax.de/~joerg/mfile/
#   https://www.nongnu.org/avr-libc/examples/demo/Makefile

MCU				= atmega8515
F_CPU			= 1000000
OPTIMIZE		= -Os

TARGET			= simple
OBJ				= simple.o

WIN_ISP_PORT	= com3
LINUX_ISP_PORT	= /dev/ttyUSB0
ISP_PROG		= usbasp

# -----------------------------------------------------------------------------
# Digite na linha de comando:
#
# make					compilar
#
# make clean			remover todos os arquivos gerados pela compilação
#
# make flash			gravar a flash do microcontrolador
#
# make erase			apagar a flash e a eeprom do microcontrolador
#
# make reset			resetar o microcontrolador
#
# make e2prom_load		gravar a eeprom do microcontrolador
#
# make e2prom_dump		mostrar o conteúdo da eeprom do microcontrolador
#
# -----------------------------------------------------------------------------

# -------------- NÃO MUDE NADA DAQUI PARA BAIXO --------------
# ---------- (a menos que saiba o que está fazendo) ----------

ifdef ComSpec
ISP_PORT	= $(WIN_ISP_PORT)
else
ISP_PORT	= $(LINUX_ISP_PORT)
endif

CDEFS		= -DF_CPU=$(F_CPU)UL
ADEFS		= -DF_CPU=$(F_CPU)
LIBS		=

CC			= @avr-gcc
OBJCOPY		= avr-objcopy
OBJDUMP		= avr-objdump
NM			= avr-nm
SIZE		= avr-size

CFLAGS		=  -g -Wall -Wextra $(OPTIMIZE) -mmcu=$(MCU) $(CDEFS)
CFLAGS		+= -funsigned-char
CFLAGS		+= -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS		+= -Wstrict-prototypes
ASFLAGS		=  -mmcu=$(MCU) -x assembler-with-cpp $(ADEFS)
ASFLAGS		+= -Wa,-adhlns=$(<:.S=.lst),-gstabs
LDFLAGS		=  -Wl,-Map,$(TARGET).map

# lembrete das variáveis automáticas...
# $@ is target
# $^ is all prerequisites
# $< is first prerequisite

all: sizebefore elf lst sym text eeprom sizeafter

# Everything depends on the ELF file
elf: $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

# Create extended listing file from ELF output file
lst: $(TARGET).lst
%.lst: %.elf
	@$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file
sym: $(TARGET).sym
%.sym: %.elf
	@$(NM) -n $< > $@

# Create .hex files for flash and eeprom programming

text: hex bin

hex: $(TARGET).hex
bin: $(TARGET).bin

%.hex: %.elf
	@$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.bin: %.elf
	@$(OBJCOPY) -j .text -j .data -O binary $< $@

eeprom: ehex ebin

ehex: $(TARGET)_eeprom.hex
ebin: $(TARGET)_eeprom.bin

%_eeprom.hex: %.elf
	@$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 --no-change-warnings \
		-O ihex $< $@ || exit 0

%_eeprom.bin: %.elf
	@$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 --no-change-warnings \
		-O binary $< $@ || exit 0

# Clean up
clean:
	@rm -rf *.o *.bak *~
	@rm -rf $(TARGET).lst $(TARGET).sym $(TARGET).hex $(TARGET)_eeprom.hex \
		$(TARGET).bin $(TARGET)_eeprom.bin $(TARGET).map $(TARGET).elf

# Size calculation
ELFSIZE = $(SIZE) -d -G $(TARGET).elf

sizebefore:
	@if [ -f $(TARGET).elf ]; then echo "Size before:"; $(ELFSIZE); fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo "Size after:"; $(ELFSIZE); fi

# Microcontroller access (programing, erasing, verifying, reseting, etc.)
verify: $(TARGET).hex
	@avrdude -q -q -c $(ISP_PROG) -P $(ISP_PORT) -p $(MCU) \
		-U flash:v:$(TARGET).hex:i && echo flash verify OK

flash: $(TARGET).hex
	@avrdude -q -q -c $(ISP_PROG) -P $(ISP_PORT) -p $(MCU) \
		-U flash:w:$(TARGET).hex:i && echo flash write OK

erase:
	@avrdude -q -q -c $(ISP_PROG) -P $(ISP_PORT) -p $(MCU) -e && echo erase OK

reset:
	@avrdude -q -q -c $(ISP_PROG) -P $(ISP_PORT) -p $(MCU) && echo $(MCU) RESET

e2prom_dump:
	@avrdude -q -q -c $(ISP_PROG) -P $(ISP_PORT) -p $(MCU) -U eeprom:r:-:r \
		| srec_cat - -Binary -o -HEX_Dump

e2prom_load: $(TARGET)_eeprom.hex
	@avrdude -q -q -c $(ISP_PROG) -P $(ISP_PORT) -p $(MCU) \
		-U eeprom:w:$(TARGET)_eeprom.hex:i && echo eeprom write OK
