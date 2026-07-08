/******************************************************************************
* LAN Driver Header for RX72M ETHERC                                          *
*                                                                             *
*  Copyright (c) 2020-2021, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Renesas RX72M                                       FY *
* 2021-02-10 Renamed FCFTR_RFDO256~2048/_RFFO2/4/16 to FCFTR_RFDO_224~2016    *
*                  /_RFFO_2/4/16, and added FCFTR_RFDO_736~1760/_RFFO_6~14 FY *
******************************************************************************/

#ifndef ETH72M_H
#define ETH72M_H
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Bit Definition of ETHERC                                                  */
/*****************************************************************************/

/* ETHERC Mode Register (ECMR) */

#define ECMR_PRM        (1 <<  0)  /* Promiscuous mode */
#define ECMR_DM         (1 <<  1)  /* Duplex mode */
#define ECMR_RTM        (1 <<  2)  /* Bit rate */
#define ECMR_ILB        (1 <<  3)  /* Internal loopback mode */
#define ECMR_TE         (1 <<  5)  /* Transmission enable */
#define ECMR_RE         (1 <<  6)  /* Reception enable */
#define ECMR_MPDE       (1 <<  9)  /* Magic packet detection enable */
#define ECMR_PRCEF      (1 << 12)  /* CRC error frame receive mode */
#define ECMR_TXF        (1 << 16)  /* Transmit flow control operating mode */
#define ECMR_RXF        (1 << 17)  /* Receive flow control operating mode */
#define ECMR_PFR        (1 << 18)  /* PAUSE frame receive mode */
#define ECMR_ZPF        (1 << 19)  /* 0 time PAUSE frame enable */
#define ECMR_TPC        (1 << 20)  /* PAUSE frame transmit */

/* ETHERC Status Register (ECSR) */

#define ECSR_ICD        (1 <<  0)  /* False carrier detect flag */
#define ECSR_MPD        (1 <<  1)  /* Magic packet detect flag */
#define ECSR_LCHNG      (1 <<  2)  /* Link signal change flag */
#define ECSR_PSRTO      (1 <<  4)  /* PAUSE frame retransmit over flag */
#define ECSR_BFR        (1 <<  5)  /* Continuous broadcast frame reception flag */

/* ETHERC Interrupt Enable Register (ECSIPR) */

#define ECSIPR_ICDIP    (1 <<  0)  /* False carrier detect interrupt enable */
#define ECSIPR_MPDIP    (1 <<  1)  /* Magic packet detect interrupt enable */
#define ECSIPR_LCHNGIP  (1 <<  2)  /* LINK signal change interrupt enable */
#define ECSIPR_PSRTOIP  (1 <<  4)  /* PAUSE frame retransmit over interrupt enable */
#define ECSIPR_BFSIPR   (1 <<  5)  /* Continuous broadcast frame reception interrupt enable */

/* PHY Interface Register (PIR) */

#define PIR_MDC         (1 <<  0)  /* MII/RMII management data clock */
#define PIR_MMD         (1 <<  1)  /* MII/RMII management mode */
#define PIR_MDO         (1 <<  2)  /* MII/RMII management data-out */
#define PIR_MDI         (1 <<  3)  /* MII/RMII management data-in */

/* PHY Status Register (PSR) */

#define PSR_LMON        (1 <<  0)  /* ETn_LINKSTA pin status flag */

/* Interpacket Gap Register (IPGR) */

#define IPGR_IPG96      0x14       /* Interpacket gap */

/*****************************************************************************/
/* Bit Definition of EDMAC                                                   */
/*****************************************************************************/

/* EDMAC Mode Register (EDMR) */

#define EDMR_SWR        (1 <<  0)  /* Software reset */
#define EDMR_DL16       (0 <<  4)  /* Transmit/Receive descriptor length 16 bytes */
#define EDMR_DL32       (1 <<  4)  /*                                    32 bytes */
#define EDMR_DL64       (2 <<  4)  /*                                    64 bytes */
#define EDMR_DE_BIG     (0 <<  6)  /* Big endian mode */
#define EDMR_DE_LITTLE  (1 <<  6)  /* Little endian mode */

/* EDMAC Transmit Request Register (EDTRR) */

#define EDTRR_TR        (1 <<  0)  /* Transmit request */

/* EDMAC Receive Request Register (EDRRR) */

#define EDRRR_RR        (1 <<  0)  /* Receive request */

/* ETHERC/EDMAC Status Register (EESR) */

