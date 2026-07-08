/******************************************************************************
* Serial Driver Header for RX72M SCI                                          *
*                                                                             *
*  Copyright (c) 2020-2025, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Renesas RX72M                                       FY *
* 2020-05-15 Added missing ICU_IPRERI, ICU_GRPSCI and ICU_GENSCI of SCI7   FY *
* 2020-05-15 Added CLK to select CLKB for SCI0~6/SCI12 or CLKA for SCI7~11 FY *
* 2022-05-31 Corrected ERI_SHIFT of SCI11 from 8 to 12                     FY *
* 2022-08-30 Adjusted comment of registers as in RX72M user's manual       FY *
* 2022-08-30 Added SSR_RDRF/TDRE, SCMR_CHR1 & SEMR_BRME/ABCSE/BGDM/RXDESEL FY *
* 2024-07-10 Added sio_er_int and replaced cisr_sio_er with dinh_sio_er    FY *
* 2025-07-29 Renamed ICU_IPRSCI to ICU_IPRxxx redefining ICU_IPR058~116    FY *
* 2025-07-29 Renamed ICU_IPRERI to ICU_IPR11x redefining ICU_IPR110~112    FY *
* 2025-07-29 Renamed INT_ERI/ICU_GRPSCI/ICU_GENSCI to INT_GRPxL0/             *
*                         ICU_GRPxL0/ICU_GENxL0 for them of GRPAL0/GRPBL0  FY *
* 2025-07-29 Removed redefinition of staticized dinh_sio_rx/_tx/_er        FY *
******************************************************************************/

