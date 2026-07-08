/******************************************************************************
* LAN Driver for RX72M ETHERC                                                 *
*                                                                             *
*  Copyright (c) 2020-2024, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Renesas RX72M                                       FY *
* 2020-11-18 Added missing ECMR_RTM (bit rate bit) set/clear in lan_link() FY *
* 2020-11-18 Fixed lan_link to reconfigure ETHERC also when speed changed  FY *
* 2020-11-18 Limited loop of TxBD empty wait in lan_stop() within 1 sec    FY *
* 2020-11-18 Moved FR bit clear before buffer empty check in lan_wai_rcv() FY *
* 2020-11-18 Moved TC bit clear before buffer full check in lan_wai_snd()  FY *
* 2021-02-02 Increased RFLR value 1518->1522 to receive VLAN-tagged frame  FY *
* 2021-02-02 Added delay after too continuous receiving to not occupy CPU  FY *
* 2021-02-10 Fixed poor setting of FCFTR/EMCR to work PAUSE flow control   FY *
* 2022-06-17 Renamed BUS_BEREN to BSC_BEREN                                FY *
* 2024-07-10 Replaced nosys4.h with nocsys.h on compiling for NORTi Oceans FY *
* 2024-07-10 Fixed bug of frequent delay after too continuous receiving    FY *
* 2024-11-11 Added releasing module stop of ETHERC0 in lan_ini_dev()          *
*                when ETHERC1 uses ET0_MDC/ET0_MDIO to interface with PHY  FY *
******************************************************************************/

#include <string.h>
#include "kernel.h"
#ifdef NORTi_Oceans
#include "nocsys.h"
#else
#include "nosys4.h"
#endif
#include "nonethw.h"

/************************************************/
/* MAC and PHY Configuration                    */
/************************************************/

#ifndef FULLDPLX                /* link speed / duplex mode */
#define FULLDPLX    0           /* 0: auto-negotiation, 1: 10M/half */
#endif                          /* 2: 10M/full, 3: 100M/half, 4: 100M/full */

#ifndef LINK_TIME               /* link check interval */
#define LINK_TIME   (1000/MSEC) /* 1 second */
#endif

#ifndef ALMUL                   /* multicast frame */
#define ALMUL       0           /* 0: discard, 1: receive all */
#endif

#ifndef PRMS                    /* promiscuous mode */
#define PRMS        0           /* 0: disable, 1: enable */
#endif
#if PRMS
#undef ALMUL
#define ALMUL       1
#endif

#ifndef LOOPBACK                /* PHY loopback test */
#define LOOPBACK    0           /* 0: disable, 1: enable(ETHERC) 2: enable(PHY) */
#endif
#ifndef LINK_CHECK
#define LINK_CHECK  1           /* link checking by interrupt */
#endif
#if LOOPBACK
#undef LINK_CHECK
#define LINK_CHECK  0           /* no checking */
#undef FULLDPLX
#define FULLDPLX    4           /* 100M/full at loopback mode */
#endif

#ifndef OVERRUN_TIME            /* time limit of continuous receiving */
#define OVERRUN_TIME 10         /* 10 ticks (0.1 sec when MSEC=10) */
#endif

/************************************************/
/* Buffer Configuration                         */
/************************************************/

#define TXBUFSZ     0x600       /* transmit buffer size */
#define RXBUFSZ     0x600       /* receive buffer size */

#ifndef NUM_TXBDS
#define NUM_TXBDS   2           /* number of transmit buffers & descriptors */
#endif
#ifndef NUM_RXBDS
#define NUM_RXBDS   8           /* number of receive buffers & descriptors */
#endif

#ifndef ALIGN
#define ALIGN(p,n)  ((((UW)(p)) + (((UW)(n)) - 1)) & (~(((UW)(n)) - 1)))
#endif

