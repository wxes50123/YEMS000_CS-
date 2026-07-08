/******************************************************************************
* Interval Timer and Interrupt Management for RX72M                           *
*                                                                             *
*  Copyright (c) 2020-2024, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Renesas RX72M                                       FY *
* 2022-06-17 Added code to copy FIT module's vector table on ROM to           *
*                           intvec[] on RAM when USE_FIT macro is defined  FY *
* 2022-06-17 Added initialization of intvec[0..15] of unconditional trap   FY *
* 2022-06-17 Added condition of __sectop("C_1") in _kernel_inROM() for FIT FY *
* 2022-08-30 Modified TC0~TC3 rounding-off to accept 2x MSEC (35->71)      FY *
* 2024-07-10 Changed intini() to skip all IPR clear when USE_FIT defined   FY *
* 2024-11-26 Corrected tickck_read() not to return time based on uncounted    *
*             TOTAL_TICKCK when called to hook ent_int() of interval timer FY *
* 2024-11-26 Added missing writing 1 to indefinite bit of CMCR             FY *
* 2024-11-26 Reduced default interrupt vector table size INTVSZ from 1024     *
*              to 512 of case of un-using software configurable interrupts FY *
* 2024-11-26 Added exception vector table excvec[] on RAM when EXCVSZ != 0 FY *
******************************************************************************/

#include <machine.h>				//割込みレベル操作用
#include "kernel.h"
#include "rx72m.h"
#include "g_include_def.h"

/* Customization Macros */

#ifndef CLK                         /* peripheral clock (PCLKB) frequency */
#define CLK         60000000        /* 60 MHz by default */
#endif

#ifndef IP                          /* timer interrupt priority */
#define IP          6               /* 1 (lowest) ~ 15 (highest) */
#endif

#if (defined(HOOK) || !defined(CH)) /* timer channel number */
#define CH          0               /* 0 ~ 1 */
#endif                              /* both timers are used when HOOK defined */

#ifndef INTVSZ                      /* size of interrupt vector table on RAM */
#define INTVSZ      512             /* 0 or 512 bytes (up to 1024 including */
#endif                              /*    software configurable interrupts) */

#ifndef EXCVSZ                      /* size of exception vector table on RAM */
#define EXCVSZ      0               /* 0 or 44 bytes (up to 124 including */
#endif                              /*        reserved exception vectors) */

/* Redefinition of Registers according to Channel */

#if (CH == 0)
#define CMT_CMCR    CMT0_CMCR
#define CMT_CMCNT   CMT0_CMCNT
#define CMT_CMCOR   CMT0_CMCOR
#define ICU_CMT_IPR ICU_IPR004
#define ICU_CMI_IR  ICU_IR028
#define INT_CMT     INT_CMI0
#elif (CH == 1)
#define CMT_CMCR    CMT1_CMCR
#define CMT_CMCNT   CMT1_CMCNT
#define CMT_CMCOR   CMT1_CMCOR
#define ICU_CMT_IPR ICU_IPR005
#define ICU_CMI_IR  ICU_IR029
#define INT_CMT     INT_CMI1
#endif

/* External Function */

extern void _kernel_interr(void);

/* Relocatable Vector Table */

#ifdef INTVEC                       /* when INTVEC is defined, */
UW * const intvec = (UW *)INTVEC;   /* use it as vector table address */
#elif (INTVSZ == 0)                 /* when INTVSZ is zero, */
extern const UW * const ROM_VEC[];  /* refer vector table on ROM */
const UW * const * const intvec = ROM_VEC;
#else                               /* (must be not zero on USE_FIT) */
UW intvec[INTVSZ/4];                /* place interrupt vector table on RAM */
#ifdef USE_FIT
extern void undefined_interrupt_source_isr(void);
#endif                              /* unused interrupt vector of FIT module */
#endif

#if (EXCVSZ != 0)                   /* (must be zero on USE_FIT) */
UW excvec[EXCVSZ/4];                /* place exception vector table on RAM */
#endif

/* Tick Counter and Constant */

volatile UW TOTAL_TICKCK;           /* system time in milliseconds x 1000 */

#define TC0 ((1UL*MSEC*CLK+  8000/2)/  8000) /* +8000/2 for rounding-off */
#define TC1 ((1UL*MSEC*CLK+ 32000/2)/ 32000)
#define TC2 ((1UL*MSEC*CLK+128000/2)/128000)
#define TC3 ((1UL*MSEC*CLK+512000/2)/512000) /* (note) overflow if MSEC > 71 */

