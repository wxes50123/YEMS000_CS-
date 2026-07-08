//******************************************************************************
// @(R)
//		UDPタスク(クライアント)
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2026.03.16	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>								//割込みレベル操作用
#include "g_include_def.h"							//共通インクルードファイル
#include <string.h>									
//******** ファイル内環境宣言部 ************************************************
//定義無し
//******** 定数定義部 **********************************************************
#define CODE_EMS		0x454D						//識別コード("EM")(EMS)
#define CODE_PCS		0x5043						//識別コード("PC")(PCS)

#define UDP_ACK			0x0006						//ACK(アンサー値)
#define UDP_NAK			0x0015						//NAK(アンサー値)
#define ANS_NO			0x80						//アンサー番号(コマンド番号+この値)
#define PCS_RUN_CMD		0x10						//10:PCS運転コマンド
//******** 変数定義部 **********************************************************
static UB seq_no = 0x01;							//シーケンス番号
static UB seq_no_check;								//シーケンス番号(送信後のチェック用)
//******** プロトタイプ宣言部 **************************************************
TASK lancli_udp_task(void);							//UDPタスク
ER udp_cli_ini(T_SOC_CB *p_scb);					//UDPタスク初期化
void udp_seqno_upd(void);							//UDPシーケンス番号更新
ER udp_callback(ID cepid,FN fncd,VP parblk);		//UDPコールバック関数
static ER udp_cli_rcv(T_SOC_CB *p_scb);				//UDPデータ受信処理
static ER udp_cli_snd(T_SOC_CB *p_scb,UH len);		//UDPデータ送信処理
ER udp_rcvdt_runcmd_store(T_SOC_CB *p_scb);			//UDP受信データ｢PCS運転コマンド｣格納処理
UH udp_pcsruncmd_senddt_create(T_SOC_CB *p_scb);	//UDP送信データ作成｢PCS運転コマンド｣

