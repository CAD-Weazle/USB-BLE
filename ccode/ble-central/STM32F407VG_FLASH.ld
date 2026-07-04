/*
 * Copyright 2014, Broadbean Corporation
 * All Rights Reserved.
 *
 */

/* #reset_handler = Reset_Handler ; */
__STACKSIZE__ = 4K;

ENTRY( Reset_Handler );

MEMORY
{
    FLASH      (rx)  : ORIGIN = 0x08000000, LENGTH = 512K - 128K
    FLASH_USER (rx)  : ORIGIN = 0x080e0000, LENGTH = 128K        /* user data storage */
    SRAM       (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    CCM_SRAM   (rwx) : ORIGIN = 0x10000000, LENGTH = 64K         /* Core Coupled SRAM. */
}

SECTIONS
{

    .vectors :
    {
        link_interrupt_vectors_location = .;
        KEEP(*(*.vectors))
    }>FLASH  AT> FLASH

    .ccm : /* core coupled memory, not suitable for DMA */
    {
        link_ccm_location = ALIGN(., 4);
        *(.bss.ccm)
        link_ccm_end = .;
        . = ALIGN(., 4);
    } > CCM_SRAM 

    .text :
    {
        . = ALIGN(4);

        KEEP(*(.text.irq ))
        *(.text .text.* .gnu.linkonce.t.*)
        _etext = .;
        _exit = .;

        . = ALIGN(0x4);

        link_const_variable_data_location = .;
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        link_const_variable_data_end = .;

        . = ALIGN(16);

    } >FLASH  AT> FLASH

    /*
     * The .ARM.exidx and .ARM.extab sections are used for C++ exception handling.
     * It is located here for completeness. Bare-metal ARM projects
     * typically cannot afford the overhead associated with C++
     * exceptions handling.
     */
    .ARM.exidx :
    {
        __exidx_start = ALIGN(4);
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH  AT> FLASH

    .ARM.extab :
    {
        __extab_start = ALIGN(4);
        *(.ARM.extab*)
        __extab_end = .;
    } > FLASH  AT> FLASH

    _sidata = .;

    .data : /* Contains the non-zero initialised global variables */
    {
        _sdata = .;
        *(.data*)
        _edata = .;
        . = ALIGN(., 4);
    }> SRAM AT> FLASH

    .bss : /* Zero initialised memory used for zero initialised variables */
    {   
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
        . = ALIGN(., 4);
    }> SRAM AT> SRAM

    /* user data stored in FLASH */
    .userflash :
    {
      . = ALIGN(4);
      KEEP(*(.USERFLASH))   /* keep my variable even if not referenced */
    } >FLASH_USER

    .stack : /* Contains the initial stack */
    {
        . = ALIGN(8);
        _sstack = .;
        . = . + 4K;
        _estack = .;
    }> SRAM AT> SRAM

    /DISCARD/ :
    {
        *(.ARM.attributes*)
        *(.comment)
        *(.init)
        *(.preinit)
        *(.fini)
        *(.fini_array)
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        *(.eh_frame_hdr)
        *(.eh_frame)
        *(.gnu.linkonce.armextab.*)
        *(.v4_bx)
        *(.vfp11_veneer)
        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)
        *(.glue*)
    }

}

/* Declare libc Heap to start at end of allocated RAM */

PROVIDE( _heap = _estack );

/* End of the heap is top of RAM, aligned 8 byte */

PROVIDE( _eheap = ALIGN( ORIGIN( SRAM ) + LENGTH( SRAM ) - 8, 8 ) );

/* ThreadX aliases */
/*
PROVIDE( __RAM_segment_used_end__ = link_stack_end );
PROVIDE( __tx_free_memory_start = link_stack_end );
PROVIDE( __tx_vectors = link_interrupt_vectors_location );
*/

