//******************************************************************************
// @(R)
//		グローバル変数定義
//		
// @(X)
//
//  Copyright (c) Hitachi Power Solutions Co., Ltd. All rights reserved.
//******************************************************************************
//****** 変更履歴 **************************************************************
//ver 1.0	25.09.11	初版作成
//******************************************************************************
#ifdef  GLOBAL_VALUE_DEFINE
#define GLOBAL
#define VALG(v)			=(v)
#define VAL1(x)			={x}
#define VAL2(x,y)		={x,y}
#define VAL3(x,y,z)		={x,y,z}
#define VAL4(x,y,z,r)	={x,y,z,r}
#define VAL5(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50)	={x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50}

#else
#define GLOBAL	extern
#define VALG(v)
#define VAL1(x)
#define VAL2(x,y)
#define VAL3(x,y,z)
#define VAL4(x,y,z,r)
#define VAL5(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50)

#endif
/******************************************************************************/
//	プロトタイプ宣言部   構造体定義とその実体
/******************************************************************************/
#define CNT_SOC_SBUF	(4096)	//送信バッファ数
#define CNT_SOC_RBUF	(4096)	//受信バッファ数

typedef struct {
//***	送受信バッファのアドレスが偶数アドレスとする必要がある。	***//
//***	奇数の場合、E_PARエラーがでる。UB,Bの定義は注意する。		***//
	ER	ercd;					//システムコール エラーコード
	ID	cepid;					//通信端点ID
	ID	repid;					//受付口ID
	T_IPV4EP	myaddr;			//自分側のIPアドレスとポート番号
	T_IPV4EP	dstaddr;		//相手側のIPアドレスとポート番号
	UW	stat;					//通信ステータス
	UW	sndlen;					//送信データサイズ
	UW	rcvlen;					//受信データサイズ
	UB	schar;					//受信開始文字
	UB	echar;					//受信終了文字
	UB	sndbuf_wk[CNT_SOC_SBUF];//送信バッファ(内部ワーク)
	UB	rcvbuf_wk[CNT_SOC_RBUF];//受信バッファ(内部ワーク)
	UB	sbuf[CNT_SOC_SBUF];		//送信バッファ(API使用エリア)
	UB	rbuf[CNT_SOC_RBUF];		//受信バッファ(API使用エリア)
	
} T_SOC_CB;						//ソケット通信制御ブロック
GLOBAL T_SOC_CB udpcb;			//UDP制御ブロック
GLOBAL T_SOC_CB tcpcb;			//TCP制御ブロック
/********** ModbusTCP管理テーブル *********************************************/
typedef struct{ UB val; UB min; UB max; }UB_RANGE;
typedef struct{  B val;  B min;  B max; }B_RANGE;
typedef struct{ UH val; UH min; UH max; }UH_RANGE;
typedef struct{  H val;  H min;  H max; }H_RANGE;
typedef struct{ UW val; UW min; UW max; }UW_RANGE;
typedef struct{  W val;  W min;  W max; }W_RANGE;

/* PCS1(I_DENCON)HOLDING REGISTERS(EMS→I_DENCON) */
typedef struct{
	
	UH_RANGE ev_chg_power;			//EV充電電力
	UH_RANGE ev_dischg_power;		//EV放電電力
	UH_RANGE bat_chg_power;			//電池充電電力
	UH_RANGE bat_dischg_power;		//電池放電電力
	UH_RANGE grid_in_power;			//系統入力電力
	UH_RANGE grid_out_power;		//系統出力電力
	UH_RANGE ev_soc_max;			//EV充電上限SOC
	UH_RANGE ev_soc_min;			//EV充電下限SOC
	UH_RANGE bat_soc_max;			//電池充電上限SOC
	UH_RANGE bat_soc_min;			//電池充電下限SOC
	UB_RANGE ope_mode;				//運転パターン
	UB_RANGE error_reset;			//エラーリセット
	UB_RANGE evconnnected;			//EV接続/切断
	UB_RANGE yobi;					//空き
	
}HOLDING_REGISTERS;
GLOBAL HOLDING_REGISTERS g_modbustcp_holding;		//Holding Registers用グローバル変数