static UW RxTxBD[(sizeof (T_BD) * (NUM_RXBDS + NUM_TXBDS) + 12) / 4];
#define RxBD ((T_BD *)ALIGN(RxTxBD, sizeof (T_BD))) /* Tx buffer descriptors */
#define TxBD (RxBD + NUM_RXBDS)                     /* Rx buffer descriptors */

static UW RxTxBuf[((RXBUFSZ * NUM_RXBDS + TXBUFSZ * NUM_TXBDS) + 32 - 4) / 4];
#define RxBuf ((UB *)ALIGN(RxTxBuf, 32))
#define TxBuf (RxBuf + RXBUFSZ * NUM_RXBDS)

#if (NUM_RXBDS <= 4)
#define FCFTR_RFFO FCFTR_RFFO_2 /* receive FIFO frame PAUSE output threshold */
#elif (NUM_RXBDS <= 6)
#define FCFTR_RFFO FCFTR_RFFO_4
#elif (NUM_RXBDS <= 8)
#define FCFTR_RFFO FCFTR_RFFO_6
#elif (NUM_RXBDS <= 10)
#define FCFTR_RFFO FCFTR_RFFO_8
#elif (NUM_RXBDS <= 12)
#define FCFTR_RFFO FCFTR_RFFO_10
#elif (NUM_RXBDS <= 14)
#define FCFTR_RFFO FCFTR_RFFO_12
#elif (NUM_RXBDS <= 16)
#define FCFTR_RFFO FCFTR_RFFO_14
#else
#define FCFTR_RFFO FCFTR_RFFO_16
#endif

/************************************************/
/* Data and Functions                           */
/************************************************/

/* LAN Driver Control Block */

typedef struct t_lan {
    UH txtid;                   /* transmit task ID */
    UH rxtid;                   /* receive task ID */
    T_BD *txbd;                 /* current Tx buffer descriptor */
    T_BD *rxbd;                 /* current Rx buffer descriptor */
    UB *txbuf;                  /* write pointer to Tx buffer */
    UB *rxbuf;                  /* read pointer to Rx buffer */
    FP callback;                /* callback function */
    ER sts_mod;                 /* linked mode */
    ER lnk_mode;                /* last linked mode */
    ER lnk_chg;                 /* link change indication flag */
    UW ltime;                   /* continuous receiving started time */
} T_LAN;

static T_LAN lan;

/* Hardware Dependent Function in nonethw.c */

extern ER lan_def_int(void);

/* Atomic Bit Operation of Peripheral Register */

#define sfr_asetl(r,d) do { UINT psw = vdis_psw(); sfr_setl(r, d); vset_psw(psw); } while (0)
#define sfr_aclrl(r,d) do { UINT psw = vdis_psw(); sfr_clrl(r, d); vset_psw(psw); } while (0)

/*****************************************************************************
* Initialize Transmit Descriptor
*
******************************************************************************/

static void txbd_ini(void)
{
    T_BD *p;
    int i;

    /* set up Tx buffer descriptors */
    lan.txbd  = TxBD;
    lan.txbuf = TxBuf;
    p = lan.txbd;
    for (i = 0; i < NUM_TXBDS; i++, p++) {
        p->status  = 0;
        p->bufsize = 0;
        p->rxsize  = 0;             /* reserved area */
        p->buf_p   = lan.txbuf;
        lan.txbuf += TXBUFSZ;
    }
    p--;
    p->status |= DESC_DL;           /* last descripter */
    lan.txbuf = NULL;
}

/*****************************************************************************
* Initialize Receive Descriptor
*
******************************************************************************/

static void rxbd_ini(void)
{
    T_BD *p;
    int i;

    /* set up Rx buffer descriptors */
    lan.rxbd  = RxBD;
    lan.rxbuf = RxBuf;
    p = lan.rxbd;
    for (i = 0; i < NUM_RXBDS; i++, p++) {
        p->status  = DESC_ACT;
        p->bufsize = RXBUFSZ;
        p->rxsize  = 0;             /* reserved area */
        p->buf_p   = lan.rxbuf;
        lan.rxbuf += RXBUFSZ;
    }
    p--;
    p->status |= DESC_DL;           /* last descripter */
    lan.rxbuf = NULL;
}

