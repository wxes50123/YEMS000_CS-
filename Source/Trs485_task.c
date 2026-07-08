//******************************************************************************
// @(R)
//		RS485タスク(シリアル通信用 通信対象：スイッチング電源(COTEK/AEK-3000))
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//ver 1.0	2026.03.19	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>															//割込みレベル操作用
#include "g_include_def.h"														//共通インクルードファイル
#include <string.h>																//
//******** ファイル内環境宣言部 ************************************************
//定義無し
//******** 定数定義部 **********************************************************
/* シリアル定義 */
#define UART_CH	0						//本タスクで使用するUARTチャンネルNo.
#define SCI_TEND	SCI0.SSR.BIT.TEND	//送信終了フラグ(シリアルステータスレジスタ)
#define SEND_TIMOUT SCI0_SEND_TIMOUT	//送信待ち時間
#define SEND_ENABLE	SCI0_SEND_ENABLE	//送信許可(受信無効)
#define RECV_ENABLE	SCI0_RECV_ENABLE	//受信許可(送信無効)
/* 送信電文種別 */
#define SEND_ADDS	0x01				//アドレス設定
#define SEND_REMS	0x02				//リモート設定
#define SEND_P_ON	0x03				//PowerON設定
#define SEND_P_OFF	0x04				//PowerOFF設定
#define SEND_ST_SV	0x07				//電圧値設定
#define SEND_ST_SI	0x08				//電流値設定
#define SEND_RV		0x09				//現在値読出し(電圧値)
#define SEND_RI		0x0A				//現在値読出し(電流値)
//******** 変数定義部 **********************************************************
static UB send_timout_flg = FLG_OFF;											//送信タイムアウトフラグ
static UB cmd_set_adds[]="ADDS  \r\n";											//ADDS設定
static UB cmd_set_rems[]="REMS  \r\n";											//リモート設定
static UB cmd_set_pon[]="POWER 1\r\n";											//電源ON
static UB cmd_set_poff[]="POWER 0\r\n";											//電源OFF
static UB cmd_read_rv[]="RV?\r\n";												//現在電圧読出し
static UB cmd_read_ri[]="RI?\r\n";												//現在電流読出し
//******** プロトタイプ宣言部 **************************************************
TASK rs485_task(void);															//RS485タスク
ER rs485_send(UB mode);															//電文送信処理
ER rs485_recv(UB mode);															//電文受信処理
void rs485_send_wait(void);														//送信完了待ち
ER find_reply_token(UB *p);														//応答電文チェック
void parse_fixed_to_from_line(UB *p, UW *out_x100);								//数値取得処理
UH rs485_rv_store(UB *buf);														//RV?コマンド受信処理
UH rs485_ri_store(UB *buf);														//RI?コマンド受信処理
static UB u32_to_dec_ascii(UB *dst, UW v);										//U32toASCII(整数,符号なし)
static UB u32_to_fixed2_ascii(UB *dst, UW v_0p01);								//U32toACCII(整数.小数2桁)
static UB rs485_make_cmd_fixed2(UB *send_buf,UB c1, UB c2,UW value_0p01,UW max_0p01);	//コマンド電文作成処理
UB rs485_make_sv_fixed2(UB *send_buf, UW vol_0p01v);									//電圧設定SVデータ作成
UB rs485_make_si_fixed2(UB *send_buf, UW cur_0p01a);									//電流設定SIデータ作成
//******** 関数定義部 **********************************************************
TASK rs485_task(void)
{
	/* 変数定義 */
	UINT status;																//イベントステータス
	ER ercd;																	//エラーコード
	
	/* タスクループ */
	while(1) {
		
		/* イベントフラグ待ち */
		WAI_FLG(ID_RS485_FLG, &status);
		
		
		/* 状態チェックイベント(電圧値) */
		if (status & F_RS485_VOLCHK_EVENT) {									//1000ms周期起動
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_RV);											//RV電文送信
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_RV);											//RV電文受信
			
		}
		
		/* 状態チェックイベント(電流値) */
		if (status & F_RS485_CURCHK_EVENT) {									//1000ms周期起動
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_RI);											//RI電文送信
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_RI);											//RI電文受信
			
		}
		
		/* ADDS設定イベント */
		if (status & F_RS485_ADDRSET_EVENT) {									
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_ADDS);										//ADDS設定電文送信
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_ADDS);										//ADDS設定電文受信
			
		}
		
		/* REMOTEモード切替イベント */
		if (status & F_RS485_REMSSET_EVENT) {									
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_REMS);										//REMS設定電文送信
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_REMS);										//ADDS設定電文受信
			
		}
		
		/* 電源ONイベント */
		if (status & F_RS485_P_ON_EVENT) {										
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_P_ON);										//PowerON電文送信
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_P_ON);										//PowerON電文受信
			
		}
		
		/* 電源OFFイベント */
		if (status & F_RS485_P_OFF_EVENT) {										
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_P_OFF);										//PowerOFF電文送信
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_P_OFF);										//PowerOFF電文受信
			
		}
		
		
		/* 電圧設定イベント */
		if (status & F_RS485_VOL_SET_EVENT) {									
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_ST_SV);										//電圧値設定
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_ST_SV);									
			
		}
		
		/* 電流設定イベント */
		if (status & F_RS485_CUR_SET_EVENT) {									
			
			/* 送信許可 */
			SEND_ENABLE;														//SCI0送信許可(受信無効)
			
			/* 送信処理 */
			ercd = rs485_send(SEND_ST_SI);										//電流値設定
			if(ercd != ER_OK){
				/* 送信エラー(処理なし) */
			}
			/* 送信待ち */
			rs485_send_wait();													//送信待ち
			
			/* 受信許可 */
			RECV_ENABLE;														//SCI0受信許可(送信無効)
			
			/* 受信処理 */
			ercd = rs485_recv(SEND_ST_SI);										//電流値設定
			
		}
		/* RS485送信待ちタイムアウトイベント */
		if(status & F_RS485_SNDTMO_EVENT){
			
			send_timout_flg = FLG_ON;											//送信待ちタイムアウト
			
		}
	}
}
//******************************************************************************
//  機能     : 電文送信処理
//  機能説明 : 引数modeに応じた電文を送信バッファに設定して送信する。
//------------------------------------------------------------------------------
//  引数     : UB mode : 電文種別
//------------------------------------------------------------------------------
//  戻り値   : ER_OK : 正常終了
//           : ER_NG : 異常終了
//******************************************************************************
ER rs485_send(UB mode)
{
	/* 変数定義 */
	ER ercd;
	UH i;
	UH len;					//送信データ長
	
	
	/* 送信バッファクリア */
	memset(g_rs485.send_buf,0,sizeof(g_rs485.send_buf));						//電文ごとに電文長が異なるため毎回クリア
	
	
	/* 電文設定 */
	switch(mode){
		
		case SEND_ADDS:/* ADDS設定 */
			
			memcpy(g_rs485.send_buf,cmd_set_adds,sizeof(cmd_set_adds));
			g_rs485.send_buf[5] = (UB)ADDRSW + 0x30;							//ADDS値設定(アスキーコード化)
			len = sizeof(cmd_set_adds)-1;										//(-1)末尾のNULL終端は送らない
			
			break;
			
		case SEND_REMS:/* REMS設定(リモート設定) */
			
			memcpy(g_rs485.send_buf,cmd_set_rems,sizeof(cmd_set_rems));
			g_rs485.send_buf[5] = (UB)REMS + 0x30;								//REMS値設定(アスキーコード化)
			len = sizeof(cmd_set_rems)-1;										//(-1)末尾のNULL終端は送らない
			
			break;
			
		case SEND_P_ON:/* Power ON */
			
			memcpy(g_rs485.send_buf,cmd_set_pon,sizeof(cmd_set_pon));
			len = sizeof(cmd_set_pon)-1;										//(-1)末尾のNULL終端は送らない
			
			break;
			
		case SEND_P_OFF:/* Power OFF */
			
			memcpy(g_rs485.send_buf,cmd_set_poff,sizeof(cmd_set_poff));
			len = sizeof(cmd_set_poff)-1;										//(-1)末尾のNULL終端は送らない
			
			break;
			
		case SEND_RV:/* 現在値読出し(電圧) */
			
			memcpy(g_rs485.send_buf,cmd_read_rv,sizeof(cmd_read_rv));
			len = sizeof(cmd_read_rv)-1;										//(-1)末尾のNULL終端は送らない
			
			break;
			
		case SEND_RI:/* 現在値読出し(電流) */
			
			memcpy(g_rs485.send_buf,cmd_read_ri,sizeof(cmd_read_ri));
			len = sizeof(cmd_read_ri)-1;										//(-1)末尾のNULL終端は送らない
			
			break;
			
			
		case SEND_ST_SV:/* 設定(電圧) */
			
			
			/* 電圧設定：SV （0～40000：0.01V単位） */
			len =  rs485_make_sv_fixed2(g_rs485.send_buf,g_rs485.vol_set);
			
			break;
			
		case SEND_ST_SI:/* 設定(電流) */
			
			
			/* 電流設定：SI （0～750：0.01A単位） */
			len =  rs485_make_si_fixed2(g_rs485.send_buf,g_rs485.cur_set);
			
			break;
			
			
		default:
			/* default文処理なし */
			break;
		
	}
	
	/* 送信処理 */
	for(i=0; i<len; i++){
		ercd = put_sio(UART_CH,g_rs485.send_buf[i],TMO_SND_CHR);				//送信処理
	}
	return ercd;
	
}
//******************************************************************************
//  機能     : 電文受信処理
//  機能説明 : 電文を受信して、受信した電文をmodeによって判定・処理する。
//------------------------------------------------------------------------------
//  引数     : UB mode : 電文種別
//------------------------------------------------------------------------------
//  戻り値   : ER_OK : 正常終了
//           : ER_NG : 異常終了
//******************************************************************************
ER rs485_recv(UB mode)
{
	
	/* 変数定義 */
	UH i;				//for文用
	ER ercd;			//エラーコード
	UB recv_flg;		//受信フラグ
	
	/* 初期化 */
	recv_flg = FLG_OFF;
	
	/* 受信バッファクリア */
	memset(g_rs485.recv_buf, 0, sizeof(g_rs485.recv_buf));
	
	/* 受信処理 */
	for(i=0;i<sizeof(g_rs485.recv_buf);i++){									//受信バッファのサイズ分(8バイト)
	
		ercd = get_sio(UART_CH,&g_rs485.recv_buf[i],TMO_RCV_CHR);				//受信バッファにデータを受信
		
		if(ercd == E_OK){														//受信データありの場合
		
			recv_flg = FLG_ON;													//受信フラグをON
			
		}else{
		
			break;																//その他の場合は抜ける
			
		}
	}
	
	if(recv_flg != FLG_ON){return ercd;}										//受信データがなければ異常終了(エラーコード)
	
	/* 正常電文か判定 */
	ercd = find_reply_token(g_rs485.recv_buf);
	if(ercd != ER_OK){return ER_NG;}
	
	/* 受信電文解析 */
	switch(mode){
		
		case SEND_ADDS:	/* ADDS設定 */
		case SEND_REMS:	/* REMS設定(リモート設定) */
		case SEND_P_ON:	/* Power ON */
		case SEND_P_OFF:/* Power OFF */
		case SEND_ST_SV:/* 電圧設定 */
		case SEND_ST_SI:/* 電流設定 */
			
			/* 正常電文が確定しているため処理なし */
			
			break;
			
		case SEND_RV:/* 現在値読出し(電圧) */
			
			g_rs485.vol_now = rs485_rv_store(g_rs485.recv_buf);
			
			break;
			
		case SEND_RI:/* 現在値読出し(電流) */
			
			g_rs485.cur_now = rs485_ri_store(g_rs485.recv_buf);
			
			break;
			
		default:
			/* default文処理なし */
			break;
		
	}
	
	return ER_OK;
}
//******************************************************************************
//  機能     : 送信完了待ち
//  機能説明 : 送信完了まで待つ。
//------------------------------------------------------------------------------
//  引数     : void : 
//------------------------------------------------------------------------------
//  戻り値   : void : 
//******************************************************************************
void rs485_send_wait(void)
{

	EntryTimer10ms(SEND_TIMOUT, (ID)ID_RS485_FLG , (FLGPTN)F_RS485_SNDTMO_EVENT);	//タイマーセット(50ms)
	while(SCI_TEND == 0){
	
		if(send_timout_flg == FLG_ON){												//送信待ちタイムアウト時は待ちを抜ける
			send_timout_flg = FLG_OFF;
			break;
		}
	
	}
	CancelTimer10ms((ID)ID_RS485_FLG, (FLGPTN)F_RS485_SNDTMO_EVENT);				//タイマーキャンセル(50ms)

}
//******************************************************************************
//  機能     : 応答電文チェック
//  機能説明 : 応答電文内に｢=>\r\n｣があるか探し、正常応答かチェックする。
//------------------------------------------------------------------------------
//  引数     : UB *p : 受信バッファを示すポインタ
//------------------------------------------------------------------------------
//  戻り値   : ER_OK : 正常な電文
//           : ER_NG : 異常な電文
//******************************************************************************
ER find_reply_token(UB *p)
{
	/* 変数定義 */
	UH i,j;
	
	for (i = 0; i<=sizeof(g_rs485.recv_buf); i++) {
		if(p[i] == '='){
			j = i + 1;
			if(p[j] == '>'){
				j++;
				if((p[j] == 0x0D) && (p[j + 1] == 0x0A)){
					return ER_OK; 								//正常な電文
				}
			}
		}
	}
	return ER_NG;												//異常な電文
}
//******************************************************************************
//  機能     : 数値取得処理
//  機能説明 : 先頭行（最初のCRLFまで）から数値を取り出し、0.01単位の整数へ変換する。
//           : 例: "7.50\r\n" -> 750
//           :     "12\r\n"   -> 1200
//           :     "0.0\r\n"  -> 0   (小数不足はゼロ埋め)
//           :     "3.1415"   -> 314 (3桁目以降は切り捨て)
//           :     "12.34V"   -> 1234（数字と'.'と'-'以外は無視）
//------------------------------------------------------------------------------
//  引数     : UB *p : 数値取得する元データへのポインタ
//           : UW *out_x100 : 変換後の文字列を格納する配列へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : void : 
//******************************************************************************
void parse_fixed_to_from_line(UB *p, UW *out_x100)
{
	/* 変数定義 */
	UW intPart = 0;				//整数部の数値
	UW fracPart = 0;			//小数部の数値
	UB fracDigits = 0;			//小数点位置
	UB seenDot = 0;				//
	UH i;						//
	UB c;						//
	
	for (i = 0; i + 1 <sizeof(g_rs485.recv_buf) ; i++) {
		if (p[i] == 0x0D && p[i + 1] == 0x0A) break;							//CRLFで行終端
		
		c = (char)p[i];
		if (c == '.') { seenDot = 1; continue; }
		
		if (c >= '0' && c <= '9') {
			if (!seenDot) {
				intPart = intPart * 10 + (c - '0');
			} else {
				if (fracDigits < 2) {											//小数2桁まで
					fracPart = fracPart * 10 + (c - '0');
					fracDigits++;
				} else {														//3桁目以降は切り捨て
				}
			}
		}
																				//'A' 'V' ' ' などは無視
	}
	
	while (fracDigits < 2) {													//小数2桁に足りない分はゼロ埋め
		fracPart *= 10;
		fracDigits++;
	}
	
	*out_x100 = (intPart * 100 + fracPart);										//0.01単位
	
}
//******************************************************************************
//  機能     : RI?コマンド受信処理
//  機能説明 : RI?コマンドで受信した電流値を単位0.01Aの整数値に変換して返す。
//           : 0.01A単位で0～750に変換する。
//------------------------------------------------------------------------------
//  引数     : UB *buf : 変換前のデータが格納された配列先頭へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : UH cur_x100 :変換した整数値
//******************************************************************************
UH rs485_ri_store(UB *buf)
{
	/* 変数定義 */
	UW cur_x100 = 0;		//0.01A単位
	
	parse_fixed_to_from_line(buf, &cur_x100);
	
	return (UH)cur_x100;
}
//******************************************************************************
//  機能     : RV?コマンド受信処理
//  機能説明 : RV?コマンドで受信した電圧値を単位0.01Vの整数値に変換して返す。
//           : 0.01V単位で0～40000に変換する。
//------------------------------------------------------------------------------
//  引数     : UB *buf : 変換前のデータが格納された配列先頭へのポインタ
//------------------------------------------------------------------------------
//  戻り値   : UH vol_x100 : 変換した整数値
//******************************************************************************
UH rs485_rv_store(UB *buf)
{
	/* 変数定義 */
	UW vol_x100 = 0;		//0.01V単位
	
	parse_fixed_to_from_line(buf, &vol_x100);
	
	return (UH)vol_x100;
}
//******************************************************************************
//  機能     : U32toASCII(整数,符号なし)
//  機能説明 : 10進数(符号無し)をASCIIにしてdstへ格納する。
//------------------------------------------------------------------------------
//  引数     : UB *dst : 変換後の文字列を格納する配列先頭ポインタ
//           : UW v : 変換前の値
//------------------------------------------------------------------------------
//  戻り値   : UB n : 桁数
//******************************************************************************
/* 10進数(符号なし)をASCIIにしてdstへ格納。戻り値=桁数 */
static UB u32_to_dec_ascii(UB *dst, UW v)
{
	UB tmp[10];
	UB n = 0;
	UB i;

	if (v == 0U) {
		dst[0] = (UB)'0';
		return 1U;
	}

	while (v > 0U) {
		tmp[n++] = (UB)('0' + (v % 10U));
		v /= 10U;
	}
	for ( i = 0; i < n; i++) {
		dst[i] = tmp[n - 1U - i];
	}
	return n;
}
//******************************************************************************
//  機能     : 電圧設定SVデータ作成
//  機能説明 : 指定した送信バッファにSVコマンドを作成する。
//           : 上限値より大きい値が設定された場合は、上限値を設定する。
//------------------------------------------------------------------------------
//  引数     : UB *send_buf : SVコマンドを作成する送信バッファ
//           : UW vol_0p01v : 設定する電圧値
//------------------------------------------------------------------------------
//  戻り値   : rs485_make_cmd_fixed2 :
//******************************************************************************
UB rs485_make_sv_fixed2(UB *send_buf, UW vol_0p01v)
{
    return rs485_make_cmd_fixed2(send_buf,(UB)'S', (UB)'V',vol_0p01v, 40000U);
}
//******************************************************************************
//  機能     : 電流設定SIデータ作成
//  機能説明 : 指定した送信バッファにSIコマンドを作成する。
//           : 上限値より大きい値が設定された場合は、上限値を設定する。
//------------------------------------------------------------------------------
//  引数     : UB *send_buf : SIコマンドを作成する送信バッファ
//           : UW cur_0p01v : 設定する電流値
//------------------------------------------------------------------------------
//  戻り値   : UB rs485_make_cmd_fixed2 : 文字数
//******************************************************************************
UB rs485_make_si_fixed2(UB *send_buf, UW cur_0p01a)
{
	return rs485_make_cmd_fixed2(send_buf,(UB)'S', (UB)'I',cur_0p01a, 750U);
}
//******************************************************************************
//  機能     : コマンド電文作成処理
//  機能説明 : コマンド文字C1C2から、C1C2 <param>CRLFを送信バッファに作成する。
//------------------------------------------------------------------------------
//  引数     : UB *send_buf  ：コマンド電文を格納する送信バッファ
//           : UB c1         ：コマンド１文字目
//           : UB c2         ：コマンド２文字目
//           : UW value_0p01 ：設定するパラメータ
//           : UW max_0p01   ：パラメータ上限値
//------------------------------------------------------------------------------
//  戻り値   : UB p  : 文字数
//******************************************************************************
static UB rs485_make_cmd_fixed2(UB *send_buf,UB c1, UB c2,UW value_0p01,UW max_0p01)
{
	if (value_0p01 > max_0p01) { return 0; }

	UB param[16];
	UB plen = u32_to_fixed2_ascii(param, value_0p01);
	UB p = 0;
	UB i;
	
	send_buf[p++] = c1;
	send_buf[p++] = c2;
	send_buf[p++] = RS485_SP;

	for (i = 0; i < plen; i++) {
		send_buf[p++] = param[i];
	}
	
	send_buf[p++] = RS485_CR;
	send_buf[p++] = RS485_LF;

	return p;
}
//******************************************************************************
//  機能     : U32toASCII(整数.小数2桁)
//  機能説明 : 単位(0.01)の整数をアスキーコードに変換して格納する。
//           : 格納時は、<整数><.(小数点)><小数2桁>とする。
//           : (例)1520⇒15.20
//           :     111⇒1.11
//           :     10⇒0.10
//------------------------------------------------------------------------------
//  引数     : UB *dst   : 変換後の文字列を格納する配列へのポインタ
//           : UW v_0p01 : 単位(0.01)の整数
//------------------------------------------------------------------------------
//  戻り値   : UB n      : 文字数
//******************************************************************************
static UB u32_to_fixed2_ascii(UB *dst, UW v_0p01)
{
	/* 変数定義 */
	UW ip = v_0p01 / 100U;				//整数部
	UW fp = v_0p01 % 100U;				//小数部(0～99)
	UB n = u32_to_dec_ascii(dst, ip);	//U32整数をアスキーコードに変換
	
	dst[n++] = (UB)'.';
	dst[n++] = (UB)('0' + (fp / 10U));
	dst[n++] = (UB)('0' + (fp % 10U));
	return n;
}