#ifdef HOOK                         /* when HOOK defined, */
#define LIMIT 0x8000                /* limit to half to prevent CMT1 from */
#else                               /* clearing before accepting CMT0 interrupt */
#define LIMIT 0x10000
#endif
#if (TC0 <= LIMIT)
#define CKS 0                       /* clock select */
#define TC  TC0                     /* time constant */
#elif (TC1 <= LIMIT)
#define CKS 1
#define TC  TC1
#elif (TC2 <= LIMIT)
#define CKS 2
#define TC  TC2
#elif (TC3 <= LIMIT)
#define CKS 3
#define TC  TC3
#else
#error CLK or MSEC is out of range!
#endif

/* Interrupt Service Routine Creation Information */

#if (INTVSZ != 0) /* when vector table is on RAM */
void isritm(VP_INT);
const T_CISR cisr_isritm = { TA_HLNG, NULL, INT_CMT, isritm, 0 };
#endif

/* Interrupt Source Priority Register */

#ifndef USE_FIT
static const UW icu_ipr[] = {
    ICU_IPR000, ICU_IPR001, ICU_IPR002, ICU_IPR003, ICU_IPR004, ICU_IPR005, ICU_IPR006, ICU_IPR007,
    ICU_IPR034, ICU_IPR035, ICU_IPR038, ICU_IPR039, ICU_IPR040, ICU_IPR041, ICU_IPR042, ICU_IPR043,
    ICU_IPR044, ICU_IPR045, ICU_IPR046, ICU_IPR047, ICU_IPR048, ICU_IPR050, ICU_IPR051, ICU_IPR052,
    ICU_IPR053, ICU_IPR054, ICU_IPR055, ICU_IPR058, ICU_IPR059, ICU_IPR060, ICU_IPR061, ICU_IPR062,
    ICU_IPR063, ICU_IPR064, ICU_IPR065, ICU_IPR066, ICU_IPR067, ICU_IPR068, ICU_IPR069, ICU_IPR070,
    ICU_IPR071, ICU_IPR072, ICU_IPR073, ICU_IPR074, ICU_IPR075, ICU_IPR076, ICU_IPR077, ICU_IPR078,
    ICU_IPR079, ICU_IPR080, ICU_IPR081, ICU_IPR082, ICU_IPR083, ICU_IPR084, ICU_IPR085, ICU_IPR086,
    ICU_IPR087, ICU_IPR088, ICU_IPR089, ICU_IPR090, ICU_IPR092, ICU_IPR093, ICU_IPR095, ICU_IPR096,
    ICU_IPR097, ICU_IPR098, ICU_IPR099, ICU_IPR100, ICU_IPR101, ICU_IPR102, ICU_IPR103, ICU_IPR104,
    ICU_IPR105, ICU_IPR106, ICU_IPR107, ICU_IPR108, ICU_IPR109, ICU_IPR110, ICU_IPR111, ICU_IPR112,
    ICU_IPR113, ICU_IPR114, ICU_IPR115, ICU_IPR116, ICU_IPR117, ICU_IPR120, ICU_IPR121, ICU_IPR122,
    ICU_IPR123, ICU_IPR124, ICU_IPR125, ICU_IPR126, ICU_IPR127, ICU_IPR128, ICU_IPR129, ICU_IPR130,
    ICU_IPR131, ICU_IPR132, ICU_IPR133, ICU_IPR134, ICU_IPR135, ICU_IPR136, ICU_IPR137, ICU_IPR138,
    ICU_IPR139, ICU_IPR140, ICU_IPR141, ICU_IPR142, ICU_IPR143, ICU_IPR144, ICU_IPR145, ICU_IPR146,
    ICU_IPR147, ICU_IPR148, ICU_IPR149, ICU_IPR150, ICU_IPR151, ICU_IPR152, ICU_IPR153, ICU_IPR154,
    ICU_IPR155, ICU_IPR156, ICU_IPR157, ICU_IPR158, ICU_IPR159, ICU_IPR160, ICU_IPR161, ICU_IPR162,
    ICU_IPR163, ICU_IPR164, ICU_IPR165, ICU_IPR166, ICU_IPR167, ICU_IPR168, ICU_IPR169, ICU_IPR170,
    ICU_IPR171, ICU_IPR172, ICU_IPR173, ICU_IPR174, ICU_IPR175, ICU_IPR176, ICU_IPR177, ICU_IPR178,
    ICU_IPR179, ICU_IPR180, ICU_IPR181, ICU_IPR182, ICU_IPR183, ICU_IPR184, ICU_IPR185, ICU_IPR186,
    ICU_IPR187, ICU_IPR188, ICU_IPR189, ICU_IPR190, ICU_IPR191, ICU_IPR192, ICU_IPR193, ICU_IPR194,
    ICU_IPR195, ICU_IPR196, ICU_IPR197, ICU_IPR198, ICU_IPR199, ICU_IPR200, ICU_IPR201, ICU_IPR202,
    ICU_IPR203, ICU_IPR204, ICU_IPR205, ICU_IPR206, ICU_IPR207, ICU_IPR208, ICU_IPR209, ICU_IPR210,
    ICU_IPR211, ICU_IPR212, ICU_IPR213, ICU_IPR214, ICU_IPR215, ICU_IPR216, ICU_IPR217, ICU_IPR218,
    ICU_IPR219, ICU_IPR220, ICU_IPR221, ICU_IPR222, ICU_IPR223, ICU_IPR224, ICU_IPR225, ICU_IPR226,
    ICU_IPR227, ICU_IPR228, ICU_IPR229, ICU_IPR230, ICU_IPR231, ICU_IPR232, ICU_IPR233, ICU_IPR234,
    ICU_IPR235, ICU_IPR236, ICU_IPR237, ICU_IPR238, ICU_IPR239, ICU_IPR240, ICU_IPR241, ICU_IPR242,
    ICU_IPR243, ICU_IPR244, ICU_IPR245, ICU_IPR246, ICU_IPR247, ICU_IPR248, ICU_IPR249, ICU_IPR250,
    ICU_IPR251, ICU_IPR252, ICU_IPR253, ICU_IPR254, ICU_IPR255
};
#endif