/* PCS1(I_DENCON)INPUT REGISTERS(I_DENCON→EMS) */
typedef struct{
	
	UB_RANGE heartbeat;				//ハートビート
	UB_RANGE system_state;			//システム状態
	UB_RANGE grid_state;			//系統状態
	UB_RANGE ev_state;				//EV状態
	UB_RANGE ev_connected;			//EV接続
	UB_RANGE pv_state;				//PV状態
	UB_RANGE bat_state;				//電池状態
	UB_RANGE error_code;			//エラーコード
	
	B_RANGE internal_temp1;			//内部温度1
	B_RANGE internal_temp2;			//内部温度2
	B_RANGE internal_temp3;			//内部温度3
	B_RANGE internal_temp4;			//内部温度4
	B_RANGE internal_temp5;			//内部温度5
	B_RANGE enclosure_temp;			//筐体温度
	B_RANGE bat_temp_max;			//電池最高温度
	B_RANGE bat_temp_avg;			//電池平均温度
	B_RANGE bat_temp_min;			//電池最低温度
	
	UH_RANGE grid_v_rs;				//系統RS相電圧
	UH_RANGE grid_v_st;				//系統ST相電圧
	UH_RANGE grid_v_tr;				//系統TR相電圧
	
	H_RANGE grid_i_r;				//系統R相電流
	H_RANGE grid_i_s;				//系統S相電流
	H_RANGE grid_i_t;				//系統T相電流
	H_RANGE grid_pf_r;				//系統R相力率
	H_RANGE grid_pf_s;				//系統S相力率
	H_RANGE grid_pf_t;				//系統T相力率
	
	UH_RANGE grid_freq;				//系統周波数
	
	H_RANGE grid_p_active;			//系統有効電力
	H_RANGE grid_p_reactive;		//系統無効電力
	
	UH_RANGE ev_voltage;			//EV電圧
	
	H_RANGE ev_current;				//EV電流
	
	UH_RANGE ev_soc;				//EV SOC
	UH_RANGE pv_voltage;			//PV電圧
	UH_RANGE pv_current;			//PV電流
	UH_RANGE bat_voltage;			//電池電圧
	
	H_RANGE bat_current;			//電池電流
	
	UH_RANGE bat_soc;				//電池SOC
	UH_RANGE ev_chg_power;			//EV充電電力
	UH_RANGE ev_dischg_power;		//EV放電電力
	UH_RANGE bat_chg_power;			//電池充電電力
	UH_RANGE bat_dischg_power;		//電池放電電力
	UH_RANGE grid_input_power;		//系統入力電力
	UH_RANGE grid_output_power;		//系統出力電力
	
	UH_RANGE ev_soc_chg_limit;		//EV充電上限SOC
	UH_RANGE ev_soc_dischg_limit;	//EV充電下限SOC
	UH_RANGE bat_soc_chg_limit;		//電池充電上限SOC
	UH_RANGE bat_soc_dischg_limit;	//電池放電下限SOC
	UB_RANGE ope_mode;				//運転パターン
	
	UW_RANGE grid_energy_in_wh;		//系統入力電力量積算
	UW_RANGE grid_energy_out_wh;	//系統出力電力量積算
	UW_RANGE ev_energy_chg_wh;		//EV充電電力量積算
	UW_RANGE ev_energy_dischg_wh;	//EV放電電力量積算
	UW_RANGE bat_energy_chg_wh;		//電池充電電力量積算
	UW_RANGE bat_energy_dischg_wh;	//電池放電電力量積算
	UW_RANGE pv_energy_wh;			//PV電力量積算
	
}INPUT_REGISTERS;
GLOBAL INPUT_REGISTERS g_modbustcp_input;			//Input Registers用グローバル変数
/********** UDP管理テーブル ***************************************************/
/* PCS1(I_DENCON)(UDP)(EMS→I_DENCON) */
/* PCS運転コマンド */
typedef struct{
	
	UH_RANGE mode;								//運転モード番号
	UH_RANGE grid_input_power_limit;			//系統部 入力系統電力上限値
	UH_RANGE grid_regen_power_limit;			//系統部 回生系統電力上限値
	UH_RANGE bat_chg_power_limit;				//蓄電池部 充電電力上限値
	UH_RANGE bat_dischg_power_limit;			//蓄電池部 放電電力上限値
	UH_RANGE bat_chg_end_soc;					//蓄電池部 充電終了SOC
	UH_RANGE bat_dischg_end_soc;				//蓄電池部 放電終了SOC
	UH_RANGE ev_chg_power_limit;				//EV部 充電電力上限値
	UH_RANGE ev_dischg_power_limit;				//EV部 放電電力上限値
	UH_RANGE ev_chg_end_soc;					//EV部 充電終了SOC
	UH_RANGE ev_dischg_end_soc;					//EV部 放電終了SOC
	UH_RANGE lamp_switch_cmd;					//ランプ点灯切り替え
	UH_RANGE grid_stabilization_mode;			//系統安定化機能モード
	UH_RANGE regen_active_power_limit;			//回生有効電力上限設定
	UH_RANGE fixed_reactive_power;				//固定無効電力設定
	UH_RANGE fixed_power_factor;				//固定力率設定
	UH_RANGE start_ramp_rate;					//開始上昇率
	UH_RANGE variation_rate;					//変動率
	UH_RANGE battery_4r_restart_cmd;			//4R電池再起動
	UH_RANGE dischg_internal_resistance_sel;	//放電内部抵抗選択
	UH_RANGE yobi;								//予備
	
}PCS_RUN_CMD;
GLOBAL PCS_RUN_CMD g_udpsnd_pcsruncmd;			//PCS運転コマンド送信用グローバル変数