/*****************************************************************************
* Call Callback Function
*
******************************************************************************/
#if (LINK_CHECK > 0)

static void lan_callback(ER ercd)
{
    FP func;

    if ((func = lan.callback) != NULL)
        ((ER (*)(int, ER, const char *))func)(NIF_NUM, ercd, "");
}

#endif
/*****************************************************************************
* Check MAC Address
*
******************************************************************************/

static int check_macaddr(UB *macaddr)
{
    int i, sum;

    for (sum = i = 0; i < 6; i++)
        sum += macaddr[i];
    if (sum == 0)
        return 0;               /* MAC address = 000000-000000 */
    else if (sum == 0xff * 6)
        return -1;              /* MAC address = FFFFFF-FFFFFF */
    return 1;                   /* other MAC address */
}

/*****************************************************************************
* Initialize LAN Driver
*
******************************************************************************/

ER lan_ini_dev(UB *macaddr, int lbtype)
{
    ER ercd;
    UW d;

    /* release module stop state */
    sfr_outw(RWP_PRCR, 0xA502);
  #if (ETH_CH == 1 && ETHERC_PIR == ETHERC0_PIR)
    sfr_clrl(LPC_MSTPCRB, 0x0000C000);
  #else
    sfr_clrl(LPC_MSTPCRB, 0x00008000 >> ETH_CH);
  #endif
    sfr_outw(RWP_PRCR, 0xA500);
    sfr_out(BSC_BEREN, 0x02);               /* enable bus timeout detection */

    /* clear LAN driver control block */
    memset(&lan, 0, sizeof (lan));

    /* issue software reset */
    sfr_outl(EDMAC_EDMR, EDMR_SWR);

    /* wait to finish initialization (64 cycles) */
    dly_1us(2);

    /* if MAC address is not valid, use local MAC address */
    if (check_macaddr(macaddr) <= 0) {
        macaddr[0] = 0x12; macaddr[1] = 0x34; macaddr[2] = 0x56;
        macaddr[3] = 0x78; macaddr[4] = 0x9A; macaddr[5] = 0xBC;
    }

    /* set MAC address to controller */
    d = macaddr[0] << 24 | macaddr[1] << 16 | macaddr[2] << 8 | macaddr[3];
    sfr_outl(ETHERC_MAHR, d);               /* MAC address high */
    d = macaddr[4] << 8 | macaddr[5];
    sfr_outl(ETHERC_MALR, d);               /* MAC address low */

    /* set up Tx/Rx descriptors */
    txbd_ini();
    rxbd_ini();

    /* initialize PHY device */
  #if (FULLDPLX == 0)   /* auto negotiation */
    lan.sts_mod = PHY_init(TRUE, AFD10 | AFD100 | AHD10 | AHD100, 0);
    lan.lnk_mode = (lan.sts_mod == LAN_LNK_OFF) ? LAN_LNK_100F : lan.sts_mod;
  #elif (FULLDPLX == 1) /* fixed to 10M/half */
    lan.sts_mod = PHY_init(FALSE, HD10, lbtype);
    lan.lnk_mode = LAN_LNK_10H;
  #elif (FULLDPLX == 2) /* fixed to 10M/full */
    lan.sts_mod = PHY_init(FALSE, FD10, lbtype);
    lan.lnk_mode = LAN_LNK_10F;
  #elif (FULLDPLX == 3) /* fixed to 100M/half */
    lan.sts_mod = PHY_init(FALSE, HD100, lbtype);
    lan.lnk_mode = LAN_LNK_100H;
  #elif (FULLDPLX == 4) /* fixed to 100M/full */
    lan.sts_mod = PHY_init(FALSE, FD100, lbtype);
    lan.lnk_mode = LAN_LNK_100F;
  #endif
    lan.lnk_chg = (lan.sts_mod == LAN_LNK_OFF);

    /* register LAN interrupt handler */
    ercd = lan_def_int();
    if (ercd != E_OK)
        return ercd;

    /* initialize EDMAC registers */
    sfr_outl(EDMAC_TDLAR, (UW)lan.txbd);
    sfr_outl(EDMAC_RDLAR, (UW)lan.rxbd);
  #ifdef BIG_ENDIAN
    sfr_outl(EDMAC_EDMR, EDMR_DL16 | EDMR_DE_BIG);
  #else
    sfr_outl(EDMAC_EDMR, EDMR_DL16 | EDMR_DE_LITTLE);
  #endif
    sfr_outl(EDMAC_EESR, 0x47FF0F9F);       /* clear all status */
    sfr_outl(EDMAC_EESIPR, EESIPR_ECIIP);
  #if (ALMUL == 1)
    sfr_outl(EDMAC_TRSCER, TRSCER_RMAFCE);
  #else
    sfr_outl(EDMAC_TRSCER, 0);
  #endif
    sfr_outl(EDMAC_TFTR, 0);
    sfr_outl(EDMAC_FDR, FDR_TFDMAX | FDR_RFDMAX);
    sfr_outl(EDMAC_RMCR, RMCR_RNR);
    sfr_outl(EDMAC_RPADIR, 0);

    /* initialize ETHERC registers */
  #if PRMS
    d = ECMR_PRM;
  #else
    d = 0;
  #endif
    if (lbtype == 1)
        d |= (ECMR_ILB | ECMR_PRM);
    else if (lbtype == 2)
        d |= ECMR_PRM;
    if (lan.lnk_mode == LAN_LNK_100F || lan.lnk_mode == LAN_LNK_10F)
        d |= ECMR_DM;
    if (lan.lnk_mode == LAN_LNK_100F || lan.lnk_mode == LAN_LNK_100H)
        d |= ECMR_RTM;
    sfr_outl(ETHERC_ECMR, d);
    sfr_outl(ETHERC_ECSR, 0x00000037);      /* clear all status */
  #if (LINK_CHECK == 1)
    sfr_outl(ETHERC_ECSIPR, ECSIPR_LCHNGIP);
  #endif
    sfr_outl(ETHERC_RFLR, 1522);
    sfr_outl(EDMAC_FCFTR, FCFTR_RFFO | FCFTR_RFDO_1760);
    sfr_outl(ETHERC_TPAUSER, 16);
    sfr_outl(ETHERC_APR, 0xFFFF);
    sfr_setl(ETHERC_ECMR, ECMR_ZPF | ECMR_RXF | ECMR_TXF);

    /* start EDMAC */
    sfr_outl(EDMAC_EDRRR, EDRRR_RR);
    sfr_outl(EDMAC_EDTRR, EDTRR_TR);

    /* start ETHERC */
    sfr_setl(ETHERC_ECMR, ECMR_TE | ECMR_RE);

    /* waiting for stability of PHY link-up */
    dly_tsk(100/MSEC);
    lan.ltime = SYSCK.ltime;

    return E_OK;
}

