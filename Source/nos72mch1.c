/******************************************************************************
* Serial Driver for RX72M SCI(ch1)                                            *
*                                                                             *
*  Copyright (c) 2020-2024, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Renesas RX72M                                       FY *
* 2020-05-15 Replaced default CLK macro definition with CLKA and CLKB      FY *
* 2020-11-18 Corrected dis/ena_int_sio to use IER instead of IPR register  FY *
* 2022-08-30 Added reading SCI_SSR before writing it to clear errors       FY *
* 2024-07-10 Changed int_sio_er from interrupt service routine to handler  FY *
* 2024-07-10 Removed unused imask member IP from T_DINH dinh_sio_rx/tx/er  FY *
* 2025-07-29 Replaced redefined INT_ERI/ICU_IPRSCI/_IPRERI/_GRPSCI/_GENSCI    *
*            with INT_GRPxL0/ICU_IPRxxx/_IPR11x/_GRPxL0/_GENxL0 straightly FY *
* 2025-07-29 Restored int_sio_er intr service routine when USE_ISR defined FY *
* 2025-07-29 Added static to dinh_sio_rx/_tx/_er to avoid redefinition     FY *
******************************************************************************/

#include <string.h>
#include "kernel.h"
#include "nosys4.h"
#include "nosio.h"

#define CHS	3					//add 2025.12.24 全使用チャンネル数3
#define CH1						//add 2025.12.24 物理チャンネルNo.1
#define CH	1					//add 2025.12.24 論理チャンネルNo.1

/* Baud Rate Generator Source Clock [Hz] */

#ifndef CLK
#define CLKA        120000000   /* peripheral clock (PCLKA) */
#define CLKB         60000000   /* peripheral clock (PCLKB) */
#endif

/* Interrupt Priority */

#ifdef SIO_IP
#undef  IP                      /* IP might be defined for interval timer, */
#define IP          SIO_IP      /* so you can use SIO_IP to override IP */
#endif
#ifndef IP
#define IP          6           /* 1 (lowest) ~ KNL_LEVEL (maximum 15) */
#endif                          /* must be equal among all channels */

/* Serial Interrupt Disable/Enable */

#ifdef DEISIO /* faster when Tx/Rx tasks have same priority */
#define DI_SIO() dis_int_sio()
#define EI_SIO() ena_int_sio()
#else
#define DI_SIO() chg_ims(IP)
#define EI_SIO() chg_ims(0)
#endif

/* Logical Channel Number */

#ifdef SIO_CH
#undef  CH                      /* CH might be defined for interval timer, */
#define CH          SIO_CH      /* so you can use SIO_CH to override CH */
#endif
#ifndef CH
#define CH          0           /* ch.0 ~ ch.7 */
#endif

/* Physical Channel Identifier */

#if !(defined(CH0)||defined(CH1)||defined(CH2)||defined(CH3)||defined(CH4)|| \
      defined(CH5)||defined(CH6)||defined(CH7)||defined(CH8)||defined(CH9)|| \
      defined(CH10)||defined(CH11)||defined(CH12))
#if (CH == 0)
#define CH0                     /* ch.0 is assigned to SCI0 by default */
#elif (CH == 1)
#define CH1                     /* ch.1 is assigned to SCI1 by default */
#elif (CH == 2)
#define CH2                     /* ch.2 is assigned to SCI2 by default */
#elif (CH == 3)
#define CH3                     /* ch.3 is assigned to SCI3 by default */
#elif (CH == 4)
#define CH4                     /* ch.4 is assigned to SCI4 by default */
#elif (CH == 5)
#define CH5                     /* ch.5 is assigned to SCI5 by default */
#elif (CH == 6)
#define CH6                     /* ch.6 is assigned to SCI6 by default */
#elif (CH == 7)
#define CH7                     /* ch.7 is assigned to SCI7 by default */
#else
#error CH is out of range!
#endif
#endif

#include "nos72m.h"

/* Receive/Transmit Buffer */

#ifndef BUFSZ
#define BUFSZ       1024        /* receive buffer size */
#endif
#ifndef TXBUFSZ
#define TXBUFSZ     BUFSZ       /* transmit buffer size */
#endif
static UB txbuf[TXBUFSZ];       /* transmit buffer */
static UB rxbuf[BUFSZ*2];       /* receive buffer (data and status) */

