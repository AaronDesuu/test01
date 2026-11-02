/****************************************************************************/
/*                                                                          */
/*  システム名      :   DLMS                                                */
/*                                                                          */
/*  サブシステム名  :   tsushin                                                */
/*                                                                          */
/*  内容            :   tsushinプロトコル処理                                  */
/*                                                                          */
/*  ファイル名      :   hdlc.c                                              */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include "def.h"
#include "data.h"
#include "ram.h"
#include "sub.h"
#pragma	interrupt	int_rcvR (vect = INTSR0)
#pragma	interrupt	int_sndR (vect = INTST0)
#pragma	interrupt	int_errR (vect = INTSRE0)

#pragma	interrupt	int_rcvB (vect = INTSR1)
#pragma	interrupt	int_sndB (vect = INTST1)
#pragma	interrupt	int_errB (vect = INTSRE1)

#pragma	interrupt	int_rcvL (vect = INTSR2)
#pragma	interrupt	int_sndL (vect = INTST2)
#pragma	interrupt	int_errL (vect = INTSRE2)

#include "devttl.h"
#include "devtmr.h"
#include "devintr.h"
#include "devport.h"
#include "aplkeikiinf.h"
#include "apleeptask.h"
#include "aplrecord.h"
#include "aplstsctl.h"
#include "aplcommand.h"
#include "aplevent.h"

#include "hrdtushin.h"
#include "dlms.h"
#include "conf.h"
#include "acse.h"
#include "aes_gcm.h"
#include "xdlms.h"
#include "flash.h"

/*--------------------------------------------------------------------------*/
#define	MINIMAM_BUF		0x0020
#define	MINIMAM_WIN		0x0001
#define HDLC_FRAE_TYPE	0xa0
#define MASK_HDLC_HED	0xf800
#define MASK_HDLC_LEN	0x07ff
#define STATE_WAIT	0		/*処理待ち*/
#define STATE_HDLC	1		/*hdlc処理中*/
#define STATE_COSEM	2		/*COSEM処理中*/
#define	OK			0x00	/* 正常 		*/
#define	NG			0x01	/* 異常 		*/
#define	END_SR		0xFE
#define	END_RJ		0xFd
#define	ER_VER		0x04
#define	ER_LLC		0x05
#define	ER_STX		0x07					/* STXなし異常				*/
#define ER_ETX		0x08					/* ETXなし異常				*/
#define	ER_BCC		0x09					/* BCC異常					*/
#define	ER_MOD		0x0a					/* 無効ﾓｰﾄﾞ異常				*/
#define	ER_ADR		0x0b					/* 無効ﾓｰﾄﾞ異常				*/
#define	ER_LEN		0x0c					/* 電文長異常				*/
#define	ER_ROVR		0x0d					/* 受信ｷｬﾗｸﾀ間隔時間異常	*/
#define	ER_SOVR		0x0e					/* 送信ｷｬﾗｸﾀ間隔時間異常	*/
#define	ER_VOID		0x0f					/* 異常無応答				*/
#define	ER_OVE		0x12					/* ｵｰﾊﾞｰﾗﾝ異常				*/
#define	ER_FEM		0x13					/* ﾌﾚｰﾐﾝｸﾞ異常				*/
#define	ER_PAE		0x14					/* ﾊﾟﾘﾃｨ異常				*/
#define	ER_TEI		0x15					/* 電文受信時間異常			*/
//#define		ER_HATE		0x15					/* その他ﾊｰﾄﾞ異常			*/
#define	ER_RFRM		0x41					/* 電文受信時間異常			*/
//#define		ER_SFRM		0x44					/* 送信時間異常				*/
//#define		ER_MKT		0x45					/* 親局ﾏｰｸ時間異常			*/
//#define		ER_IKT		0x46					/* ｶﾚﾝﾀﾞｰ設定異常			*/
//#define		ER_STEI		0x47					/* 送信中停電検出異常		*/
//#define		ER_DUMY		0xFF					/* ﾀﾞﾐｰ異常					*/

#define	LOCAL_FLG	0x24						/* Optical通信				*/
#define	WRAP_FLG	0x00
#define	HDLC_FLG	0x7E					/* DLMS ｽﾀｰﾄﾌﾗｸﾞ				*/

#define	MARK_TM1	1		/*  20.0ms*/	/* ﾏｰｸ確定ﾀｲﾏ				*/
#define	MARK_TM2	1		/*  20.0ms*/	/* ﾏｰｸ確定ﾀｲﾏ				*/
#define	RFRAM_TMA	500		/*  1000ms*/	/* 	*/
#define	RFRAM_TMB	1000	/*  2000ms*/	/* 	*/
#define	SWAIT_TM3	6		/*  12.0ms*/	/* 送信開始待ちﾀｲﾏ			*/

#define INDEX_INVALID	0x00
#define INDEX_SNRM		(INDEX_INVALID+1)
#define INDEX_DISC		(INDEX_SNRM+1)
#define INDEX_UA		(INDEX_DISC+1)
#define INDEX_UAD		(INDEX_UA+1)
#define INDEX_BUSY		(INDEX_UAD+1)
#define INDEX_DM		(INDEX_BUSY+1)
#define INDEX_RR		(INDEX_DM+1)
#define INDEX_RNR		(INDEX_RR+1)
#define INDEX_I			(INDEX_RNR+1)
#define INDEX_UI		(INDEX_I+1)
#define INDEX_FRMRw		(INDEX_UI+1)
#define INDEX_FRMRx		(INDEX_FRMRw+1)
#define INDEX_FRMRy		(INDEX_FRMRx+1)
#define INDEX_FRMRz		(INDEX_FRMRy+1)
#define INDEX_FRMRa		(INDEX_FRMRz+1)
#define INDEX_FRMRb		(INDEX_FRMRa+1)
#define INDEX_FRMRc		(INDEX_FRMRb+1)
#define INDEX_FRMRd		(INDEX_FRMRc+1)
#define INDEX_FRMR0		(INDEX_FRMRd+1)
#define INDEX_FE		(INDEX_FRMR0+1)
#define INDEX_END		INDEX_FE		/* 0x06 */
#define INDEX_WAIT		0x80

#define HDLC_SNRM		0x83U
#define HDLC_DISC		0x43U
#define HDLC_UA			0x63U
#define HDLC_DM			0x0fU
#define HDLC_FRMR		0x87U
#define HDLC_I			0x00U
#define HDLC_RR			0x01U
#define HDLC_RNR		0x05U
#define HDLC_UI			0x03U
#define HDLC_FE			0xfeU

#define HDLC_PF			0x10U
#define HDLC_I_MSK		0x01U
#define HDLC_RR_MSK		0x0fU
#define HDLC_NRM_MSK	0xefU

#define FRAME_I(x)		(HDLC_I==((x)&HDLC_I_MSK))
#define FRAME_RR(x)		(HDLC_RR==((x)&HDLC_RR_MSK))
#define FRAME_RNR(x)	(HDLC_RNR==((x)&HDLC_RR_MSK))
#define FRAME_SNRM(x)	(HDLC_SNRM==((x)&HDLC_NRM_MSK))
#define FRAME_UA(x)		(HDLC_UA==((x)&HDLC_NRM_MSK))
#define FRAME_DISC(x)	(HDLC_DISC==((x)&HDLC_NRM_MSK))
#define FRAME_UI(x)		(HDLC_UI==((x)&HDLC_NRM_MSK))

#define HDLC_NR(x)		(((x)&0xe0)>>5)
#define HDLC_NS(x)		(((x)&0x0e)>>1)

#define FRAME_FE(x)		(HDLC_FE==(x))

#define MODE_WRAP 0
#define MODE_LOCAL 1
#define MODE_BASC 2
#define MODE_DLMS 3
#define	RCVMAX		128					/* 最大受信ﾊﾞｲﾄ数			*/
#define	SNDMAX		128					/* 最大受信ﾊﾞｲﾄ数			*/
typedef ui08(*RCV)(SEQ* seq, cui08 in);
typedef ui08(*SND)(pui08 out, SEQ* seq);

static ui08	res_buf1[MAX_SIZE_BUF];
static ui08	res_buf2[MAX_SIZE_BUF];				/*hdlcフレーム分(9)+予備(7)*/
static ui08	res_buf3[MAX_SIZE_BUF];				/*hdlcフレーム分(9)+予備(7)*/
static ui08	req_buf[MAX_SIZE_BUF];
PIPE	pipe1;					/*回路1第2レイヤー*/
PIPE	pipe2;
PIPE	pipe3;
SEQ		seq1;					/*回路1第1レイヤー*/
SEQ		seq2;
SEQ		seq3;
static uint	mid_req_len;			/*第3レイヤー要求電文長*/
static uint	mid_res_len;			/*第3レイヤー応答電文長*/
static GBT gbt;
static uint gbt_len;
uint max_res_len;
static PIPE* ppip;
static ui08	local_calling[32];
static cui08 NEGO_DEF[] = {
	//	81     80   12   05   01        80   06   01        80   07   04   00   00   00   01   08   04   00   00   00   01
		0x81,0x80,0x14,0x05,0x02,0x03,0x20,0x06,0x02,0x03,0x20,0x07,0x04,0x00,0x00,0x00,0x01,0x08,0x04,0x00,0x00,0x00,0x01 };

static cui08 LLC[] = { 0xe6, 0xe6, 0x00 };
static const ui08 def_ipv6[] = { 0x09, 0x07,'0','.', '0','.', '0','.', '0' };
static const ui08 def_imei[] = { 0x09, 0x0f,'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0' };
static const ui08 def_iccid[] = { 0x09, 0x04,'n','u','l','l' };
static const ui08 def_mac[] = { 0x09, 0x11,'0','0',':','0','0',':','0','0',':','0','0',':','0','0',':','0','0' };

static void invRcvSnd(cui08 mode, PIPE* pipe, SEQ* seq) {

	pipe->inv = 1;
	if ((mode % 2) == 0) {				/*seq->res.req要求*/
		seq->inf = 0;					/*req第1レイヤ長初期化*/
		seq->pos = 0;					/*req第1レイヤ位置初期化*/
		if (mode == 0) {
			seq->req.buf = pipe->req.buf;	/*req第1レイヤポインタ初期化*/
			seq->req.posW = 0;				/*req第1レイヤwindow位置初期化*/
			pipe->req.len = 0;			/*req第2レイヤ長初期化*/
			pipe->req.pos = 0;			/*req第2レイヤ位置初期化*/
		}
		pipe->ctl = 0;					/*req状態*/
		seq->phs = 0xfe;
	}
	else {								/*seq->req.res要求*/
		seq->tmr = RFRAM_TMB;			/*上位側無応答*/
		seq->inf = 0;					/*res第1レイヤ長初期化*/
		seq->pos = 0;					/*res第1レイヤ位置初期化*/
		if (mode == 1) {
			seq->res.buf = pipe->res.buf;	/*res第1レイヤポインタ初期化*/
			seq->res.posW = 0;				/*res第1レイヤwindow位置初期化*/
			pipe->res.len = 0;			/*res第2レイヤ長初期化*/
			pipe->res.pos = 0;			/*res第2レイヤ位置初期化*/
		}
		pipe->ctl = 1;					/*res状態*/
	}
	pipe->tsk = 0;
	pipe->inv = 0;
}

void cosem_res_clear(void) {
	g_cosem_res.buf = NULL;
	g_cosem_res.pos = 0;
	g_cosem_res.len = 0;
}