ER lan_ini(UB *macaddr)
{
    return lan_ini_dev(macaddr, LOOPBACK);
}

/*****************************************************************************
* Stop LAN Driver
*
******************************************************************************/

static void lan_stop(void)
{
    int t;

    /* wait for transmission complete */
    t = 1000/MSEC;
    while (sfr_inl(EDMAC_EDTRR) & EDTRR_TR) {
        dly_tsk(1);
        if (--t == 0)
            break;
    }

    /* disable transmit and receive functions of ETHERC */
    sfr_clrl(ETHERC_ECMR, ECMR_TE | ECMR_RE);
    while (sfr_inl(ETHERC_ECMR) & (ECMR_TE | ECMR_RE))
        dly_tsk(1);

    /* disable receive function of EDMAC */
    sfr_outl(EDMAC_EDRRR, 0);
}

/*****************************************************************************
* Deinitialize LAN Driver
*
******************************************************************************/

ER lan_ext_dev(void)
{
    lan_stop();                             /* stop LAN device operation */
    sfr_outl(ETHERC_ECSIPR, 0);             /* disable all ETHERC interrupts */
    sfr_outl(EDMAC_EESIPR, 0);              /* disable all EDMAC interrupts */
    PHY_write(PHY_CTRL, PHY_CTRL_PDOWN);    /* power-down PHY device */
    return E_OK;
}