/* Serial Driver Control Block */

extern T_SIO SIO[];

#define TSF_TXRDY   TSF_RSV     /* TDR register is ready to write (reserved bit on flag) */
#define SSR_BRK     0x40        /* break detected (unused bit on rxsts) */
#define SSR_OVF     0x80        /* buffer overflow (unused bit on rxsts) */

/* Constants for Flow Control */

#define XOFF_LEN    (BUFSZ*3/4) /* buffer length to send XOFF */
#define XON_LEN     (BUFSZ*1/4) /* buffer length to send XON */

#define XOFF        0x13        /* XOFF character code (CTRL-S) */
#define XON         0x11        /* XON character code (CTRL-Q) */

/* Interrupt Handler Definition Information */

INTHDR int_sio_rx(void);
INTHDR int_sio_tx(void);
static const T_DINH dinh_sio_rx = { TA_HLNG, int_sio_rx };
static const T_DINH dinh_sio_tx = { TA_HLNG, int_sio_tx };
#ifndef USE_ISR
INTHDR int_sio_er(void);
static const T_DINH dinh_sio_er = { TA_HLNG, int_sio_er };
#else
void sio_er_int(VP_INT);
static const T_CISR cisr_sio_er = { TA_HLNG, NULL, INT_GRPxL0, sio_er_int, 0 };
#endif
static void tx_int(void);

/*****************************************************************************
* Enable/Disable Serial Interrupt
*
******************************************************************************/

static void ena_int_sio(void)
{
    UINT psw;

    psw = vdis_psw();
    ena_int(INT_RXI);
    ena_int(INT_TXI);
    sfr_setl(ICU_GENxL0, 3 << ERI_SHIFT);   /* ERIx/TEIx of GROUP AL0/BL0 */
    vset_psw(psw);
}

static void dis_int_sio(void)
{
    UINT psw;

    psw = vdis_psw();
    dis_int(INT_RXI);
    dis_int(INT_TXI);
    sfr_clrl(ICU_GENxL0, 3 << ERI_SHIFT);   /* ERIx/TEIx of GROUP AL0/BL0 */
    vset_psw(psw);
}

/*****************************************************************************
* Enable/Disable Serial Transmit/Receive
*
******************************************************************************/

static void set_scr_te(void)
{
    UB c;

    if (!((c = sfr_in(SCI_SCR)) & SCR_TE)) {
        if (c & SCR_RE)
            sfr_out(SCI_SCR, c & ~SCR_RE);
        sfr_out(SCI_SCR, c | (SCR_TIE | SCR_TE));
    }
}

static void clr_scr_te(void)
{
    UB c;

    if ((c = sfr_in(SCI_SCR)) & SCR_TE) {
        sfr_out(SCI_SCR, c & ~(SCR_RE | SCR_TIE | SCR_TE));
        if (c & SCR_RE)
            sfr_out(SCI_SCR, c & ~(SCR_TIE | SCR_TE));
    }
}

static void set_scr_re(void)
{
    UB c;

    if (!((c = sfr_in(SCI_SCR)) & SCR_RE)) {
        if (c & SCR_TE)
            sfr_out(SCI_SCR, c & ~SCR_TE);
        sfr_out(SCI_SCR, c | (SCR_RIE | SCR_RE));
    }
}

static void clr_scr_re(void)
{
    UB c;

    if ((c = sfr_in(SCI_SCR)) & SCR_RE) {
        sfr_out(SCI_SCR, c & ~(SCR_TE | SCR_RIE | SCR_RE));
        if (c & SCR_TE)
            sfr_out(SCI_SCR, c & ~(SCR_RIE | SCR_RE));
    }
}

/*****************************************************************************
* Resume Receive if Break Signal Turned Off
*
******************************************************************************/

static void pol_brk_off(void)
{
    if ((SIO[CH].rxsts & SSR_BRK) &&    /* break detected ? */
        (sfr_in(RXD_PIDR) & RXD_BIT)) { /* RxD pin = 1 ? */
        set_scr_re();                   /* enable Rx */
        SIO[CH].rxsts &= ~SSR_BRK;
    }
}

