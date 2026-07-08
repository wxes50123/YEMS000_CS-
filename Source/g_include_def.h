//******************************************************************************
// @(R)
//		共通インクルード定数定義
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//****** 変更履歴 **************************************************************
//ver1.0	2025.11.01	初版作成
//******************************************************************************
//******** ヘッダファイル参照宣言部 ********************************************
#include "kernel.h"				//NORTi用定義
#include "kernel_id.h"			//NORTi用定義
#include "iodefine.h"			//RX72M用レジスタ定義
#include "nosio.h"				//シルアル入出力関数
#include "nonet.h"				//LAN用
#include "g_variable_def.h"		//グローバル変数
#include "g_extern_def.h"		//共通関数定義(デバッグ用関数マクロ定義後にインクルードする)
/********************************************************************************/
/*		バージョン定義															*/
/********************************************************************************/
#define SOFT_VER	0x0001								//ソフトウェアバージョン
#define SOFT_REV	0x0000								//ソフトウェアレビジョン
/********************************************************************************/
/*		動作設定・デバッグ情報定義												*/
/********************************************************************************/
/* ウォッチドッグタイマ(WDT) */
#define WDT_ENABLE										//WDTを有効にする(使用しない場合はコメントアウトする)
/* デバッグ情報出力 */
//無効 #define DEBUG_OUT_ENABLE							//デバッグ情報,変数情報をRS422で出力する(出力しない場合はコメントアウトする)
/* 変数情報出力 */										//※デバッグ情報出力も有効である必要がある
//無効 #define DEBUG_VALINFO_ENABLE						//変数情報をRS422で出力する(出力しない場合はコメントアウトする)
#define DEBUG_VALINFO_TIM		1						//変数情報を何秒周期で出力するか
/********************************************************************************/
/*		デバッグ用関数定義														*/
/********************************************************************************/
#ifdef DEBUG_OUT_ENABLE									//※DEBUG_OUT_ENABLEを定義時のみ有効

#define		Pout_s(s)			DebugOut(s)				//文字の出力
#define 	Pout_time(s)		DebugOut_time(s)		//時刻の出力
#define		Pout_ito0X(s,n)		DebugOut_ito0X(s,n)		//文字とUHを16進で出力,先行０付
#define		Pout_lto0X(s,n)		DebugOut_lto0X(s,n)		//文字とUWを16進で出力,先行０付
#define		Pout_itou(s,n)		DebugOut_itou(s,n);		//文字とUHを10進数で出力,右詰め
#define		Pout_ltou(s,n)		DebugOut_ltou(s,n);		//文字とUWを10進数で出力,右詰め
#define		Pout_itod(s,n)		DebugOut_itod(s,n)		//文字とHを10進数符号付で出力
#define		Pout_ltod(s,n)		DebugOut_ltod(s,n)		//文字とWを10進数符号付で出力
#define		Pout_itou_fit(s,n)	DebugOut_itou_fit(s,n)	//文字とUHを出力（数値は桁数に応じた文字数で出力）
#define		Pout_ltou_fit(s,n)	DebugOut_ltou_fit(s,n)	//文字とUWを出力（数値は桁数に応じた文字数で出力）
#define		Pout_LF()			DebugOut_LF()			//改行出力

#else

#define		Pout_s(s)
#define 	Pout_time(s)
#define		Pout_ito0X(s,n)
#define		Pout_lto0X(s,n)
#define		Pout_itod(s,n)
#define		Pout_ltod(s,n)
#define		Pout_itou(s,n)
#define		Pout_ltou(s,n)
#define		Pout_itou_fit(s,n)
#define		Pout_ltou_fit(s,n)
#define		Pout_LF()

