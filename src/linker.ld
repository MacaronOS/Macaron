/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(start)
 
/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */

	_kernel_start = .;
	_kernel_text_start = .;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	_kernel_text_end = .;
	_kernel_rodata_start = .;
 
	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	_kernel_rodata_end = .;
	_kernel_data_start = .;
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		data = .;
		*(.data)
	}

	_kernel_data_end = .;
	_kernel_bss_start = .;
 
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		bss = .;
		*(COMMON)
		*(.bss)
	}

	_kernel_bss_end = .;
	. = ALIGN(4K);
	_kernel_end = .;

	end = .;
 
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}