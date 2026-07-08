//******************************************************************************
// @(R)
//		DIタスク
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.12.01	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include	<machine.h>															//割込みレベル操作用
#include	"g_include_def.h"													//共通インクルードファイル
//#include "g_extern_def.h"		//共通関数定義
//******** ファイル内環境宣言部 ************************************************
//定義無し
//******** 定数定義部 **********************************************************
//定義無し
//******** 変数定義部 **********************************************************
//定義無し
//******** プロトタイプ宣言部 **************************************************
TASK di_task(void);																//DIリードタスク
void di_read(void);																//入力信号現在値読み込み
void di_check(void);															//入力信号変化チェック(押しボタン制御)
void di_event(UB no);															//入力信号イベント処理
//******** 関数定義部 **********************************************************
TASK di_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	
    Pout_s("DI_TASK_RUN");														//デバッグ情報
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_DI_FLG, &status);
		
		/* イベント */
		if (status & F_DI_EVENT) {												//20ms周期起動
			
			di_read();															//入力信号現在値読み込み
			di_check();															//入力信号変化チェック
			
		}
		
	}
}
//******************************************************************************
//  機能     : 入力信号現在値読み込み
//  機能説明 : EMS基板のDI信号の現在値を各ポートから読み込む。
//           : 接点ONを(1)、接点OFFを(0)として読み込む。
//           : マイコンポート側はプルアップされているので、逆になるが
//           : ソフト上の論理は逆とする。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void di_read(void){
	//メンテナンスSW(DI確定時間が異なるので別判定するため、先頭に定義)
	g_di[PCS_MAINTE_SW].wk	= !DI_PCS_MAINTE_SW;								//PCSメンテナンスSW
	g_di[PCS_START_SW].wk 	= !DI_PCS_START_SW;									//PCS起動SW(自立運転開始)
	g_di[PCS_STOP_SW].wk	= !DI_PCS_STOP_SW;									//PCS停止SW(自立運転停止)
	g_di[PCS_AUX3].wk		= !DI_PCS_AUX3;										//AUX3
	g_di[YOBI1].wk			= !DI_YOBI1;										//予備1(未使用)
	g_di[YOBI2].wk			= !DI_YOBI2;										//予備2(未使用)
	g_di[YOBI3].wk			= !DI_YOBI3;										//予備3(未使用)
	g_di[YOBI4].wk			= !DI_YOBI4;										//予備4(未使用)
	/* DIが増える場合は追記 */
}
//******************************************************************************
//  機能     : 入力信号変化チェック(押しボタン制御)
//  機能説明 : EMS基板のDI信号の変化状態をチェックする。
//           : 連続して規定回数(DI_COUNT)時間変化した信号があった場合は
//           : グローバルのDI管理変数へ反映する。
//           : 規定回数待つのは、ノイズ・チャタリング対策のため。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void di_check(void){
	
	/* 変数定義 */
	UB i;
	
	
	/* メンテナンスSW以外の入力チェック */
	for(i=1;i<DI_MAX;i++){														//g_di[1]からチェックし、メンテナンスSWを除く
		
		if(g_di[i].old == g_di[i].wk){											//前回値と現在値が同じ場合
			
			if(g_di[i].cnt < DI_COUNT){g_di[i].cnt++;}							//規定回数未満ならカウントアップ
			
		}else{
			
			g_di[i].cnt = 0;													//前回値と現在値が異なる場合はカウンタクリア
			
		}
		
		if(g_di[i].cnt >= DI_COUNT){											//規定回数以上になった場合
			
			if(g_di[i].val != g_di[i].wk){
				
				g_di[i].val = g_di[i].wk;										//確定値として反映
				di_event(i);													//イベント処理(確定値となった初回のみ)
			}
		}
		
		g_di[i].old = g_di[i].wk;												//現在値を前回値へ格納
		
	}
	
	/* メンテナンスSWの入力チェック */
	if(g_di[PCS_MAINTE_SW].old == g_di[PCS_MAINTE_SW].wk){						//前回値と現在値が同じ場合
		
		if(g_di[PCS_MAINTE_SW].cnt <= DI_COUNT_MNT_10S){g_di[PCS_MAINTE_SW].cnt++;}	//規定回数未満ならカウントアップ
		
	}else{
		
		g_di[PCS_MAINTE_SW].cnt = 0;											//前回値と現在値が異なる場合はカウンタクリア
		
	}
	if(g_di[PCS_MAINTE_SW].wk == MNTSW_ON){
		/*10秒以上押下*/
		if(g_di[PCS_MAINTE_SW].cnt >= DI_COUNT_MNT_10S){						//規定回数以上になった場合
			
			if(g_di[PCS_MAINTE_SW].val != MNTSW_10S){
				
				g_di[PCS_MAINTE_SW].val = MNTSW_10S;							//メンテナンスSW状態反映(3:10秒以上)
				di_event(PCS_MAINTE_SW);										//イベント処理(確定値となった初回のみ)
				Pout_s("10s_UP");Pout_LF();
			}
		/* 5秒以上押下 */
		}else if(g_di[PCS_MAINTE_SW].cnt >= DI_COUNT_MNT_5S){
			
			if(g_di[PCS_MAINTE_SW].val != MNTSW_5S){
				
				g_di[PCS_MAINTE_SW].val = MNTSW_5S;								//メンテナンスSW状態反映(2:5秒以上)
				di_event(PCS_MAINTE_SW);										//イベント処理(確定値となった初回のみ)
				Pout_s("5s_UP");Pout_LF();
			}
		/* 1秒以上押下 */
		}else if(g_di[PCS_MAINTE_SW].cnt >= DI_COUNT_MNT_1S){
			
			if(g_di[PCS_MAINTE_SW].val != MNTSW_1S){
				
				g_di[PCS_MAINTE_SW].val = MNTSW_1S;								//メンテナンスSW状態反映(1:1秒以上)
				di_event(PCS_MAINTE_SW);										//イベント処理(確定値となった初回のみ)
				Pout_s("1s_UP");Pout_LF();
			}
		/* 20ms以上押下 */
		}else if(g_di[PCS_MAINTE_SW].cnt >= DI_COUNT){
			
			if(g_di[PCS_MAINTE_SW].val ==  MNTSW_OFF){
				g_di[PCS_MAINTE_SW].val = MNTSW_ON;								//メンテナンスSW状態反映(1:1秒以上)
				di_event(PCS_MAINTE_SW);										//イベント処理(確定値となった初回のみ)
				Pout_s("20ms_UP");Pout_LF();
			}
		}
		
	}else{
		/* 20ﾐﾘ秒以上OFF */
		if(g_di[PCS_MAINTE_SW].cnt >= DI_COUNT){
			
			if(g_di[PCS_MAINTE_SW].val != MNTSW_OFF){
			
				g_di[PCS_MAINTE_SW].val = MNTSW_OFF;							//メンテナンスSW状態反映(0:OFF)
				di_event(PCS_MAINTE_SW);										//イベント処理(確定値となった初回のみ)
				Pout_s("OFF");Pout_LF();
			}
			
		}
	}
	g_di[PCS_MAINTE_SW].old = g_di[PCS_MAINTE_SW].wk;							//現在値を前回値へ格納
	
}
//******************************************************************************
//  機能     : 入力信号イベント処理
//  機能説明 : 入力信号に応じた処理を設定する。
//           : 本関数は信号変化検知時の初回のみ呼び出される。
//------------------------------------------------------------------------------
//  引数     : UB no : DI管理No.
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void di_event(UB no){
	
	/* 変数定義 */
	
	switch(no){
		
		case PCS_MAINTE_SW:	/* (0)メンテナンスSW */
			
			
			if(g_di[PCS_MAINTE_SW].val == (UB)MNTSW_10S){
				
				//強制リセットフラグセット
				
			}else if(g_di[PCS_MAINTE_SW].val == (UB)MNTSW_5S){
				
				//エラーリセットフラグセット
				
			}else if(g_di[PCS_MAINTE_SW].val == (UB)MNTSW_1S){
				
				//自立→系統連携切り替えフラグセット
				
			}else if(g_di[PCS_MAINTE_SW].val == 1){
				//処理なし
				
			}else{
				//LED OFF
					SET_DO(DO_PCS_MAINTESW_LED, BIT_OFF);	//メンテナンスSW LED
				
			}
			
			
			break;
			
		case PCS_START_SW:	/* (1)PCS起動SW */
			
			g_pcs1.state_flg = FLG_ON;											//PCS状態：自立運転中
			
			break;
			
		case PCS_STOP_SW:	/* (2)PCS停止SW */
			
			g_pcs1.state_flg = FLG_OFF;											//PCS状態：自立運転停止中
			
			break;
			
		case PCS_AUX3:		/* (3)AUX3 */
																				//処理無し
			break;
			
		case YOBI1:/* (4)未使用1 */
			/* 処理無し */
			break;
		case YOBI2:/* (5)未使用2 */
			/* 処理無し */
			break;
		case YOBI3:/* (6)未使用3 */
			/* 処理無し */
			break;
		case YOBI4:/* (7)未使用4 */
			/* 処理無し */
			break;
		default:
			/* 処理無し */
			break;
	}
}	