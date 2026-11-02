/****************************************************************************/
/*																			*/
/* 	システム名			:	海外(タイ)向け計器							*/
/*																			*/
/*	サブシステム名		:	F5L										*/
/*																			*/
/*	モジュール名		:	アプリケーション層：計器情報					*/
/*	ファイル名			:	aplkeikiinf.c									*/
/*																			*/
/*	機能				:	計器情報機能									*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
/*  拡張機能宣言			*/

/*  ﾕｰｻﾞｰ･ｲﾝｸﾙｰﾄﾞ･ﾌｧｲﾙ定義  */
#include	"def.h"								/* define宣言		-		*/
#include	"data.h"
#include	"ram.h"								/* define宣言				*/
#include	"rom.h"								/* define宣言				*/
#include	"flash.h"								/* define宣言				*/
#include	"devport.h"							/* define宣言				*/
#include	"devintr.h"							/* define宣言				*/
#include	"devrtc.h"							/* define宣言				*/
#include	"devpwr.h"							/* define宣言				*/
#include	"hrdkeilsi.h"						/* define宣言				*/
#include	"hrdrtc.h"							/* define宣言				*/
#include	"hrdjiki.h"							/* define宣言				*/
#include	"hrdeepctl.h"						/* define宣言				*/
#include	"aplkeikiinf.h"						/* define宣言				*/
#include	"apleeptask.h"						/* define宣言				*/
#include	"aplevent.h"						/* define宣言				*/
#include	"aplkeiryo.h"						/* define宣言				*/
#include	"aplrecord.h"						/* define宣言				*/
#include	"apltokei.h"						/* define宣言				*/
#include	"apllcddsp.h"						/* define宣言				*/
#include	"sub.h"								/* define宣言				*/
#include	"start.h"							/* define宣言				*/
#include	"dlms.h"
#include	"conf.h"
#include	"xdlms.h"

extern INST CONST g_ist[];
extern ui16 CONST COMINF[];
extern ui08	arr_pos;
extern ui08	array_count[10];

