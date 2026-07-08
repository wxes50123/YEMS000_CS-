//******************************************************************************
// @(R)
//		デバック用関数(SCI1)
//		
//		MTEST_DEBUGが定義されている場合のみ有効
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//******** 変更履歴 ************************************************************
//Ver1.0	初版	Y.WATANABE
//******************************************************************************
#define DEBUG_FUNC_DEFINE			//デバッグ用FUNCTIONが定義されていることを示す
//******** ヘッダファイル参照宣言部 ********************************************
#include "g_include_def.h"
//******** 外部参照宣言部 ******************************************************
extern char *lto0X(char *s, unsigned long d, int n);
extern char *ito0X(char *s, unsigned short d, int n);
extern char *ito_d(char *s, short d, int n);
extern char *ito_u(char *s, unsigned short d, int n);
extern char *ito0u(char *s, unsigned short d, int n);
extern char *lto_u(char *s, unsigned long d, int n);
extern char *lto_d(char *s, long d, int n);
extern int itoulen(unsigned short d);
//******** プロトタイプ宣言部 **************************************************
void DebugOut(char *str);										//文字の出力
void DebugOut_time(void);										//時刻の出力
void DebugOut_ito0X(char *s , unsigned short n);				//文字とUHを16進数で出力,先行0付
void DebugOut_lto0X(char *s , unsigned long n);					//文字とUWを16進数で出力,先行0付

void DebugOut_itou(char *s , unsigned short n);					//文字とUHを10進数で出力,右詰め
void DebugOut_ltou(char *s , unsigned long n);					//文字とUWを10進数で出力,右詰め

void DebugOut_itod(char *s , short n);							//文字とHを10進数で出力,符号付
void DebugOut_ltod(char *s , long n);							//文字とWを10進数で出力,符号付

void DebugOut_itou_fit(char *s , unsigned short n);				//文字とUHを10進数で出力（数値は桁数に応じた文字数で出力）
void DebugOut_ltou_fit(char *s , unsigned long n);				//文字とUWを10進数で出力（数値は桁数に応じた文字数で出力）

void DebugOut_LF(void);											//改行出力
static ER DebugOutC(char *s);									//1文字ずつの送信処理
//******** 関数定義部 **********************************************************
//******************************************************************************
//  機能     : 	文字の出力
//  
//  機能説明 : 	実際の使用関数はPout_s
//******************************************************************************
void DebugOut(char *str)
{
	while(*str) {
		DebugOutC(str);
		str++;
	}
}
//******************************************************************************
//  機能     :	時刻の出力
//  
//  機能説明 :	実際の使用関数はPout_time
//******************************************************************************
void DebugOut_time(void)
{
	char str[6];
	ito0u(str , g_time.year , 4);
	DebugOut(str);
	DebugOut("/");
	ito0u(str , g_time.month , 2);
	DebugOut(str);
	DebugOut("/");
	ito0u(str , g_time.day , 2);
	DebugOut(str);
	DebugOut(" ");
	ito0u(str , g_time.hour , 2);
	DebugOut(str);
	DebugOut(":");
	ito0u(str , g_time.min , 2);
	DebugOut(str);
	DebugOut(":");	
	ito0u(str , g_time.sec , 2);
	DebugOut(str);
	DebugOut(" ");
}
//******************************************************************************
//  機能     : 	文字とUHを16進で出力(先行0付)
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_ito0X(char *s , unsigned short n)
{
	char str[6];
	DebugOut(s);
	ito0X(str , n ,4);
	DebugOut(str);
}
//******************************************************************************
//  機能     : 	文字とUWを16進で出力(先行0付)
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_lto0X(char *s , unsigned long n)
{
	char str[12];
	DebugOut(s);
	lto0X(str , n ,8);
	DebugOut(str);
}
//******************************************************************************
//  機能     : 	文字とUHを10進で出力(右詰め,符号無し)
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_itou(char *s , unsigned short n)
{
	char str[6];
	DebugOut(s);
	ito_u(str , n ,4);
	DebugOut(str);
}
//******************************************************************************
//  機能     : 	文字とUWを10進で出力(右詰め,符号無し)
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_ltou(char *s , unsigned long n)
{
	char str[12];
	DebugOut(s);
	lto_u(str , n ,8);
	DebugOut(str);
}
//******************************************************************************
//  機能     : 	文字とHを10進数で出力(右詰め,符号付)
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_itod(char *s , short n)
{
	char str[7];
	DebugOut(s);
	ito_d(str , n ,6);
	DebugOut(str);
}
//******************************************************************************
//  機能     : 	文字とWを10進数で出力(右詰め,符号付)
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_ltod(char *s , long n)
{
	char str[12];
	DebugOut(s);
	lto_d(str , n ,11);
	DebugOut(str);
}
//******************************************************************************
//  機能     : 	文字とUHを出力（数値は桁数に応じた文字数で出力）
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_itou_fit(char *s , unsigned short n) {

	int  keta;
	char str[12];
	DebugOut(s);
	keta=itoulen(n);
	ito_u(str , n , keta);
	DebugOut(str);
	
}
//******************************************************************************
//  機能     : 	文字とUWを出力（数値は桁数に応じた文字数で出力）
//  
//  機能説明 : 	実際の使用関数はPout_***
//******************************************************************************
void DebugOut_ltou_fit(char *s , unsigned long n) {

	int  keta;
	char str[12];
	DebugOut(s);
	keta=itoulen(n);
	lto_u(str , n , keta);
	DebugOut(str);
	
}
//******************************************************************************
//  機能     : 	改行出力
//  
//  機能説明 : 	実際の使用関数はPout_*LF()
//******************************************************************************
void DebugOut_LF(void) {

	DebugOut("\r\n");
}
//******************************************************************************
//  機能     : 	1文字ずつの送信処理
//  
//  機能説明 : 	タイムアウトを監視し、データの送信を行う
//******************************************************************************
static ER DebugOutC(char *s)
{
	TMO tmout = DEBUGOUT_TXTMO;
	ER ercd = E_OK;

	/* シリアル1文字出力 */
	ercd = put_sio(DEBUGOUT_CH, (UB)* s, tmout);

	return ercd;
}