/*****************************************************************************
* Sleep Task while Polling Break Off
*
******************************************************************************/

static ER slp_tsk_pol(TMO tmout)
{
    ER ercd;

    while (tmout == TMO_FEVR || tmout-- > 0) {
        DI_SIO();                       /* enter exclusive control */
        pol_brk_off();                  /* resume Rx if break off */
        EI_SIO();                       /* exit exclusive control */
        ercd = tslp_tsk(1);             /* wait for receive or transmit */
        if (ercd != E_TMOUT)
            return ercd;
    }
    return E_TMOUT;
}

/*****************************************************************************
* Initialize Serial Driver Buffer
*
******************************************************************************/

static void init_buf(void)
{
    /* clear control block */
    memset(&SIO[CH], 0, sizeof SIO[CH]);
    SIO[CH].ch = CH;

    /* set up pointers to buffers */
    SIO[CH].txputp = txbuf;             /* put-pointer to Tx buffer */
    SIO[CH].txgetp = txbuf;             /* get-pointer to Tx buffer */
    SIO[CH].rxputp = rxbuf;             /* put-pointer to Rx buffer */
    SIO[CH].rxgetp = rxbuf;             /* get-pointer to Rx buffer */
}

/*****************************************************************************
* Clear Transmit Buffer
*
******************************************************************************/

static void clr_txbuf(void)
{
    SIO[CH].txcnt = 0;
    SIO[CH].txgetp = SIO[CH].txputp;
}

/*****************************************************************************
* Clear Receive Buffer
*
******************************************************************************/

static void clr_rxbuf(void)
{
    /* initialize status, counters and pointers for Rx buffer */
    SIO[CH].rxsts = 0;
    SIO[CH].oldsts = 0;
    SIO[CH].eotcnt = 0;
    SIO[CH].rxcnt = 0;
    SIO[CH].rxgetp = SIO[CH].rxputp;

    /* release XON/OFF flow control */
  #ifndef NOXON
    if (SIO[CH].flag & TSF_TXOFF) {     /* when XOFF was sent */
        SIO[CH].flag |= TSF_TXREQ;      /* request to send XON */
        tx_int();                       /* send XON if possible */
    } else {                            /* since it might be requested */
        SIO[CH].flag &= ~TSF_TXREQ;     /* cancel sending XOFF */
    }
  #endif
}

/*****************************************************************************
* Store Sending Character into Transmit Buffer
*
******************************************************************************/

static BOOL put_txbuf(UB c)
{
    UB *p;

    /* check Tx buffer full*/
    if (++SIO[CH].txcnt > TXBUFSZ) {
        SIO[CH].txcnt = TXBUFSZ;
        return FALSE;
    }

    /* store data into Tx buffer */
    p = SIO[CH].txputp;
    *p = c;

    /* increment put-pointer for ring-buffer */
    if (++p >= txbuf + TXBUFSZ)
        p = txbuf;
    SIO[CH].txputp = p;

    /* send data if possible */
    tx_int();

    return TRUE;
}

/*****************************************************************************
* Store Received Character and Status into Buffer
*
******************************************************************************/

static void put_rxbuf(void)
{
    UB c, *p;
    int cnt;

    /* discard continuous break */
    if ((SIO[CH].rxsts & SSR_BRK) && (SIO[CH].oldsts & SSR_BRK))
        return;

    /* check Rx buffer full */
    cnt = SIO[CH].rxcnt;
    if (cnt == BUFSZ)
        return;

    /* increment received byte count */
    if (++cnt == BUFSZ)
        SIO[CH].rxsts |= SSR_OVF;       /* assign reservation bit for overflow */
    SIO[CH].rxcnt = (UH)cnt;

    /* store data into Rx buffer */
    p = SIO[CH].rxputp;
    c = SIO[CH].rxchr;
    if ((*p = c) == SIO[CH].eot)        /* if delimiter */
        SIO[CH].eotcnt++;               /* increment delimiter count */
    *(p + BUFSZ) = SIO[CH].rxsts;

    /* increment put-pointer for ring-buffer */
    if (++p >= rxbuf + BUFSZ)
        p = rxbuf;
    SIO[CH].rxputp = p;

    /* XON/OFF flow control */
  #ifndef NOXON
    if ((cnt == XOFF_LEN) &&
        ((SIO[CH].flag & (TSF_XON | TSF_TXOFF)) == TSF_XON)) {
        SIO[CH].flag |= TSF_TXREQ;      /* request to send XOFF */
        tx_int();                       /* send XOFF if possible */
    }
  #endif
}