ui08 othinf_index(const eCOMID ID) {

	ui08 ret;
	switch (ID) {
	case eGPRS:
		ret = 1;
		break;
	case ePORT:
		ret = 2;
		break;
	case ePush1a:
	case ePush2a:
	case ePush3a:
	case ePush4a:
	case ePush5a:
	case ePush6a:
	case ePush7a:
		ret = 3 * (ID / 10) + 0;
		break;
	case ePush1b:
	case ePush2b:
	case ePush3b:
	case ePush4b:
	case ePush5b:
	case ePush6b:
	case ePush7b:
		ret = 3 * (ID / 10) + 1;
		break;
	case ePush1c:
	case ePush2c:
	case ePush3c:
	case ePush4c:
	case ePush5c:
	case ePush6c:
	case ePush7c:
		ret = 3 * (ID / 10) + 2;
		break;
	case eCID1:
	case eCID2:
	case eCID3:
	case eCID4:
	case eCID5:
	case eCID6:
	case eCID7:
	case eCID8:
	case eCID9:
	case eCID10:
		ret = ID - 90;
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

ui08 othinf_defval(const eCOMID ID) {

	ui08 ret;

	switch (ID) {
	default:
	case eGPRS:
	case ePORT:
		ret = 0x30;
		break;
	case eCID2:
		ret = 0;
		break;
	case eCID1:
	case eCID3:
	case eCID4:
	case eCID5:
	case eCID6:
	case eCID7:
	case eCID8:
	case eCID9:
		ret = 0x20;
		break;
	}
	return ret;
}

ui08 cominf_defsize(const eCOMID ID) {

	ui08 ret;

	switch (ID) {
	case 1:
		ret = 32;
		break;
	case 2:
		ret = 4;
		break;
	case ePush1a:
	case ePush2a:
	case ePush3a:
	case ePush4a:
	case ePush5a:
	case ePush6a:
	case ePush7a:
		ret = 16;
		break;
	case ePush1b:
	case ePush2b:
	case ePush3b:
	case ePush4b:
	case ePush5b:
	case ePush6b:
	case ePush7b:
		ret = 4;
		break;
	case ePush1c:
	case ePush2c:
	case ePush3c:
	case ePush4c:
	case ePush5c:
	case ePush6c:
	case ePush7c:
		ret = 2;
		break;
	case eCID2:
		ret = 3;
		break;
	case eCID6:
		ret = 20;
		break;
	default:
		ret = 2;
	}
	ret += 2;
	return ret;
}

ui08 NIC_RSSI(void) {

	ui08 rssi = comsts.strength;

	abs08(&rssi);

	return rssi;
}

void init_othinf(const eCOMID id) {

	ui08	index, ini;
	OTHIF	dat;

	index = othinf_index(id);
	if (index > 0) {
		ini = othinf_defval(id);
		ramset((pui08)&dat, ini, sizeof(OTHIF));
		dat.size = cominf_defsize(id);
		switch (ini) {
		case 0:
			switch (dat.size) {
			default:
			case 2:
				dat.value[0] = DT_UNSIGNED;
				break;
			case 3:
				dat.value[0] = DT_LONG_UNSIGNED;
				break;
			case 5:
				dat.value[0] = DT_DOUBLE_LONG_UNSIGNED;
				break;
			}
			break;
		default:
		case 0x20:
		case 0x30:
			dat.value[0] = DT_OCTET_STRING;
			dat.value[1] = dat.size - 2;
			break;
		}
		eepwrite(COMINF[index], sizeof(OTHIF), (pui08)&dat);
	}
}

ui08 read_othinf(pui08 value, const eCOMID id, cui08 offset) {

	ui08 ret, index, ini;
	OTHIF dat;

	ret = 0;
	index = othinf_index(id);
	if (index > 0) {
		if (EEP_OK == eepread(COMINF[index], sizeof(OTHIF), &dat)) {
			if (id != dat.id) {
				ini = othinf_defval(id);
				ramset((pui08)&dat, ini, sizeof(OTHIF));
				dat.size = cominf_defsize(id);
				switch (ini) {
				case 0:
					switch (dat.size) {
					default:
					case 2:
						dat.value[0] = DT_UNSIGNED;
						break;
					case 3:
						dat.value[0] = DT_LONG_UNSIGNED;
						break;
					case 5:
						dat.value[0] = DT_DOUBLE_LONG_UNSIGNED;
						break;
					}
					break;
				default:
				case 0x20:
				case 0x30:
					dat.value[0] = DT_OCTET_STRING;
					dat.value[1] = dat.size - 2;
					break;
				}
			}
			if (dat.size >= offset) {
				ret = ramcpy(value, &dat.value[offset], dat.size - offset);
			}
		}
	}
	return ret;
}

ui08 write_othinf(const eCOMID id, cui08 length, cpui08 value) {

	ui08 index, ini, len;
	ui08 ret;
	OTHIF dat;

	ret = 0;
	index  = othinf_index(id);
	if (index > 0) {
		ini = othinf_defval(id);
		ramset(&dat, ini, sizeof(OTHIF));
		dat.id = id;
		len = length;
		if (len == 0) {
			switch (value[0]) {	/* type */
			case 3: case 15: case 17: case 22:
				len = 1 + 1;
				break;
			case 5: case 6: case 7:case 23:
				len = 4 + 1;
				break;
			case 16: case 18:
				len = 2 + 1;
				break;
			case 20: case 21:case 24:
				len = 8 + 1;
				break;
			case 9: case 10: case 11: case 12:
				len = value[1] + 2;
				break;
			case 25:
				len = 12 + 1;
				break;
			case 26:
				len = 5 + 1;
				break;
			case 27:
				len = 4 + 1;
				break;
			default:
				len = 0;
				break;
			}
		}
		dat.size = sizeof(OTHIF) - 2;
		if (len < dat.size) {
			dat.size = len;
		}
		ramcpy(dat.value, value, dat.size);
		ret = eepwrite(COMINF[index], sizeof(OTHIF), (pui08)&dat);
		if (ret == EEP_OK) {
			ret = dat.size;
		}
		else {
			ret = 0;
		}
	}
	return ret;
}

void get_othinf(pui08 out, uint* offset, const eCOMID id) {

	if (0 != othinf_index(id)) {
		out[(*offset)++] = 0x16;
		out[(*offset)++] = id;
		*offset += read_othinf(&out[*offset], id, 0);
	}
}

ui08 init_all_othinf(void) {

	ui08 z, ret, ok;
	OTHIF dat;

	nicsts.wd = 0;
	ret = 1;
	ramset(&dat, 0, sizeof(OTHIF));
	for (z = 0; z < 10; z++) {
		ok = eepwrite(COMINF[z], sizeof(OTHIF), &dat);
		if (ok != EEP_OK) {
			ret = 0;
			break;
		}
	}
	return ret;
}
/***********************/

/*
	1: every byo
	2: every fun
	3: every ji
	4: every day
	5: every month
	6: every year
*/
/* RTC    DLMS
	0 san -> 7
	1 mo -> 1
	2 tu -> 2
	3 we -> 3
	4 th -> 4
	5 fr -> 5
	6 sa -> 6

*/
void init_call(void) {
	gCall = 0xff;
	gCalled1 = 0;
	gCalled2 = 0;
	ramset(&gCalling, 0, sizeof(gCalling));
}


void call_cominf(cui08 ord, cui08 arg) {

	if (ord == 0) {
		if (gCalled2 == 0) {
			gCalling[0].port = 2;
			gCalling[0].inst = 0;
			gCalling[0].order = ord;
			gCalling[0].userid = 0;
			gCalling[0].arg = arg;
			gCalling[0].active = 0x01;
			gCalled2 |= gCalling[0].active;
		}
	}
	else {
		if (D_TIM_NOUP != inact1_check()) {
			if (gCalled2 == 0) {
				gCalling[0].port = 2;
				gCalling[0].inst = 0;
				gCalling[0].order = ord;
				gCalling[0].userid = 0;
				gCalling[0].arg = arg;
				gCalling[0].active = 0x01;
				gCalled2 |= gCalling[0].active;
			}
		}
	}
}

void call_script(cui16 inst, cui16 arg) {

	ui08 i, bit;

	for (i = 0, bit = 1; i < (sizeof(gCalling) / sizeof(CALLING)); i++, bit <<= 1) {
		if (gCalling[i].active == 0) {
			gCalling[i].userid = 1;
			gCalling[i].port = 1;
			gCalling[i].inst = inst;
			gCalling[i].arg = arg;
			gCalling[i].order = CMD_VOID_CALL;
			gCalling[i].active = bit;
			gCalled1 |= bit;
			break;
		}
	}
}

void call_with_push(cui16 inst, cui16 arg, cui16 rand) {

	ui08 i, bit;
	for (i = 0, bit = 1; i < (sizeof(gCalling) / sizeof(CALLING)); i++, bit <<= 1) {
		if (gCalling[i].active == 0) {
			gCalling[i].userid = 4;
			gCalling[i].port = 2;
			gCalling[i].inst = inst;
			gCalling[i].arg = arg;
			gCalling[i].order = CMD_NOTIFICATION_CALL;
			gCalling[i].active = bit;
			gCalled2 |= bit;
			if (rand > 0) {
				set_timer2(&tm_push[i], rand);
			}
			else {
				tm_push[i].hf = 0;
			}
			break;
		}
	}
}

void call_with_event_push(cui16 rand) {

	ui08 i, bit;
	for (i = 0, bit = 1; i < (sizeof(gCalling) / sizeof(CALLING)); i++, bit <<= 1) {
		if (gCalling[i].active == 0) {
			gCalling[i].userid = 4;
			gCalling[i].port = 2;
			gCalling[i].inst = IST_PUSH_SCR;
			gCalling[i].arg = 5;
			gCalling[i].order = CMD_NOTIFICATION_CALL;
			gCalling[i].active = bit;
			gCalled2 |= bit;
			if (rand > 0) {
				set_timer2(&tm_push[i], rand);
			}
			else {
				tm_push[i].hf = 0;
			}
			break;
		}
	}
}

ui16 wrrd_mem_count(const ui32 size) {

	ui16 count;

	count = size / memrd.upper.hf;
	if ((size%memrd.upper.hf) > 0) {
		count++;
	}
	return count;
}

ui32 wrrd_ram_init(const ui32 from, const ui32 to) {

	ui32 addr0, addr1, f, t;

	addr0 = 0x000fbf00;
	addr1 = 0x000fff00 - 1;

	if (from < addr0) {
		f = addr0;
	}
	else {
		f = from;
	}
	if (to > addr1 || to == 0) {
		t = addr1;
	}
	else {
		t = to;
	}
	if (t < f) {
		t = f;
	}
	memrd.adr.wd = f;
	memrd.siz.wd = t - f + 1;
	memrd.cur.wd = 0;
	
	return memrd.siz.wd;
}

ui32 wrrd_e2p_init(const ui32 from, const ui32 to) {

	ui32 addr0, addr1, f, t;

	addr0 = 0x00000000;
	addr1 = 0x000C0000 - 1;

	if (from < addr0) {
		f = addr0;
	}
	else {
		f = from;
	}
	if (to > addr1 || to == 0) {
		t = addr1;
	}
	else {
		t = to;
	}
	if (t < f) {
		t = f;
	}
	memrd.adr.wd = f;
	memrd.siz.wd = t - f + 1;
	memrd.cur.wd = 0;
	memrd.stock.hf = 0;

	return memrd.siz.wd;

}

ui32 wrrd_rom_init(const ui32 from, const ui32 to) {

	ui32 addr0, addr1, f, t;

	addr0 = 0x00000000;
	addr1 = 0x00080000 - 1;

	if (from < addr0) {
		f = addr0;
	}
	else {
		f = from;
	}
	if (to > addr1 || to == 0) {
		t = addr1;
	}
	else {
		t = to;
	}
	if (t < f) {
		t = f;
	}
	memrd.adr.wd = f;
	memrd.siz.wd = t - f + 1;
	memrd.cur.wd = 0;

	return memrd.siz.wd;
}

ui08 read_mem(pui08 out, ui16 *size) {

	ui08 ret;
	ui16 z;
	ui32 remain;
	ui08 __far *mem;

	remain = memrd.siz.wd - memrd.cur.wd;
	if (remain > memrd.upper.hf) {
		*size = memrd.upper.hf;
		ret = 1;	/*まだ続く*/
	}
	else {
		*size = remain;
		ret = 0;
	}

	mem = (ui08 __far *)(memrd.adr.wd + memrd.cur.wd);
	for (z = 0; z < *size; z++) {
		*out++ = *mem++;
	}
	memrd.cur.wd += *size;
	return ret;
}

ui08 write_ram(cpui08 in, ui16 *size) {

	ui08 ret;
	ui16 z;
	ui32 remain;
	pui08f mem;

	remain = memrd.siz.wd - memrd.cur.wd;
	if (remain > memrd.upper.hf) {
		*size = memrd.upper.hf;
		ret = 1;	/*まだ続く*/
	}
	else {
		*size = remain;
		ret = 0;
	}
	mem = (pui08f)(memrd.adr.wd + memrd.cur.wd);
	for (z = 0; z < *size; z++) {
		*mem++ = *in++;
	}
	memrd.cur.wd += *size;
	return ret;
}

ui08 read_e2p(pui08 out, ui16 *size) {

	ui08 ret,res;
	ui32 remain;

	/*送れるサイズの計算*/
	remain = memrd.upper.hf - memrd.stock.hf;
	if (remain > ONETIME) {
		*size = ONETIME;
		ret = 2;
	}
	else {	/*次の送信で今回は最後*/
		*size = remain;
		ret = 1;
	}
	/*残りサイズの計算*/
	remain = memrd.siz.wd - memrd.cur.wd;
	if (remain < *size) {
		*size = remain;
		ret = 0;
	}
	res = eepread(memrd.adr.wd + memrd.cur.wd, *size, out);
	if(res == EEP_OK){
		memrd.cur.wd += *size;
		if (ret == 2) {
			memrd.stock.hf += *size;
		}
		else {
			memrd.stock.hf = 0;
		}
	}
	else {
		ret = 0xff;
	}
	return ret;
}

ui08 write_e2p(cpui08 in, ui16 *size) {

	ui08 ret, res;
	ui32 remain;

	/*送れるサイズの計算*/
	remain = memrd.upper.hf - memrd.stock.hf;
	if (remain > ONETIME) {
		*size = ONETIME;
		ret = 2;
	}
	else {	/*次の送信で今回は最後*/
		*size = remain;
		ret = 1;
	}
	/*残りサイズの計算*/
	remain = memrd.siz.wd - memrd.cur.wd;
	if (remain < *size) {
		*size = remain;
		ret = 0;
	}
	res = eepwrite(memrd.adr.wd + memrd.cur.wd, *size, (pui08)in);
	if (res == EEP_OK) {
		memrd.cur.wd += *size;
		if (ret == 2) {
			memrd.stock.hf += *size;
		}
		else {
			memrd.stock.hf = 0;
		}
	}
	else {
		ret = 0xff;
	}
	return ret;
}

ui08 get_meter_mode(void) {

	ui08 res;

	if (gMtrCfg.userflag.bt.forward > 0) {
		res = 0;	/* Forword */
	}
	else {
		res = 1;	/*NET*/
	}
	return res;
}

ui08 get_laglead(void) {

	ui08 res;

	if (gMtrCfg.userflag.bt.laglead == 0) {
		res = 0;	/*lag + lead*/
	}
	else {
		res = 1;	/*lag only*/
	}
	return res;
}

ui08 get_paid_mode(void) {

	ui08 ret, mod;

	/*0:post payed, 1:prepaid*/
	if (gMtrCfg.userflag.bt.pripaid > 0) {
		ret = 1;
	}
	else {
		ret = 0;
	}
	return ret;
}

ui16	get_value2(cui16 obj, cui08 index, pui08 out)
{
	ui16	ret, tmp16, z;
	ui08	tmp;
	DAT4	wk0_dat4;

	ret = 0;
	switch (obj) {
	case IST_BILLING_COUNT: /* (1) 1: Cumulative Billing count */
	case IST_TAMPER_COUNT: /* (13) 13: Cumulative tamper count */
	case IST_PROG_COUNT: /* (33) 33: Cumulative program count */
	case IST_POWER_FAILURE: /* (34) 34: Number of power failures */
	case IST_RELAY_COUNT: /* (45) 45: Relay Count */
	case IST_EVENT_COUNT: /* (63) 63: Cumulative event count */
	case IST_MONTH_TCOUNT: /* (64) 64: Cumulative tamper count 1month */
	case IST_MAGNET_COUNT: /* (65) 65: Cumulative magnetic count */
	case IST_ESD_COUNT: /* (67) 67: Cumulative esd count */
	case IST_TOPCV_COUNT: /* (69) 69: Cumulative top cover count */
	case IST_DISCON_COUNT: /* (71) 71: Cumulative disconnect count */
	case IST_CONNECT_COUNT: /* (73) 73: Cumulative connect count */
	case IST_RESET_COUNT: /* (75) 75: Cumulative reset count */
	case IST_JMMR_COUNT:/* 69: Cumulative jammer count */
	case IST_VTHD_COUNT:/* 71: Cumulative vthd count */
	case IST_ATHD_COUNT:/* 73: Cumulative athd count */
	case IST_ATDD_COUNT:/* 75: Cumulative atdd count */
		switch (obj) {
		case IST_BILLING_COUNT: /* (1) 1: Cumulative Billing count */
			tmp = eBill;
			ramset(out, 0, 4);
			break;
		case IST_TAMPER_COUNT: /* (13) 13: Cumulative tamper count */
			tmp = eTamp;
			break;
		case IST_PROG_COUNT: /* (33) 33: Cumulative program count */
			tmp = eProg;
			break;
		case IST_POWER_FAILURE: /* (34) 34: Number of power failures */
			tmp = ePwFail;
			break;
		case IST_EVENT_COUNT: /* (63) 63: Cumulative event count */
			tmp = eEvent;
			break;
		case IST_MAGNET_COUNT: /* (65) 65: Cumulative magnetic count */
			tmp = eMagnet;
			break;
		case IST_ESD_COUNT: /* (67) 67: Cumulative esd count */
			tmp = eESD;
			break;
		case IST_JMMR_COUNT:/* 69: Cumulative jammer count */
			tmp = eJMMR;
			break;
		case IST_TOPCV_COUNT: /* (69) 69: Cumulative top cover count */
			tmp = eNICCover;
			break;
		case IST_RELAY_COUNT: /* (45) 45: Relay Count */
			tmp = eRelay;
			break;
		case IST_MONTH_TCOUNT: /* (64) 64: Cumulative tamper count 1month */
			tmp = eTampMonth;
			break;
		case IST_DISCON_COUNT: /* (71) 71: Cumulative disconnect count */
			tmp = eDiscon;
			break;
		case IST_CONNECT_COUNT: /* (73) 73: Cumulative connect count */
			tmp = eConnect;
			break;
		case IST_RESET_COUNT: /* (75) 75: Cumulative reset count */
			tmp = eCpuReset;
			break;
		case IST_VTHD_COUNT:/* 71: Cumulative vthd count */
			tmp = eVTHD;
			break;
		case IST_ATHD_COUNT:/* 73: Cumulative athd count */
			tmp = eATHD;
			break;
		case IST_ATDD_COUNT:/* 75: Cumulative atdd count */
			tmp = eATDD;
			break;
		}
		load_cumcnt(&cntcur, tmp);
		ret += ramcpy(out, (pui08)&cntcur.cumcnt, sizeof(ui16));
		break;
	case IST_MAGNET_DATE: /* (65) 65: Cumulative magnetic count */
	case IST_ESD_DATE: /* (67) 67: Cumulative esd count */
	case IST_TOPCV_DATE: /* (69) 69: Cumulative top cover count */
	case IST_DISCON_DATE: /* (71) 71: Cumulative disconnect count */
	case IST_CONNECT_DATE: /* (73) 73: Cumulative connect count */
	case IST_RESET_DATE: /* (75) 75: Cumulative reset count */
	case IST_JMMR_DATE:/* 69: Cumulative jammer count */
	case IST_VTHD_DATE:/* 71: Cumulative vthd count */
	case IST_ATHD_DATE:/* 73: Cumulative athd count */
	case IST_ATDD_DATE:/* 75: Cumulative atdd count */
		switch (obj) {
		case IST_MAGNET_DATE: /* (65) 65: Cumulative magnetic count */
			tmp = eMagnet;
			break;
		case IST_ESD_DATE: /* (67) 67: Cumulative esd count */
			tmp = eESD;
			break;
		case IST_TOPCV_DATE: /* (69) 69: Cumulative top cover count */
			tmp = eNICCover;
			break;
		case IST_DISCON_DATE: /* (71) 71: Cumulative disconnect count */
			tmp = eDiscon;
			break;
		case IST_CONNECT_DATE: /* (73) 73: Cumulative connect count */
			tmp = eConnect;
			break;
		case IST_RESET_DATE: /* (75) 75: Cumulative reset count */
			tmp = eCpuReset;
			break;
		case IST_JMMR_DATE:/* 69: Cumulative jammer count */
			tmp = eJMMR;
			break;
		case IST_VTHD_DATE:/* 71: Cumulative vthd count */
			tmp = eVTHD;
			break;
		case IST_ATHD_DATE:/* 73: Cumulative athd count */
			tmp = eATHD;
			break;
		case IST_ATDD_DATE:/* 75: Cumulative atdd count */
			tmp = eATDD;
			break;
		}
		load_cumcnt(&cntcur, tmp);
		wk0_dat4.wd = cntcur.timestamp;
		ret += get_datetime_from_sec(out, &wk0_dat4);
		break;

	case IST_BILLING_PERIOD: /* (2) 2: Available Billing Period */
		tmp16 = get_entry_in_use(IST_BILLING_PARAMS) - 1;
		ret += ramcpy(out, (pui08)&tmp16, 1);
		break;
	case IST_FRAME_COUNTER: /* (6) 6: Frame counter */
	case IST_FRAME_COUNTER1: /* (7) 7: Frame counter1 */
	case IST_FRAME_COUNTER2: /* (8) 8: Frame counter2 */
	case IST_FRAME_COUNTER3: /* (9) 9: Frame counter3 */
	case IST_FRAME_COUNTER4: /* (10) 10: Frame counter4 */
	case IST_FRAME_COUNTER5: /* (11) 11: Frame counter5 */
	case IST_FRAME_COUNTER6: /* (12) 12: Frame counter6 */
		wk0_dat4.wd = getCounter(2 + obj - IST_FRAME_COUNTER2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_METER_TYPE: /* (14) 14: Meter Type: /* (1Phase/3P-3W/3P-4W) */
		wk0_dat4.wd = 0;
		asc2hex_r(wk0_dat4.by, meter.id4, 1);
		bcd2hexB(out, wk0_dat4.by, 1);
		ret = 1;
		break;
	case IST_CATEGORY: /* (15) 15: Category */
		ret += ramcpy(out, meter.id3, 2);
		break;
	case IST_CURRENT_RATING: /* (16) 16: Current rating */
		ret += ramcpy(out, meter.id5, sizeof(meter.id5));
		break;
	case IST_EVENT_STATUS: /* (17) 17: Event status word */
		ret += ramcpy(out, (pui08)&gESW.reg.by[0], sizeof(gESW.reg));
		break;
	case IST_EVENT_FILTER: /* (18) 18: Event status word filter */
		ret += ramcpy(out, (pui08)&gESW.fil.by[0], sizeof(gESW.fil));
		break;
	case IST_METER_MODE: /* (19) 19: Metering mode */
	case IST_TYPE: /* (80) 80: Meter type */
		*out = get_meter_mode();
		ret = 1;
		break;
	case IST_PAYMENT: /* (20) 20: Payment mode */
		/*0:post payed, 1:prepaid*/
		*out = get_paid_mode();
		ret = 1;
		break;

	case IST_LRCF:
		load_lcrcfg(out);
		ret = 2;
		break;

	case IST_VA_AEW:
	case IST_VA_SEIPL:
		*out = get_laglead();
		ret = 1;
		break;

	case IST_RECHARGE_AMOUNT: /* (21) 21: Last token recharge amount */
		ret += ramcpy(out, (pui08)&gRechargeAmount, sizeof(gRechargeAmount));
		break;
	case IST_RECHARGE_TIME: /* (22) 22: Last token recharge time */
		ret += get_datetime_from_sec(out, &gRechargeTime);
		break;
	case IST_TOTAL_AMOUNT: /* (23) 23: Total amount at last recharge */
		ret += ramcpy(out, &gTotalAmount, sizeof(gTotalAmount));
		break;
	case IST_BALANCE_AMOUNT: /* (24) 24: Current balance amount */
		ret += ramcpy(out, &gBalanceAmount, sizeof(gBalanceAmount));
		break;
	case IST_BALANCE_TIME: /* (25) 25: Current balance time */
		ret += get_datetime_from_sec(out, &gBalanceTime);
		break;
	case IST_SERIAL_NO: /* (26) 26: Meter serial number */
		ret += ramcpy(out, &meter.id1[1], sizeof(meter.id1) - 1);
		break;
	case IST_MANUFACTURE_NAME: /* (27) 27: Manufacturer name */
		ret += ramcpy(out, meter.id6, sizeof(meter.id6));
		break;
	case IST_DEVICE_ID: /* (28) 28: Device ID */
		out[0] = FLAGS[0]; ret++;
		out[1] = FLAGS[1]; ret++;
		out[2] = FLAGS[2]; ret++;
		ret += ramcpy(&out[ret], &meter.id1[1], sizeof(meter.id1) - 1);
		break;

	case IST_METER_YEAR: /* (29) 29: Meter year of manufacture */
		wk0_dat4.wd = 0;
		asc2hex_r(wk0_dat4.by, meter.id7, sizeof(meter.id7));
		bcd2hexB(out, wk0_dat4.by, 2);
		ret = 2;
		break;

	case IST_NIC_IMEI: /* (30) 30: NIC IMEI number: /* (serial number) */
		ret += read_othinf(out, eCID3, 2);
		break;

	case IST_NIC_SERIAL: /* (31) 31: NIC serial ID */
		ret += read_othinf(out, eCID1, 2);
		break;

	case IST_NIC_SIMID:
		ret += read_othinf(out, eCID4, 2);
		break;

	case IST_NIC_YEAR: /* (32) 32: Bluetooth mac address */
		read_othinf(share_wk0, eCID7, 2);
		wk0_dat4.wd = 0;
		asc2hex_r(wk0_dat4.by, share_wk0, 4);
		bcd2hexB(out, wk0_dat4.by, 2);
		ret = 2;
		break;

	case IST_NIC_VER:
		ret += read_othinf(out, eCID8, 2);
		break;

	case IST_MAC_ADDR: /* (32) 32: Bluetooth mac address */
		ret += read_othinf(out, eCID10, 2);
		break;

	case IST_NIC_STATUS: /* (35) 35: NIC status */
		*out = get_nic_status();
		ret = 1;
		break;

	case IST_4GGSM_PRM:
		out[0] = nicinf.rsrp;
		out[1] = nicinf.rsrq;
		out[2] = nicinf.sinr;
		out[3] = nicinf.rssi;
		ret = 4;
		break;

	case IST_RATBAND:
		out[0] = nicinf.rat;
		out[1] = nicinf.band;
		ret = 2;
		break;

	case IST_CELLID:
		val2asc(out,8, nicinf.cellid.wd);
		ret = 8;
		break;

	case IST_RSSI: /* (76) 76: RSSI */
		*out = NIC_RSSI();
		ret = 1;
		break;

	case IST_RSSI_BLOCK:
		*out = rssi_blk.reg.by[0];
		ret = 1;
		break;

	case IST_RSRP: /* (77) 77: RSRP */
		*out = nicinf.rsrp;
		ret = 1;
		break;

	case IST_RSRQ: /* (78) 78: RSRQ */
		*out = nicinf.rsrq;
		ret = 1;
		break;

	case IST_SNR: /* (79) 79: SNR */
		*out = nicinf.sinr;
		ret = 1;
		break;

	case IST_EVENT1:
	case IST_EVENT2: /* (37) 37: Event2 */
	case IST_EVENT3: /* (38) 38: Event3 */
	case IST_EVENT4: /* (39) 39: Event4 */
	case IST_EVENT5: /* (40) 40: Event5 */
	case IST_EVENT6: /* (41) 41: Event6 */
	case IST_EVENT7: /* (42) 42: Event7 */
		z = obj - IST_EVENT1;
		eventCode(out, 101, z);
		ret += 2;
		break;

	case IST_EVENT1_RNO:// ((ui16)47) /* 47: Event1-recordNO */
	case IST_EVENT2_RNO:// ((ui16)48) /* 48: Event2-recordNO */
	case IST_EVENT3_RNO:// ((ui16)49) /* 49: Event3-recordNO */
	case IST_EVENT4_RNO:// ((ui16)50) /* 50: Event4-recordNO */
	case IST_EVENT5_RNO:// ((ui16)51) /* 51: Event5-recordNO */
	case IST_EVENT6_RNO:// ((ui16)52) /* 52: Event6-recordNO */
	case IST_EVENT7_RNO:// ((ui16)53) /* 53: Event7-recordNO */
		z = 3 + obj - IST_EVENT1_RNO;
		ret += ramcpy(out, &gRecPtr[z].rno, sizeof(DAT4));
		break;

	case IST_LOG_CODE: /* (43) 43: Meter log code */
//		ramcpy(out, (cpui08)&log_d, sizeof(RECLOG));
		break;
	case IST_FAULT: /* (44) 44: Meter status */
		ret += ramcpy(out, evests.fault.by, sizeof(DAT4));
		break;
	case IST_CAL_ENERGY: /* (49) 49: Energy calibration */
		ret += ramcpy(out, (cpui08)&tyosei, 23);
		break;
	case IST_RAM: /* (46) 46: RAM */
		break;
	case IST_E2P: /* (47) 47: EEPROM */
		break;
	case IST_ROM: /* (48) 48: ROM */
		break;
	case IST_ENERGY: /* (50) 50: Read energy(BF)　 */
		ret += 8;
		break;
	case IST_CAL_VOLTAMP: /* (51) 51: Voltage and ampere calibration */
		ret += ramcpy(out, (cpui08)&tyosei.sp_freq, 23);
		break;
	case IST_VOLTAMP: /* (52) 52: Read volt and ampere(D3)　 */
		readrmsHex(&wk0_dat4, V1);
		ret += ramcpy(&out[ret], &wk0_dat4.by[0], sizeof(DAT4));
		readrmsHex(&wk0_dat4, V2);
		ret += ramcpy(&out[ret], &wk0_dat4.by[0], sizeof(DAT4));
		readrmsHex(&wk0_dat4, V3);
		ret += ramcpy(&out[ret], &wk0_dat4.by[0], sizeof(DAT4));
		readrmsHex(&wk0_dat4, I1);
		ret += ramcpy(&out[ret], &wk0_dat4.by[0], sizeof(DAT4));
		readrmsHex(&wk0_dat4, I2);
		ret += ramcpy(&out[ret], &wk0_dat4.by[0], sizeof(DAT4));
		readrmsHex(&wk0_dat4, I3);
		ret += ramcpy(&out[ret], &wk0_dat4.by[0], sizeof(DAT4));
		break;

	case IST_CAL_RTC: /* (53) 53: RTC calibration */
		ret += ramcpy(out, (cpui08)&calb_rtc, sizeof(CALB_RTC));
		break;

	case IST_BUILD_NO: /* (54) 54: Lock count */
		ret += ramcpy(out, (cpui08)&buildno, sizeof(ui32));
		break;

	case IST_BUILD_DATE: /* (55) 55: Lock time */
		ret += ramcpy(out, (cpui08)&romdate, sizeof(romdate));
		break;

	case IST_RELAY_SETTING:
		ret += ramcpy(&out[ret], &gRlyCfg.interval, sizeof(gRlyCfg.interval));
		ret += ramcpy(&out[ret], &gRlyCfg.locktime, sizeof(gRlyCfg.locktime));
		out[ret] = gRlyCfg.reconnect;
		ret++;
		break;

	case IST_RELAY_CONFIG:
		ret += ramcpy(&out[ret], &gRlyCfg.interval0, sizeof(gRlyCfg.interval0));
		ret += ramcpy(&out[ret], &gRlyCfg.locktime0, sizeof(gRlyCfg.locktime0));
		out[ret] = gRlyCfg.reconnect0;
		ret++;
		ret += dttm2datetime((PDATE)&out[ret], &gRlyCfg.dttm, 0);
		break;

	case IST_USERFLAG: /* (56) 56: Setting bit array */
		ret += ramcpy(&out[0], (cpui08)&gMtrCfg.userflag, sizeof(gMtrCfg.userflag));
		break;

	case IST_UNIT0: /* (57) 57: Unit setting */
		ret += ramcpy(&out[ret], unitRYO, sizeof(unitRYO));
		ret += ramcpy(&out[ret], unitDMD, sizeof(unitDMD));
		ret += ramcpy(&out[ret], unitRTO, sizeof(unitRTO));
		ret += ramcpy(&out[ret], unitEFF, sizeof(unitEFF));
		ret += ramcpy(&out[ret], unitOTH, sizeof(unitOTH));
		break;

	case IST_DISPINT:
		*out = unitOTH[0];
		ret = 1;
		break;

	case IST_RECORD0: /* (58) 58: Record setting */
		if (index != 0xff) {
//			ret += ramcpy(out, (cpui08)&gRecSet[index], sizeof(RECFLG));
		}
		else {
//			ret += ramcpy(out, (cpui08)&gRecSet[0], sizeof(gRecSet));
		}
		break;
	case IST_EVENT_CODES: /* (59) 59: Power quality code define */
		ret += romcpy(out, PQCODE, sizeof(PQCODE));
		break;
	case IST_SETUP_PULS: /* (60) 60: Pulse setting */
		ret += ramcpy(out, (cpui08)&gMtrCfg.mode, 10);
		break;
	case IST_DETECT: /* (61) 61: Detect event setting */
		if (index != 0xff) {
			ret += ramcpy(out, (cpui08)&detset[index], sizeof(DETSET));
		}
		else {
			ret += ramcpy(out, (cpui08)&detset[0], sizeof(detset));
		}
		break;
	case IST_SERVER_PORT: /* (62) 62: Server port */
		ret += read_othinf(out, ePORT, 2);
		break;
	case IST_MODEL: /* (81) 81: Product model */
		ret += ramcpy(out, meter.id2, sizeof(meter.id2));
		break;
	case IST_FIRM_VER: /* (82) 82: Firmware version for meter */
		ret += romcpy(out, VERSION, sizeof(VERSION));
		break;
	case IST_FIRM_VER1: /* (83) 83: Active firmware version */
		ret += romcpy(&out[ret], buildno, sizeof(buildno));
		ret += romcpy(&out[ret], romdate, sizeof(romdate) - 1);
		ret += romcpy(&out[ret], romtime, sizeof(romtime) - 2);
		out[ret] = romidcd;						/* 版数	[00]				*/
		ret++;
		break;
	case IST_FIRM_SIG: /* (84) 84: Active firmware signature */
		loadSign((SIGN*)share_wk0);
		ret += ramcpy(out, (cpui08)share_wk0, 4/*/sizeof(sign)*/);
		break;
	case IST_DEMAND_PERIOD: /* (85) 85: Demand integration period */
		wk0_dat4.wd = jgndat.term;
		wk0_dat4.wd	*= gPeriod1rsv;
		wk0_dat4.wd *= 60;
		ret + ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_LOAD_PERIOD: /* (86) 86: Capture period-Block load profile */
		wk0_dat4.wd = jgndat.term;
		wk0_dat4.wd *= gPeriod2;
		wk0_dat4.wd *= 60;
		ret + ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_DAILY_PERIOD: /* (87) 87: Capture period-Daily load profile */
		wk0_dat4.wd = jgndat.term;
		wk0_dat4.wd *= gPeriod3;
		wk0_dat4.wd *= 60;
		ret + ramcpy(out, wk0_dat4.by, sizeof(ui32));
		break;
	case IST_BILLING_DATE: /* (88) 88: Billing date */
		ret += billingDate(out, 101);
		break;
	case IST_POWER_OFF_TIME: /* (89) 89: Cumulative power off duration in min */
		wk0_dat4.wd = gPwrOff;
		wk0_dat4.wd /= 60;
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BILLING_TIME: /* (90) 90: Billing power on duration in min */
		wk0_dat4.wd = gPwrOnBill;
		wk0_dat4.wd /= 60;
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_POWER_ON_TIME: /* (91) 91: Cumulative power on duration in min */
		wk0_dat4.wd = gPwrOn;
		wk0_dat4.wd /= 60;
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_BATT_VOLT: /* (92) 92: Battery voltage */
		out[0] = get_vrtc();
		out[1] = get_vbat();
		ret += 2;
		break;

	case IST_CPUTIME: /* (93) 93: Operating time objects */
		CpuTime(&wk0_dat4.wd);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_TEMPERATURE: /* (94) 94: Temperature */
		ret += ramcpy(out, (pui08)&hot1s.val1, sizeof(ui16));
		break;
	case IST_MAGNET: /* (95) 95: Magnet voltage */
		ret += ramcpy(out, (pui08)&mag1s, sizeof(JIKI));
		break;
	case IST_CPUTEMP: /* (96) 96: Temperature(CPU/MAG) */
		ret += ramcpy(out, (pui08)&hot1s.val0, sizeof(ui16));
		break;
	case IST_POWER: /* (97) 97: Active power+- */
		readImpExp(&wk0_dat4, unitDMD[1]);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_ENERGY: /* (98) 98: Cumulative Energy+ Wh */
		readRyo(&wk0_dat4, unitRYO[1], 2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_ENERGY1: /* (99) 99: Cumulative Energy+ Wh TZ1 */
	case IST_FWD_ENERGY2: /* (100) 100: Cumulative Energy+ Wh TZ2 */
	case IST_FWD_ENERGY3: /* (101) 101: Cumulative Energy+ Wh TZ3 */
	case IST_FWD_ENERGY4: /* (102) 102: Cumulative Energy+ Wh TZ4 */
	case IST_FWD_ENERGY5: /* (103) 103: Cumulative Energy+ Wh TZ5 */
	case IST_FWD_ENERGY6: /* (104) 104: Cumulative Energy+ Wh TZ6 */
	case IST_FWD_ENERGY7: /* (105) 105: Cumulative Energy+ Wh TZ7 */
	case IST_FWD_ENERGY8: /* (106) 106: Cumulative Energy+ Wh TZ8 */
		readTou(&wk0_dat4, unitRYO[1], 16 + obj - IST_FWD_ENERGY1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_W: /* (108) 108: Block Energy+ Wh */
		aveRyo(wk0_dat4.by, 2);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_WH: /* (108) 108: Block Energy+ Wh */
		aveRyo(&wk0_dat4, 2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_W1: /* (109) 109: Block Energy+ Wh TZ1 */
	case IST_FWD_BLOCK_W2: /* (110) 110: Block Energy+ Wh TZ2 */
	case IST_FWD_BLOCK_W3: /* (111) 111: Block Energy+ Wh TZ3 */
	case IST_FWD_BLOCK_W4: /* (112) 112: Block Energy+ Wh TZ4 */
	case IST_FWD_BLOCK_W5: /* (113) 113: Block Energy+ Wh TZ5 */
	case IST_FWD_BLOCK_W6: /* (114) 114: Block Energy+ Wh TZ6 */
	case IST_FWD_BLOCK_W7: /* (115) 115: Block Energy+ Wh TZ7 */
	case IST_FWD_BLOCK_W8: /* (116) 116: Block Energy+ Wh TZ8 */
		aveTou(wk0_dat4.by, 16 + obj - IST_FWD_BLOCK_W1);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_WH1: /* (109) 109: Block Energy+ Wh TZ1 */
	case IST_FWD_BLOCK_WH2: /* (110) 110: Block Energy+ Wh TZ2 */
	case IST_FWD_BLOCK_WH3: /* (111) 111: Block Energy+ Wh TZ3 */
	case IST_FWD_BLOCK_WH4: /* (112) 112: Block Energy+ Wh TZ4 */
	case IST_FWD_BLOCK_WH5: /* (113) 113: Block Energy+ Wh TZ5 */
	case IST_FWD_BLOCK_WH6: /* (114) 114: Block Energy+ Wh TZ6 */
	case IST_FWD_BLOCK_WH7: /* (115) 115: Block Energy+ Wh TZ7 */
	case IST_FWD_BLOCK_WH8: /* (116) 116: Block Energy+ Wh TZ8 */
		aveTou(&wk0_dat4, 16 + obj - IST_FWD_BLOCK_WH1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK24_W: /* (117) 117: Block24 Energy+ Wh */
		ave24Ryo(wk0_dat4.by, 2);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_FWD_BLOCK24_WH: /* (117) 117: Block24 Energy+ Wh */
		ave24Ryo(&wk0_dat4, 2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_BAK_ENERGY: /* (118) 118: Cumulative Energy- Wh */
		readRyo(&wk0_dat4, unitRYO[1], 3);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_ENERGY1: /* (119) 119: Cumulative Energy- Wh TZ1 */
	case IST_BAK_ENERGY2: /* (120) 120: Cumulative Energy- Wh TZ2 */
	case IST_BAK_ENERGY3: /* (121) 121: Cumulative Energy- Wh TZ3 */
	case IST_BAK_ENERGY4: /* (122) 122: Cumulative Energy- Wh TZ4 */
	case IST_BAK_ENERGY5: /* (123) 123: Cumulative Energy- Wh TZ5 */
	case IST_BAK_ENERGY6: /* (124) 124: Cumulative Energy- Wh TZ6 */
	case IST_BAK_ENERGY7: /* (125) 125: Cumulative Energy- Wh TZ7 */
	case IST_BAK_ENERGY8: /* (126) 126: Cumulative Energy- Wh TZ8 */
		readTou(&wk0_dat4, unitRYO[1], 24 + obj - IST_BAK_ENERGY1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_W: /* (128) 128: Block Energy- Wh */
		aveRyo(wk0_dat4.by, 3);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_WH: /* (128) 128: Block Energy- Wh */
		aveRyo(&wk0_dat4, 3);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_W1: /* (129) 129: Block Energy- Wh TZ1 */
	case IST_BAK_BLOCK_W2: /* (130) 130: Block Energy- Wh TZ2 */
	case IST_BAK_BLOCK_W3: /* (131) 131: Block Energy- Wh TZ3 */
	case IST_BAK_BLOCK_W4: /* (132) 132: Block Energy- Wh TZ4 */
	case IST_BAK_BLOCK_W5: /* (133) 133: Block Energy- Wh TZ5 */
	case IST_BAK_BLOCK_W6: /* (134) 134: Block Energy- Wh TZ6 */
	case IST_BAK_BLOCK_W7: /* (135) 135: Block Energy- Wh TZ7 */
	case IST_BAK_BLOCK_W8: /* (136) 136: Block Energy- Wh TZ8 */
		aveTou(wk0_dat4.by, 24 + obj - IST_BAK_BLOCK_W1);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_WH1: /* (129) 129: Block Energy- Wh TZ1 */
	case IST_BAK_BLOCK_WH2: /* (130) 130: Block Energy- Wh TZ2 */
	case IST_BAK_BLOCK_WH3: /* (131) 131: Block Energy- Wh TZ3 */
	case IST_BAK_BLOCK_WH4: /* (132) 132: Block Energy- Wh TZ4 */
	case IST_BAK_BLOCK_WH5: /* (133) 133: Block Energy- Wh TZ5 */
	case IST_BAK_BLOCK_WH6: /* (134) 134: Block Energy- Wh TZ6 */
	case IST_BAK_BLOCK_WH7: /* (135) 135: Block Energy- Wh TZ7 */
	case IST_BAK_BLOCK_WH8: /* (136) 136: Block Energy- Wh TZ8 */
		aveTou(&wk0_dat4, 24 + obj - IST_BAK_BLOCK_WH1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK24_W: /* (137) 137: Block24 Energy- Wh */
		ave24Ryo(wk0_dat4.by, 3);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK24_WH: /* (137) 137: Block24 Energy- Wh */
		ave24Ryo(&wk0_dat4, 3);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_QPOWER: /* (138) 138: Reactive power+- */
		readFwdBak(&wk0_dat4, unitDMD[1]);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_QENERGY: /* (139) 139: Cumulative Energy+ varh */
		readRyo(&wk0_dat4, unitRYO[2], 4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_QENERGY: /* (141) 141: Cumulative Energy- varh */
		readRyo(&wk0_dat4, unitRYO[2], 5);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_FWD_BLOCK_VARH: /* (165) 165: Block energy kvarh export-unuse */
		aveRyo(out, 4);
		ret += sizeof(DAT4);
		break;

	case IST_BLOCK_Q2: /* (165) 165: Block energy kvarh export-unuse */
		aveRyo(out, 5);
		ret += sizeof(DAT4);
		break;
	case IST_BLOCK_Q3: /* (165) 165: Block energy kvarh export-unuse */
		aveRyo(out, 6);
		ret += sizeof(DAT4);
		break;

	case IST_BAK_BLOCK_VARH: /* (165) 165: Block energy kvarh export-unuse */
		aveRyo(out, 7);
		ret += sizeof(DAT4);
		break;

	case IST_VARH_Q1: /* (143) 143: Cumulative Energy kvarh: (Q1) */
	case IST_VARH_Q2: /* (154) 154: Cumulative Energy kvarh: (Q2) */
	case IST_VARH_Q3: /* (155) 155: Cumulative Energy kvarh: (Q3) */
	case IST_VARH_Q4: /* (156) 156: Cumulative Energy kvarh: (Q4) */
		readReactiveQ(&wk0_dat4, obj - IST_VARH_Q1, unitRYO[1]);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_APPERENT: /* (167) 167: Apparent power kVA+- */
		readLftRgt(&wk0_dat4, unitDMD[1]);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_VAH: /* (168) 168: Cumulative Energy+ kVAh */
		readRyo(&wk0_dat4, unitRYO[3], 0);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_VAH1: /* (169) 169: Cumulative Energy+ kVAh TZ1 */
	case IST_FWD_VAH2: /* (170) 170: Cumulative Energy+ kVAh TZ2 */
	case IST_FWD_VAH3: /* (171) 171: Cumulative Energy+ kVAh TZ3 */
	case IST_FWD_VAH4: /* (172) 172: Cumulative Energy+ kVAh TZ4 */
	case IST_FWD_VAH5: /* (173) 173: Cumulative Energy+ kVAh TZ5 */
	case IST_FWD_VAH6: /* (174) 174: Cumulative Energy+ kVAh TZ6 */
	case IST_FWD_VAH7: /* (175) 175: Cumulative Energy+ kVAh TZ7 */
	case IST_FWD_VAH8: /* (176) 176: Cumulative Energy+ kVAh TZ8 */
		readTou(&wk0_dat4, unitRYO[3], 0 + obj - IST_FWD_VAH1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_VA: /* (178) 178: Block Energy+ VAh */
		aveRyo(&wk0_dat4, 0);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_VAH: /* (178) 178: Block Energy+ VAh */
		aveRyo(&wk0_dat4, 0);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_FWD_BLOCK_VA1: /* (179) 179: Block Energy+ VAh TZ1 */
	case IST_FWD_BLOCK_VA2: /* (180) 180: Block Energy+ VAh TZ2 */
	case IST_FWD_BLOCK_VA3: /* (181) 181: Block Energy+ VAh TZ3 */
	case IST_FWD_BLOCK_VA4: /* (182) 182: Block Energy+ VAh TZ4 */
	case IST_FWD_BLOCK_VA5: /* (183) 183: Block Energy+ VAh TZ5 */
	case IST_FWD_BLOCK_VA6: /* (184) 184: Block Energy+ VAh TZ6 */
	case IST_FWD_BLOCK_VA7: /* (185) 185: Block Energy+ VAh TZ7 */
	case IST_FWD_BLOCK_VA8: /* (186) 186: Block Energy+ VAh TZ8 */
		aveTou(&wk0_dat4, 0 + obj - IST_FWD_BLOCK_VA1);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_FWD_BLOCK_VAH1: /* (179) 179: Block Energy+ VAh TZ1 */
	case IST_FWD_BLOCK_VAH2: /* (180) 180: Block Energy+ VAh TZ2 */
	case IST_FWD_BLOCK_VAH3: /* (181) 181: Block Energy+ VAh TZ3 */
	case IST_FWD_BLOCK_VAH4: /* (182) 182: Block Energy+ VAh TZ4 */
	case IST_FWD_BLOCK_VAH5: /* (183) 183: Block Energy+ VAh TZ5 */
	case IST_FWD_BLOCK_VAH6: /* (184) 184: Block Energy+ VAh TZ6 */
	case IST_FWD_BLOCK_VAH7: /* (185) 185: Block Energy+ VAh TZ7 */
	case IST_FWD_BLOCK_VAH8: /* (186) 186: Block Energy+ VAh TZ8 */
		aveTou(out, 0 + obj - IST_FWD_BLOCK_VAH1);
		ret += sizeof(DAT4);
		break;

	case IST_FWD_BLOCK24_VA: /* (187) 187: Block24 Energy+ VAh */
		ave24Ryo(&wk0_dat4, 0);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_FWD_BLOCK24_VAH: /* (187) 187: Block24 Energy+ VAh */
		ave24Ryo(out, 0);
		ret += sizeof(DAT4);
		break;

	case IST_BAK_VAH: /* (188) 188: Cumulative Energy- kVAh */
		readRyo(out, unitRYO[3], 1);
		ret += sizeof(DAT4);
		break;

	case IST_BAK_VAH1: /* (189) 189: Cumulative Energy- kVAh TZ1 */
	case IST_BAK_VAH2: /* (190) 190: Cumulative Energy- kVAh TZ2 */
	case IST_BAK_VAH3: /* (191) 191: Cumulative Energy- kVAh TZ3 */
	case IST_BAK_VAH4: /* (192) 192: Cumulative Energy- kVAh TZ4 */
	case IST_BAK_VAH5: /* (193) 193: Cumulative Energy- kVAh TZ5 */
	case IST_BAK_VAH6: /* (194) 194: Cumulative Energy- kVAh TZ6 */
	case IST_BAK_VAH7: /* (195) 195: Cumulative Energy- kVAh TZ7 */
	case IST_BAK_VAH8: /* (196) 196: Cumulative Energy- kVAh TZ8 */
		readTou(&wk0_dat4, unitRYO[3], 8 + obj - IST_BAK_VAH1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_BAK_BLOCK_VA: /* (198) 198: Block Energy- VAh */
		aveRyo(&wk0_dat4, 1);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_VAH: /* (198) 198: Block Energy- VAh */
		aveRyo(&wk0_dat4, 1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_VA1: /* (199) 199: Block Energy- VAh TZ1 */
	case IST_BAK_BLOCK_VA2: /* (200) 200: Block Energy- VAh TZ2 */
	case IST_BAK_BLOCK_VA3: /* (201) 201: Block Energy- VAh TZ3 */
	case IST_BAK_BLOCK_VA4: /* (202) 202: Block Energy- VAh TZ4 */
	case IST_BAK_BLOCK_VA5: /* (203) 203: Block Energy- VAh TZ5 */
	case IST_BAK_BLOCK_VA6: /* (204) 204: Block Energy- VAh TZ6 */
	case IST_BAK_BLOCK_VA7: /* (205) 205: Block Energy- VAh TZ7 */
	case IST_BAK_BLOCK_VA8: /* (206) 206: Block Energy- VAh TZ8 */
		aveTou(&wk0_dat4, 8 + obj - IST_BAK_BLOCK_VA1);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK_VAH1: /* (199) 199: Block Energy- VAh TZ1 */
	case IST_BAK_BLOCK_VAH2: /* (200) 200: Block Energy- VAh TZ2 */
	case IST_BAK_BLOCK_VAH3: /* (201) 201: Block Energy- VAh TZ3 */
	case IST_BAK_BLOCK_VAH4: /* (202) 202: Block Energy- VAh TZ4 */
	case IST_BAK_BLOCK_VAH5: /* (203) 203: Block Energy- VAh TZ5 */
	case IST_BAK_BLOCK_VAH6: /* (204) 204: Block Energy- VAh TZ6 */
	case IST_BAK_BLOCK_VAH7: /* (205) 205: Block Energy- VAh TZ7 */
	case IST_BAK_BLOCK_VAH8: /* (206) 206: Block Energy- VAh TZ8 */
		aveTou(&wk0_dat4, 8 + obj - IST_BAK_BLOCK_VAH1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK24_VA: /* (207) 207: Block24 Energy- VAh */
		ave24Ryo(&wk0_dat4, 1);
		wk0_dat4.wd *= (4 / gPeriod2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_BAK_BLOCK24_VAH: /* (207) 207: Block24 Energy- VAh */
		ave24Ryo(&wk0_dat4, 1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_AMPR0: /* (208) 208: Phase current */
	case IST_AMPR1: /* (226) 226: Current L1 */
		readAmp1(&wk0_dat4, unitEFF[2]);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_AMPRC_BLOCK: /* (248) 248: Current-Avearage? */
		fixBlockAveAmp0(&wk0_dat4);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_AMPR0_BLOCK: /* (209) 209: Average phase current */
	case IST_AMPR1_BLOCK: /* (227) 227: IR-block */
		fixBlockAveAmp1(&wk0_dat4);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_AMPR0_BLOCK24: /* (210) 210: Average24 phase current */
		fixBlock24AveAmp0(&wk0_dat4);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_AVERAGE_VOLT0: /* (211) 211: Voltage0 Ave 1min value */
	case IST_AVERAGE_VOLT1: /* (228) 228: Voltage L1 Ave 1min value */
		aveVlt1(&wk0_dat4);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;
	case IST_VOLT0: /* (212) 212: Voltage */
	case IST_VOLT1: /* (229) 229: Voltage L1 */
		readVlt1(&wk0_dat4, unitEFF[1]);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;

	case IST_VOLT0_BLOCK: /* (213) 213: Average voltage */
	case IST_VOLT1_BLOCK: /* (230) 230: Voltage L1 Block */
		fixBlockAveVlt1(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;

	case IST_VOLT0_BLOCK24: /* (214) 214: Average24 voltage */
		fixBlock24AveVlt1(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;

	case IST_VOLT_THD: /* (215) 215: Voltage THD */
		ret += ramcpy(out, &thd_v, sizeof(ui16));
		break;

	case IST_FWD_PF_MON: /* (216) 216: Average power factor for billing period */
	case IST_FWD_PF1:/* 264: System Imp PF T1  */
	case IST_FWD_PF2:/* 265: System Imp PF T2  */
	case IST_FWD_PF3:/* 266: System Imp PF T3  */
	case IST_FWD_PF4:/* 267: System Imp PF T4  */
	case IST_FWD_PF5:/* 268: System Imp PF T5  */
	case IST_FWD_PF6:/* 269: System Imp PF T6  */
	case IST_FWD_PF7:/* 270: System Imp PF T7  */
	case IST_FWD_PF8:/* 271: System Imp PF T8  */
		wk0_dat4.hf[0] = readMonthAPF(1, obj - IST_FWD_PF_MON);
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_BAK_PF_MON:/* 298: Power factor exp month */
	case IST_BAK_PF1:/* 299: System Exp PF T1  */
	case IST_BAK_PF2:/* 300: System Exp PF T2  */
	case IST_BAK_PF3:/* 301: System Exp PF T3  */
	case IST_BAK_PF4:/* 302: System Exp PF T4  */
	case IST_BAK_PF5:/* 303: System Exp PF T5  */
	case IST_BAK_PF6:/* 304: System Exp PF T6  */
	case IST_BAK_PF7:/* 305: System Exp PF T7  */
	case IST_BAK_PF8:/* 306: System Exp PF T8  */
		wk0_dat4.hf[0] = readMonthAPF(2, obj - IST_BAK_PF_MON);
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_POWER_FACTOR: /* (217) 217: Signed power factor/Three phase power factor */
		tmp = readPowerFactor0(&wk0_dat4, unitRTO[1]);
		if (tmp > 0) {
			wk0_dat4.wd = 0 - wk0_dat4.wd;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_FREQ: /* (218) 218: Frequency */
	case IST_FREQ1: /* (219) 219: Frequency L1 */
		readHelz1(&wk0_dat4, unitRTO[2]);
		ret += ramcpy(out, (cpui08)&wk0_dat4.by, sizeof(ui16));
		break;
	case IST_POWER_FACTOR1: /* (231) 231: Signed power factorR */
		tmp = readPowerFactor1(&wk0_dat4, unitRTO[1]);
		if (tmp > 0) {
			wk0_dat4.wd = 0 - wk0_dat4.wd;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_AMPR2: /* (232) 232: Current L2 */
	case IST_AMPRG: /* (244) 244: Neutral current */
		readAmp2(&wk0_dat4, unitEFF[2]);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_AMPR2_BLOCK: /* (233) 233: IY-block */
	case IST_AMPRG_BLOCK: /* (245) 245: Neutral Current-Block */
		fixBlockAveAmp2(&wk0_dat4);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_POWER_FACTOR2: /* (237) 237: Signed power factorY */
		tmp = readPowerFactor2(&wk0_dat4, unitRTO[1]);
		if (tmp > 0) {
			wk0_dat4.wd = 0 - wk0_dat4.wd;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_AMPRG_BLOCK24: /* (246) 246: Neutral Current-Block24 */
		fixBlock24AveAmp2(&wk0_dat4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_AMPRC: /* (247) 247: Current */
		readAmp0(&wk0_dat4, unitEFF[2]);
		if (wk0_dat4.hf[1] == 0x0f0f) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;

	case IST_AMPR_THD: /* (249) 249: Current THD */
		wk0_dat4.wd = 0;
		ret += ramcpy(out, (pui08)&thd_i, sizeof(ui16));
		break;
	case IST_AMPR_TDD: /* (250) 250: Current TDD */
		wk0_dat4.wd = 0;
		ret += ramcpy(out, (pui08)&tdd_i, sizeof(ui16));
		break;
	case IST_DEFRAUD_POWER: /* (251) 251: Defraud Kwh */
		wk0_dat4.wd = 0;
		readDefraud(out, unitRYO[1]);
		ret  = 4;
		break;
	case IST_MAX_FWD_VA: /* (276) 276: Max Demand+ kVA */
		getMaxRyo(&wk0_dat4, 0);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_VA: /* (286) 286: Max Demand- kVA */
		getMaxRyo(&wk0_dat4, 1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_DEMAND: /* (252) 252: Max Demand+ kW */
		getMaxRyo(&wk0_dat4, 2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_DEMAND: /* (262) 262: Max Demand- kW */
		getMaxRyo(&wk0_dat4, 3);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_QDEMAND: /* (272) 272: Max.Demand+ var */
		getMaxRyo(&wk0_dat4, 4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_QDEMAND: /* (274) 274: Max.Demand- var */
		getMaxRyo(&wk0_dat4, 5);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_VA1: /* (277) 277: Max Demand+ kVA TZ1 */
	case IST_MAX_FWD_VA2: /* (278) 278: Max Demand+ kVA TZ2 */
	case IST_MAX_FWD_VA3: /* (279) 279: Max Demand+ kVA TZ3 */
	case IST_MAX_FWD_VA4: /* (280) 280: Max Demand+ kVA TZ4 */
	case IST_MAX_FWD_VA5: /* (281) 281: Max Demand+ kVA TZ5 */
	case IST_MAX_FWD_VA6: /* (282) 282: Max Demand+ kVA TZ6 */
	case IST_MAX_FWD_VA7: /* (283) 283: Max Demand+ kVA TZ7 */
	case IST_MAX_FWD_VA8: /* (284) 284: Max Demand+ kVA TZ8 */
		getMaxRyo(&wk0_dat4, 8 + obj - IST_MAX_FWD_VA1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_VA1: /* (287) 287: Max Demand- kVA TZ1 */
	case IST_MAX_BAK_VA2: /* (288) 288: Max Demand- kVA TZ2 */
	case IST_MAX_BAK_VA3: /* (289) 289: Max Demand- kVA TZ3 */
	case IST_MAX_BAK_VA4: /* (290) 290: Max Demand- kVA TZ4 */
	case IST_MAX_BAK_VA5: /* (291) 291: Max Demand- kVA TZ5 */
	case IST_MAX_BAK_VA6: /* (292) 292: Max Demand- kVA TZ6 */
	case IST_MAX_BAK_VA7: /* (293) 293: Max Demand- kVA TZ7 */
	case IST_MAX_BAK_VA8: /* (294) 294: Max Demand- kVA TZ8 */
		getMaxRyo(&wk0_dat4, 16 + obj - IST_MAX_BAK_VA1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_DEMAND1: /* (253) 253: Max Demand+ kW TZ1 */
	case IST_MAX_FWD_DEMAND2: /* (254) 254: Max Demand+ kW TZ2 */
	case IST_MAX_FWD_DEMAND3: /* (255) 255: Max Demand+ kW TZ3 */
	case IST_MAX_FWD_DEMAND4: /* (256) 256: Max Demand+ kW TZ4 */
	case IST_MAX_FWD_DEMAND5: /* (257) 257: Max Demand+ kW TZ5 */
	case IST_MAX_FWD_DEMAND6: /* (258) 258: Max Demand+ kW TZ6 */
	case IST_MAX_FWD_DEMAND7: /* (259) 259: Max Demand+ kW TZ7 */
	case IST_MAX_FWD_DEMAND8: /* (260) 260: Max Demand+ kW TZ8 */
		getMaxRyo(&wk0_dat4, 24 + obj - IST_MAX_FWD_DEMAND1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_DEMAND1: /* (263) 263: Max Demand- kW TZ1 */
	case IST_MAX_BAK_DEMAND2: /* (264) 264: Max Demand- kW TZ2 */
	case IST_MAX_BAK_DEMAND3: /* (265) 265: Max Demand- kW TZ3 */
	case IST_MAX_BAK_DEMAND4: /* (266) 266: Max Demand- kW TZ4 */
	case IST_MAX_BAK_DEMAND5: /* (267) 267: Max Demand- kW TZ5 */
	case IST_MAX_BAK_DEMAND6: /* (268) 268: Max Demand- kW TZ6 */
	case IST_MAX_BAK_DEMAND7: /* (269) 269: Max Demand- kW TZ7 */
	case IST_MAX_BAK_DEMAND8: /* (270) 270: Max Demand- kW TZ8 */
		getMaxRyo(&wk0_dat4, 32 + obj - IST_MAX_BAK_DEMAND1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_MAX24_FWD_VA: /* (285) 285: Max24 Demand+ kVA */
		getMax24Ryo(&wk0_dat4, 0);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX24_BAK_VA: /* (295) 295: Max24 Demand- kVA */
		getMax24Ryo(&wk0_dat4, 1);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_MAX24_FWD_DEMAND: /* (261) 261: Max24 Demand+ kW */
		getMax24Ryo(&wk0_dat4, 2);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX24_BAK_DEMAND: /* (271) 271: Max24 Demand- kW */
		getMax24Ryo(&wk0_dat4, 3);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_MAX24_FWD_QDEMAND: /* (273) 273: Max24 Demand+ var */
		getMax24Ryo(&wk0_dat4, 4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;
	case IST_MAX24_BAK_QDEMAND: /* (275) 275: Max24 Demand- var */
		getMax24Ryo(&wk0_dat4, 5);
		ret += ramcpy(out, wk0_dat4.by, sizeof(DAT4));
		break;

	case IST_MINIMUM_VOLT0: /* (296) 296: Minimum Voltage0 */
	case IST_MINIMUM_VOLT1: /* (297) 297: Minimum Voltage of L1 */
		getMinVlt1(&wk0_dat4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;
#if 0
	case IST_MINIMUM_VOLT2: /* (298) 298: Minimum Voltage of L2 */
		getMinVlt2(&wk0_dat4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;
	case IST_MINIMUM_VOLT3: /* (299) 299: Minimum Voltage of L3 */
		getMinVlt3(&wk0_dat4);
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16)) + 2;
		break;
#endif
	case IST_DATETIME_NOW: /* (319) 319: Date time */
		if ((event_push & 0x40)) {
			event_push &= ~0x40;
			get_datetime_from_sec((PDATETIME)&out[ret], &pshsec);
		}
		else {
			dttm2datetime((PDATETIME)&out[ret], addr_date(), jgndat.reset);
		}
		break;
	case IST_DATETIME_RTC: /* (320) 320: Date time(RTC) */
		rtc_read((pui08)share_wk0);
		dttm2datetime((PDATETIME)&out[ret], share_wk0, jgndat.reset);
		break;
	case IST_DEMAND_RESET: /* (321) 321: MDI Reset/End of Billing Period script table */
		break;
	case IST_TARIFF_SCR: /* (322) 322: Tariffication script table */
		break;
	case IST_IMG_SCR: /* (323) 323: Image activation script table */
		break;
	case IST_PUSH_SCR: /* (324) 324: Push script table */
		break;
	case IST_SPECIAL_DAYS: /* (325) 325: Special days table */
		if (index != 0xff) {
			z = index;
			tmp16 = z + 1;
		}
		else {
			z = 0;
			tmp16 = sizeof(gTouS.tous) / sizeof(TOU_SPD);
		}
		for (; z < tmp16; z++) {
			ret += ramcpy(&out[ret], (pui08)&z, 2);
			ret += dt32date((PDATE)&out[ret], &gTouS.tous[z].start);
			out[ret] = gTouS.tous[z].day;
			ret++;
		}
		break;
	case IST_IMG_TRANS: /* (333) 333: Image transfer */
		wk0_dat4.wd = image_block_size();
//		wk0_dat4.wd += 4;
		ret += ramcpy(out, wk0_dat4.by, sizeof(wk0_dat4));
		break;
	case IST_IMG_TRANS1: /* (334) 334: Image transfer1 */
		wk0_dat4.wd = SELFSIZE;
		ret += ramcpy(out, wk0_dat4.by, sizeof(wk0_dat4));
		break;
	case IST_TIMEZONE: /* (335) 335: Activity calendar for time zones */
		tcal = (TOU_CAL*)share_wk0;
		load_tou_cal(tcal, 0);
		ret += ramcpy(out, &tcal->calender[1], tcal->calender[0]);
		break;
	case IST_BILLDATE_ACT: /* (336) 336: Single action schedule for billing dates */
	case IST_IMG_ACT: /* (337) 337: Image Activation Single action schedule */
	case IST_SM_TO_HES_ACT: /* (338) 338: Trigger SM TO HES-Single action schedule */
	case IST_SM_TO_IHD_ACT: /* (339) 339: Trigger SM TO IHD-Single action schedule */
		z = obj - IST_BILLDATE_ACT;
		tmp16 = gSingle1[z].scr.inst;
		ret += romcpy(&out[ret], g_ist[tmp16].oid, 5);
		out[ret] = 0xff; ret++;
		ret += ramcpy(&out[ret], &gSingle1[z].scr.ident, 2);
		break;
	case IST_GPRS: /* (346) 346: ANP setting */
		ret += read_othinf(out, eGPRS, 2);
		break;
	case IST_BREAKER: /* (355) 355: Disconnect control */
		ret += ramcpy(out, &gRelay.output, sizeof(gRelay.output));
		break;
	case IST_TEMP_BLOCK:/* 104: Temperature block */
		BlockAveMAGT(&wk0_dat4);
		ret += ramcpy(out, &wk0_dat4.by, sizeof(ui16));
		break;
	case IST_TEMP_BLOCK24:/* 105: Temperature block24 */
		Block24AveMAGT(&wk0_dat4);
		ret += ramcpy(out, &wk0_dat4.by, sizeof(ui16));
		break;
	case IST_CPUTEMP_BLOCK:/* 108: Temperature(CPU/MAG) block */
		BlockAveTEMP(&wk0_dat4);
		ret += ramcpy(out, &wk0_dat4.by, sizeof(ui16));
		break;
	case IST_CPUTEMP_BLOCK24:/* 109: Temperature(CPU/MAG) block24 */
		Block24AveTEMP(&wk0_dat4);
		ret += ramcpy(out, &wk0_dat4.by, sizeof(ui16));
		break;
	case IST_VTHD_BLOCK:/* 249: Block voltage THD */
		BlockAveVTHD(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_VTHD_BLOCK24:/* 250: Average24 voltage THD */
		Block24AveVTHD(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_PF_BLOCK:/* 253: Signed power factor/Three phase power factor */
		wk0_dat4.hf[0] = readBlockAPF();
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_PF_BLOCK24:/* 254: Signed power factor/Three phase power factor */
		wk0_dat4.hf[0] = readBlock24APF();
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_ATHD_BLOCK:/* 287: Block current THD */
		BlockAveATHD(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_ATHD_BLOCK24:/* 288: Average24 current THD */
		Block24AveATHD(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_ATDD_BLOCK:/* 290: Block current TDD */
		BlockAveATDD(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_ATDD_BLOCK24:/* 291: Average24 current TDD */
		Block24AveATDD(&wk0_dat4);
		if (wk0_dat4.wd == 0xffffffff) {
			wk0_dat4.wd = 0;
		}
		ret += ramcpy(out, wk0_dat4.by, sizeof(ui16));
		break;
	case IST_SETUP_HDLC0: /* (340) 340: IEC HDLC setup */
		break;
	case IST_SETUP_HDLC1: /* (341) 341: HDLC Setting(Other) */
		break;
	case IST_PUSHD_HES: /* (342) 342: SM to HES push */
		break;
	case IST_PUSHD_IHD: /* (343) 343: SM TO IHD push */
		break;
	case IST_PUSHE_HES: /* (344) 344: SM TO HES event push */
		break;
	case IST_TCP_UDP: /* (345) 345: TCP-UDP setup */
		break;
	case IST_IPV6: /* (347) 347: IPV6 setup */
		break;
	case IST_SECURITY_NONE: /* (348) 348: Setup security(For security NON) */
		break;
	case IST_SECURITY_HLS: /* (349) 349: Setup security(For security HLS) */
		break;
	case IST_SECURITY_MR: /* (350) 350: MR-Security setup */
		break;
	case IST_SECURITY_US: /* (351) 351: US-Security setup */
		break;
	case IST_SECURITY_PUSH: /* (352) 352: PUSH-Security setup */
		break;
	case IST_SECURITY_FIRM: /* (353) 353: FIRM-Security setup */
		break;
	case IST_SECURITY_IHD: /* (354) 354: IHD-Security setup */
		break;
	case IST_LOAD_LIMIT: /* (356) 356: Load limit value in kW */
		break;
	case IST_FWD_DEMAND:/* 129: Cumulative kW MD Import  */
		getMaxCum((DAT4*)out, 2);
		ret = 4;
		break;
	case IST_BAK_DEMAND:/* 159: Cumulative kW MD Export */
		getMaxCum((DAT4*)out, 3);
		ret = 4;
		break;
	case IST_FWD_VA:/* 198: Cumulative kVA MD Import  */
		getMaxCum((DAT4*)out, 0);
		ret = 4;
		break;
	case IST_BAK_VA:/* 227: Cumulative kVA MD Export */
		getMaxCum((DAT4*)out, 1);
		ret = 4;
		break;
	case IST_ABS_ENERGY:/* 276: ABS Wh */
		break;
	case IST_NET_ENERGY:/* 277: NET Wh */
		readNetActive(out, unitRYO[1]);
		ret = 4;
		break;
	case IST_NET_VAH:/* 380: Apparent energy net total */
		readNetApperent(out, unitRYO[1]);
		ret = 4;
		break;
	case IST_BAK_PF_BLOCK:/* 307: Power factor exp block  */
		break;
	case IST_BAK_PF_24:/* 308: Power factor exp day */
		break;
	case IST_FUND_ENERGY:/* 316: Fundamental Energy kWh */
		readFundamental(out, unitRYO[1]);
		ret = 4;
		break;
	case IST_SELFDIAG:/* 97: Self diagnostic details */
		*out = slfdiag.by;
		ret = 1;
		break;
	case IST_POWER_THD:/* 131: THD Power Phase  */
		ret += ramcpy(out, (ui08)&thd_w, 2);
		break;
	case IST_TIME_NOW:/* 3: Time */
	case IST_DATE_NOW:/* 4: Date */
	case IST_LOGICAL_NAME:/* 5: COSEM logical device name  */
	case IST_STAMP_PRM:/* 57: Event stamping parameter for RC/DC */
	case IST_NIC_VNUM:/* 85: NIC firmware version number  */
	case IST_ABS_QENERGY:/* 278: ABS VarH */
	case IST_NET_QENERGY:/* 279: NET VarH */
	case IST_AVERAGE_VOLT2:/* 288: Voltage L2 Ave 1min value  */
	case IST_VOLT2:/* 289: Voltage L2 */
	case IST_VOLT2_BLOCK:/* 290: Voltage L2 Block */
	case IST_AMPR3:/* 292: Current L3 */
	case IST_AMPR3_BLOCK:/* 293: IB-block */
	case IST_AVERAGE_VOLT3:/* 294: Voltage L3 Ave 1min value  */
	case IST_VOLT3:/* 295: Voltage L3 */
	case IST_VOLT3_BLOCK:/* 296: Voltage L3 Block */
	case IST_POWER_FACTOR3:/* 297: Signed power factorB */
	case IST_FREQ2:/* 329: Frequency L2 */
	case IST_FREQ3:/* 330: Frequency L3 */
	case IST_MINIMUM_VOLT2:/* 422: Minimum Voltage of L2  */
	case IST_MINIMUM_VOLT3:/* 423: Minimum Voltage of L3  */
	case IST_PUSHD_SELFREG:/* 478: Send destination and method for Self Registration Push */
	break;
	default:
		break;

	};
	return ret;
}

ui16	get_value3(cui16 obj, cui08 index, pui08 out) {

	ui08	z, pos;
	ui16	ret, max, id;
	ui32	tmp32;

	ret = 0;
	switch (obj) {
	case IST_PUSHD_HES:
	case IST_PUSHD_IHD:
		read_othinf(share_wk0, 12, 2);					/*09020000*/
		if (share_wk0[0] > 8) {
			share_wk0[0] = 0;
		}
		if (share_wk0[1] > 1) {
			share_wk0[1] = 0;
		}
		z = read_othinf(share_wk0 + 2, 10, 2);			/*09080000000000000000*/
		share_wk0[2 + z] = ']';							/*区切り*/
		z++;
		share_wk0[2 + z] = ':';							/*区切り*/
		z++;
		z += read_othinf(share_wk0 + 2 + z, 11, 2);		/*090400000000*/
		out[0] = share_wk0[0];
		out[1] = '[';
		ramcpy(&out[2], &share_wk0[2], z);
		out[2 + z] = share_wk0[1];
		ret = z + 3;
		break;

	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		pos = obj - IST_PUSHE_HES;
		pos *= 10;
		read_othinf(share_wk0, ePush2c + pos, 2);					/*09020000*/
		if (share_wk0[0] > 8) {
			share_wk0[0] = 0;
		}
		if (share_wk0[1] > 1) {
			share_wk0[1] = 0;
		}
		z = read_othinf(share_wk0 + 2, ePush2a + pos, 2);			/*09080000000000000000*/
		share_wk0[2 + z] = ']';							/*区切り*/
		z++;
		share_wk0[2 + z] = ':';							/*区切り*/
		z++;
		z += read_othinf(share_wk0 + 2 + z, ePush2b + pos, 2);		/*090400000000*/
		out[0] = share_wk0[0];
		out[1] = '[';
		ramcpy(&out[2], &share_wk0[2], z);
		out[2 + z] = share_wk0[1];
		ret = z + 3;
		break;

	case IST_IMG_TRANS: /* 236: Image transfer*/
		ret += ramcpy(out, activation, sizeof(activation));
		break;

	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tterm = (TOU_TERM*)share_wk0;
		tweek = (TOU_WEEK*)share_wk1;
		load_tou_term(tterm, 0);
		load_tou_week(tweek, 0);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tterm->tout) / sizeof(ATERM);
		}
		for (; z < max; z++) {
			ret += ramcpy(&out[ret], (pui08)tterm->tout[z].season, sizeof(tterm->tout[z].season));
			ret += dttm2datetime((PDATE)&out[ret], &tterm->tout[z].start, 0);
			id = tterm->tout[z].weekidx;
			if (id < (sizeof(tweek->touw) / sizeof(AWEEK))) {
				ret += ramcpy(&out[ret], tweek->touw[id].week, sizeof(tweek->touw[id].week));
			}
			else {
				ramset(&out[ret], 0xff, sizeof(tweek->touw[id].week));
				ret += sizeof(tweek->touw[id].week);
			}
		}
		break;
	case IST_BILLDATE_ACT:
	case IST_IMG_ACT:
	case IST_TOWWAY_ACT:
	case IST_TAMPER_ACT:
	case IST_LOAD_ACT:
	case IST_DAILY_ACT:
	case IST_BILLING_ACT:
	case IST_SM_TO_IHD_ACT:
		out[0] = 1;
		ret = 1;
		break;

	case IST_SM_TO_HES_ACT: /* 240: Trigger SM TO HES-Single action schedule*/
		out[0] = 5;
		ret = 1;
		break;

	case IST_BREAKER: /* 254: Disconnect control*/
		ret += ramcpy(out, &gRelay.state, sizeof(gRelay.state));
		break;

	case IST_LOAD_LIMIT: /* 255: Load limit value in kW*/
		if (lmval != NULL) {
			tmp32 = evThreshold16(DET_LIMIT);
			if (tmp32 == 0x0000fffe) {
				tmp32 = 0;
			}
		}
		else {
			tmp32 = 0;
		}
		ret += ramcpy(out, &tmp32, sizeof(tmp32));
		break;

	default:
		if (unit_scale[obj].mode == 0 || atr_shift == 1) {
			out[0] = (ui08)unit_scale[obj].scale;
		}
		else {
			out[0] = 0;
		}
		out[1] = unit_scale[obj].unit;
		ret =  2;
		break;
	}
	return ret;
}

ui16	get_value4(cui16 obj, cui08 index, pui08 out) {

	ui16	ret, z, max;
	ui32	tmp32;

	ret = 0;
	switch (obj) {
	case IST_IMG_TRANS: /* 236: Image transfer*/
		tmp32 = gImage.blockno;
		ret += ramcpy(out,&tmp32, 4);
		break;

	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tweek = (TOU_WEEK*)share_wk0;
		load_tou_week(tweek, 0);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tweek->touw) / sizeof(AWEEK);
		}
		for (; z < max; z++) {
			ret += ramcpy(&out[ret], (cpui08)&tweek->touw[z], sizeof(AWEEK));
		}
		break;
	case IST_BILLDATE_ACT:
	case IST_IMG_ACT:
	case IST_TOWWAY_ACT:
	case IST_TAMPER_ACT:
	case IST_LOAD_ACT:
	case IST_DAILY_ACT:
	case IST_BILLING_ACT:
	case IST_SM_TO_IHD_ACT:
		z = obj - IST_BILLDATE_ACT;
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle1[z].sch);
		break;

	case IST_SM_TO_HES_ACT: /* 240: Trigger SM TO HES-Single action schedule*/
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle6.sch[0]);
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle6.sch[1]);
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle6.sch[2]);
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle6.sch[3]);
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle6.sch[4]);
		ret += dttm2timedate((PTIME)&out[ret], (PDATE)&out[ret + sizeof(TIME)], &gSingle6.sch[5]);
		break;

	case IST_IPV6:
		ret += read_othinf(out, eCID6, 2);
		break;

	case IST_BREAKER: /* 255: Load limit value in kW*/
		ret += ramcpy(out, &gRelay.mode, sizeof(gRelay.mode));
		break;

	case IST_LOAD_LIMIT: /* 255: Load limit value in kW*/
		tmp32 = evThreshold16(DET_LIMIT);
		if (tmp32 > 0x00008000) {
			tmp32 |= 0xffff0000;
		}
		ret += ramcpy(out, &tmp32, sizeof(tmp32));
		break;

	default:
		break;
	}
	return ret;
}

static cui08f rate_obis[] = { 0x00,0x00,0x0a,0x00,0x64,0xff };
void get_timestamp(pui08 out, cui16 obj) {

	switch (obj) {
	case IST_MAX_FWD_VA: /* 184: Maximum demand VA*/
		ramcpy(out, gMM.max[0].date.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_VA: /* 193: Maximum demand VA*/
		ramcpy(out, gMM.max[1].date.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_DEMAND: /* 168: Maximum demand kW*/
		ramcpy(out, gMM.max[2].date.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_DEMAND: /* 178: Max.Demand- W(Last reset) */
		ramcpy(out, gMM.max[3].date.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_QDEMAND: /* 180: Max.Demand+ Var(Last reset) */
		ramcpy(out, gMM.max[4].date.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_QDEMAND: /* 182: Max.Demand- Var(Last reset) */
		ramcpy(out, gMM.max[5].date.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_VA1: /* 185: Maximum demand VA TZ1*/
	case IST_MAX_FWD_VA2: /* 186: Maximum demand VA TZ2*/
	case IST_MAX_FWD_VA3: /* 187: Maximum demand VA TZ3*/
	case IST_MAX_FWD_VA4: /* 188: Maximum demand VA TZ4*/
	case IST_MAX_FWD_VA5: /* 189: Maximum demand VA TZ5*/
	case IST_MAX_FWD_VA6: /* 190: Maximum demand VA TZ6*/
	case IST_MAX_FWD_VA7: /* 191: Maximum demand VA TZ7*/
	case IST_MAX_FWD_VA8: /* 192: Maximum demand VA TZ8*/
		ramcpy(out, gMM.max[8 + obj - IST_MAX_FWD_VA1].date.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_VA1: /* 185: Maximum demand VA TZ1*/
	case IST_MAX_BAK_VA2: /* 186: Maximum demand VA TZ2*/
	case IST_MAX_BAK_VA3: /* 187: Maximum demand VA TZ3*/
	case IST_MAX_BAK_VA4: /* 188: Maximum demand VA TZ4*/
	case IST_MAX_BAK_VA5: /* 189: Maximum demand VA TZ5*/
	case IST_MAX_BAK_VA6: /* 190: Maximum demand VA TZ6*/
	case IST_MAX_BAK_VA7: /* 191: Maximum demand VA TZ7*/
	case IST_MAX_BAK_VA8: /* 192: Maximum demand VA TZ8*/
		ramcpy(out, gMM.max[16 + obj - IST_MAX_BAK_VA1].date.by, sizeof(DAT4));
		break;
	case IST_MAX_FWD_DEMAND1: /* 169: Maximum demand kW TZ1*/
	case IST_MAX_FWD_DEMAND2: /* 170: Maximum demand kW TZ2*/
	case IST_MAX_FWD_DEMAND3: /* 171: Maximum demand kW TZ3*/
	case IST_MAX_FWD_DEMAND4: /* 172: Maximum demand kW TZ4*/
	case IST_MAX_FWD_DEMAND5: /* 173: Maximum demand kW TZ5*/
	case IST_MAX_FWD_DEMAND6: /* 174: Maximum demand kW TZ6*/
	case IST_MAX_FWD_DEMAND7: /* 175: Maximum demand kW TZ7*/
	case IST_MAX_FWD_DEMAND8: /* 176: Maximum demand kW TZ8*/
		ramcpy(out, gMM.max[24 + obj - IST_MAX_FWD_DEMAND1].date.by, sizeof(DAT4));
		break;
	case IST_MAX_BAK_DEMAND1: /* 169: Maximum demand kW TZ1*/
	case IST_MAX_BAK_DEMAND2: /* 170: Maximum demand kW TZ2*/
	case IST_MAX_BAK_DEMAND3: /* 171: Maximum demand kW TZ3*/
	case IST_MAX_BAK_DEMAND4: /* 172: Maximum demand kW TZ4*/
	case IST_MAX_BAK_DEMAND5: /* 173: Maximum demand kW TZ5*/
	case IST_MAX_BAK_DEMAND6: /* 174: Maximum demand kW TZ6*/
	case IST_MAX_BAK_DEMAND7: /* 175: Maximum demand kW TZ7*/
	case IST_MAX_BAK_DEMAND8: /* 176: Maximum demand kW TZ8*/
		ramcpy(out, gMM.max[32 + obj - IST_MAX_BAK_DEMAND1].date.by, sizeof(DAT4));
		break;

	case IST_MAX24_FWD_VA:
		ramcpy(out, gMM.max24[0].date.by, sizeof(DAT4));
		break;
	case IST_MAX24_BAK_VA:
		ramcpy(out, gMM.max24[1].date.by, sizeof(DAT4));
		break;
	case IST_MAX24_FWD_DEMAND:
		ramcpy(out, gMM.max24[2].date.by, sizeof(DAT4));
		break;
	case IST_MAX24_BAK_DEMAND:
		ramcpy(out, gMM.max24[3].date.by, sizeof(DAT4));
		break;
	case IST_MAX24_FWD_QDEMAND:/* 368: Max24 Demand+ var */
		ramcpy(out, gMM.max24[4].date.by, sizeof(DAT4));
		break;
	case IST_MAX24_BAK_QDEMAND:/* 369: Max24 Demand- var */
		ramcpy(out, gMM.max24[5].date.by, sizeof(DAT4));
		break;
	case IST_MINIMUM_VOLT0: /* 194: Minimum Voltage0*/
	case IST_MINIMUM_VOLT1: /* 195: Minimum Voltage of L1 */
		ramcpy(out, gMM.min[1].date.by, sizeof(DAT4));
		break;
	case IST_MINIMUM_VOLT2: /* 196: Minimum Voltage of L2 */
		ramcpy(out, gMM.min[2].date.by, sizeof(DAT4));
		break;
	case IST_MINIMUM_VOLT3: /* 197: Minimum Voltage of L3 */
		ramcpy(out, gMM.min[3].date.by, sizeof(DAT4));
		break;
	case IST_RSSI_BLOCK: /* 197: Minimum Voltage of L3 */
		ramcpy(out, rssi_blk.date.by, sizeof(DAT4));
		break;
	default:
		break;
	}
}

ui16	get_value5(cui16 obj, cui08 index, pui08 out) {

	ui16	ret, z, max, id, selector, tmp16, j;

	ret = 0;
	switch (obj) {
	case IST_MAX_FWD_DEMAND: /* 168: Maximum demand kW*/
		getMaxRyoDatetime(out, 2);
		break;
	case IST_MAX_FWD_DEMAND1: /* 169: Maximum demand kW TZ1*/
	case IST_MAX_FWD_DEMAND2: /* 170: Maximum demand kW TZ2*/
	case IST_MAX_FWD_DEMAND3: /* 171: Maximum demand kW TZ3*/
	case IST_MAX_FWD_DEMAND4: /* 172: Maximum demand kW TZ4*/
	case IST_MAX_FWD_DEMAND5: /* 173: Maximum demand kW TZ5*/
	case IST_MAX_FWD_DEMAND6: /* 174: Maximum demand kW TZ6*/
	case IST_MAX_FWD_DEMAND7: /* 175: Maximum demand kW TZ7*/
	case IST_MAX_FWD_DEMAND8: /* 176: Maximum demand kW TZ8*/
		getMaxRyoDatetime(out, 24 + obj - IST_MAX_FWD_DEMAND1);
		break;
	case IST_MAX_BAK_DEMAND: /* 178: Max.Demand- W(Last reset) */
		getMaxRyoDatetime(out, 3);
		break;
	case IST_MAX_BAK_DEMAND1: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND2: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND3: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND4: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND5: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND6: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND7: /* 178: Max.Demand- W(Last reset) */
	case IST_MAX_BAK_DEMAND8: /* 178: Max.Demand- W(Last reset) */
		getMaxRyoDatetime(out, 32 + obj - IST_MAX_BAK_DEMAND1);
		break;
	case IST_MAX_FWD_QDEMAND: /* 180: Max.Demand+ Var(Last reset) */
		getMaxRyoDatetime(out, 4);
		break;
	case IST_MAX_BAK_QDEMAND: /* 182: Max.Demand- Var(Last reset) */
		getMaxRyoDatetime(out, 5);
		break;
	case IST_MAX_FWD_VA: /* 184: Maximum demand VA*/
		getMaxRyoDatetime(out, 0);
		break;
	case IST_MAX_FWD_VA1: /* 185: Maximum demand VA TZ1*/
	case IST_MAX_FWD_VA2: /* 186: Maximum demand VA TZ2*/
	case IST_MAX_FWD_VA3: /* 187: Maximum demand VA TZ3*/
	case IST_MAX_FWD_VA4: /* 188: Maximum demand VA TZ4*/
	case IST_MAX_FWD_VA5: /* 189: Maximum demand VA TZ5*/
	case IST_MAX_FWD_VA6: /* 190: Maximum demand VA TZ6*/
	case IST_MAX_FWD_VA7: /* 191: Maximum demand VA TZ7*/
	case IST_MAX_FWD_VA8: /* 192: Maximum demand VA TZ8*/
		getMaxRyoDatetime(out, 8 + obj - IST_MAX_FWD_VA1);
		break;
	case IST_MAX_BAK_VA: /* 193: Maximum demand VA*/
		getMaxRyoDatetime(out, 1);
		break;
	case IST_MAX_BAK_VA1: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA2: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA3: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA4: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA5: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA6: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA7: /* 193: Maximum demand VA*/
	case IST_MAX_BAK_VA8: /* 193: Maximum demand VA*/
		getMaxRyoDatetime(out, 16 + obj - IST_MAX_BAK_VA1);
		break;
	case IST_MAX24_FWD_VA:
		getMax24RyoDatetime(out, 0);
		break;
	case IST_MAX24_BAK_VA:
		getMax24RyoDatetime(out, 1);
		break;
	case IST_MAX24_FWD_DEMAND:
		getMax24RyoDatetime(out, 2);
		break;
	case IST_MAX24_BAK_DEMAND:
		getMax24RyoDatetime(out, 3);
		break;
	case IST_MAX24_FWD_QDEMAND:
		getMax24RyoDatetime(out, 4);
		break;
	case IST_MAX24_BAK_QDEMAND:
		getMax24RyoDatetime(out, 5);
		break;
	case IST_MINIMUM_VOLT0: /* 194: Minimum Voltage0*/
	case IST_MINIMUM_VOLT1: /* 195: Minimum Voltage of L1 */
		getMinVoltDatetime(out, 1);
		break;
	case IST_MINIMUM_VOLT2: /* 196: Minimum Voltage of L2 */
		getMinVoltDatetime(out, 2);
		break;
	case IST_MINIMUM_VOLT3: /* 197: Minimum Voltage of L3 */
		getMinVoltDatetime(out, 3);
		break;
#if 0
	case IST_EVENT1_PROFILE: /* 198: event1 profile*/
		break;
	case IST_EVENT2_PROFILE: /* 199: event2 profile*/
		break;
	case IST_EVENT3_PROFILE: /* 200: event3 profile*/
		break;
	case IST_EVENT4_PROFILE: /* 201: event4 profile*/
		break;
	case IST_EVENT5_PROFILE: /* 202: event5 profile*/
		break;
	case IST_EVENT6_PROFILE: /* 203: event6 profile*/
		break;7 profile*/
		break;
	case IST_METER_LOG: /* 205: Meter log*/
		break;
	case IST_SNAP_PARAM1: /* 206: Snapshot of instantaneous parameters A1,A14*/
		break;
#endif
	case IST_IMG_TRANS: /* 236: Image transfer*/
		gImage.enable = TRUE;	/*enable*/
		ret += ramcpy(out, &gImage.enable, 1);
		break;

	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tday = (TOU_DAY*)share_wk0;
		load_tou_day(tday, 0);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tday->toud) / sizeof(ADAY);
		}
		for (; z < max; z++) {
			id = z + 1;
			out[ret] = id;
			ret++;
			for (j = 0; j < sizeof(ADAY) / sizeof(ASCR); j++) {
				ret += tm2time((PTIME)&out[ret], &tday->toud[z].scr[j].start);
				ret += romcpy(&out[ret], rate_obis, sizeof(rate_obis));
				selector = tday->toud[z].scr[j].arg;
				ret += ramcpy(&out[ret], (cpui08)&selector, sizeof(selector));
			}
		}
		break;
	case IST_PUSHD_HES: /* 441: SM to HES push */
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		load_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		ret += ramcpy(out, &pshcfg.rand, sizeof(ui16));
		break;
	case IST_RSSI_BLOCK: /* 197: Minimum Voltage of L3 */
		get_datetime_from_sec(out, &rssi_blk.date);
		break;
	default:
		break;
	}
	return ret;
}

ui16	get_value6(cui16 obj, cui08 index, pui08 out) {

	ui16	ret, max;
	DAT4	dat;

	ret = 0;
	switch (obj) {
	case IST_IMG_TRANS: /* 236: Image transfer*/
		ret += ramcpy(out, &gImage.status, 1);
		break;

	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tcal = (TOU_CAL*)share_wk0;
		load_tou_cal(tcal, 1);
		ret += ramcpy(out, tcal->calender, sizeof(tcal->calender));
		break;

	case IST_LOAD_LIMIT: /* 255: Load limit value in kW*/
		dat.wd = evPeriod0(DET_LIMIT);
		ret += ramcpy(out, dat.by, sizeof(DAT4));
		break;
	case IST_PUSHD_HES: /* 441: SM to HES push */
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		load_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		ret += ramcpy(out, &pshcfg.retry, sizeof(ui08));
		break;
	default:
		break;
	}
	return ret;
}

ui16	get_value7(cui16 obj, cui08 index, pui08 out) {

	ui16	ret, z, max, id, size1, size2;
	DAT4	dat;

	ret = 0;

	switch (obj) {

	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tterm = (TOU_TERM*)share_wk0;
		tweek = (TOU_WEEK*)share_wk1;
		load_tou_term(tterm, 1);
		load_tou_week(tweek, 1);
		size1 = sizeof(tterm->tout[0].season);
		size2 = sizeof(tweek->touw[0].week);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tterm->tout) / sizeof(ATERM);
		}
		for (; z < max; z++) {
			ret += ramcpy(&out[ret], tterm->tout[z].season, size1);
			ret += dttm2datetime((PDATE)&out[ret], &tterm->tout[z].start, 0);
			id = tterm->tout[z].weekidx;
			if (id < (sizeof(tweek->touw) / sizeof(AWEEK))) {
				ret += ramcpy(&out[ret], tweek->touw[id].week, size2);
			}
			else {
				ramset(&out[ret], 0xff, size2);
				ret += size2;
			}
		}
		break;

	case IST_LOAD_LIMIT: /* 255: Load limit value in kW*/
		dat.wd = evPeriod1(DET_LIMIT);
		ret += ramcpy(out, dat.by, sizeof(DAT4));
		break;
	case IST_PUSHD_HES: /* 441: SM to HES push */
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		load_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		ret += ramcpy(out, &pshcfg.delay, sizeof(ui16));
		break;
	default:
		break;
	}
	return ret;
}

ui16	get_value8(cui16 obj, cui08 index, pui08 out) {

	ui16	ret, z, dat;

	ret = 0;

	switch (obj) {

	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tweek = (TOU_WEEK*)share_wk0;
		load_tou_week(tweek, 1);
		if (index != 0xff) {
			z = index;
			dat = z + 1;
		}
		else {
			z = 0;
			dat = sizeof(tweek->touw) / sizeof(AWEEK);
		}
		for (; z < dat; z++) {
			ret += ramcpy(&out[ret], (cpui08)&tweek->touw[z], sizeof(AWEEK));
		}
		break;
	default:
		break;
	}
	return ret;
}

ui16	get_value9(cui16 obj, cui08 index, pui08 out) {

	ui16	ret, z, max, id, selector, j;

	ret = 0;
	switch (obj) {
	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tday = (TOU_DAY*)share_wk0;
		load_tou_day(tday, 1);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tday->toud) / sizeof(ADAY);
		}
		for (; z < max; z++) {
			id = z + 1;
			out[ret] = id;
			ret++;
			for (j = 0; j < sizeof(ADAY) / sizeof(ASCR); j++) {
				ret += tm2time((PTIME)&out[ret], &tday->toud[z].scr[j].start);
				ret += romcpy(&out[ret], rate_obis, sizeof(rate_obis));
				selector = tday->toud[z].scr[j].arg;
				ret += ramcpy(&out[ret], (cpui08)&selector, sizeof(selector));
			}
		}
		break;
	default:
		break;
	}
	return ret;
}

ui16	get_value10(cui16 obj, cui08 index, pui08 out) {
	ui16	ret = 0;

	switch (obj) {
	case IST_TIMEZONE: /* 237: Activity calendar for time zones*/
		tcal = (TOU_CAL*)share_wk0;
		load_tou_cal(tcal, 1);
		ret += dttm2datetime((PDATETIME)out, &tcal->dttm, 0);
		break;
	default:
		break;
	}
	return ret;
}

static ui08 seconds2block(cui32 in) {
	
	ui08 ret;
	DAT4 dat;

	dat.wd = in;
	dat.wd /= 60;
	dat.wd /= jgndat.term;
	if(dat.wd > 0) {
		if(dat.wd < 5) {
			ret = dat.by[0];
		}
		else {
			ret = 4;
		}
	}
	else {
		ret = 1;
	}
	return ret;
}

ui16	set_value2(cui16 obj, cui08 index, pui08 in)
{
	DAT4 	wk0_dat4;
	ui16	ret, wk_uh1, tmp16;
	ui08	tmp, z, from, to;

	ret = 0;
	switch (obj) {
#if 0
	case IST_BILLING_COUNT: /* 1: Cumulative Billing count*/
		break;
	case IST_BILLING_PERIOD: /* 2: Available Billing Period */
		break;
	case IST_TIME_NOW: /* 3: Time*/
		break;
	case IST_DATE_NOW: /* 4: Date*/
		break;
	case IST_LOGICAL_NAME: /* 5: COSEM logical device name */
		break;
	case IST_TAMPER_COUNT: /* 6: Cumulative tamper count*/
		break;
	case IST_METER_TYPE: /* 7: Meter type*/
		ret += ramcpy(meter.id4, in, 1);
		record_log(LOG_METER, 7);
		break;
	case IST_CATEGORY: /* 8: Category*/
		ret += ramcpy(meter.id3, in, 2);
		record_log(LOG_METER, 3);
		break;
	case IST_CURRENT_RATING: /* 9: Current rating*/
		ret += ramcpy(meter.id5, in, sizeof(meter.id5));
		record_log(LOG_METER, 8);
		break;
	case IST_EVENT_STATUS: /* 10: Event status word*/
		ret = ramcpy((pui08)&gESW.reg.by[0], in, sizeof(gESW.reg));
		break;
#endif
	case IST_LRCF:
		backup_lcrcfg(in);
		record_log(LOG_LRC_ENABLE, in[0]);
		break;

	case IST_EVENT_FILTER: /* 11: Event status word filter*/
		ret = ramcpy((pui08)&gESW.fil.by[0], in, sizeof(gESW.fil));
		record_log(LOG_ESW_CHG, 1);
		break;

	case IST_RECHARGE_AMOUNT: /* 14: Last token recharge amount*/
		inc_cumcnt(eProg);
		ret += ramcpy(&gRechargeAmount, in, sizeof(gRechargeAmount));
		break;

	case IST_RECHARGE_TIME: /* 15: Last token recharge time */
		inc_cumcnt(eProg);
		gRechargeTime = get_sec_from_datetime((PDATETIME)in);
		ret += 12;
		break;

	case IST_TOTAL_AMOUNT: /* 16: Total amount at last recharge */
		inc_cumcnt(eProg);
		ret += ramcpy((pui08)&gTotalAmount, in, sizeof(gTotalAmount));
		break;

	case IST_BALANCE_AMOUNT: /* 17: Current balance amount*/
		inc_cumcnt(eProg);
		ret += ramcpy((pui08)&gBalanceAmount, in, sizeof(gBalanceAmount));
		break;

	case IST_BALANCE_TIME: /* 18: Current balance time*/
		inc_cumcnt(eProg);
		gBalanceTime = get_sec_from_datetime((PDATETIME)in);
		ret += 12;
		break;
#if 0
	case IST_SERIAL_NO: /* 19: Meter serial number*/
		ret += ramcpy(&meter.id1[2], in, sizeof(meter.id1)-2);
		record_log(LOG_METER, 1);
		break;

	case IST_MANUFACTURE_NAME: /* 20: Manufacture name*/
		ret += ramcpy(meter.id6, in, sizeof(meter.id6));
		record_log(LOG_METER, 4);
		break;

	case IST_DEVICE_ID: /* 21: Device ID*/
		break;

	case IST_METER_YEAR: /* 22: Meter year if manufacture*/
		ret += ramcpy(meter.id7, in, 4);//@
		record_log(LOG_METER, 5);
		break;

	case IST_PROG_COUNT: /* 24: Cumulative programming count*/
		break;
	case IST_POWER_FAILURE: /* 25: Number of power - failures*/
		break;
	case IST_STATUS: /* 26: Profile status */
		break;
	case IST_EVENT1: /* 27: event1*/
		break;
	case IST_EVENT2: /* 28: event2*/
		break;
	case IST_EVENT3: /* 29: event3*/
		break;
	case IST_EVENT4: /* 30: event4*/
		break;
	case IST_EVENT5: /* 31: event5*/
		break;
	case IST_EVENT6: /* 32: event6*/
		break;
	case IST_EVENT7: /* 33: event7*/
		break;
	case IST_LOG_CODE: /* 34: Meter log code*/
		break;
	case IST_FAULT: /* 35: Fault code*/
		break;
	case IST_ALERT_REG1: /* 36: Alert register 1 */
		break;
	case IST_ALERT_REG2: /* 37: Alert register 2 */
		break;
	case IST_ALERT_FIL1: /* 38: Alert Filter 1 */
		break;
	case IST_ALERT_FIL2: /* 39: Alert Filter 2 */
		break;
	case IST_ALERT_DSC1: /* 40: Alert Descriptor 1 */
		break;
	case IST_ALERT_DSC2: /* 41: Alert Descriptor 2 */
		break;
	case IST_RAM: /* 42: RAM*/
		break;
	case IST_E2P: /* 43: EEPROM*/
		break;
	case IST_ROM: /* 44: ROM*/
		break;
#endif
	case IST_CAL_ENERGY: /* 45: Energy calibration*/
		req_lsi_ini();
		ret += ramcpy((pui08)&tyosei, in, 23);
		cmd_backup_tyosei[0]();
		record_log(LOG_CMD, 11);					/* F1 */
		break;
#if 0
	case IST_ENERGY: /* 46: Read energy(BF)　*/
		break;
#endif
	case IST_CAL_VOLTAMP: /* 47: Voltage and ampere calibration*/
		req_lsi_ini();
		ret += ramcpy((pui08)&tyosei.sp_freq, in, 23);
		cmd_backup_tyosei[0]();
		record_log(LOG_CMD, 15);				/* D5 */
		break;
#if 0
	case IST_VOLTAMP: /* 48: 電圧/電流読み出し(D3)　*/
		break;
#endif
	case IST_SERVER_PORT:
		share_wk0[0] = DT_OCTET_STRING;
		ramcpy(&share_wk0[1], &in[0], in[0] + 1);
		ret += write_othinf(2, in[0] + 2, &share_wk0[0]);
		nicsts.bt.port = 1;
		comretry = 0;
		break;

	case IST_GPRS:
		share_wk0[0] = DT_OCTET_STRING;
		ramcpy(&share_wk0[1], &in[0], in[0] + 1);
		ret += write_othinf(1, in[0] + 2, &share_wk0[0]);
		nicsts.bt.gprs = 1;
		comretry = 0;
		break;

	case IST_CAL_RTC: /* 49: RTC calibration*/
		ret += ramcpy((pui08)&calb_rtc, in, sizeof(CALB_RTC));
		record_log(LOG_CMD, 31);				/* P1	*/
		break;

	case IST_USERFLAG: /* 50: Setting bit array*/
		ramcpy(gMtrCfg.userflag.by, in, sizeof(gMtrCfg.userflag));
		ret += 2;
		record_log(LOG_METER, 3);
		break;
#if 0
	case IST_UNIT0: /* 55: Unit setting*/
		ret += ramcpy((pui08)unitRYO, &in[ret], sizeof(unitRYO));
		ret += ramcpy((pui08)unitDMD, &in[ret], sizeof(unitDMD));
		ret += ramcpy((pui08)unitEFF, &in[ret], sizeof(unitEFF));
		ret += ramcpy((pui08)unitOTH, &in[ret], sizeof(unitOTH));
		cmd_backup_defset[0]();
		record_log(LOG_DEFSET, 2);
		break;

	case IST_RECORD0: /* 56: Record setting*/
		if (index != 0xff) {
//			ret += ramcpy((pui08)&gRecSet[index], in, sizeof(RECCFG));
		}
		else {
//			ret += ramcpy((pui08)&gRecSet[0], in, sizeof(gRecSet));
		}
		record_log(LOG_DEFSET, 3);
		break;

	case IST_EVENT_CODES: /* 57: Power quality code define*/
//		ret += ramcpy((pui08)&gPQCodes, in, sizeof(PQCODES));
//		cmd_backup_userset1[0]();
//		record_log(LOG_USERSET, 2);
		break;
#endif
	case IST_DISPINT:
		if (*in < 1) {
			unitOTH[0] = 1;
		}
		else {
			unitOTH[0] = *in;
		}
		ret = 1;
		break;
	case IST_SETUP_PULS: /* 58: Pulse setting*/
		/*桁上げ処理*/
		gMtrCfg.mode = *in;
		in++;

		if (*in == 0) {		/*戻す*/
			if (gMtrCfg.puls != 0) {
				gMtrCfg.puls = *in;
				P_POUT_R = 0;
//				P_POUT_W = 0;
			}
		}
		else {
			gMtrCfg.puls = *in;
			if ((gMtrCfg.puls & 0x80) > 0) {
				P_POUT_R = 1;
			}
			else {
				P_POUT_R = 0;
			}
			if ((gMtrCfg.puls & 0x40) > 0) {
//				P_POUT_W = 1;
			}
			else {
//				P_POUT_W = 0;
			}
		}
		in++;
		/* 1:coil, 2:shunt */
		tmp = gMtrCfg.source & 0x03;
		di();
		gMtrCfg.source &= ~0x83;
		if (*in == tmp) {		/*戻す*/
			gMtrCfg.source |= 0x01;
		}
		else {
			tmp = *in;
			tmp &= 0x03;
			gMtrCfg.source |= 0x80;
			gMtrCfg.source |= tmp;
		}
		ei();
		in++;

		if (*in != 0) {
			gMtrCfg.thrd0 = *in;
		}
		in++;

		if (*in != 0) {
			gMtrCfg.thrd1 = *in;
		}
		in++;

		if (*in != 0) {
			gMtrCfg.thrd2 = *in;
		}
		in++;

		if (*in != 0) {
			gMtrCfg.thrd3 = *in;
		}
		in++;

		if (*in != 0) {
			gMtrCfg.thrd4 = *in;
		}
		in++;
		wk_uh1 = UI16B(in);
		if (wk_uh1 != 0) {
			gMtrCfg.width = wk_uh1;
		}
		in += 2;
		break;

	case IST_RELAY_CONFIG:
		ret += ramcpy(&gRlyCfg.interval0, &in[ret], sizeof(gRlyCfg.interval0));
		ret += ramcpy(&gRlyCfg.locktime0, &in[ret], sizeof(gRlyCfg.locktime0));
		gRlyCfg.reconnect0 = in[ret];
		ret++;
		datetime2dttm(&gRlyCfg.dttm, (PDATETIME)&in[ret]);
		gRlyCfg.dttm.dmy = 0;
		init_relay_scheduler();
		record_log(LOG_RLY_CFG, 0);
		break;

	case IST_DETECT: /* 59: Detect event setting*/
		if (index != 0xff) {
//			ret += ramcpy((pui08)&detset[index], in, sizeof(DETECT));
		}
		else {
//			ret += ramcpy((pui08)&detset[0], in, sizeof(detset));
		}
//		event_ini(0);
		record_log(LOG_DEFSET, 4);
		break;

	case IST_METER_MODE: /* 12: Metering mode*/
	case IST_TYPE: /* 60: Meter type*/
		switch (*in) {
		case 0:
			gMtrCfg.userflag.bt.forward = 1;
			record_log(LOG_MTR_CHG, 0);
			break;
		default:
			gMtrCfg.userflag.bt.forward = 0;
			record_log(LOG_MTR_CHG, 1);
			break;
		}
//		record_log(LOG_METER, 5);
		ret = 1;
		break;

	case IST_PAYMENT: /* 13: Payment mode*/
		switch (*in) {
		case 1:
			gMtrCfg.userflag.bt.pripaid = 1;
			record_log(LOG_PAY_CHG, 1);
			break;
		default:
			gMtrCfg.userflag.bt.pripaid = 0;
			record_log(LOG_PAY_CHG, 0);
			break;
		}
		ret += 1;
		break;

	case IST_VA_AEW:
	case IST_VA_SEIPL:
		switch (*in) {
		case 1:
			gMtrCfg.userflag.bt.laglead = 1;
			record_log(LOG_VA_CFG, 1);
			break;
		default:
			gMtrCfg.userflag.bt.laglead = 0;
			record_log(LOG_VA_CFG, 0);
			break;
		}
		ret = 1;
		break;
#if 0
	case IST_MODEL: /* 61: Product model*/
		ret += ramcpy(meter.id2, in, sizeof(meter.id2));
		record_log(LOG_METER, 2);
		break;
	case IST_FIRM_VER: /* 62: Firmware version for meter*/
		break;
	case IST_FIRM_VER1: /* 63: Active firmware version */
		break;
	case IST_FIRM_SIG: /* 64: Active firmware signature */
		break;
#endif
	case IST_DEMAND_PERIOD: /* 65: Demand integration period*/
		ramcpy(wk0_dat4.by, in, sizeof(ui16));
		gPeriod1rsv	= seconds2block(wk0_dat4.hf[0]);
		record_log(LOG_DMDP_CHG, 0);
		cmd_backup_defset[0]();
		break;

	case IST_LOAD_PERIOD: /* 66: Capture period-Block load profile*/
		ramcpy(wk0_dat4.by, in, sizeof(ui16));
		gPeriod2 = seconds2block(wk0_dat4.hf[0]);
		record_log(LOG_LPP_CHG, 0);
		iniBlockLoad();
		rec_reset(IST_LOAD_PROFILE, 0);
		cmd_backup_defset[0]();
		break;

	case IST_DAILY_PERIOD: /* 67: Capture period-Daily load profile*/
		ramcpy(wk0_dat4.by, in, sizeof(ui32));
		gPeriod3 = seconds2block(wk0_dat4.wd);
		cmd_backup_defset[0]();
		break;
#if 0
	case IST_BILLING_DATE: /* 68: Billing date*/
		break;
	case IST_FRAME_COUNTER: /* 69: Frame counter*/
		break;
	case IST_POWER_OFF_TIME: /* 70: Cumulative power off duration in min*/
		break;
	case IST_BILLING_TIME: /* 71: Billing power on duration in min*/
		break;
	case IST_POWER_ON_TIME: /* 72: Cumulative power on duration in min*/
		break;
	case IST_BATT_VOLT: /* 73: Battery voltage*/
		gMtrCfg.battlv = *in;
		record_log(LOG_METER, 8);
		break;
	case IST_CPUTIME: /* 74: Operating time objects */
		break;
	case IST_MAGNET: /* 75: Magnet voltage*/
		break;
	case IST_CPUTEMP: /* 76: Temperature */
		break;
#endif
	case IST_DATETIME_NOW: /* 221: Date time*/
		datetime2dttm((PDTTM)share_wk0, (PDATETIME)in);
		write_tokei((PDTTM)share_wk0);
		break;
#if 0
	case IST_DATETIME_RTC: /* 222: Date time(RTC)*/
		break;
	case IST_DEMAND_RESET: /* 223: MDI Reset/End of Billing Period script table*/
		break;
	case IST_TARIFF_SCR: /* 224: Tariffication script table*/
		break;
	case IST_IMG_SCR: /* 225: Image activation script table*/
		break;
	case IST_PUSH_SCR: /* 226: Push script table*/
		break;
#endif
	case IST_SPECIAL_DAYS: /* 227: Special days table*/
		if (index != 0xff) {
			z = index;
			tmp16 = z + 1;
		}
		else {
			z = 0;
			tmp16 = sizeof(gTouS.tous) / sizeof(TOU_SPD);
		}
		ret = 0;
		for (; z < tmp16; z++) {
			ret += 2;
			ret += date2dt3(&gTouS.tous[z].start, (PDATE)&in[ret]);
			gTouS.tous[z].day = in[ret];
			ret++;
		}
		cmd_backup_tous[0]();
		break;
	default:
		break;
	};
	return ret;
}

static ui08 set_ipv6_data(cpui08 in, cui08 id) {

	ui08 z, j, ret;
	ui16 max;

	ret = 0;

	share_wk0[0] = DT_OCTET_STRING;
	j = 0;
	for (z = 0; z < in[1]; z++) {
		if(IsIpv6Addr(&in[z + 2]) != 0) {
			share_wk0[j + 2] = in[z + 2];
			j++; 
		}
		else {
			if(j > 0){
				break;
			}
		}
	}
	if(j > 0){
		share_wk0[1] = j;
		ret += write_othinf(id, share_wk0[1] + 2, &share_wk0[0]);
	}

	j = 0;
	for (; z < in[1]; z++) {
		if(IsPortNum(&in[z + 2]) != 0) {
			share_wk0[j + 2] = in[z + 2];
			j++; 
		} 
		else {
			if(j > 0) {
				break;
			}
		}
	}
	if(j > 0){
		share_wk0[1] = j;
		ret += write_othinf(id + 1, share_wk0[1] + 2, &share_wk0[0]);
	}
	share_wk0[0] = DT_OCTET_STRING;
	share_wk0[1] = 2;
	share_wk0[2] = in[0];
	share_wk0[3] = in[in[1] + 2];
	ret += write_othinf(id + 2, 4, &share_wk0[0]);
//	call_cominf(0, id + 3);

	return ret;
}

ui16	set_value3(cui16 obj, cui08 index, pui08 in) {

	ui08	z, j;
	ui16	ret;

	ret = 0;
	switch (obj) {
#if 0
	case IST_FWD_ENERGY: /* 78: Active power+ Wh*/
	case IST_FWD_ENERGY1: /* 79: Cumulative energy kwh import TZ1*/
	case IST_FWD_ENERGY2: /* 80: Cumulative energy kwh import TZ2*/
	case IST_FWD_ENERGY3: /* 81: Cumulative energy kwh import TZ3*/
	case IST_FWD_ENERGY4: /* 82: Cumulative energy kwh import TZ4*/
	case IST_FWD_ENERGY5: /* 83: Cumulative energy kwh import TZ5*/
	case IST_FWD_ENERGY6: /* 84: Cumulative energy kwh import TZ6*/
	case IST_FWD_ENERGY7: /* 85: Cumulative energy kwh import TZ7*/
	case IST_FWD_ENERGY8: /* 86: Cumulative energy kwh import TZ8*/
	case IST_FWD_BLOCK_WH: /* 96: Block energy kWh import*/
	case IST_FWD_BLOCK24_WH: /* 96: Block energy kWh import*/
	case IST_BAK_ENERGY: /* 98: Active power- Wh*/
	case IST_BAK_BLOCK_WH: /* 100: Block energy kWh export*/
	case IST_BAK_BLOCK24_WH: /* 100: Block energy kWh export*/
	case IST_ABS_ENERGY: /* 142: ABS Wh*/
	case IST_NET_ENERGY: /* 143: NET Wh*/
		unitRYO[1] = in[0];
		break;

	case IST_POWER: /* 77: Active power+-*/
	case IST_MAX_FWD_DEMAND: /* 168: Maximum demand kW*/
	case IST_MAX_FWD_DEMAND1: /* 169: Maximum demand kW TZ1*/
	case IST_MAX_FWD_DEMAND2: /* 170: Maximum demand kW TZ2*/
	case IST_MAX_FWD_DEMAND3: /* 171: Maximum demand kW TZ3*/
	case IST_MAX_FWD_DEMAND4: /* 172: Maximum demand kW TZ4*/
	case IST_MAX_FWD_DEMAND5: /* 173: Maximum demand kW TZ5*/
	case IST_MAX_FWD_DEMAND6: /* 174: Maximum demand kW TZ6*/
	case IST_MAX_FWD_DEMAND7: /* 175: Maximum demand kW TZ7*/
	case IST_MAX_FWD_DEMAND8: /* 176: Maximum demand kW TZ8*/
	case IST_MAX_BAK_DEMAND: /* 178: Max.Demand- W(Last reset) */
		unitDMD[1] = in[0];
		break;

	case IST_MAX_FWD_QDEMAND: /* 180: Max.Demand+ Var(Last reset) */
	case IST_MAX_BAK_QDEMAND: /* 182: Max.Demand- Var(Last reset) */
		unitDMD[1] = in[0];
		break;

	case IST_FWD_QENERGY: /* 102: Reactive power+ VarH*/
	case IST_BAK_QENERGY: /* 105: Reactive power- VarH*/
	case IST_FWD_BLOCK_VARH: /* 107: Block energy varh import*/
	case IST_BAK_BLOCK_VARH: /* 108: Block energy varh export*/
	case IST_ABS_QENERGY: /* 144: ABS VarH*/
	case IST_NET_QENERGY: /* 145: NET VarH*/
		unitRYO[2] = in[0];
		break;

	case IST_APPERENT: /* 109: Apparent power VA*/
	case IST_MAX_FWD_VA: /* 184: Maximum demand VA*/
	case IST_MAX_FWD_VA1: /* 185: Maximum demand VA TZ1*/
	case IST_MAX_FWD_VA2: /* 186: Maximum demand VA TZ2*/
	case IST_MAX_FWD_VA3: /* 187: Maximum demand VA TZ3*/
	case IST_MAX_FWD_VA4: /* 188: Maximum demand VA TZ4*/
	case IST_MAX_FWD_VA5: /* 189: Maximum demand VA TZ5*/
	case IST_MAX_FWD_VA6: /* 190: Maximum demand VA TZ6*/
	case IST_MAX_FWD_VA7: /* 191: Maximum demand VA TZ7*/
	case IST_MAX_FWD_VA8: /* 192: Maximum demand VA TZ8*/
	case IST_MAX_BAK_VA: /* 193: Maximum demand VA*/
		unitDMD[1] = in[0];
		break;

	case IST_FWD_VAH: /* 110: Cumulative energy VAh import*/
	case IST_FWD_VAH1: /* 111: Cumulative energy VAh import TZ1*/
	case IST_FWD_VAH2: /* 112: Cumulative energy VAh import TZ2*/
	case IST_FWD_VAH3: /* 113: Cumulative energy VAh import TZ3*/
	case IST_FWD_VAH4: /* 114: Cumulative energy VAh import TZ4*/
	case IST_FWD_VAH5: /* 115: Cumulative energy VAh import TZ5*/
	case IST_FWD_VAH6: /* 116: Cumulative energy VAh import TZ6*/
	case IST_FWD_VAH7: /* 117: Cumulative energy VAh import TZ7*/
	case IST_FWD_VAH8: /* 118: Cumulative energy VAh import TZ8*/
	case IST_FWD_BLOCK_VAH: /* 128: Block energy VAh import*/
	case IST_BAK_VAH: /* 129: Cumulative energy VAh export*/
	case IST_BAK_BLOCK_VAH: /* 130: Block energy VAh export*/
		unitRYO[3] = in[0];
		break;

	case IST_AMPR0: /* 146: Current L1*/
	case IST_AMPR0_BLOCK: /* 132: Phase average current*/
	case IST_AMPR1_BLOCK: /* 147: IR-block*/
	case IST_AMPR2: /* 152: Current L2*/
	case IST_AMPR2_BLOCK: /* 153: IY-block*/
	case IST_AMPR3: /* 158: Current L3*/
	case IST_AMPR3_BLOCK: /* 159: IB-block*/
	case IST_AMPRC: /* 131: Phase current*/
	case IST_AMPRG: /* 164: Neutral current*/
		unitEFF[2] = in[0];			/* scaler値(-1)ｾｯﾄ			*/
		break;

	case IST_AVERAGE_VOLT0: /* 133: Voltage0 Ave 1min value */
	case IST_VOLT0: /* 134: Voltage*/
	case IST_VOLT0_BLOCK: /* 135: Average voltage*/
	case IST_AVERAGE_VOLT1: /* 148: Voltage L1 Ave 1min value */
	case IST_VOLT1: /* 149: Voltage L1*/
	case IST_VOLT1_BLOCK: /* 150: Voltage L1 Block*/
	case IST_AVERAGE_VOLT2: /* 154: Voltage L2 Ave 1min value */
	case IST_VOLT2: /* 155: Voltage L2*/
	case IST_VOLT2_BLOCK: /* 156: Voltage L2 Block*/
	case IST_AVERAGE_VOLT3: /* 160: Voltage L3 Ave 1min value */
	case IST_VOLT3: /* 161: Voltage L3*/
	case IST_VOLT3_BLOCK: /* 162: Voltage L3 Block*/
	case IST_MINIMUM_VOLT0: /* 194: Minimum Voltage0*/
	case IST_MINIMUM_VOLT1: /* 195: Minimum Voltage of L1 */
	case IST_MINIMUM_VOLT2: /* 196: Minimum Voltage of L2 */
	case IST_MINIMUM_VOLT3: /* 197: Minimum Voltage of L3 */
		unitEFF[1]= in[0];			/* scaler値ｾｯﾄ				*/
		break;
#endif
	case IST_PUSHD_HES:
		ret = set_ipv6_data(in, ePush1a);
		nicsts.bt.adr1 = 1;
		record_log(LOG_PERIOD_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;
	case IST_PUSHE_HES:
		ret = set_ipv6_data(in, ePush2a);
		nicsts.bt.adr2 = 1;
		record_log(LOG_ALERT_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
		break;
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
		ret = set_ipv6_data(in, ePush3a);
		nicsts.bt.adr3 = 1;
		record_log(LOG_LOAD_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
		ret = set_ipv6_data(in, ePush4a);
		nicsts.bt.adr4 = 1;
		record_log(LOG_DAILY_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
		ret = set_ipv6_data(in, ePush5a);
		nicsts.bt.adr5 = 1;
		record_log(LOG_SELF_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
		ret = set_ipv6_data(in, ePush6a);
		nicsts.bt.adr6 = 1;
		record_log(LOG_BILL_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		ret = set_ipv6_data(in, ePush7a);
		nicsts.bt.adr7 = 1;
		record_log(LOG_TAMP_IP, 0);
		set_timer1(&tm_cominf, 1);
		break;

	default:
		break;
	}
	return ret;
}

ui16	set_value4(cui16 obj, cui08 index, pui08 in) {

	ui16	ret;
	DAT4	dat;
	ui08	tmp0, tmp1;

	ret = 0;

	switch (obj) {
	default:
		break;
	case IST_BILLDATE_ACT:
	case IST_IMG_ACT:
	case IST_TOWWAY_ACT:
	case IST_TAMPER_ACT:
	case IST_LOAD_ACT:
	case IST_DAILY_ACT:
	case IST_BILLING_ACT:
	case IST_SM_TO_IHD_ACT:
		tmp0 = obj - IST_BILLDATE_ACT;
		record_log(LOG_SAS_CHG, tmp0);
		ramset(&gSingle1[tmp0].sch, 0xff, sizeof(DTTM));
		gSingle1[tmp0].sch.dmy = 0;
		ret += timedate2dttm(&gSingle1[tmp0].sch, &in[0], &in[0 + sizeof(TIME)]);
		init_single_scheduler(tmp0);
		if (tmp0 == 0) {
			ramset(&gSingle1[6].sch, 0xff, sizeof(DTTM));
			gSingle1[6].sch.dmy = 0;
			timedate2dttm(&gSingle1[6].sch, &in[0], &in[0 + sizeof(TIME)]);
			init_single_scheduler(6);
		}
		//		gSingle[z].sch.dmy = in[ret + 3];
		break;
	case IST_SM_TO_HES_ACT:
		tmp0 = obj - IST_SM_TO_HES_ACT + 7;
		record_log(LOG_SAS_CHG, tmp0);
		for (tmp1 = 0; tmp1 < 6; tmp1++) {
			ramset((pui08)&gSingle6.sch[tmp1], 0xff, sizeof(DTTM));
			gSingle6.sch[tmp1].dmy = 0;
			if (tmp1 < array_count[0]) {
				ret += timedate2dttm(&gSingle6.sch[tmp1], &in[ret], &in[ret + sizeof(TIME)]);
			}
		}
		init_single_scheduler(tmp0);
		break;

	case IST_BREAKER:
		disconnect_mode_change(*in);
		ret = 1;
		break;

	case IST_LOAD_LIMIT:
		record_log(LOG_LMT_SET, 0);
		ret += ramcpy(dat.by, in, sizeof(DAT4));
		if (dat.wd != 0xfffffffe) {
			if (dat.wd > 20000) {
				dat.wd = 20000;
			}
		}
		setThreshold16(dat.hf[0], DET_LIMIT);
		break;
	}
	return ret;
}

ui16	set_value5(cui16 obj, cui08 index, pui08 in) {

	ui16	ret, tmp;

	ret = 0;
	switch (obj) {
	case IST_IMG_TRANS: /* 236: Image transfer*/
		ret += ramcpy(&gImage.enable, in , 1);
		break;
	case IST_PUSHD_HES: /* 441: SM to HES push */
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		tmp = obj - IST_PUSHD_HES;
		record_log(LOG_RAND_DELY, tmp);
		load_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		ret += ramcpy(&pshcfg.rand, in, sizeof(ui16));
		backup_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		break;
	default:
		break;
	}
	return ret;

}

ui16	set_value6(cui16 obj, cui08 index, pui08 in) {

	ui16	ret;
	DAT4	dat;

	ret = 0;

	switch (obj) {
	case IST_TIMEZONE:
		tcal = (TOU_CAL*)share_wk0;
		load_tou_cal(tcal, 1);
		ret += ramcpy((pui08)tcal->calender, in, sizeof(tcal->calender));
		backup_tou_cal(tcal, 1);
		break;

	case IST_LOAD_LIMIT:
		ret += ramcpy(dat.by, in, sizeof(DAT4));
		setPeriod0(dat.hf[0], DET_LIMIT);
		break;
	case IST_PUSHD_HES: /* 441: SM to HES push */
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		load_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		ret += ramcpy(&pshcfg.retry, in, sizeof(ui08));
		backup_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		break;

	default:
		break;
	}
	return ret;
}

ui16	set_value7(cui16 obj, cui08 index, pui08 in) {

	ui08	z, j, name[8];
	ui16	ret, max, max2, size1, size2;
	DAT4	dat;

	ret = 0;

	switch (obj) {
	case IST_TIMEZONE:
		tterm = (TOU_TERM*)share_wk0;
		tweek = (TOU_WEEK*)share_wk1;
		load_tou_term(tterm, 1);
		load_tou_week(tweek, 1);

		size1 = sizeof(tterm->tout[z].season);
		max2 = sizeof(tweek->touw) / sizeof(AWEEK);
		size2 = sizeof(tweek->touw[0].week);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tterm->tout) / sizeof(ATERM);
		}
		for (; z < max; z++) {
			ret += ramcpy((pui08)tterm->tout[z].season, &in[ret], size1);
			datetime2dttm(&tterm->tout[z].start, (PDATE)&in[ret]);
			ret += sizeof(DATETIME);
			ret += ramcpy((pui08)name, &in[ret], size2);
			tterm->tout[z].weekidx = 0xff;
			for (j = 0; j < max2; j++) {
				if (0 == ramcmp(name, tweek->touw[j].week, size2)) {
					tterm->tout[z].weekidx = j;
					break;
				}
			}
		}
		backup_tou_term(tterm, 1);
		break;

	case IST_LOAD_LIMIT:
		ret += ramcpy(dat.by, in, sizeof(DAT4));
		setPeriod1(dat.hf[0], DET_LIMIT);
		break;
	case IST_PUSHD_HES: /* 441: SM to HES push */
	case IST_PUSHD_IHD: /* 442: SM TO IHD push */
	case IST_PUSHE_HES: /* 443: SM TO HES event push */
	case IST_PUSHD_LOD: /* 444: SM to HES Load profle push */
	case IST_PUSHD_DAY: /* 445: SM to HES daily profle push */
	case IST_PUSHD_SELFREG: /* 446: Self Registration Push */
	case IST_PUSHD_BILL: /* 447: SM to HES Billing profle push */
	case IST_PUSHD_TAMP:  /* 448: SM to HES Tamper push */
		load_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		ret += ramcpy(&pshcfg.delay, in, sizeof(ui16));
		backup_pushcfg(&pshcfg, obj - IST_PUSHD_HES);
		break;

	default:
		break;
	}
	return ret;
}

ui16	set_value8(cui16 obj, cui08 index, pui08 in) {

	ui16	ret, max , z;

	ret = 0;

	switch (obj) {
	case IST_TIMEZONE:
		tweek = (TOU_WEEK*)share_wk0;
		load_tou_week(tweek, 1);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tweek->touw) / sizeof(AWEEK);
		}
		for (; z < max; z++) {
			ret += ramcpy((pui08)&tweek->touw[z], &in[ret], sizeof(AWEEK));
		}
		backup_tou_week(tweek, 1);
		break;
	default:
		break;
	}
	return ret;
}

ui16	set_value9(cui16 obj, cui08 index, pui08 in) {

	ui08	z, j;
	ui16	ret, max, selector;

	ret = 0;

	switch (obj) {
	case IST_TIMEZONE:
		tday = (TOU_DAY*)share_wk0;
		load_tou_day(tday, 1);
		if (index != 0xff) {
			z = index;
			max = z + 1;
		}
		else {
			z = 0;
			max = sizeof(tday->toud) / sizeof(ADAY);
		}
		for (; z < max; z++) {
			ret++; /*= ramcpy(&out[ret], (pui08)&z, 1);*/
			for (j = 0; j < sizeof(ADAY) / sizeof(ASCR); j++) {
				ret += time2tm(&tday->toud[z].scr[j].start, (PTIME)&in[ret]);
				ret += 6; /* ramcpy(&out[ret], (pui08)rate_obis, sizeof(rate_obis));*/
				ret += ramcpy((pui08)&selector, &in[ret], sizeof(selector));
				tday->toud[z].scr[j].arg = selector;
			}
		}
		backup_tou_day(tday, 1);
		break;
	default:
		break;
	}
	return ret;
}

ui16	set_value10(cui16 obj, cui08 index, pui08 in) {

	ui16	ret;

	ret = 0;

	switch (obj) {
	case IST_TIMEZONE:
		tcal = (TOU_CAL*)share_wk0;
		load_tou_cal(tcal, 1);
		init_active_calender();
		record_log(LOG_CAL_ACT, 0);
		datetime2dttm(&tcal->dttm, (PDATETIME)in);
		tcal->dttm.dmy = 0x00;
		backup_tou_cal(tcal, 1);
		ret = sizeof(DATETIME);
		break;
	default:
		break;
	}
	return ret;
}

/****************************************************************************/
/*																			*/
/*	関数名	:	void	set_scaler_unit(ui16 obj, ui08 s_dat)				*/
/*																			*/
/*	内  容	:	ClassID=3用scaler設定処理									*/
/*																			*/
/*	引  数	:	第１引数	ui16		オブジェクトID						*/
/*				第２引数	ui08		設定データ							*/
/*																			*/
/*	戻り値	:	なし														*/
/*																			*/
/*	備考	:																*/
/*																			*/
/****************************************************************************/
void	set_scaler_unit(ui16 obj, ui08 s_dat)
{
	switch (obj)
	{
		case IST_VOLT1:							/* 平均電圧値(L1)			*/
		case IST_VOLT3:							/* 平均電圧値(L3)			*/
			break;

		default:
			break;
	}
}

/****************************************************************************/
/*																			*/
/*	関数名	:	void	reset_value(ui16)									*/
/*																			*/
/*	内  容	:	設定値初期化処理(ClassID=3)									*/
/*																			*/
/*	引  数	:	第１引数	ui16		オブジェクトID						*/
/*																			*/
/*	戻り値	:	なし														*/
/*																			*/
/*	備考	:																*/
/*																			*/
/****************************************************************************/
void	reset_value(const ui16 obj, const ui16 option)
{
	switch (obj)
	{
	default:
		break;
#if 0
	case IST_BATT_VOLT:
		break;
#endif
	case IST_FRAME_COUNTER2:
	case IST_FRAME_COUNTER3:
	case IST_FRAME_COUNTER4:
	case IST_FRAME_COUNTER5:
	case IST_FRAME_COUNTER6:
		clearCounter(2 + obj - IST_FRAME_COUNTER2);
		break;
	case IST_CPUTIME:
		if (option >= 127) {
			Initialization(option%127);
			set_rsvlog(LOG_RESET, 1);
		}
		else {
			ramset((pui08)&life, 0x00, sizeof(LIFE));
			life.count3 = 6;
			record_log(LOG_RESET, 3);
		}
		break;
#if 0
	case IST_CPUTEMP:
		break;
#endif
	case IST_FWD_PF_MON:
	case IST_BAK_PF_MON:
		resetMonthAPF();
		break;

	case IST_FWD_ENERGY:
		ryo.add[POSIT].wd = 0;
		ramset((pui08)&ryo.keicos[POSIT], 0, sizeof(KEICOS));
		cmd_backup_ryo[0]();
		di();
		ramset((pui08)&hrdbun[POSIT], 0, sizeof(HRDBUN));
		ei();
		record_log(LOG_RESET, 5);
		break;

	case IST_FWD_ENERGY1:
		ryo.tou[8].wd = 0;
		record_log(LOG_RESET, 5);
		break;

	case IST_BAK_ENERGY:
		ryo.add[NEGAT].wd = 0;
		ramset((pui08)&ryo.keicos[NEGAT], 0, sizeof(KEICOS));
		cmd_backup_ryo[0]();
		di();
		ramset((pui08)&hrdbun[NEGAT], 0, sizeof(HRDBUN));
		ei();
		record_log(LOG_RESET, 6);
		break;

	case IST_FWD_QENERGY:
		ryo.add[POSV].wd = 0;
		ramset((pui08)&ryo.keicos[POSV], 0, sizeof(KEICOS));
		cmd_backup_ryo[0]();
		di();
		ramset((pui08)&hrdbun[POSV], 0, sizeof(HRDBUN));
		ei();
		record_log(LOG_RESET, 5);
		break;

	case IST_BAK_QENERGY:
		ryo.add[NEGV].wd = 0;
		ramset((pui08)&ryo.keicos[NEGV], 0, sizeof(KEICOS));
		cmd_backup_ryo[0]();
		di();
		ramset((pui08)&hrdbun[NEGV], 0, sizeof(HRDBUN));
		ei();
		record_log(LOG_RESET, 10);
		break;

	case IST_NET_ENERGY:
		record_log(LOG_RESET, 9);
		break;

	case IST_ABS_ENERGY:
		record_log(LOG_RESET, 10);
		break;

	case IST_AVERAGE_VOLT0:
	case IST_AVERAGE_VOLT1:
	case IST_AVERAGE_VOLT2:
	case IST_AVERAGE_VOLT3:
		iniTotalVol();
		record_log(LOG_RESET, 20);
		break;

	case IST_VOLT0_BLOCK:
	case IST_VOLT1_BLOCK:
	case IST_VOLT2_BLOCK:
	case IST_VOLT3_BLOCK:
		iniTotalBlockVol();
		record_log(LOG_RESET, 21);
		break;

	case IST_MINIMUM_VOLT0:
	case IST_MINIMUM_VOLT1:
	case IST_MINIMUM_VOLT2:
	case IST_MINIMUM_VOLT3:
		resetMinVlt0();
		break;

	case IST_ENERGY:
		lsiwk.eng[0].tim = option / 128;
		lsiwk.eng[1].tim = option / 128;
		addreq();									/* 加算電力量算出要求		*/
		record_log(LOG_RESET, 40);
		break;
	case IST_MAX_FWD_DEMAND:
	case IST_MAX_FWD_VA:
		resetMaxRyo();
		resetMax24Ryo();
		break;
	case IST_MAX_BAK_DEMAND:
	case IST_MAX_BAK_VA:
		break;
	case IST_POWER_OFF_TIME: /* 70: Cumulative power off duration in min*/
		gPwrOff = 0;
		break;
	case IST_BILLING_TIME: /* 71: Billing power on duration in min*/
		gPwrOnBill = 0;
		break;
	case IST_POWER_ON_TIME: /* 72: Cumulative power on duration in min*/
		gPwrOn = 0;
		break;
	};
}

void	inact0_stop(void) {
	tm_inact0.hf = 0x0000;
}
void	inact0_start(ui16 tim) {
	set_timer2(&tm_inact0.hf, tim * 500);
}
ui08	inact0_check(void) {
	ui08	ret;
	ret = D_TIM_NOSTR;				/* ﾀｲﾏｰ未ｽﾀｰﾄｾｯﾄ			*/

	if (tm_inact0.hf != 0x0000)		/* timer動作中				*/
	{
		if (tm_inact0.bt.up == 1)	/* timerﾀｲﾑｱｯﾌﾟ				*/
		{
			ret = D_TIM_UP;			/* ﾀｲﾑｱｯﾌﾟｾｯﾄ				*/
		}
		else
		{
			ret = D_TIM_NOUP;		/* 未ﾀｲﾑｱｯﾌﾟｾｯﾄ				*/
		}
	}
	return (ret);
}

void	inact1_stop(void) {
	tm_inact1.hf = 0x0000;
}

void	inact1_start(ui16 tim) {
	set_timer2(&tm_inact1.hf, tim * 500);
}

ui08	inact1_check(void) {
	ui08	ret;
	ret = D_TIM_NOSTR;					/* ﾀｲﾏｰ未ｽﾀｰﾄｾｯﾄ			*/
	if (tm_inact1.hf != 0x0000)			/* timer動作中				*/
	{
		if (tm_inact1.bt.up == 1)		/* timerﾀｲﾑｱｯﾌﾟ				*/
		{
			ret = D_TIM_UP;				/* ﾀｲﾑｱｯﾌﾟｾｯﾄ				*/
		}
		else
		{
			ret = D_TIM_NOUP;			/* 未ﾀｲﾑｱｯﾌﾟｾｯﾄ				*/
		}
	}
	return (ret);
}

void	inact2_stop(void) {
	tm_inact2.hf = 0x0000;
}

void	inact2_start(ui16 tim) {
	set_timer2(&tm_inact2.hf, tim * 500);
}

ui08	inact2_check(void) {
	ui08	ret;
	ret = D_TIM_NOSTR;					/* ﾀｲﾏｰ未ｽﾀｰﾄｾｯﾄ			*/
	if (tm_inact2.hf != 0x0000)			/* timer動作中				*/
	{
		if (tm_inact2.bt.up == 1)		/* timerﾀｲﾑｱｯﾌﾟ				*/
		{
			ret = D_TIM_UP;				/* ﾀｲﾑｱｯﾌﾟｾｯﾄ				*/
		}
		else
		{
			ret = D_TIM_NOUP;			/* 未ﾀｲﾑｱｯﾌﾟｾｯﾄ				*/
		}
	}
	return (ret);
}

static void VisibleTolcd8(PLCD8 out, const ui08* in) {
	out->dt0 = in[0] - 0x30;
	out->dt1 = in[1] - 0x30;
	out->dt2 = in[2] - 0x30;
	out->dt3 = in[3] - 0x30;
	out->dt4 = in[4] - 0x30;
	out->dt5 = in[5] - 0x30;
	out->dt6 = in[6] - 0x30;
	out->dt7 = in[7] - 0x30;
}

static void	dateTolcd8(PLCD8 out, const PBCD10 pbcd) {
	out->dt2 = pbcd->dt01 >> 4;
	out->dt3 = pbcd->dt01 & 0x0f;
	out->dt4 = pbcd->dt23 >> 4;
	out->dt5 = pbcd->dt23 & 0x0f;
	out->dt6 = pbcd->dt45 >> 4;
	out->dt7 = pbcd->dt45 & 0x0f;
}

static void	timeTolcd8(PLCD8 out, const PBCD10 pbcd) {
	out->dt0 = pbcd->dt01 >> 4;
	out->dt1 = pbcd->dt01 & 0x0f;
	out->dt2 = endofLCD - 1;
	out->dt3 = pbcd->dt23 >> 4;
	out->dt4 = pbcd->dt23 & 0x0f;
	out->dt5 = endofLCD - 1;
	out->dt6 = pbcd->dt45 >> 4;
	out->dt7 = pbcd->dt45 & 0x0f;
}

static void hex32tolcd8(PLCD8 out, const ui32 in) {

	ui32	hex;
	ui08	top, tmp, bcd[4], flg;
	hex = in;
	flg = abs32(&hex);

	bcd[3] = hex % 10;
	hex /= 10;
	bcd[3] |= (hex % 10) << 4;
	hex /= 10;
	bcd[2] = hex % 10;
	hex /= 10;
	bcd[2] |= (hex % 10) << 4;
	hex /= 10;
	bcd[1] = hex % 10;
	hex /= 10;
	bcd[1] |= (hex % 10) << 4;
	hex /= 10;
	bcd[0] = hex % 10;
	hex /= 10;

	top = 0;
	tmp = bcd[0] >> 4;
	if (tmp != 0) {
		out->dt0 = tmp;
		top = 1;
	}
	else {
		if (flg > 0) {
			out->dt0 = endofLCD - 3;
		}
	}
	tmp = bcd[0] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt1 = tmp;
		top = 1;
	}
	tmp = bcd[1] >> 4;
	if (tmp != 0) {
		out->dt2 = tmp;
		top = 1;
	}
	tmp = bcd[1] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt3 = tmp;
		top = 1;
	}
	tmp = bcd[2] >> 4;
	if (tmp != 0 || top > 0) {
		out->dt4 = tmp;
		top = 1;
	}
	tmp = bcd[2] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt5 = tmp;
		top = 1;
	}
	tmp = bcd[3] >> 4;
	if (tmp != 0 || top > 0) {
		out->dt6 = tmp;
		top = 1;
	}
	out->dt7 = bcd[3] & 0x0f;
}

static void time6tolcd8(PLCD8 out) {

	ui08 bcd[4];

	bcd[0] = addr_date()->ji;
	bcd[1] = addr_date()->fun;
	bcd[2] = addr_date()->byo;

	out->dt0 = bcd[0] >> 4;
	out->dt1 = bcd[0] & 0x0f;
	out->dt2 = endofLCD - 1;
	out->dt3 = bcd[1] >> 4;
	out->dt4 = bcd[1] & 0x0f;
	out->dt5 = endofLCD - 1;
	out->dt6 = bcd[2] >> 4;
	out->dt7 = bcd[2] & 0x0f;
}

/*32ビットの下2バイトをHEX表示する*/
static void hex32tolcd4(PLCD8 out, const ui32 in) {

	ui32	hex;
	ui08	top, tmp, bcd[4];

	hex = in;

	bcd[3] = hex % 16;
	hex /= 16;
	bcd[3] |= (hex % 16) << 4;
	hex /= 16;
	bcd[2] = hex % 16;
	hex /= 16;
	bcd[2] |= (hex % 16) << 4;
	hex /= 16;
	bcd[1] = hex % 16;
	hex /= 16;
	bcd[1] |= (hex % 16) << 4;
	hex /= 16;
	bcd[0] = hex % 16;
	hex /= 16;
	bcd[0] |= (hex % 16) << 4;

	top = 0;
	tmp = bcd[0] >> 4;
	if (tmp != 0) {
		out->dt2 = tmp;
		top = 1;
	}
	tmp = bcd[0] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt3 = tmp;
		top = 1;
	}
	tmp = bcd[1] >> 4;
	if (tmp != 0) {
		out->dt2 = tmp;
		top = 1;
	}
	tmp = bcd[1] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt3 = tmp;
		top = 1;
	}
	tmp = bcd[2] >> 4;
	if (tmp != 0 || top > 0) {
		out->dt4 = tmp;
		top = 1;
	}
	tmp = bcd[2] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt5 = tmp;
		top = 1;
	}
	tmp = bcd[3] >> 4;
	if (tmp != 0 || top > 0) {
		out->dt6 = tmp;
		top = 1;
	}
	out->dt7 = bcd[3] & 0x0f;
}

static void hex16tolcd6(PLCD6 out, cui16 in) {

	ui16	hex;
	ui08	top, tmp, bcd[3];
	hex = in;

	bcd[2] = hex % 10;
	hex /= 10;
	bcd[2] |= (hex % 10) << 4;
	hex /= 10;
	bcd[1] = hex % 10;
	hex /= 10;
	bcd[1] |= (hex % 10) << 4;
	hex /= 10;
	bcd[0] = hex % 10;
	hex /= 10;
	bcd[0] |= (hex % 10) << 4;

	top = 0;
	tmp = bcd[0] >> 4;
	if (tmp != 0) {
		out->dt0 = tmp;
		top = 1;
	}
	tmp = bcd[0] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt1 = tmp;
		top = 1;
	}
	tmp = bcd[1] >> 4;
	if (tmp != 0 || top > 0) {
		out->dt2 = tmp;
		top = 1;
	}
	tmp = bcd[1] & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt3 = tmp;
		top = 1;
	}
	tmp = bcd[2] >> 4;
	if (tmp != 0 || top > 0) {
		out->dt4 = tmp;
		top = 1;
	}
	out->dt5 = bcd[2] & 0x0f;
}

static void bcd10tolcd8(PLCD8 out, const PBCD10 pbcd) {
	ui08	top, tmp;

	top = 0;
	tmp = pbcd->dt01 >> 4;
	if (tmp != 0) {
		out->dt2 = tmp;
		top = 1;
	}
	tmp = pbcd->dt01 & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt3 = tmp;
		top = 1;
	}
	tmp = pbcd->dt23 >> 4;
	if (tmp != 0 || top > 0) {
		out->dt4 = tmp;
		top = 1;
	}
	tmp = pbcd->dt23 & 0x0f;
	if (tmp != 0 || top > 0) {
		out->dt5 = tmp;
		top = 1;
	}
	tmp = pbcd->dt45 >> 4;
	if (tmp != 0 || top > 0) {
		out->dt6 = tmp;
		top = 1;
	}
	out->dt7 = pbcd->dt45 & 0x0f;
}

static void NorTolcd8(PLCD8 out, const ui32 code, ui08 sub) {

	CUMCNT cnt;
	ui32 status = code;
	ui08 rlt;
	ui08 pos;

	if (dsppos == 0) {
		pos = sub + dsppos;
	}
	else {
		pos = sub + 2;
	}
	switch (pos) {
	case 0:	/*RTC*/
		out->dt0 = 'C' - 'A' + 10;
		out->dt1 = 'L' - 'A' + 10;
		out->dt2 = endofLCD - 1;
		rlt = status & 0x00000080;
		break;
	case 1:	/*BATTERY*/
		out->dt0 = 'B' - 'A' + 10;
		out->dt1 = 'T' - 'A' + 10;
		out->dt2 = endofLCD - 1;
		rlt = status & 0x00000002;
		break;
	case 2:	/*EEPY*/
		out->dt0 = 'E' - 'A' + 10;
		out->dt1 = 'P' - 'A' + 10;
		out->dt2 = endofLCD - 1;
		rlt = status & 0x00000040;
		break;
	case 3:	/*RST*/
		out->dt0 = 'R' - 'A' + 10;
		out->dt1 = 'T' - 'A' + 10;
		out->dt2 = endofLCD - 1;
		rlt = status & 0x00000010;
		break;
	}
	if (pos == 3) {
		load_cumcnt(&cnt, eCpuReset);
		if (cnt.cumcnt > 999) {
			cnt.cumcnt = 999;
		}
		hex32tolcd8(out, cnt.cumcnt);
	}
	else{
		if (rlt > 0) {
			out->dt3 = 'F' - 'A' + 10;
			out->dt4 = 'A' - 'A' + 10;
			out->dt5 = 'I' - 'A' + 10;
			out->dt6 = 'L' - 'A' + 10;
		}
		else {
			out->dt3 = 'G' - 'A' + 10;
			out->dt4 = 'O' - 'A' + 10;
			out->dt5 = 'O' - 'A' + 10;
			out->dt6 = 'D' - 'A' + 10;
		}
		out->dt7 = endofLCD - 1;
	}
}

static void MissingNeutral(PLCD8 out) {

	out->dt0 = 'N' - 'A' + 10;
	out->dt1 = 'M' - 'A' + 10;
	out->dt2 = 7;
	out->dt3 = endofLCD - 1;
	out->dt4 = endofLCD - 1;
	out->dt5 = endofLCD - 1;
	out->dt6 = endofLCD - 1;
	out->dt7 = endofLCD - 1;
}

static void NICTolcd8(PLCD8 out, cpui08 string) {

	out->dt0 = 'N' - 'A' + 10;
	out->dt1 = 'I' - 'A' + 10;
	out->dt2 = 'C' - 'A' + 10;
	out->dt3 = endofLCD - 1;
	if (string[0] != 0x20) {
		out->dt4 = 'O' - 'A' + 10;
		out->dt5 = 'N' - 'A' + 10;
		out->dt6 = endofLCD - 1;
		out->dt7 = endofLCD - 1;
	}
	else {
		out->dt4 = 'O' - 'A' + 10;
		out->dt5 = 'F' - 'A' + 10;
		out->dt6 = 'F' - 'A' + 10;
		out->dt7 = endofLCD - 1;
	}
}

static void mode_and_pay(PLCD8 out) {

	ui08 pay, mode;

	pay = get_paid_mode();
	mode = get_meter_mode();

	if (mode == 0) {
		out->dt0 = 'F' - 'A' + 10;
		out->dt1 = 'O' - 'A' + 10;
		out->dt2 = 'R' - 'A' + 10;
	}
	else {
		out->dt0 = 'N' - 'A' + 10;
		out->dt1 = 'E' - 'A' + 10;
		out->dt2 = 'T' - 'A' + 10;
	}
	out->dt3 = endofLCD - 3;
	if (pay == 0) {
		out->dt4 = 'P' - 'A' + 10;
		out->dt5 = 'O' - 'A' + 10;
	}
	else {
		out->dt4 = 'P' - 'A' + 10;
		out->dt5 = 'R' - 'A' + 10;
	}
	out->dt6 = endofLCD - 1;
	out->dt7 = endofLCD - 1;
}

static void CodeTolcd8(PLCD8 out, const ui08 code) {
	
	ui08 status = code;

	if (status & 0x80) {
		out->dt0 = 'U' - 'A' + 10;
		out->dt1 = 'N' - 'A' + 10;
		out->dt2 = 'D' - 'A' + 10;
		out->dt3 = 'E' - 'A' + 10;
		out->dt4 = 'F' - 'A' + 10;
		out->dt5 = endofLCD - 1;
		out->dt6 = endofLCD - 1;
		out->dt7 = endofLCD - 1;
	}
	else {
		out->dt0 = 'E' - 'A' + 10;
		out->dt1 = 'R' - 'A' + 10;
		out->dt2 = 'R' - 'A' + 10;
		out->dt3 = endofLCD - 1;
		out->dt4 = endofLCD - 1;
		out->dt7 = status % 10;
		status /= 10;
		out->dt6 = status % 10;
		status /= 10;
		out->dt5 = status % 10;
	}
}

static ui16 nowdisp;
static ui08 dispBuffer[256];
void dispDigit(PLCD8 out, DISSET *disp, cui08 sub)
{
	ui08	digit, num, flg, yuko, dot, cur, mode;
	DTTM	dttm;
	ui16	obj;
	BCD10	bcd;
	DAT4	dat0, dat1;

	ramset((pui08)out, endofLCD - 1, sizeof(LCD8));	/*スペースで初期化*/
	dat0.wd = 0;

	obj = disp->lidx.bt.idx;
	cur = disp->lidx.bt.prv;
	mode = 0;
	if (disp->lidx.bt.div > 0) {
		switch (obj) {
		case IST_DATETIME_NOW:
		case IST_RECHARGE_TIME:
		case IST_BALANCE_TIME:
			if (sub == 0) {
				mode = 254;
			}
			else {
				mode = 255;
			}
			break;
		default:
			break;
		}
	}
	if (disp->lidx.bt.rgx > 0) {
		switch (sub) {
		case 1:
			mode = 254;
			break;
		case 2:
			mode = 255;
			break;
		default:
			break;
		}
	}
	if (mode == 0) {
		digit = disp->option.by[0] & 0x0f;	/*表示桁*/
		num = disp->option.by[0] >> 4;		/*整数桁*/
		dot = num - digit;
		dot += (disp->option.by[1] & 0x0f);
		if (disp->option.by[1] & 0x80) {	/*ZERO埋め*/
			yuko = digit - num + 1;
			switch (yuko) {
			case 8:
				out->dt0 = 0;
			case 7:
				out->dt1 = 0;
			case 6:
				out->dt2 = 0;
			case 5:
				out->dt3 = 0;
			case 4:
				out->dt4 = 0;
			case 3:
				out->dt5 = 0;
			case 2:
				out->dt6 = 0;
			case 1:
				out->dt7 = 0;
				break;
			}
		}
		else {
			switch (digit) {
			case 8:
				out->dt0 = 0;
			case 7:
				out->dt1 = 0;
			case 6:
				out->dt2 = 0;
			case 5:
				out->dt3 = 0;
			case 4:
				out->dt4 = 0;
			case 3:
				out->dt5 = 0;
			case 2:
				out->dt6 = 0;
			case 1:
				out->dt7 = 0;
				break;
			}
		}
	}
	switch (obj)
	{
	case IST_FAULT:/* Meter rom event related objects */
		mode = 2;
		NorTolcd8(out, evests.fault.wd, sub);
		break;

	case IST_BATT_VOLT:
		mode = 1;
		dat0.wd = get_vrtc();
		dat0.wd *= 100;
		dat0.wd += get_vbat();
		break;

	case IST_TEMPERATURE:
		dat0.wd = hot1s.val1;
		break;

	case IST_CPUTEMP:
		dat0.wd = hot1s.val0;
		break;

	case IST_FWD_ENERGY:					/* 有効電力量(順潮流)		*/
		if (cur == 0) {
			if (sub == 0) {
				readRyo(&dat0, dot, 2);
			}
			else {
				readTou(&dat0, dot, 16 + sub - 1);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff20) {
					nowdisp = 0xff20;
					ExtractBillingRecord(dispBuffer, 100 + cur, 2);
				}
			}
			ramcpy(dat1.by, ((RECBLD2*)dispBuffer)->imp[sub].cum, 4);
			if (dat1.wd == 0xaaaaaaaa) {
				dat0.wd = 0;
			}
			else {
				digit5ConvTruncate(dat0.by, &dat1, dot);
			}
		}
		break;

	case IST_BAK_ENERGY:					/* 有効電力量(順潮流)		*/
		if (cur == 0) {
			if (sub == 0) {
				readRyo(&dat0, dot, 3);
			}
			else {
				readTou(&dat0, dot, 24 + sub - 1);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff21) {
					nowdisp = 0xff21;
					ExtractBillingRecord(dispBuffer, 100 + cur, 2);
				}
			}
			ramcpy(dat1.by, ((RECBLD2*)dispBuffer)->exp[sub].cum, 4);
			if (dat1.wd == 0xaaaaaaaa) {
				dat0.wd = 0;
			}
			else {
				digit5ConvTruncate(dat0.by, &dat1, dot);
			}
		}
		break;

	case IST_FWD_VAH:
		if (cur == 0) {
			if (sub == 0) {
				readRyo(&dat0, dot, 0);
			}
			else {
				readTou(&dat0, dot, sub - 1);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff30) {
					nowdisp = 0xff30;
					ExtractBillingRecord(dispBuffer, 100 + cur, 3);
				}
			}
			ramcpy(dat1.by, ((RECBLD3*)dispBuffer)->imp[sub].cum, 4);
			if (dat1.wd == 0xaaaaaaaa) {
				dat0.wd = 0;
			}
			else {
				digit5ConvTruncate(dat0.by, &dat1, dot);
			}
		}
		break;

	case IST_BAK_VAH:
		if (cur == 0) {
			if (sub == 0) {
				readRyo(&dat0, dot, 1);
			}
			else {
				readTou(&dat0, dot, 8 + sub - 1);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff31) {
					nowdisp = 0xff31;
					ExtractBillingRecord(dispBuffer, 100 + cur, 3);
				}
			}
			ramcpy(dat1.by, ((RECBLD3*)dispBuffer)->exp[sub].cum, 4);
			if (dat1.wd == 0xaaaaaaaa) {
				dat0.wd = 0;
			}
			else {
				digit5ConvTruncate(dat0.by, &dat1, dot);
			}
		}
		break;

	case IST_FREQ:
		readHelz1(&dat0, unitRTO[2]);
		break;

	case IST_FWD_QENERGY:					/* 有効電力量(順潮流)		*/
		dat0.wd = getRyo(&ryo.add[4]);
		dat1.wd = getRyo(&ryo.add[6]);
		dat0.wd += dat1.wd;
		dat0.wd %= MAX_RYO;
		dat0.wd /= 1000;
#if 0
		if (cur == 0) {
			if (sub == 0) {
				readRyo(&dat0, dot, 4);
			}
			else {
				readTou(&dat0, dot, 32 + sub - 1);
			}
		}
		else {
			if (nowdisp != 0xff32) {
				nowdisp = 0xff32;
				ExtractBillingRecord(dispBuffer, 100 + cur, 3);
			}
			ramcpy(dat1.by, ((RECBLD3*)dispBuffer)->varh[0].u, 4);
			if (dat1.wd == 0xaaaaaaaa) {
				dat0.wd = 0;
			}
			else {
				digit5ConvTruncate(dat0.by, &dat1, dot);
			}
		}
#endif
		break;

	case IST_BAK_QENERGY:					/* 有効電力量(順潮流)		*/
		dat0.wd = getRyo(&ryo.add[5]);
		dat1.wd = getRyo(&ryo.add[7]);
		dat0.wd += dat1.wd;
		dat0.wd %= MAX_RYO;
		dat0.wd /= 1000;
#if 0
		if (cur == 0) {
			if (sub == 0) {
				readRyo(&dat0, dot, 5);
			}
			else {
				readTou(&dat0, dot, 40 + sub - 1);
			}
		}
		else {
			if (nowdisp != 0xff33) {
				nowdisp = 0xff33;
				ExtractBillingRecord(dispBuffer, 100 + cur, 3);
			}
			ramcpy(dat1.by, ((RECBLD3*)dispBuffer)->varh[1].u, 4);
			if (dat1.wd == 0xaaaaaaaa) {
				dat0.wd = 0;
			}
			else {
				digit5ConvTruncate(dat0.by, &dat1, dot);
			}
		}
#endif
		break;
#if 0
	case IST_ABS_ENERGY:
		readAbsActive(&dat0, dot);			/*3=kiro*/
		break;

	case IST_ABS_QENERGY:
		readAbsReactive(&dat0, dot);		/*3=kiro*/
		break;

	case IST_NET_ENERGY:
		flg = readNetActive(&dat0, dot);	/*3=kiro*/
		if (flg == 0xff) {
			dat0.wd = 0;
		}
		else {
			abs32(&dat0.wd);
		}
		break;

	case IST_NET_QENERGY:
		readNetReactive(&dat0, dot);		/*3=kiro*/
		break;
#endif
	case IST_AMPRC:							/* 平均電流値(L1)			*/
		readAmp0(&dat0, dot);
		if (dat0.hf[1] != 0x0f0f) {
			abs32(&dat0.wd);
		}
		else {
			dat0.wd = 0;
		}
		break;

	case IST_AMPR0:
	case IST_AMPR1:
		readAmp1(&dat0, dot);
		if (dat0.hf[1] != 0x0f0f) {
			abs32(&dat0.wd);
		}
		else {
			dat0.wd = 0;
		}
		break;

	case IST_AMPR2: /* 152: Current L2*/
	case IST_AMPRG: /* 164: Neutral current*/
		readAmp2(&dat0, dot);
		if (dat0.hf[1] != 0x0f0f) {
			abs32(&dat0.wd);
		}
		else {
			dat0.wd = 0;
		}
		break;
#if 0
	case IST_AMPR3:							/* 平均電流値(L3)			*/
		readAmp3(&dat0, dot);
		if (dat0.hf[1] == 0x0f0f) {
			dat0.wd = 0;
		}
		flg = abs32(&dat0.wd);
		break;
#endif
	case IST_VOLT0:
	case IST_VOLT1:							/* 平均電圧値(L1)			*/
		readVlt1(&dat0, dot);
		if (dat0.hf[1] == 0x0f0f) {
			dat0.wd = 0;
		}
		break;
#if 0
	case IST_VOLT2:							/* 平均電圧値(L1)			*/
		readVlt2(&dat0, dot);
		if (dat0.hf[1] == 0x0f0f) {
			dat0.wd = 0;
		}
		break;

	case IST_VOLT3:							/* 平均電圧値(L3)			*/
		readVlt3(&dat0, dot);
		if (dat0.hf[1] == 0x0f0f) {
			dat0.wd = 0;
		}
		break;
#endif
	case IST_POWER:
		readImpExp(&dat0, dot);		/*3=kiro*/
		flg = abs32(&dat0.wd);
		break;

	case IST_APPERENT:
		readLftRgt(&dat0, dot);		/*3=kiro*/
		abs32(&dat0.wd);
		break;

	case IST_QPOWER:
		readFwdBak(&dat0, dot);		/*3=kiro*/
		flg = abs32(&dat0.wd);
		break;

	case IST_CPUTIME:
		CpuTime((ui32*)&dat0.wd);
		break;

	case IST_PF_BLOCK:
		flg = 0;
		dat0.wd = readBlockAPF();
		break;

	case IST_POWER_FACTOR:
		dat0.wd = powerfactor.wd;
		break;

	case IST_POWER_FACTOR1:
		flg = readPowerFactor1(&dat0, dot);
		if (flg > 0) {
			dat0.wd = 0 - dat0.wd;
		}
		break;

	case IST_POWER_FACTOR2:
		flg = readPowerFactor2(&dat0, dot);
		if (flg > 0) {
			dat0.wd = 0 - dat0.wd;
		}
		break;
#if 0
	case IST_POWER_FACTOR3:
		readPowerFactor3(&dat0, dot);
		break;
#endif
	case IST_MAX_FWD_DEMAND:
		if (cur == 0) {
			if (sub < 3) {
				if (sub == 0) {
					getMaxRyo(&dat0, 2);
				}
				else {
					getMaxRyoDate(&dat0, 2);
				}
			}
			else {
				getMaxRyo(&dat0, 22 + sub - 2);
			}
		}
		else {
			if (sub < 3) {
				if (sub == 0) {
					if (nowdisp != 0xff22) {
						nowdisp = 0xff22;
						ExtractBillingRecord(dispBuffer, 100 + cur, 2);
					}
					ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->imp[0].max, 4);
				}
				else {
					ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->imp[0].date, 4);
				}
			}
			else {
				ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->imp[sub - 2].max, 4);
			}
		}
		break;

	case IST_MAX_BAK_DEMAND:
		if (cur == 0) {
			if (sub < 3) {
				if (sub == 0) {
					getMaxRyo(&dat0, 3);
				}
				else {
					getMaxRyoDate(&dat0, 3);
				}
			}
			else {
				getMaxRyo(&dat0, 30 + sub - 2);
			}
		}
		else {
			if (sub < 3) {
				if (sub == 0) {
					if (nowdisp != 0xff23) {
						nowdisp = 0xff23;
						ExtractBillingRecord(dispBuffer, 100 + cur, 2);
					}
					ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->exp[0].max, 4);
				}
				else {
					ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->exp[0].date, 4);
				}
			}
			else {
				ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->exp[sub - 2].max, 4);
			}
		}
		break;

	case IST_MAX_FWD_VA:
		if (cur == 0) {
			if (sub < 3) {
				if (sub == 0) {
					getMaxRyo(&dat0, 0);
				}
				else {
					getMaxRyoDate(&dat0, 0);
				}
			}
			else {
				getMaxRyo(&dat0, 6 + sub - 2);
			}
		}
		else {
			if (sub < 3) {
				if (sub == 0) {
					if (nowdisp != 0xff34) {
						nowdisp = 0xff34;
						ExtractBillingRecord(dispBuffer, 100 + cur, 3);
					}
					ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->imp[0].max, 4);
				}
				else {
					ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->imp[0].date, 4);
				}
			}
			else {
				ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->imp[sub - 2].max, 4);
			}
		}
		break;

	case IST_MAX_BAK_VA:
		if (cur == 0) {
			if (sub < 3) {
				if (sub == 0) {
					getMaxRyo(&dat0, 1);
				}
				else {
					getMaxRyoDate(&dat0, 1);
				}
			}
			else {
				getMaxRyo(&dat0, 14 + sub - 2);
			}
		}
		else {
			if (sub < 3) {
				if (sub == 0) {
					if (nowdisp != 0xff35) {
						nowdisp = 0xff35;
						ExtractBillingRecord(dispBuffer, 100 + cur, 3);
					}
					ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->exp[0].max, 4);
				}
				else {
					ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->exp[0].date, 4);
				}
			}
			else {
				ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->exp[sub - 2].max, 4);
			}
		}
		break;

	case IST_MAX_FWD_QDEMAND:
		getMaxRyo(&dat0, 4);
		break;

	case IST_MAX_BAK_QDEMAND:
		getMaxRyo(&dat0, 5);
		break;

	case IST_MAGNET:
		if (sub == 0) {
		}
		else {
			flg = sub - 1;
			switch (flg) {
			case 0:
				getMagX(&dat0, dot);
				flg = abs32(&dat0.wd);
				break;
			case 1:
				getMagY(&dat0, dot);
				flg = abs32(&dat0.wd);
				break;
			case 2:
				getMagZ(&dat0, dot);
				flg = abs32(&dat0.wd);
				break;
			case 3:
//				getMagT(&dat0, dot);
				flg = abs32(&dat0.wd);
				break;
			}
		}
		break;

	case IST_RECHARGE_AMOUNT: /* 14: Last token recharge amount*/
		dat0.wd = gRechargeAmount;
		break;

	case IST_RECHARGE_TIME: /* 15: Last token recharge time */
		dat0.wd = gRechargeTime;
		break;

	case IST_TOTAL_AMOUNT: /* 16: Total amount at last recharge */
		dat0.wd = gTotalAmount;
		break;

	case IST_BALANCE_AMOUNT: /* 17: Current balance amount*/
		dat0.wd = gBalanceAmount;
		break;

	case IST_BALANCE_TIME: /* 18: Current balance time*/
		dat0.wd = gBalanceTime;
		break;

	case IST_SERIAL_NO:
		VisibleTolcd8(out, &meter.id1[2]);
		mode = 2;
		break;

	case IST_DATETIME_NOW:
		nowdisp = obj;
		dat0.wd = cursec.wd;
		break;

	case IST_MAX24_FWD_DEMAND:
		if (cur == 0) {
			if (sub == 0) {
				getMax24Ryo(&dat0, 2);
			}
			else {
				getMax24RyoDate(&dat0, 2);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff24) {
					nowdisp = 0xff24;
					ExtractBillingRecord(dispBuffer, 100 + cur, 2);
				}
				ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->max24[0].max, 4);
			}
			else {
				ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->max24[0].date, 4);
			}
		}
		break;

	case IST_MAX24_BAK_DEMAND:
		if (cur == 0) {
			if (sub == 0) {
				getMax24Ryo(&dat0, 3);
			}
			else {
				getMax24RyoDate(&dat0, 3);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff25) {
					nowdisp = 0xff25;
					ExtractBillingRecord(dispBuffer, 100 + cur, 2);
				}
				ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->max24[1].max, 4);
			}
			else {
				ramcpy(dat0.by, ((RECBLD2*)dispBuffer)->max24[1].date, 4);
			}
		}
		break;

	case IST_MAX24_FWD_VA:
		if (cur == 0) {
			if (sub == 0) {
				getMax24Ryo(&dat0, 0);
			}
			else {
				getMax24RyoDate(&dat0, 0);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff26) {
					nowdisp = 0xff26;
					ExtractBillingRecord(dispBuffer, 100 + cur, 3);
				}
				ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->max24[0].max, 4);
			}
			else {
				ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->max24[0].date, 4);
			}
		}
		break;

	case IST_MAX24_BAK_VA:
		if (cur == 0) {
			if (sub == 0) {
				getMax24Ryo(&dat0, 1);
			}
			else {
				getMax24RyoDate(&dat0, 1);
			}
		}
		else {
			if (sub == 0) {
				if (nowdisp != 0xff27) {
					nowdisp = 0xff27;
					ExtractBillingRecord(dispBuffer, 100 + cur, 3);
				}
				ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->max24[1].max, 4);
			}
			else {
				ramcpy(dat0.by, ((RECBLD3*)dispBuffer)->max24[1].date, 4);
			}
		}
		break;

	case IST_FWD_PF_MON:
		if (cur == 0) {
			dat0.wd = readMonthAPF(0, 0);
		}
		else {
			if (nowdisp != 0xff10) {
				nowdisp = 0xff10;
				ExtractBillingRecord(dispBuffer, 100 + cur, 1);
			}
			ramcpy(dat0.by, ((RECBLD1*)dispBuffer)->apfp, 2);
			if (dat0.hf[0] == 0xaaaa) {
				dat0.wd = 0;
			}
		}
		break;

	case IST_TAMPER_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_TAMPER_COUNT) {
				nowdisp = IST_TAMPER_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eTamp);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_BILLING_COUNT:
		dsppos = 1;
		if (sub == 0) {
			if (nowdisp != IST_BILLING_COUNT) {
				nowdisp = IST_BILLING_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eBill);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_MAGNET_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_MAGNET_COUNT) {
				nowdisp = IST_MAGNET_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eMagnet);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_ESD_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_ESD_COUNT) {
				nowdisp = IST_ESD_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eESD);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_TOPCV_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_TOPCV_COUNT) {
				nowdisp = IST_TOPCV_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eNICCover);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_CONNECT_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_CONNECT_COUNT) {
				nowdisp = IST_CONNECT_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eConnect);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_DISCON_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_DISCON_COUNT) {
				nowdisp = IST_DISCON_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eDiscon);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_EVENT_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_EVENT_COUNT) {
				nowdisp = IST_EVENT_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eEvent);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_RESET_COUNT:
		if (sub == 0) {
			if (nowdisp != IST_RESET_COUNT) {
				nowdisp = IST_RESET_COUNT;
				load_cumcnt(((CUMCNT*)dispBuffer), eCpuReset);
			}
			dat0.wd = ((CUMCNT*)dispBuffer)->cumcnt;
		}
		else {
			dat0.wd = ((CUMCNT*)dispBuffer)->timestamp;
		}
		break;

	case IST_RSSI: /* (76) 76: RSSI */
		dat0.wd = NIC_RSSI();
		break;

	case IST_NIC_SERIAL:
		read_othinf(share_wk0, eCID1, 2);
		NICTolcd8(out, share_wk0);
		mode = 2;
		break;

	case IST_NIC_STATUS:
		CodeTolcd8(out, get_nic_status());
		mode = 2;
		break;

	case IST_METER_MODE:
		nowdisp = obj;
		mode_and_pay(out);
		mode = 2;
		break;

	default:								/* 上記以外の場合			*/
		break;
	}
	if (dat0.wd == 0xaaaaaaaa || dat0.wd == 0xffffffff || dat0.wd == 0x0f0f0f0f) {
		dat0.wd = 0;
	}
	switch (mode) {
	case 0:
		hex32tolcd8(out, dat0.wd);
		break;
	case 1:
		hex32tolcd4(out, dat0.wd);
		break;
	case 2:
		break;
	case 254:
		get_dttm_from_sec(&dttm, &dat0);
		bcd.dt01 = dttm.hi;
		bcd.dt23 = dttm.gatu;
		bcd.dt45 = dttm.nen;
		dateTolcd8(out, &bcd);
		break;

	case 255:
		get_dttm_from_sec(&dttm, &dat0);
		bcd.dt01 = dttm.ji;
		bcd.dt23 = dttm.fun;
		bcd.dt45 = dttm.byo;
		timeTolcd8(out, &bcd);
		break;
	}
}

