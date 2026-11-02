#ifndef _XDLMS_H_
#define _XDLMS_H_

/********************/
/*  OBJECT DEFINE   */
/********************/
/*
Average current 1.0.11.27.0.255
Meterring mode	0.0.94.96.19.255
Payment mode	0.0.94.96.20.255
Last token recharge amount 0.0.94.96.21.255
Last token recharge time 0.0.94.96.22.255
Total amount at last recharge 0.0.94.96.23.255
Current balance amouunt 0.0.94.96.24.255
Current balance time 0.0.94.96.25.255
ESWF 0.94.91.26.255
LLS secret 0.0.40.2.255
Hls key 0.0.40.0.e.255
Global key change 0.0.43.0.e.255
image activation single action schedule 0.0.15.0.2.255
MD reset 0.0.10.0.1.255

*/
/***************************/
/* DATA TYPE(A-XDR) DEFINE */
/***************************/
#define DT_NULL_DATA			0
#define DT_ARRAY				1
#define DT_STRUCTURE			2
#define DT_BOOLEAN				3
#define DT_BIT_STRING			4
#define DT_DOUBLE_LONG			5
#define DT_DOUBLE_LONG_UNSIGNED	6
#define DT_OCTET_STRING			9
#define DT_VISIBLE_STRING		10
#define DT_UTF8_STRING			12
#define DT_BCD					13
#define DT_INTEGER				15
#define DT_LONG					16
#define DT_UNSIGNED				17
#define DT_LONG_UNSIGNED		18
#define DT_LONG64				20
#define DT_LONG64_UNSIGNED		21
#define DT_ENUM					22
#define DT_FLOAT32				23
#define DT_FLOAT64				24
#define DT_DATE_TIME			25
#define DT_DATE					26
#define DT_TIME					27
#define DT_UNDEF				DLMS_INVALID

/***************************/
/* DATA uint(A-XDR) DEFINE */
/***************************/
#define SZ_NULL_DATA			1
#define SZ_ARRAY				DLMS_INVALID
#define SZ_STRUCTURE			DLMS_INVALID
#define SZ_BOOLEAN				1
#define SZ_BIT_STRING			DLMS_INVALID
#define SZ_DOUBLE_LONG			4
#define SZ_DOUBLE_LONG_UNSIGNED	4
#define SZ_OCTET_STRING			DLMS_INVALID
#define SZ_VISIBLE_STRING		DLMS_INVALID
#define SZ_UTF8_STRING			DLMS_INVALID
#define SZ_BCD					1
#define SZ_INTEGER				1
#define SZ_LONG					2
#define SZ_UNSIGNED				1
#define SZ_LONG_UNSIGNED		2
#define SZ_LONG64				8
#define SZ_LONG64_UNSIGNED		8
#define SZ_ENUM					1
#define SZ_FLOAT32				4
#define SZ_FLOAT64				8
#define SZ_DATE_TIME			12
#define SZ_DATE					5
#define SZ_TIME					4

