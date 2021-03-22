BUILD_DIR=./build
ENTRY_POINT=0xc0001500
AS=nasm
CC=gcc
LD=ld
LIB=-I kernel/ -I kernel/lib/ -I kernel/drivers/
ASFLAG=-f elf
CFLAG=$(LIB) -c -fno-builtin -fno-stack-protector -Wall -m32
LDFLAG= -Ttext $(ENTRY_POINT) -e main -m elf_i386
OBJS=$(BUILD_DIR)/main.o $(BUILD_DIR)/print.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/debug.o \
	 $(BUILD_DIR)/interrupt_entry.o $(BUILD_DIR)/print_char.o $(BUILD_DIR)/string.o $(BUILD_DIR)/bitmap.o \
	 $(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/timer.o $(BUILD_DIR)/list.o $(BUILD_DIR)/init.o \
	 $(BUILD_DIR)/switch_to.o $(BUILD_DIR)/sync.o $(BUILD_DIR)/console.o $(BUILD_DIR)/keyboard.o \
	 $(BUILD_DIR)/ioqueue.o
COMMON_C_BUILD=$(CC) $(CFLAG) $< -o $@
############## C programs  ################
$(BUILD_DIR)/main.o: kernel/main.c kernel/lib/print.h kernel/thread.h kernel/init.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/print.o: kernel/lib/print.c kernel/lib/print.h kernel/io.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/lib/print.h kernel/lib/stdint.h kernel/io.h kernel/interrupt.h kernel/global.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h kernel/lib/print.h kernel/interrupt.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/string.o: kernel/lib/string.c kernel/debug.h kernel/lib/stdint.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/bitmap.o: kernel/lib/bitmap.c kernel/lib/bitmap.h kernel/lib/print.h kernel/lib/stdint.h kernel/lib/string.h kernel/debug.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/memory.o: kernel/memory.c kernel/memory.h kernel/lib/stdint.h kernel/lib/print.h kernel/global.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/thread.o: kernel/thread.c kernel/thread.h kernel/memory.h kernel/lib/string.h kernel/lib/stdint.h kernel/lib/list.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/list.o: kernel/lib/list.c kernel/lib/list.h kernel/interrupt.h kernel/lib/stdint.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/timer.o: kernel/drivers/timer.c kernel/io.h kernel/lib/stdint.h kernel/lib/print.h kernel/thread.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/init.o: kernel/init.c kernel/init.h kernel/interrupt.h kernel/memory.h kernel/thread.h kernel/lib/print.h kernel/drivers/keyboard.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/sync.o: kernel/sync.c kernel/lib/list.h kernel/thread.h kernel/debug.h kernel/interrupt.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/console.o: kernel/lib/console.c kernel/lib/console.h kernel/lib/stdint.h kernel/lib/print.h kernel/sync.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/keyboard.o: kernel/drivers/keyboard.c kernel/lib/stdint.h kernel/io.h kernel/lib/print.h kernel/interrupt.h
	$(COMMON_C_BUILD)

$(BUILD_DIR)/ioqueue.o: kernel/lib/ioqueue.c kernel/lib/ioqueue.h kernel/lib/stdint.h kernel/thread.h kernel/sync.h 
	$(COMMON_C_BUILD)
############# Assembly programs ###############
$(BUILD_DIR)/interrupt_entry.o: kernel/interrupt_entry.S
	$(AS) $(ASFLAG) $< -o $@

$(BUILD_DIR)/print_char.o: kernel/lib/print_char.S
	$(AS) $(ASFLAG) $< -o $@

$(BUILD_DIR)/switch_to.o: kernel/switch_to.S 
	$(AS) $(ASFLAG) $< -o $@
############## linking all object files ###############
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAG) $^ -o $@




.PHONY:mk_dir hd clean all

# mk_dir: 
# 	if[[!-d $(BUILD_DIR)]];then mkdir $(BUILD_DIR);fi 

hd: 
	dd if=$(BUILD_DIR)/kernel.bin of=bochs_hardware/hd60M.img bs=512 count=200 seek=20 conv=notrunc

clean:

build:$(BUILD_DIR)/kernel.bin

all:build hd