static void req_layer1(PPIP in, SEQ *seq) {
	ui08	cipher, order, i;
	ui08	offset;

	cipher = FALSE;
	order_cipher = 0xff;
	g_xdlms_res_len = 0;
	g_raw_len = 0;
	g_portQ = in->prt;
	g_addrQ = seq->req.adr2.by[0];	/*Client address*/
	g_userQ = DLMS_INVALID;
	for (i = 0; i < sizeof(USERS)/sizeof(USER); i++) {
		if (USERS[i].sys.maddr == g_addrQ) {
			g_userQ = i;
			break;
		}
	}
	if (g_userQ != DLMS_INVALID) {
		in->sta = STATE_COSEM;
		g_cosem_res.buf = in->res.buf;
		if (0 == ramcmp(LLC, in->req.buf, 3)) {
			g_cosem_res.buf[0] = 0xe6;
			g_cosem_res.buf[1] = 0xe7;
			g_cosem_res.buf[2] = 0x00;
			g_apli_offset = 3;
		}
		else {
			g_apli_offset = 0;
		}
		g_cosem_res.pos = g_apli_offset;
		if (g_userQ == 4) {
			push_associate();
		}
		offset = OFFSET_CMD + g_apli_offset;
		order = in->req.buf[g_cosem_res.pos];
		g_cosem_res.buf[g_cosem_res.pos] = order;
		set_security_policyQ(DLMS_INVALID);
		init_system_titleQ();
		mid_req_len = in->req.len - g_cosem_res.pos;
		switch (order) {
		case CMD_AARQ:
			g_cosem_res.buf[g_cosem_res.pos]++;
			g_cosem_res.pos++;
			g_acse_req.len = mid_req_len;					/*要求電文長*/
			g_acse_req.buf = &in->req.buf[offset];		/* 要求電文の先頭アドレスを保存 */
			return;

		case CMD_RLRQ:
			g_cosem_res.buf[g_cosem_res.pos]++;
			g_cosem_res.pos++;
			if (associated_client()) {
				g_acse_req.len = mid_req_len;				/*要求電文長*/
				g_acse_req.buf = &in->req.buf[offset];	/* 要求電文の先頭アドレスを保存 */
				return;
			}
			break;
		case CMD_GET_REQUEST:
		case CMD_SET_REQUEST:
		case CMD_ACT_REQUEST:
		case CMD_VOID_CALL:
			g_cosem_res.buf[g_cosem_res.pos] += CONV_REQ_RES_LN;	/**/
			g_cosem_res.pos++;
			if (associated_client()) {
				g_xdlms_req.len = mid_req_len;				/*要求電文長*/
				g_xdlms_req.buf = &in->req.buf[offset];	/* 要求電文の先頭アドレスを保存 */
				return;
			}
			break;

		case CMD_NOTIFICATION_CALL:
			g_cosem_res.buf[g_cosem_res.pos] = CMD_GENERAL_GLO;	/**/
			g_cosem_res.pos++;
			g_xdlms_req.len = mid_req_len;				/*要求電文長*/
			g_xdlms_req.buf = &in->req.buf[offset];		/* 要求電文の先頭アドレスを保存 */
			return;

		case CMD_GENERAL_GLO:
			cipher = TRUE;
			g_cosem_res.buf[g_cosem_res.pos] = CMD_GENERAL_GLO;	/**/
			g_cosem_res.pos++;
			if (authenticated_associated_client()) {
				g_gcm_req.len = mid_req_len;		/*要求電文長*/
				g_gcm_req.buf = &in->req.buf[offset];	/* 要求電文の先頭アドレスを保存 */
				return;
			}
			break;

		case CMD_GLO_GET_REQUEST:			/**/
		case CMD_GLO_SET_REQUEST:			/**/
		case CMD_GLO_ACT_REQUEST:			/**/
			cipher = TRUE;
			g_cosem_res.buf[g_cosem_res.pos] += CONV_REQ_RES_LN;	/**/
			g_cosem_res.pos++;
			if (challenge_client()) {
				g_gcm_req.len = mid_req_len;				/*要求電文長*/
				g_gcm_req.buf = &in->req.buf[offset];	/* 要求電文の先頭アドレスを保存 */
				return;
			}
			else {
				if (authenticated_associated_client()) {
					g_gcm_req.len = mid_req_len;			/*要求電文長*/
					g_gcm_req.buf = &in->req.buf[offset];	/* 要求電文の先頭アドレスを保存 */
					return;
				}
			}
			break;
		case CMD_DED_GET_REQUEST:
		case CMD_DED_SET_REQUEST:
		case CMD_DED_ACT_REQUEST:
		case CMD_GENERAL_DED:
			cipher = TRUE;
			g_cosem_res.buf[g_cosem_res.pos] += CONV_REQ_RES_LN;	/**/
			g_cosem_res.pos++;
			if (authenticated_associated_client()) {
				g_gcm_req.len = mid_req_len;				/*要求電文長*/
				g_gcm_req.buf = &in->req.buf[offset];	/* 要求電文の先頭アドレスを保存 */
				return;
			}
			break;
/*			case CMD_READREQUEST: */
/*			case CMD_WRITEREQUEST: */
/*			case CMD_GLO_READREQUEST: */
/*			case CMD_GLO_WRITEREQUEST: */
/*			case CMD_DED_READREQUEST: */
/*			case CMD_DED_WRITEREQUEST: */
/*			case CMD_CONFIRMEDSERVICEERROR: */
/*			case CMD_GLO_CONFIRMEDSERVICEERROR: */
/*			case CMD_DED_CONFIRMEDSERVICEERROR: */
/*			case CMD_UNCONFIRMEDWRITEREQUEST: */
/*			case CMD_GLO_UNCONFIRMEDWRITEREQUEST: */
/*			case CMD_DED_UNCONFIRMEDWRITEREQUEST: */
/*			case CMD_GLO_EVENT_NOTIFICATION_REQUEST: */
/*			case CMD_DED_EVENT_NOTIFICATION_REQUEST: */
/*			case CMD_INFORMATIONREPORTREQUEST: */
/*			case CMD_GLO_INFORMATIONREPORTREQUEST: */
/*			case CMD_DED_INFORMATIONREPORTREQUEST: */
/*			case CMD_EXCEPTION_RESPONSE: */
/*			case CMD_RESERVED: */
		default:
			break;
		};
		if (associated_client()) {
			if (cipher && (FALSE == authenticated_client())) {
				/* Info : invalid request by LLS client */
				ConfirmServiceError(ERROR_SECURITY_INVALID_AUTH);
			}
			else {
				/* Info : invalid request by authenticated client */
				ConfirmServiceError(ERROR_HDLC_INVALID_ORDER);
			}

		}
		else {
			/* Info : invalid request */
			ConfirmServiceError(ERROR_HDLC_ASSOCIATION_INCOMPLETE);
		}
	}
	else {
		/* Info : invalid request by LLS client */
		ConfirmServiceError(ERROR_SECURITY_INVALID_AUTH);
	}
}

static void res_layer1(PPIP in, SEQ *seq) {

	switch (g_cosem_res.buf[OFFSET_CMD + g_apli_offset])
	{
	case CMD_AARQ + 1:
		if (FALSE == g_authQ) {
			ramcpy(&g_cosem_res.buf[g_cosem_res.pos], g_xdlms_res, g_xdlms_res_len);
			g_cosem_res.pos += g_xdlms_res_len;
			ramcpy(&g_cosem_res.buf[g_cosem_res.pos], g_raw, g_raw_len);
			g_cosem_res.pos += g_raw_len;
		}
		else {
			/*その他は、GCMのモジュールで生成された電文を使用*/
			delete_gcm();
		}
		in->res.len = g_cosem_res.pos;
		break;
	case CMD_RLRQ + 1:
		in->res.len = g_cosem_res.pos;
		break;
	case CMD_GET_REQUEST + CONV_REQ_RES_LN:
	case CMD_SET_REQUEST + CONV_REQ_RES_LN:
	case CMD_ACT_REQUEST + CONV_REQ_RES_LN:
		ramcpy(&g_cosem_res.buf[g_cosem_res.pos], g_xdlms_res, g_xdlms_res_len);
		g_cosem_res.pos += g_xdlms_res_len;
		ramcpy(&g_cosem_res.buf[g_cosem_res.pos], g_raw, g_raw_len);
		g_cosem_res.pos += g_raw_len;
		in->res.len = g_cosem_res.pos;
		break;
	case CMD_GLO_GET_REQUEST + CONV_REQ_RES_LN:	
	case CMD_GLO_SET_REQUEST + CONV_REQ_RES_LN:	
	case CMD_GLO_ACT_REQUEST + CONV_REQ_RES_LN:
	case CMD_GENERAL_GLO:
#if 0
		if (challenge_client()) {
			set_non_associated(3);
		}
		delete_gcm();
#endif
		in->res.len = g_cosem_res.pos;
		break;
	case CMD_DED_GET_REQUEST + CONV_REQ_RES_LN:
	case CMD_DED_SET_REQUEST + CONV_REQ_RES_LN:
	case CMD_DED_ACT_REQUEST + CONV_REQ_RES_LN:
	case CMD_GENERAL_DED:
//		set_dlms_error0();
		in->res.len = g_cosem_res.pos;
		break;

	case CMD_VOID:
		in->res.len = 0xffff;
		break;
	default:	/*expect only Confirm Service Error*/
		in->res.len = g_cosem_res.pos;
		break;
		/*			case CMD_INITIATE_REQUEST:				*/
		/*			case CMD_CONFIRMEDSERVICEERROR:			*/
		/*			case CMD_READREQUEST:					*/
		/*			case CMD_WRITEREQUEST:					*/
		/*			case CMD_UNCONFIRMEDWRITEREQUEST:		*/
		/*			case CMD_EVENT_NOTIFICATION_REQUEST:	*/
		/*			case CMD_INFORMATIONREPORTREQUEST:		*/
		/*			case CMD_EXCEPTION_RESPONSE:			*/
		/*			case CMD_RESERVED:						*/

		/*			case CMD_GLO_INITIATE_REQUEST:			*/
		/*			case CMD_GLO_GET_REQUEST:				*/
		/*			case CMD_GLO_SET_REQUEST:				*/
		/*			case CMD_GLO_CONFIRMEDSERVICEERROR:		*/
		/*			case CMD_GLO_READREQUEST:				*/
		/*			case CMD_GLO_WRITEREQUEST:				*/
		/*			case CMD_GLO_UNCONFIRMEDWRITEREQUEST:	*/
		/*			case CMD_GLO_EVENT_NOTIFICATION_REQUEST:*/
		/*			case CMD_GLO_INFORMATIONREPORTREQUEST:	*/

		/*			case CMD_DED_INITIATE_REQUEST:			*/
		/*			case CMD_DED_CONFIRMEDSERVICEERROR:		*/
		/*			case CMD_DED_READREQUEST:				*/
		/*			case CMD_DED_WRITEREQUEST:				*/
		/*			case CMD_DED_UNCONFIRMEDWRITEREQUEST:	*/
		/*			case CMD_DED_EVENT_NOTIFICATION_REQUEST:*/
		/*			case CMD_DED_INFORMATIONREPORTREQUEST:	*/
	};
	if (g_userQ == 4) {
		set_non_associated(3);
	}
	if (max_res_len < in->res.len) {
		max_res_len = in->res.len;
	}
	in->sta = STATE_WAIT;
	cosem_res_clear();
}

ui08 isCosem(void) {

	ui08 ret;

	if (NULL == g_cosem_res.buf) {	/*COSEM処理中*/
		ret = FALSE;
	}
	else {
		ret = TRUE;
	}
	return ret;
}

static ui08 isBusy(cui08 port) {

	ui08 ret;

	ret = FALSE;
	if (port == 1) {
		if (0 != seq2.res.adr2.hf) {
			ret = TRUE;
		}
	}
	else {
		if (0 != seq1.res.adr2.hf) {
			ret = TRUE;
		}
	}
	return ret;
}

static ui08 set_dataID(const ui08 dataid, cui08* in) {

	ui08 ret, cmp;

	switch (dataid) {
	case 1:	/*serial number*/
		ret = write_othinf(eCID1, 0, in);
		ramcpy(meter.id9, &in[2], 10);
		nicsts.bt.inf1 = 0;
		break;

	case 2:	/*imei*/
		ret = write_othinf(eCID3, 0, in);
		ramcpy(meter.id10, &in[2], 15);
		cmp = ramcmp(in, def_imei, sizeof(def_imei));
		if (cmp != 0) {
			nicsts.bt.inf2 = 0;
		}
		else {
			nicsts.bt.inf2 = 1;
		}
		break;

	case 3:/*bluetooth*/
		ret = write_othinf(eCID10, 0, in);
		ramcpy(meter.id11, &in[2], 17);
		break;

	case 4:/*pdu*/
		ret = write_othinf(eCID2, 0, in);
		break;

	case 5:/*ipv6 */
		ret = write_othinf(eCID6, 0, in);
		cmp = ramcmp(in, def_ipv6, sizeof(def_ipv6));
		if (cmp != 0) {
			nicsts.bt.inf3 = 0;
		}
		else {
			nicsts.bt.inf3 = 1;
		}
		break;

	case 6:/*iccid 20*/
		ret = write_othinf(eCID4, 0, in);
		cmp = ramcmp(in, def_iccid, sizeof(def_iccid));
		if (cmp != 0) {
			nicsts.bt.inf4 = 0;
		}
		else {
			nicsts.bt.inf4 = 1;
		}
		break;

	case 7:/*nic year*/
		ret = write_othinf(eCID7, 0, in);
		nicsts.bt.inf5 = 0;
		break;

	case 10:/*version*/
		ret = write_othinf(eCID8, 0, in);
		nicsts.bt.inf6 = 0;
		break;

	default:
		ret = write_othinf(eCID9, 0, in);
		break;
	}
	return ret;
}

