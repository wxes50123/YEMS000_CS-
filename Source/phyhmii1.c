/******************************************************************************
* PHY Control Functions for Renesas SuperH and RX ETHERC 2nd Channel          *
*                                                                             *
*  Copyright (c) 2018-2025, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2025-03-17 Copied phyhmii.c, and replaced "nonethw.h" with "nonethw1.h"  FY *
******************************************************************************/

#include "kernel.h"
#include "nonethw1.h"

/* PHY Address */

#ifndef PHYAD
#define PHYAD       0x00                /* PHY address (0x00~0x1F) */
#endif

/* Local Variables */

static UH PHY_mode; /* speed & duplex when fixed / capabilities when auto-neg */
static UB PHY_auto; /* 0: fixed mode, !0: auto-negotiation mode */

/*****************************************************************************
* Write 1 bit of MII Register on MDIO
*
******************************************************************************/

static void mdio_wr_bit(int c)
{
    UW d;

    d = PIR_MMD | ((c) ? PIR_MDO : 0);
    sfr_outl(ETHERC_PIR, d);            /* MMD = 1, MDC = 0, MDO = 1/0 */
    dly_1us(1);                         /* wait for MDIO set-up (>= 10nsec) */
    sfr_outl(ETHERC_PIR, d | PIR_MDC);  /* MMD = 1, MDC = 1, MDO = 1/0 */
    dly_1us(1);                         /* wait for MDC high (>= 200nsec) */
    sfr_outl(ETHERC_PIR, d);            /* MMD = 1, MDC = 0, MDO = 1/0 */
    dly_1us(1);                         /* wait for MDC low (>= 200nsec) */
}

/*****************************************************************************
* Read 1 Bit of MII Register on MDIO
*
******************************************************************************/

static BOOL mdio_rd_bit(void)
{
    UW d;

    sfr_outl(ETHERC_PIR, PIR_MDC);      /* MMD = 0, MDC = 1 */
    dly_1us(1);                         /* wait for MDC high */
    d = sfr_inl(ETHERC_PIR);            /* MDI = 1/0 ? */
    sfr_outl(ETHERC_PIR, 0);            /* MMD = 0, MDC = 0 */
    dly_1us(1);                         /* wait for MDC low */
    return (d & PIR_MDI);
}

/*****************************************************************************
* Put MDIO High-Z for Turn Around
*
******************************************************************************/

static void mdio_hi_z(void)
{
    sfr_outl(ETHERC_PIR, 0);            /* MMD = 0, MDC = 0 */
    dly_1us(1);                         /* wait for MDIO set-up */
    sfr_outl(ETHERC_PIR, PIR_MDC);      /* MMD = 0, MDC = 1 */
    dly_1us(1);                         /* wait for MDC high */
    sfr_outl(ETHERC_PIR, 0);            /* MMD = 0, MDC = 0 */
    dly_1us(1);                         /* wait for MDC low */
}

/*****************************************************************************
* Put MDIO in IDLE Condition
*
******************************************************************************/

static void mdio_idle(void)
{
    sfr_outl(ETHERC_PIR, 0);            /* MMD = 0, MDC = 0 */
    dly_1us(1);                         /* wait for MDC low */
}

/*****************************************************************************
* Write 16 bits of MII Register on MDIO
*
******************************************************************************/

static void mdio_write(UH d)
{
    UH m;

    for (m = 0x8000; m != 0; m >>= 1)
        mdio_wr_bit(d & m);
}

/*****************************************************************************
* Read 16 bits of MII Register on MDIO
*
******************************************************************************/

static UH mdio_read(void)
{
    UH d, m;

    for (d = 0, m = 0x8000; m != 0; m >>= 1)
        d |= (mdio_rd_bit() ? m : 0);
    sfr_outl(ETHERC_PIR, PIR_MDC);      /* MMD = 0, MDC = 1 */
    dly_1us(1);                         /* wait for MDC high */
    return d;
}

/*****************************************************************************
* Write Data to PHY
*
******************************************************************************/

