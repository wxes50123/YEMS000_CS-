//******************************************************************************
// @(R)
//		ModbusTCPタスク(クライアント)
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2025.12.13	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>						//割込みレベル操作用
#include "g_include_def.h"					//共通インクルードファイル
#include <string.h>							//memsetを使用
//******** ファイル内環境宣言部 ************************************************
//******** 定数定義部 **********************************************************
#define MBAP_HEADER_SIZE	7									//MBAPヘッダーサイズ7byte
#define HLDREG_CNT			14									//HOLDINGレジスタ数(32001～32014→14個)
#define HLDREG_START		32001								//HLODINGレジスタの先頭レジスタ
#define RCV_MAX_SIZE		150									//受信最大サイズ(>(MBAP_HEADER_SIZE+2+(66*2)))

#define PHASE_WRITE_HOLDING	0x01								//Holding Registers writeフェーズ
#define PHASE_READ_INPUT	0x02								//Input Registers readフェーズ
//******** 変数定義部 **********************************************************
//******** プロトタイプ宣言部 **************************************************
TASK modbus_tcp_task(void);										//ModbusTCPタスク
void tcp_cli_ini(T_SOC_CB *p_scb);								//ModbusTCPタスク初期化
ER tcp_callback(ID cepid,FN fncd,VP parblk);					//TCPコールバック関数
//static ER ainLinecharReceive(T_SOC_CB *p_scb);				//TCPデータ受信
ER tcp_cli_cep(T_SOC_CB *p_scb);								//TCP通信端点生成処理	(Connnection Endpoint)
ER tcp_cli_rep(T_SOC_CB *p_scb);								//TCP受付口生成処理		(Reception Point)
ER tcp_cli_con(T_SOC_CB *p_scb);								//TCP接続要求(能動オープン) (Accept)
ER tcp_cli_cls(T_SOC_CB *p_scb);								//TCP通信端点クローズ処理
ER tcp_cli_rcv(T_SOC_CB *p_scb);								//TCPデータ受信処理
ER tcp_cli_snd(T_SOC_CB *p_scb ,UH len);						//TCPデータ送信処理
ER tcp_rcv_data_store(T_SOC_CB *p_scb);							//受信データ格納処理
UH holdingreg_senddt_create(T_SOC_CB *p_scb);					//送信データ作成①(Write要求 EMS基板→I_DENCON:Holding Registers)
UH inputreg_senddt_create(T_SOC_CB *p_scb);						//送信データ作成②(Read要求 I_DENCON→EMS基板:Input Registers)
ER holdingreg_rcvdt_convert(UB *p_buf);							//受信データ変換取り込み①(Write要求 EMS基板→I_DENCON:Holding Registers)
ER inputreg_rcvdt_convert(UB *p_buf);							//受信データ変換取り込み②(Read要求 I_DENCON→EMS基板:Input Registers)
//******** 関数定義部 **********************************************************
TASK modbus_tcp_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	ER ercd;																	//エラーコード
	UB phase;																	//フェーズ(WRITE or READ)
	UH len;																		//送信文字数
	
	/* TCP制御ブロック情報初期化 */
	tcp_cli_ini(&tcpcb);														//構造体初期化、IPアドレス設定
	
	/* TCP通信端点生成 */
	ercd = tcp_cli_cep(&tcpcb);													//TCP通信端点生成処理	(Connnection Endpoint)
	Pout_itod("tcp_cli_cep=",ercd);Pout_LF();
	if(ercd < 0){ext_tsk();}													//異常時はタスク起動しない
	
	/* TCP制御ブロック情報 */
	tcpcb.stat = (UH)STS_CLI_ACP;												//ステータス更新(コネクション要求待ち)

	/* 送信データフェーズ設定 */
	phase = PHASE_WRITE_HOLDING;												//送信データ(Write要求 EMS基板→I_DENCON:Holding Registers)
	
    Pout_s("TCP/IP_TASK_RUN");													//デバッグ情報
	Pout_LF();
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_LANCLI_TCP_FLG, &status);
		
		/* TCPイベント */
		if (status & F_TCP_EVENT) {
			
			/* コネクション要求 */
			if(tcpcb.stat == STS_CLI_ACP){
			
				ercd = tcp_cli_con(&tcpcb);										//コネクション要求
				if(ercd == E_OK){												//接続完了時のみ次へ遷移
					tcpcb.stat = (UH)STS_CLI_SND;								//ステータス更新：データ送信へ
				}
				
			}
			
			/* データ送信 */
			if(tcpcb.stat == STS_CLI_SND){
				
				EntryTimer10ms(TMO_TCP_RESET, (ID)ID_LANCLI_TCP_FLG , (FLGPTN)F_TCP_RESET_EVENT);	//タイマーセット(受信でクリア)
				
				/* Holding Registers writeフェーズ */
				if(phase == PHASE_WRITE_HOLDING){
					
					len = holdingreg_senddt_create(&tcpcb);						//送信データ作成(EMS基板→I_DENCON:Holding Registers)
					ercd = tcp_cli_snd(&tcpcb,len);								//TCPデータ送信処理
					if(ercd == E_OK){
						tcpcb.stat = (UH)STS_CLI_RCVWAI;						//ステータス更新：データ受信へ
						tcp_cli_rcv(&tcpcb);									//受信待ちを設定
						phase = PHASE_READ_INPUT;								//フェーズ変更
					}else{
						
						tcpcb.stat = STS_CLI_CLS;								//コネクション切断へ
						
					}
					
				/* Input Registers readフェーズ */
				}else if(phase == PHASE_READ_INPUT){
					
					len = inputreg_senddt_create(&tcpcb);						//送信データ作成(EMS基板→I_DENCON:Holding Registers)
					ercd = tcp_cli_snd(&tcpcb,len);								//TCPデータ送信処理
					if(ercd == E_OK){
						tcpcb.stat = (UH)STS_CLI_RCVWAI;						//ステータス更新：データ受信へ
						tcp_cli_rcv(&tcpcb);									//受信待ちを設定
						phase = PHASE_WRITE_HOLDING;							//フェーズ変更
					}else{
						
						tcpcb.stat = STS_CLI_CLS;								//コネクション切断へ
						
					}
					
				}
				
			}
			
			/* コネクション切断 */
			if(tcpcb.stat == STS_CLI_CLS){
				ercd = tcp_cli_cls(&tcpcb);										//TCP通信端点クローズ処理
				if(ercd == E_OK || ercd == E_OBJ || ercd == E_TMOUT){
					
					tcpcb.stat = (UH)STS_CLI_ACP;								//ステータス更新：コネクション要求待ち
																				//切断検知後は接続要求待ち状態へ
																				
				}
			}
		}
		
		/* TCPリセットイベント */
		if (status & F_TCP_RESET_EVENT) {
			tcpcb.stat = STS_CLI_CLS;											//コネクション切断へ
		}
		
		/* TCP受信イベント */
		if (status & F_TCP_RCVDT_CPY_EVENT) {
			
			CancelTimer10ms((ID)ID_LANCLI_TCP_FLG, (FLGPTN)F_TCP_RESET_EVENT);	//タイマーキャンセル
			tcp_rcv_data_store(&tcpcb);											//受信データ格納処理
			tcpcb.stat = STS_CLI_SND;											//データ送信へ
			
		}
	}
}
//******** 内部関数定義部 ******************************************************