static void local(PPIP in, SEQ *seq) {

	ui08 tmp1, tmp2, tmp3, tmp4, i;
	ui16 len;
	DAT4 dat;

	switch (seq->req.cmd) {
	case 0:
		if (seq->cks != 0) {
			seq->res.cmd = 1;
			in->res.buf[in->res.pos++] = in->req.buf[in->req.pos++];
		}
		break;

	case 1:	/* request */
		if (seq->cks != 0) {
			seq->res.cmd = 2;
			tmp1 = in->req.buf[in->req.pos++];
			switch (tmp1) {
			case 0xff:
				in->res.buf[in->res.pos++] = 0x02;
				in->res.buf[in->res.pos++] = 0x00;
				break;
			case 13:
			case 23:
			case 33:
			case 43:
			case 53:
			case 63:
			case 73:
				if (tmp1 == 13) {
					nicsts.bt.adr1 = 0;
				}
				if (tmp1 == 23) {
					nicsts.bt.adr2 = 0;
				}
				if (tmp1 == 33) {
					nicsts.bt.adr3 = 0;
				}
				if (tmp1 == 43) {
					nicsts.bt.adr4 = 0;
				}
				if (tmp1 == 53) {
					nicsts.bt.adr5 = 0;
				}
				if (tmp1 == 63) {
					nicsts.bt.adr6 = 0;
				}
				if (tmp1 == 73) {
					nicsts.bt.adr7 = 0;
				}
				tmp1 -= 3;
				in->res.buf[in->res.pos++] = 0x02;
				in->res.buf[in->res.pos++] = 0x02;
				get_othinf(in->res.buf, &in->res.pos, tmp1);
				get_othinf(in->res.buf, &in->res.pos, tmp1 + 1);
				nicsts.bt.sts = 0;
				break;
			default:
				if (tmp1 == 1) {
					nicsts.bt.gprs = 0;
				}
				if (tmp1 == 2) {
					nicsts.bt.port = 0;
				}
				if (tmp1 == 13) {
					nicsts.bt.adr1 = 0;
				}
				if (tmp1 == 23) {
					nicsts.bt.adr2 = 0;
				}
				if (tmp1 == 33) {
					nicsts.bt.adr3 = 0;
				}
				if (tmp1 == 43) {
					nicsts.bt.adr4 = 0;
				}
				if (tmp1 == 53) {
					nicsts.bt.adr5 = 0;
				}
				if (tmp1 == 63) {
					nicsts.bt.adr6 = 0;
				}
				if (tmp1 == 73) {
					nicsts.bt.adr7 = 0;
				}
				get_othinf(in->res.buf, &in->res.pos, tmp1);
				nicsts.bt.sts = 0;
				break;
			}
		}
		else {
			seq->res.cmd = 255;
			in->res.buf[in->res.pos++] = seq->req.cmd;
		}
		break;

	case 3:	/* write */
		if (seq->cks != 0) {
			if (sealed() != 2) {
				seq->res.cmd = seq->req.cmd;
				tmp1 = in->req.buf[in->req.pos++];	/*structure or single*/
				switch (tmp1) {
				case 0x02:	/*structure*//*2424nn030201160902AAAA*/
					tmp1 = in->req.buf[in->req.pos++];/*count*/
					for (i = 0; i < tmp1; i++) {
						tmp2 = in->req.buf[in->req.pos++];		/*ENUM*/
						if (tmp2 == 0x16) {
							tmp2 = in->req.buf[in->req.pos++];	/*ID*/
							len = write_othinf(tmp2, 0, &in->req.buf[in->req.pos]);
							if (len == 0) {
								break;
							}
							else {
								if (tmp2 == 1) {
									nicsts.bt.gprs = 1;
								}
								if (tmp2 == 2) {
									nicsts.bt.port = 1;
								}
								if (tmp2 == 13) {
									nicsts.bt.adr1 = 1;
								}
								if (tmp2 == 23) {
									nicsts.bt.adr2 = 1;
								}
								if (tmp2 == 33) {
									nicsts.bt.adr3 = 0;
								}
								if (tmp2 == 43) {
									nicsts.bt.adr4 = 0;
								}
								if (tmp2 == 53) {
									nicsts.bt.adr5 = 0;
								}
								if (tmp2 == 63) {
									nicsts.bt.adr6 = 0;
								}
								if (tmp2 == 73) {
									nicsts.bt.adr7 = 0;
								}
								in->req.pos += len;
							}
						}
						else {
							len = 0;
							break;
						}
					}
					if (len == 0) {
						in->res.buf[in->res.pos++] = 0;
					}
					else {
						in->res.buf[in->res.pos++] = 1;
					}
					break;

				default:/*2424nn0316010902AAAA*/
					if (tmp1 == 0x16) {
						tmp1 = in->req.buf[in->req.pos++];
						len = write_othinf(tmp1, 0, &in->req.buf[in->req.pos]);
						if (len == 0) {
							in->res.buf[in->res.pos++] = 0;
						}
						else {
							if (tmp2 == 1) {
								nicsts.bt.gprs = 1;
							}
							if (tmp2 == 2) {
								nicsts.bt.port = 1;
							}
							if (tmp2 == 13) {
								nicsts.bt.adr1 = 1;
							}
							if (tmp2 == 23) {
								nicsts.bt.adr2 = 1;
							}
							if (tmp2 == 33) {
								nicsts.bt.adr3 = 1;
							}
							if (tmp2 == 43) {
								nicsts.bt.adr4 = 1;
							}
							if (tmp2 == 53) {
								nicsts.bt.adr5 = 1;
							}
							if (tmp2 == 63) {
								nicsts.bt.adr6 = 1;
							}
							if (tmp2 == 73) {
								nicsts.bt.adr7 = 1;
							}
							in->res.buf[in->res.pos++] = 1;
						}
						in->req.pos += len;
					}
					break;
				}
			}
			else {
				seq->res.cmd = 255;
				in->res.buf[in->res.pos++] = seq->req.cmd;
			}
		}
		else {
			seq->res.cmd = 255;
			in->res.buf[in->res.pos++] = seq->req.cmd;
		}
		break;

	case 0x10:
		if (seq->cks != 0) {
			seq->res.cmd = 0x10;
			tmp1 = in->req.buf[in->req.pos];
			in->res.buf[in->res.pos++] = tmp1;
			switch (tmp1) {
			case 0x01:
				break;
			case 0x02:
				break;
			}
		}
		break;

	case 0x11:
		if (seq->cks != 0) {
			tmp1 = in->req.buf[in->req.pos++];		/*Type*/
			if (tmp1 == 0x16) {
				tmp2 = in->req.buf[in->req.pos++];	/*ID*/
				switch (tmp2) {
				case 0x01:	/*RSSI*/
					in->req.pos++;	/*Type*/
					nicinf.rssi = in->req.buf[in->req.pos];
					in->req.pos += 1;
					break;
				case 0x02:/*Status*/
					in->req.pos++;	/*Type*/
					ramcpy(dat.by, &in->req.buf[in->req.pos], sizeof(nicinf.status));
					nicinf.status.by[0] = dat.by[3];
					nicinf.status.by[1] = dat.by[2];
					nicinf.status.by[2] = dat.by[1];
					nicinf.status.by[3] = dat.by[0];
					in->req.pos += 4;
					break;
				case 0x03:	/*RSRP*/
					in->req.pos++;	/*Type*/
					nicinf.rsrp = in->req.buf[in->req.pos];
					in->req.pos += 1;
					break;
				case 0x04:	/*RSRQ*/
					in->req.pos++;	/*Type*/
					nicinf.rsrq = in->req.buf[in->req.pos];
					in->req.pos += 1;
					break;
				case 0x05:	/*SNR*/
					in->req.pos++;	/*Type*/
					nicinf.sinr = in->req.buf[in->req.pos];
					in->req.pos += 1;
					break;
				case 0x06:	/*CELLID*/
					in->req.pos++;	/*Type*/
					ramcpy(dat.by, &in->req.buf[in->req.pos], sizeof(nicinf.status));
					nicinf.cellid.by[0] = dat.by[3];
					nicinf.cellid.by[1] = dat.by[2];
					nicinf.cellid.by[2] = dat.by[1];
					nicinf.cellid.by[3] = dat.by[0];
					in->req.pos += 4;
					break;
				case 0x07:	/*RAT*/
					in->req.pos++;	/*Type*/
					nicinf.rat = in->req.buf[in->req.pos];
					in->req.pos += 1;
					break;
				case 0x08:	/*BAND*/
					in->req.pos++;	/*Type*/
					nicinf.band = in->req.buf[in->req.pos];
					in->req.pos += 1;
					break;
				}
				nicinf.status.by[0] &= ~0x01;
			}
			seq->res.adr2.hf = 0;
			invRcvSnd(0, in, seq);	/*受信処理へ*/
		}
		break;

	case 0x20:
		if (seq->cks != 0) {
			seq->res.cmd = 0x20;
			in->res.buf[in->res.pos++] = in->req.buf[in->req.pos++];
		}
		break;

	case 0x21:
		if (seq->cks != 0) {
			tmp1 = in->req.buf[in->req.pos++];	/*structure or single*/
			switch (tmp1) {
			case 0x02:	/*structure*//*2424nn030201160902AAAA*/
				tmp1 = in->req.buf[in->req.pos++];	/*count*/
				tmp4 = tmp1;
				for (i = 0; i < tmp1; i++) {
					tmp2 = in->req.buf[in->req.pos++];		/*ENUM*/
					if (tmp2 == 0x16) {
						tmp2 = in->req.buf[in->req.pos++];	/*ID*/
						tmp3 = set_dataID(tmp2, &in->req.buf[in->req.pos]);
						in->req.pos += tmp3;
					}
				}
				nicsts.bt.sts = 0;
				break;

			default:/*2424nn0316010902AAAA*/
				if (tmp1 == 0x16) {
					tmp2 = in->req.buf[in->req.pos++];	/*ID*/
					tmp3 = set_dataID(tmp2, &in->req.buf[in->req.pos]);
					in->req.pos += tmp3;
					nicsts.bt.sts = 0;
				}
				break;
			}
			invRcvSnd(0, in, seq);	/*受信処理へ*/
			seq->res.adr2.hf = 0;
		}
		break;

	case 0x99:	/* initialize */
		if (seq->cks != 0) {
			seq->res.cmd = seq->req.cmd;
			if (sealed() != 2) {
				tmp1 = init_all_othinf();
			}
			else {
				tmp1 = 0xff;
			}
			in->res.buf[in->res.pos++] = tmp1;
		}
		else {
			seq->res.cmd = 255;
			in->res.buf[in->res.pos++] = seq->req.cmd;
		}
		break;
	default:
		seq->res.cmd = 255;
		in->res.buf[in->res.pos++] = seq->req.cmd;
		break;
	}
	in->res.len = in->res.pos;
	in->res.pos = 0;
	in->req.len = 0;		/*要求電文クリア*/
}

static void cosem(PPIP in, SEQ *seq) {

	if (0 < in->req.len) {					/*要求電文有り?*/

		if (isCosem() == FALSE) {			/*COSEM処理中*/
			req_layer1(in, seq);			/*要求処理*/
			in->req.len = 0;				/*要求電文クリア*/
			in->res.len = 0;				/*応答電文の初期化*/
		}
	}
	if (!(in->sta == STATE_WAIT)) {			/*応答状態*/
		if ((0 < mid_res_len) || (0 < g_cosem_res.len))	/*応答電文有無判定*/
		{
			res_layer1(in, seq);			/*応答処理*/
		}
	}
}

/**************** optical recieve/send ****************/
/*layer1=layer2*/
static ui08 _localR(SEQ *seq, cui08 in) {

	ui08 ret;

	ret = NO_ERR;
	seq->tmr = seq->oct;
	seq->pos++;
	switch (seq->phs) {
	case 0:
		if (in == 0x24) {
			seq->phs++;
		}
		else {
			ret = ER_STX;
		}
		break;

	case 1:/*長さ*/
		seq->req.len = in + 3;
		seq->phs = 3;
		break;

	case 3:
		seq->req.cmd = in;
		seq->inf = 0;
		seq->phs++;
		break;

	default:
		seq->req.buf[seq->inf++] = in;				/*データセット*/
		if (seq->pos >= seq->req.len) {
			seq->cks = 1;
//			seq->phs = 0xff;
			ret = END_SR;
		}
		break;
	}
	return ret;
}

static ui08 _localS(pui08 out, SEQ *seq) {

	ui08 ret;

	ret = NO_ERR;
	switch (seq->phs) {
	case 0:
		seq->pos++;
		*out = 0x24;
		seq->phs++;
		break;

	case 1:/*長さ*/
		seq->pos++;
		*out = seq->res.len;
		seq->res.len += 3;
		seq->inf = 0;
		seq->phs = 3;
		break;

	case 2:/*長さ*/
		seq->pos++;
		seq->cur--;
		if (seq->cur == 1) {
			*out = (seq->res.len >> 8);
		}
		else {
			*out = seq->res.len & 0x00ff;
			seq->phs++;
			seq->inf = 0;
			seq->res.len += seq->ads;
			seq->res.len += 3;
			seq->ads = 0;
		}
		break;

	case 3:
		seq->pos++;
		*out = seq->res.cmd;
		seq->inf = 0;
		seq->phs++;
		break;

	case 4:
		seq->pos++;
		*out = seq->res.buf[seq->inf];			/*データセット*/
		if (seq->pos < seq->res.len) {
			seq->inf++;
		}
		else {
			seq->phs++;
		}
		break;

	default:
		ret = END_SR;
//		seq->phs = 0xff;
		break;
	}
	return ret;
}

/**************** basic recieve/send ****************/
/*layer1=layer2*/
static ui08 _basicR(SEQ *seq, cui08 in) {

	ui08 ret;

	seq->tmr = RFRAM_TMA;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
	ret = NO_ERR;

	/*回路1を使っているなら、ここではじく、inf0でENDSR*/
	seq->req.buf[seq->pos++] = in;			/*データセット*/
	if (seq->pos >= seq->req.len) {
		if (in == (ui08)(seq->cks & 0x00ff)) {
//				seq->phs = 0xff;
			ret = END_SR;
		}
		else {
			ret = ER_ETX;
		}
	}
	else {
		seq->cks ^= in;
		if (in == CD_ETX) {
			seq->req.len = seq->pos + 1;	/* 受信ﾃﾞｰﾀ最大数更新	*/
		}
		ret = NO_ERR;
	}
	return ret;
}

static ui08 _basicS(pui08 out, SEQ *seq) {

	ui08 ret;

	switch (seq->phs) {
	case 0:
		if ((seq->pos + 1) < seq->res.len) {
			*out = seq->res.buf[seq->pos++];			/*データセット*/
			seq->cks ^= *out;
		}
		else {
			*out = (ui08)(seq->cks & 0x00ff);
			seq->phs++;
		}
		ret = NO_ERR;
		break;
	default:
//		seq->phs = 0xff;
		ret = END_SR;
		break;
	}
	return ret;
}

