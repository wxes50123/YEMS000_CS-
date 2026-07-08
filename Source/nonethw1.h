/******************************************************************************
* Hardware Dependent Header for RX72M ETHERC 2nd Channel                      *
*                                                                             *
*  Copyright (c) 2020, MiSPO Co., Ltd.                                        *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-12-02 Copied nonethw.h, and changed ETH_CH 0->1 & PHYAD 0x01->0x02  FY *
******************************************************************************/

#ifndef NONETHW_H
#define NONETHW_H
#ifdef __cplusplus
extern "C" {
#endif

#define ETH_CH      1               /* ETHERC channel number (0~1) */
#define PHYAD       0x02            /* PHY address (0x00~0x1F) */

#include "nonelan.h"
#include "rx72m.h"
#include "eth72m.h"
#include "phymii.h"

/* PHY Interface Register of ETHERC1 is Used for ETHERC0 */

#undef ETHERC_PIR
#define ETHERC_PIR      ETHERC0_PIR

#ifdef __cplusplus
}
#endif
#endif /* NONETHW_H */