#define EESR_CERF       (1 <<  0)  /* CRC error flag */
#define EESR_PRE        (1 <<  1)  /* PHY-LSI receive error flag */
#define EESR_RTSF       (1 <<  2)  /* Frame-too-short error flag */
#define EESR_RTLF       (1 <<  3)  /* Frame-too-long error flag */
#define EESR_RRF        (1 <<  4)  /* Alignment error flag */
#define EESR_RMAF       (1 <<  7)  /* Multicast address frame receive flag */
#define EESR_TRO        (1 <<  8)  /* Transmit retry over flag */
#define EESR_CD         (1 <<  9)  /* Late collision detect flag */
#define EESR_DLC        (1 << 10)  /* Loss of carrier detect flag */
#define EESR_CND        (1 << 11)  /* Carrier not detect flag */
#define EESR_RFOF       (1 << 16)  /* Receive FIFO overflow flag */
#define EESR_RDE        (1 << 17)  /* Receive descriptor empty flag */
#define EESR_FR         (1 << 18)  /* Frame receive flag */
#define EESR_TFUF       (1 << 19)  /* Transmit FIFO underflow flag */
#define EESR_TDE        (1 << 20)  /* Transmit descriptor empty flag */
#define EESR_TC         (1 << 21)  /* Frame transfer complete flag */
#define EESR_ECI        (1 << 22)  /* ETHERC status register source flag */
#define EESR_ADE        (1 << 23)  /* Address error flag */
#define EESR_RFCOF      (1 << 24)  /* Receive frame counter overflow flag */
#define EESR_RABT       (1 << 25)  /* Receive abort detect flag */
#define EESR_TABT       (1 << 26)  /* Transmit abort detect flag */
#define EESR_TWB        (1 << 30)  /* Write-back complete flag */

/* ETHERC/EDMAC Status Interrupt Enable Register (EESIPR) */

#define EESIPR_CERFIP   (1 <<  0)  /* CRC error interrupt request enable */
#define EESIPR_PREIP    (1 <<  1)  /* PHY-LSI receive error interrupt request enable */
#define EESIPR_RTSFIP   (1 <<  2)  /* Frame-too-short error interrupt request enable */
#define EESIPR_RTLFIP   (1 <<  3)  /* Frame-too-long error interrupt request enable */
#define EESIPR_RRFIP    (1 <<  4)  /* Alignment error interrupt request enable */
#define EESIPR_RMAFIP   (1 <<  7)  /* Multicast address frame receive interrupt request enable */
#define EESIPR_TROIP    (1 <<  8)  /* Transmit retry over interrupt request enable */
#define EESIPR_CDIP     (1 <<  9)  /* Late collision detect interrupt request enable */
#define EESIPR_DLCIP    (1 << 10)  /* Loss of carrier detect interrupt request enable */
#define EESIPR_CNDIP    (1 << 11)  /* Carrier not detect interrupt request enable */
#define EESIPR_RFOFIP   (1 << 16)  /* Receive FIFO overflow interrupt request enable */
#define EESIPR_RDEIP    (1 << 17)  /* Receive descriptor empty interrupt request enable */
#define EESIPR_FRIP     (1 << 18)  /* Frame receive interrupt request enable */
#define EESIPR_TFUFIP   (1 << 19)  /* Transmit FIFO underflow interrupt request enable */
#define EESIPR_TDEIP    (1 << 20)  /* Transmit descriptor empty interrupt request enable */
#define EESIPR_TCIP     (1 << 21)  /* Frame transfer complete interrupt request enable */
#define EESIPR_ECIIP    (1 << 22)  /* ETHERC status register source interrupt request enable */
#define EESIPR_ADEIP    (1 << 23)  /* Address error interrupt request enable */
#define EESIPR_RFCOFIP  (1 << 24)  /* Receive frame counter overflow interrupt request enable */
#define EESIPR_RABTIP   (1 << 25)  /* Receive abort detect interrupt request enable */
#define EESIPR_TABTIP   (1 << 26)  /* Transmit abort detect interrupt request enable */
#define EESIPR_TWBIP    (1 << 30)  /* Write-back complete interrupt request enable */

/* ETHERC/EDMAC Transmit/Receive Status Copy Enable Register (TRSCER) */

#define TRSCER_RRFCE    (1 <<  4)  /* RRF flag copy enable */
#define TRSCER_RMAFCE   (1 <<  7)  /* RMAF flag copy enable */

/* FIFO Depth Register (FDR) */

#define FDR_RFD4096    (15 <<  0)  /* Receive FIFO depth */
#define FDR_TFD2048     (7 <<  8)  /* Transmit FIFO depth */
#define FDR_RFDMAX    FDR_RFD4096
#define FDR_TFDMAX    FDR_TFD2048

/* Receive Method Control Register (RMCR) */

#define RMCR_RNR        (1 <<  0)  /* Receive request reset */

/* Independent Output Signal Setting Register (IOSR) */

#define IOSR_ELB        (1 <<  0)  /* External loopback mode */

/* Flow Control Start FIFO Threshold Setting Register (FCFTR) */

