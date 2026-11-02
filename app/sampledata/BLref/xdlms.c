/****************************************************************************/
/*                                                                          */
/*  システム名      :   DLMS                                                */
/*                                                                          */
/*  サブシステム名  :   XDLMS                                               */
/*                                                                          */
/*  内容            :   データ処理(A-XDR)                                   */
/*                                                                          */
/*  ファイル名      :   xdlms.c                                             */
/*                                                                          */
/*                                                                          */
/*  変更履歴        :                                                       */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*					ユーザー・インクルード・ファイル定義					*/
/****************************************************************************/
#include "def.h"				/* define宣言					*/
#include "data.h"				/* define宣言					*/
#include "devpwr.h"				/* define宣言					*/
#include "hrdtushin.h"			/* define宣言					*/
#include "hrdrtc.h"				/* define宣言				*/
#include "aplstsctl.h"			/* define宣言					*/
#include "aplkeikiinf.h"		/* define宣言					*/
#include "apleeptask.h"			/* define宣言					*/
#include "apllcddsp.h"			/* define宣言					*/
#include "aplevent.h"			/* define宣言					*/
#include "apltokei.h"			/* define宣言					*/
#include "aplrecord.h"			/* define宣言					*/
#include "aplkeiryo.h"			/* define宣言					*/
#include "ram.h"				/* define宣言					*/
#include "rom.h"				/* define宣言					*/
#include "sub.h"

#include "dlms.h"				/* define宣言					*/
#include "conf.h"				/* define宣言					*/
#include "acse.h"				/* define宣言					*/
#include "aes_gcm.h"			/* define宣言					*/
#include "xdlms.h"				/* define宣言					*/
#include "xdlms_map.h"			/* define宣言					*/
#include "flash.h"				/* define宣言					*/

#define REJECT_EEP	0
#define CLOCK_STATUS_UPDATE_STATUS(x)				((x)&0x07)>0?((x)|0x08):((x)&0xf7)
#define CLOCK_STATUS_INVALID_VALUE(x,y)				(y)?((x)|0x01):((x)&0xfe)
#define CLOCK_STATUS_DOUBTFUL_VALUE(x,y)			(y)?((x)|0x02):((x)&0xfd)
#define CLOCK_STATUS_DIFFERENT_CLOCK_BASE(x,y)		(y)?((x)|0x04):((x)&0xfb)
#define CLOCK_STATUS_DAYLIGHT_SAVING_ACTIVE(x,y)	(y)?((x)|0x80):((x)&0x7f)

/*ビット置換*/
#define vBIT8(x) ((x)?0x80:0)
#define vBIT7(x) ((x)?0x40:0)
#define vBIT6(x) ((x)?0x20:0)
#define vBIT5(x) ((x)?0x10:0)
#define vBIT4(x) ((x)?0x08:0)
#define vBIT3(x) ((x)?0x04:0)
#define vBIT2(x) ((x)?0x02:0)
#define vBIT1(x) ((x)?0x01:0)

/*ビット取得*/
#define bBIT8(x) x&0x80?1:0
#define bBIT7(x) x&0x40?1:0
#define bBIT6(x) x&0x20?1:0
#define bBIT5(x) x&0x10?1:0
#define bBIT4(x) x&0x08?1:0
#define bBIT3(x) x&0x04?1:0
#define bBIT2(x) x&0x02?1:0
#define bBIT1(x) x&0x01?1:0

#define IDX_CLASS_DATA                     IST_BILLING_COUNT
#define IDX_CLASS_REGISTER                 IST_BILLING_DATE
#define IDX_CLASS_REGISTER_EX              IST_RSSI_BLOCK
#define IDX_CLASS_PROFILE_GENERIC          IST_NAME_PLATE
#define IDX_CLASS_CLOCK                    IST_DATETIME_NOW
#define IDX_CLASS_ASSOCIATION_LN           IST_ASSO_LN0
#define IDX_CLASS_HDLC_SETUP               IST_SETUP_HDLC0
#define IDX_CLASS_SECURITY_SETUP           IST_SECURITY_NONE

/*オブジェクトタイプ定義*/
#define TYP_00	0x00
#define TYP_08	0x01
#define TYP_16	0x02
#define TYP_24	0x03
#define TYP_32	0x04

#define CMP_MATCH		0
#define CMP_VALID		1
#define CMP_INVALID		DLMS_INVALID

/* MODE */
#define ORDER_NORMAL			0x01U
#define ORDER_BLOCK				0x02U
#define ORDER_WITH_LIST			0x03U

/*
(0) non-associated,
(1) association-pending,
(2) associated
*/
#define NON_ASSOCIATED			0x00U
#define ASSOCIATION_PENDING		0x01U
#define ASSOCIATED				0x02U

/*(0) global unicast encryption key,*/
/*(1) global broadcast encryption key,*/
/*(2) authentication key*/
#define global_unicast_encryption_key		0
#define global_broadcast_encryption_key		1
#define authentication_key					2
#define master_unicast_encryption_key		100
/*Logical_Name_Referencing_No_Ciphering ::= context_id(1) */
/*Short_Name_Referencing_No_Ciphering ::= context_id(2) */
/*Logical_Name_Referencing_With_Ciphering ::= context_id(3) */
/*Short_Name_Referencing_With_Ciphering ::= context_id(4) */

#pragma section COMM
/************************/
/* プロトタイプ関数宣言 */
/************************/
OBJMAP0(CP, PRO, PRO)
/* void get_cl1_ar2_serial_no( void ); */

/************************/
/*  Attribute宣言     */
/************************/
/* LIST(CP,ATR,CPY); */

/************************/
/* Attributeアクセス関数宣言 */
/************************/
OBJMAP0(CP, ACA, ACM)

/************************/
/******Method宣言*********/
/************************/
#if 1	/* ACCEPT_ALL */
/* クライアント0用 */
//OBJMAP0(MM0, CTA, CTM)
#endif	/* ACCEPT_ALL */
//#pragma section MM1
/* クライアント1用 */
OBJMAP1(MM1, CTA, CTM)
//#pragma section

/* クライアント2用 */
OBJMAP2(MM2, CTA, CTM)

/* クライアント2用 */
//OBJMAP3(MM3, CTA, CTM)
#pragma section

/****************************************************************************/
/*					Conformance (xDLMS kernel)定義							*/
/*		LN対応																*/
/*		GET/SET/ACTION対応													*/
/*		Block送信対応														*/
/*		SelectiveAccess対応													*/
/****************************************************************************/
#define conf_zero								0		/* bit  0 */ /* 未使用 */
#define	conf_general_protection					0		/* bit  1 */ /* 未使用 */
#define	conf_general_block_transfer				1		/* bit  2 */ /* 未使用 */
#define	conf_read								0		/* bit  3 */ /* 未使用 */
#define	conf_write								0		/* bit  4 */ /* 未使用 */
#define	conf_unconfirmed_write					0		/* bit  5 */ /* 未使用 */
#define	conf_delta_value_encoding				0		/* bit  6 */ /* 未使用 */
#define	conf_seven								0		/* bit  7 */ /* 未使用 */
#define	conf_attribute0_supported_with_set		0		/* bit  8 */ /* 未使用 */
#define	conf_priority_mgmt_supported			0		/* bit  9 */ /* 未使用 */
#define	conf_attribute0_supported_with_get		0		/* bit 10 */ /* attr0 対応 */
#define	conf_block_transfer_with_get_or_read	1		/* bit 11 */ /* Block送信有効 */
#define	conf_block_transfer_with_set_or_write	1		/* bit 12 */ /* 未使用 */
#define	conf_block_transfer_with_action			0		/* bit 13 */ /* 未使用 */
#define	conf_multiple_references				0		/* bit 14 */ /* 未使用 */
#define	conf_information_report					0		/* bit 15 */ /* 未使用 */
#define conf_data_notification					1		/* bit 16 */ /* 未使用 */
#define conf_access								0		/* bit 17 */ /* 未使用 */
#define	conf_parameterized_access				0		/* bit 18 */ /* 未使用 */
#define	conf_get								1		/* bit 19 */ /* Get有効(LN用) */
#define	conf_set								1		/* bit 20 */ /* Set有効(LN用) */
#define	conf_selective_access					1		/* bit 21 */ /* SelectiveAccess有効 */
#define	conf_event_notification					1		/* bit 22 */ /* 未使用 */
#define	conf_action								1		/* bit 23 */ /* Action有効(LN用) */
#define MaxPduSize0 ((MAX_XDLMS_RAW&0xff00)>>8)			/* Max Info field size in tsushin Frame */
#define MaxPduSize1 ((MAX_XDLMS_RAW&0xff))				/* Max Info field size in tsushin Frame */

/************************Conformance 生成 *****************************/
/* 1バイト目 */
#define sConformance0 0x00
/* 2バイト目 */
#define sConformance1 \
vBIT8(conf_zero) + \
vBIT7(conf_general_protection) + \
vBIT6(conf_general_block_transfer) + \
vBIT5(conf_read) + \
vBIT4(conf_write) + \
vBIT3(conf_unconfirmed_write) + \
vBIT2(conf_delta_value_encoding) + \
vBIT1(conf_seven)

/* 3バイト目 */
#define sConformance2 \
vBIT8(conf_attribute0_supported_with_set) + \
vBIT7(conf_priority_mgmt_supported) +\
vBIT6(conf_attribute0_supported_with_get) +\
vBIT5(conf_block_transfer_with_get_or_read) +\
vBIT4(conf_block_transfer_with_set_or_write) +\
vBIT3(conf_block_transfer_with_action) +\
vBIT2(conf_multiple_references) +\
vBIT1(conf_information_report)

/* 4バイト目 */
#define sConformance3 \
vBIT8(conf_data_notification) +\
vBIT7(conf_access) +\
vBIT6(conf_parameterized_access) +\
vBIT5(conf_get) +\
vBIT4(conf_set) +\
vBIT3(conf_selective_access) +\
vBIT2(conf_event_notification) +\
vBIT1(conf_action)


/****************************/
/* Application Context Name */
/****************************/
cui08 application_name[][LEN_CONTEXT_NAME + 1] = {
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x00, 0x00 },		/* 未定義 */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0x00 },		/* LN_NO_CIPHER */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x02, 0x00 },		/* SN_NO_CIPHER */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x03, 0x00 },		/* LN_CIPHER */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x04, 0x00 }		/* SN_CIPHER */
};

/****************************/
/* Mechanism name */
/****************************/
cui08 mechanism_name[][LEN_CONTEXT_NAME + 1] = {
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x00, 0x00 },/* MECHA_NONE */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x01, 0x00 },/* MECHA_LLS */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x02, 0x00 },/* MECHA_HLS */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x03, 0x00 },/* MECHA_HLS_MD5 */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x04, 0x00 },/* MECHA_HLS_SHA1 */
	{ 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x05, 0x00 }/* MECHA_HLS_GMAC */
};


static cui08f CONFORMANCE[][4] = {
	{ 0x00, 0xff, 0xff, 0xff },
	{ 0x00, 0x20, 0x18, 0x9d },/*US*/
	{ 0x00, 0x00, 0x00, 0x19 },/*FW*/
	{ 0x00, 0x00, 0x10, 0x14 },/*MR*/
	{ 0x00, 0x60, 0x10, 0x90 },/*PUSH*/
	{ 0x00, 0x40, 0x10, 0x98 },/*IHD*/
	{ 0x00, 0x00, 0x00, 0x10 },/*PC*/
	{ 0x00, 0x00, 0x00, 0x10 }/*PC*/
};
const ui08 CTX[][28] = {
	{ 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x10, 0xE2, 0x02, 0xE2, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x10, 0x14, 0xE2, 0x02, 0xE2, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x20, 0x18, 0x9d, 0xE2, 0x02, 0xE2, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x60, 0x10, 0x90, 0xE2, 0x02, 0xE2, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x19, 0xE2, 0x02, 0xE2, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
};

/************************/
/* INITIATE_RESPONSE 定義 */
/************************/
static cui08f INITIATE_RESPONSE[LEN_INITIATE_RESPONSE + 1] = {
	0x00,			/* 0:quality_of_service  (Fixed) */
	0x06,			/* 1:dlms_version_number (Fixed) */
	0x5F,			/* 2:Conformance Apl tag (Fixed) */
	0x1F,			/* 3:Conformance Apl tag (Fixed) */
	0x04,			/* 4:Length              (Fixed) */
	0x00,			/* 5:sConformance0    (Variable) */
	0x00,			/* 6:sConformance1    (Variable) */
	0x00,			/* 7:sConformance2    (Variable) */
	0x00,			/* 8:sConformance3    (Variable) */
	MaxPduSize0,	/* 9:max_receive_pdu_size */
	MaxPduSize1,	/* 10:                 (Variable) */
	0x00,			/* 11:vaa_name           (Fixed) */
	0x07,			/* 12:vaa_name           (Fixed) */
	0x00			/* padding */
};

ui08	I(1, 2, firm_sig)[24];
ui08	obis_code[6];		/* */
ui16	idxobj, idxdat;
static	ui08	class_id[2];		/* */
static	ui08	I(23, 2, setup_hdlc0);		/* comm_speed enum (6) 19200 */
static	ui08	I(23, 3, setup_hdlc0);		/* window_size_transmit */
static	ui08	I(23, 4, setup_hdlc0);		/* window_size_recieve */
static	ui08	I(23, 2, setup_hdlc1);		/* comm_speed enum (6) 19200 */
static	ui08	I(23, 3, setup_hdlc1);		/* window_size_transmit */
static	ui08	I(23, 4, setup_hdlc1);		/* window_size_recieve */

		ui16	I(23, 5, setup_hdlc0);		/* max_info_field_length_transmit */
		ui16	I(23, 6, setup_hdlc0);		/* max_info_field_length_receive */
static	ui16	I(23, 7, setup_hdlc0);		/* inter_octet_time_out */
static	ui16	I(23, 8, setup_hdlc0);		/* inactivity_time_out */
static	ui16	I(23, 9, setup_hdlc0);		/* device_address */
		ui16	I(23, 5, setup_hdlc1);		/* max_info_field_length_transmit */
		ui16	I(23, 6, setup_hdlc1);		/* max_info_field_length_receive */
static	ui16	I(23, 7, setup_hdlc1);		/* inter_octet_time_out */
static	ui16	I(23, 8, setup_hdlc1);		/* inactivity_time_out */
static	ui16	I(23, 9, setup_hdlc1);		/* device_address */

static	ui08	I(15, 3, asso_ln)[3];		/* associated_partners_id */
		ui08	I(15, 5, asso_ln)[LEN_XDLMS_CTX_INF];	/* xDLMS_context_info */
static	ui08	I(15, 8, asso_ln);			/* association status */

ui08	stream;
static	ui08	xdlms_phase;		/* */
static	ui08	result_xdlms;		/* xDLMS処理結果       */
static	ui08	last_block;			/* ラストブロック      */
static	ui08	execute;			/* */
static	ui08	selectiveAccess;	/* */

static	ui08	order_xdlms;		/* */
static	ui08	res_mode;			/* */
static	ui08	invoke_id_and_priority;			/* */
static	ui08	sel_mem;


static	ui08	I(64, 4, security)[8];		/* 4.client_system_title */
static	ui08	I(8, 2, datetime_now)[SZ_DATE_TIME];		/* 2. datetime */
static	ui08	SYSTEM_TITLE[8];			/* 5.server_system_title */
static	ui08	record_from[LEN_CL7_PRM1];	/* 行範囲指定:最大はDATETIMEサイズ+Tag */
static	ui08	record_to[LEN_CL7_PRM1];	/* 行範囲指定:最大はDATETIMEサイズ+Tag */
static	ui08	colm_from[LEN_CL7_PRM2];	/* 列範囲指定 */

static	ui08	colm_to[LEN_CL7_PRM2];		/* 列範囲指定 */
static	ui08	colm_sel[144];

static	ui16	class_from;			/* */
static	ui16	class_to;			/* */
static	ui16	profile;
static	ui16	restricting;
static	ui16	record_count;		/* 検索該当全件数 */

static	uint	in_offset;		/* 要求電文オフセット位置 */
static	uint	out_offset;		/* 応答電文オフセット位置 */
static	uint	axdr_max;			/* 要求電文データ長 */
		uint	object_list_pos;
		uint	apdu_size;
static	uint	in_size;
//static	uint	object_list_pos;
static	pui08	ixdlms;				/* 要求電文先頭ポインタ */
static	pui08	oxdlms;				/* 応答電文先頭ポインタ */

static	uint	sent_count;			/* */
static	uint	send_count;			/* */
static	uint	record_offset;		/* 応答電文オフセット位置 */
static	uint	record_i;			/* 応答電文オフセット位置 */
static	uint	record_j;			/* 応答電文オフセット位置 */
static	uint	cl7_idx;
static	uint	record_size;

static	pui08	I(15, 3, asso_ln0);	/* associated_partners_id */
static	pui08	I(15, 5, asso_ln0);	/* xDLMS_context_info */
static	pui08	I(15, 8, asso_ln0);	/* association status */
static	pui16	I(15, 9, asso_ln0);	/* security_setup_reference */
static	pui08	I(15, 6, asso_ln1);
static	pui08	I(15, 6, asso_ln0);
static	pui08	I(15, 6, asso_ln2);
static	pui08	I(15, 6, asso_ln3);
static	pui08	I(15, 6, asso_ln4);
static	pui08	I(15, 6, asso_ln5);
static	pui08	I(15, 6, asso_ln6);

static	pui08	map_order;			/* */
//static	ui32	block_no;			/* ブロック番号        */
static	ui32	block_no;			/* ブロック番号        */
static	PCAPDEF	capdef;
static	IDENT	cap_idt;
static	PADR	cap_lst;
static	ARG		optional_process;
static	PADR	as_ln0;
static	ACCESS	Access;
static	UI08R	phase_selective;
static	FNC		asso_ln03;
static	FNC		asso_ln04;
static	FNC		asso_ln05;
static	FNC		asso_ln06;
static	FNC		asso_ln07;
static	FNC		asso_ln08;
static	FNC		asso_ln09;
static	UI08R	thread_process;

static	VALUE	getset;
static	CADR	*cur_ident;
static	IDENT	put_ident;
static	ui16	cur_inst;
static	ui16	inactivity_timeout;		/* inactivity_time_out */
ui08	arr_pos;
ui08	array_count[10];

/*******************************************/
/************ interface class 64************/
/*******************************************/

#define OPENCLOSE_NONE		0x30U	/* 開閉機能無し */
#define OPENCLOSE_EXIST		0x31U	/* 開閉機能有り */
#define OPENCLOSE_CURRENT	0x32U	/* 開閉機能有り(電流制限付き) */
#define OPENCLOSE_TIMESW	0x33U	/* 開閉機能有り(タイムスイッチ付き) */
#define BATT_NONE			0x30U	/* 電池コード無し */
#define BATT_EXIST			0x31U	/* 電池コード有り */
#define ONE_WAY				0x30U	/* 双方向計量無し */
#define TWO_WAY				0x31U	/* 双方向計量有り */

/* モード用変数 */
/* 1. Request-normal->response-normal */
/* 2. Request-normal->response-oneblock */
/* 3. Request-next->response-oneblock */
/* 4. Request-with-list->response-withlist */
/* 5. Request-with-list->response-oneblock */
/*
任意のモードから、one-blockモードに切り替える条件
・取得するデータサイズが、MAX_PDUサイズより大きい場合
　(MAX_PDUサイズは、サーバー、クライアントのどちらか小さい方のサイズ。)
 ・取得するデータがResponseのタイムアウト内に収まらない場合
 */

void Execute(cui08 sel, cui16 ist, cui08 attr);

/* アクセス関数、Method関数の位置リスト(32ビットOBJ用) */
cui08 POS_OBJ[] = OFFSET(OFFSET_OBJ);
/* GET関数インデックスリスト */
cui08 FG[8] = { 0x01U, 0x03U, 0x05U, 0x07U, 0x09U, 0x0bU, 0x0dU, 0x00U };
/* SET関数インデックスリスト */
cui08 FS[8] = { 0x02U, 0x04U, 0x06U, 0x08U, 0x0aU, 0x0cU, 0x0eU, 0x00U };

/************************/
/*
クラス定義
クラスID、バージョン、OBJECT型、Attributeの数、Methodの数を宣言する。
クラスIDが同じであっても、Attributeの数、Methodの数が異なれば定義する。
*/
/************************/
#define CL_SKIP	0
#define CL_DAT	1
#define CL_REG0	2
#define CL_REG1	3
#define CL_EXR0	4
#define CL_EXR1	5
#define CL_PRG0	6
#define CL_PRG1	7
#define CL_PRG2	8
#define CL_CLK	9
#define CL_SCR	10
#define CL_SPD	11
#define CL_ASL0	12
#define CL_ASL1	13
#define CL_IMG	14
#define CL_OPT	15
#define CL_CAL	16
#define CL_SNG	17
#define CL_HDL	18
#define CL_PSH  19
#define CL_TCP  20
#define CL_GPRS 21
#define CL_IPV6 22
#define CL_SEC0	23
#define CL_SEC1	24
#define CL_DIS	25
#define CL_LMT	26
#define CL_END	(CL_SEC1+1)
#pragma section COMM
CONST CLASS g_cls[] = {
	0x0000U,0x00U,TYP_00,0,0,	/* 不正クラス */
	0x0001U,0x00U,TYP_16,1,0,	/* Data Class attr 1, method 0 */
	0x0003U,0x00U,TYP_16,2,0,	/* Register Class attr 2, method 0 */
	0x0003U,0x00U,TYP_16,2,1,	/* Register Class attr 2, method 1 */
	0x0004U,0x00U,TYP_16,4,0,	/* Extend Register Class attr 4, method 0 */
	0x0004U,0x00U,TYP_16,4,1,	/* Extend Register Class attr 4, method 1 */
	0x0007U,0x01U,TYP_16,7,0,	/* Profile Generic Class attr 7, method 0 */
	0x0007U,0x01U,TYP_16,7,1,	/* Profile Generic Class attr 7, method 1 */
	0x0007U,0x01U,TYP_16,7,2,	/* Profile Generic Class attr 7, method 1 */
	0x0008U,0x00U,TYP_16,8,0,	/* Clock Class attr 9, method 0 */
	0x0009U,0x00U,TYP_16,1,1,	/* Script Table Class attr 2, method 1 */
	0x000bU,0x01U,TYP_16,1,2,	/* Special Days attr 1, method 2 */
	0x000fU,0x01U,TYP_16,8,0,	/* Association LN attr 8, method 0 */
	0x000fU,0x01U,TYP_16,8,2,	/* Association LN attr 8, method 2 */
	0x0012U,0x00U,TYP_16,6,4,	/* IMG */
	0x0013U,0x01U,TYP_16,8,0,	/* OPT */
	0x0014U,0x00U,TYP_16,9,1,	/* Active calender */
	0x0016U,0x00U,TYP_16,3,0,	/* Single action */
	0x0017U,0x01U,TYP_16,8,0,	/* HDLC_SETUP */
	0x0028U,0x00U,TYP_16,6,1,	/* Push */
	0x0029U,0x00U,TYP_16,5,0,	/* TCP_UDP */
	0x002DU,0x00U,TYP_16,3,0,	/* GPRS */
	0x0030U,0x00U,TYP_16,3,0,	/* IPV6 */
	0x0040U,0x00U,TYP_16,4,0,	/* 64:Security Setup attr 4, method 0 */
	0x0040U,0x00U,TYP_16,4,2,	/* 64:Security Setup attr 4, method 2 */
	0x0046U,0x00U,TYP_16,3,2,	/* Disconnect control */
	0x0047U,0x00U,TYP_16,10,0	/* Load Limiter */
};

/************************/
/* OBIS-クラステーブル */
/* OBISコードのクラスを定義する。 */
/************************/
INST CONST g_ist[] = {
	CL_SKIP,{ 0x00,0x00,0x00,0x00,0x00},/* start marker */

CL_DAT,{0x00,0x00,0x00,0x01,0x00},/* 1: Cumulative Billing count */
CL_DAT,{0x00,0x00,0x00,0x01,0x01},/* 2: Available Billing Period  */
CL_DAT,{0x00,0x00,0x00,0x09,0x01},/* 3: Time */
CL_DAT,{0x00,0x00,0x00,0x09,0x02},/* 4: Date */
CL_DAT,{0x00,0x00,0x02,0x80,0x00},/* 5: NIC SIM ID */
CL_DAT,{0x00,0x00,0x2a,0x00,0x00},/* 6: COSEM logical device name  */
CL_DAT,{0x00,0x00,0x2b,0x01,0x00},/* 7: Frame counter */
CL_DAT,{0x00,0x00,0x2b,0x01,0x01},/* 8: Frame counter1 */
CL_DAT,{0x00,0x00,0x2b,0x01,0x02},/* 9: Frame counter2 */
CL_DAT,{0x00,0x00,0x2b,0x01,0x03},/* 10: Frame counter3 */
CL_DAT,{0x00,0x00,0x2b,0x01,0x04},/* 11: Frame counter4 */
CL_DAT,{0x00,0x00,0x2b,0x01,0x05},/* 12: Frame counter5 */
CL_DAT,{0x00,0x00,0x2b,0x01,0x06},/* 13: Frame counter6 */
CL_DAT,{0x00,0x00,0x5e,0x5b,0x00},/* 14: Cumulative tamper count */
CL_DAT,{0x00,0x00,0x5e,0x5b,0x09},/* 15: Meter Type (1Phase/3P-3W/3P-4W) */
CL_DAT,{0x00,0x00,0x5e,0x5b,0x0b},/* 16: Category */
CL_DAT,{0x00,0x00,0x5e,0x5b,0x0c},/* 17: Current rating */
CL_DAT,{0x00,0x00,0x5e,0x5b,0x12},/* 18: Event status word */
CL_DAT,{0x00,0x00,0x5e,0x5b,0x1a},/* 19: Event status word filter */
CL_DAT,{0x00,0x00,0x5e,0x60,0x13},/* 20: Metering mode */
CL_DAT,{0x00,0x00,0x5e,0x60,0x14},/* 21: Payment mode */
CL_DAT,{0x00,0x00,0x5e,0x60,0x15},/* 22: Last token recharge amount */
CL_DAT,{0x00,0x00,0x5e,0x60,0x16},/* 23: Last token recharge time  */
CL_DAT,{0x00,0x00,0x5e,0x60,0x17},/* 24: Total amount at last recharge  */
CL_DAT,{0x00,0x00,0x5e,0x60,0x18},/* 25: Current balance amount */
CL_DAT,{0x00,0x00,0x5e,0x60,0x19},/* 26: Current balance time */
CL_DAT,{0x00,0x00,0x60,0x01,0x00},/* 27: Meter serial number */
CL_DAT,{0x00,0x00,0x60,0x01,0x01},/* 28: Manufacturer name */
CL_DAT,{0x00,0x00,0x60,0x01,0x02},/* 29: Device ID */
CL_DAT,{0x00,0x00,0x60,0x01,0x04},/* 30: Meter year of manufacture */
CL_DAT,{0x00,0x00,0x60,0x01,0x05},/* 31: NIC IMEI number (serial number) */
CL_DAT,{0x00,0x00,0x60,0x01,0x06},/* 32: NIC Year of manufacture */
CL_DAT,{0x00,0x00,0x60,0x01,0x07},/* 33: NIC serial ID */
CL_DAT,{0x00,0x00,0x60,0x01,0x09},/* 34: Bluetooth mac address */
CL_DAT,{0x00,0x00,0x60,0x01,0xc0},/* 35: NIC Firmware Version */
CL_DAT,{0x00,0x00,0x60,0x02,0x00},/* 36: Cumulative program count */
CL_DAT,{0x00,0x00,0x60,0x07,0x00},/* 37: Number of power failures */
CL_DAT,{0x00,0x00,0x60,0x0a,0x01},/* 38: NIC status  */
CL_DAT,{0x00,0x00,0x60,0x0b,0x00},/* 39: Event1 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x01},/* 40: Event2 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x02},/* 41: Event3 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x03},/* 42: Event4 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x04},/* 43: Event5 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x05},/* 44: Event6 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x06},/* 45: Event7 */
CL_DAT,{0x00,0x00,0x60,0x0b,0x0a},/* 46: Meter log code */
CL_DAT,{0x00,0x00,0x60,0x0b,0x0b},/* 47: Meter status */
CL_DAT,{0x00,0x00,0x60,0x0f,0x00},/* 48: Event1-recordNO */
CL_DAT,{0x00,0x00,0x60,0x0f,0x01},/* 49: Event2-recordNO */
CL_DAT,{0x00,0x00,0x60,0x0f,0x02},/* 50: Event3-recordNO */
CL_DAT,{0x00,0x00,0x60,0x0f,0x03},/* 51: Event4-recordNO */
CL_DAT,{0x00,0x00,0x60,0x0f,0x04},/* 52: Event5-recordNO */
CL_DAT,{0x00,0x00,0x60,0x0f,0x05},/* 53: Event6-recordNO */
CL_DAT,{0x00,0x00,0x60,0x0f,0x06},/* 54: Event7-recordNO */
CL_DAT,{0x00,0x00,0x60,0x32,0x01},/* 55: Relay Count */
CL_DAT,{0x00,0x00,0x60,0x39,0x26},/* 56: Cell ID */
CL_DAT,{0x00,0x00,0x60,0x63,0x01},/* 57: 4G&NB fallback to GSM parameter */
CL_DAT,{0x00,0x00,0x60,0x63,0x7c},/* 58: RAT & Band */
CL_DAT,{0x00,0x00,0x61,0x03,0x02},/* 59: Event stamping parameter for RC/DC */
CL_DAT,{0x00,0x00,0x80,0x00,0x00},/* 60: RAM */
CL_DAT,{0x00,0x00,0x80,0x00,0x01},/* 61: EEPROM */
CL_DAT,{0x00,0x00,0x80,0x00,0x02},/* 62: ROM */
CL_DAT,{0x00,0x00,0x81,0x00,0x00},/* 63: Energy calibration */
CL_DAT,{0x00,0x00,0x81,0x00,0x01},/* 64: Read energy(BF)　 */
CL_DAT,{0x00,0x00,0x82,0x00,0x00},/* 65: Voltage and ampere calibration */
CL_DAT,{0x00,0x00,0x82,0x00,0x01},/* 66: Read volt and ampere(D3)　 */
CL_DAT,{0x00,0x00,0x83,0x00,0x00},/* 67: RTC calibration */
CL_DAT,{0x00,0x00,0x84,0x00,0x01},/* 68: BuildNO */
CL_DAT,{0x00,0x00,0x84,0x00,0x02},/* 69: BildDate */
CL_DAT,{0x00,0x00,0x85,0x00,0x00},/* 70: Setting bit array */
CL_DAT,{0x00,0x00,0x86,0x00,0x00},/* 71: Unit setting */
CL_DAT,{0x00,0x00,0x86,0x00,0x01},/* 72: Display time */
CL_DAT,{0x00,0x00,0x87,0x00,0x00},/* 73: Record setting */
CL_DAT,{0x00,0x00,0x88,0x00,0x00},/* 74: Power quality code define */
CL_DAT,{0x00,0x00,0x89,0x00,0x00},/* 75: Pulse setting */
CL_DAT,{0x00,0x00,0x8a,0x00,0x01},/* 76: Detect event setting */
CL_DAT,{0x00,0x00,0x90,0x00,0x00},/* 77: Server port */
CL_DAT,{0x00,0x00,0x91,0x00,0x00},/* 78: Cumulative event count */
CL_DAT,{0x00,0x00,0x92,0x00,0x00},/* 79: Cumulative tamper count 1month */
CL_DAT,{0x00,0x00,0x93,0x00,0x00},/* 80: Cumulative top cover count */
CL_DAT,{0x00,0x00,0x93,0x00,0x01},/* 81: Top cover tamper datetime */
CL_DAT,{0x00,0x00,0x94,0x00,0x00},/* 82: Cumulative reset count */
CL_DAT,{0x00,0x00,0x94,0x00,0x01},/* 83: Reset datetime */
CL_DAT,{0x00,0x01,0x90,0x0c,0x05},/* 84: NIC RSSI */
CL_DAT,{0x00,0x01,0x90,0x0c,0x06},/* 85: RSRP */
CL_DAT,{0x00,0x01,0x90,0x0c,0x07},/* 86: RSRQ */
CL_DAT,{0x00,0x01,0x90,0x0c,0x08},/* 87: SNR */
CL_DAT,{0x00,0x80,0x60,0x00,0x09},/* 88: NIC firmware version number  */
CL_DAT,{0x00,0x80,0x98,0x80,0x80},/* 89: Apparent energy computation type */
CL_DAT,{0x00,0x80,0xa2,0x0d,0x80},/* 90: LRCF */
CL_DAT,{0x01,0x00,0x00,0x00,0x00},/* 91: Meter type */
CL_DAT,{0x01,0x00,0x00,0x00,0x01},/* 92: Product model */
CL_DAT,{0x01,0x00,0x00,0x02,0x00},/* 93: Firmware version for meter */
CL_DAT,{0x01,0x00,0x00,0x02,0x01},/* 94: Active firmware version  */
CL_DAT,{0x01,0x00,0x00,0x02,0x08},/* 95: Active firmware signature  */
CL_DAT,{0x01,0x00,0x00,0x08,0x00},/* 96: Demand integration period */
CL_DAT,{0x01,0x00,0x00,0x08,0x04},/* 97: Capture period-Block load profile */
CL_DAT,{0x01,0x00,0x00,0x08,0x05},/* 98: Capture period-Daily load profile */
CL_DAT,{0x01,0x00,0x09,0x08,0xfa},/* 99: Apparent energy computation type */
CL_DAT,{0x01,0x00,0x60,0x05,0x00},/* 100: Self diagnostic details */
CL_DAT,{0x01,0x00,0x60,0x80,0x19},/* 101: Current active relay time */
CL_DAT,{0x01,0x00,0x60,0x80,0x1e},/* 102: Active relay time setting */
CL_DAT,{0x01,0x00,0x95,0x00,0x00},/* 103: Cumulative magnetic count */
CL_DAT,{0x01,0x00,0x95,0x00,0x01},/* 104: Magnet tamper datetime */
CL_DAT,{0x01,0x00,0x96,0x00,0x00},/* 105: Cumulative esd count */
CL_DAT,{0x01,0x00,0x96,0x00,0x01},/* 106: ESD tamper datetime */
CL_DAT,{0x01,0x00,0x97,0x00,0x00},/* 107: Cumulative jammer count */
CL_DAT,{0x01,0x00,0x97,0x00,0x01},/* 108: Jammer tamper datetime */
CL_DAT,{0x01,0x00,0x98,0x00,0x00},/* 109: Cumulative vthd count */
CL_DAT,{0x01,0x00,0x98,0x00,0x01},/* 110: vthd tamper datetime */
CL_DAT,{0x01,0x00,0x99,0x00,0x00},/* 111: Cumulative athd count */
CL_DAT,{0x01,0x00,0x99,0x00,0x01},/* 112: athd tamper datetime */
CL_DAT,{0x01,0x00,0x9a,0x00,0x00},/* 113: Cumulative atdd count */
CL_DAT,{0x01,0x00,0x9a,0x00,0x01},/* 114: atdd tamper datetime */
CL_DAT,{0x01,0x00,0x9c,0x00,0x00},/* 115: Cumulative disconnect count */
CL_DAT,{0x01,0x00,0x9c,0x00,0x01},/* 116: Disconnect datetime */
CL_DAT,{0x01,0x00,0x9d,0x00,0x00},/* 117: Cumulative connect count */
CL_DAT,{0x01,0x00,0x9d,0x00,0x01},/* 118: Connect datetime */
CL_REG0,{0x00,0x00,0x00,0x01,0x02},/* 119: Billing date */
CL_REG1,{0x00,0x00,0x5e,0x5b,0x08},/* 120: Cumulative power off duration in min */
CL_REG1,{0x00,0x00,0x5e,0x5b,0x0d},/* 121: Billing power on duration in min */
CL_REG1,{0x00,0x00,0x5e,0x5b,0x0e},/* 122: Cumulative power on duration in min */
CL_REG1,{0x00,0x00,0x60,0x06,0x03},/* 123: Battery voltage */
CL_REG1,{0x00,0x00,0x90,0x08,0x00},/* 124: Operating time objects  */
CL_REG0,{0x00,0x00,0x90,0x09,0x80},/* 125: Temperature */
CL_REG1,{0x00,0x00,0x90,0x1d,0x00},/* 126: Temperature block */
CL_REG1,{0x00,0x00,0x90,0x1d,0xff},/* 127: Temperature block24 */
CL_REG0,{0x00,0x00,0xb0,0x00,0x00},/* 128: Magnet voltage */
CL_REG1,{0x00,0x00,0xb1,0x1d,0x00},/* 129: Temperature(CPU/MAG) block */
CL_REG1,{0x00,0x00,0xb1,0x1d,0xff},/* 130: Temperature(CPU/MAG) block24 */
CL_REG0,{0x01,0x00,0x01,0x02,0x00},/* 131: Cumulative kW MD Import  */
CL_REG0,{0x01,0x00,0x01,0x07,0x00},/* 132: Active power+- */
CL_REG0,{0x01,0x00,0x01,0x07,0x7c},/* 133: THD Power Phase  */
CL_REG1,{0x01,0x00,0x01,0x08,0x00},/* 134: Cumulative Energy+ Wh */
CL_REG1,{0x01,0x00,0x01,0x08,0x01},/* 135: Cumulative Energy+ Wh TZ1 */
CL_REG1,{0x01,0x00,0x01,0x08,0x02},/* 136: Cumulative Energy+ Wh TZ2 */
CL_REG1,{0x01,0x00,0x01,0x08,0x03},/* 137: Cumulative Energy+ Wh TZ3 */
CL_REG1,{0x01,0x00,0x01,0x08,0x04},/* 138: Cumulative Energy+ Wh TZ4 */
CL_REG1,{0x01,0x00,0x01,0x08,0x05},/* 139: Cumulative Energy+ Wh TZ5 */
CL_REG1,{0x01,0x00,0x01,0x08,0x06},/* 140: Cumulative Energy+ Wh TZ6 */
CL_REG1,{0x01,0x00,0x01,0x08,0x07},/* 141: Cumulative Energy+ Wh TZ7 */
CL_REG1,{0x01,0x00,0x01,0x08,0x08},/* 142: Cumulative Energy+ Wh TZ8 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x00},/* 143: Block Energy+ W */
CL_REG1,{0x01,0x00,0x01,0x1b,0x01},/* 144: Block Energy+ W TZ1 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x02},/* 145: Block Energy+ W TZ2 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x03},/* 146: Block Energy+ W TZ3 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x04},/* 147: Block Energy+ W TZ4 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x05},/* 148: Block Energy+ W TZ5 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x06},/* 149: Block Energy+ W TZ6 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x07},/* 150: Block Energy+ W TZ7 */
CL_REG1,{0x01,0x00,0x01,0x1b,0x08},/* 151: Block Energy+ W TZ8 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x00},/* 152: Block Energy+ Wh */
CL_REG1,{0x01,0x00,0x01,0x1d,0x01},/* 153: Block Energy+ Wh TZ1 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x02},/* 154: Block Energy+ Wh TZ2 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x03},/* 155: Block Energy+ Wh TZ3 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x04},/* 156: Block Energy+ Wh TZ4 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x05},/* 157: Block Energy+ Wh TZ5 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x06},/* 158: Block Energy+ Wh TZ6 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x07},/* 159: Block Energy+ Wh TZ7 */
CL_REG1,{0x01,0x00,0x01,0x1d,0x08},/* 160: Block Energy+ Wh TZ8 */
CL_REG0,{0x01,0x00,0x02,0x02,0x00},/* 161: Cumulative kW MD Export */
CL_REG1,{0x01,0x00,0x02,0x08,0x00},/* 162: Cumulative Energy- Wh */
CL_REG1,{0x01,0x00,0x02,0x08,0x01},/* 163: Cumulative Energy- Wh TZ1 */
CL_REG1,{0x01,0x00,0x02,0x08,0x02},/* 164: Cumulative Energy- Wh TZ2 */
CL_REG1,{0x01,0x00,0x02,0x08,0x03},/* 165: Cumulative Energy- Wh TZ3 */
CL_REG1,{0x01,0x00,0x02,0x08,0x04},/* 166: Cumulative Energy- Wh TZ4 */
CL_REG1,{0x01,0x00,0x02,0x08,0x05},/* 167: Cumulative Energy- Wh TZ5 */
CL_REG1,{0x01,0x00,0x02,0x08,0x06},/* 168: Cumulative Energy- Wh TZ6 */
CL_REG1,{0x01,0x00,0x02,0x08,0x07},/* 169: Cumulative Energy- Wh TZ7 */
CL_REG1,{0x01,0x00,0x02,0x08,0x08},/* 170: Cumulative Energy- Wh TZ8 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x00},/* 171: Block Energy- W */
CL_REG1,{0x01,0x00,0x02,0x1b,0x01},/* 172: Block Energy- W TZ1 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x02},/* 173: Block Energy- W TZ2 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x03},/* 174: Block Energy- W TZ3 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x04},/* 175: Block Energy- W TZ4 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x05},/* 176: Block Energy- W TZ5 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x06},/* 177: Block Energy- W TZ6 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x07},/* 178: Block Energy- W TZ7 */
CL_REG1,{0x01,0x00,0x02,0x1b,0x08},/* 179: Block Energy- W TZ8 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x00},/* 180: Block Energy- Wh */
CL_REG1,{0x01,0x00,0x02,0x1d,0x01},/* 181: Block Energy- Wh TZ1 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x02},/* 182: Block Energy- Wh TZ2 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x03},/* 183: Block Energy- Wh TZ3 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x04},/* 184: Block Energy- Wh TZ4 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x05},/* 185: Block Energy- Wh TZ5 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x06},/* 186: Block Energy- Wh TZ6 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x07},/* 187: Block Energy- Wh TZ7 */
CL_REG1,{0x01,0x00,0x02,0x1d,0x08},/* 188: Block Energy- Wh TZ8 */
CL_REG0,{0x01,0x00,0x03,0x07,0x00},/* 189: Reactive power+- */
CL_REG1,{0x01,0x00,0x03,0x08,0x00},/* 190: Cumulative Energy+ varh */
CL_REG1,{0x01,0x00,0x04,0x08,0x00},/* 191: Cumulative Energy- varh */
CL_REG1,{0x01,0x00,0x05,0x08,0x00},/* 192: Cumulative Energy kvarh (Q1) */
CL_REG1,{0x01,0x00,0x05,0x1d,0x00},/* 193: Block Energy+ kvarh */
CL_REG1,{0x01,0x00,0x06,0x08,0x00},/* 194: Cumulative Energy kvarh (Q2) */
CL_REG0,{0x01,0x00,0x06,0x1d,0x00},/* 195: Block Energy kvarh (Q2) */
CL_REG1,{0x01,0x00,0x07,0x08,0x00},/* 196: Cumulative Energy kvarh (Q3) */
CL_REG0,{0x01,0x00,0x07,0x1d,0x00},/* 197: Block Energy kvarh (Q3) */
CL_REG1,{0x01,0x00,0x08,0x08,0x00},/* 198: Cumulative Energy kvarh (Q4) */
CL_REG1,{0x01,0x00,0x08,0x1d,0x00},/* 199: Block Energy- kvarh */
CL_REG0,{0x01,0x00,0x09,0x02,0x00},/* 200: Cumulative kVA MD Import  */
CL_REG1,{0x01,0x00,0x09,0x07,0x00},/* 201: Apparent power kVA+- */
CL_REG1,{0x01,0x00,0x09,0x08,0x00},/* 202: Cumulative Energy+ kVAh */
CL_REG1,{0x01,0x00,0x09,0x08,0x01},/* 203: Cumulative Energy+ kVAh TZ1 */
CL_REG1,{0x01,0x00,0x09,0x08,0x02},/* 204: Cumulative Energy+ kVAh TZ2 */
CL_REG1,{0x01,0x00,0x09,0x08,0x03},/* 205: Cumulative Energy+ kVAh TZ3 */
CL_REG1,{0x01,0x00,0x09,0x08,0x04},/* 206: Cumulative Energy+ kVAh TZ4 */
CL_REG1,{0x01,0x00,0x09,0x08,0x05},/* 207: Cumulative Energy+ kVAh TZ5 */
CL_REG1,{0x01,0x00,0x09,0x08,0x06},/* 208: Cumulative Energy+ kVAh TZ6 */
CL_REG1,{0x01,0x00,0x09,0x08,0x07},/* 209: Cumulative Energy+ kVAh TZ7 */
CL_REG1,{0x01,0x00,0x09,0x08,0x08},/* 210: Cumulative Energy+ kVAh TZ8 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x00},/* 211: Block Energy+ VA */
CL_REG1,{0x01,0x00,0x09,0x1b,0x01},/* 212: Block Energy+ VA TZ1 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x02},/* 213: Block Energy+ VA TZ2 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x03},/* 214: Block Energy+ VA TZ3 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x04},/* 215: Block Energy+ VA TZ4 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x05},/* 216: Block Energy+ VA TZ5 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x06},/* 217: Block Energy+ VA TZ6 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x07},/* 218: Block Energy+ VA TZ7 */
CL_REG1,{0x01,0x00,0x09,0x1b,0x08},/* 219: Block Energy+ VA TZ8 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x00},/* 220: Block Energy+ VAh */
CL_REG1,{0x01,0x00,0x09,0x1d,0x01},/* 221: Block Energy+ VAh TZ1 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x02},/* 222: Block Energy+ VAh TZ2 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x03},/* 223: Block Energy+ VAh TZ3 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x04},/* 224: Block Energy+ VAh TZ4 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x05},/* 225: Block Energy+ VAh TZ5 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x06},/* 226: Block Energy+ VAh TZ6 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x07},/* 227: Block Energy+ VAh TZ7 */
CL_REG1,{0x01,0x00,0x09,0x1d,0x08},/* 228: Block Energy+ VAh TZ8 */
CL_REG0,{0x01,0x00,0x0a,0x02,0x00},/* 229: Cumulative kVA MD Export */
CL_REG1,{0x01,0x00,0x0a,0x08,0x00},/* 230: Cumulative Energy- kVAh */
CL_REG1,{0x01,0x00,0x0a,0x08,0x01},/* 231: Cumulative Energy- kVAh TZ1 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x02},/* 232: Cumulative Energy- kVAh TZ2 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x03},/* 233: Cumulative Energy- kVAh TZ3 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x04},/* 234: Cumulative Energy- kVAh TZ4 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x05},/* 235: Cumulative Energy- kVAh TZ5 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x06},/* 236: Cumulative Energy- kVAh TZ6 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x07},/* 237: Cumulative Energy- kVAh TZ7 */
CL_REG1,{0x01,0x00,0x0a,0x08,0x08},/* 238: Cumulative Energy- kVAh TZ8 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x00},/* 239: Block Energy- VA */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x01},/* 240: Block Energy- VA TZ1 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x02},/* 241: Block Energy- VA TZ2 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x03},/* 242: Block Energy- VA TZ3 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x04},/* 243: Block Energy- VA TZ4 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x05},/* 244: Block Energy- VA TZ5 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x06},/* 245: Block Energy- VA TZ6 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x07},/* 246: Block Energy- VA TZ7 */
CL_REG1,{0x01,0x00,0x0a,0x1b,0x08},/* 247: Block Energy- VA TZ8 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x00},/* 248: Block Energy- VAh */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x01},/* 249: Block Energy- VAh TZ1 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x02},/* 250: Block Energy- VAh TZ2 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x03},/* 251: Block Energy- VAh TZ3 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x04},/* 252: Block Energy- VAh TZ4 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x05},/* 253: Block Energy- VAh TZ5 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x06},/* 254: Block Energy- VAh TZ6 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x07},/* 255: Block Energy- VAh TZ7 */
CL_REG1,{0x01,0x00,0x0a,0x1d,0x08},/* 256: Block Energy- VAh TZ8 */
CL_REG0,{0x01,0x00,0x0b,0x07,0x00},/* 257: Phase current */
CL_REG0,{0x01,0x00,0x0b,0x07,0x7c},/* 258: THD Current  */
CL_REG1,{0x01,0x00,0x0b,0x1b,0x00},/* 259: Block phase current */
CL_REG1,{0x01,0x00,0x0c,0x04,0x00},/* 260: Voltage0 Ave 1min value  */
CL_REG0,{0x01,0x00,0x0c,0x07,0x00},/* 261: Voltage */
CL_REG0,{0x01,0x00,0x0c,0x07,0x7c},/* 262: THD Voltage  */
CL_REG1,{0x01,0x00,0x0c,0x1b,0x00},/* 263: Block voltage */
CL_REG1,{0x01,0x00,0x0c,0x80,0x7d},/* 264: Block voltage THD */
CL_REG1,{0x01,0x00,0x0d,0x00,0x00},/* 265: Average power factor for billing period */
CL_REG0,{0x01,0x00,0x0d,0x00,0x01},/* 266: System Imp PF T1  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x02},/* 267: System Imp PF T2  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x03},/* 268: System Imp PF T3  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x04},/* 269: System Imp PF T4  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x05},/* 270: System Imp PF T5  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x06},/* 271: System Imp PF T6  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x07},/* 272: System Imp PF T7  */
CL_REG0,{0x01,0x00,0x0d,0x00,0x08},/* 273: System Imp PF T8  */
CL_REG0,{0x01,0x00,0x0d,0x07,0x00},/* 274: Signed power factor/Three phase power factor */
CL_REG1,{0x01,0x00,0x0d,0x1b,0x00},/* 275: Signed power factor/Three phase power factor */
CL_REG1,{0x01,0x00,0x0d,0x1b,0xff},/* 276: Signed power factor/Three phase power factor */
CL_REG1,{0x01,0x00,0x0d,0x1c,0x00},/* 277: Power factor imp day */
CL_REG0,{0x01,0x00,0x0e,0x07,0x00},/* 278: Frequency */
CL_REG1,{0x01,0x00,0x0f,0x08,0x00},/* 279: ABS Wh */
CL_REG1,{0x01,0x00,0x10,0x08,0x00},/* 280: NET Wh */
CL_REG1,{0x01,0x00,0x11,0x08,0x00},/* 281: ABS VarH */
CL_REG1,{0x01,0x00,0x12,0x08,0x00},/* 282: NET VarH */
CL_REG0,{0x01,0x00,0x1f,0x07,0x00},/* 283: Current L1 */
CL_REG1,{0x01,0x00,0x1f,0x1b,0x00},/* 284: IR-block */
CL_REG1,{0x01,0x00,0x20,0x04,0x00},/* 285: Voltage L1 Ave 1min value  */
CL_REG0,{0x01,0x00,0x20,0x07,0x00},/* 286: Voltage L1 */
CL_REG1,{0x01,0x00,0x20,0x1b,0x00},/* 287: Voltage L1 Block */
CL_REG0,{0x01,0x00,0x21,0x07,0x00},/* 288: Signed power factorR */
CL_REG0,{0x01,0x00,0x33,0x07,0x00},/* 289: Current L2 */
CL_REG1,{0x01,0x00,0x33,0x1b,0x00},/* 290: IY-block */
CL_REG1,{0x01,0x00,0x34,0x04,0x00},/* 291: Voltage L2 Ave 1min value  */
CL_REG0,{0x01,0x00,0x34,0x07,0x00},/* 292: Voltage L2 */
CL_REG1,{0x01,0x00,0x34,0x1b,0x00},/* 293: Voltage L2 Block */
CL_REG0,{0x01,0x00,0x35,0x07,0x00},/* 294: Signed power factorY */
CL_REG0,{0x01,0x00,0x47,0x07,0x00},/* 295: Current L3 */
CL_REG1,{0x01,0x00,0x47,0x1b,0x00},/* 296: IB-block */
CL_REG1,{0x01,0x00,0x48,0x04,0x00},/* 297: Voltage L3 Ave 1min value  */
CL_REG0,{0x01,0x00,0x48,0x07,0x00},/* 298: Voltage L3 */
CL_REG1,{0x01,0x00,0x48,0x1b,0x00},/* 299: Voltage L3 Block */
CL_REG0,{0x01,0x00,0x49,0x07,0x00},/* 300: Signed power factorB */
CL_REG0,{0x01,0x00,0x54,0x00,0x00},/* 301: Power factor exp month */
CL_REG0,{0x01,0x00,0x54,0x00,0x01},/* 302: System Exp PF T1  */
CL_REG0,{0x01,0x00,0x54,0x00,0x02},/* 303: System Exp PF T2  */
CL_REG0,{0x01,0x00,0x54,0x00,0x03},/* 304: System Exp PF T3  */
CL_REG0,{0x01,0x00,0x54,0x00,0x04},/* 305: System Exp PF T4  */
CL_REG0,{0x01,0x00,0x54,0x00,0x05},/* 306: System Exp PF T5  */
CL_REG0,{0x01,0x00,0x54,0x00,0x06},/* 307: System Exp PF T6  */
CL_REG0,{0x01,0x00,0x54,0x00,0x07},/* 308: System Exp PF T7  */
CL_REG0,{0x01,0x00,0x54,0x00,0x08},/* 309: System Exp PF T8  */
CL_REG1,{0x01,0x00,0x54,0x1b,0x00},/* 310: Power factor exp block  */
CL_REG1,{0x01,0x00,0x54,0x1c,0x00},/* 311: Power factor exp day */
CL_REG0,{0x01,0x00,0x5b,0x07,0x00},/* 312: Neutral current */
CL_REG1,{0x01,0x00,0x5b,0x1b,0x00},/* 313: Block neutral current */
CL_REG1,{0x01,0x00,0x5b,0x1d,0xff},/* 314: Average24 neutral current */
CL_REG0,{0x01,0x00,0x5e,0x5b,0x0e},/* 315: Current */
CL_REG0,{0x01,0x00,0x80,0x08,0x00},/* 316: Defraud Kwh */
CL_REG0,{0x01,0x00,0x80,0x08,0x71},/* 317: Temperature(CPU/MAG) */
CL_REG0,{0x01,0x00,0x80,0x1b,0x00},/* 318: Current-Avearage? */
CL_REG0,{0x01,0x00,0x81,0x08,0x00},/* 319: Fundamental Energy kWh */
CL_REG1,{0x01,0x00,0x81,0x1d,0x00},/* 320: Block current THD */
CL_REG1,{0x01,0x00,0x81,0x1d,0xff},/* 321: Average24 current THD */
CL_REG0,{0x01,0x00,0x82,0x00,0x00},/* 322: Current TDD */
CL_REG1,{0x01,0x00,0x82,0x1d,0x00},/* 323: Block current TDD */
CL_REG1,{0x01,0x00,0x82,0x1d,0xff},/* 324: Average24 current TDD */
CL_REG0,{0x01,0x00,0x9f,0x07,0x01},/* 325: Frequency L1  */
CL_REG0,{0x01,0x00,0x9f,0x07,0x02},/* 326: Frequency L2 */
CL_REG0,{0x01,0x00,0x9f,0x07,0x03},/* 327: Frequency L3 */
CL_REG1,{0x01,0x00,0xa1,0x1b,0xff},/* 328: Block24 Energy+ W */
CL_REG1,{0x01,0x00,0xa1,0x1d,0xff},/* 329: Block24 Energy+ Wh */
CL_REG1,{0x01,0x00,0xa2,0x1b,0xff},/* 330: Block24 Energy- W */
CL_REG1,{0x01,0x00,0xa2,0x1d,0xff},/* 331: Block24 Energy- Wh */
CL_REG1,{0x01,0x00,0xa5,0x1d,0xff},/* 332: Block24 Energy+ kvarh */
CL_REG1,{0x01,0x00,0xa8,0x1d,0xff},/* 333: Block24 Energy- kvarh */
CL_REG1,{0x01,0x00,0xa9,0x1b,0xff},/* 334: Block24 Energy+ VA */
CL_REG1,{0x01,0x00,0xa9,0x1d,0xff},/* 335: Block24 Energy+ VAh */
CL_REG1,{0x01,0x00,0xaa,0x1b,0xff},/* 336: Block24 Energy- VA */
CL_REG1,{0x01,0x00,0xaa,0x1d,0xff},/* 337: Block24 Energy- VAh */
CL_REG1,{0x01,0x00,0xab,0x1b,0xff},/* 338: Average24 phase current */
CL_REG1,{0x01,0x00,0xac,0x1b,0xff},/* 339: Average24 voltage */
CL_REG1,{0x01,0x00,0xac,0x80,0x7e},/* 340: Average24 voltage THD */
CL_REG0,{0x01,0x81,0x09,0x08,0x00},/* 341: Apparent energy net total */
CL_EXR1,{0x00,0x01,0x60,0x0c,0x05},/* 342: Average signal strength(RSSI) */
CL_EXR1,{0x01,0x00,0x01,0x06,0x00},/* 343: Max Demand+ kW */
CL_EXR1,{0x01,0x00,0x01,0x06,0x01},/* 344: Max Demand+ kW TZ1 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x02},/* 345: Max Demand+ kW TZ2 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x03},/* 346: Max Demand+ kW TZ3 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x04},/* 347: Max Demand+ kW TZ4 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x05},/* 348: Max Demand+ kW TZ5 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x06},/* 349: Max Demand+ kW TZ6 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x07},/* 350: Max Demand+ kW TZ7 */
CL_EXR1,{0x01,0x00,0x01,0x06,0x08},/* 351: Max Demand+ kW TZ8 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x00},/* 352: Max Demand- kW */
CL_EXR1,{0x01,0x00,0x02,0x06,0x01},/* 353: Max Demand- kW TZ1 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x02},/* 354: Max Demand- kW TZ2 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x03},/* 355: Max Demand- kW TZ3 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x04},/* 356: Max Demand- kW TZ4 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x05},/* 357: Max Demand- kW TZ5 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x06},/* 358: Max Demand- kW TZ6 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x07},/* 359: Max Demand- kW TZ7 */
CL_EXR1,{0x01,0x00,0x02,0x06,0x08},/* 360: Max Demand- kW TZ8 */
CL_EXR1,{0x01,0x00,0x03,0x06,0x00},/* 361: Max.Demand+ var */
CL_EXR1,{0x01,0x00,0x04,0x06,0x00},/* 362: Max.Demand- var */
CL_EXR1,{0x01,0x00,0x09,0x06,0x00},/* 363: Max Demand+ kVA */
CL_EXR1,{0x01,0x00,0x09,0x06,0x01},/* 364: Max Demand+ kVA TZ1 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x02},/* 365: Max Demand+ kVA TZ2 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x03},/* 366: Max Demand+ kVA TZ3 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x04},/* 367: Max Demand+ kVA TZ4 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x05},/* 368: Max Demand+ kVA TZ5 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x06},/* 369: Max Demand+ kVA TZ6 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x07},/* 370: Max Demand+ kVA TZ7 */
CL_EXR1,{0x01,0x00,0x09,0x06,0x08},/* 371: Max Demand+ kVA TZ8 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x00},/* 372: Max Demand- kVA */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x01},/* 373: Max Demand- kVA TZ1 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x02},/* 374: Max Demand- kVA TZ2 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x03},/* 375: Max Demand- kVA TZ3 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x04},/* 376: Max Demand- kVA TZ4 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x05},/* 377: Max Demand- kVA TZ5 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x06},/* 378: Max Demand- kVA TZ6 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x07},/* 379: Max Demand- kVA TZ7 */
CL_EXR1,{0x01,0x00,0x0a,0x06,0x08},/* 380: Max Demand- kVA TZ8 */
CL_EXR1,{0x01,0x00,0x0c,0x03,0x00},/* 381: Minimum Voltage0 */
CL_EXR1,{0x01,0x00,0x20,0x03,0x00},/* 382: Minimum Voltage of L1  */
CL_EXR1,{0x01,0x00,0x34,0x03,0x00},/* 383: Minimum Voltage of L2  */
CL_EXR1,{0x01,0x00,0x48,0x03,0x00},/* 384: Minimum Voltage of L3  */
CL_EXR1,{0x01,0x00,0xa1,0x06,0xff},/* 385: Max24 Demand+ kW */
CL_EXR1,{0x01,0x00,0xa2,0x06,0xff},/* 386: Max24 Demand- kW */
CL_EXR1,{0x01,0x00,0xa3,0x06,0xff},/* 387: Max24 Demand+ var */
CL_EXR1,{0x01,0x00,0xa4,0x06,0xff},/* 388: Max24 Demand- var */
CL_EXR1,{0x01,0x00,0xa9,0x06,0xff},/* 389: Max24 Demand+ kVA */
CL_EXR1,{0x01,0x00,0xaa,0x06,0xff},/* 390: Max24 Demand- kVA */
CL_PRG1,{0x00,0x00,0x5e,0x5b,0x0a},/* 391: Name plate detail */
CL_PRG1,{0x00,0x00,0x63,0x62,0x00},/* 392: Event1 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x01},/* 393: Event2 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x02},/* 394: Event3 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x03},/* 395: Event4 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x04},/* 396: Event5 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x05},/* 397: Event6 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x06},/* 398: Event7 profile */
CL_PRG1,{0x00,0x00,0x63,0x62,0x0a},/* 399: Relay log */
CL_PRG1,{0x00,0x00,0x63,0x62,0xff},/* 400: Meter log */
CL_PRG1,{0x01,0x00,0x5e,0x5b,0x00},/* 401: Snapshot of instantaneous parameters A1,A14 */
CL_PRG1,{0x01,0x00,0x5e,0x5b,0x03},/* 402: Scaler profile-A1,A14 */
CL_PRG1,{0x01,0x00,0x5e,0x5b,0x04},/* 403: Scaler profile-A2,A15 */
CL_PRG1,{0x01,0x00,0x5e,0x5b,0x05},/* 404: Scaler profile-A3,A16 */
CL_PRG1,{0x01,0x00,0x5e,0x5b,0x06},/* 405: Scaler profile-A4,A17 */
CL_PRG1,{0x01,0x00,0x5e,0x5b,0x07},/* 406: Scaler profile-A5-A11 */
CL_PRG1,{0x01,0x00,0x62,0x01,0x00},/* 407: Billing profile parameters */
CL_PRG1,{0x01,0x00,0x63,0x01,0x00},/* 408: Block load profile */
CL_PRG1,{0x01,0x00,0x63,0x02,0x00},/* 409: Daily load profile */
CL_CLK,{0x00,0x00,0x01,0x00,0x00},/* 410: Date time */
CL_CLK,{0x00,0x00,0x01,0x00,0x01},/* 411: Date time(RTC) */
CL_SCR,{0x00,0x00,0x0a,0x00,0x01},/* 412: MDI Reset/End of Billing Period script table */
CL_SCR,{0x00,0x00,0x0a,0x00,0x64},/* 413: Tariffication script table */
CL_SCR,{0x00,0x00,0x0a,0x00,0x6b},/* 414: Image activation script table */
CL_SCR,{0x00,0x00,0x0a,0x00,0x6c},/* 415: Push script table */
CL_SPD,{0x00,0x00,0x0b,0x00,0x00},/* 416: Special days table */
CL_ASL1,{0x00,0x00,0x28,0x00,0x00},/* 417: Current Association */
CL_ASL0,{0x00,0x00,0x28,0x00,0x01},/* 418: PC-Association */
CL_ASL0,{0x00,0x00,0x28,0x00,0x02},/* 419: MR-Association */
CL_ASL1,{0x00,0x00,0x28,0x00,0x03},/* 420: US-Association */
CL_ASL0,{0x00,0x00,0x28,0x00,0x04},/* 421: PUSH-Association */
CL_ASL1,{0x00,0x00,0x28,0x00,0x05},/* 422: FIRM-Association */
CL_ASL0,{0x00,0x00,0x28,0x00,0x06},/* 423: IHD-Association */
CL_IMG,{0x00,0x00,0x2c,0x00,0x00},/* 424: Image transfer */
CL_IMG,{0x00,0x00,0x2c,0x00,0x01},/* 425: Image transfer1 */
CL_CAL,{0x00,0x00,0x0d,0x00,0x00},/* 426: Activity calendar for time zones */
CL_SNG,{0x00,0x00,0x0f,0x00,0x00},/* 427: Single action schedule for billing dates */
CL_SNG,{0x00,0x00,0x0f,0x00,0x02},/* 428: Image Activation Single action schedule */
CL_SNG,{0x00,0x00,0x0f,0x00,0x04},/* 429: Trigger SM TO HES-Single action schedule */
CL_SNG,{0x00,0x00,0x0f,0x00,0x8e},/* 430: Single action schedule for Two Way Communication  */
CL_SNG,{0x00,0x00,0x0f,0x00,0x8f},/* 431: Single action schedule for Tamper data Push */
CL_SNG,{0x00,0x00,0x0f,0x01,0x04},/* 432: Trigger SM TO IHD-Single action schedule */
CL_SNG,{0x00,0x04,0x0f,0x00,0x04},/* 433: Block load profile push schedule change */
CL_SNG,{0x00,0x05,0x0f,0x00,0x04},/* 434: Daily load profile push schedule change */
CL_SNG,{0x00,0x06,0x0f,0x00,0x04},/* 435: Billing rofile push schedule change */
CL_HDL,{0x00,0x00,0x16,0x00,0x00},/* 436: IEC HDLC setup */
CL_HDL,{0x00,0x01,0x16,0x00,0x00},/* 437: HDLC Setting(Other) */
CL_PSH,{0x00,0x00,0x19,0x09,0x00},/* 438: SM to HES push */
CL_PSH,{0x00,0x03,0x19,0x09,0x00},/* 439: SM TO IHD push */
CL_PSH,{0x00,0x04,0x19,0x09,0x00},/* 440: SM TO HES event push */
CL_PSH,{0x00,0x05,0x19,0x09,0x00},/* 441: SM to HES Load profle push */
CL_PSH,{0x00,0x06,0x19,0x09,0x00},/* 442: SM to HES daily profle push */
CL_PSH,{0x00,0x82,0x19,0x09,0x00},/* 443: Self Registration Push */
CL_PSH,{0x00,0x84,0x19,0x09,0x00},/* 444: SM to HES Billing profle push */
CL_PSH,{0x00,0x86,0x19,0x09,0x00},/* 445: SM to HES Tamper push */
CL_TCP,{0x00,0x00,0x19,0x00,0x00},/* 446: TCP-UDP setup */
CL_GPRS,{0x00,0x00,0x19,0x04,0x00},/* 447: ANP setting */
CL_IPV6,{0x00,0x00,0x19,0x07,0x00},/* 448: IPV6 setup */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x00},/* 449: Setup security(For security NON) */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x01},/* 450: Setup security(For security HLS) */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x02},/* 451: MR-Security setup */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x03},/* 452: US-Security setup */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x04},/* 453: PUSH-Security setup */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x05},/* 454: FIRM-Security setup */
CL_SEC1,{0x00,0x00,0x2b,0x00,0x06},/* 455: IHD-Security setup */
CL_DIS,{0x00,0x00,0x60,0x03,0x0a},/* 456: Disconnect control */
CL_LMT,{0x00,0x00,0x11,0x00,0x00},/* 457: Load limit value in kW */

CL_SKIP,{ 0xFF,0xFF,0xFF,0xFF,0xFF}/* end marker */
};

/************************/
/* クライアント-オブジェクトテーブル */
/* クライアント毎に使用できるオブジェクトを定義する。 */
/************************/
static ui08  CONST OBJMAP[] = {
	0x00,

0x04,	/* 1: Cumulative Billing count */
0x04,	/* 2: Available Billing Period  */
0x00,	/* 3: Time */
0x00,	/* 4: Date */
0x04,	/* 5: NIC SIM ID */
0x03,	/* 6: COSEM logical device name  */
0x00,	/* 7: Frame counter */
0x00,	/* 8: Frame counter1 */
0x03,	/* 9: Frame counter2 */
0x03,	/* 10: Frame counter3 */
0x03,	/* 11: Frame counter4 */
0x03,	/* 12: Frame counter5 */
0x00,	/* 13: Frame counter6 */
0x04,	/* 14: Cumulative tamper count */
0x04,	/* 15: Meter Type (1Phase/3P-3W/3P-4W) */
0x04,	/* 16: Category */
0x04,	/* 17: Current rating */
0x06,	/* 18: Event status word */
0x07,	/* 19: Event status word filter */
0x07,	/* 20: Metering mode */
0x07,	/* 21: Payment mode */
0x07,	/* 22: Last token recharge amount */
0x07,	/* 23: Last token recharge time  */
0x07,	/* 24: Total amount at last recharge  */
0x07,	/* 25: Current balance amount */
0x07,	/* 26: Current balance time */
0x04,	/* 27: Meter serial number */
0x04,	/* 28: Manufacturer name */
0x04,	/* 29: Device ID */
0x04,	/* 30: Meter year of manufacture */
0x04,	/* 31: NIC IMEI number (serial number) */
0x04,	/* 32: NIC Year of manufacture */
0x04,	/* 33: NIC serial ID */
0x04,	/* 34: Bluetooth mac address */
0x04,	/* 35: NIC Firmware Version */
0x04,	/* 36: Cumulative program count */
0x04,	/* 37: Number of power failures */
0x06,	/* 38: NIC status  */
0x04,	/* 39: Event1 */
0x04,	/* 40: Event2 */
0x04,	/* 41: Event3 */
0x04,	/* 42: Event4 */
0x04,	/* 43: Event5 */
0x04,	/* 44: Event6 */
0x04,	/* 45: Event7 */
0x00,	/* 46: Meter log code */
0x06,	/* 47: Meter status */
0x04,	/* 48: Event1-recordNO */
0x04,	/* 49: Event2-recordNO */
0x04,	/* 50: Event3-recordNO */
0x04,	/* 51: Event4-recordNO */
0x04,	/* 52: Event5-recordNO */
0x04,	/* 53: Event6-recordNO */
0x04,	/* 54: Event7-recordNO */
0x04,	/* 55: Relay Count */
0x04,	/* 56: Cell ID */
0x04,	/* 57: 4G&NB fallback to GSM parameter */
0x04,	/* 58: RAT & Band */
0x04,	/* 59: Event stamping parameter for RC/DC */
0x00,	/* 60: RAM */
0x00,	/* 61: EEPROM */
0x00,	/* 62: ROM */
0x00,	/* 63: Energy calibration */
0x00,	/* 64: Read energy(BF)　 */
0x00,	/* 65: Voltage and ampere calibration */
0x00,	/* 66: Read volt and ampere(D3)　 */
0x00,	/* 67: RTC calibration */
0x08,	/* 68: BuildNO */
0x08,	/* 69: BildDate */
0x00,	/* 70: Setting bit array */
0x00,	/* 71: Unit setting */
0x08,	/* 72: Display time */
0x00,	/* 73: Record setting */
0x00,	/* 74: Power quality code define */
0x00,	/* 75: Pulse setting */
0x00,	/* 76: Detect event setting */
0x08,	/* 77: Server port */
0x04,	/* 78: Cumulative event count */
0x04,	/* 79: Cumulative tamper count 1month */
0x04,	/* 80: Cumulative top cover count */
0x04,	/* 81: Top cover tamper datetime */
0x04,	/* 82: Cumulative reset count */
0x04,	/* 83: Reset datetime */
0x04,	/* 84: NIC RSSI */
0x00,	/* 85: RSRP */
0x00,	/* 86: RSRQ */
0x00,	/* 87: SNR */
0x04,	/* 88: NIC firmware version number  */
0x07,	/* 89: Apparent energy computation type */
0x07,	/* 90: LRCF */
0x00,	/* 91: Meter type */
0x00,	/* 92: Product model */
0x01,	/* 93: Firmware version for meter */
0x00,	/* 94: Active firmware version  */
0x00,	/* 95: Active firmware signature  */
0x07,	/* 96: Demand integration period */
0x07,	/* 97: Capture period-Block load profile */
0x07,	/* 98: Capture period-Daily load profile */
0x07,	/* 99: Apparent energy computation type */
0x07,	/* 100: Self diagnostic details */
0x08,	/* 101: Current active relay time */
0x08,	/* 102: Active relay time setting */
0x04,	/* 103: Cumulative magnetic count */
0x04,	/* 104: Magnet tamper datetime */
0x04,	/* 105: Cumulative esd count */
0x04,	/* 106: ESD tamper datetime */
0x04,	/* 107: Cumulative jammer count */
0x04,	/* 108: Jammer tamper datetime */
0x04,	/* 109: Cumulative vthd count */
0x04,	/* 110: vthd tamper datetime */
0x04,	/* 111: Cumulative athd count */
0x04,	/* 112: athd tamper datetime */
0x04,	/* 113: Cumulative atdd count */
0x04,	/* 114: atdd tamper datetime */
0x04,	/* 115: Cumulative disconnect count */
0x04,	/* 116: Disconnect datetime */
0x04,	/* 117: Cumulative connect count */
0x04,	/* 118: Connect datetime */
0x04,	/* 119: Billing date */
0x04,	/* 120: Cumulative power off duration in min */
0x04,	/* 121: Billing power on duration in min */
0x04,	/* 122: Cumulative power on duration in min */
0x00,	/* 123: Battery voltage */
0x00,	/* 124: Operating time objects  */
0x05,	/* 125: Temperature */
0x05,	/* 126: Temperature block */
0x05,	/* 127: Temperature block24 */
0x05,	/* 128: Magnet voltage */
0x05,	/* 129: Temperature(CPU/MAG) block */
0x05,	/* 130: Temperature(CPU/MAG) block24 */
0x0a,	/* 131: Cumulative kW MD Import  */
0x0a,	/* 132: Active power+- */
0x0a,	/* 133: THD Power Phase  */
0x0c,	/* 134: Cumulative Energy+ Wh */
0x0c,	/* 135: Cumulative Energy+ Wh TZ1 */
0x0c,	/* 136: Cumulative Energy+ Wh TZ2 */
0x0c,	/* 137: Cumulative Energy+ Wh TZ3 */
0x0c,	/* 138: Cumulative Energy+ Wh TZ4 */
0x0c,	/* 139: Cumulative Energy+ Wh TZ5 */
0x0c,	/* 140: Cumulative Energy+ Wh TZ6 */
0x0c,	/* 141: Cumulative Energy+ Wh TZ7 */
0x0c,	/* 142: Cumulative Energy+ Wh TZ8 */
0x0b,	/* 143: Block Energy+ W */
0x0b,	/* 144: Block Energy+ W TZ1 */
0x0b,	/* 145: Block Energy+ W TZ2 */
0x0b,	/* 146: Block Energy+ W TZ3 */
0x0b,	/* 147: Block Energy+ W TZ4 */
0x0b,	/* 148: Block Energy+ W TZ5 */
0x0b,	/* 149: Block Energy+ W TZ6 */
0x0b,	/* 150: Block Energy+ W TZ7 */
0x0b,	/* 151: Block Energy+ W TZ8 */
0x0b,	/* 152: Block Energy+ Wh */
0x0b,	/* 153: Block Energy+ Wh TZ1 */
0x0b,	/* 154: Block Energy+ Wh TZ2 */
0x0b,	/* 155: Block Energy+ Wh TZ3 */
0x0b,	/* 156: Block Energy+ Wh TZ4 */
0x0b,	/* 157: Block Energy+ Wh TZ5 */
0x0b,	/* 158: Block Energy+ Wh TZ6 */
0x0b,	/* 159: Block Energy+ Wh TZ7 */
0x0b,	/* 160: Block Energy+ Wh TZ8 */
0x0a,	/* 161: Cumulative kW MD Export */
0x0c,	/* 162: Cumulative Energy- Wh */
0x0c,	/* 163: Cumulative Energy- Wh TZ1 */
0x0c,	/* 164: Cumulative Energy- Wh TZ2 */
0x0c,	/* 165: Cumulative Energy- Wh TZ3 */
0x0c,	/* 166: Cumulative Energy- Wh TZ4 */
0x0c,	/* 167: Cumulative Energy- Wh TZ5 */
0x0c,	/* 168: Cumulative Energy- Wh TZ6 */
0x0c,	/* 169: Cumulative Energy- Wh TZ7 */
0x0c,	/* 170: Cumulative Energy- Wh TZ8 */
0x0b,	/* 171: Block Energy- W */
0x0b,	/* 172: Block Energy- W TZ1 */
0x0b,	/* 173: Block Energy- W TZ2 */
0x0b,	/* 174: Block Energy- W TZ3 */
0x0b,	/* 175: Block Energy- W TZ4 */
0x0b,	/* 176: Block Energy- W TZ5 */
0x0b,	/* 177: Block Energy- W TZ6 */
0x0b,	/* 178: Block Energy- W TZ7 */
0x0b,	/* 179: Block Energy- W TZ8 */
0x0b,	/* 180: Block Energy- Wh */
0x0b,	/* 181: Block Energy- Wh TZ1 */
0x0b,	/* 182: Block Energy- Wh TZ2 */
0x0b,	/* 183: Block Energy- Wh TZ3 */
0x0b,	/* 184: Block Energy- Wh TZ4 */
0x0b,	/* 185: Block Energy- Wh TZ5 */
0x0b,	/* 186: Block Energy- Wh TZ6 */
0x0b,	/* 187: Block Energy- Wh TZ7 */
0x0b,	/* 188: Block Energy- Wh TZ8 */
0x0a,	/* 189: Reactive power+- */
0x0c,	/* 190: Cumulative Energy+ varh */
0x0c,	/* 191: Cumulative Energy- varh */
0x0c,	/* 192: Cumulative Energy kvarh (Q1) */
0x0c,	/* 193: Block Energy+ kvarh */
0x0c,	/* 194: Cumulative Energy kvarh (Q2) */
0x0b,	/* 195: Block Energy kvarh (Q2) */
0x0c,	/* 196: Cumulative Energy kvarh (Q3) */
0x0b,	/* 197: Block Energy kvarh (Q3) */
0x0c,	/* 198: Cumulative Energy kvarh (Q4) */
0x0c,	/* 199: Block Energy- kvarh */
0x0a,	/* 200: Cumulative kVA MD Import  */
0x0c,	/* 201: Apparent power kVA+- */
0x0c,	/* 202: Cumulative Energy+ kVAh */
0x0c,	/* 203: Cumulative Energy+ kVAh TZ1 */
0x0c,	/* 204: Cumulative Energy+ kVAh TZ2 */
0x0c,	/* 205: Cumulative Energy+ kVAh TZ3 */
0x0c,	/* 206: Cumulative Energy+ kVAh TZ4 */
0x0c,	/* 207: Cumulative Energy+ kVAh TZ5 */
0x0c,	/* 208: Cumulative Energy+ kVAh TZ6 */
0x0c,	/* 209: Cumulative Energy+ kVAh TZ7 */
0x0c,	/* 210: Cumulative Energy+ kVAh TZ8 */
0x0b,	/* 211: Block Energy+ VA */
0x0b,	/* 212: Block Energy+ VA TZ1 */
0x0b,	/* 213: Block Energy+ VA TZ2 */
0x0b,	/* 214: Block Energy+ VA TZ3 */
0x0b,	/* 215: Block Energy+ VA TZ4 */
0x0b,	/* 216: Block Energy+ VA TZ5 */
0x0b,	/* 217: Block Energy+ VA TZ6 */
0x0b,	/* 218: Block Energy+ VA TZ7 */
0x0b,	/* 219: Block Energy+ VA TZ8 */
0x0b,	/* 220: Block Energy+ VAh */
0x0b,	/* 221: Block Energy+ VAh TZ1 */
0x0b,	/* 222: Block Energy+ VAh TZ2 */
0x0b,	/* 223: Block Energy+ VAh TZ3 */
0x0b,	/* 224: Block Energy+ VAh TZ4 */
0x0b,	/* 225: Block Energy+ VAh TZ5 */
0x0b,	/* 226: Block Energy+ VAh TZ6 */
0x0b,	/* 227: Block Energy+ VAh TZ7 */
0x0b,	/* 228: Block Energy+ VAh TZ8 */
0x0b,	/* 229: Cumulative kVA MD Export */
0x0b,	/* 230: Cumulative Energy- kVAh */
0x0c,	/* 231: Cumulative Energy- kVAh TZ1 */
0x0c,	/* 232: Cumulative Energy- kVAh TZ2 */
0x0c,	/* 233: Cumulative Energy- kVAh TZ3 */
0x0c,	/* 234: Cumulative Energy- kVAh TZ4 */
0x0c,	/* 235: Cumulative Energy- kVAh TZ5 */
0x0c,	/* 236: Cumulative Energy- kVAh TZ6 */
0x0c,	/* 237: Cumulative Energy- kVAh TZ7 */
0x0c,	/* 238: Cumulative Energy- kVAh TZ8 */
0x0b,	/* 239: Block Energy- VA */
0x0b,	/* 240: Block Energy- VA TZ1 */
0x0b,	/* 241: Block Energy- VA TZ2 */
0x0b,	/* 242: Block Energy- VA TZ3 */
0x0b,	/* 243: Block Energy- VA TZ4 */
0x0b,	/* 244: Block Energy- VA TZ5 */
0x0b,	/* 245: Block Energy- VA TZ6 */
0x0b,	/* 246: Block Energy- VA TZ7 */
0x0b,	/* 247: Block Energy- VA TZ8 */
0x0b,	/* 248: Block Energy- VAh */
0x0b,	/* 249: Block Energy- VAh TZ1 */
0x0b,	/* 250: Block Energy- VAh TZ2 */
0x0b,	/* 251: Block Energy- VAh TZ3 */
0x0b,	/* 252: Block Energy- VAh TZ4 */
0x0b,	/* 253: Block Energy- VAh TZ5 */
0x0b,	/* 254: Block Energy- VAh TZ6 */
0x0b,	/* 255: Block Energy- VAh TZ7 */
0x0b,	/* 256: Block Energy- VAh TZ8 */
0x0a,	/* 257: Phase current */
0x0a,	/* 258: THD Current  */
0x0a,	/* 259: Block phase current */
0x0b,	/* 260: Voltage0 Ave 1min value  */
0x0a,	/* 261: Voltage */
0x0a,	/* 262: THD Voltage  */
0x0b,	/* 263: Block voltage */
0x0b,	/* 264: Block voltage THD */
0x0b,	/* 265: Average power factor for billing period */
0x0b,	/* 266: System Imp PF T1  */
0x0b,	/* 267: System Imp PF T2  */
0x0b,	/* 268: System Imp PF T3  */
0x0b,	/* 269: System Imp PF T4  */
0x0b,	/* 270: System Imp PF T5  */
0x0b,	/* 271: System Imp PF T6  */
0x0b,	/* 272: System Imp PF T7  */
0x0b,	/* 273: System Imp PF T8  */
0x0b,	/* 274: Signed power factor/Three phase power factor */
0x0b,	/* 275: Signed power factor/Three phase power factor */
0x0b,	/* 276: Signed power factor/Three phase power factor */
0x0b,	/* 277: Power factor imp day */
0x0b,	/* 278: Frequency */
0x00,	/* 279: ABS Wh */
0x00,	/* 280: NET Wh */
0x00,	/* 281: ABS VarH */
0x00,	/* 282: NET VarH */
0x00,	/* 283: Current L1 */
0x0a,	/* 284: IR-block */
0x0a,	/* 285: Voltage L1 Ave 1min value  */
0x0a,	/* 286: Voltage L1 */
0x0a,	/* 287: Voltage L1 Block */
0x0a,	/* 288: Signed power factorR */
0x00,	/* 289: Current L2 */
0x00,	/* 290: IY-block */
0x00,	/* 291: Voltage L2 Ave 1min value  */
0x00,	/* 292: Voltage L2 */
0x00,	/* 293: Voltage L2 Block */
0x00,	/* 294: Signed power factorY */
0x00,	/* 295: Current L3 */
0x00,	/* 296: IB-block */
0x00,	/* 297: Voltage L3 Ave 1min value  */
0x00,	/* 298: Voltage L3 */
0x00,	/* 299: Voltage L3 Block */
0x00,	/* 300: Signed power factorB */
0x0b,	/* 301: Power factor exp month */
0x0b,	/* 302: System Exp PF T1  */
0x0b,	/* 303: System Exp PF T2  */
0x0b,	/* 304: System Exp PF T3  */
0x0b,	/* 305: System Exp PF T4  */
0x0b,	/* 306: System Exp PF T5  */
0x0b,	/* 307: System Exp PF T6  */
0x0b,	/* 308: System Exp PF T7  */
0x0b,	/* 309: System Exp PF T8  */
0x0b,	/* 310: Power factor exp block  */
0x0b,	/* 311: Power factor exp day */
0x0a,	/* 312: Neutral current */
0x0a,	/* 313: Block neutral current */
0x0a,	/* 314: Average24 neutral current */
0x0a,	/* 315: Current */
0x0a,	/* 316: Defraud Kwh */
0x05,	/* 317: Temperature(CPU/MAG) */
0x0a,	/* 318: Current-Avearage? */
0x0a,	/* 319: Fundamental Energy kWh */
0x0a,	/* 320: Block current THD */
0x0a,	/* 321: Average24 current THD */
0x0a,	/* 322: Current TDD */
0x0a,	/* 323: Block current TDD */
0x0a,	/* 324: Average24 current TDD */
0x0b,	/* 325: Frequency L1  */
0x00,	/* 326: Frequency L2 */
0x00,	/* 327: Frequency L3 */
0x0b,	/* 328: Block24 Energy+ W */
0x0b,	/* 329: Block24 Energy+ Wh */
0x0b,	/* 330: Block24 Energy- W */
0x0b,	/* 331: Block24 Energy- Wh */
0x0c,	/* 332: Block24 Energy+ kvarh */
0x0c,	/* 333: Block24 Energy- kvarh */
0x0b,	/* 334: Block24 Energy+ VA */
0x0b,	/* 335: Block24 Energy+ VAh */
0x0b,	/* 336: Block24 Energy- VA */
0x0b,	/* 337: Block24 Energy- VAh */
0x0a,	/* 338: Average24 phase current */
0x0b,	/* 339: Average24 voltage */
0x0b,	/* 340: Average24 voltage THD */
0x0a,	/* 341: Apparent energy net total */
0x0a,	/* 342: Average signal strength(RSSI) */
0x0a,	/* 343: Max Demand+ kW */
0x0a,	/* 344: Max Demand+ kW TZ1 */
0x0c,	/* 345: Max Demand+ kW TZ2 */
0x0c,	/* 346: Max Demand+ kW TZ3 */
0x0c,	/* 347: Max Demand+ kW TZ4 */
0x0c,	/* 348: Max Demand+ kW TZ5 */
0x0c,	/* 349: Max Demand+ kW TZ6 */
0x0c,	/* 350: Max Demand+ kW TZ7 */
0x0c,	/* 351: Max Demand+ kW TZ8 */
0x0c,	/* 352: Max Demand- kW */
0x0c,	/* 353: Max Demand- kW TZ1 */
0x0c,	/* 354: Max Demand- kW TZ2 */
0x0c,	/* 355: Max Demand- kW TZ3 */
0x0c,	/* 356: Max Demand- kW TZ4 */
0x0c,	/* 357: Max Demand- kW TZ5 */
0x0c,	/* 358: Max Demand- kW TZ6 */
0x0c,	/* 359: Max Demand- kW TZ7 */
0x0c,	/* 360: Max Demand- kW TZ8 */
0x0c,	/* 361: Max.Demand+ var */
0x0c,	/* 362: Max.Demand- var */
0x0b,	/* 363: Max Demand+ kVA */
0x0b,	/* 364: Max Demand+ kVA TZ1 */
0x0b,	/* 365: Max Demand+ kVA TZ2 */
0x0b,	/* 366: Max Demand+ kVA TZ3 */
0x0b,	/* 367: Max Demand+ kVA TZ4 */
0x0b,	/* 368: Max Demand+ kVA TZ5 */
0x0b,	/* 369: Max Demand+ kVA TZ6 */
0x0b,	/* 370: Max Demand+ kVA TZ7 */
0x0b,	/* 371: Max Demand+ kVA TZ8 */
0x0b,	/* 372: Max Demand- kVA */
0x0b,	/* 373: Max Demand- kVA TZ1 */
0x0b,	/* 374: Max Demand- kVA TZ2 */
0x0b,	/* 375: Max Demand- kVA TZ3 */
0x0b,	/* 376: Max Demand- kVA TZ4 */
0x0b,	/* 377: Max Demand- kVA TZ5 */
0x0b,	/* 378: Max Demand- kVA TZ6 */
0x0b,	/* 379: Max Demand- kVA TZ7 */
0x0b,	/* 380: Max Demand- kVA TZ8 */
0x0b,	/* 381: Minimum Voltage0 */
0x00,	/* 382: Minimum Voltage of L1  */
0x00,	/* 383: Minimum Voltage of L2  */
0x00,	/* 384: Minimum Voltage of L3  */
0x0c,	/* 385: Max24 Demand+ kW */
0x0c,	/* 386: Max24 Demand- kW */
0x0c,	/* 387: Max24 Demand+ var */
0x0c,	/* 388: Max24 Demand- var */
0x0b,	/* 389: Max24 Demand+ kVA */
0x0b,	/* 390: Max24 Demand- kVA */
0x0e,	/* 391: Name plate detail */
0x0d,	/* 392: Event1 profile */
0x0d,	/* 393: Event2 profile */
0x0d,	/* 394: Event3 profile */
0x0d,	/* 395: Event4 profile */
0x0d,	/* 396: Event5 profile */
0x0d,	/* 397: Event6 profile */
0x0d,	/* 398: Event7 profile */
0x0d,	/* 399: Relay log */
0x00,	/* 400: Meter log */
0x0e,	/* 401: Snapshot of instantaneous parameters A1,A14 */
0x0e,	/* 402: Scaler profile-A1,A14 */
0x0e,	/* 403: Scaler profile-A2,A15 */
0x0e,	/* 404: Scaler profile-A3,A16 */
0x0e,	/* 405: Scaler profile-A4,A17 */
0x0e,	/* 406: Scaler profile-A5-A11 */
0x0f,	/* 407: Billing profile parameters */
0x0f,	/* 408: Block load profile */
0x0f,	/* 409: Daily load profile */
0x11,	/* 410: Date time */
0x00,	/* 411: Date time(RTC) */
0x12,	/* 412: MDI Reset/End of Billing Period script table */
0x12,	/* 413: Tariffication script table */
0x02,	/* 414: Image activation script table */
0x12,	/* 415: Push script table */
0x12,	/* 416: Special days table */
0x13,	/* 417: Current Association */
0x14,	/* 418: PC-Association */
0x15,	/* 419: MR-Association */
0x16,	/* 420: US-Association */
0x17,	/* 421: PUSH-Association */
0x18,	/* 422: FIRM-Association */
0x19,	/* 423: IHD-Association */
0x02,	/* 424: Image transfer */
0x00,	/* 425: Image transfer1 */
0x07,	/* 426: Activity calendar for time zones */
0x07,	/* 427: Single action schedule for billing dates */
0x02,	/* 428: Image Activation Single action schedule */
0x07,	/* 429: Trigger SM TO HES-Single action schedule */
0x07,	/* 430: Single action schedule for Two Way Communication  */
0x07,	/* 431: Single action schedule for Tamper data Push */
0x00,	/* 432: Trigger SM TO IHD-Single action schedule */
0x07,	/* 433: Block load profile push schedule change */
0x07,	/* 434: Daily load profile push schedule change */
0x07,	/* 435: Billing rofile push schedule change */
0x08,	/* 436: IEC HDLC setup */
0x08,	/* 437: HDLC Setting(Other) */
0x08,	/* 438: SM to HES push */
0x00,	/* 439: SM TO IHD push */
0x08,	/* 440: SM TO HES event push */
0x08,	/* 441: SM to HES Load profle push */
0x08,	/* 442: SM to HES daily profle push */
0x08,	/* 443: Self Registration Push */
0x08,	/* 444: SM to HES Billing profle push */
0x08,	/* 445: SM to HES Tamper push */
0x08,	/* 446: TCP-UDP setup */
0x08,	/* 447: ANP setting */
0x08,	/* 448: IPV6 setup */
0x00,	/* 449: Setup security(For security NON) */
0x00,	/* 450: Setup security(For security HLS) */
0x15,	/* 451: MR-Security setup */
0x16,	/* 452: US-Security setup */
0x17,	/* 453: PUSH-Security setup */
0x18,	/* 454: FIRM-Security setup */
0x19,	/* 455: IHD-Security setup */
0x09,	/* 456: Disconnect control */
0x09,	/* 457: Load limit value in kW */
0x00,
};

/* PUBクライアント用テーブル */
static CADR as_ln1[] =
{
	NULL,

(ADDR)NULL,/* 1: Cumulative Billing count */
(ADDR)NULL,/* 2: Available Billing Period  */
(ADDR)NULL,/* 3: Time */
(ADDR)NULL,/* 4: Date */
(ADDR)NULL,/* 5: NIC SIM ID */
(ADDR)&ln1_logical_name[0], /* 6: COSEM logical device name  */
(ADDR)NULL,/* 7: Frame counter */
(ADDR)NULL,/* 8: Frame counter1 */
(ADDR)NULL,/* 9: Frame counter2 */
(ADDR)&ln1_u32_dat_r[0], /* 10: Frame counter3 */
(ADDR)&ln1_u32_dat_r[0], /* 11: Frame counter4 */
(ADDR)&ln1_u32_dat_r[0], /* 12: Frame counter5 */
(ADDR)NULL,/* 13: Frame counter6 */
(ADDR)NULL,/* 14: Cumulative tamper count */
(ADDR)NULL,/* 15: Meter Type (1Phase/3P-3W/3P-4W) */
(ADDR)NULL,/* 16: Category */
(ADDR)NULL,/* 17: Current rating */
(ADDR)NULL,/* 18: Event status word */
(ADDR)NULL,/* 19: Event status word filter */
(ADDR)NULL,/* 20: Metering mode */
(ADDR)NULL,/* 21: Payment mode */
(ADDR)NULL,/* 22: Last token recharge amount */
(ADDR)NULL,/* 23: Last token recharge time  */
(ADDR)NULL,/* 24: Total amount at last recharge  */
(ADDR)NULL,/* 25: Current balance amount */
(ADDR)NULL,/* 26: Current balance time */
(ADDR)&ln1_serial_no[0], /* 27: Meter serial number */
(ADDR)NULL,/* 28: Manufacturer name */
(ADDR)NULL,/* 29: Device ID */
(ADDR)NULL,/* 30: Meter year of manufacture */
(ADDR)NULL,/* 31: NIC IMEI number (serial number) */
(ADDR)NULL,/* 32: NIC Year of manufacture */
(ADDR)NULL,/* 33: NIC serial ID */
(ADDR)NULL,/* 34: Bluetooth mac address */
(ADDR)NULL,/* 35: NIC Firmware Version */
(ADDR)NULL,/* 36: Cumulative program count */
(ADDR)NULL,/* 37: Number of power failures */
(ADDR)NULL,/* 38: NIC status  */
(ADDR)NULL,/* 39: Event1 */
(ADDR)NULL,/* 40: Event2 */
(ADDR)NULL,/* 41: Event3 */
(ADDR)NULL,/* 42: Event4 */
(ADDR)NULL,/* 43: Event5 */
(ADDR)NULL,/* 44: Event6 */
(ADDR)NULL,/* 45: Event7 */
(ADDR)NULL,/* 46: Meter log code */
(ADDR)NULL,/* 47: Meter status */
(ADDR)NULL,/* 48: Event1-recordNO */
(ADDR)NULL,/* 49: Event2-recordNO */
(ADDR)NULL,/* 50: Event3-recordNO */
(ADDR)NULL,/* 51: Event4-recordNO */
(ADDR)NULL,/* 52: Event5-recordNO */
(ADDR)NULL,/* 53: Event6-recordNO */
(ADDR)NULL,/* 54: Event7-recordNO */
(ADDR)NULL,/* 55: Relay Count */
(ADDR)NULL,/* 56: Cell ID */
(ADDR)NULL,/* 57: 4G&NB fallback to GSM parameter */
(ADDR)NULL,/* 58: RAT & Band */
(ADDR)NULL,/* 59: Event stamping parameter for RC/DC */
(ADDR)NULL,/* 60: RAM */
(ADDR)NULL,/* 61: EEPROM */
(ADDR)NULL,/* 62: ROM */
(ADDR)NULL,/* 63: Energy calibration */
(ADDR)NULL,/* 64: Read energy(BF)　 */
(ADDR)NULL,/* 65: Voltage and ampere calibration */
(ADDR)NULL,/* 66: Read volt and ampere(D3)　 */
(ADDR)NULL,/* 67: RTC calibration */
(ADDR)NULL,/* 68: BuildNO */
(ADDR)NULL,/* 69: BildDate */
(ADDR)NULL,/* 70: Setting bit array */
(ADDR)NULL,/* 71: Unit setting */
(ADDR)NULL,/* 72: Display time */
(ADDR)NULL,/* 73: Record setting */
(ADDR)NULL,/* 74: Power quality code define */
(ADDR)NULL,/* 75: Pulse setting */
(ADDR)NULL,/* 76: Detect event setting */
(ADDR)NULL,/* 77: Server port */
(ADDR)NULL,/* 78: Cumulative event count */
(ADDR)NULL,/* 79: Cumulative tamper count 1month */
(ADDR)NULL,/* 80: Cumulative top cover count */
(ADDR)NULL,/* 81: Top cover tamper datetime */
(ADDR)NULL,/* 82: Cumulative reset count */
(ADDR)NULL,/* 83: Reset datetime */
(ADDR)NULL,/* 84: NIC RSSI */
(ADDR)NULL,/* 85: RSRP */
(ADDR)NULL,/* 86: RSRQ */
(ADDR)NULL,/* 87: SNR */
(ADDR)NULL,/* 88: NIC firmware version number  */
(ADDR)NULL,/* 89: Apparent energy computation type */
(ADDR)NULL,/* 90: LRCF */
(ADDR)NULL,/* 91: Meter type */
(ADDR)NULL,/* 92: Product model */
(ADDR)NULL,/* 93: Firmware version for meter */
(ADDR)NULL,/* 94: Active firmware version  */
(ADDR)NULL,/* 95: Active firmware signature  */
(ADDR)NULL,/* 96: Demand integration period */
(ADDR)NULL,/* 97: Capture period-Block load profile */
(ADDR)NULL,/* 98: Capture period-Daily load profile */
(ADDR)NULL,/* 99: Apparent energy computation type */
(ADDR)NULL,/* 100: Self diagnostic details */
(ADDR)NULL,/* 101: Current active relay time */
(ADDR)NULL,/* 102: Active relay time setting */
(ADDR)NULL,/* 103: Cumulative magnetic count */
(ADDR)NULL,/* 104: Magnet tamper datetime */
(ADDR)NULL,/* 105: Cumulative esd count */
(ADDR)NULL,/* 106: ESD tamper datetime */
(ADDR)NULL,/* 107: Cumulative jammer count */
(ADDR)NULL,/* 108: Jammer tamper datetime */
(ADDR)NULL,/* 109: Cumulative vthd count */
(ADDR)NULL,/* 110: vthd tamper datetime */
(ADDR)NULL,/* 111: Cumulative athd count */
(ADDR)NULL,/* 112: athd tamper datetime */
(ADDR)NULL,/* 113: Cumulative atdd count */
(ADDR)NULL,/* 114: atdd tamper datetime */
(ADDR)NULL,/* 115: Cumulative disconnect count */
(ADDR)NULL,/* 116: Disconnect datetime */
(ADDR)NULL,/* 117: Cumulative connect count */
(ADDR)NULL,/* 118: Connect datetime */
(ADDR)NULL,/* 119: Billing date */
(ADDR)NULL,/* 120: Cumulative power off duration in min */
(ADDR)NULL,/* 121: Billing power on duration in min */
(ADDR)NULL,/* 122: Cumulative power on duration in min */
(ADDR)NULL,/* 123: Battery voltage */
(ADDR)NULL,/* 124: Operating time objects  */
(ADDR)NULL,/* 125: Temperature */
(ADDR)NULL,/* 126: Temperature block */
(ADDR)NULL,/* 127: Temperature block24 */
(ADDR)NULL,/* 128: Magnet voltage */
(ADDR)NULL,/* 129: Temperature(CPU/MAG) block */
(ADDR)NULL,/* 130: Temperature(CPU/MAG) block24 */
(ADDR)NULL,/* 131: Cumulative kW MD Import  */
(ADDR)NULL,/* 132: Active power+- */
(ADDR)NULL,/* 133: THD Power Phase  */
(ADDR)NULL,/* 134: Cumulative Energy+ Wh */
(ADDR)NULL,/* 135: Cumulative Energy+ Wh TZ1 */
(ADDR)NULL,/* 136: Cumulative Energy+ Wh TZ2 */
(ADDR)NULL,/* 137: Cumulative Energy+ Wh TZ3 */
(ADDR)NULL,/* 138: Cumulative Energy+ Wh TZ4 */
(ADDR)NULL,/* 139: Cumulative Energy+ Wh TZ5 */
(ADDR)NULL,/* 140: Cumulative Energy+ Wh TZ6 */
(ADDR)NULL,/* 141: Cumulative Energy+ Wh TZ7 */
(ADDR)NULL,/* 142: Cumulative Energy+ Wh TZ8 */
(ADDR)NULL,/* 143: Block Energy+ W */
(ADDR)NULL,/* 144: Block Energy+ W TZ1 */
(ADDR)NULL,/* 145: Block Energy+ W TZ2 */
(ADDR)NULL,/* 146: Block Energy+ W TZ3 */
(ADDR)NULL,/* 147: Block Energy+ W TZ4 */
(ADDR)NULL,/* 148: Block Energy+ W TZ5 */
(ADDR)NULL,/* 149: Block Energy+ W TZ6 */
(ADDR)NULL,/* 150: Block Energy+ W TZ7 */
(ADDR)NULL,/* 151: Block Energy+ W TZ8 */
(ADDR)NULL,/* 152: Block Energy+ Wh */
(ADDR)NULL,/* 153: Block Energy+ Wh TZ1 */
(ADDR)NULL,/* 154: Block Energy+ Wh TZ2 */
(ADDR)NULL,/* 155: Block Energy+ Wh TZ3 */
(ADDR)NULL,/* 156: Block Energy+ Wh TZ4 */
(ADDR)NULL,/* 157: Block Energy+ Wh TZ5 */
(ADDR)NULL,/* 158: Block Energy+ Wh TZ6 */
(ADDR)NULL,/* 159: Block Energy+ Wh TZ7 */
(ADDR)NULL,/* 160: Block Energy+ Wh TZ8 */
(ADDR)NULL,/* 161: Cumulative kW MD Export */
(ADDR)NULL,/* 162: Cumulative Energy- Wh */
(ADDR)NULL,/* 163: Cumulative Energy- Wh TZ1 */
(ADDR)NULL,/* 164: Cumulative Energy- Wh TZ2 */
(ADDR)NULL,/* 165: Cumulative Energy- Wh TZ3 */
(ADDR)NULL,/* 166: Cumulative Energy- Wh TZ4 */
(ADDR)NULL,/* 167: Cumulative Energy- Wh TZ5 */
(ADDR)NULL,/* 168: Cumulative Energy- Wh TZ6 */
(ADDR)NULL,/* 169: Cumulative Energy- Wh TZ7 */
(ADDR)NULL,/* 170: Cumulative Energy- Wh TZ8 */
(ADDR)NULL,/* 171: Block Energy- W */
(ADDR)NULL,/* 172: Block Energy- W TZ1 */
(ADDR)NULL,/* 173: Block Energy- W TZ2 */
(ADDR)NULL,/* 174: Block Energy- W TZ3 */
(ADDR)NULL,/* 175: Block Energy- W TZ4 */
(ADDR)NULL,/* 176: Block Energy- W TZ5 */
(ADDR)NULL,/* 177: Block Energy- W TZ6 */
(ADDR)NULL,/* 178: Block Energy- W TZ7 */
(ADDR)NULL,/* 179: Block Energy- W TZ8 */
(ADDR)NULL,/* 180: Block Energy- Wh */
(ADDR)NULL,/* 181: Block Energy- Wh TZ1 */
(ADDR)NULL,/* 182: Block Energy- Wh TZ2 */
(ADDR)NULL,/* 183: Block Energy- Wh TZ3 */
(ADDR)NULL,/* 184: Block Energy- Wh TZ4 */
(ADDR)NULL,/* 185: Block Energy- Wh TZ5 */
(ADDR)NULL,/* 186: Block Energy- Wh TZ6 */
(ADDR)NULL,/* 187: Block Energy- Wh TZ7 */
(ADDR)NULL,/* 188: Block Energy- Wh TZ8 */
(ADDR)NULL,/* 189: Reactive power+- */
(ADDR)NULL,/* 190: Cumulative Energy+ varh */
(ADDR)NULL,/* 191: Cumulative Energy- varh */
(ADDR)NULL,/* 192: Cumulative Energy kvarh (Q1) */
(ADDR)NULL,/* 193: Block Energy+ kvarh */
(ADDR)NULL,/* 194: Cumulative Energy kvarh (Q2) */
(ADDR)NULL,/* 195: Block Energy kvarh (Q2) */
(ADDR)NULL,/* 196: Cumulative Energy kvarh (Q3) */
(ADDR)NULL,/* 197: Block Energy kvarh (Q3) */
(ADDR)NULL,/* 198: Cumulative Energy kvarh (Q4) */
(ADDR)NULL,/* 199: Block Energy- kvarh */
(ADDR)NULL,/* 200: Cumulative kVA MD Import  */
(ADDR)NULL,/* 201: Apparent power kVA+- */
(ADDR)NULL,/* 202: Cumulative Energy+ kVAh */
(ADDR)NULL,/* 203: Cumulative Energy+ kVAh TZ1 */
(ADDR)NULL,/* 204: Cumulative Energy+ kVAh TZ2 */
(ADDR)NULL,/* 205: Cumulative Energy+ kVAh TZ3 */
(ADDR)NULL,/* 206: Cumulative Energy+ kVAh TZ4 */
(ADDR)NULL,/* 207: Cumulative Energy+ kVAh TZ5 */
(ADDR)NULL,/* 208: Cumulative Energy+ kVAh TZ6 */
(ADDR)NULL,/* 209: Cumulative Energy+ kVAh TZ7 */
(ADDR)NULL,/* 210: Cumulative Energy+ kVAh TZ8 */
(ADDR)NULL,/* 211: Block Energy+ VA */
(ADDR)NULL,/* 212: Block Energy+ VA TZ1 */
(ADDR)NULL,/* 213: Block Energy+ VA TZ2 */
(ADDR)NULL,/* 214: Block Energy+ VA TZ3 */
(ADDR)NULL,/* 215: Block Energy+ VA TZ4 */
(ADDR)NULL,/* 216: Block Energy+ VA TZ5 */
(ADDR)NULL,/* 217: Block Energy+ VA TZ6 */
(ADDR)NULL,/* 218: Block Energy+ VA TZ7 */
(ADDR)NULL,/* 219: Block Energy+ VA TZ8 */
(ADDR)NULL,/* 220: Block Energy+ VAh */
(ADDR)NULL,/* 221: Block Energy+ VAh TZ1 */
(ADDR)NULL,/* 222: Block Energy+ VAh TZ2 */
(ADDR)NULL,/* 223: Block Energy+ VAh TZ3 */
(ADDR)NULL,/* 224: Block Energy+ VAh TZ4 */
(ADDR)NULL,/* 225: Block Energy+ VAh TZ5 */
(ADDR)NULL,/* 226: Block Energy+ VAh TZ6 */
(ADDR)NULL,/* 227: Block Energy+ VAh TZ7 */
(ADDR)NULL,/* 228: Block Energy+ VAh TZ8 */
(ADDR)NULL,/* 229: Cumulative kVA MD Export */
(ADDR)NULL,/* 230: Cumulative Energy- kVAh */
(ADDR)NULL,/* 231: Cumulative Energy- kVAh TZ1 */
(ADDR)NULL,/* 232: Cumulative Energy- kVAh TZ2 */
(ADDR)NULL,/* 233: Cumulative Energy- kVAh TZ3 */
(ADDR)NULL,/* 234: Cumulative Energy- kVAh TZ4 */
(ADDR)NULL,/* 235: Cumulative Energy- kVAh TZ5 */
(ADDR)NULL,/* 236: Cumulative Energy- kVAh TZ6 */
(ADDR)NULL,/* 237: Cumulative Energy- kVAh TZ7 */
(ADDR)NULL,/* 238: Cumulative Energy- kVAh TZ8 */
(ADDR)NULL,/* 239: Block Energy- VA */
(ADDR)NULL,/* 240: Block Energy- VA TZ1 */
(ADDR)NULL,/* 241: Block Energy- VA TZ2 */
(ADDR)NULL,/* 242: Block Energy- VA TZ3 */
(ADDR)NULL,/* 243: Block Energy- VA TZ4 */
(ADDR)NULL,/* 244: Block Energy- VA TZ5 */
(ADDR)NULL,/* 245: Block Energy- VA TZ6 */
(ADDR)NULL,/* 246: Block Energy- VA TZ7 */
(ADDR)NULL,/* 247: Block Energy- VA TZ8 */
(ADDR)NULL,/* 248: Block Energy- VAh */
(ADDR)NULL,/* 249: Block Energy- VAh TZ1 */
(ADDR)NULL,/* 250: Block Energy- VAh TZ2 */
(ADDR)NULL,/* 251: Block Energy- VAh TZ3 */
(ADDR)NULL,/* 252: Block Energy- VAh TZ4 */
(ADDR)NULL,/* 253: Block Energy- VAh TZ5 */
(ADDR)NULL,/* 254: Block Energy- VAh TZ6 */
(ADDR)NULL,/* 255: Block Energy- VAh TZ7 */
(ADDR)NULL,/* 256: Block Energy- VAh TZ8 */
(ADDR)NULL,/* 257: Phase current */
(ADDR)NULL,/* 258: THD Current  */
(ADDR)NULL,/* 259: Block phase current */
(ADDR)NULL,/* 260: Voltage0 Ave 1min value  */
(ADDR)NULL,/* 261: Voltage */
(ADDR)NULL,/* 262: THD Voltage  */
(ADDR)NULL,/* 263: Block voltage */
(ADDR)NULL,/* 264: Block voltage THD */
(ADDR)NULL,/* 265: Average power factor for billing period */
(ADDR)NULL,/* 266: System Imp PF T1  */
(ADDR)NULL,/* 267: System Imp PF T2  */
(ADDR)NULL,/* 268: System Imp PF T3  */
(ADDR)NULL,/* 269: System Imp PF T4  */
(ADDR)NULL,/* 270: System Imp PF T5  */
(ADDR)NULL,/* 271: System Imp PF T6  */
(ADDR)NULL,/* 272: System Imp PF T7  */
(ADDR)NULL,/* 273: System Imp PF T8  */
(ADDR)NULL,/* 274: Signed power factor/Three phase power factor */
(ADDR)NULL,/* 275: Signed power factor/Three phase power factor */
(ADDR)NULL,/* 276: Signed power factor/Three phase power factor */
(ADDR)NULL,/* 277: Power factor imp day */
(ADDR)NULL,/* 278: Frequency */
(ADDR)NULL,/* 279: ABS Wh */
(ADDR)NULL,/* 280: NET Wh */
(ADDR)NULL,/* 281: ABS VarH */
(ADDR)NULL,/* 282: NET VarH */
(ADDR)NULL,/* 283: Current L1 */
(ADDR)NULL,/* 284: IR-block */
(ADDR)NULL,/* 285: Voltage L1 Ave 1min value  */
(ADDR)NULL,/* 286: Voltage L1 */
(ADDR)NULL,/* 287: Voltage L1 Block */
(ADDR)NULL,/* 288: Signed power factorR */
(ADDR)NULL,/* 289: Current L2 */
(ADDR)NULL,/* 290: IY-block */
(ADDR)NULL,/* 291: Voltage L2 Ave 1min value  */
(ADDR)NULL,/* 292: Voltage L2 */
(ADDR)NULL,/* 293: Voltage L2 Block */
(ADDR)NULL,/* 294: Signed power factorY */
(ADDR)NULL,/* 295: Current L3 */
(ADDR)NULL,/* 296: IB-block */
(ADDR)NULL,/* 297: Voltage L3 Ave 1min value  */
(ADDR)NULL,/* 298: Voltage L3 */
(ADDR)NULL,/* 299: Voltage L3 Block */
(ADDR)NULL,/* 300: Signed power factorB */
(ADDR)NULL,/* 301: Power factor exp month */
(ADDR)NULL,/* 302: System Exp PF T1  */
(ADDR)NULL,/* 303: System Exp PF T2  */
(ADDR)NULL,/* 304: System Exp PF T3  */
(ADDR)NULL,/* 305: System Exp PF T4  */
(ADDR)NULL,/* 306: System Exp PF T5  */
(ADDR)NULL,/* 307: System Exp PF T6  */
(ADDR)NULL,/* 308: System Exp PF T7  */
(ADDR)NULL,/* 309: System Exp PF T8  */
(ADDR)NULL,/* 310: Power factor exp block  */
(ADDR)NULL,/* 311: Power factor exp day */
(ADDR)NULL,/* 312: Neutral current */
(ADDR)NULL,/* 313: Block neutral current */
(ADDR)NULL,/* 314: Average24 neutral current */
(ADDR)NULL,/* 315: Current */
(ADDR)NULL,/* 316: Defraud Kwh */
(ADDR)NULL,/* 317: Temperature(CPU/MAG) */
(ADDR)NULL,/* 318: Current-Avearage? */
(ADDR)NULL,/* 319: Fundamental Energy kWh */
(ADDR)NULL,/* 320: Block current THD */
(ADDR)NULL,/* 321: Average24 current THD */
(ADDR)NULL,/* 322: Current TDD */
(ADDR)NULL,/* 323: Block current TDD */
(ADDR)NULL,/* 324: Average24 current TDD */
(ADDR)NULL,/* 325: Frequency L1  */
(ADDR)NULL,/* 326: Frequency L2 */
(ADDR)NULL,/* 327: Frequency L3 */
(ADDR)NULL,/* 328: Block24 Energy+ W */
(ADDR)NULL,/* 329: Block24 Energy+ Wh */
(ADDR)NULL,/* 330: Block24 Energy- W */
(ADDR)NULL,/* 331: Block24 Energy- Wh */
(ADDR)NULL,/* 332: Block24 Energy+ kvarh */
(ADDR)NULL,/* 333: Block24 Energy- kvarh */
(ADDR)NULL,/* 334: Block24 Energy+ VA */
(ADDR)NULL,/* 335: Block24 Energy+ VAh */
(ADDR)NULL,/* 336: Block24 Energy- VA */
(ADDR)NULL,/* 337: Block24 Energy- VAh */
(ADDR)NULL,/* 338: Average24 phase current */
(ADDR)NULL,/* 339: Average24 voltage */
(ADDR)NULL,/* 340: Average24 voltage THD */
(ADDR)NULL,/* 341: Apparent energy net total */
(ADDR)NULL,/* 342: Average signal strength(RSSI) */
(ADDR)NULL,/* 343: Max Demand+ kW */
(ADDR)NULL,/* 344: Max Demand+ kW TZ1 */
(ADDR)NULL,/* 345: Max Demand+ kW TZ2 */
(ADDR)NULL,/* 346: Max Demand+ kW TZ3 */
(ADDR)NULL,/* 347: Max Demand+ kW TZ4 */
(ADDR)NULL,/* 348: Max Demand+ kW TZ5 */
(ADDR)NULL,/* 349: Max Demand+ kW TZ6 */
(ADDR)NULL,/* 350: Max Demand+ kW TZ7 */
(ADDR)NULL,/* 351: Max Demand+ kW TZ8 */
(ADDR)NULL,/* 352: Max Demand- kW */
(ADDR)NULL,/* 353: Max Demand- kW TZ1 */
(ADDR)NULL,/* 354: Max Demand- kW TZ2 */
(ADDR)NULL,/* 355: Max Demand- kW TZ3 */
(ADDR)NULL,/* 356: Max Demand- kW TZ4 */
(ADDR)NULL,/* 357: Max Demand- kW TZ5 */
(ADDR)NULL,/* 358: Max Demand- kW TZ6 */
(ADDR)NULL,/* 359: Max Demand- kW TZ7 */
(ADDR)NULL,/* 360: Max Demand- kW TZ8 */
(ADDR)NULL,/* 361: Max.Demand+ var */
(ADDR)NULL,/* 362: Max.Demand- var */
(ADDR)NULL,/* 363: Max Demand+ kVA */
(ADDR)NULL,/* 364: Max Demand+ kVA TZ1 */
(ADDR)NULL,/* 365: Max Demand+ kVA TZ2 */
(ADDR)NULL,/* 366: Max Demand+ kVA TZ3 */
(ADDR)NULL,/* 367: Max Demand+ kVA TZ4 */
(ADDR)NULL,/* 368: Max Demand+ kVA TZ5 */
(ADDR)NULL,/* 369: Max Demand+ kVA TZ6 */
(ADDR)NULL,/* 370: Max Demand+ kVA TZ7 */
(ADDR)NULL,/* 371: Max Demand+ kVA TZ8 */
(ADDR)NULL,/* 372: Max Demand- kVA */
(ADDR)NULL,/* 373: Max Demand- kVA TZ1 */
(ADDR)NULL,/* 374: Max Demand- kVA TZ2 */
(ADDR)NULL,/* 375: Max Demand- kVA TZ3 */
(ADDR)NULL,/* 376: Max Demand- kVA TZ4 */
(ADDR)NULL,/* 377: Max Demand- kVA TZ5 */
(ADDR)NULL,/* 378: Max Demand- kVA TZ6 */
(ADDR)NULL,/* 379: Max Demand- kVA TZ7 */
(ADDR)NULL,/* 380: Max Demand- kVA TZ8 */
(ADDR)NULL,/* 381: Minimum Voltage0 */
(ADDR)NULL,/* 382: Minimum Voltage of L1  */
(ADDR)NULL,/* 383: Minimum Voltage of L2  */
(ADDR)NULL,/* 384: Minimum Voltage of L3  */
(ADDR)NULL,/* 385: Max24 Demand+ kW */
(ADDR)NULL,/* 386: Max24 Demand- kW */
(ADDR)NULL,/* 387: Max24 Demand+ var */
(ADDR)NULL,/* 388: Max24 Demand- var */
(ADDR)NULL,/* 389: Max24 Demand+ kVA */
(ADDR)NULL,/* 390: Max24 Demand- kVA */
(ADDR)NULL,/* 391: Name plate detail */
(ADDR)NULL,/* 392: Event1 profile */
(ADDR)NULL,/* 393: Event2 profile */
(ADDR)NULL,/* 394: Event3 profile */
(ADDR)NULL,/* 395: Event4 profile */
(ADDR)NULL,/* 396: Event5 profile */
(ADDR)NULL,/* 397: Event6 profile */
(ADDR)NULL,/* 398: Event7 profile */
(ADDR)NULL,/* 399: Relay log */
(ADDR)NULL,/* 400: Meter log */
(ADDR)NULL,/* 401: Snapshot of instantaneous parameters A1,A14 */
(ADDR)NULL,/* 402: Scaler profile-A1,A14 */
(ADDR)NULL,/* 403: Scaler profile-A2,A15 */
(ADDR)NULL,/* 404: Scaler profile-A3,A16 */
(ADDR)NULL,/* 405: Scaler profile-A4,A17 */
(ADDR)NULL,/* 406: Scaler profile-A5-A11 */
(ADDR)NULL,/* 407: Billing profile parameters */
(ADDR)NULL,/* 408: Block load profile */
(ADDR)NULL,/* 409: Daily load profile */
(ADDR)&ln1_datetime_now[0], /* 410: Date time */
(ADDR)NULL,/* 411: Date time(RTC) */
(ADDR)NULL,/* 412: MDI Reset/End of Billing Period script table */
(ADDR)NULL,/* 413: Tariffication script table */
(ADDR)NULL,/* 414: Image activation script table */
(ADDR)NULL,/* 415: Push script table */
(ADDR)NULL,/* 416: Special days table */
(ADDR)&ln1_asso_ln0[0], /* 417: Current Association */
(ADDR)&ln1_asso_ln1[0], /* 418: PC-Association */
(ADDR)NULL,/* 419: MR-Association */
(ADDR)NULL,/* 420: US-Association */
(ADDR)NULL,/* 421: PUSH-Association */
(ADDR)NULL,/* 422: FIRM-Association */
(ADDR)NULL,/* 423: IHD-Association */
(ADDR)NULL,/* 424: Image transfer */
(ADDR)NULL,/* 425: Image transfer1 */
(ADDR)NULL,/* 426: Activity calendar for time zones */
(ADDR)NULL,/* 427: Single action schedule for billing dates */
(ADDR)NULL,/* 428: Image Activation Single action schedule */
(ADDR)NULL,/* 429: Trigger SM TO HES-Single action schedule */
(ADDR)NULL,/* 430: Single action schedule for Two Way Communication  */
(ADDR)NULL,/* 431: Single action schedule for Tamper data Push */
(ADDR)NULL,/* 432: Trigger SM TO IHD-Single action schedule */
(ADDR)NULL,/* 433: Block load profile push schedule change */
(ADDR)NULL,/* 434: Daily load profile push schedule change */
(ADDR)NULL,/* 435: Billing rofile push schedule change */
(ADDR)NULL,/* 436: IEC HDLC setup */
(ADDR)NULL,/* 437: HDLC Setting(Other) */
(ADDR)NULL,/* 438: SM to HES push */
(ADDR)NULL,/* 439: SM TO IHD push */
(ADDR)NULL,/* 440: SM TO HES event push */
(ADDR)NULL,/* 441: SM to HES Load profle push */
(ADDR)NULL,/* 442: SM to HES daily profle push */
(ADDR)NULL,/* 443: Self Registration Push */
(ADDR)NULL,/* 444: SM to HES Billing profle push */
(ADDR)NULL,/* 445: SM to HES Tamper push */
(ADDR)NULL,/* 446: TCP-UDP setup */
(ADDR)NULL,/* 447: ANP setting */
(ADDR)NULL,/* 448: IPV6 setup */
(ADDR)NULL,/* 449: Setup security(For security NON) */
(ADDR)NULL,/* 450: Setup security(For security HLS) */
(ADDR)NULL,/* 451: MR-Security setup */
(ADDR)NULL,/* 452: US-Security setup */
(ADDR)NULL,/* 453: PUSH-Security setup */
(ADDR)NULL,/* 454: FIRM-Security setup */
(ADDR)NULL,/* 455: IHD-Security setup */
(ADDR)NULL,/* 456: Disconnect control */
(ADDR)NULL,/* 457: Load limit value in kW */

NULL,
};

/* HHU/VENクライアント用テーブル */
static CADR   as_ln2[] =
{
	NULL,

(ADDR)&ln2_u32_dat_rw[0], /* 1: Cumulative Billing count */
(ADDR)&ln2_billing_period[0], /* 2: Available Billing Period  */
(ADDR)NULL,/* 3: Time */
(ADDR)NULL,/* 4: Date */
(ADDR)&ln2_variable[0], /* 5: NIC SIM ID */
(ADDR)&ln2_logical_name[0], /* 6: COSEM logical device name  */
(ADDR)NULL,/* 7: Frame counter */
(ADDR)NULL,/* 8: Frame counter1 */
(ADDR)NULL,/* 9: Frame counter2 */
(ADDR)&ln2_u32_dat_r[0], /* 10: Frame counter3 */
(ADDR)&ln2_u32_dat_r[0], /* 11: Frame counter4 */
(ADDR)&ln2_u32_dat_r[0], /* 12: Frame counter5 */
(ADDR)NULL,/* 13: Frame counter6 */
(ADDR)&ln2_tamper_count[0], /* 14: Cumulative tamper count */
(ADDR)&ln2_u08_dat_rw[0], /* 15: Meter Type (1Phase/3P-3W/3P-4W) */
(ADDR)&ln2_category[0], /* 16: Category */
(ADDR)&ln2_current_rating[0], /* 17: Current rating */
(ADDR)&ln2_event_status[0], /* 18: Event status word */
(ADDR)&ln2_event_filter[0], /* 19: Event status word filter */
(ADDR)&ln2_u08_dat_rw[0], /* 20: Metering mode */
(ADDR)&ln2_u08_dat_rw[0], /* 21: Payment mode */
(ADDR)&ln2_recharge_amount[0], /* 22: Last token recharge amount */
(ADDR)&ln2_recharge_time[0], /* 23: Last token recharge time  */
(ADDR)&ln2_total_amount[0], /* 24: Total amount at last recharge  */
(ADDR)&ln2_balance_amount[0], /* 25: Current balance amount */
(ADDR)&ln2_balance_time[0], /* 26: Current balance time */
(ADDR)&ln2_serial_no[0], /* 27: Meter serial number */
(ADDR)&ln2_variable[0], /* 28: Manufacturer name */
(ADDR)&ln2_device_id[0], /* 29: Device ID */
(ADDR)&ln2_manu_year[0], /* 30: Meter year of manufacture */
(ADDR)&ln2_variable[0], /* 31: NIC IMEI number (serial number) */
(ADDR)&ln2_manu_year[0], /* 32: NIC Year of manufacture */
(ADDR)&ln2_variable[0], /* 33: NIC serial ID */
(ADDR)&ln2_variable[0], /* 34: Bluetooth mac address */
(ADDR)&ln2_variable[0], /* 35: NIC Firmware Version */
(ADDR)&ln2_prog_count[0], /* 36: Cumulative program count */
(ADDR)&ln2_power_failure[0], /* 37: Number of power failures */
(ADDR)&ln2_u08_dat_r[0], /* 38: NIC status  */
(ADDR)&ln2_u16_dat_r[0], /* 39: Event1 */
(ADDR)&ln2_u16_dat_r[0], /* 40: Event2 */
(ADDR)&ln2_u16_dat_r[0], /* 41: Event3 */
(ADDR)&ln2_u16_dat_r[0], /* 42: Event4 */
(ADDR)&ln2_u16_dat_r[0], /* 43: Event5 */
(ADDR)&ln2_u16_dat_r[0], /* 44: Event6 */
(ADDR)&ln2_u16_dat_r[0], /* 45: Event7 */
(ADDR)NULL,/* 46: Meter log code */
(ADDR)NULL,/* 47: Meter status */
(ADDR)&ln2_u32_dat_r[0], /* 48: Event1-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 49: Event2-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 50: Event3-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 51: Event4-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 52: Event5-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 53: Event6-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 54: Event7-recordNO */
(ADDR)&ln2_counter[0], /* 55: Relay Count */
(ADDR)&ln2_cellid[0], /* 56: Cell ID */
(ADDR)&ln2_4ggsm_prm[0], /* 57: 4G&NB fallback to GSM parameter */
(ADDR)&ln2_ratband[0], /* 58: RAT & Band */
(ADDR)&ln2_stamp_prm[0], /* 59: Event stamping parameter for RC/DC */
(ADDR)NULL,/* 60: RAM */
(ADDR)NULL,/* 61: EEPROM */
(ADDR)NULL,/* 62: ROM */
(ADDR)NULL,/* 63: Energy calibration */
(ADDR)NULL,/* 64: Read energy(BF)　 */
(ADDR)NULL,/* 65: Voltage and ampere calibration */
(ADDR)NULL,/* 66: Read volt and ampere(D3)　 */
(ADDR)NULL,/* 67: RTC calibration */
(ADDR)NULL,/* 68: BuildNO */
(ADDR)NULL,/* 69: BildDate */
(ADDR)NULL,/* 70: Setting bit array */
(ADDR)NULL,/* 71: Unit setting */
(ADDR)NULL,/* 72: Display time */
(ADDR)NULL,/* 73: Record setting */
(ADDR)NULL,/* 74: Power quality code define */
(ADDR)NULL,/* 75: Pulse setting */
(ADDR)NULL,/* 76: Detect event setting */
(ADDR)&ln2_server_port[0], /* 77: Server port */
(ADDR)&ln2_counter[0], /* 78: Cumulative event count */
(ADDR)&ln2_counter[0], /* 79: Cumulative tamper count 1month */
(ADDR)&ln2_counter[0], /* 80: Cumulative top cover count */
(ADDR)&ln2_timestamp[0], /* 81: Top cover tamper datetime */
(ADDR)&ln2_counter[0], /* 82: Cumulative reset count */
(ADDR)&ln2_timestamp[0], /* 83: Reset datetime */
(ADDR)&ln2_rssi[0], /* 84: NIC RSSI */
(ADDR)&ln2_rsrp[0], /* 85: RSRP */
(ADDR)&ln2_rsrq[0], /* 86: RSRQ */
(ADDR)&ln2_snr[0], /* 87: SNR */
(ADDR)&ln2_nic_vnum[0], /* 88: NIC firmware version number  */
(ADDR)&ln2_va_seipl[0], /* 89: Apparent energy computation type */
(ADDR)&ln2_lrcf[0], /* 90: LRCF */
(ADDR)NULL,/* 91: Meter type */
(ADDR)&ln2_model[0], /* 92: Product model */
(ADDR)&ln2_firm_ver[0], /* 93: Firmware version for meter */
(ADDR)NULL,/* 94: Active firmware version  */
(ADDR)NULL,/* 95: Active firmware signature  */
(ADDR)&ln2_demand_period[0], /* 96: Demand integration period */
(ADDR)&ln2_load_period[0], /* 97: Capture period-Block load profile */
(ADDR)&ln2_daily_period[0], /* 98: Capture period-Daily load profile */
(ADDR)&ln2_va_aew[0], /* 99: Apparent energy computation type */
(ADDR)NULL,/* 100: Self diagnostic details */
(ADDR)&ln2_relay_setting[0], /* 101: Current active relay time */
(ADDR)NULL,/* 102: Active relay time setting */
(ADDR)&ln2_counter[0], /* 103: Cumulative magnetic count */
(ADDR)&ln2_timestamp[0], /* 104: Magnet tamper datetime */
(ADDR)&ln2_counter[0], /* 105: Cumulative esd count */
(ADDR)&ln2_timestamp[0], /* 106: ESD tamper datetime */
(ADDR)&ln2_counter[0], /* 107: Cumulative jammer count */
(ADDR)&ln2_timestamp[0], /* 108: Jammer tamper datetime */
(ADDR)&ln2_counter[0], /* 109: Cumulative vthd count */
(ADDR)&ln2_timestamp[0], /* 110: vthd tamper datetime */
(ADDR)&ln2_counter[0], /* 111: Cumulative athd count */
(ADDR)&ln2_timestamp[0], /* 112: athd tamper datetime */
(ADDR)&ln2_counter[0], /* 113: Cumulative atdd count */
(ADDR)&ln2_timestamp[0], /* 114: atdd tamper datetime */
(ADDR)&ln2_counter[0], /* 115: Cumulative disconnect count */
(ADDR)&ln2_timestamp[0], /* 116: Disconnect datetime */
(ADDR)&ln2_counter[0], /* 117: Cumulative connect count */
(ADDR)&ln2_timestamp[0], /* 118: Connect datetime */
(ADDR)&ln2_billing_date[0], /* 119: Billing date */
(ADDR)&ln2_power_off_time[0], /* 120: Cumulative power off duration in min */
(ADDR)&ln2_billing_time[0], /* 121: Billing power on duration in min */
(ADDR)&ln2_power_on_time[0], /* 122: Cumulative power on duration in min */
(ADDR)NULL,/* 123: Battery voltage */
(ADDR)NULL,/* 124: Operating time objects  */
(ADDR)&ln2_temperature[0], /* 125: Temperature */
(ADDR)&ln2_temp_block[0], /* 126: Temperature block */
(ADDR)&ln2_temp_block[0], /* 127: Temperature block24 */
(ADDR)&ln2_magnet[0], /* 128: Magnet voltage */
(ADDR)&ln2_cputemp_block[0], /* 129: Temperature(CPU/MAG) block */
(ADDR)&ln2_cputemp_block[0], /* 130: Temperature(CPU/MAG) block24 */
(ADDR)&ln2_fwd_energy[0], /* 131: Cumulative kW MD Import  */
(ADDR)&ln2_power[0], /* 132: Active power+- */
(ADDR)&ln2_thd[0], /* 133: THD Power Phase  */
(ADDR)&ln2_fwd_energy[0], /* 134: Cumulative Energy+ Wh */
(ADDR)&ln2_fwd_energy[0], /* 135: Cumulative Energy+ Wh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 136: Cumulative Energy+ Wh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 137: Cumulative Energy+ Wh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 138: Cumulative Energy+ Wh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 139: Cumulative Energy+ Wh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 140: Cumulative Energy+ Wh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 141: Cumulative Energy+ Wh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 142: Cumulative Energy+ Wh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 143: Block Energy+ W */
(ADDR)&ln2_fwd_block_w[0], /* 144: Block Energy+ W TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 145: Block Energy+ W TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 146: Block Energy+ W TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 147: Block Energy+ W TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 148: Block Energy+ W TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 149: Block Energy+ W TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 150: Block Energy+ W TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 151: Block Energy+ W TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 152: Block Energy+ Wh */
(ADDR)&ln2_fwd_block_wh[0], /* 153: Block Energy+ Wh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 154: Block Energy+ Wh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 155: Block Energy+ Wh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 156: Block Energy+ Wh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 157: Block Energy+ Wh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 158: Block Energy+ Wh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 159: Block Energy+ Wh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 160: Block Energy+ Wh TZ8 */
(ADDR)&ln2_fwd_energy[0], /* 161: Cumulative kW MD Export */
(ADDR)&ln2_fwd_energy[0], /* 162: Cumulative Energy- Wh */
(ADDR)&ln2_fwd_energy[0], /* 163: Cumulative Energy- Wh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 164: Cumulative Energy- Wh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 165: Cumulative Energy- Wh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 166: Cumulative Energy- Wh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 167: Cumulative Energy- Wh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 168: Cumulative Energy- Wh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 169: Cumulative Energy- Wh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 170: Cumulative Energy- Wh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 171: Block Energy- W */
(ADDR)&ln2_fwd_block_w[0], /* 172: Block Energy- W TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 173: Block Energy- W TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 174: Block Energy- W TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 175: Block Energy- W TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 176: Block Energy- W TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 177: Block Energy- W TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 178: Block Energy- W TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 179: Block Energy- W TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 180: Block Energy- Wh */
(ADDR)&ln2_fwd_block_wh[0], /* 181: Block Energy- Wh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 182: Block Energy- Wh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 183: Block Energy- Wh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 184: Block Energy- Wh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 185: Block Energy- Wh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 186: Block Energy- Wh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 187: Block Energy- Wh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 188: Block Energy- Wh TZ8 */
(ADDR)&ln2_power[0], /* 189: Reactive power+- */
(ADDR)&ln2_fwd_energy[0], /* 190: Cumulative Energy+ varh */
(ADDR)&ln2_fwd_energy[0], /* 191: Cumulative Energy- varh */
(ADDR)&ln2_fwd_energy[0], /* 192: Cumulative Energy kvarh (Q1) */
(ADDR)NULL,/* 193: Block Energy+ kvarh */
(ADDR)&ln2_fwd_energy[0], /* 194: Cumulative Energy kvarh (Q2) */
(ADDR)&ln2_fwd_block_wh[0], /* 195: Block Energy kvarh (Q2) */
(ADDR)&ln2_fwd_energy[0], /* 196: Cumulative Energy kvarh (Q3) */
(ADDR)&ln2_fwd_block_wh[0], /* 197: Block Energy kvarh (Q3) */
(ADDR)&ln2_fwd_energy[0], /* 198: Cumulative Energy kvarh (Q4) */
(ADDR)NULL,/* 199: Block Energy- kvarh */
(ADDR)&ln2_fwd_energy[0], /* 200: Cumulative kVA MD Import  */
(ADDR)&ln2_power[0], /* 201: Apparent power kVA+- */
(ADDR)&ln2_fwd_energy[0], /* 202: Cumulative Energy+ kVAh */
(ADDR)&ln2_fwd_energy[0], /* 203: Cumulative Energy+ kVAh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 204: Cumulative Energy+ kVAh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 205: Cumulative Energy+ kVAh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 206: Cumulative Energy+ kVAh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 207: Cumulative Energy+ kVAh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 208: Cumulative Energy+ kVAh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 209: Cumulative Energy+ kVAh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 210: Cumulative Energy+ kVAh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 211: Block Energy+ VA */
(ADDR)&ln2_fwd_block_w[0], /* 212: Block Energy+ VA TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 213: Block Energy+ VA TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 214: Block Energy+ VA TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 215: Block Energy+ VA TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 216: Block Energy+ VA TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 217: Block Energy+ VA TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 218: Block Energy+ VA TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 219: Block Energy+ VA TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 220: Block Energy+ VAh */
(ADDR)&ln2_fwd_block_wh[0], /* 221: Block Energy+ VAh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 222: Block Energy+ VAh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 223: Block Energy+ VAh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 224: Block Energy+ VAh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 225: Block Energy+ VAh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 226: Block Energy+ VAh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 227: Block Energy+ VAh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 228: Block Energy+ VAh TZ8 */
(ADDR)&ln2_fwd_energy[0], /* 229: Cumulative kVA MD Export */
(ADDR)&ln2_fwd_energy[0], /* 230: Cumulative Energy- kVAh */
(ADDR)&ln2_fwd_energy[0], /* 231: Cumulative Energy- kVAh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 232: Cumulative Energy- kVAh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 233: Cumulative Energy- kVAh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 234: Cumulative Energy- kVAh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 235: Cumulative Energy- kVAh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 236: Cumulative Energy- kVAh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 237: Cumulative Energy- kVAh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 238: Cumulative Energy- kVAh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 239: Block Energy- VA */
(ADDR)&ln2_fwd_block_w[0], /* 240: Block Energy- VA TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 241: Block Energy- VA TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 242: Block Energy- VA TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 243: Block Energy- VA TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 244: Block Energy- VA TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 245: Block Energy- VA TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 246: Block Energy- VA TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 247: Block Energy- VA TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 248: Block Energy- VAh */
(ADDR)&ln2_fwd_block_wh[0], /* 249: Block Energy- VAh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 250: Block Energy- VAh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 251: Block Energy- VAh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 252: Block Energy- VAh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 253: Block Energy- VAh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 254: Block Energy- VAh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 255: Block Energy- VAh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 256: Block Energy- VAh TZ8 */
(ADDR)&ln2_ampr[0], /* 257: Phase current */
(ADDR)&ln2_thd[0], /* 258: THD Current  */
(ADDR)&ln2_ampr_block[0], /* 259: Block phase current */
(ADDR)&ln2_average_volt[0], /* 260: Voltage0 Ave 1min value  */
(ADDR)&ln2_volt[0], /* 261: Voltage */
(ADDR)&ln2_thd[0], /* 262: THD Voltage  */
(ADDR)&ln2_volt_block[0], /* 263: Block voltage */
(ADDR)&ln2_thd[0], /* 264: Block voltage THD */
(ADDR)&ln2_pf_block[0], /* 265: Average power factor for billing period */
(ADDR)&ln2_pf_block[0], /* 266: System Imp PF T1  */
(ADDR)&ln2_pf_block[0], /* 267: System Imp PF T2  */
(ADDR)&ln2_pf_block[0], /* 268: System Imp PF T3  */
(ADDR)&ln2_pf_block[0], /* 269: System Imp PF T4  */
(ADDR)&ln2_pf_block[0], /* 270: System Imp PF T5  */
(ADDR)&ln2_pf_block[0], /* 271: System Imp PF T6  */
(ADDR)&ln2_pf_block[0], /* 272: System Imp PF T7  */
(ADDR)&ln2_pf_block[0], /* 273: System Imp PF T8  */
(ADDR)&ln2_power_factor[0], /* 274: Signed power factor/Three phase power factor */
(ADDR)&ln2_pf_block[0], /* 275: Signed power factor/Three phase power factor */
(ADDR)&ln2_pf_block[0], /* 276: Signed power factor/Three phase power factor */
(ADDR)&ln2_pf_block[0], /* 277: Power factor imp day */
(ADDR)&ln2_freq[0], /* 278: Frequency */
(ADDR)NULL,/* 279: ABS Wh */
(ADDR)NULL,/* 280: NET Wh */
(ADDR)NULL,/* 281: ABS VarH */
(ADDR)NULL,/* 282: NET VarH */
(ADDR)NULL,/* 283: Current L1 */
(ADDR)NULL,/* 284: IR-block */
(ADDR)NULL,/* 285: Voltage L1 Ave 1min value  */
(ADDR)NULL,/* 286: Voltage L1 */
(ADDR)NULL,/* 287: Voltage L1 Block */
(ADDR)NULL,/* 288: Signed power factorR */
(ADDR)NULL,/* 289: Current L2 */
(ADDR)NULL,/* 290: IY-block */
(ADDR)NULL,/* 291: Voltage L2 Ave 1min value  */
(ADDR)NULL,/* 292: Voltage L2 */
(ADDR)NULL,/* 293: Voltage L2 Block */
(ADDR)NULL,/* 294: Signed power factorY */
(ADDR)NULL,/* 295: Current L3 */
(ADDR)NULL,/* 296: IB-block */
(ADDR)NULL,/* 297: Voltage L3 Ave 1min value  */
(ADDR)NULL,/* 298: Voltage L3 */
(ADDR)NULL,/* 299: Voltage L3 Block */
(ADDR)&ln2_power_factor[0], /* 300: Signed power factorB */
(ADDR)&ln2_pf_block[0], /* 301: Power factor exp month */
(ADDR)&ln2_pf_block[0], /* 302: System Exp PF T1  */
(ADDR)&ln2_pf_block[0], /* 303: System Exp PF T2  */
(ADDR)&ln2_pf_block[0], /* 304: System Exp PF T3  */
(ADDR)&ln2_pf_block[0], /* 305: System Exp PF T4  */
(ADDR)&ln2_pf_block[0], /* 306: System Exp PF T5  */
(ADDR)&ln2_pf_block[0], /* 307: System Exp PF T6  */
(ADDR)&ln2_pf_block[0], /* 308: System Exp PF T7  */
(ADDR)&ln2_pf_block[0], /* 309: System Exp PF T8  */
(ADDR)&ln2_pf_block[0], /* 310: Power factor exp block  */
(ADDR)&ln2_pf_block[0], /* 311: Power factor exp day */
(ADDR)&ln2_ampr[0], /* 312: Neutral current */
(ADDR)&ln2_ampr_block[0], /* 313: Block neutral current */
(ADDR)&ln2_ampr_block[0], /* 314: Average24 neutral current */
(ADDR)&ln2_ampr[0], /* 315: Current */
(ADDR)&ln2_fwd_energy[0], /* 316: Defraud Kwh */
(ADDR)&ln2_cputemp[0], /* 317: Temperature(CPU/MAG) */
(ADDR)&ln2_ampr_block[0], /* 318: Current-Avearage? */
(ADDR)&ln2_fwd_energy[0], /* 319: Fundamental Energy kWh */
(ADDR)&ln2_thd[0], /* 320: Block current THD */
(ADDR)&ln2_thd[0], /* 321: Average24 current THD */
(ADDR)&ln2_thd[0], /* 322: Current TDD */
(ADDR)&ln2_thd[0], /* 323: Block current TDD */
(ADDR)&ln2_thd[0], /* 324: Average24 current TDD */
(ADDR)&ln2_freq[0], /* 325: Frequency L1  */
(ADDR)&ln2_freq[0], /* 326: Frequency L2 */
(ADDR)&ln2_freq[0], /* 327: Frequency L3 */
(ADDR)&ln2_fwd_block_w[0], /* 328: Block24 Energy+ W */
(ADDR)&ln2_fwd_block_wh[0], /* 329: Block24 Energy+ Wh */
(ADDR)&ln2_fwd_block_w[0], /* 330: Block24 Energy- W */
(ADDR)&ln2_fwd_block_wh[0], /* 331: Block24 Energy- Wh */
(ADDR)NULL,/* 332: Block24 Energy+ kvarh */
(ADDR)NULL,/* 333: Block24 Energy- kvarh */
(ADDR)&ln2_fwd_block_w[0], /* 334: Block24 Energy+ VA */
(ADDR)&ln2_fwd_block_wh[0], /* 335: Block24 Energy+ VAh */
(ADDR)&ln2_fwd_block_w[0], /* 336: Block24 Energy- VA */
(ADDR)&ln2_fwd_block_wh[0], /* 337: Block24 Energy- VAh */
(ADDR)&ln2_ampr_block[0], /* 338: Average24 phase current */
(ADDR)&ln2_volt_block[0], /* 339: Average24 voltage */
(ADDR)&ln2_thd[0], /* 340: Average24 voltage THD */
(ADDR)&ln2_fwd_energy[0], /* 341: Apparent energy net total */
(ADDR)&ln2_rssi_block[0], /* 342: Average signal strength(RSSI) */
(ADDR)&ln2_max_fwd_demand[0], /* 343: Max Demand+ kW */
(ADDR)&ln2_max_fwd_demand[0], /* 344: Max Demand+ kW TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 345: Max Demand+ kW TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 346: Max Demand+ kW TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 347: Max Demand+ kW TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 348: Max Demand+ kW TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 349: Max Demand+ kW TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 350: Max Demand+ kW TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 351: Max Demand+ kW TZ8 */
(ADDR)&ln2_max_fwd_demand[0], /* 352: Max Demand- kW */
(ADDR)&ln2_max_fwd_demand[0], /* 353: Max Demand- kW TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 354: Max Demand- kW TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 355: Max Demand- kW TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 356: Max Demand- kW TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 357: Max Demand- kW TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 358: Max Demand- kW TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 359: Max Demand- kW TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 360: Max Demand- kW TZ8 */
(ADDR)&ln2_max_fwd_demand[0], /* 361: Max.Demand+ var */
(ADDR)&ln2_max_fwd_demand[0], /* 362: Max.Demand- var */
(ADDR)&ln2_max_fwd_demand[0], /* 363: Max Demand+ kVA */
(ADDR)&ln2_max_fwd_demand[0], /* 364: Max Demand+ kVA TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 365: Max Demand+ kVA TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 366: Max Demand+ kVA TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 367: Max Demand+ kVA TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 368: Max Demand+ kVA TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 369: Max Demand+ kVA TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 370: Max Demand+ kVA TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 371: Max Demand+ kVA TZ8 */
(ADDR)&ln2_max_fwd_demand[0], /* 372: Max Demand- kVA */
(ADDR)&ln2_max_fwd_demand[0], /* 373: Max Demand- kVA TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 374: Max Demand- kVA TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 375: Max Demand- kVA TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 376: Max Demand- kVA TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 377: Max Demand- kVA TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 378: Max Demand- kVA TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 379: Max Demand- kVA TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 380: Max Demand- kVA TZ8 */
(ADDR)&ln2_minimum_volt[0], /* 381: Minimum Voltage0 */
(ADDR)&ln2_minimum_volt[0], /* 382: Minimum Voltage of L1  */
(ADDR)&ln2_minimum_volt[0], /* 383: Minimum Voltage of L2  */
(ADDR)&ln2_minimum_volt[0], /* 384: Minimum Voltage of L3  */
(ADDR)&ln2_max_fwd_demand[0], /* 385: Max24 Demand+ kW */
(ADDR)&ln2_max_fwd_demand[0], /* 386: Max24 Demand- kW */
(ADDR)&ln2_max_fwd_demand[0], /* 387: Max24 Demand+ var */
(ADDR)&ln2_max_fwd_demand[0], /* 388: Max24 Demand- var */
(ADDR)&ln2_max_fwd_demand[0], /* 389: Max24 Demand+ kVA */
(ADDR)&ln2_max_fwd_demand[0], /* 390: Max24 Demand- kVA */
(ADDR)&ln2_name_plate[0], /* 391: Name plate detail */
(ADDR)&ln2_event_profile[0], /* 392: Event1 profile */
(ADDR)&ln2_event_profile[0], /* 393: Event2 profile */
(ADDR)&ln2_operate_profile[0], /* 394: Event3 profile */
(ADDR)&ln2_operate_profile[0], /* 395: Event4 profile */
(ADDR)&ln2_event_profile[0], /* 396: Event5 profile */
(ADDR)&ln2_operate_profile[0], /* 397: Event6 profile */
(ADDR)&ln2_operate_profile[0], /* 398: Event7 profile */
(ADDR)&ln2_relay_log[0], /* 399: Relay log */
(ADDR)NULL,/* 400: Meter log */
(ADDR)&ln2_snap_param1[0], /* 401: Snapshot of instantaneous parameters A1,A14 */
(ADDR)&ln2_scal_unit1[0], /* 402: Scaler profile-A1,A14 */
(ADDR)&ln2_scal_unit2[0], /* 403: Scaler profile-A2,A15 */
(ADDR)&ln2_scal_unit3[0], /* 404: Scaler profile-A3,A16 */
(ADDR)&ln2_scal_unit4[0], /* 405: Scaler profile-A4,A17 */
(ADDR)&ln2_scal_unit5[0], /* 406: Scaler profile-A5-A11 */
(ADDR)&ln2_billing_params[0], /* 407: Billing profile parameters */
(ADDR)&ln2_load_profile[0], /* 408: Block load profile */
(ADDR)&ln2_daily_profile[0], /* 409: Daily load profile */
(ADDR)&ln2_datetime_now[0], /* 410: Date time */
(ADDR)NULL,/* 411: Date time(RTC) */
(ADDR)&ln2_demand_reset[0], /* 412: MDI Reset/End of Billing Period script table */
(ADDR)NULL,/* 413: Tariffication script table */
(ADDR)NULL,/* 414: Image activation script table */
(ADDR)&ln2_push_scr[0], /* 415: Push script table */
(ADDR)NULL,/* 416: Special days table */
(ADDR)&ln2_asso_ln0[0], /* 417: Current Association */
(ADDR)NULL,/* 418: PC-Association */
(ADDR)&ln2_asso_ln2[0], /* 419: MR-Association */
(ADDR)NULL,/* 420: US-Association */
(ADDR)&ln2_asso_ln4[0], /* 421: PUSH-Association */
(ADDR)NULL,/* 422: FIRM-Association */
(ADDR)&ln2_asso_ln6[0], /* 423: IHD-Association */
(ADDR)NULL,/* 424: Image transfer */
(ADDR)NULL,/* 425: Image transfer1 */
(ADDR)&ln2_timezone[0], /* 426: Activity calendar for time zones */
(ADDR)&ln2_billdate_act[0], /* 427: Single action schedule for billing dates */
(ADDR)NULL,/* 428: Image Activation Single action schedule */
(ADDR)&ln2_billdate_act[0], /* 429: Trigger SM TO HES-Single action schedule */
(ADDR)&ln2_billdate_act[0], /* 430: Single action schedule for Two Way Communication  */
(ADDR)&ln2_billdate_act[0], /* 431: Single action schedule for Tamper data Push */
(ADDR)NULL,/* 432: Trigger SM TO IHD-Single action schedule */
(ADDR)&ln2_billdate_act[0], /* 433: Block load profile push schedule change */
(ADDR)&ln2_billdate_act[0], /* 434: Daily load profile push schedule change */
(ADDR)&ln2_billdate_act[0], /* 435: Billing rofile push schedule change */
(ADDR)&ln2_setup_hdlc0[0], /* 436: IEC HDLC setup */
(ADDR)NULL,/* 437: HDLC Setting(Other) */
(ADDR)&ln2_pushd_hes[0], /* 438: SM to HES push */
(ADDR)NULL,/* 439: SM TO IHD push */
(ADDR)&ln2_pushd_hes[0], /* 440: SM TO HES event push */
(ADDR)&ln2_pushd_hes[0], /* 441: SM to HES Load profle push */
(ADDR)&ln2_pushd_hes[0], /* 442: SM to HES daily profle push */
(ADDR)&ln2_pushd_hes[0], /* 443: Self Registration Push */
(ADDR)&ln2_pushd_hes[0], /* 444: SM to HES Billing profle push */
(ADDR)&ln2_pushd_hes[0], /* 445: SM to HES Tamper push */
(ADDR)&ln2_tcp_udp[0], /* 446: TCP-UDP setup */
(ADDR)NULL,/* 447: ANP setting */
(ADDR)&ln2_ipv6[0], /* 448: IPV6 setup */
(ADDR)NULL,/* 449: Setup security(For security NON) */
(ADDR)NULL,/* 450: Setup security(For security HLS) */
(ADDR)&ln2_security_mr[0], /* 451: MR-Security setup */
(ADDR)NULL,/* 452: US-Security setup */
(ADDR)&ln2_security_push[0], /* 453: PUSH-Security setup */
(ADDR)NULL,/* 454: FIRM-Security setup */
(ADDR)&ln2_security_ihd[0], /* 455: IHD-Security setup */
(ADDR)&ln2_breaker[0], /* 456: Disconnect control */
(ADDR)&ln2_load_limit[0], /* 457: Load limit value in kW */

NULL,
};

/* COM/PEAクライアント用テーブル */
static CADR   as_ln3[] =
{
	NULL,

(ADDR)&ln2_u32_dat_rw[0], /* 1: Cumulative Billing count */
(ADDR)&ln2_billing_period[0], /* 2: Available Billing Period  */
(ADDR)&ln2_time_now[0], /* 3: Time */
(ADDR)&ln2_date_now[0], /* 4: Date */
(ADDR)&ln2_variable[0], /* 5: NIC SIM ID */
(ADDR)&ln2_logical_name[0], /* 6: COSEM logical device name  */
(ADDR)&ln2_u32_dat_r[0], /* 7: Frame counter */
(ADDR)&ln2_u32_dat_r[0], /* 8: Frame counter1 */
(ADDR)&ln2_u32_dat_r[0], /* 9: Frame counter2 */
(ADDR)&ln2_u32_dat_r[0], /* 10: Frame counter3 */
(ADDR)&ln2_u32_dat_r[0], /* 11: Frame counter4 */
(ADDR)&ln2_u32_dat_r[0], /* 12: Frame counter5 */
(ADDR)&ln2_u32_dat_r[0], /* 13: Frame counter6 */
(ADDR)&ln2_tamper_count[0], /* 14: Cumulative tamper count */
(ADDR)&ln2_u08_dat_rw[0], /* 15: Meter Type (1Phase/3P-3W/3P-4W) */
(ADDR)&ln2_category[0], /* 16: Category */
(ADDR)&ln2_current_rating[0], /* 17: Current rating */
(ADDR)&ln2_event_status[0], /* 18: Event status word */
(ADDR)&ln2_event_filter[0], /* 19: Event status word filter */
(ADDR)&ln2_u08_dat_rw[0], /* 20: Metering mode */
(ADDR)&ln2_u08_dat_rw[0], /* 21: Payment mode */
(ADDR)&ln2_recharge_amount[0], /* 22: Last token recharge amount */
(ADDR)&ln2_recharge_time[0], /* 23: Last token recharge time  */
(ADDR)&ln2_total_amount[0], /* 24: Total amount at last recharge  */
(ADDR)&ln2_balance_amount[0], /* 25: Current balance amount */
(ADDR)&ln2_balance_time[0], /* 26: Current balance time */
(ADDR)&ln2_serial_no[0], /* 27: Meter serial number */
(ADDR)&ln2_variable[0], /* 28: Manufacturer name */
(ADDR)&ln2_device_id[0], /* 29: Device ID */
(ADDR)&ln2_manu_year[0], /* 30: Meter year of manufacture */
(ADDR)&ln2_variable[0], /* 31: NIC IMEI number (serial number) */
(ADDR)&ln2_manu_year[0], /* 32: NIC Year of manufacture */
(ADDR)&ln2_variable[0], /* 33: NIC serial ID */
(ADDR)&ln2_variable[0], /* 34: Bluetooth mac address */
(ADDR)&ln2_variable[0], /* 35: NIC Firmware Version */
(ADDR)&ln2_prog_count[0], /* 36: Cumulative program count */
(ADDR)&ln2_power_failure[0], /* 37: Number of power failures */
(ADDR)&ln2_u08_dat_r[0], /* 38: NIC status  */
(ADDR)&ln2_u16_dat_r[0], /* 39: Event1 */
(ADDR)&ln2_u16_dat_r[0], /* 40: Event2 */
(ADDR)&ln2_u16_dat_r[0], /* 41: Event3 */
(ADDR)&ln2_u16_dat_r[0], /* 42: Event4 */
(ADDR)&ln2_u16_dat_r[0], /* 43: Event5 */
(ADDR)&ln2_u16_dat_r[0], /* 44: Event6 */
(ADDR)&ln2_u16_dat_r[0], /* 45: Event7 */
(ADDR)&ln2_log_code[0], /* 46: Meter log code */
(ADDR)&ln2_fault[0], /* 47: Meter status */
(ADDR)&ln2_u32_dat_r[0], /* 48: Event1-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 49: Event2-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 50: Event3-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 51: Event4-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 52: Event5-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 53: Event6-recordNO */
(ADDR)&ln2_u32_dat_r[0], /* 54: Event7-recordNO */
(ADDR)&ln2_counter[0], /* 55: Relay Count */
(ADDR)&ln2_cellid[0], /* 56: Cell ID */
(ADDR)&ln2_4ggsm_prm[0], /* 57: 4G&NB fallback to GSM parameter */
(ADDR)&ln2_ratband[0], /* 58: RAT & Band */
(ADDR)&ln2_stamp_prm[0], /* 59: Event stamping parameter for RC/DC */
(ADDR)&ln2_ram[0], /* 60: RAM */
(ADDR)&ln2_e2p[0], /* 61: EEPROM */
(ADDR)&ln2_rom[0], /* 62: ROM */
(ADDR)&ln2_cal_energy[0], /* 63: Energy calibration */
(ADDR)&ln2_energy[0], /* 64: Read energy(BF)　 */
(ADDR)&ln2_cal_voltamp[0], /* 65: Voltage and ampere calibration */
(ADDR)&ln2_voltamp[0], /* 66: Read volt and ampere(D3)　 */
(ADDR)&ln2_cal_rtc[0], /* 67: RTC calibration */
(ADDR)&ln2_build_no[0], /* 68: BuildNO */
(ADDR)&ln2_build_date[0], /* 69: BildDate */
(ADDR)&ln2_userflag[0], /* 70: Setting bit array */
(ADDR)&ln2_unit0[0], /* 71: Unit setting */
(ADDR)&ln2_dispint[0], /* 72: Display time */
(ADDR)&ln2_record0[0], /* 73: Record setting */
(ADDR)&ln2_event_codes[0], /* 74: Power quality code define */
(ADDR)&ln2_setup_puls[0], /* 75: Pulse setting */
(ADDR)&ln2_detect[0], /* 76: Detect event setting */
(ADDR)&ln2_server_port[0], /* 77: Server port */
(ADDR)&ln2_counter[0], /* 78: Cumulative event count */
(ADDR)&ln2_counter[0], /* 79: Cumulative tamper count 1month */
(ADDR)&ln2_counter[0], /* 80: Cumulative top cover count */
(ADDR)&ln2_timestamp[0], /* 81: Top cover tamper datetime */
(ADDR)&ln2_counter[0], /* 82: Cumulative reset count */
(ADDR)&ln2_timestamp[0], /* 83: Reset datetime */
(ADDR)&ln2_rssi[0], /* 84: NIC RSSI */
(ADDR)&ln2_rsrp[0], /* 85: RSRP */
(ADDR)&ln2_rsrq[0], /* 86: RSRQ */
(ADDR)&ln2_snr[0], /* 87: SNR */
(ADDR)&ln2_nic_vnum[0], /* 88: NIC firmware version number  */
(ADDR)&ln2_va_seipl[0], /* 89: Apparent energy computation type */
(ADDR)&ln2_lrcf[0], /* 90: LRCF */
(ADDR)&ln2_type[0], /* 91: Meter type */
(ADDR)&ln2_model[0], /* 92: Product model */
(ADDR)&ln2_firm_ver[0], /* 93: Firmware version for meter */
(ADDR)&ln2_firm_ver1[0], /* 94: Active firmware version  */
(ADDR)&ln2_firm_sig[0], /* 95: Active firmware signature  */
(ADDR)&ln2_demand_period[0], /* 96: Demand integration period */
(ADDR)&ln2_load_period[0], /* 97: Capture period-Block load profile */
(ADDR)&ln2_daily_period[0], /* 98: Capture period-Daily load profile */
(ADDR)&ln2_va_aew[0], /* 99: Apparent energy computation type */
(ADDR)&ln2_selfdiag[0], /* 100: Self diagnostic details */
(ADDR)&ln2_relay_setting[0], /* 101: Current active relay time */
(ADDR)&ln2_relay_config[0], /* 102: Active relay time setting */
(ADDR)&ln2_counter[0], /* 103: Cumulative magnetic count */
(ADDR)&ln2_timestamp[0], /* 104: Magnet tamper datetime */
(ADDR)&ln2_counter[0], /* 105: Cumulative esd count */
(ADDR)&ln2_timestamp[0], /* 106: ESD tamper datetime */
(ADDR)&ln2_counter[0], /* 107: Cumulative jammer count */
(ADDR)&ln2_timestamp[0], /* 108: Jammer tamper datetime */
(ADDR)&ln2_counter[0], /* 109: Cumulative vthd count */
(ADDR)&ln2_timestamp[0], /* 110: vthd tamper datetime */
(ADDR)&ln2_counter[0], /* 111: Cumulative athd count */
(ADDR)&ln2_timestamp[0], /* 112: athd tamper datetime */
(ADDR)&ln2_counter[0], /* 113: Cumulative atdd count */
(ADDR)&ln2_timestamp[0], /* 114: atdd tamper datetime */
(ADDR)&ln2_counter[0], /* 115: Cumulative disconnect count */
(ADDR)&ln2_timestamp[0], /* 116: Disconnect datetime */
(ADDR)&ln2_counter[0], /* 117: Cumulative connect count */
(ADDR)&ln2_timestamp[0], /* 118: Connect datetime */
(ADDR)&ln2_billing_date[0], /* 119: Billing date */
(ADDR)&ln2_power_off_time[0], /* 120: Cumulative power off duration in min */
(ADDR)&ln2_billing_time[0], /* 121: Billing power on duration in min */
(ADDR)&ln2_power_on_time[0], /* 122: Cumulative power on duration in min */
(ADDR)&ln2_batt_volt[0], /* 123: Battery voltage */
(ADDR)&ln2_cputime[0], /* 124: Operating time objects  */
(ADDR)&ln2_temperature[0], /* 125: Temperature */
(ADDR)&ln2_temp_block[0], /* 126: Temperature block */
(ADDR)&ln2_temp_block[0], /* 127: Temperature block24 */
(ADDR)&ln2_magnet[0], /* 128: Magnet voltage */
(ADDR)&ln2_cputemp_block[0], /* 129: Temperature(CPU/MAG) block */
(ADDR)&ln2_cputemp_block[0], /* 130: Temperature(CPU/MAG) block24 */
(ADDR)&ln2_fwd_energy[0], /* 131: Cumulative kW MD Import  */
(ADDR)&ln2_power[0], /* 132: Active power+- */
(ADDR)&ln2_thd[0], /* 133: THD Power Phase  */
(ADDR)&ln2_fwd_energy[0], /* 134: Cumulative Energy+ Wh */
(ADDR)&ln2_fwd_energy[0], /* 135: Cumulative Energy+ Wh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 136: Cumulative Energy+ Wh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 137: Cumulative Energy+ Wh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 138: Cumulative Energy+ Wh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 139: Cumulative Energy+ Wh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 140: Cumulative Energy+ Wh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 141: Cumulative Energy+ Wh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 142: Cumulative Energy+ Wh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 143: Block Energy+ W */
(ADDR)&ln2_fwd_block_w[0], /* 144: Block Energy+ W TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 145: Block Energy+ W TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 146: Block Energy+ W TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 147: Block Energy+ W TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 148: Block Energy+ W TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 149: Block Energy+ W TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 150: Block Energy+ W TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 151: Block Energy+ W TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 152: Block Energy+ Wh */
(ADDR)&ln2_fwd_block_wh[0], /* 153: Block Energy+ Wh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 154: Block Energy+ Wh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 155: Block Energy+ Wh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 156: Block Energy+ Wh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 157: Block Energy+ Wh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 158: Block Energy+ Wh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 159: Block Energy+ Wh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 160: Block Energy+ Wh TZ8 */
(ADDR)&ln2_fwd_energy[0], /* 161: Cumulative kW MD Export */
(ADDR)&ln2_fwd_energy[0], /* 162: Cumulative Energy- Wh */
(ADDR)&ln2_fwd_energy[0], /* 163: Cumulative Energy- Wh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 164: Cumulative Energy- Wh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 165: Cumulative Energy- Wh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 166: Cumulative Energy- Wh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 167: Cumulative Energy- Wh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 168: Cumulative Energy- Wh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 169: Cumulative Energy- Wh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 170: Cumulative Energy- Wh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 171: Block Energy- W */
(ADDR)&ln2_fwd_block_w[0], /* 172: Block Energy- W TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 173: Block Energy- W TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 174: Block Energy- W TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 175: Block Energy- W TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 176: Block Energy- W TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 177: Block Energy- W TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 178: Block Energy- W TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 179: Block Energy- W TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 180: Block Energy- Wh */
(ADDR)&ln2_fwd_block_wh[0], /* 181: Block Energy- Wh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 182: Block Energy- Wh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 183: Block Energy- Wh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 184: Block Energy- Wh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 185: Block Energy- Wh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 186: Block Energy- Wh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 187: Block Energy- Wh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 188: Block Energy- Wh TZ8 */
(ADDR)&ln2_power[0], /* 189: Reactive power+- */
(ADDR)&ln2_fwd_energy[0], /* 190: Cumulative Energy+ varh */
(ADDR)&ln2_fwd_energy[0], /* 191: Cumulative Energy- varh */
(ADDR)&ln2_fwd_energy[0], /* 192: Cumulative Energy kvarh (Q1) */
(ADDR)&ln2_fwd_block_wh[0], /* 193: Block Energy+ kvarh */
(ADDR)&ln2_fwd_energy[0], /* 194: Cumulative Energy kvarh (Q2) */
(ADDR)&ln2_fwd_block_wh[0], /* 195: Block Energy kvarh (Q2) */
(ADDR)&ln2_fwd_energy[0], /* 196: Cumulative Energy kvarh (Q3) */
(ADDR)&ln2_fwd_block_wh[0], /* 197: Block Energy kvarh (Q3) */
(ADDR)&ln2_fwd_energy[0], /* 198: Cumulative Energy kvarh (Q4) */
(ADDR)&ln2_fwd_block_wh[0], /* 199: Block Energy- kvarh */
(ADDR)&ln2_fwd_energy[0], /* 200: Cumulative kVA MD Import  */
(ADDR)&ln2_power[0], /* 201: Apparent power kVA+- */
(ADDR)&ln2_fwd_energy[0], /* 202: Cumulative Energy+ kVAh */
(ADDR)&ln2_fwd_energy[0], /* 203: Cumulative Energy+ kVAh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 204: Cumulative Energy+ kVAh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 205: Cumulative Energy+ kVAh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 206: Cumulative Energy+ kVAh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 207: Cumulative Energy+ kVAh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 208: Cumulative Energy+ kVAh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 209: Cumulative Energy+ kVAh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 210: Cumulative Energy+ kVAh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 211: Block Energy+ VA */
(ADDR)&ln2_fwd_block_w[0], /* 212: Block Energy+ VA TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 213: Block Energy+ VA TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 214: Block Energy+ VA TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 215: Block Energy+ VA TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 216: Block Energy+ VA TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 217: Block Energy+ VA TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 218: Block Energy+ VA TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 219: Block Energy+ VA TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 220: Block Energy+ VAh */
(ADDR)&ln2_fwd_block_wh[0], /* 221: Block Energy+ VAh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 222: Block Energy+ VAh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 223: Block Energy+ VAh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 224: Block Energy+ VAh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 225: Block Energy+ VAh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 226: Block Energy+ VAh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 227: Block Energy+ VAh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 228: Block Energy+ VAh TZ8 */
(ADDR)&ln2_fwd_energy[0], /* 229: Cumulative kVA MD Export */
(ADDR)&ln2_fwd_energy[0], /* 230: Cumulative Energy- kVAh */
(ADDR)&ln2_fwd_energy[0], /* 231: Cumulative Energy- kVAh TZ1 */
(ADDR)&ln2_fwd_energy[0], /* 232: Cumulative Energy- kVAh TZ2 */
(ADDR)&ln2_fwd_energy[0], /* 233: Cumulative Energy- kVAh TZ3 */
(ADDR)&ln2_fwd_energy[0], /* 234: Cumulative Energy- kVAh TZ4 */
(ADDR)&ln2_fwd_energy[0], /* 235: Cumulative Energy- kVAh TZ5 */
(ADDR)&ln2_fwd_energy[0], /* 236: Cumulative Energy- kVAh TZ6 */
(ADDR)&ln2_fwd_energy[0], /* 237: Cumulative Energy- kVAh TZ7 */
(ADDR)&ln2_fwd_energy[0], /* 238: Cumulative Energy- kVAh TZ8 */
(ADDR)&ln2_fwd_block_w[0], /* 239: Block Energy- VA */
(ADDR)&ln2_fwd_block_w[0], /* 240: Block Energy- VA TZ1 */
(ADDR)&ln2_fwd_block_w[0], /* 241: Block Energy- VA TZ2 */
(ADDR)&ln2_fwd_block_w[0], /* 242: Block Energy- VA TZ3 */
(ADDR)&ln2_fwd_block_w[0], /* 243: Block Energy- VA TZ4 */
(ADDR)&ln2_fwd_block_w[0], /* 244: Block Energy- VA TZ5 */
(ADDR)&ln2_fwd_block_w[0], /* 245: Block Energy- VA TZ6 */
(ADDR)&ln2_fwd_block_w[0], /* 246: Block Energy- VA TZ7 */
(ADDR)&ln2_fwd_block_w[0], /* 247: Block Energy- VA TZ8 */
(ADDR)&ln2_fwd_block_wh[0], /* 248: Block Energy- VAh */
(ADDR)&ln2_fwd_block_wh[0], /* 249: Block Energy- VAh TZ1 */
(ADDR)&ln2_fwd_block_wh[0], /* 250: Block Energy- VAh TZ2 */
(ADDR)&ln2_fwd_block_wh[0], /* 251: Block Energy- VAh TZ3 */
(ADDR)&ln2_fwd_block_wh[0], /* 252: Block Energy- VAh TZ4 */
(ADDR)&ln2_fwd_block_wh[0], /* 253: Block Energy- VAh TZ5 */
(ADDR)&ln2_fwd_block_wh[0], /* 254: Block Energy- VAh TZ6 */
(ADDR)&ln2_fwd_block_wh[0], /* 255: Block Energy- VAh TZ7 */
(ADDR)&ln2_fwd_block_wh[0], /* 256: Block Energy- VAh TZ8 */
(ADDR)&ln2_ampr[0], /* 257: Phase current */
(ADDR)&ln2_thd[0], /* 258: THD Current  */
(ADDR)&ln2_ampr_block[0], /* 259: Block phase current */
(ADDR)&ln2_average_volt[0], /* 260: Voltage0 Ave 1min value  */
(ADDR)&ln2_volt[0], /* 261: Voltage */
(ADDR)&ln2_thd[0], /* 262: THD Voltage  */
(ADDR)&ln2_volt_block[0], /* 263: Block voltage */
(ADDR)&ln2_thd[0], /* 264: Block voltage THD */
(ADDR)&ln2_pf_block[0], /* 265: Average power factor for billing period */
(ADDR)&ln2_pf_block[0], /* 266: System Imp PF T1  */
(ADDR)&ln2_pf_block[0], /* 267: System Imp PF T2  */
(ADDR)&ln2_pf_block[0], /* 268: System Imp PF T3  */
(ADDR)&ln2_pf_block[0], /* 269: System Imp PF T4  */
(ADDR)&ln2_pf_block[0], /* 270: System Imp PF T5  */
(ADDR)&ln2_pf_block[0], /* 271: System Imp PF T6  */
(ADDR)&ln2_pf_block[0], /* 272: System Imp PF T7  */
(ADDR)&ln2_pf_block[0], /* 273: System Imp PF T8  */
(ADDR)&ln2_power_factor[0], /* 274: Signed power factor/Three phase power factor */
(ADDR)&ln2_pf_block[0], /* 275: Signed power factor/Three phase power factor */
(ADDR)&ln2_pf_block[0], /* 276: Signed power factor/Three phase power factor */
(ADDR)&ln2_pf_block[0], /* 277: Power factor imp day */
(ADDR)&ln2_freq[0], /* 278: Frequency */
(ADDR)&ln2_abs_energy[0], /* 279: ABS Wh */
(ADDR)&ln2_net_energy[0], /* 280: NET Wh */
(ADDR)&ln2_abs_energy[0], /* 281: ABS VarH */
(ADDR)&ln2_net_energy[0], /* 282: NET VarH */
(ADDR)&ln2_ampr[0], /* 283: Current L1 */
(ADDR)&ln2_ampr_block[0], /* 284: IR-block */
(ADDR)&ln2_average_volt[0], /* 285: Voltage L1 Ave 1min value  */
(ADDR)&ln2_volt[0], /* 286: Voltage L1 */
(ADDR)&ln2_volt_block[0], /* 287: Voltage L1 Block */
(ADDR)&ln2_power_factor[0], /* 288: Signed power factorR */
(ADDR)&ln2_ampr[0], /* 289: Current L2 */
(ADDR)&ln2_ampr_block[0], /* 290: IY-block */
(ADDR)&ln2_average_volt[0], /* 291: Voltage L2 Ave 1min value  */
(ADDR)&ln2_volt[0], /* 292: Voltage L2 */
(ADDR)&ln2_volt_block[0], /* 293: Voltage L2 Block */
(ADDR)&ln2_power_factor[0], /* 294: Signed power factorY */
(ADDR)&ln2_ampr[0], /* 295: Current L3 */
(ADDR)&ln2_ampr_block[0], /* 296: IB-block */
(ADDR)&ln2_average_volt[0], /* 297: Voltage L3 Ave 1min value  */
(ADDR)&ln2_volt[0], /* 298: Voltage L3 */
(ADDR)&ln2_volt_block[0], /* 299: Voltage L3 Block */
(ADDR)&ln2_power_factor[0], /* 300: Signed power factorB */
(ADDR)&ln2_pf_block[0], /* 301: Power factor exp month */
(ADDR)&ln2_pf_block[0], /* 302: System Exp PF T1  */
(ADDR)&ln2_pf_block[0], /* 303: System Exp PF T2  */
(ADDR)&ln2_pf_block[0], /* 304: System Exp PF T3  */
(ADDR)&ln2_pf_block[0], /* 305: System Exp PF T4  */
(ADDR)&ln2_pf_block[0], /* 306: System Exp PF T5  */
(ADDR)&ln2_pf_block[0], /* 307: System Exp PF T6  */
(ADDR)&ln2_pf_block[0], /* 308: System Exp PF T7  */
(ADDR)&ln2_pf_block[0], /* 309: System Exp PF T8  */
(ADDR)&ln2_pf_block[0], /* 310: Power factor exp block  */
(ADDR)&ln2_pf_block[0], /* 311: Power factor exp day */
(ADDR)&ln2_ampr[0], /* 312: Neutral current */
(ADDR)&ln2_ampr_block[0], /* 313: Block neutral current */
(ADDR)&ln2_ampr_block[0], /* 314: Average24 neutral current */
(ADDR)&ln2_ampr[0], /* 315: Current */
(ADDR)&ln2_fwd_energy[0], /* 316: Defraud Kwh */
(ADDR)&ln2_cputemp[0], /* 317: Temperature(CPU/MAG) */
(ADDR)&ln2_ampr_block[0], /* 318: Current-Avearage? */
(ADDR)&ln2_fwd_energy[0], /* 319: Fundamental Energy kWh */
(ADDR)&ln2_thd[0], /* 320: Block current THD */
(ADDR)&ln2_thd[0], /* 321: Average24 current THD */
(ADDR)&ln2_thd[0], /* 322: Current TDD */
(ADDR)&ln2_thd[0], /* 323: Block current TDD */
(ADDR)&ln2_thd[0], /* 324: Average24 current TDD */
(ADDR)&ln2_freq[0], /* 325: Frequency L1  */
(ADDR)&ln2_freq[0], /* 326: Frequency L2 */
(ADDR)&ln2_freq[0], /* 327: Frequency L3 */
(ADDR)&ln2_fwd_block_w[0], /* 328: Block24 Energy+ W */
(ADDR)&ln2_fwd_block_wh[0], /* 329: Block24 Energy+ Wh */
(ADDR)&ln2_fwd_block_w[0], /* 330: Block24 Energy- W */
(ADDR)&ln2_fwd_block_wh[0], /* 331: Block24 Energy- Wh */
(ADDR)&ln2_fwd_block_wh[0], /* 332: Block24 Energy+ kvarh */
(ADDR)&ln2_fwd_block_wh[0], /* 333: Block24 Energy- kvarh */
(ADDR)&ln2_fwd_block_w[0], /* 334: Block24 Energy+ VA */
(ADDR)&ln2_fwd_block_wh[0], /* 335: Block24 Energy+ VAh */
(ADDR)&ln2_fwd_block_w[0], /* 336: Block24 Energy- VA */
(ADDR)&ln2_fwd_block_wh[0], /* 337: Block24 Energy- VAh */
(ADDR)&ln2_ampr_block[0], /* 338: Average24 phase current */
(ADDR)&ln2_volt_block[0], /* 339: Average24 voltage */
(ADDR)&ln2_thd[0], /* 340: Average24 voltage THD */
(ADDR)&ln2_fwd_energy[0], /* 341: Apparent energy net total */
(ADDR)&ln2_rssi_block[0], /* 342: Average signal strength(RSSI) */
(ADDR)&ln2_max_fwd_demand[0], /* 343: Max Demand+ kW */
(ADDR)&ln2_max_fwd_demand[0], /* 344: Max Demand+ kW TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 345: Max Demand+ kW TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 346: Max Demand+ kW TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 347: Max Demand+ kW TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 348: Max Demand+ kW TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 349: Max Demand+ kW TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 350: Max Demand+ kW TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 351: Max Demand+ kW TZ8 */
(ADDR)&ln2_max_fwd_demand[0], /* 352: Max Demand- kW */
(ADDR)&ln2_max_fwd_demand[0], /* 353: Max Demand- kW TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 354: Max Demand- kW TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 355: Max Demand- kW TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 356: Max Demand- kW TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 357: Max Demand- kW TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 358: Max Demand- kW TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 359: Max Demand- kW TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 360: Max Demand- kW TZ8 */
(ADDR)&ln2_max_fwd_demand[0], /* 361: Max.Demand+ var */
(ADDR)&ln2_max_fwd_demand[0], /* 362: Max.Demand- var */
(ADDR)&ln2_max_fwd_demand[0], /* 363: Max Demand+ kVA */
(ADDR)&ln2_max_fwd_demand[0], /* 364: Max Demand+ kVA TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 365: Max Demand+ kVA TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 366: Max Demand+ kVA TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 367: Max Demand+ kVA TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 368: Max Demand+ kVA TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 369: Max Demand+ kVA TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 370: Max Demand+ kVA TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 371: Max Demand+ kVA TZ8 */
(ADDR)&ln2_max_fwd_demand[0], /* 372: Max Demand- kVA */
(ADDR)&ln2_max_fwd_demand[0], /* 373: Max Demand- kVA TZ1 */
(ADDR)&ln2_max_fwd_demand[0], /* 374: Max Demand- kVA TZ2 */
(ADDR)&ln2_max_fwd_demand[0], /* 375: Max Demand- kVA TZ3 */
(ADDR)&ln2_max_fwd_demand[0], /* 376: Max Demand- kVA TZ4 */
(ADDR)&ln2_max_fwd_demand[0], /* 377: Max Demand- kVA TZ5 */
(ADDR)&ln2_max_fwd_demand[0], /* 378: Max Demand- kVA TZ6 */
(ADDR)&ln2_max_fwd_demand[0], /* 379: Max Demand- kVA TZ7 */
(ADDR)&ln2_max_fwd_demand[0], /* 380: Max Demand- kVA TZ8 */
(ADDR)&ln2_minimum_volt[0], /* 381: Minimum Voltage0 */
(ADDR)&ln2_minimum_volt[0], /* 382: Minimum Voltage of L1  */
(ADDR)&ln2_minimum_volt[0], /* 383: Minimum Voltage of L2  */
(ADDR)&ln2_minimum_volt[0], /* 384: Minimum Voltage of L3  */
(ADDR)&ln2_max_fwd_demand[0], /* 385: Max24 Demand+ kW */
(ADDR)&ln2_max_fwd_demand[0], /* 386: Max24 Demand- kW */
(ADDR)&ln2_max_fwd_demand[0], /* 387: Max24 Demand+ var */
(ADDR)&ln2_max_fwd_demand[0], /* 388: Max24 Demand- var */
(ADDR)&ln2_max_fwd_demand[0], /* 389: Max24 Demand+ kVA */
(ADDR)&ln2_max_fwd_demand[0], /* 390: Max24 Demand- kVA */
(ADDR)&ln2_name_plate[0], /* 391: Name plate detail */
(ADDR)&ln2_event_profile[0], /* 392: Event1 profile */
(ADDR)&ln2_event_profile[0], /* 393: Event2 profile */
(ADDR)&ln2_operate_profile[0], /* 394: Event3 profile */
(ADDR)&ln2_operate_profile[0], /* 395: Event4 profile */
(ADDR)&ln2_event_profile[0], /* 396: Event5 profile */
(ADDR)&ln2_operate_profile[0], /* 397: Event6 profile */
(ADDR)&ln2_operate_profile[0], /* 398: Event7 profile */
(ADDR)&ln2_relay_log[0], /* 399: Relay log */
(ADDR)&ln2_meter_log[0], /* 400: Meter log */
(ADDR)&ln2_snap_param1[0], /* 401: Snapshot of instantaneous parameters A1,A14 */
(ADDR)&ln2_scal_unit1[0], /* 402: Scaler profile-A1,A14 */
(ADDR)&ln2_scal_unit2[0], /* 403: Scaler profile-A2,A15 */
(ADDR)&ln2_scal_unit3[0], /* 404: Scaler profile-A3,A16 */
(ADDR)&ln2_scal_unit4[0], /* 405: Scaler profile-A4,A17 */
(ADDR)&ln2_scal_unit5[0], /* 406: Scaler profile-A5-A11 */
(ADDR)&ln2_billing_params[0], /* 407: Billing profile parameters */
(ADDR)&ln2_load_profile[0], /* 408: Block load profile */
(ADDR)&ln2_daily_profile[0], /* 409: Daily load profile */
(ADDR)&ln2_datetime_now[0], /* 410: Date time */
(ADDR)&ln2_datetime_rtc[0], /* 411: Date time(RTC) */
(ADDR)&ln2_demand_reset[0], /* 412: MDI Reset/End of Billing Period script table */
(ADDR)&ln2_tariff_scr[0], /* 413: Tariffication script table */
(ADDR)&ln2_img_scr[0], /* 414: Image activation script table */
(ADDR)&ln2_push_scr[0], /* 415: Push script table */
(ADDR)&ln2_special_days[0], /* 416: Special days table */
(ADDR)&ln2_asso_ln0[0], /* 417: Current Association */
(ADDR)&ln2_asso_ln1[0], /* 418: PC-Association */
(ADDR)&ln2_asso_ln2[0], /* 419: MR-Association */
(ADDR)&ln2_asso_ln3[0], /* 420: US-Association */
(ADDR)&ln2_asso_ln4[0], /* 421: PUSH-Association */
(ADDR)&ln2_asso_ln5[0], /* 422: FIRM-Association */
(ADDR)&ln2_asso_ln6[0], /* 423: IHD-Association */
(ADDR)&ln2_img_trans[0], /* 424: Image transfer */
(ADDR)&ln2_img_trans1[0], /* 425: Image transfer1 */
(ADDR)&ln2_timezone[0], /* 426: Activity calendar for time zones */
(ADDR)&ln2_billdate_act[0], /* 427: Single action schedule for billing dates */
(ADDR)&ln2_billdate_act[0], /* 428: Image Activation Single action schedule */
(ADDR)&ln2_billdate_act[0], /* 429: Trigger SM TO HES-Single action schedule */
(ADDR)&ln2_billdate_act[0], /* 430: Single action schedule for Two Way Communication  */
(ADDR)&ln2_billdate_act[0], /* 431: Single action schedule for Tamper data Push */
(ADDR)&ln2_billdate_act[0], /* 432: Trigger SM TO IHD-Single action schedule */
(ADDR)&ln2_billdate_act[0], /* 433: Block load profile push schedule change */
(ADDR)&ln2_billdate_act[0], /* 434: Daily load profile push schedule change */
(ADDR)&ln2_billdate_act[0], /* 435: Billing rofile push schedule change */
(ADDR)&ln2_setup_hdlc0[0], /* 436: IEC HDLC setup */
(ADDR)&ln2_setup_hdlc1[0], /* 437: HDLC Setting(Other) */
(ADDR)&ln2_pushd_hes[0], /* 438: SM to HES push */
(ADDR)&ln2_pushd_hes[0], /* 439: SM TO IHD push */
(ADDR)&ln2_pushd_hes[0], /* 440: SM TO HES event push */
(ADDR)&ln2_pushd_hes[0], /* 441: SM to HES Load profle push */
(ADDR)&ln2_pushd_hes[0], /* 442: SM to HES daily profle push */
(ADDR)&ln2_pushd_hes[0], /* 443: Self Registration Push */
(ADDR)&ln2_pushd_hes[0], /* 444: SM to HES Billing profle push */
(ADDR)&ln2_pushd_hes[0], /* 445: SM to HES Tamper push */
(ADDR)&ln2_tcp_udp[0], /* 446: TCP-UDP setup */
(ADDR)&ln2_gprs[0], /* 447: ANP setting */
(ADDR)&ln2_ipv6[0], /* 448: IPV6 setup */
(ADDR)&ln2_security_none[0], /* 449: Setup security(For security NON) */
(ADDR)&ln2_security_hls[0], /* 450: Setup security(For security HLS) */
(ADDR)&ln2_security_mr[0], /* 451: MR-Security setup */
(ADDR)&ln2_security_us[0], /* 452: US-Security setup */
(ADDR)&ln2_security_push[0], /* 453: PUSH-Security setup */
(ADDR)&ln2_security_firm[0], /* 454: FIRM-Security setup */
(ADDR)&ln2_security_ihd[0], /* 455: IHD-Security setup */
(ADDR)&ln2_breaker[0], /* 456: Disconnect control */
(ADDR)&ln2_load_limit[0], /* 457: Load limit value in kW */

NULL,
};

/*************************************************************/
/**************入出力データ型定義(基本)***********************/
/*************************************************************/
CONST IDENT ID_NULL_DATA = { DT_NULL_DATA , 0, NULL };
CONST IDENT ID_ARRAY0 = { DT_ARRAY , 0, NULL };
CONST IDENT ID_STRUCTURE0 = { DT_STRUCTURE , 0, NULL };
CONST IDENT ID_BOOLEAN = { DT_BOOLEAN , 1, NULL };
CONST IDENT ID_BIT_STRING0 = { DT_BIT_STRING , 0, NULL };
CONST IDENT ID_BIT_STRING8 = { DT_BIT_STRING , 8, NULL };
CONST IDENT ID_BIT_STRING24 = { DT_BIT_STRING , 24, NULL };
CONST IDENT ID_BIT_STRING32 = { DT_BIT_STRING , 24, NULL };
CONST IDENT ID_BIT_STRING128 = { DT_BIT_STRING , 128, NULL };
CONST IDENT ID_BIT_STRING512 = { DT_BIT_STRING , 512, NULL };
CONST IDENT ID_DOUBLE_LONG = { DT_DOUBLE_LONG , 4, NULL };
CONST IDENT ID_DOUBLE_LONG_UNSIGNED = { DT_DOUBLE_LONG_UNSIGNED , 4, NULL };
CONST IDENT ID_OCTET_STRING0 = { DT_OCTET_STRING , 0, NULL };
CONST IDENT ID_OCTET_STRING1 = { DT_OCTET_STRING , 1, NULL };
CONST IDENT ID_OCTET_STRING2 = { DT_OCTET_STRING , 2, NULL };
CONST IDENT ID_OCTET_STRING4 = { DT_OCTET_STRING , 4, NULL };
CONST IDENT ID_OCTET_STRING5 = { DT_OCTET_STRING , 5, NULL };
CONST IDENT ID_OCTET_STRING6 = { DT_OCTET_STRING , 6, NULL };
CONST IDENT ID_OCTET_STRING7 = { DT_OCTET_STRING , 7, NULL };
CONST IDENT ID_OCTET_STRING8 = { DT_OCTET_STRING , 8, NULL };
CONST IDENT ID_OCTET_STRING10 = { DT_OCTET_STRING , 10, NULL };
CONST IDENT ID_OCTET_STRING12 = { DT_OCTET_STRING , 12, NULL };
CONST IDENT ID_OCTET_STRING16 = { DT_OCTET_STRING , 16, NULL };
CONST IDENT ID_OCTET_STRING18 = { DT_OCTET_STRING , 18, NULL };
CONST IDENT ID_OCTET_STRING24 = { DT_OCTET_STRING , 24, NULL };
CONST IDENT ID_OCTET_STRING31 = { DT_OCTET_STRING , 31, NULL };
CONST IDENT ID_OCTET_STRING32 = { DT_OCTET_STRING , 32, NULL };
CONST IDENT ID_OCTET_STRING48 = { DT_OCTET_STRING , 32, NULL };
CONST IDENT ID_OCTET_STRING64 = { DT_OCTET_STRING , 64, NULL };
CONST IDENT ID_OCTET_STRING110 = { DT_OCTET_STRING , 110, NULL };
CONST IDENT ID_OCTET_STRING128 = { DT_OCTET_STRING , 128, NULL };
CONST IDENT ID_OCTET_STRING512 = { DT_OCTET_STRING , 512, NULL };
CONST IDENT ID_VISIBLE_STRING0 = { DT_VISIBLE_STRING , 0, NULL };
CONST IDENT ID_VISIBLE_STRING1 = { DT_VISIBLE_STRING , 1, NULL };
CONST IDENT ID_VISIBLE_STRING2 = { DT_VISIBLE_STRING , 2, NULL };
CONST IDENT ID_VISIBLE_STRING3 = { DT_VISIBLE_STRING , 3, NULL };
CONST IDENT ID_VISIBLE_STRING4 = { DT_VISIBLE_STRING , 4, NULL };
CONST IDENT ID_VISIBLE_STRING6 = { DT_VISIBLE_STRING , 6, NULL };
CONST IDENT ID_VISIBLE_STRING8 = { DT_VISIBLE_STRING , 8, NULL };
CONST IDENT ID_VISIBLE_STRING10 = { DT_VISIBLE_STRING , 10, NULL };
CONST IDENT ID_VISIBLE_STRING11 = { DT_VISIBLE_STRING , 11, NULL };
CONST IDENT ID_VISIBLE_STRING12 = { DT_VISIBLE_STRING , 12, NULL };
CONST IDENT ID_VISIBLE_STRING13 = { DT_VISIBLE_STRING , 13, NULL };
CONST IDENT ID_VISIBLE_STRING14 = { DT_VISIBLE_STRING , 14, NULL };
CONST IDENT ID_VISIBLE_STRING15 = { DT_VISIBLE_STRING , 15, NULL };
CONST IDENT ID_VISIBLE_STRING16 = { DT_VISIBLE_STRING , 16, NULL };
CONST IDENT ID_VISIBLE_STRING17 = { DT_VISIBLE_STRING , 17, NULL };
CONST IDENT ID_VISIBLE_STRING20 = { DT_VISIBLE_STRING , 20, NULL };
CONST IDENT ID_VISIBLE_STRING30 = { DT_VISIBLE_STRING , 30, NULL };
CONST IDENT ID_VISIBLE_STRING40 = { DT_VISIBLE_STRING , 40, NULL };
CONST IDENT ID_UTF8_STRING0 = { DT_UTF8_STRING , 0, NULL };
CONST IDENT ID_BCD0 = { DT_BCD , 0, NULL };
CONST IDENT ID_BCD1 = { DT_BCD , 1, NULL };
CONST IDENT ID_BCD2 = { DT_BCD , 2, NULL };
CONST IDENT ID_INTEGER = { DT_INTEGER , 1, NULL };
CONST IDENT ID_LONG = { DT_LONG , 2, NULL };
CONST IDENT ID_UNSIGNED = { DT_UNSIGNED , 1, NULL };
CONST IDENT ID_LONG_UNSIGNED = { DT_LONG_UNSIGNED , 2, NULL };
CONST IDENT ID_LONG64 = { DT_LONG64 , 8, NULL };
CONST IDENT ID_LONG64_UNSIGNED = { DT_LONG64_UNSIGNED , 8, NULL };
CONST IDENT ID_ENUM = { DT_ENUM , 1, NULL };
CONST IDENT ID_FLOAT32 = { DT_FLOAT32 , 4, NULL };
CONST IDENT ID_FLOAT64 = { DT_FLOAT64 , 8, NULL };
CONST IDENT ID_DATE_TIME = { DT_DATE_TIME , 12, NULL };
CONST IDENT ID_DATE = { DT_DATE , 5, NULL };
CONST IDENT ID_TIME = { DT_TIME , 4, NULL };
CONST IDENT ID_GMAC = { DT_OCTET_STRING ,LEN_CMD + LEN_FRAME_COUNTER + LEN_AUTH_TAG, NULL };
#if 1
#define DT_WATVARVA	 ID_FLOAT32
#define DT_WATVARVAH ID_FLOAT32
#define DT_VOLT ID_FLOAT32
#define DT_AMP ID_FLOAT32
#define DT_POWER_FACTOR ID_FLOAT32
#define DT_FREQ ID_FLOAT32
#define DT_TEMP ID_FLOAT32
#define DT_FFT ID_FLOAT32
#define DT_ON_TIME ID_FLOAT32
#define DT_SIGNED_BLOCK ID_LONG_UNSIGNED
#define DT_UNSIGNED_BLOCK ID_LONG_UNSIGNED
#else
#define DT_WATVARVA	 ID_DOUBLE_LONG_UNSIGNED
#define DT_WATVARVAH ID_DOUBLE_LONG_UNSIGNED
#define DT_VOLT ID_LONG_UNSIGNED
#define DT_AMP ID_LONG
#define DT_POWER_FACTOR ID_LONG
#define DT_FREQ ID_LONG_UNSIGNED
#define DT_TEMP ID_LONG
#define DT_FFT ID_LONG_UNSIGNED
#define DT_ON_TIME ID_LONG_UNSIGNED
#define DT_SIGNED_BLOCK ID_LONG
#define DT_UNSIGNED_BLOCK ID_LONG_UNSIGNED
#endif
#pragma section

void set_axdr(cpui08 in, pui08 out, cuint buffer_size) {
	in_offset = 0U;
	ixdlms = (pui08)in;
	out_offset = 0U;
	oxdlms = (pui08)out;
	axdr_max = buffer_size;
}

static void atz(pui08 out, pui08 in, ui16 len) {

	ui08 buf[8];
	ramcpy(buf, in, len);
	switch (len) {
	default:
	case 1:
		out[0] = buf[0];
		break;
	case 2:
		out[0] = buf[1];
		out[1] = buf[0];
		break;
	case 3:
		out[0] = buf[2];
		out[1] = buf[1];
		out[2] = buf[0];
		break;
	case 4:
		out[0] = buf[3];
		out[1] = buf[2];
		out[2] = buf[1];
		out[3] = buf[0];
		break;
	case 5:
		out[0] = buf[4];
		out[1] = buf[3];
		out[2] = buf[2];
		out[3] = buf[1];
		out[4] = buf[0];
		break;
	case 6:
		out[0] = buf[5];
		out[1] = buf[4];
		out[2] = buf[3];
		out[3] = buf[2];
		out[4] = buf[1];
		out[5] = buf[0];
		break;
	case 7:
		out[0] = buf[6];
		out[1] = buf[5];
		out[2] = buf[4];
		out[3] = buf[3];
		out[4] = buf[2];
		out[5] = buf[1];
		out[6] = buf[0];
		break;
	case 8:
		out[0] = buf[7];
		out[1] = buf[6];
		out[2] = buf[5];
		out[3] = buf[4];
		out[4] = buf[3];
		out[5] = buf[2];
		out[6] = buf[1];
		out[7] = buf[0];
		break;
	}
}
/****************************************************************/
/***************************** put ******************************/
/****************************************************************/
/****************************************************************/
/*																*/
/*	関数名				:固定長データ型応答生成					*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void pt_fx(CADR prm)	/* fix length type */
{
	uint len;

	len = (((PIDNT)prm)->l);
	if (axdr_max > (len + 1)) {
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		ramcpy(&oxdlms[out_offset + 1], &ixdlms[in_offset], len);
		in_offset += len;
		len++;
		out_offset += len;
		axdr_max -= len;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}

void pt_ufxAtZ(CADR prm)	/* fix length type */
{
	uint len;
	DAT8 in;

	len = (((PIDNT)prm)->l);
	if (axdr_max > (len + 1)) {
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		if (in_size == 0) {
			in_size = len;
		}
		in.ll = 0;
		switch (in_size) {
		case 1:
			in.by[0] = ixdlms[in_offset + 0];
			break;
		case 2:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			break;
		default:
		case 4:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			break;
		case 8:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			in.by[4] = ixdlms[in_offset + 4];
			in.by[5] = ixdlms[in_offset + 5];
			in.by[6] = ixdlms[in_offset + 6];
			in.by[7] = ixdlms[in_offset + 7];
			break;
		}
		abs64(&in);
		atz(&oxdlms[out_offset + 1], in.by, len);
		in_offset += in_size;
		len++;
		out_offset += len;
		axdr_max -= len;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}

void pt_fxAtZ(CADR prm)	/* fix length type */
{
	uint len;
	DAT8 in;

	len = (((PIDNT)prm)->l);
	if (axdr_max > (len + 1)) {
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		if (in_size == 0) {
			in_size = len;
		}
		in.ll = 0;
		switch (in_size) {
		case 1:
			in.by[0] = ixdlms[in_offset + 0];
			if ((in.by[0] & 0x80) > 0) {
				in.by[1] = 0xff;
				in.hf[1] = 0xffff;
				in.hf[2] = 0xffff;
				in.hf[3] = 0xffff;
			}
			break;
		case 2:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			if ((in.by[1] & 0x80) > 0) {
				in.hf[1] = 0xffff;
				in.hf[2] = 0xffff;
				in.hf[3] = 0xffff;
			}
			break;
		default:
		case 4:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			if ((in.by[3] & 0x80) > 0) {
				in.hf[2] = 0xffff;
				in.hf[3] = 0xffff;
			}
			break;
		case 8:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			in.by[4] = ixdlms[in_offset + 4];
			in.by[5] = ixdlms[in_offset + 5];
			in.by[6] = ixdlms[in_offset + 6];
			in.by[7] = ixdlms[in_offset + 7];
			break;
		}
		atz(&oxdlms[out_offset + 1], in.by, len);
		in_offset += in_size;
		len++;
		out_offset += len;
		axdr_max -= len;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}

void pt_fxfloat32(CADR prm)	/* fix length type */
{
	uint	len;
	ui08	flg;
	char	i;
	float	ft;
	DAT4	in;

	len = (((PIDNT)prm)->l);
	if (axdr_max > (len + 1)) {
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		in.wd = 0;
		if (in_size == 0) {
			in_size = len;
		}
		switch (in_size) {
		case 1:
			in.by[0] = ixdlms[in_offset + 0];
			if ((in.by[0] & 0x80) > 0) {
				in.by[1] = 0xff;
				in.by[2] = 0xff;
				in.by[3] = 0xff;
			}
			break;
		case 2:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			if ((in.by[1] & 0x80) > 0) {
				in.by[2] = 0xff;
				in.by[3] = 0xff;
			}
			break;
		default:
		case 4:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			break;
		}
		flg = abs32(&in.wd);
		if (unit_scale[idxdat].scale > 0) {
			for (i = 0; i < unit_scale[idxdat].scale; i++) {
				in.wd /= 10;
			}
		}
		ft = in.wd;
		if (unit_scale[idxdat].scale < 0) {
			for (i = unit_scale[idxdat].scale; i < 0; i++) {
				ft /= 10.0;
			}
		}
		if (flg > 0) {
			ft = 0.0 - ft;
		}
		atz(&oxdlms[out_offset + 1], (pui08)&ft, sizeof(float));
		in_offset += in_size;
		len++;
		out_offset += len;
		axdr_max -= len;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}

void pt_fxfloat64(CADR prm)	/* fix length type */
{
	uint	len;
	ui08	flg;
	char	i;
	double	ft;
	DAT8	in;

	len = (((PIDNT)prm)->l);
	if (axdr_max > (len + 1)) {
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		in.ll = 0;
		if (in_size == 0) {
			in_size = len;
		}
		switch (in_size) {
		case 1:
			in.by[0] = ixdlms[in_offset + 0];
			if ((in.by[0] & 0x80) > 0) {
				in.by[1] = 0xff;
				in.hf[1] = 0xffff;
				in.hf[2] = 0xffff;
				in.hf[3] = 0xffff;
			}
			break;
		case 2:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			if ((in.by[1] & 0x80) > 0) {
				in.hf[1] = 0xffff;
				in.hf[2] = 0xffff;
				in.hf[3] = 0xffff;
			}
			break;
		case 4:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			if ((in.by[3] & 0x80) > 0) {
				in.hf[2] = 0xffff;
				in.hf[3] = 0xffff;
			}
			break;
		default:
		case 8:
			in.by[0] = ixdlms[in_offset + 0];
			in.by[1] = ixdlms[in_offset + 1];
			in.by[2] = ixdlms[in_offset + 2];
			in.by[3] = ixdlms[in_offset + 3];
			in.by[4] = ixdlms[in_offset + 4];
			in.by[5] = ixdlms[in_offset + 5];
			in.by[6] = ixdlms[in_offset + 6];
			in.by[7] = ixdlms[in_offset + 7];
			break;
		}
		flg = abs64(&in);
		if (unit_scale[idxdat].scale > 0) {
			for (i = 0; i < unit_scale[idxdat].scale; i++) {
				in.ll /= 10;
			}
		}
		ft = in.ll;
		if (unit_scale[idxdat].scale < 0) {
			for (i = unit_scale[idxdat].scale; i < 0; i++) {
				ft /= 10.0;
			}
		}
		if (flg > 0) {
			ft = 0.0 - ft;
		}
		atz(&oxdlms[out_offset + 1], (pui08)&ft, sizeof(double));
		in_offset += in_size;
		len++;
		out_offset += len;
		axdr_max -= len;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}
/****************************************************************/
/*																*/
/*	関数名				:BITSTRINGデータ型応答生成				*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void pt_bt(CADR prm)	/* bit string type */
{
	uint len, lenbit, offset;

	lenbit = ((PIDNT)prm)->l;
	len = (lenbit >> 3) + ((lenbit % 8) > 0 ? 1 : 0);
	if (axdr_max > (len + 4)) {
		offset = out_offset;
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		out_offset++;
		encode_len(oxdlms, &out_offset, lenbit);
		ramcpy(&oxdlms[out_offset], &ixdlms[in_offset], len);
		in_offset += len;
		out_offset += len;
		axdr_max -= out_offset - offset;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}

/****************************************************************/
/*																*/
/*	関数名				:可変長データ型応答生成					*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void pt_vr(CADR prm)	/* veriable length type */
{
	uint len, offset;

	len = ((PIDNT)prm)->l;
	if (axdr_max > (len + 4)) {
		offset = out_offset;
		oxdlms[out_offset] = ((PIDNT)prm)->t;
		out_offset++;
		if (len == 0) {
			len = in_size;
		}
		encode_len(oxdlms, &out_offset, len);
		ramcpy(&oxdlms[out_offset], &ixdlms[in_offset], len);
		in_offset += len;
		out_offset += len;
		axdr_max -= out_offset - offset;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
}

/****************************************************************/
/*																*/
/*	関数名				:COMPLEX型応答生成						*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 ppos;
ui08 pidx[16]; 
void put(CADR prm);
void pt_cx(CADR prm)	/* complex type */
{
	uint len, offset;

	if (axdr_max > 4) {
		ppos++;
		if (pidx[ppos] == 0xff) {
			oxdlms[out_offset++] = ((PIDNT)prm)->t;
			axdr_max--;
		}
		if (NULL == ((PIDNT)prm)->e) {
			ppos--;
			return;								/* テンプレートが無ければ、その時点のOFFSETを返して終了 */
		}
		len = ((PIDNT)prm)->l;
#if 0
		if (128 < len) {
			/* Fatal : mutiple members */
			axdr_max = 0U;						/* 0xffを超えると再帰が長くなるので、エラーにする。 */
			return;
		}
#endif
		if (pidx[ppos] == 0xff) {
			offset = out_offset;
			encode_len(oxdlms, &out_offset, len);
			axdr_max -= out_offset - offset;
			pidx[ppos] = 0;
		}
		for (; pidx[ppos] < len; pidx[ppos]++) {
			if (DT_ARRAY == ((PIDNT)prm)->t) {
				in_size = 0;
				put(((PIDNT)prm)->e);					/* ARRAYの場合は、固定のデータ型を使用 */
			}
			else {
				in_size = 0;
				put(((PADR)((PIDNT)prm)->e)[pidx[ppos]]);	/* ARRAY以外は、可変データ型を使用 */
			}
			if (axdr_max == 0) {
				break;
			}
		}
		if (pidx[ppos] == len) {
			pidx[ppos] = 0xff;
		}
		ppos--;
	}
	else {
		/* Fatal : buffer overflow */
		axdr_max = 0U;
	}
	return;
}

/****************************************************************/
/*	データ型-応答生成関数マップ			*/
/****************************************************************/
static const  FNCCAD data_put[] = {
	pt_fx,		/* 0 DT_NULL_DATA */
	pt_cx,		/* 1 DT_ARRAY */
	pt_cx,		/* 2 DT_STRUCTURE */
	pt_fx,		/* 3 DT_BOOLEAN */
	pt_bt,		/* 4 DT_BIT_STRING */
	pt_fxAtZ,	/* 5 DT_DOUBLE_LONG */
	pt_ufxAtZ,	/* 6 DT_DOUBLE_LONG_UNSIGNED */
	NULL,		/* 7 */
	NULL,		/* 8 */
	pt_vr,		/* 9 DT_OCTET_STRING */
	pt_vr,		/* 10 DT_VISIBLE_STRING */
	NULL,		/* 11 */
	pt_vr,		/* 12 DT_UTF8_STRING */
	pt_vr,		/* 13 DT_BCD */
	NULL,		/* 14 */
	pt_fxAtZ,	/* 15 DT_INTEGER */
	pt_fxAtZ,	/* 16 DT_LONG */
	pt_ufxAtZ,	/* 17 DT_UNSIGNED */
	pt_ufxAtZ,	/* 18 DT_LONG_UNSIGNED */
	NULL,		/* 19 */
	pt_fxAtZ,	/* 20 DT_LONG64 */
	pt_ufxAtZ,	/* 21 DT_LONG64_UNSIGNED */
	pt_fxAtZ,	/* 22 DT_ENUM */
	pt_fxfloat32,	/* 23 DT_FLOAT32 */
	pt_fxfloat64,	/* 24 DT_FLOAT64 */
	pt_fx,		/* 25 DT_DATE_TIME */
	pt_fx,		/* 26 DT_DATE */
	pt_fx,		/* 27 DT_TIME */
};

/****************************************************************/
/*																*/
/*	関数名				:応答生成								*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void put(CADR prm)
{
	PIDNT ident;
	
	ident = (PIDNT)prm;
	data_put[ident->t](prm);
}


/**************************************************/
/***************** take　**************************/
/**************************************************/
/****************************************************************/
/*																*/
/*	関数名				:固定長データ型要求取得					*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 tk_fx(CADR prm)
{
	uint len, offset;

	offset = in_offset;		/* オフセット位置をコピーしておく。 */
	if (ixdlms[offset] == ((PIDNT)prm)->t)
	{
		len = ((PIDNT)prm)->l;
		offset++;
		if (axdr_max < (offset + len))
		{
			/* Error : incorrect data length */
			return FALSE;
		}
		if (0 < len)
		{
			ramcpy(&oxdlms[out_offset], &ixdlms[offset], len);
			out_offset += len;
			offset += len;
		}
		in_offset = offset;
		return TRUE;
	}
	/* Error : incorrect data type */
	return FALSE;
}

ui08 tk_fxAtZ(CADR prm)
{
	uint len, offset;

	offset = in_offset;		/* オフセット位置をコピーしておく。 */
	if (ixdlms[offset] == ((PIDNT)prm)->t)
	{
		len = ((PIDNT)prm)->l;
		offset++;
		if (axdr_max < (offset + len))
		{
			/* Error : incorrect data length */
			return FALSE;
		}
		if (0 < len)
		{
			atz(&oxdlms[out_offset], &ixdlms[offset], len);
			out_offset += len;
			offset += len;
		}
		in_offset = offset;
		return TRUE;
	}
	/* Error : incorrect data type */
	return FALSE;
}
/****************************************************************/
/*																*/
/*	関数名				:可変長データ型要求取得					*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 tk_bt(CADR prm)
{
	uint len, lenbit, offset;

	lenbit = ((PIDNT)prm)->l;
	len = (lenbit >> 3) + ((lenbit % 8) > 0 ? 1U : 0U);
	offset = in_offset;		/* オフセット位置をコピーしておく。 */
	if (ixdlms[offset] == ((PIDNT)prm)->t)
	{
		offset++;
		if (axdr_max < (offset + len))
		{
			/* Error : incorrect data length */
			return FALSE;
		}
		if (lenbit == decode_len(&offset, ixdlms))
		{
			if (0 < lenbit)
			{
				ramcpy(&oxdlms[out_offset], &ixdlms[offset], len);
				out_offset += len;
				offset += len;
			}
			in_offset = offset;
			return TRUE;
		}
	}
	/* Error : incorrect data type */
	return FALSE;
}
/****************************************************************/
/*																*/
/*	関数名				:可変長データ型要求取得					*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 tk_vr(CADR prm)
{
	uint len, offset, size;

	offset = in_offset;		/* オフセット位置をコピーしておく。 */
	if (ixdlms[offset] == ((PIDNT)prm)->t) {
		offset++;
		len = ((PIDNT)prm)->l;
		size = decode_len(&offset, ixdlms);
		if (0 < size) {
			if (len > 0) {
				ramcpy(&oxdlms[out_offset], &ixdlms[offset], size);
				out_offset += size;
			}
			else {	/*len=0の場合はサイズを含める*/
				len = size + (offset - (in_offset + 1));
				ramcpy(&oxdlms[out_offset], &ixdlms[in_offset + 1], len);
				out_offset += size + 1;
			}
			offset += size;
		}
		in_offset = offset;
		return TRUE;
	}
	/* Error : incorrect data type */
	return FALSE;
}
/****************************************************************/
/*																*/
/*	関数名				:COMPLEX型要求取得						*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 take(CADR prm);
ui08 tk_cx(CADR prm)
{
	uint	i, len, offset, count;
	ADDR	p;

	offset = in_offset;
	if (ixdlms[offset] == ((PIDNT)prm)->t) {
		offset++;
		len = ((PIDNT)prm)->l;
		if ((NULL == ((PIDNT)prm)->e) || (0U == len)) {
			in_offset = offset;
			return TRUE;
		}
		/* データ型が決定している場合のみ、データを取得する。 */
		count = decode_len(&offset, ixdlms);
		if ((128 < count) || (0U == count)) {
			/* Error : biggest data length!!! */
			return FALSE;
		}
		if (len < count) {
			/* Error : incorrect data length */
			return FALSE;
		}
		if (DT_ARRAY == ((PIDNT)prm)->t) {
			array_count[arr_pos] = count;
			arr_pos++;
		}
		in_offset = offset;
		for (i = 0; i < count; i++) {
			p = NULL;
			if (DT_ARRAY == ((PIDNT)prm)->t) {
				p = ((PIDNT)prm)->e;
			}
			else {
				p = ((PADR)((PIDNT)prm)->e)[i];
			}
			if (NULL == p) {
				return TRUE;
			}
			if (FALSE == take(p)) {
				return FALSE;
			}
		}
		return TRUE;
	}
	/* Error : incorrect data type */
	return FALSE;
}

/****************************************************************/
/*	データ型-要求取得関数マップ			*/
/****************************************************************/
static const  ARG data_take[] = {
	tk_fx,		/* 0 DT_NULL_DATA */
	tk_cx,		/* 1 DT_ARRAY */
	tk_cx,		/* 2 DT_STRUCTURE */
	tk_fx,		/* 3 DT_BOOLEAN */
	tk_bt,		/* 4 DT_BIT_STRING */
	tk_fxAtZ,	/* 5 DT_DOUBLE_LONG */
	tk_fxAtZ,	/* 6 DT_DOUBLE_LONG_UNSIGNED */
	NULL,		/* 7 */
	NULL,		/* 8 */
	tk_vr,		/* 9 DT_OCTET_STRING */
	tk_vr,		/* 10 DT_VISIBLE_STRING */
	NULL,		/* 11 */
	tk_vr,		/* 12 DT_UTF8_STRING */
	tk_vr,		/* 13 DT_BCD */
	NULL,		/* 14 */
	tk_fx,		/* 15 DT_INTEGER */
	tk_fxAtZ,	/* 16 DT_LONG */
	tk_fx,		/* 17 DT_UNSIGNED */
	tk_fxAtZ,	/* 18 DT_LONG_UNSIGNED */
	NULL,		/* 19 */
	tk_fxAtZ,	/* 20 DT_LONG64 */
	tk_fxAtZ,	/* 21 DT_LONG64_UNSIGNED */
	tk_fxAtZ,	/* 22 DT_ENUM */
	tk_fx,		/* 23 DT_FLOAT32 */
	tk_fx,		/* 24 DT_FLOAT64 */
	tk_fx,		/* 25 DT_DATE_TIME */
	tk_fx,		/* 26 DT_DATE */
	tk_fx,		/* 27 DT_TIME */
};
/****************************************************************/
/*																*/
/*	関数名				:要求取得								*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 take(CADR prm)
{
	ui08 res;

	res = data_take[((PIDNT)prm)->t](prm);

	return res;
}

/****************************************************************/
/*																*/
/*	関数名				: Attribute応答生成						*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui16 put_attr(cpui08 in, cui08 size, CADR ident) {

	if (stream == 0) {
		set_axdr(in, &g_raw[g_raw_len], apdu_size - g_raw_len - 32);
	}
	else {
		set_axdr(in, g_raw, 0x1000);
	}
	if (block_no == 0) {
		ramset(pidx, 0xff, 10);
		ppos = 0;
	}
	in_size = size;
	put(ident);
	g_raw_len += out_offset;
	if (stream > 0) {
		stream0_write(g_raw, out_offset);
	}
	if (axdr_max == 0) {
		if (0 == block_no) {
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
	}
	else {
		last_block = TRUE;
	}
	return in_offset;
}

ui16 put_attr_non(cpui08 in, cui08 size, CADR ident)
{
	if (stream == 0) {
		set_axdr(in, &g_raw[g_raw_len], apdu_size - g_raw_len - 32);
	}
	else {
		set_axdr(in, g_raw, 0x1000);
	}
	if (block_no == 0) {
		ramset(pidx, 0xff, sizeof(pidx));
		ppos = 0;
	}
	in_size = size;
	put(ident);
	g_raw_len += out_offset;
	if (stream > 0) {
		stream0_write(g_raw, out_offset);
	}
	return in_offset;
}

void put_complex_count(cuint count, CPIDT ident)
{
	if (stream == 0) {
		set_axdr(NULL, &g_raw[g_raw_len], apdu_size - g_raw_len - 32);
	}
	else {
		set_axdr(NULL, g_raw, 0x1000);
	}
	if (block_no == 0) {
		ramset(pidx, 0xff, 10);
		ppos = 0;
	}
	in_size = 0;
	put(ident);
	g_raw_len += out_offset;
	if (stream > 0) {
		stream0_write(g_raw, out_offset);
	}
	if (axdr_max == 0) {
		if (0 == block_no) {
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
	}
	else {
		last_block = TRUE;
	}
	if (axdr_max > 4)
	{
		if (stream > 0) {
			out_offset = 0;
			encode_len(g_raw, &out_offset, count);
			stream0_write(g_raw, out_offset);
			g_raw_len += out_offset;
		}
		else {
			encode_len(g_raw, &g_raw_len, count);
		}
	}
	else
	{
		/* Fatal : incorrect remain raw */
		axdr_max = 0U;
	}
}

void put_complex_count_non(cuint count, CPIDT ident)
{
	if (stream == 0) {
		set_axdr(NULL, &g_raw[g_raw_len], apdu_size - g_raw_len - 32);
	}
	else {
		set_axdr(NULL, g_raw, 0x1000);
	}
	if (block_no == 0) {
		ramset(pidx, 0xff, sizeof(pidx));
		ppos = 0;
	}
	in_size = 0;
	put(ident);
	g_raw_len += out_offset;
	if (stream > 0) {
		stream0_write(g_raw, out_offset);
	}
	if (axdr_max > 4) {
		if (stream > 0) {
			out_offset = 0;
			encode_len(g_raw, &out_offset, count);
			stream0_write(g_raw, out_offset);
			g_raw_len += out_offset;
		}
		else {
			encode_len(g_raw, &g_raw_len, count);
		}
	}
	else
	{
		/* Fatal : incorrect remain raw */
		axdr_max = 0U;
	}
}

static	ui08 get_value_thread(void) {

	ui16 ret;

	ret = getset(cur_inst, obis_code[5], g_xdlms_io);
	if (ret != 0xffff) {
		put_attr(g_xdlms_io, ret, cur_ident);
		return PHASE_RESPONSE;
	}
	else {
		return PHASE_THREAD;
	}
}

void put_value(cui08 attr, cui16 ist, CADR ident) {

	ui08 ret;

	ramset(g_xdlms_io, 0, 16);	/*サイズコントロルしているため*/
	idxdat = idxobj;
	switch (attr) {
	case 2:
		getset = get_value2;
		break;
	case 3:
		getset = get_value3;
		break;
	case 4:
		getset = get_value4;
		break;
	case 5:
		getset = get_value5;
		break;
	case 6:
		getset = get_value6;
		break;
	case 7:
		getset = get_value7;
		break;
	case 8:
		getset = get_value8;
		break;
	case 9:
		getset = get_value9;
		break;
	case 10:
		getset = get_value10;
		break;
	}
	ret = getset(ist, obis_code[5], g_xdlms_io);
	if (ret != 0xffff) {
		put_attr(g_xdlms_io, ret, ident);
		xdlms_phase = PHASE_RESPONSE;
	}
	else {
		cur_inst = ist;
		cur_ident = ident;
		thread_process = get_value_thread;
		xdlms_phase = PHASE_THREAD;
	}
}

/****************************************************************/
/*																*/
/*	関数名				: Attribute要求取得						*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 take_attr(CADR ident)
{
	set_axdr(&g_xdlms_req.buf[g_xdlms_req.pos], g_xdlms_io, g_xdlms_req.len - g_xdlms_req.pos);
	arr_pos = 0;
	if (FALSE == take(ident))
	{
		/* Fatal : incorrect remain raw */
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return FALSE;
	}
	g_xdlms_req.pos += in_offset;
	return TRUE;
}

void take_value2(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value2(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value3(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value3(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value4(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value4(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value5(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value5(ist, obis_code[5], g_xdlms_io);
	}
}
void take_value6(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value6(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value7(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value7(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value8(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value8(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value9(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value9(ist, obis_code[5], g_xdlms_io);
	}
}

void take_value10(cui16 ist, CADR ident)
{
	if (TRUE == take_attr(ident)) {
		set_value10(ist, obis_code[5], g_xdlms_io);
	}
}
void take_value_boolean(cui16 ist)
{
	ui08 t;

	if (TRUE == take_attr((ADDR)&ID_BOOLEAN)) {
		t = (FALSE == (*g_xdlms_io)) ? FALSE : 1;
		set_value2(ist, obis_code[5], &t);	/* fx, vr　 */
	}
}

/****************************************************************/
/*																*/
/*	関数名				:計量ユニット値(BOOL型)要求取得			*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/

ui08 isAssociating(void) {
	
	ui08 ret;
	if (as_ln0 == NULL) {
		ret = FALSE;
	}
	else {
		ret = TRUE;
	}
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:年が製造年内なのかをチェックする。		*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckYear(cpui08 pdate)
{
	ui16 from, to;
	ui16 utmp16 = UI16L(pdate);

	calender(&from, &to);

	if (utmp16 < from)
	{
		/* Error : out of begin year */
		return FALSE;					/* 開始、終了年以外はFALSE */
	}
	if (utmp16 > to)
	{
		/* Error : out of year */
		return FALSE;					/* 開始、終了年以外はFALSE */
	}
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:月が月内なのかをチェックする。			*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckMonth(cpui08 pdate)
{
	if (0U == *(pdate + 2))
	{
		/* Error : out of begin month */
		return FALSE;					/* 0 or 12より大きければFALSE */
	}

	if (12 < *(pdate + 2))
	{
		/* Error : out of month */
		return FALSE;					/* 0 or 12より大きければFALSE */
	}
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:月の最終日を取得する。					*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 uGetLastDayOfMonth(cpui08 pdate)
{
	ui08	day;
	day = LASTDAY[*(pdate + 2) - 1];
	if ((2 == *(pdate + 2)) && (0U == (UI16L(pdate) % 4)))
	{
		day = 29;
	}
	return day;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:日が月の日内なのかをチェックする。	*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckDayOfMonth(cpui08 pdate)
{
	ui08 day;

	day = uGetLastDayOfMonth(pdate);
	if (0U == *(pdate + 3))
	{
		/* Error : out of begin day of Month */
		return FALSE;					/* 　dayより大きい、もしくは0ならFALSE */
	}
	if (day < *(pdate + 3))
	{
		/* Error : out of day of Month */
		return FALSE;					/* 　dayより大きい、もしくは0ならFALSE */
	}
	return TRUE;
}

#if 0	/* DATE_DAYOFWEEK */
/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
/* 曜日が曜日内なのかをチェックする。 */
ui08 bCheckDayOfWeek(cpui08 pdate)
{
	if ((0U == *(pdate + 4)) || (7 < *(pdate + 4)))
	{
		/* Error : out of dayofWeek */
		return FALSE;					/* 7より大きい、もしくは0ならFALSE */
	}
	return TRUE;
}
#endif	/* DATE_DAYOFWEEK */

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*		日付をチェックする。 */
/*		ワイルドカードを使わない場合、年、月、日がカレンダー日付であることをチェックする。 */
/*		ワイルドカードの場合は、カレンダーの日付であること、もしくは、ワイルドカードであることをチェックする。 */
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckDate(cpui08 pdate, cui08 allow_wild)
{
	ui08 res1, res2;

	res1 = bCheckYear(pdate);
	res2 = bCheckMonth(pdate);
	if (res1 && res2)
	{
		if (TRUE == bCheckDayOfMonth(pdate))
		{
			return TRUE;
		}
	}
	if (allow_wild)	/* res1 == FALSE or res2 == FALSE or day == FALSE */
	{
		if ((FALSE == res1) && (0xffff != UI16L(pdate)))
		{
			/* Error : invalid calender year */
			return FALSE; /* カレンダー通りの年でもなく、ワイルドカードでもない */
		}
		if ((FALSE == res2) && (0xff != *(pdate + 2)))
		{
			/* Error : invalid calender month */
			return FALSE; /* カレンダー通りの月でもなく、ワイルドカードでもない */
		}
		if (0xfd <= *(pdate + 3)) /* 年もしくは月が不定なのでWildカードの日のみ指定可 */
		{
			return TRUE;
		}
	}
	/* Error : invalid date */
	return FALSE;/* ワイルドカード不可でカレンダー通りの日ではない。または、ワイルドカード可でワイルドカードではない日を指定 */
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckHour(cpui08 time)
{
	if (23 < *(time + 0))
	{
		/* Error : invalid hour */
		return FALSE;					/* 23より大きいならFALSE */
	}
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckMinute(cpui08 time)
{
	if (59 < *(time + 1))
	{
		/* Error : invalid minute */
		return FALSE;					/* 59より大きいならFALSE */
	}
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckSecond(cpui08 time)
{
	if (59 < *(time + 2))
	{
		/* Error : invalid second */
		return FALSE;					/* 59より大きいならFALSE */
	}
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckTime(cpui08 time, cui08 allow_wild)
{
	ui08 res1, res2, res3;

	res1 = bCheckHour(time);
	res2 = bCheckMinute(time);
	res3 = bCheckSecond(time);

	if (res1 && res2 && res3) {
		return TRUE;
	}
	if (allow_wild)
	{
		if ((FALSE == res1) && (0xff != *(time + 0)))
		{
			/* Error : invalid wildcard hour */
			return FALSE;
		}
		if ((FALSE == res2) && (0xff != *(time + 1)))
		{
			/* Error : invalid wildcard minute */
			return FALSE;
		}
		if ((FALSE == res3) && (0xff != *(time + 2)))
		{
			/* Error : invalid wildcard second */
			return FALSE;
		}
		return TRUE;
	}
	/* Error : invalid time */
	return FALSE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 bCheckDateTime(cpui08 date_time, cui08 allow_wild)
{
	if (FALSE == bCheckDate(date_time, allow_wild))
	{
		return FALSE;
	}
	if (FALSE == bCheckTime((date_time + 5), allow_wild))
	{
		return FALSE;
	}
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:1バイト値大小比較						*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 compare_u08(cpui08 from, cpui08 to)
{
	if (!((*((pui08)from)) == (*((pui08)to))))
	{
		if ((*((pui08)from)) < (*((pui08)to)))
		{
			return CMP_VALID;
		}
		else
		{
			/* from is bigger(newer) than to */
			return CMP_INVALID;
		}
	}
	return CMP_MATCH;
}
/****************************************************************/
/*																*/
/*	関数名				:2バイト値大小比較						*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 compare_u16(cpui08 from, cpui08 to) {
	ui16	f, t;

	f = UI16B(from);
	t = UI16B(to);

	if (!(f == t))
	{
		if (f < t)
		{
			return CMP_VALID;
		}
		else
		{
			/* from is bigger(newer) than to */
			return CMP_INVALID;
		}
	}
	return CMP_MATCH;
}

/****************************************************************/
/*																*/
/*	関数名				:4バイト値大小比較					*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 compare_u32(cpui08 from, cpui08 to) {
	ui08	ret;
	ui32	f, t;

	f = UI32B(from);
	t = UI32B(to);

	if (!(f == t))
	{
		if (f < t)
		{
			ret = CMP_VALID;
		}
		else
		{
			/* from is bigger(newer) than to */
			ret = CMP_INVALID;
		}
	}
	else
	{
		ret = CMP_MATCH;
	}
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名				:日付時新旧比較							*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
/*
	日付の大小を比較する。
	NotSpecifiedが設定されていた場合は比較しない。
 */
ui08 compare_date(cpui08 from, cpui08 to)
{
	ui08 r = CMP_VALID;
#if 0
	if (FALSE == bCheckDate(from, FALSE))
	{
		/* invalid from date */
		return CMP_INVALID;
	}
	if (FALSE == bCheckDate(to, FALSE))
	{
		/* invalid to date */
		return CMP_INVALID;
	}
#endif
	r = compare_u16(&from[0], &to[0]);
	if (!(CMP_MATCH == r))
	{
		return r;
	}
	r = compare_u08(&from[2], &to[2]);
	if (!(CMP_MATCH == r))
	{
		return r;
	}
	r = compare_u08(&from[3], &to[3]);
	if (!(CMP_MATCH == r))
	{
		return r;
	}
	return r;
}

/****************************************************************/
/*																*/
/*	関数名				:時間新旧比較							*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 compare_time(cpui08 from, cpui08 to)
{
	ui08 r = CMP_VALID;
#if 0
	if (FALSE == bCheckTime(from, FALSE))
	{
		/* invalid from time */
		return CMP_INVALID;
	}
	if (FALSE == bCheckTime(to, FALSE))
	{
		/* invalid to time */
		return CMP_INVALID;
	}
#endif
	r = compare_u08(&from[0], &to[0]);
	if (!(CMP_MATCH == r))
	{
		return r;
	}
	r = compare_u08(&from[1], &to[1]);
	if (!(CMP_MATCH == r))
	{
		return r;
	}
	r = compare_u08(&from[2], &to[2]);
	if (!(CMP_MATCH == r))
	{
		return r;
	}
	return r;
}

/*******************************************/
/************ interface class 8 ************/
/*******************************************/
/* データ型定義 */
CADR  datetime_now[] = {
	(ADDR)&ID_OCTET_STRING12,/* attr 2 */
	(ADDR)&ID_LONG,			/* attr 3 */
	(ADDR)&ID_UNSIGNED,		/* attr 4 */
	(ADDR)&ID_OCTET_STRING12,	/* attr 5 */
	(ADDR)&ID_OCTET_STRING12,	/* attr 6 */
	(ADDR)&ID_INTEGER,		/* attr 7 */
	(ADDR)&ID_BOOLEAN,		/* attr 8 */
	(ADDR)&ID_ENUM,			/* attr 9 */
};

CADR  datetime_now_atr2 = (ADDR)&ID_OCTET_STRING12;	/* OCTET STRING型の日時 */
#ifdef ALL_CLOCK_ATTRIBUTE
cui08f INVALID_DATETIME[] = { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x00,0x00 };
cui08 I(8, 3, datetime_now)[] = { 0x80, 0x00 };	/* 3. time_zone */
cpui08  I(8, 5, datetime_now) = INVALID_DATETIME;/* 5. daylight_savings_begin */
cpui08  I(8, 6, datetime_now) = INVALID_DATETIME;/* 6. daylight_savings_end */
cui08 I(8, 7, datetime_now) = 0x00;		/* 7. daylight_savings_deviation */
cui08 I(8, 8, datetime_now) = 0x00;		/* 8. daylight_savings_enabled */
#endif
pui08 get_meter_datetime(void)
{
	get_value2(IST_DATETIME_NOW, 0xff, I(8, 2, datetime_now));
	return ((pui08)I(8, 2, datetime_now));
}

void G(0, 8, 2, datetime_now)(void)	/* 現在日時 備考: 計量ユニットへ問い合わせ */
{
	put_value( 2, IST_DATETIME_NOW, (ADDR)&ID_OCTET_STRING12);
}

void S(0, 8, 2, datetime_now)(void)
{
	ui08 res;

	set_axdr(&g_xdlms_req.buf[g_xdlms_req.pos], g_xdlms_io, g_xdlms_req.len - g_xdlms_req.pos);	/* 取得位置のセット */
	if (FALSE == take((ADDR)datetime_now[0]))			/* DATATIME型データ取得 */
	{
		if (FALSE == take((ADDR)datetime_now_atr2))	/* OCTETSTRING型データ取得 */
		{
			/* Fatal : incorrect remain raw */
			record_log(ETSN1, 20);
			result_xdlms = RET_TYPE_UNMATCHED;			/* データ型不一致 */
			return;
		}
	}
	g_xdlms_req.pos += in_offset;					/* 取得位置オフセット移動 */

	res = bCheckDateTime(g_xdlms_io, FALSE);
	if (FALSE == res)		/* カレンダ日時のチェック */
	{
		/* Fatal : incorrect remain raw */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;			/* データ型不一致 */
	}
	else
	{
		set_value2(IST_DATETIME_NOW, obis_code[5], g_xdlms_io);
	}
	return;
}

#ifdef ALL_CLOCK_ATTRIBUTE
void G(0, 8, 3, datetime_now)(void)	/* 3. time_zone */
{

	put_attr(I(8, 3, datetime_now), 0,(ADDR)datetime_now[1]);
}

void G(0, 8, 4, datetime_now)(void)	/* 4. status */
{
	put_attr(&get_meter_datetime()[11], 0,(ADDR)datetime_now[2]);
}
void G(0, 8, 5, datetime_now)(void)	/* 5. daylight_savings_begin */
{

	put_attr((pui08)I(8, 5, datetime_now), 0,(ADDR)datetime_now[3]);
}

void G(0, 8, 6, datetime_now)(void)	/* 6. daylight_savings_end */
{
	put_attr(I(8, 6, datetime_now), 0,(ADDR)datetime_now[4]);
}

void G(0, 8, 7, datetime_now)(void)	/* 7. daylight_savings_deviation */
{
	put_attr(&I(8, 7, datetime_now), 0,(ADDR)datetime_now[5]);
}

void G(0, 8, 8, datetime_now)(void)	/* 8. daylight_savings_enabled */
{
	put_attr(&I(8, 8, datetime_now), 0,(ADDR)datetime_now[6]);
}
#endif

/*******************************************/
/************ interface class 8 ************/
/*******************************************/
/* データ型定義 */
pui08 get_meter_datetime1(void)
{
	get_value2(IST_DATETIME_RTC, 0xff, g_xdlms_io);
	return ((pui08)I(8, 2, datetime_now));
}

void G(0, 8, 2, datetime_rtc)(void)	/* 現在日時 備考: 計量ユニットへ問い合わせ */
{
	put_value( 2, IST_DATETIME_RTC, (ADDR)&ID_OCTET_STRING12);
}
/* SET関数 */
/* ACTION関数 */
/*********************end of interface class 8 ****************************/

/****************************************************************/
/*																*/
/*	関数名	:init_system_title									*/
/*																*/
/*	内容		:システムタイトル成形									*/
/*																*/
/*																*/
/*	引　数	:なし												*/
/*																*/
/*	戻り値	:なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 						*/
/*																*/
/****************************************************************/
void init_system_title(void)
{
	DAT8 no;

	SYSTEM_TITLE[0] = FLAGS[0];
	SYSTEM_TITLE[1] = FLAGS[1];
	SYSTEM_TITLE[2] = FLAGS[2];
	no.ll = 0;
	get_value2(IST_SERIAL_NO, 0xff, g_xdlms_io);	/*8桁*/
	g_xdlms_io[0] = 0x30;
	g_xdlms_io[1] = 0x30;
	g_xdlms_io[2] = 0x30;
	asc10Tohex64(no.by, &g_xdlms_io[1]);
#if 1	
	SYSTEM_TITLE[3] = no.by[4];
	SYSTEM_TITLE[4] = no.by[3]; 
	SYSTEM_TITLE[5] = no.by[2];
	SYSTEM_TITLE[6] = no.by[1];
	SYSTEM_TITLE[7] = no.by[0];
#else
	SYSTEM_TITLE[3] = 0x30;
	SYSTEM_TITLE[4] = 0x30; 
	SYSTEM_TITLE[5] = 0x30;
	SYSTEM_TITLE[6] = 0x30;
	SYSTEM_TITLE[7] = 0x30;
#endif
	delay = no.wd[0] % 300;
}

static void put_value_with_block(ui16 ist, CPIDT ar, CPIDT st) {

	ui16 i, offset, size;

	/* バッファーに登録数をセット */
	if (0 == block_no) {		/* 最初の応答 */
		if (obis_code[5] != 0xff) {
			sent_count = obis_code[5];
			send_count = 0;
			record_count = sent_count + 1;
			put_complex_count(1, &ID_ARRAY0);
		}
		else {
			sent_count = 0;
			send_count = 0;
			record_count = ar->l;
			put_complex_count(record_count, &ID_ARRAY0);
		}
		offset = 0;
	}
	else{
		offset = in_offset;
	}
	for (i = sent_count; i < record_count; i++) {
		size = getset(ist, i, g_xdlms_io);
		put_attr(&g_xdlms_io[offset], size,(ADDR)st);
		if (axdr_max == 0) {
			break;
		}
		else {
			offset = 0;
			send_count++;
		}
	}
	sent_count += send_count;
	send_count = 0;
	if (0 == block_no){		/* 最初の応答 */
		if (record_count > sent_count) {
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
	}
	if (record_count <= sent_count) {
		last_block = TRUE;	/* 件数が超えた場合も最終フラグON */
	}
}

static void take_value_with_block(ui16 ist, CPIDT ar, CPIDT st) {

	ui16 i;

	if (1 >= block_no) {		/* 最初の応答 */
		sent_count = 0;
		send_count = 0;
		record_count = ar->l;
		if (g_xdlms_req.buf[g_xdlms_req.pos] != DT_ARRAY) {
			result_xdlms = RET_TYPE_UNMATCHED;
			return;
		}
		g_xdlms_req.pos++;
		record_count = decode_len(&g_xdlms_req.pos, g_xdlms_req.buf);
		if (record_count != ar->l) {
			result_xdlms = RET_TYPE_UNMATCHED;
			return;
		}
	}
	for (i = sent_count; i < record_count; i++) {

		if (TRUE == take_attr((ADDR)st)) {
			getset(ist, i, g_xdlms_io);
			send_count++;
			if (g_xdlms_req.pos >= g_xdlms_req.len) {
				break;
			}
		}
		else {
			break;
		}
	}
	sent_count += send_count;
	if (last_block > 0) {
		if (sent_count < record_count) {
			result_xdlms = RET_NO_LONG_SET_IN_PROGRESS;
		}
	}
	else {
		if (sent_count >= record_count) {
			result_xdlms = RET_NO_LONG_SET_IN_PROGRESS;
		}
	}
	send_count = 0;
}

/*******************************************/
/************ interface class 1 ************/
/*******************************************/
/* データ型定義 */
CADR  firm_ver_list[] = {
	(ADDR)&ID_DOUBLE_LONG,
	(ADDR)&ID_VISIBLE_STRING11,
	(ADDR)&ID_VISIBLE_STRING8,
	(ADDR)&ID_BCD1
};
CONST IDENT firm_ver_st = { DT_STRUCTURE, 4, (ADDR)firm_ver_list };
CADR  firm_ver1 = (ADDR)&firm_ver_st;

CADR  firm_sig_list[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING24,
	(ADDR)&ID_OCTET_STRING24
};
CONST IDENT firm_sig_st = { DT_STRUCTURE, 3, (ADDR)firm_sig_list };
//CADR  firm_sig = (ADDR)&firm_sig_st;
CADR  firm_sig = (ADDR)&ID_OCTET_STRING4;

CADR  unit0_list[] = {
	(ADDR)&ID_OCTET_STRING4,
	(ADDR)&ID_OCTET_STRING4,
	(ADDR)&ID_OCTET_STRING4,
	(ADDR)&ID_OCTET_STRING4,
	(ADDR)&ID_OCTET_STRING4,
	(ADDR)&ID_OCTET_STRING4,
};
CONST IDENT unit0_st = { DT_STRUCTURE, 6, (ADDR)unit0_list };
CADR  unit0 = (ADDR)&unit0_st;

CADR  recmap_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED
};
CONST IDENT recmap_st = { DT_STRUCTURE, 4, (ADDR)recmap_list };

CADR  reccfg_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&recmap_st,
	(ADDR)&recmap_st,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
};
CONST IDENT record0_st = { DT_STRUCTURE, 9, (ADDR)reccfg_list };
CONST IDENT record0_ar = { DT_ARRAY, 6, (ADDR)&record0_st };
CADR  record0 = (ADDR)&record0_ar;

CONST IDENT calb_tbl = { DT_ARRAY, 28, (ADDR)&ID_LONG_UNSIGNED };
CADR  calb_rtc0_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&calb_tbl,
	(ADDR)&ID_LONG_UNSIGNED,
};
CONST IDENT calb_rtc0_st = { DT_STRUCTURE, 8, (ADDR)calb_rtc0_list };
CADR  cal_rtc = (ADDR)&calb_rtc0_st;

CADR  calb_jiki0_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED
};
CONST IDENT calb_jiki0_st = { DT_STRUCTURE, 4, (ADDR)calb_jiki0_list };
CADR  calb_jiki0 = (ADDR)&calb_jiki0_st;

CADR  detect_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED
};
CONST IDENT detect_st = { DT_STRUCTURE, 4, (ADDR)detect_list };
CONST IDENT detect_ar = { DT_ARRAY, 32, (ADDR)&detect_st };
CADR  detect = (ADDR)&detect_ar;

CADR  setup_puls_list[] = {
	(ADDR)&ID_UNSIGNED,	/*mode*/
	(ADDR)&ID_UNSIGNED,	/*puls*/
	(ADDR)&ID_UNSIGNED,	/*source*/
	(ADDR)&ID_UNSIGNED,	/*thrd0*/
	(ADDR)&ID_UNSIGNED, /*thrd1*/
	(ADDR)&ID_UNSIGNED,	/*thrd2*/
	(ADDR)&ID_UNSIGNED, /*thrd3*/
	(ADDR)&ID_UNSIGNED,	/*thrd4*/
	(ADDR)&ID_LONG_UNSIGNED,	/*width*/
};
CONST IDENT setup_puls_st = { DT_STRUCTURE, 9, (ADDR)setup_puls_list };
CADR  setup_puls = (ADDR)&setup_puls_st;

CADR  voltamp_list[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,/*v1*/
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,/*V2*/
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,/*V3*/
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,/*i1*/
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,/*i2*/
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,/*i3*/
};
CONST IDENT voltamp_st = { DT_STRUCTURE, 6, (ADDR)voltamp_list };
CADR  voltamp = (ADDR)&voltamp_st;

CADR  cal_energy_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
};
CONST IDENT cal_energy_st = { DT_STRUCTURE, 12, (ADDR)cal_energy_list };
CADR  cal_energy = (ADDR)&cal_energy_st;

CADR  cal_voltamp_list[] = {
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
};
CONST IDENT cal_voltamp_st = { DT_STRUCTURE, 12, (ADDR)cal_voltamp_list };
CADR  cal_voltamp = (ADDR)&cal_voltamp_st;
CADR  sel_wrrd_mem_list[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING0,
};
CONST IDENT sel_wrrd_mem = { DT_STRUCTURE, 3, (ADDR)sel_wrrd_mem_list };

CADR  relay_setting_list[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED
};
CONST IDENT relay_setting = { DT_STRUCTURE, 3, (ADDR)relay_setting_list };

CADR  relay_config_list[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_OCTET_STRING12
};
CONST IDENT relay_config = { DT_STRUCTURE, 4, (ADDR)relay_config_list };


/****************************************************************/
#define cl1_put_value(ist, type, INST) \
void G(0,1,2,ist)( void )\
{\
	put_value( 2,  IST_##INST, type );\
}

void G(0, 1, 2, logical_name)(void)		/* COSEM Logical device name */
{
	put_attr(get_system_title(), 0,(ADDR)&ID_OCTET_STRING8);
}

void G(0, 1, 2, frame_counter)(void) {
	put_value( 2, idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 1, 2, eventn)(void) {
	put_value( 2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void G(0, 1, 2, eventn_rno)(void) {
	put_value(2, idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 1, 2, counter)(void) {
	put_value( 2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void G(0, 1, 2, timestamp)(void) {
	put_value(2, idxobj, (ADDR)&ID_OCTET_STRING12);
}

void G(0, 1, 2, u08_dat_r)(void) {
	put_value(2, idxobj, (ADDR)&ID_UNSIGNED);
}
void G(0, 1, 2, u16_dat_r)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void G(0, 1, 2, u32_dat_r)(void) {
	put_value(2, idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}
void G(0, 1, 2, u08_dat_rw)(void) {
	put_value(2, idxobj, (ADDR)&ID_UNSIGNED);
}
void G(0, 1, 2, u16_dat_rw)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void G(0, 1, 2, u32_dat_rw)(void) {
	put_value(2, idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}
void G(0, 1, 2, variable)(void) {
	put_value(2, idxobj, (ADDR)&ID_VISIBLE_STRING0);
}

cl1_put_value(billing_count, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, BILLING_COUNT)
cl1_put_value(billing_period, (ADDR)&ID_UNSIGNED, BILLING_PERIOD)
cl1_put_value(tamper_count, (ADDR)&ID_LONG_UNSIGNED, TAMPER_COUNT)
cl1_put_value(meter_type, (ADDR)&ID_UNSIGNED, METER_TYPE)
cl1_put_value(lrcf, (ADDR)&ID_OCTET_STRING2, LRCF)
cl1_put_value(category, (ADDR)&ID_VISIBLE_STRING2, CATEGORY)
cl1_put_value(current_rating, (ADDR)&ID_VISIBLE_STRING10, CURRENT_RATING)
cl1_put_value(event_status, (ADDR)&ID_BIT_STRING128, EVENT_STATUS)
cl1_put_value(event_filter, (ADDR)&ID_BIT_STRING128, EVENT_FILTER)
cl1_put_value(meter_mode, (ADDR)&ID_UNSIGNED, METER_MODE)
cl1_put_value(payment, (ADDR)&ID_UNSIGNED, PAYMENT)
cl1_put_value(va_seipl, (ADDR)&ID_UNSIGNED, VA_SEIPL)
cl1_put_value(va_aew, (ADDR)&ID_UNSIGNED, VA_AEW)
cl1_put_value(recharge_amount, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, RECHARGE_AMOUNT)
cl1_put_value(recharge_time, (ADDR)&ID_OCTET_STRING12, RECHARGE_TIME)
cl1_put_value(total_amount, (ADDR)&ID_DOUBLE_LONG, TOTAL_AMOUNT)
cl1_put_value(balance_amount, (ADDR)&ID_DOUBLE_LONG, BALANCE_AMOUNT)
cl1_put_value(balance_time, (ADDR)&ID_OCTET_STRING12, BALANCE_TIME)
cl1_put_value(serial_no, (ADDR)&ID_VISIBLE_STRING11, SERIAL_NO)
cl1_put_value(manufacture_name, (ADDR)&ID_VISIBLE_STRING30, MANUFACTURE_NAME)
cl1_put_value(device_id, (ADDR)&ID_VISIBLE_STRING14, DEVICE_ID)
cl1_put_value(meter_year, (ADDR)&ID_LONG_UNSIGNED, METER_YEAR)
cl1_put_value(nic_serial, (ADDR)&ID_VISIBLE_STRING10, NIC_SERIAL)
cl1_put_value(nic_imei, (ADDR)&ID_VISIBLE_STRING15, NIC_IMEI)
cl1_put_value(mac_addr, (ADDR)&ID_VISIBLE_STRING17, MAC_ADDR)
cl1_put_value(nic_vnum, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, NIC_VNUM)
cl1_put_value(rssi, (ADDR)&ID_UNSIGNED, RSSI)
cl1_put_value(rsrp, (ADDR)&ID_VISIBLE_STRING4, RSRP)
cl1_put_value(rsrq, (ADDR)&ID_VISIBLE_STRING4, RSRQ)
cl1_put_value(snr, (ADDR)&ID_VISIBLE_STRING4, SNR)
cl1_put_value(prog_count, (ADDR)&ID_LONG_UNSIGNED, PROG_COUNT)
cl1_put_value(power_failure, (ADDR)&ID_LONG_UNSIGNED, POWER_FAILURE)
cl1_put_value(nic_status, (ADDR)&ID_UNSIGNED, NIC_STATUS)
cl1_put_value(log_code, (ADDR)&ID_UNSIGNED, LOG_CODE)
cl1_put_value(fault, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, FAULT)
cl1_put_value(cal_energy, cal_energy, CAL_ENERGY)
cl1_put_value(energy, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, ENERGY)
cl1_put_value(cal_voltamp, cal_voltamp, CAL_VOLTAMP)
cl1_put_value(voltamp, voltamp, VOLTAMP)
cl1_put_value(cal_rtc, cal_rtc, CAL_RTC)
cl1_put_value(userflag, (ADDR)&ID_LONG_UNSIGNED, USERFLAG)
cl1_put_value(build_no, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, BUILD_NO)
cl1_put_value(build_date, (ADDR)&ID_VISIBLE_STRING0, BUILD_DATE)
cl1_put_value(relay_setting, (ADDR)&relay_setting, RELAY_SETTING)
cl1_put_value(relay_config, (ADDR)&relay_config, RELAY_CONFIG)
//cl1_put_value(object_map0, object_map0, OBJECT_MAP0)
//cl1_put_value(client0, client0, CLIENT0)
//cl1_put_value(access_tbl0, access_tbl0, ACCESS_TBL0)
//cl1_put_value(display0, display0, DISPLAY0)
cl1_put_value(dispint, (ADDR)&ID_UNSIGNED, DISPINT)
cl1_put_value(record0, record0, RECORD0)
cl1_put_value(event_codes, (ADDR)&ID_OCTET_STRING128, EVENT_CODES)
cl1_put_value(setup_puls, setup_puls, SETUP_PULS)
//cl1_put_value(detect, detect, DETSET)
cl1_put_value(type, (ADDR)&ID_UNSIGNED, TYPE)
cl1_put_value(model, (ADDR)&ID_VISIBLE_STRING10, MODEL)
cl1_put_value(firm_ver, &ID_VISIBLE_STRING15, FIRM_VER)
cl1_put_value(firm_ver1, firm_ver1, FIRM_VER1)
cl1_put_value(firm_sig, firm_sig, FIRM_SIG)
cl1_put_value(demand_period, (ADDR)&ID_LONG_UNSIGNED, DEMAND_PERIOD)
cl1_put_value(load_period, (ADDR)&ID_LONG_UNSIGNED, LOAD_PERIOD)
cl1_put_value(daily_period, (ADDR)&ID_LONG_UNSIGNED, DAILY_PERIOD)

cl1_put_value(cellid, (ADDR)&ID_VISIBLE_STRING8, CELLID)/*@*/
cl1_put_value(4ggsm_prm, (ADDR)&ID_OCTET_STRING4, 4GGSM_PRM)/*@*/
cl1_put_value(ratband, (ADDR)&ID_OCTET_STRING2, RATBAND)/*@*/
cl1_put_value(nic_ver, (ADDR)&ID_OCTET_STRING0, NIC_VER)/*@*/
cl1_put_value(selfdiag, (ADDR)&ID_UNSIGNED, SELFDIAG)/*@*/
cl1_put_value(stamp_prm, (ADDR)&ID_LONG_UNSIGNED, STAMP_PRM)/*@*/

void G(0, 1, 2, server_port)(void)	/* 現在時刻 */
{
	put_value(2, idxobj, (ADDR)&ID_OCTET_STRING0);
}

void G(0, 1, 2, manu_year)(void)	/* 現在時刻 */
{
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void G(0, 1, 2, detect)(void) {
	getset = get_value2;
	put_value_with_block(IST_DETECT, &detect_ar, &detect_st);
}

void G(0, 1, 2, unit0)(void) {
	put_value( 2, IST_UNIT0, (ADDR)&unit0_st);
}

void G(0, 1, 2, time_now)(void) {/* 現在時刻 */
	put_attr(&get_meter_datetime()[SZ_DATE], 0, (ADDR)&ID_TIME);
}

void G(0, 1, 2, date_now)(void) {/* 現在月日 */
	put_attr(get_meter_datetime(), 0, (ADDR)&ID_DATE);
}

typedef struct tgWRRDMEM {

	WRRD_MEM_INIT	init;
	RD_MEM			read;
	WR_MEM			write;
}WRRDMEM;

WRRDMEM const wrrdmem[] = {
   { wrrd_ram_init,read_mem,write_ram },
   { wrrd_e2p_init,read_e2p,write_e2p },
   { wrrd_rom_init,read_mem,NULL },
};

ui08 do_read_mem(void) {

	ui08 ret, res;
	ui16 len;

	res = wrrdmem[sel_mem].read(&g_xdlms_io[record_offset], &len);
	record_offset += len;
	switch (res)
	{
	default:
	case 0x00:
		last_block = TRUE;
		ret = PHASE_RESPONSE;
		break;
	case 0x01:
		if (0 == block_no) {
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
		ret = PHASE_RESPONSE;
		break;
	case 0x02:
		ret = PHASE_THREAD;
		break;
	case 0xff:
		last_block = TRUE;
		result_xdlms = RET_HARDWARE_FAULT;
		ret = PHASE_RESPONSE;
		break;
	};
	if (RET_SUCCESS == result_xdlms) {
		if (PHASE_RESPONSE == ret) {
			/*ノーマルモード or ブロックモード*/
			g_raw[g_raw_len] = DT_OCTET_STRING;
			g_raw_len++;
			encode_len(g_raw, &g_raw_len, record_offset);
			ramcpy(&g_raw[g_raw_len], g_xdlms_io, record_offset);
			g_raw_len += record_offset;
			record_offset = 0;
		}
		else {
			/* 継続 */
		}
	}
	return ret;
}

void rd_mem_init(cui08 order) {

	ui32 size, tmp0, tmp1;
	ui16 count;

	if (0 == block_no)
	{
		tmp0 = 0;
		tmp1 = 0;
		switch (order) {
		default:
		case 0:
			size = wrrdmem[sel_mem].init(tmp0, tmp1);
			break;
		case 1:
			if (TRUE == take_attr((ADDR)&sel_wrrd_mem)) {
				tmp0 = UI32B(&g_xdlms_io[0]);
				tmp1 = UI32B(&g_xdlms_io[4]);
				size = wrrdmem[sel_mem].init(tmp0, tmp1);
			}
			else {
				/*result_xdlms == RET_TEMPORARY_FAILURE;*//*take_attr内で設定*/
			}
			break;
		}
		if (RET_SUCCESS == result_xdlms) {
			record_offset = 0;
			count = wrrd_mem_count(size);
			put_complex_count(count, &ID_ARRAY0);
		}
		else {
			return;
		}
	}
	thread_process = do_read_mem;
	xdlms_phase = PHASE_THREAD;			/* 読み出し処理開始 */
}

void G(0, 1, 2, ram)(void)
{
	sel_mem = 0;
	rd_mem_init(0);
}

void G(1, 1, 2, ram)(void)
{
	sel_mem = 0;
	rd_mem_init(1);
}

void G(0, 1, 2, e2p)(void)
{
	sel_mem = 1;
	rd_mem_init(0);
}

void G(1, 1, 2, e2p)(void)
{
	sel_mem = 1;
	rd_mem_init(1);
}

void G(0, 1, 2, rom)(void)
{
	sel_mem = 2;
	rd_mem_init(0);
}

void G(1, 1, 2, rom)(void)
{
	sel_mem = 2;
	rd_mem_init(1);
}

ui08 do_write_mem(void) {

	ui08 ret, res;
	ui16 size;

	res = wrrdmem[sel_mem].write(&g_xdlms_io[record_offset], &size);
	record_offset += size;
	switch (res)
	{
	default:
	case 0x00:
		last_block = TRUE;
		ret = PHASE_RESPONSE;
		break;
	case 0x01:
		if (0 == block_no) {
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
		ret = PHASE_RESPONSE;
		break;
	case 0x02:
		ret = PHASE_THREAD;
		break;
	case 0xff:
		last_block = TRUE;
		result_xdlms = RET_HARDWARE_FAULT;
		ret = PHASE_RESPONSE;
		break;
	};
	return ret;
}

void wr_mem_init(cui08 order) {

	ui32 tmp0, tmp1;
	ui16 offset = 0;

	if (0 == block_no)
	{
		record_log(LOG_MEMORY, sel_mem + 1);
		tmp0 = 0;
		tmp1 = 0;
		switch (order) {
		default:
		case 0:
			wrrdmem[sel_mem].init(tmp0, tmp1);
			break;
		case 1:
			if (TRUE == take_attr((ADDR)&sel_wrrd_mem)) {
				tmp0 = UI32B(&g_xdlms_io[offset]);
				offset += 4;
				tmp1 = UI32B(&g_xdlms_io[offset]);
				offset += 4;
				tmp0 = wrrdmem[sel_mem].init(tmp0, tmp1);
				tmp1 = decode_len(&offset, g_xdlms_io);
				if (tmp0 != tmp1) {
					result_xdlms = RET_TEMPORARY_FAILURE;	/*受信サイズ不一致*/
				}
			}
			else {
				//result_xdlms == RET_TEMPORARY_FAILURE;/*take_attr内で設定*
			}
			break;
		}
		if (RET_SUCCESS == result_xdlms) {
			record_offset = offset;
		}
		else {
			return;
		}
	}
	thread_process = do_write_mem;
	xdlms_phase = PHASE_THREAD;			/* 読み出し処理開始 */
}

void S(1, 1, 2, ram)(void)
{
	sel_mem = 0;
	wr_mem_init(1);
}

void S(1, 1, 2, e2p)(void)
{
	sel_mem = 1;
	wr_mem_init(1);
}

void S(0, 1, 2, serial_no)(void)
{
	take_value2(idxobj, (ADDR)&ID_OCTET_STRING10);
}

void S(0, 1, 2, model)(void)
{
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING10);
}

void S(0, 1, 2, type)(void)
{
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}

void S(0, 1, 2, device_id)(void)
{
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING11);
}

void S(0, 1, 2, server_port)(void)
{
	take_value2(idxobj, (ADDR)&ID_OCTET_STRING0);
}

void S(0, 1, 2, userflag)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void S(0, 1, 2, unit0)(void)
{
	take_value2(idxobj, unit0);
}

void S(0, 1, 2, record0)(void)
{
	take_value2(idxobj, record0);
}

void S(0, 1, 2, cal_rtc)(void)
{
	take_value2(idxobj, cal_rtc);
}

void S(0, 1, 2, detect)(void)
{
	getset = set_value2;
	take_value_with_block(idxobj, &detect_ar, &detect_st);
}

void S(0, 1, 2, relay_config)(void)
{
	take_value2(idxobj, (ADDR)&relay_config);
}

void S(0, 1, 2, setup_puls)(void)
{
	take_value2(idxobj, setup_puls);
}

void S(0, 1, 2, event_codes)(void)
{
	take_value2(idxobj, (ADDR)&ID_OCTET_STRING128);
}

void S(0, 1, 2, cal_energy)(void)
{
	take_value2(idxobj, cal_energy);
}

void S(0, 1, 2, cal_voltamp)(void)
{
	take_value2(idxobj, cal_voltamp);
}

void S(0, 1, 2, demand_period)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void S(0, 1, 2, load_period)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void S(0, 1, 2, daily_period)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void S(0, 1, 2, meter_mode)(void)
{
	if (TRUE == take_attr(&ID_UNSIGNED)) {
		if (g_xdlms_io[0] < 2) {
			set_value2(idxobj, obis_code[5], g_xdlms_io);
		}
		else {
			result_xdlms = RET_TEMPORARY_FAILURE;	/*受信サイズ不一致*/
		}
	}
}
void S(0, 1, 2, payment)(void)
{
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}
void S(0, 1, 2, va_aew)(void)
{
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}
void S(0, 1, 2, va_seipl)(void)
{
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}

void S(0, 1, 2, recharge_amount)(void)
{
	take_value2(idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}
void S(0, 1, 2, recharge_time)(void)
{
	take_value2(idxobj, (ADDR)&ID_OCTET_STRING12);
}
void S(0, 1, 2, manu_year)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void S(0, 1, 2, power_failure)(void)
{
	take_value2(idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}

void S(0, 1, 2, balance_time)(void)
{
	take_value2(idxobj, (ADDR)&ID_OCTET_STRING12);
}

void S(0, 1, 2, balance_amount)(void)
{
	take_value2(idxobj, (ADDR)&ID_DOUBLE_LONG);
}

void S(0, 1, 2, event_filter)(void)
{
	take_value2(idxobj, (ADDR)&ID_BIT_STRING128);
}
void S(0, 1, 2, total_amount)(void)
{
	take_value2(idxobj, (ADDR)&ID_DOUBLE_LONG);
}

#if 0
void S(0, 1, 2, billing_period)(void)
{
	take_value2(IST_BILLING_PERIOD, (ADDR)&ID_UNSIGNED);
}
#endif
void S(0, 1, 2, category)(void)
{
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING2);
}
void S(0, 1, 2, event_status)(void)
{
	take_value2(idxobj, (ADDR)&ID_BIT_STRING128);
}
void S(0, 1, 2, tamper_count)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void S(0, 1, 2, billing_count)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void S(0, 1, 2, prog_count)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void S(0, 1, 2, lrcf)(void)
{
	take_value2(idxobj, (ADDR)&ID_OCTET_STRING2);
}
void S(0, 1, 2, meter_type)(void)
{
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}
void S(0, 1, 2, current_rating)(void)
{
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING10);
}
void S(0, 1, 2, variable)(void) {
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING0);
}

void S(0, 1, 2, nic_imei)(void) {
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING20);
}

void S(0, 1, 2, nic_serial)(void) {
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING10);
}
void S(0, 1, 2, mac_addr)(void) {
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING20);
}

void S(0, 1, 2, selfdiag)(void) {/*@*/
	take_value2(idxobj, (ADDR)&ID_VISIBLE_STRING20);
}

void S(0, 1, 2, dispint)(void) {
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}

void S(0, 1, 2, u08_dat_rw)(void)
{
	take_value2(idxobj, (ADDR)&ID_UNSIGNED);
}

void S(0, 1, 2, u16_dat_rw)(void)
{
	take_value2(idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void S(0, 1, 2, u32_dat_rw)(void)
{
	take_value2(idxobj, (ADDR)&ID_DOUBLE_LONG_UNSIGNED);
}

/*********************end of interface class 1 ****************************/

/*******************************************/
/********* interface class 3  **********/
/*******************************************/
CADR  scaler_unit_list[] = {
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_ENUM
};
CONST IDENT scaler_unit = { DT_STRUCTURE,2,(ADDR)scaler_unit_list };

CADR  battery_list[] = {
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED
};
CONST IDENT battrery = { DT_STRUCTURE,2,(ADDR)battery_list };

CADR  magnet_list[] = {
	(ADDR)&ID_DOUBLE_LONG,
	(ADDR)&ID_DOUBLE_LONG,
	(ADDR)&ID_DOUBLE_LONG,
	(ADDR)&ID_DOUBLE_LONG,
	(ADDR)&ID_DOUBLE_LONG,
	(ADDR)&ID_DOUBLE_LONG
};
CONST IDENT magnet = { DT_STRUCTURE,6,(ADDR)magnet_list };

CADR  energy_list[] = {
	(ADDR)&ID_LONG64_UNSIGNED,/*Q1*/
	(ADDR)&ID_LONG64_UNSIGNED,/*Q2*/
	(ADDR)&ID_LONG64_UNSIGNED,/*Q3*/
	(ADDR)&ID_LONG64_UNSIGNED,/*Q4*/
};
CONST IDENT energy_st = { DT_STRUCTURE, 4, (ADDR)energy_list };
CADR  energy = (ADDR)&energy_st;

/****************************************************************/
#define cl3_put_value(ist, type, INST) \
void G(0,3,2,ist)( void )\
{\
	put_value( 2,  IST_##INST, type);\
}

void G(0, 3, 2, fwd_energy)(void) {
	put_value(2, idxobj, (ADDR)&DT_WATVARVAH);
}
void G(0, 3, 2, fwd_block_w)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void G(0, 3, 2, fwd_block_wh)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void G(0, 3, 2, freq)(void) {
	put_value( 2, idxobj, (ADDR)&DT_FREQ);
}

void G(0, 3, 2, ampr)(void) {
	put_value( 2, idxobj, (ADDR)&DT_AMP);
}

void G(0, 3, 2, ampr_block)(void) {
	put_value(2, idxobj, (ADDR)&DT_SIGNED_BLOCK);
}

void G(0, 3, 2, average_volt)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void G(0, 3, 2, volt)(void) {
	put_value(2, idxobj, (ADDR)&DT_VOLT);
}

void G(0, 3, 2, volt_block)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}

void G(0, 3, 2, power_factor)(void) {
	put_value(2, idxobj, (ADDR)&DT_POWER_FACTOR);
}

void G(0, 3, 2, pf_block)(void) {

	put_value(2, idxobj, (ADDR)&DT_SIGNED_BLOCK);
}

void G(0, 3, 2, temp_block)(void) {
	put_value(2, idxobj, (ADDR)&DT_SIGNED_BLOCK);
}

void G(0, 3, 2, cputemp_block)(void) {
	put_value(2, idxobj, (ADDR)&DT_SIGNED_BLOCK);
}

void G(0, 3, 2, thd)(void) {
	put_value(2, idxobj, (ADDR)&DT_FFT);
}

void G(0, 3, 2, thd_block)(void) {
	put_value(2, idxobj, (ADDR)&DT_FFT);
}

void G(0, 3, 2, atdd_block)(void) {
	put_value(2, idxobj, (ADDR)&DT_FFT);
}
void G(0, 3, 2, power)(void) {
	put_value(2, idxobj, (ADDR)&DT_WATVARVA);
}

void G(0, 3, 2, net_energy)(void) {
	put_value(2, idxobj, (ADDR)&ID_DOUBLE_LONG);
}
void G(0, 3, 2, abs_energy)(void) {
	put_value(2, idxobj, (ADDR)&DT_WATVARVAH);
}

cl3_put_value(billing_date, (ADDR)&ID_OCTET_STRING12, BILLING_DATE)
cl3_put_value(power_off_time, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, POWER_OFF_TIME)
cl3_put_value(billing_time, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, BILLING_TIME)
cl3_put_value(power_on_time, (ADDR)&DT_ON_TIME, POWER_ON_TIME)
cl3_put_value(batt_volt, (ADDR)&battrery, BATT_VOLT)
cl3_put_value(cputime, (ADDR)&ID_DOUBLE_LONG_UNSIGNED, CPUTIME)
cl3_put_value(magnet, (ADDR)&magnet, MAGNET)
cl3_put_value(cputemp, (ADDR)&DT_TEMP, CPUTEMP)
cl3_put_value(temperature, (ADDR)&DT_TEMP, TEMPERATURE)
cl3_put_value(volt_thd, (ADDR)&DT_FFT, VOLT_THD)
cl3_put_value(ampr_thd, (ADDR)&DT_FFT, AMPR_THD)
cl3_put_value(ampr_tdd, (ADDR)&DT_FFT, AMPR_TDD)
cl3_put_value(net_qenergy, (ADDR)&DT_WATVARVAH, NET_QENERGY)
cl3_put_value(defraud_power, (ADDR)&DT_WATVARVAH, DEFRAUD_POWER)


#define cl3_put_scaler_unit(ist, type, INST) \
void G(0,3,3,ist)( void )\
{\
	put_value( 3,  IST_##INST, type);\
}

cl3_put_scaler_unit(billing_date, (ADDR)&scaler_unit, BILLING_DATE)
cl3_put_scaler_unit(power_off_time, (ADDR)&scaler_unit, POWER_OFF_TIME)
cl3_put_scaler_unit(billing_time, (ADDR)&scaler_unit, BILLING_TIME)
cl3_put_scaler_unit(power_on_time, (ADDR)&scaler_unit, POWER_ON_TIME)
cl3_put_scaler_unit(batt_volt, (ADDR)&scaler_unit, BATT_VOLT)
cl3_put_scaler_unit(volt_thd, (ADDR)&scaler_unit, VOLT_THD)
cl3_put_scaler_unit(ampr_thd, (ADDR)&scaler_unit, AMPR_THD)
cl3_put_scaler_unit(ampr_tdd, (ADDR)&scaler_unit, AMPR_TDD)
cl3_put_scaler_unit(cputime, (ADDR)&scaler_unit, CPUTIME)
cl3_put_scaler_unit(magnet, (ADDR)&scaler_unit, MAGNET)
cl3_put_scaler_unit(cputemp, (ADDR)&scaler_unit, CPUTEMP)
cl3_put_scaler_unit(temperature, (ADDR)&scaler_unit, TEMPERATURE)
cl3_put_scaler_unit(defraud_power, (ADDR)&scaler_unit, DEFRAUD_POWER)

void G(0, 3, 3, power)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}
void G(0, 3, 3, fwd_energy)(void) {
	put_value( 3, idxobj, (ADDR)&scaler_unit);
}
void G(0, 3, 3, fwd_block_w)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}
void G(0, 3, 3, fwd_block_wh)(void) {
	put_value( 3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, freq)(void) {
	put_value( 3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, ampr)(void) {
	put_value( 3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, ampr_block)(void) {

	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, average_volt)(void) {

	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, volt)(void) {

	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, volt_block)(void) {

	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, power_factor)(void) {

	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, pf_block)(void) {

	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, temp_block)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, cputemp_block)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 3, 3, thd)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}
void G(0, 3, 3, thd_block)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}
cl3_put_scaler_unit(abs_energy, (ADDR)&scaler_unit, ABS_ENERGY)
cl3_put_scaler_unit(net_energy, (ADDR)&scaler_unit, NET_ENERGY)
cl3_put_scaler_unit(abs_qenergy, (ADDR)&scaler_unit, ABS_QENERGY)
cl3_put_scaler_unit(net_qenergy, (ADDR)&scaler_unit, NET_QENERGY)

void S(0, 3, 2, batt_volt)(void)
{
	take_value2(IST_BATT_VOLT, (ADDR)&battrery);
}

void S(0, 3, 2, billing_date)(void)
{
	take_value2(IST_BILLING_DATE, (ADDR)&ID_OCTET_STRING12);
}

/****************************************************************/
void reset(ui16 ist, PRST method)
{
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos])
	{
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_INTEGER))
	{
		return;
	}
	method(ist, *g_xdlms_io);
	return;
}

#define cl3_method(ist,INST,method) \
void M(3,1,ist)( void )\
{\
	reset(IST_##INST,method);\
}

void M(3, 1, fwd_energy)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, fwd_block_w)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, fwd_block_wh)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, freq)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, ampr)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, ampr_block)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, average_volt)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, volt)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, volt_block)(void) {
	reset(idxobj, reset_value);
}

void M(3, 1, power_factor)(void) {

	reset(idxobj, reset_value);
}
void M(3, 1, cputemp_block)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, temp_block)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, pf_block)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, thd)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, thd_block)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, rssi_block)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, abs_energy)(void) {
	reset(idxobj, reset_value);
}
void M(3, 1, net_energy)(void) {
	reset(idxobj, reset_value);
}

cl3_method(billing_date, BILLING_DATE, reset_value)
cl3_method(power_off_time, POWER_OFF_TIME, reset_value)
cl3_method(billing_time, BILLING_TIME, reset_value)
cl3_method(power_on_time, POWER_ON_TIME, reset_value)
cl3_method(batt_volt, BATT_VOLT, reset_value)
cl3_method(cputime, CPUTIME, reset_value)
cl3_method(magnet, MAGNET, reset_value)
cl3_method(cputemp, CPUTEMP, reset_value)
cl3_method(temperature, TEMPERATURE, reset_value)
cl3_method(volt_thd, VOLT_THD, reset_value)
cl3_method(ampr_thd, AMPR_THD, reset_value)
cl3_method(ampr_tdd, AMPR_TDD, reset_value)
cl3_method(power, POWER, reset_value)
cl3_method(apperent, APPERENT, reset_value)
cl3_method(defraud_power, DEFRAUD_POWER, reset_value)
cl3_method(abs_qenergy, ABS_QENERGY, reset_value)
cl3_method(net_qenergy, NET_QENERGY, reset_value)

/*********************end of interface class 3 ****************************/

/*******************************************/
/********* interface class 4  **********/
/*******************************************/
void G(0, 4, 2, max_fwd_demand)(void) {
	put_value( 2, idxobj, (ADDR)&DT_WATVARVAH);
}
void G(0, 4, 2, minimum_volt)(void) {
	put_value(2, idxobj, (ADDR)&ID_LONG_UNSIGNED);
}
void G(0, 4, 2, rssi_block)(void) {
	put_value(2, idxobj, (ADDR)&ID_UNSIGNED);
}

void G(0, 4, 3, max_fwd_demand)(void) {
	put_value( 3, idxobj, (ADDR)&scaler_unit);
}
void G(0, 4, 3, minimum_volt)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}
void G(0, 4, 3, rssi_block)(void) {
	put_value(3, idxobj, (ADDR)&scaler_unit);
}

void G(0, 4, 5, max_fwd_demand)(void) {
	put_value( 5, idxobj, (ADDR)&ID_OCTET_STRING12);
}
void G(0, 4, 5, minimum_volt)(void) {
	put_value(5, idxobj, (ADDR)&ID_OCTET_STRING12);
}
void G(0, 4, 5, rssi_block)(void) {
	put_value(5, idxobj, (ADDR)&ID_OCTET_STRING12);
}

void M(4, 1, max_fwd_demand)(void) {
	reset(idxobj, reset_value);
}
void M(4, 1, minimum_volt)(void) {
	reset(idxobj, reset_value);
}
void M(4, 1, rssi_block)(void) {
	reset(idxobj, reset_value);
}

/*********************end of interface class 4 ****************************/

/**************************************************************************/
/*******************************************/
/************ interface class 7 ************/
/* http://www.dlms.com/faqanswers/forum/index.php?action=view&topicid=343 */
/*******************************************/
cui08 colm_sel_def[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, };
#if 0
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };
#endif
cui32 I(7, 4, profile_generic) = 0;		/* capture_period - 0 */
cui08 I(7, 5, profile_generic) = 1;		/* sort_method - fifo */
cui08 I(7, 6, profile_generic)[] = { 0,0,0,0,0,0,0,0,0,0,0 };	/* sort object */

/* データ型定義 */
CADR  object_definition[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING6,
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_LONG_UNSIGNED,
};
CONST IDENT capture_objects = { DT_STRUCTURE, 4, (ADDR)object_definition };

CONST CADR by_date_range[] = {
	(ADDR)&ID_OCTET_STRING12,
	(ADDR)&ID_OCTET_STRING12,
};
CONST CADR by_entry_range[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
};
CONST IDENT restriction_by_date_st = { DT_STRUCTURE, 2, (ADDR)by_date_range };
CONST IDENT restriction_by_entry_st = { DT_STRUCTURE, 2, (ADDR)by_entry_range };

CONST CADR restriction_by_date[] = {
	(ADDR)&ID_ENUM,
	(ADDR)&restriction_by_date_st,
};
CONST CADR restriction_by_entry[] = {
	(ADDR)&ID_ENUM,
	(ADDR)&restriction_by_entry_st,
};
CONST IDENT restriction1 = { DT_STRUCTURE, 2, (ADDR)restriction_by_date };
CONST IDENT restriction2 = { DT_STRUCTURE, 2, (ADDR)restriction_by_entry };

CADR  object_definition2[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING6,
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&restriction2,
};
CONST IDENT capture_objects2 = { DT_STRUCTURE, 5, (ADDR)object_definition2 };

CADR  sel_entry_list[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
};
CONST IDENT sel_entry_desc = { DT_STRUCTURE, 4, (ADDR)sel_entry_list };
#if 0
cui08 cl7_atr6[] = { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };	/* sort object - 40 */
#endif

CADR  cl7_common_atr[] = {
	/* 4 */	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	/* 5 */	(ADDR)&ID_ENUM,
	/* 6 */	(ADDR)&capture_objects,
	/* 7 */	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	/* 8 */	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
};
CADR  capture_scal_unit[] = {
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
	(ADDR)&scaler_unit,
};

/***************************/
/* Capture Object list定義 */
/***************************/
/*****************************************/
CADR  capture_meter_log[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING2,
	(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, meter_log)[] = {
	0x0000,IST_CPUTIME,0x02,{ 0x00, 0x00 },
	0x0004,IST_LOG_CODE,0x02,{ 0x00, 0x00 },
	0x0006,0,0,{0,0},
};
CONST IDENT meter_log_atr3 = { DT_ARRAY, 2, (ADDR)&capture_objects };

/*****************A5************************/
CADR  capture_event_profile[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&DT_VOLT,
(ADDR)&DT_AMP,
(ADDR)&DT_AMP,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
(ADDR)&ID_VISIBLE_STRING11,
(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, event_profile)[] = {
0x0000,IST_DATETIME_NOW,0x02,{ 0x00, 0x00 },
0x000C,IST_EVENT1, 0x02,{0x00, 0x00},
0x000E,IST_VOLT0,0x02,{ 0x00, 0x00},
0x0012,IST_AMPRC,0x02,{ 0x00, 0x00},
0x0014,IST_AMPRG,0x02,{ 0x00, 0x00},
0x0016,IST_POWER_FACTOR,0x02,{ 0x00, 0x00},
0x0018,IST_FWD_ENERGY,0x02,{0x00,0x00},
0x001C,IST_FWD_VAH,0x02,{0x00,0x00},
0x0020,IST_EVENT1_RNO, 0x02,{ 0x00, 0x00},
0x0024,IST_SERIAL_NO, 0x02,{ 0x00, 0x00},
0x002F,0,0,{0,0},
};
CONST IDENT event_profile_atr3 = { DT_ARRAY, 10, (ADDR)&capture_objects };

CONST CAPDEF K(7, 3, scal_unit5)[] = {
0x000E,IST_VOLT0,0x03,{ 0x00, 0x00},
0x0010,IST_AMPRC,0x03,{ 0x00, 0x00},
0x0012,IST_AMPRG,0x03,{ 0x00, 0x00},
0x0014,IST_POWER_FACTOR,0x03,{ 0x00, 0x00},
0x0016,IST_FWD_ENERGY,0x03,{0x00,0x00},
0x001A,IST_FWD_VAH,0x03,{0x00,0x00},
0x002A,0,0,{0,0},
};
CONST IDENT scal_unit5_atr3 = { DT_ARRAY, 6, (ADDR)&capture_objects };

CADR  capture_operate_profile[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
(ADDR)&ID_VISIBLE_STRING11,
(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, operate_profile)[] = {
0x0000,IST_DATETIME_NOW,0x02,{ 0x00, 0x00 },
0x000C,IST_EVENT1, 0x02,{0x00, 0x00},
0x000E,IST_EVENT1_RNO, 0x02,{ 0x00, 0x00},
0x0012,IST_SERIAL_NO, 0x02,{ 0x00, 0x00},
0x001D,0,0,{0,0},
};

CONST IDENT operate_profile_atr3 = { DT_ARRAY, 4, (ADDR)&capture_objects };

/********************relay log*********************/
CADR  capture_relay_log[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_NULL_DATA,
};

CONST IDENT relay_log_list = { DT_STRUCTURE, 25, (ADDR)capture_relay_log };
CONST IDENT ID_RELAY = { DT_ARRAY, 5, (ADDR)&relay_log_list };

CONST CAPDEF K(7, 3, relay_log)[] = {
0x0000,IST_DATETIME_NOW,0x02,{0x00,0x00},
0x000C,IST_FWD_ENERGY,0x02,{0x00,0x00},
0x0010,IST_FWD_ENERGY1,0x02,{0x00,0x00},
0x0014,IST_FWD_ENERGY2,0x02,{0x00,0x00},
0x0018,IST_FWD_ENERGY3,0x02,{0x00,0x00},
0x001C,IST_FWD_ENERGY4,0x02,{0x00,0x00},
0x0020,IST_FWD_ENERGY5,0x02,{0x00,0x00},
0x0024,IST_FWD_ENERGY6,0x02,{0x00,0x00},
0x0028,IST_FWD_ENERGY7,0x02,{0x00,0x00},
0x002C,IST_FWD_ENERGY8,0x02,{0x00,0x00},
0x0030,IST_BAK_ENERGY,0x02,{0x00,0x00},
0x0034,IST_FWD_VAH,0x02,{0x00,0x00},
0x0038,IST_FWD_VAH1,0x02,{0x00,0x00},
0x003C,IST_FWD_VAH2,0x02,{0x00,0x00},
0x0040,IST_FWD_VAH3,0x02,{0x00,0x00},
0x0044,IST_FWD_VAH4,0x02,{0x00,0x00},
0x0048,IST_FWD_VAH5,0x02,{0x00,0x00},
0x004C,IST_FWD_VAH6,0x02,{0x00,0x00},
0x0050,IST_FWD_VAH7,0x02,{0x00,0x00},
0x0054,IST_FWD_VAH8,0x02,{0x00,0x00},
0x0058,IST_BAK_VAH,0x02,{0x00,0x00},
0x005C,IST_VARH_Q1,0x02,{0x00,0x00},
0x0060,IST_VARH_Q2,0x02,{0x00,0x00},
0x0064,IST_VARH_Q3,0x02,{0x00,0x00},
0x0068,IST_VARH_Q4,0x02,{0x00,0x00},
0x006C,0,0,{0,0},
};
CONST IDENT relay_log_atr3 = { DT_ARRAY, 25, (ADDR)&capture_objects };

/********************A4*********************/
CADR  capture_billing_params[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, billing_params)[] = {
0x0000,IST_BILLING_DATE,0x02,{0x00,0x00},
0x000C,IST_FWD_PF_MON,0x02,{0x00,0x00},
0x000E,IST_FWD_ENERGY,0x02,{0x00,0x00},
0x0012,IST_FWD_ENERGY1,0x02,{0x00,0x00},
0x0016,IST_FWD_ENERGY2,0x02,{0x00,0x00},
0x001A,IST_FWD_ENERGY3,0x02,{0x00,0x00},
0x001E,IST_FWD_ENERGY4,0x02,{0x00,0x00},
0x0022,IST_FWD_ENERGY5,0x02,{0x00,0x00},
0x0026,IST_FWD_ENERGY6,0x02,{0x00,0x00},
0x002A,IST_FWD_ENERGY7,0x02,{0x00,0x00},
0x002E,IST_FWD_ENERGY8,0x02,{0x00,0x00},
0x0032,IST_FWD_VAH,0x02,{0x00,0x00},
0x0036,IST_FWD_VAH1,0x02,{0x00,0x00},
0x003A,IST_FWD_VAH2,0x02,{0x00,0x00},
0x003E,IST_FWD_VAH3,0x02,{0x00,0x00},
0x0042,IST_FWD_VAH4,0x02,{0x00,0x00},
0x0046,IST_FWD_VAH5,0x02,{0x00,0x00},
0x004A,IST_FWD_VAH6,0x02,{0x00,0x00},
0x004E,IST_FWD_VAH7,0x02,{0x00,0x00},
0x0052,IST_FWD_VAH8,0x02,{0x00,0x00},
0x0056,IST_MAX_FWD_DEMAND,0x02,{0x00,0x00},
0x005A,IST_MAX_FWD_DEMAND,0x05,{0x00,0x00},
0x0066,IST_MAX_FWD_VA,0x02,{0x00,0x00},
0x006A,IST_MAX_FWD_VA,0x05,{0x00,0x00},
0x0076,IST_BILLING_TIME,0x02,{0x00,0x00},
0x007A,IST_BAK_ENERGY,0x02,{0x00,0x00},
0x007E,IST_BAK_VAH,0x02,{0x00,0x00},
0x0082,IST_MAX_FWD_DEMAND1,0x02,{0x00,0x00},
0x0086,IST_MAX_FWD_DEMAND1,0x05,{0x00,0x00},
0x0092,IST_MAX_FWD_DEMAND2,0x02,{0x00,0x00},
0x0096,IST_MAX_FWD_DEMAND2,0x05,{0x00,0x00},
0x00A2,IST_MAX_FWD_DEMAND3,0x02,{0x00,0x00},
0x00A6,IST_MAX_FWD_DEMAND3,0x05,{0x00,0x00},
0x00B2,IST_MAX_FWD_DEMAND4,0x02,{0x00,0x00},
0x00B6,IST_MAX_FWD_DEMAND4,0x05,{0x00,0x00},
0x00C2,IST_MAX_FWD_DEMAND5,0x02,{0x00,0x00},
0x00C6,IST_MAX_FWD_DEMAND5,0x05,{0x00,0x00},
0x00D2,IST_MAX_FWD_DEMAND6,0x02,{0x00,0x00},
0x00D6,IST_MAX_FWD_DEMAND6,0x05,{0x00,0x00},
0x00E2,IST_MAX_FWD_DEMAND7,0x02,{0x00,0x00},
0x00E6,IST_MAX_FWD_DEMAND7,0x05,{0x00,0x00},
0x00F2,IST_MAX_FWD_DEMAND8,0x02,{0x00,0x00},
0x00F6,IST_MAX_FWD_DEMAND8,0x05,{0x00,0x00},
0x0102,IST_MAX_FWD_VA1,0x02,{0x00,0x00},
0x0106,IST_MAX_FWD_VA1,0x05,{0x00,0x00},
0x0112,IST_MAX_FWD_VA2,0x02,{0x00,0x00},
0x0116,IST_MAX_FWD_VA2,0x05,{0x00,0x00},
0x0122,IST_MAX_FWD_VA3,0x02,{0x00,0x00},
0x0126,IST_MAX_FWD_VA3,0x05,{0x00,0x00},
0x0132,IST_MAX_FWD_VA4,0x02,{0x00,0x00},
0x0136,IST_MAX_FWD_VA4,0x05,{0x00,0x00},
0x0142,IST_MAX_FWD_VA5,0x02,{0x00,0x00},
0x0146,IST_MAX_FWD_VA5,0x05,{0x00,0x00},
0x0152,IST_MAX_FWD_VA6,0x02,{0x00,0x00},
0x0156,IST_MAX_FWD_VA6,0x05,{0x00,0x00},
0x0162,IST_MAX_FWD_VA7,0x02,{0x00,0x00},
0x0166,IST_MAX_FWD_VA7,0x05,{0x00,0x00},
0x0172,IST_MAX_FWD_VA8,0x02,{0x00,0x00},
0x0176,IST_MAX_FWD_VA8,0x05,{0x00,0x00},
0x0182,IST_FWD_PF1,0x02,{0x00,0x00},
0x0184,IST_FWD_PF2,0x02,{0x00,0x00},
0x0186,IST_FWD_PF3,0x02,{0x00,0x00},
0x0188,IST_FWD_PF4,0x02,{0x00,0x00},
0x018A,IST_FWD_PF5,0x02,{0x00,0x00},
0x018C,IST_FWD_PF6,0x02,{0x00,0x00},
0x018E,IST_FWD_PF7,0x02,{0x00,0x00},
0x0190,IST_FWD_PF8,0x02,{0x00,0x00},
0x0192,IST_BAK_PF1,0x02,{0x00,0x00},
0x0194,IST_BAK_PF2,0x02,{0x00,0x00},
0x0196,IST_BAK_PF3,0x02,{0x00,0x00},
0x0198,IST_BAK_PF4,0x02,{0x00,0x00},
0x019A,IST_BAK_PF5,0x02,{0x00,0x00},
0x019C,IST_BAK_PF6,0x02,{0x00,0x00},
0x019E,IST_BAK_PF7,0x02,{0x00,0x00},
0x01A0,IST_BAK_PF8,0x02,{0x00,0x00},
0x01A2,IST_BAK_ENERGY1,0x02,{0x00,0x00},
0x01A6,IST_BAK_ENERGY2,0x02,{0x00,0x00},
0x01AA,IST_BAK_ENERGY3,0x02,{0x00,0x00},
0x01AE,IST_BAK_ENERGY4,0x02,{0x00,0x00},
0x01B2,IST_BAK_ENERGY5,0x02,{0x00,0x00},
0x01B6,IST_BAK_ENERGY6,0x02,{0x00,0x00},
0x01BA,IST_BAK_ENERGY7,0x02,{0x00,0x00},
0x01BE,IST_BAK_ENERGY8,0x02,{0x00,0x00},
0x01C2,IST_BAK_VAH1,0x02,{0x00,0x00},
0x01C6,IST_BAK_VAH2,0x02,{0x00,0x00},
0x01CA,IST_BAK_VAH3,0x02,{0x00,0x00},
0x01CE,IST_BAK_VAH4,0x02,{0x00,0x00},
0x01D2,IST_BAK_VAH5,0x02,{0x00,0x00},
0x01D6,IST_BAK_VAH6,0x02,{0x00,0x00},
0x01DA,IST_BAK_VAH7,0x02,{0x00,0x00},
0x01DE,IST_BAK_VAH8,0x02,{0x00,0x00},
0x01E2,IST_MAX_BAK_DEMAND,0x02,{0x00,0x00},
0x01E6,IST_MAX_BAK_DEMAND,0x05,{0x00,0x00},
0x01F2,IST_MAX_BAK_DEMAND1,0x02,{0x00,0x00},
0x01F6,IST_MAX_BAK_DEMAND1,0x05,{0x00,0x00},
0x0202,IST_MAX_BAK_DEMAND2,0x02,{0x00,0x00},
0x0206,IST_MAX_BAK_DEMAND2,0x05,{0x00,0x00},
0x0212,IST_MAX_BAK_DEMAND3,0x02,{0x00,0x00},
0x0216,IST_MAX_BAK_DEMAND3,0x05,{0x00,0x00},
0x0222,IST_MAX_BAK_DEMAND4,0x02,{0x00,0x00},
0x0226,IST_MAX_BAK_DEMAND4,0x05,{0x00,0x00},
0x0232,IST_MAX_BAK_DEMAND5,0x02,{0x00,0x00},
0x0236,IST_MAX_BAK_DEMAND5,0x05,{0x00,0x00},
0x0242,IST_MAX_BAK_DEMAND6,0x02,{0x00,0x00},
0x0246,IST_MAX_BAK_DEMAND6,0x05,{0x00,0x00},
0x0252,IST_MAX_BAK_DEMAND7,0x02,{0x00,0x00},
0x0256,IST_MAX_BAK_DEMAND7,0x05,{0x00,0x00},
0x0262,IST_MAX_BAK_DEMAND8,0x02,{0x00,0x00},
0x0266,IST_MAX_BAK_DEMAND8,0x05,{0x00,0x00},
0x0272,IST_MAX_BAK_VA,0x02,{0x00,0x00},
0x0276,IST_MAX_BAK_VA,0x05,{0x00,0x00},
0x0282,IST_MAX_BAK_VA1,0x02,{0x00,0x00},
0x0286,IST_MAX_BAK_VA1,0x05,{0x00,0x00},
0x0292,IST_MAX_BAK_VA2,0x02,{0x00,0x00},
0x0296,IST_MAX_BAK_VA2,0x05,{0x00,0x00},
0x02A2,IST_MAX_BAK_VA3,0x02,{0x00,0x00},
0x02A6,IST_MAX_BAK_VA3,0x05,{0x00,0x00},
0x02B2,IST_MAX_BAK_VA4,0x02,{0x00,0x00},
0x02B6,IST_MAX_BAK_VA4,0x05,{0x00,0x00},
0x02C2,IST_MAX_BAK_VA5,0x02,{0x00,0x00},
0x02C6,IST_MAX_BAK_VA5,0x05,{0x00,0x00},
0x02D2,IST_MAX_BAK_VA6,0x02,{0x00,0x00},
0x02D6,IST_MAX_BAK_VA6,0x05,{0x00,0x00},
0x02E2,IST_MAX_BAK_VA7,0x02,{0x00,0x00},
0x02E6,IST_MAX_BAK_VA7,0x05,{0x00,0x00},
0x02F2,IST_MAX_BAK_VA8,0x02,{0x00,0x00},
0x02F6,IST_MAX_BAK_VA8,0x05,{0x00,0x00},
0x0302,IST_BAK_PF_MON,0x02,{0x00,0x00},
0x0304,IST_NET_ENERGY,0x02,{0x00,0x00},
0x0308,IST_NET_VAH,0x02,{0x00,0x00},
0x030C,IST_VARH_Q1,0x02,{0x00,0x00},
0x0310,IST_VARH_Q2,0x02,{0x00,0x00},
0x0314,IST_VARH_Q3,0x02,{0x00,0x00},
0x0318,IST_VARH_Q4,0x02,{0x00,0x00},
0x031C,0,0,{0,0},
};
CONST IDENT billing_params_atr3 = { DT_ARRAY, sizeof(capture_billing_params) / sizeof(ADDR) - 1, (ADDR)&capture_objects };

/**********post paid ************/
CONST CAPDEF K(7, 3, scal_unit4)[] = {
0x0000,IST_BILLING_DATE,0x03,{0x00,0x00},
0x000C,IST_FWD_PF_MON,0x03,{0x00,0x00},
0x000E,IST_FWD_ENERGY,0x03,{0x00,0x00},
0x0012,IST_FWD_ENERGY1,0x03,{0x00,0x00},
0x0016,IST_FWD_ENERGY2,0x03,{0x00,0x00},
0x001A,IST_FWD_ENERGY3,0x03,{0x00,0x00},
0x001E,IST_FWD_ENERGY4,0x03,{0x00,0x00},
0x0022,IST_FWD_ENERGY5,0x03,{0x00,0x00},
0x0026,IST_FWD_ENERGY6,0x03,{0x00,0x00},
0x002A,IST_FWD_ENERGY7,0x03,{0x00,0x00},
0x002E,IST_FWD_ENERGY8,0x03,{0x00,0x00},
0x0032,IST_FWD_VAH,0x03,{0x00,0x00},
0x0036,IST_FWD_VAH1,0x03,{0x00,0x00},
0x003A,IST_FWD_VAH2,0x03,{0x00,0x00},
0x003E,IST_FWD_VAH3,0x03,{0x00,0x00},
0x0042,IST_FWD_VAH4,0x03,{0x00,0x00},
0x0046,IST_FWD_VAH5,0x03,{0x00,0x00},
0x004A,IST_FWD_VAH6,0x03,{0x00,0x00},
0x004E,IST_FWD_VAH7,0x03,{0x00,0x00},
0x0052,IST_FWD_VAH8,0x03,{0x00,0x00},
0x0056,IST_MAX_FWD_DEMAND,0x03,{0x00,0x00},
0x0066,IST_MAX_FWD_VA,0x03,{0x00,0x00},
0x0076,IST_BILLING_TIME,0x03,{0x00,0x00},
0x007A,IST_BAK_ENERGY,0x03,{0x00,0x00},
0x007E,IST_BAK_VAH,0x03,{0x00,0x00},
0x0082,IST_MAX_FWD_DEMAND1,0x03,{0x00,0x00},
0x0092,IST_MAX_FWD_DEMAND2,0x03,{0x00,0x00},
0x00A2,IST_MAX_FWD_DEMAND3,0x03,{0x00,0x00},
0x00B2,IST_MAX_FWD_DEMAND4,0x03,{0x00,0x00},
0x00C2,IST_MAX_FWD_DEMAND5,0x03,{0x00,0x00},
0x00D2,IST_MAX_FWD_DEMAND6,0x03,{0x00,0x00},
0x00E2,IST_MAX_FWD_DEMAND7,0x03,{0x00,0x00},
0x00F2,IST_MAX_FWD_DEMAND8,0x03,{0x00,0x00},
0x0102,IST_MAX_FWD_VA1,0x03,{0x00,0x00},
0x0112,IST_MAX_FWD_VA2,0x03,{0x00,0x00},
0x0122,IST_MAX_FWD_VA3,0x03,{0x00,0x00},
0x0132,IST_MAX_FWD_VA4,0x03,{0x00,0x00},
0x0142,IST_MAX_FWD_VA5,0x03,{0x00,0x00},
0x0152,IST_MAX_FWD_VA6,0x03,{0x00,0x00},
0x0162,IST_MAX_FWD_VA7,0x03,{0x00,0x00},
0x0172,IST_MAX_FWD_VA8,0x03,{0x00,0x00},
0x0182,IST_FWD_PF1,0x03,{0x00,0x00},
0x0184,IST_FWD_PF2,0x03,{0x00,0x00},
0x0186,IST_FWD_PF3,0x03,{0x00,0x00},
0x0188,IST_FWD_PF4,0x03,{0x00,0x00},
0x018A,IST_FWD_PF5,0x03,{0x00,0x00},
0x018C,IST_FWD_PF6,0x03,{0x00,0x00},
0x018E,IST_FWD_PF7,0x03,{0x00,0x00},
0x0190,IST_FWD_PF8,0x03,{0x00,0x00},
0x0192,IST_BAK_PF1,0x03,{0x00,0x00},
0x0194,IST_BAK_PF2,0x03,{0x00,0x00},
0x0196,IST_BAK_PF3,0x03,{0x00,0x00},
0x0198,IST_BAK_PF4,0x03,{0x00,0x00},
0x019A,IST_BAK_PF5,0x03,{0x00,0x00},
0x019C,IST_BAK_PF6,0x03,{0x00,0x00},
0x019E,IST_BAK_PF7,0x03,{0x00,0x00},
0x01A0,IST_BAK_PF8,0x03,{0x00,0x00},
0x01A2,IST_BAK_ENERGY1,0x03,{0x00,0x00},
0x01A6,IST_BAK_ENERGY2,0x03,{0x00,0x00},
0x01AA,IST_BAK_ENERGY3,0x03,{0x00,0x00},
0x01AE,IST_BAK_ENERGY4,0x03,{0x00,0x00},
0x01B2,IST_BAK_ENERGY5,0x03,{0x00,0x00},
0x01B6,IST_BAK_ENERGY6,0x03,{0x00,0x00},
0x01BA,IST_BAK_ENERGY7,0x03,{0x00,0x00},
0x01BE,IST_BAK_ENERGY8,0x03,{0x00,0x00},
0x01C2,IST_BAK_VAH1,0x03,{0x00,0x00},
0x01C6,IST_BAK_VAH2,0x03,{0x00,0x00},
0x01CA,IST_BAK_VAH3,0x03,{0x00,0x00},
0x01CE,IST_BAK_VAH4,0x03,{0x00,0x00},
0x01D2,IST_BAK_VAH5,0x03,{0x00,0x00},
0x01D6,IST_BAK_VAH6,0x03,{0x00,0x00},
0x01DA,IST_BAK_VAH7,0x03,{0x00,0x00},
0x01DE,IST_BAK_VAH8,0x03,{0x00,0x00},
0x01E2,IST_MAX_BAK_DEMAND,0x03,{0x00,0x00},
0x01F2,IST_MAX_BAK_DEMAND1,0x03,{0x00,0x00},
0x0202,IST_MAX_BAK_DEMAND2,0x03,{0x00,0x00},
0x0212,IST_MAX_BAK_DEMAND3,0x03,{0x00,0x00},
0x0222,IST_MAX_BAK_DEMAND4,0x03,{0x00,0x00},
0x0232,IST_MAX_BAK_DEMAND5,0x03,{0x00,0x00},
0x0242,IST_MAX_BAK_DEMAND6,0x03,{0x00,0x00},
0x0252,IST_MAX_BAK_DEMAND7,0x03,{0x00,0x00},
0x0262,IST_MAX_BAK_DEMAND8,0x03,{0x00,0x00},
0x0272,IST_MAX_BAK_VA,0x03,{0x00,0x00},
0x0282,IST_MAX_BAK_VA1,0x03,{0x00,0x00},
0x0292,IST_MAX_BAK_VA2,0x03,{0x00,0x00},
0x02A2,IST_MAX_BAK_VA3,0x03,{0x00,0x00},
0x02B2,IST_MAX_BAK_VA4,0x03,{0x00,0x00},
0x02C2,IST_MAX_BAK_VA5,0x03,{0x00,0x00},
0x02D2,IST_MAX_BAK_VA6,0x03,{0x00,0x00},
0x02E2,IST_MAX_BAK_VA7,0x03,{0x00,0x00},
0x02F2,IST_MAX_BAK_VA8,0x03,{0x00,0x00},
0x0302,IST_BAK_PF_MON,0x03,{0x00,0x00},
0x0304,IST_NET_ENERGY,0x03,{0x00,0x00},
0x0308,IST_NET_VAH,0x03,{0x00,0x00},
0x030C,IST_VARH_Q1,0x03,{0x00,0x00},
0x0310,IST_VARH_Q2,0x03,{0x00,0x00},
0x0314,IST_VARH_Q3,0x03,{0x00,0x00},
0x0318,IST_VARH_Q4,0x03,{0x00,0x00},
0x031C,0,0,{0,0},
};
CONST IDENT scal_unit4_atr3 = { DT_ARRAY, 98, (ADDR)&capture_objects };

/*********************A2********************/
CADR  capture_load_profile[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_VOLT,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_AMP,
(ADDR)&DT_AMP,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&ID_NULL_DATA,
};
CONST CAPDEF K(7, 3, load_profile)[] = {
0x0000,IST_DATETIME_NOW,0x02,{0x00,0x00},
0x000C,IST_VOLT0_BLOCK,0x02,{0x00,0x00},
0x0010,IST_FWD_BLOCK_WH,0x02,{0x00,0x00},
0x0014,IST_FWD_BLOCK_VAH,0x02,{0x00,0x00},
0x0018,IST_BAK_BLOCK_WH,0x02,{0x00,0x00},
0x001C,IST_BAK_BLOCK_VAH,0x02,{0x00,0x00},
0x0020,IST_AMPR0_BLOCK,0x02,{0x00,0x00},
0x0022,IST_AMPRG_BLOCK,0x02,{0x00,0x00},
0x0024,IST_FWD_BLOCK_VARH,0x02,{0x00,0x00},
0x0028,IST_BLOCK_Q2,0x02,{0x00,0x00},
0x002C,IST_BLOCK_Q3,0x02,{0x00,0x00},
0x0030,IST_BAK_BLOCK_VARH,0x02,{0x00,0x00},
0x0034,0,0,{0,0},
};
CONST IDENT load_profile_atr3 = { DT_ARRAY, sizeof(capture_load_profile) / sizeof(ADDR) - 1, (ADDR)&capture_objects };

CONST CAPDEF K(7, 3, scal_unit2)[] = {
0x000C,IST_VOLT0_BLOCK,0x03,{0x00,0x00},
0x000E,IST_FWD_BLOCK_WH,0x03,{0x00,0x00},
0x0012,IST_FWD_BLOCK_VAH,0x03,{0x00,0x00},
0x0016,IST_BAK_BLOCK_WH,0x03,{0x00,0x00},
0x001A,IST_BAK_BLOCK_VAH,0x03,{0x00,0x00},
0x001E,IST_AMPR0_BLOCK,0x03,{0x00,0x00},
0x0020,IST_AMPRG_BLOCK,0x03,{0x00,0x00},
0x0022,IST_FWD_BLOCK_VARH,0x03,{0x00,0x00},
0x0026,IST_BLOCK_Q2,0x03,{0x00,0x00},
0x002A,IST_BLOCK_Q3,0x03,{0x00,0x00},
0x002E,IST_BAK_BLOCK_VARH,0x03,{0x00,0x00},
0x0032,0,0,{0,0},
};
CONST IDENT scal_unit2_atr3 = { DT_ARRAY, 11, (ADDR)&capture_objects };

/********* daily profile  A3*************************/
CADR  capture_daily_profile[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVA,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&DT_WATVARVA,
(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, daily_profile)[] = {
0x0000,IST_DATETIME_NOW,0x02,{0x00,0x00},
0x000C,IST_BAK_ENERGY,0x02,{0x00,0x00},
0x0010,IST_BAK_VAH,0x02,{0x00,0x00},
0x0014,IST_FWD_ENERGY,0x02,{0x00,0x00},
0x0018,IST_FWD_ENERGY1,0x02,{0x00,0x00},
0x001C,IST_FWD_ENERGY2,0x02,{0x00,0x01},
0x0020,IST_FWD_ENERGY3,0x02,{0x00,0x02},
0x0024,IST_FWD_ENERGY4,0x02,{0x00,0x03},
0x0028,IST_FWD_ENERGY5,0x02,{0x00,0x04},
0x002C,IST_FWD_ENERGY6,0x02,{0x00,0x05},
0x0030,IST_FWD_ENERGY7,0x02,{0x00,0x06},
0x0034,IST_FWD_ENERGY8,0x02,{0x00,0x07},
0x0038,IST_FWD_VAH,0x02,{0x00,0x00},
0x003C,IST_FUND_ENERGY,0x02,{0x00,0x00},
0x0040,IST_MAX_FWD_DEMAND,0x02,{0x00,0x00},
0x0044,IST_MAX_FWD_DEMAND,0x05,{0x00,0x00},
0x0050,IST_MAX_FWD_VA,0x02,{ 0x00, 0x00 },
0x0054,IST_MAX_FWD_VA,0x05,{ 0x00, 0x00 },
0x0060,IST_VARH_Q1,0x02,{ 0x00, 0x00 },
0x0064,IST_VARH_Q2,0x02,{ 0x00, 0x00 },
0x0068,IST_VARH_Q3,0x02,{0x00,0x00},
0x006C,IST_VARH_Q4,0x02,{0x00,0x00},
0x0070,0,0,{0,0},
};
CONST IDENT daily_profile_atr3 = { DT_ARRAY, sizeof(capture_daily_profile) / sizeof(ADDR) - 1, (ADDR)&capture_objects };

CONST CAPDEF K(7, 3, scal_unit3)[] = {
0x000C,IST_BAK_ENERGY,0x03,{0x00,0x00},
0x0010,IST_BAK_VAH,0x03,{0x00,0x00},
0x0014,IST_FWD_ENERGY,0x03,{0x00,0x00},
0x0018,IST_FWD_ENERGY1,0x03,{0x00,0x00},
0x001C,IST_FWD_ENERGY2,0x03,{0x00,0x01},
0x0020,IST_FWD_ENERGY3,0x03,{0x00,0x02},
0x0024,IST_FWD_ENERGY4,0x03,{0x00,0x03},
0x0028,IST_FWD_ENERGY5,0x03,{0x00,0x04},
0x002C,IST_FWD_ENERGY6,0x03,{0x00,0x05},
0x0030,IST_FWD_ENERGY7,0x03,{0x00,0x06},
0x0034,IST_FWD_ENERGY8,0x03,{0x00,0x07},
0x0038,IST_FWD_VAH,0x03,{0x00,0x00},
0x003C,IST_FUND_ENERGY,0x03,{0x00,0x00},
0x0040,IST_MAX_FWD_DEMAND,0x03,{0x00,0x00},
0x0050,IST_MAX_FWD_VA,0x03,{ 0x00, 0x00 },
0x0060,IST_VARH_Q1,0x03,{ 0x00, 0x00 },
0x0064,IST_VARH_Q2,0x03,{ 0x00, 0x00 },
0x0068,IST_VARH_Q3,0x03,{0x00,0x00},
0x006C,IST_VARH_Q4,0x03,{0x00,0x00},
0x0070,0,0,{0,0},
};
CONST IDENT scal_unit3_atr3 = { DT_ARRAY, 19, (ADDR)&capture_objects };

#pragma section COMM
/*******************A1***************************/
CADR  capture_snap_param1[] = {
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_VOLT,
(ADDR)&DT_AMP,
(ADDR)&DT_AMP,
(ADDR)&DT_POWER_FACTOR,
(ADDR)&DT_FREQ,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&ID_FLOAT32,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_BOOLEAN,
(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_FLOAT32,
(ADDR)&ID_FLOAT32,
(ADDR)&ID_FLOAT32,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&DT_WATVARVAH,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_OCTET_STRING12,
(ADDR)&ID_OCTET_STRING4,
(ADDR)&ID_VISIBLE_STRING8,
(ADDR)&ID_OCTET_STRING2,
(ADDR)&ID_FLOAT32,
(ADDR)&DT_WATVARVAH,
(ADDR)&ID_UNSIGNED,
(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, snap_param1)[] = {
0x0000,IST_DATETIME_NOW,0x02,{ 0x00, 0x00 },
0x000C,IST_VOLT0,0x02,{ 0x00, 0x00 },
0x0010,IST_AMPR0,0x02,{ 0x00, 0x00 },
0x0012,IST_AMPRG,0x02,{ 0x00, 0x00 },
0x0014,IST_POWER_FACTOR,0x02,{ 0x00, 0x00 },
0x0016,IST_FREQ,0x02,{ 0x00, 0x00 },
0x0018,IST_APPERENT,0x02,{ 0x00, 0x00 },
0x001C,IST_POWER,0x02,{ 0x00, 0x00 },
0x0020,IST_FWD_ENERGY,0x02,{ 0x00, 0x00 },
0x0024,IST_FWD_VAH,0x02,{ 0x00, 0x00 },
0x0028,IST_MAX_FWD_DEMAND,0x02,{ 0x00, 0x00 },
0x002C,IST_MAX_FWD_DEMAND,0x05,{ 0x00, 0x00 },
0x0038,IST_MAX_FWD_VA,0x02,{ 0x00, 0x00 },
0x003C,IST_MAX_FWD_VA,0x05,{ 0x00, 0x00 },
0x0048,IST_POWER_ON_TIME,0x02,{ 0x00, 0x00 },
0x004C,IST_TAMPER_COUNT,0x02,{ 0x00, 0x00 },
0x004E,IST_BILLING_COUNT,0x02,{ 0x00, 0x00 },
0x0050,IST_PROG_COUNT,0x02,{ 0x00, 0x00 },
0x0052,IST_BAK_ENERGY,0x02,{ 0x00, 0x00 },
0x0056,IST_BAK_VAH,0x02,{ 0x00, 0x00 },
0x005A,IST_BREAKER,0x02,{ 0x00, 0x00 },
0x005C,IST_LOAD_LIMIT,0x03,{ 0x00, 0x00 },
0x0060,IST_BILLING_DATE,0x02,{ 0x00, 0x00 },
0x006C,IST_FWD_DEMAND,0x02,{ 0x00, 0x00 },
0x0070,IST_FWD_VA,0x02,{ 0x00, 0x00 },
0x0074,IST_VOLT_THD,0x02,{ 0x00, 0x00 },
0x0076,IST_AMPR_THD,0x02,{ 0x00, 0x00 },
0x0078,IST_POWER_THD,0x02,{ 0x00, 0x00 },
0x007A,IST_RELAY_COUNT,0x02,{ 0x00, 0x00 },
0x007C,IST_FUND_ENERGY,0x02,{ 0x00, 0x00 },
0x0080,IST_MAX_BAK_DEMAND,0x02,{ 0x00, 0x00 },
0x0084,IST_MAX_BAK_DEMAND,0x05,{ 0x00, 0x00 },
0x0090,IST_MAX_BAK_VA,0x02,{ 0x00, 0x00 },
0x0094,IST_MAX_BAK_VA,0x05,{ 0x00, 0x00 },
0x00A0,IST_4GGSM_PRM,0x02,{ 0x00, 0x00 },
0x00A4,IST_CELLID,0x02,{ 0x00, 0x00 },
0x00AC,IST_RATBAND,0x02,{ 0x00, 0x00 },
0x00AE,IST_CPUTEMP,0x02,{ 0x00, 0x00 },
0x00B0,IST_DEFRAUD_POWER,0x02,{ 0x00, 0x00 },
0x00B4,IST_SELFDIAG,0x02,{ 0x00, 0x00 },
0x00B5,0,0,{0,0},
};

CONST IDENT snap_param1_atr3 = { DT_ARRAY, sizeof(capture_snap_param1) / sizeof(ADDR) - 1, (ADDR)&capture_objects };

CONST CAPDEF K(7, 3, scal_unit1)[] = {
0x000C,IST_VOLT0,0x03,{ 0x00, 0x00 },
0x0010,IST_AMPR0,0x03,{ 0x00, 0x00 },
0x0012,IST_AMPRG,0x03,{ 0x00, 0x00 },
0x0014,IST_POWER_FACTOR,0x03,{ 0x00, 0x00 },
0x0016,IST_FREQ,0x03,{ 0x00, 0x00 },
0x0018,IST_APPERENT,0x03,{ 0x00, 0x00 },
0x001C,IST_POWER,0x03,{ 0x00, 0x00 },
0x0020,IST_FWD_ENERGY,0x03,{ 0x00, 0x00 },
0x0024,IST_FWD_VAH,0x03,{ 0x00, 0x00 },
0x0028,IST_MAX_FWD_DEMAND,0x03,{ 0x00, 0x00 },
0x0038,IST_MAX_FWD_VA,0x03,{ 0x00, 0x00 },
0x0048,IST_POWER_ON_TIME,0x03,{ 0x00, 0x00 },
0x0052,IST_BAK_ENERGY,0x03,{ 0x00, 0x00 },
0x0056,IST_BAK_VAH,0x03,{ 0x00, 0x00 },
0x005F,IST_BILLING_DATE,0x03,{ 0x00, 0x00 },
0x006B,IST_FWD_DEMAND,0x03,{ 0x00, 0x00 },
0x006F,IST_FWD_VA,0x03,{ 0x00, 0x00 },
0x0073,IST_VOLT_THD,0x03,{ 0x00, 0x00 },
0x0075,IST_AMPR_THD,0x03,{ 0x00, 0x00 },
0x0077,IST_POWER_THD,0x03,{ 0x00, 0x00 },
0x007B,IST_FUND_ENERGY,0x03,{ 0x00, 0x00 },
0x007F,IST_MAX_BAK_DEMAND,0x03,{ 0x00, 0x00 },
0x008F,IST_MAX_BAK_VA,0x03,{ 0x00, 0x00 },
0x00A9,IST_CPUTEMP,0x03,{ 0x00, 0x00 },
0x00AB,IST_DEFRAUD_POWER,0x03,{ 0x00, 0x00 },
0x00B0,0,0,{0,0},
};
CONST IDENT scal_unit1_atr3 = { DT_ARRAY, 25, (ADDR)&capture_objects };

/************ name plate ********************/
CADR  capture_name_plate[] = {
(ADDR)&ID_VISIBLE_STRING11,
(ADDR)&ID_VISIBLE_STRING14,
(ADDR)&ID_VISIBLE_STRING11,
(ADDR)&ID_VISIBLE_STRING15,
(ADDR)&ID_UNSIGNED,
(ADDR)&ID_VISIBLE_STRING2,
(ADDR)&ID_VISIBLE_STRING10,
(ADDR)&ID_LONG_UNSIGNED,
(ADDR)&ID_VISIBLE_STRING15,
(ADDR)&ID_VISIBLE_STRING40,
(ADDR)&ID_NULL_DATA,
};

CONST CAPDEF K(7, 3, name_plate)[] = {
0x0000,IST_SERIAL_NO,0x02, { 0x00, 0x00 },
0x000B,IST_DEVICE_ID,0x02, { 0x00, 0x00 },
0x0019,IST_MANUFACTURE_NAME,0x02, { 0x00, 0x00 },
0x0024,IST_FIRM_VER,0x02, { 0x00, 0x00 },
0x0033,IST_METER_TYPE,0x02, { 0x00, 0x00 },
0x0034,IST_CATEGORY,0x02, { 0x00, 0x00 },
0x0036,IST_CURRENT_RATING,0x02, { 0x00, 0x00 },
0x0040,IST_METER_YEAR,0x02, { 0x00, 0x00 },
0x0042,IST_NIC_IMEI,0x02, { 0x00, 0x00 },
0x0051,IST_NIC_VER,0x02, { 0x00, 0x00 },
0x0079,0,0,{0,0},
};
CONST IDENT name_plate_atr3 = { DT_ARRAY, 10, (ADDR)&capture_objects };

/**********************************************/
/**********************************************/
#define	CL7_METER_LOG			0
#define	CL7_SNAP_PARAM1			1
#define	CL7_SCAL_UNIT1			2
#define	CL7_LOAD_PROFILE		3
#define	CL7_SCAL_UNIT2			4
#define	CL7_DAILY_PROFILE		5
#define	CL7_SCAL_UNIT3			6
#define	CL7_BILLING_PARAMS		7
#define	CL7_SCAL_UNIT4			8
#define	CL7_EVENT_PROFILE		9
#define	CL7_OPERATE_PROFILE		10
#define	CL7_SCAL_UNIT5			11
#define	CL7_RELAY_LOG			12
#define	CL7_NAME_PLATE			13

CONST IDENT cl7_entry[] = {
	{ DT_STRUCTURE, 2, (ADDR)capture_meter_log},
	{ DT_STRUCTURE, 40, (ADDR)capture_snap_param1},
	{ DT_STRUCTURE, 25, (ADDR)capture_scal_unit},/*scal_unit1*/
	{ DT_STRUCTURE, 12, (ADDR)capture_load_profile },
	{ DT_STRUCTURE, 11, (ADDR)capture_scal_unit},/*scal_unit2*/
	{ DT_STRUCTURE, 22, (ADDR)capture_daily_profile },
	{ DT_STRUCTURE, 19, (ADDR)capture_scal_unit},/*scal_unit3*/
	{ DT_STRUCTURE, 134, (ADDR)capture_billing_params },
	{ DT_STRUCTURE, 98, (ADDR)capture_scal_unit},/*scal_unit4*/
	{ DT_STRUCTURE, 10, (ADDR)capture_event_profile},
	{ DT_STRUCTURE, 4, (ADDR)capture_operate_profile},
	{ DT_STRUCTURE, 6, (ADDR)capture_scal_unit},/*scal_unit5*/
	{ DT_STRUCTURE, 25, (ADDR)capture_relay_log},
	{ DT_STRUCTURE, 10, (ADDR)capture_name_plate},
};
#pragma section

/****************************************************************/
ui08 echo_function(CADR in)
{
	return TRUE;
}
#if 0
/****************************************************************/
ui08 compare_data(cpui08 from, cui32 to)
{
	ui08 type, res, data[4];

	data[0] = UI8_PTR(&to)[3];
	data[1] = UI8_PTR(&to)[2];
	data[2] = UI8_PTR(&to)[1];
	data[3] = UI8_PTR(&to)[0];
	res = CMP_INVALID;
	type = from[0];
	switch (type)
	{
	case DT_DOUBLE_LONG_UNSIGNED:	/* 1+4 */
		res = compare_u32(&from[1], data);
		break;

	case DT_UNSIGNED:				/* 1+1 */
		res = compare_u08(&from[1], &data[3]);
		break;

	case DT_LONG_UNSIGNED:			/* 1+2 */
		res = compare_u16(&from[1], &data[2]);
		break;

	case DT_LONG64_UNSIGNED:		/* 1+8 */
	case DT_DATE_TIME:
	case DT_DATE:					/* 1+5 */
	case DT_TIME:					/* 1+4 */
	case DT_DOUBLE_LONG:			/* 1+4 */
	case DT_INTEGER:				/* 1+1 */
	case DT_LONG:					/* 1+2 */
	case DT_LONG64:					/* 1+8 */
	case DT_FLOAT32:				/* 1+4 */
	case DT_FLOAT64:				/* 1+8 */
	case DT_OCTET_STRING:			/* 可変長 */
	case DT_VISIBLE_STRING:			/* 可変長 */
	case DT_UTF8_STRING:			/* 可変長 */
	default:
		break;
	};
	return res;
}
#endif
/****************************************************************/
ui08 compare_data08(cpui08 from, cpui08 to)
{
	ui08 type, res;

	res = CMP_INVALID;
	type = from[0];
	switch (type)
	{
	case DT_DOUBLE_LONG_UNSIGNED:	/* 1+4 */
		res = compare_u32(&from[1], &to[1]);
		break;

	case DT_UNSIGNED:				/* 1+1 */
		res = compare_u08(&from[1], &to[1]);
		break;

	case DT_LONG_UNSIGNED:			/* 1+2 */
		res = compare_u16(&from[1], &to[1]);
		break;

	case DT_LONG64_UNSIGNED:		/* 1+8 */
		res = compare_u32(&from[1], &to[1]);
		if (CMP_MATCH == res)
		{
			res = compare_u32(&from[5], &to[5]);
		}
		break;
	case DT_DATE_TIME:
		res = compare_date(&from[1], &to[1]);
		if (CMP_MATCH == res)
		{
			res = compare_time(&from[6], &to[6]);
		}
		break;

	case DT_DATE:					/* 1+5 */
		res = compare_date(&from[1], &to[1]);
		break;

	case DT_TIME:					/* 1+4 */
		res = compare_time(&from[1], &to[1]);
		break;

	case DT_DOUBLE_LONG:			/* 1+4 */
	case DT_INTEGER:				/* 1+1 */
	case DT_LONG:					/* 1+2 */
	case DT_LONG64:					/* 1+8 */
	case DT_FLOAT32:				/* 1+4 */
	case DT_FLOAT64:				/* 1+8 */
	case DT_OCTET_STRING:			/* 可変長 */
	case DT_VISIBLE_STRING:			/* 可変長 */
	case DT_UTF8_STRING:			/* 可変長 */
	default:
		break;
	};
	return res;
}

/****************************************************************/
void cpy_correct_datetime(pui08 out, cpui08 in)
{
	ui08 day;

	/* カレンダー型でワイルドカードを使っていれば、現在時刻に置き換える。 */
	if (0xffff == UI16L(in))	/* year */
	{
		ramcpy(out, I(8, 2, datetime_now), 2);	/* ワイルドカード処理 */
	}
	else
	{
		ramcpy(out, (pui08)in, 2);
	}

	if (0xff == in[2])			/* month */
	{
		out[2] = I(8, 2, datetime_now)[2];				/* ワイルドカード処理 */
	}
	else
	{
		out[2] = in[2];
	}

	if (0xfd <= in[3])			/* dayOfMonth */
	{
		/* ワイルドカード処理 */
		out[3] = I(8, 2, datetime_now)[3];
		if (bCheckYear(out) && bCheckMonth(out))
		{
			day = uGetLastDayOfMonth(out);
			if (0xfe == in[3])
			{
				out[3] = day;
			}
			if (0xfd == in[3])
			{
				out[3] = day - 1;
			}
		}
	}
	else
	{
		out[3] = in[3];
	}

	if (0xff == in[5])			/* hour */
	{
		out[5] = I(8, 2, datetime_now)[5];	/* ワイルドカード処理 */
	}
	else
	{
		out[5] = in[5];
	}

	if (0xff == in[6])			/* min */
	{
		out[6] = I(8, 2, datetime_now)[6];	/* ワイルドカード処理 */
	}
	else
	{
		out[6] = in[6];
	}

	if (0xff == in[7])			/* sec */
	{
		out[7] = I(8, 2, datetime_now)[7];	/* ワイルドカード処理 */
	}
	else
	{
		out[7] = in[7];
	}
}

/****************************************************************/
ui08 take_capture_object(void)
{
	ui08 i, ret;
	ui16 obis;

	ret = DLMS_INVALID;
	if (TRUE == take_attr((ADDR)&capture_objects)) {
		if (UI16B(&g_xdlms_io[0]) == g_cls[g_ist[IST_DATETIME_NOW].cls].cid) {
			if (0 == romcmp(&g_xdlms_io[2], g_ist[IST_DATETIME_NOW].oid, 5)) {
				if (cl7_idx == CL7_BILLING_PARAMS) {
					obis = IST_BILLING_DATE;
				}
				else {
					obis = IST_DATETIME_NOW;
				}
				for (i = 0; i < cl7_entry[cl7_idx].l; i++) {
					if (0 == romcmp(g_ist[obis].oid, g_ist[capdef[i].ist].oid, 5)) {
						ret = i;
						break;
					}
				}
				if (g_xdlms_io[8] != ((ui08)capdef[ret].atr)) {
					ret = DLMS_INVALID;
				}
				if (UI16B(&g_xdlms_io[9]) != UI16L(capdef[ret].dat)) {
					ret = DLMS_INVALID;
				}
			}
		}
		else {
			for (i = 0; i < cl7_entry[cl7_idx].l; i++) {
				if (0 == romcmp(&g_xdlms_io[2], g_ist[capdef[i].ist].oid, 5)) {
					ret = i;
					break;
				}
			}
			if (g_xdlms_io[8] != ((ui08)capdef[ret].atr)) {
				ret = DLMS_INVALID;
			}
			if (UI16B(&g_xdlms_io[9]) != UI16L(capdef[ret].dat)) {
				ret = DLMS_INVALID;
			}
		}
	}
	if (ret == DLMS_INVALID){
		result_xdlms = RET_TEMPORARY_FAILURE;
	}
	return ret;
}

/****************************************************************/
ui08 cl7_selacc1(void)
{
	ui08	i, tar;
	uint	count;
	ui08	cnt_line[8];
	PIDNT	pidnt;

	if (FALSE == take_attr((ADDR)&ID_STRUCTURE0)) {
		return FALSE;
	}

	count = decode_len(&g_xdlms_req.pos, g_xdlms_req.buf);
	if (!(4 == count))
	{
		/* Error : unmatch selective access1 parameter member */
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return FALSE;
	}

	tar = take_capture_object();
	if (DLMS_INVALID == tar)
	{
		/* Fatal : incorrect remain raw */
		record_log(ETSN1, 21);
		return FALSE;
	}

	/* 検索対象列の設定 */
	restricting = capdef[tar].ist;
	pidnt = (PIDNT)cap_lst[tar];
	/* ワイルドカード用に日付を取得する。 */
	get_meter_datetime();

	if (FALSE == take_attr((ADDR)pidnt))
	{
		return FALSE;
	}
	if (pidnt->t == 0x09 && pidnt->l == 0x0c) {
		/* 開始値の設定 */
		record_from[0] = DT_DATE_TIME;
	}
	else {
		/* 開始値の設定 */
		record_from[0] = pidnt->t;
	}
	if (DT_DATE_TIME == record_from[0])
	{
		cpy_correct_datetime(&record_from[1], g_xdlms_io);
		if (FALSE == bCheckDateTime(&record_from[1], FALSE))
		{
			/* Error : invalid from datetime */
			record_log(ETSN1, 21);
			result_xdlms = RET_TEMPORARY_FAILURE;
			return FALSE;
		}
	}
	else
	{
		/* 日時型以外はコピー */
		ramcpy(&record_from[1], g_xdlms_io, pidnt->l);
	}

	if (FALSE == take_attr((ADDR)pidnt))
	{
		return FALSE;
	}

	/* 終了値 */
	if (pidnt->t == 0x09 && pidnt->l == 0x0c) {
		/* 開始値の設定 */
		record_to[0] = DT_DATE_TIME;
	}
	else {
		/* 開始値の設定 */
		record_to[0] = pidnt->t;
	}
	if (DT_DATE_TIME == record_to[0])
	{
		cpy_correct_datetime(&record_to[1], g_xdlms_io);
		if (FALSE == bCheckDateTime(&record_to[1], FALSE))
		{
			/* Error : invalid to datetime */
			record_log(ETSN1, 21);
			result_xdlms = RET_TEMPORARY_FAILURE;
			return FALSE;
		}
	}
	else
	{
		/* 日時型以外はコピー */
		ramcpy(&record_to[1], g_xdlms_io, pidnt->l);
	}

	if (CMP_VALID < compare_data08(record_from, record_to))	/* from is bigger(newest) than to */
	{
		/* Error : from is bigger(newer) than to */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}

	if (FALSE == take_attr((ADDR)&ID_ARRAY0))
	{
		return FALSE;
	}

	/* 個数の取得 */
	count = decode_len(&g_xdlms_req.pos, g_xdlms_req.buf);
	if (count > cl7_entry[cl7_idx].l)
	{
		/* Error : line member bigger than capture menber */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	if (0 < count)	/* selected_values is specified */
	{
		ramset(cnt_line, 0x00, sizeof(cnt_line));	/* 選択列をすべて0x00にセットする。 */
		for (i = 0; i < count; i++)
		{
			tar = take_capture_object();
			if (DLMS_INVALID == tar)
			{
				return FALSE;
			}
			colm_sel[i] = tar;/* 収集列のセット */
			cnt_line[tar]++;
			if (cnt_line[tar] > 1)
			{
				/* Error : invalid line requested */
				record_log(ETSN1, 21);
				result_xdlms = RET_TEMPORARY_FAILURE;
				return FALSE;
			}
		}
	}
	else
	{
		ramcpy(colm_sel, colm_sel_def, sizeof(colm_sel));
	}
	return TRUE;
}

/****************************************************************/
ui08 cl7_selacc2(void) {
	uint	i, j;
	ui32	f, t, entry;
	ui08	mbr;

	entry = record_count;
	record_from[0] = DT_DOUBLE_LONG_UNSIGNED;
	record_to[0] = DT_DOUBLE_LONG_UNSIGNED;
	colm_from[0] = DT_LONG_UNSIGNED;
	colm_to[0] = DT_LONG_UNSIGNED;

	if (FALSE == take_attr((ADDR)&sel_entry_desc)) {
		/* Error : unmatch selective access2 parameter type */
		return FALSE;
	}

	ramcpy(&record_from[1], &g_xdlms_io[0], SZ_DOUBLE_LONG_UNSIGNED);
	ramcpy(&record_to[1], &g_xdlms_io[4], SZ_DOUBLE_LONG_UNSIGNED);
	ramcpy(&colm_from[1], &g_xdlms_io[8], SZ_LONG_UNSIGNED);
	ramcpy(&colm_to[1], &g_xdlms_io[10], SZ_LONG_UNSIGNED);

	f = UI32B(&record_from[1]);
	t = UI32B(&record_to[1]);

	if (0 == f) {	/* 0 ならエラー */
		/* Error : record from is zero */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}

	if (!(0 == t))	/* 0 ならcl7_entry in use を使う */
	{
		if (CMP_VALID < compare_data08(record_from, record_to))/* comp big or small, new or old */
		{
			/* Error : record from bigger than to */
			record_log(ETSN1, 21);
			result_xdlms = RET_TEMPORARY_FAILURE;
			return FALSE;
		}
	}
	if (f > entry)				/* 開始Indexが範囲外 */
	{
		record_count = 0;
	}
	else {
		/* fromは、entries_in_useより小さいので、toの評価 */
		if ((0 == t) || (t > entry))
		{
			/* toが0、もしくは、entries_in_useより大きいければ丸める */
			ramcpy(&record_to[1], (cpui08)&entry, 4);
			/* Info : Map record to */
		}
	}

	if (0 == UI16B(&colm_from[1]))
	{
		/* Error : line from is zero */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}

	mbr = cl7_entry[cl7_idx].l;
	if (UI16B(&colm_from[1]) > mbr)
	{
		/* Error : line from bigger than capture objects */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	if ((0 == UI16B(&colm_to[1])) || (mbr < UI16B(&colm_to[1])))
	{
		colm_to[1] = 0;
		colm_to[2] = mbr;
		/* Info : Map line to */
	}
	if (CMP_VALID < compare_data08(colm_from, colm_to))
	{
		/* Error : line from bigger than to */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	j = 0;
	for (i = UI16B(&colm_from[1]); i <= UI16B(&colm_to[1]); i++)
	{
		if (i > mbr)
		{
			break;
		}
		colm_sel[j] = i - 1;
		j++;
	}
	return TRUE;
}

/****************************************************************/
void init_search_pram(void)
{
	/* 初期化 */
	cap_idt.t = cl7_entry[cl7_idx].t;
	cap_idt.e = NULL;
	cap_idt.l = 0;
	cap_lst = (PADR)cl7_entry[cl7_idx].e;

	ramset(record_from, 0, sizeof(record_from));
	ramset(record_to, 0, sizeof(record_to));
	ramset(colm_from, 0, sizeof(colm_from));
	ramset(colm_to, 0, sizeof(colm_to));
	ramset(colm_sel, DLMS_INVALID, sizeof(colm_sel));	/* 選択列をすべて0xffにセットする。 */
	restricting = IST_ENTRY;								/* 検索対象列の初期化 */
	sent_count = 0;
	send_count = 0;
	record_offset = 0;
	record_i = 0;
	record_j = 0;
}

/****************************************************************/
void init_record_count_meter(void) {
	/* 登録数の取得 */
	record_count = get_entry_in_use(profile);	/* エントリー数の取得 */
}

/****************************************************************/
void init_record_count_meter1(void) {
	/* 登録数の取得 */
	record_count = 1;
}

/****************************************************************/
void set_search_param(ui08 order) {
	uint	i, arg;
	ui32	one = 1;
	ui08	mbr;
	PIDNT	pidnt;

	mbr = cl7_entry[cl7_idx].l;
	arg = order;
	switch (order)
	{
	case 1:	/* SelectiveAccess1 */
		if (TRUE == cl7_selacc1())
		{
			optional_process((ADDR)&arg);
		}
		break;
	case 2:	/* SelectiveAccess2 */
		if (TRUE == cl7_selacc2())
		{
			optional_process((ADDR)&arg);
		}
		break;
	default:	/* バッファ取得 */
		/* 開始位置をセット */
		record_from[0] = DT_DOUBLE_LONG_UNSIGNED;
		ramcpy(&record_from[1], (cpui08)&one, 4);
		/* 終了位置をセット */
		record_to[0] = DT_DOUBLE_LONG_UNSIGNED;
		ramcpy(&record_to[1], (cpui08)&record_count, 2);
		/* 列をセット */
		ramcpy(colm_sel, (pui08)colm_sel_def, mbr);
		optional_process((ADDR)&arg);
		break;

	};
	/* レコードサイズの計算と出力列のカウント */
	if (RET_SUCCESS == result_xdlms) {
		record_size = 0;
		cap_idt.l = 0;
		for (i = 0; i < mbr; i++)
		{
			if (!(DLMS_INVALID == colm_sel[i]))
			{
				/* 開始列の評価 */
				if (colm_sel[i] < mbr)
				{
					cap_idt.l++;
				}
				else
				{
					colm_sel[i] = DLMS_INVALID;
				}
			}
			if (NULL != cap_lst[i])
			{
				pidnt = (PIDNT)cap_lst[i];
				record_size += capdef[i + 1].oft - capdef[i].oft;
			}
		}
	}
}

/****************************************************************/
/*																*/
/*	関数名	: search_record( void )							*/
/*																*/
/*	内容		:													*/
/*		イベントデータ/有効電力量30分値/平均電圧30分値				*/
/*		/開閉動作履歴検索											*/
/*																*/
/*	引　数	:なし												*/
/*																*/
/*	戻り値	:なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴														*/
/*			2014-06-30 T.Kuroda get_searchdata 戻り値追加		*/
/*			2014-10-14 T.Kuroda lastblock判定改善				*/
/*																*/
/****************************************************************/
ui08 search_record(void)
{
	uint	offset, diff, proceed;
	ui16	find;
	PIDNT	pidnt;

	ui08	cnt, ret, res, colm;

	ret = PHASE_RESPONSE;
	res = SRCH_END;

	if (0 == record_count) {
		put_complex_count_non(record_count, &ID_ARRAY0);
	}
	else {
		if (heapsize(record_offset) == 0) {
			cnt = 0;
			if (0 == block_no) {
				find = 0;
				res = get_searchdata(&find, &cnt);
				if ((SRCH_HOLD != res) && (SRCH_ERROR != res)) {
					record_count = find;
					put_complex_count_non(record_count, &ID_ARRAY0);
				}
			}
			else {					/* second or later response */
				res = get_searchdata_next(&cnt);
			}
			send_count += cnt;
			switch (res) {
			case SRCH_ERROR:
				last_block = TRUE;
				result_xdlms = RET_HARDWARE_FAULT;
				break;
			case SRCH_HOLD:
				ret = PHASE_THREAD;		/* 一旦メインに抜ける */
				break;
			case SRCH_END:
				sent_count = record_count - send_count;
				break;
			default:
				break;
			};
		}
		if (RET_SUCCESS == result_xdlms) {
			if (PHASE_RESPONSE == ret) {
				readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset);
				offset = 0;
				proceed = 0 - record_offset;
				cnt = cl7_entry[cl7_idx].l;
				for (; record_i < send_count; record_i++) {
					if (record_j == 0) {
						put_complex_count_non(cap_idt.l, &ID_STRUCTURE0);
						put_ident.t = 0;
					}
					for (; record_j < cnt; record_j++) {
						colm = colm_sel[record_j];
						idxdat = capdef[colm].ist;
						if (!(DLMS_INVALID == colm)) {
							diff = capdef[colm + 1].oft - capdef[colm].oft;
							if ((proceed + capdef[colm].oft + diff) > MAX_XDLMS_BUF) {
								readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset);
								proceed = 0 - record_offset;
								offset = 0;
							}
							if (put_ident.t == 0) {
								pidnt = (PIDNT)cap_lst[colm];
								put_ident.t = pidnt->t;
								offset = proceed + capdef[colm].oft;
								if (put_ident.t == DT_ARRAY) {
									put_ident.l = g_xdlms_io[offset];
									put_ident.e = pidnt->e;
									offset += 2;
									record_offset+=2;
								}
								else {
									put_ident.l = pidnt->l;
									put_ident.e = NULL;
								}
							}
							record_offset += put_attr_non(&g_xdlms_io[offset], diff, (ADDR)&put_ident);
							if (axdr_max == 0) {
								break;
							}
							else {
								put_ident.t = 0;
							}
						}
					}
					if (axdr_max == 0) {
						break;
					}
					else {
						record_j = 0;
						proceed += record_size;
					}
				}
				if (axdr_max == 0) {
					if (0 == block_no) {
						last_block = FALSE;
						res_mode = ORDER_BLOCK;
					}
				}
				else {
					sent_count += send_count;
					if (0 == block_no) {		/* 最初の応答 */
						if (record_count > sent_count) {
							last_block = FALSE;
							res_mode = ORDER_BLOCK;
						}
					}
					if (record_count <= sent_count) {
						last_block = TRUE;		/* 件数が超えた場合も最終フラグON */
					}
					else {
						last_block = FALSE;
					}
					send_count = 0;
					record_offset = 0;
					record_i = 0;
				}
			}
			else {
				/* 継続 */
			}
		}
	}
	return ret;
}

/****************************************************************/
void search_record1(void)
{
	ui08	cur[6];
	ui08	atr;
	ui16	ist;

	/* バッファーに登録数をセット */
	if (0 == block_no) {		/* 最初の応答 */
		record_count = cl7_entry[cl7_idx].l;
		put_complex_count(1, &ID_ARRAY0);
		put_complex_count(cap_idt.l, &ID_STRUCTURE0);
	}
	ramcpy(cur, obis_code, sizeof(obis_code));
	for (; sent_count < record_count; sent_count++) {
		if (!(DLMS_INVALID == colm_sel[sent_count])) {
			ist = capdef[colm_sel[sent_count]].ist;
			romcpy(obis_code, g_ist[ist].oid, 5);
			atr = capdef[colm_sel[sent_count]].atr;
			if ((atr & 0x80) > 0) {
				atr &= ~0x80;
				atr_shift = 1;
			}
			else {
				atr_shift = 0;
			}
			Execute(G0, ist, atr - 2);
			if (xdlms_phase == PHASE_THREAD) {
				while (PHASE_RESPONSE != thread_process());
			}
			atr_shift = 0;
			if (!(RET_SUCCESS == result_xdlms)) {
				record_log(ETSN2, 7);
				break;
			}
			if (axdr_max == 0) {
				break;
			}
		}
	}
	ramcpy(obis_code, cur, sizeof(obis_code));
	xdlms_phase = PHASE_RESPONSE;
	if (axdr_max != 0) {
		last_block = TRUE;	/* 件数が超えた場合も最終フラグON */
	}
	else {
		if (0 == block_no) {		/* 最初の応答 */
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
	}
}

/****************************************************************/
void put_capture_objects(PCAPDEF pcap, cui08 count) {

	uint size, i;
	DAT2 dat2;

	size = 0;
	for (i = 0; i < count; i++) {
		dat2.hf = g_cls[g_ist[pcap[i].ist].cls].cid;
		size += ramcpy(&share_wk0[size], dat2.by, 2);
		size += romcpy(&share_wk0[size], &g_ist[pcap[i].ist].oid[0], 5);
		share_wk0[size] = 0xff;
		size++;
		share_wk0[size] = pcap[i].atr;
		size++;
		size += romcpy(&share_wk0[size], pcap[i].dat, 2);
		writeheap(share_wk0, size);
		size = 0;
	}
}

void put_capture_objects2(PCAPDEF2 pcap, cui08 count) {

	uint size, i;
	DAT2 dat2;

	size = 0;
	for (i = 0; i < count; i++) {
		dat2.hf = g_cls[g_ist[pcap[i].ist].cls].cid;
		size += ramcpy(&share_wk0[size], dat2.by, 2);
		size += romcpy(&share_wk0[size], &g_ist[pcap[i].ist].oid[0], 5);
		share_wk0[size] = 0xff;
		size++;
		share_wk0[size] = pcap[i].atr;
		size++;
		size += romcpy(&share_wk0[size], pcap[i].dat, 2);
//		size += romcpy(&share_wk0[size], pcap[i].restriction, sizeof(pcap[i].restriction));
		size += romcpy(&share_wk0[size], pcap[i].restriction, 9);
		writeheap(share_wk0, size);
		size = 0;
	}
}
/****************************************************************/
void init_search(ui08 order) {
	ui16	search_code[2];

	if (0 == block_no)
	{
		init_search_pram();
		init_record_count_meter();
		set_search_param(order);
		if (!(RET_SUCCESS == result_xdlms)) {
			return;
		}
		if (record_count > 0) {
			/* 検索実行 */
			search_code[0] = restricting;
			search_code[1] = profile;
			rec_search_ini((pui08)search_code, &record_from[1], &record_to[1]);
		}
	}
	thread_process = search_record;
	xdlms_phase = PHASE_THREAD;
}

#define cl7_function( ist, INST )\
void G(0,7,2,ist)( void ) \
{\
	cl7_idx	=	CL7_##INST;\
	profile	=	IST_##INST;\
	capdef	=	(PCAPDEF)K(7,3,ist);\
	optional_process = optional_##ist;\
	init_search( 0 );\
}\
void G(1,7,2,ist)( void )\
{\
	cl7_idx	=	CL7_##INST;\
	profile	=	IST_##INST;\
	capdef	=	(PCAPDEF)K(7,3,ist);\
	optional_process = optional_##ist;\
	init_search( 1 );\
}\
void G(2,7,2,ist)( void )\
{\
	cl7_idx	=	CL7_##INST;\
	profile	=	IST_##INST;\
	capdef	=	(PCAPDEF)K(7,3,ist);\
	optional_process = optional_##ist;\
	init_search( 2 );\
}\
void G(0,7,3,ist)( void )\
{\
	if(block_no == 0){\
		record_offset = 0;\
		put_capture_objects( &K(7,3,ist)[0], cl7_entry[CL7_##INST].l);\
	}\
	readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset); \
	record_offset += put_attr(g_xdlms_io, 0, &##ist##_atr3);\
}\
void G(0,7,5,ist)( void ) {\
	put_attr( (pui08)&I(7,5,profile_generic), 0, (ADDR)cl7_common_atr[0] );\
}\
void G(0,7,6,ist)( void ) {\
	put_attr( (pui08)I(7,6,profile_generic), 0, (ADDR)cl7_common_atr[2] );\
}\
void G(0,7,7,ist)( void ) {\
	ui32 count;\
	count = get_entry_in_use( IST_##INST );\
	put_attr( (pui08)&count, 0, (ADDR)cl7_common_atr[3] );\
}\
void G(0,7,8,ist)( void ) {\
	ui32 count;\
	count = get_profile_entry( IST_##INST );\
	put_attr( (pui08)&count, 0, (ADDR)cl7_common_atr[4] );\
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void init_search1(ui08 order) {
	if (0 == block_no)
	{
		init_search_pram();
		init_record_count_meter1();
		set_search_param(order);
		if (!(RET_SUCCESS == result_xdlms))
		{
			return;
		}
	}
	search_record1();
}

#define cl7_function1( ist, INST )\
void G(0,7,2,ist)( void ) \
{\
	cl7_idx	=	CL7_##INST;\
	profile	=	IST_##INST;\
	capdef	=	(PCAPDEF)K(7,3,ist);\
	optional_process = optional_##ist;\
	init_search1( 0 );\
}\
void G(1,7,2,ist)( void ) \
{\
	cl7_idx	=	CL7_##INST;\
	profile	=	IST_##INST;\
	capdef	=	(PCAPDEF)K(7,3,ist);\
	optional_process = optional_##ist;\
	init_search1( 0 );\
}\
void G(2,7,2,ist)( void ) \
{\
	cl7_idx	=	CL7_##INST;\
	profile	=	IST_##INST;\
	capdef	=	(PCAPDEF)K(7,3,ist);\
	optional_process = optional_##ist;\
	init_search1( 0 );\
}\
void G(0,7,3,ist)( void )\
{\
	if (block_no == 0) {\
		record_offset = 0; \
		put_capture_objects(&K(7, 3, ist)[0], cl7_entry[CL7_##INST].l); \
	}\
	readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset); \
	record_offset += put_attr(g_xdlms_io,0, &##ist##_atr3);\
}\
void G(0, 7, 4, ist)(void) { \
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]); \
}\
void G(0, 7, 5, ist)(void) { \
	put_attr((pui08)&I(7, 5, profile_generic), 0, (ADDR)cl7_common_atr[1]); \
}\
void G(0, 7, 6, ist)(void) {\
	put_attr((pui08)I(7, 6, profile_generic), 0, (ADDR)cl7_common_atr[2]); \
}\
void G(0, 7, 7, ist)(void) {\
	ui32 count = 1; \
	put_attr((pui08)&count, 0, (ADDR)cl7_common_atr[3]); \
}\
void G(0, 7, 8, ist)(void) {\
	ui32 count = 1; \
	put_attr((pui08)&count, 0, (ADDR)cl7_common_atr[4]); \
}

#define cl7_function0( ist, INST )\
void G(0,7,2,ist)( void ){}\
void G(0,7,3,ist)( void ){}

/********* event_profile *********/
const  ARG optional_meter_log = echo_function;
cl7_function(meter_log, METER_LOG)
void G(0, 7, 4, meter_log)(void) {
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]);
}
void M(7, 1, meter_log)(void) {
	reset(IST_METER_LOG, rec_reset);
}

/********* event_profile *********/
void G(0,7,2, event_profile)( void ) 
{
	cl7_idx	=	CL7_EVENT_PROFILE;
	profile	=	idxobj;
	capdef = (PCAPDEF)K(7, 3, event_profile);
	optional_process = echo_function;
	init_search( 0 );
}
void G(1,7,2, event_profile)( void )
{
	cl7_idx	=	CL7_EVENT_PROFILE;
	profile	= idxobj;
	capdef = (PCAPDEF)K(7, 3, event_profile);
	optional_process = echo_function;
	init_search( 1 );
}

void G(2,7,2, event_profile)( void )
{
	cl7_idx	=	CL7_EVENT_PROFILE;
	profile	= idxobj;
	capdef = (PCAPDEF)K(7, 3, event_profile);
	optional_process = echo_function;
	init_search( 2 );
}
void G(0, 7, 3, event_profile)(void) {

	CAPDEF evedef[sizeof(K(7, 3, event_profile))/sizeof(CAPDEF)];
	if (block_no == 0) {
		record_offset = 0; 
		romcpy(&evedef[0], &K(7, 3, event_profile)[0], sizeof(evedef));
		evedef[1].ist = IST_EVENT1 + obis_code[4];
		evedef[8].ist = IST_EVENT1_RNO + obis_code[4];
		put_capture_objects(&evedef[0], cl7_entry[CL7_EVENT_PROFILE].l);
	}
	readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset);
	record_offset += put_attr(g_xdlms_io, 0, &event_profile_atr3);
}
void G(0, 7, 4, event_profile)(void) {
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]);
}
void G(0, 7, 5, event_profile)(void) {
	put_attr((pui08)&I(7, 5, profile_generic), 0, (ADDR)cl7_common_atr[1]);
}
void G(0, 7, 6, event_profile)(void) {
	put_attr((pui08)I(7, 6, profile_generic), 0, (ADDR)cl7_common_atr[2]);
}
void G(0,7,7, event_profile)( void ) {
	ui32 count;
	count = get_entry_in_use(idxobj);
	put_attr( (pui08)&count, 0, (ADDR)cl7_common_atr[3] );
}

void G(0,7,8, event_profile)( void ) {
	ui32 count;
	count = get_profile_entry(idxobj);
	put_attr( (pui08)&count, 0, (ADDR)cl7_common_atr[4] );
}

void M(7, 1, event_profile)(void) {
	reset(idxobj, rec_reset);
}

/********* operate_profile *********/
void G(0, 7, 2, operate_profile)(void) {
	cl7_idx = CL7_OPERATE_PROFILE;
	profile = idxobj;
	capdef = (PCAPDEF)K(7, 3, operate_profile);
	optional_process = echo_function;
	init_search(0);
}

void G(1, 7, 2, operate_profile)(void) {
	cl7_idx = CL7_OPERATE_PROFILE;
	profile = idxobj;
	capdef = (PCAPDEF)K(7, 3, operate_profile);
	optional_process = echo_function;
	init_search(1);
}

void G(2, 7, 2, operate_profile)(void) {
	cl7_idx = CL7_OPERATE_PROFILE;
	profile = idxobj;
	capdef = (PCAPDEF)K(7, 3, operate_profile);
	optional_process = echo_function;
	init_search(2);
}
void G(0, 7, 3, operate_profile)(void) {
	CAPDEF evedef[sizeof(K(7, 3, operate_profile)) / sizeof(CAPDEF)];
	if (block_no == 0) {
		record_offset = 0;
		romcpy(&evedef[0], &K(7, 3, operate_profile)[0], sizeof(evedef));
		evedef[1].ist = IST_EVENT1 + obis_code[4];
		evedef[2].ist = IST_EVENT1_RNO + obis_code[4];
		put_capture_objects(&evedef[0], cl7_entry[CL7_OPERATE_PROFILE].l);
	}
	readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset);
	record_offset += put_attr(g_xdlms_io, 0, &operate_profile_atr3);
}
void G(0, 7, 4, operate_profile)(void) {
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]);
}
void G(0, 7, 5, operate_profile)(void) {
	put_attr((pui08)&I(7, 5, profile_generic), 0, (ADDR)cl7_common_atr[1]);
}
void G(0, 7, 6, operate_profile)(void) {
	put_attr((pui08)I(7, 6, profile_generic), 0, (ADDR)cl7_common_atr[2]); \
}
void G(0, 7, 7, operate_profile)(void) {
	ui32 count;
	count = get_entry_in_use(idxobj);
	put_attr((pui08)&count, 0, (ADDR)cl7_common_atr[3]);
}
void G(0, 7, 8, operate_profile)(void) {
	ui32 count;
	count = get_profile_entry(idxobj);
	put_attr((pui08)&count, 0, (ADDR)cl7_common_atr[4]);
}
void M(7, 1, operate_profile)(void) {
	reset(idxobj, rec_reset);
}

/********* relay_log *********/
ui08 optional_relay_log(CADR in) {
	ui08 order = (ui08)*in;
	if ((order == 1) && !(restricting == IST_DATETIME_NOW)) {
		/* invalid target */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	/*	remort_str( ); */
	return TRUE;
}

cl7_function(relay_log, RELAY_LOG)
void G(0, 7, 4, relay_log)(void) {
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]);
}

void M(7, 1, relay_log)(void) {
	reset(IST_RELAY_LOG, rec_reset);
}

/********* billing_params *********/
ui08 optional_billing_params(CADR in) {
	ui08 order = (ui08)*in;
	if ((order == 1) && (restricting != IST_BILLING_DATE)&& (restricting != IST_DATETIME_NOW))
	{
		/* invalid target */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	/*	remort_str( ); */
	return TRUE;
}
void G(0, 7, 2, billing_params)(void) {

	cl7_idx = CL7_BILLING_PARAMS;
	capdef = (PCAPDEF)K(7, 3, billing_params);
	profile = IST_BILLING_PARAMS;
	optional_process = optional_billing_params;
	if (block_no == 0) {
		billsec.wd = cursec.wd;
		record_bld(0);
	}
	init_search(0);
}

void G(1, 7, 2, billing_params)(void) {

	cl7_idx = CL7_BILLING_PARAMS;
	capdef = (PCAPDEF)K(7, 3, billing_params);
	profile = IST_BILLING_PARAMS;
	optional_process = optional_billing_params;
	if (block_no == 0) {
		billsec.wd = cursec.wd;
		record_bld(0);
	}
	init_search(1);
}

void G(2, 7, 2, billing_params)(void) {

	cl7_idx = CL7_BILLING_PARAMS;
	capdef = (PCAPDEF)K(7, 3, billing_params);
	profile = IST_BILLING_PARAMS;
	optional_process = optional_billing_params;
	if (block_no == 0) {
		billsec.wd = cursec.wd;
		record_bld(0);
	}
	init_search(2);
}
void G(0, 7, 3, billing_params)(void) {

	if (block_no == 0) {
		record_offset = 0;
		put_capture_objects(&K(7, 3, billing_params)[0], cl7_entry[CL7_BILLING_PARAMS].l );
	}
	readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset);
	record_offset += put_attr(g_xdlms_io, 0, &billing_params_atr3);
}
void G(0, 7, 4, billing_params)(void) {
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]);
}
void G(0, 7, 5, billing_params)(void) {
	put_attr((pui08)&I(7, 5, profile_generic), 0, (ADDR)cl7_common_atr[1]);
}
void G(0, 7, 6, billing_params)(void) {
	put_attr((pui08)I(7, 6, profile_generic), 0, (ADDR)cl7_common_atr[2]);
}
void G(0,7,7,billing_params)( void ) {
	ui32 count;
	count = get_entry_in_use( IST_BILLING_PARAMS );
	put_attr( (pui08)&count, 0, (ADDR)cl7_common_atr[3] );
}

void G(0,7,8,billing_params)( void ) {
	ui32 count;
	count = get_profile_entry( IST_BILLING_PARAMS );
	put_attr( (pui08)&count, 0, (ADDR)cl7_common_atr[4] );
}

void M(7, 1, billing_params)(void) {
	reset(IST_BILLING_PARAMS, rec_reset);
}

void M(7, 2, billing_params)(void) {

	ui08 ret;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_INTEGER)) {
		return;
	}
	billsec.wd = cursec.wd;
	ret = record_bld(1);
	if (ret > 0) {
		inc_cumcnt(eBill);
		record_log(LOG_BILL_CNT, 0);
		rst_cumcnt(eTampMonth);
	}
	else {
		result_xdlms = RET_OTHER_REASON;
	}
}

/********* load_profile *********/
ui08 optional_load_profile(CADR in)
{
	ui08 order = (ui08)*in;
	if ((order == 1) && !(restricting == IST_DATETIME_NOW))
	{
		/* invalid target */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	/*	remort_str( ); */
	return TRUE;
}

cl7_function(load_profile, LOAD_PROFILE)
void G(0, 7, 4, load_profile)(void) {

	ui32 value;
	value = 0;
	get_value2(IST_LOAD_PERIOD, 0, (pui08)&value);
	put_attr((pui08)&value, 0, (ADDR)cl7_common_atr[0]);
}

void M(7, 1, load_profile)(void) {
	reset(IST_LOAD_PROFILE, rec_reset);
}

/********* daily_profile *********/
ui08 optional_daily_profile(CADR in) {
	ui08 order = (ui08)*in;
	if ((order == 1) && !(restricting == IST_DATETIME_NOW))
	{
		/* invalid target */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	/*	remort_str( ); */
	return TRUE;
}
cl7_function(daily_profile, DAILY_PROFILE)
void G(0, 7, 4, daily_profile)(void) {

	ui32 value;
	value = 0;
	get_value2(IST_DAILY_PERIOD, 0, (pui08)&value);
	put_attr((pui08)&value, 0, (ADDR)cl7_common_atr[0]);
}

void M(7, 1, daily_profile)(void)
{
	reset(IST_DAILY_PROFILE, rec_reset);
}

/********* snap_param1 *********/
ui08 optional_snap_param1(CADR in) {
	ui08 order = (ui08)*in;
	if ((order == 1) && !(restricting == IST_DATETIME_NOW))
	{
		/* invalid target */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return FALSE;
	}
	/*	remort_str( ); */
	return TRUE;
}
void G(0, 7, 2, snap_param1)(void) {

	cl7_idx = CL7_SNAP_PARAM1;
	capdef = (PCAPDEF)K(7, 3, snap_param1);
	profile = IST_SNAP_PARAM1;
	optional_process = optional_snap_param1;
	if ((block_no == 0) && (g_user0 != 4)) {
		record_psh1();
	}
	init_search(0);
}

void G(1, 7, 2, snap_param1)(void) {

	cl7_idx = CL7_SNAP_PARAM1;
	capdef = (PCAPDEF)K(7, 3, snap_param1);
	profile = IST_SNAP_PARAM1;
	optional_process = optional_snap_param1;
	if ((block_no == 0) && (g_user0 != 4)) {
		record_psh1();
	}
	init_search(1);
}

void G(2, 7, 2, snap_param1)(void) {

	cl7_idx = CL7_SNAP_PARAM1;
	capdef = (PCAPDEF)K(7, 3, snap_param1);
	profile = IST_SNAP_PARAM1;
	optional_process = optional_snap_param1;
	if ((block_no == 0) && (g_user0 != 4)) {
		record_psh1();
	}
	init_search(2);
}

void G(0, 7, 3, snap_param1)(void) {

	if (block_no == 0) {
		record_offset = 0;
		put_capture_objects(&K(7, 3, snap_param1)[0], cl7_entry[CL7_SNAP_PARAM1].l);
	}
	readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset);
	record_offset += put_attr(g_xdlms_io, 0, &snap_param1_atr3);
}
void G(0, 7, 4, snap_param1)(void) {
	put_attr((pui08)&I(7, 4, profile_generic), 0, (ADDR)cl7_common_atr[0]);
}
void G(0, 7, 5, snap_param1)(void) {
	put_attr((pui08)&I(7, 5, profile_generic), 0, (ADDR)cl7_common_atr[1]);
}
void G(0, 7, 6, snap_param1)(void) {
	put_attr((pui08)I(7, 6, profile_generic), 0, (ADDR)cl7_common_atr[2]);
}
void G(0, 7, 7, snap_param1)(void) {
	ui32 count;
	count = 1;
	put_attr((pui08)&count, 0, (ADDR)cl7_common_atr[3]);
}

void G(0, 7, 8, snap_param1)(void) {
	ui32 count;
	count = 1;
	put_attr((pui08)&count, 0, (ADDR)cl7_common_atr[4]);
}
void M(7, 1, snap_param1)(void) {
	reset(IST_SNAP_PARAM1, rec_reset);
}

/***************************************************************************/
/***************************************************************************/
/********* scal_unit1 *********/

const  ARG optional_scal_unit1 = echo_function;
cl7_function1(scal_unit1, SCAL_UNIT1)

const  ARG optional_scal_unit2 = echo_function;
cl7_function1(scal_unit2, SCAL_UNIT2)

const  ARG optional_scal_unit3 = echo_function;
cl7_function1(scal_unit3, SCAL_UNIT3)

const  ARG optional_scal_unit5 = echo_function;
cl7_function1(scal_unit5, SCAL_UNIT5)

const  ARG optional_name_plate = echo_function;
cl7_function1(name_plate, NAME_PLATE)

const  ARG optional_scal_unit4 = echo_function;
cl7_function1(scal_unit4, SCAL_UNIT4)

/*********************end of interface class 7 ****************************/


/*******************************************/
/************ interface class 9************/
/*******************************************/
#pragma section COMM
CADR  actions[] = {
	(ADDR)&ID_ENUM,				/* 1 */
	(ADDR)&ID_LONG_UNSIGNED,	/* 1 */
	(ADDR)&ID_OCTET_STRING6,	/* 2 */
	(ADDR)&ID_INTEGER,			/* 1 */
	(ADDR)&ID_OCTET_STRING5,	/* 1 */
};
CONST  IDENT action_specification = { DT_STRUCTURE, 5, (ADDR)actions };
CONST  ACTIONS demand_reset_actions[] = {
	{ IST_BILLING_PARAMS,2,2,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_FWD_PF_MON,2,1,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_BILLING_TIME,2,1,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_MAX_FWD_DEMAND,2,1,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_MAX_BAK_DEMAND,2,1,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_MAX_FWD_VA,2,1,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_MAX_BAK_VA,2,1,0x01,0x0f,0x00,0x00,0x00 },
	{ IST_MINIMUM_VOLT1,2,1,0x01,0x0f,0x00,0x00,0x00 },
};


CONST ACTION_SCRIPTS demand_reset_lists[] = {
	{{0x00,0x01}, sizeof(demand_reset_actions) / sizeof(ACTIONS), demand_reset_actions},	/* global reset */
	{{0x00,0x09}, sizeof(demand_reset_actions) / sizeof(ACTIONS), demand_reset_actions},	/* global reset */
};

CONST  IDENT demand_reset_script = {
	DT_ARRAY, sizeof(demand_reset_actions) / sizeof(ACTIONS), (ADDR)&action_specification 
};

CADR  demand_reset_global_script[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&demand_reset_script
};

CONST  IDENT demand_reset_scripts_define[] = {
	{DT_STRUCTURE, 2, (ADDR)demand_reset_global_script },
};
/*****************************************/
CONST  ACTIONS tariff_actions[] = {
	{ IST_END,2,1,0x01,0x0f,0x00,0x00,0x00 },
};

CONST ACTION_SCRIPTS tariff_lists[] = {
	{{0x01,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x02,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x03,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x04,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x05,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x06,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x07,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
	{{0x08,0x00}, sizeof(tariff_actions) / sizeof(ACTIONS), &tariff_actions[0]},	/* global reset */
};

CONST  IDENT tariff_script = {
	DT_ARRAY, sizeof(tariff_actions) / sizeof(ACTIONS), (ADDR)&action_specification
};

CADR  tariff_global_script[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&tariff_script
};

CONST  IDENT tariff_scripts_define[] = {
	{DT_STRUCTURE, 2, (ADDR)tariff_global_script },
};
CONST ACTIONS image_activation_actions1[] = {
	{ IST_IMG_TRANS,2,4,0x01,0x0f,0x00,0x00,0x00 },
};

CONST ACTION_SCRIPTS image_activation_lists[] = {
	{{0x00,0x01}, sizeof(image_activation_actions1) / sizeof(ACTIONS), (PACTS)image_activation_actions1},	/* global reset */
};
#pragma section

void demand_reset_function(void) {

	ui08 ret;

	ret = record_bld(1);
	if (ret > 0) {
		inc_cumcnt(eBill);
		record_log(LOG_BILL_CNT, 0);
		rst_cumcnt(eTampMonth);
	}
	reset_value(IST_FWD_PF_MON, 0);
	reset_value(IST_BILLING_TIME, 0);
	reset_value(IST_MAX_FWD_DEMAND, 0);
	reset_value(IST_MINIMUM_VOLT0, 0);
}
/*****************************************/
void G(0, 9, 2, demand_reset)(void)
{
	uint	offset = 0, i, j, cnt;
	PADR	padr;
	PIDNT	pidnt;
	PACTS	pacts;

	cnt = sizeof(demand_reset_lists) / sizeof(ACTION_SCRIPTS);
	put_complex_count(cnt, &ID_ARRAY0);

	for (i = 0; i < cnt; i++)
	{
		offset = 0;
		g_xdlms_io[offset] = demand_reset_lists[i].ident[0];
		offset++;
		g_xdlms_io[offset] = demand_reset_lists[i].ident[1];
		offset++;
		padr = &((PADR)demand_reset_scripts_define[i].e)[1];
		pidnt = (PIDNT)*padr;
		for (j = 0; j < pidnt->l; j++)
		{
			pacts = &demand_reset_lists[i].pacts[j];
			g_xdlms_io[offset] = pacts->service_id;
			offset++;
			ramcpy(&g_xdlms_io[offset], g_cls[g_ist[pacts->instance].cls].cid, 2);
			offset += 2;
			ramcpy(&g_xdlms_io[offset], g_ist[pacts->instance].oid, 5);
			offset += 5;
			g_xdlms_io[offset] = 0xff; offset++;
			g_xdlms_io[offset] = pacts->index;
			offset++;
			g_xdlms_io[offset] = pacts->parameter[0];
			offset++;
		}
		put_attr(g_xdlms_io, 0, (ADDR)&demand_reset_scripts_define[i]);
	}
}

static ui08 RunScript(PACT_SCRIPTS scripts, cui16 data, cui08 cnt)
{
	ui08 ret = 0;
	ui16 i, j, ident, inst, exe;
	PACTS	pacts;

	for (i = 0; i < cnt; i++) {
		ident = UI16L(scripts[i].ident);
		if (ident == data) {
			for (j = 0; j < scripts[i].count; j++) {
				pacts = &scripts[i].pacts[j];
				inst = pacts->instance;
				g_xdlms_req.len = 5;
				g_xdlms_req.pos = 0;
				g_xdlms_req.buf[g_xdlms_req.pos + 0] = pacts->parameter[0];//仮
				g_xdlms_req.buf[g_xdlms_req.pos + 1] = pacts->parameter[1];
				g_xdlms_req.buf[g_xdlms_req.pos + 2] = pacts->parameter[2];
				g_xdlms_req.buf[g_xdlms_req.pos + 3] = pacts->parameter[3];
				g_xdlms_req.buf[g_xdlms_req.pos + 4] = pacts->parameter[4];
				if (pacts->service_id == 1) {
					exe = pacts->index;
					Execute(S0, inst, exe - 2);
				}
				else {
					exe = pacts->index + g_cls[g_ist[inst].cls].cnt_a;
					Execute(AT, inst, exe - 1);
				}
			}
			ret = ident;
		}
	}
	return ret;
}

void M(9, 1, demand_reset)(void)
{
	ui08 ret = 0;
	ui16 data;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (DT_LONG_UNSIGNED != g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	data = UI16L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	g_xdlms_req.pos += 2;
	RunScript(&demand_reset_lists[0], data, sizeof(demand_reset_lists) / sizeof(ACTION_SCRIPTS));
	if (data == 1) {
		man_dmd = 1;
		record_log(LOG_RESET, 30);
	}
}


/*****************************************/
void G(0, 9, 2, tariff_scr)(void)
{
	uint	offset = 0, i, j, cnt;
	PADR	padr;
	PIDNT	pidnt;
	PACTS	pacts;

	cnt = sizeof(tariff_lists) / sizeof(ACTION_SCRIPTS);
	put_complex_count(cnt, &ID_ARRAY0);
	for (i = 0; i < cnt; i++) {
		offset = 0;
		g_xdlms_io[offset] = tariff_lists[i].ident[0];
		offset++;
		g_xdlms_io[offset] = tariff_lists[i].ident[1];
		offset++;
		padr = &((PADR)tariff_scripts_define[0].e)[1];
		pidnt = (PIDNT)*padr;
		for (j = 0; j < pidnt->l; j++) {
			pacts = &tariff_lists[i].pacts[j];
			g_xdlms_io[offset] = pacts->service_id;
			offset++;
			romcpy(&g_xdlms_io[offset], &g_cls[g_ist[pacts->instance].cls].cid, 2);
			offset += 2;
			romcpy(&g_xdlms_io[offset], g_ist[pacts->instance].oid, 5);
			offset += 5;
			g_xdlms_io[offset] = 0xff; offset++;
			g_xdlms_io[offset] = pacts->index;
			offset++;
			g_xdlms_io[offset] = pacts->parameter[0];
			offset++;
			g_xdlms_io[offset] = pacts->parameter[1];
			offset++;
			g_xdlms_io[offset] = i + 1;
			offset++;
			g_xdlms_io[offset] = pacts->parameter[3];
			offset++;
			g_xdlms_io[offset] = pacts->parameter[4];
			offset++;
		}
		put_attr(g_xdlms_io, 0, (ADDR)&tariff_scripts_define[0]);
	}
}


void M(9, 1, tariff_scr)(void) {

	uint data;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos])
	{
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (DT_LONG_UNSIGNED != g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	data = UI16L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	g_xdlms_req.pos += 2;

	if (data == 65535) {
		init_tou_scheduler();
		deactivate_tou();
	}
	else {
		tariff_rate((ui08)data);
	}
}


void G(0, 9, 2, img_scr)(void) {

}

void M(9, 1, img_scr)(void) {

	ui08 ret = 0;
	ui16 data;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (DT_LONG_UNSIGNED != g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	data = UI16L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	g_xdlms_req.pos += 2;

	RunScript(&image_activation_lists[0], data, sizeof(image_activation_lists) / sizeof(ACTION_SCRIPTS));
}


void G(0, 9, 2, push_scr)(void) {

}

#pragma section COMM
CONST ACTIONS push_actions1[] = {
	{ IST_PUSHD_HES,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions4[] = {
	{ IST_PUSHD_IHD,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions5[] = {
	{ IST_PUSHE_HES,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions10[] = {
	{ IST_PUSHD_LOD,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions11[] = {
	{ IST_PUSHD_DAY,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions12[] = {
	{ IST_PUSHD_SELFREG,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions13[] = {
	{ IST_PUSHD_BILL,2,1,0x01,0x0f,0x00,0x00,0x00 },
};
CONST ACTIONS push_actions14[] = {
	{ IST_PUSHD_TAMP,2,1,0x01,0x0f,0x00,0x00,0x00 },
};

CONST ACTION_SCRIPTS push_action_lists[] = {
	{{0x00,0x01}, sizeof(push_actions1) / sizeof(ACTIONS), (PACTS)push_actions1},
	{{0x00,0x04}, sizeof(push_actions4) / sizeof(ACTIONS), (PACTS)push_actions4},
	{{0x00,0x05}, sizeof(push_actions5) / sizeof(ACTIONS), (PACTS)push_actions5},
	{{0x00,0x0a}, sizeof(push_actions10) / sizeof(ACTIONS), (PACTS)push_actions10},
	{{0x00,0x0b}, sizeof(push_actions11) / sizeof(ACTIONS), (PACTS)push_actions11},
	{{0x00,0x0c}, sizeof(push_actions12) / sizeof(ACTIONS), (PACTS)push_actions12},
	{{0x00,0x0d}, sizeof(push_actions13) / sizeof(ACTIONS), (PACTS)push_actions13},
	{{0x00,0x0e}, sizeof(push_actions14) / sizeof(ACTIONS), (PACTS)push_actions14},
};
#pragma section

void M(9, 1, push_scr)(void) {

	ui08 ret = 0;
	uint data;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (DT_LONG_UNSIGNED != g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	data = UI16L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	g_xdlms_req.pos += 2;

	RunScript(&push_action_lists[0], data, sizeof(push_action_lists) / sizeof(ACTION_SCRIPTS));
}

#pragma section COMM
static cui08f CALL_ACT[] = {
	0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x10,0xc3,0x01,0x41,0x00,0x09,0x00,0x00,0x00,0x09,0x02,0xff,0x01,0x01,0x12,0x00,0x01 };

static cui08f CALL_COMINF[] = {
	0x24,0x24,0x02,0x00,0x00 };
#pragma section


void createCall(ui08* out) {

	DAT2 tmp;
	CALLING* p = &gCalling[gCall];

	if (p->inst != 0) {
		romcpy(out, CALL_ACT, sizeof(CALL_ACT));
		out[3] = USERS[p->userid].sys.maddr;	/*Client addr*/
		out[3] >>= 1;
		out[8] = p->order;	/*get/set/act*/
		if (p->inst != 0xffff) {
			romcpy(&out[13], g_ist[p->inst].oid, 5);	/*OBIS*/
		}
		else {
			ramset(&out[13], 0xff, 5);	/*OBIS*/
		}
		out[18] = 0xff;
		if (p->inst == IST_PUSH_SCR) {
			switch (p->arg) {
			case 5:
				out[1] = 0x02;	/*univesal*/
				break;
			case 1:
				out[1] = 0x03;	/*univesal*/
				break;
			case 10:
				out[1] = 0x04;	/*univesal*/
				break;
			case 11:
				out[1] = 0x05;	/*univesal*/
				break;
			case 12:
				out[1] = 0x06;	/*univesal*/
				break;
			case 13:
				out[1] = 0x07;	/*univesal*/
				break;
			case 14:
				out[1] = 0x08;	/*univesal*/
				break;
			}
		}
		tmp.hf = p->arg;
		out[22] = tmp.by[1];	/*idntity*/
		out[23] = tmp.by[0];
	}
	else {
		romcpy(out, CALL_COMINF, sizeof(CALL_COMINF));
		out[3] = p->order;
		out[4] = p->arg;
	}
}

#pragma section COMM
cui08f PUSH_CLIENT1[] = { 0x50,0x75,0x73,0x68,0x20,0x20,0x20,0x20 };
cui08f PUSH_CLIENT2[] = { 0x50,0x75,0x73,0x68,0x20,0x20,0x20,0x20 };
cui08f PUSH_CLIENT3[] = { 0x50,0x75,0x73,0x68,0x20,0x20,0x20,0x20 };
#pragma section

void script_associate(void) {

	ui08 rank;

	CALLING *p = &gCalling[gCall];

	g_user0 = p->userid;
	g_userQ = p->userid;
	setCounter(g_user0);
	rank = Rank(p->userid);	/* Associationクライアント情報のコピー */
	g_AA = TRUE;
	I(15, 8, asso_ln) = ASSOCIATED;	/* 認証確立 */
	I(15, 8, asso_ln0) = &I(15, 8, asso_ln);			/* association status */
	if (COSEM_PUBLIC == rank) {
		g_authQ = FALSE;
		as_ln0 = (PADR)&as_ln1[0];
	}
	if (COSEM_READER == rank) {
		g_authQ = TRUE;
		as_ln0 = (PADR)&as_ln2[0];
		install_global_key();
	}
	if (COSEM_ADMIN == rank) {
		g_authQ = TRUE;
		as_ln0 = (PADR)&as_ln3[0];
		set_BtoA(PUSH_CLIENT2, 8);
		fixed_secret();
		install_global_key();
	}
	g_addrQ  = USERS[g_userQ].sys.maddr;
	security_control = USERS[g_userQ].sys.sc;
	I(15, 3, asso_ln)[0] = g_addrQ >> 1;
	I(15, 3, asso_ln)[1] = 0x01;
	I(15, 3, asso_ln)[2] = 0x00;
	I(15, 3, asso_ln0) = (pui08)I(15, 3, asso_ln);		/* associated_partners_id */
}

/*********************end of interface class 9 ****************************/

/*******************************************/
/************ interface class 11************/
/*******************************************/
CADR  spec_day_entry[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING5,
	(ADDR)&ID_UNSIGNED
};

CONST IDENT spec_day_st = { DT_STRUCTURE, 3, (ADDR)spec_day_entry };
CONST IDENT spec_days_ar = { DT_ARRAY, 100, (ADDR)&spec_day_st };

void G(0, 11, 2, special_days)(void) {

	getset = get_value2;
	put_value_with_block(IST_SPECIAL_DAYS, &spec_days_ar, &spec_day_st);
}

void S(0, 11, 2, special_days)(void) {

	getset = set_value2;
	take_value_with_block(IST_SPECIAL_DAYS, &spec_days_ar, &spec_day_st);
}

/*********************end of interface class 11 ****************************/

/*******************************************/
/************ interface class 15************/
/*******************************************/
CADR  associated_partners_type[] = {
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_LONG_UNSIGNED,
};

/*
	conformance: bitstring(24),
	max_receive_pdu_size: long-unsigned,
	max_send_pdu_size: long-unsigned,
	dlms_version_number: unsigned,
	quality_of_service: integer,
	cyphering_info: octet-string
 */
CADR  xDLMS_context_type0[] = {
	(ADDR)&ID_BIT_STRING24,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_OCTET_STRING0,
};

CADR  xDLMS_context_typex[] = {
	(ADDR)&ID_BIT_STRING24,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_OCTET_STRING16,
};
CONST IDENT associationln_atr3 = { DT_STRUCTURE, 2, (ADDR)&associated_partners_type[0] };
CONST IDENT associationln_atr5_0 = { DT_STRUCTURE, 6, (ADDR)&xDLMS_context_type0[0] };
CONST IDENT associationln_atr5_x = { DT_STRUCTURE, 6, (ADDR)&xDLMS_context_typex[0] };

CADR  associationln1[] = {
	(ADDR)&ID_NULL_DATA,			/* 2 */
	(ADDR)&associationln_atr3,		/* 3 */
	(ADDR)&ID_OCTET_STRING7,		/* 4 */
	(ADDR)&associationln_atr5_0,	/* 5 */
	(ADDR)&ID_OCTET_STRING7,		/* 6 */
	(ADDR)&ID_OCTET_STRING0,		/* 7 */
	(ADDR)&ID_ENUM,					/* 8 */
	(ADDR)&ID_OCTET_STRING6,		/* 9 */
};

CADR  associationlnx[] = {
	(ADDR)&ID_NULL_DATA,			/* 2 */
	(ADDR)&associationln_atr3,		/* 3 */
	(ADDR)&ID_OCTET_STRING7,		/* 4 */
	(ADDR)&associationln_atr5_x,	/* 5 */
	(ADDR)&ID_OCTET_STRING7,		/* 6 */
	(ADDR)&ID_OCTET_STRING0,		/* 7 */
	(ADDR)&ID_ENUM,					/* 8 */
	(ADDR)&ID_OCTET_STRING6,		/* 9 */
};

ui08 ZERO16(cui16 ctl)
{
	ui08 ret;

	if (CNT16&ctl)
	{
		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

ui08 AUTH16(cui16 ctl)
{
	ui08 ret;

	if (ATH16&ctl)
	{
		ret = !((g_authQ & 0x0f) && g_AA);
	}
	else
	{
		ret = !(((g_authQ & 0x0f) + g_AA) >= 1);
		//		ret = !(((g_authQ & 0x0f) + g_AA) == 1);
	}
	return ret;
}

ui08 F16(cui16 ctl, cui16 sel)
{
	ui16 and, val;
	ui08 ret;

	and = 0x0007;
	val = sel;
	for (ret = 4; ret > 0;)
	{
		if ((ctl&and) == val)
		{
			break;
		}
		and <<= 3;
		val <<= 3;
		ret--;
	}
	return ret;
}

void get_object_list(CADR *as_lnx)
{
	uint	i, j, tmp16, cls, fixedlen, cur_object;
	ui08	idx[4], cnt_a, cnt_m, tmp08, obis[6], rank;
	ACC1	acc1;
	ACC4	acc4i;
	POCTL	octl;
	POBJ16	pobj;
	PACC	acc0 = &USERS[0].acc;
	PACC	acc = &USERS[g_user0].acc;

	rank = Rank(g_user0);
	if (gMtrCfg.userflag.bt.power_user > 0) {
		if (rank > COSEM_POWER) {
			rank = 1;
		}
		else {
			rank = 0;
		}
	}
	else {
		if (rank > COSEM_ADMIN) {
			rank = 1;
		}
		else {
			rank = 0;
		}
	}
	if (0 == block_no) {
		object_list_pos = 0;
		j = 0;
		for (i = 0; i < IST_END; i++) {
			if (!((g_ist[i].cls == CL_SKIP) || (NULL == as_lnx[i]))) {
				octl = (POCTL)&OBJMAP[i];
				tmp08 = octl->bt.idx;
				if (octl->bt.four > 0) {
					acc4i.wd = acc->acc4[tmp08];
					acc4i.wd &= acc0->acc4[tmp08];
					if ((acc4i.hf[1] & 0xe000) > 0) {
						j++;
					}
				}
				else {
					acc1.by = acc->acc1[tmp08];
					acc1.by &= acc0->acc1[tmp08];
					if ((acc1.by & 0xe0) > 0) {
						j++;
					}
				}
			}
		}
		put_complex_count_non(j, &ID_ARRAY0);
	}
	cur_object = 0xffff;
	obis[5] = 0xff;
	for (; object_list_pos < IST_END; ) {
		cls = g_ist[object_list_pos].cls;
		if (!((cls == CL_SKIP) || (NULL == as_lnx[object_list_pos]))) {
			if (object_list_pos == IST_ASSO_LN0) {
				cur_object = object_list_pos;
				switch (g_user0) {
				case 1:	/*US*/
					object_list_pos = IST_ASSO_LN3;
					break;
				case 2:	/*FW*/
					object_list_pos = IST_ASSO_LN5;
					break;
				case 3:	/*MR*/
					object_list_pos = IST_ASSO_LN2;
					break;
				case 4:	/*PUSH*/
					object_list_pos = IST_ASSO_LN4;
					break;
				default:
				case 5:
				case 6:
				case 7:	/*PC*/
					object_list_pos = IST_ASSO_LN1;
					break;
				}
			}
			j = 0;
			octl = (POCTL)&OBJMAP[object_list_pos];
			tmp08 = octl->bt.idx;
			if (octl->bt.four > 0) {
				acc4i.wd = acc->acc4[tmp08];
				acc4i.wd &= acc0->acc4[tmp08];
				if ((acc4i.by[3] & 0xe0) > 0) {
					j = acc4i.by[3];
				}
			}
			else {
				acc1.by = acc->acc1[tmp08];
				acc1.by &= acc0->acc1[tmp08];
				if ((acc1.by & 0xe0) > 0) {
					j = acc1.by;
				}
			}
			if (j > 0) {
				cnt_a = g_cls[g_ist[object_list_pos].cls].cnt_a;
				if (rank > 0) {
					cnt_m = 0;
				}
				else {
					cnt_m = g_cls[g_ist[object_list_pos].cls].cnt_m;
				}
				fixedlen = g_raw_len;
				put_complex_count_non(4, &ID_STRUCTURE0);				/*object list element*//*2*/
				tmp16 = g_cls[cls].cid;
				put_attr_non((pui08)&tmp16, 0, (ADDR)&ID_LONG_UNSIGNED);/*1*//*3*/

				tmp08 = g_cls[cls].ver;
				put_attr_non((pui08)&tmp08, 0, (ADDR)&ID_UNSIGNED);		/*2*//*2*/
				if (cur_object != 0xffff) {
					romcpy(obis, g_ist[cur_object].oid, 5);
				}
				else{
					romcpy(obis, g_ist[object_list_pos].oid, 5);
				}
				put_attr_non(obis, 0, (ADDR)&ID_OCTET_STRING6);			/*3*//*8*/

				put_complex_count_non(2, &ID_STRUCTURE0);				/*4*//*access_right*//*2*/
				put_complex_count_non(cnt_a + 1, &ID_ARRAY0);			/*attribute access*//*2*/

				put_complex_count_non(3, &ID_STRUCTURE0);				/*attribute access descriptor*//*2*/
				tmp08 = 1;
				put_attr_non(&tmp08, 0, (ADDR)&ID_INTEGER);				/*attribute 1*//*2*/
				tmp08 = 1;
				put_attr_non(&tmp08, 0, (ADDR)&ID_ENUM);					/*read-access*//*2*/
				put_attr_non(NULL, 0, (ADDR)&ID_NULL_DATA);				/*access selectors*//*1*/

				for (i = 0; i < cnt_a; i++) {							/*attribute x*/
					put_complex_count_non(3, &ID_STRUCTURE0);			/*attribute access descriptor*//*2*/
					tmp08 = i + 2;
					put_attr_non(&tmp08, 0, (ADDR)&ID_INTEGER);			/*attribute x*//*2*/
					pobj = (POBJ16)(as_lnx[object_list_pos] + POS_OBJ[i]);
					if (0 == ZERO16(pobj->ctl)) {
						tmp08 = 0;		/* No Access */
						put_attr_non(&tmp08, 0, (ADDR)&ID_ENUM);		/*2*/
						put_attr_non(NULL, 0, (ADDR)&ID_NULL_DATA);		/*1*/
					}
					else {
						tmp08 = 0;
						if (j & 0x80) {
							tmp08 += ((F16(pobj->ctl, G0)) ? G0 : 0);
						}
						if (j & 0x40) {
							tmp08 += ((F16(pobj->ctl, S0)) ? S0 : 0);
						}
						tmp08 += ((pobj->ctl&ATH16) ? 0x00 : 0);

						put_attr_non(&tmp08, 0, (ADDR)&ID_ENUM);	/*2*/
						tmp08 = 0;
						if (F16(pobj->ctl, G1) || F16(pobj->ctl, S1)) {
							idx[tmp08] = 1; tmp08++;
						}
						if (F16(pobj->ctl, G2) || F16(pobj->ctl, S2)) {
							idx[tmp08] = 2; tmp08++;
						}
						if (F16(pobj->ctl, G3)) {		/* 16ビットは、5メンバのみ */
							idx[tmp08] = 3; tmp08++;
						}
						if (tmp08 > 0) {
							put_complex_count_non(tmp08, &ID_ARRAY0);/*2*/
							for (tmp16 = 0; tmp16 < tmp08; tmp16++) {
								put_attr_non(&idx[tmp16], 0, (ADDR)&ID_INTEGER);/*2*/
							}
						}
						else {
							put_attr_non(NULL, 0, (ADDR)&ID_NULL_DATA);/*2*/
						}
					}
				}
				put_complex_count_non(cnt_m, &ID_ARRAY0);		/*2*/
				for (i = 0; i < cnt_m; i++) {
					put_complex_count_non(2, &ID_STRUCTURE0);	/*2*/
					tmp08 = i + 1;
					put_attr_non(&tmp08, 0, (ADDR)&ID_INTEGER);	/*2*/
					tmp08 = 0;
					pobj = (POBJ16)(as_lnx[object_list_pos] + POS_OBJ[i + cnt_a]);
					if (0 == ZERO16(pobj->ctl)) {
						tmp08 = 0;
					}
					else {
						if ((j & 0x20)>0) {
							tmp08 += ((F16(pobj->ctl, AT)) ? AT : 0);
						}
						tmp08 += ((pobj->ctl&ATH16) ? 0x00 : 0);
					}
					put_attr_non(&tmp08, 0, (ADDR)&ID_ENUM);	/*2*/
				}
				if (axdr_max == 0) {
					g_raw_len = fixedlen;
					if (cur_object != 0xffff) {
						object_list_pos = cur_object;
						cur_object = 0xffff;
					}
					break;
				}
			}
			if (cur_object != 0xffff) {
				object_list_pos = cur_object;
				cur_object = 0xffff;
			}
		}
		object_list_pos++;
	}
	if (!(IST_END == object_list_pos)) {
		if (0 == block_no) {		/* 最初の応答 */
			last_block = FALSE;
			res_mode = ORDER_BLOCK;
		}
	}
	else {
		last_block = TRUE;
	}
}

/********* asso_ln1 *********/
ccpu08  I(15, 4, asso_ln1) = application_name[1];	/* application_context_name */
cui16 I(15, 9, asso_ln1) = IST_SECURITY_NONE;		/* security_setup_reference */
/********* asso_ln2 *********/
ccpu08  I(15, 4, asso_ln2) = application_name[3];	/* application_context_name */
cui16 I(15, 9, asso_ln2) = IST_SECURITY_MR;			/* security_setup_reference */
/********* asso_ln3 *********/
ccpu08  I(15, 4, asso_ln3) = application_name[3];	/* application_context_name */
cui16 I(15, 9, asso_ln3) = IST_SECURITY_US;			/* security_setup_reference */
/********* asso_ln4 *********/
ccpu08  I(15, 4, asso_ln4) = application_name[3];	/* application_context_name */
cui16 I(15, 9, asso_ln4) = IST_SECURITY_PUSH;		/* security_setup_reference */
/********* asso_ln5 *********/
ccpu08  I(15, 4, asso_ln5) = application_name[3];	/* application_context_name */
cui16 I(15, 9, asso_ln5) = IST_SECURITY_FIRM;		/* security_setup_reference */
/********* asso_ln6 *********/
ccpu08  I(15, 4, asso_ln6) = application_name[3];	/* application_context_name */
cui16 I(15, 9, asso_ln6) = IST_SECURITY_IHD;		/* security_setup_reference */

ui08 exist_aa(void)
{
	if (!(NULL == as_ln0)) {
		return *I(15, 8, asso_ln0);
	}
	return NON_ASSOCIATED;
}

ui08 get_addr(void)/* associated_partners_id */
{
	ui08 res = DLMS_INVALID;

	if (!(NULL == I(15, 3, asso_ln0))) {
		res = ((I(15, 3, asso_ln0)[0] << 1) | 0x01);
	}
	return res;
}

void G(0, 15, 3, asso_ln0)(void)/* associated_partners_id */
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln03();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void G(0, 15, 4, asso_ln0)(void)	/* application_context_name */
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln04();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void G(0, 15, 5, asso_ln0)(void)		/* xDLMS_context_info */
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln05();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void G(0, 15, 6, asso_ln0)(void)		/* authentication_mechanism_name */
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln06();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void S(0, 15, 7, asso_ln0)(void)
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln07();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void G(0, 15, 8, asso_ln0)(void)		/* association status */
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln08();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void G(0, 15, 9, asso_ln0)(void)		/* security_setup_reference */
{
	if (ASSOCIATED == exist_aa())
	{
		asso_ln09();
	}
	else
	{
		/* Error : Not associated */
		record_log(ETSN2, 7);
		result_xdlms = RET_OTHER_REASON;
		return;
	}
}

void G(0, 15, 2, asso_ln0)(void)		/*Current association*/
{
	get_object_list(&as_ln0[0]);
}
void G(0, 15, 2, asso_ln1)(void)		/*PC*/
{
	get_object_list(&as_ln1[0]);
}
void G(0, 15, 2, asso_ln2)(void)		/*MR*/
{
	get_object_list(&as_ln2[0]);
}
void G(0, 15, 2, asso_ln3)(void)		/*US*/
{
	get_object_list(&as_ln3[0]);
}
void G(0, 15, 2, asso_ln4)(void)		/*Push*/
{
	get_object_list(&as_ln2[0]);
}
void G(0, 15, 2, asso_ln5)(void)		/*FW*/
{
	get_object_list(&as_ln3[0]);
}
void G(0, 15, 2, asso_ln6)(void)		/*IHD*/
{
	get_object_list(&as_ln2[0]);
}

static void get_partnerd_id(cui08 sel) {

	ui08 def[3];

	def[1] = 0x01;
	def[2] = 0;
	switch (sel) {
	case 0x07:
		def[0] = 0x10;
		break;
	case 0x03:
		def[0] = 0x20;
		break;
	case 0x00:
	case 0x01:
		def[0] = 0x30;
		break;
	case 0x04:
		def[0] = 0x40;
		break;
	case 0x02:
		def[0] = 0x50;
		break;
	case 0x05:
		def[0] = 0x60;
		break;
	case 0x06:
		def[0] = 0x10;
		break;
	}
	put_attr((pui08)def, 0, (ADDR)&associationln_atr3);
}

void G(0, 15, 3, asso_ln1)(void) {
	get_partnerd_id(7);
}
void G(0, 15, 3, asso_ln2)(void) {
	get_partnerd_id(3);
}
void G(0, 15, 3, asso_ln3)(void) {
	get_partnerd_id(1);
}
void G(0, 15, 3, asso_ln4)(void) {
	get_partnerd_id(4);
}
void G(0, 15, 3, asso_ln5)(void) {
	get_partnerd_id(2);
}
void G(0, 15, 3, asso_ln6)(void) {
	get_partnerd_id(5);
}

void G(0, 15, 4, asso_ln1)(void) {
	put_attr((pui08)application_name[1], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 4, asso_ln2)(void) {
	put_attr((pui08)application_name[3], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 4, asso_ln3)(void) {
	put_attr((pui08)application_name[3], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 4, asso_ln4)(void) {
	put_attr((pui08)application_name[3], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 4, asso_ln5)(void) {
	put_attr((pui08)application_name[3], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 4, asso_ln6)(void) {
	put_attr((pui08)application_name[3], 0, (ADDR)&ID_OCTET_STRING7);
}

void G(0, 15, 6, asso_ln1)(void)
{
	put_attr((pui08)mechanism_name[0], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 6, asso_ln2)(void)
{
	put_attr((pui08)mechanism_name[1], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 6, asso_ln3)(void)
{
	put_attr((pui08)mechanism_name[2], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 6, asso_ln4)(void)
{
	put_attr((pui08)mechanism_name[1], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 6, asso_ln5)(void)
{
	put_attr((pui08)mechanism_name[2], 0, (ADDR)&ID_OCTET_STRING7);
}
void G(0, 15, 6, asso_ln6)(void)
{
	put_attr((pui08)mechanism_name[1], 0, (ADDR)&ID_OCTET_STRING7);
}

void G(0, 15, 5, asso_ln1)(void) {

	pui08 ctx;
	if (7 != g_user0) {
		ctx = CTX[1];
	}
	else {
		ctx = I(15, 5, asso_ln);
	}
	put_attr((pui08)ctx, 0, (ADDR)&associationln_atr5_0);
}

void G(0, 15, 5, asso_ln2)(void) {
	
	pui08 ctx;
	if (3 != g_user0) {
		ctx = CTX[2];
	}
	else {
		ctx = I(15, 5, asso_ln);
	}
	put_attr((pui08)ctx, 0, (ADDR)&associationln_atr5_0);
}

void G(0, 15, 5, asso_ln3)(void) {

	pui08 ctx;
	if (1 != g_user0) {
		ctx = CTX[3];
	}
	else {
		ctx = I(15, 5, asso_ln);
	}
	put_attr((pui08)ctx, 0, (ADDR)&associationln_atr5_0);
}

void G(0, 15, 5, asso_ln4)(void) {

	pui08 ctx;
	if (4 != g_user0) {
		ctx = CTX[4];
	}
	else {
		ctx = I(15, 5, asso_ln);
	}
	put_attr((pui08)ctx, 0, (ADDR)&associationln_atr5_0);
}

void G(0, 15, 5, asso_ln5)(void) {

	pui08 ctx;
	if (2 != g_user0) {
		ctx = CTX[5];
	}
	else {
		ctx = I(15, 5, asso_ln);
	}
	put_attr((pui08)ctx, 0, (ADDR)&associationln_atr5_0);
}

void G(0, 15, 5, asso_ln6)(void) {

	pui08 ctx;
	if (5 != g_user0) {
		ctx = CTX[4];
	}
	else {
		ctx = I(15, 5, asso_ln);
	}
	put_attr((pui08)ctx, 0, (ADDR)&associationln_atr5_0);
}

static void get_association_status(cui08 sel) {

	ui08 def[1] = { 0x00 };
	if (sel == g_user0) {
		put_attr((pui08)&I(15, 8, asso_ln), 0, (ADDR)&ID_ENUM);
	}
	else {
		put_attr((pui08)def, 0, (ADDR)&ID_ENUM);
	}
}
void G(0, 15, 8, asso_ln1)(void) {
	get_association_status(7);
}
void G(0, 15, 8, asso_ln2)(void) {
	get_association_status(3);
}
void G(0, 15, 8, asso_ln3)(void) {
	get_association_status(1);
}
void G(0, 15, 8, asso_ln4)(void) {
	get_association_status(4);
}
void G(0, 15, 8, asso_ln5)(void) {
	get_association_status(2);
}
void G(0, 15, 8, asso_ln6)(void) {
	get_association_status(5);
}

#define cl15_function( ist , prm)\
void G(0,15,7,ist)( void )\
{\
}\
void G(0,15,9,ist)( void )\
{\
	ui08 tmp[6];\
	romcpy(tmp, g_ist[ I(15,9,ist) ].oid, 5);\
	tmp[5] = 0xff;\
	put_attr( tmp, 0, (ADDR)prm[7] );\
}

/********* asso_ln1 *********/
cl15_function(asso_ln1, associationln1)
/********* asso_ln2 *********/
cl15_function(asso_ln2, associationln1)
/********* asso_ln3 *********/
cl15_function(asso_ln3, associationlnx)
/********* asso_ln4 *********/
cl15_function(asso_ln4, associationln1)
/********* asso_ln5 *********/
cl15_function(asso_ln5, associationlnx)
/********* asso_ln6 *********/
cl15_function(asso_ln6, associationln1)
/********* end of asso_ln3 *********/
static void set_secret(cui08 userid) {

	uint len, len0, index;

	len0 = g_xdlms_req.len - g_xdlms_req.pos;
	if (g_xdlms_req.buf[g_xdlms_req.pos] != DT_OCTET_STRING) {	/*type*/
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	len0--;
	g_xdlms_req.pos++;
	len0--;
	len = g_xdlms_req.buf[g_xdlms_req.pos];		/*size*/
	if (len > 16 || len < 8) {
		result_xdlms = RET_TEMPORARY_FAILURE;
		return;
	}
	if (len0 != len) {
		result_xdlms = RET_TEMPORARY_FAILURE;
		return;
	}
	index = 8 * 3 + userid;	/*auth value*/
	start_change_key(&result_xdlms, index, &g_xdlms_req.buf[g_xdlms_req.pos]);
}

void S(0, 15, 7, asso_ln2)(void) {
	set_secret(3);
}

void S(0, 15, 7, asso_ln4)(void) {
	set_secret(4);
}

void S(0, 15, 7, asso_ln6)(void) {
	set_secret(5);
}
/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
/* http://www.dlms.com/faqanswers/forum/index.php?action=view&topicid=185 */
#define OFFSET_GMAC_SC 0x00
#define OFFSET_GMAC_FC (OFFSET_GMAC_SC+LEN_SC)
#define OFFSET_GMAC_TG (OFFSET_GMAC_FC+LEN_FRAME_COUNTER)

ui08 reply_to_HLS_authentication(void)
{
	ui08 iv[12];
	ui08 secret[16];
	ui08 res, rank;

	res = FALSE;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		/* No MAC */
		result_xdlms = RET_TYPE_UNMATCHED;
		return res;
	}
	g_xdlms_req.pos++;

	if (I(15, 6, asso_ln0)[6] == 0x02) {
		if (FALSE == take_attr((ADDR)&ID_OCTET_STRING16)) {
			return res;
		}
		if (DLMS_SUCCESS != hls_secret(secret, OFFSET_ATOB)) {
			return res;
		}
		if (!(0 == ramcmp(secret, g_xdlms_io, 16))) {
			/* Invalid f(StoC) */
			result_xdlms = RET_TEMPORARY_FAILURE;
			if (g_portQ == 2) {
				comsts.status.wd |= 0x00000800;
			}
			return res;
		}
		else{
			if (g_portQ == 2) {
				comsts.status.wd &= ~0x00000800;
			}
		}
		hls_secret(g_xdlms_io, OFFSET_BTOA);
		g_raw_len = 0;
		g_raw[g_raw_len] = 0x01;
		g_raw_len++;/* BOOL */
		g_raw[g_raw_len] = 0x00;
		g_raw_len++;/* [0] DATA */
		put_attr(g_xdlms_io, 0, (ADDR)&ID_OCTET_STRING16);
	}
	else {
		if (FALSE == take_attr((ADDR)&ID_GMAC)) {
			return res;
		}
		/* [SC][AK][ServerChallenge] */
		if (!(0x10 == g_xdlms_io[OFFSET_GMAC_SC])) {
			result_xdlms = RET_TYPE_UNMATCHED;
			return res;
		}
		/* set IV */
		ramcpy(&iv[0], get_system_title0(), LEN_SYSTEM_TITLE);
		ramcpy(&iv[8], &g_xdlms_io[OFFSET_GMAC_FC], LEN_FRAME_COUNTER);		/* frame count */
		set_ive(iv, LEN_INITIAL_VECTOR);
		/* set AAD */
		set_aad_gmac_a();
		/* Cipher */
		AES_GCM_arr(secret, 0, LEN_AUTH_TAG);
		if (!(0 == ramcmp(secret, &g_xdlms_io[OFFSET_GMAC_TG], LEN_AUTH_TAG))) {
			/* Invalid f(StoC) */
			result_xdlms = RET_TEMPORARY_FAILURE;
			if (g_portQ == 2) {
				comsts.status.wd |= 0x00000800;
			}
#if (SW_AES)
			set_aad_key_e();
#endif
			return res;
		}
		else {
			if (g_portQ == 2) {
				comsts.status.wd &= ~0x00000800;
			}
		}
		/* set IV */
		ramcpy(iv, get_system_title(), LEN_SYSTEM_TITLE);
		Counter(&iv[8]);
		set_ive(iv, LEN_INITIAL_VECTOR);
		/* SC */
		g_xdlms_io[OFFSET_GMAC_SC] = 0x10;
		/* FC */
		ramcpy(&g_xdlms_io[OFFSET_GMAC_FC], &iv[8], LEN_FRAME_COUNTER);
		/* set AAD */
		set_aad_gmac_b();
		/* Cipher */
		AES_GCM_arr(&g_xdlms_io[OFFSET_GMAC_TG], 0, LEN_AUTH_TAG);
		g_raw_len = 0;
		g_raw[g_raw_len] = 0x01;
		g_raw_len++;/* BOOL */
		g_raw[g_raw_len] = 0x00;
		g_raw_len++;/* [0] DATA */
		put_attr(g_xdlms_io, 0, (ADDR)&ID_GMAC);
#if (SW_AES)
		set_aad_key_e();
#endif
	}
	rank = Rank(g_userQ);
	if (RET_SUCCESS == result_xdlms) {
		res = TRUE;
	}
	return res; /* 接続確立状態 */
}

void M(15, 1, asso_ln0)(void)
{
	if (g_AA) {
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
	g_AA = reply_to_HLS_authentication();
}
#if 0	/* KALKI */
void M(15, 1, asso_ln1)(void)
{
	if (g_AA)
	{
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
	g_AA = reply_to_HLS_authentication();
}
#endif	/* KALKI */

void M(15, 1, asso_ln3)(void)
{
	if (g_AA) {
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
	g_AA = reply_to_HLS_authentication();
}

void M(15, 1, asso_ln5)(void)
{
	if (g_AA) {
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
	g_AA = reply_to_HLS_authentication();
}

void M(15, 2, asso_ln0)(void)
{
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	set_secret(g_user0);
	record_log(LOG_HLS_CHG, g_user0);
}

void M(15, 2, asso_ln3)(void)
{
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	set_secret(1);
	record_log(LOG_HLS_CHG, 1);
}

void M(15, 2, asso_ln5)(void)
{
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	set_secret(2);
	record_log(LOG_HLS_CHG, 2);
}

/*********************end of interface class 15 ****************************/

/*******************************************/
/*********** interface class 18 ************/
/*******************************************/
#pragma section COMM
CADR _image_initiate[] = {
	(ADDR)&ID_OCTET_STRING8,
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED
};
CONST IDENT img_initiate = { DT_STRUCTURE, 2, (ADDR)_image_initiate };

CADR  _image_block_transfer[] = {
	(ADDR)&ID_DOUBLE_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING512
}; 
CONST IDENT image_block_transfer = { DT_STRUCTURE, 2, (ADDR)_image_block_transfer };
#pragma section

void G(0, 18, 2, img_trans)(void) {	/*image_block_size*/

	put_value( 2, IST_IMG_TRANS, &ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 18, 3, img_trans)(void) {	/*image_transferred_blocks_status*/

	put_value( 3, IST_IMG_TRANS, &ID_BIT_STRING512);
}

void G(0, 18, 4, img_trans)(void) {	/*image_first_not_transferred_block_number*/

	put_value( 4, IST_IMG_TRANS, &ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 18, 5, img_trans)(void) {	/*image_transfer_enabled*/

	put_value( 5, IST_IMG_TRANS, &ID_BOOLEAN);
}
void S(0, 18, 5, img_trans)(void) {	/*image_transfer_enabled*/

	take_value5(IST_IMG_TRANS, (CADR)&ID_BOOLEAN);
}

void G(0, 18, 6, img_trans)(void) {	/*image_transfer_status*/

	put_value( 6, IST_IMG_TRANS, &ID_ENUM);
}

void M(18, 1, img_trans)(void) {
	/*	image_transfer_initiate(data)*/
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&img_initiate)) {
		return;
	}

	/*
	image_idenntifier	octet-string
	image_size			double long unsigned
	*/
	if (FALSE == init_image(g_xdlms_io)) {
		result_xdlms = RET_TEMPORARY_FAILURE;
	}
}

void M(18, 2, img_trans)(void) {
	/*	image_block_transfer(data)*/

	ui32 blockno, len;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	if (FALSE == take_attr((ADDR)&image_block_transfer)) {
		return;
	}
	blockno = UI32B(g_xdlms_io);
	if (SUCCESS != backup_image(&g_xdlms_io[4], blockno)) {
		result_xdlms = RET_TEMPORARY_FAILURE;
	}
}

void M(18, 3, img_trans)(void) {
	/*	image_verify(data)*/
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	if (FALSE == take_attr((ADDR)&ID_INTEGER)) {
		return;
	}
	if (gImage.status != verification_successful) {
		result_xdlms = RET_OTHER_REASON;
	}
}

void M(18, 4, img_trans)(void) {
	/*	image_activate(data)*/
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos])
	{
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;
	if (FALSE == take_attr((ADDR)&ID_INTEGER)) {
		return;
	}
	if (get_vbat() > 0) {
		if (gImage.status == verification_successful) {
//			P_COMMON2 = 0;//@
			gImage.status = activation_initiated;
			record_log(LOG_FLASH, 1);
			sleep_request();
			if (gImage.ctl0.bt.skip_backup == 0) {
				cmd_backup_current_image[0]();
			}
			cmd_call_image[0]();
		}
		else {
			result_xdlms = RET_OTHER_REASON;
		}
	}
	else {
		result_xdlms = RET_HARDWARE_FAULT;
	}
}

void G(0, 18, 2, img_trans1)(void) {
	put_value( 2, IST_IMG_TRANS1, &ID_DOUBLE_LONG_UNSIGNED);
}

void M(18, 1, img_trans1)(void) {
	/*	image_transfer_initiate(data)*/
}

void M(18, 2, img_trans1)(void) {
	/*	image_block_transfer(data)*/
	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos])
	{
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_INTEGER))
	{
		return;
	}
	if (get_vbat() > 0) {
		sleep_request();
		cmd_call_image[0]();
	}
	else {
		result_xdlms = RET_OTHER_REASON;
	}
}
void M(18, 3, img_trans1)(void) {
	/*	image_verify(data)*/
}
void M(18, 4, img_trans1)(void) {
	/*	image_activate(data)*/
}

/*******************************************/
/*********** interface class 20 ************/
/*******************************************/
CADR  season_entry[] = {
	(ADDR)&ID_OCTET_STRING16,
	(ADDR)&ID_OCTET_STRING12,
	(ADDR)&ID_OCTET_STRING6
};
CONST IDENT season_st = { DT_STRUCTURE, 3, (ADDR)season_entry };
CONST IDENT season_ar = { DT_ARRAY, 4, (ADDR)&season_st };

CADR  week_entry[] = {
	(ADDR)&ID_OCTET_STRING6,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED
};
CONST IDENT week_st = { DT_STRUCTURE, 8, (ADDR)week_entry };
CONST IDENT week_ar = { DT_ARRAY, 4, (ADDR)&week_st };

CADR  time_entry[] = {
	(ADDR)&ID_OCTET_STRING4,
	(ADDR)&ID_OCTET_STRING6,
	(ADDR)&ID_LONG_UNSIGNED
};
CONST IDENT time_st = { DT_STRUCTURE, 3, (ADDR)time_entry };
CONST IDENT time_ar = { DT_ARRAY, 8, (ADDR)&time_st };

CADR  day_entry[] = {
	(ADDR)&ID_UNSIGNED,
	(ADDR)&time_ar,
};
CONST IDENT day_st = { DT_STRUCTURE, 2, (ADDR)day_entry };
CONST IDENT day_ar = { DT_ARRAY, 4, (ADDR)&day_st };


void G(0, 20, 2, timezone)(void) {

	ui08 len;
	IDENT ident;

	ident.l = get_value2(IST_TIMEZONE, obis_code[5], g_xdlms_io);
	ident.t = DT_OCTET_STRING;
	ident.e = NULL;
	put_attr(g_xdlms_io, ident.l, &ident);
}

void G(0, 20, 3, timezone)(void) {
	getset = get_value3;
	put_value_with_block(IST_TIMEZONE, &season_ar, &season_st);
}

void G(0, 20, 4, timezone)(void) {
	getset = get_value4;
	put_value_with_block(IST_TIMEZONE, &week_ar, &week_st);
}

void G(0, 20, 5, timezone)(void) {
	getset = get_value5;
	put_value_with_block(IST_TIMEZONE, &day_ar, &day_st);
}

void G(0, 20, 6, timezone)(void) {

	ui08 len;
	IDENT ident;

	ident.l = get_value6(IST_TIMEZONE, obis_code[5], g_xdlms_io);
	ident.t = DT_OCTET_STRING;
	ident.e = NULL;
	put_attr(g_xdlms_io, ident.l, &ident);
}

void S(0, 20, 6, timezone)(void) {
	take_value6(IST_TIMEZONE, (CADR)&ID_OCTET_STRING0);
}

void G(0, 20, 7, timezone)(void) {
	getset = get_value7;
	put_value_with_block(IST_TIMEZONE, &season_ar, &season_st);
}

void S(0, 20, 7, timezone)(void)
{
	getset = set_value7;
	take_value_with_block(IST_TIMEZONE, &season_ar, &season_st);
}

void G(0, 20, 8, timezone)(void)
{
	getset = get_value8;
	put_value_with_block(IST_TIMEZONE, &week_ar, &week_st);
}

void S(0, 20, 8, timezone)(void)
{
	getset = set_value8;
	take_value_with_block(IST_TIMEZONE, &week_ar, &week_st);
}

void G(0, 20, 9, timezone)(void)
{
	getset = get_value9;
	put_value_with_block(IST_TIMEZONE, &day_ar, &day_st);
}

void S(0, 20, 9, timezone)(void)
{
	getset = set_value9;
	take_value_with_block(IST_TIMEZONE, &day_ar, &day_st);
}

void G(0, 20, 10, timezone)(void)
{
	put_value( 10, IST_TIMEZONE, (CADR)&ID_OCTET_STRING12);
}

void S(0, 20, 10, timezone)(void)
{
	take_value10(IST_TIMEZONE, (CADR)&ID_OCTET_STRING12);
}

void M(20, 1, timezone)(void) {

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos])
	{
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_INTEGER))
	{
		return;
	}
	if (g_xdlms_io[0] == 0) {
		passiveActivation();
		RunScript(&demand_reset_lists[0], 1, sizeof(demand_reset_lists) / sizeof(ACTION_SCRIPTS));
		man_dmd = 1;
		record_log(LOG_CAL_ACT, 0);
//		record_log(LOG_RESET, 30);
	}
}

/*******************************************/
/*********** interface class 22 ************/
/*******************************************/
CADR script[] = { (ADDR)&ID_OCTET_STRING6,(ADDR)&ID_LONG_UNSIGNED };

CONST IDENT exeuted_scrpt[] = {
	{DT_STRUCTURE, 2, (ADDR)script },
};

CADR exeution_time_date[] = { (ADDR)&ID_OCTET_STRING4,(ADDR)&ID_OCTET_STRING5 };

CONST IDENT exeution_time_date_st[] = {
	{DT_STRUCTURE, 2, (ADDR)exeution_time_date },
};
CONST IDENT exeution_time_date1 = { DT_ARRAY, 1, (ADDR)&exeution_time_date_st };
CONST IDENT exeution_time_date3 = { DT_ARRAY, 3, (ADDR)&exeution_time_date_st };
CONST IDENT exeution_time_date6 = { DT_ARRAY, 6, (ADDR)&exeution_time_date_st };
/*---------------------------------*/
void G(0, 22, 2, billdate_act)(void)
{
	put_value( 2, idxobj, (CADR)&exeuted_scrpt);
}
void G(0, 22, 3, billdate_act)(void)
{
	put_value( 3, idxobj, (CADR)&ID_ENUM);
}
void G(0, 22, 4, billdate_act)(void)
{
	if (idxobj == IST_SM_TO_HES_ACT) {
		put_value(4, IST_SM_TO_HES_ACT, (CADR)&exeution_time_date6);

	}
	else {
		put_value(4, idxobj, (CADR)&exeution_time_date1);
	}
}
void S(0, 22, 4, billdate_act)(void)
{
	if (idxobj == IST_SM_TO_HES_ACT) {
		take_value4(IST_SM_TO_HES_ACT, (CADR)&exeution_time_date6);
	}
	else {
		take_value4(idxobj, (CADR)&exeution_time_date1);
	}
}
/*******************************************/
/*********** interface class 23 ************/
/*******************************************/

CADR  setup_hdlc0[] = {
	(ADDR)&ID_ENUM,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_LONG_UNSIGNED,
};

void set_tsnset(const USERSET1 *in)
{
	I(23, 2, setup_hdlc0) = in->tsncfg[0].baud;			/* comm_speed enum (6) 19200 */
	I(23, 3, setup_hdlc0) = in->tsncfg[0].win_size_snd;	/* window_size_transmit */
	I(23, 4, setup_hdlc0) = in->tsncfg[0].win_size_rcv;	/* window_size_recieve */
	I(23, 5, setup_hdlc0) = in->tsncfg[0].info_len_snd;	/* max_info_field_length_transmit */
	I(23, 6, setup_hdlc0) = in->tsncfg[0].info_len_rcv;	/* max_info_field_length_receive */
	I(23, 7, setup_hdlc0) = in->tsncfg[0].inter_timeout;/* inter_octet_time_out */
	I(23, 8, setup_hdlc0) = in->tsncfg[0].inact_timeout;
	I(23, 9, setup_hdlc0) = 256;

	I(23, 2, setup_hdlc1) = in->tsncfg[1].baud;			/* comm_speed enum (6) 19200 */
	I(23, 3, setup_hdlc1) = in->tsncfg[1].win_size_snd;	/* window_size_transmit */
	I(23, 4, setup_hdlc1) = in->tsncfg[1].win_size_rcv;	/* window_size_recieve */
	I(23, 5, setup_hdlc1) = in->tsncfg[1].info_len_snd;	/* max_info_field_length_transmit */
	I(23, 6, setup_hdlc1) = in->tsncfg[1].info_len_rcv;	/* max_info_field_length_receive */
	I(23, 7, setup_hdlc1) = in->tsncfg[1].inter_timeout;/* inter_octet_time_out */
	I(23, 8, setup_hdlc1) = in->tsncfg[1].inact_timeout;
	I(23, 9, setup_hdlc1) = 256;

	if (!(0 == I(23, 8, setup_hdlc0)))
	{
		if (0x0e10 < I(23, 8, setup_hdlc0))		/* 上限3600より大きい? */
		{/* 初期値に設定 */
			I(23, 8, setup_hdlc0) = 0x012c;		/* inactivity_time_out */
		}
		if (0x000a > I(23, 8, setup_hdlc0))		/* 下限30より小さい? */
		{/* 初期値に設定 */
			I(23, 8, setup_hdlc0) = 0x0a;		/* inactivity_time_out */
		}
	}
	if (!(0 == I(23, 8, setup_hdlc1)))
	{
		if (0x0e10 < I(23, 8, setup_hdlc1))		/* 上限3600より大きい? */
		{/* 初期値に設定 */
			I(23, 8, setup_hdlc1) = 0x012c;		/* inactivity_time_out */
		}
		if (0x000a > I(23, 8, setup_hdlc1))		/* 下限30より小さい? */
		{/* 初期値に設定 */
			I(23, 8, setup_hdlc1) = 0x0a;		/* inactivity_time_out */
		}
	}
}

void get_tsnset(USERSET1 *out)
{
	out->tsncfg[0].baud = I(23, 2, setup_hdlc0);		 /* comm_speed enum (6) 19200 */
	out->tsncfg[0].win_size_snd = I(23, 3, setup_hdlc0); /* window_size_transmit */
	out->tsncfg[0].win_size_rcv = I(23, 4, setup_hdlc0); /* window_size_recieve */
	out->tsncfg[0].info_len_snd = I(23, 5, setup_hdlc0); /* max_info_field_length_transmit */
	out->tsncfg[0].info_len_rcv = I(23, 6, setup_hdlc0); /* max_info_field_length_receive */
	out->tsncfg[0].inter_timeout = I(23, 7, setup_hdlc0);/* inter_octet_time_out */
	out->tsncfg[0].inact_timeout = I(23, 8, setup_hdlc0);

	out->tsncfg[1].baud = I(23, 2, setup_hdlc1);		/* comm_speed enum (6) 19200 */
	out->tsncfg[1].win_size_snd = I(23, 3, setup_hdlc1);/* window_size_transmit */
	out->tsncfg[1].win_size_rcv = I(23, 4, setup_hdlc1);/* window_size_recieve */
	out->tsncfg[1].info_len_snd = I(23, 5, setup_hdlc1);/* max_info_field_length_transmit */
	out->tsncfg[1].info_len_rcv = I(23, 6, setup_hdlc1); /* max_info_field_length_receive */
	out->tsncfg[1].inter_timeout = I(23, 7, setup_hdlc1);/* inter_octet_time_out */
	out->tsncfg[1].inact_timeout = I(23, 8, setup_hdlc1);
}

static void get_inactivity_time_out(pui16 timeout, cui08 port)
{
	if (port == 1) {
		*timeout = I(23, 8, setup_hdlc0);
		if (jgndat.reset == 0x0d) {
			*timeout = 1;
		}
	}
	else {
		*timeout = I(23, 8, setup_hdlc1);
	}
}

void G(0, 23, 2, setup_hdlc0)(void)
{
	put_attr(&I(23, 2, setup_hdlc0), 0, (ADDR)setup_hdlc0[0]);
}

void G(0, 23, 3, setup_hdlc0)(void)
{
	put_attr(&I(23, 3, setup_hdlc0), 0, (ADDR)setup_hdlc0[1]);
}

void G(0, 23, 4, setup_hdlc0)(void)
{
	put_attr(&I(23, 4, setup_hdlc0), 0, (ADDR)setup_hdlc0[2]);
}

void G(0, 23, 5, setup_hdlc0)(void)
{
	put_attr((pui08)&I(23, 5, setup_hdlc0), 0, (ADDR)setup_hdlc0[3]);
}

void G(0, 23, 6, setup_hdlc0)(void)
{
	put_attr((pui08)&I(23, 6, setup_hdlc0), 0, (ADDR)setup_hdlc0[4]);
}

void G(0, 23, 7, setup_hdlc0)(void)
{
	put_attr((pui08)&I(23, 7, setup_hdlc0), 0, (ADDR)setup_hdlc0[5]);
}

void G(0, 23, 8, setup_hdlc0)(void)
{
	put_attr((pui08)&I(23, 8, setup_hdlc0), 0, (ADDR)setup_hdlc0[6]);
}

void G(0, 23, 9, setup_hdlc0)(void)
{
	put_attr((pui08)&I(23, 9, setup_hdlc0), 0, (ADDR)setup_hdlc0[7]);
}

void S(0, 23, 8, setup_hdlc0)(void)
{
	ui16 value;

	if (FALSE == take_attr((ADDR)setup_hdlc0[6]))
	{
		return;
	}

	value = UI16B(g_xdlms_io);
	if (!(0 == value))
	{
		if (0x0e10 < value)	{
			/* Error : invalid bottom parameter */
			record_log(ETSN1, 21);
			result_xdlms = RET_TEMPORARY_FAILURE;
			return;
		}
		if (0x000a > value)	{
			/* Error : invalid upper parameter */
			record_log(ETSN1, 21);
			result_xdlms = RET_TEMPORARY_FAILURE;
			return;
		}
	}
	I(23, 8, setup_hdlc0) = value;
}

void G(0, 23, 2, setup_hdlc1)(void)/* tsushin設定 備考: 追加2013/02/05 0.7版 ACSE用 */
{
	put_attr(&I(23, 2, setup_hdlc1), 0, (ADDR)setup_hdlc0[0]);
}

void G(0, 23, 3, setup_hdlc1)(void)/* tsushin設定 備考: 追加2013/02/05 0.7版 ACSE用 */
{
	put_attr(&I(23, 3, setup_hdlc1), 0, (ADDR)setup_hdlc0[1]);
}

void G(0, 23, 4, setup_hdlc1)(void)/* tsushin設定 備考: 追加2013/02/05 0.7版 ACSE用 */
{
	put_attr(&I(23, 4, setup_hdlc1), 0, (ADDR)setup_hdlc0[2]);
}

void G(0, 23, 5, setup_hdlc1)(void)
{
	put_attr((pui08)&I(23, 5, setup_hdlc1), 0, (ADDR)setup_hdlc0[3]);
}

void G(0, 23, 6, setup_hdlc1)(void)
{
	put_attr((pui08)&I(23, 6, setup_hdlc1), 0, (ADDR)setup_hdlc0[4]);
}

void G(0, 23, 7, setup_hdlc1)(void)
{
	put_attr((pui08)&I(23, 7, setup_hdlc1), 0, (ADDR)setup_hdlc0[5]);
}

void G(0, 23, 8, setup_hdlc1)(void)
{
	put_attr((pui08)&I(23, 8, setup_hdlc1), 0, (ADDR)setup_hdlc0[6]);
}

void G(0, 23, 9, setup_hdlc1)(void)
{
	put_attr((pui08)&I(23, 9, setup_hdlc1), 0, (ADDR)setup_hdlc0[7]);
}

void S(0, 23, 8, setup_hdlc1)(void)
{
	ui16 value;

	if (TRUE == take_attr((ADDR)setup_hdlc0[6]))
	{
		value = UI16B(g_xdlms_io);
		if (!(0 == value))
		{
			if (0x0e10 < value) {
				/* Error : invalid bottom parameter */
				record_log(ETSN1, 21);
				result_xdlms = RET_TEMPORARY_FAILURE;
				return;
			}
			if (0x000a > value) {
				/* Error : invalid upper parameter */
				record_log(ETSN1, 21);
				result_xdlms = RET_TEMPORARY_FAILURE;
				return;
			}
		}
		I(23, 8, setup_hdlc1) = value;
	}
}

#define SNRM_I5 5
#define SNRM_I6 9
#define SNRM_I7 13
#define SNRM_I8 16

ui08 getNegoBaud0(void) {
	return I(23, 2, setup_hdlc0);
}

ui08 getNegoBaud1(void) {
	return I(23, 2, setup_hdlc1);
}

void getNegoParam1(pui16 len_t, pui16 len_r, pui08 win_t, pui08 win_r) {

	*win_t = I(23, 3, setup_hdlc0);
	*win_r = I(23, 4, setup_hdlc0);
	*len_t = I(23, 5, setup_hdlc0);
	*len_r = I(23, 6, setup_hdlc0);
}

void getNegoParam2(pui16 len_t, pui16 len_r, pui08 win_t, pui08 win_r) {

	*win_t = I(23, 3, setup_hdlc1);
	*win_r = I(23, 4, setup_hdlc1);
	*len_t = I(23, 5, setup_hdlc1);
	*len_r = I(23, 6, setup_hdlc1);
}

void getTimeout1(pui16 oct, pui16 act) {
	*oct = I(23, 7, setup_hdlc0) / 2;
	*act = I(23, 8, setup_hdlc0) / 2;
}

void getTimeout2(pui16 oct, pui16 act) {
	*oct = I(23, 7, setup_hdlc1) / 2;
	*act = I(23, 8, setup_hdlc1) / 2;
}

/*********************end of interface class 23 ****************************/

/*******************************************/
/*********** interface class 40 ************/
/*******************************************/

CADR restriction_element[] = { (ADDR)&ID_ENUM,(ADDR)&ID_NULL_DATA };
CONST IDENT restriction[] = {
	{DT_STRUCTURE, 2, (ADDR)restriction_element },
};

CADR  push_list[] = {
	(ADDR)&ID_LONG_UNSIGNED,
	(ADDR)&ID_OCTET_STRING6,
	(ADDR)&ID_INTEGER,
	(ADDR)&ID_LONG_UNSIGNED,
#if 0
	(ADDR)&restriction
#endif
};

CONST IDENT push_object_definition = { DT_STRUCTURE, 4, (ADDR)push_list };
void put_push_objects(PCAPDEF2 cap, CPIDT ident)
{
	DAT2 dat2;
	uint offset = 0, i;
	PCAPDEF2	pcap;

	for (i = 0; i < ident->l; i++) {
		pcap = (PCAPDEF2)&cap[i];
		dat2.hf = g_cls[g_ist[pcap->ist].cls].cid;
		ramcpy(&g_xdlms_io[offset], dat2.by, 2);
		offset += 2;
		romcpy(&g_xdlms_io[offset], &g_ist[pcap->ist].oid[0], 5);
		offset += 5;
		g_xdlms_io[offset] = 0xff;
		offset++;
		g_xdlms_io[offset] = pcap->atr;
		offset++;
		g_xdlms_io[offset] = pcap->dat[1];
		offset++;
		g_xdlms_io[offset] = pcap->dat[0];
		offset++;
#if 0
		g_xdlms_io[offset] = 0x00;
		offset++;
#endif
	}
	put_attr(g_xdlms_io, 0, (ADDR)ident);
}

cui08 push_attr3[] = {
	0xc8,
	0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,
	0x80 };

CADR send_destination_and_method_st[] = {
	(ADDR)&ID_ENUM,
	(ADDR)&ID_OCTET_STRING0,
	(ADDR)&ID_ENUM
};

CONST IDENT send_destination_and_method[] = {
	{DT_STRUCTURE, 3, (ADDR)send_destination_and_method_st },
};

cui08 push_attr4[] = {
	0x01,0x01,
		0x02,0x02,
		0x09,0x0c,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0x09,0x0c,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

CADR window_element[] = { (ADDR)&ID_OCTET_STRING12,(ADDR)&ID_OCTET_STRING12 };
CONST IDENT communication_window_st[] = {
	{DT_STRUCTURE, 2, (ADDR)window_element },
};

cui08 push_attr5[] = {0x12,0x00,0x00};
cui08 push_attr6[] = {0x11,0x00};
cui08 push_attr7[] = {0x12,0x00,0x00};
cui08 push_attr8[] = {0x09,0x00};
cui08 push_attr9[] = {0x0f,0x30};
cui08 push_attr10[] ={
	0x01,0x01,
		0x02,0x02,
			0x16,0x00,
			0x02,0x05,
				0x09,0x01,0x00,
				0x09,0x08,0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
				0x09,0x08,0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
				0x09,0x01,0x00,
				0x02,0x02,
					0x16,0x00,
					0x16,0x00
};
/*
	Device ID
	Push setup ID
	Real time clock
	Instantaneous Parameter1
	Instantaneous Parameter2
	Instantaneous Parameter3
	Instantaneous Parameter4
	Instantaneous Parameter5
	Instantaneous Parameter6
	Instantaneous Parameter7
	Instantaneous Parameter8
	Instantaneous Parameter9
	Instantaneous Parameter10
*/
#pragma section COMM
CONST CAPDEF K(40, 2, pushd_hes)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHD_HES,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_SNAP_PARAM1,0x0002,{ 0x01, 0x10 },
};
CONST IDENT pushd_hes_atr2 = { DT_ARRAY, 4, (ADDR)&capture_objects };

CONST CAPDEF K(40, 2, pushd_lod)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHD_LOD,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_LOAD_PROFILE,	0x0002,{ 0x04, 0x10 },
};
CONST IDENT pushd_lod_atr2 = { DT_ARRAY, 4, (ADDR)&capture_objects };

CONST CAPDEF K(40, 2, pushd_day)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHD_DAY,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_DAILY_PROFILE,0x0002,{ 0x01, 0x10 },
};
CONST IDENT pushd_day_atr2 = { DT_ARRAY, 4, (ADDR)&capture_objects };

CONST CAPDEF K(40, 2, pushd_tamp)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHD_TAMP,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT1,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT2,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT3,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT4,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT5,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT6,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT7,0x0002,{ 0x00, 0x00 },
};
CONST IDENT pushd_tamp_atr2 = { DT_ARRAY, 10, (ADDR)&capture_objects };

CONST CAPDEF K(40, 2, pushd_bill)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHD_BILL,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_BILLING_PARAMS,0x0002,{ 0x01, 0x10 },
};
CONST IDENT pushd_bill_atr2 = { DT_ARRAY, 4, (ADDR)&capture_objects };

CONST CAPDEF K(40, 2, pushd_selfreg)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHD_SELFREG,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_IPV6,	0x0004,{ 0x00, 0x00 },
	0,IST_NIC_SIMID,0x0002,{ 0x00, 0x00 },
	0,IST_NIC_IMEI,0x0002,{ 0x00, 0x00 },
};
CONST IDENT pushd_selfreg_atr2 = { DT_ARRAY, 6, (ADDR)&capture_objects };

CONST CAPDEF K(40, 2, pushe_hes)[] = {
	0,IST_DEVICE_ID,0x0002,{ 0x00, 0x00 },
	0,IST_PUSHE_HES,0x0001,{ 0x00, 0x00 },
	0,IST_DATETIME_NOW,0x0002,{ 0x00, 0x00 },
	0,IST_EVENT_STATUS,0x0002,{ 0x00, 0x00 },
	0,IST_SECURITY_PUSH,0x0005,{ 0x00, 0x00 },
	0,IST_IPV6,0x0004,{ 0x00, 0x00 },
};
CONST IDENT pushe_hes_atr2 = { DT_ARRAY, 6, (ADDR)&capture_objects };

#pragma section

/*---------------*/
void G(0, 40, 2, pushd_hes)(void) {

	PCAPDEF pcap;
	PIDNT	pidnt;

	switch (idxobj) {
	case IST_PUSHD_TAMP: /* 444: SM to HES Tamper push */
		pcap = &K(40, 2, pushd_tamp)[0];
		pidnt = &pushd_tamp_atr2;
		break;
	case IST_PUSHE_HES: /* 446: SM TO HES event push */
		pcap = &K(40, 2, pushe_hes)[0];
		pidnt = &pushe_hes_atr2;
		break;
	case IST_PUSHD_SELFREG: /* 447: Self Registration Push */
		pcap = &K(40, 2, pushd_selfreg)[0];
		pidnt = &pushd_selfreg_atr2;
		break;
	case IST_PUSHD_HES:
		pcap = &K(40, 2, pushd_hes)[0];
		pidnt = &pushd_hes_atr2;
		break;
	case IST_PUSHD_DAY: /* 442: SM to HES daily profle push */
		pcap = &K(40, 2, pushd_day)[0];
		pidnt = &pushd_day_atr2;
		break;
	case IST_PUSHD_BILL: /* 443: SM to HES Billing profle push */
		pcap = &K(40, 2, pushd_bill)[0];
		pidnt = &pushd_bill_atr2;
		break;
	case IST_PUSHD_LOD:
		pcap = &K(40, 2, pushd_lod)[0];
		pidnt = &pushd_lod_atr2;
		break;
	default:
		pcap = NULL;
		pidnt = NULL;
		break;
	}
	if (pidnt != NULL) {
		if (block_no == 0) {
			record_offset = 0;
			put_capture_objects(pcap, pidnt->l);
		}
		readheap(g_xdlms_io, MAX_XDLMS_BUF, record_offset); 
		record_offset += put_attr(g_xdlms_io, 0, pidnt);
	}
}

void G(0, 40, 3, pushd_hes)(void) {

	ui16 len;
	IDENT type, st;
	ADDR mbr[3];

	len = get_value3(idxobj, obis_code[5], g_xdlms_io);
	type.t = DT_OCTET_STRING;
	type.l = len - 2;
	type.e = NULL;

	mbr[0] = &ID_ENUM;
	mbr[1] = &type;
	mbr[2] = &ID_ENUM;

	st.t = DT_STRUCTURE;
	st.l = 3;
	st.e = mbr;
	put_attr(g_xdlms_io, 0, &st);
}

void S(0, 40, 3, pushd_hes)(void) {
	take_value3(idxobj, (CADR)&send_destination_and_method);
}

void G(0, 40, 4, pushd_hes)(void) {
	g_raw_len = sizeof(push_attr4);
	ramcpy(g_raw, push_attr4, g_raw_len);
}

void G(0, 40, 5, pushd_hes)(void) {
	g_raw_len = sizeof(push_attr5);
	ramcpy(g_raw, push_attr5, g_raw_len);
}

void S(0, 40, 5, pushd_hes)(void) {
	take_value5(idxobj, (CADR)&send_destination_and_method);
}

void G(0, 40, 6, pushd_hes)(void) {
	g_raw_len = sizeof(push_attr6);
	ramcpy(g_raw, push_attr6, g_raw_len);
}

void S(0, 40, 6, pushd_hes)(void) {
	take_value6(idxobj, (CADR)&send_destination_and_method);
}


void G(0, 40, 7, pushd_hes)(void) {
	g_raw_len = sizeof(push_attr7);
	ramcpy(g_raw, push_attr7, g_raw_len);
}
void S(0, 40, 7, pushd_hes)(void) {
	take_value7(idxobj, (CADR)&send_destination_and_method);
}

void M(40, 1, pushd_hes)(void) {

	ui08 i, max;
	ui08 obis[6];

	g_raw_len = 0;
//	g_raw[g_raw_len] = 0x01;
//	g_raw_len++;/* BOOL */
//	g_raw[g_raw_len] = 0x00;
//	g_raw_len++;/* [0] DATA */
	max = pushd_hes_atr2.l;
	put_complex_count(max, &ID_STRUCTURE0);
	for (i = 0; i < max; i++) {
		if (K(40, 2, pushd_hes)[i].atr == 1) {
			romcpy(obis, g_ist[K(40, 2, pushd_hes)[i].ist].oid, 5);
			obis[5] = 0xff;
			put_attr(obis, 0, (ADDR)&ID_OCTET_STRING6);
		}
		else {
			Execute(G0, K(40, 2, pushd_hes)[i].ist, K(40, 2, pushd_hes)[i].atr - 2);
			if (xdlms_phase == PHASE_THREAD) {
				while (PHASE_RESPONSE != thread_process());
			}
		}
	}
	xdlms_phase = PHASE_RESPONSE;
}

void G(0, 40, 2, pushe_hes)(void) {
	put_push_objects(&K(40, 2, pushe_hes)[0], &pushe_hes_atr2);
}

void G(0, 40, 3, pushe_hes)(void) {

	ui16 len;
	IDENT type, st;
	ADDR mbr[3];

	len = get_value3(IST_PUSHE_HES, obis_code[5], g_xdlms_io);
	type.t = DT_OCTET_STRING;
	type.l = len - 2;
	type.e = NULL;

	mbr[0] = &ID_ENUM;
	mbr[1] = &type;
	mbr[2] = &ID_ENUM;

	st.t = DT_STRUCTURE;
	st.l = 3;
	st.e = mbr;
	put_attr(g_xdlms_io, 0, &st);
}

void S(0, 40, 3, pushe_hes)(void) {

	take_value3(IST_PUSHE_HES, (CADR)&send_destination_and_method);
}

void G(0, 40, 4, pushe_hes)(void) {
	g_raw_len = sizeof(push_attr4);
	ramcpy(g_raw, push_attr4, g_raw_len);
}
void G(0, 40, 5, pushe_hes)(void) {
	g_raw_len = sizeof(push_attr5);
	ramcpy(g_raw, push_attr5, g_raw_len);
}
void G(0, 40, 6, pushe_hes)(void) {
	g_raw_len = sizeof(push_attr6);
	ramcpy(g_raw, push_attr6, g_raw_len);
}
void G(0, 40, 7, pushe_hes)(void) {
	g_raw_len = sizeof(push_attr7);
	ramcpy(g_raw, push_attr7, g_raw_len);
}

void M(40, 1, pushe_hes)(void) {

	ui08 i, max;
	ui08 obis[6];

	g_raw_len = 0;
//	g_raw[g_raw_len] = 0x01;
//	g_raw_len++;/* BOOL */
//	g_raw[g_raw_len] = 0x00;
//	g_raw_len++;/* [0] DATA */
	event_push |= 0x40;
	max = pushe_hes_atr2.l;
	put_complex_count(max, &ID_STRUCTURE0);
	for (i = 0; i < max; i++) {
		if (K(40, 2, pushe_hes)[i].atr == 1) {
			romcpy(obis, g_ist[K(40, 2, pushe_hes)[i].ist].oid, 5);
			obis[5] = 0xff;
			put_attr(obis, 0, (ADDR)&ID_OCTET_STRING6);
		}
		else {
			Execute(G0, K(40, 2, pushe_hes)[i].ist, K(40, 2, pushe_hes)[i].atr - 2);
			if (xdlms_phase == PHASE_THREAD) {
				while (PHASE_RESPONSE != thread_process());
			}
		}
	}
	event_push |= 0x0f;
	xdlms_phase = PHASE_RESPONSE;
}
/*******************************************/
/*********** interface class 41 ************/
/*******************************************/
cui08 tcp_udp_attr2[] = { 0x12,0x0f,0xdb };

#if 0
cui08 tcp_udp_attr3[] = { 0x09,0x06,0x00,0x00,0x00,0x00,0x00,0x00};
cui08 tcp_udp_attr4[] = { 0x12,0x02,0x40 };
cui08 tcp_udp_attr5[] = { 0x11,0x00 };
cui08 tcp_udp_attr6[] = { 0x12,0x01,0x2c };
#endif
void G(0, 41, 2, tcp_udp)(void) {
	g_raw_len = sizeof(tcp_udp_attr2);
	ramcpy(g_raw, tcp_udp_attr2, g_raw_len);
}
#if 0
void G(0, 41, 3, tcp_udp)(void) {
	g_raw_len = sizeof(tcp_udp_attr3);
	ramcpy(g_raw, tcp_udp_attr3, g_raw_len);
}
void G(0, 41, 4, tcp_udp)(void) {
	g_raw_len = sizeof(tcp_udp_attr4);
	ramcpy(g_raw, tcp_udp_attr4, g_raw_len);
}
void G(0, 41, 5, tcp_udp)(void) {
	g_raw_len = sizeof(tcp_udp_attr5);
	ramcpy(g_raw, tcp_udp_attr5, g_raw_len);
}
void G(0, 41, 6, tcp_udp)(void) {
	g_raw_len = sizeof(tcp_udp_attr6);
	ramcpy(g_raw, tcp_udp_attr6, g_raw_len);
}
#endif
/*******************************************/
/*********** interface class 45 ************/
/*******************************************/
void G(0, 45, 2, gprs)(void) {

	put_value(2, idxobj, (ADDR)&ID_OCTET_STRING0);
}


void S(0, 45, 2, gprs)(void) {
	take_value2(IST_GPRS, (CADR)&ID_OCTET_STRING0);
}

/*******************************************/
/*********** interface class 48 ************/
/*******************************************/
cui08 ipv6_attr2[] = { 0x09,0x00 };
cui08 ipv6_attr3[] = { 0x16,0x02 };
cui08 ipv6_attr9[] = { 0x11,0x00 };
cui08 ipv6_attr10[] = { 0x01,0x01,0x02,0x03,0x11,0x00,0x12,0x00,0x00,0x06,0x00,0x00,0x00,0x00 };

void put_octet_string(cpui08 in, cui08 len) {

	g_raw[g_raw_len++] = DT_OCTET_STRING;
	g_raw[g_raw_len++] = len;
	ramcpy(&g_raw[g_raw_len], in, len);
	g_raw_len += len;
}

ui08 take_addr_octet_string(pui08 out, pui08 in, uint *offset) {

	ui08 len;

	len = in[(*offset)++];
	if (len < 8) {
		ramcpy(out, &in[(*offset)++], len);
		*offset += len;
	}
	return len;
}

void G(0, 48, 2, ipv6)(void) {
	g_raw_len = sizeof(ipv6_attr2);
	ramcpy(g_raw, ipv6_attr2, g_raw_len);
}

void G(0, 48, 3, ipv6)(void) {/*information[0]*/
	g_raw_len = sizeof(ipv6_attr3);
	ramcpy(g_raw, ipv6_attr3, g_raw_len);
}

void G(0, 48, 4, ipv6)(void) {/*unicast*/

	put_complex_count(1, &ID_ARRAY0);
	put_value(4, IST_IPV6, (CADR)&ID_OCTET_STRING0);
}

/*******************************************/
/************ interface class 64************/
/*******************************************/
CADR  security_none[] = {
	(ADDR)&ID_ENUM,
	(ADDR)&ID_ENUM,
	(ADDR)&ID_OCTET_STRING8,
	(ADDR)&ID_OCTET_STRING8,
};
CONST IDENT security_hls_key_type = { DT_ENUM, 1, NULL };
CONST IDENT security_hls_key_value = { DT_OCTET_STRING, 0x18, NULL };
CADR  security_hls_key[] = { (ADDR)&security_hls_key_type, (ADDR)&security_hls_key_value };
CONST IDENT security_hls_key_st = { DT_STRUCTURE, 0x02, (ADDR)security_hls_key };
CONST IDENT security_hls_method = { DT_ARRAY, 0x01, (ADDR)&security_hls_key_st };

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
#define cl64_function( ist )\
void G(0,64,2,ist)( void )\
{\
	put_attr( &I(64,2,ist), 0, (ADDR)security_none[0] );\
}\
void G(0,64,3,ist)( void )\
{\
	put_attr( &I(64,3,ist), 0, (ADDR)security_none[1] );\
}\
void G(0,64,4,ist)( void )\
{\
	put_attr( I(64,4,security), 0, (ADDR)security_none[2] );\
}\
void G(0,64,5,ist)( void )\
{\
	put_attr( get_system_title( ), 0, (ADDR)security_none[3] );\
}

cui08 SC[] = { 0x00, 0x10, 0x20, 0x30 };
/********* security_none *********/
cui08 I(64, 2, security_none) = SECURITY_POLICY_NN;	/* 2.security_policy */
cui08 I(64, 3, security_none) = 0x00;					/* 3.security_suite */

cl64_function(security_none)

/********* end of security_none *********/
/********* security_lls *********/
cui08 I(64, 2, security_lls) = SECURITY_POLICY_NN;	/* 2.security_policy */
cui08 I(64, 3, security_lls) = 0x00;				/* 3.security_suite */

cl64_function(security_lls)

/********* end of security_none *********/

/********* security_hls *********/
cui08 I(64, 2, security_hls) = SECURITY_POLICY_AE;	/* 2.security_policy */
cui08 I(64, 3, security_hls) = 0x00;				/* 3.security_suite */

cl64_function(security_hls)

/********* security_ihd *********/
cui08 I(64, 2, security_ihd) = SECURITY_POLICY_NE;	/* 2.security_policy */
cui08 I(64, 3, security_ihd) = 0x00;				/* 3.security_suite */

cl64_function(security_ihd)

/********* security_push *********/
cui08 I(64, 2, security_push) = SECURITY_POLICY_AE;	/* 2.security_policy */
cui08 I(64, 3, security_push) = 0x00;				/* 3.security_suite */

cl64_function(security_push)

/********* security_mr *********/
cui08 I(64, 2, security_mr) = SECURITY_POLICY_NE;	/* 2.security_policy */
cui08 I(64, 3, security_mr) = 0x00;					/* 3.security_suite */

cl64_function(security_mr)

/********* security_firm *********/
cui08 I(64, 2, security_firm) = SECURITY_POLICY_AE;	/* 2.security_policy */
cui08 I(64, 3, security_firm) = 0x00;				/* 3.security_suite */

cl64_function(security_firm)

/********* security_us *********/
cui08 I(64, 2, security_us) = SECURITY_POLICY_AE;	/* 2.security_policy */
cui08 I(64, 3, security_us) = 0x00;					/* 3.security_suite */

cl64_function(security_us)

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void M(64, 1, security_mr)(void) {

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_ENUM)) {
		return;
	}
	//	I(64, 2, security_mr) = g_xdlms_io[0];
	return;
}

static cui08 iv_init[] = { 0xa6,0xa6,0xa6,0xa6,0xa6,0xa6,0xa6,0xa6 };
void M(64, 2, security_none)(void) {
	return;
}
void change_key_method(cui08 user) {

	ui08 res, type;

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		/* Fatal : incorrect remain raw */
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	res = take_attr((ADDR)&security_hls_method);
	if (FALSE == res) {
		return;
	}
	type = 0xff;
	switch (g_xdlms_io[0]) {
	case 0:	/*global unicast*/
	case 1: /*global broadcast*/
		type = 8 + user;
		break;
	case 2:	/*authentication*/
		type = 16 + user;
		break;
	case 3: /*master key*/
		type = 0 + user;
		break;
	}
	if (0xff == type) {
		/* Fatal : incorrect key id */
		record_log(ETSN1, 21);
		result_xdlms = RET_TEMPORARY_FAILURE;
		return;
	}

	res = unwrap_key(&g_xdlms_io[1], iv_init);
	if (!(DLMS_SUCCESS == res))
	{
		/* Fatal : unwrap */
		if ((DLMS_ERROR == res) && (0 == type))
		{
			record_log(ETSN2, 9);
		}
		else
		{
			record_log(ETSN2, 4);
		}
		result_xdlms = RET_TEMPORARY_FAILURE;
		return;
	}
	g_xdlms_io[8] = 0x10;	/*長さを保存*/
	start_change_key(&result_xdlms, type, &g_xdlms_io[8]);
	return;

}

void M(64, 2, security_hls)(void) {
	if (g_user0 == 0) {
		change_key_method(0);
	}
	else {
		result_xdlms = RET_OTHER_REASON;
	}
}

void M(64, 2, security_mr)(void){
	change_key_method(3);
	return;
}

void M(64, 2, security_us)(void) {
	change_key_method(1);
	return;
}

void M(64, 2, security_push)(void) {
	change_key_method(4);
	return;
}

void M(64, 2, security_firm)(void) {
	change_key_method(2);
	return;
}
void M(64, 2, security_ihd)(void)
{
	change_key_method(5);
	return;
}

/********* end of security_hls *********/

/*********************end of interface class 64 ****************************/
/*******************************************/
/*********** interface class 70 ************/
/*******************************************/

void G(0, 70, 2, breaker)(void) {
	put_value( 2, IST_BREAKER, (CADR)&ID_BOOLEAN);
}
void G(0, 70, 3, breaker)(void) {
	put_value( 3, IST_BREAKER, (CADR)&ID_ENUM);
}
void G(0, 70, 4, breaker)(void) {
	put_value( 4, IST_BREAKER, (CADR)&ID_ENUM);
}
void S(0, 70, 4, breaker)(void) {
	take_value4(IST_BREAKER, (CADR)&ID_ENUM);
}

void M(70, 1, breaker)(void) {

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_INTEGER)) {
		return;
	}
	if (g_xdlms_io[0] == 0) {
		remote_disconnect();
	}
}

void M(70, 2, breaker)(void) {

	if (FALSE == g_xdlms_req.buf[g_xdlms_req.pos]) {
		record_log(ETSN1, 20);
		result_xdlms = RET_TYPE_UNMATCHED;
		return;
	}
	g_xdlms_req.pos++;

	if (FALSE == take_attr((ADDR)&ID_INTEGER)) {
		return;
	}
	if (g_xdlms_io[0] == 0) {
		remote_connect();
	}
}
/*******************************************/
/*********** interface class 71 ************/
/*******************************************/
#if 0
void G(0, 71, 2, load_limit)(void)
{
	/*2. monitored_value*/
	put_value( 2, IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG);
}
#endif
void G(0, 71, 3, load_limit)(void)
{
	/*3. threshold_active*/
	put_value( 3, IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 71, 4, load_limit)(void)
{
	/*4. threshold_normal*/
	put_value( 4, IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}
void S(0, 71, 4, load_limit)(void)
{
	take_value4(IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}
void G(0, 71, 5, load_limit)(void)
{
	/*5. threshold_emergency*/
	put_value( 5, IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}
void S(0, 71, 5, load_limit)(void)
{
	take_value5(IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 71, 6, load_limit)(void)
{
	/*6. min_over_threshold_duration*/
	put_value( 6, IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}
void S(0, 71, 6, load_limit)(void)
{
	take_value6(IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}

void G(0, 71, 7, load_limit)(void)
{
	/*7. min_under_threshold_duration*/
	put_value( 7, IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}
void S(0, 71, 7, load_limit)(void)
{
	take_value7(IST_LOAD_LIMIT, (CADR)&ID_DOUBLE_LONG_UNSIGNED);
}

/***************************************************************************/
/************************* Control function ********************************/
/***************************************************************************/

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 get_security_control(void)
{
	return SC[SECURITY_POLICY_AE];
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
pui08 get_system_title0(void)
{
	return I(64, 4, security);
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
pui08 get_system_title(void)
{
	return SYSTEM_TITLE;
}

#if 0
/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 is_client_accept_get(void)
{
	return (I(15, 5, asso_ln0)[3] & 0x10);
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 is_client_accept_set(void)
{
	return (I(15, 5, asso_ln0)[3] & 0x08);
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 is_client_accept_action(void)
{
	return (I(15, 5, asso_ln0)[3] & 0x01);
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 is_client_accept_block_transfer(void)
{
	return (I(15, 5, asso_ln0)[2] & 0x10);
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui16 get_max_pud_size(void)
{
	ui16 res;
	if (UI16L(&I(15, 5, asso_ln0)[4]) >= UI16L(&I(15, 5, asso_ln0)[6]))
	{
		res = UI16L(&I(15, 5, asso_ln0)[6]);
	}
	else
	{
		res = UI16L(&I(15, 5, asso_ln0)[4]);
	}
	return res;
}
#endif
/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 authenticated_client(void)
{
	if (
		(*I(15, 9, asso_ln0) == IST_SECURITY_HLS) ||
		(*I(15, 9, asso_ln0) == IST_SECURITY_IHD) ||
		(*I(15, 9, asso_ln0) == IST_SECURITY_FIRM) ||
		(*I(15, 9, asso_ln0) == IST_SECURITY_PUSH) ||
		(*I(15, 9, asso_ln0) == IST_SECURITY_MR) ||
		(*I(15, 9, asso_ln0) == IST_SECURITY_US)
		)
	{
		return TRUE;
	}
	return FALSE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 connected_client(void)
{
	if (NON_ASSOCIATED != exist_aa())
	{
		if ((g_portQ == g_port0) && (g_addrQ == get_addr()))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 associated_client(void)
{
	if (connected_client())
	{
		if (g_AA)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 authenticated_associated_client(void)
{
	if (associated_client())
	{
		if (authenticated_client())
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 challenge_client(void)
{
	if (connected_client())
	{
		if (authenticated_client())
		{
			if (FALSE == g_AA)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void init_association(void)
{
	as_ln0 = NULL;
	I(15, 3, asso_ln0) = NULL;	/* associated_partners_id */
/*	I(15,4,asso_ln0)	= NULL;*//* application_context_name */
	I(15, 5, asso_ln0) = NULL;/* xDLMS_context_info */
/*	I(15,6,asso_ln0)	= NULL;*//* authentication_mechanism_name */
/*	I(15,7,asso_ln0)	= NULL;*//* secret0 */
	I(15, 8, asso_ln0) = NULL;/* association status */
	I(15, 9, asso_ln0) = NULL;/* security_setup_reference */
	asso_ln03 = NULL;
	asso_ln04 = NULL;
	asso_ln05 = NULL;
	asso_ln06 = NULL;
	asso_ln07 = NULL;
	asso_ln08 = NULL;
	asso_ln09 = NULL;
	g_port0 = DLMS_INVALID;
	g_AA = FALSE;
	if (g_authQ & 0x80) {
		g_authQ = FALSE;
	}
	ramset(secret0, 0x00, LEN_SECRET);/* secret0 */
}

/****************************************************************/
/*																*/
/*	関数名				:	接続、認証を解放する。				*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void set_non_associated(cui08 sel) {
	ui08 port;

	if (sel == 3) {
		port = g_port0;
	}
	else {
		port = sel;
	}
	if (port == g_port0) {
		if (!(NULL == I(15, 5, asso_ln0))) {
			ramset(I(15, 5, asso_ln0), 0xff, LEN_XDLMS_CTX_INF);	/* xDLMS_context_info */
		}
		if (!(NULL == I(15, 8, asso_ln0))) {
			*I(15, 8, asso_ln0) = NON_ASSOCIATED;						/* association status */
		}
		if (!(NULL == I(15, 9, asso_ln0))) {
			ramset(I(64, 4, security), 0xff, LEN_SYSTEM_TITLE);		/* 4.client_system_title */
		}
		init_association();
	}
}

ui08 set_associated(cpui08 xdlms_info)
{
	ui08 ret, midx, rank;

	midx = get_mechanism_name_idxQ();
	rank = Rank(g_userQ);
	ret = FALSE;
	/* Associationクライアント情報のコピー */
	apdu_size -= 12;	/*get-set-act headder*/
	I(15, 6, asso_ln0) = (pui08)mechanism_name[midx];
	if (COSEM_PUBLIC == rank) {
		/* info : pub client */
		g_AA = TRUE;
		g_authQ = FALSE;
		g_port0 = g_portQ;
		g_user0 = g_userQ;
		as_ln0 = (PADR)&as_ln1[0];
		ramcpy(I(15, 5, asso_ln), xdlms_info, LEN_XDLMS_CTX_INF);
		ramcpy(I(64, 4, security), get_system_titleQ(), LEN_SYSTEM_TITLE);
		I(15, 3, asso_ln)[0] = g_addrQ >> 1;
		I(15, 3, asso_ln)[1] = 0x01;
		I(15, 3, asso_ln)[2] = 0x00;
		I(15, 3, asso_ln0) = (pui08)I(15, 3, asso_ln);		/* associated_partners_id */
/*		I(15,4,asso_ln0)	= (pui08)I(15,4,asso_ln1);*/	/* application_context_name */
		I(15, 5, asso_ln0) = I(15, 5, asso_ln);				/* xDLMS_context_info */
		I(15, 6, asso_ln1) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
		I(15, 8, asso_ln) = ASSOCIATED;						/* 認証確立 */
		I(15, 8, asso_ln0) = &I(15, 8, asso_ln);			/* association status */
		I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln1);	/* security_setup_reference */
		asso_ln03 = G(0, 15, 3, asso_ln1);
		asso_ln04 = G(0, 15, 4, asso_ln1);
		asso_ln05 = G(0, 15, 5, asso_ln1);
		asso_ln06 = G(0, 15, 6, asso_ln1);
		asso_ln07 = NULL;
		asso_ln08 = G(0, 15, 8, asso_ln1);
		asso_ln09 = G(0, 15, 9, asso_ln1);
		ret = TRUE;
	}

	if (COSEM_POWER == rank || COSEM_READER == rank) {
		apdu_size -= 7;
		/* info : ven client */
		g_AA = TRUE;
		g_port0 = g_portQ;
		g_user0 = g_userQ;
		as_ln0 = (PADR)&as_ln2[0];
		ramcpy(I(15, 5, asso_ln), xdlms_info, LEN_XDLMS_CTX_INF);
		ramcpy(I(64, 4, security), get_system_titleQ(), LEN_SYSTEM_TITLE);
		I(15, 3, asso_ln)[0] = g_addrQ >> 1;
		I(15, 3, asso_ln)[1] = 0x01;
		I(15, 3, asso_ln)[2] = 0x00;
		I(15, 3, asso_ln0) = (pui08)I(15, 3, asso_ln);			/* associated_partners_id */
/*		I(15,4,asso_ln0)	= (pui08)I(15,4,asso_ln2);*/		/* application_context_name */
		I(15, 5, asso_ln0) = I(15, 5, asso_ln);					/* xDLMS_context_info */
		I(15, 8, asso_ln) = ASSOCIATED;							/* 認証確立 */
		I(15, 8, asso_ln0) = &I(15, 8, asso_ln);				/* association status */
		switch (g_user0) {
		case 3:
			I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln2);	/* security_setup_reference */
			I(15, 6, asso_ln2) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
			asso_ln03 = G(0, 15, 3, asso_ln2);
			asso_ln04 = G(0, 15, 4, asso_ln2);
			asso_ln05 = G(0, 15, 5, asso_ln2);
			asso_ln06 = G(0, 15, 6, asso_ln2);
			asso_ln07 = S(0, 15, 7, asso_ln2);
			asso_ln08 = G(0, 15, 8, asso_ln2);
			asso_ln09 = G(0, 15, 9, asso_ln2);
			break;
		case 4:
			I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln4);	/* security_setup_reference */
			I(15, 6, asso_ln4) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
			asso_ln03 = G(0, 15, 3, asso_ln4);
			asso_ln04 = G(0, 15, 4, asso_ln4);
			asso_ln05 = G(0, 15, 5, asso_ln4);
			asso_ln06 = G(0, 15, 6, asso_ln4);
			asso_ln07 = S(0, 15, 7, asso_ln4);
			asso_ln08 = G(0, 15, 8, asso_ln4);
			asso_ln09 = G(0, 15, 9, asso_ln4);
			break;
		case 5:
			I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln6);	/* security_setup_reference */
			I(15, 6, asso_ln6) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
			asso_ln03 = G(0, 15, 3, asso_ln6);
			asso_ln04 = G(0, 15, 4, asso_ln6);
			asso_ln05 = G(0, 15, 5, asso_ln6);
			asso_ln06 = G(0, 15, 6, asso_ln6);
			asso_ln07 = S(0, 15, 7, asso_ln6);
			asso_ln08 = G(0, 15, 8, asso_ln6);
			asso_ln09 = G(0, 15, 9, asso_ln6);
			break;
		}
		ret = TRUE;
	}

	if (COSEM_ADMIN == rank) {
		apdu_size -= 19;
		/* info : com client */
		g_AA = FALSE;
		g_port0 = g_portQ;
		g_user0 = g_userQ;
		as_ln0 = (PADR)&as_ln3[0];
		fixed_secret();
		ramcpy(I(15, 5, asso_ln), xdlms_info, LEN_XDLMS_CTX_INF);
		ramcpy(I(64, 4, security), get_system_titleQ(), LEN_SYSTEM_TITLE);
		I(15, 3, asso_ln)[0] = g_addrQ >> 1;
		I(15, 3, asso_ln)[1] = 0x01;
		I(15, 3, asso_ln)[2] = 0x00;
		I(15, 3, asso_ln0) = (pui08)I(15, 3, asso_ln);	/* associated_partners_id */
/*		I(15,4,asso_ln0)	= (pui08)I(15,4,asso_ln2);*//* application_context_name */
		I(15, 5, asso_ln0) = I(15, 5, asso_ln);			/* xDLMS_context_info */
		I(15, 8, asso_ln) = ASSOCIATED;						/* 認証確立 */
		I(15, 8, asso_ln0) = &I(15, 8, asso_ln);			/* association status */
		switch (g_user0) {
		case 0:
		case 1:
			I(15, 6, asso_ln3) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
			I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln3);	/* security_setup_reference */
			asso_ln03 = G(0, 15, 3, asso_ln3);
			asso_ln04 = G(0, 15, 4, asso_ln3);
			asso_ln05 = G(0, 15, 5, asso_ln3);
			asso_ln06 = G(0, 15, 6, asso_ln3);
			asso_ln07 = NULL;
			asso_ln08 = G(0, 15, 8, asso_ln3);
			asso_ln09 = G(0, 15, 9, asso_ln3);
			break;
		case 2:
			I(15, 6, asso_ln5) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
			I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln5);	/* security_setup_reference */
			asso_ln03 = G(0, 15, 3, asso_ln5);
			asso_ln04 = G(0, 15, 4, asso_ln5);
			asso_ln05 = G(0, 15, 5, asso_ln5);
			asso_ln06 = G(0, 15, 6, asso_ln5);
			asso_ln07 = NULL;
			asso_ln08 = G(0, 15, 8, asso_ln5);
			asso_ln09 = G(0, 15, 9, asso_ln5);
			break;
		}
		ret = TRUE;
	}

	return ret;
}

/****************************************************************/
/*																*/
/*	関数名	: inactivity_timeout								*/
/*																*/
/*	内容		: inactivity timeout判断						*/
/*																*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当:							*/
/*																*/
/****************************************************************/
ui08 inactivity_timeout0(void)
{
	ui08 check, ret;

	ret = FALSE;
	if (!(0 == I(23, 8, setup_hdlc0))) {
		check = inact0_check();
		if (D_TIM_UP == check) {
			ret = TRUE;
		}
	}
	return ret;
}

ui08 inactivity_timeout1(void)
{
	ui08 check, ret;

	ret = FALSE;
	if (!(0 == I(23, 8, setup_hdlc1))) {
		check = inact1_check();
		if (D_TIM_UP == check) {
			ret = TRUE;
		}
	}
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名	: clear_inactivity_timeout							*/
/*																*/
/*	内容		: inactivity timeout開始処理					*/
/*																*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当:							*/
/*																*/
/****************************************************************/
void	clear_inactivity_timeout(cui08 port){
	ui16 timeout;

	get_inactivity_time_out(&timeout, port);
	if (!(0 == timeout))
	{
		if (port == 1) {
			inact0_start(timeout);
		}
		else {
			inact1_start(timeout);
		}
	}
}

/****************************************************************/
/*																*/
/*	関数名	: auth_com_client									*/
/*																*/
/*	内容	:													*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2019-03-15	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当:							*/
/*																*/
/****************************************************************/
ui08 session_client(void)
{
	ui08 ret;

	ret = 0;

	if (associated_client())
	{
		if (authenticated_client())
		{
			ret = Rank(g_user0);	/* should PEA or COM */
		}
		else
		{
			ret = COSEM_PUBLIC;		/*その他はPUBクライアントとして扱う*/
		}
	}
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 InitiateRequest(void)
{
#if 0
	ui08	associated;
#endif
	DAT2	pdu0, pdu1;
	ui08	response[LEN_INITIATE_RESPONSE], tmp;

	if ((MAX_XDLMS_RAW - g_raw_len) > LEN_INITIATE_RESPONSE) {
		romcpy(response, INITIATE_RESPONSE, LEN_INITIATE_RESPONSE);
	}
	else {
		/* Fatal : no remain raw */
		record_log(ETSN2, 5);
		return ERROR_HDLC_INVALID_ORDER;
	}

	tmp = g_xdlms_req.buf[g_xdlms_req.pos];
	g_xdlms_req.pos++;	/* bool */
	if (!(FALSE == tmp)) {	/* dedicated key */
		if (security_policyQ() == SECURITY_POLICY_NN) {
			return ERROR_ACSE_INVALID_INITIATE;
		}
#if 0
		if (get_mechanism_name_idxQ() < MECHA_HLS) {
			return ERROR_ACSE_INVALID_INITIATE;
		}
#endif
		tmp = g_xdlms_req.buf[g_xdlms_req.pos];
		g_xdlms_req.pos++;	/* len */
		if ((tmp % 8) > 0) {
			/* Error : invalid dedicated key length */
			return ERROR_ACSE_INVALID_INITIATE;
		}
		ramcpy(&g_xdlms_io[9], &g_xdlms_req.buf[g_xdlms_req.pos], tmp);
		g_xdlms_req.pos += tmp;
	}
	g_xdlms_req.pos++;									/* response allowed */

	g_xdlms_io[8] = g_xdlms_req.buf[g_xdlms_req.pos];	/* quality of service */
	if (g_xdlms_io[8] != 0) {
		g_xdlms_req.pos++;
//		return ERROR_ACSE_INVALID_INITIATE;
	}
	g_xdlms_req.pos++;

	g_xdlms_io[7] = g_xdlms_req.buf[g_xdlms_req.pos];	/* dlms version no */
	if (g_xdlms_io[7] < 6) {
		return ERROR_ACSE_INVALID_DLMS;
	}
	g_xdlms_req.pos++;

	if (
		(g_xdlms_req.buf[g_xdlms_req.pos + 0] == 0x5f) &&
		(g_xdlms_req.buf[g_xdlms_req.pos + 1] == 0x1f) &&
		(g_xdlms_req.buf[g_xdlms_req.pos + 2] == 0x04)) {
		g_xdlms_req.pos += 3;
		g_xdlms_req.pos += 1;	/*0x00*/
		/* kernel情報を保存する。 */
		if (g_xdlms_req.buf[g_xdlms_req.pos] != 0xff) {
			if ((g_xdlms_req.buf[g_xdlms_req.pos] & 0x1c) > 0) {
				return ERROR_ACSE_INVALID_CONF;
			}
		}
		response[6] = CONFORMANCE[g_userQ][1] & g_xdlms_req.buf[g_xdlms_req.pos];
		g_xdlms_req.pos++;
		response[7] = CONFORMANCE[g_userQ][2] & g_xdlms_req.buf[g_xdlms_req.pos];
		g_xdlms_req.pos++;
		response[8] = CONFORMANCE[g_userQ][3] & g_xdlms_req.buf[g_xdlms_req.pos];
		g_xdlms_req.pos++;
		ramcpy(g_xdlms_io, &response[6], 3);	/* conformance */
	}
	else
	{
		/* Error : incorrect conformance */
		return ERROR_ACSE_INVALID_INITIATE;
	}
	pdu0.by[0] = g_xdlms_req.buf[g_xdlms_req.pos + 1];	/* max send pdu size */
	pdu0.by[1] = g_xdlms_req.buf[g_xdlms_req.pos + 0];	/* max send pdu size */
	g_xdlms_req.pos += 2;
	pdu1.by[0] = response[9 + 1];
	pdu1.by[1] = response[9 + 0];

	if (pdu0.hf < 12) {
		return ERROR_ACSE_INVALID_PDU;
	}
	if (pdu0.hf > pdu1.hf) {
		pdu0.hf = pdu1.hf;
	}
	else {
		if (pdu0.hf < pdu1.hf) {
			pdu1.hf = pdu0.hf;
			response[9 + 1] = pdu1.by[0];
			response[9 + 0] = pdu1.by[1];
		}
	}
	g_xdlms_io[3] = pdu1.by[0];							/* max recieve pdu size */
	g_xdlms_io[4] = pdu1.by[1];							/* max recieve pdu size */
	g_xdlms_io[5] = pdu0.by[0];
	g_xdlms_io[6] = pdu0.by[1];

	if (g_xdlms_req.pos > g_xdlms_req.len) {
		/* Error : invalid length */
		return ERROR_ACSE_INVALID_INITIATE;
	}
	/* AA確立前、確立中であれば、AAを解放 */
	set_non_associated(3);
	apdu_size = pdu0.hf;
	if (TRUE == set_associated(g_xdlms_io)) {
		setUpperBuff(apdu_size);/*ブロック応答+データ型*/
		ramcpy(&g_raw[g_raw_len], response, LEN_INITIATE_RESPONSE);
		g_raw_len += LEN_INITIATE_RESPONSE;
		return DLMS_SUCCESS;
	}
	else
	{
		record_log(ETSN2, 7);
		return ERROR_HDLC_INVALID_ORDER;
	}
}


/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 get_dedicated_key(pui08 key)
{
	ui08 len;

	len = 0;
	if (NON_ASSOCIATED != exist_aa()) {
		if (TRUE == g_AA) {
			/* g_AA済クライアントが存在する. */
			if (authenticated_client()) {
				len = ramcpy(key, &I(15, 5, asso_ln0)[9], 16);
			}
		}
	}
	/* g_AA済クライアントは存在しない. */
	return len;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void clear_xdlms_req(void)
{
	ramset((pui08)&g_xdlms_req, 0, sizeof(IO));
	order_xdlms = DLMS_INVALID;
	xdlms_phase = PHASE_INITIAL;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
cui08 cse00 = 0x00;
cui08 cse01 = 0x01;
cui08 cse02 = 0x02;
cui08 cse03 = 0x03;
cui08 cse04 = 0x04;
cui08 cse05 = 0x05;
cui08 cse06 = 0x06;
cui08 cse0a = 0x0a;

void set_confirm_service_error(cpui08 in1, cpui08 in2, cpui08 in3)
{
	g_cosem_res.buf[g_cosem_res.pos] = CMD_CONFIRMEDSERVICEERROR;
	g_cosem_res.pos++;
	g_cosem_res.buf[g_cosem_res.pos] = *in1;
	g_cosem_res.pos++;
	g_cosem_res.buf[g_cosem_res.pos] = *in2;
	g_cosem_res.pos++;
	if (!(NULL == in3))
	{
		g_cosem_res.buf[g_cosem_res.pos] = *in3;
		g_cosem_res.pos++;
	}
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void ConfirmServiceError(cui08 err_id)
{
	ui08	order, offset;
	uint	tmp;

	clear_acse_req();
	clear_security();
	clear_xdlms_req();
	g_xdlms_res_len = 0;
	g_raw_len = 0;

	offset = OFFSET_CMD + g_apli_offset;
	order = g_cosem_res.buf[offset];
	g_cosem_res.pos = offset;
	if (order == (CMD_AARQ + 1)) {

		switch (err_id) {
		case ERROR_ACSE_INVALID_APPLI_TITLE:
			set_confirm_error_aare1(0x0e, 2, eBoolean);	/*eInteger*/
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_APPLICATION_TAG:
			set_confirm_error_aare2(0x01, 1);	/*SERVICE USER*/
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_APPLI_NAME:
			set_confirm_error_aare1(0x02, 2, eBoolean);	/*eInteger*/
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_VERSION:
			set_confirm_error_aare1(0x02, 2, eInteger);	/*eInteger*/
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_APPLI_NAME_VAL:
			set_confirm_error_aare2(0x02, 1);	/*SERVICE USER*/
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_APPLI_VALUE:
			set_confirm_error_aare1(0x0d, 2, eBoolean);	/*eInteger*/
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_AARQ:
		case ERROR_ACSE_REQUIREMENTS:
		case ERROR_ACSE_INVALID_MECHA_NAME:
		case ERROR_ACSE_INVALID_USER_INFORMATION:
			/* Fatal : incorrect remain raw */
			set_confirm_error_aare(0x0d, 3);
			set_confirm_service_error(&cse01, &cse0a, NULL);

			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_SECURITY_INVALID_LEN_SC:
		case ERROR_SECURITY_INVALID_AUTH:
			set_confirm_error_aare(0x01, 4);
			set_confirm_service_error(&cse01, &cse00, &cse06);

			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_INITIATE:
			/* Fatal : incorrect remain raw */
			set_confirm_error_aare(0x01, 4);
			set_confirm_service_error(&cse01, &cse06, &cse00);

			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_DLMS:
			/* Fatal : incorrect remain raw */
			set_confirm_error_aare(0x01, 4);
			set_confirm_service_error(&cse01, &cse06, &cse01);

			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_CONF:
			/* Fatal : incorrect remain raw */
			set_confirm_error_aare(0x01, 4);
			set_confirm_service_error(&cse01, &cse06, &cse02);
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_PDU:
			/* Fatal : incorrect remain raw */
			set_confirm_error_aare(0x01, 4);
			set_confirm_service_error(&cse01, &cse06, &cse03);
			tmp = offset + 1;
			encode_len(g_cosem_res.buf, &tmp, g_cosem_res.pos - offset - 2);
			break;

		case ERROR_ACSE_INVALID_PRIORITY:
			/* Fatal : incorrect remain raw */
			set_confirm_service_error(&cse01, &cse00, &cse03);
			break;

		case ERROR_HDLC_LOCK:
			set_confirm_service_error(&cse01, &cse06, &cse00);
			break;
		case ERROR_HARDWARE_FAULT:
			set_confirm_service_error(&cse01, &cse05, &cse03);
			break;
			/* Fatal : dose not support in ConfirmServiceError */
		default:
			record_log(ETSN2, 7);
		case ERROR_HDLC_INVALID_ORDER:
			set_confirm_service_error(&cse01, &cse05, &cse00);
			break;
		};
		if (security_policyQ() == SECURITY_POLICY_AE) {
			if (challenge_client()) {
				/* Info : abort challenge */
				set_non_associated(3);	/* チャレンジクライアント(AARE済),ReplayToHLSに失敗なら解放 */
			}
		}
	}
	else
	{
		switch (err_id)
		{
		case ERROR_HDLC_ASSOCIATION_INCOMPLETE:
		case ERROR_XDLMS_ASSOCIATION_INCOMPLETE:
			set_confirm_service_error(&cse01, &cse03, &cse02);
			break;

		case ERROR_SECURITY_INVALID_LEN_SC:
		case ERROR_SECURITY_INVALID_SC:
		case ERROR_SECURITY_INVALID_AUTH:
		case ERROR_ACSE_INVALID_INITIATE:
			if ((order == CMD_GLO_GET_REQUEST + CONV_REQ_RES_LN) ||
				(order == CMD_DED_GET_REQUEST + CONV_REQ_RES_LN))
			{
				set_confirm_service_error(&cse05, &cse00, &cse06);
			}
			else
			{
				set_confirm_service_error(&cse06, &cse00, &cse06);
			}
			break;

		case ERROR_HDLC_LOCK:
			set_confirm_service_error(&cse01, &cse06, &cse00);
			break;
		case ERROR_HARDWARE_FAULT:
			set_confirm_service_error(&cse01, &cse05, &cse03);
			break;
			/* Fatal : dose not support in ConfirmServiceError */
		default:
			record_log(ETSN2, 7);
		case ERROR_HDLC_INVALID_ORDER:
			set_confirm_service_error(&cse01, &cse05, &cse00);
			break;
		};
		if (challenge_client()) {
			/* Info : abort challenge */
			set_non_associated(3);	/* チャレンジクライアント(AARE済),ReplayToHLSに失敗なら解放 */
		}
		else
		{
			/* カウントアップしない指令 */
		}
	}
	g_cosem_res.len = g_cosem_res.pos;
	install_dedicated_key();
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
cui16 ist_idx[] = {
	IDX_CLASS_DATA,
	IDX_CLASS_REGISTER,
	IDX_CLASS_REGISTER_EX,
	IDX_CLASS_PROFILE_GENERIC,
	IDX_CLASS_CLOCK,
	IDX_CLASS_ASSOCIATION_LN,
	IDX_CLASS_HDLC_SETUP,
	IST_END,
};
#define LST_CLASS_DATA				0
#define LST_CLASS_REGISTER			1
#define LST_CLASS_REGISTER_EX		2
#define LST_CLASS_PROFILE_GENERIC	3
#define LST_CLASS_CLOCK				4
#define LST_CLASS_ASSOCIATION_LN	5
#define LST_CLASS_HDLC_SETUP		6
#define LST_CLASS_SECURITY_SETUP	7

ui08 set_class_id_to_idx(void)
{
	ui16	cid;
	ui08	pos;

	class_from = 0xffff;
	class_to = 0xffff;
	pos = DLMS_INVALID;
	cid = UI16L(class_id);

	switch (cid)
	{
	case 0x0001:
		pos = LST_CLASS_DATA;
		selectiveAccess = 1;
		break;
	case 0x0003:
		pos = LST_CLASS_REGISTER;
		selectiveAccess = 0;
		break;
	case 0x0004:
		pos = LST_CLASS_REGISTER_EX;
		selectiveAccess = 0;
		break;
	case 0x0007:
		pos = LST_CLASS_PROFILE_GENERIC;
		selectiveAccess = 2;
		break;
	case 0x0008:
	case 0x0009:
	case 0x000B:
		pos = LST_CLASS_CLOCK;
		selectiveAccess = 0;
		break;
	case 0x000f:
	case 0x0012:
	case 0x0013:
	case 0x0014:
	case 0x0016:
		pos = LST_CLASS_ASSOCIATION_LN;
		selectiveAccess = 4;
		break;
	case 0x0017:
	case 0x0028:
	case 0x0029:
	case 0x002d:
	case 0x0030:
	case 0x0040:
	case 0x0046:
	case 0x0047:
		pos = LST_CLASS_HDLC_SETUP;
		selectiveAccess = 0;
		break;
	default:
		pos = DLMS_INVALID;
		break;
	};
	if (!(DLMS_INVALID == pos))
	{
		class_from = ist_idx[pos];
		class_to = ist_idx[pos + 1];
		return TRUE;
	}
	return FALSE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 phase_execute(void)
{
	if ((g_xdlms_req.pos + 9) > g_xdlms_req.len)
	{
		/* Error : parameter is not enought */
		result_xdlms = RET_TYPE_UNMATCHED;
		return FALSE;
	}
	/* Class ID セット */
	ramcpy(class_id, &g_xdlms_req.buf[g_xdlms_req.pos], 2);
	g_xdlms_req.pos += 2;
	if (FALSE == set_class_id_to_idx())
	{
		/* Error : incorrect class Id */
		result_xdlms = RET_OBJECT_CLASS_INCONSISTENT;
		return FALSE;
	}
	/* OBISコードセット */
	ramcpy(obis_code, &g_xdlms_req.buf[g_xdlms_req.pos], 6);
	g_xdlms_req.pos += 6;

	/* execute番号のセット */
	execute = g_xdlms_req.buf[g_xdlms_req.pos];
	g_xdlms_req.pos++;

	return TRUE;
}


/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 SetAttrSelective(void)	/* ke2p processing attribute */
{
	/* SelectiveAccessのセット */
	if ((g_xdlms_req.pos + 1) > g_xdlms_req.len)
	{
		/* Error : parameter is not enought */
		result_xdlms = RET_TYPE_UNMATCHED;
		return FALSE;
	}
	if (!(0 == g_xdlms_req.buf[g_xdlms_req.pos]))
	{
		g_xdlms_req.pos++;
		if ((g_xdlms_req.pos + 1) > g_xdlms_req.len)
		{
			/* Error : parameter is not enought */
			result_xdlms = RET_TYPE_UNMATCHED;
			return FALSE;
		}
		if (selectiveAccess < g_xdlms_req.buf[g_xdlms_req.pos])
		{
			/* Error : invalid selective Access no */
			result_xdlms = RET_TEMPORARY_FAILURE;
			return FALSE;
		}
		if (0 == g_xdlms_req.buf[g_xdlms_req.pos])
		{
			/* Error : selective Access zero */
			result_xdlms = RET_TEMPORARY_FAILURE;
			return FALSE;
		}
		selectiveAccess = g_xdlms_req.buf[g_xdlms_req.pos];
	}
	else
	{
		selectiveAccess = 0;
	}
	g_xdlms_req.pos++;
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
ui08 SetMethSelective(void)	/* ke2p processing attribute */
{
	/* SelectiveAccessのセット */
	selectiveAccess = 0;
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void Execute(cui08 sel, cui16 ist, cui08 atr)
{
	POBJ16 obj;
	ui08 idx;

	idxobj = ist;
	obj = (POBJ16)(as_ln0[ist] + POS_OBJ[atr]);
	if (0 == ZERO16(obj->ctl))
	{
		/* Fatal : no method presented */
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
	if (AUTH16(obj->ctl))
	{
		/* Error : incorrect authentication */
		result_xdlms = RET_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	/* Set map_order of "Get/Set Function" */
	idx = F16(obj->ctl, sel);
	if (0 < idx)
	{
		((FNC)((PADR)obj->mem)[idx - 1])();		/*実行処理*/
	}
	else
	{
		/* Error : incorrect function index */
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
}
#if 0
void Execute0(cui08 sel, cui16 ist, cui08 atr)
{
	POBJ16 obj;
	ui08 idx;

	obj = (POBJ16)(as_ln2[ist] + POS_OBJ[atr]);
	idx = F16(obj->ctl, sel);
	if (0 < idx) {
		((FNC)((PADR)obj->mem)[idx - 1])();		/*実行処理*/
	}
}
#endif
/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
void Access_Attribute(cui16 i)
{
	ui16 obis_ctl;
	ui08 exe, sel, oid[6];

	sel = map_order[selectiveAccess];
	if (0 == execute)	/* attribute Zero Access */
	{
		/* Error : attribute 0 Access */
		result_xdlms = RET_READ_WRITE_DENIED;
		return;
	}
	if (1 == execute)	/* logical name */
	{
		if (authenticated_client())
		{
			obis_ctl = CTL16(f16_r0a, f16_n, f16_n, f16_n, , , );
		}
		else
		{
			obis_ctl = CTL16(f16_r0, f16_n, f16_n, f16_n, , , );
		}
		if (AUTH16(obis_ctl))
		{
			/* Error : incorrect authentication */
			result_xdlms = RET_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}
		if (G0 == sel)
		{
			romcpy(oid, g_ist[i].oid, 5);
			oid[5] = 0xff;
			put_attr(oid, 0,(ADDR)&ID_OCTET_STRING6);
		}
		else
		{
			/* Error : cannot Access(By set) and dose not support */
			result_xdlms = RET_READ_WRITE_DENIED;
		}
		return;
	}
	exe = execute - 2;
	if ((exe + 1) > g_cls[g_ist[i].cls].cnt_a)
	{
		/* Error : invalid attribute id */
		result_xdlms = RET_OBJECT_CLASS_INCONSISTENT;
		return;
	}
	Execute(sel, i, exe);
	return;
}

void Access_Method(cui16 i)
{
	ui08	exe;

	if (0 == execute)
	{
		/* Error : method 0 Access */
		result_xdlms = RET_OBJECT_CLASS_INCONSISTENT;
		return;
	}
	exe = execute - 1;
	if ((exe + 1) > g_cls[g_ist[i].cls].cnt_m)
	{
		/* Error : invalid method id */
		result_xdlms = RET_OBJECT_CLASS_INCONSISTENT;
		return;
	}
	exe += g_cls[g_ist[i].cls].cnt_a;
	Execute(AT, i, exe);
	return;
}

/****************************************************************/
/*																*/
/*	関数名				:	Exe									*/
/*																*/
/*	内容				:	指定OBISコードの関数を実行する		*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴													*/
/*			2014-10-14 T.Kuroda 検定モード有効無効情報削除		*/
/*			2015-03-04 T.Kuroda	改定04対応						*/
/*																*/
/****************************************************************/
void Exe(void)
{
	ui16	i;
	ui08	idx, go;
	ACC1	acc1;
	ACC4	acc4i;
	POCTL	octl;

	go = 0;
	for (i = class_from; i < class_to; i++) {
		if (0 == romcmp(obis_code, g_ist[i].oid,  5)) {
			if (g_user0 == 0) {
				go = 1;	/*super user*/
			}
			else {
				octl = (POCTL)&OBJMAP[i];
				idx = octl->bt.idx;
				if (octl->bt.four > 0) {
					acc4i.wd = USERS[g_user0].acc.acc4[idx];
					acc4i.wd &= USERS[0].acc.acc4[idx];
					switch (order_xdlms)
					{
					case CMD_GET_REQUEST:
						if (acc4i.bt.get > 0) {
							go = 1;
						}
						else {
							result_xdlms = RET_READ_WRITE_DENIED;
						}
						break;
					case CMD_SET_REQUEST:
						if (acc4i.bt.set > 0) {
							go = 1;
						}
						else {
							result_xdlms = RET_READ_WRITE_DENIED;
						}
						break;

					case CMD_ACT_REQUEST:
					case CMD_NOTIFICATION_CALL:
					case CMD_VOID_CALL:
						if (acc4i.bt.act > 0) {
							go = 1;
						}
						else {
							result_xdlms = RET_READ_WRITE_DENIED;
						}
						break;
					};
#if 0
					if (go > 0) {
						if (obis_code[5] != 0xff) {
							if (obis_code[5] > 29) {
								wk32 = 1;
								for (shift = 30; shift < obis_code[5]; shift++) {
									wk32 <<= 1;
								}
								acc1.by = ((pui08)&USERS[g_user0].acc.acc4[5])[idx];
								acc1.by &= ((pui08)&USERS[0].acc.acc4[5])[idx];
								if ((acc1.bt.data&wk32) > 0) {
									go = 1;
								}
								else {
									go = 0;
									result_xdlms = RET_READ_WRITE_DENIED;
								}
							}
							else {
								wk32 = 1;
								for (shift = 0; shift < obis_code[5]; shift++) {
									wk32 <<= 1;
								}
								if ((acc4i.bt.data&wk32) > 0) {
									go = 1;
								}
								else {
									go = 0;
									result_xdlms = RET_READ_WRITE_DENIED;
								}
							}
						}
						else {
							if (acc4i.bt.data == 0x1fffffff) {
								go = 1;
							}
							else {
								go = 0;
								result_xdlms = RET_READ_WRITE_DENIED;
							}
						}
					}
#endif
				}
				else {
					acc1.by = USERS[g_user0].acc.acc1[idx];
					acc1.by &= USERS[0].acc.acc1[idx];
					switch (order_xdlms)
					{
					case CMD_GET_REQUEST:
						if (acc1.bt.get > 0) {
							go = 1;
						}
						else {
							result_xdlms = RET_READ_WRITE_DENIED;
						}
						break;
					case CMD_SET_REQUEST:
						if (acc1.bt.set > 0) {
							go = 1;
						}
						else {
							result_xdlms = RET_READ_WRITE_DENIED;
						}
						break;
					case CMD_ACT_REQUEST:
					case CMD_NOTIFICATION_CALL:
					case CMD_VOID_CALL:
						if (acc1.bt.act > 0) {
							go = 1;
						}
						else {
							result_xdlms = RET_READ_WRITE_DENIED;
						}
						break;
					};
#if 0
					if (go > 0) {
						if (obis_code[5] != 0xff) {
							wk32 = 1;
							for (shift = 0; shift < obis_code[5]; shift++) {
								wk32 <<= 1;
							}
							if ((acc1.bt.data&wk32) > 0) {
								go = 1;
							}
							else {
								go = 0;
								result_xdlms = RET_READ_WRITE_DENIED;
							}
						}
						else {
							if (acc1.bt.data == 0x1f) {
								go = 1;
							}
							else {
								go = 0;
								result_xdlms = RET_READ_WRITE_DENIED;
							}
						}
					}
#endif
				}
			}
			if (go > 0) {
				result_xdlms = RET_SUCCESS;
				if (NULL == as_ln0[i])
				{
					/* Error : cannot Access( No Access ) */
					result_xdlms = RET_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				Access(i);
			}
			return;
		}
	}
	/* Error : unknown obis code */
	result_xdlms = RET_OBJECT_UNDEFINED;
}

/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
static void req_normal(void)
{
	result_xdlms = RET_SUCCESS;
	block_no = 0;
	last_block = TRUE;
	heapinit();

	/* Class Id, OBIS code */
	if (FALSE == phase_execute())
	{
		/* Fatal : incorrect remain raw */
		return;
	}
	/* COSEM Selective Descriptorの保存 */
	if (FALSE == phase_selective()) 
	{
		/* Fatal : incorrect remain raw */
		return;
	}
}

static void res_normal(void)
{
	/* モードを保存 */
	g_xdlms_res[0] = res_mode;
	if (RET_SUCCESS == result_xdlms) {
		/* 成功した場合は、0を保存 */
		g_xdlms_res[g_xdlms_res_len] = 0x00;
		g_xdlms_res_len++;
		if (
			(CMD_ACT_REQUEST == order_xdlms) ||
			(CMD_NOTIFICATION_CALL == order_xdlms) ||
			(CMD_VOID_CALL == order_xdlms))
		{
			if (0 == g_raw_len) {
				g_xdlms_res[g_xdlms_res_len] = 0x00;	/* return-parameters */
				g_xdlms_res_len++;
			}
		}
	}
	else	/* error */
	{
		if (CMD_GET_REQUEST == order_xdlms) {
			g_xdlms_res[g_xdlms_res_len] = 1;
			g_xdlms_res_len++;	/* Get-Data-Result [0] Data */
			/* エラーの場合は、1を保存して、結果をRAWに保存 */
			g_raw[0] = result_xdlms;
			g_raw_len = 1;
		}
		else {
			g_xdlms_res[g_xdlms_res_len] = result_xdlms;
			g_xdlms_res_len++;
			if (
				(CMD_ACT_REQUEST == order_xdlms) ||
				(CMD_NOTIFICATION_CALL == order_xdlms) ||
				(CMD_VOID_CALL == order_xdlms))
			{
				g_raw[0] = 0x00;	/* return-parameters */
				g_raw_len = 1;
			}
		}
	}
}


/****************************************************************/
/*																*/
/*	関数名				:										*/
/*																*/
/*	内容				:										*/
/*																*/
/*																*/
/*	引　数				:なし									*/
/*																*/
/*	戻り値				:なし									*/
/*																*/
/*	作成日				: 2013-04-05	担当: T.Kuroda			*/
/*																*/
/*	変更履歴			: 201x-xx-xx  	担当: 					*/
/*																*/
/****************************************************************/
static void req_blockG(void)
{
	ui32	no;

	result_xdlms = RET_SUCCESS;
	if ((g_xdlms_req.pos + 4) > g_xdlms_req.len) {
		/* Error : parameter is not enought */
		result_xdlms = RET_TYPE_UNMATCHED;
		block_no = 0;
		return;
	}
	no = UI32L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	if (0 == block_no) {
		/* Error : invalid block Request */
		result_xdlms = RET_NO_LONG_GET_IN_PROGRESS;
		block_no = no;
		return;
	}
	if (!(block_no == no)) {
		/* Error : invalid block NO request */
		result_xdlms = RET_LONG_GET_ABORTED;
		block_no = no;
		return;
	}
	g_xdlms_req.pos += 4;
}

static void res_blockG(void)
{
	/* モードを保存 */
	g_xdlms_res[0] = res_mode;
	block_no++; 	/* ブロック応答であればインクリメント */
	/* ブロック応答、通常応答用ヘッダの保存 */
	if (RET_SUCCESS == result_xdlms) {
		/* ブロック応答用ヘッダの保存 */
		g_xdlms_res[g_xdlms_res_len] = last_block;
		g_xdlms_res_len++;/* last-block BOOLEAN */
		ENDCPY4(&g_xdlms_res[g_xdlms_res_len], block_no);
		g_xdlms_res_len += 4;
		g_xdlms_res[g_xdlms_res_len] = 0;
		g_xdlms_res_len++;	/* [0] IMPLICIT OCTET STRING */
		encode_len(g_xdlms_res, &g_xdlms_res_len, g_raw_len);
	}
	else {
		last_block = TRUE;
		/* ブロック応答用ヘッダの保存 */
		g_xdlms_res[g_xdlms_res_len] = last_block;
		g_xdlms_res_len++;/* last-block BOOLEAN */
		ENDCPY4(&g_xdlms_res[g_xdlms_res_len], block_no);
		g_xdlms_res_len += 4;
		g_xdlms_res[g_xdlms_res_len] = 1;
		g_xdlms_res_len++;	/* [1] Data Access Result [1] */
		g_raw[0] = result_xdlms;
		g_raw_len = 1;	/* result */
	}
}

static void req_first(void) {

	ui32	no, len;

	block_no = 1;
	result_xdlms = RET_SUCCESS;

	/* Class Id, OBIS code */
	if (FALSE == phase_execute()) {
		/* Fatal : incorrect remain raw */
		return;
	}
	/* COSEM Selective Descriptorの保存 */
	if (FALSE == phase_selective()) {
		/* Fatal : incorrect remain raw */
		return;
	}
	if ((g_xdlms_req.pos + 5) > g_xdlms_req.len) {
		/* Error : parameter is not enought */
		result_xdlms = RET_TYPE_UNMATCHED;
		block_no = 0;
		return;
	}
	last_block = g_xdlms_req.buf[g_xdlms_req.pos];
	g_xdlms_req.pos++;
	no = UI32L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	g_xdlms_req.pos += 4;
	if (!(block_no == no)) {
		/* Error : invalid block NO request */
		result_xdlms = RET_LONG_SET_ABORTED;
		block_no = no;
		return;
	}
	len = decode_len(&g_xdlms_req.pos, g_xdlms_req.buf);
	if ((g_xdlms_req.pos + len) > g_xdlms_req.len) {
		/* Error : parameter is not enought */
		result_xdlms = RET_TYPE_UNMATCHED;
		block_no = 0;
		return;
	}
}

static void req_blockSA(void) {

	ui32	no, len;

	block_no++;
	result_xdlms = RET_SUCCESS;

	last_block = g_xdlms_req.buf[g_xdlms_req.pos];
	g_xdlms_req.pos++;
	no = UI32L(&g_xdlms_req.buf[g_xdlms_req.pos]);
	g_xdlms_req.pos += 4;
	if (!(block_no == no)) {
		/* Error : invalid block NO request */
		result_xdlms = RET_NO_LONG_SET_IN_PROGRESS;
		block_no = no;
		return;
	}
	len = decode_len(&g_xdlms_req.pos, g_xdlms_req.buf);
	if ((g_xdlms_req.pos + len) > g_xdlms_req.len) {
		/* Error : parameter is not enought */
		result_xdlms = RET_TYPE_UNMATCHED;
		block_no = 0;
		return;
	}
}

static void res_blockSA(void) {

	if (RET_SUCCESS != result_xdlms) {
		last_block = TRUE;
	}
	if (last_block > 0) {
		g_xdlms_res[0] = 3;
		g_xdlms_res[g_xdlms_res_len] = result_xdlms;
		g_xdlms_res_len++;
	}
	else {
		g_xdlms_res[0] = 2;
	}
	ENDCPY4(&g_xdlms_res[g_xdlms_res_len], block_no);
	g_xdlms_res_len += 4;
}

const  FNC GET_REQ[] = {
	NULL,
	req_normal,
	req_blockG,
	NULL,
};
const  FNC SET_REQ[] = {
	NULL,
	req_normal,
	req_first,
	req_blockSA,
};

const  FNC ACT_REQ[] = {
	NULL,
	req_normal,
	NULL,
	NULL,
};

const  FNC GET_RES[] = {
	NULL,
	res_normal,
	res_blockG,
	NULL,
};

const  FNC SET_RES[] = {
	NULL,
	res_normal,
	res_blockSA,
	res_blockSA,
};

const  FNC ACT_RES[] = {
	NULL,
	res_normal,
	NULL,
	NULL,
};

#define GET_MODE	2
#define SET_MODE	3
#define ACT_MODE	1

/****************************************************************/
/*																*/
/*	関数名	:init_xDLMS											*/
/*																*/
/*	内容		:XDLMS初期化.									*/
/*																*/
/*	引　数	:なし												*/
/*																*/
/*	戻り値	:なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 												*/
/*			2015-03-04	T.Kuroda	改定04対応					*/
/*																*/
/****************************************************************/
void init_xDLMS(void)
{
	/* 接続確立情報の初期化 */
	init_association();
	clear_xdlms_req();
}

/****************************************************************/
/*																*/
/*	関数名	:void xDLMS( void )									*/
/*																*/
/*	内容	:xDLMSメイン										*/
/*																*/
/*																*/
/*	引　数	:なし												*/
/*																*/
/*	戻り値	:なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴													*/
/*		2014-04-11 T.Kuroda 開閉器の動作監視					*/
/*																*/
/* http://www.dlms.com/faqanswers/forum/index.php?action=view&topicid=50 */
/****************************************************************/

void xDLMS(void)
{
	ui08	mode;
	ui08	res;
	DAT4	tmp;

	if (0 < g_xdlms_req.len)
	{
		if (PHASE_INITIAL == xdlms_phase)
		{
			xdlms_phase = PHASE_RESPONSE;
			map_order = NULL;
			order_xdlms = g_xdlms_req.buf[g_xdlms_req.pos];
			g_xdlms_req.pos++;
			switch (order_xdlms)
			{
			case CMD_INITIATE_REQUEST:
				res = InitiateRequest();
				if (!(DLMS_SUCCESS == res))
				{
					ConfirmServiceError(res);	/* INITIATE_RESPONSEでセットした値を使用 */
					return;
				}
				break;
			case CMD_GET_REQUEST:
			case CMD_SET_REQUEST:
			case CMD_ACT_REQUEST:
			case CMD_NOTIFICATION_CALL:
			case CMD_VOID_CALL:
				if ((g_xdlms_req.pos + 2) > g_xdlms_req.len)	/* 2 = mode + invoke_id_and_priority */
				{
					/* Error : xdlms request is too short */
					ConfirmServiceError(ERROR_HDLC_INVALID_ORDER);
					return;
				}
				mode = g_xdlms_req.buf[g_xdlms_req.pos];	/* Get モード(normal/block/withlistの取得) */
				g_xdlms_req.pos++;

				if (CMD_GET_REQUEST == order_xdlms)
				{
					if (mode > GET_MODE)
					{
						mode = DLMS_INVALID;
					}
					else
					{
						Access = Access_Attribute;
						map_order = (pui08)FG;		/* Getインデックス */
						phase_selective = SetAttrSelective;
					}
				}
				if (CMD_SET_REQUEST == order_xdlms)
				{
					if (mode > SET_MODE)
					{
						mode = DLMS_INVALID;
					}
					else
					{
						Access = Access_Attribute;
						map_order = (pui08)FS;		/* Setインデックス */
						phase_selective = SetAttrSelective;
					}
				}
				if (
					(CMD_ACT_REQUEST == order_xdlms) ||
					(CMD_NOTIFICATION_CALL == order_xdlms) ||
					(CMD_VOID_CALL == order_xdlms) )
				{
					if (mode > ACT_MODE)
					{
						mode = DLMS_INVALID;
					}
					else
					{
						Access = Access_Method;
						phase_selective = SetMethSelective;
					}
				}
				if (!((0x00 == mode) || (DLMS_INVALID == mode))) {
					/* デフォルトの応答モードを設定 */
					res_mode = mode;
					if (CMD_GET_REQUEST == order_xdlms) {
						/* invoke_id_and_priorityの保存 */
						invoke_id_and_priority = g_xdlms_req.buf[g_xdlms_req.pos];
						g_xdlms_req.pos++;
						GET_REQ[mode]();
					}
					if (CMD_SET_REQUEST == order_xdlms) {
						/* invoke_id_and_priorityの保存 */
						invoke_id_and_priority = g_xdlms_req.buf[g_xdlms_req.pos];
						g_xdlms_req.pos++;
						SET_REQ[mode]();
					}
					if (CMD_ACT_REQUEST == order_xdlms)	{
						/* invoke_id_and_priorityの保存 */
						invoke_id_and_priority = g_xdlms_req.buf[g_xdlms_req.pos];
						g_xdlms_req.pos++;
						ACT_REQ[mode]();
					}
					if (CMD_VOID_CALL == order_xdlms) {
						/* invoke_id_and_priorityの保存 */
						g_xdlms_req.pos++;
						ACT_REQ[mode]();
					}
					if (CMD_NOTIFICATION_CALL == order_xdlms) {
						/* invoke_id_and_priorityの保存 */
						tmp.wd = liiap.wd;
						tmp.by[3] = 0;
						if (tmp.wd < 0x00ffffff) {
							tmp.wd++;
						}
						else {
							tmp.wd = 0;
						}
						liiap.by[0] = tmp.by[0];
						liiap.by[1] = tmp.by[1];
						liiap.by[2] = tmp.by[2];
						g_xdlms_req.pos++;
						ACT_REQ[mode]();
					}
					if (result_xdlms == RET_SUCCESS) {
						Exe();
					}
				}
				else {
					/* Error : invalid mode */
					result_xdlms = RET_TYPE_UNMATCHED;
					res_mode = ORDER_NORMAL;
					/* invoke_id_and_priority処理 */
					invoke_id_and_priority = 0x41;
				}
				break;
			default:
				/* Fatal : invalid xdlms command */
				record_log(ETSN2, 7);
				result_xdlms = RET_OTHER_REASON;
				res_mode = g_xdlms_req.buf[g_xdlms_req.pos];	/* Get モード(normal/block/withlistの取得) */
				g_xdlms_req.pos++;
				/* invoke_id_and_priority処理 */
				invoke_id_and_priority = 0x41;
				break;
			};
		}
		else if (PHASE_THREAD == xdlms_phase) {
			xdlms_phase = thread_process();
		}
		else if (PHASE_RESPONSE == xdlms_phase)	{/* 応答処理 */
			switch(order_xdlms){
				case CMD_INITIATE_REQUEST:
					break;
				case CMD_NOTIFICATION_CALL:
					g_xdlms_res[g_xdlms_res_len] = liiap.by[3];
					g_xdlms_res_len++;
					g_xdlms_res[g_xdlms_res_len] = liiap.by[2];
					g_xdlms_res_len++;
					g_xdlms_res[g_xdlms_res_len] = liiap.by[1];
					g_xdlms_res_len++;
					g_xdlms_res[g_xdlms_res_len] = liiap.by[0];
					g_xdlms_res_len++;
					g_xdlms_res[g_xdlms_res_len] = 0x0c;
					g_xdlms_res_len++;
					ramcpy(&g_xdlms_res[g_xdlms_res_len], get_meter_datetime(), 0x0c);
					g_xdlms_res_len+=12;
					if (!(FALSE == last_block)) {
						block_no = 0;	/* end of block transfer and reject for block transfer request */
					}
					break;
				case CMD_VOID_CALL:
					g_cosem_res.len = g_cosem_res.pos;
					break;
				default:
					g_xdlms_res_len++;										/* 応答用のモード領域はスキップ */
					g_xdlms_res[g_xdlms_res_len] = invoke_id_and_priority;	/* invoke_id_and_priority処理 */
					g_xdlms_res_len++;
					if (CMD_GET_REQUEST == order_xdlms) {
						GET_RES[res_mode]();
					}
					if (CMD_SET_REQUEST == order_xdlms) {
						SET_RES[res_mode]();
					}
					if (CMD_ACT_REQUEST == order_xdlms) {
						ACT_RES[res_mode]();
					}
					if (!(FALSE == last_block)) {
						block_no = 0;	/* end of block transfer and reject for block transfer request */
					}
					break;
			}
			if (g_authQ == TRUE) {
				/* XDLMS用ヘッダ、暗号応答用ヘッダ、RAWデータの結合 */
				g_gcm_res.len = g_xdlms_res_len + g_raw_len;		/* SECURITY */
				g_gcm_res.pos = 0;
				g_gcm_res.buf = &g_cosem_res.buf[g_cosem_res.pos];
				if (!(g_xdlms_req.pos == g_xdlms_req.len)) 
				{
					if (RET_SUCCESS == result_xdlms)
					{
//						record_log(ETSN1, 22);
						/* Error : length of request is not match */
						/* result_xdlms = RET_OTHER_REASON; */
					}
				}
			}
			else {
				/* XDLMS用ヘッダ、RAWデータの結合 */
				g_cosem_res.len = g_xdlms_res_len + g_raw_len;		/* tsushin */
			}
			clear_xdlms_req();
		}
		else
		{
			record_log(ETSN2, 7);
			ConfirmServiceError(ERROR_HDLC_INVALID_ORDER);
		}
	}
}

void push_associate(void) {

	/* info : ven client */
	ui08 data[8];

	g_AA = TRUE;
	g_port0 = g_portQ;
	g_user0 = g_userQ;
	setCounter(g_user0);
	apdu_size = MAX_XDLMS_RAW;
	apdu_size -= 24;
	apdu_size -= 7;
	read_othinf(data, eCID2, 1);
	if ((data[0] != 0x00) && (data[1] != 0x00)) {
		if (UI16L(&data[0]) < apdu_size) {
			apdu_size = UI16L(&data[0]);
		}
	}
	as_ln0 = (PADR)&as_ln2[0];
	//	ramcpy(I(15, 5, asso_ln), xdlms_info, LEN_XDLMS_CTX_INF);
	romcpy(I(64, 4, security), push_client, LEN_SYSTEM_TITLE);
	I(15, 3, asso_ln)[0] = g_addrQ >> 1;
	I(15, 3, asso_ln)[1] = 0x01;
	I(15, 3, asso_ln)[2] = 0x00;
	I(15, 3, asso_ln0) = (pui08)I(15, 3, asso_ln);			/* associated_partners_id */
/*		I(15,4,asso_ln0)	= (pui08)I(15,4,asso_ln2);*/		/* application_context_name */

	I(15, 5, asso_ln)[0] = CONFORMANCE[4][1];
	I(15, 5, asso_ln)[1] = CONFORMANCE[4][2];
	I(15, 5, asso_ln)[2] = CONFORMANCE[4][3];
	I(15, 5, asso_ln)[3] = 0xe2;
	I(15, 5, asso_ln)[4] = 0x02;
	I(15, 5, asso_ln)[5] = 0xe2;
	I(15, 5, asso_ln)[6] = 0x02;
	I(15, 5, asso_ln)[7] = 0x06;
	I(15, 5, asso_ln)[8] = 0x00;
	I(15, 5, asso_ln)[9] = 0x00;
	I(15, 5, asso_ln0) = I(15, 5, asso_ln);					/* xDLMS_context_info */
	I(15, 8, asso_ln) = ASSOCIATED;							/* 認証確立 */
	I(15, 8, asso_ln0) = &I(15, 8, asso_ln);				/* association status */
	I(15, 9, asso_ln0) = (pui16)&I(15, 9, asso_ln4);	/* security_setup_reference */
	I(15, 6, asso_ln4) = I(15, 6, asso_ln0);			/* authentication_mechanism_name */
	asso_ln03 = G(0, 15, 3, asso_ln4);
	asso_ln04 = G(0, 15, 4, asso_ln4);
	asso_ln05 = G(0, 15, 5, asso_ln4);
	asso_ln06 = G(0, 15, 6, asso_ln4);
	asso_ln07 = S(0, 15, 7, asso_ln4);
	asso_ln08 = G(0, 15, 8, asso_ln4);
	asso_ln09 = G(0, 15, 9, asso_ln4);
	record_log(LOG_PUSH, get_nic_status());
}