#define IST_BILLING_COUNT ((ui16)1) /* 1: Cumulative Billing count */
#define IST_BILLING_PERIOD ((ui16)2) /* 2: Available Billing Period  */
#define IST_TIME_NOW ((ui16)3) /* 3: Time */
#define IST_DATE_NOW ((ui16)4) /* 4: Date */
#define IST_NIC_SIMID ((ui16)5) /* 5: NIC SIM ID */
#define IST_LOGICAL_NAME ((ui16)6) /* 6: COSEM logical device name  */
#define IST_FRAME_COUNTER ((ui16)7) /* 7: Frame counter */
#define IST_FRAME_COUNTER1 ((ui16)8) /* 8: Frame counter1 */
#define IST_FRAME_COUNTER2 ((ui16)9) /* 9: Frame counter2 */
#define IST_FRAME_COUNTER3 ((ui16)10) /* 10: Frame counter3 */
#define IST_FRAME_COUNTER4 ((ui16)11) /* 11: Frame counter4 */
#define IST_FRAME_COUNTER5 ((ui16)12) /* 12: Frame counter5 */
#define IST_FRAME_COUNTER6 ((ui16)13) /* 13: Frame counter6 */
#define IST_TAMPER_COUNT ((ui16)14) /* 14: Cumulative tamper count */
#define IST_METER_TYPE ((ui16)15) /* 15: Meter Type (1Phase/3P-3W/3P-4W) */
#define IST_CATEGORY ((ui16)16) /* 16: Category */
#define IST_CURRENT_RATING ((ui16)17) /* 17: Current rating */
#define IST_EVENT_STATUS ((ui16)18) /* 18: Event status word */
#define IST_EVENT_FILTER ((ui16)19) /* 19: Event status word filter */
#define IST_METER_MODE ((ui16)20) /* 20: Metering mode */
#define IST_PAYMENT ((ui16)21) /* 21: Payment mode */
#define IST_RECHARGE_AMOUNT ((ui16)22) /* 22: Last token recharge amount */
#define IST_RECHARGE_TIME ((ui16)23) /* 23: Last token recharge time  */
#define IST_TOTAL_AMOUNT ((ui16)24) /* 24: Total amount at last recharge  */
#define IST_BALANCE_AMOUNT ((ui16)25) /* 25: Current balance amount */
#define IST_BALANCE_TIME ((ui16)26) /* 26: Current balance time */
#define IST_SERIAL_NO ((ui16)27) /* 27: Meter serial number */
#define IST_MANUFACTURE_NAME ((ui16)28) /* 28: Manufacturer name */
#define IST_DEVICE_ID ((ui16)29) /* 29: Device ID */
#define IST_METER_YEAR ((ui16)30) /* 30: Meter year of manufacture */
#define IST_NIC_IMEI ((ui16)31) /* 31: NIC IMEI number (serial number) */
#define IST_NIC_YEAR ((ui16)32) /* 32: NIC Year of manufacture */
#define IST_NIC_SERIAL ((ui16)33) /* 33: NIC serial ID */
#define IST_MAC_ADDR ((ui16)34) /* 34: Bluetooth mac address */
#define IST_NIC_VER ((ui16)35) /* 35: NIC Firmware Version */
#define IST_PROG_COUNT ((ui16)36) /* 36: Cumulative program count */
#define IST_POWER_FAILURE ((ui16)37) /* 37: Number of power failures */
#define IST_NIC_STATUS ((ui16)38) /* 38: NIC status  */
#define IST_EVENT1 ((ui16)39) /* 39: Event1 */
#define IST_EVENT2 ((ui16)40) /* 40: Event2 */
#define IST_EVENT3 ((ui16)41) /* 41: Event3 */
#define IST_EVENT4 ((ui16)42) /* 42: Event4 */
#define IST_EVENT5 ((ui16)43) /* 43: Event5 */
#define IST_EVENT6 ((ui16)44) /* 44: Event6 */
#define IST_EVENT7 ((ui16)45) /* 45: Event7 */
#define IST_LOG_CODE ((ui16)46) /* 46: Meter log code */
#define IST_FAULT ((ui16)47) /* 47: Meter status */
#define IST_EVENT1_RNO ((ui16)48) /* 48: Event1-recordNO */
#define IST_EVENT2_RNO ((ui16)49) /* 49: Event2-recordNO */
#define IST_EVENT3_RNO ((ui16)50) /* 50: Event3-recordNO */
#define IST_EVENT4_RNO ((ui16)51) /* 51: Event4-recordNO */
#define IST_EVENT5_RNO ((ui16)52) /* 52: Event5-recordNO */
#define IST_EVENT6_RNO ((ui16)53) /* 53: Event6-recordNO */
#define IST_EVENT7_RNO ((ui16)54) /* 54: Event7-recordNO */
#define IST_RELAY_COUNT ((ui16)55) /* 55: Relay Count */
#define IST_CELLID ((ui16)56) /* 56: Cell ID */
#define IST_4GGSM_PRM ((ui16)57) /* 57: 4G&NB fallback to GSM parameter */
#define IST_RATBAND ((ui16)58) /* 58: RAT & Band */
#define IST_STAMP_PRM ((ui16)59) /* 59: Event stamping parameter for RC/DC */
#define IST_RAM ((ui16)60) /* 60: RAM */
#define IST_E2P ((ui16)61) /* 61: EEPROM */
#define IST_ROM ((ui16)62) /* 62: ROM */
#define IST_CAL_ENERGY ((ui16)63) /* 63: Energy calibration */
#define IST_ENERGY ((ui16)64) /* 64: Read energy(BF)　 */
#define IST_CAL_VOLTAMP ((ui16)65) /* 65: Voltage and ampere calibration */
#define IST_VOLTAMP ((ui16)66) /* 66: Read volt and ampere(D3)　 */
#define IST_CAL_RTC ((ui16)67) /* 67: RTC calibration */
#define IST_BUILD_NO ((ui16)68) /* 68: BuildNO */
#define IST_BUILD_DATE ((ui16)69) /* 69: BildDate */
#define IST_USERFLAG ((ui16)70) /* 70: Setting bit array */
#define IST_UNIT0 ((ui16)71) /* 71: Unit setting */
#define IST_DISPINT ((ui16)72) /* 72: Display time */
#define IST_RECORD0 ((ui16)73) /* 73: Record setting */
#define IST_EVENT_CODES ((ui16)74) /* 74: Power quality code define */
#define IST_SETUP_PULS ((ui16)75) /* 75: Pulse setting */
#define IST_DETECT ((ui16)76) /* 76: Detect event setting */
#define IST_SERVER_PORT ((ui16)77) /* 77: Server port */
#define IST_EVENT_COUNT ((ui16)78) /* 78: Cumulative event count */
#define IST_MONTH_TCOUNT ((ui16)79) /* 79: Cumulative tamper count 1month */
#define IST_TOPCV_COUNT ((ui16)80) /* 80: Cumulative top cover count */
#define IST_TOPCV_DATE ((ui16)81) /* 81: Top cover tamper datetime */
#define IST_RESET_COUNT ((ui16)82) /* 82: Cumulative reset count */
#define IST_RESET_DATE ((ui16)83) /* 83: Reset datetime */
#define IST_RSSI ((ui16)84) /* 84: NIC RSSI */
#define IST_RSRP ((ui16)85) /* 85: RSRP */
#define IST_RSRQ ((ui16)86) /* 86: RSRQ */
#define IST_SNR ((ui16)87) /* 87: SNR */
#define IST_NIC_VNUM ((ui16)88) /* 88: NIC firmware version number  */
#define IST_VA_SEIPL ((ui16)89) /* 89: Apparent energy computation type */
#define IST_LRCF ((ui16)90) /* 90: LRCF */
#define IST_TYPE ((ui16)91) /* 91: Meter type */
#define IST_MODEL ((ui16)92) /* 92: Product model */
#define IST_FIRM_VER ((ui16)93) /* 93: Firmware version for meter */
#define IST_FIRM_VER1 ((ui16)94) /* 94: Active firmware version  */
#define IST_FIRM_SIG ((ui16)95) /* 95: Active firmware signature  */
#define IST_DEMAND_PERIOD ((ui16)96) /* 96: Demand integration period */
#define IST_LOAD_PERIOD ((ui16)97) /* 97: Capture period-Block load profile */
#define IST_DAILY_PERIOD ((ui16)98) /* 98: Capture period-Daily load profile */
#define IST_VA_AEW ((ui16)99) /* 99: Apparent energy computation type */
#define IST_SELFDIAG ((ui16)100) /* 100: Self diagnostic details */
#define IST_RELAY_SETTING ((ui16)101) /* 101: Current active relay time */
#define IST_RELAY_CONFIG ((ui16)102) /* 102: Active relay time setting */
#define IST_MAGNET_COUNT ((ui16)103) /* 103: Cumulative magnetic count */
#define IST_MAGNET_DATE ((ui16)104) /* 104: Magnet tamper datetime */
#define IST_ESD_COUNT ((ui16)105) /* 105: Cumulative esd count */
#define IST_ESD_DATE ((ui16)106) /* 106: ESD tamper datetime */
#define IST_JMMR_COUNT ((ui16)107) /* 107: Cumulative jammer count */
#define IST_JMMR_DATE ((ui16)108) /* 108: Jammer tamper datetime */
#define IST_VTHD_COUNT ((ui16)109) /* 109: Cumulative vthd count */
#define IST_VTHD_DATE ((ui16)110) /* 110: vthd tamper datetime */
#define IST_ATHD_COUNT ((ui16)111) /* 111: Cumulative athd count */
#define IST_ATHD_DATE ((ui16)112) /* 112: athd tamper datetime */
#define IST_ATDD_COUNT ((ui16)113) /* 113: Cumulative atdd count */
#define IST_ATDD_DATE ((ui16)114) /* 114: atdd tamper datetime */
#define IST_DISCON_COUNT ((ui16)115) /* 115: Cumulative disconnect count */
#define IST_DISCON_DATE ((ui16)116) /* 116: Disconnect datetime */
#define IST_CONNECT_COUNT ((ui16)117) /* 117: Cumulative connect count */
#define IST_CONNECT_DATE ((ui16)118) /* 118: Connect datetime */
#define IST_BILLING_DATE ((ui16)119) /* 119: Billing date */
#define IST_POWER_OFF_TIME ((ui16)120) /* 120: Cumulative power off duration in min */
#define IST_BILLING_TIME ((ui16)121) /* 121: Billing power on duration in min */
#define IST_POWER_ON_TIME ((ui16)122) /* 122: Cumulative power on duration in min */
#define IST_BATT_VOLT ((ui16)123) /* 123: Battery voltage */
#define IST_CPUTIME ((ui16)124) /* 124: Operating time objects  */
#define IST_TEMPERATURE ((ui16)125) /* 125: Temperature */
#define IST_TEMP_BLOCK ((ui16)126) /* 126: Temperature block */
#define IST_TEMP_BLOCK24 ((ui16)127) /* 127: Temperature block24 */
#define IST_MAGNET ((ui16)128) /* 128: Magnet voltage */
#define IST_CPUTEMP_BLOCK ((ui16)129) /* 129: Temperature(CPU/MAG) block */
#define IST_CPUTEMP_BLOCK24 ((ui16)130) /* 130: Temperature(CPU/MAG) block24 */
#define IST_FWD_DEMAND ((ui16)131) /* 131: Cumulative kW MD Import  */
#define IST_POWER ((ui16)132) /* 132: Active power+- */
#define IST_POWER_THD ((ui16)133) /* 133: THD Power Phase  */
#define IST_FWD_ENERGY ((ui16)134) /* 134: Cumulative Energy+ Wh */
#define IST_FWD_ENERGY1 ((ui16)135) /* 135: Cumulative Energy+ Wh TZ1 */
#define IST_FWD_ENERGY2 ((ui16)136) /* 136: Cumulative Energy+ Wh TZ2 */
#define IST_FWD_ENERGY3 ((ui16)137) /* 137: Cumulative Energy+ Wh TZ3 */
#define IST_FWD_ENERGY4 ((ui16)138) /* 138: Cumulative Energy+ Wh TZ4 */
#define IST_FWD_ENERGY5 ((ui16)139) /* 139: Cumulative Energy+ Wh TZ5 */
#define IST_FWD_ENERGY6 ((ui16)140) /* 140: Cumulative Energy+ Wh TZ6 */
#define IST_FWD_ENERGY7 ((ui16)141) /* 141: Cumulative Energy+ Wh TZ7 */
#define IST_FWD_ENERGY8 ((ui16)142) /* 142: Cumulative Energy+ Wh TZ8 */
#define IST_FWD_BLOCK_W ((ui16)143) /* 143: Block Energy+ W */
#define IST_FWD_BLOCK_W1 ((ui16)144) /* 144: Block Energy+ W TZ1 */
#define IST_FWD_BLOCK_W2 ((ui16)145) /* 145: Block Energy+ W TZ2 */
#define IST_FWD_BLOCK_W3 ((ui16)146) /* 146: Block Energy+ W TZ3 */
#define IST_FWD_BLOCK_W4 ((ui16)147) /* 147: Block Energy+ W TZ4 */
#define IST_FWD_BLOCK_W5 ((ui16)148) /* 148: Block Energy+ W TZ5 */
#define IST_FWD_BLOCK_W6 ((ui16)149) /* 149: Block Energy+ W TZ6 */
#define IST_FWD_BLOCK_W7 ((ui16)150) /* 150: Block Energy+ W TZ7 */
#define IST_FWD_BLOCK_W8 ((ui16)151) /* 151: Block Energy+ W TZ8 */
#define IST_FWD_BLOCK_WH ((ui16)152) /* 152: Block Energy+ Wh */
#define IST_FWD_BLOCK_WH1 ((ui16)153) /* 153: Block Energy+ Wh TZ1 */
#define IST_FWD_BLOCK_WH2 ((ui16)154) /* 154: Block Energy+ Wh TZ2 */
#define IST_FWD_BLOCK_WH3 ((ui16)155) /* 155: Block Energy+ Wh TZ3 */
#define IST_FWD_BLOCK_WH4 ((ui16)156) /* 156: Block Energy+ Wh TZ4 */
#define IST_FWD_BLOCK_WH5 ((ui16)157) /* 157: Block Energy+ Wh TZ5 */
#define IST_FWD_BLOCK_WH6 ((ui16)158) /* 158: Block Energy+ Wh TZ6 */
#define IST_FWD_BLOCK_WH7 ((ui16)159) /* 159: Block Energy+ Wh TZ7 */
#define IST_FWD_BLOCK_WH8 ((ui16)160) /* 160: Block Energy+ Wh TZ8 */
#define IST_BAK_DEMAND ((ui16)161) /* 161: Cumulative kW MD Export */
#define IST_BAK_ENERGY ((ui16)162) /* 162: Cumulative Energy- Wh */
#define IST_BAK_ENERGY1 ((ui16)163) /* 163: Cumulative Energy- Wh TZ1 */
#define IST_BAK_ENERGY2 ((ui16)164) /* 164: Cumulative Energy- Wh TZ2 */
#define IST_BAK_ENERGY3 ((ui16)165) /* 165: Cumulative Energy- Wh TZ3 */
#define IST_BAK_ENERGY4 ((ui16)166) /* 166: Cumulative Energy- Wh TZ4 */
#define IST_BAK_ENERGY5 ((ui16)167) /* 167: Cumulative Energy- Wh TZ5 */
#define IST_BAK_ENERGY6 ((ui16)168) /* 168: Cumulative Energy- Wh TZ6 */
#define IST_BAK_ENERGY7 ((ui16)169) /* 169: Cumulative Energy- Wh TZ7 */
#define IST_BAK_ENERGY8 ((ui16)170) /* 170: Cumulative Energy- Wh TZ8 */
#define IST_BAK_BLOCK_W ((ui16)171) /* 171: Block Energy- W */
#define IST_BAK_BLOCK_W1 ((ui16)172) /* 172: Block Energy- W TZ1 */
#define IST_BAK_BLOCK_W2 ((ui16)173) /* 173: Block Energy- W TZ2 */
#define IST_BAK_BLOCK_W3 ((ui16)174) /* 174: Block Energy- W TZ3 */
#define IST_BAK_BLOCK_W4 ((ui16)175) /* 175: Block Energy- W TZ4 */
#define IST_BAK_BLOCK_W5 ((ui16)176) /* 176: Block Energy- W TZ5 */
#define IST_BAK_BLOCK_W6 ((ui16)177) /* 177: Block Energy- W TZ6 */
#define IST_BAK_BLOCK_W7 ((ui16)178) /* 178: Block Energy- W TZ7 */
#define IST_BAK_BLOCK_W8 ((ui16)179) /* 179: Block Energy- W TZ8 */
#define IST_BAK_BLOCK_WH ((ui16)180) /* 180: Block Energy- Wh */
#define IST_BAK_BLOCK_WH1 ((ui16)181) /* 181: Block Energy- Wh TZ1 */
#define IST_BAK_BLOCK_WH2 ((ui16)182) /* 182: Block Energy- Wh TZ2 */
#define IST_BAK_BLOCK_WH3 ((ui16)183) /* 183: Block Energy- Wh TZ3 */
#define IST_BAK_BLOCK_WH4 ((ui16)184) /* 184: Block Energy- Wh TZ4 */
#define IST_BAK_BLOCK_WH5 ((ui16)185) /* 185: Block Energy- Wh TZ5 */
#define IST_BAK_BLOCK_WH6 ((ui16)186) /* 186: Block Energy- Wh TZ6 */
#define IST_BAK_BLOCK_WH7 ((ui16)187) /* 187: Block Energy- Wh TZ7 */
#define IST_BAK_BLOCK_WH8 ((ui16)188) /* 188: Block Energy- Wh TZ8 */
#define IST_QPOWER ((ui16)189) /* 189: Reactive power+- */
#define IST_FWD_QENERGY ((ui16)190) /* 190: Cumulative Energy+ varh */
#define IST_BAK_QENERGY ((ui16)191) /* 191: Cumulative Energy- varh */
#define IST_VARH_Q1 ((ui16)192) /* 192: Cumulative Energy kvarh (Q1) */
#define IST_FWD_BLOCK_VARH ((ui16)193) /* 193: Block Energy+ kvarh */
#define IST_VARH_Q2 ((ui16)194) /* 194: Cumulative Energy kvarh (Q2) */
#define IST_BLOCK_Q2 ((ui16)195) /* 195: Block Energy kvarh (Q2) */
#define IST_VARH_Q3 ((ui16)196) /* 196: Cumulative Energy kvarh (Q3) */
#define IST_BLOCK_Q3 ((ui16)197) /* 197: Block Energy kvarh (Q3) */
#define IST_VARH_Q4 ((ui16)198) /* 198: Cumulative Energy kvarh (Q4) */
#define IST_BAK_BLOCK_VARH ((ui16)199) /* 199: Block Energy- kvarh */
#define IST_FWD_VA ((ui16)200) /* 200: Cumulative kVA MD Import  */
#define IST_APPERENT ((ui16)201) /* 201: Apparent power kVA+- */
#define IST_FWD_VAH ((ui16)202) /* 202: Cumulative Energy+ kVAh */
#define IST_FWD_VAH1 ((ui16)203) /* 203: Cumulative Energy+ kVAh TZ1 */
#define IST_FWD_VAH2 ((ui16)204) /* 204: Cumulative Energy+ kVAh TZ2 */
#define IST_FWD_VAH3 ((ui16)205) /* 205: Cumulative Energy+ kVAh TZ3 */
#define IST_FWD_VAH4 ((ui16)206) /* 206: Cumulative Energy+ kVAh TZ4 */
#define IST_FWD_VAH5 ((ui16)207) /* 207: Cumulative Energy+ kVAh TZ5 */
#define IST_FWD_VAH6 ((ui16)208) /* 208: Cumulative Energy+ kVAh TZ6 */
#define IST_FWD_VAH7 ((ui16)209) /* 209: Cumulative Energy+ kVAh TZ7 */
#define IST_FWD_VAH8 ((ui16)210) /* 210: Cumulative Energy+ kVAh TZ8 */
#define IST_FWD_BLOCK_VA ((ui16)211) /* 211: Block Energy+ VA */
#define IST_FWD_BLOCK_VA1 ((ui16)212) /* 212: Block Energy+ VA TZ1 */
#define IST_FWD_BLOCK_VA2 ((ui16)213) /* 213: Block Energy+ VA TZ2 */
#define IST_FWD_BLOCK_VA3 ((ui16)214) /* 214: Block Energy+ VA TZ3 */
#define IST_FWD_BLOCK_VA4 ((ui16)215) /* 215: Block Energy+ VA TZ4 */
#define IST_FWD_BLOCK_VA5 ((ui16)216) /* 216: Block Energy+ VA TZ5 */
#define IST_FWD_BLOCK_VA6 ((ui16)217) /* 217: Block Energy+ VA TZ6 */
#define IST_FWD_BLOCK_VA7 ((ui16)218) /* 218: Block Energy+ VA TZ7 */
#define IST_FWD_BLOCK_VA8 ((ui16)219) /* 219: Block Energy+ VA TZ8 */
#define IST_FWD_BLOCK_VAH ((ui16)220) /* 220: Block Energy+ VAh */
#define IST_FWD_BLOCK_VAH1 ((ui16)221) /* 221: Block Energy+ VAh TZ1 */
#define IST_FWD_BLOCK_VAH2 ((ui16)222) /* 222: Block Energy+ VAh TZ2 */
#define IST_FWD_BLOCK_VAH3 ((ui16)223) /* 223: Block Energy+ VAh TZ3 */
#define IST_FWD_BLOCK_VAH4 ((ui16)224) /* 224: Block Energy+ VAh TZ4 */
#define IST_FWD_BLOCK_VAH5 ((ui16)225) /* 225: Block Energy+ VAh TZ5 */
#define IST_FWD_BLOCK_VAH6 ((ui16)226) /* 226: Block Energy+ VAh TZ6 */
#define IST_FWD_BLOCK_VAH7 ((ui16)227) /* 227: Block Energy+ VAh TZ7 */
#define IST_FWD_BLOCK_VAH8 ((ui16)228) /* 228: Block Energy+ VAh TZ8 */
#define IST_BAK_VA ((ui16)229) /* 229: Cumulative kVA MD Export */
#define IST_BAK_VAH ((ui16)230) /* 230: Cumulative Energy- kVAh */
#define IST_BAK_VAH1 ((ui16)231) /* 231: Cumulative Energy- kVAh TZ1 */
#define IST_BAK_VAH2 ((ui16)232) /* 232: Cumulative Energy- kVAh TZ2 */
#define IST_BAK_VAH3 ((ui16)233) /* 233: Cumulative Energy- kVAh TZ3 */
#define IST_BAK_VAH4 ((ui16)234) /* 234: Cumulative Energy- kVAh TZ4 */
#define IST_BAK_VAH5 ((ui16)235) /* 235: Cumulative Energy- kVAh TZ5 */
#define IST_BAK_VAH6 ((ui16)236) /* 236: Cumulative Energy- kVAh TZ6 */
#define IST_BAK_VAH7 ((ui16)237) /* 237: Cumulative Energy- kVAh TZ7 */
#define IST_BAK_VAH8 ((ui16)238) /* 238: Cumulative Energy- kVAh TZ8 */
#define IST_BAK_BLOCK_VA ((ui16)239) /* 239: Block Energy- VA */
#define IST_BAK_BLOCK_VA1 ((ui16)240) /* 240: Block Energy- VA TZ1 */
#define IST_BAK_BLOCK_VA2 ((ui16)241) /* 241: Block Energy- VA TZ2 */
#define IST_BAK_BLOCK_VA3 ((ui16)242) /* 242: Block Energy- VA TZ3 */
#define IST_BAK_BLOCK_VA4 ((ui16)243) /* 243: Block Energy- VA TZ4 */
#define IST_BAK_BLOCK_VA5 ((ui16)244) /* 244: Block Energy- VA TZ5 */
#define IST_BAK_BLOCK_VA6 ((ui16)245) /* 245: Block Energy- VA TZ6 */
#define IST_BAK_BLOCK_VA7 ((ui16)246) /* 246: Block Energy- VA TZ7 */
#define IST_BAK_BLOCK_VA8 ((ui16)247) /* 247: Block Energy- VA TZ8 */
#define IST_BAK_BLOCK_VAH ((ui16)248) /* 248: Block Energy- VAh */
#define IST_BAK_BLOCK_VAH1 ((ui16)249) /* 249: Block Energy- VAh TZ1 */
#define IST_BAK_BLOCK_VAH2 ((ui16)250) /* 250: Block Energy- VAh TZ2 */
#define IST_BAK_BLOCK_VAH3 ((ui16)251) /* 251: Block Energy- VAh TZ3 */
#define IST_BAK_BLOCK_VAH4 ((ui16)252) /* 252: Block Energy- VAh TZ4 */
#define IST_BAK_BLOCK_VAH5 ((ui16)253) /* 253: Block Energy- VAh TZ5 */
#define IST_BAK_BLOCK_VAH6 ((ui16)254) /* 254: Block Energy- VAh TZ6 */
#define IST_BAK_BLOCK_VAH7 ((ui16)255) /* 255: Block Energy- VAh TZ7 */
#define IST_BAK_BLOCK_VAH8 ((ui16)256) /* 256: Block Energy- VAh TZ8 */
#define IST_AMPR0 ((ui16)257) /* 257: Phase current */
#define IST_AMPR_THD ((ui16)258) /* 258: THD Current  */
#define IST_AMPR0_BLOCK ((ui16)259) /* 259: Block phase current */
#define IST_AVERAGE_VOLT0 ((ui16)260) /* 260: Voltage0 Ave 1min value  */
#define IST_VOLT0 ((ui16)261) /* 261: Voltage */
#define IST_VOLT_THD ((ui16)262) /* 262: THD Voltage  */
#define IST_VOLT0_BLOCK ((ui16)263) /* 263: Block voltage */
#define IST_VTHD_BLOCK ((ui16)264) /* 264: Block voltage THD */
#define IST_FWD_PF_MON ((ui16)265) /* 265: Average power factor for billing period */
#define IST_FWD_PF1 ((ui16)266) /* 266: System Imp PF T1  */
#define IST_FWD_PF2 ((ui16)267) /* 267: System Imp PF T2  */
#define IST_FWD_PF3 ((ui16)268) /* 268: System Imp PF T3  */
#define IST_FWD_PF4 ((ui16)269) /* 269: System Imp PF T4  */
#define IST_FWD_PF5 ((ui16)270) /* 270: System Imp PF T5  */
#define IST_FWD_PF6 ((ui16)271) /* 271: System Imp PF T6  */
#define IST_FWD_PF7 ((ui16)272) /* 272: System Imp PF T7  */
#define IST_FWD_PF8 ((ui16)273) /* 273: System Imp PF T8  */
#define IST_POWER_FACTOR ((ui16)274) /* 274: Signed power factor/Three phase power factor */
#define IST_PF_BLOCK ((ui16)275) /* 275: Signed power factor/Three phase power factor */
#define IST_PF_BLOCK24 ((ui16)276) /* 276: Signed power factor/Three phase power factor */
#define IST_FWD_PF_24 ((ui16)277) /* 277: Power factor imp day */
#define IST_FREQ ((ui16)278) /* 278: Frequency */
#define IST_ABS_ENERGY ((ui16)279) /* 279: ABS Wh */
#define IST_NET_ENERGY ((ui16)280) /* 280: NET Wh */
#define IST_ABS_QENERGY ((ui16)281) /* 281: ABS VarH */
#define IST_NET_QENERGY ((ui16)282) /* 282: NET VarH */
#define IST_AMPR1 ((ui16)283) /* 283: Current L1 */
#define IST_AMPR1_BLOCK ((ui16)284) /* 284: IR-block */
#define IST_AVERAGE_VOLT1 ((ui16)285) /* 285: Voltage L1 Ave 1min value  */
#define IST_VOLT1 ((ui16)286) /* 286: Voltage L1 */
#define IST_VOLT1_BLOCK ((ui16)287) /* 287: Voltage L1 Block */
#define IST_POWER_FACTOR1 ((ui16)288) /* 288: Signed power factorR */
#define IST_AMPR2 ((ui16)289) /* 289: Current L2 */
#define IST_AMPR2_BLOCK ((ui16)290) /* 290: IY-block */
#define IST_AVERAGE_VOLT2 ((ui16)291) /* 291: Voltage L2 Ave 1min value  */
#define IST_VOLT2 ((ui16)292) /* 292: Voltage L2 */
#define IST_VOLT2_BLOCK ((ui16)293) /* 293: Voltage L2 Block */
#define IST_POWER_FACTOR2 ((ui16)294) /* 294: Signed power factorY */
#define IST_AMPR3 ((ui16)295) /* 295: Current L3 */
#define IST_AMPR3_BLOCK ((ui16)296) /* 296: IB-block */
#define IST_AVERAGE_VOLT3 ((ui16)297) /* 297: Voltage L3 Ave 1min value  */
#define IST_VOLT3 ((ui16)298) /* 298: Voltage L3 */
#define IST_VOLT3_BLOCK ((ui16)299) /* 299: Voltage L3 Block */
#define IST_POWER_FACTOR3 ((ui16)300) /* 300: Signed power factorB */
#define IST_BAK_PF_MON ((ui16)301) /* 301: Power factor exp month */
#define IST_BAK_PF1 ((ui16)302) /* 302: System Exp PF T1  */
#define IST_BAK_PF2 ((ui16)303) /* 303: System Exp PF T2  */
#define IST_BAK_PF3 ((ui16)304) /* 304: System Exp PF T3  */
#define IST_BAK_PF4 ((ui16)305) /* 305: System Exp PF T4  */
#define IST_BAK_PF5 ((ui16)306) /* 306: System Exp PF T5  */
#define IST_BAK_PF6 ((ui16)307) /* 307: System Exp PF T6  */
#define IST_BAK_PF7 ((ui16)308) /* 308: System Exp PF T7  */
#define IST_BAK_PF8 ((ui16)309) /* 309: System Exp PF T8  */
#define IST_BAK_PF_BLOCK ((ui16)310) /* 310: Power factor exp block  */
#define IST_BAK_PF_24 ((ui16)311) /* 311: Power factor exp day */
#define IST_AMPRG ((ui16)312) /* 312: Neutral current */
#define IST_AMPRG_BLOCK ((ui16)313) /* 313: Block neutral current */
#define IST_AMPRG_BLOCK24 ((ui16)314) /* 314: Average24 neutral current */
#define IST_AMPRC ((ui16)315) /* 315: Current */
#define IST_DEFRAUD_POWER ((ui16)316) /* 316: Defraud Kwh */
#define IST_CPUTEMP ((ui16)317) /* 317: Temperature(CPU/MAG) */
#define IST_AMPRC_BLOCK ((ui16)318) /* 318: Current-Avearage? */
#define IST_FUND_ENERGY ((ui16)319) /* 319: Fundamental Energy kWh */
#define IST_ATHD_BLOCK ((ui16)320) /* 320: Block current THD */
#define IST_ATHD_BLOCK24 ((ui16)321) /* 321: Average24 current THD */
#define IST_AMPR_TDD ((ui16)322) /* 322: Current TDD */
#define IST_ATDD_BLOCK ((ui16)323) /* 323: Block current TDD */
#define IST_ATDD_BLOCK24 ((ui16)324) /* 324: Average24 current TDD */
#define IST_FREQ1 ((ui16)325) /* 325: Frequency L1  */
#define IST_FREQ2 ((ui16)326) /* 326: Frequency L2 */
#define IST_FREQ3 ((ui16)327) /* 327: Frequency L3 */
#define IST_FWD_BLOCK24_W ((ui16)328) /* 328: Block24 Energy+ W */
#define IST_FWD_BLOCK24_WH ((ui16)329) /* 329: Block24 Energy+ Wh */
#define IST_BAK_BLOCK24_W ((ui16)330) /* 330: Block24 Energy- W */
#define IST_BAK_BLOCK24_WH ((ui16)331) /* 331: Block24 Energy- Wh */
#define IST_FWD_BLOCK24_VARH ((ui16)332) /* 332: Block24 Energy+ kvarh */
#define IST_BAK_BLOCK24_VARH ((ui16)333) /* 333: Block24 Energy- kvarh */
#define IST_FWD_BLOCK24_VA ((ui16)334) /* 334: Block24 Energy+ VA */
#define IST_FWD_BLOCK24_VAH ((ui16)335) /* 335: Block24 Energy+ VAh */
#define IST_BAK_BLOCK24_VA ((ui16)336) /* 336: Block24 Energy- VA */
#define IST_BAK_BLOCK24_VAH ((ui16)337) /* 337: Block24 Energy- VAh */
#define IST_AMPR0_BLOCK24 ((ui16)338) /* 338: Average24 phase current */
#define IST_VOLT0_BLOCK24 ((ui16)339) /* 339: Average24 voltage */
#define IST_VTHD_BLOCK24 ((ui16)340) /* 340: Average24 voltage THD */
#define IST_NET_VAH ((ui16)341) /* 341: Apparent energy net total */
#define IST_RSSI_BLOCK ((ui16)342) /* 342: Average signal strength(RSSI) */
#define IST_MAX_FWD_DEMAND ((ui16)343) /* 343: Max Demand+ kW */
#define IST_MAX_FWD_DEMAND1 ((ui16)344) /* 344: Max Demand+ kW TZ1 */
#define IST_MAX_FWD_DEMAND2 ((ui16)345) /* 345: Max Demand+ kW TZ2 */
#define IST_MAX_FWD_DEMAND3 ((ui16)346) /* 346: Max Demand+ kW TZ3 */
#define IST_MAX_FWD_DEMAND4 ((ui16)347) /* 347: Max Demand+ kW TZ4 */
#define IST_MAX_FWD_DEMAND5 ((ui16)348) /* 348: Max Demand+ kW TZ5 */
#define IST_MAX_FWD_DEMAND6 ((ui16)349) /* 349: Max Demand+ kW TZ6 */
#define IST_MAX_FWD_DEMAND7 ((ui16)350) /* 350: Max Demand+ kW TZ7 */
#define IST_MAX_FWD_DEMAND8 ((ui16)351) /* 351: Max Demand+ kW TZ8 */
#define IST_MAX_BAK_DEMAND ((ui16)352) /* 352: Max Demand- kW */
#define IST_MAX_BAK_DEMAND1 ((ui16)353) /* 353: Max Demand- kW TZ1 */
#define IST_MAX_BAK_DEMAND2 ((ui16)354) /* 354: Max Demand- kW TZ2 */
#define IST_MAX_BAK_DEMAND3 ((ui16)355) /* 355: Max Demand- kW TZ3 */
#define IST_MAX_BAK_DEMAND4 ((ui16)356) /* 356: Max Demand- kW TZ4 */
#define IST_MAX_BAK_DEMAND5 ((ui16)357) /* 357: Max Demand- kW TZ5 */
#define IST_MAX_BAK_DEMAND6 ((ui16)358) /* 358: Max Demand- kW TZ6 */
#define IST_MAX_BAK_DEMAND7 ((ui16)359) /* 359: Max Demand- kW TZ7 */
#define IST_MAX_BAK_DEMAND8 ((ui16)360) /* 360: Max Demand- kW TZ8 */
#define IST_MAX_FWD_QDEMAND ((ui16)361) /* 361: Max.Demand+ var */
#define IST_MAX_BAK_QDEMAND ((ui16)362) /* 362: Max.Demand- var */
#define IST_MAX_FWD_VA ((ui16)363) /* 363: Max Demand+ kVA */
#define IST_MAX_FWD_VA1 ((ui16)364) /* 364: Max Demand+ kVA TZ1 */
#define IST_MAX_FWD_VA2 ((ui16)365) /* 365: Max Demand+ kVA TZ2 */
#define IST_MAX_FWD_VA3 ((ui16)366) /* 366: Max Demand+ kVA TZ3 */
#define IST_MAX_FWD_VA4 ((ui16)367) /* 367: Max Demand+ kVA TZ4 */
#define IST_MAX_FWD_VA5 ((ui16)368) /* 368: Max Demand+ kVA TZ5 */
#define IST_MAX_FWD_VA6 ((ui16)369) /* 369: Max Demand+ kVA TZ6 */
#define IST_MAX_FWD_VA7 ((ui16)370) /* 370: Max Demand+ kVA TZ7 */
#define IST_MAX_FWD_VA8 ((ui16)371) /* 371: Max Demand+ kVA TZ8 */
#define IST_MAX_BAK_VA ((ui16)372) /* 372: Max Demand- kVA */
#define IST_MAX_BAK_VA1 ((ui16)373) /* 373: Max Demand- kVA TZ1 */
#define IST_MAX_BAK_VA2 ((ui16)374) /* 374: Max Demand- kVA TZ2 */
#define IST_MAX_BAK_VA3 ((ui16)375) /* 375: Max Demand- kVA TZ3 */
#define IST_MAX_BAK_VA4 ((ui16)376) /* 376: Max Demand- kVA TZ4 */
#define IST_MAX_BAK_VA5 ((ui16)377) /* 377: Max Demand- kVA TZ5 */
#define IST_MAX_BAK_VA6 ((ui16)378) /* 378: Max Demand- kVA TZ6 */
#define IST_MAX_BAK_VA7 ((ui16)379) /* 379: Max Demand- kVA TZ7 */
#define IST_MAX_BAK_VA8 ((ui16)380) /* 380: Max Demand- kVA TZ8 */
#define IST_MINIMUM_VOLT0 ((ui16)381) /* 381: Minimum Voltage0 */
#define IST_MINIMUM_VOLT1 ((ui16)382) /* 382: Minimum Voltage of L1  */
#define IST_MINIMUM_VOLT2 ((ui16)383) /* 383: Minimum Voltage of L2  */
#define IST_MINIMUM_VOLT3 ((ui16)384) /* 384: Minimum Voltage of L3  */
#define IST_MAX24_FWD_DEMAND ((ui16)385) /* 385: Max24 Demand+ kW */
#define IST_MAX24_BAK_DEMAND ((ui16)386) /* 386: Max24 Demand- kW */
#define IST_MAX24_FWD_QDEMAND ((ui16)387) /* 387: Max24 Demand+ var */
#define IST_MAX24_BAK_QDEMAND ((ui16)388) /* 388: Max24 Demand- var */
#define IST_MAX24_FWD_VA ((ui16)389) /* 389: Max24 Demand+ kVA */
#define IST_MAX24_BAK_VA ((ui16)390) /* 390: Max24 Demand- kVA */
#define IST_NAME_PLATE ((ui16)391) /* 391: Name plate detail */
#define IST_EVENT1_PROFILE ((ui16)392) /* 392: Event1 profile */
#define IST_EVENT2_PROFILE ((ui16)393) /* 393: Event2 profile */
#define IST_EVENT3_PROFILE ((ui16)394) /* 394: Event3 profile */
#define IST_EVENT4_PROFILE ((ui16)395) /* 395: Event4 profile */
#define IST_EVENT5_PROFILE ((ui16)396) /* 396: Event5 profile */
#define IST_EVENT6_PROFILE ((ui16)397) /* 397: Event6 profile */
#define IST_EVENT7_PROFILE ((ui16)398) /* 398: Event7 profile */
#define IST_RELAY_LOG ((ui16)399) /* 399: Relay log */
#define IST_METER_LOG ((ui16)400) /* 400: Meter log */
#define IST_SNAP_PARAM1 ((ui16)401) /* 401: Snapshot of instantaneous parameters A1,A14 */
#define IST_SCAL_UNIT1 ((ui16)402) /* 402: Scaler profile-A1,A14 */
#define IST_SCAL_UNIT2 ((ui16)403) /* 403: Scaler profile-A2,A15 */
#define IST_SCAL_UNIT3 ((ui16)404) /* 404: Scaler profile-A3,A16 */
#define IST_SCAL_UNIT4 ((ui16)405) /* 405: Scaler profile-A4,A17 */
#define IST_SCAL_UNIT5 ((ui16)406) /* 406: Scaler profile-A5-A11 */
#define IST_BILLING_PARAMS ((ui16)407) /* 407: Billing profile parameters */
#define IST_LOAD_PROFILE ((ui16)408) /* 408: Block load profile */
#define IST_DAILY_PROFILE ((ui16)409) /* 409: Daily load profile */
#define IST_DATETIME_NOW ((ui16)410) /* 410: Date time */
#define IST_DATETIME_RTC ((ui16)411) /* 411: Date time(RTC) */
#define IST_DEMAND_RESET ((ui16)412) /* 412: MDI Reset/End of Billing Period script table */
#define IST_TARIFF_SCR ((ui16)413) /* 413: Tariffication script table */
#define IST_IMG_SCR ((ui16)414) /* 414: Image activation script table */
#define IST_PUSH_SCR ((ui16)415) /* 415: Push script table */
#define IST_SPECIAL_DAYS ((ui16)416) /* 416: Special days table */
#define IST_ASSO_LN0 ((ui16)417) /* 417: Current Association */
#define IST_ASSO_LN1 ((ui16)418) /* 418: PC-Association */
#define IST_ASSO_LN2 ((ui16)419) /* 419: MR-Association */
#define IST_ASSO_LN3 ((ui16)420) /* 420: US-Association */
#define IST_ASSO_LN4 ((ui16)421) /* 421: PUSH-Association */
#define IST_ASSO_LN5 ((ui16)422) /* 422: FIRM-Association */
#define IST_ASSO_LN6 ((ui16)423) /* 423: IHD-Association */
#define IST_IMG_TRANS ((ui16)424) /* 424: Image transfer */
#define IST_IMG_TRANS1 ((ui16)425) /* 425: Image transfer1 */
#define IST_TIMEZONE ((ui16)426) /* 426: Activity calendar for time zones */
#define IST_BILLDATE_ACT ((ui16)427) /* 427: Single action schedule for billing dates */
#define IST_IMG_ACT ((ui16)428) /* 428: Image Activation Single action schedule */
#define IST_SM_TO_HES_ACT ((ui16)429) /* 429: Trigger SM TO HES-Single action schedule */
#define IST_TOWWAY_ACT ((ui16)430) /* 430: Single action schedule for Two Way Communication  */
#define IST_TAMPER_ACT ((ui16)431) /* 431: Single action schedule for Tamper data Push */
#define IST_SM_TO_IHD_ACT ((ui16)432) /* 432: Trigger SM TO IHD-Single action schedule */
#define IST_LOAD_ACT ((ui16)433) /* 433: Block load profile push schedule change */
#define IST_DAILY_ACT ((ui16)434) /* 434: Daily load profile push schedule change */
#define IST_BILLING_ACT ((ui16)435) /* 435: Billing rofile push schedule change */
#define IST_SETUP_HDLC0 ((ui16)436) /* 436: IEC HDLC setup */
#define IST_SETUP_HDLC1 ((ui16)437) /* 437: HDLC Setting(Other) */
#define IST_PUSHD_HES ((ui16)438) /* 438: SM to HES push */
#define IST_PUSHD_IHD ((ui16)439) /* 439: SM TO IHD push */
#define IST_PUSHE_HES ((ui16)440) /* 440: SM TO HES event push */
#define IST_PUSHD_LOD ((ui16)441) /* 441: SM to HES Load profle push */
#define IST_PUSHD_DAY ((ui16)442) /* 442: SM to HES daily profle push */
#define IST_PUSHD_SELFREG ((ui16)443) /* 443: Self Registration Push */
#define IST_PUSHD_BILL ((ui16)444) /* 444: SM to HES Billing profle push */
#define IST_PUSHD_TAMP ((ui16)445) /* 445: SM to HES Tamper push */
#define IST_TCP_UDP ((ui16)446) /* 446: TCP-UDP setup */
#define IST_GPRS ((ui16)447) /* 447: ANP setting */
#define IST_IPV6 ((ui16)448) /* 448: IPV6 setup */
#define IST_SECURITY_NONE ((ui16)449) /* 449: Setup security(For security NON) */
#define IST_SECURITY_HLS ((ui16)450) /* 450: Setup security(For security HLS) */
#define IST_SECURITY_MR ((ui16)451) /* 451: MR-Security setup */
#define IST_SECURITY_US ((ui16)452) /* 452: US-Security setup */
#define IST_SECURITY_PUSH ((ui16)453) /* 453: PUSH-Security setup */
#define IST_SECURITY_FIRM ((ui16)454) /* 454: FIRM-Security setup */
#define IST_SECURITY_IHD ((ui16)455) /* 455: IHD-Security setup */
#define IST_BREAKER ((ui16)456) /* 456: Disconnect control */
#define IST_LOAD_LIMIT ((ui16)457) /* 457: Load limit value in kW */