static ui08 _wrapperR(SEQ *seq, cui08 in) {

	ui08 ret;

	ret = NO_ERR;

	seq->tmr = seq->oct;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
	seq->pos++;
	if (seq->pos == 2) {
		if (in > 0x03 || in == 0x00) {
			ret = ER_VER;
		}
		else {
			seq->neg = in;
		}
	}
	else if (seq->pos == 3) {	/*クライアントアドレス*/
		seq->ads = 0;
		seq->req.adr2.hf = in;
		seq->req.adr2.hf <<= 8;
	}
	else if (seq->pos == 4) {
		seq->req.adr2.hf += in;
		seq->req.adr2.hf <<= 1;
		seq->req.adr2.hf |= 0x0001;
	}
	else if (seq->pos == 5) {	/*メータアドレス*/
		if (in != 0x00) {
			ret = ER_ADR;
		}
	}
	else if (seq->pos == 6) {
		if (in != 0x01) {
			ret = ER_ADR;
		}
	}
	else if (seq->pos == 7) {
		seq->req.len = in;
		seq->req.len <<= 8;
	}
	else if (seq->pos == 8) {
		seq->req.len += in;
		if (seq->req.len >= seq->req.siz) {
			ret = ER_LEN;
		}
		else {
			if (seq->req.len < 5) {
				ret = ER_LEN;
			}
			else {
				seq->req.len += 8;
			}
		}
		seq->inf = 0;
	}
	else {
		seq->req.buf[seq->inf++] = in;			/*データセット*/
		if (seq->pos >= seq->req.len) {
			seq->cks = 1;
//			seq->phs = 0xff;
			ret = END_SR;
		}
	}
	return ret;
}

static ui08 _wrapperS(pui08 out, SEQ *seq) {

	ui08 ret;
	DAT2 dat2;

	ret = NO_ERR;
	seq->pos++;
	switch (seq->phs) {
	case 0:
		*out = seq->neg;
		seq->phs++;
		break;

	case 1:
		*out = 0x00;
		seq->phs++;
		break;

	case 2:
		*out = 0x01;
		seq->phs++;
		break;

	case 3:
		dat2.hf = seq->res.adr2.hf;
		dat2.hf >>= 1;
		*out = dat2.by[1];
		seq->phs++;
		break;

	case 4:
		dat2.hf = seq->res.adr2.hf;
		dat2.hf >>= 1;
		*out = dat2.by[0];
		seq->phs++;
		break;

	case 5:
		if (seq->inf > 0) {
			if ((seq->inf + gbt_len) <= seq->res.siz) {
				seq->res.len += seq->inf;
			}
			else {
				seq->res.len += seq->res.siz;
			}
		}
		seq->inf += gbt_len;
		dat2.hf = seq->inf;
		*out = dat2.by[1];
		seq->phs++;
		break;

	case 6:
		dat2.hf = seq->inf;
		*out = dat2.by[0];
		seq->inf = 0;
		if (stream > 0) {
			seq->phs++;
		}
		else {
			seq->phs = 8;
		}
		break;

	case 7:
		*out = ((pui08)&gbt)[seq->inf];
		seq->inf++;
		if (seq->inf >= gbt_len) {
			seq->phs++;
			seq->inf = 0;
			seq->pos -= gbt_len;
		}
		break;

	case 8:
		*out = seq->res.buf[seq->inf];			/*データセット*/
		if (seq->pos < seq->res.len) {
			seq->inf++;
		}
		else {
			seq->phs++;
		}
		break;
	default:
//		seq->phs = 0xff;
		ret = END_SR;
		break;
	}
	return ret;
}

/**************** dlms recieve/send ****************/
static ui08 _hdlcR(SEQ *seq, cui08 in) {

	ui08 ret, tmp;
	DAT4 adr;

	ret = NO_ERR;

	seq->tmr = seq->oct;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
	seq->pos++;

	switch (seq->phs) {
	case 0:	/*長さ*/
		seq->req.len = (in & 0x07);
		seq->req.len <<= 8;
		seq->req.seg = in & 0x08;
		tmp = in & 0xf0;
		if (tmp != 0xa0) {
			ret = ER_VER;
		}
		set_crcs(seq, in);
		seq->phs++;
		break;

	case 1:/*長さ*/
		seq->req.len += in;				/* 受信ﾃﾞｰﾀ最大数更新	*/
		seq->req.len += 2;				/* フラグ2バイト*/
		if (seq->req.len < 7) {
			ret = ER_LEN;
		}
		set_crcs(seq, in);
		seq->cur = 0;
		seq->phs++;
		break;

	case 2:/*メーターアドレス1,2,4*/
		if ((in & 0x01) > 0) {
			seq->adr4.by[seq->cur] = in;
			seq->cur++;
			if (seq->cur == 2 || seq->cur == 3) {
				ret = ER_ADR;
			}
			else {
				if (seq->cur == 1) {
					adr.by[0] = seq->adr4.by[0];
					adr.by[0] >>= 1;
					if (adr.by[0] != 1) {
						ret = ER_ADR;
					}
				}
				else {
					adr.wd = UI32L(seq->adr4.by);
					adr.wd >>= 1;
					if (adr.by[2] != 1) {
						ret = ER_ADR;
					}
				}
			}
			seq->ads = seq->cur;
			seq->cur = 0;
			seq->phs++;
		}
		else {
			if (seq->cur < 3) {
				seq->adr4.by[seq->cur] = in;
				seq->cur++;
			}
			else {
				ret = ER_ADR;	/*seq->cur = 5*/
			}
		}
		set_crcs(seq, in);
		break;

	case 3:/*クライアントアドレス*/
		if ((in & 0x01) > 0) {
			seq->req.adr2.by[seq->cur] = in;
			seq->cur++;
			if (seq->cur < 2) {
				seq->ads |= (seq->cur << 4);
				seq->cur = 0;
				seq->phs++;
			}
			else {
				ret = ER_ADR;
			}
		}
		else {
			if (seq->cur < 1) {
				seq->req.adr2.by[seq->cur] = in;
				seq->cur++;
			}
			else {
				ret = ER_ADR;
			}
		}
		set_crcs(seq, in);
		break;

	case 4:
		seq->req.cmd = in;
		seq->req.pfb = in & HDLC_PF;
		set_crcs(seq, in);
		seq->phs++;
		seq->cur = seq->pos;
		break;

	case 5:
		if ((seq->pos + 3) <= seq->req.len) {
			if (seq->pos == (seq->cur + 1)) {				/*HCS*/
				seq->cks = get_crcs(seq);
				tmp = (seq->cks & 0x00ff);
				if (tmp != in) {
					ret = ER_BCC;
				}
			}
			else if (seq->pos == (seq->cur + 2)) {
				tmp = (seq->cks >> 8);
				if (tmp != in) {
					ret = ER_BCC;
				}
				else {
					seq->inf = 0;
				}
			}
			else {
				if ((seq->inf + 7) > seq->req.siz) {
					seq->pos = seq->inf + 7;
					ret = END_SR;
				}
				else {
					seq->req.buf[seq->inf++] = in;			/*データセット*/
				}
			}
			set_crcs(seq, in);
		}
		else {
			seq->cks = get_crcs(seq);
			tmp = (seq->cks & 0x00ff);
			if (tmp != in) {
				ret = ER_BCC;
			}
			else {
				seq->phs++;
			}
		}
		break;

	case 6:
		tmp = (seq->cks >> 8);
		if (tmp != in) {
			ret = ER_BCC;
		}
		else {
			seq->phs++;
		}
		break;

	case 7:
		if (in == HDLC_FLG) {
//			seq->phs = 0xff;
			ret = END_SR;
		}
		else {
			ret = ER_ETX;
		}
		break;
	}

	return ret;
}

static ui08 _hdlcS(pui08 out, SEQ *seq) {

	ui08 ret, pos;

	ret = NO_ERR;
	seq->pos++;
	switch (seq->phs) {
	case 0:/*長さ*/
		if (seq->inf > 0) {
			if (seq->inf <= seq->res.siz) {
				seq->res.len += seq->inf;
			}
			else {
				seq->res.len += seq->res.siz;
			}
			seq->res.len += 2;			/*HCS*/	
		}
		*out = seq->res.len >> 8;
		if (seq->res.seg > 0) {
			*out |= 0x08;
		}
		*out |= 0xa0;
		set_crcs(seq, *out);
		seq->phs++;
		break;

	case 1:/*長さ*/
		*out = seq->res.len & 0x00ff;
		seq->res.len += 2;	/*全長にする*/
		set_crcs(seq, *out);
		seq->phs++;
		seq->cur = 0;
		break;

	case 2:/*クライアントアドレス*/
		*out = seq->res.adr2.by[seq->cur];
		set_crcs(seq, *out);
		seq->cur++;
		pos = seq->ads & 0xf0;
		pos >>= 4;
		if (seq->cur == pos) {
			seq->cur = 0;
			seq->phs++;
		}
		break;

	case 3:/*メーターアドレス1-4*/
		*out = seq->adr4.by[seq->cur];
		set_crcs(seq, *out);
		seq->cur++;
		pos = seq->ads & 0x0f;
		if (seq->cur == pos) {
			seq->phs++;
			seq->cur = 0;
		}
		break;

	case 4:
		*out = seq->res.cmd;
		set_crcs(seq, *out);
		seq->phs++;
		seq->cur = seq->pos;
		seq->inf = 0;
		break;

	case 5:
		if ((seq->pos + 3) <= seq->res.len) {
			if (seq->pos == (seq->cur + 1)) {				/*HCS*/
				seq->cks = get_crcs(seq);
				*out = (ui08)(seq->cks & 0x00ff);
			}
			else if (seq->pos == (seq->cur + 2)) {
				*out = (ui08)(seq->cks >> 8);
			}
			else {
				*out = seq->res.buf[seq->inf++];	/*データセット*/
			}
			set_crcs(seq, *out);
		}
		else {
			seq->cks = get_crcs(seq);
			*out = (ui08)(seq->cks & 0x00ff);
			seq->phs++;
		}
		break;

	case 6:
		*out = (ui08)(seq->cks >> 8);
		seq->phs++;
		break;
		
	case 7:
		*out = 0x7e;
		seq->phs++;
		break;
	default:
//		seq->phs = 0xff;
		ret = END_SR;
		break;
	}
	return ret;
}

static	ui08(*const _stateR[])(SEQ *seq, cui08 in) = {
	_wrapperR,
	_localR,
	_basicR,
	_hdlcR
};

static	ui08(*const _stateS[])(pui08 out, SEQ *seq) = {
	_wrapperS,
	_localS,
	_basicS,
	_hdlcS
};

static ui08 _reciveBuf(SEQ *seq, cui08 in) {

	ui08 ret;

	ret = NO_ERR;
	if (seq->pos == 0) {
		seq->phs = 0;
		switch (in) {
		case LOCAL_FLG:
			seq->tmr = seq->oct;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
			seq->mod = MODE_LOCAL;
			break;
		case CD_STX:
			seq->tmr = RFRAM_TMA;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
			seq->mod = MODE_BASC;
			seq->cks = 0x00;
			seq->req.len = sizeof(req_buf);
			break;
		case WRAP_FLG:
			seq->tmr = seq->oct;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
			seq->mod = MODE_WRAP;
			break;
		case HDLC_FLG:
			seq->tmr = seq->oct;				/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
			seq->mod = MODE_DLMS;
			ini_crcs(seq);
			break;
		default:
			ret = ER_STX;
			break;
		};
		seq->pos++;
	}
	else {
		ret = _stateR[seq->mod](seq, in);
	}
	return ret;
}

static ui08 _sendBuf(pui08 out, SEQ *seq) {
	ui08 ret;

	ret = NO_ERR;
	seq->tmr = seq->oct;						/* 受信ｷｬﾗｸﾀﾀｲﾏｾｯﾄ	(ｔＡ)	*/
	if (seq->pos == 0) {
		seq->phs = 0;
		switch (seq->mod) {
		case MODE_LOCAL:
			seq->cks = 0;
			*out = LOCAL_FLG;
			break;
		case MODE_BASC:
			seq->cks = 0;
			seq->res.cmd = HDLC_PF;					/*STXの場合は、応答フラグオン */
			*out = CD_STX;
			break;
		case MODE_WRAP:
			*out = WRAP_FLG;
			break;
		case MODE_DLMS:
			ini_crcs(seq);
			*out = HDLC_FLG;
			break;
		default:
			ret = ER_STX;
			break;
		};
		seq->pos++;
	}
	else {
		ret = _stateS[seq->mod](out, seq);
	}
	return ret;
}

/************* モード処理 **********************/
/*tarからto+1までの個数算出. from==toなら1*/
static ui08 seqNumRange(cui08 from, cui08 to, cui08 window){

	ui08 tmp, i, ret = 0;

	tmp = from;
	for (i = 0; i < window; i++) {
		if (to == tmp) {
			ret = i + 1;
			break;
		}
		tmp++;		/*1を足す*/
		tmp %= 8;	/*0-7循環*/
	}
	return ret;
}

static ui32 getUserInfo(pui08 id, pui08 in, puint offset){

	ui08 len;
	ui16 ret;

	*id = in[(*offset)++];
	len =in[(*offset)++];
	if (len < 5) {
		ret = 0;
		switch (len) {
		case 4:
			ret += in[(*offset)++];
			ret <<= 8;
		case 3:
			ret += in[(*offset)++];
			ret <<= 8;
		case 2:
			ret += in[(*offset)++];
			ret <<= 8;
		case 1:
			ret += in[(*offset)++];
			break;
		}
	}
	else {

	}
	return ret;
}

