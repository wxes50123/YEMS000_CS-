//******************************************************************************
// @(R)   
//		共通関数
//		
//		(その関数を記述するファイル先頭に***_FUNC_DEFINEを定義すること。)
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.11.10	初版作成
//******************************************************************************
/* 初期化関数 */
#ifndef INIT_FUNC_DEFINE
extern void ini_cpu(void);			//周辺I/O端子設定
extern void ini_ioport(void);		//周辺I/O端子初期化
extern void ini_global(void);		//グローバル変数初期化
#endif

/* タイマー関数 */
#ifndef TIMER_FUNC_DEFINE
extern ER EntryTimer10ms(UW interval_time, ID evt_id,  UINT bitptn);
extern ER CancelTimer10ms(ID evt_id, UINT bitptn);
#endif

/* EEPROM関数 */
#ifndef EEPROM_FUNC_DEFINE
extern ER eep_mac_read(UB *macaddr);
#endif


/* デバッグ関数 */
#ifndef DEBUG_FUNC_DEFINE
extern void DebugOut(char *str);
extern void DebugOut_time(void);
extern void DebugOut_ito0X(char *s , unsigned short n);
extern void DebugOut_lto0X(char *s , unsigned long n);
extern void DebugOut_itou(char *s , unsigned short n);
extern void DebugOut_ltou(char *s , unsigned long n);
extern void DebugOut_itod(char *s , short n);
extern void DebugOut_ltod(char *s , long n);
extern void DebugOut_itou_fit(char *s , unsigned short n);
extern void DebugOut_ltou_fit(char *s , unsigned long n);
extern void DebugOut_LF(void);
#endif

/* END */
