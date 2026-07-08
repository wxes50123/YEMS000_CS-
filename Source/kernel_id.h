/*****************************************************************************
* Kernel Configuration
******************************************************************************/
/* タスクID */
#define ID_TIMER_TSK		1
#define ID_IDLE_TSK			2
#define ID_RS485_TSK		3
#define ID_MODBUS_TSK		4
#define ID_LANINI_TSK		5
#define ID_LANCLI_UDP_TSK	6
#define ID_LANCLI_TCP_TSK	7
#define ID_DI_TSK			8
#define ID_MAIN_TSK			9
#define ID_AI_TSK			10
#define ID_CAN_TSK			11

/* イベントフラグID */
#define ID_TIMER_FLG		1
#define ID_RS485_FLG		2
#define ID_MODBUS_FLG		3
#define ID_DI_FLG			4
#define ID_LANCLI_UDP_FLG	5
#define ID_LANCLI_TCP_FLG	6
#define ID_MAIN_FLG			7
#define ID_AI_FLG			8
#define ID_CAN_FLG			9

/* TCP/IP コンフィグレーション */
#define TCP_REPID_MAX	2				/* TCP受付口ID上限 */
#define TCP_CEPID_MAX	2				/* TCP通信端点ID上限 */
#define UDP_CEPID_MAX	2				/* UDP通信端点ID上限 */
#define TCP_TSKID_TOP 	0				/* TCP/IP用タスク先頭ID */
#define TCP_SEMID_TOP	0				/* TCP/IP用セマフォ先頭ID */
#define TCP_MBXID_TOP	0				/* TCP/IP用メールボックス先頭ID */
#define TCP_MPFID_TOP	0				/* TCP/IP用固定長メモリプール先頭ID */
#define PRI_IP_SND_TSK	3				/* IP送信タスク優先度 *///LANタスクより優先度が高いこと！！
#define PRI_IP_RCV_TSK	PRI_IP_SND_TSK	/* IP受信タスク優先度 *///LANタスクより優先度が高いこと！！
#define SSZ_IP_SND_TSK	(4096)			/* スタックサイズ IP送信タスク */
#define SSZ_IP_RCV_TSK	(4096)			/* スタックサイズ IP受信タスク */
#define TCP_KTIME_INI	(600)			/* キープアライブパケットが */
										/* 送信されるまでの時間(10分) */
										/* デフォルト=7200(2時間) */
