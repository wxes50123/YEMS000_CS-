//******************************************************************************
// @(R)
//		リセット処理
//		
// @(X)	 ver 1.0
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd.  All rights reserved.
//******************************************************************************
//****** 変更履歴 **************************************************************
//ver 1.0 2025.12.18	初版作成
//******** ヘッダファイル参照宣言部 ********************************************
#include <machine.h>
#include <_h_c_lib.h>
#include <string.h>				//memcpy関数用
#include "rx72m.h"				//NORTi用定義
#include "g_include_def.h"		//共通インクルードファイル
//******** 定数定義部 **********************************************************

//******** プロトタイプ宣言部 **************************************************
extern int main(void);
void ini_cpu(void);					//周辺I/O端子設定
void ini_rtc(void);					//RTC初期設定
void ini_sci(void);					//SCI初期設定
void ini_do(void);					//DO端子初期化
void ini_g_val(void);				//グローバル変数初期化
void ini_wdt(void);					//ウォッチドッグタイマ初期化

/* Option-Setting Memory of RX72M */
#pragma address MDE  = 0xfe7f5d00	//エンディアン選択レジスタ
#pragma address OFS0 = 0xfe7f5d04	//オプション機能選択レジスタ0
#pragma address OFS1 = 0xfe7f5d08	//オプション機能選択レジスタ1
#pragma address TMINF= 0xfe7f5d10	//TM識別データレジスタ
#pragma address SPCC = 0xfe7f5d40	//シリアルプログラマコマンド制御レジスタ
#pragma address TMEF = 0xfe7f5d48	//TMイネーブルフラグレジスタ
#pragma address OSIS = 0xfe7f5d50	//OCD/シリアルプログラマID設定レジスタ
#pragma address FAW  = 0xfe7f5d64	//フラッシュアクセスウィンドウ設定レジスタ
#pragma address RCP  = 0xfe7f5d70	//ROMコードプロテクトレジスタ

#ifdef BIG_ENDIAN					//シングルチップモードでは選択可
const UW MDE         = 0xfffffff8;	//ビッグエンディアン
#else
const UW MDE         = 0xffffffff;	//リトルエンディアン
#endif
const UW OFS0        = 0xffffffff;
const UW OFS1        = 0xffffffff;
const UW TMINF       = 0xffffffff;
const UW SPCC        = 0xffffffff;
const UW TMEF        = 0xffffffff;
const UW OSIS[4]   = { 0xffffffff,
                       0xffffffff,
                       0xffffffff,
                       0xffffffff };
const UW FAW = 0xffffffff;
const UW RCP = 0xffffffff;

/* Additional Input/Output Macros for Peripherals */
#define sfr_read(n,c) while(sfr_inl(n)!=(c)) 		//read surely when optimized
/*****************************************************************************
* セクションの初期化とmain関数の呼び出し
*
******************************************************************************/
void _INIT(void)
{
	_INITSCT();			//セクションを初期化
	
	ini_do();			//DO端子初期化
	
	ini_rtc();			//RTC初期設定
	
	ini_g_val();		//グローバル変数初期化
	
#ifdef WDT_ENABLE
	ini_wdt();			//ウォッチドッグタイマ初期化
	WDT_START;			//ウォッチドッグタイマスタート
#endif

	main();				//ユーザープログラムのmain関数を呼び出し
	for (;;);			//万一、main関数が終了したら、無限ループ
}
/*****************************************************************************
* 標準ライブラリの_INITSCT関数が参照するテーブル
*
******************************************************************************/
#pragma section C C$DSEC
extern const struct {
    void *rom_s; /* 初期化データセクションのROM上の先頭アドレス */
    void *rom_e; /* 初期化データセクションのROM上の最終アドレス */
    void *ram_s; /* 初期化データセクションのRAM上の先頭アドレス */
} _DTBL[] = {
    { __sectop("D_1"), __secend("D_1"), __sectop("R_1") },
    { __sectop("D_2"), __secend("D_2"), __sectop("R_2") },
    { __sectop("D"),   __secend("D"),   __sectop("R")   }
  #ifdef __DBL8
   ,{ __sectop("D_8"), __secend("D_8"), __sectop("R_8") }
  #endif
};

