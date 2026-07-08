//******************************************************************************
// @(r)   
//		BSPë„ë÷ópÉwÉbÉ_Å[
//		Å¶BSPÇégópÇµÇ»Ç¢ÇΩÇﬂÅAFITìÆçÏÇ…ïKóvÇ»íËã`ÇãLèq
// @(x)         ver 1.0 ( 2025/12/17 )
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
/**********************************************************************************/
/******Å°Å°Å°r_typedefs.hÇ©ÇÁÇÃà⁄êA******/
/**********************************************************************************/


/**********************************************************************************/
/******r_bsp_common.hÇ©ÇÁÇÃà⁄êA****************************************************/
/**********************************************************************************/
/* Version Number of r_bsp. */
#define R_BSP_VERSION_MAJOR           (5)
#define R_BSP_VERSION_MINOR           (52)

/**********************************************************************************/
/******r_rx_compiler.hÇ©ÇÁÇÃà⁄êA***************************************************/
/**********************************************************************************/

/* ========== Keywords ========== */
/* CC-RX' C++ mode does not support Pragma operator and variadic macros */
#define R_BSP_PRAGMA(x)
#define R_BSP_VOLATILE_EVENACCESS        volatile __evenaccess
#define R_BSP_EVENACCESS                 __evenaccess
#define R_BSP_EVENACCESS_SFR             __evenaccess
#define R_BSP_VOLATILE_SFR               volatile
#define R_BSP_SFR                        /* none */


/* ---------- Inline Expansion of Function ---------- */
#define R_BSP_PRAGMA_INLINE(function_name)           R_BSP_PRAGMA(inline function_name)\
                                                     extern
#define R_BSP_PRAGMA_STATIC_INLINE(function_name)    R_BSP_PRAGMA(inline function_name)\
                                                     static
                                                     
/**********************************************************************************/
/******r_rx_intrinsic_functions.hÇ©ÇÁÇÃà⁄êA****************************************/
/**********************************************************************************/
/* ---------- Special Instructions ---------- */
/* void brk(void) */
#define R_BSP_BRK()    brk()
/* void int_exception(signed long num) */
#define R_BSP_INT(x)   int_exception((signed long)(x))
/* void wait(void) */
#define R_BSP_WAIT()   wait()
/* void nop(void) */
#define R_BSP_NOP()    nop()

/**********************************************************************************/
/******r_bsp_cpu.hÇ©ÇÁÇÃà⁄êA*******************************************************/
/**********************************************************************************/
/* The different types of registers that can be protected. */
typedef enum
{
    /* PRC0
       Enables writing to the registers related to the clock generation circuit: SCKCR, SCKCR2, SCKCR3, PACKCR, PLLCR,
       PLLCR2, PPLLCR, PPLLCR2, BCKCR, MOSCCR, SOSCCR, LOCOCR, ILOCOCR, HOCOCR, HOCOCR2, OSTDCR, OSTDSR, CKOCR. */
    BSP_REG_PROTECT_CGC = 0,

    /* PRC1
       Enables writing to the registers related to operating modes, low power consumption, the clock generation circuit,
       and software reset: SYSCR0, SYSCR1, SBYCR, MSTPCRA, MSTPCRB, MSTPCRC, MSTPCRD, OPCCR, RSTCKCR,
       DPSBYCR, DPSIER0, DPSIER1, DPSIER2, DPSIER3, DPSIFR0, DPSIFR1, DPSIFR2, DPSIFR3,
       DPSIEGR0, DPSIEGR1, DPSIEGR2, DPSIEGR3, MOSCWTCR, SOSCWTCR, MOFCR, HOCOPCR, SWRR. */
    BSP_REG_PROTECT_LPC_CGC_SWR,

    /* PRC3
       Enables writing to the registers related to the LVD:LVCMPCR, LVDLVLR, LVD1CR0, LVD1CR1, LVD1SR, LVD2CR0,
       LVD2CR1, LVD2SR. */
    BSP_REG_PROTECT_LVD,

    /* MPC.PWPR
       Enables writing to MPC's PFS registers. */
    BSP_REG_PROTECT_MPC,

    /* This entry is used for getting the number of enum items. This must be the last entry. DO NOT REMOVE THIS ENTRY!*/
    BSP_REG_PROTECT_TOTAL_ITEMS
} bsp_reg_protect_t;

/* Exported global functions (to be accessed by other files) */
void     R_BSP_InterruptsDisable(void);
void     R_BSP_InterruptsEnable(void);
uint32_t R_BSP_CpuInterruptLevelRead(void);
_Bool    R_BSP_CpuInterruptLevelWrite(uint32_t level);
void     R_BSP_RegisterProtectEnable(bsp_reg_protect_t regs_to_protect);
void     R_BSP_RegisterProtectDisable(bsp_reg_protect_t regs_to_unprotect);
void     R_BSP_SoftwareReset(void);
void     bsp_register_protect_open(void); //r_bsp internal function. DO NOT CALL.
void     bsp_ram_initialize(void);
/**********************************************************************************/
/******mcu_interrupts.hÇ©ÇÁÇÃà⁄êA**************************************************/
/**********************************************************************************/
/* Type to be used for pdata argument in Control function. */
typedef union
{
    uint32_t ipl;                         /* Used at the following times.
                                             - When enabling an interrupt to set that interrupt's priority level 
                                               by BSP_INT_CMD_GROUP_INTERRUPT_ENABLE command.
                                             - When disabling an interrupt to save that interrupt's priority level 
                                               by BSP_INT_CMD_FIT_INTERRUPT_DISABLE command.
                                             - When enabling an interrupt to set that interrupt's priority level 
                                               by BSP_INT_CMD_FIT_INTERRUPT_ENABLE command. */
} bsp_int_ctrl_t;
/* End */