/*window通信の場合、各ブロックサイズは128*/
/*セグメント通信の場合は、可変長でもよいのか？*/
static void req_frame_i(PIPE *pipe, SEQ *seq) {

	ui08 rnr, rns, snr, sns, ret;

	/*N(S),N(R)チェック*/
	sns = HDLC_NS(seq->req.cmd);/*異常フレームによる番号変更を避けるため、評価用に保存*/
	snr = HDLC_NR(seq->req.cmd);
#if 0
	seq->res.ns = snr;
	seq->res.nr = sns;
#endif
	rns = seq->res.ns % 8;
	rnr = seq->res.nr % 8;
	if (snr != rns) {
		/* クライアントRがサーバーSと不一致=クライアントがサーバーのS番データを受け取れていない*/
		/* サーバー、クライアント相互に128バイトを超える電文をやり取りするシーケンス*/
		/* サーバーS番のデータを受け取れていないけれど、クライアントS 番データを送ってきた */
		/* 送信Rが受信SにおけるIフレーム+メッセージ受信の条件に該当するケースがないため、RRフレームで対応*/
		pipe->cmd = INDEX_FRMRz;
	}
	else { /*クライアントRがサーバーSと一致*/
		ret = seqNumRange(sns, rnr, seq->req.maxW);	/*クライアントSに対するサーバーRはクライアントS+1. 通常は1.*/
		if (ret) {
			ret--;
			if (ret == 0) {
				seq->res.nr++;				/*クライアントに通知する次に受け取る番号*/
				seq->req.ns = sns;			/*評価結果*/
				seq->req.nr = snr;
				seq->req.posW++;
				pipe->req.pos += seq->inf;			/*segment+window=1*/
				seq->req.buf = &pipe->req.buf[pipe->req.pos];
				pipe->cmd = INDEX_I;
			}
			else {						/* 番号が違えばseq->res.nrは更新せずRR送信し、順番通りのデータを要求 */
				pipe->cmd = INDEX_RR;
			}
		}
		else {	/*受信バッファに存在しない番号指定のためフレーム削除要求Z*/
			pipe->cmd = INDEX_RR;
		}
	}
	if (seq->req.pfb > 0) {		/*応答処理*/
		if (pipe->cmd == INDEX_I) {
			if (seq->req.seg == 0) {
				pipe->req.len = pipe->req.pos;/*上層処理用に長さをセット*/
				pipe->req.pos = 0;
			}
			else {
				pipe->cmd = INDEX_RR;	/*セグメンテーションの続きを要求*/
			}
		}
		else {
			/*RR,frmr*/
		}
		seq->res.len = 1;
		invRcvSnd(1, pipe, seq);	/*送信処理へ	*/
	}
	else {	/* 無応答処理 */
		if (pipe->cmd == INDEX_I) {
			invRcvSnd(2, pipe, seq);	/*受信処理へ*/
			seq->res.adr2.hf = 0;
		}
		else {	/*final=0のRR、FRMR応答はエラー*/
			pipe->err = ER_VOID;		/*無応答*/
		}
		if (seq->req.seg) {
		}
		else {
		}
	}
}

static void res_frame_i(PIPE *pipe, SEQ *seq) {

	ui08 ns;
	ui16 len;

	if (!(0 == pipe->res.len)) {					/*上位からの応答が来てから処理*/
		if (seq->res.len == 0) {	/*resN_end呼び出し*/
			if (seq->res.pfb != 0) {
				if (pipe->res.pos >= pipe->res.len) {
					invRcvSnd(0, pipe, seq);			/*受信処理へ*/
				}
				else {
					invRcvSnd(2, pipe, seq);			/*受信処理へ*/
				}
			}
			else {
				seq->res.len = 1;
				invRcvSnd(3, pipe, seq);				/*送信処理へ*/
			}
		}
		else {					/*resN_rst呼び出し*/
			seq->res.posW++;
			len = pipe->res.len - pipe->res.pos;	/*残りサイズ*/
			if (len <= seq->res.siz) {
				seq->res.pfb = 1;
				seq->res.seg = 0;
			}
			else {						/*通常*/
				if(pipe->res.len != 0xffff) {
					len = seq->res.siz;
					if (seq->res.posW < seq->res.maxW) {		/*分割送信*/
						seq->res.pfb = 0;		/**/
						seq->res.seg = 0;		/**/
					}
					else {						/*セグメント送信*/
						seq->res.pfb = 1;		/*final*/
						seq->res.seg = 1;		/*segment*/
					}
				}
				else {
					pipe->err = ER_VOID;
				}
			}
			ns = seq->res.ns % 8;
			seq->res.cmd = seq->res.nr % 8;
			seq->res.cmd <<= 1;
			seq->res.cmd += seq->res.pfb;
			seq->res.cmd <<= 3;
			seq->res.cmd += ns;				/*古いIDを送る*/
			seq->res.cmd <<= 1;
			seq->res.ns++;
			seq->res.buf = &pipe->res.buf[pipe->res.pos];
			pipe->res.pos += len;
			seq->inf = len;
		}
	}
}

static void req_frame_rr(PIPE *pipe, SEQ *seq) {

	ui08 snr, rns, ret;
	
	if (seq->req.pfb > 0 && (seq->req.seg == 0)) {
		/*N(S),N(R)チェック*/
		snr = HDLC_NR(seq->req.cmd);
		rns = seq->res.ns % 8;
		if (snr != rns) {
			/* 送信Rが受信Sと不一致=クライアントがサーバーのS番データを受け取れていない*/
			/* サーバーが128バイトを超える電文を応答し、クライアントが受信できなかった時のシーケンス処理*/
			/*IEC13239 B.2.3.2 NRM secondary information frame error(pattern B)*/
			ret = seqNumRange(snr, rns, seq->res.maxW);	/*差分+1の算出*/
			if (ret) {	/* 不一致なので 最低 2 */
				ret--;	/* 最低2→1 */
				if (ret <= seq->res.posW) {	/*送信済みデータあり*/
					seq->req.nr = snr;
					seq->res.posW -= ret;	/*戻す*/
					seq->res.ns -= ret;		/*戻す*/
					pipe->res.pos = seq->res.posW*seq->res.siz;
					pipe->cmd = INDEX_I;
				}
				else {					/*window size 1 の処理*/
					pipe->cmd = INDEX_FRMRz;
				}
			}
			else {	/* windowサイズ超え。フレーム削除要求Z*/
				pipe->cmd = INDEX_FRMRz;
			}
		}
		else {
			if (seq->res.seg == 0) {
				pipe->cmd = INDEX_RR;
			}
			else {
				pipe->cmd = INDEX_I;
			}
		}
		/*応答処理*/
		seq->res.len = 1;				/*一旦サイズを設定*/
		if (pipe->cmd != INDEX_I) {		/*エラーもしくはRR*/
			invRcvSnd(1, pipe, seq);	/*送信処理へ*/
		}
		else {							/*再送信*/
			invRcvSnd(3, pipe, seq);	/*送信処理へ*/
		}
	}
	else {	/*RRフレームはWindow送信不可のためエラー*/
		pipe->err = ER_VOID;				/*無応答*/
		if (seq->req.seg) {
		}
		else {
		}
	}
}

static void res_frame_rr(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(2, pipe, seq);
	}
	else {
		/*RR送信*/
		seq->res.seg = 0;
		seq->res.cmd = seq->res.nr % 8;
		seq->res.cmd <<= 5;
		seq->res.cmd += HDLC_RR;
		seq->res.cmd |= HDLC_PF;
	}
}

static void res_frame_rnr(PIPE *pipe, SEQ *seq) {

	/*RNR送信*/
	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		seq->res.seg = 0;
		seq->res.cmd = seq->res.nr % 8;
		seq->res.cmd <<= 5;
		seq->res.cmd += HDLC_RNR;
		seq->res.cmd |= HDLC_PF;
	}
}

static void rstSeqCtl(RAW *raw) {
	raw->ns = 0;
	raw->nr = 0;
}

static void req_frame_snrm(PIPE *pipe, SEQ *seq) {
	pui08 buf;
	ui08 tmp;
	ui16 len;
	ui32 val;

	if (pipe->err == NO_ERR) {
		if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {
			if (pipe->prt == 2) {
				getNegoParam2(&seq->res.siz, &seq->req.siz, &seq->res.maxW, &seq->req.maxW);
			}
			else {
				getNegoParam1(&seq->res.siz, &seq->req.siz, &seq->res.maxW, &seq->req.maxW);
			}
			rstSeqCtl(&seq->res);		/*シーケンス番号の初期化*/
			seq->neg = 0;
			pipe->cmd = INDEX_UA;
			pipe->res.pos = 0;
			pipe->res.len = pipe->res.pos;

			if (seq->inf > 0) {		/*以降、第2レイヤ処理だけど、そのまま処理。*/
				seq->res.buf = pipe->res.buf;	/*res第1レイヤポインタ初期化*/
				seq->res.posW = 0;				/*res第1レイヤwindow位置初期化*/
				buf = &pipe->res.buf[pipe->res.pos];
				ramcpy(buf, NEGO_DEF, sizeof(NEGO_DEF));
				if (seq->req.buf[0] == 0x81 && seq->req.buf[1] == 0x80) {
					seq->neg = 1;
					len = seq->req.buf[2] + 3;
					for (seq->inf = 3; seq->inf < len;) {
						val = getUserInfo(&tmp, seq->req.buf, &seq->inf);
						switch (tmp) {
						case 5:				/*maximum information field length-transmit*/
							if (val != seq->req.siz) {
								if (val < seq->req.siz) {
									if (val < MINIMAM_BUF) {
										val = MINIMAM_BUF;
									}
									seq->req.maxW = seq->req.siz / val;
									seq->req.siz = val;
									buf[9] = (seq->req.siz >> 8);
									buf[10] = (seq->req.siz & 0x00ff);
								}
							}
							break;
						case 6:				/*maximum information field length-recieve*/
							if (val != seq->res.siz) {
								if (val < seq->res.siz) {
									if (val < MINIMAM_BUF) {
										val = MINIMAM_BUF;
									}
									seq->res.maxW = seq->res.siz / val;
									seq->res.siz = val;
									buf[5] = (seq->res.siz >> 8);
									buf[6] = (seq->res.siz & 0x00ff);
								}
							}
							break;
						case 7:				/*(window size-transmit)*/
							if (val != seq->req.maxW) {
								if (val < seq->req.maxW) {
									if (val < MINIMAM_WIN) {
										val = MINIMAM_WIN;
									}
									seq->req.maxW = val;
									buf[22] = seq->req.maxW;
								}
							}
							break;
						case 8:				/*(window size-recieve)*/
							if (val != seq->res.maxW) {
								if (val < seq->res.maxW) {
									if (val < MINIMAM_WIN) {
										val = MINIMAM_WIN;
									}
									seq->res.maxW = val;
									buf[16] = seq->res.maxW;
								}
							}
							break;
						};
					}
				}
				else {
					/*unknown information format*/
				}
				pipe->res.pos = sizeof(NEGO_DEF);
				pipe->res.len = pipe->res.pos;
				pipe->res.pos = 0;
				invRcvSnd(3, pipe, seq);	/*第2レイヤ処理前に送信処理へ*/
			}
			else {
				invRcvSnd(1, pipe, seq);	/*第2レイヤ処理前に送信処理へ*/
				/*no negosiation*/
			}
			seq->res.adr2.hf = seq->req.adr2.hf;
			seq->res.len = 1;/*?*/
		}
		else {	/*segimentation ON*/
			pipe->err = ER_VOID;
		}
	}
}

static void res_frame_ua(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		if (!(0 == pipe->res.len)) {
			seq->inf = pipe->res.len;
			pipe->res.len = 0;
		}
		seq->res.seg = 0;
		seq->res.cmd = HDLC_UA;
		seq->res.cmd |= HDLC_PF;
	}
}
static void req_frame_disc(PIPE *pipe, SEQ *seq) {

	if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {	/*応答処理*/
		if (seq->res.adr2.hf == 0) {
			pipe->cmd = INDEX_DM;
		}
		else {
			pipe->cmd = INDEX_UAD;
		}
		seq->res.adr2.hf = seq->req.adr2.hf;
		seq->res.len = 1;/*?*/
		invRcvSnd(1, pipe, seq);
	}
	else {
		pipe->err = ER_VOID;
	}
}

static void req_frame_frmrw(PIPE *pipe, SEQ *seq) {

	if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {	/*応答処理*/
		pipe->cmd = INDEX_FRMRw;
		seq->res.len = 1;/*?*/
		invRcvSnd(1, pipe, seq);
	}
	else {
		pipe->err = ER_VOID;
	}
}

static void req_frame_frmrb(PIPE *pipe, SEQ *seq) {

	if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {	/*応答処理*/
		pipe->cmd = INDEX_FRMRb;
		seq->res.len = 1;/*?*/
		invRcvSnd(1, pipe, seq);
	}
	else {
		pipe->err = ER_VOID;
	}
}

static void req_frame_frmr(PIPE *pipe, SEQ *seq) {

	if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {	/*応答処理*/
		pipe->cmd = INDEX_FRMR0;
		seq->res.len = 1;/*?*/
		invRcvSnd(1, pipe, seq);
	}
	else {
		pipe->err = ER_VOID;
	}
}

static void req_frame_unknown(PIPE *pipe, SEQ *seq) {

	if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {	/*応答処理*/
		pipe->cmd = INDEX_DM;
		seq->res.len = 1;/*?*/
		seq->res.adr2.hf = seq->req.adr2.hf;
		invRcvSnd(1, pipe, seq);
	}
	else {
		pipe->err = ER_VOID;
	}
}

static void req_frame_busy(PIPE *pipe, SEQ *seq) {

	pipe->cmd = INDEX_BUSY;
	seq->res.len = 1;/*?*/
	invRcvSnd(1, pipe, seq);
}

static void res_frame_busy(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		seq->res.seg = 0;
		seq->res.cmd = HDLC_DM;
		seq->res.cmd |= HDLC_PF;
	}
}

static void res_frame_dm(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
		rstSeqCtl(&seq->req);
		rstSeqCtl(&seq->res);
		seq->res.adr2.hf = 0;
	}
	else {
		seq->res.seg = 0;
		seq->res.cmd = HDLC_DM;
		seq->res.cmd |= HDLC_PF;
	}
}

static void res_frame_uad(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
		rstSeqCtl(&seq->req);
		rstSeqCtl(&seq->res);
		seq->res.adr2.hf = 0;
	}
	else {
		seq->res.seg = 0;
		seq->res.cmd = HDLC_UA;
		seq->res.cmd |= HDLC_PF;
	}
}

