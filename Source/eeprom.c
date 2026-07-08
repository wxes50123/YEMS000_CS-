//******************************************************************************
// @(R)
//		EEPROM関数
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//****** 変更履歴 **************************************************************
//ver 1.0	2025.12.10	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#define EEPROM_FUNC_DEFINE
#include <stdio.h>
#include <machine.h>
#include "g_include_def.h"
//******** 定数定義部 **********************************************************
#define EEPROMCMD_WRITE_ENABLE	0x06
#define EEPROMCMD_WRITE_DISABLE	0x04
#define EEPROMCMD_READ_STATUS	0x05
#define EEPROMCMD_WRITE_STATUS	0x01
#define EEPROMCMD_READ_ARRAY	0x03
#define EEPROMCMD_WRITE_ARRAY	0x02
#define UNPROTECT_WR_STATUS		0x00
#define PROTECT_WR_STATUS		0x0C
#define EEP_BUSY_BIT			0x01
//******** 変数定義部 **********************************************************
//定義無し
//******** プロトタイプ宣言部 **************************************************
static void io_cmd_exe_rdmode(unsigned char *ope, int ope_sz, unsigned char *rd, int rd_sz);	//コマンド実行(リードデータあり)
static int io_rspi_transfer(unsigned char *write_data, unsigned char *read_data, int data_sz);	//RSPIデータ転送
ER eep_mac_read(UB *macaddr);													//MACアドレス読み込み
//******************************************************************************
//  機能     : MACアドレス読み込み
//  機能説明 : 
//------------------------------------------------------------------------------
//  引数     : UB *macaddr:MACアドレス格納用バッファの先頭アドレス
//------------------------------------------------------------------------------
//  戻り値   : ER_OK:バッファにMACアドレスを読み出せた場合は正常終了
//           : ER_NG:バッファが全て初期値(0x00)だった場合は異常終了
//******************************************************************************
ER eep_mac_read(UB *macaddr){
	
	/* 変数定義 */
	UH i,k;
	UB cmd[2];
	int ipl;
	
	ipl=get_ipl();																//割込み禁止
	set_ipl(15);
	
	/* EEPROMアドレス0xFAから0xFFまで読出し */
	for( i=0xFA,k=0; i<=0xFF; i++,k++ ){
		cmd[0] = EEPROMCMD_READ_ARRAY;
		cmd[1] = i;
		io_cmd_exe_rdmode(cmd, 2, macaddr+k, 1);
	}
	
	set_ipl(ipl);																//割込み禁止解除
	
	/* MACアドレス読出しチェック */
	for( i=0; i<6; i++ ){
		
		if(*macaddr+i != 0x00){
			return ER_OK;														//MACアドレス読出し正常
		}
	}
	
	return ER_NG;																//MACアドレス読出し異常
																				//全て0x00の場合は読出しできなかった判定
}

//******************************************************************************
//******************************************************************************
//  機能     : コマンド実行(リードデータあり)
//  機能説明 : 指定されたコマンドを実行する。
//           : 引数opeを送信した後、引数rdにデータを受信します。
//           : ope_szは 0～8 のいずれかの値を設定してください。
//           : rd_szは 0以上 の値を設定することが可能です。
//------------------------------------------------------------------------------
//  引数     : unsigned char *ope : 送信するオペコード部とアドレス部の先頭アドレス
//           : int ope_sz         : オペコード部とアドレス部のバイト数
//           : unsigned char *rd  : 受信データを格納するバッファアドレス
//           : int rd_sz          : データ部のバイト数
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
static void io_cmd_exe_rdmode(unsigned char *ope, int ope_sz, unsigned char *rd, int rd_sz)
{	
	/* SPI転送許可 */
	RSPI2.SPCR.BIT.SPE = 1;
	
	/* MISO */
	io_rspi_transfer(ope, NULL, ope_sz);
	io_rspi_transfer(NULL, rd, rd_sz);
	
	/* SPI転送終了（SSLネゲート）*/
	RSPI2.SPCR.BIT.SPE = 0;
}
//******************************************************************************
//  機能     : RSPIデータ転送
//  機能説明 : コマンド、データをバイト単位で転送する。
//           : 引数write_dataからオペコードまたはデータを送信し、
//           : 引数read_dataにデータを受信する。
//           : 引数write_dataがNULLの場合、ダミーデータ(0xff)の送信を行う。
//           : 引数read_dataがNULLの場合、データの受信は行う。
//------------------------------------------------------------------------------
//  引数     : unsigned char *write_data	: 送信データの先頭アドレス
//           : unsinged char *read_data		: 受信データを格納するバッファアドレス
//           : int data_sz 					: 送信、受信データのバイト数
//------------------------------------------------------------------------------
//  戻り値   : 0 : 正常転送
//           : -1: オーバランエラー発生
//******************************************************************************
static int io_rspi_transfer(unsigned char *write_data, unsigned char *read_data, int data_sz)
{
	unsigned char tmp;

	while(data_sz--){
		while(RSPI2.SPSR.BIT.SPTEF == 0){
			/* wait */
		}
		/* データレジスタへ転送データを書き込み */
		if(write_data != (unsigned char *)0){
			tmp = (unsigned char)*write_data++;
		}
		else{
			tmp = 0xff;															//ダミーライト用データ
		}
		
		RSPI2.SPDR.BYTE.HH = 0xff & tmp;

		RSPI2.SPSR.BIT.SPTEF = 0;												//送信のための0クリア
		
		/* 受信完了待ち */
		while((RSPI2.SPSR.BYTE & 0x81) == 0x00){
			/* 受信バッファがフル状態になるか、オーバランエラー発生までウェイト */
		}
		
		/* オーバランエラー発生? */
		if(RSPI2.SPSR.BIT.OVRF == 1){
			RSPI2.SPSR.BIT.OVRF = 0;
			return -1;															//オーバランエラー発生
		}
		
		/* 受信データリード */
		tmp = RSPI2.SPDR.BYTE.HH;
		if(read_data != (unsigned char *)0){
			*read_data++ = (unsigned char)tmp;
		}
		RSPI2.SPSR.BIT.SPRF = 0;	
	}

	return 0;
}