/*****************************************************************************
* Check Link Status
*
******************************************************************************/
#if (LINK_CHECK > 0)

static void lan_link(void)
{
    ER sts;
    UW d;

    lan.lnk_chg = 0;
    sts = PHY_link();           /* update link status */
    if (sts == lan.sts_mod)     /* if link status was not changed */
        return;

    if (sts == LAN_LNK_OFF || sts == lan.lnk_mode)
        goto exit;              /* no need to change link setting of ETHERC */

    lan_stop();                 /* stop LAN device operation */

    /* change speed and duplex */
    d = sfr_inl(ETHERC_ECMR) & ~(ECMR_RTM | ECMR_DM);
    if (sts == LAN_LNK_100F || sts == LAN_LNK_100H)
        d |= ECMR_RTM;          /* 100Mbps */
    if (sts == LAN_LNK_100F || sts == LAN_LNK_10F)
        d |= ECMR_DM;           /* full duplex */
    sfr_outl(ETHERC_ECMR, d);

    rxbd_ini();                                 /* discard receive data */
    sfr_outl(EDMAC_EDRRR, EDRRR_RR);            /* start receive function of EDMAC */
    lan.rxbd = (T_BD *)sfr_inl(EDMAC_RDFAR);    /* update receive descriptor pointer */
    sfr_setl(ETHERC_ECMR, ECMR_TE | ECMR_RE);   /* start transmit and receive function of ETHERC */
    sfr_outl(EDMAC_EDTRR, EDTRR_TR);            /* start transmit function of EDMAC */

    lan.lnk_mode = sts;

exit:
    lan.sts_mod = sts;
    lan_callback(sts);          /* notify new link status */
}

#endif
/*****************************************************************************
* LAN Interrupt Handler
*
******************************************************************************/

void lan_intr(void)
{
    UW eesr;
    UW eesipr;
  #if (LINK_CHECK == 1)
    UW ecsr;
  #endif

    /* read interrupt status */
    eesr = sfr_inl(EDMAC_EESR);
    eesipr = sfr_inl(EDMAC_EESIPR);
    eesr &= eesipr;
    sfr_outl(EDMAC_EESR, eesr);             /* clear */

  #if (LINK_CHECK == 1)
    /* detection change link status */
    if (eesr & EESR_ECI) {
        ecsr = sfr_inl(ETHERC_ECSR);
        ecsr &= sfr_inl(ETHERC_ECSIPR);
        sfr_outl(ETHERC_ECSR, ecsr);        /* clear */

        if (ecsr & ECSR_LCHNG) {
            lan.lnk_chg = 1;
            if (lan.rxtid) {                /* if Rx task was slept, wake it up */
                iwup_tsk(lan.rxtid);
                lan.rxtid = 0;
                sfr_clrl(EDMAC_EESIPR, EESIPR_FRIP);
            }
        }
    }
  #endif

    /* receive interrupt */
    if (lan.rxtid) {                        /* if Rx task was slept, wake it up */
        if (eesr & EESR_FR) {
            iwup_tsk(lan.rxtid);
            lan.rxtid = 0;
            sfr_clrl(EDMAC_EESIPR, EESIPR_FRIP);
        }
    }

    /* transmit interrrupt */
    if (lan.txtid) {                        /* if Tx task was slept, wake it up */
        if (eesr & EESR_TC) {
            iwup_tsk(lan.txtid);
            lan.txtid = 0;
            sfr_clrl(EDMAC_EESIPR, EESIPR_TCIP);
        }
    }
}