#ifndef NOS72M_H
#define NOS72M_H
#ifdef __cplusplus
extern "C" {
#endif

#include "rx72m.h"


#define TXD_P20	//ch0’è‹`’Ç‰Á
#define RXD_P21	

#define TXD_PF0	//ch1’è‹`’Ç‰Á
#define RXD_PF2	

#define TXD_P13	//ch2’è‹`’Ç‰Á
#define RXD_P12	


/*****************************************************************************/
/* Bit Definition of SCI Registers                                           */
/*****************************************************************************/

/* Serial Mode Register (SMR) */

#define SMR_CKS0    (1 << 0)  /* Clock Select 0 */
#define SMR_CKS1    (1 << 1)  /* Clock Select 1 */
#define SMR_MP      (1 << 2)  /* Multi-Processor Mode */
#define SMR_STOP    (1 << 3)  /* Stop Bit Length (0:1 stop bit, 1:2 stop bits) */
#define SMR_PM      (1 << 4)  /* Parity Mode (0:Even Parity, 1:Odd Parity) */
#define SMR_PE      (1 << 5)  /* Parity Enable */
#define SMR_CHR     (1 << 6)  /* Character Length (0:8 Bits, 1:7 Bits) */
#define SMR_CM      (1 << 7)  /* Communication Mode (0:Asynchronous, 1:Clock Synchronous) */

/* Serial Control Register (SCR) */

#define SCR_CKE0    (1 << 0)  /* Clock Enable 0 */
#define SCR_CKE1    (1 << 1)  /* Clock Enable 1 */
#define SCR_TEIE    (1 << 2)  /* Transmit End Interrupt Enable */
#define SCR_MPIE    (1 << 3)  /* Multi-Processor Interrupt Enable */
#define SCR_RE      (1 << 4)  /* Receive Enable */
#define SCR_TE      (1 << 5)  /* Transmit Enable */
#define SCR_RIE     (1 << 6)  /* Receive Interrupt Enable */
#define SCR_TIE     (1 << 7)  /* Transmit Interrupt Enable */

/* Serial Status Register (SSR) */

#define SSR_MPBT    (1 << 0)  /* Multi-Processor Bit Transfer */
#define SSR_MPB     (1 << 1)  /* Multi-Processor */
#define SSR_TEND    (1 << 2)  /* Transmit End Flag */
#define SSR_PER     (1 << 3)  /* Parity Error Flag */
#define SSR_FER     (1 << 4)  /* Framing Error Flag */
#define SSR_ORER    (1 << 5)  /* Overrun Error Flag */
#define SSR_RDRF    (1 << 6)  /* Receive Data Full Flag */
#define SSR_TDRE    (1 << 7)  /* Transmit Data Empty Flag */

/* Smart Card Mode Register (SCMR) */

#define SCMR_SMIF   (1 << 0)  /* Smart Card Interface Mode Select */
#define SCMR_SINV   (1 << 2)  /* Transmitted/Received Data Invert */
#define SCMR_SDIR   (1 << 3)  /* Transmitted/Received Data Transfer Direction */
#define SCMR_CHR1   (1 << 4)  /* Character Length 1 */
#define SCMR_BCP2   (1 << 7)  /* Base Clock Pulse 2 */

/* Serial Extended Mode Register (SEMR) */

#define SEMR_ACS0   (1 << 0)  /* Asynchronous Mode Clock Source Select */
#define SEMR_BRME   (1 << 2)  /* Bit Rate Modulation Enable */
#define SEMR_ABCSE  (1 << 3)  /* Asynchronous Mode Base Clock Select Extended */
#define SEMR_ABCS   (1 << 4)  /* Asynchronous Mode Base Clock Select */
#define SEMR_NFEN   (1 << 5)  /* Digital Noise Filter Function Enable */
#define SEMR_BGDM   (1 << 6)  /* Baud Rate Generator Double-Speed Mode Select */
#define SEMR_RXDESEL (1<< 7)  /* Asynchronous Start Bit Edge Detection Select */

/*****************************************************************************/
/* Redefinition of Registers according to CH0~CH12                           */
/*****************************************************************************/

/* SCI0 */

#ifdef CH0
#define SCI_SMR     SCI0_SMR
#define SCI_BRR     SCI0_BRR
#define SCI_SCR     SCI0_SCR
#define SCI_TDR     SCI0_TDR
#define SCI_SSR     SCI0_SSR
#define SCI_RDR     SCI0_RDR
#define SCI_SCMR    SCI0_SCMR
#define SCI_SEMR    SCI0_SEMR
#define ICU_IPRxxx  ICU_IPR058
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI0
#define INT_TXI     INT_SCI_TXI0
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   0
#if defined(TXD_P32)            /* when P32 is assigned to TXD0 */
#define MPC_TXDPFS  MPC_P32PFS
#define PSEL_TXD    0x0B
#define TXD_PMR     P3_PMR
#define TXD_PODR    P3_PODR
#define TXD_PDR     P3_PDR
#define TXD_BIT     0x04
#else                           /* when P20 is assigned to TXD0 */
#define MPC_TXDPFS  MPC_P20PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P2_PMR
#define TXD_PODR    P2_PODR
#define TXD_PDR     P2_PDR
#define TXD_BIT     0x01
#endif
#if defined(RXD_P33)            /* when P33 is assigned to RXD0 */
#define MPC_RXDPFS  MPC_P33PFS
#define PSEL_RXD    0x0B
#define RXD_PMR     P3_PMR
#define RXD_PIDR    P3_PIDR
#define RXD_BIT     0x08
#else                           /* when P21 is assigned to RXD0 */
#define MPC_RXDPFS  MPC_P21PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P2_PMR
#define RXD_PIDR    P2_PIDR
#define RXD_BIT     0x02
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x80000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI1 */

#ifdef CH1
#define SCI_SMR     SCI1_SMR
#define SCI_BRR     SCI1_BRR
#define SCI_SCR     SCI1_SCR
#define SCI_TDR     SCI1_TDR
#define SCI_SSR     SCI1_SSR
#define SCI_RDR     SCI1_RDR
#define SCI_SCMR    SCI1_SCMR
#define SCI_SEMR    SCI1_SEMR
#define ICU_IPRxxx  ICU_IPR060
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI1
#define INT_TXI     INT_SCI_TXI1
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   2
#if defined(TXD_PF0)            /* when PF0 is assigned to TXD1 */
#define MPC_TXDPFS  MPC_PF0PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PF_PMR
#define TXD_PODR    PF_PODR
#define TXD_PDR     PF_PDR
#define TXD_BIT     0x01
#elif defined(TXD_P26)          /* when P26 is assigned to TXD1 */
#define MPC_TXDPFS  MPC_P26PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P2_PMR
#define TXD_PODR    P2_PODR
#define TXD_PDR     P2_PDR
#define TXD_BIT     0x40
#else                           /* when P16 is assigned to TXD1 */
#define MPC_TXDPFS  MPC_P16PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P1_PMR
#define TXD_PODR    P1_PODR
#define TXD_PDR     P1_PDR
#define TXD_BIT     0x40
#endif
#if defined(RXD_PF2)            /* when PF2 is assigned to RXD1 */
#define MPC_RXDPFS  MPC_PF2PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PF_PMR
#define RXD_PIDR    PF_PIDR
#define RXD_BIT     0x04
#elif defined(RXD_P30)          /* when P30 is assigned to RXD1 */
#define MPC_RXDPFS  MPC_P30PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P3_PMR
#define RXD_PIDR    P3_PIDR
#define RXD_BIT     0x01
#else                           /* when P15 is assigned to RXD1 */
#define MPC_RXDPFS  MPC_P15PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P1_PMR
#define RXD_PIDR    P1_PIDR
#define RXD_BIT     0x20
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x40000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI2 */

#ifdef CH2
#define SCI_SMR     SCI2_SMR
#define SCI_BRR     SCI2_BRR
#define SCI_SCR     SCI2_SCR
#define SCI_TDR     SCI2_TDR
#define SCI_SSR     SCI2_SSR
#define SCI_RDR     SCI2_RDR
#define SCI_SCMR    SCI2_SCMR
#define SCI_SEMR    SCI2_SEMR
#define ICU_IPRxxx  ICU_IPR062
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI2
#define INT_TXI     INT_SCI_TXI2
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   4
#if defined(TXD_P50)            /* when P50 is assigned to TXD2 */
#define MPC_TXDPFS  MPC_P50PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P5_PMR
#define TXD_PODR    P5_PODR
#define TXD_PDR     P5_PDR
#define TXD_BIT     0x01
#else                           /* when P13 is assigned to TXD2 */
#define MPC_TXDPFS  MPC_P13PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P1_PMR
#define TXD_PODR    P1_PODR
#define TXD_PDR     P1_PDR
#define TXD_BIT     0x08
#endif
#if defined(RXD_P52)            /* when P52 is assigned to RXD2 */
#define MPC_RXDPFS  MPC_P52PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P5_PMR
#define RXD_PIDR    P5_PIDR
#define RXD_BIT     0x04
#else                           /* when P12 is assigned to RXD2 */
#define MPC_RXDPFS  MPC_P12PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P1_PMR
#define RXD_PIDR    P1_PIDR
#define RXD_BIT     0x04
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x20000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI3 */

#ifdef CH3
#define SCI_SMR     SCI3_SMR
#define SCI_BRR     SCI3_BRR
#define SCI_SCR     SCI3_SCR
#define SCI_TDR     SCI3_TDR
#define SCI_SSR     SCI3_SSR
#define SCI_RDR     SCI3_RDR
#define SCI_SCMR    SCI3_SCMR
#define SCI_SEMR    SCI3_SEMR
#define ICU_IPRxxx  ICU_IPR080
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI3
#define INT_TXI     INT_SCI_TXI3
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   6
#if defined(TXD_P23)            /* when P23 is assigned to TXD3 */
#define MPC_TXDPFS  MPC_P23PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P2_PMR
#define TXD_PODR    P2_PODR
#define TXD_PDR     P2_PDR
#define TXD_BIT     0x08
#else                           /* when P17 is assigned to TXD3 */
#define MPC_TXDPFS  MPC_P17PFS
#define PSEL_TXD    0x0B
#define TXD_PMR     P1_PMR
#define TXD_PODR    P1_PODR
#define TXD_PDR     P1_PDR
#define TXD_BIT     0x80
#endif
#if defined(RXD_P25)            /* when P25 is assigned to RXD3 */
#define MPC_RXDPFS  MPC_P25PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P2_PMR
#define RXD_PIDR    P2_PIDR
#define RXD_BIT     0x20
#else                           /* when P16 is assigned to RXD3 */
#define MPC_RXDPFS  MPC_P16PFS
#define PSEL_RXD    0x0B
#define RXD_PMR     P1_PMR
#define RXD_PIDR    P1_PIDR
#define RXD_BIT     0x40
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x10000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI4 */

#ifdef CH4
#define SCI_SMR     SCI4_SMR
#define SCI_BRR     SCI4_BRR
#define SCI_SCR     SCI4_SCR
#define SCI_TDR     SCI4_TDR
#define SCI_SSR     SCI4_SSR
#define SCI_RDR     SCI4_RDR
#define SCI_SCMR    SCI4_SCMR
#define SCI_SEMR    SCI4_SEMR
#define ICU_IPRxxx  ICU_IPR082
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI4
#define INT_TXI     INT_SCI_TXI4
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   8
#define MPC_TXDPFS  MPC_PB1PFS  /* assign PB1 to TXD4 */
#define PSEL_TXD    0x0A
#define TXD_PMR     PB_PMR
#define TXD_PODR    PB_PODR
#define TXD_PDR     PB_PDR
#define TXD_BIT     0x02
#define MPC_RXDPFS  MPC_PB0PFS  /* assign PB0 to RXD4 */
#define PSEL_RXD    0x0A
#define RXD_PMR     PB_PMR
#define RXD_PIDR    PB_PIDR
#define RXD_BIT     0x01
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x08000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI5 */

#ifdef CH5
#define SCI_SMR     SCI5_SMR
#define SCI_BRR     SCI5_BRR
#define SCI_SCR     SCI5_SCR
#define SCI_TDR     SCI5_TDR
#define SCI_SSR     SCI5_SSR
#define SCI_RDR     SCI5_RDR
#define SCI_SCMR    SCI5_SCMR
#define SCI_SEMR    SCI5_SEMR
#define ICU_IPRxxx  ICU_IPR084
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI5
#define INT_TXI     INT_SCI_TXI5
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   10
#if defined(TXD_PC3)            /* when PC3 is assigned to TXD5 */
#define MPC_TXDPFS  MPC_PC3PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PC_PMR
#define TXD_PODR    PC_PODR
#define TXD_PDR     PC_PDR
#define TXD_BIT     0x08
#else                           /* when PA4 is assigned to TXD5 */
#define MPC_TXDPFS  MPC_PA4PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PA_PMR
#define TXD_PODR    PA_PODR
#define TXD_PDR     PA_PDR
#define TXD_BIT     0x10
#endif
#if defined(RXD_PC2)            /* when PC2 is assigned to RXD5 */
#define MPC_RXDPFS  MPC_PC2PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PC_PMR
#define RXD_PIDR    PC_PIDR
#define RXD_BIT     0x04
#elif defined(RXD_PA3)          /* when PA3 is assigned to RXD5 */
#define MPC_RXDPFS  MPC_PA3PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PA_PMR
#define RXD_PIDR    PA_PIDR
#define RXD_BIT     0x08
#else                           /* when PA2 is assigned to RXD5 */
#define MPC_RXDPFS  MPC_PA2PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PA_PMR
#define RXD_PIDR    PA_PIDR
#define RXD_BIT     0x04
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x04000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI6 */

#ifdef CH6
#define SCI_SMR     SCI6_SMR
#define SCI_BRR     SCI6_BRR
#define SCI_SCR     SCI6_SCR
#define SCI_TDR     SCI6_TDR
#define SCI_SSR     SCI6_SSR
#define SCI_RDR     SCI6_RDR
#define SCI_SCMR    SCI6_SCMR
#define SCI_SEMR    SCI6_SEMR
#define ICU_IPRxxx  ICU_IPR086
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI6
#define INT_TXI     INT_SCI_TXI6
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   12
#if defined(TXD_PB1)            /* when PB1 is assigned to TXD6 */
#define MPC_TXDPFS  MPC_PB1PFS
#define PSEL_TXD    0x0B
#define TXD_PMR     PB_PMR
#define TXD_PODR    PB_PODR
#define TXD_PDR     PB_PDR
#define TXD_BIT     0x02
#elif defined(TXD_P32)          /* when P32 is assigned to TXD6 */
#define MPC_TXDPFS  MPC_P32PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P3_PMR
#define TXD_PODR    P3_PODR
#define TXD_PDR     P3_PDR
#define TXD_BIT     0x04
#else                           /* when P00 is assigned to TXD6 */
#define MPC_TXDPFS  MPC_P00PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P0_PMR
#define TXD_PODR    P0_PODR
#define TXD_PDR     P0_PDR
#define TXD_BIT     0x01
#endif
#if defined(RXD_PB0)            /* when PB0 is assigned to RXD6 */
#define MPC_RXDPFS  MPC_PB0PFS
#define PSEL_RXD    0x0B
#define RXD_PMR     PB_PMR
#define RXD_PIDR    PB_PIDR
#define RXD_BIT     0x01
#elif defined(RXD_P33)          /* when P33 is assigned to RXD6 */
#define MPC_RXDPFS  MPC_P33PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P3_PMR
#define RXD_PIDR    P3_PIDR
#define RXD_BIT     0x08
#else                           /* when P01 is assigned to RXD6 */
#define MPC_RXDPFS  MPC_P01PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P0_PMR
#define RXD_PIDR    P0_PIDR
#define RXD_BIT     0x02
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x02000000
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/* SCI7 */

#ifdef CH7
#define SCI_SMR     SCI7_SMR
#define SCI_BRR     SCI7_BRR
#define SCI_SCR     SCI7_SCR
#define SCI_TDR     SCI7_TDR
#define SCI_SSR     SCI7_SSR
#define SCI_RDR     SCI7_RDR
#define SCI_SCMR    SCI7_SCMR
#define SCI_SEMR    SCI7_SEMR
#define ICU_IPRxxx  ICU_IPR098
#define ICU_IPR11x  ICU_IPR112
#define ICU_GRPxL0  ICU_GRPAL0
#define ICU_GENxL0  ICU_GENAL0
#define INT_RXI     INT_SCI_RXI7
#define INT_TXI     INT_SCI_TXI7
#define INT_GRPxL0  INT_GRPAL0
#define ERI_SHIFT   22
#if defined(TXD_PH2)            /* when PH2 is assigned to TXD7 */
#define MPC_TXDPFS  MPC_PH2PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PH_PMR
#define TXD_PODR    PH_PODR
#define TXD_PDR     PH_PDR
#define TXD_BIT     0x04
#elif defined(TXD_P90)          /* when P90 is assigned to TXD7 */
#define MPC_TXDPFS  MPC_P90PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P9_PMR
#define TXD_PODR    P9_PODR
#define TXD_PDR     P9_PDR
#define TXD_BIT     0x01
#else                           /* when P55 is assigned to TXD7 */
#define MPC_TXDPFS  MPC_P55PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P5_PMR
#define TXD_PODR    P5_PODR
#define TXD_PDR     P5_PDR
#define TXD_BIT     0x20
#endif
#if defined(RXD_PH1)            /* when PH1 is assigned to RXD7 */
#define MPC_RXDPFS  MPC_PH1PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PH_PMR
#define RXD_PIDR    PH_PIDR
#define RXD_BIT     0x02
#elif defined(RXD_P92)          /* when P92 is assigned to RXD7 */
#define MPC_RXDPFS  MPC_P92PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P9_PMR
#define RXD_PIDR    P9_PIDR
#define RXD_BIT     0x04
#else                           /* when P57 is assigned to RXD7 */
#define MPC_RXDPFS  MPC_P57PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P5_PMR
#define RXD_PIDR    P5_PIDR
#define RXD_BIT     0x80
#endif
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x01000000
#ifndef CLK
#define CLK         CLKA
#endif
#endif

/* SCI8 */

#ifdef CH8
#define SCI_SMR     SCI8_SMR
#define SCI_BRR     SCI8_BRR
#define SCI_SCR     SCI8_SCR
#define SCI_TDR     SCI8_TDR
#define SCI_SSR     SCI8_SSR
#define SCI_RDR     SCI8_RDR
#define SCI_SCMR    SCI8_SCMR
#define SCI_SEMR    SCI8_SEMR
#define ICU_IPRxxx  ICU_IPR100
#define ICU_IPR11x  ICU_IPR112
#define ICU_GRPxL0  ICU_GRPAL0
#define ICU_GENxL0  ICU_GENAL0
#define INT_RXI     INT_SCI_RXI8
#define INT_TXI     INT_SCI_TXI8
#define INT_GRPxL0  INT_GRPAL0
#define ERI_SHIFT   0
#if defined(TXD_PK2)            /* when PK2 is assigned to TXD8 */
#define MPC_TXDPFS  MPC_PK2PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PK_PMR
#define TXD_PODR    PK_PODR
#define TXD_PDR     PK_PDR
#define TXD_BIT     0x04
#elif defined(TXD_PJ2)          /* when PJ2 is assigned to TXD8 */
#define MPC_TXDPFS  MPC_PJ2PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PJ_PMR
#define TXD_PODR    PJ_PODR
#define TXD_PDR     PJ_PDR
#define TXD_BIT     0x04
#else                           /* when PC7 is assigned to TXD8 */
#define MPC_TXDPFS  MPC_PC7PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PC_PMR
#define TXD_PODR    PC_PODR
#define TXD_PDR     PC_PDR
#define TXD_BIT     0x80
#endif
#if defined(RXD_PK1)            /* when PK1 is assigned to RXD8 */
#define MPC_RXDPFS  MPC_PK1PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PK_PMR
#define RXD_PIDR    PK_PIDR
#define RXD_BIT     0x02
#elif defined(RXD_PJ1)          /* when PJ1 is assigned to RXD8 */
#define MPC_RXDPFS  MPC_PJ1PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PJ_PMR
#define RXD_PIDR    PJ_PIDR
#define RXD_BIT     0x02
#else                           /* when PC6 is assigned to RXD8 */
#define MPC_RXDPFS  MPC_PC6PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PC_PMR
#define RXD_PIDR    PC_PIDR
#define RXD_BIT     0x40
#endif
#define LPC_MSTPCR  LPC_MSTPCRC
#define MSTP_BIT    0x08000000
#ifndef CLK
#define CLK         CLKA
#endif
#endif

/* SCI9 */

#ifdef CH9
#define SCI_SMR     SCI9_SMR
#define SCI_BRR     SCI9_BRR
#define SCI_SCR     SCI9_SCR
#define SCI_TDR     SCI9_TDR
#define SCI_SSR     SCI9_SSR
#define SCI_RDR     SCI9_RDR
#define SCI_SCMR    SCI9_SCMR
#define SCI_SEMR    SCI9_SEMR
#define SCI_SPMR    SCI9_SPMR
#define ICU_IPRxxx  ICU_IPR102
#define ICU_IPR11x  ICU_IPR112
#define ICU_GRPxL0  ICU_GRPAL0
#define ICU_GENxL0  ICU_GENAL0
#define INT_RXI     INT_SCI_RXI9
#define INT_TXI     INT_SCI_TXI9
#define INT_GRPxL0  INT_GRPAL0
#define ERI_SHIFT   4
#if defined(TXD_PL2)            /* when PL2 is assigned to TXD9 */
#define MPC_TXDPFS  MPC_PL2PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PL_PMR
#define TXD_PODR    PL_PODR
#define TXD_PDR     PL_PDR
#define TXD_BIT     0x04
#else                           /* when PB7 is assigned to TXD9 */
#define MPC_TXDPFS  MPC_PB7PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PB_PMR
#define TXD_PODR    PB_PODR
#define TXD_PDR     PB_PDR
#define TXD_BIT     0x80
#endif
#if defined(RXD_PL1)            /* when PL1 is assigned to RXD9 */
#define MPC_RXDPFS  MPC_PL1PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PL_PMR
#define RXD_PIDR    PL_PIDR
#define RXD_BIT     0x02
#else                           /* when PB6 is assigned to RXD9 */
#define MPC_RXDPFS  MPC_PB6PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PB_PMR
#define RXD_PIDR    PB_PIDR
#define RXD_BIT     0x40
#endif
#define LPC_MSTPCR  LPC_MSTPCRC
#define MSTP_BIT    0x04000000
#ifndef CLK
#define CLK         CLKA
#endif
#endif

/* SCI10 */

#ifdef CH10
#define SCI_SMR     SCI10_SMR
#define SCI_BRR     SCI10_BRR
#define SCI_SCR     SCI10_SCR
#define SCI_TDR     SCI10_TDR
#define SCI_SSR     SCI10_SSR
#define SCI_RDR     SCI10_RDR
#define SCI_SCMR    SCI10_SCMR
#define SCI_SEMR    SCI10_SEMR
#define SCI_SPMR    SCI10_SPMR
#define ICU_IPRxxx  ICU_IPR104
#define ICU_IPR11x  ICU_IPR112
#define ICU_GRPxL0  ICU_GRPAL0
#define ICU_GENxL0  ICU_GENAL0
#define INT_RXI     INT_SCI_RXI10
#define INT_TXI     INT_SCI_TXI10
#define INT_GRPxL0  INT_GRPAL0
#define ERI_SHIFT   8
#if defined(TXD_PM2)            /* when PM2 is assigned to TXD10 */
#define MPC_TXDPFS  MPC_PM2PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PM_PMR
#define TXD_PODR    PM_PODR
#define TXD_PDR     PM_PDR
#define TXD_BIT     0x04
#elif defined(TXD_PC7)          /* when PC7 is assigned to TXD10 */
#define MPC_TXDPFS  MPC_PC7PFS
#define PSEL_TXD    0x24
#define TXD_PMR     PC_PMR
#define TXD_PODR    PC_PODR
#define TXD_PDR     PC_PDR
#define TXD_BIT     0x80
#elif defined(TXD_P87)          /* when P87 is assigned to TXD10 */
#define MPC_TXDPFS  MPC_P87PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P8_PMR
#define TXD_PODR    P8_PODR
#define TXD_PDR     P8_PDR
#define TXD_BIT     0x80
#else                           /* when P82 is assigned to TXD10 */
#define MPC_TXDPFS  MPC_P82PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P8_PMR
#define TXD_PODR    P8_PODR
#define TXD_PDR     P8_PDR
#define TXD_BIT     0x04
#endif
#if defined(RXD_PM1)            /* when PM1 is assigned to RXD10 */
#define MPC_RXDPFS  MPC_PM1PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PM_PMR
#define RXD_PIDR    PM_PIDR
#define RXD_BIT     0x02
#elif defined(RXD_PC6)          /* when PC6 is assigned to RXD10 */
#define MPC_RXDPFS  MPC_PC6PFS
#define PSEL_RXD    0x24
#define RXD_PMR     PC_PMR
#define RXD_PIDR    PC_PIDR
#define RXD_BIT     0x40
#elif defined(RXD_P86)          /* when P86 is assigned to RXD10 */
#define MPC_RXDPFS  MPC_P86PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P8_PMR
#define RXD_PIDR    P8_PIDR
#define RXD_BIT     0x40
#else                           /* when P81 is assigned to RXD10 */
#define MPC_RXDPFS  MPC_P81PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P8_PMR
#define RXD_PIDR    P8_PIDR
#define RXD_BIT     0x02
#endif
#define LPC_MSTPCR  LPC_MSTPCRC
#define MSTP_BIT    0x02000000
#ifndef CLK
#define CLK         CLKA
#endif
#endif

/* SCI11 */

#ifdef CH11
#define SCI_SMR     SCI11_SMR
#define SCI_BRR     SCI11_BRR
#define SCI_SCR     SCI11_SCR
#define SCI_TDR     SCI11_TDR
#define SCI_SSR     SCI11_SSR
#define SCI_RDR     SCI11_RDR
#define SCI_SCMR    SCI11_SCMR
#define SCI_SEMR    SCI11_SEMR
#define SCI_SPMR    SCI11_SPMR
#define ICU_IPRxxx  ICU_IPR114
#define ICU_IPR11x  ICU_IPR112
#define ICU_GRPxL0  ICU_GRPAL0
#define ICU_GENxL0  ICU_GENAL0
#define INT_RXI     INT_SCI_RXI11
#define INT_TXI     INT_SCI_TXI11
#define INT_GRPxL0  INT_GRPAL0
#define ERI_SHIFT   12
#if defined(TXD_PQ2)            /* when PQ2 is assigned to TXD11 */
#define MPC_TXDPFS  MPC_PQ2PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     PQ_PMR
#define TXD_PODR    PQ_PODR
#define TXD_PDR     PQ_PDR
#define TXD_BIT     0x04
#elif defined(TXD_PB7)          /* when PB7 is assigned to TXD11 */
#define MPC_TXDPFS  MPC_PB7PFS
#define PSEL_TXD    0x24
#define TXD_PMR     PB_PMR
#define TXD_PODR    PB_PODR
#define TXD_PDR     PB_PDR
#define TXD_BIT     0x80
#else                           /* when P77 is assigned to TXD11 */
#define MPC_TXDPFS  MPC_P77PFS
#define PSEL_TXD    0x0A
#define TXD_PMR     P7_PMR
#define TXD_PODR    P7_PODR
#define TXD_PDR     P7_PDR
#define TXD_BIT     0x80
#endif
#if defined(RXD_PQ1)            /* when PQ1 is assigned to RXD11 */
#define MPC_RXDPFS  MPC_PQ1PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     PQ_PMR
#define RXD_PIDR    PQ_PIDR
#define RXD_BIT     0x02
#elif defined(RXD_PB6)          /* when PB6 is assigned to RXD11 */
#define MPC_RXDPFS  MPC_PB6PFS
#define PSEL_RXD    0x24
#define RXD_PMR     PB_PMR
#define RXD_PIDR    PB_PIDR
#define RXD_BIT     0x40
#else                           /* when P76 is assigned to RXD11 */
#define MPC_RXDPFS  MPC_P76PFS
#define PSEL_RXD    0x0A
#define RXD_PMR     P7_PMR
#define RXD_PIDR    P7_PIDR
#define RXD_BIT     0x40
#endif
#define LPC_MSTPCR  LPC_MSTPCRC
#define MSTP_BIT    0x01000000
#ifndef CLK
#define CLK         CLKA
#endif
#endif

/* SCI12 */

#ifdef CH12
#define SCI_SMR     SCI12_SMR
#define SCI_BRR     SCI12_BRR
#define SCI_SCR     SCI12_SCR
#define SCI_TDR     SCI12_TDR
#define SCI_SSR     SCI12_SSR
#define SCI_RDR     SCI12_RDR
#define SCI_SCMR    SCI12_SCMR
#define SCI_SEMR    SCI12_SEMR
#define ICU_IPRxxx  ICU_IPR116
#define ICU_IPR11x  ICU_IPR110
#define ICU_GRPxL0  ICU_GRPBL0
#define ICU_GENxL0  ICU_GENBL0
#define INT_RXI     INT_SCI_RXI12
#define INT_TXI     INT_SCI_TXI12
#define INT_GRPxL0  INT_GRPBL0
#define ERI_SHIFT   16
#define MPC_TXDPFS  MPC_PE1PFS  /* assign PE1 to TXD12 */
#define PSEL_TXD    0x0C
#define TXD_PMR     PE_PMR
#define TXD_PODR    PE_PODR
#define TXD_PDR     PE_PDR
#define TXD_BIT     0x02
#define MPC_RXDPFS  MPC_PE2PFS  /* assign PE2 to RXD12 */
#define PSEL_RXD    0x0C
#define RXD_PMR     PE_PMR
#define RXD_PIDR    PE_PIDR
#define RXD_BIT     0x04
#define LPC_MSTPCR  LPC_MSTPCRB
#define MSTP_BIT    0x00000010
#ifndef CLK
#define CLK         CLKB
#endif
#endif

/*****************************************************************************/
/* Redefinition of Functions according to CH                                 */
/*****************************************************************************/

#ifdef CH
#if (CH == 0)
#define ini_sio         ini_sio0
#define ext_sio         ext_sio0
#define get_sio         get_sio0
#define put_sio         put_sio0
#define ctl_sio         ctl_sio0
#define ref_sio         ref_sio0
#define fls_sio         fls_sio0
#define int_sio_rx      int_sio0_rx
#define int_sio_tx      int_sio0_tx
#define int_sio_er      int_sio0_er
#define sio_er_int      sio0_er_int
#elif (CH == 1)
#define ini_sio         ini_sio1
#define ext_sio         ext_sio1
#define get_sio         get_sio1
#define put_sio         put_sio1
#define ctl_sio         ctl_sio1
#define ref_sio         ref_sio1
#define fls_sio         fls_sio1
#define int_sio_rx      int_sio1_rx
#define int_sio_tx      int_sio1_tx
#define int_sio_er      int_sio1_er
#define sio_er_int      sio1_er_int
#elif (CH == 2)
#define ini_sio         ini_sio2
#define ext_sio         ext_sio2
#define get_sio         get_sio2
#define put_sio         put_sio2
#define ctl_sio         ctl_sio2
#define ref_sio         ref_sio2
#define fls_sio         fls_sio2
#define int_sio_rx      int_sio2_rx
#define int_sio_tx      int_sio2_tx
#define int_sio_er      int_sio2_er
#define sio_er_int      sio2_er_int
#elif (CH == 3)
#define ini_sio         ini_sio3
#define ext_sio         ext_sio3
#define get_sio         get_sio3
#define put_sio         put_sio3
#define ctl_sio         ctl_sio3
#define ref_sio         ref_sio3
#define fls_sio         fls_sio3
#define int_sio_rx      int_sio3_rx
#define int_sio_tx      int_sio3_tx
#define int_sio_er      int_sio3_er
#define sio_er_int      sio3_er_int
#elif (CH == 4)
#define ini_sio         ini_sio4
#define ext_sio         ext_sio4
#define get_sio         get_sio4
#define put_sio         put_sio4
#define ctl_sio         ctl_sio4
#define ref_sio         ref_sio4
#define fls_sio         fls_sio4
#define int_sio_rx      int_sio4_rx
#define int_sio_tx      int_sio4_tx
#define int_sio_er      int_sio4_er
#define sio_er_int      sio4_er_int
#elif (CH == 5)
#define ini_sio         ini_sio5
#define ext_sio         ext_sio5
#define get_sio         get_sio5
#define put_sio         put_sio5
#define ctl_sio         ctl_sio5
#define ref_sio         ref_sio5
#define fls_sio         fls_sio5
#define int_sio_rx      int_sio5_rx
#define int_sio_tx      int_sio5_tx
#define int_sio_er      int_sio5_er
#define sio_er_int      sio5_er_int
#elif (CH == 6)
#define ini_sio         ini_sio6
#define ext_sio         ext_sio6
#define get_sio         get_sio6
#define put_sio         put_sio6
#define ctl_sio         ctl_sio6
#define ref_sio         ref_sio6
#define fls_sio         fls_sio6
#define int_sio_rx      int_sio6_rx
#define int_sio_tx      int_sio6_tx
#define int_sio_er      int_sio6_er
#define sio_er_int      sio6_er_int
#elif (CH == 7)
#define ini_sio         ini_sio7
#define ext_sio         ext_sio7
#define get_sio         get_sio7
#define put_sio         put_sio7
#define ctl_sio         ctl_sio7
#define ref_sio         ref_sio7
#define fls_sio         fls_sio7
#define int_sio_rx      int_sio7_rx
#define int_sio_tx      int_sio7_tx
#define int_sio_er      int_sio7_er
#define sio_er_int      sio7_er_int
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif /* NOS72M_H */
