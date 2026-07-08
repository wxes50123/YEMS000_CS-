#include "kernel.h"
#include "kernel_id.h"
#include "nonet.h"

#include "nonetc.h"						// 本ヘッダーは1箇所のみでインクルードのこと

#include "g_extern_def.h"				//共通関数

/* 外部参照(タスク定義) */
extern TASK main_task( VP_INT );		//メインタスク
extern TASK ai_task( VP_INT );			//AIタスク
extern TASK can_task( VP_INT );			//CANタスク
extern TASK di_task( VP_INT );			//DIタスク
extern TASK idle_task( VP_INT );		//アイドルタスク
extern TASK lancli_udp_task( VP_INT );	//UDP/IPタスク
extern TASK modbus_tcp_task( VP_INT );	//ModbusTCPタスク
extern TASK lanini_task( VP_INT );		//LANイニシャルタスク
extern TASK rs485_task( VP_INT );		//RS485タスク(RS-485通信用 通信対象：スイッチング電源)
extern TASK modbus_rtu_task( VP_INT );	//ModbusRTUタスク(Modbus通信用 通信対象:山洋電気製PCS)
extern TASK timer_task( VP_INT );		//タイマタスク
extern ini_sci();						//シリアル端子初期化

/* Kernel Configuration */
#define TPRI_MAX 9						//タスク優先度上限
#define TSKID_MAX  11+(TCP_NTSK)		/* task */
#define SEMID_MAX  1+(TCP_NSEM)			/* semaphore */
#define FLGID_MAX  9+(TCP_NFLG)			/* eventflag */
#define MBXID_MAX  1+(TCP_NMBX)			/* mailbox */
#define MBFID_MAX  5+(TCP_NMBF)			/* message buffer */
#define PORID_MAX  1+(TCP_NPOR)			/* rendezvous port */
#define MPLID_MAX  1+(TCP_NMPL)			/* variable-size memory pool */
#define MPFID_MAX  1+(TCP_NMPF)			/* fixed-size memory pool */
#define DTQID_MAX  1+(TCP_NDTQ)			/* data queue */
#define MTXID_MAX  1+(TCP_NMTX)			/* mutex */
#define ISRID_MAX  4+(TCP_NISR)			/* interrupt service routine */
#define CYCNO_MAX  1+(TCP_NCYC)			/* cyclic handler */
#define ALMNO_MAX  1+(TCP_NALM)			/* alarm handler */


#define ISTKSZ	512					/* size of stack for interrupt handlers */
#define TSTKSZ	512					/* size of stack for time event handlers */


/* 割込みレベル */
#define KNL_LEVEL		14			//カーネルレベルの割込み禁止レベル


#include "nocfg4.h"					//標準コンフィグレーションファイル	//ver00_001
									//上記までに#define定義されていない設定は標準設定をここから設定される。


/* タスク定義 */
//lancli_udp_task,modbus_tcp_taskはlaninit_taskから起動
const T_CTSK ctsk_001 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)timer_task,       2, 1024, NULL, (B*)"timer_task"      };
const T_CTSK ctsk_002 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)idle_task,        9, 1024, NULL, (B*)"idle_task"       };
const T_CTSK ctsk_003 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)rs485_task,       4, 1024, NULL, (B*)"rs485_task"      };
const T_CTSK ctsk_004 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)modbus_rtu_task,  4, 1024, NULL, (B*)"modbus_rtu_task" };
const T_CTSK ctsk_005 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)lanini_task,      3, 1024, NULL, (B*)"lanini_task"     };
const T_CTSK ctsk_006 = { TA_HLNG,          (VP_INT)0,(FP)lancli_udp_task,  4, 1024, NULL, (B*)"lancli_udp_task" };
const T_CTSK ctsk_007 = { TA_HLNG,          (VP_INT)0,(FP)modbus_tcp_task,  4, 1024, NULL, (B*)"modbus_tcp_task" };
const T_CTSK ctsk_008 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)di_task,          4, 1024, NULL, (B*)"di_task"         };
const T_CTSK ctsk_009 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)main_task,        5, 1024, NULL, (B*)"main_task"       };
const T_CTSK ctsk_010 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)ai_task,          4, 1024, NULL, (B*)"ai_task"         };
const T_CTSK ctsk_011 = { TA_HLNG | TA_ACT, (VP_INT)0,(FP)can_task,         4, 1024, NULL, (B*)"can_task"        };

/* イベントフラグ定義 */
const T_CFLG cflg_001 = { TA_WSGL,0,(B*)"flg_timer"       };
const T_CFLG cflg_002 = { TA_WSGL,0,(B*)"flg_rs485"       };
const T_CFLG cflg_003 = { TA_WSGL,0,(B*)"flg_modbus"      };
const T_CFLG cflg_004 = { TA_WSGL,0,(B*)"flg_di"          };
const T_CFLG cflg_005 = { TA_WSGL,0,(B*)"flg_lancli_udp"  };
const T_CFLG cflg_006 = { TA_WSGL,0,(B*)"flg_lancli_tcp"  };
const T_CFLG cflg_007 = { TA_WSGL,0,(B*)"flg_main"        };
const T_CFLG cflg_008 = { TA_WSGL,0,(B*)"flg_ai"          };
const T_CFLG cflg_009 = { TA_WSGL,0,(B*)"flg_can"         };

/*****************************************************************************
* Main Function (Initialize Handler for NORTi)
*
******************************************************************************/

int main(void)
{
	/* initialize system */
	sysini();
	ini_sci();								//NORTiシリアル通信ドライバ初期設定
	
	/* タスク作成 */
	cre_tsk(ID_TIMER_TSK,      &ctsk_001);
	cre_tsk(ID_IDLE_TSK,       &ctsk_002);
	cre_tsk(ID_RS485_TSK,      &ctsk_003);
	cre_tsk(ID_MODBUS_TSK,     &ctsk_004);
	cre_tsk(ID_LANINI_TSK,     &ctsk_005);
	cre_tsk(ID_LANCLI_UDP_TSK, &ctsk_006);
	cre_tsk(ID_LANCLI_TCP_TSK, &ctsk_007);
	cre_tsk(ID_DI_TSK,         &ctsk_008);
	cre_tsk(ID_MAIN_TSK,       &ctsk_009);
	cre_tsk(ID_AI_TSK,         &ctsk_010);
	cre_tsk(ID_CAN_TSK,        &ctsk_011);
	
	/* イベントフラグ定義 */
	cre_flg(ID_TIMER_FLG,      &cflg_001);
	cre_flg(ID_RS485_FLG,      &cflg_002);
	cre_flg(ID_MODBUS_FLG,     &cflg_003);
	cre_flg(ID_DI_FLG,         &cflg_004);
	cre_flg(ID_LANCLI_UDP_FLG, &cflg_005);
	cre_flg(ID_LANCLI_TCP_FLG, &cflg_006);
	cre_flg(ID_MAIN_FLG,       &cflg_007);
	cre_flg(ID_AI_FLG,         &cflg_008);
	cre_flg(ID_CAN_FLG,        &cflg_009);
	
	/* start system */
	intsta();							/* start interval timer interrupt */
	syssta();							/* enter into multitasking */
}
/* end */