/********** RS485管理テーブル *************************************************/
typedef struct{
	
	UB send_buf[30];		//送信バッファ
	UB recv_buf[30];		//受信バッファ
	UH vol_now;				//電圧値(現在値)(単位0.01V)(0～40000)
	UH cur_now;				//電流値(現在値)(単位0.01A)(0～750)
	UH vol_set;				//電圧値(設定値)(単位0.01V)(0～40000)
	UH cur_set;				//電流値(設定値)(単位0.01A)(0～750)
	
}RS485_TBL;
GLOBAL RS485_TBL g_rs485;						//RS485管理用グローバル変数

/********** RTC用タイムテーブル ***********************************************/
typedef struct {

	UH	year;					//年
	UB	month;					//月
	UB	day;					//日
	UB	hour;					//時
	UB	min;					//分
	UB	sec;					//秒
	UB	weekday;				//曜日(使用しない)
	
} TIME_TABLE;
GLOBAL TIME_TABLE g_time;	//RTC用タイムテーブル
/********** DI信号管理テーブル ************************************************/
#define DI_MAX	10			//接点入力信号最大数
typedef struct{
	
	UB	old;					//前回値
	UB	wk;						//ワーク
	UB	val;					//現在値
	UH	cnt;					//カウンタ
	
} DITABLE;
GLOBAL DITABLE g_di[DI_MAX];
/********** CAN管理テーブル ***************************************************/
#define CAN_SNDID_MAX	29		//CAN送信ID 最大数
#define CAN_RCVID_MAX	3		//CAN受信ID 最大数

/* データタイプ表示用 */
typedef enum{

	CAN_TYPE_UB,				//UB型
	CAN_TYPE_UH,				//UH型
	CAN_TYPE_UW					//UW型

}CAN_DATA_TYPE;