#endif
/********************************************************************************/
/*		共通定数定義															*/
/********************************************************************************/
#define ER_OK		0						//正常戻り値
#define ER_NG		(-1)					//異常戻り値
#define FLG_ON		1						//フラグON
#define FLG_OFF		0						//フラグOFF
#define BIT_ON		1						//ビットON
#define BIT_OFF		0						//ビットOFF
#define LED_ON		1						//LEDON
#define LED_OFF		0						//LEDOFF
#define ERROR_NONE	0						//エラー無し
#define ERROR_EXIST	1						//エラー有り
/********************************************************************************/
/*		WDT定数定義																*/
/********************************************************************************/
#define	WDT_LOW		WDT.WDTRR = 0x00				//WDTクリア LOW
#define	WDT_HIGH	WDT.WDTRR = 0xFF				//WDTクリア HIGH
#define WDT_START	do{WDT_LOW;WDT_HIGH;}while(0)	//WDTスタート
#define WDT_CLR		WDT_START						//WDTクリア
/********************************************************************************/
/*		動作ﾊﾟﾗﾒｰﾀ(将来EEPROM用)												*/
/********************************************************************************/
/* 本バージョンでは固定とするが、将来的にEEPROM経由で取得する */
#define BAT_SOC_START_THRESHOLD		100											//発電機始動SOC閾値[0.1%単位]
#define BAT_VOL_START_THRESHOLD		3500										//発電機始動電圧閾値[0.1V単位]
#define GEN_CHARGE_START_TIME		1020										//発電機充電開始時刻[分]0(00:00)～1439(23:59) 1020(17:00)
#define GEN_CHARGE_END_TIME			360											//発電機充電終了時刻[分]0(00:00)～1439(23:59)  360(06:00)
/* 目標値 */
#define GEN_60HZ_OUT				11000										//発電機出力(60Hz):11kW
#define GEN_50HZ_OUT				5000										//発電機出力(50Hz):5kW
#define GEN_BAT_CHG_POWER			9500										//蓄電池への充電電力:9.5kW
/********************************************************************************/
/*		基板LED定義																*/
/********************************************************************************/
#define LED1_RED	PORT0.PODR.BIT.B0				//基板LED1(赤)
#define LED2_GREEN	PORT0.PODR.BIT.B1				//基板LED2(緑)
#define LED3_GREEN	PORT0.PODR.BIT.B2				//基板LED3(緑)
#define LED4_GREEN	PORT0.PODR.BIT.B3				//基板LED4(緑) LiveLED
#define SET_LED(led,onoff)	((led)=(onoff))			//基板LED ON/OFF設定
#define LED_ALL_OFF	do{PORT0.PODR.BIT.B0 = 0;PORT0.PODR.BIT.B2 = 0;PORT0.PODR.BIT.B3 = 0;}while(0)	//基板LED全消去
/********************************************************************************/
/*		DI/DO定義																*/
/********************************************************************************/
/* DO出力ポート割当 */
#define DO_PCS_START_SW_LED	PORTB.PODR.BIT.B0		//PCS起動SW LED
#define DO_PCS_STOP_SW_LED	PORTB.PODR.BIT.B1		//PCS停止SW LED
#define DO_PCS_MAINTESW_LED	PORTB.PODR.BIT.B2		//メンテナンスSW LED
#define DO_PCS_SHT_RY		PORTB.PODR.BIT.B3		//SHT制御リレー
#define DO_YOBI2			PORTB.PODR.BIT.B4		//(未使用2)
#define DO_YOBI3			PORTB.PODR.BIT.B5		//(未使用3)
#define DO_EG_RY			PORTB.PODR.BIT.B6		//発電機始動リレー(E/G発停リレー)
#define DO_FAN				PORTB.PODR.BIT.B7		//ファン
#define DO_YOBI4			PORTA.PODR.BIT.B6		//(未使用4)
#define DO_YOBI5			PORTA.PODR.BIT.B7		//(未使用5)
#define DO_YOBI6			PORTA.PODR.BIT.B5		//(未使用6)
#define SET_DO(outport,onoff)	((outport)=(onoff))	//DO ON/OFF設定
/* DI入力ポート割当 */
#define DI_PCS_START_SW		PORT5.PIDR.BIT.B0		//PCS起動SW
#define DI_PCS_STOP_SW		PORT5.PIDR.BIT.B1		//PCS停止SW
#define DI_PCS_MAINTE_SW	PORT5.PIDR.BIT.B2		//メンテナンスSW
#define DI_PCS_AUX3			PORT5.PIDR.BIT.B3		//AUX3
#define DI_YOBI1			PORT1.PIDR.BIT.B0		//(未使用1)
#define DI_YOBI2			PORT1.PIDR.BIT.B1		//(未使用2)
#define DI_YOBI3			PORTJ.PIDR.BIT.B0		//(未使用3)
#define DI_YOBI4			PORTJ.PIDR.BIT.B1		//(未使用4)
/* DI入力判定(メンテナンスSW以外) */
#define DI_COUNT			3						//DI信号値確定カウント(20ms×3回=60msで入力信号確定)
/* DI入力判定(メンテナンスSW) */
#define DI_COUNT_MNT_1S		50						//DI信号｢メンテナンスSW｣押下時間判定1秒以上(20ms×50回)
#define DI_COUNT_MNT_5S		250						//DI信号｢メンテナンスSW｣押下時間判定5秒以上(20ms×250回)
#define DI_COUNT_MNT_10S	500						//DI信号｢メンテナンスSW｣押下時間判定10秒以上(20ms×500回)
/* メンテナンスSW状態(g_di[PCS_MAINTE_SW].valの中身) */
#define MNTSW_OFF			0						//メンテナンスSWOFF状態(20ms以上)
#define MNTSW_ON			1						//メンテナンスSW20m以上ON状態
#define MNTSW_1S			2						//メンテナンスSW1秒以上ON状態
#define MNTSW_5S			3						//メンテナンスSW5秒以上ON状態
#define MNTSW_10S			4						//メンテナンスSW10秒以上ON状態
/* メンテナンスSW点滅間隔 */
#define MNT_BLINK_STOP		0						//メンテナンスSW点滅停止
#define MNT_BLINK1			2						//メンテナンスSW点滅間隔パターン1(1秒:500ms×2カウント)
#define MNT_BLINK2			4						//メンテナンスSW点滅間隔パターン2(2秒:500ms×4カウント)
/* 発電機始動リレーパラメータ */
#define GEN_RETRY			3						//発電機始動最大リトライ回数(回)
#define CLEANING_TIM		9						//接点クリーニング時間(秒)
#define OFFWAIT_TIM			15						//発電機始動リレーOFF待機時間(秒)
/* DI管理No.(g_di[DI_MAX]で使用) */
#define PCS_MAINTE_SW		0						//CN6-3 メンテナンスSW(DI確定時間が異なるので別判定するため、先頭に定義)
#define PCS_START_SW		1						//CN6-1 PCS起動SW
#define PCS_STOP_SW			2						//CN6-2 PCS停止SW
#define PCS_AUX3			3						//CN6-4 AUX3
#define YOBI1				4						//CN6-5 (未使用1)
#define YOBI2				5						//CN11-1(未使用2)
#define YOBI3				6						//CN11-2(未使用3)
#define YOBI4				7						//CN11-3(未使用4)
/********************************************************************************/
/*		各タスク起動周期														*/
/********************************************************************************/
#define BASE_MS				20						//メインタスク起動周期(20ms)(ベースサイクル)
#define UDP_TSK_CYCLE		500						//LAN(UDP)タスク起動周期(500ms)
#define TCP_TSK_CYCLE		500						//LAN(TCP)タスク起動周期(500ms)
#define CAN_TSK_CYCLE		1000					//CANタスク起動周期(1000ms)1秒
#define RS485_TSK_CYCLE		1000					//RS485タスク起動周期(1000ms)1秒
#define MODBUS_TSK_CYCLE	1000					//Modbusタスク起動周期(1000ms)1秒
#define DI_TSK_CYCLE		20						//DIタスク起動周期(20ms)
#define AI_TSK_CYCLE		10000					//AIタスク起動周期(10000ms)10秒
/********************************************************************************/
/*		各タスクフラグ管理														*/
/********************************************************************************/
/* ID_TIMER_FLG フラグパターン */
#define BIT_TIMER_10MSEC		(UINT)0x00000001		//指定時間経過通知10msタイマー
#define BIT_TIMER_100MSEC		(UINT)0x00000002		//指定時間経過通知100msタイマー
#define BIT_TIMER_500MSEC		(UINT)0x00000004		//指定時間経過通知500msタイマー
/* ID_MAIN_FLG フラグパターン */
#define F_MAIN_EVENT				(UINT)0x00000001	//メインタスク起動イベント
#define F_MAIN_LEDUPD_EVENT			(UINT)0x00000002	//LED状態更新イベント
#define F_MAIN_GEN_STARTOFF_15SEC	(UINT)0x00000004	//発電機始動フラグOFF後15秒
#define F_MAIN_GEN_BREAKEROFF_10SEC	(UINT)0x00000008	//発電機ブレーカ遮断判定10秒
/* ID_LANCLI_UDP_FLG フラグパターン */
#define F_UDP_EVENT					(UINT)0x00000001	//UDP起動イベント
#define F_UDP_RESET_EVENT			(UINT)0x00000002	//UDPリセットイベント
#define F_UDP_RCVDT_RUNCMD_EVENT	(UINT)0x00000004	//UDP｢PCS運転コマンド｣受信イベント
/* ID_LANCLI_TCP_FLG フラグパターン */
#define F_TCP_EVENT				(UINT)0x00000001		//TCP起動イベント
#define F_TCP_RESET_EVENT		(UINT)0x00000002		//TCPリセットイベント
#define F_TCP_RCVDT_CPY_EVENT	(UINT)0x00000004		//TCPデータ受信イベント
/* ID_CAN_FLG フラグパターン */
#define F_CAN_EVENT				(UINT)0x00000001		//CANタスク起動イベント
#define F_CAN_VOL_CNTL_EVENT	(UINT)0x00000002		//発電機電圧制御イベント
#define F_CAN_TIMOUT_EXT		(UINT)0x00000004		//外部コントロール通信断イベント
#define F_CAN_TIMOUT_AVR		(UINT)0x00000008		//AVR基板通信断イベント
#define F_CAN_TIMOUT_DISP		(UINT)0x00000010		//表示基板通信断イベント
/* ID_RS485_FLG フラグパターン */
#define F_RS485_SNDTMO_EVENT	(UINT)0x00000001		//RS485送信待ちタイムアウトイベント
#define F_RS485_VOLCHK_EVENT	(UINT)0x00000002		//状態チェックイベント(電圧現在値)
#define F_RS485_CURCHK_EVENT	(UINT)0x00000004		//状態チェックイベント(電流現在値)
#define F_RS485_ADDRSET_EVENT	(UINT)0x00000008		//ADDR設定イベント
#define F_RS485_REMSSET_EVENT	(UINT)0x00000010		//REMOTEモード切替イベント
#define F_RS485_P_ON_EVENT		(UINT)0x00000020		//電源ONイベント
#define F_RS485_P_OFF_EVENT		(UINT)0x00000040		//電源OFFイベント
#define F_RS485_VOL_SET_EVENT	(UINT)0x00000080		//電圧設定イベント
#define F_RS485_CUR_SET_EVENT	(UINT)0x00000100		//電流設定イベント
/* ID_MODBUS_FLG フラグパターン */
#define F_MODBUS_REQ_EVENT		(UINT)0x00000001		//MODBUS状態要求イベント
/* ID_DI_FLG フラグパターン */
#define F_DI_EVENT				(UINT)0x00000001		//DIリード起動イベント
/* ID_AI_FLG フラグパターン */
#define F_AI_EVENT				(UINT)0x00000001		//AIリード起動イベント
#define F_AI_TMO_EVENT			(UINT)0x00000002		//AD変換待ちタイムアウトイベント