#define IST_END						(IST_LOAD_LIMIT+1)/* */
//#define IST_END						(IST_LOAD_LIMIT+1)/* */
#define IST_ENTRY                 0xFFFF/* エントリーオブジェクト*/

#define LN_NO_CIPHER	1
#define SN_NO_CIPHER	2
#define LN_CIPHER		3
#define SN_CIPHER		4
extern cui08 application_name[][LEN_CONTEXT_NAME+1];

/*
enum:
(0) nothing,
(1) all messages to be authenticated,
(2) all messages to be encrypted,
(3) all messages to be authenticated and encrypted
*/
#define SECURITY_POLICY_NN	0
#define SECURITY_POLICY_AN	1
#define SECURITY_POLICY_NE	2
#define SECURITY_POLICY_AE	3

/*COSEM_lowest_level_security_mechanism_name ::=mechanism_id(0)*/
/*COSEM_low_level_security_mechanism_name ::=mechanism_id(1)*/
/*COSEM_high_level_security_mechanism_name ::=mechanism_id(2)*/
/*COSEM_high_level_security_mechanism_name_using_MD5 ::=mechanism_id(3)*/
/*COSEM_high_level_security_mechanism_name_using_SHA-1 ::=mechanism_id(4)*/
/*COSEM_High_Level_Security_Mechanism_Name_Using_GMAC ::=mechanism_id(5)*/
#define MECHA_NONE		0
#define MECHA_LLS_PASS	1
#define MECHA_HLS		2
#define MECHA_HLS_MD5	3
#define MECHA_HLS_SHA1	4
#define MECHA_HLS_GMAC	5

