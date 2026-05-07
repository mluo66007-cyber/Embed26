/*
******************************************************************************
**
** @file        : STM32N647X0HXQ_LRUN_RAMxspi1.ld
** @brief       : Clean LRUN linker script for STM32N647, internal SRAM code,
**                internal SRAM data, and external HyperRAM on XSPI1.
**
** Notes:
**   - ROM region here is internal SRAM used as the load/run area for code.
**   - RAM region is internal SRAM used for data, bss, heap and stack.
**   - EXTRAM region is external HyperRAM at 0x90000000.
**
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x800;

/* Memories definition */
MEMORY
{
  ROM    (xrw) : ORIGIN = 0x34000400, LENGTH = 511K
  RAM    (xrw) : ORIGIN = 0x34080000, LENGTH = 1536K
  EXTRAM (rw)  : ORIGIN = 0x90000000, LENGTH = 0x04000000
}

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);
_sstack = _estack - _Min_Stack_Size;

/* Sections */
SECTIONS
{
  /* Startup code */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >ROM

  /* Program code */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    *(.RamFunc)
    *(.RamFunc*)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
  } >ROM

  /* Constant data */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >ROM

  .ARM.extab :
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >ROM

  .ARM :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >ROM

  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >ROM

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >ROM

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >ROM

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)

    . = ALIGN(4);
    _edata = .;
  } >RAM AT> ROM

  /* Non-cacheable data */
  .noncacheable :
  {
    . = ALIGN(32);
    __snoncacheable = .;
    KEEP(*(.noncacheable))
    KEEP(*(.noncacheable*))
    . = ALIGN(32);
    __enoncacheable = .;
  } >RAM

  /* Secure Gateway stubs */
  .gnu.sgstubs :
  {
    _sNSCVeneer = .;
    . = ALIGN(4);
    *(.gnu.sgstubs*)
    . = ALIGN(4);
    _eNSCVeneer = .;
  } >ROM

  /* Uninitialized data */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;

    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* External HyperRAM section.
     Variables marked with section(".EXTRAM") / section(".extram")
     will be placed at 0x90000000. */
  .EXTRAM (NOLOAD) :
  {
    . = ALIGN(32);
    __EXTRAM_start__ = .;

    *(.EXTRAM)
    *(.EXTRAM*)
    *(.extram)
    *(.extram*)

    . = ALIGN(32);
    __EXTRAM_end__ = .;
  } >EXTRAM

  /* Heap and stack */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