static void frmrCreator(SEQ *seq, cui08 wxyz) {

	ui08 tmp;

	seq->res.seg = 0;
	seq->res.cmd = HDLC_FRMR;
	seq->res.cmd |= HDLC_PF;
/*a) The rejected frame control field shall be the control field of the received frame which caused the frame reject exception
condition.*/
	seq->res.buf[seq->inf++] = seq->req.cmd;
/*b) N(S) shall be the current value of the send state variable at the secondary/combined station (bit 10 = low-order bit).*/
	tmp = (seq->res.ns % 8);
	tmp <<= 4;
/*c) C/R set to "1" shall indicate that the frame which caused the frame reject exception condition was a response frame, and
C/R set to "0" shall indicate that the frame which caused the frame reject exception condition was a command frame.*/
	tmp |= 0;	/* c/rbit */
	tmp <<= 1;
/*d) N(R) shall be the current value of the receive state variable at the secondary/combined station (bit 14 = low-order bit).*/
	tmp |= (seq->res.nr % 8);
	seq->res.buf[seq->inf++] = tmp;
	seq->res.buf[seq->inf++] = wxyz;
}

/*e) w set to "1" shall indicate that the control field received and returned in bits 1 to 8 inclusive was undefined or not
implemented.*/
static void res_frmr_w(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		frmrCreator(seq, 0x80);
	}
}

/*f) x set to "1" shall indicate that the control field received and returned in bits 1 to 8 inclusive was considered invalid
because the frame contained an information field which is not permitted with this command or response. Bit w shall be set
to "1" in conjunction with this bit.*/
static void res_frmr_x(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		frmrCreator(seq, 0x40);
	}
}

/*g) y set to "1" shall indicate that the information field received exceeded the maximum information field length which can be
accommodated by the secondary/combined station.*/
static void res_frmr_y(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		frmrCreator(seq, 0x20);
	}
}

/*h) z set to "1" shall indicate that the control field received and returned in bits 1 to 8 inclusive contained an invalid N(R).*/
static void res_frmr_z(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		frmrCreator(seq, 0x10);
	}
}

static void res_frmr_b(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		frmrCreator(seq, 0x04);
	}
}

static void res_frmr_other(PIPE *pipe, SEQ *seq) {

	if (seq->res.len == 0) {
		invRcvSnd(0, pipe, seq);
	}
	else {
		seq->res.seg = 0;
		seq->res.cmd = HDLC_FRMR;
		seq->res.cmd |= HDLC_PF;
	}
}

static void req_frame_ui(PIPE *pipe, SEQ *seq) {

	if ((seq->req.pfb > 0) && (seq->req.seg == 0)) {	/*応答処理*/
		pipe->cmd = INDEX_UI;
		pipe->req.len = seq->inf;
		seq->res.len = 1;
		invRcvSnd(1, pipe, seq);
	}
	else {
		pipe->err = ER_VOID;
	}
}

static void res_frame_ui(PIPE *pipe, SEQ *seq) {

	ui16 len;

	if (!(0 == pipe->res.len)) {
		if (seq->res.len == 0) {		/*応答完了*/
			invRcvSnd(0, pipe, seq);	/*受信処理へ*/
		}
		else {
			seq->res.seg = 0;
			seq->res.cmd = HDLC_UI;
			seq->res.cmd |= HDLC_PF;
			len = pipe->res.len - pipe->res.pos;
			if (len <= seq->res.siz) {
				seq->res.buf = &pipe->res.buf[pipe->res.pos];
				pipe->res.pos += len;
				seq->inf = len;
			}
			else {
				pipe->res.pos = 0;
				pipe->res.len = 0; 
				pipe->err = ER_VOID;
			}
		}
	}
}
static void localCtrl(PIPE *pipe, SEQ *seq) {

	ui16 len;
	ui08 go;

	if (pipe->ctl > 0) {
		go = 0;
		if (seq->res.len == 0) {		/*応答完了*/
			invRcvSnd(0, pipe, seq);	/*受信処理へ*/
			seq->res.adr2.hf = 0;
		}
		else {
			local(pipe, seq);
			len = pipe->res.len - pipe->res.pos + 1;
			if (len <= seq->res.siz) {
				seq->res.buf = &pipe->res.buf[pipe->res.pos];
				pipe->res.pos += len;
				seq->inf = len;
				go = seq->inf > 0;	/*llcあり*/
			}
			else {
				pipe->res.pos = 0;
				pipe->res.len = 0; 
				pipe->err = ER_VOID;
			}
		}
		if (go > 0) {
			seq->res.len = seq->inf;
			seq->pos = 0;
		}
	}
	else {
		if (seq->cks != 0) {
			if (isBusy(pipe->prt) == FALSE) {	/*回路の排他*/
				seq->req.adr2.hf = 0xff;
				seq->res.adr2.hf = seq->req.adr2.hf;
				pipe->req.len = seq->inf;
				seq->res.len = 1;
				invRcvSnd(1, pipe, seq);
				seq->req.len = 0;
			}
			else {
				pipe->err = ER_VOID;
			}
		}
		else {
			pipe->err = ER_VOID;
		}
	}
}

static void basicCtrl(PIPE *pipe, SEQ *seq) {

	ui16 len;

	if (pipe->ctl>0) {	/*送信*/
		aplcmd();						/* ｺﾏﾝﾄﾞ機能	*/
		if (!(0 == pipe->res.len)) {
			if (pipe->res.len <= pipe->res.pos) {
				invRcvSnd(0, pipe, seq);
			}
			else {
				len = pipe->res.len - pipe->res.pos;
				seq->res.buf = &pipe->res.buf[pipe->res.pos];
				pipe->res.pos += len;
				seq->res.len = len;
				seq->pos = 0;
			}
		}
	}
	else {				/*受信*/
		if (seq->cks != 0) {
//@			if (sealed() != 2) {
				aplcmd_ini();
				ppip = pipe;
				pipe->req.len = seq->req.len;
				seq->req.len = 0;
				invRcvSnd(1, pipe, seq);
//@			}
//@			else {
//@				pipe->err = ER_MOD;
//@			}
		}
		else {
			pipe->err = ER_MOD;
		}
	}
}

static void wrapCtrl(PIPE *pipe, SEQ *seq) {
	ui16 len;
	ui08 go;
	ui16 num;

	if (pipe->ctl > 0) {
		go = 0;
		DLMS();
		cosem(pipe, seq);
		if (!(0 == pipe->res.len)) {
			if (seq->res.len == 0) {		/*応答完了*/
				invRcvSnd(0, pipe, seq);		/*受信処理へ*/
				seq->res.adr2.hf = 0;
			}
			else {
				len = pipe->res.len - pipe->res.pos;
				if (len > apdu_size) {
					seq->res.buf = pipe->res.buf;
					pipe->res.pos += apdu_size;
					seq->inf = apdu_size;
				}
				else {
					seq->res.buf = pipe->res.buf;
					pipe->res.pos += len;
					seq->inf = len;
					gbt.ctl.bt.last = 1;
				}
				if (stream > 0) {
					gbt_len = 0;
					num = UI16L(gbt.num);
					num++;
					if (stream == 2) {
						stream1_read(pipe->res.buf, seq->inf);
					}
					*((pui16)gbt.num) = UI16L(&num);
					encode_len(gbt.size, &gbt_len, seq->inf);
					gbt_len += 6;
					if (pipe->tsk != 0) {
						seq->tmr = 1000;
						pipe->tsk = 1;
					}
				}
				go = seq->inf > 0;	/*llcあり*/
			}
		}
		if (go > 0) {
			seq->res.len = 8;
			seq->pos = 0;
		}
	}
	else {
		if (seq->cks != 0) {
			gbt_len = 0;
			ramset(&gbt, 0, sizeof(GBT));
			gbt.cmd = CMD_GBT;
			if (isBusy(pipe->prt) == FALSE) {	/*回路の排他*/
				clear_inactivity_timeout(pipe->prt);
				seq->res.adr2.hf = seq->req.adr2.hf;
				pipe->req.len = seq->inf;
				seq->res.len = 1;
				invRcvSnd(1, pipe, seq);
				seq->req.len = 0;
			}
			else {
				pipe->err = ER_VOID;
			}
		}
		else {
			pipe->err = ER_VOID;
		}
	}
}

