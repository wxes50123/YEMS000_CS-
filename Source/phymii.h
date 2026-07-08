/******************************************************************************
* MII Management Register Set Definition for IEEE 802.3                       *
*                                                                             *
*  Copyright (c) 2003-2016, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2003-12-10 Created as phy.h from dp83843.h                               OH *
* 2004-06-29 Changed PHY_init() declaration to support PHY loopback        OK *
* 2004-06-29 Added PHY_link() declaration to check link status             OK *
* 2006-07-21 Added PHY_read() & PHY_write() declaration for SH-Ether       AK *
* 2010-08-08 Added PHY_reset() declaration for TI EMAC/MDIO                KM *
* 2011-10-21 Unified and renamed to phymii.h for universal use             MI *
* 2011-12-02 Added PHY_???0/1 declarations with DIF_NUM for multi-channel  FY *
* 2013-03-23 Added declaration of PHY_pause() for negotiate pause ability  SZ *
* 2013-10-08 Corrected PHY_ES_100T_FD/100T_HD to PHY_ES_1000T_FD/1000T_HD  SZ *
* 2013-10-08 Changed HD10~AFD100 to abstract values to add HD1000~AFD1000  SZ *
* 2013-10-08 Added FIX_HD10~_FD1000/AUTO_HD10~_FD1000 macros with actual      *
*              values of PHY_CTRL/ANA/MSC registers instead of HD10~AFD100 SZ *
* 2016-10-18 Deleted incompatible FIX_HD10~FD1000/AUTO_HD10~FD1000 macros,    *
*               and recovered HD10~AFD100 to actual values of PHY_CTRL/ANA MI *
* 2016-10-18 Added PHY_ANA_RSV/_EX_NP to assign them to AHD1000/AFD1000,      *
*                              and corrected PHY_ANA_TAF 0x1FE0 to 0x0FE0  MI *
* 2016-10-18 Removed PHY_???0/1 since they are declared outside this file  MI *
******************************************************************************/

#ifndef __PHY__H__
#define __PHY__H__