/*RLRE result*/
#define RL_NORMAL				0
#define RL_NOT_FINISHED			1

/*--------------------------------------------------------------------------*/
#define PHASE_RESPONSE		0x00U
#define PHASE_THREAD		0x01U
#define PHASE_CHANGE_KEY1	0x02U
#define PHASE_CHANGE_KEY2	0x03U
#define PHASE_CHANGE_END	0x04U
#define PHASE_INITIAL		0xffU

/*変数の書式*/
#define I(cls,atr,ist)			cl##cls##_ar##atr##_##ist				/* I(1,2,serial_no):: cl1_ar2_serial_no*/

/*関数の書式*/
#define G(sel,cls,atr,ist)		get_sl##sel##_cl##cls##_ar##atr##_##ist	/* G(0,1,2,serial_no):: get_sl0_cl1_ar2_serial_no */
#define S(sel,cls,atr,ist)		set_sl##sel##_cl##cls##_ar##atr##_##ist	/* S(0,1,2,serial_no):: set_sl0_cl1_ar2_serial_no */
#define M(cls,mth,ist)			act_cl##cls##_mh##mth##_##ist			/* M(1,2,serial_no)  :: act_cl1_mh2_serial_no     */

ui08	getNegoBaud0(void);
ui08	getNegoBaud1(void);
void	getTimeout1(pui16 oct, pui16 act);
void	getTimeout2(pui16 oct, pui16 act);
void	getNegoParam1(pui16 len_t, pui16 len_r, pui08 win_t, pui08 win_r);
void	getNegoParam2(pui16 len_t, pui16 len_r, pui08 win_t, pui08 win_r);
ui08	isAssociating(void);