/* データ共用体 */
typedef union{
	
	UB ub;						//UB(1byte型)
	UH uh;						//UH(2byte型)
	UW uw;						//UW(4byte型)
	
}CAN_DATA_VALUE;

/* データ項目定義 */
typedef struct{
	
	CAN_DATA_TYPE  type;		//データタイプ
	CAN_DATA_VALUE data;		//データ(現在値)
	CAN_DATA_VALUE bunkai;		//分解能
	CAN_DATA_VALUE offset;		//offset
	CAN_DATA_VALUE def;			//DefaultData
	CAN_DATA_VALUE min;			//最小値
	CAN_DATA_VALUE max;			//最大値
	
}CAN_DATA_ENTRY;

/* CANメッセージ構造体 */
typedef struct{
	
	UW id;							//CANID
	CAN_DATA_ENTRY table[8];		//データ項目数(最大は1byteデータが8項目)
	UB entry_count;					//項目数(このIDで使用するデータ項目数)
	
}CAN_MESSAGE;

/* CAN送信データ */
GLOBAL CAN_MESSAGE g_can_sndmsg[CAN_SNDID_MAX];			//CAN送信データ
/* CAN受信データ */
GLOBAL CAN_MESSAGE g_can_rcvmsg[CAN_RCVID_MAX];			//CAN受信データ
/********** ModbusRTU(RS485)管理テーブル **************************************/
/* 要求メッセージ */
typedef union{
	struct{
		UB slave_addr;			//スレーブアドレス
		UB func_code;			//ファンクションコード
		UB start_high_addr;		//開始アドレス(上位)
		UB start_low_addr;		//開始アドレス(下位)
		UB rgnum_high_addr;		//レジスタの数(上位)
		UB rgnum_low_addr;		//レジスタの数(下位)
		UB err_crc_low;			//エラーチェック(下位)※エラーチェックのみ(上位、下位が逆)
		UB err_crc_high;		//エラーチェック(上位)
	}req;
	UB buf[8];					//バッファ
}UNION_REQ;
GLOBAL UNION_REQ g_modbus_req;	//