/* MACRO for FLAG */
	#define WAI_FLG(i,s)			wai_flg( ((ID)(i)) , (FLGPTN)0xFFFFFFFF	, (UINT)(TWF_ORW|TWF_CLR) , ((UINT PFAR *)(s)))
//	#define WAI_FLG_NoClr(i,s)		wai_flg( ((ID)(i)) , (FLGPTN)0xFFFFFFFF	, (UINT)(TWF_ORW) ,((UINT PFAR *)(s)))
//	#define WAI_Resp(i,s,r)			wai_flg( ((ID)(i)) , (FLGPTN)(r)			, (UINT)(TWF_ORW|TWF_CLR) ,  ((UINT PFAR *)(s)))
//	#define POLL_FLG(i,s)			pol_flg( ((ID)(i)) , (FLGPTN)0xFFFFFFFF	, (UINT)(TWF_ORW|TWF_CLR) , ((UINT PFAR *)(s)))

/********************************************************************************/
/*		LAN通信定数定義															*/
/********************************************************************************/
/* ポート番号 */
#define UDP_MY_PORT		(10001)				//UDPポート番号(自分:EMS基板)
#define UDP_TG_PORT		(10000)				//UDPポート番号(相手:PCS)
#define TCP_CLI_PORT	(502)				//TCPポート番号クライアント回線(自分:EMS基板)
#define TCP_SRV_PORT	(502)				//TCPポート番号サーバ回線(相手:PCS_IDENCON)
/* 自分側IPアドレス(EMS基板IPアドレス:10.10.10.45) */
#define OCT_IP_SRC_1	(10)				//第1オクテット
#define OCT_IP_SRC_2	(10)				//第2オクテット
#define OCT_IP_SRC_3	(10)				//第3オクテット
#define OCT_IP_SRC_4	(45)				//第4オクテット
/* 相手側IPアドレス(PCS UDP IPアドレス:10.10.10.10) */
#define OCT_IP_DST1_1	(10)				//第1オクテット
#define OCT_IP_DST1_2	(10)				//第2オクテット
#define OCT_IP_DST1_3	(10)				//第3オクテット
#define OCT_IP_DST1_4	(10)				//第4オクテット※注意！010と記述すると8進数になり'8'になる
/* 相手側IPアドレス(PCS TCP IPアドレス:10.10.10.11) */
#define OCT_IP_DST2_1	(10)				//第1オクテット
#define OCT_IP_DST2_2	(10)				//第2オクテット
#define OCT_IP_DST2_3	(10)				//第3オクテット
#define OCT_IP_DST2_4	(11)				//第4オクテット※注意！010と記述すると8進数になり'8'になる
/* デフォルトゲートウェイ:0.0.0.0 */
#define OCT_DG_1		(0)					//第1オクテット
#define OCT_DG_2		(0)					//第2オクテット
#define OCT_DG_3		(0)					//第3オクテット
#define OCT_DG_4		(0)					//第4オクテット
/* サブネットマスク:255.255.255.0 */
#define OCT_SM_1		(255)				//第1オクテット
#define OCT_SM_2		(255)				//第2オクテット
#define OCT_SM_3		(255)				//第3オクテット
#define OCT_SM_4		(000)				//第4オクテット
/* TCPクライアント回線状態 */
#define STS_CLI_ACP		0x0001				//コネクション要求
#define STS_CLI_SND		0x0002				//データ送信
#define STS_CLI_RCVWAI	0x0004				//データ受信待ち
#define STS_CLI_CLS		0x0008				//コネクション切断
/* タイムアウト値 */
#define TMO_UDP_RCV		(3000/MSEC)			//UDP受信タイムアウト値
#define TMO_UDP_SND		(3000/MSEC)			//UDP送信タイムアウト値
#define TMO_TCP_RCV		(TMO_NBLK)			//TCP受信タイムアウト値
#define TMO_TCP_SND		(3000/MSEC)			//TCP送信タイムアウト値
#define TMO_TCP_ACP		(10000/MSEC)		//TCP接続要求待ち
#define TMO_TCP_CLS		(200/MSEC)			//TCP接続切断待ち
#define TMO_TCP_RESET	(5000/MSEC)			//TCPコネクションリセット用タイムアウト値(5秒:10ms*500)
/* ModbusTCP/UDPタスク 16bit→8bit変換 */
#define HI8_U16(v)		((UB)(((UH)(v)>>8) & 0xFF))				//U16bit上位8bit取得
#define LO8_U16(v)		((UB)(((UH)(v)   ) & 0xFF))				//U16bit下位8bit取得
#define MAKE_U16(hi,lo)	((UH)((((UH)(hi))<<8) | ((UH)(lo))))	//U16bit生成
#define HI16_U32(v)		((UH)(((UW)(v)>>16) & 0xFFFF))			//U32bit上位16bit取得
#define LO16_U32(v)		((UH)(((UW)(v)    ) & 0xFFFF))			//U32bit下位16bit取得
#define MAKE_U32(hi,lo)	((UW)((((UW)(hi))<<16 | ((UW)(lo)))))	//U32bit生成
/********************************************************************************/
/*		発電機定数定義															*/
/********************************************************************************/
#define GEN_STOP		0					//発電機停止中
#define GEN_RUN			1					//発電機運転中
/* 発電機電圧制御 */
#define GEN_VOL_KEEP	0					//電圧調整フラグ:そのまま
#define GEN_VOL_UP		1					//電圧調整フラグ:上げる
#define GEN_VOL_DOWN	2					//電圧調整フラグ:下げる
/* 発電制御電圧調整パターン */
#define GEN_VOL_ADJUST_KEEP			0		//現電圧維持(送信周期はCANタスク起動周期と同じ)
#define GEN_VOL_ADJUST_KEEP_300V	1		//電圧300V維持(送信周期はCANタスク起動周期と同じ)
#define GEN_VOL_ADJUST_KEEP_280V	2		//電圧280V維持(送信周期はCANタスク起動周期と同じ)
#define GEN_VOL_ADJUST_KEEP_250V	3		//電圧250V維持(送信周期はCANタスク起動周期と同じ)
#define GEN_VOL_ADJUST_15S1VUP		4		//15秒ごとに1VUP
#define GEN_VOL_ADJUST_2S1VUP		5		//2秒ごとに1VUP
#define GEN_VOL_ADJUST_01SVDOWN		6		//0.1秒ごとに1VDOWN
/* 電圧調整時間パラメータ */
#define GEN_VOL_ADJUST_TIME1000MS	1000	//1秒(1000ms)
#define GEN_VOL_ADJUST_TIME2000MS	2000	//2秒(2000ms)
#define GEN_VOL_ADJUST_TIME15000MS	15000	//15秒(15000ms)
#define GEN_VOL_ADJUST_TIME100MS	100		//0.1秒(100ms)
/* 発電機(CAN)通信断検知時間 */
#define GEN_EXT_COMM_ERR_TIM	200		//外部コントロール通信断検知時間(2秒:10ms*200)
#define GEN_AVR_COMM_ERR_TIM	200		//AVR基板通信断時間(2秒:10ms*200)
#define GEN_DISP_COMM_ERR_TIM	200		//表示基板通信断検知時間(2秒:10ms*200)
#define GEN_RUN_RPM_THRESHOLD	700		//発電機運転回転数しきい値(rpm)
#define GEN_RUN_FREQ_THRESHOLD	1600	//発電機周波数判定しきい値(rpm)
/* 発電機始動時間 */
#define GEN_60SEC				3000	//60秒(20ms*3000count)
#define GEN_11MIN				33000	//11分(20ms*33000count)
#define GEN_START_FLGOFF		0		//発電機始動時間フラグOFF
#define GEN_START_UNDER_60SEC	1		//発電機始動時間フラグON後60秒未満
#define GEN_START_OVER_60SEC	2		//発電機始動時間フラグON後60秒以上11分未満
#define GEN_START_OVER_11MIN	3		//発電機始動時間フラグON後11分以上
#define GEN_BREAKER_OFF_10SEC	4		//ブレーカ遮断判定停止後10秒以上
/* 発電機始動停止 */
#define GEN_START_SOC_BIT1		0x01	//発電機始動(SOC要因)
#define GEN_START_SC_BIT2		0x02	//発電機始動(SC要因)
#define GEN_STOP_BIT3			0x04	//発電機停止
#define GEN_PCSSTOP_BIT4		0x08	//PCS停止
#define GEN_PCS_PDOWN_BIT5		0x10	//PCS電源断
/* 発電機始動要因 */
#define GEN_START_CAUSE_CLR		0x00	//発電機始動要因クリア
#define GEN_START_CAUSE_SOC		0x01	//発電機始動要因(bit0:SOC基準値以下)
#define GEN_START_CAUSE_BAT_VOL	0x02	//発電機始動要因(bit1:電池電圧基準値以下)
#define GEN_START_CAUSE_TIME	0x04	//発電機始動要因(bit2:充電スケジュール時間内)
/********************************************************************************/
/*		シリアル通信共通定義													*/
/********************************************************************************/
#define DEBUGOUT_CH		1							//DebugOut関数の出力先(SCI1)
#define DEBUGOUT_TXTMO	(50/MSEC)					//1文字送信タイムアウト時間
#define UART0		0								//SCI0のチャンネル:0
#define UART1		1								//SCI1のチャンネル:1
#define UART2		2								//SCI2のチャンネル:2
#define UART0_CFG	"4800 B8 PN S1"					//4800bps, 8bit,パリティ無,ストップbit1
#define UART1_CFG	"115200 B8 PN S1"				//115200bps, 8bit,パリティ無,ストップbit1
#define UART2_CFG	"19200 B8 PN S1"				//19200bps,8bit,パリティ無,ストップbit1
#define UART0_CTL	TSIO_RXE|TSIO_TXE|TSIO_RTSON	//受信有効,送信有効,RTS信号ON
#define UART1_CTL	TSIO_RXE|TSIO_TXE|TSIO_RTSON	//受信有効,送信有効,RTS信号ON
#define UART2_CTL	TSIO_RXE|TSIO_TXE|TSIO_RTSON	//受信有効,送信有効,RTS信号ON
/* タイムアウト値 */
#define TMO_SND_CHR		(50/MSEC)					//1文字送信待ち
#define TMO_RCV_CHR		(400/MSEC)					//1文字受信待ち
/********************************************************************************/
/*		ModbusRTUタスク定数定義													*/
/********************************************************************************/
#define SLAVE_ADDR		0x01	//スレーブアドレス(初期値:1)
#define FNC_READ		0x04	//ファンクションコード：入力レジスタ読出し
/* レジスタアドレス */
#define ADDR_OFFSET		30001	//開始アドレス算出用オフセット値(実アドレスからこの値をマイナスして設定する)
#define STATUS1_ADDR	30001	//ステータス情報1
#define STATUS2_ADDR	30002	//ステータス情報2
#define STATUS3_ADDR	30003	//ステータス情報3
#define STATUS4_ADDR	30004	//ステータス情報4
#define STATUS5_ADDR	30005	//ステータス情報5
/* 非公開エリア 30006～30020 */
#define DCV_ADDR		30021	//直流電圧
#define DCA_ADDR_T		30022	//直流電流(上位)
#define DCA_ADDR_D		30023	//直流電流(下位)
#define DCW_ADDR_T		30024	//直流電力(上位)
#define DCW_ADDR_D		30025	//直流電力(下位)
#define ACVRS_ADDR		30026	//交流電圧R-S
#define ACVST_ADDR		30027	//交流電圧S-T
#define ACVTR_ADDR		30028	//交流電圧T-R
#define ACAR_ADDR_T		30029	//交流電流R(上位)
#define ACAR_ADDR_D		30030	//交流電流R(下位)
#define ACAS_ADDR_T		30031	//交流電流S(上位)
#define ACAS_ADDR_D		30032	//交流電流S(下位)
#define ACAT_ADDR_T		30033	//交流電流T(上位)
#define ACAT_ADDR_D		30034	//交流電流T(下位)
#define ACWP_ADDR_T		30035	//交流電力 正(上位)
#define ACWP_ADDR_D		30036	//交流電力 正(下位)
#define ACWN_ADDR_T		30037	//交流電力 負(上位)
#define ACWN_ADDR_D		30038	//交流電力 負(下位)
#define FREQ_ADDR		30039	//系統周波数
#define UNITGEN_ADDR_T	30040	//単機積算発電量(上位)
#define UNITGEN_ADDR_D	30041	//単機積算発電量(下位)
#define UNITCHG_ADDR_T	30042	//単機積算充電量(上位)
#define UNITCHG_ADDR_D	30043	//単機積算充電量(下位)
#define SOLAR_ADDR		30044	//日射強度
#define TEMP_ADDR		30045	//気温
#define YOBI1_ADDR		30046	//予備1
#define YOBI2_ADDR		30047	//予備2
/* 非公開エリア 30048～30082 */
#define OUT_CON_ADDR	30083	//出力制御指令
/* 非公開エリア 30084～30200 */
/********************************************************************************/
/*		RS485タスク定数定義														*/
/********************************************************************************/
/* SCI0定義 */
#define SCI0_SEND_ENABLE do{PORT1.PODR.BIT.B6 = 1;PORT1.PODR.BIT.B7 = 1;}while(0)	//SCI0送信許可(受信無効)ｽｲｯﾁﾝｸﾞ電源
#define SCI0_RECV_ENABLE do{PORT1.PODR.BIT.B6 = 0;PORT1.PODR.BIT.B7 = 0;}while(0)	//SCI0受信許可(送信無効)
#define SCI0_SEND_TIMOUT (50/MSEC)													//SCI0送信タイムアウト
/* SCI2定義 */
#define SCI2_SEND_ENABLE do{PORT8.PODR.BIT.B6 = 1;PORT8.PODR.BIT.B7 = 1;}while(0)	//SCI2送信許可(受信無効)PCS
#define SCI2_RECV_ENABLE do{PORT8.PODR.BIT.B6 = 0;PORT8.PODR.BIT.B7 = 0;}while(0)	//SCI2受信許可(送信無効)
#define SCI2_SEND_TIMOUT (50/MSEC)													//SCI2送信タイムアウト
/* スイッチング電源定義 */
#define ADDRSW		0					//ADDR=0(使用するスイッチング電源のAddr.SWと合わせる)
#define REMS		1					//REMS=1(1:リモート設定 0:ローカル設定 通常は1で運用する)
/* 文字コード */
#define RS485_CR	0x0D				//CR
#define RS485_LF	0x0A				//LF
#define RS485_SP	0x20				//半角スペース
/********************************************************************************/
/*		DIタスク用定義															*/
/********************************************************************************/
#define DISET_COUNT			3					//DI確定カウント数(20msec × 3回 = DI信号確定時間60(msec))
/********************************************************************************/
/*		AIタスク用定義															*/
/********************************************************************************/
#define AD_TIMOUT	(100/MSEC)					//AD変換待ちタイムアウト
#define AD_ADST		S12AD.ADCSR.BIT.ADST		//AD変換スタートビット
#define NUM_CH_S12AD0		6					//AN000～AN005
/* スキャン対象 */
#define AD_AN000_USED		1					//AN000をスキャン対象とする(1:有効,0:無効)
#define AD_AN001_USED		1					//AN001をスキャン対象とする(1:有効,0:無効)
#define AD_AN002_USED		1					//AN002をスキャン対象とする(1:有効,0:無効)
#define AD_AN003_USED		1					//AN003をスキャン対象とする(1:有効,0:無効)
#define AD_AN004_USED		1					//AN004をスキャン対象とする(1:有効,0:無効)
#define AD_AN005_USED		1					//AN005をスキャン対象とする(1:有効,0:無効)
/* 加算/平均モード選択 */
#define AD_AN000_ADD_USED	1					//AN000変換値加算/平均モード選択(1:有効,0:無効)
#define AD_AN001_ADD_USED	1					//AN001変換値加算/平均モード選択(1:有効,0:無効)
#define AD_AN002_ADD_USED	1					//AN002変換値加算/平均モード選択(1:有効,0:無効)
#define AD_AN003_ADD_USED	1					//AN003変換値加算/平均モード選択(1:有効,0:無効)
#define AD_AN004_ADD_USED	1					//AN004変換値加算/平均モード選択(1:有効,0:無効)
#define AD_AN005_ADD_USED	1					//AN005変換値加算/平均モード選択(1:有効,0:無効)
/* サンプリングステート */
#define AD_AN000_SAMPLING_STATE	0x0B			//AN000サンプリングステート
#define AD_AN001_SAMPLING_STATE	0x0B			//AN001サンプリングステート
#define AD_AN002_SAMPLING_STATE	0x0B			//AN002サンプリングステート
#define AD_AN003_SAMPLING_STATE	0x0B			//AN003サンプリングステート
#define AD_AN004_SAMPLING_STATE	0x0B			//AN004サンプリングステート
#define AD_AN005_SAMPLING_STATE	0x0B			//AN005サンプリングステート
/********************************************************************************/
/*		ModbusTCPタスクテーブル													*/
/********************************************************************************/
/* 標準運転パターン(Input Registers)I_DENCON⇒EMS */
#define OP_PATTERN_STOP					0	//停止
#define OP_PATTERN_PV_REGEN				9	//PV回生
#define OP_PATTERN_BAT_CHRG_PV_SURPLUS	24	//電池充電+PV余剰回生
#define OP_PATTERN_EV_TO_ISLAND_GEN		29	//EV→自立発電
#define OP_PATTERN_BAT_REGEN_WITH_PV	30	//電池回生(PV有)
#define OP_PATTERN_BAT_TO_ISLAND_GEN	32	//電池→自立発電
/* システム状態(Input Registers) */
#define SYSTEM_STATE_RUN				6	//運転中
/* エラーコード(Input Registers) */
#define INPUTREG_ERCD_NOERROR			0	//異常なし
#define INPUTREG_ERCD_EMGSTOP			32	//非常停止
/* PCS1(I_DENCON)HOLDING REGISTERS(EMS⇒I_DENCON)初期化テーブル定義 */
static const HOLDING_REGISTERS modbustcp_holding_init = {
	
	.ev_chg_power		={.val = 0, .min = 0, .max = 10000},					//EV充電電力
	.ev_dischg_power	={.val = 0, .min = 0, .max = 10000},					//EV放電電力
	.bat_chg_power		={.val = 0, .min = 0, .max = 10000},					//電池充電電力
	.bat_dischg_power	={.val = 0, .min = 0, .max = 10000},					//電池放電電力
	.grid_in_power		={.val = 0, .min = 0, .max = 10000},					//系統入力電力
	.grid_out_power		={.val = 0, .min = 0, .max = 10000},					//系統出力電力
	.ev_soc_max			={.val = 0, .min = 0, .max = 100  },					//EV充電上限SOC
	.ev_soc_min			={.val = 0, .min = 0, .max = 100  },					//EV充電下限SOC
	.bat_soc_max		={.val = 0, .min = 0, .max = 100  },					//電池充電上限SOC
	.bat_soc_min		={.val = 0, .min = 0, .max = 100  },					//電池充電下限SOC
	.ope_mode			={.val = 0, .min = 0, .max = 61   },					//運転パターン
	.error_reset		={.val = 0, .min = 0, .max = 1    },					//エラーリセット
	.evconnnected		={.val = 0, .min = 0, .max = 1    },					//EV接続/切断
	.yobi				={.val = 0, .min = 0, .max = 0    }						//空き
	
};
/* PCS1(I_DENCON)INPUT REGISTERS(I_DENCON→EMS)初期化テーブル定義 */
static const INPUT_REGISTERS modbustcp_input_init = {
	.heartbeat				={.val = 0, .min = 0,      .max = 255},				//ハートビート
	.system_state			={.val = 0, .min = 0,      .max = 0  },				//システム状態
	.grid_state				={.val = 0, .min = 0,      .max = 0  },				//系統状態
	.ev_state				={.val = 0, .min = 0,      .max = 0  },				//EV状態
	.ev_connected			={.val = 0, .min = 0,      .max = 0  },				//EV接続
	.pv_state				={.val = 0, .min = 0,      .max = 0  },				//PV状態
	.bat_state				={.val = 0, .min = 0,      .max = 0  },				//電池状態
	.error_code				={.val = 0, .min = 0,      .max = 0  },				//エラーコード
	.internal_temp1			={.val = 0, .min = -50,    .max = 125},				//内部温度1
	.internal_temp2			={.val = 0, .min = -50,    .max = 125},				//内部温度2
	.internal_temp3			={.val = 0, .min = -50,    .max = 125},				//内部温度3
	.internal_temp4			={.val = 0, .min = -50,    .max = 125},				//内部温度4
	.internal_temp5			={.val = 0, .min = -50,    .max = 125},				//内部温度5
	.enclosure_temp			={.val = 0, .min = -50,    .max = 125},				//筐体温度
	.bat_temp_max			={.val = 0, .min = -50,    .max = 125},				//電池最高温度
	.bat_temp_avg			={.val = 0, .min = -50,    .max = 125},				//電池平均温度
	.bat_temp_min			={.val = 0, .min = -50,    .max = 125},				//電池最低温度
	.grid_v_rs				={.val = 0, .min = 0,      .max = 9990},			//系統RS相電圧
	.grid_v_st				={.val = 0, .min = 0,      .max = 9990},			//系統ST相電圧
	.grid_v_tr				={.val = 0, .min = 0,      .max = 9990},			//系統TR相電圧
	.grid_i_r				={.val = 0, .min = -1000,  .max = 1000},			//系統R相電流
	.grid_i_s				={.val = 0, .min = -1000,  .max = 1000},			//系統S相電流
	.grid_i_t				={.val = 0, .min = -1000,  .max = 1000},			//系統T相電流
	.grid_pf_r				={.val = 0, .min = -10000, .max = 10000},			//系統R相力率
	.grid_pf_s				={.val = 0, .min = -10000, .max = 10000},			//系統S相力率
	.grid_pf_t				={.val = 0, .min = -10000, .max = 10000},			//系統T相力率
	.grid_freq				={.val = 0, .min = 0,      .max = 8000},			//系統周波数
	.grid_p_active			={.val = 0, .min = -10000, .max = 10000},			//系統有効電力
	.grid_p_reactive		={.val = 0, .min = -10000, .max = 10000},			//系統無効電力
	.ev_voltage				={.val = 0, .min = 0,      .max = 9990},			//EV電圧
	.ev_current				={.val = 0, .min = -1000,  .max = 1000},			//EV電流
	.ev_soc					={.val = 0, .min = 0,      .max = 1000},			//EV SOC
	.pv_voltage				={.val = 0, .min = 0,      .max = 9990},			//PV電圧
	.pv_current				={.val = 0, .min = 0,      .max = 1000},			//PV電流
	.bat_voltage			={.val = 0, .min = 0,      .max = 9990},			//電池電圧
	.bat_current			={.val = 0, .min = -1000,  .max = 1000},			//電池電流
	.bat_soc				={.val = 0, .min = 0,      .max = 1000},			//電池SOC
	.ev_chg_power			={.val = 0, .min = 0,      .max = 10000},			//EV充電電力
	.ev_dischg_power		={.val = 0, .min = 0,      .max = 10000},			//EV放電電力
	.bat_chg_power			={.val = 0, .min = 0,      .max = 10000},			//電池充電電力
	.bat_dischg_power		={.val = 0, .min = 0,      .max = 10000},			//電池放電電力
	.grid_input_power		={.val = 0, .min = 0,      .max = 10000},			//系統入力電力
	.grid_output_power		={.val = 0, .min = 0,      .max = 10000},			//系統出力電力
	.ev_soc_chg_limit		={.val = 0, .min = 0,      .max = 100},				//EV充電上限SOC
	.ev_soc_dischg_limit	={.val = 0, .min = 0,      .max = 100},				//EV充電下限SOC
	.bat_soc_chg_limit		={.val = 0, .min = 0,      .max = 100},				//電池充電上限SOC
	.bat_soc_dischg_limit	={.val = 0, .min = 0,      .max = 100},				//電池放電下限SOC
	.ope_mode				={.val = 0, .min = 0,      .max = 61},				//運転パターン
	.grid_energy_in_wh		={.val = 0, .min = 0,      .max = 999999999},		//系統入力電力量積算
	.grid_energy_out_wh		={.val = 0, .min = 0,      .max = 999999999},		//系統出力電力量積算
	.ev_energy_chg_wh		={.val = 0, .min = 0,      .max = 999999999},		//EV充電電力量積算
	.ev_energy_dischg_wh	={.val = 0, .min = 0,      .max = 999999999},		//EV放電電力量積算
	.bat_energy_chg_wh		={.val = 0, .min = 0,      .max = 999999999},		//電池充電電力量積算
	.bat_energy_dischg_wh	={.val = 0, .min = 0,      .max = 999999999},		//電池放電電力量積算
	.pv_energy_wh			={.val = 0, .min = 0,      .max = 999999999}		//PV電力量積算
	
};
/********************************************************************************/
/*		CANタスク用定義															*/
/********************************************************************************/
/* CANチャンネル定義 */
#define CH_NO			0					//CANチャンネル定義(CH0)
#define MBOX_MODE		0					//メールボックスモード(通常モード0固定)
/* CAN送信メールボックス */
#define CANBOX_TX_MAX	4					//送信BOX総数
#define CANBOX_TX0		0x00				//(汎用)
#define CANBOX_TX1		0x01				//(汎用)
#define CANBOX_TX2		0x02				//(汎用)
#define CANBOX_TX3		0x03				//(汎用)
/* CAN受信メールボックス */
#define CANBOX_RX_MAX	3					//受信BOX総数
#define CANBOX_RX0		0x06				//0x1AF0CAE0 外部コントロール(受信ID固定)
#define CANBOX_RX1		0x07				//0x1AF0C0F0 発電機AVR基盤より受信(受信ID固定)
#define CANBOX_RX2		0x08				//0x1AF1C100 発電機表示基盤より受信(受信ID固定)
/* CAN0 ボーレート設定 */
/* ボーレート250kbps,samplepoint85% */
#define CAN0_BRP		(12)				//BRP
#define CAN0_SJW		(2)					//SJW
#define CAN0_TSEG1		(16)				//TSEG1
#define CAN0_TSEG2		(3)					//TSEG2
/* CAN受信データ移動平均データ数 */
#define AVG_ITEM_COUNT			5			//移動平均対象項目(5種類のデータ)
#define AVG_NUM					5			//移動平均データ数(過去5回分のデータを平均化する)
#define INDX_ENGINE_RPM			0			//エンジン回転数 indexNo.
#define INDX_GEN_VOLT_MEAS		1			//発電機電圧測定値 indexNo.
#define INDX_GEN_VOLT_TARGET	2			//発電機電圧目標値 indexNo.
#define INDX_GEN_CURR_R			3			//発電機電流R相 indexNo.
#define INDX_GEN_CURR_T			4			//発電機電流T相 indexNo.
/* CAN送信ID No.定義 */
#define CAN_SNDID_BATT1				0		//0x1AF0CA01 電池関連(そのまま)
#define CAN_SNDID_BATT2				1		//0x1AF0CA02 電池関連(充電、放電等)
#define CAN_SNDID_BT_TEMP			2		//0x1AF0CA03 温度(電池)
#define CAN_SNDID_TEMP				3		//0x1AF0CA04 温度(内部)
#define CAN_SNDID_KT1				4		//0x1AF0CA05 系統関連1(系統RS,ST,TR電圧,R電流)
#define CAN_SNDID_KT2				5		//0x1AF0CA06 系統関連2(系統S,Y電流,R,S相力率)
#define CAN_SNDID_KT3				6		//0x1AF0CA07 系統関連3(系統T相力率,有効無効電力)
#define CAN_SNDID_KT4				7		//0x1AF0CA08 系統関連4(系統入出力電力指令値)
#define CAN_SNDID_PV				8		//0x1AF0CA09 PV関連
#define CAN_SNDID_EV1				9		//0x1AF0CA0A EV関連1(EV電圧流 実測値,充放電力 指令値)
#define CAN_SNDID_EV2				10		//0x1AF0CA0B EV関連2(EV充電上下限SOC指令値,EV車載蓄電池SOC)
#define CAN_SNDID_W1				11		//0x1AF0CA0C 積算電力(EV充電電力量積算)
#define CAN_SNDID_W2				12		//0x1AF0CA0D 積算電力(電池充電積算)
#define CAN_SNDID_W3				13		//0x1AF0CA0E 積算電力(PV電力量積算)
#define CAN_SNDID_EMS_TO_GEN		14		//0x1AF0CA20 EMS→発電機(発電制御電圧)
#define CAN_SNDID_STATE_SYS			15		//0x1AF0CA40 状態(システム状態)
#define CAN_SNDID_STATE_SOFTVER		16		//0x1AF0CA41 状態(ソフトバージョン)
#define CAN_SNDID_ERRCODE			17		//0x1AF0CA50 エラーコード
#define CAN_SNDID_PCS1				18		//0x1AF0CAF0 その他山洋PCS(PV,IV実測値)
#define CAN_SNDID_PCS2				19		//0x1AF0CAF1 その他山洋PCS(PV積算発電量)
#define CAN_SNDID_PCS3				20		//0x1AF0CAF2 その他山洋PCS(日射,気温)
#define CAN_SNDID_PCS4				21		//0x1AF0CAF3 その他山洋PCS(系統RS,ST,TR_電圧)
#define CAN_SNDID_PCS5				22		//0x1AF0CAF4 その他山洋PCS(系統R,S,T電流)
#define CAN_SNDID_OTHER_PV1			23		//0x1AF0CAF5 その他合算PV(PV,IV実測値)
#define CAN_SNDID_OTHER_PV2			24		//0x1AF0CAF6 その他合算PV(PV電力 積算)
#define CAN_SNDID_OTHER_DATA1		25		//0x1AF0CAF7 その他データ(CO2削減量,サービスコード)
#define CAN_SNDID_OTHER_DATA2		26		//0x1AF0CAF8 その他データ(予備データ1-4)
#define CAN_SNDID_DEBUG1			27		//0x1AF0CAE0 デバック用外部コントロール
#define CAN_SNDID_DEBUG2			28		//0x1AF0CAE1 デバック用外部コントロール(空白)
/* CAN受信ID No.定義 */
#define CAN_RCVID_GAIBUCTRL			0		//0x1AF0CAE0 外部コントロール
#define CAN_RCVID_GENAVR			1		//0x1AF0C0F0 発電機AVR基盤より受信
#define CAN_RCVID_GENKIBAN			2		//0x1AF1C100 発電機表示基盤より受信

