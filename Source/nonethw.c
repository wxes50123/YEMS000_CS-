/******************************************************************************
* Hardware Dependent Functions for RX72M ETHERC                               *
*                                                                             *
*  Copyright (c) 2020-2025, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Tessera TS-RX72M-COM having dual Ethernet ports     FY *
* 2020-12-02 Ported to AlphaProject AP-RX72M-0A without change             FY *
* 2024-07-10 Modified lan_def_int() replacing #ifdef POL with #ifndef POL  FY *
* 2025-06-27 Swapped highest and lowest in comment of LANC_IP              FY *
******************************************************************************/

#include "kernel.h"
#include "nonethw.h"

/* Interrupt Priority Level */

#define LANC_IP     5               /* 1(lowest) ~ 15(highest) */

/* External Function */

extern void lan_intr(void);

/* Interrupt Service Routine Creation Information */

static void lan_isr(VP_INT);
static const T_CISR cisr_lan = { TA_HLNG, NULL, INT_GRPAL1, (FP)lan_isr, 0 };

/* Bit of Group AL1 Interrupt Request Enable Register */

#define EN_EINT (0x00000010 << ETH_CH)  /* EDMAC interrupt request enable */

/*****************************************************************************
* LAN Interrupt Service Routine
*
******************************************************************************/

static void lan_isr(VP_INT exinf)
{
    lan_intr();
}

/*****************************************************************************
* Create LAN Interrupt Service Routine
*
******************************************************************************/

ER lan_def_int(void)
{
  #ifndef POL /* in polling mode, ISR is not needed */
    UINT psw;
    ER ercd;

    if (!(sfr_inl(ICU_GENAL1) & EN_EINT)) {
        ercd = acre_isr(&cisr_lan);
        if (ercd < 0)
            return ercd;
    }

    psw = vdis_psw();
    sfr_set(ICU_IPR113, LANC_IP);
    sfr_setl(ICU_GENAL1, EN_EINT);
    ercd = ena_int(INT_GRPAL1);
    vset_psw(psw);

  #endif
    return E_OK;
}

/* end */