//******** 関数定義部 **********************************************************
TASK lancli_udp_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	ER ercd;																	//エラーコード
	UH len;																		//送信文字数
	
	/* UDP制御ブロック情報初期化 */
	ercd = udp_cli_ini(&udpcb);													//LANクライアントタスク初期化
	if (ercd != E_OK) {ext_tsk();}												//UDP通信端点の生成エラーの場合
																				//端点生成に失敗した場合はタスク起動しない
    Pout_s("UDP/IP_TASK_RUN");													//デバッグ情報
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_LANCLI_UDP_FLG, &status);
		
		/* UDPイベント */
		if (status & F_UDP_EVENT) {
			
			/* 要求電文作成 */
			len = udp_pcsruncmd_senddt_create(&udpcb);							//UDP送信データ作成｢PCS運転コマンド｣
			/* 送信バッファ設定 */
			memcpy(udpcb.sbuf,udpcb.sndbuf_wk,len);
			/* 送信処理 */
			ercd = udp_cli_snd(&udpcb,len);
			if(ercd != ER_OK){
				/* 異常処理なし */
			}
			
			/* シーケンス番号更新 */
			udp_seqno_upd();													//シーケンス番号更新
			
		}
		
		/* UDP受信イベント */
		if (status &  F_UDP_RCVDT_RUNCMD_EVENT) {
			
			/* 受信処理 */
			ercd = udp_rcvdt_runcmd_store(&udpcb);								//UDP受信データ｢PCS運転コマンド｣格納処理
			if(ercd != ER_OK){
				/* 異常処理なし */
			}
			
		}
	}
}
//******** 内部関数定義部 ******************************************************
//******************************************************************************
//  機能     : LANクライアントタスク初期化
//  機能説明 : LANクライアントタスクの初期化処理を行う。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：UDP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : E_OK				：正常終了
//           : その他			：エラーコード
//******************************************************************************
ER udp_cli_ini(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	T_UDP_CCEP ccep;										//UDP用通信端点
	ER ercd;												//エラーコード
	
	/* 構造体初期化 */
	memset(p_scb, 0, sizeof(T_SOC_CB));						//制御ブロック初期化
	memset(&ccep, 0, sizeof(T_UDP_CCEP));					//UDP 通信端点生成情報パケット初期化
	
	
	//UDP通信端点生成情報
	ccep.callback = (FP)udp_callback;						//UDPコールバック関数の登録
	ccep.myaddr.ipaddr = byte4_to_long(default_ipaddr); 	//自分IP(10.10.10.45)
	ccep.myaddr.portno = UDP_MY_PORT;						//自分UDPポート番号(10001)
	
	//UDP制御ブロック情報
	p_scb->myaddr.ipaddr = byte4_to_long(default_ipaddr); 	//自分IP(10.10.10.45)
	p_scb->myaddr.portno = UDP_MY_PORT;						//自分UDPポート番号(10001)
	p_scb->dstaddr.ipaddr = byte4_to_long(dst_ipaddr1);		//相手IP(10.10.10.11)
	p_scb->dstaddr.portno = UDP_TG_PORT;					//相手UDPポート番号(10000)
	
	/* UDP通信端点の生成 */
	p_scb->cepid = ercd = udp_vcre_cep(&ccep);				//UDP通信端点の生成(ID自動割り当て)
	if (p_scb->cepid <= 0) {								//エラーチェック
		return ercd;
	}
	
	/* ブロードキャストパケットの送受信を許可 */
	udp_set_opt(p_scb->cepid, IP_BROADCAST,(VP)TRUE,sizeof(BOOL));
	
	return E_OK;
}
//******************************************************************************
//  機能     : UDPコールバック関数
//  機能説明 : ポートへUDPの受信要求が発生した場合に呼ばれる。
//------------------------------------------------------------------------------
//  引数     : ID cepid  : UDP通信端点ID
//           : FN fncd   : 終了したサービスコールの機能コード(TFN_UDP_RCV_DAT完了通知のみ)
//           : VP parblk : パケットの長さが格納されている領域へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : 戻り値無し(0)
//******************************************************************************
ER udp_callback(ID cepid,FN fncd,VP parblk){
	
	/* 変数定義 */
	int ipl;						//割禁マスクレベル
	ER ercd;						//エラーコード
	UH code;						//識別コード
	UB cmdno;						//コマンド番号
	
	ercd = udp_cli_rcv(&udpcb);													//受信処理
	if(E_OK == ercd){															//受信正常の場合
		
		ipl=get_ipl();															//割込み禁止
		set_ipl(15);
		memcpy(udpcb.rcvbuf_wk,udpcb.rbuf,sizeof(udpcb.rcvbuf_wk));
		set_ipl(ipl);															//割禁解除
		
		/* データ長チェック */
		if(udpcb.rcvlen == 0){return ER_NG;}
		
		/* 識別コードチェック */
		code = MAKE_U16(udpcb.rcvbuf_wk[0],udpcb.rcvbuf_wk[1]);
		if(code != CODE_PCS){return ER_NG;}										//PCS以外からの応答はエラー
		
		/* シーケンス番号チェック */
		if(udpcb.rcvbuf_wk[2] != seq_no_check){return ER_NG;}					//送信済みのシーケンス番号以外はエラー
		
		/* アンサー番号チェック */
		cmdno = (udpcb.rcvbuf_wk[3] - (UB)ANS_NO);								//アンサー番号からコマンド番号算出
		
		/* 受信電文判定 */
		switch(cmdno){
			
			case PCS_RUN_CMD:														//｢PCS運転コマンド｣
				set_flg((ID)ID_LANCLI_UDP_FLG, (FLGPTN)F_UDP_RCVDT_RUNCMD_EVENT);	//｢PCS運転コマンド｣受信イベント発行
				break;
				
				
				
				
				//その他コマンドを追記
				
				
				
			default:
				/* default文処理なし */
				break;
		}
		
	}
	return ER_OK;
}
//******************************************************************************
//  機能     : UDPシーケンス番号更新
//  機能説明 : シーケンス番号を更新する。受信時のチェック用に送信時のシーケンス
//           : 番号を記録する。
//------------------------------------------------------------------------------
//  引数     : void :
//------------------------------------------------------------------------------
//  戻り値   : void :
//******************************************************************************
void udp_seqno_upd(void)
{
	
	seq_no_check = seq_no;														//送信後、チェック用シーケンス番号を記録
	seq_no++;																	//シーケンス番号更新
	if(seq_no == 0){seq_no = 1;}												//0x00は使用しない
	
}
//******************************************************************************
//  機能     : UDPデータ受信処理
//  機能説明 : UDPでデータを受信する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：UDP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK			：正常終了
//           : その他			：エラーコード
//******************************************************************************
static ER udp_cli_rcv(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	TMO tmout = TMO_UDP_RCV;		//受信タイムアウト時間(3000msec)
	ER ercd;						//エラーコード
	
	ercd = udp_rcv_dat(p_scb->cepid , &p_scb->dstaddr , p_scb->rbuf , sizeof(p_scb->rbuf) , tmout);	//UDP 50byte
	Pout_itod("udp_rcv_dat=",ercd);Pout_LF();									//デバッグ情報出力
	if (ercd > 0) {																//受信バイト数が0より大きい場合
		p_scb->rcvlen = ercd;													//受信バイト数格納
		p_scb->ercd = E_OK;														//正常終了
	} else {																	//0以下の場合
		p_scb->ercd = ercd;														//エラーコードを設定
	}
	return p_scb->ercd;															//戻り値
}
//******************************************************************************
//  機能     : UDP受信データ｢PCS運転コマンド｣格納処理
//  機能説明 : 受信バッファに格納されたデータを取り出して解析して格納する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：UDP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK			：正常終了
//           : その他			：エラーコード
//******************************************************************************
ER udp_rcvdt_runcmd_store(T_SOC_CB *p_scb)
{
	
	/* 受信処理 */
	Pout_s("UDP_RUN_CMD_RCV");
	Pout_LF();
	
	
	return ER_OK;
}
//******************************************************************************
//  機能     : UDPデータ送信処理
//  機能説明 : UDPでデータを送信する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：UDP制御ブロック構造体へのポインタ
//           : UH len	：データ数(byte)
//------------------------------------------------------------------------------
//  戻り値   : ER_OK			：正常終了
//           : その他			：エラーコード
//******************************************************************************
static ER udp_cli_snd(T_SOC_CB *p_scb,UH len)
{
	/* 変数定義 */
	TMO tmout = TMO_UDP_SND;		//送信タイムアウト時間
	ER ercd;						//エラーコード(戻り値)
	
	ercd = udp_snd_dat(p_scb->cepid , &p_scb->dstaddr , p_scb->sbuf , len , tmout);
	if(ercd > 0){ercd = E_OK;}
	
	return ercd;
	
}
//******************************************************************************
//  機能     : UDP送信データ作成｢PCS運転コマンド｣
//  機能説明 : PCS運転コマンドの電文を作成する。
//           : 送信バッファに設定したデータ数を返す。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	： UDP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : UH len	：送信データ数(byte)
//******************************************************************************
UH udp_pcsruncmd_senddt_create(T_SOC_CB *p_scb){

	/*変数定義*/
	UH len = 0;			//データ数(64byte)
	UH fixed_val = 1;	//固定値(0x0001)
	
	/* 送信データ設定 */
	p_scb->sndbuf_wk[len++] = HI8_U16(CODE_EMS);											//識別コード"EM"(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(CODE_EMS);											//識別コード"EM"(下位)
	p_scb->sndbuf_wk[len++] = seq_no;														//シーケンス番号
	p_scb->sndbuf_wk[len++] = PCS_RUN_CMD;													//PCSコマンド番号
	
	/* データバイト長は末尾で格納 */
	p_scb->sndbuf_wk[len++] = 0;//HI8_U16(len);											//データバイト長(上位)ダミー格納
	p_scb->sndbuf_wk[len++] = 0;//LO8_U16(len);											//データバイト長(下位)ダミー格納
	
	p_scb->sndbuf_wk[len++] = HI8_U16(fixed_val);											//固定値:1(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(fixed_val);											//固定値:1(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.mode.val);							//運転モード番号(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.mode.val);							//運転モード番号(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.grid_input_power_limit.val);		//系統部 入力系統電力上限値(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.grid_input_power_limit.val);		//系統部 入力系統電力上限値(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.grid_regen_power_limit.val);		//系統部 回生系統電力上限値(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.grid_regen_power_limit.val);		//系統部 回生系統電力上限値(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.bat_chg_power_limit.val);			//蓄電池部 充電電力上限値(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.bat_chg_power_limit.val);			//蓄電池部 充電電力上限値(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.bat_dischg_power_limit.val);		//蓄電池部 放電電力上限値(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.bat_dischg_power_limit.val);		//蓄電池部 放電電力上限値(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.bat_chg_end_soc.val);				//蓄電池部 充電終了SOC(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.bat_chg_end_soc.val);				//蓄電池部 充電終了SOC(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.bat_dischg_end_soc.val);			//EV部 充電終了SOC(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.bat_dischg_end_soc.val);			//EV部 充電終了SOC(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.ev_chg_power_limit.val);			//EV部 充電電力上限値(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.ev_chg_power_limit.val);			//EV部 充電電力上限値(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.ev_dischg_power_limit.val);		//EV部 放電電力上限値(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.ev_dischg_power_limit.val);		//EV部 放電電力上限値(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.ev_chg_end_soc.val);				//EV部 充電終了SOC(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.ev_chg_end_soc.val);				//EV部 充電終了SOC(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.ev_dischg_end_soc.val);			//EV部 放電終了SOC(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.ev_dischg_end_soc.val);			//EV部 放電終了SOC(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.lamp_switch_cmd.val);				//ランプ点灯切り替え(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.lamp_switch_cmd.val);				//ランプ点灯切り替え(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.grid_stabilization_mode.val);		//系統安定化機能モード(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.grid_stabilization_mode.val);		//系統安定化機能モード(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.regen_active_power_limit.val);		//回生有効電力上限設定(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.regen_active_power_limit.val);		//回生有効電力上限設定(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.fixed_reactive_power.val);			//固定無効電力設定(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.fixed_reactive_power.val);			//固定無効電力設定(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.fixed_power_factor.val);			//固定力率設定(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.fixed_power_factor.val);			//固定力率設定(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.start_ramp_rate.val);				//開始上昇率(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.start_ramp_rate.val);				//開始上昇率(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.variation_rate.val);				//変動率(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.variation_rate.val);				//変動率(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.battery_4r_restart_cmd.val);		//4R電池再起動(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.battery_4r_restart_cmd.val);		//4R電池再起動(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.dischg_internal_resistance_sel.val);	//放電内部抵抗選択(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.dischg_internal_resistance_sel.val);	//放電内部抵抗選択(下位)
	
	/* 予備(16byte) */
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備1(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備1(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備2(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備2(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備3(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備3(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備4(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備4(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備5(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備5(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備6(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備6(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備7(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備7(下位)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備8(上位)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_udpsnd_pcsruncmd.yobi.val);					//予備8(下位)
	
	/* データ数設定 */
	p_scb->sndbuf_wk[4]  = HI8_U16(len);										//データバイト長(上位)
	p_scb->sndbuf_wk[5]  = LO8_U16(len);										//データバイト長(下位)
	
	return len;
	
}