/* CAN送信テーブル定義 */
static const CAN_MESSAGE can_sndmsg_init[CAN_SNDID_MAX]=
{
	{/* 電池関連1 */
	.id = 0x1AF0CA01,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0   },{.uh=0},{.uh=0},{.uh=3000}},	//電池電圧
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=3000},{.uh=0},{.uh=0},{.uh=6000}},	//電池電流
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0   },{.uh=0},{.uh=0},{.uh=3000}},	//電池SOC
			{CAN_TYPE_UH,{.uh=0},{.uh=100},{.uh=300 },{.uh=0},{.uh=0},{.uh=600 }}	//電池電力
		}
	},
	{/* 電池関連2 */
	.id = 0x1AF0CA02,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=30000},{.uh=0},{.uh=0},{.uh=60000}},	//電池充電電力 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=30000},{.uh=0},{.uh=0},{.uh=60000}},	//電池放電電力 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0    },{.uh=0},{.uh=0},{.uh=3000 }},	//電池充電上限SOC 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0    },{.uh=0},{.uh=0},{.uh=3000 }}	//電池放電下限SOC 指令値
		}
	},
	{/* 温度1 */
	.id = 0x1AF0CA03,
	.entry_count = 8,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//電池最高温度
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//電池平均温度
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//電池最低温度
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//電池温度
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }}	//未使用
		}
	},
	{/* 温度2 */
	.id = 0x1AF0CA04,
	.entry_count = 8,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//内部温度1
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//内部温度2
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//内部温度3
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//内部温度4
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//内部温度5
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=50},{.ub=0},{.ub=0},{.ub=178}},	//筐体温度
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }}	//未使用
		}
	},
	{/*系統関連1*/
	.id = 0x1AF0CA05,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統RS相電圧
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統ST相電圧
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統TR相電圧
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}}	//系統R相電流
		}
	},
	{/*系統関連2*/
	.id = 0x1AF0CA06,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統S相電流
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統T相電流
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統R相力率
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}}	//系統S相力率
		}
	},
	{/*系統関連3*/
	.id = 0x1AF0CA07,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//系統T相力率
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000 }},	//系統周波数
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//系統有効電力
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=0},{.uh=0},{.uh=0},{.uh=30000}}	//系統無効電力
		}
	},
	{/*系統関連4*/
	.id = 0x1AF0CA08,
	.entry_count = 6,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//系統入力電力 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//系統出力電力 指令値
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0    }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0    }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0    }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0    }}	//未使用
		}
	},
	{/*PV関連*/
	.id = 0x1AF0CA09,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//PV電圧 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//PV電流 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=100},{.uh=0},{.uh=0},{.uh=0},{.uh=300 }},	//PV電力 計算値
			{CAN_TYPE_UB,{.ub=0},{.ub=0  },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0  },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*EV関連1*/
	.id = 0x1AF0CA0A,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000 }},	//EV電圧 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000 }},	//EV電流 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//EV充電電力 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=0},{.uh=0},{.uh=0},{.uh=30000}}	//EV放電電力 指令値
		}
	},
	{/*EV関連2*/
	.id = 0x1AF0CA0B,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//EV充電上限SOC 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//EV放電下限SOC 指令値
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=100 }},	//EV車載蓄電池のSOC
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*積算電力1*/
	.id = 0x1AF0CA0C,
	.entry_count = 2,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UW,{.uw=0},{.uw=1},{.uw=0},{.uw=0},{.uw=0},{.uw=30000}},	//EV充電電力量積算
			{CAN_TYPE_UW,{.uw=0},{.uw=1},{.uw=0},{.uw=0},{.uw=0},{.uw=30000}}	//EV放電電力量積算
		}
	},
	{/*積算電力2*/
	.id = 0x1AF0CA0D,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UW,{.uw=0},{.uw=10},{.uw=0},{.uw=0},{.uw=0},{.uw=30000}},	//電池充電電力量積算
			{CAN_TYPE_UW,{.uw=0},{.uw=10},{.uw=0},{.uw=0},{.uw=0},{.uw=30000}}	//電池放電電力量積算
		}
	},
	{/*積算電力3*/
	.id = 0x1AF0CA0E,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UW,{.uw=0},{.uw=10},{.uw=0},{.uw=0},{.uw=0},{.uw=30000}},	//PV電力量積算
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*EMS→発電機*/
	.id = 0x1AF0CA20,
	.entry_count = 7,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=450}},	//発電制御電圧
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0  }}		//未使用
		}
	},
	{/*状態1*/
	.id = 0x1AF0CA40,
	.entry_count = 7,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//システム状態
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//系統状態
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//EV状態
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//EV接続
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//PV状態
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//電池状態
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}}	//運転パターン 指令値
		}
	},
	{/*状態2*/
	.id = 0x1AF0CA41,
	.entry_count = 7,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=255  }},	//未使用
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}}	//ソフトウェアバージョン(1.00なら100)
		}
	},
	{/*エラーコード*/
	.id = 0x1AF0CA50,
	.entry_count = 6,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//エラーコード
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//エラーコード
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*その他山洋PCS1*/
	.id = 0x1AF0CAF0,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//PV電圧 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//PV電流 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=100},{.uh=0},{.uh=0},{.uh=0},{.uh=300 }},	//PV電力 計算値
			{CAN_TYPE_UB,{.ub=0},{.ub=0  },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0  },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*その他山洋PCS2*/
	.id = 0x1AF0CAF1,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UW,{.uw=0},{.uw=10},{.uw=0},{.uw=0},{.uw=0},{.uw=99999999}},	//PV電力 積算発電量
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }}	//未使用
		}
	},
	{/*その他山洋PCS3*/
	.id = 0x1AF0CAF2,
	.entry_count = 6,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1000},{.uh=0 },{.uh=0},{.uh=0},{.uh=1  }},	//日射強度 
			{CAN_TYPE_UH,{.uh=0},{.uh=10  },{.uh=50},{.uh=0},{.uh=0},{.uh=130}},	//気温
			{CAN_TYPE_UB,{.ub=0},{.ub=0   },{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0   },{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0   },{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0   },{.ub=0 },{.ub=0},{.ub=0},{.ub=0  }}		//未使用
		}
	},
	{/*その他山洋PCS4*/
	.id = 0x1AF0CAF3,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統RS相 電圧 
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統ST相 電圧
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統TR相 電圧
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*その他山洋PCS5*/
	.id = 0x1AF0CAF4,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統R相 電流
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統S相 電流
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//系統T相 電流
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*その他合算PV1*/
	.id = 0x1AF0CAF5,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//PV電圧 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=10 },{.uh=0},{.uh=0},{.uh=0},{.uh=3000}},	//PV電流 実測値
			{CAN_TYPE_UH,{.uh=0},{.uh=100},{.uh=0},{.uh=0},{.uh=0},{.uh=300 }},	//PV電力 計算値
			{CAN_TYPE_UB,{.ub=0},{.ub=0  },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0  },{.ub=0},{.ub=0},{.ub=0},{.ub=0   }}	//未使用
		}
	},
	{/*その他合算PV2*/
	.id = 0x1AF0CAF6,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UW,{.uw=0},{.uw=10},{.uw=0},{.uw=0},{.uw=0},{.uw=99999999}},	//PV電圧 積算発電量
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0       }}	//未使用
		}
	},
	{/*その他データ1*/
	.id = 0x1AF0CAF7,
	.entry_count = 6,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=0},{.uh=0},{.uh=0},{.uh=3000 }},	//CO2削減量
			{CAN_TYPE_UH,{.uh=0},{.uh=1 },{.uh=0},{.uh=0},{.uh=0},{.uh=30000}},	//サービスコード
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0    }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0    }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0    }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0 },{.ub=0},{.ub=0},{.ub=0},{.ub=0    }}	//未使用
		}
	},
	{/*その他データ2*/
	.id = 0x1AF0CAF8,
	.entry_count = 4,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=3000},{.uh=0},{.uh=0},{.uh=6000}},	//予備データ1 
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=3000},{.uh=0},{.uh=0},{.uh=6000}},	//予備データ2
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=3000},{.uh=0},{.uh=0},{.uh=6000}},	//予備データ3
			{CAN_TYPE_UH,{.uh=0},{.uh=10},{.uh=3000},{.uh=0},{.uh=0},{.uh=6000}}	//予備データ4
		}
	},
	{/*デバッグ用外部コントロール1*/
	.id = 0x1AF0CAE0,
	.entry_count = 8,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//発電機始動停止
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//運転モード指令
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//充電器電流セット(x10)
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//発電機電流制限
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//充電器の充電モード
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//CANデバッグモード
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}}		//回生充電禁止

		}
	},
	{/*デバッグ用外部コントロール2*/
	.id = 0x1AF0CAE1,
	.entry_count = 8,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0}}		//未使用
		}
	}

};