ui08	bCheckDateTime(cpui08 date_time, cui08 allow_wild );
ui08 	get_dedicated_key( pui08 key );
pui08 	get_system_title0( void );
pui08 	get_system_title( void );
ui08	get_security_control( void );

pui08	get_meter_datetime( void );
void	ConfirmServiceError( cui08 r );
ui08	exist_aa( void );
void	set_non_associated(cui08 port);/*0:all, 1,2*/
ui08	associated_client( void );
ui08	authenticated_client( void );
ui08	authenticated_associated_client( void );
ui08	challenge_client( void );
ui08	get_addr( void );

ui08	inactivity_timeout0(void);
ui08	inactivity_timeout1(void);
void	clear_inactivity_timeout( cui08 port );
void	set_tsnset(const USERSET1 *in);
void	get_tsnset(USERSET1 * const out);
void	init_system_title( void );
void	init_xDLMS( void );
void	xDLMS( void );
ui08	set_associated(cpui08 xdlms_info);
void	script_associate(void);
void	push_associate(void);

void	createCall(ui08 *out);

extern uint		apdu_size;
extern pui08	g_xdlms_io;
extern cui08	mechanism_name[][LEN_CONTEXT_NAME + 1];
extern CONST	UNIT_SCAL unit_scale[512];

/**********************/
#endif /* _XDLMS_H_*/