/*****************************************************************************
* Get Character from Receive Buffer
*
******************************************************************************/

static int get_rxbuf(UB *c)
{
    int cnt, ssr;
    UB *p;

    /* decrement Rx byte counter */
    cnt = SIO[CH].rxcnt;
    if (--cnt == -1)
        return cnt;                     /* return -1 if empty */
    SIO[CH].rxcnt = (UH)cnt;

    /* XON/OFF flow control */
  #ifndef NOXON
    if ((cnt == XON_LEN) && (SIO[CH].flag & TSF_TXOFF)) {
        SIO[CH].flag |= TSF_TXREQ;      /* request to send XON */
        tx_int();                       /* send XON if possible */
    }
  #endif

    /* get data from Rx buffer */
    p = SIO[CH].rxgetp;
    if ((*c = *p) == SIO[CH].eot)       /* if delimiter */
        SIO[CH].eotcnt--;               /* decrement delimiter count */
    ssr = *(p + BUFSZ);

    /* increment get-pointer for ring-buffer */
    if (++p >= rxbuf + BUFSZ)
        p = rxbuf;
    SIO[CH].rxgetp = p;

    return ssr;
}

/*****************************************************************************
* Receive Interrupt Handler Body
*
******************************************************************************/

#pragma noinline(rx_int)
static void rx_int(void)
{
    UB c, ssr;
    ID tid;

    /* input received data/status and clear error status */
    c = sfr_in(SCI_RDR);
    if ((ssr = sfr_in(SCI_SSR) & (SSR_PER|SSR_FER|SSR_ORER)) != 0)
        sfr_out(SCI_SSR, 0xC0);

    if ((ssr & SSR_FER) &&              /* framing error and */
        (c == 0x00) &&                  /* receiving 0x00 and */
       !(sfr_in(RXD_PIDR) & RXD_BIT)) { /* RxD pin = 0 ? */
        ssr |= SSR_BRK;                 /* break detected */
        clr_scr_re();                   /* disable Rx */
    }

    SIO[CH].oldsts = SIO[CH].rxsts;     /* last received status */
    SIO[CH].rxchr = c;
    SIO[CH].rxsts = ssr;

    /* XON/OFF flow control */
  #ifndef NOXON
    if (SIO[CH].flag & TSF_XON) {
        /* stop sending when XOFF received */
        if (c == XOFF) {
            SIO[CH].flag |= TSF_RXOFF;
            return;
        }
        /* restart sending when XON received */
        if (c == XON) {
            SIO[CH].flag &= ~TSF_RXOFF;
            tx_int();                   /* send data if possible */
            return;
        }
    }
  #endif

    /* store received data into Rx buffer */
    put_rxbuf();

    /* wake up the task waiting for receiving */
    if ((tid = SIO[CH].rxtid) != 0)
        iwup_tsk(tid);
}

/*****************************************************************************
* Transmit Interrupt Handler Body
*
******************************************************************************/