#define FCFTR_RFDO_224  (0 <<  0)  /* Receive FIFO data PAUSE output threshold */
#define FCFTR_RFDO_480  (1 <<  0)
#define FCFTR_RFDO_736  (2 <<  0)
#define FCFTR_RFDO_992  (3 <<  0)
#define FCFTR_RFDO_1248 (4 <<  0)
#define FCFTR_RFDO_1504 (5 <<  0)
#define FCFTR_RFDO_1760 (6 <<  0)
#define FCFTR_RFDO_2016 (7 <<  0)
#define FCFTR_RFFO_2    (0 << 16)  /* Receive FIFO frame PAUSE output threshold */
#define FCFTR_RFFO_4    (1 << 16)
#define FCFTR_RFFO_6    (2 << 16)
#define FCFTR_RFFO_8    (3 << 16)
#define FCFTR_RFFO_10   (4 << 16)
#define FCFTR_RFFO_12   (5 << 16)
#define FCFTR_RFFO_14   (6 << 16)
#define FCFTR_RFFO_16   (7 << 16)

/* Transmit Interrupt Setting Register (TRIMD) */

#define TRIMD_TIS       (1 <<  0)  /* Transmit interrupt enable */
#define TRIMD_TIM       (1 <<  4)  /* Transmit interrupt mode */

/*****************************************************************************/
/* Buffer Descripter                                                         */
/*****************************************************************************/

typedef struct BufferDescript {
    UW status;
  #ifdef BIG_ENDIAN
    UH bufsize;
    UH rxsize;
  #else
    UH rxsize;
    UH bufsize;
  #endif
    UB *buf_p;
    UW reserved[1]; /* user working area */
} T_BD;

/* Buffer Descripter Status */

#define DESC_WBI        (1 << 26)
#define DESC_FE         (1 << 27)
#define DESC_FP0        (1 << 28)
#define DESC_FP1        (1 << 29)
#define DESC_DL         (1 << 30)
#define DESC_ACT      (1UL << 31)

/*****************************************************************************/
/* Functions for Link Status Check                                           */
/*****************************************************************************/

#ifndef NONELAN_H
extern ER lan_def_cbk(FP);
extern ER lan_def_cbk1(FP);
#define lan_def_cbk(f)  (lan_def_cbk)((FP)(f))
#define lan_def_cbk1(f) (lan_def_cbk1)((FP)(f))
extern int lan_lnk_sts(void);
extern int lan_lnk_sts1(void);
#endif

/* Link Status Code */

#define LAN_LNK_OFF     1          /* link down */
#define LAN_LNK_10H     2          /* linked at 10Mbps half-duplex */
#define LAN_LNK_10F     3          /* linked at 10Mbps full-duplex */
#define LAN_LNK_100H    4          /* linked at 100Mbps half-duplex */
#define LAN_LNK_100F    5          /* linked at 100Mbps full-duplex */
#define LAN_RCV_OVW     6          /* receiver buffer overflow */
#define LAN_ANEG_FAIL   7          /* auto-negotiation failure */

/*****************************************************************************/
/* Redefinition of ETHERC/EDMAC Registers for Dual Ethernet Port             */
/*****************************************************************************/

#if (ETH_CH == 0)
#define ETHERC_ECMR     ETHERC0_ECMR
#define ETHERC_ECSR     ETHERC0_ECSR
#define ETHERC_ECSIPR   ETHERC0_ECSIPR
#define ETHERC_RFLR     ETHERC0_RFLR
#define ETHERC_PIR      ETHERC0_PIR
#define ETHERC_MAHR     ETHERC0_MAHR
#define ETHERC_MALR     ETHERC0_MALR
#define ETHERC_PSR      ETHERC0_PSR
#define ETHERC_TROCR    ETHERC0_TROCR
#define ETHERC_CDCR     ETHERC0_CDCR
#define ETHERC_LCCR     ETHERC0_LCCR
#define ETHERC_CNDCR    ETHERC0_CNDCR
#define ETHERC_CEFCR    ETHERC0_CEFCR
#define ETHERC_FRECR    ETHERC0_FRECR
#define ETHERC_TSFRCR   ETHERC0_TSFRCR
#define ETHERC_TLFRCR   ETHERC0_TLFRCR
#define ETHERC_RFCR     ETHERC0_RFCR
#define ETHERC_MAFCR    ETHERC0_MAFCR
#define ETHERC_IPGR     ETHERC0_IPGR
#define ETHERC_APR      ETHERC0_APR
#define ETHERC_MPR      ETHERC0_MPR
#define ETHERC_TPAUSER  ETHERC0_TPAUSER
#define ETHERC_RDMLR    ETHERC0_RDMLR
#define ETHERC_RFCF     ETHERC0_RFCF
#define ETHERC_TPAUSECR ETHERC0_TPAUSECR
#define ETHERC_BCFRR    ETHERC0_BCFRR
#define EDMAC_EDMR      EDMAC0_EDMR
#define EDMAC_EDTRR     EDMAC0_EDTRR
#define EDMAC_EDRRR     EDMAC0_EDRRR
#define EDMAC_TDLAR     EDMAC0_TDLAR
#define EDMAC_RDLAR     EDMAC0_RDLAR
#define EDMAC_EESR      EDMAC0_EESR
#define EDMAC_EESIPR    EDMAC0_EESIPR
#define EDMAC_TRSCER    EDMAC0_TRSCER
#define EDMAC_RMFCR     EDMAC0_RMFCR
#define EDMAC_TFTR      EDMAC0_TFTR
#define EDMAC_FDR       EDMAC0_FDR
#define EDMAC_RMCR      EDMAC0_RMCR
#define EDMAC_TFUCR     EDMAC0_TFUCR
#define EDMAC_RFOCR     EDMAC0_RFOCR
#define EDMAC_RBWAR     EDMAC0_RBWAR
#define EDMAC_RDFAR     EDMAC0_RDFAR
#define EDMAC_TBRAR     EDMAC0_TBRAR
#define EDMAC_TDFAR     EDMAC0_TDFAR
#define EDMAC_FCFTR     EDMAC0_FCFTR
#define EDMAC_RPADIR    EDMAC0_RPADIR
#define EDMAC_TRIMD     EDMAC0_TRIMD
#define EDMAC_IOSR      EDMAC0_IOSR