/*****************************************************************************
* Finish Received Packet Reading
*
******************************************************************************/

ER lan_get_end(void)
{
    T_BD *p;

    /* mark buffer descriptor as empty and
       update current buffer descriptor pointer */
    p = lan.rxbd;
    p->rxsize = 0;
    if (p != RxBD + NUM_RXBDS - 1) {
        p->status = DESC_ACT;
        lan.rxbd = p + 1;
    } else {
        p->status = DESC_ACT | DESC_DL;
        lan.rxbd = p - (NUM_RXBDS - 1);
    }
    if (!(sfr_inl(EDMAC_EDRRR) & EDRRR_RR))
        sfr_outl(EDMAC_EDRRR, EDRRR_RR);
    return E_OK;
}

/*****************************************************************************
* Wait for Receive Packet
*
******************************************************************************/

ER lan_wai_rcv(TMO tmout)
{
    UW sts;
    TMO t;
    ER ercd;

    for (;;) {

      #if (LINK_CHECK > 0)
        if (lan.lnk_chg)
            lan_link();                         /* link change process */
      #endif
        sfr_outl(EDMAC_EESR, EESR_FR);          /* clear frame receive interrupt source */
        sts = lan.rxbd->status;
        if (!(sts & DESC_ACT)) {                /* packet received */
            if (SYSCK.ltime - lan.ltime > OVERRUN_TIME) {
                dly_tsk(1);
                lan.ltime = SYSCK.ltime;
            }
            if (sts & DESC_FE) {                /* if with error, skip packet */
                lan_get_end();
                continue;
            }
            return E_OK;
        }

      #if (LINK_CHECK == 2)
        t = (tmout == TMO_FEVR || tmout > LINK_TIME) ? LINK_TIME : tmout;
      #else
        t = tmout;
      #endif
        lan.rxtid = vget_tid();
        sfr_asetl(EDMAC_EESIPR, EESIPR_FRIP);   /* enable receive interrupt */
        ercd = tslp_tsk(t);                     /* wait for wake-up from receive interrupt */
        sfr_aclrl(EDMAC_EESIPR, EESIPR_FRIP);   /* disable receive interrupt */
        lan.ltime = SYSCK.ltime;                /* record received time */
        lan.rxtid = 0;
        vcan_wup();

        if (ercd == E_TMOUT) {                  /* timeout */
          #if (LINK_CHECK == 2)
            lan.lnk_chg = 1;
          #endif
            if (tmout != TMO_FEVR) {
                if ((tmout -= t) == 0)
                    return ercd;
            }
        } else if (ercd != E_OK) {              /* error */
            return ercd;
        }
      #if (LINK_CHECK == 2)
        else if (lan.sts_mod == LAN_LNK_OFF) {
            lan.lnk_chg = 1;
        }
      #endif
    }
}

/*****************************************************************************
* Wait until Ready to Transmit Packet
*
******************************************************************************/

ER lan_wai_snd(TMO tmout)
{
    ER ercd;

    for (;;) {
        sfr_outl(EDMAC_EESR, EESR_TC);          /* claer frame transmit interrupt source */
        if (!(lan.txbd->status & DESC_ACT))
            break;
        lan.txtid = vget_tid();
        sfr_asetl(EDMAC_EESIPR, EESIPR_TCIP);   /* enable transmit interrupt */
        ercd = tslp_tsk(tmout);                 /* wait for wake-up from transmit interrupt */
        sfr_aclrl(EDMAC_EESIPR, EESIPR_TCIP);   /* disable transmit interrupt */
        lan.txtid = 0;
        vcan_wup();
        if (ercd != E_OK)
            return ercd;
    }
    return E_OK;
}