#pragma noinline(tx_int)
static void tx_int(void)
{
    BOOL intr;
    UB *p;
    ID tid;

    intr = (INEST != 0);                /* in interrupt handler ? */
    if (!intr && !(SIO[CH].flag & TSF_TXRDY))
        return;

    /* when Tx disabled, send nothing */
    if (!(sfr_in(SCI_SCR) & SCR_TE)) {
        sfr_clr(SCI_SCR, SCR_TIE);
        SIO[CH].flag &= ~TSF_TXRDY;
        return;
    }
    SIO[CH].flag |= TSF_TXRDY;

  #ifndef NOXON
    /* send XON/OFF when requested */
    if (SIO[CH].flag & TSF_TXREQ) {
        SIO[CH].flag &= ~TSF_TXREQ;
        if (SIO[CH].flag & TSF_TXOFF) {
            SIO[CH].flag &= ~TSF_TXOFF;
            sfr_out(SCI_TDR, XON);
        } else {
            SIO[CH].flag |= TSF_TXOFF;
            sfr_out(SCI_TDR, XOFF);
        }
        return;
    }

    /* when XOFF received, send nothing and return */
    if (SIO[CH].flag & TSF_RXOFF)
        return;
  #endif

    /* check Tx buffer empty */
    if (--SIO[CH].txcnt == (UH)-1) {
        SIO[CH].txcnt = 0;
        sfr_set(SCI_SCR, SCR_TEIE);
        if (sfr_in(SCI_SSR) & SSR_TEND) {
            sfr_clr(SCI_SCR, SCR_TEIE);
            if ((tid = SIO[CH].tetid) != 0) {
                if (intr)               /* if task is waiting for all data sent */
                    iwup_tsk(tid);      /* wake it up */
                else
                    wup_tsk(tid);
            }
        }
        return;
    }

    /* send data */
    p = SIO[CH].txgetp;
    sfr_out(SCI_TDR, *p);
    SIO[CH].flag &= ~TSF_TXRDY;

    /* increment get-pointer for ring-buffer */
    if (++p >= txbuf + TXBUFSZ)
        p = txbuf;
    SIO[CH].txgetp = p;

    /* wake up the task waiting for Tx buffer empty */
    if ((tid = SIO[CH].txtid) != 0) {
        if (intr)
            iwup_tsk(tid);
        else
            wup_tsk(tid);
    }
}

/*****************************************************************************
* Transmit End Interrupt Handler Body
*
******************************************************************************/

static void te_int(void)
{
    ID tid;

    /* wake up the task waiting for all data sent */
    if ((tid = SIO[CH].tetid) != 0)
        iwup_tsk(tid);

    /* disable Tx end interrupt */
    sfr_clr(SCI_SCR, SCR_TEIE);
}

/*****************************************************************************
* Serial Receive/Transmit Interrupt Handler
*
******************************************************************************/

INTHDR int_sio_rx(void)
{
    ent_int();                          /* enter into interrupt handler */
    rx_int();                           /* call interrupt handler body */
    ret_int();                          /* return from interrupt handler */
}

INTHDR int_sio_tx(void)
{
    ent_int();
    tx_int();
    ret_int();
}

/*****************************************************************************
* Transmit End & Serial Error Interrupt Handler or Interrupt Service Routine
*
******************************************************************************/
#ifndef USE_ISR

void sio0_er_int(void);
void sio1_er_int(void);
void sio2_er_int(void);
void sio3_er_int(void);
void sio4_er_int(void);
void sio5_er_int(void);
void sio6_er_int(void);
void sio7_er_int(void);

#pragma noinline(sio_er_int)
void sio_er_int(void)
{
    if (sfr_inl(ICU_GRPxL0) & (1 << ERI_SHIFT))
        te_int();
    if (sfr_inl(ICU_GRPxL0) & (2 << ERI_SHIFT))
        rx_int();
}

INTHDR int_sio_er(void)
{
    ent_int();
    sio0_er_int();
  #if (CHS > 1)
    sio1_er_int();
  #if (CHS > 2)
    sio2_er_int();
  #if (CHS > 3)
    sio3_er_int();
  #if (CHS > 4)
    sio4_er_int();
  #if (CHS > 5)
    sio5_er_int();
  #if (CHS > 6)
    sio6_er_int();
  #if (CHS > 7)
    sio7_er_int();
  #endif
  #endif
  #endif
  #endif
  #endif
  #endif
  #endif
    ret_int();
}

#else

void sio_er_int(VP_INT exinf)
{
    if (sfr_inl(ICU_GRPxL0) & (1 << ERI_SHIFT))
        te_int();
    if (sfr_inl(ICU_GRPxL0) & (2 << ERI_SHIFT))
        rx_int();
}

#endif
/*****************************************************************************
* Parse and Setup Serial Parameter
*
******************************************************************************/

