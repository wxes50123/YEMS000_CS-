//******************************************************************************
// @(R)	
//		メインタスク
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.11.10	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>															//割込みレベル操作用
#include "g_include_def.h"														//共通インクルードファイル
//******** ファイル内環境宣言部 ************************************************
typedef struct{																	//各タスク起動用カウンタ
	UH udp,tcp,can,rs485,modbus,di,ai;
}COUNTER;
//******** 定数定義部 **********************************************************
#define COUNT_20MS(x) do{(x).udp++;(x).tcp++;(x).can++;(x).rs485++;(x).modbus++;(x).di++;(x).ai++;}while(0)
//******** 変数定義部 **********************************************************

//******** プロトタイプ宣言部 **************************************************
TASK main_task(void);															//メインタスク
void generator_start_time(void);												//発電機始動時間計測
void generator_charge_time_check(void);											//発電機充電スケジュール時間判定
void generator_control_judge(void);												//発電機発停制御
void generator_state_update(void);												//発電機状態更新
void generator_start_retry(void);												//発電機始動リトライ
void generator_voltage_control(void);											//発電機電圧制御
void relay_control(void);														//リレー制御
void pcs1mode_control(void);													//PCS1動作モード制御
void led_update(void);															//LED状態更新処理
void valinfo_output(void);														//変数情報出力処理
//******** 関数定義部 **********************************************************
TASK main_task(void)
{
	/* 変数定義 */
	UINT status;														//イベントステータス
	static COUNTER cnt={0};
	
    Pout_s("MAIN_TASK_RUN");													//デバッグ情報
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_MAIN_FLG, &status);
		
		/* メインタスク起動イベント */
		if(status & F_MAIN_EVENT){
			
			/* 20msカウントアップ */
			COUNT_20MS(cnt);
			
			/* 各タスク起動 */
			/* DI起動イベント */
			if(cnt.di >= (DI_TSK_CYCLE/BASE_MS)){
				set_flg((ID)ID_DI_FLG, (FLGPTN)F_DI_EVENT);						//DIタスクへイベント発行
				cnt.di = 0;
			}
			/* UDP起動イベント */
			if(cnt.udp >= (UDP_TSK_CYCLE/BASE_MS)){
				set_flg((ID)ID_LANCLI_UDP_FLG, (FLGPTN)F_UDP_EVENT);			//UDPタスクへイベント発行
				cnt.udp = 0;
			}
			/* TCP起動イベント */
			if(cnt.tcp >= (TCP_TSK_CYCLE/BASE_MS)){
				set_flg((ID)ID_LANCLI_TCP_FLG, (FLGPTN)F_TCP_EVENT);			//TCPタスクへイベント発行
				cnt.tcp = 0;
			}
			/* CAN起動イベント */
			if(cnt.can >= (CAN_TSK_CYCLE/BASE_MS)){
				set_flg((ID)ID_CAN_FLG, (FLGPTN)F_CAN_EVENT);					//CANタスクへイベント発行
				cnt.can = 0;
			}
			/* RS485起動イベント */
			if(cnt.rs485 >= (RS485_TSK_CYCLE/BASE_MS)){
				
				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_VOLCHK_EVENT);		//RS485タスクへイベント発行
				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_CURCHK_EVENT);		//RS485タスクへイベント発行
/* RS485動作確認用 */
//				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_P_ON_EVENT);			//RS485タスクへイベント発行
//				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_REMSSET_EVENT);		//RS485タスクへイベント発行
//				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_ADDRSET_EVENT);		//RS485タスクへイベント発行
//				g_rs485.vol_set = 0;
//				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_VOL_SET_EVENT);		//RS485タスクへイベント発行
//				g_rs485.cur_set = 0;
//				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_CUR_SET_EVENT);		//RS485タスクへイベント発行
//				set_flg((ID)ID_RS485_FLG, (FLGPTN)F_RS485_P_OFF_EVENT);			//RS485タスクへイベント発行
				
				cnt.rs485 = 0;
			}
			/* Modbus起動イベント */
			if(cnt.modbus >= (MODBUS_TSK_CYCLE/BASE_MS)){
				set_flg((ID)ID_MODBUS_FLG, (FLGPTN)F_MODBUS_REQ_EVENT);			//MODBUSタスクへイベント発行
				cnt.modbus = 0;
			}
			/* AI起動イベント */
			if(cnt.ai >= (AI_TSK_CYCLE/BASE_MS)){
				set_flg((ID)ID_AI_FLG, (FLGPTN)F_AI_EVENT);						//AIタスクへイベント発行
				cnt.ai = 0;
				
			}
			/* 通常処理 */
			generator_start_time();												//発電機始動時間計測
			generator_charge_time_check();										//発電機充電スケジュール時間判定
			generator_control_judge();											//発電機発停制御
			generator_state_update();											//発電機状態更新
			generator_start_retry();											//発電機始動リトライ
			generator_voltage_control();										//発電機電圧制御
			relay_control();													//リレー制御
			//押しボタン制御はDIタスク
			pcs1mode_control();													//PCS1動作モード制御
			
			
			/* 内部情報出力処理 */
#ifdef DEBUG_VALINFO_ENABLE
			valinfo_output();													//変数情報出力処理
#endif

#ifdef WDT_ENABLE
			/* WDTクリア処理 */
			WDT_CLR;															//LOW(0x00)→HIGH(0xFF)でクリア