void PHY_write(UB reg, UH d)
{
    UB m;

    mdio_write(0xffff);                  /* 32-bits preamble */
    mdio_write(0xffff);
    mdio_wr_bit(0);                      /* 2-bits ST (start of frame) */
    mdio_wr_bit(1);
    mdio_wr_bit(0);                      /* 2-bits OP (write opcode) */
    mdio_wr_bit(1);
    for (m = 0x10; m != 0; m >>= 1)
        mdio_wr_bit(PHYAD & m);          /* 5-bits PHY address */
    for (m = 0x10; m != 0; m >>= 1)
        mdio_wr_bit(reg & m);            /* 5-bits register address */
    mdio_wr_bit(1);                      /* 2-bits TA (turn-around) */
    mdio_wr_bit(0);
    mdio_write(d);                       /* data */
    mdio_idle();
}

/*****************************************************************************
* Read Data from PHY
*
******************************************************************************/

UH PHY_read(UB reg)
{
    UB m;

    mdio_write(0xffff);                  /* 32-bits preamble */
    mdio_write(0xffff);
    mdio_wr_bit(0);                      /* 2-bits ST (start of frame) */
    mdio_wr_bit(1);
    mdio_wr_bit(1);                      /* 2-bits OP (read opcode) */
    mdio_wr_bit(0);
    for (m = 0x10; m != 0; m >>= 1)
        mdio_wr_bit(PHYAD & m);          /* 5-bits PHY address */
    for (m = 0x10; m != 0; m >>= 1)
        mdio_wr_bit(reg & m);            /* 5-bits register address */
    mdio_hi_z();                         /* Hi-Z and TA (turn-around) */
    return mdio_read();                  /* data */
}

/*****************************************************************************
* Get Link Status
*
******************************************************************************/

ER PHY_link(void)
{
    UH m;

    /* read status from PHY */
    PHY_read(PHY_STAT);                 /* read to clear latched link status */
    m = PHY_read(PHY_STAT);
    if (!(m & PHY_STAT_LINK_STAT))
        return LAN_LNK_OFF;

    /* fixed mode */
    if (!PHY_auto) {
        m = PHY_mode;
        if (m == FD100)
            return LAN_LNK_100F;
        if (m == HD100)
            return LAN_LNK_100H;
        if (m == FD10)
            return LAN_LNK_10F;
        return LAN_LNK_10H;
    }

    /* auto-negotiation mode */
    if (!(m & PHY_STAT_ANEG_COMP))
        return LAN_LNK_OFF;
    m = PHY_mode & PHY_read(PHY_ANLPA);
    if (m & AFD100)
        return LAN_LNK_100F;
    if (m & AHD100)
        return LAN_LNK_100H;
    if (m & AFD10)
        return LAN_LNK_10F;
    return LAN_LNK_10H;
}

/*****************************************************************************
* Initialize PHY
*
******************************************************************************/

ER PHY_init(BOOL nego, int mode, int lbtype)
{
    int i;

    PHY_mode = (UH)mode;
    PHY_auto = (UB)nego;

    /* auto-negotiation mode */
    if (nego) {
        /* set speed and duplex capabilities */
        PHY_write(PHY_ANA, ((UH)mode & PHY_ANA_TAF) | PHY_ANA_SF_802_3);
        /* restart auto-negotiation */
        PHY_write(PHY_CTRL, PHY_CTRL_ANE|PHY_CTRL_RS_ANP);
        /* wait for completion & link-up (up to 5 seconds) */
        for (i = 5000; i > 0; i -= mSEC) {
            dly_tsk(1);
            if ((PHY_read(PHY_STAT) & (PHY_STAT_ANEG_COMP | PHY_STAT_LINK_STAT))
                 == (PHY_STAT_ANEG_COMP | PHY_STAT_LINK_STAT))
                break;
        }

    /* fixed mode */
    } else {
        /* set loopback forcingly */
        if (lbtype != 0)
            mode |= PHY_CTRL_LOOPBK | PHY_CTRL_DUPLEX;
        /* set speed and duplex mode */
        PHY_write(PHY_CTRL, (UH)mode);
        /* 10BASE-T + loopback is excluded from link check */
        if (mode == (FD10 | PHY_CTRL_LOOPBK))
            return LAN_LNK_10F;
        /* wait for link-up (up to 3 seconds) */
        for (i = 3000; i > 0; i -= MSEC) {
            dly_tsk(1);
            if (PHY_read(PHY_STAT) & PHY_STAT_LINK_STAT)
                break;
        }
    }
    return PHY_link();
}

/* end */