static BOOL set_param(const char *s, UB *smr, UW *br)
{
    int c;
    UW b;

    /* parity, stop bit, data length */
    c = 0;
    if (strstr(s, "PE") != NULL)   c |= SMR_PE;
    if (strstr(s, "PO") != NULL)   c |= SMR_PE|SMR_PM;
    if (strstr(s, "S2") != NULL)   c |= SMR_STOP;
    if (strstr(s, "B7") != NULL)   c |= SMR_CHR;
    *smr = (UB)c;

    /* flow control */
  #ifndef NOXON
    if (strstr(s, "XON") != NULL)  SIO[CH].flag |= TSF_XON;
  #endif

    /* delimiter character code */
    c = 0;
    if (strstr(s, "ETX") != NULL)  c = 0x03;
    if (strstr(s, "EOT") != NULL)  c = 0x04;
    if (strstr(s, "CR" ) != NULL)  c = 0x0d;
    if (strstr(s, "LF" ) != NULL)  c = 0x0a;
    SIO[CH].eot = (UB)c;

    /* baud rate */
    for (;;) {
        /* search for three or more digits */
        c = *s++;
        if (c == '\0') {
            *br = 9600L;                /* 9600 baud by default */
            break;
        }
        if (c < '0' || c > '9')
            continue;
        c = *s++;
        if (c < '0' || c > '9')
            continue;
        c = *s++;
        if (c < '0' || c > '9')
            continue;

        /* convert from 1st digit */
        s -= 3;
        b = 0L;
        for (;;) {
            c = *s++;
            if (c < '0' || c > '9')
                break;
            b = b * 10 + (c - '0');
        }
        *br = b;
        break;
    }
    return TRUE;
}

/*****************************************************************************
* Initialize Serial Controller
*
******************************************************************************/

ER ini_sio(const char *param)
{
    UB smr;
    UW br, brr;
    int cks, abcs;
  #ifdef USE_ISR
    ER ercd;
  #endif

    dis_int_sio();                      /* disable serial interrupt */

    init_buf();                         /* initialize buffers */
    SIO[CH].flag |= TSF_INIT;           /* initialized */

    if (!set_param(param, &smr, &br))   /* read parameter string */
        return E_PAR;

    /* set I/O port functions */
    sfr_out(MPC_PWPR, 0x00);            /* B0WI=0 (enable writing to PFSWE bit) */
    sfr_out(MPC_PWPR, PWPR_PFSWE);      /* PFSWE=1 (enable writing to PFS register) */
    sfr_out(MPC_TXDPFS, PSEL_TXD);      /* select pin function as TxD */
    sfr_out(MPC_RXDPFS, PSEL_RXD);      /* select pin function as RxD */
    sfr_out(MPC_PWPR, PWPR_B0WI);       /* B0WI=1 (disable writing to PFSWE bit) */

    sfr_set(TXD_PMR, TXD_BIT);          /* set port mode for TxD pin */
    sfr_set(RXD_PMR, RXD_BIT);          /* set port mode for RxD pin */

    sfr_outw(RWP_PRCR, 0xA502);         /* enable writing to low-power consumption register */
    sfr_clrl(LPC_MSTPCR, MSTP_BIT);     /* cancel SCI module stop state */
    sfr_outw(RWP_PRCR, 0xA500);         /* protect writing */

    /* calculate baud rate preset value */
    br <<= 4;   /* 32×2^(-1)×B */
    for (cks = 0; cks < 4; cks++, br <<= 1) {
        abcs = SEMR_ABCS;               /* 8 base clock cycles for 1-bit period */
        brr = (CLK / br) - 1;
        if (brr <= 0xff)
            break;
        br <<= 1;
        abcs = 0;                       /* 16 base clock cycles for 1-bit period */
        brr = (CLK / br) - 1;
        if (brr <= 0xff)
            break;
    }
    if (cks == 4)
        return E_PAR;

    /* set up baud rate generator */
    sfr_out(SCI_SCR, 0x00);             /* CKE=00 (on-chip baud rate generator) */
    sfr_out(SCI_SMR, (UB)(smr | cks));
    sfr_out(SCI_SEMR, (UB)abcs);
    sfr_out(SCI_SCMR, 0xF2);            /* SMIF=0 (serial communications interface mode) */
    sfr_out(SCI_BRR, (UB)brr);

    /* register interrupt handlers */
    def_inh(INT_RXI, &dinh_sio_rx);
    def_inh(INT_TXI, &dinh_sio_tx);
  #ifndef USE_ISR
    def_inh(INT_GRPxL0, &dinh_sio_er);
  #else
    if (!(sfr_inl(ICU_GENxL0) & (1 << ERI_SHIFT))) {
        if ((ercd = acre_isr(&cisr_sio_er)) < 0)
            return ercd;
    }
  #endif

    /* enable interrupts */
    sfr_out(ICU_IPRxxx, IP);            /* RXI */
    sfr_out(ICU_IPRxxx + 1, IP);        /* TXI */
    sfr_out(ICU_IPR11x, IP);            /* TEI, ERI */
    ena_int(INT_GRPxL0);
    sfr_set(SCI_SCR, SCR_TIE | SCR_RIE | SCR_TE | SCR_RE);
//    sfr_set(SCI_SCR, SCR_TE | SCR_RE);

    ena_int_sio();
    return E_OK;
}