static void hdlcCtrl(PIPE *pipe, SEQ *seq) {
	DAT4 adr;
	ui08 tmp, go;

	if (pipe->ctl > 0) {
		go = 0;
		DLMS();
		cosem(pipe, seq);
		switch (pipe->cmd) {
		case INDEX_I:
			res_frame_i(pipe, seq);
			go = seq->inf > 0;	/*llcあり*/
			break;
		case INDEX_UA:
			res_frame_ua(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_UAD:
			res_frame_uad(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_BUSY:
			res_frame_busy(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_DM:
			res_frame_dm(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_RR:
			res_frame_rr(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_RNR:
			res_frame_rnr(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_UI:
			res_frame_ui(pipe, seq);
			go = seq->inf > 0;	/*llcあり*/
			break;
		case INDEX_FRMRw:
			res_frmr_w(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_FRMRx:
			res_frmr_x(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_FRMRy:
			res_frmr_y(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_FRMRz:
			res_frmr_z(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_FRMRb:
			res_frmr_b(pipe, seq);
			go = seq->res.len > 0;
			break;
		case INDEX_FRMR0:
			res_frmr_other(pipe, seq);
			go = seq->res.len > 0;
			break;
		default:
			break;
		};
		if (go > 0) {
			seq->res.len = 5 + (seq->ads & 0x0f);
			seq->res.len += ((seq->ads & 0xf0) >> 4);
			seq->pos = 0;
		}
	}
	else {
		go = 1;
		tmp = (seq->ads & 0xf0) >> 4;	/*クライアント*/
		if (tmp != 1) {
			go = 0;
		}
		else {
			adr.hf[0] = seq->req.adr2.by[0];
			adr.hf[0] >>= 1;
			adr.hf[0] &= 0x00f0;
			switch (adr.hf[0]) {
			case UNIT_PC: case UNIT_US: case UNIT_MR: case UNIT_PUSH: case UNIT_FW: case UNIT_IHD:
				break;
			default:
				go = 0;
				break;
			};
		}
		if (seq->cks == 0x0000) {
			clear_inactivity_timeout(pipe->prt);
			req_frame_unknown(pipe, seq);
			go = 0;
		}
		if (go > 0) {
			if (FRAME_SNRM(seq->req.cmd)) {
				clear_inactivity_timeout(pipe->prt);
				req_frame_snrm(pipe, seq);
			}
			else if (FRAME_DISC(seq->req.cmd)) {
				req_frame_disc(pipe, seq);
				set_non_associated(pipe->prt);
				if (pipe->prt == 1) {
					inact0_stop();
				}
				else {
					inact1_stop();
				}
			}
			else {
				if (seq->req.adr2.hf == 0) {
					clear_inactivity_timeout(pipe->prt);
					pipe->err = ER_ADR;
				}
				else {
					if (seq->res.adr2.hf != seq->req.adr2.hf) {
						clear_inactivity_timeout(pipe->prt);
						pipe->err = ER_ADR;
					}
					else {
#if 1
						/************************/
						if (seq->neg == 0) {
							if (seq->inf > 128) {
								if (seq->req.buf[0] != 0xe6) {
									seq->req.siz = 128;
								}
							}
						}
						/************************/
#endif
						if (seq->req.len > seq->req.siz) {
							req_frame_frmrb(pipe, seq);
						}
						else {
							if (FRAME_I(seq->req.cmd)) {
								clear_inactivity_timeout(pipe->prt);
								req_frame_i(pipe, seq);
							}
							else if (FRAME_RR(seq->req.cmd)) {
								clear_inactivity_timeout(pipe->prt);
								req_frame_rr(pipe, seq);
							}
							else if (FRAME_RNR(seq->req.cmd)) {
								clear_inactivity_timeout(pipe->prt);
								req_frame_rr(pipe, seq);
							}
							else if (FRAME_UI(seq->req.cmd)) {
								clear_inactivity_timeout(pipe->prt);
								req_frame_ui(pipe, seq);
							}
							else {
								req_frame_frmrw(pipe, seq);
							}
						}
					}
				}
			}
		}
		else {
			req_frame_unknown(pipe, seq);
		}
		seq->req.len = 0;
	}
}

static	void(*const modeCtrl[])(PIPE *pipe, SEQ *seq) = {
	wrapCtrl,
	localCtrl,
	basicCtrl,
	hdlcCtrl
};

static void blank_func(void) {

}

/********* 回路1用 **********/
static	void	req1_rst(void) {
	localouttxd(TXD_MRK);				/* 停止レベル */
	pipe1.cmd = 0;
	localrcvstr();						/* 受信動作許可				*/
	seq1.tmr = 1;				/*  8,  9 各種監視ﾀｲﾏｸﾘｱ		*/
	pipe1.tsk++;
}

static	void	req1_rcv(void) {
}

static	void	req1_end(void) {
	modeCtrl[seq1.mod](&pipe1, &seq1);
}

static	void(*const tbl_mr1[])(void) =	/* ﾒｲﾝ管理状態ﾃｰﾌﾞﾙ			*/
{
	req1_rst,
	req1_rcv,
	req1_end,
};

static	void	req1_int_rcv(void) {

	if (seq1.pos != 0x0000)	{				/* STX受信済み ?			*/
		seq1.tmr--;							/* 受信ﾌﾚｰﾑ限界ﾀｲﾏ更新		*/
		if (seq1.tmr == 0) {			/* 受信ﾌﾚｰﾑﾀｲﾏﾀｲﾑｱｯﾌﾟ ?		*/
			localrcvend();
			if (seq1.phs == 0xfe) {
				pipe1.tsk++;
			}
			else {
				pipe1.err = ER_RFRM;			/* 電文受信ｵｰﾊﾞｰ異常ｺｰﾄﾞｾｯﾄ	*/
			}
		}
	}
}

static	void(*const tbl_ir1[])(void) =		/* 2ms管理状態ﾃｰﾌﾞﾙ			*/
{
	blank_func,
	req1_int_rcv,
	blank_func,
};

static	void	res1_pool(void)
{
	modeCtrl[seq1.mod](&pipe1, &seq1);
	if (seq1.res.len > 1) {
		pipe1.tsk++;
	}
}

static	void	res1_start(void) {

	ui08 tmp;

	if (seq1.tmr < (RFRAM_TMB - seq1.oct)) {
		localsndstr();					/* 送信動作許可・開始		*/
		_sendBuf(&tmp, &seq1);
		localputsnd(tmp);
		pipe1.tsk++;
	}
}

static	void	res1_end(void)
{
	if (localgettxd() == TXD_OFF) {		/*RunOrStop*/
		localsndend();
		seq1.pos = 0;
		seq1.res.len = 0;
		modeCtrl[seq1.mod](&pipe1, &seq1);
		pipe1.cnt = 0;
		if (flash != flashQ) {
			powerlv = 200;				/* ソフトウェアリセット		*/
		}
	}
}

static	void(*const tbl_ms1[])(void) =	/* ﾒｲﾝ管理状態ﾃｰﾌﾞﾙ			*/
{
	res1_pool,
	res1_start,
	blank_func,
	res1_end,
};

static	void	res1_int_rst0(void) {

	if (seq1.tmr != 0) {
		seq1.tmr--;
		if (seq1.tmr == 0) {
			localsndend();
			pipe1.err = ER_VOID;
		}
	}
}

static	void	res1_int_rst1(void) {

	if (seq1.tmr != 0) {
		seq1.tmr--;
		if (seq1.tmr == 0) {
			localsndend();
			if (seq1.phs == 0xfe) {
				pipe1.tsk++;
			}
			else {
				pipe1.err = ER_VOID;
			}
		}
	}
}

static	void(*const tbl_is1[])(void) =	/* 2ms管理状態ﾃｰﾌﾞﾙ			*/
{
	blank_func,
	res1_int_rst0,
	res1_int_rst1,
	blank_func,
};

#pragma section text .int_rcvL
void	int_rcvL(void) 
{
	ui08	ret, rxbb;

	rxbb = localgetrcv();					/* 受信ﾃﾞｰﾀ読出し			*/
	pipe1.err = localrcverr();				/* 受信ﾃﾞｰﾀﾊｰﾄﾞ･ｴﾗｰ取得		*/
	if (pipe1.err == UE_NOT) {				/* ﾊｰﾄﾞ･ｴﾗｰなし ?			*/
		ret = _reciveBuf(&seq1, rxbb);
		switch (ret) {
		case 0:/*no error*/
			break;
		case END_SR:							/* Finish*/
			seq1.req.len = seq1.pos;
			localrcvend();						/* 受信動作禁止				*/
			pipe1.tsk++;
			break;
		default:
			pipe1.err = ret;
			localrcvend();						/* 受信動作禁止				*/
			break;
		}
	}
	else {
		localrcvend();							/* 受信動作禁止				*/
	}
}
#pragma section

#pragma section text .int_sndL
static void int_sndL(void)
{
	ui08	ret, txbb;

	ret = _sendBuf(&txbb, &seq1);
	if (ret == END_SR){
		pipe1.tsk++;
	}
	else {
		localputsnd(txbb);						/* 送信ﾃﾞｰﾀｾｯﾄ				*/
	}
}
#pragma section

#pragma section text .int_errL
static void int_errL(void)
{
	if (pipe1.err == NO_ERR) {		/* 割込みが遅い場合、エラーをクリアするため*/
		localgetrcv();				/* 受信ﾃﾞｰﾀ空読み			*/
		pipe1.err = localrcverr();	/* 受信ﾃﾞｰﾀﾊｰﾄﾞ･ｴﾗｰ取得		*/
		localrcvend();				/* 受信動作禁止				*/
	}
}
#pragma section 

void seq_send(cui16 len)
{
	ppip->res.len = len;
}

pui08 get_sndads(void) {
	return ppip->res.buf;
}

pui08 get_rcvads(void) {
	return ppip->req.buf;
}

void set_tsushin_error(cui08 err) {
	ppip->err = err;
}

ui16 get_rcnt(void) {
	return ppip->req.len;
}

static void	local_call1(void) {
	ui08 i;
	ui08 ret, bit;

	switch (pipe1.notify) {
	case 0:
		if (pipe2.notify == 0) {
			if (powerlv < 6) {
				set_non_associated(3);
				inact0_stop();
				inact1_stop();
				inact2_stop();
				seq1.res.adr2.hf = 0;	/*DISC*/
				seq2.res.adr2.hf = 0;	/*DISC*/
				seq3.res.adr2.hf = 0;	/*DISC*/
			}
			if (inactivity_timeout0()) {
				set_non_associated(1);
				inact0_stop();
				seq1.res.adr2.hf = 0;	/*DISC*/
			}
			if (gCalled1 != 0) {
				if (D_TIM_NOUP != inact0_check()) {
					if (D_TIM_NOUP != inact1_check()) {
						if (D_TIM_NOUP != inact2_check()) {
							for (i = 0, bit = 0x01; i < 8; i++, bit <<= 1) {
								if ((gCalled1 & bit) > 0) {
									gCall = i;
									break;
								}
							}
							pipe1.notify++;
						}
					}
				}
			}
		}
		break;

	case 1:
		if (pipe1.ctl == 0) {
			localrcvend();
			createCall(local_calling);
			for (i = 0; i < sizeof(local_calling); i++) {
				ret = _reciveBuf(&seq1, local_calling[i]);
				if (0 != ret) {
					seq1.req.len = seq1.pos;
					break;
				}
			}
			seq1.res.adr2.hf = seq1.req.adr2.hf;	/* SNRM */
			g_port0 = gCalling[gCall].port;
			i = sizeof(tbl_mr1) / 4;
			pipe1.tsk = i - 1;
			if (gCalling[gCall].inst != 0) {
				script_associate();
			}
			makecall = 0;
			pipe1.notify++;
		}
		break;

	case 2:
		if (seq1.phs == 0xfe) {
			pipe1.notify++;
		}
		else {
			makecall++;
			if (makecall > 1000) {
				invRcvSnd(0, &pipe1, &seq1);
				pipe1.notify++;
			}
		}
		break;

	case 3:
		if (gCalling[gCall].inst != 0) {
			set_non_associated(1);
			inact0_stop();
		}
		gCalled1 &= ~gCalling[gCall].active;
		gCalling[gCall].active = 0;
	default:
		seq1.res.adr2.hf = 0;	/*DISC*/
		pipe1.notify = 0;
		makecall = 0;
		break;
	}
}

void	seq1_main(void) {

	local_call1();
	if (pipe1.ctl > 0) {
		(*tbl_ms1[pipe1.tsk])();			/* 通信ﾀｽｸ分割処理			*/
	}
	else {
		(*tbl_mr1[pipe1.tsk])();			/* 通信ﾀｽｸ分割処理			*/
	}
	if (pipe1.err != NO_ERR) {
//		localrcvend();
//		localsndend();
		invRcvSnd(0, &pipe1, &seq1);
		pipe1.err = 0;
		pipe1.cnt++;
	}
}

void	seq1_xtal(void)
{
	if (pipe1.inv == 0) {
		if (pipe1.ctl > 0) {
			pipe1.tsk %= 4;
			(*tbl_is1[pipe1.tsk])();
		}
		else {
			pipe1.tsk %= 3;
			(*tbl_ir1[pipe1.tsk])();
		}
	}
}
/********* 回路2用 **********/
static	void	req2_rst(void) {
	uart0outtxd(TXD_MRK);
	pipe2.cmd = 0;
	uart0rcvstr();			/* 受信動作許可				*/
	seq2.tmr = MARK_TM2;		/*  8,  9 各種監視ﾀｲﾏｸﾘｱ		*/
	pipe2.tsk++;
}

static	void	req2_rcv(void) {
}

static	void	req2_end(void) {
	modeCtrl[seq2.mod](&pipe2, &seq2);
}

static	void(*const tbl_mr2[])(void) =		/* ﾒｲﾝ管理状態ﾃｰﾌﾞﾙ			*/
{
	req2_rst,
	req2_rcv,
	req2_end,
};

static	void	req2_int_rcv(void)
{
	if (seq2.pos != 0x0000)	{			/* STX受信済み ?			*/
		seq2.tmr--;						/* 受信ﾌﾚｰﾑ限界ﾀｲﾏ更新		*/
		if (seq2.tmr == 0x0000) {		/* 受信ﾌﾚｰﾑﾀｲﾏﾀｲﾑｱｯﾌﾟ ?		*/
			uart0rcvend();
			if (seq2.phs == 0xff) {
				pipe2.tsk++;
			}
			else {
				pipe2.err = ER_RFRM;		/* 電文受信ｵｰﾊﾞｰ異常ｺｰﾄﾞｾｯﾄ	*/
			}
		}
	}
}

static	void(* const tbl_ir2[])(void) =	/* 2ms管理状態ﾃｰﾌﾞﾙ			*/
{
	blank_func,
	req2_int_rcv,
	blank_func,
};

static	void	res2_pool(void)
{
	modeCtrl[seq2.mod](&pipe2, &seq2);
	if (seq2.res.len > 1) {
		pipe2.tsk++;
	}
}

static	void	res2_start(void) {

	ui08 tmp;

	if (seq2.tmr < (RFRAM_TMB - seq2.oct)) {
		uart0sndstr();					/* 送信動作許可・開始		*/
		_sendBuf(&tmp, &seq2);
		uart0putsnd(tmp);
		pipe2.tsk++;
	}
}

static	void	res2_end(void)
{
	if (uart0gettxd() == TXD_OFF) {
		uart0sndend();
		seq2.pos = 0;
		seq2.res.len = 0;
		if (pipe2.notify == 0) {
			if ((seq2.mod == 0) || (seq2.mod == 3)) {
				pipe2.cnt = 0;
				nicsts.bt.sts = 0;
			}
		}
		modeCtrl[seq2.mod](&pipe2, &seq2);
		if (flash != flashQ) {
			powerlv = 200;				/* ソフトウェアリセット		*/
		}
	}
}

static	void(* const tbl_ms2[])(void) =	/* ﾒｲﾝ管理状態ﾃｰﾌﾞﾙ			*/
{
	res2_pool,
	res2_start,
	blank_func,
	res2_end,
};

static	void	res2_int_rst0(void) {

	if (seq2.tmr != 0) {
		seq2.tmr--;
		if (seq2.tmr == 0) {
			uart0sndend();
			pipe2.err = ER_VOID;
		}
	}
}

static	void	res2_int_rst1(void) {

	if (seq2.tmr != 0) {
		seq2.tmr--;
		if (seq2.tmr == 0) {
			uart0sndend();
			if (seq2.phs == 0xfe) {
				pipe2.tsk++;
			}
			else {
				pipe2.err = ER_VOID;
			}
		}
	}
}

static	void(* const tbl_is2[])(void) =	/* 2ms管理状態ﾃｰﾌﾞﾙ			*/
{
	blank_func,
	res2_int_rst0,
	res2_int_rst1,
	blank_func,
};

#pragma section text .int_rcvR
void	int_rcvR(void)
{
	ui08	ret, rxbb;

	rxbb = uart0getrcv();					/* 受信ﾃﾞｰﾀ読出し			*/
	pipe2.err = uart0rcverr();			/* 受信ﾃﾞｰﾀﾊｰﾄﾞ･ｴﾗｰ取得		*/
	if (pipe2.err == UE_NOT) {				/* ﾊｰﾄﾞ･ｴﾗｰなし ?			*/
		ret = _reciveBuf(&seq2, rxbb);
		switch (ret) {
		case 0:/*no error*/
			break;
		case END_SR:/* Finish*/
			seq2.req.len = seq2.pos;
			uart0rcvend();				/* 受信動作禁止				*/
			pipe2.tsk++;
			break;
		default:
			pipe2.err = ret;
			uart0rcvend();				/* 受信動作禁止				*/
			break;
		}
	}
	else {
		uart0rcvend();				/* 受信動作禁止				*/
	}
}
#pragma section

#pragma section text .int_sndR
static void int_sndR(void) {
	ui08	ret, txbb;

	ret = _sendBuf(&txbb, &seq2);
	if (ret == END_SR) {					/* Finish*/
		pipe2.tsk++;
	}
	else {
		uart0putsnd(txbb);					/* 送信ﾃﾞｰﾀｾｯﾄ				*/
	}
}
#pragma section

#pragma section text .int_errR
static void int_errR(void)
{
	if (pipe2.err == NO_ERR) {
		uart0getrcv();					/* 受信ﾃﾞｰﾀ空読み			*/
		pipe2.err = uart0rcverr();		/* 受信ﾃﾞｰﾀﾊｰﾄﾞ･ｴﾗｰ取得		*/
		uart0rcvend();					/* 受信動作禁止				*/
	}
}
#pragma section

static void	local_call2(void) {
	ui08 i;
	ui08 ret, bit;

	switch (pipe2.notify) {
	case 0:
		if (pipe1.notify == 0) {
			if (powerlv < 6) {
				set_non_associated(3);
				inact0_stop();
				inact1_stop();
				inact2_stop();
				seq1.res.adr2.hf = 0;	/*DISC*/
				seq2.res.adr2.hf = 0;	/*DISC*/
				seq3.res.adr2.hf = 0;	/*DISC*/
			}
			if (inactivity_timeout1()) {
				set_non_associated(2);
				inact1_stop();
				seq2.res.adr2.hf = 0;
			}
			if (gCalled2 != 0) {
				if (D_TIM_NOUP != inact0_check()) {
					if (D_TIM_NOUP != inact1_check()) {
						if (D_TIM_NOUP != inact2_check()) {
							for (i = 0, bit = 0x01; i < 8; i++, bit <<= 1) {
								if ((gCalled2 & bit) > 0) {
									if (powerlv < 6) {
										gCall = i;
										pipe2.notify++;
										break;
									}
									else {
										if (tm_push[i].bt.drv == 0) {
											gCall = i;
											pipe2.notify++;
											break;
										}
										else {
											if (tm_push[i].bt.up == 1) {
												gCall = i;
												pipe2.notify++;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;

	case 1:
		if (pipe2.ctl == 0) {
			uart0rcvend();
			createCall(local_calling);
			for (i = 0; i < sizeof(local_calling); i++) {
				ret = _reciveBuf(&seq2, local_calling[i]);
				if (0 != ret) {
					seq2.req.len = seq2.pos;
					break;
				}
			}
			seq2.res.adr2.hf = seq2.req.adr2.hf;
			g_port0 = gCalling[gCall].port;
			i = sizeof(tbl_mr2) / 4;
			pipe2.tsk = i - 1;
			if (gCalling[gCall].inst != 0) {
				script_associate();
			}
			makecall = 0;
			pipe2.notify++;
		}
		break;

	case 2:
		if (seq2.phs == 0xfe) {
			pipe2.notify++;
		}
		else {
			makecall++;
			if (makecall > 1000) {
				invRcvSnd(0, &pipe2, &seq2);
				pipe2.notify++;
			}
		}
		break;

	case 3:
		if (gCalling[gCall].inst != 0) {
			set_non_associated(2);
			inact1_stop();
		}
		gCalled2 &= ~gCalling[gCall].active;
		gCalling[gCall].active = 0;
		set_timer2(&tm_interval, 10);
		makecall = 0;
		pipe2.notify++;
		break;
	default:
		makecall++;
		if ((tm_interval.bt.up == 1) || (makecall > 10)) {
			seq2.res.adr2.hf = 0;
			pipe2.notify = 0;
			makecall = 0;
			tm_interval.hf = 0;
		}
		break;
	}
}

/************ extern *************************/
void	seq2_main(void)
{
	local_call2();
	if (pipe2.ctl > 0) {
		pipe2.tsk %= 4;
		(*tbl_ms2[pipe2.tsk])();			/* 通信ﾀｽｸ分割処理	*/
	}
	else {
		pipe2.tsk %= 3;
		(*tbl_mr2[pipe2.tsk])();			/* 通信ﾀｽｸ分割処理	*/
	}
	if (pipe2.err != NO_ERR) {
//		uart0rcvend();
//		uart0sndend();
		if ((seq2.mod == 0) || (seq2.mod == 3)) {
			pipe2.cnt++;
		}
		invRcvSnd(0, &pipe2, &seq2);
		pipe2.err = 0;
	}
}

void	seq2_xtal(void)
{
	if (pipe2.inv == 0) {
		if (pipe2.ctl > 0) {
			(*tbl_is2[pipe2.tsk])();
		}
		else {
			(*tbl_ir2[pipe2.tsk])();
		}
	}
}

/********* 回路3用 **********/
static	void	req3_rst(void) {
	uart1outtxd(TXD_MRK);
	pipe3.cmd = 0;
	uart1rcvstr();			/* 受信動作許可				*/
	seq3.tmr = MARK_TM2;		/*  8,  9 各種監視ﾀｲﾏｸﾘｱ		*/
	pipe3.tsk++;
}

static	void	req3_rcv(void) {
}

static	void	req3_end(void) {
	modeCtrl[seq3.mod](&pipe3, &seq3);
}

static	void(* const tbl_mr3[])(void) =		/* ﾒｲﾝ管理状態ﾃｰﾌﾞﾙ			*/
{
	req3_rst,
	req3_rcv,
	req3_end,
};

static	void	req3_int_rcv(void)
{
	if (seq3.pos != 0x0000) {			/* STX受信済み ?			*/
		seq3.tmr--;						/* 受信ﾌﾚｰﾑ限界ﾀｲﾏ更新		*/
		if (seq3.tmr == 0x0000)			/* 受信ﾌﾚｰﾑﾀｲﾏﾀｲﾑｱｯﾌﾟ ?		*/
		{
			uart1rcvend();
			pipe3.err = ER_RFRM;		/* 電文受信ｵｰﾊﾞｰ異常ｺｰﾄﾞｾｯﾄ	*/
		}
	}
}

static	void(* const tbl_ir3[])(void) =	/* 2ms管理状態ﾃｰﾌﾞﾙ			*/
{
	blank_func,
	req3_int_rcv,
	blank_func,
};

static	void	res3_pool(void)
{
	modeCtrl[seq3.mod](&pipe3, &seq3);
	if (seq3.res.len > 1) {
		pipe3.tsk++;
	}
}

static	void	res3_start(void) {

	ui08 tmp;

	if (seq3.tmr < (RFRAM_TMB - seq3.oct)) {
		uart1sndstr();					/* 送信動作許可・開始		*/
		_sendBuf(&tmp, &seq3);
		uart1putsnd(tmp);
		pipe3.tsk++;
	}
}

static	void	res3_end(void)
{
	if (uart1gettxd() == TXD_OFF) {
		uart1sndend();
		seq3.pos = 0;
		seq3.res.len = 0;
		modeCtrl[seq3.mod](&pipe3, &seq3);
		if (flash != flashQ) {
			flash = flashQ;
			powerlv = 200;						/* ソフトウェアリセット		*/
		}
	}
}

static	void(* const tbl_ms3[])(void) =	/* ﾒｲﾝ管理状態ﾃｰﾌﾞﾙ			*/
{
	res3_pool,
	res3_start,
	blank_func,
	res3_end,
};

static	void	res3_int_rst0(void) {

	if (seq3.tmr != 0x0000) {
		seq3.tmr--;
		if (seq3.tmr == 0) {
			uart1sndend();
			pipe3.err = ER_VOID;
		}
	}
}

static	void	res3_int_rst1(void) {

	if (seq3.tmr != 0x0000) {
		seq3.tmr--;
		if (seq3.tmr == 0) {
			uart1sndend();
			if (seq3.phs == 0xff) {
				pipe3.tsk++;
			}
			else {
				pipe3.err = ER_VOID;
			}
		}
	}
}

static	void(* const tbl_is3[])(void) =	/* 2ms管理状態ﾃｰﾌﾞﾙ			*/
{
	res3_int_rst0,
	res3_int_rst0,
	res3_int_rst1,
	blank_func,
};

#pragma section text .int_rcvB
void	int_rcvB(void)
{
	ui08	ret, rxbb;

	rxbb = uart1getrcv();					/* 受信ﾃﾞｰﾀ読出し			*/
	pipe3.err = uart1rcverr();			/* 受信ﾃﾞｰﾀﾊｰﾄﾞ･ｴﾗｰ取得		*/
	if (pipe3.err == UE_NOT)				/* ﾊｰﾄﾞ･ｴﾗｰなし ?			*/
	{
		ret = _reciveBuf(&seq3, rxbb);
		switch (ret) {
		case 0:/*no error*/
			break;
		case END_SR:/* Finish*/
			seq3.req.len = seq3.pos;
			uart1rcvend();				/* 受信動作禁止				*/
			pipe3.tsk++;
			break;
		default:
			pipe3.err = ret;
			uart1rcvend();				/* 受信動作禁止				*/
			break;
		}
	}
	else {
		uart1rcvend();				/* 受信動作禁止				*/
	}
}
#pragma section

#pragma section text .int_sndB
static void int_sndB(void) {
	ui08	ret, txbb;

	ret = _sendBuf(&txbb, &seq3);
	if (ret == END_SR) {					/* Finish*/
		pipe3.tsk++;
	}
	else {
		uart1putsnd(txbb);					/* 送信ﾃﾞｰﾀｾｯﾄ				*/
	}
}
#pragma section

#pragma section text .int_errB
static void int_errB(void)
{
	if (pipe3.err == NO_ERR) {
		uart1getrcv();					/* 受信ﾃﾞｰﾀ空読み			*/
		pipe3.err = uart1rcverr();		/* 受信ﾃﾞｰﾀﾊｰﾄﾞ･ｴﾗｰ取得		*/
		uart1rcvend();					/* 受信動作禁止				*/
	}
}
#pragma section

/************ extern *************************/
void	seq3_main(void)
{
	if (pipe3.ctl > 0) {
		pipe3.tsk %= 4;
		(*tbl_ms3[pipe3.tsk])();			/* 通信ﾀｽｸ分割処理	*/
	}
	else {
		pipe3.tsk %= 3;
		(*tbl_mr3[pipe3.tsk])();			/* 通信ﾀｽｸ分割処理	*/
	}
	if (pipe3.err != NO_ERR) {
//		uart0rcvend();
//		uart0sndend();
		invRcvSnd(0, &pipe3, &seq3);
		pipe3.err = 0;
	}
}

void	seq3_xtal(void)
{
	if (pipe3.inv == 0) {
		if (pipe3.ctl > 0) {
			(*tbl_is3[pipe3.tsk])();
		}
		else {
			(*tbl_ir3[pipe3.tsk])();
		}
	}
}

//extern ui08 I(64, 2, security_mr);
void tsushin_ini(void)
{
	ALLOCBUF(0);
	max_res_len = 0;
	//	I(64, 2, security_mr) = SECURITY_POLICY_NE;

	ramset((pui08)&seq1, 0, sizeof(SEQ));
	seq1.req.buf = req_buf;
	seq1.res.buf = res_buf1;
	seq1.req.adr2.hf = 0;
	seq1.res.adr2.hf = 0;
	getNegoParam1(&seq1.res.siz, &seq1.req.siz, &seq1.res.maxW, &seq1.req.maxW);/*回路1設定初期化*/
	getTimeout1(&seq1.oct, &seq1.act);

	ramset((pui08)&pipe1, 0, sizeof(PIPE));	/*回路1初期化*/
	pipe1.prt = 1;					/*回路番号定義*/
	pipe1.sta = STATE_WAIT;		/*状態初期化*/
	pipe1.req.buf = req_buf;
	pipe1.res.buf = res_buf1;

	ramset((pui08)&seq2, 0, sizeof(SEQ));
	seq2.req.buf = req_buf;
	seq2.res.buf = res_buf2;
	seq2.req.adr2.hf = 0;
	seq2.res.adr2.hf = 0;
	getNegoParam2(&seq2.res.siz, &seq2.req.siz, &seq2.res.maxW, &seq2.req.maxW);/*回路2設定初期化*/
	getTimeout2(&seq2.oct, &seq2.act);

	ramset((pui08)&pipe2, 0, sizeof(PIPE));	/*回路2初期化*/
	pipe2.prt = 2;					/*回路番号定義*/
	pipe2.sta = STATE_WAIT;			/*状態初期化*/
	pipe2.req.buf = req_buf;
	pipe2.res.buf = res_buf2;

	ramset((pui08)&seq3, 0, sizeof(SEQ));
	seq3.req.buf = req_buf;
	seq3.res.buf = res_buf3;
	seq3.req.adr2.hf = 0;
	seq3.res.adr2.hf = 0;
	getNegoParam2(&seq3.res.siz, &seq3.req.siz, &seq3.res.maxW, &seq3.req.maxW);
	getTimeout2(&seq3.oct, &seq3.act);

	ramset((pui08)&pipe3, 0, sizeof(PIPE));	/*回路2初期化*/
	pipe3.prt = 3;					/*回路番号定義*/
	pipe3.sta = STATE_WAIT;			/*状態初期化*/
	pipe3.req.buf = req_buf;
	pipe3.res.buf = res_buf3;

	mid_req_len = 0;				/*hdlc要求フレーム長初期化*/
	mid_res_len = 0;				/*hdlc応答フレーム長初期化*/
	g_xdlms_res = ALLOCBUF(LEN_XDLMS_RES);
	g_xdlms_io = ALLOCBUF(MAX_XDLMS_BUF);
	cosem_res_clear();				/*COSEM応答処理初期化*/
	init_CONF();
	init_DLMS();
	init_ACSE();					/*ACSE初期化*/
	init_SECURITY();				/*SECURITY初期化*/
	init_xDLMS();					/*XDLMS初期化*/
	inact0_stop();					/* DLMSﾀｲﾏｰ 初期設定		*/
	inact1_stop();					/* DLMSﾀｲﾏｰ 初期設定		*/
	inact2_stop();					/* DLMSﾀｲﾏｰ 初期設定		*/
	tm_interval.hf = 0;
}

ui08 pushing(void) {

	ui08 ret = 0;

	if (pipe2.notify > 0) {
		ret = 1;
	}
	else {
		ret = 0;
	}
	return ret;
}

void tsushin(void)
{
	seq1_main();				/* 通信制御	*/
	if (pipe1.ctl == 0) {
		if (pipe1.tsk == 0) {
			seq1_main();		/* 通信制御	*/
		}
	}
	seq2_main();				/* 通信制御	*/
	if (pipe2.ctl == 0) {
		if (pipe2.tsk == 0) {
			seq2_main();		/* 通信制御	*/
		}
	}
	seq3_main();				/* 通信制御	*/
	if (pipe3.ctl == 0) {
		if (pipe3.tsk == 0) {
			seq3_main();		/* 通信制御	*/
		}
	}
}
/* end of tsushin.c */
