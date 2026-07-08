//******************************************************************************
// @(R)   
//		タイマータスク
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.**.**	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>															//割込みレベル操作用
#include "g_include_def.h"														//共通インクルードファイル
#include <string.h>																//memsetを使用
//******** 定数定義部 **********************************************************
#define DTMR_NUM_BUFFER			0x20	//DMTRタイマサービスバッファ数
#define DTMR_NO_USE				0x00	//バッファ未使用ステータス
#define DTMR_USED				0x01	//バッファ使用中ステータス
/* タイマー関数エラーコード */
#define RTN_BUF_FULL			0x01	//タイマー用バッファが満杯
#define RTN_PARAM_ERR			0x02	//パラメータエラー
//******** ファイル内環境宣言部 ************************************************
static struct DTMR_INF_10MS	{													//タイマー情報
	UW		interval_time	;													//設定時間(10ms単位)
	ID	 	evt_id			;													//通知するイベントID
	FLGPTN	bitptn			;													//通知するイベントのビットパターン
	UW		leftl_time		;													//残り時間(10ms単位)
	UH		status			;													//ステータス
} DTMR_inf_10ms[DTMR_NUM_BUFFER];
//******** 変数定義部 **********************************************************

//******** プロトタイプ宣言部 **************************************************
TASK timer_task(void);															//タイマータスク
void DTMR_Init(void);															//タイマードライバ初期化
static void DTMR_TimCountDown10ms(void);										//タイマードライバカウントダウン処理(10ms)
ER EntryTimer10ms(UW interval_time, ID evt_id,  UINT bitptn);					//タイマー登録
ER CancelTimer10ms(ID evt_id, UINT bitptn);										//タイマーキャンセル
void get_rtc_time(void);														//RTC時刻取得
//******** 関数定義部 **********************************************************
TASK timer_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	static UB count_20ms = 0;													//20msイベントカウンタ
	static UB count_100ms = 0;													//100msイベントカウンタ
	UB blink_cnt = 0;															//メンテナンスSW点滅制御用カウンタ
	UB cleaning_cnt = 0;														//接点クリーニング制御用カウンタ
	int ipl;																	//割り込み
	
	Pout_s("TIMER_TASK_RUN");
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_TIMER_FLG, &status);
		
		/* 10msイベント */
		if (status & BIT_TIMER_10MSEC) {										//10ms周期起動
			
			/* タイマーカウントダウン処理 */
			ipl=get_ipl();														//割込み禁止
			set_ipl(15);
			DTMR_TimCountDown10ms();											//カウントダウン処理(10ms)
			set_ipl(ipl);														//割込み禁止解除
			
			/* メインタスク起動イベント */
			count_20ms ^= 1;
			if(count_20ms == 0){
				set_flg((ID)ID_MAIN_FLG, (FLGPTN)F_MAIN_EVENT);					//メインタスク起動イベントセット
			}
		}
		/* 100msイベント */
		if (status & BIT_TIMER_100MSEC) {										//100ms周期起動
			
			count_100ms++;														//100msカウント
			
			/* 発電機電圧調整 */
			switch(g_gen.voltage_ajust){
				
				case GEN_VOL_ADJUST_KEEP:										//現電圧維持(送信周期はCANタスク起動周期と同じ)
					if(count_100ms == (GEN_VOL_ADJUST_TIME1000MS/100)){
						
						g_gen.voltage_set = g_gen.avr_volt_meas;				//発電制御電圧設定
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				case GEN_VOL_ADJUST_KEEP_300V:									//電圧300V維持(送信周期はCANタスク起動周期と同じ)
					if(count_100ms == (GEN_VOL_ADJUST_TIME1000MS/100)){
						
						g_gen.voltage_set = 300;								//発電制御電圧設定
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				case GEN_VOL_ADJUST_KEEP_280V:									//電圧280V維持(送信周期はCANタスク起動周期と同じ)
					if(count_100ms == (GEN_VOL_ADJUST_TIME1000MS/100)){
						
						g_gen.voltage_set = 280;								//発電制御電圧設定
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				case GEN_VOL_ADJUST_KEEP_250V:									//電圧250V維持(送信周期はCANタスク起動周期と同じ)
					if(count_100ms == (GEN_VOL_ADJUST_TIME1000MS/100)){
						
						g_gen.voltage_set = 250;								//発電制御電圧設定
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				case GEN_VOL_ADJUST_15S1VUP:									//15秒ごとに1VUP
					if(count_100ms == (GEN_VOL_ADJUST_TIME15000MS/100)){
						
						g_gen.voltage_set = g_gen.avr_volt_meas + 1;			//発電制御電圧設定+1V
						if(g_gen.voltage_set >= 450){g_gen.voltage_set = 450;}	
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				case GEN_VOL_ADJUST_2S1VUP:										//2秒ごとに1VUP
					if(count_100ms == (GEN_VOL_ADJUST_TIME2000MS/100)){
						
						g_gen.voltage_set = g_gen.avr_volt_meas + 1;			//発電制御電圧設定+1V
						if(g_gen.voltage_set >= 450){g_gen.voltage_set = 450;}	
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				case GEN_VOL_ADJUST_01SVDOWN:									//0.1秒ごとに1VDOWN
					if(count_100ms == (GEN_VOL_ADJUST_TIME100MS/100)){
						
						g_gen.voltage_set = g_gen.avr_volt_meas - 1;			//発電制御電圧設定-1V
						if(g_gen.voltage_set <= 0){g_gen.voltage_set = 0;}	
						set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_VOL_CNTL_EVENT);	//CANタスクへイベント発行
						
						count_100ms = 0;										//カウンタクリア
					}
					break;
					
				default:
					/* default文処理なし */
					break;
			}
			
			
		}
		/* 500msイベント */
		if (status & BIT_TIMER_500MSEC) {										//500ms周期起動
			
			/* LiveLED制御 */
			LED4_GREEN = ~LED4_GREEN;											//LiveLED点滅(LED4)
			
			/* RTC時刻取得 */
			get_rtc_time();														//RTC時刻をグローバル変数へ読み出す
			
			/* メンテナンスSW LED点滅制御 */
			if(g_pcs1.mntsw_led_flg == (UB)MNT_BLINK1){
			
				blink_cnt++;													//カウンタカウントアップ
				
				if(blink_cnt >= (UB)MNT_BLINK1){
					
					DO_PCS_MAINTESW_LED = ~DO_PCS_MAINTESW_LED;					//メンテナンスSW LED点滅
					
					blink_cnt = 0;												//カウンタリセット
				}
				
			}else if(g_pcs1.mntsw_led_flg == (UB)MNT_BLINK2){
			
				blink_cnt++;													//カウンタカウントアップ
				
				if(blink_cnt >= (UB)MNT_BLINK2){
					
					DO_PCS_MAINTESW_LED = ~DO_PCS_MAINTESW_LED;					//メンテナンスSW LED点滅
					
					blink_cnt = 0;												//カウンタリセット
				}
				
			}else{
				blink_cnt = 0;													//カウンタリセット
			}
			
			/* 接点クリーニング制御 */
			if(g_gen.contact_cleaning_flg == FLG_ON &&							//接点クリーニングフラグがONの場合(メインタスクがFLG_ONにする)
				g_gen.off_wait_tim_flg == FLG_OFF								//発電機始動リレーOFF待機中(15秒未満)ではない場合
			){
				
				DO_EG_RY = ~DO_EG_RY;											//E/G発停リレー 接点出力ON/OFF
				cleaning_cnt++;													//カウントアップ(+500ms)
				
				if(cleaning_cnt >= (CLEANING_TIM*1000/500)){					//9sec/500ms⇒18回
					
					g_gen.contact_cleaning_flg = FLG_OFF;
					cleaning_cnt = 0;
					SET_DO(DO_EG_RY, BIT_OFF);									//発電機始動リレーOFF(必ずOFFで抜ける)
					
				}
				
			}
			
		}
	}
}
//******** 内部関数定義部 ******************************************************
//******************************************************************************
//  機能     : タイマードライバ初期化
//  機能説明 : タイマードライバを初期化する。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void DTMR_Init(void)
{
	/* 変数定義 */
	W cnt;
	struct DTMR_INF_10MS *inf10ms = DTMR_inf_10ms;
	
	for (cnt = (UB)0; cnt < (UB)DTMR_NUM_BUFFER; cnt++, inf10ms++) {
		memset(inf10ms, 0, sizeof(*inf10ms));
	}
	return;
}
//******************************************************************************
//  機能     : タイマードライバカウントダウン処理(10ms)
//  機能説明 : タイマードライバのカウントダウン処理(10ms)を行う。
//           : 残り時間0なら、指定タスクにイベントフラグをセットする。
//           : それ以外は、カウントダウンを行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
static void DTMR_TimCountDown10ms(void)
{
	/* 変数定義 */
	UW cnt;
	struct DTMR_INF_10MS *inf10ms = DTMR_inf_10ms;

	/* バッファ全体を対象に、使用中のみ処理する	*/
	for (cnt = (UB)0; cnt < (UB)DTMR_NUM_BUFFER; cnt++, inf10ms++) {
		if (inf10ms->status == (UH)DTMR_USED) {
			/* 使用中のバッファは残りタイムを減らす */
			inf10ms->leftl_time--;
			if (inf10ms->leftl_time == (UW)0) {
				/* 残り時間が0なら、イベントフラグをセット	*/
				set_flg((ID)inf10ms->evt_id, (FLGPTN)inf10ms->bitptn);
				/* イベントフラグをセット後は、未使用にする */
				inf10ms->status = (UH)DTMR_NO_USE;
			}
		}
	}
	/* 終了処理 */
	return;
}
//******************************************************************************
//  機能     : タイマー登録
//  機能説明 : タイマーの登録を行う。
//           : タスクでのみ使用可。
//           : 10msカウンタは、tsk_dtmr でカウントアップする。
//------------------------------------------------------------------------------
//  引数     : UW interval_tim : 設定時間(10ms単位)
//           : ID evt_id : 通知するイベントID
//           : FLGPTN bitptn : 通知するイベントのビットパターン
//------------------------------------------------------------------------------
//  戻り値   : ER rtcd : ER_OK : 正常終了
//           :         : RTN_BUF_FULL : タイマー用バッファが満杯
//******************************************************************************
ER EntryTimer10ms(UW interval_time, ID evt_id, FLGPTN bitptn)
{
	/* 変数定義 */
	UW cnt;
	ER rtcd;										/* リターンコード */
	int ipl;										/* 割り込みマスクレベル */
	struct DTMR_INF_10MS *inf10ms = DTMR_inf_10ms;

	/* 初期化 */
	rtcd = (ER)RTN_BUF_FULL;

	/* パラメータチェック */
	if (interval_time == (UW)0 || evt_id == (ID)0 || bitptn == (FLGPTN)0) {
		return (ER)RTN_PARAM_ERR;
	}
	/* 空きバッファをサーチ */
	for (cnt = (int)0; cnt < (int)DTMR_NUM_BUFFER; cnt++, inf10ms++) {
		if (inf10ms->status == (UH)DTMR_NO_USE) {
			rtcd = (ER)ER_OK;
			break;
		}
	}
	if (rtcd != (ER)ER_OK) {
		/* 空きバッファが見つからない場合 */
		//Pout_s("** ERROR! TIMER BUFFER NOT EMPTY ** \n\r");  // DEBUG OUT
		return rtcd;
	}

	/* バッファにエントリ */
	/* !!OSの割り禁コマンドとマイコンの割り禁コマンドを混合して使うと
	     フリーズする可能性があるため、混合して使用しない。						*/
	ipl=get_ipl();								//割込み禁止
	set_ipl(15);
	inf10ms->interval_time = interval_time;
	inf10ms->evt_id = evt_id;
	inf10ms->bitptn = bitptn;
	inf10ms->leftl_time = interval_time;
	inf10ms->status = (UH)DTMR_USED;
	set_ipl(ipl);								//割込み禁止解除
	
	/* 終了処理 */
	return rtcd;
}
//******************************************************************************
//  機能     : タイマードライバのキャンセル処理(10ms)
//  機能説明 : タイマードライバのキャンセル処理
//           : (01) 引数のフラグパターンのタイマーをキャンセル
//           : (02) 該当するフラグパターンが無い場合は何も行わない
//------------------------------------------------------------------------------
//  引数     : ID evt_id : キャンセルするイベントID
//           : FLGPTN bitptn : キャンセルするイベントのビットパターン
//------------------------------------------------------------------------------
//  戻り値   : ER num : キャンセルしたタイマー数
//******************************************************************************
ER CancelTimer10ms(ID evt_id, FLGPTN bitptn)
{
	/* 変数定義 */
	UW cnt;
	ER num;												//HIT数
	int ipl;											//割り込みマスクレベル
	struct DTMR_INF_10MS *inf10ms = DTMR_inf_10ms;

	/* 初期化 */
	num  = 0;

	/* パラメータチェック */
	if (evt_id == (ID)0 || bitptn == (FLGPTN)0) {
		return (ER)RTN_PARAM_ERR;
	}

	/* 該当するイベントIDをサーチ */
	for (cnt = (int)0; cnt < (int)DTMR_NUM_BUFFER; cnt++, inf10ms++) {
		if ((inf10ms->evt_id == (ID)evt_id) &&
		(inf10ms->bitptn == (FLGPTN)bitptn)) {
			/* HIT数を更新 */
			num++;
			ipl=get_ipl();								//割込み禁止
			set_ipl(15);
			inf10ms->status = (UH)DTMR_NO_USE;
			set_ipl(ipl);								//割込み禁止解除
		}
	}

	/* キャンセル数を返す */
	return num;
}
//******************************************************************************
//  機能     : RTC時刻取得
//  機能説明 : RTCレジスタからグローバル変数へ年月日時刻を取得する。
//           : 時刻境界で更新しないようにsec1とsec2を比較して一致した場合だけ
//           : 時刻更新を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void get_rtc_time(void)
{
	/* 変数定義 */
	UB sec1,sec2;																//RTC時刻(sec)比較用
	
	/* RTC時刻読出し */
	do{
		sec1 =        (RTC.RSECCNT.BYTE & 0x0F)  + (((RTC.RSECCNT.BYTE & 0x70)>>4)*10);				//秒更新の境界で更新しないようにsec1とsec2で比較する
		g_time.min =  (RTC.RMINCNT.BYTE & 0x0F)  + (((RTC.RMINCNT.BYTE & 0x70)>>4)*10);				//分
		g_time.hour = (RTC.RHRCNT.BYTE  & 0x0F)  + (((RTC.RHRCNT.BYTE  & 0x30)>>4)*10);				//時
		g_time.day =  (RTC.RDAYCNT.BYTE & 0x0F)  + (((RTC.RDAYCNT.BYTE & 0x30)>>4)*10);				//日
		g_time.month =(RTC.RMONCNT.BYTE & 0x0F)  + (((RTC.RMONCNT.BYTE & 0x10)>>4)*10);				//月
		g_time.year = (RTC.RYRCNT.WORD  & 0x000F)+ (((RTC.RYRCNT.WORD  & 0x00F0)>>4)*10) + 2000;	//年 ※このレジスタだけWORD
		sec2 =        (RTC.RSECCNT.BYTE & 0x0F)  + (((RTC.RSECCNT.BYTE & 0x70)>>4)*10);				//秒2
	}while(sec1 != sec2);
	g_time.sec= sec2;																				//秒1秒2一致の場合のみRTC時刻取得
	
}