/*****************************************************************************
* Terminate/Close Serial Controller Channel
*
******************************************************************************/

void ext_sio(void)
{
    dis_int_sio();                      /* disable serial interrupt */
    sfr_out(SCI_SCR, 0x00);             /* stop Tx/Rx */
    SIO[CH].flag &= ~TSF_INIT;          /* uninitialized */
}

/*****************************************************************************
* Read Single Character from Serial Port
*
******************************************************************************/

ER get_sio(UB *c, TMO tmout)
{
    int ssr;
    ER ercd;

    for (;;) {
        DI_SIO();                       /* enter exclusive control between serial interrupt */

        ssr = get_rxbuf(c);             /* get data & status from Rx buffer */

        if (ssr != -1) {                /* if valid data received */
            EI_SIO();                   /* exit exclusive control and return with error code */
            if (ssr & (SSR_OVF | SSR_ORER | SSR_BRK | SSR_FER | SSR_PER)) {
                if (ssr & SSR_OVF)
                    ercd = EV_SIOOVF;   /* Rx buffer overflow */
                else if (ssr & SSR_ORER)
                    ercd = EV_SIOORN;   /* overrun error */
                else if (ssr & SSR_BRK)
                    ercd = EV_SIOBRK;   /* break detected */
                else if (ssr & SSR_FER)
                    ercd = EV_SIOFRM;   /* framing error */
                else    /* SSR_PER */
                    ercd = EV_SIOPTY;   /* parity error */
            } else {
                ercd = E_OK;
            }
            return ercd;
        }
                                        /* if Rx buffer is empty */
        SIO[CH].rxtid = vget_tid();
        EI_SIO();                       /* exit exclusive control */
        ercd = slp_tsk_pol(tmout);      /* poll break off & wait for data reception */
        SIO[CH].rxtid = 0;
        vcan_wup();                     /* cancel queued wakeup requests */
        if (ercd != E_OK)
            return ercd;                /* return error if timeout */
    }
}

/*****************************************************************************
* Write Single Character to Serial Port
*
******************************************************************************/

ER put_sio(UB c, TMO tmout)
{
    ER ercd;

    for (;;) {
        DI_SIO();                       /* enter exclusive control */

        if (put_txbuf(c)) {             /* when data stored into Tx buffer */
            EI_SIO();                   /* exit exclusive control and return */
            return E_OK;
        }
                                        /* if Tx buffer is full */
        SIO[CH].txtid = vget_tid();
        EI_SIO();                       /* exit exclusive control */
        ercd = slp_tsk_pol(tmout);      /* poll break off & wait for buffer space to free */
        SIO[CH].txtid = 0;
        vcan_wup();
        if (ercd != E_OK)
            return ercd;                /* return error if timeout */
    }
}

/*****************************************************************************
* Control Serial Port
*
******************************************************************************/