/*****************************************************************************
* Initialize Interrupt Management - System Call (NORTi unique)
*
******************************************************************************/

ER intini(void)
{
    /* This function is called by sysini() and sets up interrupt management
       controls under the assumption that each interrupt request is not
       activated yet. All interrupts are set to the lowest priority level. */

  #if (defined(USE_FIT) || (EXCVSZ != 0))
    UW *p;
  #endif
  #if (!defined(USE_FIT) || (INTVSZ != 0) || (EXCVSZ != 0))
    int i;
  #endif

    /* disable all interrupts */
  #ifndef USE_FIT
    for (i = 0; i < sizeof icu_ipr / sizeof (UW); i++)
        sfr_out(icu_ipr[i], 0);
  #endif

    /* initialize interrupt vector table on RAM */
  #if (INTVSZ != 0)
    for (i = 0; i < INTVSZ / 4; i++)
        intvec[i] = (UW)_kernel_interr;

   /* copy interrupt vectors of FIT module from ROM */
   #ifdef USE_FIT
    p = (UW *)__sectop("C$VECT");
    for (i = 0; i < INTVSZ / 4; i++) {
        if (p[i] != (UW)undefined_interrupt_source_isr)
            intvec[i] = p[i];
    }
   #endif
  #endif

    /* set interrupt table register */
    set_intb((void *)intvec);

    /* copy exception vectors on ROM */
  #if (EXCVSZ != 0)
    p = (UW *)(0xFFFFFF80 + 124 - EXCVSZ);
    for (i = 0; i < EXCVSZ / 4; i++)
        excvec[i] = p[i];
    set_extb(excvec + i - 124 / 4);
  #endif

    TOTAL_TICKCK = 0;
    return E_OK;
}

/*****************************************************************************
* Check whether Region is in ROM
*
******************************************************************************/

BOOL _kernel_inROM(const void *p)
{
    /* This function is called by kernel to check whether the region pointed
       by p is in ROM (code or constant section).
       Returns TRUE when the region is in ROM, and returns FALSE when not in
       ROM. */

    return (UW)p >= (UW)__sectop("C_1") || (UW)p >= (UW)__sectop("P");
}

/*****************************************************************************
* Check Interrupt Service Routine Creation Information
*
******************************************************************************/

