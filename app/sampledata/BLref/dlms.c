/****************************************************************************/
/*                                                                          */
/*  システム名      :   DLMS                                                */
/*                                                                          */
/*  サブシステム名  :   DLMS                                                */
/*                                                                          */
/*  内容            :   DLMS共通処理                                        */
/*                                                                          */
/*  ファイル名      :   dlms.c                                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include "def.h"				/* define宣言					*/
#include "data.h"				/* define宣言					*/
#include "ram.h"				/* define宣言					*/
#include "sub.h"				/* define宣言					*/
#include "dlms.h"
#include "conf.h"
#include "hrdtushin.h"
#include "aplrecord.h"			/* define宣言					*/
#include "apleeptask.h"			/* define宣言					*/
#include "acse.h"
#include "aes_gcm.h"
#include "xdlms.h"

#pragma section COMM
CONST ui08 VERSION[] = { 'V','1','.','0','0','A',' ','3','1','0','3','2','0','2','6' };
CONST ui08 FLAGS[] = { 'F','E','I' };
CONST ui08 push_client[] = { 'C','L', 'I', 'E', 'N', 'T', '0', '0' };
CONST USER USERS[] = {
	{{{ 0x53, 0x75, 0x70, 0x65, 0x72, 0x20, 0x20, 0x20 },	0x30, 0x7f },	/*Super */
	{{
		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,
		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,
		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,
		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff
	},
	{0xfffffffe,0xfffffffe,0xfffffffe,0xfffffffe,0xfffffffe,0xfffffffe}}},
	{{{ 0x43, 0x4C, 0x49, 0x45, 0x4e, 0x54, 0x30, 0x30 },	0x30, 0x61 },	/*Admin*/
	{{
		0x00,		0xff,		0xff,		0xff,		0x9f,		0x9f,		0x9f,		0xff,
		0xff,		0xff,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
		0x9f,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,
		0xff,		0xff,		0xff,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}},
	{{{ 0x43, 0x4C, 0x49, 0x45, 0x4e, 0x54, 0x30, 0x30 },	0x30, 0xa1 },	/*FW*/
	{{
		0x00,		0xff,		0xff,		0xff,		0x00,		0x00,		0x00,		0x00,
		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,
		0x00,		0x00,		0x00,		0xff,		0x00,		0x00,		0x00,		0x00,
		0xff,		0xff,		0xff,		0x00,		0x00,		0x00,		0x00,		0x00
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}},
	{{{ 0x43, 0x4C, 0x49, 0x45, 0x4e, 0x54, 0x30, 0x30 },	0x20, 0x41 },	/*MR*/
	{{
		0x00,		0x9f,		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
//		0x9f,		0x9f,		0x9f,		0x9f,		0x00,		0x9f,		0x00,		0x00,
		0x9f,		0x9f,		0x9f,		0x9f,		0x00,		0xbf,		0x00,		0x00,
		0x00,		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}},
	{{{ 0x43, 0x4C, 0x49, 0x45, 0x4e, 0x54, 0x30, 0x30 },	0x20, 0x81 },	/*Push*/
	{{
		0x00,		0x9f,		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
		0x9f,		0x9f,		0xbf,		0x9f,		0x00,		0x00,		0x00,		0x9f,
		0x00,		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}},
	{{{ 0x43, 0x4C, 0x49, 0x45, 0x4e, 0x54, 0x30, 0x30 },	0x20, 0xc1 },	/*IHD*/
	{{
		0x00,		0x9f,		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,
		0x9f,		0x9f,		0xbf,		0x9f,		0x00,		0x00,		0x00,		0x00,
		0x00,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f,		0x9f
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}},
	{{{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },	0xff, 0x00 },	/*Anony*/
	{{
		0x00,		0x00,		0x00,		0x00,		0x9f,		0x00,		0x00,		0x00,
		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,
		0x00,		0x9f,		0x00,		0x9f,		0x9f,		0x00,		0x00,		0x00,
		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}},
	{{{ 0x43, 0x4C, 0x49, 0x45, 0x4e, 0x54, 0x30, 0x30 },	0x00, 0x21},
	{{
		0x00,		0x00,		0x00,		0x9f,		0x9f,		0x00,		0x00,		0x00,
		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,
		0x00,		0x9f,		0x00,		0x9f,		0x9f,		0x00,		0x00,		0x00,
		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00,		0x00
	},
	{0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xfffffffe}}}
};


UNIT_SCAL CONST unit_scale[] = {
	{ 0, e_invalid, 0},

{ 0, e_count, 0}, /* 1: Cumulative Billing count */
{ 0, e_minute, 0}, /* 2: Available Billing Period  */
{ 0, e_time6, 0}, /* 3: Time */
{ 0, e_date6, 0}, /* 4: Date */
{ 0, e_count, 0}, /* 5: NIC SIM ID */
{ 0, e_count, 0}, /* 6: NIC IMEI number (serial number) */
{ 0, e_count, 0}, /* 7: COSEM logical device name  */
{ 0, e_count, 0}, /* 8: Frame counter */
{ 0, e_count, 0}, /* 9: Frame counter1 */
{ 0, e_count, 0}, /* 10: Frame counter2 */
{ 0, e_count, 0}, /* 11: Frame counter3 */
{ 0, e_count, 0}, /* 12: Frame counter4 */
{ 0, e_count, 0}, /* 13: Frame counter5 */
{ 0, e_count, 0}, /* 14: Frame counter6 */
{ 0, e_count, 0}, /* 15: Cumulative tamper count */
{ 0, e_count, 0}, /* 16: Meter Type (1Phase/3P-3W/3P-4W) */
{ 0, e_count, 0}, /* 17: Category */
{ 0, e_count, 0}, /* 18: Current rating */
{ 0, e_count, 0}, /* 19: Event status word */
{ 0, e_count, 0}, /* 20: Event status word filter */
{ 0, e_count, 0}, /* 21: Metering mode */
{ 0, e_count, 0}, /* 22: Payment mode */
{ 0, e_rupy, 0}, /* 23: Last token recharge amount */
{ 0, e_count, 0}, /* 24: Last token recharge time  */
{ 0, e_rupy, 0}, /* 25: Total amount at last recharge  */
{ 0, e_rupy, 0}, /* 26: Current balance amount */
{ 0, e_count, 0}, /* 27: Current balance time */
{ 0, e_count, 0}, /* 28: Meter serial number */
{ 0, e_count, 0}, /* 29: Manufacturer name */
{ 0, e_count, 0}, /* 30: Device ID */
{ 0, e_count, 0}, /* 31: Meter year of manufacture */
{ 0, e_count, 0}, /* 32: NIC serial ID */
{ 0, e_count, 0}, /* 33: Bluetooth mac address */
{ 0, e_count, 0}, /* 34: NIC Firmware Version */
{ 0, e_count, 0}, /* 35: Cumulative program count */
{ 0, e_count, 0}, /* 36: Number of power failures */
{ 0, e_count, 0}, /* 37: NIC status  */
{ 0, e_count, 0}, /* 38: Event1 */
{ 0, e_count, 0}, /* 39: Event2 */
{ 0, e_count, 0}, /* 40: Event3 */
{ 0, e_count, 0}, /* 41: Event4 */
{ 0, e_count, 0}, /* 42: Event5 */
{ 0, e_count, 0}, /* 43: Event6 */
{ 0, e_count, 0}, /* 44: Event7 */
{ 0, e_count, 0}, /* 45: Meter log code */
{ 0, e_count, 0}, /* 46: Meter status */
{ 0, e_count, 0}, /* 47: Event1-recordNO */
{ 0, e_count, 0}, /* 48: Event2-recordNO */
{ 0, e_count, 0}, /* 49: Event3-recordNO */
{ 0, e_count, 0}, /* 50: Event4-recordNO */
{ 0, e_count, 0}, /* 51: Event5-recordNO */
{ 0, e_count, 0}, /* 52: Event6-recordNO */
{ 0, e_count, 0}, /* 53: Event7-recordNO */
{ 0, e_count, 0}, /* 54: Relay Count */
{ 0, e_count, 0}, /* 55: Cell ID */
{ 0, e_count, 0}, /* 56: 4G&NB fallback to GSM parameter */
{ 0, e_count, 0}, /* 57: RAT & Band */
{ 0, e_count, 0}, /* 58: Event stamping parameter for RC/DC */
{ 0, e_count, 0}, /* 59: RAM */
{ 0, e_count, 0}, /* 60: EEPROM */
{ 0, e_count, 0}, /* 61: ROM */
{ 0, e_count, 0}, /* 62: Energy calibration */
{ 0, e_count, 0}, /* 63: Read energy(BF)　 */
{ 0, e_count, 0}, /* 64: Voltage and ampere calibration */
{ 0, e_count, 0}, /* 65: Read volt and ampere(D3)　 */
{ 0, e_count, 0}, /* 66: RTC calibration */
{ 0, e_count, 0}, /* 67: Lock count */
{ 0, e_count, 0}, /* 68: Lock time */
{ 0, e_count, 0}, /* 69: Setting bit array */
{ 0, e_count, 0}, /* 70: Unit setting */
{ 0, e_count, 0}, /* 71: Record setting */
{ 0, e_count, 0}, /* 72: Power quality code define */
{ 0, e_count, 0}, /* 73: Pulse setting */
{ 0, e_count, 0}, /* 74: Detect event setting */
{ 0, e_count, 0}, /* 75: Server port */
{ 0, e_count, 0}, /* 76: Cumulative event count */
{ 0, e_count, 0}, /* 77: Cumulative tamper count 1month */
{ 0, e_count, 0}, /* 78: Cumulative top cover count */
{ 0, e_count, 0}, /* 79: Top cover tamper datetime */
{ 0, e_count, 0}, /* 80: Cumulative reset count */
{ 0, e_count, 0}, /* 81: Reset datetime */
{ 0, e_count, 0}, /* 82: NIC RSSI */
{ 0, e_count, 0}, /* 83: RSRP */
{ 0, e_count, 0}, /* 84: RSRQ */
{ 0, e_count, 0}, /* 85: SNR */
{ 0, e_count, 0}, /* 86: NIC firmware version number  */
{ 0, e_count, 0}, /* 87: Apparent energy computation type */
{ 0, e_count, 0}, /* 88: LRCF */
{ 0, e_count, 0}, /* 89: Meter type */
{ 0, e_count, 0}, /* 90: Product model */
{ 0, e_count, 0}, /* 91: Firmware version for meter */
{ 0, e_count, 0}, /* 92: Active firmware version  */
{ 0, e_count, 0}, /* 93: Active firmware signature  */
{ 0, e_second, 0}, /* 94: Demand integration period */
{ 0, e_second, 0}, /* 95: Capture period-Block load profile */
{ 0, e_second, 0}, /* 96: Capture period-Daily load profile */
{ 0, e_count, 0}, /* 97: Apparent energy computation type */
{ 0, e_count, 0}, /* 98: Self diagnostic details */
{ 0, e_minute, 0}, /* 99: Current active relay time */
{ 0, e_minute, 0}, /* 100: Active relay time setting */
{ 0, e_count, 0}, /* 101: Cumulative magnetic count */
{ 0, e_count, 0}, /* 102: Magnet tamper datetime */
{ 0, e_count, 0}, /* 103: Cumulative esd count */
{ 0, e_count, 0}, /* 104: ESD tamper datetime */
{ 0, e_count, 0}, /* 105: Cumulative jammer count */
{ 0, e_count, 0}, /* 106: Jammer tamper datetime */
{ 0, e_count, 0}, /* 107: Cumulative vthd count */
{ 0, e_count, 0}, /* 108: vthd tamper datetime */
{ 0, e_count, 0}, /* 109: Cumulative athd count */
{ 0, e_count, 0}, /* 110: athd tamper datetime */
{ 0, e_count, 0}, /* 111: Cumulative atdd count */
{ 0, e_count, 0}, /* 112: atdd tamper datetime */
{ 0, e_count, 0}, /* 113: Relay datetime */
{ 0, e_count, 0}, /* 114: Cumulative disconnect count */
{ 0, e_count, 0}, /* 115: Disconnect datetime */
{ 0, e_count, 0}, /* 116: Cumulative connect count */
{ 0, e_count, 0}, /* 117: Connect datetime */
{ 0, e_count, 0}, /* 118: Billing date */
{ 0, e_minute, 0}, /* 119: Cumulative power off duration in min */
{ 0, e_minute, 0}, /* 120: Billing power on duration in min */
{ 0, e_minute, 0}, /* 121: Cumulative power on duration in min */
{ 0, e_count, 0}, /* 122: Battery voltage */
{ 0, e_second, 0}, /* 123: Operating time objects  */
{ 1, e_degree_celsius, -2}, /* 124: Temperature */
{ 1, e_degree_celsius, -2}, /* 125: Temperature block */
{ 1, e_degree_celsius, -2}, /* 126: Temperature block24 */
{ 0, e_count, 0}, /* 127: Magnet voltage */
{ 1, e_degree_celsius, -2}, /* 128: Temperature(CPU/MAG) block */
{ 1, e_degree_celsius, -2}, /* 129: Temperature(CPU/MAG) block24 */
{ 0, e_watt, 0}, /* 130: Cumulative kW MD Import  */
{ 0, e_watt, 0}, /* 131: Active power+- */
{ 1, e_percent, -2}, /* 132: THD Power Phase  */
{ 0, e_watt_hour, 0}, /* 133: Cumulative Energy+ Wh */
{ 0, e_watt_hour, 0}, /* 134: Cumulative Energy+ Wh TZ1 */
{ 0, e_watt_hour, 0}, /* 135: Cumulative Energy+ Wh TZ2 */
{ 0, e_watt_hour, 0}, /* 136: Cumulative Energy+ Wh TZ3 */
{ 0, e_watt_hour, 0}, /* 137: Cumulative Energy+ Wh TZ4 */
{ 0, e_watt_hour, 0}, /* 138: Cumulative Energy+ Wh TZ5 */
{ 0, e_watt_hour, 0}, /* 139: Cumulative Energy+ Wh TZ6 */
{ 0, e_watt_hour, 0}, /* 140: Cumulative Energy+ Wh TZ7 */
{ 0, e_watt_hour, 0}, /* 141: Cumulative Energy+ Wh TZ8 */
{ 0, e_watt, 0}, /* 142: Block Energy+ W */
{ 0, e_watt, 0}, /* 143: Block Energy+ W TZ1 */
{ 0, e_watt, 0}, /* 144: Block Energy+ W TZ2 */
{ 0, e_watt, 0}, /* 145: Block Energy+ W TZ3 */
{ 0, e_watt, 0}, /* 146: Block Energy+ W TZ4 */
{ 0, e_watt, 0}, /* 147: Block Energy+ W TZ5 */
{ 0, e_watt, 0}, /* 148: Block Energy+ W TZ6 */
{ 0, e_watt, 0}, /* 149: Block Energy+ W TZ7 */
{ 0, e_watt, 0}, /* 150: Block Energy+ W TZ8 */
{ 0, e_watt_hour, 0}, /* 151: Block Energy+ Wh */
{ 0, e_watt_hour, 0}, /* 152: Block Energy+ Wh TZ1 */
{ 0, e_watt_hour, 0}, /* 153: Block Energy+ Wh TZ2 */
{ 0, e_watt_hour, 0}, /* 154: Block Energy+ Wh TZ3 */
{ 0, e_watt_hour, 0}, /* 155: Block Energy+ Wh TZ4 */
{ 0, e_watt_hour, 0}, /* 156: Block Energy+ Wh TZ5 */
{ 0, e_watt_hour, 0}, /* 157: Block Energy+ Wh TZ6 */
{ 0, e_watt_hour, 0}, /* 158: Block Energy+ Wh TZ7 */
{ 0, e_watt_hour, 0}, /* 159: Block Energy+ Wh TZ8 */
{ 0, e_watt_hour, 0}, /* 160: Cumulative kW MD Export */
{ 0, e_watt_hour, 0}, /* 161: Cumulative Energy- Wh */
{ 0, e_watt_hour, 0}, /* 162: Cumulative Energy- Wh TZ1 */
{ 0, e_watt_hour, 0}, /* 163: Cumulative Energy- Wh TZ2 */
{ 0, e_watt_hour, 0}, /* 164: Cumulative Energy- Wh TZ3 */
{ 0, e_watt_hour, 0}, /* 165: Cumulative Energy- Wh TZ4 */
{ 0, e_watt_hour, 0}, /* 166: Cumulative Energy- Wh TZ5 */
{ 0, e_watt_hour, 0}, /* 167: Cumulative Energy- Wh TZ6 */
{ 0, e_watt_hour, 0}, /* 168: Cumulative Energy- Wh TZ7 */
{ 0, e_watt_hour, 0}, /* 169: Cumulative Energy- Wh TZ8 */
{ 0, e_watt, 0}, /* 170: Block Energy- W */
{ 0, e_watt, 0}, /* 171: Block Energy- W TZ1 */
{ 0, e_watt, 0}, /* 172: Block Energy- W TZ2 */
{ 0, e_watt, 0}, /* 173: Block Energy- W TZ3 */
{ 0, e_watt, 0}, /* 174: Block Energy- W TZ4 */
{ 0, e_watt, 0}, /* 175: Block Energy- W TZ5 */
{ 0, e_watt, 0}, /* 176: Block Energy- W TZ6 */
{ 0, e_watt, 0}, /* 177: Block Energy- W TZ7 */
{ 0, e_watt, 0}, /* 178: Block Energy- W TZ8 */
{ 0, e_watt_hour, 0}, /* 179: Block Energy- Wh */
{ 0, e_watt_hour, 0}, /* 180: Block Energy- Wh TZ1 */
{ 0, e_watt_hour, 0}, /* 181: Block Energy- Wh TZ2 */
{ 0, e_watt_hour, 0}, /* 182: Block Energy- Wh TZ3 */
{ 0, e_watt_hour, 0}, /* 183: Block Energy- Wh TZ4 */
{ 0, e_watt_hour, 0}, /* 184: Block Energy- Wh TZ5 */
{ 0, e_watt_hour, 0}, /* 185: Block Energy- Wh TZ6 */
{ 0, e_watt_hour, 0}, /* 186: Block Energy- Wh TZ7 */
{ 0, e_watt_hour, 0}, /* 187: Block Energy- Wh TZ8 */
{ 0, e_var, 0}, /* 188: Reactive power+- */
{ 0, e_var_hour, 0}, /* 189: Cumulative Energy+ varh */
{ 0, e_var_hour, 0}, /* 190: Cumulative Energy- varh */
{ 0, e_var_hour, 0}, /* 191: Cumulative Energy kvarh (Q1) */
{ 0, e_var_hour, 0}, /* 192: Block Energy kvarh (Q1) */
{ 0, e_var_hour, 0}, /* 193: Cumulative Energy kvarh (Q2) */
{ 0, e_var_hour, 0}, /* 194: Block Energy kvarh (Q2) */
{ 0, e_var_hour, 0}, /* 195: Cumulative Energy kvarh (Q3) */
{ 0, e_var_hour, 0}, /* 196: Block Energy kvarh (Q3) */
{ 0, e_var_hour, 0}, /* 197: Cumulative Energy kvarh (Q4) */
{ 0, e_var_hour, 0}, /* 198: Block Energy kvarh (Q4) */
{ 0, e_volt_ampere, 0}, /* 199: Cumulative kVA MD Import  */
{ 0, e_volt_ampere, 0}, /* 200: Apparent power kVA+- */
{ 0, e_volt_ampere_hour, 0}, /* 201: Cumulative Energy+ kVAh */
{ 0, e_volt_ampere_hour, 0}, /* 202: Cumulative Energy+ kVAh TZ1 */
{ 0, e_volt_ampere_hour, 0}, /* 203: Cumulative Energy+ kVAh TZ2 */
{ 0, e_volt_ampere_hour, 0}, /* 204: Cumulative Energy+ kVAh TZ3 */
{ 0, e_volt_ampere_hour, 0}, /* 205: Cumulative Energy+ kVAh TZ4 */
{ 0, e_volt_ampere_hour, 0}, /* 206: Cumulative Energy+ kVAh TZ5 */
{ 0, e_volt_ampere_hour, 0}, /* 207: Cumulative Energy+ kVAh TZ6 */
{ 0, e_volt_ampere_hour, 0}, /* 208: Cumulative Energy+ kVAh TZ7 */
{ 0, e_volt_ampere_hour, 0}, /* 209: Cumulative Energy+ kVAh TZ8 */
{ 0, e_volt_ampere, 0}, /* 210: Block Energy+ VA */
{ 0, e_volt_ampere, 0}, /* 211: Block Energy+ VA TZ1 */
{ 0, e_volt_ampere, 0}, /* 212: Block Energy+ VA TZ2 */
{ 0, e_volt_ampere, 0}, /* 213: Block Energy+ VA TZ3 */
{ 0, e_volt_ampere, 0}, /* 214: Block Energy+ VA TZ4 */
{ 0, e_volt_ampere, 0}, /* 215: Block Energy+ VA TZ5 */
{ 0, e_volt_ampere, 0}, /* 216: Block Energy+ VA TZ6 */
{ 0, e_volt_ampere, 0}, /* 217: Block Energy+ VA TZ7 */
{ 0, e_volt_ampere, 0}, /* 218: Block Energy+ VA TZ8 */
{ 0, e_volt_ampere_hour, 0}, /* 219: Block Energy+ VAh */
{ 0, e_volt_ampere_hour, 0}, /* 220: Block Energy+ VAh TZ1 */
{ 0, e_volt_ampere_hour, 0}, /* 221: Block Energy+ VAh TZ2 */
{ 0, e_volt_ampere_hour, 0}, /* 222: Block Energy+ VAh TZ3 */
{ 0, e_volt_ampere_hour, 0}, /* 223: Block Energy+ VAh TZ4 */
{ 0, e_volt_ampere_hour, 0}, /* 224: Block Energy+ VAh TZ5 */
{ 0, e_volt_ampere_hour, 0}, /* 225: Block Energy+ VAh TZ6 */
{ 0, e_volt_ampere_hour, 0}, /* 226: Block Energy+ VAh TZ7 */
{ 0, e_volt_ampere_hour, 0}, /* 227: Block Energy+ VAh TZ8 */
{ 0, e_volt_ampere_hour, 0}, /* 228: Cumulative kVA MD Export */
{ 0, e_volt_ampere_hour, 0}, /* 229: Cumulative Energy- kVAh */
{ 0, e_volt_ampere_hour, 0}, /* 230: Cumulative Energy- kVAh TZ1 */
{ 0, e_volt_ampere_hour, 0}, /* 231: Cumulative Energy- kVAh TZ2 */
{ 0, e_volt_ampere_hour, 0}, /* 232: Cumulative Energy- kVAh TZ3 */
{ 0, e_volt_ampere_hour, 0}, /* 233: Cumulative Energy- kVAh TZ4 */
{ 0, e_volt_ampere_hour, 0}, /* 234: Cumulative Energy- kVAh TZ5 */
{ 0, e_volt_ampere_hour, 0}, /* 235: Cumulative Energy- kVAh TZ6 */
{ 0, e_volt_ampere_hour, 0}, /* 236: Cumulative Energy- kVAh TZ7 */
{ 0, e_volt_ampere_hour, 0}, /* 237: Cumulative Energy- kVAh TZ8 */
{ 0, e_volt_ampere, 0}, /* 238: Block Energy- VA */
{ 0, e_volt_ampere, 0}, /* 239: Block Energy- VA TZ1 */
{ 0, e_volt_ampere, 0}, /* 240: Block Energy- VA TZ2 */
{ 0, e_volt_ampere, 0}, /* 241: Block Energy- VA TZ3 */
{ 0, e_volt_ampere, 0}, /* 242: Block Energy- VA TZ4 */
{ 0, e_volt_ampere, 0}, /* 243: Block Energy- VA TZ5 */
{ 0, e_volt_ampere, 0}, /* 244: Block Energy- VA TZ6 */
{ 0, e_volt_ampere, 0}, /* 245: Block Energy- VA TZ7 */
{ 0, e_volt_ampere, 0}, /* 246: Block Energy- VA TZ8 */
{ 0, e_volt_ampere_hour, 0}, /* 247: Block Energy- VAh */
{ 0, e_volt_ampere_hour, 0}, /* 248: Block Energy- VAh TZ1 */
{ 0, e_volt_ampere_hour, 0}, /* 249: Block Energy- VAh TZ2 */
{ 0, e_volt_ampere_hour, 0}, /* 250: Block Energy- VAh TZ3 */
{ 0, e_volt_ampere_hour, 0}, /* 251: Block Energy- VAh TZ4 */
{ 0, e_volt_ampere_hour, 0}, /* 252: Block Energy- VAh TZ5 */
{ 0, e_volt_ampere_hour, 0}, /* 253: Block Energy- VAh TZ6 */
{ 0, e_volt_ampere_hour, 0}, /* 254: Block Energy- VAh TZ7 */
{ 0, e_volt_ampere_hour, 0}, /* 255: Block Energy- VAh TZ8 */
{ 1, e_ampere, -2}, /* 256: Phase current */
{ 1, e_percent, -2}, /* 257: THD Current  */
{ 1, e_ampere, -2}, /* 258: Block phase current */
{ 1, e_volt, -2}, /* 259: Voltage0 Ave 1min value  */
{ 1, e_volt, -2}, /* 260: Voltage */
{ 1, e_percent, -2}, /* 261: THD Voltage  */
{ 1, e_volt, -2}, /* 262: Block voltage */
{ 1, e_percent, -2}, /* 263: Block voltage THD */
{ 1, e_count, -2}, /* 264: Power factor imp month */
{ 1, e_count, -2}, /* 265: System Imp PF T1  */
{ 1, e_count, -2}, /* 266: System Imp PF T2  */
{ 1, e_count, -2}, /* 267: System Imp PF T3  */
{ 1, e_count, -2}, /* 268: System Imp PF T4  */
{ 1, e_count, -2}, /* 269: System Imp PF T5  */
{ 1, e_count, -2}, /* 270: System Imp PF T6  */
{ 1, e_count, -2}, /* 271: System Imp PF T7  */
{ 1, e_count, -2}, /* 272: System Imp PF T8  */
{ 1, e_count, -2}, /* 273: Signed power factor/Three phase power factor */
{ 1, e_count, -2}, /* 274: Power factor imp block  */
{ 1, e_count, -2}, /* 275: Power factor imp day */
{ 1, e_hertz, -2}, /* 276: Frequency */
{ 0, e_watt_hour, 0}, /* 277: ABS Wh */
{ 0, e_watt_hour, 0}, /* 278: NET Wh */
{ 0, e_var_hour, 0}, /* 279: ABS VarH */
{ 0, e_var_hour, 0}, /* 280: NET VarH */
{ 1, e_ampere, -2}, /* 281: Current L1 */
{ 1, e_ampere, -2}, /* 282: IR-block */
{ 1, e_volt, -2}, /* 283: Voltage L1 Ave 1min value  */
{ 1, e_volt, -2}, /* 284: Voltage L1 */
{ 1, e_volt, -2}, /* 285: Voltage L1 Block */
{ 1, e_count, -2}, /* 286: Signed power factorR */
{ 1, e_ampere, -2}, /* 287: Current L2 */
{ 1, e_ampere, -2}, /* 288: IY-block */
{ 1, e_volt, -2}, /* 289: Voltage L2 Ave 1min value  */
{ 1, e_volt, -2}, /* 290: Voltage L2 */
{ 1, e_volt, -2}, /* 291: Voltage L2 Block */
{ 1, e_count, -2}, /* 292: Signed power factorY */
{ 1, e_ampere, -2}, /* 293: Current L3 */
{ 1, e_ampere, -2}, /* 294: IB-block */
{ 1, e_volt, -2}, /* 295: Voltage L3 Ave 1min value  */
{ 1, e_volt, -2}, /* 296: Voltage L3 */
{ 1, e_volt, -2}, /* 297: Voltage L3 Block */
{ 1, e_count, -2}, /* 298: Signed power factorB */
{ 1, e_count, -2}, /* 299: Power factor exp month */
{ 1, e_count, -2}, /* 300: System Exp PF T1  */
{ 1, e_count, -2}, /* 301: System Exp PF T2  */
{ 1, e_count, -2}, /* 302: System Exp PF T3  */
{ 1, e_count, -2}, /* 303: System Exp PF T4  */
{ 1, e_count, -2}, /* 304: System Exp PF T5  */
{ 1, e_count, -2}, /* 305: System Exp PF T6  */
{ 1, e_count, -2}, /* 306: System Exp PF T7  */
{ 1, e_count, -2}, /* 307: System Exp PF T8  */
{ 1, e_count, -2}, /* 308: Power factor exp block  */
{ 1, e_count, -2}, /* 309: Power factor exp day */
{ 1, e_ampere, -2}, /* 310: Neutral current */
{ 1, e_ampere, -2}, /* 311: Block neutral current */
{ 1, e_ampere, -2}, /* 312: Average24 neutral current */
{ 1, e_ampere, -2}, /* 313: Current */
{ 0, e_watt_hour, 0}, /* 314: Defraud Kwh */
{ 1, e_degree_celsius, -2}, /* 315: Temperature(CPU/MAG) */
{ 1, e_ampere, -2}, /* 316: Current-Avearage? */
{ 0, e_watt_hour, 0}, /* 317: Fundamental Energy kWh */
{ 1, e_percent, -2}, /* 318: Block current THD */
{ 1, e_percent, -2}, /* 319: Average24 current THD */
{ 1, e_percent, -2}, /* 320: Current TDD */
{ 1, e_percent, -2}, /* 321: Block current TDD */
{ 1, e_percent, -2}, /* 322: Average24 current TDD */
{ 0, e_watt_hour, 0}, /* 323: Cumulative Energy+ Wh(High) */
{ 0, e_watt_hour, 0}, /* 324: Cumulative Energy- Wh(High) */
{ 0, e_var_hour, 0}, /* 325: Cumulative Energy+ varh(High) */
{ 0, e_var_hour, 0}, /* 326: Cumulative Energy- varh(High) */
{ 0, e_volt_ampere_hour, 0}, /* 327: Cumulative Energy+ kVAh(High) */
{ 0, e_volt_ampere_hour, 0}, /* 328: Cumulative Energy- kVAh(High) */
{ 1, e_hertz, -2}, /* 329: Frequency L1  */
{ 1, e_hertz, -2}, /* 330: Frequency L2 */
{ 1, e_hertz, -2}, /* 331: Frequency L3 */
{ 0, e_watt, 0}, /* 332: Block24 Energy+ W */
{ 0, e_watt_hour, 0}, /* 333: Block24 Energy+ Wh */
{ 0, e_watt, 0}, /* 334: Block24 Energy- W */
{ 0, e_watt_hour, 0}, /* 335: Block24 Energy- Wh */
{ 0, e_var_hour, 0}, /* 336: Block24 Energy+ kvarh */
{ 0, e_var_hour, 0}, /* 337: Block24 Energy- kvarh */
{ 0, e_volt_ampere, 0}, /* 338: Block24 Energy+ VA */
{ 0, e_volt_ampere_hour, 0}, /* 339: Block24 Energy+ VAh */
{ 0, e_volt_ampere, 0}, /* 340: Block24 Energy- VA */
{ 0, e_volt_ampere_hour, 0}, /* 341: Block24 Energy- VAh */
{ 1, e_ampere, -2}, /* 342: Average24 phase current */
{ 1, e_volt, -2}, /* 343: Average24 voltage */
{ 1, e_percent, -2}, /* 344: Average24 voltage THD */
{ 0, e_volt_ampere_hour, 0}, /* 345: Apparent energy net total */
{ 0, e_count, 0}, /* 346: Average signal strength(RSSI) */
{ 0, e_watt, 0}, /* 347: Max Demand+ kW */
{ 0, e_watt, 0}, /* 348: Max Demand+ kW TZ1 */
{ 0, e_watt, 0}, /* 349: Max Demand+ kW TZ2 */
{ 0, e_watt, 0}, /* 350: Max Demand+ kW TZ3 */
{ 0, e_watt, 0}, /* 351: Max Demand+ kW TZ4 */
{ 0, e_watt, 0}, /* 352: Max Demand+ kW TZ5 */
{ 0, e_watt, 0}, /* 353: Max Demand+ kW TZ6 */
{ 0, e_watt, 0}, /* 354: Max Demand+ kW TZ7 */
{ 0, e_watt, 0}, /* 355: Max Demand+ kW TZ8 */
{ 0, e_watt, 0}, /* 356: Max Demand- kW */
{ 0, e_watt, 0}, /* 357: Max Demand- kW TZ1 */
{ 0, e_watt, 0}, /* 358: Max Demand- kW TZ2 */
{ 0, e_watt, 0}, /* 359: Max Demand- kW TZ3 */
{ 0, e_watt, 0}, /* 360: Max Demand- kW TZ4 */
{ 0, e_watt, 0}, /* 361: Max Demand- kW TZ5 */
{ 0, e_watt, 0}, /* 362: Max Demand- kW TZ6 */
{ 0, e_watt, 0}, /* 363: Max Demand- kW TZ7 */
{ 0, e_watt, 0}, /* 364: Max Demand- kW TZ8 */
{ 0, e_var, 0}, /* 365: Max.Demand+ var */
{ 0, e_var, 0}, /* 366: Max.Demand- var */
{ 0, e_volt_ampere, 0}, /* 367: Max Demand+ kVA */
{ 0, e_volt_ampere, 0}, /* 368: Max Demand+ kVA TZ1 */
{ 0, e_volt_ampere, 0}, /* 369: Max Demand+ kVA TZ2 */
{ 0, e_volt_ampere, 0}, /* 370: Max Demand+ kVA TZ3 */
{ 0, e_volt_ampere, 0}, /* 371: Max Demand+ kVA TZ4 */
{ 0, e_volt_ampere, 0}, /* 372: Max Demand+ kVA TZ5 */
{ 0, e_volt_ampere, 0}, /* 373: Max Demand+ kVA TZ6 */
{ 0, e_volt_ampere, 0}, /* 374: Max Demand+ kVA TZ7 */
{ 0, e_volt_ampere, 0}, /* 375: Max Demand+ kVA TZ8 */
{ 0, e_volt_ampere, 0}, /* 376: Max Demand- kVA */
{ 0, e_volt_ampere, 0}, /* 377: Max Demand- kVA TZ1 */
{ 0, e_volt_ampere, 0}, /* 378: Max Demand- kVA TZ2 */
{ 0, e_volt_ampere, 0}, /* 379: Max Demand- kVA TZ3 */
{ 0, e_volt_ampere, 0}, /* 380: Max Demand- kVA TZ4 */
{ 0, e_volt_ampere, 0}, /* 381: Max Demand- kVA TZ5 */
{ 0, e_volt_ampere, 0}, /* 382: Max Demand- kVA TZ6 */
{ 0, e_volt_ampere, 0}, /* 383: Max Demand- kVA TZ7 */
{ 0, e_volt_ampere, 0}, /* 384: Max Demand- kVA TZ8 */
{ 1, e_volt, -2}, /* 385: Minimum Voltage0 */
{ 1, e_volt, -2}, /* 386: Minimum Voltage of L1  */
{ 1, e_volt, -2}, /* 387: Minimum Voltage of L2  */
{ 1, e_volt, -2}, /* 388: Minimum Voltage of L3  */
{ 0, e_watt, 0}, /* 389: Max24 Demand+ kW */
{ 0, e_watt, 0}, /* 390: Max24 Demand- kW */
{ 0, e_var, 0}, /* 391: Max24 Demand+ var */
{ 0, e_var, 0}, /* 392: Max24 Demand- var */
{ 0, e_volt_ampere, 0}, /* 393: Max24 Demand+ kVA */
{ 0, e_volt_ampere, 0}, /* 394: Max24 Demand- kVA */
{ 0, e_count, 0}, /* 395: Name plate detail */
{ 0, e_count, 0}, /* 396: Event1 profile */
{ 0, e_count, 0}, /* 397: Event2 profile */
{ 0, e_count, 0}, /* 398: Event3 profile */
{ 0, e_count, 0}, /* 399: Event4 profile */
{ 0, e_count, 0}, /* 400: Event5 profile */
{ 0, e_count, 0}, /* 401: Event6 profile */
{ 0, e_count, 0}, /* 402: Event7 profile */
{ 0, e_count, 0}, /* 403: Relay log */
{ 0, e_count, 0}, /* 404: Meter log */
{ 0, e_count, 0}, /* 405: Snapshot of instantaneous parameters A1,A14 */
{ 0, e_count, 0}, /* 406: Scaler profile-A1,A14 */
{ 0, e_count, 0}, /* 407: Scaler profile-A2,A15 */
{ 0, e_count, 0}, /* 408: Scaler profile-A3,A16 */
{ 0, e_count, 0}, /* 409: Scaler profile-A4,A17 */
{ 0, e_count, 0}, /* 410: Scaler profile-A5-A11 */
{ 0, e_count, 0}, /* 411: Billing profile parameters */
{ 0, e_count, 0}, /* 412: Block load profile */
{ 0, e_count, 0}, /* 413: Daily load profile */
{ 0, e_count, 0}, /* 414: Date time */
{ 0, e_count, 0}, /* 415: Date time(RTC) */
{ 0, e_count, 0}, /* 416: MDI Reset/End of Billing Period script table */
{ 0, e_count, 0}, /* 417: Tariffication script table */
{ 0, e_count, 0}, /* 418: Image activation script table */
{ 0, e_count, 0}, /* 419: Push script table */
{ 0, e_count, 0}, /* 420: Special days table */
{ 0, e_count, 0}, /* 421: Current Association */
{ 0, e_count, 0}, /* 422: PC-Association */
{ 0, e_count, 0}, /* 423: MR-Association */
{ 0, e_count, 0}, /* 424: US-Association */
{ 0, e_count, 0}, /* 425: PUSH-Association */
{ 0, e_count, 0}, /* 426: FIRM-Association */
{ 0, e_count, 0}, /* 427: IHD-Association */
{ 0, e_count, 0}, /* 428: Image transfer */
{ 0, e_count, 0}, /* 429: Image transfer1 */
{ 0, e_count, 0}, /* 430: Activity calendar for time zones */
{ 0, e_count, 0}, /* 431: Single action schedule for billing dates */
{ 0, e_count, 0}, /* 432: Image Activation Single action schedule */
{ 0, e_count, 0}, /* 433: Single action schedule for Two Way Communication  */
{ 0, e_count, 0}, /* 434: Single action schedule for Tamper data Push */
{ 0, e_count, 0}, /* 435: Block load profile push schedule change */
{ 0, e_count, 0}, /* 436: Daily load profile push schedule change */
{ 0, e_count, 0}, /* 437: Billing rofile push schedule change */
{ 0, e_count, 0}, /* 438: Trigger SM TO HES-Single action schedule */
{ 0, e_count, 0}, /* 439: Trigger SM TO IHD-Single action schedule */
{ 0, e_count, 0}, /* 440: IEC HDLC setup */
{ 0, e_count, 0}, /* 441: HDLC Setting(Other) */
{ 0, e_count, 0}, /* 442: SM to HES push */
{ 0, e_count, 0}, /* 443: SM TO IHD push */
{ 0, e_count, 0}, /* 444: SM TO HES event push */
{ 0, e_count, 0}, /* 445: SM to HES Load profle push */
{ 0, e_count, 0}, /* 446: SM to HES daily profle push */
{ 0, e_count, 0}, /* 447: Self Registration Push */
{ 0, e_count, 0}, /* 448: SM to HES Billing profle push */
{ 0, e_count, 0}, /* 449: SM to HES Tamper push */
{ 0, e_count, 0}, /* 450: TCP-UDP setup */
{ 0, e_count, 0}, /* 451: ANP setting */
{ 0, e_count, 0}, /* 452: IPV6 setup */
{ 0, e_count, 0}, /* 453: Setup security(For security NON) */
{ 0, e_count, 0}, /* 454: Setup security(For security HLS) */
{ 0, e_count, 0}, /* 455: MR-Security setup */
{ 0, e_count, 0}, /* 456: US-Security setup */
{ 0, e_count, 0}, /* 457: PUSH-Security setup */
{ 0, e_count, 0}, /* 458: FIRM-Security setup */
{ 0, e_count, 0}, /* 459: IHD-Security setup */
{ 0, e_count, 0}, /* 460: Disconnect control */
{ 0, e_count, 0}, /* 461: Load limit value in kW */

{ 0, e_invalid, 0 },
};

#pragma section

IO		g_gcm_req;					/*Security要求電文*/
IO		g_vect_req;					/*Security要求電文*/
IO		g_gcm_res;					/*Security応答電文*/

IO		g_acse_req;					/*ACSE要求電文*/
IO		g_xdlms_req;				/*XLDMS要求電文*/
IO		g_cosem_res;				/*COSEM応答電文*/
uint	g_raw_len;					/*XDLMS応答RAW電文長*/
uint	g_xdlms_res_len;			/*XDLMS応答電文ヘッダ長*/
uint	g_apli_offset;

pui08	g_xdlms_res;				/*XDLMS応答電文ヘッダ*/
pui08	g_xdlms_io;					/* データIO            */
ui08	g_raw[768];						/*XDLMS応答RAW電文*/

ui08	g_user0;
ui08	g_port0;					/*認証済クライアント回路番号*/
ui08	g_AA; 						/*認証済クライアント認証状態 */
ui08	g_userQ;
ui08	g_portQ;					/*通信クライアント回路番号*/
ui08	g_addrQ;					/*通信クライアントtsushinアドレス*/
ui08	g_authQ;					/*通信クライアントメッセージ認証状態*/
ui08    order_cipher;
ui08	security_control;

/****************************************************************/
/*                                                              */
/*  関数名      :ramcmp                                        */
/*                                                              */
/*  内容        :メモリ比較                                     */
/*                                                              */
/*  引　数1     :比較対象1開始アドレス                          */
/*  引　数2     :比較対象2開始アドレス                          */
/*  引　数3     :比較サイズ                                     */
/*                                                              */
/*  戻り値      : 一致(0), 不一致(1)                            */
/*                                                              */
/*  作成日      : 2013-09-03    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    :                                               */
/*      2017-07-31 T.Kuroda 16ビット化                          */
/*                                                              */
/****************************************************************/
ui08 zero_cmp(cpui08 buf1, cuint size)
{
	uint i = size;
	while (i--)							/* 1バイト処理 */
	{
		if (!(*(buf1++) == 0x00))		/* 不一致確認(1バイト) */
		{
			return 1;					/* 不一致 */
		}
	}
	return 0;							/* 一致 */
}
/****************************************************************/
/*                                                              */
/*  関数名      : decode_len                                    */
/*                                                              */
/*  内容        : BERエンコードしたデータから長さを取得する     */
/*                                                              */
/*  引　数1     : BERエンコードしたデータの先頭アドレス         */
/*  引　数2     : 長さを保存している先頭アドレスからの位置      */
/*                                                              */
/*  戻り値      : 長さ                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    :                                               */
/*      2017-07-31 T.Kuroda 16ビット化                          */
/*                                                              */
/****************************************************************/
uint decode_len( puint offset, cpui08 in )
{
	uint	r = 0;
	ui08	i, cnt;

	if( 0 == ( in[ *offset ]&0x80 ) )							/* 長さ長さビットOFF? */
	{
		r = in[ *offset ]&0x7f;									/* 長さ保存 */
	}
	else
	{
		cnt = (in[ *offset ]&0x7f);								/* 長さ長さを保存 */
		if( (0 < cnt) && (MAXW > cnt) )							/* 長さ長さは許容桁? */
		{
			(*offset)++;										/* 処理位置移動 */
			for( i=1; i < cnt ; i++ )			/* 桁上げ、処理位置移動 */
			{
				r += in[ *offset ];								/* 長さ保存 */
				r<<=8;
				(*offset)++;
			}
			r += in[ *offset ];									/* 最終バイトの長さを保存 */
		}
	}
	(*offset)++;												/* 処理位置移動 */

	return r;													/* 長さ */
}

/****************************************************************/
/*                                                              */
/*  関数名      : encode_len                                    */
/*                                                              */
/*  内容        : 長さをBERエンコードする                       */
/*                                                              */
/*  引　数1     : BERエンコードするデータの先頭アドレス         */
/*  引　数2     : 長さを保存する先頭アドレスからの位置          */
/*  引　数3     : 保存する長さ                                  */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    :                                               */
/*      2017-07-31 T.Kuroda 16ビット化                          */
/*                                                              */
/****************************************************************/
void encode_len( pui08 out, puint offset, cuint len )
{
	uint num = len;
	ui08 cnt = 0, i;

	out[ *offset ] = 0;											/* 初期値保存 */
	if( 0x7f >= num )											/* 長さ長さは不要? */
	{
		out[ *offset ] = (ui08)num;							/* 長さを保存 */
	}
	else
	{
		cnt++;													/* 長さ長さは1 */
		if( 0xff < num )										/* 長さは0xffより大きい */
		{
			cnt++;												/* 長さ長さ+1 */
		}
#if (DLMS_BIT==DLMS_32_BIT)										
		if( 0xffff < num )										/* 長さは0xffffより大きい? */
		{
			cnt++;												/* 長さ長さ+1  */
		}
		if( 0xffffff < num )									/* 長さは0xffffffより大きい? */
		{
			cnt++;												/* 長さ長さ+1 */
		}
#endif
		out[ *offset ] = 0x80;									/* 長さ長さビットを1 */
		out[ *offset ] += cnt;									/* 長さ長さを保存 */
		/* □□□■ ⇒ □□■■ ⇒ □■■■⇒ ■■■■ 下位バイトから保存 */
		for( i = cnt ; i > 0 ; i--)
		{
			out[ (*offset) + i ] = num & 0xff;					/* 長さを保存 */
			num >>= 8;
		}
		(*offset)+= cnt;										/* 最終バイトに長さを保存 */
	}
	(*offset)++;												/* 処理位置移動 */
}

void encode_len32(pui08 out, puint offset, cui32 len)
{
	ui32 num = len;
	ui08 cnt = 0, i;

	out[*offset] = 0;											/* 初期値保存 */
	if (0x7f >= num)											/* 長さ長さは不要? */
	{
		out[*offset] = (ui08)num;							/* 長さを保存 */
	}
	else
	{
		cnt++;													/* 長さ長さは1 */
		if (0xff < num)											/* 長さは0xffより大きい */
		{
			cnt++;												/* 長さ長さ+1 */
		}
		if (0xffff < num)										/* 長さは0xffffより大きい? */
		{
			cnt++;												/* 長さ長さ+1  */
		}
		if (0xffffff < num)										/* 長さは0xffffffより大きい? */
		{
			cnt++;												/* 長さ長さ+1 */
		}
		out[*offset] = 0x80;									/* 長さ長さビットを1 */
		out[*offset] += cnt;									/* 長さ長さを保存 */
																/* □□□■ ⇒ □□■■ ⇒ □■■■⇒ ■■■■ 下位バイトから保存 */
		for (i = cnt; i > 0; i--)
		{
			out[(*offset) + i] = num & 0xff;					/* 長さを保存 */
			num >>= 8;
		}
		(*offset) += cnt;										/* 最終バイトに長さを保存 */
	}
	(*offset)++;												/* 処理位置移動 */
}
/****************************************************************/
/*                                                              */
/*  関数名      : decode_tag                                    */
/*                                                              */
/*  内容        : BERエンコードしたデータからタグを取得する     */
/*                                                              */
/*  引　数1     : デコード先BER                                 */
/*  引　数2     : タグを保存している先頭アドレスからの位置      */
/*  引　数3     : BERエンコードしたデータの先頭アドレス         */
/*                                                              */
/*  戻り値      : 成功(0)/失敗(DLMS_INVALID)                    */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 decode_tag( PBER  ber, puint offset, cpui08 in )
{
	ber->c = (eCLASS)((in[*offset]&0xc0)>>6);					/* clssを保存 */
	ber->p = (ePrimConst)((in[*offset]&0x20)>>5);				/* 格納方法を保存 */
	if( !(0x1f == (in[*offset]&0x1f) ) )						/* 桁上げIDではない */
	{
		ber->i = (in[*offset]&0x1f);							/* タグIDを保存 */
	}
	else
	{
		(*offset)++;											/* 処理位置移動 */
		for(ber->i=0 ; 0x80 == (in[*offset]&0x80) ; (*offset)++, ber->i<<=7 )
		{
			ber->i += (in[*offset]&0x7f);						/* タグIDビットを保存 */
			if( ber->i > MAX_DECODE_ID )						/* 最大値を超えた？*/
			{
				/* Error : invalid tag id */
				ber->i = MAX_DECODE_ID;							/* 255を超えるIDは許可しない. */
				return DLMS_INVALID;							/* 異常 */
			}
		}
		ber->i += (in[*offset]&0x7f);							/* 最終バイトのタグIDを保存 */
	}
	(*offset)++;												/* 処理位置移動 */
	return 0;
}

/****************************************************************/
/*                                                              */
/*  関数名      : encode_tag                                    */
/*                                                              */
/*  内容        : BERエンコードする                             */
/*                                                              */
/*  引　数1     : エンコードするBER                             */
/*  引　数2     : BERエンコードするデータの先頭アドレス         */
/*  引　数3     : タグを保存する先頭アドレスからの位置          */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    :                                               */
/*      2017-07-31 T.Kuroda 16ビット化                          */
/*                                                              */
/****************************************************************/
void encode_tag( pui08 out, puint offset, CPBR ber )
{
	uint	tmp;
	ui08	i, j;
	ui08	val[MAXW];

	ramset( val, 0 , sizeof(val) );

	out[*offset] = 0x00;
	switch( ber->c )
	{
		case eCL_application:
			out[*offset] |= 0x40;
			break;
		case eCL_context_specific:
			out[*offset] |= 0x80;
			break;
		case eCL_private:
			out[*offset] |= 0xc0;
			break;
		case eCL_universal:
		default:
			out[*offset] |= 0x00;
			break;
	};
	if ( !(ber->p == ePC_primitive) )
	{
		out[*offset] |= 0x20;
	}
	tmp	= ber->i;
	if( 0x1f > tmp )		/* 11110b */
	{
		out[*offset] |= ( tmp & 0x1f);
	}
	else
	{
		out[*offset] |= ( tmp & 0x1f);
		(*offset)++;
		for( i = 0 ; tmp > 0 && i < ((ui08)sizeof(val)); i++, tmp >>= 7 )
		{
			val[i] = tmp & 0x7f;  /* MSB側の値を代入 */
		}
		for( j = i ; j > 1 ; j--, (*offset)++ )
		{
			out[*offset]  = val[j - 1];
			out[*offset] |= 0x80;
		}
		out[*offset] = val[0];
	}
	(*offset)++;
}

/****************************************************************/
/*                                                              */
/*  関数名      : encode                                        */
/*                                                              */
/*  内容        : BERエンコードする                             */
/*                                                              */
/*  引　数1     : BERエンコードするデータの先頭アドレス         */
/*  引　数2     : エンコードBER                                 */
/*  引　数3     : BERを保存する先頭アドレスからの位置           */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void encode( pui08 out, puint offset, CPBR ber )
{
	uint	i;

	encode_tag( out, offset, ber );
	encode_len( out, offset, ber->l );
	if( !(NULL == ber->v) )
	{
		for( i = 0; i < ber->l ; i++ )
		{
			out[*offset + i ] = ber->v[i];
		}
		*offset += ber->l;
	}
}

/****************************************************************/
/*                                                              */
/*  関数名      : decode                                        */
/*                                                              */
/*  内容        : BERデコードする                               */
/*                                                              */
/*  引　数1     : BERデコードするデータの先頭アドレス           */
/*  引　数2     : デコードBER                                   */
/*  引　数3     : BERを取得する先頭アドレスからの位置           */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void decode( PBER  ber, puint offset, cpui08 in )
{
	ramset( (pui08)ber, 0, sizeof(BER) );

	if( !(0 == decode_tag( ber, offset, in )) )
	{
		return;
	}

	ber->l = decode_len( offset, in );
	ber->v = (pui08)&in[ *offset ];
	(*offset) += ber->l;
}

ui08 Rank(cui08 index) {
	ui08 ret = 0xff;

	switch (USERS[index].sys.maddr) {
	case 0x61:
	case 0xa1:
	case 0x7f:
		ret = COSEM_ADMIN;
		break;
	case 0x41:
		ret = COSEM_POWER;
		break;
	case 0x81:
	case 0xc1:
		ret = COSEM_READER;
		break;
	case 0x21:
		ret = COSEM_PUBLIC;
		break;
	}
	return ret;
}

/****************************************************************/
/*                                                              */
/*  関数名      : init_DLMS                                     */
/*                                                              */
/*  内容        : DLMSの初期化関数                              */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void init_DLMS( void )
{
	SECURITY = AESGCM;
}

/****************************************************************/
/*                                                              */
/*  関数名      : DLMS                                          */
/*                                                              */
/*  内容        : DLMSのメイン関数                              */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void DLMS( void )
{
	ACSE( );
	SECURITY( );
	xDLMS( );
}

ui32	UI32L( cpui08f in )
{
	DAT4 val;

	val.by[0] = in[3];
	val.by[1] = in[2];
	val.by[2] = in[1];
	val.by[3] = in[0];

	return val.wd;
}

ui16	UI16L( cpui08f in )
{
	DAT2 val;

	val.by[0] = in[1];
	val.by[1] = in[0];

	return val.hf;
}

ui32	UI32B(cpui08f in)
{
	DAT4 val;

	val.by[0] = in[0];
	val.by[1] = in[1];
	val.by[2] = in[2];
	val.by[3] = in[3];

	return val.wd;
}

ui16	UI16B(cpui08f in)
{
	DAT2 val;

	val.by[0] = in[0];
	val.by[1] = in[1];

	return val.hf;
}

void ENDCPY4( pui08 dst, cui32 in )
{
	DAT4 dat;

	dat.wd = in;
	dst[0] = dat.by[3];
	dst[1] = dat.by[2];
	dst[2] = dat.by[1];
	dst[3] = dat.by[0];
}

void ENDCPY42( pui08 dst, cpui16f in )
{
	dst[0] = 0;
	dst[1] = 0;
	dst[2] = UI8_PTR( in )[1];
	dst[3] = UI8_PTR( in )[0];
}

void ENDCPY2( pui08 dst, cpui16f in )
{
	dst[0] = UI8_PTR( in )[1];
	dst[1] = UI8_PTR( in )[0];
}
