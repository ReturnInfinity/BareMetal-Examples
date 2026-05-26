OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS
{
	. = 0xFFFF800000000000;

	.text : {
		*(.text .text.*)
	}

	. = ALIGN(16);

	.rodata : {
		*(.rodata .rodata.*)
	}

	. = ALIGN(16);

	.init_array : {
		__init_array_start = .;
		*(.init_array*)
		__init_array_end = .;
	}

	. = ALIGN(16);

	.fini_array : {
		__fini_array_start = .;
		*(.fini_array*)
		__fini_array_end = .;
	}

	. = ALIGN(16);

	.data : {
		*(.data .data.*)
	}

	. = ALIGN(16);

	__bss_start = .;
	.bss : {
		*(.bss .bss.*)
	}
	__bss_stop = .;

	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
		*(.intrep)
	}
}