//******************************************************************************
//  機能     : LANクライアントタスク初期化
//  機能説明 : LANクライアントタスクの初期化処理を行う。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : 無し
//******************************************************************************
void tcp_cli_ini(T_SOC_CB *p_scb)
{
	/* 構造体初期化 */
	memset(p_scb, 0, sizeof(T_SOC_CB));											//制御ブロック初期化
	
	/* TCP制御ブロック情報 */
	p_scb->myaddr.ipaddr = byte4_to_long(default_ipaddr); 						//自IP 192.168.80.5
	p_scb->myaddr.portno = TCP_CLI_PORT;										//TCPポート番号
	p_scb->dstaddr.ipaddr = byte4_to_long(dst_ipaddr2);							//相手IP 192.168.80.101
	p_scb->dstaddr.portno = TCP_SRV_PORT;										//TCPポート番号
	
}
//******************************************************************************
//  機能     : TCP通信端点生成処理
//  機能説明 : TCP/IPの通信端点をID自動割り当てで生成する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK			：正常終了
//           : その他			：エラーコード
//******************************************************************************
ER tcp_cli_cep(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	ER ercd = ER_OK;															//エラーコード(戻り値)
	T_TCP_CCEP ccep;															//TCP用通信端点
	
	/* 構造体初期化 */
	memset(p_scb, 0, sizeof(T_SOC_CB));											//制御ブロック初期化
	memset(&ccep, 0, sizeof(T_TCP_CCEP));										//TCP通信端点生成情報パケット初期化
	
	/* TCP通信端点生成情報設定 */
	ccep.sbuf = p_scb->sbuf;													//送信バッファ先頭アドレス設定
	ccep.sbufsz = sizeof(p_scb->sbuf);											//送信バッファサイズ設定
	ccep.rbuf = p_scb->rbuf;													//受信バッファ先頭アドレス設定
	ccep.rbufsz = sizeof(p_scb->rbuf);											//受信バッファサイズ設定
	ccep.callback = (FP)tcp_callback;											//TCPコールバック関数の登録
	
	/* TCP制御ブロック情報 */
	p_scb->myaddr.ipaddr = byte4_to_long(default_ipaddr); 						//自分IP(10.10.10.45)
	p_scb->myaddr.portno = TCP_CLI_PORT;										//自分TCPポート番号(502)
	p_scb->dstaddr.ipaddr = byte4_to_long(dst_ipaddr2);							//相手IP(10.10.10.11)
	p_scb->dstaddr.portno = TCP_SRV_PORT;										//相手TCPポート番号(502)
	
	/* TCP通信端点生成 */
	p_scb->cepid = ercd = tcp_vcre_cep(&ccep);									//TCP通信端点の生成(ID自動割り当て)
	Pout_itod("p_scb->cepid=",p_scb->cepid);
	Pout_LF();
	return ercd;																//エラーチェック(0以下は異常、正常な場合1以上のIDが割当)
	
}
//******************************************************************************
//  機能     : TCP受付口生成処理
//  機能説明 : TCP/IPの受付口をID自動割り当てで生成する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ercd	：1以上：TCP受付口ID
//           : 		：0以下：エラーコード
//******************************************************************************
ER tcp_cli_rep(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	ER ercd = E_OK;																//エラーコード(戻り値)
	T_TCP_CREP crep;															//TCP受付口
	
	/* 構造体初期化 */
	memset(&crep, 0, sizeof(T_TCP_CREP));										//TCP用受付口初期化
	
	/* TCP用受付口生成情報 */
	crep.myaddr.ipaddr = p_scb->myaddr.ipaddr; 									//自IP 192.168.80.5
	crep.myaddr.portno = p_scb->myaddr.portno;									//(10001) TCPポート番号
	
	/* TCP受付口生成 */
	p_scb->repid = ercd = tcp_vcre_rep(&crep);									//TCP受付口の生成(ID自動割り当て)
	
	return ercd;																//(0以下は異常、正常な場合1以上のIDが割当)
	
}
//******************************************************************************
//  機能     : TCP接続要求(能動オープン)
//  機能説明 : TCP/IPの能動オープンする。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK			：正常終了
//           : その他			：エラーコード
//******************************************************************************
ER tcp_cli_con(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	TMO tmout = TMO_TCP_ACP;		//タイムアウト時間
	ER ercd;						//エラーコード(戻り値)
	
	/* 接続要求待ち(受動オープン) */
	ercd = tcp_con_cep(p_scb->cepid,&p_scb->myaddr,&p_scb->dstaddr,tmout);		//10秒タイムアウト

	return ercd;																//0(E_OK)以外で異常

}
//******************************************************************************
//  機能     : TCP通信端点クローズ処理
//  機能説明 : 生成済みのTCP通信端点をクローズ処理する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK			：正常終了
//           : その他			：エラーコード
//******************************************************************************
ER tcp_cli_cls(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	TMO tmout = TMO_TCP_CLS;		//タイムアウト時間
	ER ercd;						//エラーコード(戻り値)
	
	ercd = tcp_cls_cep(p_scb->cepid,tmout);										//通信端点クローズ
	
	return ercd;
}
//******************************************************************************
//  機能     : TCPデータ受信処理
//  機能説明 : TCP受信バッファにデータが受信されるまで待つ。
//           : 受信はコールバック関数に通知される。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : E_TMOUT(-50)	:正常終了(受信データなし)
//           : 正の値(>0)	:正常終了(取り出したデータの長さ)
//           : E_CLS(-87)	:TCP接続が切断された
//           : その他		:エラーコード
//******************************************************************************
ER tcp_cli_rcv(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	TMO tmout = TMO_TCP_RCV;		//ノンブロッキング
	ER ercd;						//エラーコード(戻り値)
	
	/* 受信処理 */
	ercd = tcp_rcv_dat(p_scb->cepid , p_scb->rcvbuf_wk ,RCV_MAX_SIZE , tmout);
	
	return ercd;
}
//******************************************************************************
//  機能     : TCPデータ送信処理
//  機能説明 : TCP送信バッファに格納されたデータを送信する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	：TCP制御ブロック構造体へのポインタ
//           : len :送信する文字数(バイト数)
//------------------------------------------------------------------------------
//  戻り値   : E_OK			：正常終了(正の値)
//           : その他		：エラーコード
//******************************************************************************
ER tcp_cli_snd(T_SOC_CB *p_scb,UH len)
{
	/* 変数定義 */
	TMO tmout = TMO_TCP_SND;		//送信タイムアウト時間
	ER ercd;						//エラーコード(戻り値)
	
	ercd = tcp_snd_dat(p_scb->cepid , p_scb->sndbuf_wk , len , tmout);
	
	if(ercd > 0){ercd = E_OK;}
	
	return ercd;
}
//******************************************************************************
//  機能     : TCPコールバック関数
//  機能説明 : ポートへTCPの受信要求が発生した場合に呼ばれる。
//------------------------------------------------------------------------------
//  引数     : ID cepid  : TCP通信端点ID
//           : FN fncd   : 終了したサービスコールの機能コード(TFN_TCP_RCV_DAT完了通知のみ)
//           : VP parblk : パケットの長さが格納されている領域へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : 無し(コールバック関数は戻り値未使用なNORTi仕様)
//******************************************************************************
ER tcp_callback(ID cepid,FN fncd,VP parblk){
	
	/* 変数定義 */
	ER len;						//受信データ長(byte)
	
	
	len = *(ER *)parblk;		//受信データ長取得
	if(len>0){
	
		tcpcb.rcvlen = len;														//受信データ長格納
		set_flg((ID)ID_LANCLI_TCP_FLG, (FLGPTN)F_TCP_RCVDT_CPY_EVENT);			//受信イベント発行
		
	}else{
		/* 回線正常切断判定 */
		tcpcb.stat = STS_CLI_CLS;												//ステータス更新：コネクション切断へ
		
		/* PCS1通信断回復 */
		g_pcs1.comm_error_flg = FLG_OFF;										//通信断回復(0)
	}
	
	return ER_OK;
}
//******************************************************************************
//  機能     : 受信データ格納処理
//  機能説明 : 受信バッファに格納されたデータを取り出して解析して格納する。
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb	: TCP制御ブロック構造体へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK : 正常終了
//           : ER_NG : 異常終了(電文内容エラー)
//******************************************************************************
ER tcp_rcv_data_store(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	ER ercd;					//エラーコード
	UH transaction_id = 0;		//トランザクションID
	
	/* TCP受信完了 */
	if(p_scb->rcvlen == 0){return ER_NG;}
	
	
	/* トランザクションIDで判定 */
	transaction_id = MAKE_U16(p_scb->rcvbuf_wk[0],p_scb->rcvbuf_wk[1]);			//トランザクションIDを結合
	
	/* トランザクションID:1 */
	if(transaction_id == 0x0001){												//応答(Write要求 EMS基板→I_DENCON:Holding Registers)
		
		ercd = holdingreg_rcvdt_convert(p_scb->rcvbuf_wk);						//受信データ変換取り込み①(Write要求 EMS基板→I_DENCON:Holding Registers)
		if(ercd != ER_OK){
			/* エラー処理 */
			/* 処理無し、必要であれば追加する */
			return ER_NG;														//電文不正
		}
		
	/* トランザクションID:2 */
	}else if(transaction_id == 0x0002){											//応答(Read要求  I_DENCON→EMS基板:Input Registers)
		
		ercd = inputreg_rcvdt_convert(p_scb->rcvbuf_wk);						//受信データ変換取り込み②(Read要求 I_DENCON→EMS基板:Input Registers)
		if(ercd != ER_OK){
			/* エラー処理 */
			/* 処理無し、必要であれば追加する */
			return ER_NG;														//電文不正
		}
	
	}else{
	
		return ER_NG;															//トランザクションIDが不正
		
	}
	
	/* PCS1通信断回復 */
	g_pcs1.comm_error_flg = FLG_OFF;											//通信断回復(0)
	
	/* ステータス更新 */
	tcpcb.stat = (UH)STS_CLI_SND;												//データ送信へ
	
	return ER_OK;
	
	
}
//******************************************************************************
//  機能     : 送信データ作成①(Write要求 EMS基板→I_DENCON:Holding Registers)
//  機能説明 : PCS1(I_DENCON)のHolding Registersに送信するデータを作成する。
//           : 書き込みレジスタ数：32001～32014(14個、空きを含む)
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb：ソケット通信制御ブロック
//------------------------------------------------------------------------------
//  戻り値   : UH len :送信データ長(byte)
//******************************************************************************
UH holdingreg_senddt_create(T_SOC_CB *p_scb)
{	
	
	/* 変数定義 */
	UH len = 0;		//データ数
	
	/* ワークバッファ初期化 */
	memset(p_scb->sndbuf_wk,0,sizeof(p_scb->sndbuf_wk));
	
	/* MBAP header(7byte) */
	p_scb->sndbuf_wk[len++] = 0x00;					//トランザクションID(Hi)(0固定)
	p_scb->sndbuf_wk[len++] = 0x01;					//トランザクションID(Lo)(1固定)
	p_scb->sndbuf_wk[len++] = 0x00;					//プロトコル識別子(Hi)(0固定)
	p_scb->sndbuf_wk[len++] = 0x00;					//プロトコル識別子(Lo)(0固定)
	p_scb->sndbuf_wk[len++] = 0x00;					//データバイト長(Hi)
	p_scb->sndbuf_wk[len++] = 0x1f;					//データバイト長(Lo)(1+30)
	p_scb->sndbuf_wk[len++] = 0x01;					//ユニットID(1固定)
	/* PDU(34byte) */
	p_scb->sndbuf_wk[len++] = 0x10;					//ファンクションコード(0x10:Write Multiple Registers)
	p_scb->sndbuf_wk[len++] = (UB)(HLDREG_START >>8) & 0xFF;		//0x7D;					//スタートアドレス(Hi)(32001)
	p_scb->sndbuf_wk[len++] = (UB)(HLDREG_START & 0xFF);			//0x01;					//スタートアドレス(Lo)
	p_scb->sndbuf_wk[len++] = 0x00;					//レジスタ数(Hi)
	p_scb->sndbuf_wk[len++] = 0x0c;					//レジスタ数(Lo)
	p_scb->sndbuf_wk[len++] = (HLDREG_CNT * 2);		//バイト数14*2
	/* DATA(holding registers) */
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.ev_chg_power.val);		//32001	EV充電電力(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.ev_chg_power.val);		//		EV充電電力(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.ev_dischg_power.val);		//32002	EV放電電力(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.ev_dischg_power.val);		//		EV放電電力(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.bat_chg_power.val);		//32003	電池充電電力(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.bat_chg_power.val);		//		電池充電電力(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.bat_dischg_power.val);	//32004	電池放電電力(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.bat_dischg_power.val);	//		電池放電電力(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.grid_in_power.val);		//32005	系統入力電力(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.grid_in_power.val);		//		系統入力電力(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.grid_out_power.val);		//32006	系統出力電力(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.grid_out_power.val);		//		系統出力電力(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.ev_soc_max.val);			//32007	EV充電上限SoC(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.ev_soc_max.val);			//		EV充電上限SoC(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.ev_soc_min.val);			//32008	EV放電下限SoC(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.ev_soc_min.val);			//		EV放電下限SoC(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.bat_soc_max.val);			//32009	電池充電上限SoC(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.bat_soc_max.val);			//		電池充電上限SoC(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.bat_soc_min.val);			//32010	電池充電下限SoC(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.bat_soc_min.val);			//		電池充電下限SoC(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.ope_mode.val);			//32011	運転パターン(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.ope_mode.val);			//		運転パターン(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.error_reset.val);			//32012	エラーリセット(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.error_reset.val);			//		エラーリセット(Lo)

