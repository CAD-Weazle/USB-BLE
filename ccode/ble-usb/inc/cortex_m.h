//#define SRAM_BASE   0x20000000

#define SYSTEM_CONTROL_BASE 0x400FE000

#define ITM_TER0	0xE0000E00
#define ITM_TPR		0xE0000E40
#define ITM_TCR		0xE0000E80
#define ITM_LAR		0xE0000FB0
#define ITM_LAR_KEY	0xC5ACCE55

#define CPUID		0xE000ED00
/* Debug Control Block */
#define DCB_DHCSR	0xE000EDF0
#define DCB_DCRSR	0xE000EDF4
#define DCB_DCRDR	0xE000EDF8
#define DCB_DEMCR	0xE000EDFC

#define DCRSR_WnR	(1 << 16)

#define DWT_CTRL	0xE0001000
#define DWT_CYCCNT	0xE0001004
#define DWT_COMP0	0xE0001020
#define DWT_MASK0	0xE0001024
#define DWT_FUNCTION0	0xE0001028

#define BP_CTRL		0xE0002000
#define BP_REMAP	0xE0002004
#define BP_COMP0	0xE0002008
#define BP_COMP1	0xE000200C
#define BP_COMP2	0xE0002010
#define BP_COMP3	0xE0002014
#define BP_COMP4	0xE0002018
#define BP_COMP5	0xE000201C
#define BP_COMP6	0xE0002020
#define BP_COMP7	0xE0002024

#define FPU_CPACR	0xE000ED88
#define FPU_FPCCR	0xE000EF34
#define FPU_FPCAR	0xE000EF38
#define FPU_FPDSCR	0xE000EF3C

#define TPIU_SSPSR	0xE0040000
#define TPIU_CSPSR	0xE0040004
#define TPIU_ACPR	0xE0040010
#define TPIU_SPPR	0xE00400F0
#define TPIU_FFSR	0xE0040300
#define TPIU_FFCR	0xE0040304
#define TPIU_FSCR	0xE0040308

/* DCB_DHCSR bit and field definitions */
#define DBGKEY		(0xA05F << 16)
#define C_DEBUGEN	(1 << 0)
#define C_HALT		(1 << 1)
#define C_STEP		(1 << 2)
#define C_MASKINTS	(1 << 3)
#define S_REGRDY	(1 << 16)
#define S_HALT		(1 << 17)
#define S_SLEEP		(1 << 18)
#define S_LOCKUP	(1 << 19)
#define S_RETIRE_ST	(1 << 24)
#define S_RESET_ST	(1 << 25)

/* DCB_DEMCR bit and field definitions */
#define TRCENA			(1 << 24)
#define VC_HARDERR		(1 << 10)
#define VC_INTERR		(1 << 9)
#define VC_BUSERR		(1 << 8)
#define VC_STATERR		(1 << 7)
#define VC_CHKERR		(1 << 6)
#define VC_NOCPERR		(1 << 5)
#define VC_MMERR		(1 << 4)
#define VC_CORERESET	(1 << 0)

#define NVIC_ICTR		0xE000E004
#define NVIC_ISE0		0xE000E100
#define NVIC_ICSR		0xE000ED04
#define NVIC_AIRCR		0xE000ED0C
#define NVIC_SHCSR		0xE000ED24
#define NVIC_CFSR		0xE000ED28
#define NVIC_MMFSRb		0xE000ED28
#define NVIC_BFSRb		0xE000ED29
#define NVIC_USFSRh		0xE000ED2A
#define NVIC_HFSR		0xE000ED2C
#define NVIC_DFSR		0xE000ED30
#define NVIC_MMFAR		0xE000ED34
#define NVIC_BFAR		0xE000ED38

/* NVIC_AIRCR bits */
#define AIRCR_VECTKEY		(0x5FA << 16)
#define AIRCR_SYSRESETREQ	(1 << 2)
#define AIRCR_VECTCLRACTIVE	(1 << 1)
#define AIRCR_VECTRESET		(1 << 0)
/* NVIC_SHCSR bits */
#define SHCSR_BUSFAULTENA	(1 << 17)
/* NVIC_DFSR bits */
#define DFSR_HALTED			1
#define DFSR_BKPT			2
#define DFSR_DWTTRAP		4
#define DFSR_VCATCH			8

#define FPCR_CODE 0
#define FPCR_LITERAL 1
#define FPCR_REPLACE_REMAP  (0 << 30)
#define FPCR_REPLACE_BKPT_LOW  (1 << 30)
#define FPCR_REPLACE_BKPT_HIGH  (2 << 30)
#define FPCR_REPLACE_BKPT_BOTH  (3 << 30)