#endif
		}
		
		/* LED更新イベント */
		if(status & F_MAIN_LEDUPD_EVENT){
			
			led_update();														//LED更新処理
			
		}
		
		/* タイマーイベント */
		if(status & F_MAIN_GEN_STARTOFF_15SEC){g_gen.off_wait_tim_flg = FLG_OFF;}		//発電機始動フラグOFF後15秒未満フラグOFF
		if(status & F_MAIN_GEN_BREAKEROFF_10SEC){g_gen.breaker_off_tim_flg = FLG_ON;}	//発電機ブレーカ遮断判定10秒経過フラグON
		
	}
}
//******** 内部関数定義部 ******************************************************
//******************************************************************************
//  機能     : 発電機始動時間計測
//  機能説明 : 発電機の始動時間を計測する。
//           : 本関数の呼び出し1回を20ms(メインタスク起動周期)として計算する。
//           : UH(16bit)変数で計測可能な時間上限は約21分とする。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void generator_start_time(void)
{
	/*変数定義*/
	static UH time_count = 0;													//計測時間
	
	if(g_gen.start_flg == FLG_ON){
	
		if(time_count < GEN_60SEC){												//60秒未満
			
			time_count++;														//計測時間カウント(+20ms)
			g_gen.start_on_tim_flg = GEN_START_UNDER_60SEC;						//発電機始動時間フラグON後60秒未満
			
		}else if((time_count >= GEN_60SEC) && (time_count < GEN_11MIN)){		//60秒以上11分未満
			
			time_count++;														//計測時間カウント(+20ms)
			g_gen.start_on_tim_flg = GEN_START_OVER_60SEC;						//発電機始動時間フラグON後60秒以上11分未満
			
		}else{																	//11分以上
		
			g_gen.start_on_tim_flg = GEN_START_OVER_11MIN;						//発電機始動時間フラグON後11分以上
			
		}
		
	}else{
		
		time_count = 0;															//計測時間クリア
		g_gen.start_on_tim_flg = GEN_START_FLGOFF;								//発電機始動時間フラグOFF
		
	}
}
//******************************************************************************
//  機能     : 発電機充電スケジュール時間判定
//  機能説明 : 発電機の充電スケジュール時間かRTC時刻から判定する。
//           : 充電開始時間と充電終了時間はGEN_CHARGE_START_TIMEと
//           : GEN_CHARGE_END_TIMEで定義する。将来的にEEPROMに格納する。
//           : 24時間を分換算して、0～1439分の概念で判定する。
//           : 判定結果はグローバル変数(gen_charge_time_flg)に格納する。
//           : gen_charge_time_flg:ON (1)→充電スケジュール時間内
//           : gen_charge_time_flg:OFF(0)→充電スケジュール時間外
//           : 例外として、GEN_CHARGE_START_TIMEとGEN_CHARGE_END_TIMEが両方同じ場合
//           : 充電スケジュール時間外(機能無効)とする。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void generator_charge_time_check(void){
	
	/* 変数定義 */
	UH now_time;
	
	/* 開始時刻と終了時刻が同じ場合：機能無効 */
	if(GEN_CHARGE_START_TIME == GEN_CHARGE_END_TIME){
		g_gen.charge_time_flg = FLG_OFF;
		return;
	}
	
	/* 現在時刻取得(RTC時刻) */
	now_time = (g_time.hour * 60) + g_time.min;									//RTC時刻を分換算
	
	/* 開始時間 < 終了時間設定の場合(例:17:00～22:00など) */
	if(GEN_CHARGE_START_TIME < GEN_CHARGE_END_TIME){
		
		if((now_time >= GEN_CHARGE_START_TIME) && (now_time < GEN_CHARGE_END_TIME)){
			g_gen.charge_time_flg = FLG_ON;										//充電時間フラグON
		}else{
			g_gen.charge_time_flg = FLG_OFF;									//充電時間フラグOFF
		}
		
	/* 日跨ぎの場合(例:17:00～03:00など) */
	}else{
		
		if((now_time >= GEN_CHARGE_START_TIME) || (now_time < GEN_CHARGE_END_TIME)){
			g_gen.charge_time_flg = FLG_ON;										//充電時間フラグON
		}else{
			g_gen.charge_time_flg = FLG_OFF;									//充電時間フラグOFF
		}
		
	}
	
}
//******************************************************************************
//  機能     : 発電機発停制御
//  機能説明 : 発電機の発電、停止制御を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void generator_control_judge(void)												//発電機発停制御
{	
	/* PCS1運転状態が自立運転中かつシステム状態運転中か？ */
	if((g_modbustcp_input.ope_mode.val == OP_PATTERN_BAT_TO_ISLAND_GEN) &&		//標準運転パターンがNo.32:運転パターンか？
		(g_modbustcp_input.system_state.val == SYSTEM_STATE_RUN)				//システム状態がNo.6運転中か？
	){
		/* ブレーカ遮断判定2回以下か？ */
		if(g_pcs1.breaker_trip_count <= 2){
			
			/*始動判定*/
			if(g_modbustcp_input.bat_soc.val <= BAT_SOC_START_THRESHOLD){		//電池SOCが100[単位0.1%]以下か？
				
				g_gen.start_flg = FLG_ON;										//発電機始動フラグセット
				g_gen.start_cause |= GEN_START_CAUSE_SOC;						//発電機始動要因記録(SOC基準値以下)
			}
			if(g_modbustcp_input.bat_voltage.val <= BAT_VOL_START_THRESHOLD){	//電池電圧が3500[単位0.1V]以下か？
				
				g_gen.start_flg = FLG_ON;										//発電機始動フラグセット
				g_gen.start_cause |= GEN_START_CAUSE_BAT_VOL;					//発電機始動要因記録(電池電圧基準値低下)
			}
			if(g_gen.charge_time_flg == FLG_ON){								//充電スケジュール時間内か？
				
				g_gen.start_flg = FLG_ON;										//発電機始動フラグセット
				g_gen.start_cause |= GEN_START_CAUSE_TIME;						//発電機始動要因記録(スケジュール時間)
			}
			
			/* 停止判定 */
			if((g_gen.start_flg != FLG_OFF) || (g_gen.run_state == GEN_RUN)){	//発電機始動フラグOFF以外(運転状態) または 発電機運転状態
				
				if(g_modbustcp_input.bat_soc.val >= BAT_SOC_START_THRESHOLD){	//電池SOCが100[単位0.1%]以上か？
					
					g_gen.start_flg = FLG_OFF;									//発電機始動フラグクリア
					g_gen.start_cause = GEN_START_CAUSE_CLR;					//発電機始動要因クリア
				}
				/*ブレーカ復帰判定*/
				if(g_gen.start_cause == GEN_START_CAUSE_CLR){					//発電機始動要因無しか？
					
					g_gen.breaker_off_flg = FLG_OFF;							//ブレーカ遮断なし判定
					g_gen.start_flg = FLG_OFF;									//発電機始動フラグクリア
					g_gen.start_cause = GEN_START_CAUSE_CLR;					//発電機始動要因クリア
					g_pcs1.breaker_trip_count = 0;								//ブレーカ遮断判定クリア
					
				}
				
				/* ブレーカ遮断判定 */
				if((g_gen.avr_volt_meas >= g_can_rcvmsg[CAN_RCVID_GENAVR].table[1].max.uh) &&	//発電機AVR基盤より受信(発電電圧測定値)最大値以上か？
					(g_gen.disp_current_r == 0 && g_gen.disp_current_t == 0)					//発電機表示基盤より受信(R相,T相)電流が0か？
				){
					
					g_gen.breaker_off_flg = FLG_ON;												//ブレーカ遮断中判定
					g_gen.start_flg = FLG_OFF;													//発電機始動フラグクリア
					
					EntryTimer10ms(10000/MSEC, (ID)ID_MAIN_FLG , (FLGPTN)F_MAIN_GEN_BREAKEROFF_10SEC);	//タイマーセット(10s)
					
				}
				
				//ブレーカ遮断判定停止後10秒経過？
				if(g_gen.breaker_off_tim_flg == FLG_ON){
					
					g_gen.start_flg = FLG_OFF;									//発電機始動フラグクリア
					g_gen.start_cause = GEN_START_CAUSE_CLR;					//発電機始動要因クリア
					g_gen.breaker_off_tim_flg = FLG_OFF;						//ブレーカ遮断判定クリア
				}
				
				/* PCSとの通信断もしくは非常停止ボタン？ */
//				if((g_pcs1.comm_error_flg == FLG_ON) ||							//PCS通信断発生中 または
				if((g_gen.avr_msg_timout_flg == FLG_ON) ||						//AVR基板通信断発生中 または
					(g_modbustcp_input.error_code.val == INPUTREG_ERCD_EMGSTOP)	//非常停止ボタン押下中
				){
					
					g_gen.start_flg = FLG_OFF;									//発電機始動フラグクリア
					g_gen.start_cause = GEN_START_CAUSE_CLR;					//発電機始動要因クリア
					
				}
			}
		}
	}
}
//******************************************************************************
//  機能     : 発電機状態更新
//  機能説明 : 発電機状態と周波数をエンジン回転数から判定して更新する。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void generator_state_update(void)
{
	
	/* AVR基板通信タイムアウトの場合 */
	if(g_gen.avr_msg_timout_flg == FLG_ON){										//AVR基板通信タイムアウトフラグ(0:正常,1:通信断)
		
		g_gen.avr_rpm = 0;														//回転数0を設定
		
	}
	
	/* 運転中判定 */
	if(g_gen.avr_rpm >= GEN_RUN_RPM_THRESHOLD){									//回転数が700回転以上の場合
		
		g_gen.run_state = GEN_RUN;												//発電機：運転中
		
	}else{
	
		g_gen.run_state = GEN_STOP;												//発電機：停止中
		g_gen.avr_freq = 0;														//周波数：0Hz
		
		return;																	//発電機停止中の場合は処理終了
	
	}
	
	/* 周波数判定 */
	if(g_gen.avr_rpm >= GEN_RUN_FREQ_THRESHOLD){								//回転数が1600回転以上の場合
	
		g_gen.avr_freq = 60;													//周波数:60Hz
		
	}else{
	
		g_gen.avr_freq = 50;													//周波数:50Hz
		
	}
	
}
//******************************************************************************
//  機能     : 発電機始動リトライ
//  機能説明 : 発電機始動フラグ状態を時間で管理し、必要に応じて接点クリーニング
//           : を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void generator_start_retry(void)
{

	/*変数定義*/
	static UB retry = 0;														//リトライ回数
	
	if(g_gen.contact_cleaning_flg == FLG_ON){return;}							//接点クリーニング中は処理しない
	if(g_gen.off_wait_tim_flg == FLG_ON){return;}								//発電機始動リレーOFF待機中(15秒未満)は処理しない
	
	
	/* 始動フラグONかつ発電機運転中か？ */
	if((g_gen.start_flg == FLG_ON) && (g_gen.run_state == GEN_RUN)){
		
		/* 始動フラグON後60秒経過後または11分経過後か？ */
		if((g_gen.start_on_tim_flg == GEN_START_OVER_60SEC) || (g_gen.start_on_tim_flg == GEN_START_OVER_11MIN)){
			
			if(retry <= GEN_RETRY){												//リトライ回数3回以下か
				
				//接点クリーニング
				g_gen.contact_cleaning_flg = FLG_ON;							//接点クリーニングフラグON(タイマータスクがFLG_OFFにする)
				g_gen.off_wait_tim_flg = FLG_ON;								//15秒間始動リレーをOFF
				SET_DO(DO_EG_RY, BIT_OFF);										//発電機始動リレーOFFで待機
				EntryTimer10ms((OFFWAIT_TIM*1000)/MSEC, (ID)ID_MAIN_FLG , (FLGPTN)F_MAIN_GEN_STARTOFF_15SEC);	//タイマーセット(15s)
				
				retry++;														//リトライ回数加算
				
			}else{
				/* 始動フラグ11分経過後か？ */
				if(g_gen.start_on_tim_flg == GEN_START_OVER_11MIN){
				
					retry = 0;													//リトライ初期化
					g_gen.off_wait_tim_flg = FLG_ON;							//15秒間始動リレーをOFF
					SET_DO(DO_EG_RY, BIT_OFF);									//発電機始動リレーOFFで待機
					EntryTimer10ms((OFFWAIT_TIM*1000)/MSEC, (ID)ID_MAIN_FLG , (FLGPTN)F_MAIN_GEN_STARTOFF_15SEC);	//タイマーセット(15s)
					
				}
				
			}
			
		}
		
	}else{
		
		retry = 0;																//リトライ初期化
		
	}
	
}
//******************************************************************************
//  機能     : 発電機電圧制御
//  機能説明 : 発電機電圧を制御するために、各判定を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void generator_voltage_control(void)
{
	/* 変数定義 */
	static const double sqrt3 = 1.7320508075688772;	//皮相電力計算用定数
	double  gen_out_power = 0;						//発電機の出力電力
	UB voltage_adjust_flg = 0;						//電圧調整フラグ(0:そのまま,1:上げる,2:下げるを格納する)
	W  bat_chg_power = 0;							//蓄電池への充電電力
	W  bat_chg_current = 0;							//蓄電池への充電電流
	UW bat_chg_power_limit = 0;						//蓄電池の充電可能電力

	
	
	/* 発電機停止中か？ */
	if(g_gen.avr_freq == 0 || g_gen.run_state == GEN_STOP){
	
		g_gen.voltage_ajust = GEN_VOL_ADJUST_KEEP_250V;							//発電制御電圧250V維持
		return;
	}
	
	/* 蓄電池への充電電力計算 */
	bat_chg_power = (g_modbustcp_input.bat_voltage.val/10) * (g_modbustcp_input.bat_current.val/10);//単位[0.1V]*[0.1A]→[W]

	/* 蓄電池への充電電流計算 */
	bat_chg_current = (g_modbustcp_input.bat_current.val/10);//単位[0.1A]→[A]
	
	/* 発電機の出力電力計算 */
	gen_out_power = sqrt3 * g_gen.disp_current_r * g_gen.disp_current_t;		//√3×発電R相電流(V)×発電T相電流(A)
	
	/* 蓄電池への充電可能電力 */
	bat_chg_power_limit = 0;													//★確認中(UDPから取得)
	
	/* 60Hzの場合 */
	if(g_gen.avr_freq == 60){
		
		/* 蓄電池への充電電力 < 9.4kW？ */
		if(bat_chg_power < 9400){
			
			/* 蓄電池への充電電流 < 33A？ */
			if(bat_chg_current < 33){
				
				/* 発電機の出力電力 < 10.5kW？ */
				if(gen_out_power < 10500){
				
					voltage_adjust_flg = GEN_VOL_UP;							//【UP】電圧上げるフラグセット
					
				}else{
					/* 発電機の出力電力 > 11.5kW？ */
					if(gen_out_power > 11500){
					
						voltage_adjust_flg = GEN_VOL_DOWN;						//【DOWN】電圧下げるフラグセット
						
					}else{
					
						voltage_adjust_flg = GEN_VOL_KEEP;						//【KEEP】電圧そのままフラグセット
					}
				}
				
			}else{
			
				/* 蓄電池への充電電流 > 35A？ */
				if(bat_chg_current > 35){
					voltage_adjust_flg = GEN_VOL_DOWN;							//【DOWN】電圧下げるフラグセット
				}else{
					voltage_adjust_flg = GEN_VOL_KEEP;							//【KEEP】電圧そのままフラグセット
				}
				
			}
			
		}else{
		
			/* 蓄電池への充電電力 < 9.6kW？ */
			if(bat_chg_power < 9600){
				
				/* 蓄電池への充電電流 > 35A？ */
				if(bat_chg_current > 35){
					voltage_adjust_flg = GEN_VOL_DOWN;							//【DOWN】電圧下げるフラグセット
				}else{
					voltage_adjust_flg = GEN_VOL_KEEP;							//【KEEP】電圧そのままフラグセット
				}
				
			}else{
				voltage_adjust_flg = GEN_VOL_DOWN;								//【KEEP】電圧下げるフラグセット
			}
			
		}
		
	/* 50Hzの場合 */
	}else{
		/* 蓄電池への充電電力 < 4.9kW？ */
		if(bat_chg_power < 4900){
			
			/* 蓄電池への充電電流 < 33A？ */
			if(bat_chg_current < 33){
				
				/* 発電機の出力電力 < 10.5kW？ */
				if(gen_out_power < 10500){
				
					voltage_adjust_flg = GEN_VOL_UP;							//【UP】電圧上げるフラグセット
					
				}else{
					/* 発電機の出力電力 > 11.5kW？ */
					if(gen_out_power > 11500){
					
						voltage_adjust_flg = GEN_VOL_DOWN;						//【DOWN】電圧下げるフラグセット
						
					}else{
					
						voltage_adjust_flg = GEN_VOL_KEEP;						//【KEEP】電圧そのままフラグセット
					}
				}
				
			}else{
			
				/* 蓄電池への充電電流 > 35A？ */
				if(bat_chg_current > 35){
					voltage_adjust_flg = GEN_VOL_DOWN;							//【DOWN】電圧下げるフラグセット
				}else{
					voltage_adjust_flg = GEN_VOL_KEEP;							//【KEEP】電圧そのままフラグセット
				}
				
			}
			
		}else{
		
			/* 蓄電池への充電電力 < 5.1kW？ */
			if(bat_chg_power < 5100){
				
				/* 蓄電池への充電電流 > 35A？ */
				if(bat_chg_current > 35){
					voltage_adjust_flg = GEN_VOL_DOWN;							//【DOWN】電圧下げるフラグセット
				}else{
					voltage_adjust_flg = GEN_VOL_KEEP;							//【KEEP】電圧そのままフラグセット
				}
				
			}else{
				voltage_adjust_flg = GEN_VOL_DOWN;								//【KEEP】電圧下げるフラグセット
			}
			
		}
		
	}
	
	/* 蓄電池充電可能電力 > 蓄電池への充電電力 */								//判定分確認中 あげる？下げる？
																				//★確認中(UDPから取得)
	if(bat_chg_power_limit > bat_chg_power){
		
		voltage_adjust_flg = GEN_VOL_DOWN;										//【KEEP】電圧下げるフラグセット
		
	}
	
	
	/* 電圧調整フラグ */
	if(voltage_adjust_flg == GEN_VOL_UP){										//電圧を上げる場合
		
		/* 60Hzの場合 */
		if(g_gen.avr_freq == 60){
			
			if(
				(((GEN_60HZ_OUT+1500)<gen_out_power) && ((GEN_60HZ_OUT-1500)>gen_out_power)) ||				//発電機出力電力が目標値前後か？
				(((GEN_BAT_CHG_POWER+1500)<bat_chg_power) && ((GEN_BAT_CHG_POWER-1500)>bat_chg_power))		//蓄電池充電電力が目標値前後か？
			){
				
				//15秒ごとにアップ
				g_gen.voltage_ajust = GEN_VOL_ADJUST_15S1VUP;					//15秒ごとに1V上げる(タイマータスクでチェック)
				
			}else{
			
				//2秒ごとにアップ
				g_gen.voltage_ajust = GEN_VOL_ADJUST_2S1VUP;					//2秒ごとに1V上げる(タイマータスクでチェック)
			
			}
			
		/* 50Hzの場合 */
		}else if(g_gen.avr_freq == 50){
		
			if(
				(((GEN_50HZ_OUT+1500)<gen_out_power) && ((GEN_50HZ_OUT-1500)>gen_out_power)) ||				//発電機出力電力が目標値前後か？
				(((GEN_BAT_CHG_POWER+1500)<bat_chg_power) && ((GEN_BAT_CHG_POWER-1500)>bat_chg_power))		//蓄電池充電電力が目標値前後か？
			){
				
				//15秒ごとにアップ
				g_gen.voltage_ajust = GEN_VOL_ADJUST_15S1VUP;					//15秒ごとに1V上げる(タイマータスクでチェック)
				
			}else{
			
				//2秒ごとにアップ
				g_gen.voltage_ajust = GEN_VOL_ADJUST_2S1VUP;					//2秒ごとに1V上げる(タイマータスクでチェック)
			
			}
		
		}
		
	}else if(voltage_adjust_flg == GEN_VOL_DOWN){								//電圧を下げる場合
		
		g_gen.voltage_ajust = GEN_VOL_ADJUST_01SVDOWN;							//0.1秒ごとに1V下げる(タイマータスクでチェック)
		
	}else if(voltage_adjust_flg == GEN_VOL_KEEP){								//電圧を現状維持する場合
		
		g_gen.voltage_ajust = GEN_VOL_ADJUST_KEEP;								//現電圧維持(タイマータスクでチェック)
		
	}else{
		/* 処理なし */
	}
	
	/* 蓄電池電圧≧403V */
	if(g_modbustcp_input.bat_voltage.val >= 403){
		g_gen.voltage_ajust = GEN_VOL_ADJUST_KEEP_300V;							//発電制御電圧300V維持
	}
	/* ブレーカ遮断中か？ */
	if(g_gen.breaker_off_flg == FLG_ON){
		g_gen.voltage_ajust = GEN_VOL_ADJUST_KEEP_280V;							//発電制御電圧280V維持
	}
	/*PCS1との通信断中か？*/
	if(g_pcs1.comm_error_flg == FLG_ON){
		g_gen.voltage_ajust = GEN_VOL_ADJUST_KEEP_250V;							//発電制御電圧250V維持
	}
	
}
//******************************************************************************
//  機能     : リレー制御
//  機能説明 : 各リレー制御を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void relay_control(void)
{
	
	/* 発電機始動リレー制御 */
	if(g_gen.start_flg == FLG_ON){
	
		SET_DO(DO_EG_RY,BIT_ON);												//E/G発停リレーON(発電機始動リレーON)
		
	}else{
	
		SET_DO(DO_EG_RY,BIT_OFF);												//E/G発停リレーOFF(発電機始動リレーOFF)
		
	}
	
	/* 冷却ファンリレー制御 */
	if(g_gen.start_flg == FLG_ON){												//発電機始動中は冷却ファンもON
	
		SET_DO(DO_FAN,BIT_ON);													//冷却ファンリレーON
		
	}else{
	
		if(g_gen.ext_genstop & (GEN_START_SOC_BIT1 | GEN_START_SC_BIT2)){		//発電機始動中か？(本体操作で起動した場合)
																				//CAN受信情報(外部コントロール)で判定
		
			SET_DO(DO_FAN,BIT_ON);												//冷却ファンリレーON
			
		}else{
		
			SET_DO(DO_FAN,BIT_OFF);												//冷却ファンリレーOFF
			
		}
		
	}
	
	/* PCS出力用ブレーカ遮断リレー制御 */
	if(g_pcs1.state_flg == FLG_ON){												//自立運転中の場合
	
		SET_DO(DO_PCS_SHT_RY,BIT_OFF);											//SHT制御リレーOFF
		
	}else{
	
		SET_DO(DO_PCS_SHT_RY,BIT_ON);											//SHT制御リレーON
		
	}
	
}
//******************************************************************************
//  機能     : PCS1動作モード制御
//  機能説明 : PCS1(I_DENCON/IKS社製)の動作モードを切り替える。
//           : 
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void pcs1mode_control(void)
{
	
	/*変数定義*/
	UB ope_mode = 0;		//運転モード
	
	/* 系統電力ありかつ自立運転停止中か？ */
	if(
		/* 系統電力あり(ModbusUDP経由で取得する★) かつ */
		((g_modbustcp_input.ope_mode.val == OP_PATTERN_BAT_TO_ISLAND_GEN) ||	//運転モードがNo.32:電池→自立発電または
		 (g_modbustcp_input.ope_mode.val == OP_PATTERN_EV_TO_ISLAND_GEN))		//運転モードがNo.29:EV→自立発電か？
	){
		
		/*EV充電中か？*/
		if(!(g_modbustcp_input.ev_current.val > 0)){
			
			/* 太陽光電圧80V以下？ */
			if(g_modbustcp_input.pv_voltage.val <= 80 ){
				
				/* 蓄電池SOC25%超えている？ */
				if(g_modbustcp_input.bat_soc.val > 250){						//電池SOC25%(単位[0.1%])
					ope_mode = OP_PATTERN_BAT_REGEN_WITH_PV;					//運転モードNo.30:電池回生(PV有)
				}else{
					ope_mode = OP_PATTERN_PV_REGEN;								//運転モードNo.9:PV回生
				}
				
			}else{
				
				/* 蓄電池SOC99%超えている？ */
				if(g_modbustcp_input.bat_soc.val > 990){						//電池SOC99%(単位[0.1%])
					ope_mode = OP_PATTERN_PV_REGEN;								//運転モードNo.9:PV回生
				}else{
					ope_mode = OP_PATTERN_BAT_CHRG_PV_SURPLUS;					//運転モードNo.24:電池充電+PV余剰回生
				}
				
			}
		}
		
	}else{
		
		ope_mode = OP_PATTERN_BAT_TO_ISLAND_GEN;								//運転モードNo.32:(電池→自立発電)
		
	}
	
	/* CANによる運転モード切替 */
	if((g_gen.ext_mode & 0x80) != 0){											//BIT7:指令が有効か？(1で有効)
		
		/* 運転モード設定 */
		ope_mode = (g_gen.ext_mode & 0x7F);										//BIT0-BIT6:運転モード設定値を取り出す
		//値の範囲チェック必要か？
		
	}
	
	/* 運転モード設定 */
	g_modbustcp_holding.ope_mode.val = ope_mode;								//運転モードを設定
	
}
//******************************************************************************
//  機能     : LED更新処理
//  機能説明 : 各SWのLED状態を最新状態に更新する。
//           : メンテナンスSWの点滅制御はタイマータスク内で行う。
//           : 対象：メンテナンスSW用LED、PCS1起動SW用LED、PCS1停止用LED
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void led_update(void)
{
	
	/* PCS1起動SW用LED制御(自立運転開始) */
	if(g_pcs1.state_flg == FLG_ON){
	
		SET_DO(DO_PCS_START_SW_LED, BIT_ON);									//PCS1起動SW LED点灯
		SET_DO(DO_PCS_STOP_SW_LED, BIT_OFF);									//PCS1停止SW LED消灯
		
	}
	
	/* PCS1停止SW用LED制御(自立運転停止) */
	if(g_pcs1.state_flg == FLG_OFF){
	
		SET_DO(DO_PCS_START_SW_LED, BIT_OFF);									//PCS1起動SW LED消灯
		SET_DO(DO_PCS_STOP_SW_LED, BIT_ON);										//PCS1停止SW LED点灯
		
	}
	
	/* メンテナンスSW用LED制御 */
	if(g_pcs1.err_flg != ERROR_NONE){											//エラーコードありの場合
	
		g_pcs1.mntsw_led_flg = (UB)MNT_BLINK1;									//点滅間隔1秒設定(タイマータスクがチェック)
		
	}else if(g_pcs1.gridreturn_flg != FLG_OFF){									//自立運転中に系統電力復帰の場合
	
		g_pcs1.mntsw_led_flg = (UB)MNT_BLINK2;									//点滅間隔2秒設定(タイマータスクがチェック)
		
	}else{																		//メンテナンスSW押下判定
	
		g_pcs1.mntsw_led_flg = (UB)MNT_BLINK_STOP;								//点滅停止
		
		if(g_di[PCS_MAINTE_SW].val == BIT_ON){
			
			SET_DO(DO_PCS_MAINTESW_LED, BIT_ON);								//メンテナンスSW LED点灯
			
		}else{
			
			SET_DO(DO_PCS_MAINTESW_LED, BIT_OFF);								//メンテナンスSW LED消灯
			
		}
	}
	
}
//******************************************************************************
//  機能     : 変数情報出力処理
//  機能説明 : EMS基板内部の情報をRS422ポートから出力する。
//           : デバッグ用途で使用する。
//           : 変数情報は｢DEBUG_VALINFO_TIM｣で定義した秒数ごとに出力する。
//           : 変数情報のヘッダーは｢DEBUG_VALINFO_HEADER｣で定義した回数ごとに
//           : 出力する。負荷低減のため1回に送信するデータは１種類のみとする。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void valinfo_output(void)
{
	/* 変数定義 */
	UB dispdata[] = {0,1,2,3,4,5,6,7,8,9,10,11,0xFF};				//出力するデーバッグ情報(末尾0xFFは固定)
	static UB datakind=0;											//送信するデータ種類
	static UH tim_cnt=0;											//時間カウント(1カウント20msec)(変数情報出力判定に使用)
	
	
	tim_cnt++;									//カウントアップ
	
	/* 変数情報出力 */
	if(tim_cnt >= (DEBUG_VALINFO_TIM*1000/BASE_MS)){
		
		/* システム時刻出力 */
		Pout_time();Pout_s("VALUE_INFO");Pout_LF();
		
		switch(dispdata[datakind]){
			

			case 0:/* DI状態出力 */
				Pout_itou_fit("DI_PCS_MAINTE_SW   =" , g_di[PCS_MAINTE_SW].val);Pout_LF();	//メンテナンスSW
				Pout_itou_fit("DI_PCS_START_SW    =" , g_di[PCS_START_SW].val);Pout_LF();	//PCS起動SW
				Pout_itou_fit("DI_PCS_STOP_SW     =" , g_di[PCS_STOP_SW].val);Pout_LF();	//PCS停止SW
				Pout_itou_fit("DI_PCS_AUX3        =" , g_di[PCS_AUX3].val);Pout_LF();		//AUX3
				Pout_itou_fit("DI_YOBI1           =" , g_di[YOBI1].val);Pout_LF();			//予備1
				Pout_itou_fit("DI_YOBI2           =" , g_di[YOBI2].val);Pout_LF();			//予備2
				Pout_itou_fit("DI_YOBI3           =" , g_di[YOBI3].val);Pout_LF();			//予備3
				Pout_itou_fit("DI_YOBI4           =" , g_di[YOBI4].val);Pout_LF();			//予備4
				break;
				
			case 1:/* DO状態出力 */
				Pout_itou_fit("DO_PCS_START_SW_LED=" , DO_PCS_START_SW_LED);Pout_LF();		//PCS起動SW LED
				Pout_itou_fit("DO_PCS_STOP_SW_LED =" , DO_PCS_STOP_SW_LED);Pout_LF();		//PCS停止SW LED
				Pout_itou_fit("DO_PCS_MAINTESW_LED=" , DO_PCS_MAINTESW_LED);Pout_LF();		//メンテナンスSW LED
				Pout_itou_fit("DO_PCS_SHT_RY      =" , DO_PCS_SHT_RY);Pout_LF();			//SHT制御リレー
				Pout_itou_fit("DO_YOBI2           =" , DO_YOBI2);Pout_LF();					//予備2
				Pout_itou_fit("DO_YOBI3           =" , DO_YOBI3);Pout_LF();					//予備3
				Pout_itou_fit("DO_EG_RY           =" , DO_EG_RY);Pout_LF();					//発電機始動リレー(E/G発停リレー)
				Pout_itou_fit("DO_FAN             =" , DO_FAN);Pout_LF();					//ファン
				Pout_itou_fit("DO_YOBI4           =" , DO_YOBI4);Pout_LF();					//予備4
				Pout_itou_fit("DO_YOBI5           =" , DO_YOBI5);Pout_LF();					//予備5
				Pout_itou_fit("DO_YOBI6           =" , DO_YOBI6);Pout_LF();					//予備6
				break;
				
			case 2:/* PCS状態出力 */
				Pout_ito0X("g_modbustcp_input.heartbeat.val           =",g_modbustcp_input.heartbeat.val);		Pout_LF();	//00 ハートビート
				Pout_ito0X("g_modbustcp_input.system_state.val        =",g_modbustcp_input.system_state.val);	Pout_LF();	//01 システム状態
				Pout_ito0X("g_modbustcp_input.grid_state.val          =",g_modbustcp_input.grid_state.val);		Pout_LF();	//02 系統状態
				Pout_ito0X("g_modbustcp_input.ev_state.val            =",g_modbustcp_input.ev_state.val);		Pout_LF();	//03 EV状態
				Pout_ito0X("g_modbustcp_input.ev_connected.val        =",g_modbustcp_input.ev_connected.val);	Pout_LF();	//04 EV接続
				Pout_ito0X("g_modbustcp_input.pv_state.val            =",g_modbustcp_input.pv_state.val);		Pout_LF();	//05 PV状態
				Pout_ito0X("g_modbustcp_input.bat_state.val           =",g_modbustcp_input.bat_state.val);		Pout_LF();	//06 電池状態
				Pout_ito0X("g_modbustcp_input.error_code.val          =",g_modbustcp_input.error_code.val);		Pout_LF();	//07 エラーコード
				Pout_itod("g_modbustcp_input.internal_temp1.val       =",g_modbustcp_input.internal_temp1.val);	Pout_LF();	//08 内部温度1
				Pout_itod("g_modbustcp_input.internal_temp2.val       =",g_modbustcp_input.internal_temp2.val);	Pout_LF();	//09 内部温度2
				Pout_itod("g_modbustcp_input.internal_temp3.val       =",g_modbustcp_input.internal_temp3.val);	Pout_LF();	//10 内部温度3
				Pout_itod("g_modbustcp_input.internal_temp4.val       =",g_modbustcp_input.internal_temp4.val);	Pout_LF();	//11 内部温度4
				Pout_itod("g_modbustcp_input.internal_temp5.val       =",g_modbustcp_input.internal_temp5.val);	Pout_LF();	//12 内部温度5
				Pout_itod("g_modbustcp_input.enclosure_temp.val       =",g_modbustcp_input.enclosure_temp.val);	Pout_LF();	//13 筐体温度
				Pout_itod("g_modbustcp_input.bat_temp_max.val         =",g_modbustcp_input.bat_temp_max.val);	Pout_LF();	//14 電池最高温度
				Pout_itod("g_modbustcp_input.bat_temp_avg.val         =",g_modbustcp_input.bat_temp_avg.val);	Pout_LF();	//15 電池平均温度
				Pout_itod("g_modbustcp_input.bat_temp_min.val         =",g_modbustcp_input.bat_temp_min.val);	Pout_LF();	//16 電池最低温度
				Pout_ito0X("g_modbustcp_input.grid_v_rs.val           =",g_modbustcp_input.grid_v_rs.val);		Pout_LF();	//17 系統RS相電圧
				Pout_ito0X("g_modbustcp_input.grid_v_st.val           =",g_modbustcp_input.grid_v_st.val);		Pout_LF();	//18 系統ST相電圧
				Pout_ito0X("g_modbustcp_input.grid_v_tr.val           =",g_modbustcp_input.grid_v_tr.val);		Pout_LF();	//19 系統TR相電圧
				Pout_itod("g_modbustcp_input.grid_i_r.val             =",g_modbustcp_input.grid_i_r.val);		Pout_LF();	//20 系統R相電流
				Pout_itod("g_modbustcp_input.grid_i_s.val             =",g_modbustcp_input.grid_i_s.val);		Pout_LF();	//21 系統S相電流
				Pout_itod("g_modbustcp_input.grid_i_t.val             =",g_modbustcp_input.grid_i_t.val);		Pout_LF();	//22 系統T相電流
				Pout_itod("g_modbustcp_input.grid_pf_r.val            =",g_modbustcp_input.grid_pf_r.val);		Pout_LF();	//23 系統R相力率
				Pout_itod("g_modbustcp_input.grid_pf_s.val            =",g_modbustcp_input.grid_pf_s.val);		Pout_LF();	//24 系統S相力率
				Pout_itod("g_modbustcp_input.grid_pf_t.val            =",g_modbustcp_input.grid_pf_t.val);		Pout_LF();	//25 系統T相力率
				Pout_ito0X("g_modbustcp_input.grid_freq.val           =",g_modbustcp_input.grid_freq.val);		Pout_LF();	//26 系統周波数
				Pout_itod("g_modbustcp_input.grid_p_active.val        =",g_modbustcp_input.grid_p_active.val);	Pout_LF();	//27 系統有効電力
				Pout_itod("g_modbustcp_input.grid_p_reactive.val      =",g_modbustcp_input.grid_p_reactive.val);Pout_LF();	//28 系統無効電力
				Pout_ito0X("g_modbustcp_input.ev_voltage.val          =",g_modbustcp_input.ev_voltage.val);		Pout_LF(); 	//29 EV電圧
				Pout_itod("g_modbustcp_input.ev_current.val           =",g_modbustcp_input.ev_current.val);		Pout_LF();	//30 EV電流
				Pout_ito0X("g_modbustcp_input.ev_soc.val              =",g_modbustcp_input.ev_soc.val);			Pout_LF();	//31 EV SOC
				Pout_ito0X("g_modbustcp_input.pv_voltage.val          =",g_modbustcp_input.pv_voltage.val);		Pout_LF();	//32 PV電圧
				Pout_ito0X("g_modbustcp_input.pv_current.val          =",g_modbustcp_input.pv_current.val);		Pout_LF();	//33 PV電流
				Pout_ito0X("g_modbustcp_input.bat_voltage.val         =",g_modbustcp_input.bat_voltage.val);	Pout_LF();	//34 電池電圧
				Pout_itod("g_modbustcp_input.bat_current.val          =",g_modbustcp_input.bat_current.val);	Pout_LF();	//35 電池電流
				Pout_ito0X("g_modbustcp_input.bat_soc.val             =",g_modbustcp_input.bat_soc.val);		Pout_LF();	//36 電池SOC
				Pout_ito0X("g_modbustcp_input.ev_chg_power.val        =",g_modbustcp_input.ev_chg_power.val);	Pout_LF();	//37 EV充電電力
				Pout_ito0X("g_modbustcp_input.ev_dischg_power.val     =",g_modbustcp_input.ev_dischg_power.val);Pout_LF();	//38 EV放電電力
				Pout_ito0X("g_modbustcp_input.bat_chg_power.val       =",g_modbustcp_input.bat_chg_power.val);	Pout_LF();	//39 電池充電電力
				Pout_ito0X("g_modbustcp_input.bat_dischg_power.val    =",g_modbustcp_input.bat_dischg_power.val);		Pout_LF();	//40 電池放電電力
				Pout_ito0X("g_modbustcp_input.grid_input_power.val    =",g_modbustcp_input.grid_input_power.val);		Pout_LF();	//41 系統入力電力
				Pout_ito0X("g_modbustcp_input.grid_output_power.val   =",g_modbustcp_input.grid_output_power.val);		Pout_LF();	//42 系統出力電力
				Pout_ito0X("g_modbustcp_input.ev_soc_chg_limit.val    =",g_modbustcp_input.ev_soc_chg_limit.val);		Pout_LF(); 	//43 EV充電上限SOC
				Pout_ito0X("g_modbustcp_input.ev_soc_dischg_limit.val =",g_modbustcp_input.ev_soc_dischg_limit.val);	Pout_LF();	//44 EV充電下限SOC
				Pout_ito0X("g_modbustcp_input.bat_soc_chg_limit.val   =",g_modbustcp_input.bat_soc_chg_limit.val);		Pout_LF();	//45 電池充電上限SOC
				Pout_ito0X("g_modbustcp_input.bat_soc_dischg_limit.val=",g_modbustcp_input.bat_soc_dischg_limit.val);	Pout_LF();	//46 電池放電下限SOC
				Pout_ito0X("g_modbustcp_input.ope_mode.val            =",g_modbustcp_input.ope_mode.val);				Pout_LF();	//47 運転パターン
				Pout_lto0X("g_modbustcp_input.grid_energy_in_wh.val   =",g_modbustcp_input.grid_energy_in_wh.val);		Pout_LF();	//48-49 系統入力電力量積算
				Pout_lto0X("g_modbustcp_input.grid_energy_out_wh.val  =",g_modbustcp_input.grid_energy_out_wh.val);		Pout_LF();	//50-51 系統出力電力量積算
				Pout_lto0X("g_modbustcp_input.ev_energy_chg_wh.val    =",g_modbustcp_input.ev_energy_chg_wh.val);		Pout_LF();	//52-53 EV充電電力量積算
				Pout_lto0X("g_modbustcp_input.ev_energy_dischg_wh.val =",g_modbustcp_input.ev_energy_dischg_wh.val);	Pout_LF();	//54-55 EV放電電力量積算
				Pout_lto0X("g_modbustcp_input.bat_energy_chg_wh.val   =",g_modbustcp_input.bat_energy_chg_wh.val);		Pout_LF();	//56-57 電池充電電力量積算
				Pout_lto0X("g_modbustcp_input.bat_energy_dischg_wh.val=",g_modbustcp_input.bat_energy_dischg_wh.val);	Pout_LF();	//58-59 電池放電電力量積算
				Pout_lto0X("g_modbustcp_input.pv_energy_wh.val        =",g_modbustcp_input.pv_energy_wh.val);			Pout_LF();	//60-61 PV電力量積算
				break;
				
			case 3:/* CAN状態出力1 */
				//0x1AF0CA01 電池関連(そのまま)
				/* 電池関連1 */
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT1].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_BATT1].table[0].data.uh);Pout_LF();		//電池電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT1].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_BATT1].table[1].data.uh);Pout_LF();		//電池電流
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT1].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_BATT1].table[2].data.uh);Pout_LF();		//電池SOC
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT1].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_BATT1].table[3].data.uh);Pout_LF();		//電池電力
				
				//0x1AF0CA02 電池関連(充電、放電等)
				/* 電池関連2 */
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT2].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_BATT2].table[0].data.uh);Pout_LF();		//電池充電電力 指令値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT2].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_BATT2].table[1].data.uh);Pout_LF();		//電池放電電力 指令値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT2].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_BATT2].table[2].data.uh);Pout_LF();		//電池充電上限SOC 指令値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BATT2].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_BATT2].table[3].data.uh);Pout_LF();		//電池放電下限SOC 指令値
				
				//0x1AF0CA03 温度(電池)
				/* 温度1 */
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[0].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[0].data.ub);Pout_LF();	//電池最高温度
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[1].data.ub);Pout_LF();	//電池平均温度
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[2].data.ub);Pout_LF();	//電池最低温度
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[3].data.ub);Pout_LF();	//電池温度(℃)(=電池最高温度)
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[4].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[5].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[6].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[6].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_BT_TEMP].table[7].data.ub =",g_can_sndmsg[CAN_SNDID_BT_TEMP].table[7].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CA04 温度(内部)
				/* 温度2 */
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[0].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[0].data.ub);Pout_LF();			//内部温度1
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[1].data.ub);Pout_LF();			//内部温度2
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[2].data.ub);Pout_LF();			//内部温度3
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[3].data.ub);Pout_LF();			//内部温度4
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[4].data.ub);Pout_LF();			//内部温度5
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[5].data.ub);Pout_LF();			//筐体温度
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[6].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[6].data.ub);Pout_LF();			//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_TEMP].table[7].data.ub =",g_can_sndmsg[CAN_SNDID_TEMP].table[7].data.ub);Pout_LF();			//未使用
				break;
				
			case 4:/* CAN状態出力2 */
				//0x1AF0CA05 系統関連1(系統RS,ST,TR電圧,R電流)
				/*系統関連1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT1].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_KT1].table[0].data.uh);Pout_LF();		//系統RS相電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT1].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_KT1].table[1].data.uh);Pout_LF();		//系統ST相電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT1].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_KT1].table[2].data.uh);Pout_LF();		//系統TR相電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT1].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_KT1].table[3].data.uh);Pout_LF();		//系統R相電流
				//0x1AF0CA06 系統関連2(系統S,Y電流,R,S相力率)
				/*系統関連2*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT2].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_KT2].table[0].data.uh);Pout_LF();		//系統S相電流
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT2].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_KT2].table[1].data.uh);Pout_LF();		//系統T相電流
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT2].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_KT2].table[2].data.uh);Pout_LF();		//系統R相力率
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT2].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_KT2].table[3].data.uh);Pout_LF();		//系統S相力率
				
				//0x1AF0CA07 系統関連3(系統T相力率,有効無効電力)
				/*系統関連3*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT3].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_KT3].table[0].data.uh);Pout_LF();		//系統T相力率
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT3].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_KT3].table[1].data.uh);Pout_LF();		//系統周波数
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT3].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_KT3].table[2].data.uh);Pout_LF();		//系統有効電力
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT3].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_KT3].table[3].data.uh);Pout_LF();		//系統無効電力
				
				//0x1AF0CA08 系統関連4(系統入出力電力指令値)
				/*系統関連4*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT4].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_KT4].table[0].data.uh);Pout_LF();		//系統入力電力
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT4].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_KT4].table[1].data.uh);Pout_LF();		//系統出力電力
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT4].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_KT4].table[2].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT4].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_KT4].table[3].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT4].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_KT4].table[4].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_KT4].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_KT4].table[5].data.ub);Pout_LF();		//未使用
				
				//0x1AF0CA09 PV関連
				/*PV関連*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PV].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_PV].table[0].data.uh);Pout_LF();		//PV電圧 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PV].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_PV].table[1].data.uh);Pout_LF();		//PV電流 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PV].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_PV].table[2].data.uh);Pout_LF();		//PV電力 計算値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PV].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_PV].table[3].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PV].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_PV].table[4].data.ub);Pout_LF();		//未使用
				break;
				
			case 5:/* CAN状態出力3 */
				//0x1AF0CA0A EV関連1(EV電圧流 実測値,充放電力 指令値)
				/*EV関連1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV1].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_EV1].table[0].data.uh);Pout_LF();	//EV電圧 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV1].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_EV1].table[1].data.uh);Pout_LF();	//EV電流 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV1].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_EV1].table[2].data.uh);Pout_LF();	//EV充電電力 指令値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV1].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_EV1].table[3].data.uh);Pout_LF();	//EV放電電力 指令値
				
				//0x1AF0CA0B EV関連2(EV充電上下限SOC指令値,EV車載蓄電池SOC)
				/*EV関連2*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV2].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_EV2].table[0].data.uh);Pout_LF();	//EV充電上限SOC 指令値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV2].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_EV2].table[1].data.uh);Pout_LF();	//EV放電下限SOC 指令値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV2].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_EV2].table[2].data.uh);Pout_LF();	//EV車載蓄電池のSOC
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV2].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_EV2].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EV2].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_EV2].table[4].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CA0C 積算電力(EV充電電力量積算)
				/*積算電力1*/
				Pout_ltou("g_can_sndmsg[CAN_SNDID_W1].table[0].data.uw =",g_can_sndmsg[CAN_SNDID_W1].table[0].data.uw);Pout_LF();		//EV充電電力量積算
				Pout_ltou("g_can_sndmsg[CAN_SNDID_W1].table[1].data.uw =",g_can_sndmsg[CAN_SNDID_W1].table[1].data.uw);Pout_LF();		//EV放電電力量積算
				
				//0x1AF0CA0D 積算電力(電池充電積算)
				/*積算電力2*/
				Pout_ltou("g_can_sndmsg[CAN_SNDID_W2].table[0].data.uw =",g_can_sndmsg[CAN_SNDID_W2].table[0].data.uw);Pout_LF();		//電池充電電力量積算
				Pout_ltou("g_can_sndmsg[CAN_SNDID_W2].table[1].data.uw =",g_can_sndmsg[CAN_SNDID_W2].table[1].data.uw);Pout_LF();		//電池放電電力量積算
				
				//0x1AF0CA0E 積算電力(PV電力量積算)
				/*積算電力3*/
				Pout_ltou("g_can_sndmsg[CAN_SNDID_W3].table[0].data.uw =",g_can_sndmsg[CAN_SNDID_W3].table[0].data.uw);Pout_LF();		//PV電力量積算
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_W3].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_W3].table[1].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_W3].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_W3].table[2].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_W3].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_W3].table[3].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_W3].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_W3].table[4].data.ub);Pout_LF();		//未使用
				
				//0x1AF0CA20 EMS→発電機(発電制御電圧)
				/*EMS→発電機*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[0].data.uh);Pout_LF();		//発電制御電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[1].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[2].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[3].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[4].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[5].data.ub);Pout_LF();		//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[6].data.ub =",g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[6].data.ub);Pout_LF();		//未使用
				break;
				
			case 6:/* CAN状態出力4 */
				//0x1AF0CA40 状態(システム状態)
				/*状態1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[0].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[0].data.ub);Pout_LF();			//システム状態
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[1].data.ub);Pout_LF();			//系統状態
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[2].data.ub);Pout_LF();			//EV状態
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[3].data.ub);Pout_LF();			//EV接続
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[4].data.ub);Pout_LF();			//PV状態
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[5].data.ub);Pout_LF();			//電池状態
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SYS].table[6].data.uh =",g_can_sndmsg[CAN_SNDID_STATE_SYS].table[6].data.uh);Pout_LF();			//運転パターン 指令値
							
				//0x1AF0CA41 状態(ソフトバージョン)
				/*状態2*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[0].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[0].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[1].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[2].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[4].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[5].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[6].data.uh =",g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[6].data.uh);Pout_LF();	//ソフトウェアバージョン(1.00なら100)
						
				//0x1AF0CA50 エラーコード
				/*エラーコード*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_ERRCODE].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_ERRCODE].table[0].data.uh);Pout_LF();				//エラーコード
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_ERRCODE].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_ERRCODE].table[1].data.uh);Pout_LF();				//エラーコード(とりあえず同じ値)
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_ERRCODE].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_ERRCODE].table[2].data.ub);Pout_LF();				//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_ERRCODE].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_ERRCODE].table[3].data.ub);Pout_LF();				//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_ERRCODE].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_ERRCODE].table[4].data.ub);Pout_LF();				//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_ERRCODE].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_ERRCODE].table[5].data.ub);Pout_LF();				//未使用
				break;
				
			case 7:/* CAN状態出力5 */
				//0x1AF0CAF0 その他山洋PCS(PV,IV実測値)
				/*その他山洋PCS1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS1].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_PCS1].table[0].data.uh);Pout_LF();	//PV電圧 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS1].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_PCS1].table[1].data.uh);Pout_LF();	//PV電流 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS1].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_PCS1].table[2].data.uh);Pout_LF();	//PV電力 計算値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS1].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_PCS1].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS1].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_PCS1].table[4].data.ub);Pout_LF();	//未使用
				
			 	//0x1AF0CAF1 その他山洋PCS(PV積算発電量)
				/*その他山洋PCS2*/
				Pout_ltou("g_can_sndmsg[CAN_SNDID_PCS2].table[0].data.uw =",g_can_sndmsg[CAN_SNDID_PCS2].table[0].data.uw);Pout_LF();	//PV電力 積算発電量
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS2].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_PCS2].table[1].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS2].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_PCS2].table[2].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS2].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_PCS2].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS2].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_PCS2].table[4].data.ub);Pout_LF();	//未使用
				//0x1AF0CAF2 その他山洋PCS(日射,気温)
				/*その他山洋PCS3*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS3].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_PCS3].table[0].data.uh);Pout_LF();	//日射強度
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS3].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_PCS3].table[1].data.uh);Pout_LF();	//気温
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS3].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_PCS3].table[2].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS3].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_PCS3].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS3].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_PCS3].table[4].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS3].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_PCS3].table[5].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CAF3 その他山洋PCS(系統RS,ST,TR_電圧)
				/*その他山洋PCS4*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS4].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_PCS4].table[0].data.uh);Pout_LF();	//系統RS相 電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS4].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_PCS4].table[1].data.uh);Pout_LF();	//系統ST相 電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS4].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_PCS4].table[2].data.uh);Pout_LF();	//系統TR相 電圧
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS4].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_PCS4].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS4].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_PCS4].table[4].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CAF4 その他山洋PCS(系統R,S,T電流)
				/*その他山洋PCS5*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS5].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_PCS5].table[0].data.uh);Pout_LF();	//系統R相 電流
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS5].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_PCS5].table[1].data.uh);Pout_LF();	//系統S相 電流
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS5].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_PCS5].table[2].data.uh);Pout_LF();	//系統T相 電流
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS5].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_PCS5].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_PCS5].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_PCS5].table[4].data.ub);Pout_LF();	//未使用
				break;
				
			case 8:/* CAN状態出力6 */
				//0x1AF0CAF5 その他合算PV(PV,IV実測値)
				/*その他合算PV1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[0].data.uh);Pout_LF();	//PV電圧 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[1].data.uh);Pout_LF();	//PV電流 実測値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[2].data.uh);Pout_LF();	//PV電力 計算値
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[4].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CAF6 その他合算PV(PV電力 積算)
				/*その他合算PV2*/
				Pout_ltou("g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[0].data.uw =",g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[0].data.uw);Pout_LF();	//PV電圧 積算発電量
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[1].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[2].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[4].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CAF7 その他データ(CO2削減量,サービスコード)
				/*その他データ1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[0].data.uh);Pout_LF();	//CO2削減量
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[1].data.uh);Pout_LF();	//サービスコード
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[2].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[4].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[5].data.ub);Pout_LF();	//未使用
				
				//0x1AF0CAF8 その他データ(予備データ1-4)
				/*その他データ2*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[0].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[0].data.uh);Pout_LF();	//予備データ1
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[1].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[1].data.uh);Pout_LF();	//予備データ2
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[2].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[2].data.uh);Pout_LF();	//予備データ3
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[3].data.uh =",g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[3].data.uh);Pout_LF();	//予備データ4
						
				//0x1AF0CAE0 デバック用外部コントロール
				/*デバッグ用外部コントロール1*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[0].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[0].data.ub);Pout_LF();	//発電機始動停止
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[1].data.ub);Pout_LF();	//運転モード指令
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[2].data.ub);Pout_LF();	//充電器電流セット(x10)
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[4].data.ub);Pout_LF();	//発電機電流制限
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[5].data.ub);Pout_LF();	//充電器の充電モード
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[6].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[6].data.ub);Pout_LF();	//CANデバッグモード
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG1].table[7].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG1].table[7].data.ub);Pout_LF();	//回生充電禁止
				
				//0x1AF0CAE1 デバック用外部コントロール(空白)
				/*デバッグ用外部コントロール2*/
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[0].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[0].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[1].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[1].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[2].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[2].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[3].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[3].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[4].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[4].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[5].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[5].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[6].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[6].data.ub);Pout_LF();	//未使用
				Pout_ito0X("g_can_sndmsg[CAN_SNDID_DEBUG2].table[7].data.ub =",g_can_sndmsg[CAN_SNDID_DEBUG2].table[7].data.ub);Pout_LF();	//未使用
				break;
				
			case 9:/* CAN状態入力1(外部コントロール) */
				
				Pout_itou("g_gen.ext_genstop          = ",g_gen.ext_genstop);			Pout_LF();				//発電機始動停止
				Pout_itou("g_gen.ext_mode             = ", g_gen.ext_mode);				Pout_LF();				//運転モード指令
				Pout_itou("g_gen.ext_charge_reset     = ", g_gen.ext_charge_reset);		Pout_LF();				//充電器電流リセット
				Pout_itou("g_gen.ext_none1            = ", g_gen.ext_none1);			Pout_LF(); 				//未使用1(外部コントロール)
				Pout_itou("g_gen.ext_gencurrent_limit = ",g_gen.ext_gencurrent_limit);	Pout_LF();				//発電機電流制限
				Pout_itou("g_gen.ext_charge_mode      = ", g_gen.ext_charge_mode);		Pout_LF();				//充電器の充電モード
				Pout_itou("g_gen.ext_can_debug        = ", g_gen.ext_can_debug);		Pout_LF();				//CANデバッグモード
				Pout_itou("g_gen.ext_regen_inhibit    = ",g_gen.ext_regen_inhibit);		Pout_LF();				//回生充電禁止
				break;
				
			case 10:/* CAN状態入力2(AVR基板) */
				
				Pout_itou("g_gen.avr_rpm              = ",g_gen.avr_rpm);			Pout_LF();					//エンジン回転数(rpm)(移動平均後の値)
				Pout_itou("g_gen.avr_volt_meas        = ",g_gen.avr_volt_meas);		Pout_LF();					//発電電圧測定値(V)(移動平均後の値)
				Pout_itou("g_gen.avr_volt_target      = ",g_gen.avr_volt_target);	Pout_LF();					//発電電圧目標値(V)(移動平均後の値)
				break;
				
			case 11:/* CAN状態入力3(表示基板) */
				
				Pout_itou("g_gen.disp_current_r       = ",g_gen.ext_genstop);		Pout_LF();					//発電R相電流(V)(移動平均後の値)
				Pout_itou("g_gen.disp_current_t       = ", g_gen.ext_mode);			Pout_LF();					//発電T相電流(A)(移動平均後の値)
				break;
				
				
			default:
				/* 処理なし */
				break;
				
		}
		Pout_s("--------------------------------------------------------------");
		Pout_LF();
		
		tim_cnt = 0;															//変数情報を出力したため、時間カウントをクリア
		datakind++;																//データ種類を次の種類へ
		if(dispdata[datakind] == 0xFF){datakind=0;}								//データ種類が上限値に達したら元に戻す
		
	}
	
}
/*END*/