ER _kernel_check_cisr(const T_CISR *pk_cisr)
{
    /* This function is called by kernel to check whether the information
       pointed by pk_cisr is proper.
       Returns E_OK when the interrupt number and priority is within the range,
       and returns E_PAR if not. */

  #if (INTVSZ == 0) /* when vector table is on ROM, */
    return E_PAR;   /* interrupt service routine cannot be created */
  #else
    if (pk_cisr->intno >= INTVSZ / 4)
        return E_PAR;
    return E_OK;
  #endif
}

/*****************************************************************************
* Set Interrupt Handler Address in Interrupt Vector Table
*
******************************************************************************/

void _kernel_set_inthdr(FP hdr, const T_CISR *pk_cisr)
{
    /* This function is called by kernel to setup interrupt vector. */

  #if (INTVSZ != 0)
    intvec[pk_cisr->intno] = (UW)hdr;
  #endif
}

/*****************************************************************************
* Get Interrupt Handler Address in Interrupt Vector Table
*
******************************************************************************/

FP _kernel_get_inthdr(const T_CISR *pk_cisr)
{
    /* This function is called by kernel to refer interrupt vector. */

    return (FP)intvec[pk_cisr->intno];
}

/*****************************************************************************
* Define Interrupt Handler - System Call
*
******************************************************************************/

ER v4_def_inh(INHNO inhno, const V4_T_DINH *pk_dinh)
{
    /* Register an interrupt handler address to the interrupt vector table or
       an exception handler address to the exception vector table on RAM.
       The interrupt/exception handler address is specified by pk_dinh->inthdr.
       Removes its registration if pk_dinh is specified as NULL. Returns E_PAR
       when inhno is out of range of the interrupt/exception number. */

  #if ((INTVSZ != 0) || (EXCVSZ != 0))
    FP fp;
  #endif

    /* interrupt handler */
    if ((int)inhno >= 0) {
      #if (INTVSZ == 0) /* when interrupt vector table is on ROM, */
        return E_NOSPT; /* interrupt handler cannot be defined dynamically */
      #else
        if (inhno >= INTVSZ / 4)
            return E_PAR;
        /* deregister interrupt handler */
        if (pk_dinh == (T_DINH *)NULL)
            fp = (FP)_kernel_interr;
        /* register interrupt handler */
        else
            fp = pk_dinh->inthdr;
        intvec[inhno] = (UW)fp;
        return E_OK;
      #endif

    /* exception handler */
    } else {
      #if (EXCVSZ == 0) /* when exception vector table is on ROM, */
        return E_NOSPT; /* exception handler cannot be defined dynamically */
      #else
        if ((inhno = (INTNO)(-(int)inhno)) >= EXCVSZ / 4)
            return E_PAR;
        /* deregister exception handler (restore vector on ROM) */
        if (pk_dinh == (T_DINH *)NULL)
            fp = (FP)*(UW *)(0xFFFFFF80 + EXCVSZ - inhno * 4);
        /* register exception handler */
        else
            fp = pk_dinh->inthdr;
        excvec[EXCVSZ / 4 - inhno] = (UW)fp;
        return E_OK;
      #endif
    }
}

/*****************************************************************************
* Enable Interrupt - System Call
*
******************************************************************************/

ER v3_ena_int(UINT intno)
{
    UINT psw;

    /* set interrupt request enable bit */
    psw = vdis_psw();
    sfr_set(ICU_IERm + (intno >> 3), 1 << (intno & 0x07));
    vset_psw(psw);

    return E_OK;
}

/*****************************************************************************
* Disable Interrupt - System Call
*
******************************************************************************/

ER v3_dis_int(UINT intno)
{
    UINT psw;

    /* clear interrupt request enable bit */
    psw = vdis_psw();
    sfr_clr(ICU_IERm + (intno >> 3), 1 << (intno & 0x07));
    vset_psw(psw);

    return E_OK;
}

/*****************************************************************************
* Start Interval Timer - System Call (NORTi unique)
*
******************************************************************************/