ER ctl_sio(UH fncd)
{
    DI_SIO();                           /* enter exclusive control */

    /* clear Rx/Tx buffers */
    if (fncd & TSIO_RXCLR)
        clr_rxbuf();
    if (fncd & TSIO_TXCLR)
        clr_txbuf();

    /* Rx/Tx enable/disable */
    if (fncd & TSIO_RXE) set_scr_re();
    if (fncd & TSIO_RXD) clr_scr_re();
    if (fncd & TSIO_TXE) set_scr_te();
    if (fncd & TSIO_TXD) clr_scr_te();

    /* if Rx disable, clear status register */
    if (fncd & TSIO_RXD) {
        if ((sfr_in(SCI_SSR) & (SSR_PER|SSR_FER|SSR_ORER)) != 0)
            sfr_out(SCI_SSR, 0xC0);
        SIO[CH].rxsts &= ~SSR_BRK;
    } else {
        pol_brk_off();                  /* resume Rx if break off */
    }

    /* break signal on/off control */
    if (fncd & TSIO_SBON) {
        clr_scr_te();
        sfr_clr(TXD_PODR, TXD_BIT);     /* output low level */
        sfr_set(TXD_PDR, TXD_BIT);      /* output port */
        sfr_clr(TXD_PMR, TXD_BIT);      /* use pin as general I/O port */
    }
    if (fncd & TSIO_SBOFF) {
        set_scr_te();
        sfr_set(TXD_PMR, TXD_BIT);      /* use pin as I/O port for peripheral functions */
    }

    EI_SIO();                           /* exit exclusive control */
    return E_OK;
}

/*****************************************************************************
* Refer to Serial Port Status
*
******************************************************************************/

ER ref_sio(T_SIOS *pk_sios)
{
    UB stat, ssr;

    DI_SIO();                           /* enter exclusive control */
    pol_brk_off();                      /* resume Rx if break off */

    /* read present status */
    ssr = sfr_in(SCI_SSR) & (SSR_TEND|SSR_PER|SSR_FER|SSR_ORER);

    /* add received error status */
    ssr |= SIO[CH].rxsts & (SSR_BRK|SSR_PER|SSR_FER);

    /* map to driver status bits */
    stat = TSIO_DSR|TSIO_CD|TSIO_CTS;   /* DSR, CD, CTS always ON */
    if (ssr & SSR_TEND)                 /* Tx empty */
        stat |= TSIO_TXEMP;
    if (ssr & SSR_PER)                  /* parity error */
        stat |= TSIO_PE;
    if (ssr & SSR_ORER)                 /* overrun error */
        stat |= TSIO_OE;
    if (ssr & SSR_FER)                  /* framing error */
        stat |= TSIO_FE;
    if (ssr & SSR_BRK)                  /* break detected */
        stat |= TSIO_BD;
    if (SIO[CH].txcnt != 0)
        stat &= ~TSIO_TXEMP;
    pk_sios->siostat = stat;

    /* set Rx data length, free Tx buffer size and delimiter count */
    pk_sios->rxlen   = SIO[CH].rxcnt;
    pk_sios->frbufsz = (UH)(TXBUFSZ - SIO[CH].txcnt);
    pk_sios->eotcnt  = SIO[CH].eotcnt;

    EI_SIO();                           /* exit exclusive control */
    return E_OK;
}

/*****************************************************************************
* Flush Transmit Buffer of Serial Port
*
******************************************************************************/

ER fls_sio(TMO tmout)
{
    ER ercd;

    for (;;) {
        DI_SIO();                       /* enter exclusive control */

        if (!(sfr_in(SCI_SCR) & SCR_TE) ||
            (SIO[CH].txcnt == 0 && (SIO[CH].flag & TSF_TXRDY) && !(sfr_in(SCI_SCR) & SCR_TEIE))) {
            EI_SIO();                   /* if it isn't being sent, */
            return E_OK;                /* exit exclusive control and return */
        }

        SIO[CH].tetid = vget_tid();
        EI_SIO();                       /* exit exclusive control */
        ercd = slp_tsk_pol(tmout);      /* poll break off & wait for all data to be sent */
        SIO[CH].tetid = 0;
        vcan_wup();                     /* cancel queued wakeup requests */
        if (ercd != E_OK)
            return ercd;                /* return error if timeout */
    }
}

/* end */
