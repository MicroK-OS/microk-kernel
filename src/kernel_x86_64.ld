/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

/* We want the symbol _start to be our entry point */
ENTRY(EfiMain)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
	text	PT_LOAD	FLAGS((1 << 0) | (1 << 2));	/* Execute + Read */
	rodata	PT_LOAD	FLAGS((1 << 2));		/* Read only */
	data	PT_LOAD	FLAGS((1 << 1) | (1 << 2));	/* Write + Read */
}

SECTIONS
{
	/* We wanna be placed in the topmost 2GiB of the address space, for optimisations */
	/* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
	/* that is the beginning of the region. */

	. = 0xffffffff80000000;

	syscall_start = .;

	.syscall : {
		KEEP(*(.syscall.entrypoint))
		*(.syscall.*)
	} :text

	syscall_end = .;

	/* Move to the next memory page for .text */
	. += 0x1000;

	.text : {
		*(.text .text.*)
	} :text

	/* Move to the next memory page for .rodata */
	. += 0x1000;

	.rodata : {
		*(.rodata .rodata.*)
	} :rodata

	/* Move to the next memory page for .data */
	. += 0x1000;

	.data : {
		*(.data .data.*)
	} :data

	.bss : {
		*(COMMON)
		*(.bss .bss.*)
	} :data


	/* Discard .note.* and .eh_frame since they may cause issues on some hosts. */
	/DISCARD/ : {
		*(.eh_frame .eh_frame.*)
		*(.note .note.*)
	}
}