/* CAN受信テーブル定義*/
static const CAN_MESSAGE can_rcvmsg_init[CAN_RCVID_MAX]=
{

	{/* 外部コントロール */
	.id = 0x1AF0CAE0,
	.entry_count = 8,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//発電機始動停止
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//運転モード指令
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//充電器電流リセット
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//発電機電流制限
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//充電器の充電モード
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}},		//CANデバッグモード
			{CAN_TYPE_UB,{.ub=0},{.ub=1},{.ub=0},{.ub=0},{.ub=0},{.ub=0}}		//回生充電禁止
		}
	},
	{/* 発電機AVR基盤より受信 */
	.id = 0x1AF0C0F0,
	.entry_count = 5,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=8000}},	//エンジン回転数
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=1000}},	//発電電圧測定値
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=1000}},	//発電電圧目標値
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0	  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0	  }}	//未使用
		}
	},
	{/* 発電機表示基盤より受信 */
	.id = 0x1AF1C100,
	.entry_count = 6,
	.table = {//【type】【data】【分解能(1/x)】【offset(abs)】【default】【min(+abs(offset))】【max(+abs(offset))】
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=5000}},	//発電R相電流
			{CAN_TYPE_UH,{.uh=0},{.uh=1},{.uh=0},{.uh=0},{.uh=0},{.uh=5000}},	//発電T相電流
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0	  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0	  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0	  }},	//未使用
			{CAN_TYPE_UB,{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0},{.ub=0	  }}	//未使用
		}
	}
};
