//******************************************************************************
// @(R)
//		CANタスク(CAN0)
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.12.13	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>															//割込みレベル操作用
#include "g_include_def.h"														//共通インクルードファイル
#include <string.h>																//memsetを使用
#include "r_can_rx_if.h"														
//******** ファイル内環境宣言部 ************************************************
//定義無し
//******** 定数定義部 **********************************************************
/* CANID設定のチェック */
#if (FRAME_ID_MODE != EXT_ID_MODE)
	#error	"This feature is available only for extended IDs."					//拡張IDのみ使用可能
#endif
#if (MBOX_MODE != 0)
	#error	"This feature is available only for normal mailbox mode."			//通常メールボックスのみ使用可能
#endif
/* CANエラー定義 */
enum APP_ERR_ENUM{
	APP_NO_ERR 			= 0x0000,
	APP_ERR_CAN_PERIPH 	= 0x0001,
	APP_ERR_CAN_INIT 	= 0x0002,
	APP_TABLE_ERR 		= 0x0004,
	APP_STATE_ERR 		= 0x0008
	/* ユーザ定義するエラーもここに追記する(2のべき乗) */
};
//******** 変数定義部 **********************************************************
UB can0_ch_no;																	//CANチャンネルNo.設定
enum APP_ERR_ENUM can0_app_err_nr;												//CANAPIエラーNo.
can_frame_t can0_tx_dataframe;													//送信データフレーム
can_frame_t can0_rx_dataframe[CANBOX_RX_MAX];									//受信データフレーム(ID3個分専用に確保)
can_bitrate_config_t can0_bitrate_cfg;											//ビットレート
static UW can0_bus_status;														//bus状態
static UH average_buf[AVG_ITEM_COUNT][AVG_NUM];									//移動平均データ用バッファ
static UB average_send_enable;													//移動平均データ送信許可フラグ
//******** プロトタイプ宣言部 **************************************************
TASK can_task(void);															//CANタスク
ER can_init(void);																//CAN初期化
void can_senddt_create(UB id_indx);												//送信データ作成
ER can_send(UB id_indx);														//送信処理
ER can_rcv(UB mb_no,UB dataframe_no);											//受信処理
B sendbox_serch(void);															//送信メールボックス検索処理
void data_copy_ext(UB dataframe_no);											//受信データ格納処理(外部コントロール)
void data_copy_avr(UB dataframe_no);											//受信データ格納処理(AVR基板)
void data_copy_disp(UB dataframe_no);											//受信データ格納処理(表示基板)
void data_average();															//移動平均算出処理
void reset_all_errors();														//全エラー初期化
UH swap_uh(UH val);																//バイトスワップ処理(UH16bit)
UW swap_uw(UW val);																//バイトスワップ処理(UW32bit)
H abs_s16(H s);																	//絶対値算出処理(UH16bit)
//******** 関数定義部 **********************************************************
TASK can_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	ER ercd;																	//エラーコード
	int i,k;
	
	/* CAN0初期設定 */
	reset_all_errors();															//全エラー初期化
	ercd = can_init();															//CAN0初期化
	if(ercd != ER_OK){															//異常時はタスクを起動しない
		while(1);																
	}
	
	Pout_s("CAN_TASK_RUN");														//デバッグ情報
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_CAN_FLG, &status);
		
		/* イベント */
		if (status & F_CAN_EVENT) {												//1000ms周期起動
			
			/* busエラーチェック処理 */
			can0_bus_status = R_CAN_CheckErr(can0_ch_no);						//busエラーチェック
			while (can0_bus_status == R_CAN_STATUS_BUSOFF){						//busOFF状態の場合は通常動作になるまで待つ
			
				can0_bus_status = R_CAN_CheckErr(can0_ch_no);						
				if(can0_bus_status == R_CAN_STATUS_ERROR_ACTIVE){						
																				//エラー復帰処理(処理無し)
				}
				
			}
			
			/* 受信処理 */
			for(i=CANBOX_RX0,k=0; i<=CANBOX_RX2; i++,k++){						//全受信メールボックスをチェック
				
				ercd = can_rcv(i,k);											//CAN受信処理
				if(ercd == ER_OK){												//受信成功時に受信データ格納
																				//k=0:外部コントロール,1:AVR基板,2:表示基板
					if(k==0){
					
						data_copy_ext(k);										//外部コントロールデータ
						g_gen.ext_msg_timout_flg = FLG_OFF;						//外部コントロール通信タイムアウトフラグ(0:正常(通信断無し))
						
					}else if(k==1){
					
						data_copy_avr(k);										//AVR基板からの受信データ
						g_gen.avr_msg_timout_flg = FLG_OFF;						//AVR基板通信タイムアウトフラグ(0:正常(通信断無し))
						
					}else if(k==2){
					
						data_copy_disp(k);										//表示基板からの受信データ
						g_gen.disp_msg_timout_flg = FLG_OFF;					//表示基板タイムアウトフラグ(0:正常(通信断無し))
					}
					
				}
				
			}
			
			/* 移動平均化処理 */
			data_average();														//移動平均化後グローバル変数へ格納
			
			/* 送信処理 */
			if(average_send_enable == 0){										//移動平均データ送信許可状態であれば送信する
				
				for(i=0; i<CAN_SNDID_MAX; i++){
					if(CAN_SNDID_EMS_TO_GEN != i){								//0x1AF0CA20 EMS→発電機(発電制御電圧)以外を送信する
																				//0x1AF0CA20は周期が変動するため、別イベントフラグで送信する
						can_senddt_create(i);									//CAN送信データ作成
						ercd = can_send(i);										//CAN送信処理
						if(ercd != ER_OK){;}									//エラー処理(処理無し、必要であれば追加する)
						
					}
				}
			}
		}
		/* 発電機電圧制御イベント */
		if (status & F_CAN_VOL_CNTL_EVENT) {									//発電機電圧制御イベント(EMS基板→発電機)
			
			can_senddt_create(CAN_SNDID_EMS_TO_GEN);							//CAN送信データ作成
			ercd = can_send(CAN_SNDID_EMS_TO_GEN);								//CAN送信処理
																				//0x1AF0CA20 EMS→発電機(発電制御電圧)を送信する
			if(ercd != ER_OK){;}												//エラー処理(処理無し、必要であれば追加する)
			
		}
		/* 外部コントロール通信断イベント */
		if (status & F_CAN_TIMOUT_EXT) {
		
			g_gen.ext_msg_timout_flg = FLG_ON;									//外部コントロール通信タイムアウトフラグ(1:通信断)
			
		}
		/* AVR基板通信断イベント */
		if (status & F_CAN_TIMOUT_AVR) {
		
			g_gen.avr_msg_timout_flg = FLG_ON;									//AVR基板通信タイムアウトフラグ(1:通信断)
			average_send_enable = AVG_NUM;										//移動平均データ送信許可フラグに平均回数を格納(減算して0になるとFLG_ON)
			
		}
		/* 表示基板通信断イベント */
		if (status & F_CAN_TIMOUT_DISP) {
		
			g_gen.disp_msg_timout_flg = FLG_ON;									//表示基板通信タイムアウトフラグ(1:通信断)
			average_send_enable = AVG_NUM;										//移動平均データ送信許可フラグに平均回数を格納(減算して0になるとFLG_ON)
			
		}
	}
}
//******************************************************************************
//  機能     : CAN初期化
//  機能説明 : CANモジュールを初期化する。
//------------------------------------------------------------------------------
//  引数     : UB ch     : 使用するCANのチャンネル
//------------------------------------------------------------------------------
//  戻り値   : ER_OK(0)  : 正常終了
//           : ER_NG(-1) : 異常終了
//******************************************************************************
ER can_init(void){
	
	/* 変数定義 */
	UW api_status = R_CAN_OK;													//APIステータスチェック用(初期値R_CAN_OK(0))
	UB i;																		//ループ用
	
	/* 移動平均関連初期化 */
	average_send_enable = AVG_NUM;												//移動平均データ送信許可フラグに平均回数を格納(減算して0になるとFLG_ON)
	memset(average_buf,0,sizeof(average_buf));									//移動平均データ用バッファ
	
	/* CANチャンネルNo.設定 */
	can0_ch_no = CH_NO;															//チャンネルNo.設定(0)
	
	/* CAN0ビットレート設定 */
	can0_bitrate_cfg.BRP = CAN0_BRP;
	can0_bitrate_cfg.SJW = CAN0_SJW;
	can0_bitrate_cfg.TSEG1 = CAN0_TSEG1;
	can0_bitrate_cfg.TSEG2 = CAN0_TSEG2;
	
	/* CAN bus監視ステータス初期化 */
	can0_bus_status = R_CAN_STATUS_ERROR_ACTIVE;								//CAN bus状態：通常動作
	
	/* CAN周辺回路初期化 */
	api_status = R_CAN_Create(can0_ch_no,0,can0_bitrate_cfg, NULL, NULL, NULL, NULL,NULL);
	if(api_status != R_CAN_OK){return ER_NG;}									//異常終了
	
	/* CANポート端子(CH0)を有効 */
	api_status = R_CAN_PortSet(can0_ch_no, ENABLE);								//CH0(固定)
	if(api_status != R_CAN_OK){return ER_NG;}									//異常終了
	
	
	/* メールボックス設定(受信) */
	api_status =  (UW)R_CAN_RxSetXid(can0_ch_no, CANBOX_RX0,g_can_rcvmsg[CAN_RCVID_GAIBUCTRL].id, DATA_FRAME);	//外部コントロール
	api_status |= (UW)R_CAN_RxSetXid(can0_ch_no, CANBOX_RX1,g_can_rcvmsg[CAN_RCVID_GENAVR].id,    DATA_FRAME);	//発電機AVR基盤
	api_status |= (UW)R_CAN_RxSetXid(can0_ch_no, CANBOX_RX2,g_can_rcvmsg[CAN_RCVID_GENKIBAN].id,  DATA_FRAME);	//発電機表示基盤
	
	if(api_status != R_CAN_OK){return ER_NG;}									//異常終了
	
	/* 受信メールボックス マスク設定 */
	R_CAN_RxSetMask(can0_ch_no, CANBOX_RX0, 0x1FFFFFFF);						//受信BOX0 29bit全て一致で受信
	R_CAN_RxSetMask(can0_ch_no, CANBOX_RX1, 0x1FFFFFFF);						//受信BOX1 29bit全て一致で受信
	R_CAN_RxSetMask(can0_ch_no, CANBOX_RX2, 0x1FFFFFFF);						//受信BOX2 29bit全て一致で受信
	
	/* 受信データフレーム 受信IDセット */
	for(i=0; i<CANBOX_RX_MAX; i++){												//受信IDセット
		can0_rx_dataframe[i].id = g_can_rcvmsg[i].id;
	}
	
	/* CAN初期化の正常終了チェック */
	if(api_status != R_CAN_OK){
		can0_app_err_nr = APP_ERR_CAN_INIT;
		return ER_NG;															//異常終了
	}
	
	return ER_OK;																//正常終了
}
//******************************************************************************
//  機能     : CAN送信データ作成
//  機能説明 : CAN送信用バッファに送信データを設定する。
//------------------------------------------------------------------------------
//  引数     : UB id_index : 作成する送信データのID添え字
//           : (0～28が0x1AF0CA01～0x1AF0CAE1に対応する)
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void can_senddt_create(UB id_indx)
{
	
	switch(id_indx){
		
		case CAN_SNDID_BATT1:					//0x1AF0CA01 電池関連(そのまま)
		
			/* 電池関連1 */
			g_can_sndmsg[CAN_SNDID_BATT1].table[0].data.uh = g_modbustcp_input.bat_voltage.val;															//電池電圧(0.1V)
			g_can_sndmsg[CAN_SNDID_BATT1].table[1].data.uh = (g_modbustcp_input.bat_current.val + g_can_sndmsg[CAN_SNDID_BATT1].table[1].offset.uh);	//電池電流(0.1A)
			g_can_sndmsg[CAN_SNDID_BATT1].table[2].data.uh = g_modbustcp_input.bat_soc.val;																//電池SOC(0.1%)
			g_can_sndmsg[CAN_SNDID_BATT1].table[3].data.uh = (((g_modbustcp_input.bat_voltage.val) * (g_modbustcp_input.bat_current.val)/1000)
																 + g_can_sndmsg[CAN_SNDID_BATT1].table[3].offset.uh);									//電池電力(0.01kW)
			break;
			
		case CAN_SNDID_BATT2:					//0x1AF0CA02 電池関連(充電、放電等)
			/* 電池関連2 */
			g_can_sndmsg[CAN_SNDID_BATT2].table[0].data.uh = g_modbustcp_input.bat_chg_power.val + g_can_sndmsg[CAN_SNDID_BATT2].table[0].offset.uh;	//電池充電電力 指令値(W)
			g_can_sndmsg[CAN_SNDID_BATT2].table[1].data.uh = g_modbustcp_input.bat_dischg_power.val + g_can_sndmsg[CAN_SNDID_BATT2].table[1].offset.uh;	//電池放電電力 指令値(W)
			g_can_sndmsg[CAN_SNDID_BATT2].table[2].data.uh = g_modbustcp_input.bat_soc_chg_limit.val * 10;			//電池充電上限SOC 指令値(0.1%←%)
			g_can_sndmsg[CAN_SNDID_BATT2].table[3].data.uh = g_modbustcp_input.bat_soc_dischg_limit.val *10;		//電池放電下限SOC 指令値(0.1%←%)
			break;
			
		case CAN_SNDID_BT_TEMP:					//0x1AF0CA03 温度(電池)
			/* 温度1 */
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[0].data.ub = g_modbustcp_input.bat_temp_max.val + g_can_sndmsg[CAN_SNDID_BT_TEMP].table[0].offset.ub;//電池最高温度(℃)
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[1].data.ub = g_modbustcp_input.bat_temp_avg.val + g_can_sndmsg[CAN_SNDID_BT_TEMP].table[1].offset.ub;//電池平均温度(℃)
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[2].data.ub = g_modbustcp_input.bat_temp_min.val + g_can_sndmsg[CAN_SNDID_BT_TEMP].table[2].offset.ub;//電池最低温度(℃)
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[3].data.ub = g_modbustcp_input.bat_temp_max.val + g_can_sndmsg[CAN_SNDID_BT_TEMP].table[0].offset.ub;//電池温度(℃)(=電池最高温度)
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[4].data.ub = 0;			//未使用
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[5].data.ub = 0;			//未使用
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[6].data.ub = 0;			//未使用
			g_can_sndmsg[CAN_SNDID_BT_TEMP].table[7].data.ub = 0;			//未使用
			break;
			
		case CAN_SNDID_TEMP:					//0x1AF0CA04 温度(内部)
			/* 温度2 */
			g_can_sndmsg[CAN_SNDID_TEMP].table[0].data.ub = g_modbustcp_input.internal_temp1.val + g_can_sndmsg[CAN_SNDID_TEMP].table[0].offset.ub;//内部温度1(℃)
			g_can_sndmsg[CAN_SNDID_TEMP].table[1].data.ub = g_modbustcp_input.internal_temp2.val + g_can_sndmsg[CAN_SNDID_TEMP].table[1].offset.ub;//内部温度2(℃)
			g_can_sndmsg[CAN_SNDID_TEMP].table[2].data.ub = g_modbustcp_input.internal_temp3.val + g_can_sndmsg[CAN_SNDID_TEMP].table[2].offset.ub;//内部温度3(℃)
			g_can_sndmsg[CAN_SNDID_TEMP].table[3].data.ub = g_modbustcp_input.internal_temp4.val + g_can_sndmsg[CAN_SNDID_TEMP].table[3].offset.ub;//内部温度4(℃)
			g_can_sndmsg[CAN_SNDID_TEMP].table[4].data.ub = g_modbustcp_input.internal_temp5.val + g_can_sndmsg[CAN_SNDID_TEMP].table[4].offset.ub;//内部温度5(℃)
			g_can_sndmsg[CAN_SNDID_TEMP].table[5].data.ub = g_modbustcp_input.enclosure_temp.val + g_can_sndmsg[CAN_SNDID_TEMP].table[5].offset.ub;//筐体温度(℃)
			g_can_sndmsg[CAN_SNDID_TEMP].table[6].data.ub = 0;			//未使用
			g_can_sndmsg[CAN_SNDID_TEMP].table[7].data.ub = 0;			//未使用
			break;
			
		case CAN_SNDID_KT1:						//0x1AF0CA05 系統関連1(系統RS,ST,TR電圧,R電流)
			/*系統関連1*/
			g_can_sndmsg[CAN_SNDID_KT1].table[0].data.uh = g_modbustcp_input.grid_v_rs.val;					//系統RS相電圧(0.1V)
			g_can_sndmsg[CAN_SNDID_KT1].table[1].data.uh = g_modbustcp_input.grid_v_st.val;					//系統ST相電圧(0.1V)
			g_can_sndmsg[CAN_SNDID_KT1].table[2].data.uh = g_modbustcp_input.grid_v_tr.val;					//系統TR相電圧(0.1V)
			g_can_sndmsg[CAN_SNDID_KT1].table[3].data.uh = abs_s16(g_modbustcp_input.grid_i_r.val);			//系統R相電流(0.1A),絶対値
			break;
			
		case CAN_SNDID_KT2:						//0x1AF0CA06 系統関連2(系統S,Y電流,R,S相力率)
			/*系統関連2*/
			g_can_sndmsg[CAN_SNDID_KT2].table[0].data.uh = abs_s16(g_modbustcp_input.grid_i_s.val);			//系統S相電流(0.1A),絶対値
			g_can_sndmsg[CAN_SNDID_KT2].table[1].data.uh = abs_s16(g_modbustcp_input.grid_i_t.val);			//系統T相電流(0.1A),絶対値
			g_can_sndmsg[CAN_SNDID_KT2].table[2].data.uh = abs_s16(g_modbustcp_input.grid_pf_r.val/10);		//系統R相力率(0.1%←0.01%),絶対値
			g_can_sndmsg[CAN_SNDID_KT2].table[3].data.uh = abs_s16(g_modbustcp_input.grid_pf_s.val/10);		//系統S相力率(0.1%←0.01%),絶対値
			break;
			
		case CAN_SNDID_KT3:						//0x1AF0CA07 系統関連3(系統T相力率,有効無効電力)
			/*系統関連3*/
			g_can_sndmsg[CAN_SNDID_KT3].table[0].data.uh = abs_s16(g_modbustcp_input.grid_pf_t.val/10);		//系統T相力率(0.1%←0.01%),絶対値
			g_can_sndmsg[CAN_SNDID_KT3].table[1].data.uh = g_modbustcp_input.grid_freq.val / 10;			//系統周波数(0.1Hz←0.01Hz)
			g_can_sndmsg[CAN_SNDID_KT3].table[2].data.uh = abs_s16(g_modbustcp_input.grid_p_active.val);	//系統有効電力(W),絶対値
			g_can_sndmsg[CAN_SNDID_KT3].table[3].data.uh = abs_s16(g_modbustcp_input.grid_p_reactive.val);	//系統無効電力(W),絶対値
			break;
			
		case CAN_SNDID_KT4:						//0x1AF0CA08 系統関連4(系統入出力電力指令値)
			/*系統関連4*/
			g_can_sndmsg[CAN_SNDID_KT4].table[0].data.uh = g_modbustcp_input.grid_input_power.val;			//系統入力電力
			g_can_sndmsg[CAN_SNDID_KT4].table[1].data.uh = g_modbustcp_input.grid_output_power.val;			//系統出力電力
			g_can_sndmsg[CAN_SNDID_KT4].table[2].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_KT4].table[3].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_KT4].table[4].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_KT4].table[5].data.ub = 0;		//未使用
			break;
			
		case CAN_SNDID_PV:						//0x1AF0CA09 PV関連
			/*PV関連*/
			g_can_sndmsg[CAN_SNDID_PV].table[0].data.uh = g_modbustcp_input.pv_voltage.val;					//PV電圧 実測値(0.1V)
			g_can_sndmsg[CAN_SNDID_PV].table[1].data.uh = g_modbustcp_input.pv_current.val;					//PV電流 実測値(0.1A)
			g_can_sndmsg[CAN_SNDID_PV].table[2].data.uh = (g_modbustcp_input.pv_voltage.val * g_modbustcp_input.pv_current.val / 1000);//PV電力 計算値(0.01kW)
			g_can_sndmsg[CAN_SNDID_PV].table[3].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_PV].table[4].data.ub = 0;		//未使用
			break;
			
		case CAN_SNDID_EV1:						//0x1AF0CA0A EV関連1(EV電圧流 実測値,充放電力 指令値)
			/*EV関連1*/
			g_can_sndmsg[CAN_SNDID_EV1].table[0].data.uh = g_modbustcp_input.ev_voltage.val;		//EV電圧 実測値(0.1V)
			g_can_sndmsg[CAN_SNDID_EV1].table[1].data.uh = g_modbustcp_input.ev_current.val;		//EV電流 実測値(0.1A)
			g_can_sndmsg[CAN_SNDID_EV1].table[2].data.uh = g_modbustcp_input.ev_chg_power.val;		//EV充電電力 指令値(W)
			g_can_sndmsg[CAN_SNDID_EV1].table[3].data.uh = g_modbustcp_input.ev_dischg_power.val;	//EV放電電力 指令値(W)
			break;
			
		case CAN_SNDID_EV2:						//0x1AF0CA0B EV関連2(EV充電上下限SOC指令値,EV車載蓄電池SOC)
			/*EV関連2*/
			g_can_sndmsg[CAN_SNDID_EV2].table[0].data.uh = (g_modbustcp_input.ev_soc_chg_limit.val * 10);		//EV充電上限SOC 指令値(0.1%)
			g_can_sndmsg[CAN_SNDID_EV2].table[1].data.uh = (g_modbustcp_input.ev_soc_dischg_limit.val * 10);	//EV放電下限SOC 指令値(0.1%)
			g_can_sndmsg[CAN_SNDID_EV2].table[2].data.uh = g_modbustcp_input.ev_soc.val;						//EV車載蓄電池のSOC(0.1%)
			g_can_sndmsg[CAN_SNDID_EV2].table[3].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_EV2].table[4].data.ub = 0;		//未使用
			break;
			
		case CAN_SNDID_W1:						//0x1AF0CA0C 積算電力(EV充電電力量積算)
			/*積算電力1*/
			g_can_sndmsg[CAN_SNDID_W1].table[0].data.uw = (g_modbustcp_input.ev_energy_chg_wh.val / 1000);		//EV充電電力量積算(kWh)
			g_can_sndmsg[CAN_SNDID_W1].table[1].data.uw = (g_modbustcp_input.ev_energy_dischg_wh.val / 1000);	//EV放電電力量積算(kWh)
			break;
			
		case CAN_SNDID_W2:						//0x1AF0CA0D 積算電力(電池充電積算)
			/*積算電力2*/
			g_can_sndmsg[CAN_SNDID_W2].table[0].data.uw = (g_modbustcp_input.bat_energy_chg_wh.val / 1000);		//電池充電電力量積算(kWh)
			g_can_sndmsg[CAN_SNDID_W2].table[1].data.uw = (g_modbustcp_input.bat_energy_dischg_wh.val / 1000);	//電池放電電力量積算(kWh)
			break;
			
		case CAN_SNDID_W3:						//0x1AF0CA0E 積算電力(PV電力量積算)
			/*積算電力3*/
			g_can_sndmsg[CAN_SNDID_W3].table[0].data.uw = (g_modbustcp_input.pv_energy_wh.val / 100);			//PV電力量積算(0.1kWh)
			g_can_sndmsg[CAN_SNDID_W3].table[1].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_W3].table[2].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_W3].table[3].data.ub = 0;		//未使用
			g_can_sndmsg[CAN_SNDID_W3].table[4].data.ub = 0;		//未使用
			break;
			
		case CAN_SNDID_EMS_TO_GEN:				//0x1AF0CA20 EMS→発電機(発電制御電圧)
			/*EMS→発電機*/
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[0].data.uh = g_gen.voltage_set;							//発電制御電圧
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[1].data.ub = 0;											//未使用
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[2].data.ub = 0;											//未使用
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[3].data.ub = 0;											//未使用
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[4].data.ub = 0;											//未使用
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[5].data.ub = 0;											//未使用
			g_can_sndmsg[CAN_SNDID_EMS_TO_GEN].table[6].data.ub = 0;											//未使用
			break;
			
		case CAN_SNDID_STATE_SYS:				//0x1AF0CA40 状態(システム状態)
			/*状態1*/
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[0].data.ub = g_modbustcp_input.system_state.val;			//システム状態
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[1].data.ub = g_modbustcp_input.grid_state.val;				//系統状態
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[2].data.ub = g_modbustcp_input.ev_state.val;				//EV状態
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[3].data.ub = g_modbustcp_input.ev_connected.val;			//EV接続
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[4].data.ub = g_modbustcp_input.pv_state.val;				//PV状態
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[5].data.ub = g_modbustcp_input.bat_state.val;				//電池状態
			g_can_sndmsg[CAN_SNDID_STATE_SYS].table[6].data.uh = g_modbustcp_input.ope_mode.val;				//運転パターン 指令値
			break;
			
			
		case CAN_SNDID_STATE_SOFTVER:			//0x1AF0CA41 状態(ソフトバージョン)
			/*状態2*/
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[0].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[1].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[2].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[3].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[4].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[5].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_STATE_SOFTVER].table[6].data.uh= 0;	//ソフトウェアバージョン(1.00なら100)
			break;
			
		case CAN_SNDID_ERRCODE:					//0x1AF0CA50 エラーコード
			/*エラーコード*/
			g_can_sndmsg[CAN_SNDID_ERRCODE].table[0].data.uh= g_modbustcp_input.error_code.val;	//エラーコード
			g_can_sndmsg[CAN_SNDID_ERRCODE].table[1].data.uh= g_modbustcp_input.error_code.val;	//エラーコード
			g_can_sndmsg[CAN_SNDID_ERRCODE].table[2].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_ERRCODE].table[3].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_ERRCODE].table[4].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_ERRCODE].table[5].data.ub= 0;	//未使用
			break;
			
		case CAN_SNDID_PCS1:					//0x1AF0CAF0 その他山洋PCS(PV,IV実測値)
			/*その他山洋PCS1*/
			g_can_sndmsg[CAN_SNDID_PCS1].table[0].data.uh= 0;	//PV電圧 実測値
			g_can_sndmsg[CAN_SNDID_PCS1].table[1].data.uh= 0;	//PV電流 実測値
			g_can_sndmsg[CAN_SNDID_PCS1].table[2].data.uh= 0;	//PV電力 計算値
			g_can_sndmsg[CAN_SNDID_PCS1].table[3].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_PCS1].table[4].data.ub= 0;	//未使用
			break;
			
		 case CAN_SNDID_PCS2:					//0x1AF0CAF1 その他山洋PCS(PV積算発電量)
			/*その他山洋PCS2*/
			g_can_sndmsg[CAN_SNDID_PCS2].table[0].data.uw= 0;	//PV電力 積算発電量
			g_can_sndmsg[CAN_SNDID_PCS2].table[1].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_PCS2].table[2].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_PCS2].table[3].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_PCS2].table[4].data.ub= 0;	//未使用
			break;
			
		case CAN_SNDID_PCS3:					//0x1AF0CAF2 その他山洋PCS(日射,気温)
			/*その他山洋PCS3*/
			g_can_sndmsg[CAN_SNDID_PCS3].table[0].data.uh= g_ai[1];	//日射強度
			g_can_sndmsg[CAN_SNDID_PCS3].table[1].data.uh= g_ai[0];	//気温
			g_can_sndmsg[CAN_SNDID_PCS3].table[2].data.ub= 0;		//未使用
			g_can_sndmsg[CAN_SNDID_PCS3].table[3].data.ub= 0;		//未使用
			g_can_sndmsg[CAN_SNDID_PCS3].table[4].data.ub= 0;		//未使用
			g_can_sndmsg[CAN_SNDID_PCS3].table[5].data.ub= 0;		//未使用
			break;
			
		case CAN_SNDID_PCS4:					//0x1AF0CAF3 その他山洋PCS(系統RS,ST,TR_電圧)
			/*その他山洋PCS4*/
			g_can_sndmsg[CAN_SNDID_PCS4].table[0].data.uh= 0;	//系統RS相 電圧
			g_can_sndmsg[CAN_SNDID_PCS4].table[1].data.uh= 0;	//系統ST相 電圧
			g_can_sndmsg[CAN_SNDID_PCS4].table[2].data.uh= 0;	//系統TR相 電圧
			g_can_sndmsg[CAN_SNDID_PCS4].table[3].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_PCS4].table[4].data.ub= 0;	//未使用
			break;
			
		case CAN_SNDID_PCS5:					//0x1AF0CAF4 その他山洋PCS(系統R,S,T電流)
			/*その他山洋PCS5*/
			g_can_sndmsg[CAN_SNDID_PCS5].table[0].data.uh= 0;	//系統R相 電流
			g_can_sndmsg[CAN_SNDID_PCS5].table[1].data.uh= 0;	//系統S相 電流
			g_can_sndmsg[CAN_SNDID_PCS5].table[2].data.uh= 0;	//系統T相 電流
			g_can_sndmsg[CAN_SNDID_PCS5].table[3].data.ub= 0;	//未使用
			g_can_sndmsg[CAN_SNDID_PCS5].table[4].data.ub= 0;	//未使用
			break;
			
		case CAN_SNDID_OTHER_PV1:				//0x1AF0CAF5 その他合算PV(PV,IV実測値)
			/*その他合算PV1*/
			g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[0].data.uh = 0;	//PV電圧 実測値
			g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[1].data.uh = 0;	//PV電流 実測値
			g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[2].data.uh = 0;	//PV電力 計算値
			g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[3].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_PV1].table[4].data.ub = 0;	//未使用
			break;
			
		case CAN_SNDID_OTHER_PV2:				//0x1AF0CAF6 その他合算PV(PV電力 積算)
			/*その他合算PV2*/
			g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[0].data.uw = 0;	//PV電圧 積算発電量
			g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[1].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[2].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[3].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_PV2].table[4].data.ub = 0;	//未使用
			break;
			
		case CAN_SNDID_OTHER_DATA1:				//0x1AF0CAF7 その他データ(CO2削減量,サービスコード)
			/*その他データ1*/
			g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[0].data.uh = 0;	//CO2削減量
			g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[1].data.uh = 0;	//サービスコード
			g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[2].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[3].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[4].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_OTHER_DATA1].table[5].data.ub = 0;	//未使用
			break;
			
		case CAN_SNDID_OTHER_DATA2:				//0x1AF0CAF8 その他データ(予備データ1-4)
			/*その他データ2*/
			g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[0].data.uh = 0;	//予備データ1
			g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[1].data.uh = 0;	//予備データ2
			g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[2].data.uh = 0;	//予備データ3
			g_can_sndmsg[CAN_SNDID_OTHER_DATA2].table[3].data.uh = 0;	//予備データ4
			break;
			
		case CAN_SNDID_DEBUG1:					//0x1AF0CAE0 デバック用外部コントロール
			/*デバッグ用外部コントロール1*/
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[0].data.ub = 0;	//発電機始動停止
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[1].data.ub = 0;	//運転モード指令
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[2].data.ub = 0;	//充電器電流セット(x10)
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[3].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[4].data.ub = 0;	//発電機電流制限
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[5].data.ub = 0;	//充電器の充電モード
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[6].data.ub = 0;	//CANデバッグモード
			g_can_sndmsg[CAN_SNDID_DEBUG1].table[7].data.ub = 0;	//回生充電禁止
			break;
			
		case CAN_SNDID_DEBUG2:					//0x1AF0CAE1 デバック用外部コントロール(空白)
			/*デバッグ用外部コントロール2*/
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[0].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[1].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[2].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[3].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[4].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[5].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[6].data.ub = 0;	//未使用
			g_can_sndmsg[CAN_SNDID_DEBUG2].table[7].data.ub = 0;	//未使用
			break;
	}
}
//******************************************************************************
//  機能     : CAN送信処理
//  機能説明 : 
//------------------------------------------------------------------------------
//  引数     : UB id_index :送信するデータのID添え字
//           : (0～28が0x1AF0CA01～0x1AF0CAE1に対応する。)
//------------------------------------------------------------------------------
//  戻り値   : ER_OK(0)  :正常終了
//           : ER_NG(-1) :異常終了
//******************************************************************************
ER can_send(UB id_index)
{
	/* 変数定義 */
	UW api_status = R_CAN_OK;			//APIステータスチェック用(初期値R_CAN_OK(0))
	B mb_no;							//メールボックスNo.
	UB *dst = can0_tx_dataframe.data;	//CAN0送信バッファ
	UB pos = 0;							//CAN0送信バッファ配列用ポインタ([0]～[7]を示す)
	CAN_DATA_ENTRY *e;					//CANデータ構造体用ポインタ
	UB i;								//ループ用
	
	/* 送信メールボックス検索 */
	mb_no = sendbox_serch();
	if(mb_no == ER_NG){return ER_NG;}											//全メールボックスが使用中の場合エラー
	
	/* 送信ID設定 */
	can0_tx_dataframe.id = g_can_sndmsg[id_index].id;							//送信ID
	can0_tx_dataframe.dlc = 8;													//データ数(8byte固定)
	
	/* 送信データ設定 */
	for(i=0; i<g_can_sndmsg[id_index].entry_count;i++){
		
		e = &g_can_sndmsg[id_index].table[i];									//CANデータ構造体の先頭アドレスを取得
		
		switch(e->type){														//当該データタイプを判定(UB,UH,UWか)
			
			case CAN_TYPE_UB:													//UB型の場合
				dst[pos++] = e->data.ub;										//送信バッファにそのまま格納
				break;
			
			case CAN_TYPE_UH:													//UH型の場合
				dst[pos++] = (UB)(e->data.uh & 0xFF);							//送信バッファにリトルエンディアンで格納
				dst[pos++] = (UB)((e->data.uh >> 8) & 0xFF);					
				break;
				
			case CAN_TYPE_UW:													//UW型の場合
				dst[pos++] = (UB)(e->data.uw & 0xFF);							//送信バッファにリトルエンディアンで格納
				dst[pos++] = (UB)((e->data.uw >> 8) & 0xFF);					
				dst[pos++] = (UB)((e->data.uw >> 16) & 0xFF);					
				dst[pos++] = (UB)((e->data.uw >> 24) & 0xFF);					
				break;
			default:
				/* 処理無し */
				break;
		}
		
		if(pos >= 8){break;}													//CANデータの設定ミスなどでpos値が8以上の場合はエラーで抜ける
	}

	/* 動作モード設定 */
	api_status = R_CAN_Control(can0_ch_no, OPERATE_CANMODE);					//CAN0動作モード設定(オペレーションモード)
	if (api_status != R_CAN_OK){
		return ER_NG;
	}									//異常終了
	
	/* メールボックス設定(送信) */
	api_status = R_CAN_TxSetXid(can0_ch_no, MBOX_MODE, mb_no,&can0_tx_dataframe, DATA_FRAME);
	if(api_status != R_CAN_OK){
		return ER_NG;
	}									//異常終了
	
	/* 送信処理 */
	api_status = R_CAN_Tx(can0_ch_no, MBOX_MODE, mb_no);
	if(api_status != R_CAN_OK){
		return ER_NG;
	}									//異常終了
	
	
	return ER_OK;																//正常終了
}
//******************************************************************************
//  機能     : 送信メールボックス検索処理
//  機能説明 : 
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : B mb_no: 使用可能なメールボックスNo.
//                    : ER_NG(-1)の場合は、使用可能なメールボックスが無し
//******************************************************************************
B sendbox_serch(void)
{
	/* 変数定義 */
	UB mb_no;																	//メールボックスNo.
	
	for(mb_no=0; mb_no<CANBOX_TX_MAX; mb_no++){									//送信メールボックス全てを検索
		
		if(CAN0.MCTL[mb_no].BIT.TX.TRMACTIVE == 0){return mb_no;}				//送信中ステータスフラグでメールボックスが
																				//使用可能かチェックする
																				//正常終了
	}
	
	return ER_NG;																//異常終了
																				//ER_NG(-1)
}
//******************************************************************************
//  機能     : CAN受信処理
//  機能説明 : 
//------------------------------------------------------------------------------
//  引数     : UB mb_no  :受信処理するメールボックスNo.
//           : UB dtframe_no :受信データを取り込むデータフレームNo.
//------------------------------------------------------------------------------
//  戻り値   : ER_OK(0)  :正常終了
//           : ER_NG(-1) :異常終了
//******************************************************************************
ER can_rcv(UB mb_no,UB dataframe_no)
{
	/* 変数定義 */
	UW api_status = R_CAN_OK;		//APIステータスチェック用(初期値R_CAN_OK(0))
	
	api_status = R_CAN_RxPoll(can0_ch_no, mb_no);								//受信メッセージ有無チェック
	
	if(api_status == R_CAN_OK){													//受信メッセージがある場合
		
		api_status = R_CAN_RxRead(can0_ch_no, MBOX_MODE, mb_no, &can0_rx_dataframe[dataframe_no]);
		
		
		/* 外部コントロール通信断検知用タイマー設定 */
		if(dataframe_no == 0){
			
			CancelTimer10ms((ID)ID_CAN_FLG, (FLGPTN)F_CAN_TIMOUT_EXT);							//タイマーキャンセル
			EntryTimer10ms(GEN_EXT_COMM_ERR_TIM, (ID)ID_CAN_FLG, (FLGPTN)F_CAN_TIMOUT_EXT);		//タイマーセット
			
		/* AVR基板通信断検知用タイマー設定 */	
		}else if(dataframe_no == 1){
			
			CancelTimer10ms((ID)ID_CAN_FLG, (FLGPTN)F_CAN_TIMOUT_AVR);							//タイマーキャンセル
			EntryTimer10ms(GEN_AVR_COMM_ERR_TIM, (ID)ID_CAN_FLG, (FLGPTN)F_CAN_TIMOUT_AVR);		//タイマーセット
			
		/* 表示基板通信断検知用タイマー設定 */
		}else if(dataframe_no == 2){
			
			CancelTimer10ms((ID)ID_CAN_FLG, (FLGPTN)F_CAN_TIMOUT_DISP);							//タイマーキャンセル
			EntryTimer10ms(GEN_DISP_COMM_ERR_TIM, (ID)ID_CAN_FLG, (FLGPTN)F_CAN_TIMOUT_DISP);	//タイマーセット
			
		}
		
	}else{																		//受信メッセージがない場合
		/* エラー処理 */
		/* 処理無し、必要であれば追加する */
		return ER_NG;															//異常終了
	}
	
	return ER_OK;																//正常終了
}
//******************************************************************************
//  機能     : 受信データ格納処理(外部コントロール)
//  機能説明 : CAN受信データをグローバル変数に格納する。
//------------------------------------------------------------------------------
//  引数     : UB dataframe_no :データフレームNo.0 外部コントロールデータ
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void data_copy_ext(UB dataframe_no)
{
	g_gen.ext_genstop           = can0_rx_dataframe[dataframe_no].data[0];		//発電機始動停止
	g_gen.ext_mode              = can0_rx_dataframe[dataframe_no].data[1];		//運転モード指令
	g_gen.ext_charge_reset      = can0_rx_dataframe[dataframe_no].data[2];		//充電器電流リセット
	g_gen.ext_none1             = can0_rx_dataframe[dataframe_no].data[3];		//未使用1(外部コントロール)
	g_gen.ext_gencurrent_limit  = can0_rx_dataframe[dataframe_no].data[4];		//発電機電流制限
	g_gen.ext_charge_mode       = can0_rx_dataframe[dataframe_no].data[5];		//充電器の充電モード
	g_gen.ext_can_debug         = can0_rx_dataframe[dataframe_no].data[6];		//CANデバッグモード
	g_gen.ext_regen_inhibit     = can0_rx_dataframe[dataframe_no].data[7];		//回生充電禁止
	
}
//******************************************************************************
//  機能     : 受信データ格納処理(AVR基板)
//  機能説明 : CAN受信データを移動平均化用バッファにコピーする。
//------------------------------------------------------------------------------
//  引数     : UB dataframe_no :データフレームNo.1 AVR基板受信データ
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void data_copy_avr(UB dataframe_no){
	
	/* 変数定義 */
	static UB avgbuf_avr_pointer = 0;											//移動平均化用バッファポインタ
	
	average_buf[INDX_ENGINE_RPM][avgbuf_avr_pointer]      = (can0_rx_dataframe[dataframe_no].data[0]) |		//エンジン回転数(下位)
															(can0_rx_dataframe[dataframe_no].data[1]<<8);	//エンジン回転数(上位)
	average_buf[INDX_GEN_VOLT_MEAS][avgbuf_avr_pointer]   = (can0_rx_dataframe[dataframe_no].data[2]) |		//発電電圧測定値(下位)
															(can0_rx_dataframe[dataframe_no].data[3]<<8);	//発電電圧測定値(上位)
	average_buf[INDX_GEN_VOLT_TARGET][avgbuf_avr_pointer] = (can0_rx_dataframe[dataframe_no].data[4]) |		//発電電圧目標値(下位)
															(can0_rx_dataframe[dataframe_no].data[5]<<8);	//発電電圧目標値(上位)
	
	avgbuf_avr_pointer++;														//ポインタを進める
	if(avgbuf_avr_pointer >= AVG_NUM){avgbuf_avr_pointer = 0;}					//ポインタを末尾からバッファ先頭に戻す
	
}
//******************************************************************************
//  機能     : 受信データ格納処理(表示基板)
//  機能説明 : CAN受信データを移動平均化用バッファにコピーする。
//------------------------------------------------------------------------------
//  引数     : UB dataframe_no :データフレームNo.2 表示基板受信データ
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void data_copy_disp(UB dataframe_no){
	
	/* 変数定義 */
	static UB avgbuf_disp_pointer = 0;											//移動平均化用バッファポインタ
	
	average_buf[INDX_GEN_CURR_R][avgbuf_disp_pointer] = (can0_rx_dataframe[dataframe_no].data[0]) |		//発電R相電流(下位)
														(can0_rx_dataframe[dataframe_no].data[1]<<8);	//発電R相電流(上位)
	average_buf[INDX_GEN_CURR_T][avgbuf_disp_pointer] = (can0_rx_dataframe[dataframe_no].data[2]) |		//発電T相電流(下位)
														(can0_rx_dataframe[dataframe_no].data[3]<<8);	//発電T相電流(上位)
	avgbuf_disp_pointer++;														//ポインタを進める
	if(avgbuf_disp_pointer >= AVG_NUM){avgbuf_disp_pointer = 0;}				//ポインタを末尾からバッファ先頭に戻す
	
}
//******************************************************************************
//  機能     : 受信データ移動平均化処理
//  機能説明 : CAN受信データの以下<対象データ>の移動平均を算出して
//           : グローバル変数に格納する。
//           :<対象データ>
//           : エンジン回転数(rpm),発電電圧測定値(V),発電電圧目標値(V),
//           : 発電R相電流(A),発電T相電流(A)
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void data_average(void){
	
	/* 変数定義 */
	UW avgwk;		//平均値算出用ワーク
	UB i;			//ループ用
	
	/* 移動平均化処理送信許可判定 */
	if(average_send_enable != 0){												//移動平均データ送信許可フラグ
		average_send_enable--;													//初期値AVG_NUMから減算して行き0で送信許可判定
	}
	
	/* エンジン回転数(rpm) */
	avgwk = 0;
	for(i=0;i<AVG_NUM;i++){
		avgwk = avgwk + average_buf[INDX_ENGINE_RPM][i];
	}
	g_gen.avr_rpm = avgwk / AVG_NUM;											//エンジン回転数(rpm)(移動平均後の値)
	
	/* 発電電圧測定値(V) */
	avgwk = 0;
	for(i=0;i<AVG_NUM;i++){
		avgwk = avgwk + average_buf[INDX_GEN_VOLT_MEAS][i];
	}
	g_gen.avr_volt_meas = avgwk / AVG_NUM;										//発電電圧測定値(V)(移動平均後の値)
	
	/* 発電電圧目標値(V) */
	avgwk = 0;
	for(i=0;i<AVG_NUM;i++){
		avgwk = avgwk + average_buf[INDX_GEN_VOLT_TARGET][i];
	}
	g_gen.avr_volt_target = avgwk / AVG_NUM;									//発電電圧目標値(V)(移動平均後の値)

	
	/* 発電R相電流(A) */
	avgwk = 0;
	for(i=0;i<AVG_NUM;i++){
		avgwk = avgwk + average_buf[INDX_GEN_CURR_R][i];
	}
	g_gen.disp_current_r = avgwk / AVG_NUM;										//発電R相電流(V)(移動平均後の値)
	
	
	/* 発電T相電流(A) */
	avgwk = 0;
	for(i=0;i<AVG_NUM;i++){
		avgwk = avgwk + average_buf[INDX_GEN_CURR_T][i];
	}
	g_gen.disp_current_t = avgwk / AVG_NUM;										//発電T相電流(A)(移動平均後の値)
	
}
//******************************************************************************
//  機能     : 全エラー初期化
//  機能説明 : 
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void reset_all_errors(void){
	
	/* 変数初期化 */
	can0_app_err_nr = APP_NO_ERR;												//CANAPIエラーNo.初期化
	can0_bus_status = R_CAN_STATUS_ERROR_ACTIVE;								//CAN bus状態：通常動作
	
	/* レジスタ初期化 */
	CAN0.EIFR.BYTE = 0;															//エラー割込み要因判定レジスタクリア
	CAN0.ECSR.BYTE = 0;															//エラーコード格納レジスタ
	CAN0.RECR = 0;																//受信エラーカウントレジスタクリア
	CAN0.TECR = 0;																//送信エラーカウントレジスタクリア
	
}
//******************************************************************************
//  機能     : バイトスワップ処理(UH16bit)
//  機能説明 : UH型変数の上位・下位を入れ替えて返す。
//           : 例：0x1234→0x3412
//------------------------------------------------------------------------------
//  引数     : UH val : 変換する値
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
UH swap_uh(UH val)
{
	return (UH)((val<<8) | (val>>8));
}
//******************************************************************************
//  機能     : バイトスワップ処理(UW32bit)
//  機能説明 : 例：0x12345678→0x78563412
//------------------------------------------------------------------------------
//  引数     : UW val : 変換する値
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
UW swap_uw(UW val)
{
	return (UW)((val<<24)|
				((val<<8) & 0x00FF0000) |
				((val>>8) & 0x0000FF00) |
				(val>>24));
}
//******************************************************************************
//  機能     : 絶対値算出処理(UH16bit)
//  機能説明 : 16bit変数の絶対値を返す。
//           : 但し、本関数は-32768の動作は保障しない。
//------------------------------------------------------------------------------
//  引数     : UH s  : 変換元の値
//------------------------------------------------------------------------------
//  戻り値   : UH s  : 引数が正数の場合の絶対値
//           : UH -s : 引数が負数の場合の絶対値
//******************************************************************************
H abs_s16(H s)
{
	return (0 > s) ? (-s) : s;
}