#else /* ETH_CH == 1 */
#define ETHERC_ECMR     ETHERC1_ECMR
#define ETHERC_ECSR     ETHERC1_ECSR
#define ETHERC_ECSIPR   ETHERC1_ECSIPR
#define ETHERC_RFLR     ETHERC1_RFLR
#define ETHERC_PIR      ETHERC1_PIR
#define ETHERC_MAHR     ETHERC1_MAHR
#define ETHERC_MALR     ETHERC1_MALR
#define ETHERC_PSR      ETHERC1_PSR
#define ETHERC_TROCR    ETHERC1_TROCR
#define ETHERC_CDCR     ETHERC1_CDCR
#define ETHERC_LCCR     ETHERC1_LCCR
#define ETHERC_CNDCR    ETHERC1_CNDCR
#define ETHERC_CEFCR    ETHERC1_CEFCR
#define ETHERC_FRECR    ETHERC1_FRECR
#define ETHERC_TSFRCR   ETHERC1_TSFRCR
#define ETHERC_TLFRCR   ETHERC1_TLFRCR
#define ETHERC_RFCR     ETHERC1_RFCR
#define ETHERC_MAFCR    ETHERC1_MAFCR
#define ETHERC_IPGR     ETHERC1_IPGR
#define ETHERC_APR      ETHERC1_APR
#define ETHERC_MPR      ETHERC1_MPR
#define ETHERC_TPAUSER  ETHERC1_TPAUSER
#define ETHERC_RDMLR    ETHERC1_RDMLR
#define ETHERC_RFCF     ETHERC1_RFCF
#define ETHERC_TPAUSECR ETHERC1_TPAUSECR
#define ETHERC_BCFRR    ETHERC1_BCFRR
#define EDMAC_EDMR      EDMAC1_EDMR
#define EDMAC_EDTRR     EDMAC1_EDTRR
#define EDMAC_EDRRR     EDMAC1_EDRRR
#define EDMAC_TDLAR     EDMAC1_TDLAR
#define EDMAC_RDLAR     EDMAC1_RDLAR
#define EDMAC_EESR      EDMAC1_EESR
#define EDMAC_EESIPR    EDMAC1_EESIPR
#define EDMAC_TRSCER    EDMAC1_TRSCER
#define EDMAC_RMFCR     EDMAC1_RMFCR
#define EDMAC_TFTR      EDMAC1_TFTR
#define EDMAC_FDR       EDMAC1_FDR
#define EDMAC_RMCR      EDMAC1_RMCR
#define EDMAC_TFUCR     EDMAC1_TFUCR
#define EDMAC_RFOCR     EDMAC1_RFOCR
#define EDMAC_RBWAR     EDMAC1_RBWAR
#define EDMAC_RDFAR     EDMAC1_RDFAR
#define EDMAC_TBRAR     EDMAC1_TBRAR
#define EDMAC_TDFAR     EDMAC1_TDFAR
#define EDMAC_FCFTR     EDMAC1_FCFTR
#define EDMAC_RPADIR    EDMAC1_RPADIR
#define EDMAC_TRIMD     EDMAC1_TRIMD
#define EDMAC_IOSR      EDMAC1_IOSR
#endif

#ifdef __cplusplus
}
#endif
#endif /* ETH72M_H */