ER intsta(void)
{
  #if (INTVSZ != 0)
    ER ercd;

    /* create interrupt service routine for interval timer */
    ercd = acre_isr(&cisr_isritm);
    if (ercd <= 0)
        return ercd;
  #endif

    sfr_set(ICU_CMT_IPR, IP);               /* set interrupt priority level */

    sfr_outw(RWP_PRCR,    0xA502);
    sfr_clrl(LPC_MSTPCRA, 0x00008000);      /* release CMT0~1 module stop state */
    sfr_outw(RWP_PRCR,    0xA500);

    sfr_outw(CMT_CMCOR,   (UH)(TC - 1));    /* compare value */
    sfr_outw(CMT_CMCR,    0x0080 |          /* write 1 to indefinite bit */
                          CKS |             /* select clock */
                          0x0040);          /* enable compare match interrupt */
    ena_int(INT_CMT);                       /* enable timer interrupt */
    sfr_setw(CMT_CMSTR0,  1 << CH);         /* start timer */
  #ifdef HOOK
    sfr_outw(CMT1_CMCOR,  (UH)0xFFFF);      /* compare value */
    sfr_outw(CMT1_CMCR,   0x0080 |          /* write 1 to indefinite bit */
                          CKS);             /* select clock */
    sfr_setw(CMT_CMSTR0,  2);               /* start another timer */
  #endif

    return E_OK;
}

/*****************************************************************************
* Interval Timer Interrupt Service Routine / Handler
*
******************************************************************************/
static UB count1 = 0;		//100ms判定用カウンタ
static UB count2 = 0;		//500ms判定用カウンタ
#if (INTVSZ == 0)         /* when vector table is on ROM, */
#pragma noinline(isritm)  /* implemented as interrupt handler */
void isritm(void)
#else                     /* when vector table is on RAM, */
void isritm(VP_INT exinf) /* implemented as interrupt service routine */
#endif
{
  #ifdef HOOK
    UINT psw;

    psw = vdis_psw();
    sfr_outw(CMT1_CMCNT, 0x0000);           /* clear another timer counter */
    TOTAL_TICKCK += MSEC * 1000;
    vset_psw(psw);
  #else
    TOTAL_TICKCK += MSEC * 1000;            /* count by 1000 microseconds */
  #endif
    isig_tim();                             /* supply time tick to kernel */
    
	
	/*10msイベント*/
	set_flg((ID)ID_TIMER_FLG, (FLGPTN)BIT_TIMER_10MSEC);						//タイマータスクへの10ms起動イベント
	
	/* 100msイベント */
	if(count1 >= (100/MSEC)){
	
		set_flg((ID)ID_TIMER_FLG, (FLGPTN)BIT_TIMER_100MSEC);					//タイマータスクへの100ms起動イベント
		count1 = 0;
		
    }
    
    /* 500msイベント */
	if(count2 >= (500/MSEC)){
		
		set_flg((ID)ID_TIMER_FLG, (FLGPTN)BIT_TIMER_500MSEC);					//タイマータスクへの500ms起動イベント
		count2 = 0;
		
    }
    /* カウントアップ */
    count1++;
    count2++;
}

#if (INTVSZ == 0)
INTHDR inthdr(void)
{
    ent_int();                              /* start interrupt handler */
    isritm();                               /* call interrupt handler body */
    ret_int();                              /* return interrupt handler */
}
#endif

/*****************************************************************************
* Read Tick Clock in microseconds
*
******************************************************************************/

UW tickck_read(void)
{
    volatile UW *p;
    UW tt, ck;

    p = &TOTAL_TICKCK;                      /* system time in milliseconds x 1000 */
L1:
    tt = *p;
    ck = sfr_inw(CMT_CMCNT);
    if (!(sfr_inw(CMT_CMSTR0) & (1 << CH))) /* return some value before timer starts */
        return (*p)++ / 5;                  /* (should adjust for 1 microsecond) */
  #ifdef HOOK
    if (ck < sfr_inw(CMT1_CMCNT))           /* when another timer counter is bigger, */
  #else
    if (sfr_in(ICU_CMI_IR) != 0)            /* when interrupt is pending, */
  #endif
        ck = sfr_inw(CMT_CMCNT) + TC;       /* compensate for uncounted TOTAL_TICKCK */
    if (tt != *p)
        goto L1;                            /* retry if interrupt occurs on here */

    return tt + (ck * MSEC * 1000) / TC;
}

/*****************************************************************************
* Delay in Microsecond Unit - System Call (NORTi unique)
*
******************************************************************************/

void dly_1us(W time)
{
    UW t;

    if (time < 0) {                         /* if you use this before startup */
        TOTAL_TICKCK = 0;                   /* call dly_1us(-1) to initialize */
        return;
    }

    t = tickck_read();
    while (tickck_read() - t < (UW)time)
        ;
}

/* end */