#pragma section C C$BSEC
extern const struct {
    void *b_s;   /* 未初期化データセクションの先頭アドレス */
    void *b_e;   /* 未初期化データセクションの最終アドレス */
} _BTBL[] = {
    { __sectop("B_1"), __secend("B_1") },
    { __sectop("B_2"), __secend("B_2") },
    { __sectop("B"),   __secend("B")   }
  #ifdef __DBL8
   ,{ __sectop("B_8"), __secend("B_8") }
  #endif
};

#pragma section
extern void * const _CTBL[] = { /* セクションが空の時のワーニング防止用 */
    __sectop("C_1"), __sectop("C_2"), __sectop("C"),
  #ifdef __DBL8
    __sectop("C_8"),
  #endif
    __sectop("W_1"), __sectop("W_2"), __sectop("W"),
    __sectop("L"),   __sectop("STACK")
};
//******************************************************************************
//  機能     : 周辺I/O端子設定
//  機能説明 : IOポート、クロックの設定を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ini_cpu(void)
{

	/* 周波数の変更 */
	sfr_outw(RWP_PRCR, 0xA503);			/* クロック発生回路関連レジスタ等の書き込み許可 */

	sfr_outw(CGC_PACKCR, 0x0011);		/* OUTCKSEL = 1 (Ethernet-PHY向け外部クロックソース選択
											PPLL回路選択) */
	sfr_out(CGC_MEMWAIT, 0x01);			/* MEMWAIT = 1 (1ウェイト) */
	sfr_outl(CGC_SCKCR, 0x20C91222);	/* FCK = 0010b (FlashIFクロック 4分周)
											ICK = 0000b (システムクロック 1分周)
											PSTOP1 = 1 (BCLK端子出力停止)
											PSTOP0 = 1 (SDCLK端子出力停止)
											BCK = 1001b (外部バスクロック 3分周)
											PCLKA = 0001b (周辺モジュールクロックA 2分周)
											PCLKB-PCLKD = 0010b (周辺モジュールクロックB-D 4分周) */
    sfr_out(CGC_MOFCR, 0x00);			/* MOSEL = 0 (メインクロック発振子切り替え 発振子)
											MODRV2 = 00b (20.1MHz～24MHz)
											MOFXIN = 0 (強制発振なし) */
    sfr_out(CGC_MOSCWTCR, 90);			/* MSTS = 90 (90クロック) */
    sfr_out(CGC_MOSCCR, 0x00);			/* MOSTP = 0 (メインクロック発振器動作) */
    sfr_outw(CGC_PLLCR, 0x1300);		/* STC = 010011b (逓倍率10.0倍)
											PLLSRCSEL = 0 (クロックソース メインクロック発振器)
											PLIDIV = 00b (入力分周比 1分周) */
	sfr_out(CGC_PLLCR2, 0x00);			/* PLLEN = 0 (PLL動作) */
	while (!(sfr_in(CGC_OSCOVFSR) & 0x04))
		;                               /* PLOVF = 0 ? (PLLクロック発振安定待ち) */
	sfr_outw(CGC_SCKCR3, 0x0400);		/* CKSEL = 100b (クロックソース PLL回路選択) */

	sfr_outw(CGC_PPLLCR, 0x3102);		/* PPLSTC = 110001b (×25.0)
											PPLDIV = 10b (3分周) */
	sfr_out(CGC_PPLLCR2, 0x00);			/* PPLLEN = 0 (PPLL動作) */
	while (!(sfr_in(CGC_OSCOVFSR) & 0x20))
		;								/* PPLOVF = 0 ? (PPLLクロック発振安定待ち) */

	sfr_outw(RWP_PRCR, 0xA500);			/* クロック発生回路関連レジスタ等の書き込み禁止 */

    /* Ethernet端子の設定 */
	sfr_out(MPC_PWPR, 0x00);				// B0WI = 0 (PFSWEビット書き込み許可)
	sfr_out(MPC_PWPR, PWPR_PFSWE);			// PFSWE = 1 (PFSレジスタ書き込み許可)
	sfr_out(MPC_PFENET, 0x30);				// PHYMODE1 = 1 (ETHERC1 MIIモード)
											// PHYMODE0 = 1 (ETHERC0 MIIモード)
	/* Ethernet_CH1 */
	sfr_out(MPC_PG2PFS, 0x14);				// PG2:ET1TXCLK
	sfr_out(MPC_PD6PFS, 0x14);				// PD6:ET1RXCLK
	sfr_out(MPC_P60PFS, 0x14);				// P60:ET1TXEN_RMII1TXDEN
	sfr_out(MPC_PG6PFS, 0x14);				// PG6:ET1ETXD3
	sfr_out(MPC_PG5PFS, 0x14);				// PG5:ET1ETXD2
	sfr_out(MPC_PG4PFS, 0x14);				// PG4:ET1ETXD1_RMII1TXD1
	sfr_out(MPC_PG3PFS, 0x14);				// PG3:ET1ETXD0_RMII1TXD0
	sfr_out(MPC_P90PFS, 0x14);				// P90:ET1RXDV
	sfr_out(MPC_P97PFS, 0x14);				// P97:ET1ERXD3
	sfr_out(MPC_P96PFS, 0x14);				// P96:ET1ERXD2
	sfr_out(MPC_P61PFS, 0x14);				// P61:ET1ERXD1_RMII1RXD1
	sfr_out(MPC_P62PFS, 0x14);				// P62:ET1ERXD0_RMII1RXD0
	sfr_out(MPC_PD7PFS, 0x14);				// PD7:ET1RXER_RMII1RXER
	sfr_out(MPC_P92PFS, 0x14);				// P92:ET1CRS_RMII1CRSDV
	sfr_out(MPC_P91PFS, 0x14);				// P91:ET1COL
	sfr_out(MPC_P31PFS, 0x14);				// P31:ET1MDC
	sfr_out(MPC_P30PFS, 0x14);				// P30:ET1MDIO
	sfr_out(MPC_P84PFS, 0x14);				// P84:ET1LINKSTA
	sfr_out(MPC_PG7PFS, 0x14);				// PG7:ET1TXER
	sfr_out(MPC_PJ2PFS, 0x2A);				// PJ2:CLKOUT25M
	sfr_out(MPC_PWPR, PWPR_B0WI);			// PFSWE = 0b (PFSレジスタ書き込み禁止)
											// B0WI = 1b (PFSWEビット書き込み禁止)
	sfr_set(P3_PMR, 0x03);					// P30,P31:周辺モジュールとして使用
	sfr_set(P6_PMR, 0x07);					// P60,P61,P62:周辺モジュールとして使用
	sfr_set(P8_PMR, 0x10);					// P84:周辺モジュールとして使用
	sfr_set(P9_PMR, 0xC7);					// P90,P91,P92,P96,P97：周辺モジュールとして使用
	sfr_set(PG_PMR, 0xFC);					// PG2,PG3,PG4,PG5,PG6,PG7：周辺モジュールとして使用
	sfr_set(PJ_PMR, 0x04);					// PJ2：周辺モジュールとして使用
	sfr_set(PD_PMR, 0xC0);					// PD6,PD7：周辺モジュールとして使用
	
	sfr_outw(RWP_PRCR, 0xA502);
	sfr_clrl(LPC_MSTPCRA, 0x00008000);
	sfr_outw(RWP_PRCR, 0xA500);
	
	sfr_set(PD_PDR, 0x01);					// PD0:ET1EXOUT */
	dly_1us(25000);							// wait 25msec */
	sfr_set(PD_PODR, 0x01);					// EtherEXOUT(RESET) negate */
	dly_1us(1000);							// wait 1msec */
	
	/* CAN0レジスタ設定 */
	MPC.PWPR.BIT.B0WI = 0;					// PWPR.PFSWE write protect off
	MPC.PWPR.BIT.PFSWE = 1;					// PFS register write protect off
	/* CAN0 */
	//CANのスタンバイ解除(MSTPCRB)は、CAN-API内部で実施
	MPC.PJ2PFS.BYTE = 0x2AU;				// Set CLKOUT25M pin
	PORTJ.PMR.BYTE |= 0x04U;				
	MPC.P33PFS.BYTE = 0x10U;				// Set CRX0 pin
	PORT3.PMR.BYTE |= 0x08U;				
	MPC.P32PFS.BYTE = 0x10U;				// Set CTX0 pin
	PORT3.PMR.BYTE |= 0x04U;
							
	/* CAN1 */
	MPC.P15PFS.BYTE = 0x10U;				// Set CRX1 pin
	PORT1.PMR.BYTE |= 0x20U;
	MPC.P23PFS.BYTE = 0x10U;				// Set CTX1 pin
	PORT2.PMR.BYTE |= 0x08U;
							
	MPC.PWPR.BYTE = 0x80;					// PFS register write protect on
	
	/* RSPI2設定(EEPROM用) */
	MPC.PWPR.BIT.B0WI = 0;					// PWPR.PFSWE write protect off
	MPC.PWPR.BIT.PFSWE = 1;					// PFS register write protect off
	
	SYSTEM.PRCR.WORD = 0xA502;				//プロテクトOFF
	MSTP(RSPI2) = 0;						// RSPI2省電力モード解除
	SYSTEM.PRCR.WORD = 0xA500;				// プロテクトON
	
	/* MOSIC-B pin定義 */
	MPC.P54PFS.BYTE = 0x0D;		// P54 MOSIC-B
	MPC.P55PFS.BYTE = 0x0D;		// P55 MISOC-B
	MPC.P56PFS.BYTE = 0x0D;		// P56 RSPCKC-B
	MPC.P57PFS.BYTE = 0x0D;		// P57 SSLC0-B
	PORT5.PMR.BYTE =  0xF0;
	
	RSPI2.SPCR.BYTE = 0x08;		// 0:SPI動作(4線式)
								// 0:全二重通信(受信回路動作)
								// 0:モードフォルトエラー検出を禁止
								// 1:マスタモード
								// 0:エラー割込み要求の生成を禁止
								// 0:送信バッファエンプティ割込み要求の生成を禁止
								// 0:RSPI機能は無効
								// 0:受信バッファフル割込み要求の生成を禁止
	RSPI2.SSLP.BYTE   =	0x00;	// SSLP = b'0 SSL signal 0-active */
	RSPI2.SPPCR.BYTE  =	0x00;	// MOSIアイドル固定値 = 0 */
	RSPI2.SPBR        =	0x05;	// ベースのビットレートを4.17MHzに設定(PΦ=50MHz) */
	RSPI2.SPDCR.BYTE  =	0x40;	// SPDRレジスタのアクセス幅：16ビット */
	RSPI2.SPCKD.BYTE  =	0x00;	// RSPCK遅延：1 RSPCK */
	RSPI2.SSLND.BYTE  =	0x00;	// SSLネゲート遅延：1 RSPCK */
	RSPI2.SPND.BYTE   =	0x00;	// 次アクセス遅延：1 RSPCK */ 
	RSPI2.SPCR2.BYTE  =	0x00;	// SPPE = 0 
								// SPOE = 0 
	RSPI2.SPSCR.BYTE  =	0x00;	// シーケンス長：1 (SPCMD0のみ使用) */
								
	RSPI2.SPCMD0.WORD = 0x0780;	// MSBファースト */
								// データ長：8bit */
								// 転送終了後もSSL信号レベルを保持する */
								// ビットレート：ベースビットレートの分周なし */
								// アイドル時のRSPCK：1 */
								// 奇数エッジでデータ変化、偶数エッジでデータサンプル */
								
	RSPI2.SPDCR2.BYTE = 0x00;	// SPDRのデータをバイト単位でスワップしない */
							
	RSPI2.SPCR.BYTE   =	0x48;	// マスタモード */
								// 割り込み禁止 */
								// RSPIの動作許可 */
	MPC.PWPR.BIT.PFSWE = 0;		// PWPR.PFSWE write protect on
	MPC.PWPR.BIT.B0WI = 1;		// PFS register write protect on
	
	
	/* ポート方向レジスタ設定 */
	/* 出力設定 */
	PORT0.PDR.BYTE = 0x0F;		// (P00)基板LED1(赤)
								// (P01)基板LED2(緑) LiveLED
								// (P02)基板LED3(緑)
								// (P03)基板LED4(緑)
	PORTB.PDR.BYTE = 0xFF;		// (PB0)PCS起動SW LED
								// (PB1)PCS停止SW LED
								// (PB2)メンテナンスSW LED/* 入力設定 */
								// (PB3)(未使用1)
								// (PB4)(未使用2)
								// (PB5)(未使用3)
								// (PB6)E/G発停リレー
								// (PB7)ファン
	PORTA.PDR.BYTE = 0xE0;		// (PA5)(未使用4)
								// (PA6)(未使用5)
								// (PA7)(未使用6)
	PORT1.PDR.BYTE = 0xC0;		// (P16)SCI0送信許可
								// (P17)SCI0受信許可
	PORT8.PDR.BYTE = 0xC0;		// (P86)SCI2送信許可
								// (P87)SCI2受信許可
								
								
   	/* ポート方向レジスタ設定 */
	/* 入力設定 */
	//リセット後は初期値0となるが、念のため0を設定して入力ポート設定を行う
	PORT5.PDR.BIT.B0 = 0;		// PCS起動SW
	PORT5.PDR.BIT.B1 = 0;		// PCS停止SW
	PORT5.PDR.BIT.B2 = 0;		// メンテナンスSW
	PORT5.PDR.BIT.B3 = 0;		// AUX3
	PORT1.PDR.BIT.B0 = 0;		// (未使用1)
	PORT1.PDR.BIT.B1 = 0;		// (未使用2)
	PORTJ.PDR.BIT.B0 = 0;		// (未使用3)
	PORTJ.PDR.BIT.B1 = 0;		// (未使用4)
	
	/* AI(analog input)設定 */
	MSTP(S12AD) = 0U;			//S12AD省電力モード解除
	
	/* Disable and clear interrupt flags of S12AD0 module */
	S12AD.ADCSR.BIT.ADIE = 0;
	S12AD.ADCMPCR.BIT.CMPAIE = 0;
	S12AD.ADCMPCR.BIT.CMPBIE = 0;
	IEN(PERIB, INTB186) = 0;
	
	/* 各チャンネルのサンプリング時間設定 */
	S12AD.ADSSTR0 = AD_AN000_SAMPLING_STATE;
	S12AD.ADSSTR1 = AD_AN001_SAMPLING_STATE;
	S12AD.ADSSTR2 = AD_AN002_SAMPLING_STATE;
	S12AD.ADSSTR3 = AD_AN003_SAMPLING_STATE;
	S12AD.ADSSTR4 = AD_AN004_SAMPLING_STATE;
	S12AD.ADSSTR5 = AD_AN005_SAMPLING_STATE;
	
	/* ADスキャン対象チャンネルの定義 */
	S12AD.ADANSA0.WORD = ((AD_AN000_USED) | (AD_AN001_USED<<1) | (AD_AN002_USED<<2) | (AD_AN003_USED<<3) | (AD_AN004_USED<<4) | (AD_AN005_USED<<5));
	/* AD変換値加算/平均モード選択 */
	S12AD.ADADS0.WORD = ((AD_AN000_ADD_USED) | (AD_AN001_ADD_USED<<1) | (AD_AN002_ADD_USED<<2) | (AD_AN003_ADD_USED<<3) | (AD_AN004_ADD_USED<<4) | (AD_AN005_ADD_USED<<5));
	/* ADコントロール拡張レジスタ */
	S12AD.ADCER.WORD = 0x0000;				// A/D変換精度指定ビット：(0)12ビット精度
											// A/Dデータレジスタ自動クリアイネーブルビット：(0) 自動クリアを禁止
											// 12ビットA/Dコンバータの自己診断を実施しない：(0) 自己診断を実施しない
											// A/Dデータのレジスタのフォーマットを右詰めにする：(0) 右詰め
	/* ADコントロールレジスタ */
	S12AD.ADCSR.WORD = 0x0000;				// スキャン終了割込み発生の禁止,シングルスキャンモード
	/* AD変換値加算/平均回数選択レジスタ */
	S12AD.ADADC.BYTE = 0x83;				// 加算回数選択ビット:0x03:4回変換
											// 平均モードイネーブルビット:(1)平均モードを選択
	/* ピン設定 */
	PORT4.PMR.BYTE &= 0xFEU;				// AN000 pin(汎用ポート、入力ポート、アナログ端子で使用)
	PORT4.PDR.BYTE &= 0xFEU;
	MPC.P40PFS.BYTE = 0x80U;
	PORT4.PMR.BYTE &= 0xFDU;				// AN001 pin(汎用ポート、入力ポート、アナログ端子で使用)
	PORT4.PDR.BYTE &= 0xFDU;
	MPC.P41PFS.BYTE = 0x80U;
	PORT4.PMR.BYTE &= 0xFBU;				// AN002 pin(汎用ポート、入力ポート、アナログ端子で使用)
	PORT4.PDR.BYTE &= 0xFBU;
	MPC.P42PFS.BYTE = 0x80U;
	PORT4.PMR.BYTE &= 0xF7U;				// AN003 pin(汎用ポート、入力ポート、アナログ端子で使用)
	PORT4.PDR.BYTE &= 0xF7U;
	MPC.P43PFS.BYTE = 0x80U;
	PORT4.PMR.BYTE &= 0xEFU;				// AN004 pin(汎用ポート、入力ポート、アナログ端子で使用)
	PORT4.PDR.BYTE &= 0xEFU;
	MPC.P44PFS.BYTE = 0x80U;
	PORT4.PMR.BYTE &= 0xDFU;				// AN005 pin(汎用ポート、入力ポート、アナログ端子で使用)
	PORT4.PDR.BYTE &= 0xDFU;
	MPC.P45PFS.BYTE = 0x80U;
	
}
//******************************************************************************
//  機能     : RTC初期設定
//  機能説明 : RX72M内臓のRTCの設定を行う。
//           : バッテリー切れなどによって、RTCコントロールレジスタが時刻情報を
//           : 失っている場合のみ初期化を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ini_rtc(void)
{
	if(RTC.RCR2.BIT.RESET != 1){return;}							//リセットビットが1の場合のみRTCを設定する

	
	/* RTCコントロールレジスタチェック */
	SYSTEM.PRCR.WORD = 0xA502;										//書き込み保護解除
	
	RTC.RCR4.BYTE = 0;												//サブクロック発信器を選択
	while (1U != RTC.RCR3.BIT.RTCEN){RTC.RCR3.BIT.RTCEN = 1U;}		//サブクロック発信器動作
	
	RTC.RCR2.BYTE = 0x00U;											//全カウンタ停止
	while (RTC.RCR2.BIT.START != 0U){}

	RTC.RCR2.BIT.RESET = 1U;										//RTCソフトウェアリセット
	while (RTC.RCR2.BIT.RESET != 0U){}

	RTC.RCR2.BIT.START = 0;											//RTC停止
	while(RTC.RCR2.BIT.START != 0);									//STARTビットが0になったか
	
	RTC.RCR2.BIT.CNTMD = 0;											//カレンダモード設定
	while(RTC.RCR2.BIT.CNTMD != 0);									//カレンダモードが0になったか
	
	RTC.RCR2.BIT.HR24 = 1;											//24時間モード
	while(RTC.RCR2.BIT.HR24 != 1);									//時間モードビットが1になったか
	
	/* 初期時刻設定 */
	RTC.RSECCNT.BYTE = 0x00;										//0秒
	RTC.RMINCNT.BYTE = 0x00;										//0分
	RTC.RHRCNT.BYTE  = 0x00;										//0時
	RTC.RDAYCNT.BYTE = 0x01;										//1日
	RTC.RMONCNT.BYTE = 0x01;										//1月
	RTC.RYRCNT.WORD  = 0x2026;										//2026年

	/* RTC開始 */
	RTC.RCR2.BIT.START = 1;											//RTC開始
	while(RTC.RCR2.BIT.START != 1);									//STARTビットが1になったか
	
	SYSTEM.PRCR.WORD = 0xA500;										//書き込み保護設定
	
}
//******************************************************************************
//  機能     : DO端子初期化
//  機能説明 : DO端子へ出力の初期値を設定する。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ini_do(void)
{
	/* LED出力 */
	SET_LED(LED1_RED,   LED_OFF);			//基板LED1(赤) 出力OFF
	SET_LED(LED2_GREEN, LED_OFF);			//基板LED2(緑) 出力OFF
	SET_LED(LED3_GREEN, LED_OFF);			//基板LED3(緑) 出力OFF
	SET_LED(LED4_GREEN, LED_OFF);			//基板LED4(緑) 出力OFF
	
	/* DO出力 */
	SET_DO(DO_PCS_START_SW_LED, BIT_OFF);	//PCS起動SW LED
	SET_DO(DO_PCS_STOP_SW_LED,  BIT_ON );	//PCS停止SW LED
	SET_DO(DO_PCS_MAINTESW_LED, BIT_OFF);	//メンテナンスSW LED
	SET_DO(DO_PCS_SHT_RY,       BIT_OFF);	//SHT制御リレー
	SET_DO(DO_YOBI2,            BIT_OFF);	//(未使用2)
	SET_DO(DO_YOBI3,            BIT_OFF);	//(未使用3)
	SET_DO(DO_EG_RY,            BIT_OFF);	//E/G発停リレー
	SET_DO(DO_FAN,              BIT_OFF);	//ファン
	SET_DO(DO_YOBI4,            BIT_OFF);	//(未使用4)
	SET_DO(DO_YOBI5,            BIT_OFF);	//(未使用5)
	SET_DO(DO_YOBI6,            BIT_OFF);	//(未使用6)
	
	/* RS485(SCI0,SCI2)送受信許可 */
	SCI0_RECV_ENABLE;						//SCI0受信許可(送信無効)
	SCI2_RECV_ENABLE;						//SCI2受信許可(送信無効)
	
}
//******************************************************************************
//  機能     : SCI端子初期化
//  機能説明 : シリアル通信用のSCI端子の初期設定を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ini_sci(void)
{
	
	/* UART0 スイッチング電源用 */
	ini_sio(UART0, UART0_CFG);				//シリアル入出力初期化
	ctl_sio(UART0, UART0_CTL);				//シリアル入出力制御
	
	/* UART1 デバッグ情報出力用 */
	ini_sio(UART1, UART1_CFG);				//シリアル入出力初期化
	ctl_sio(UART1, UART1_CTL);				//シリアル入出力制御
	
	/* UART2 山洋電気製PCS用 */
	ini_sio(UART2, UART2_CFG);				//シリアル入出力初期化
	ctl_sio(UART2, UART2_CTL);				//シリアル入出力制御
	
}
//******************************************************************************
//  機能     : グローバル変数初期化処理
//  機能説明 : グローバル変数の初期化処理を行う。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ini_g_val(void)
{
	
	/* 変数定義 */
	int i;
	
	/* CAN送信テーブル初期化 */
	memcpy(g_can_sndmsg,can_sndmsg_init,sizeof(g_can_sndmsg));
	/* CAN受信テーブル初期化 */
	memcpy(g_can_rcvmsg,can_rcvmsg_init,sizeof(g_can_rcvmsg));
	/* 接点入力信号管理テーブル初期化 */
	for(i=0;i<DI_MAX;i++){
		g_di[i].old = 0;
		g_di[i].wk  = 0;
		g_di[i].val = 0;
		g_di[i].cnt = 0;
	}
	/* PCS1状態管理フラグ初期化 */
	memset(&g_pcs1, 0, sizeof(g_pcs1));
	/* 発電機状態フラグ初期化 */
	memset(&g_gen, 0, sizeof(g_gen));
	/* ModbusTCPテーブル初期化 PCS1(I_DENCON)HOLDING REGISTERS(EMS→I_DENCON) */
	memcpy(&g_modbustcp_holding,&modbustcp_holding_init,sizeof(g_modbustcp_holding));
	/* ModbusTCPテーブル初期化 PCS1(I_DENCON)INPUT REGISTERS(I_DENCON→EMS) */
	memcpy(&g_modbustcp_input,&modbustcp_input_init,sizeof(g_modbustcp_input));
	/* UDP管理テーブル初期化 PCS運転コマンド */
	memset(&g_udpsnd_pcsruncmd, 0, sizeof(g_udpsnd_pcsruncmd));
	/* RS485管理テーブル初期化 */
	memset(&g_rs485, 0, sizeof(g_rs485));
	/* ModbusRTU(RS485)管理テーブル */
	memset(&g_modbus_req, 0, sizeof(g_modbus_req));
	memset(&g_modbus_res, 0, sizeof(g_modbus_res));
	/* AI値格納変数初期化 */
	memset(&g_ai, 0, sizeof(g_ai));
}
//******************************************************************************
//  機能     : ウォッチドッグタイマ初期化処理
//  機能説明 : ウォッチドッグタイマの初期化処理を行う。
//           : タイムアウト値:約2.2秒
//           : WDTCR レジスタへの書き込みは、リセット解除後から
//           : 最初のリフレッシュ動作までの間に1 回のみ可能であるため
//           : 必ずWORDアクセスで書き込む。
//------------------------------------------------------------------------------
//  引数     : void
//------------------------------------------------------------------------------
//  戻り値   : void
//******************************************************************************
void ini_wdt(void)
{
	/* WDTコントロールレジスタ設定 */
	WDT.WDTCR.WORD = 0x3383;		//設定項目は下記とする
	
	/* 参考 */
	//WDT.WDTCR.BIT.TOPS = 0x03;		//タイムアウト期間選択ビット  ：16384cyc
	//WDT.WDTCR.BIT.CKS  = 0x08;		//クロック分周比選択ビット    ：8192分周
	//WDT.WDTCR.BIT.RPES = 0x03;		//ウィンドウ終了位置選択ビット：0%
	//WDT.WDTCR.BIT.RPSS = 0x03;		//ウィンドウ開始位置選択ビット：100%
	
}
/* end */