#ifdef __cplusplus
extern "C" {
#endif

/* MII Management Register Address */

#define PHY_CTRL              0x00   /* Control */
#define PHY_STAT              0x01   /* Status */
#define PHY_ID1               0x02   /* PHY Identifier 1 */
#define PHY_ID2               0x03   /* PHY Identifier 2 */
#define PHY_ANA               0x04   /* Auto-Negotiation Advertisement */
#define PHY_ANLPA             0x05   /* Auto-Negotiation Link Partner Ability */
#define PHY_ANE               0x06   /* Auto-Negotiation Expansion */
#define PHY_ANNPT             0x07   /* Auto-Negotiation Next Page Transmit */
#define PHY_ANLPR             0x08   /* Auto-Negotiation Link Partner Received Next Page */
#define PHY_MSC               0x09   /* MASTER-SLAVE Control */
#define PHY_MSS               0x0A   /* MASTER-SLAVE Status */
#define PHY_PSE_C             0x0B   /* PSE Control */
#define PHY_PSE_S             0x0C   /* PSE Status */
#define PHY_ES                0x0F   /* Extended Status */

/* (Register 0) Control Register */

#define PHY_CTRL_RESET        0x8000 /* PHY reset */
#define PHY_CTRL_LOOPBK       0x4000 /* enable loopback mode */
#define PHY_CTRL_SPD_LSB      0x2000
#define PHY_CTRL_ANE          0x1000 /* enable auto-negotiation process */
#define PHY_CTRL_PDOWN        0x0800 /* power down */
#define PHY_CTRL_ISOL         0x0400 /* electrically isolate PHY from MII or GMII */
#define PHY_CTRL_RS_ANP       0x0200 /* restart auto-negotiation process */
#define PHY_CTRL_DUPLEX       0x0100 /* 1 = full duplex, 0 = half duplex */
#define PHY_CTRL_COL_TEST     0x0080 /* enable collision signal test */
#define PHY_CTRL_SPD_MSB      0x0040

/* (Register 1) Status Register */

#define PHY_STAT_100_T4       0x8000 /* PHY able to perform 100BASE-T4 */
#define PHY_STAT_100_X_FD     0x4000 /* PHY able to perform full duplex 100BASE-X */
#define PHY_STAT_100_X_HD     0x2000 /* PHY able to perfomr half duplex 100BASE-X */
#define PHY_STAT_10_FD        0x1000 /* PHY able to operate at 10 Mb/s in full duplex */
#define PHY_STAT_10_HD        0x0800 /* PHY able to operate at 10 Mb/s in half duplex */
#define PHY_STAT_100_T2_FD    0x0400 /* PHY able to perform full duplex 100BASE-T2 */
#define PHY_STAT_100_T2_HD    0x0200 /* PHY able to perform half duplex 100BASE-T2 */
#define PHY_STAT_EX_STAT      0x0100 /* exist extended status info register 15 */
#define PHY_STAT_PREAMBLE     0x0040 /* PHY will accept management frames with preample suppressed */
#define PHY_STAT_ANEG_COMP    0x0020 /* auto-negotiation process completed */
#define PHY_STAT_RM_FAULT     0x0010 /* remote fault condition detected */
#define PHY_STAT_AN_ABLE      0x0008 /* PHY is able to perform auto-negotiation */
#define PHY_STAT_LINK_STAT    0x0004 /* 1 = link is up, 0 = link is down */
#define PHY_STAT_JABBER       0x0002 /* jabber condition detect */
#define PHY_STAT_EXT_CAP      0x0001 /* register capabilities: 0=extended, 1=basic */

/* (Register 4) Auto-Negotiation Advertisement Register */

#define PHY_ANA_NP            0x8000 /* next page */
#define PHY_ANA_RSV           0x4000 /* reserved */
#define PHY_ANA_R_FAULT       0x2000 /* remote fault */
#define PHY_ANA_EX_NP         0x1000 /* extended next page */
#define PHY_ANA_TAF           0x0FE0 /* technology ability field */
#define PHY_ANA_TAF_APO       0x0800 /*   asymmetric pause operation for full duplex links */
#define PHY_ANA_TAF_PO        0x0400 /*   pause operation for full duplex links */
#define PHY_ANA_TAF_100T4     0x0200 /*   100BASE-T4 */
#define PHY_ANA_TAF_100TX_F   0x0100 /*   100BASE-TX full duplex */
#define PHY_ANA_TAF_100TX_H   0x0080 /*   100BASE-TX half duplex */
#define PHY_ANA_TAF_10T_F     0x0040 /*   10BASE-TX full duplex */
#define PHY_ANA_TAF_10T_H     0x0020 /*   10BASE-TX half duplex */
#define PHY_ANA_SF            0x001F /* selector field */
#define PHY_ANA_SF_802_5      0x0003 /*   IEEE std 802.5 */
#define PHY_ANA_SF_802_9      0x0002 /*   IEEE std 802.9 ISLAN-16T */
#define PHY_ANA_SF_802_3      0x0001 /*   IEEE std 802.3 */

/* (Register 5) Auto-Negotiation Link Partner Abilty Register (Base Page) */

#define PHY_ANLPA_B_NP        0x8000 /* next page */
#define PHY_ANLPA_B_ACK       0x4000 /* acknowledge */
#define PHY_ANLPA_B_R_FAULT   0x2000 /* remote fault */
#define PHY_ANLPA_B_TAF       0x1FE0 /* technology ability field */
#define PHY_ANLPA_B_TAF_APO   0x0800 /*   asymmetric pause operation for full duplex links */
#define PHY_ANLPA_B_TAF_PO    0x0400 /*   pause operation for full duplex links */
#define PHY_ANLPA_B_TAF_T4    0x0200 /*   100BASE-T4 */
#define PHY_ANLPA_B_TAF_TX_F  0x0100 /*   100BASE-TX full duplex */
#define PHY_ANLPA_B_TAF_TX_H  0x0080 /*   100BASE-TX half duplex */
#define PHY_ANLPA_B_TAF_T_F   0x0040 /*   10BASE-T full duplex */
#define PHY_ANLPA_B_TAF_T_H   0x0020 /*   10BASE-T half duplex */
#define PHY_ANLPA_B_SF        0x001F /* selector field */
#define PHY_ANLPA_B_SF_802_5  0x0003 /*   IEEE std 802.5 */
#define PHY_ANLPA_B_SF_802_9  0x0002 /*   IEEE std 802.9 ISLAN-16T */
#define PHY_ANLPA_B_SF_802_3  0x0001 /*   IEEE std 802.3 */

/* (Register 5) Auto-Negotiation Link Partner Abilty Register (Next Page) */

#define PHY_ANLPA_N_NP        0x8000 /* next page */
#define PHY_ANLPA_N_ACK       0x4000 /* acknowledge */
#define PHY_ANLPA_N_MP        0x2000 /* message page */
#define PHY_ANLPA_N_ACK2      0x1000 /* acknowledge 2 */
#define PHY_ANLPA_N_TOGGLE    0x0800 /* toggle */
#define PHY_ANLPA_N_MCF       0x07FF /* message/unformatted code field */
#define PHY_ANLPA_N_MCF_NULL  0x0001 /*   null message */
#define PHY_ANLPA_N_MCF_TAE1  0x0002 /*   technology ability extension 1 */
#define PHY_ANLPA_N_MCF_TAE2  0x0003 /*   technology ability extension 2 */
#define PHY_ANLPA_N_MCF_RFN   0x0004 /*   remote fault number */
#define PHY_ANLPA_N_MCF_OUI   0x0005 /*   organizationally unique identifier */
#define PHY_ANLPA_N_MCF_PI    0x0006 /*   PHY identifier */
#define PHY_ANLPA_N_MCF_100T2 0x0007 /*   100BASE-T2 technology message */
#define PHY_ANLPA_N_MCF_1000T 0x0008 /*   1000BASE-T technology message */

/* (Register 6) Auto-Negotiation Expansion */

#define PHY_ANE_PDF           0x0010 /* parallel detection fault */
#define PHY_ANE_LP_NPA        0x0008 /* link partner next page able */
#define PHY_ANE_LOCAL_NPA     0x0004 /* local device is next page able */
#define PHY_ANE_P_RECIEVED    0x0002 /* a new page has benn reveived */
#define PHY_ANE_LP_ANA        0x0001 /* link partner is auto-negotiation able */

/* (Register 7) Auto-Negotiation Next Page Transmit Register */

#define PHY_ANNPT_NP          0x8000 /* next page */
#define PHY_ANNPT_MP          0x2000 /* message page */
#define PHY_ANNPT_ACK2        0x1000 /* acknowledge2 */
#define PHY_ANNPT_TOGGLE      0x0800 /* toggle */
#define PHY_ANNPT_MCF         0x07FF /* message/unformatted code field */
#define PHY_ANNPT_MCF_NULL    0x0001 /*   null message */
#define PHY_ANNPT_MCF_TAE1    0x0002 /*   technology ability extension 1 */
#define PHY_ANNPT_MCF_TAE2    0x0003 /*   technology ability extension 2 */
#define PHY_ANNPT_MCF_RFN     0x0004 /*   remote fault number */
#define PHY_ANNPT_MCF_OUI     0x0005 /*   organizationally unique identifier */
#define PHY_ANNPT_MCF_PI      0x0006 /*   PHY identifier */
#define PHY_ANNPT_MCF_100T2   0x0007 /*   100BASE-T2 technology message */
#define PHY_ANNPT_MCF_1000T   0x0008 /*   1000BASE-T technology message */

/* (Register 8) Auto-Negotiation Link Partner Received Next Page Register */

#define PHY_ANLPR_NP          0x8000 /* next page */
#define PHY_ANLPR_ACK         0x4000 /* acknowledge */
#define PHY_ANLPR_MP          0x2000 /* message page */
#define PHY_ANLPR_ACK2        0x1000 /* acknowledge2 */
#define PHY_ANLPR_TOGGLE      0x0800 /* toggle */
#define PHY_ANLPR_MCF         0x07FF /* message/unformatted code field */
#define PHY_ANLPR_MCF_NULL    0x0001 /*   null message */
#define PHY_ANLPR_MCF_TAE1    0x0002 /*   technology ability extension 1 */
#define PHY_ANLPR_MCF_TAE2    0x0003 /*   technology ability extension 2 */
#define PHY_ANLPR_MCF_RFN     0x0004 /*   remote fault number */
#define PHY_ANLPR_MCF_OUI     0x0005 /*   organizationally unique identifier */
#define PHY_ANLPR_MCF_PI      0x0006 /*   PHY identifier */
#define PHY_ANLPR_MCF_100T2   0x0007 /*   100BASE-T2 technology message */
#define PHY_ANLPR_MCF_1000T   0x0008 /*   1000BASE-T technology message */

/* (Register 9) MASTER-SLAVE Control Register (100BASE-T2) */

#define PHY_MSC_100_TTM       0xC000 /* transmitter test mode */
#define PHY_MSC_100_RTM       0x2000 /* receiver test mode */
#define PHY_MSC_100_MCE       0x1000 /* master-slave manual configuration enable */
#define PHY_MSC_100_MCV       0x0800 /* master-slave manual configuration value */
#define PHY_MSC_100_T2R_DTE   0x0400 /* 1 = repeater device port, 0 = DTE deice */

/* (Register 9) MASTER-SLAVE Control Register (1000BASE-T) */

#define PHY_MSC_1000_TTM      0xE000 /* transmitter test mode */
#define PHY_MSC_1000_MCE      0x1000 /* master-slave manual configuration enable */
#define PHY_MSC_1000_MCV      0x0800 /* master-slave manual configuration value */
#define PHY_MSC_1000_PT       0x0400 /* port type: 1 = multiport, 1 = single-port */
#define PHY_MSC_1000_FD       0x0200 /* 1000BASE-T full duplex capable */
#define PHY_MSC_1000_HD       0x0100 /* 1000BASE-T half duplex capable */

/* (Register 10) MASTER-SLAVE Status Register (100BASE-T2) */

#define PHY_MSS_100_MCF       0x8000 /* master-slave manual configuration fault */
#define PHY_MSS_100_CRC       0x4000 /* master-slave configuration resolution complete */
#define PHY_MSS_100_LRS       0x2000 /* local receiver status: 1=ok */
#define PHY_MSS_100_RRS       0x1000 /* remote receiver status: 1=ok */
#define PHY_MSS_100_IE_CNT    0x00FF /* idle error count */

/* (Register 10) MASTER-SLAVE Status Register (1000BASE-T) */

#define PHY_MSS_1000_MCF      0x8000 /* master-slave manual configuration fault */
#define PHY_MSS_1000_CR       0x4000 /* master-slave configuration resolution: 1 = master, 0 = slave */
#define PHY_MSS_1000_LRS      0x2000 /* local receiver status: 1=ok */
#define PHY_MSS_1000_RRS      0x1000 /* remote receiver status: 1=ok */
#define PHY_MSS_1000_LP_FD    0x0800 /* link partner is capable of 1000BASE-T half duplex */
#define PHY_MSS_1000_LP_HD    0x0400 /* link partner is capable of 1000BASE-T full duplex */
#define PHY_MSS_1000_IE_CNT   0x00FF /* idle error count */

/* (Register 11) PSE Control Register */

#define PHY_PSE_C_PC          0x000C /* pair control */
#define PHY_PSE_C_PC_PO_A_B   0x0008 /*   PSE pinout alternative b */
#define PHY_PSE_C_PC_PO_A_A   0x0004 /*   PSE pinout alternative a */
#define PHY_PSE_C_PE          0x0003 /* PSE enable */
#define PHY_PSE_C_PE_FPTM     0x0002 /*   force power test mode */
#define PHY_PSE_C_PE_EN       0x0001 /*   PSE enabled */
#define PHY_PSE_C_PE_DIS      0x0000 /*   PSE disabled */

/* (Register 12) PSE Status Register */

#define PHY_PSE_S_PD          0x1000 /* power denied */
#define PHY_PSE_S_VS          0x0800 /* valid signature */
#define PHY_PSE_S_IS          0x0400 /* invalid signature */
#define PHY_PSE_S_SC          0x0200 /* short circuit */
#define PHY_PSE_S_OL          0x0100 /* overload */
#define PHY_PSE_S_MIPS_ABSENT 0x0080 /* MIPS Absent */
#define PHY_PSE_S_PD_CLASS    0x0070 /* PD class */
#define PHY_PSE_S_PD_CLASS_4  0x0040 /*   class 4 */
#define PHY_PSE_S_PD_CLASS_3  0x0030 /*   class 3 */
#define PHY_PSE_S_PD_CLASS_2  0x0020 /*   class 2 */
#define PHY_PSE_S_PD_CLASS_1  0x0010 /*   class 1 */
#define PHY_PSE_S_PD_CLASS_0  0x0000 /*   class 0 */
#define PHY_PSE_S_STAT        0x000E /* PSE status */
#define PHY_PSE_S_STAT_ISF    0x000A /*   implementation specific fault*/
#define PHY_PSE_S_STAT_TE     0x0008 /*   test error */
#define PHY_PSE_S_STAT_TM     0x0006 /*   test mode */
#define PHY_PSE_S_STAT_DP     0x0004 /*   delivering power */
#define PHY_PSE_S_STAT_S      0x0002 /*   searcing */
#define PHY_PSE_S_STAT_D      0x0000 /*   disabled */
#define PHY_PSE_S_PCA         0x0001 /* pair control ability */

/* (Register 15) Extended Status Register */

#define PHY_ES_1000X_FD       0x8000 /* able to perform full duplex 1000BASE-X */
#define PHY_ES_1000X_HD       0x4000 /* able to perform half duplex 1000BASE-X */
#define PHY_ES_1000T_FD       0x2000 /* able to perform full duplex 1000BASE-T */
#define PHY_ES_1000T_HD       0x1000 /* able to perform half duplex 1000BASE-T */

/* Speed/Duplex Mode to PHY_init() */

#define HD10    0x0000                            /*   10Mbps, half duplex */
#define HD100   PHY_CTRL_SPD_LSB                  /*  100Mbps, half duplex */
#define FD10    PHY_CTRL_DUPLEX                   /*   10Mbps, full duplex */
#define FD100  (PHY_CTRL_SPD_LSB|PHY_CTRL_DUPLEX) /*  100Mbps, full duplex */

/* Speed/Duplex Capabilities to PHY_init() */

#define AHD10   PHY_ANA_TAF_10T_H                 /*   10Mbps, half duplex */
#define AHD100  PHY_ANA_TAF_100TX_H               /*  100Mbps, half duplex */
#define AFD10   PHY_ANA_TAF_10T_F                 /*   10Mbps, full duplex */
#define AFD100  PHY_ANA_TAF_100TX_F               /*  100Mbps, full duplex */
#define AHD1000 PHY_ANA_RSV                       /* 1000Mbps, half duplex */
#define AFD1000 PHY_ANA_EX_NP                     /* 1000Mbps, full duplex */

/* Negotiation Result from PHY_pause() */

#define PAUSE_TX_RX      3
#define PAUSE_TX         2
#define PAUSE_RX         1
#define PAUSE_DISABLE    0

/* PHY Control Functions */

extern ER PHY_init(BOOL, int, int);
extern ER PHY_link(void);
extern ER PHY_pause(void);
extern UH PHY_read(UB);
extern void PHY_write(UB, UH);
extern void PHY_reset(void);

#ifdef __cplusplus
}
#endif
#endif /* __PHY__H__ */