void subdispDigit(PLCD6 out, DISSET *disp)
{
	ui16 val;

	val = disp->lidx.bt.prv;
	val *= 1000;
	val += disp->serial.hf;

	ramset((pui08)out, endofLCD - 1, sizeof(LCD6));	/*スペースで初期化*/
	if (val < 65536) {
		hex16tolcd6(out, val);
		switch (disp->serial.hf) {
		case 0:
			out->dt1 = 5;
			break;
		case 3:
			out->dt1 = 'I' - 'A' + 0x0a;
			out->dt2 = 'P' - 'A' + 0x0a;
			break;
		case 4:
			out->dt1 = 'I' - 'A' + 0x0a;
			out->dt2 = 'N' - 'A' + 0x0a;
			break;
		case 6:
			out->dt1 = 'P' - 'A' + 0x0a;
			out->dt2 = 'F' - 'A' + 0x0a;
			break;
		case 10: case 12: case 14:
		case 20: case 22:
			if (disp->lidx.bt.prv > 0) {
				out->dt1 = 'B' - 'A' + 0x0a;
				out->dt2 = 0x0f;
				out->dt3 = endofLCD - 1;
			}
			else {
				out->dt1 = 0x0f;
			}
			break;
		case 11: case 13: case 15:
		case 21: case 23:
			if (disp->lidx.bt.prv > 0) {
				out->dt1 = 'B' - 'A' + 0x0a;
				out->dt2 = 0x0e;
				out->dt3 = endofLCD - 1;
			}
			else {
				out->dt1 = 0x0e;
			}
			break;
		case 50:
			if (disp->lidx.bt.prv > 0) {
				out->dt1 = 'B' - 'A' + 0x0a;
				out->dt2 = 'A' - 'A' + 0x0a;
				out->dt3 = 'P' - 'A' + 0x0a;
				out->dt4 = 'F' - 'A' + 0x0a;
				out->dt5 = endofLCD - 1;
			}
			else {
				out->dt1 = 'A' - 'A' + 0x0a;
				out->dt2 = 'P' - 'A' + 0x0a;
				out->dt3 = 'F' - 'A' + 0x0a;
				out->dt4 = endofLCD - 1;
				out->dt5 = endofLCD - 1;
			}
			break;
		case 410:
		case 412:
			out->dt1 = 'H' - 'A' + 0x0a;
			out->dt2 = 'F' - 'A' + 0x0a;
			out->dt3 = endofLCD - 1;
			break;
		case 411:
		case 413:
			out->dt1 = 'H' - 'A' + 0x0a;
			out->dt2 = 'E' - 'A' + 0x0a;
			out->dt3 = endofLCD - 1;
			break;
		case 500:
			out->dt1 = 'L' - 'A' + 0x0a;
			out->dt2 = 'R' - 'A' + 0x0a;
			out->dt3 = endofLCD - 1;
			break;
		case 510:
			out->dt1 = 'A' - 'A' + 0x0a;
			out->dt2 = 'L' - 'A' + 0x0a;
			out->dt3 = 'R' - 'A' + 0x0a;
			break;
		case 520:
			out->dt1 = 'B' - 'A' + 0x0a;
			out->dt2 = 'A' - 'A' + 0x0a;
			out->dt3 = 'L' - 'A' + 0x0a;
			break;
		case 999:
//			out->dt4 = endofLCD - 1;
			break;
		default:
			break;
		}
	}
}