/* レスポンスメッセージ(最大30件) */
typedef union{
	struct{
		UB slave_addr;			//スレーブアドレス
		UB func_code;			//ファンクションコード
		UB data_len;			//データバイト数
		UB data_high[60];		//データ
		UB err_crc_low;			//エラーチェック(下位)※エラーチェックのみ(上位、下位が逆)
		UB err_crc_high;		//エラーチェック(上位)
	}res;
	UB buf[65];					//バッファ
}UNION_RES;
GLOBAL UNION_RES g_modbus_res;	//グローバル変数実体
/********* PCS1状態管理フラグ *************************************************/
/* PCS1:I_DENCON(IKS) */
typedef struct {
	
	UB state_flg;				//PCS1状態(0:自立運転停止中,1:自立運転開始中) 
	UB err_flg;					//PCS1エラー状態(0:エラーなし,1:エラー発生中)
	UB gridreturn_flg;			//PCS1自立運転中の系統電力復帰フラグ(0:自立運転中,1:系統電力復帰)
	UB mntsw_led_flg;			//メンテナンスSWLEDの点滅状態(0:無し,1:1秒間隔点滅,2:2秒間隔点滅)
	UB mode;					//PCS動作モード
	UB breaker_trip_count;		//ブレーカ遮断回数
	UB comm_error_flg;			//PCS1通信異常(0:通信異常なし,1:通信異常あり)
	
} PCS1_ST_TABLE;
GLOBAL PCS1_ST_TABLE g_pcs1;	//グローバル変数実体
/********* 発電機状態管理フラグ ***********************************************/
typedef struct {
	
	UB run_state;				//発電機運転状態(0:停止中,1:運転中)
	UB start_flg;				//発電機始動フラグ(0:OFF,1:ON)
	UB start_cause;				//発電機始動要因(bit0:SOC基準値以下,bit1:電池電圧基準値以下,bit2:スケジュール時間)
	UB charge_time_flg;			//発電機充電スケジュールフラグ(ON:充電スケジュール時間内,OFF:充電スケジュール時間外)
	UB contact_cleaning_flg;	//接点クリーニングフラグ(0:接点クリーニング無し,1:接点クリーニング中)
	
	/* 外部コントロール */
	UB ext_genstop;				//発電機始動停止
	UB ext_mode;				//運転モード指令
	UB ext_charge_reset;		//充電器電流リセット
	UB ext_none1;				//未使用1(外部コントロール)
	UB ext_gencurrent_limit;	//発電機電流制限
	UB ext_charge_mode;			//充電器の充電モード
	UB ext_can_debug;			//CANデバッグモード
	UB ext_regen_inhibit;		//回生充電禁止
	/* AVR基板 */
	UB avr_freq;				//周波数(50Hz or 60Hz or 0Hz)(移動平均後の値)
	UH avr_rpm;					//エンジン回転数(0-8000rpm)(移動平均後の値)
	UH avr_volt_meas;			//発電電圧測定値(V)(移動平均後の値)
	UH avr_volt_target;			//発電電圧目標値(V)(移動平均後の値)
	UB avr_none1;				//未使用1(AVR基板)
	UB avr_none2;				//未使用2(AVR基板)
	/* 表示基板 */
	UH disp_current_r;			//発電電流R相(A)(移動平均後の値)
	UH disp_current_t;			//発電電流T相(A)(移動平均後の値)
	UB disp_none1;				//未使用1(表示基板)
	UB disp_none2;				//未使用2(表示基板)
	UB disp_none3;				//未使用3(表示基板)
	UB disp_none4;				//未使用4(表示基板)
	/* 通信断(タイムアウト) */
	UB ext_msg_timout_flg;		//外部コントロール通信タイムアウトフラグ(0:正常,1:通信断)
	UB avr_msg_timout_flg;		//AVR基板通信タイムアウトフラグ(0:正常,1:通信断)
	UB disp_msg_timout_flg;		//表示基板通信タイムアウトフラグ(0:正常,1:通信断)
	/*  */
	UB start_on_tim_flg;		//発電機始動時間フラグON後経過フラグ(0:OFF,1:60秒以上,2:11分以上)
	UB off_wait_tim_flg;		//発電機始動リレーOFF待機フラグ(0:それ以外,1:待機中(15秒以内))
	/* ブレーカ */
	UB breaker_off_flg;			//ブレーカ遮断フラグ(0:ブレーカ遮断なし,1:ブレーカ遮断中)
	UB breaker_off_tim_flg;		//ブレーカ遮断判定停止後フラグ(0:未経過,1:10秒以上経過)
	
	/* 発電制御電圧 */
	UB voltage_ajust;			//電圧調整(0:現状維持,1:15秒ごとに1VUP,2:2秒ごとに1VUP,3:0.1秒ごとに1VDOWN)
	UH voltage_set;				//発電制御電圧設定値
	
} GENERATOR_ST_TABLE;
GLOBAL GENERATOR_ST_TABLE g_gen;//グローバル変数実体
/********** LAN通信用 *********************************************************/
GLOBAL UB default_ipaddr[4];	//自分側IPアドレス
GLOBAL UB ethernet_addr[6];		//MACアドレス
GLOBAL UB default_gateway[4];	//デフォルトゲートウェイ
GLOBAL UB subnet_mask[4];		//サブネットマスク
GLOBAL UB dst_ipaddr1[4];		//相手側IPアドレス1(PCS_UDP)
GLOBAL UB dst_ipaddr2[4];		//相手側IPアドレス2(PCS_TCP)
/********** AI用 **************************************************************/
GLOBAL UH g_ai[6];				//AI値格納変数(AN000～AN0005)