//やまびこ殿場内のI_DENCONはコメントアウトで確認
//	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.evconnnected.val);		//32013	EV接続/切断(Hi)
//	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.evconnnected.val);		//		EV接続/切断(Lo)
//	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.yobi.val);				//32014	空き(Hi)
//	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.yobi.val);				//		空き(Lo)

	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.evconnnected.val);		//32013	EV接続/切断(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.evconnnected.val);		//		EV接続/切断(Lo)
	p_scb->sndbuf_wk[len++] = HI8_U16(g_modbustcp_holding.yobi.val);				//32014	空き(Hi)
	p_scb->sndbuf_wk[len++] = LO8_U16(g_modbustcp_holding.yobi.val);				//		空き(Lo)
	
	return len;
	
}
//******************************************************************************
//  機能     : 送信データ作成②(Read要求  I_DENCON→EMS基板:Input Registers)
//  機能説明 : PCS1(I_DENCON)のInput Registersからデータを読み出す。
//           : 読出しレジスタ数：67個(空きを含む)
//------------------------------------------------------------------------------
//  引数     : T_SOC_CB *p_scb：ソケット通信制御ブロック
//------------------------------------------------------------------------------
//  戻り値   : UH len :送信データ長(byte)
//******************************************************************************
UH inputreg_senddt_create(T_SOC_CB *p_scb)
{
	/* 変数定義 */
	UH len = 0;				//データ長
	
	/* ワークバッファ初期化 */
	memset(p_scb->sndbuf_wk,0,sizeof(p_scb->sndbuf_wk));
	
	/* MBAP header(7byte) */
	p_scb->sndbuf_wk[len++] = 0x00;				//トランザクションID(Hi)(0固定)
	p_scb->sndbuf_wk[len++] = 0x02;				//トランザクションID(Lo)(2固定)
	p_scb->sndbuf_wk[len++] = 0x00;				//プロトコル識別子(Hi)(0固定)
	p_scb->sndbuf_wk[len++] = 0x00;				//プロトコル識別子(Lo)(0固定)
	p_scb->sndbuf_wk[len++] = 0x00;				//データバイト長(Hi)
	p_scb->sndbuf_wk[len++] = 0x06;				//データバイト長(Lo)(ユニットID1byte+PDU5byte)
	p_scb->sndbuf_wk[len++] = 0x01;				//ユニットID(0固定)
	/* PDU(5byte) */
	p_scb->sndbuf_wk[len++] = 0x04;				//ファンクションコード(0x04:Read Input Registers)
	p_scb->sndbuf_wk[len++] = 0x00;				//スタートアドレス(Hi)
	p_scb->sndbuf_wk[len++] = 0x00;				//スタートアドレス(Lo)(アドレス0)
	p_scb->sndbuf_wk[len++] = 0x00;				//レジスタ数(Hi)
	p_scb->sndbuf_wk[len++] = 0x43;				//レジスタ数(Lo)(レジスタ67個)
//やまびこ殿場内のI_DENCONはこっちで確認⇒	p_scb->sndbuf_wk[len++] = 0x30;				//レジスタ数(Lo)(レジスタ67個)
	
	return len;
}
//******************************************************************************
//  機能     : 受信データ変換取り込み①(EMS基板→I_DENCON:Holding Registers)
//  機能説明 : PCS1(I_DENCON)のHolding Registersから受信したデータを
//           : EMS基板のグローバル変数へ取り込む。
//------------------------------------------------------------------------------
//  引数     : UB *p_buf：ローカルバッファ用ポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK : 正常終了(0)
//           : ER_NG : 異常終了(-1)
//******************************************************************************
ER holdingreg_rcvdt_convert(UB *p_buf)
{
	/* 変数定義 */
	UH trans_id;			//トランザクションID
	UB func;				//ファンクションコード
	
	/* ヘッダー取得 */
	trans_id = MAKE_U16(p_buf[0],p_buf[1]);										//トランザクションID取得
	func = p_buf[MBAP_HEADER_SIZE];												//ファンクションコード取得
	
	/* 異常判定 */
	if(trans_id != 0x0001){return ER_NG;}										//トランザクションIDが1以外は異常
	if(func != 0x10){return ER_NG;}												//0x10:複数Holding Registers書き込み以外異常
	
	return ER_OK;
	
}
//******************************************************************************
//  機能     : 受信データ変換取り込み②(I_DENCON→EMS基板:Input Registers)
//  機能説明 : PCS1(I_DENCON)のInput Registersから受信したデータを
//           : EMS基板のグローバル変数へ取り込む。
//------------------------------------------------------------------------------
//  引数     : UB *p_buf：ローカルバッファ用ポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK : 正常終了(0)
//           : ER_NG : 異常終了(-1)
//******************************************************************************
ER inputreg_rcvdt_convert(UB *p_buf)
{
	/* 変数定義 */
	UH trans_id;			//トランザクションID
	UB func;				//ファンクションコード
//	UH reg_cnt;				//レジスタ数
	UB *p_reg;				//レジスタデータ先頭
	
	/* ヘッダー取得 */
	trans_id = MAKE_U16(p_buf[0],p_buf[1]);										//トランザクションID取得
	func = p_buf[MBAP_HEADER_SIZE];												//ファンクションコード取得
	
	/* 異常判定 */
	if(trans_id != 0x0002){return ER_NG;}										//トランザクションIDが2以外は異常
	if(func != 0x04){return ER_NG;}												//0x04:InputRegisters読出し以外異常
	
//	reg_cnt = (UH)(byte_cnt/2);													//レジスタ数算出
	p_reg = &p_buf[MBAP_HEADER_SIZE +2];										//レジスタデータ先頭
	
	
	/* レジスタデータ変換・取得 */
	g_modbustcp_input.heartbeat.val = 			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//00 ハートビート
	g_modbustcp_input.system_state.val = 		(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//01 システム状態
	g_modbustcp_input.grid_state.val = 			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//02 系統状態
	g_modbustcp_input.ev_state.val = 			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//03 EV状態
	g_modbustcp_input.ev_connected.val =		(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//04 EV接続
	g_modbustcp_input.pv_state.val =			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//05 PV状態
	g_modbustcp_input.bat_state.val =			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//06 電池状態
	g_modbustcp_input.error_code.val =			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//07 エラーコード
	g_modbustcp_input.internal_temp1.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//08 内部温度1
	g_modbustcp_input.internal_temp2.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//09 内部温度2
	g_modbustcp_input.internal_temp3.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//10 内部温度3
	g_modbustcp_input.internal_temp4.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//11 内部温度4
	g_modbustcp_input.internal_temp5.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//12 内部温度5
	g_modbustcp_input.enclosure_temp.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//13 筐体温度
	g_modbustcp_input.bat_temp_max.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//14 電池最高温度
	g_modbustcp_input.bat_temp_avg.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//15 電池平均温度
	g_modbustcp_input.bat_temp_min.val =		(B)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//16 電池最低温度
	g_modbustcp_input.grid_v_rs.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//17 系統RS相電圧
	g_modbustcp_input.grid_v_st.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//18 系統ST相電圧
	g_modbustcp_input.grid_v_tr.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//19 系統TR相電圧
	g_modbustcp_input.grid_i_r.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//20 系統R相電流
	g_modbustcp_input.grid_i_s.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//21 系統S相電流
	g_modbustcp_input.grid_i_t.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//22 系統T相電流
	g_modbustcp_input.grid_pf_r.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//23 系統R相力率
	g_modbustcp_input.grid_pf_s.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//24 系統S相力率
	g_modbustcp_input.grid_pf_t.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//25 系統T相力率
	g_modbustcp_input.grid_freq.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//26 系統周波数
	g_modbustcp_input.grid_p_active.val =		(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//27 系統有効電力
	g_modbustcp_input.grid_p_reactive.val =		(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//28 系統無効電力
	g_modbustcp_input.ev_voltage.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//29 EV電圧
	g_modbustcp_input.ev_current.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//30 EV電流
	g_modbustcp_input.ev_soc.val =				(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//31 EV SOC
	g_modbustcp_input.pv_voltage.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//32 PV電圧
	g_modbustcp_input.pv_current.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//33 PV電流
	g_modbustcp_input.bat_voltage.val =			(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//34 電池電圧
	g_modbustcp_input.bat_current.val =			(H)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//35 電池電流
	g_modbustcp_input.bat_soc.val =				(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//36 電池SOC
	g_modbustcp_input.ev_chg_power.val =		(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//37 EV充電電力
	g_modbustcp_input.ev_dischg_power.val =		(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//38 EV放電電力
	g_modbustcp_input.bat_chg_power.val =		(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//39 電池充電電力
	g_modbustcp_input.bat_dischg_power.val =	(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//40 電池放電電力
	g_modbustcp_input.grid_input_power.val =	(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//41 系統入力電力
	g_modbustcp_input.grid_output_power.val =	(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//42 系統出力電力
	g_modbustcp_input.ev_soc_chg_limit.val = 	(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//43 EV充電上限SOC
	g_modbustcp_input.ev_soc_dischg_limit.val =	(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//44 EV充電下限SOC
	g_modbustcp_input.bat_soc_chg_limit.val =	(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//45 電池充電上限SOC
	g_modbustcp_input.bat_soc_dischg_limit.val =(UH)MAKE_U16(p_reg[0],p_reg[1]);p_reg+=2;			//46 電池放電下限SOC
	g_modbustcp_input.ope_mode.val =			(UB)(MAKE_U16(p_reg[0],p_reg[1])&0x00FF);p_reg+=2;	//47 運転パターン
//やまびこ殿場内のI_DENCONはコメントアウトで確認
//	g_modbustcp_input.grid_energy_in_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//48-49 系統入力電力量積算
//	g_modbustcp_input.grid_energy_out_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//50-51 系統出力電力量積算
//	g_modbustcp_input.ev_energy_chg_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//52-53 EV充電電力量積算
//	g_modbustcp_input.ev_energy_dischg_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//54-55 EV放電電力量積算
//	g_modbustcp_input.bat_energy_chg_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//56-57 電池充電電力量積算
//	g_modbustcp_input.bat_energy_dischg_wh.val =(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//58-59 電池放電電力量積算
//	g_modbustcp_input.pv_energy_wh.val =		(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));			//60-61 PV電力量積算
	
	g_modbustcp_input.grid_energy_in_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//48-49 系統入力電力量積算
	g_modbustcp_input.grid_energy_out_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//50-51 系統出力電力量積算
	g_modbustcp_input.ev_energy_chg_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//52-53 EV充電電力量積算
	g_modbustcp_input.ev_energy_dischg_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//54-55 EV放電電力量積算
	g_modbustcp_input.bat_energy_chg_wh.val =	(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//56-57 電池充電電力量積算
	g_modbustcp_input.bat_energy_dischg_wh.val =(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));p_reg+=4;	//58-59 電池放電電力量積算
	g_modbustcp_input.pv_energy_wh.val =		(UW)(MAKE_U32(MAKE_U16(p_reg[0],p_reg[1]),MAKE_U16(p_reg[2],p_reg[3])));			//60-61 PV電力量積算
	
	return ER_OK;
}