/*****************************************************************************
* Get Received Packet Length
*
******************************************************************************/

ER lan_get_len(UH *len)
{
    *len = lan.rxbd->rxsize;
    lan.rxbuf = lan.rxbd->buf_p;
    return E_OK;
}

/*****************************************************************************
* Read Data from Received Packet
*
******************************************************************************/

ER lan_get_pkt(void *buf, int len)
{
    memcpy(buf, lan.rxbuf, len);
    lan.rxbuf += len;
    return E_OK;
}

/*****************************************************************************
* Skip Received Packet Reading
*
******************************************************************************/

ER lan_skp_pkt(int len)
{
    return lan_get_end();
}

/*****************************************************************************
* Set Transmit Packet Length
*
******************************************************************************/

ER lan_set_len(int len)
{
    lan.txbd->bufsize = (UH)len;
    lan.txbuf = lan.txbd->buf_p;
    return E_OK;
}

/*****************************************************************************
* Write Data into Transmit Packet
*
******************************************************************************/

ER lan_put_pkt(const void *data, int len)
{
    memcpy(lan.txbuf, data, len);
    lan.txbuf += len;
    return E_OK;
}

/*****************************************************************************
* Write Padding Bytes into Transmit Packet
*
******************************************************************************/

ER lan_put_dmy(int len)
{
    memset(lan.txbuf, 0, len);
    lan.txbuf += len;
    return E_OK;
}

/*****************************************************************************
* Finish Transmit Packet Writing
*
******************************************************************************/

ER lan_put_end(void)
{
    T_BD *p;

  #if (LINK_CHECK > 0)
    /* if link was down, throw out Tx packet */
    if (lan.sts_mod == LAN_LNK_OFF)
        return E_OK;
  #endif

    p = lan.txbd;
    if (p != TxBD + NUM_TXBDS - 1) {
        p->status = DESC_FP1 | DESC_FP0 | DESC_ACT;
        lan.txbd = p + 1;
    } else {
        p->status = DESC_FP1 | DESC_FP0 | DESC_ACT | DESC_DL;
        lan.txbd = p - (NUM_TXBDS - 1);
    }

    if (!(sfr_inl(EDMAC_EDTRR) & EDTRR_TR))
        sfr_outl(EDMAC_EDTRR, EDTRR_TR);   /* issue transmit request */

    return E_OK;
}

/*****************************************************************************
* Register Callback Function
*
******************************************************************************/

ER (lan_def_cbk)(FP func)
{
    lan.callback = func;
    return E_OK;
}

/*****************************************************************************
* Get Link and Initialization Status
*
******************************************************************************/

int lan_lnk_sts(void)
{
    return (sfr_inl(ETHERC_ECMR) & (ECMR_TE | ECMR_RE)) ? lan.sts_mod : -1;
}

/*****************************************************************************
* Initialize LAN controller and get ability of checksum offload 
*
******************************************************************************/

ER lan_ini_cko(UB *macaddr, UB *nif_cko)
{
    return E_NOSPT;
}

/*****************************************************************************
* Request checksum offload of transmit packet
*
******************************************************************************/

ER lan_snd_cko(UW cko, UW sum)
{
    return E_NOSPT;
}

/*****************************************************************************
* Register IP Multicast Address
*
******************************************************************************/

ER lan_add_mcst(UW ipaddr)
{
    return E_NOSPT;
}

/*****************************************************************************
* Unregister IP Multicast Address
*
******************************************************************************/

ER lan_del_mcst(UW ipaddr)
{
    return E_NOSPT;
}

/*****************************************************************************
* Register IPv6 Multicast Address
*
******************************************************************************/

ER lan_add_mcv6(UW gid)
{
    return E_NOSPT;
}

/*****************************************************************************
* Unregister IPv6 Multicast Address
*
******************************************************************************/

ER lan_del_mcv6(UW gid)
{
    return E_NOSPT;
}

/* end */
