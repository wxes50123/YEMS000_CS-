//******************************************************************************
// @(R)   
//		AIタスク
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.**.**	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include	<machine.h>															//割込みレベル操作用
#include	"g_include_def.h"													//共通インクルードファイル
//******** ファイル内環境宣言部 ************************************************
//定義無し
//******** 定数定義部 **********************************************************
/* AN000変換用定数 */
#define ADC_MIN_TEMP	(UH)0x028F												//0.48V⇒-50.0℃
#define ADC_MAX_TEMP	(UH)0x0CCC												//2.4V⇒50.0℃
#define ADC_TEMP_SPAN	(W)(ADC_MAX_TEMP - ADC_MIN_TEMP)						//2621
#define TEMP_MIN_OUT	0														//最低温度+OFFSET(単位:0.1℃)
#define TEMP_MAX_OUT	1000													//最高温度+OFFSET(単位:0.1℃)
#define TEMP_SPAN_DECI	1000													//温度範囲(-50℃～50℃幅)(単位:0.1℃)
#define TEMP_OFFSET_DECI 500													//温度オフセット(単位:0.1℃)
/* AN001変換用定数 */
#define ADC_MIN_SOLAR	(UH)0x028F												//0.48V⇒0kW/m^2
#define ADC_MAX_SOLAR	(UH)0x0CCC												//2.4V⇒1.43kW/m^2
#define ADC_SOLAR_SPAN	(W)(ADC_MAX_SOLAR - ADC_MIN_SOLAR)						//2621
#define SOLAR_MIN_WM2	(0)														//最低日射量(単位:0.001W/m^2)
#define SOLAR_MAX_WM2	(1430)													//最高日射量(単位:0.001W/m^2)
//******** 変数定義部 **********************************************************
static UB ad_timout_flg = FLG_OFF;												//AD変換待ちタイムアウトフラグ
//******** プロトタイプ宣言部 **************************************************
TASK ai_task(void);																//AIタスク
void ai_data_store(void);														//AIデータ格納処理
UH adc_to_temp(UH adval);														//AD値→温度変換処理
UH adc_to_solar(UH adval);														//AD値→日射量変換処理
//******** 関数定義部 **********************************************************
TASK ai_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	
	Pout_s("AI_TASK_RUN");														//デバッグ情報
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_AI_FLG, &status);
		
		/* イベント */
		if (status & F_AI_EVENT) {												//10s周期起動
			
			AD_ADST= 1;															//AD変換スタート

			EntryTimer10ms(AD_TIMOUT, (ID)ID_AI_FLG , (FLGPTN)F_AI_TMO_EVENT);	//タイマーセット(100ms)
			while(AD_ADST != 0){												//AD変換完了まで待ち
				
				/* AD変換待ち */
				if(ad_timout_flg == FLG_ON){									//タイムアウト時は待ちを抜ける
					ad_timout_flg = FLG_OFF;
					break;
				}
				
			}
			
			if(AD_ADST == 0){													//AD変換完了
				
				CancelTimer10ms((ID)ID_AI_FLG, (FLGPTN)F_AI_TMO_EVENT);			//タイマーキャンセル(100ms)
				ai_data_store();												//AIデータ格納処理
				
			}
			
		}
		/* AD変換待ちタイムアウトイベント */
		if (status & F_AI_TMO_EVENT) {											//AD変換待ちタイムアウトイベント
			
			ad_timout_flg = FLG_ON;												//タイムアウト
			
		}
	}
}
//******************************************************************************
//  機能     : AIデータ格納処理
//  機能説明 : AD変換データを格納する。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ai_data_store(void){
	
	/* AD変換値取り込み */
	g_ai[0] = adc_to_temp((UH)S12AD.ADDR0);										//AN000(気温)
	g_ai[1] = adc_to_solar((UH)S12AD.ADDR1);									//AN001(日射量)
	g_ai[2] = (UH)S12AD.ADDR2;													//AN002
	g_ai[3] = (UH)S12AD.ADDR3;													//AN003
	g_ai[4] = (UH)S12AD.ADDR4;													//AN004
	g_ai[5] = (UH)S12AD.ADDR5;													//AN005
	
}
//******************************************************************************
//  機能     : AD値→温度変換処理
//  機能説明 : AD値を温度データに変換して返す。
//           : 英弘精機(MT-052-A)用関数
//           : -50℃⇒0x28F(AD値)⇒0(温度データ)
//           :   0℃⇒0x7AD(AD値)⇒500(温度データ)
//           :  50℃⇒0xCCC(AD値)⇒1000(温度データ)
//------------------------------------------------------------------------------
//  引数     : UH adval : AD値
//------------------------------------------------------------------------------
//  戻り値   : UH temp_deci : 温度データ(単位0.1℃、オフセット加算済みデータ)
//******************************************************************************
UH adc_to_temp(UH adval){														//AD値温度変換処理
	
	/*変数定義*/
	W wk;			//AD値ワーク
	W num;			
	W temp_deci;
	
	wk = (UW)adval;
	
	/* 範囲外は飽和 */
	if(wk <= (W)ADC_MIN_TEMP){return (UH)TEMP_MIN_OUT;}							//下限値以下は-50.0℃+OFFSET⇒0
	if(wk >= (W)ADC_MAX_TEMP){return (UH)TEMP_MAX_OUT;}							//上限値以上は50.0℃+OFFSET⇒1000
	
	/* 直線補間 */
	num = (wk -(W)ADC_MIN_TEMP)*(W)TEMP_SPAN_DECI;
	temp_deci = (num + (ADC_TEMP_SPAN/2)) / ADC_TEMP_SPAN;						//丸め処理
	
	/* 値チェック */
	if(temp_deci < TEMP_MIN_OUT){temp_deci = TEMP_MIN_OUT;}						//下限値より下は-50.0℃+OFFSET⇒0
	if(temp_deci > TEMP_MAX_OUT){temp_deci = TEMP_MAX_OUT;}						//上限値より上は50.0℃+OFFSET⇒1000
	
	return (UH)temp_deci;
}
//******************************************************************************
//  機能     : AD値→日射量変換処理
//  機能説明 : AD値を日射量データに変換して返す。
//           : 英弘精機(MS-40A)用関数
//           : 0kW/m^2⇒0x28F(AD値)⇒0W/m^2(日射データ)
//           : 0.715kW/m^2⇒0x7AD(AD値)⇒715W/m^2(日射データ)
//           : 1.43kW/m^2⇒0xCCC(AD値)⇒1430W/m^2(日射データ)
//------------------------------------------------------------------------------
//  引数     : UH adval : AD値
//------------------------------------------------------------------------------
//  戻り値   : UH solar_wm2 : 日射データ(単位W/m^2)
//******************************************************************************
UH adc_to_solar(UH adval){														//AD値→日射量変換処理
	
	/*変数定義*/
	W wk;			//AD値ワーク
	W num;			
	W solar_wm2;
	
	wk = (UW)adval;
	
	/* 範囲外は飽和 */
	if(wk <= (W)ADC_MIN_SOLAR){return (UH)SOLAR_MIN_WM2;}						//下限値以下は0
	if(wk >= (W)ADC_MAX_SOLAR){return (UH)SOLAR_MAX_WM2;}						//上限値以上1430
	
	/* 直線補間 */
	//solar_wm2 = (ad - ADMIN)*1430/2621
	num = (wk -(W)ADC_MIN_SOLAR)*(W)SOLAR_MAX_WM2;
	solar_wm2 = (num + (ADC_SOLAR_SPAN/2)) / ADC_SOLAR_SPAN;					//丸め処理
	
	/* 値チェック */
	if(solar_wm2 < SOLAR_MIN_WM2){solar_wm2 = SOLAR_MIN_WM2;}					//下限値より下は⇒0
	if(solar_wm2 > SOLAR_MAX_WM2){solar_wm2 = SOLAR_MAX_WM2;}					//上限値より上は⇒1430
	
	return (UH)solar_wm2;
}
