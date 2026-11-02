/****************************************************************************/
/*																			*/
/* 	システム名			:	海外(タイ)向け計器							*/
/*																			*/
/*	サブシステム名		:	F5L										*/
/*																			*/
/*	モジュール名		:	共通											*/
/*						:	<ヘッダー宣言>									*/
/*	ファイル名			:	data.h											*/
/*																			*/
/*	機能				:	ｸﾞﾛｰﾊﾞﾙ変数 typedef宣言							*/
/*																			*/
/*																			*/
/****************************************************************************/
#ifndef _DATA_H_
#define _DATA_H_

/*  拡張機能宣言			*/

/*  ﾕｰｻﾞｰ･ｲﾝｸﾙｰﾄﾞ･ﾌｧｲﾙ定義  */
/* ----- RAM保持ﾃﾞｰﾀ数用ｼﾝﾎﾞﾙ -----										*/
#define		RAMCNT2			0x02			/* 通信ﾛｸﾞ(4件)				*/
#define		RAMCNT4			0x04			/* 通信ﾛｸﾞ(4件)				*/
#define		RAMCNT7			0x07			/* 24時間統計ﾃﾞｰﾀ(7件)		*/
#define		RAMCNT8			0x08
#define		RAMCNT10		0x0A			/* 開閉動作履歴(10件)		*/
#define		ONETIME			0x0100
//#define		ONETIME			0x0080

#define		RMSK2			0x0001
#define		RMSK4			0x0003
#define		RMSK8			0x0007
#define		RECORD			22
#define		ADCMAX			12

typedef enum	tgRun {
	eDef,
	eRestart,
	eFlash,
	eFlashR,
	eFlashC,
	eTest,
	eTestR,
	eTestC,
	eMemReset,
}eRun;

typedef enum	tgCOMID {
	eGPRS = 1,
	ePORT,
	ePush1a = 10,
	ePush1b,
	ePush1c,
	ePush2a = 20,
	ePush2b,
	ePush2c,
	ePush3a = 30,
	ePush3b,
	ePush3c,
	ePush4a = 40,
	ePush4b,
	ePush4c,
	ePush5a = 50,
	ePush5b,
	ePush5c,
	ePush6a = 60,
	ePush6b,
	ePush6c,
	ePush7a = 70,
	ePush7b,
	ePush7c,
	ePush8a = 80,
	ePush8b,
	ePush8c,
	eCID1 = 101,				/* Serial */
	eCID2,						/* Max PDU */
	eCID3,						/* IMEI */
	eCID4,						/* SIM ICCID */
	eCID5,						/* SIM information */
	eCID6,						/* SIM IPV6 address */
	eCID7,						/* NIC year */
	eCID8,						/* Version */
	eCID9,						/*  */
	eCID10,						/* Bluetooth */
}eCOMID;

typedef enum	tgelogs		/*0xaa=170はEEPROM「データ」の初期値のため使用不可*/
{
	LOG_ACCESS_TBL,
	LOG_ALERT_IP,
	LOG_ALM_ACT,
	LOG_ALM_OFF,
	LOG_AMPA,
	LOG_AMPA_C,
	LOG_AMPA_R,
	LOG_AMPB,
	LOG_AMPB_C,
	LOG_AMPB_R,
	LOG_AMPC,
	LOG_AMPC_C,
	LOG_AMPC_R,
	LOG_AMPD,
	LOG_AMPD_C,
	LOG_AMPD_R,
	LOG_ARM,
	LOG_ATDD,
	LOG_ATDD_C,
	LOG_ATDD_R,
	LOG_ATHD,
	LOG_ATHD_C,
	LOG_ATHD_R,
	LOG_BATT_STS,
	LOG_BILL_CNT,
	LOG_BILL_CNT_C,
	LOG_BILL_CNT_R,
	LOG_BILL_IP,
	LOG_CAL_ACT,
	LOG_CLIENT,
	LOG_CLK_CHG,
	LOG_CLKINV,
	LOG_CLKINV_C,
	LOG_CLKINV_R,
	LOG_CMD,
	LOG_COVER,
	LOG_COVER_C,
	LOG_COVER_R,
	LOG_DAILY_IP,
	LOG_DEFSET,
	LOG_DISCON,
	LOG_DISCON_C,
	LOG_DISCON_R,
	LOG_DIST,
	LOG_DIST_C,
	LOG_DIST_R,
	LOG_DMDP_CHG,
	LOG_ESD,
	LOG_ESD_C,
	LOG_ESD_R,
	LOG_ESW_CHG,
	LOG_FLASH,
	LOG_GPRS,
	LOG_HLS_CHG,
	LOG_INOUT,
	LOG_INOUT_C,
	LOG_INOUT_R,
	LOG_JMMR,
	LOG_JMMR_C,
	LOG_JMMR_R,
	LOG_KEY_CHG,
	LOG_KIDO,
	LOG_LIMITER,
	LOG_LIMITER_C,
	LOG_LIMITER_G,
	LOG_LIMITER_R,
	LOG_LLS_CHG,
	LOG_LMT_DIS,
	LOG_LMT_ENA,
	LOG_LMT_SET,
	LOG_LOAD_IP,
	LOG_LPP_CHG,
	LOG_LRC_CLOSE,
	LOG_LRC_ENABLE,
	LOG_MAG,
	LOG_MAG_C,
	LOG_MAG_R,
	LOG_MELT,
	LOG_MELT_C,
	LOG_MELT_R,
	LOG_MEMORY,
	LOG_METER,
	LOG_MISSV,
	LOG_MISSV_C,
	LOG_MISSV_R,
	LOG_MODE0,
	LOG_MODE4,
	LOG_MTR_CHG,
	LOG_NIC,
	LOG_NIC_C,
	LOG_NIC_R,
	LOG_OBJMAP,
	LOG_ONM,
	LOG_OVER,
	LOG_OVER_C,
	LOG_OVER_G,
	LOG_OVER_R,
	LOG_PAY_CHG,
	LOG_PERIOD_IP,
	LOG_PFLO,
	LOG_PFLO_C,
	LOG_PFLO_R,
	LOG_PUSH,
	LOG_PWOF,
	LOG_PWOFR,
	LOG_RAND_DELY,
	LOG_REC_RST,
	LOG_RESET,
	LOG_RLY_CFG,
	LOG_RTC,
	LOG_RTMP,
	LOG_RTMP_C,
	LOG_RTMP_R,
	LOG_RTMPL,
	LOG_RTMPLC,
	LOG_RTMPLR,
	LOG_SAS_CHG,
	LOG_SELF_IP,
	LOG_SIGN,
	LOG_SUPER,
	LOG_TAMP_IP,
	LOG_TEI,
	LOG_TEIR,
	LOG_TEMP,
	LOG_TEMP_C,
	LOG_TEMP_R,
	LOG_TEMPL,
	LOG_TEMPLC,
	LOG_TEMPLR,
	LOG_TOU_DAY,
	LOG_TOU_RATE,
	LOG_TOU_SPD,
	LOG_TOU_TIME,
	LOG_TOU_WEEK,
	LOG_TSNOPC,
	LOG_TSNOPC_C,
	LOG_TSNOPC_R,
	LOG_USERSET,
	LOG_VA_CFG,
	LOG_VLTB,
	LOG_VLTB_C,
	LOG_VLTB_R,
	LOG_VLTH,
	LOG_VLTH_C,
	LOG_VLTH_R,
	LOG_VLTL,
	LOG_VLTL_C,
	LOG_VLTL_R,
	LOG_VTHD,
	LOG_VTHD_C,
	LOG_VTHD_R,
	LOG_WRONG,
	LOG_WRONG_C,
	LOG_WRONG_R,
	ETSN1,
	ETSN2,
	ELOG_CPU,
	ELOG_CPUC,
	ELOG_CPUR,
	ELOG_EEP,
	ELOG_EEPC,
	ELOG_EEPR,
	ELOG_LCD,
	ELOG_LCDC,
	ELOG_LCDR,
	ELOG_MAG,
	ELOG_MAGC,
	ELOG_MAGR,
	ELOG_RESET,
	ELOG_RESETC,
	ELOG_RESETR,
	ELOG_RTC,
	ELOG_RTCC,
	ELOG_RTCR,
	ELOG_TSN,
	ELOG_TSNC,
	ELOG_TSNR,
	ELOG_VAR,
	ELOG_VARC,
	ELOG_VARR,
	ELOG_VBAT,
	ELOG_VBAT_C,
	ELOG_VBAT_R,
	ELOG_VI,
	ELOG_VIC,
	ELOG_VIR,
	ELOG_VRTC,
	ELOG_VRTC_C,
	ELOG_VRTC_R,
	ELOG_WAT,
	ELOG_WATC,
	ELOG_WATR,
	LOGKOMOKUMAX,/*LOG項目数*/
}elogs;

typedef enum tgeUnit {
	e_invalid, /*   */
	e_year, /*a  year  time  */
	e_month, /*mo  month  time  */
	e_week, /*wk  week  time  */
	e_day, /*d  day  time  */
	e_hour, /*h  hour  time  */
	e_minute, /*min  minute  time  */
	e_second, /*s  second  time (t)  */
	e_degree, /*°  degree  (phase) angle  */
	e_degree_celsius, /*°C  degree-celsius  temperature (T)  */
	e_currency, /*currency    (local) currency  */
	e_not_use11, /*m  metre  length (l)  */
	e_not_use12, /*m/s  metre per second  speed (v)  */
	e_not_use13, /*m3  cubic metre  volume (V) rV , meter constant or pulse value (volume)  */
	e_not_use14, /*m3  cubic metre  corrected volume a  */
	e_not_use15, /*m3/h  cubic metre per hour  volume flux  */
	e_not_use16, /*m3/h  cubic metre per hour  corrected volume flux a  */
	e_not_use17, /*m3/d  cubic metre per day  volume flux  */
	e_not_use18, /*m3/d  cubic metre per day  corrected volume flux a  */
	e_not_use19, /*l  litre  volume  */
	e_not_use20, /*kg  kilogram  mass (m)  */
	e_not_use21, /*N  newton  force (F)  */
	e_not_use22, /*Nm  newton meter  energy   */
	e_not_use23, /*Pa  pascal  pressure (p)  */
	e_not_use24, /*bar  bar  pressure (p)  */
	e_not_use25, /*J  joule  energy   */
	e_not_use26, /*J/h  joule per hour  thermal power, rate of change  */
	e_watt, /*W  watt  active power (P)  */
	e_volt_ampere, /*VA  volt-ampere  apparent power (S)  */
	e_var, /*var  var  reactive power (Q)  */
	e_watt_hour, /*Wh  watt-hour  active energy.rW , active energy meter constant or pulse value  */
	e_volt_ampere_hour, /*VAh  volt-ampere-hour  apparent energy rS , apparent energy meter constant or pulse value  */
	e_var_hour, /*varh  var-hour  reactive energy rB , reactive energy meter constant or pulse value  */
	e_ampere, /*A  ampere  current (I)  */
	e_not_use34, /*C  coulomb  electrical charge (Q)  */
	e_volt, /*V  volt  voltage (U)  */
	e_not_use36, /*V/m  volt per metre  electric field strength (E)  */
	e_not_use37, /*F  farad  capacitance (C)  */
	e_not_use38, /*?  ohm  resistance (R)  */
	e_not_use39, /*?m2/m    resistivity (ρ)  */
	e_not_use40, /*Wb  weber  magnetic flux (Φ)  */
	e_not_use41, /*T  tesla  magnetic flux density (B)  */
	e_not_use42, /*A/m  ampere per metre  magnetic field strength (H)  */
	e_not_use43, /*H  henry  inductance (L)  */
	e_hertz, /*Hz  hertz  frequency (f, ω)  */
	e_not_use45, /*1/(Wh)    RW , active energy meter constant or pulse value  */
	e_not_use46, /*1/(varh)    RB , reactive energy meter constant or pulse value  */
	e_not_use47, /*1/(VAh)    RS , apparent energy meter constant or pulse value  */
	e_not_use48, /*V2h  volt-squared-hours  volt-squared hour, rU2h , volt-squared hour meter constant or pulse value  */
	e_not_use49, /*A2h  ampere-squaredhours  ampere-squared hour, rI2h ,  ampere-squared hour meter constant or pulse value  */
	e_not_use50, /*kg/s  kilogram per second  mass flux  */
	e_not_use51, /*S, mho  siemens  conductance  */
	e_not_use52, /*K  kelvin  temperature (T)  */
	e_not_use53, /*1/(V2h)    RU2h , volt-squared hour meter constant or pulse value  */
	e_not_use54, /*1/(A2h)    RI2h , ampere-squared hour meter constant or pulse value  */
	e_not_use55, /*1/m3    RV , meter constant or pulse value (volume)   */
	e_percent, /*  %  percentage  */
	e_not_use57, /*Ah  Ampere-hour  ampere-hours  */
	e_not_use58, /*  reserved  reserved  */
	e_not_use59, /* reserved   */
	e_not_use60, /*Wh/m3  3,6*103 J/m3  energy per volume  */
	e_not_use61, /*J/m3    calorific value, wobbe  */
	e_not_use62, /*Mol %  mole percent  molar fraction of gas composition  */
	e_not_use63, /*g/m3    mass density, quantity of material  */
	e_not_use64, /*Pa s  pascal second  dynamic viscosity  */
	e_not_use65, /*J/kg  Joule / kilogram  specific energy NOTE The amount of energy per unit of mass of a substance  */
	e_not_use66, /*g/cm2  gram per square centimeter  pressure  */
	e_not_use67, /*atm  atmosphere  pressure  */
	e_not_use68, /*  reserved  reserved  */
	e_not_use69, /* reserved   */
	e_not_use70, /*dBm    signal strength, dB milliwatt (e.g. of GSM radio systems)  */
	e_not_use71, /*dbμV    signal strength, dB microvolt  */
	e_not_use72, /*dB    logarithmic unit that expresses the ratio between two values of a physical quantity  */
	e_not_use73, /*    reserved  */
	e_not_use74, /*  Non ? SI Units  */
	e_not_use75, /*   */
	e_not_use76, /*   */
	e_not_use77, /*   */
	e_not_use78, /*   */
	e_not_use79, /*   */
	e_not_use80, /*   */
	e_not_use81, /*   */
	e_not_use82, /*   */
	e_not_use83, /*   */
	e_not_use84, /*   */
	e_not_use85, /*   */
	e_not_use86, /*   */
	e_not_use87, /*   */
	e_not_use88, /*   */
	e_not_use89, /*   */
	e_not_use90, /*   */
	e_not_use91, /*   */
	e_not_use92, /*   */
	e_not_use93, /*   */
	e_not_use94, /*   */
	e_not_use95, /*   */
	e_not_use96, /*   */
	e_not_use97, /*   */
	e_not_use98, /*   */
	e_not_use99, /*   */
	e_not_use100, /*   */
	e_not_use101, /*   */
	e_not_use102, /*   */
	e_not_use103, /*   */
	e_not_use104, /*   */
	e_not_use105, /*   */
	e_not_use106, /*   */
	e_not_use107, /*   */
	e_not_use108, /*   */
	e_not_use109, /*   */
	e_not_use110, /*   */
	e_not_use111, /*   */
	e_not_use112, /*   */
	e_not_use113, /*   */
	e_not_use114, /*   */
	e_not_use115, /*   */
	e_not_use116, /*   */
	e_not_use117, /*   */
	e_not_use118, /*   */
	e_not_use119, /*   */
	e_not_use120, /*   */
	e_not_use121, /*   */
	e_not_use122, /*   */
	e_not_use123, /*   */
	e_not_use124, /*   */
	e_not_use125, /*   */
	e_not_use126, /*   */
	e_not_use127, /*   */
	e_not_use128, /*in inch  length (l)  */
	e_not_use129, /*ft  foot  length (l)  */
	e_not_use130, /*lb  pound  mass (m)  */
	e_not_use131, /*°F  degree Fahrenheit  temperature  */
	e_not_use132, /*°R  degree Rankine  temperature  */
	e_not_use133, /*sq. in  square inch  area   */
	e_not_use134, /*sq ft  square foot  area   */
	e_not_use135, /*ac  acre  area  */
	e_not_use136, /*cu in  cubic inch  volume  */
	e_not_use137, /*cu ft  cubic foot  volume  */
	e_not_use138, /*ac ft  acre-foot  volume  */
	e_not_use139, /*gal (imp)  gallon (imperial)  volume  */
	e_not_use140, /*gal (US)  gallon (US)  volume  */
	e_not_use141, /*lbf  pound force  force  */
	e_not_use142, /*psi  Pound force per square inch  pressure (p)  */
	e_not_use143, /*lb/cu ft  pound per cubic foot  density  */
	e_not_use144, /*lb/(ft .s)  pound per (foot . second)  dynamic viscosity  */
	e_not_use145, /*sq ft/s  square foot per second  kinematic viscosity  */
	e_not_use146, /*Btu  British thermal unit  energy   */
	e_not_use147, /*thm(EC)  Therm(EU)  energy  */
	e_not_use148, /*thm(US)  Therm(US)  energy  */
	e_not_use149, /*Btu/lb  British thermal unit per pound  calorific value of mass, enthalpy  */
	e_not_use150, /*Btu/cu ft  British thermal unit per cubic foot  calorific value of volume, wobbe  */
	e_not_use151, /*cu ft  cubic feet  volume (V) rv, meter constant or pulse value (volume)  */
	e_not_use152, /*ft/s  foot per second  speed (v)  */
	e_not_use153, /*cu ft/s  cubic foot per second  volume flux  */
	e_not_use154, /*cu ft/min  cubic foot per min  volume flux  */
	e_not_use155, /*cu ft/h  cubic foot per hour  volume flux  */
	e_not_use156, /*cu ft/d  cubic foot per day  volume flux  */
	e_not_use157, /*ac ft/s  acre foot per second  volume flux  */
	e_not_use158, /*ac ft/min  acre foot per min  volume flux  */
	e_not_use159, /*ac ft/h  acre foot per hour  volume flux  */
	e_not_use160, /*ac ft/d  acre foot per day  volume flux  */
	e_not_use161, /*gal (imp)  imperial gallon  volume (V) rv, meter constant or pulse value (volume)  */
	e_not_use162, /*gal (imp)/ s   imperial gallon per second  volume flux  */
	e_not_use163, /*gal (imp) / min  imperial gallon per min  volume flux  */
	e_not_use164, /*gal (imp) / h  imperial gallon per hour  volume flux  */
	e_not_use165, /*gal (imp) / d  imperial gallon per day  volume flux  */
	e_not_use166, /*gal (US)  US gallon  volume (V) rv, meter constant or pulse value (volume)  */
	e_not_use167, /*gal (US) / s US gallon per second  volume flux  */
	e_not_use168, /*gal (US) / min  US gallon per min  volume flux  */
	e_not_use169, /*gal (US) / h  US gallon per hour  volume flux  */
	e_not_use170, /*gal (US) / d  US gallon per day  volume flux  */
	e_not_use171, /*Btu/s  British thermal unit per second  energy flow, heat, power, change rate   */
	e_not_use172, /*Btu/min  British thermal unit per minute  energy flow, heat, power, change rate  */
	e_not_use173, /*Btu/h  British thermal unit per hour  energy flow, heat, power, change rate  */
	e_not_use174, /*Btu/d  British thermal unit per day  energy flow, heat, power, change rate  */
	e_not_use175, /*   */
	e_not_use176, /*   */
	e_not_use177, /*   */
	e_not_use178, /*   */
	e_not_use179, /*   */
	e_not_use180, /*   */
	e_not_use181, /*   */
	e_not_use182, /*   */
	e_not_use183, /*   */
	e_not_use184, /*   */
	e_not_use185, /*   */
	e_not_use186, /*   */
	e_not_use187, /*   */
	e_not_use188, /*   */
	e_not_use189, /*   */
	e_not_use190, /*   */
	e_not_use191, /*   */
	e_not_use192, /*   */
	e_not_use193, /*   */
	e_not_use194, /*   */
	e_not_use195, /*   */
	e_not_use196, /*   */
	e_not_use197, /*   */
	e_not_use198, /*   */
	e_not_use199, /*   */
	e_date_time,/*   */
	e_date6, /*   */
	e_time6, /*   */
	e_date4, /*   */
	e_time4, /*   */
	e_rupy, /*   */
	e_not_use206, /*   */
	e_not_use207, /*   */
	e_not_use208, /*   */
	e_not_use209, /*   */
	e_not_use210, /*   */
	e_not_use211, /*   */
	e_not_use212, /*   */
	e_not_use213, /*   */
	e_not_use214, /*   */
	e_not_use215, /*   */
	e_not_use216, /*   */
	e_not_use217, /*   */
	e_not_use218, /*   */
	e_not_use219, /*   */
	e_not_use220, /*   */
	e_not_use221, /*   */
	e_not_use222, /*   */
	e_not_use223, /*   */
	e_not_use224, /*   */
	e_not_use225, /*   */
	e_not_use226, /*   */
	e_not_use227, /*   */
	e_not_use228, /*   */
	e_not_use229, /*   */
	e_not_use230, /*   */
	e_not_use231, /*   */
	e_not_use232, /*   */
	e_not_use233, /*   */
	e_not_use234, /*   */
	e_not_use235, /*   */
	e_not_use236, /*   */
	e_not_use237, /*   */
	e_not_use238, /*   */
	e_not_use239, /*   */
	e_not_use240, /*   */
	e_not_use241, /*   */
	e_not_use242, /*   */
	e_not_use243, /*   */
	e_not_use244, /*   */
	e_not_use245, /*   */
	e_not_use246, /*   */
	e_not_use247, /*   */
	e_not_use248, /*   */
	e_not_use249, /*   */
	e_not_use250, /*   */
	e_not_use251, /*   */
	e_extended, /*extended table of units    */
	e_not_use253, /*    extended table of units  */
	e_other, /*other    other unit  */
	e_count, /*count    no unit, unitless, count  */
}eUNIT;


typedef enum	tCOUNTER {		/*0xaa=170はEEPROM「データ」の初期値のため使用不可*/
	eNone = 0xff,
	eBill = 0,
	eTamp,
	eTampMonth,
	eProg,
	ePwFail,
	eEvent,
	eMagnet,
	eESD,
	eJMMR,
	eVTHD,
	eATHD,
	eATDD,
	eNICCover,
	eRelay,
	eDiscon,
	eConnect,
	eLCD,
	eCpuReset,
	eMeter,
	eEEP,
	eRTC,
}COUNTER;

typedef enum tgIMSTS {
	transfer_not_initiated = 0,
	transfer_initiated,
	verification_initiated,
	verification_successful,
	verification_failed,
	activation_initiated,
	activation_successful,
	activation_failed
}IMSTS;

/*
7 over voltage - occurrence
8 over voltage - restoration
9  low voltage - occurrence
10 low voltage - restoration

A5 Curent related event - tamper count
51 current reverse - occurrence
52 current reverse - restoration
67 over current in any phase - occurrence
68 over current in any phase - restoration
69 Earth loading Occurrence
70 Earth loading Restoration

A6 Power related event
101 Power failure occurrence
102 Power failure restoration

A7 Transaction event
151 Real time clock - date and time
152 Demand integration period
153 Profile capture period
154 Single action schedule for billing dates
155 Activity calendar for time zones
157 New firmware activated
158 Load limit set
159 Enable - load limit fuction
160 Disable - load limit function
161 LLS secret(MR) change
162 HLS key(US) change
163 HLS key(FW) change
164 Global key change(Encryption and authentication)
165 ESWF change
166 MD reset

A8 Other event  - tamper count
201 Abnormal external magnetic influence - Occurrence
202 Abnormal external magnetic influence - restoration
203 Neutral disturbance - Occurrence
204 Neutral disturbance - restoration
207 Single wire operation(neutral missing) - occurrence
208 Single wire operation(neutral missing) - restoration
209 Plug in communication module removal - occurrence
210 plug in communication module removal - restoration
211 Configuration change to post-paid mode
212 Configuration change to pre-paid mode
213 configuration change to fowarded only mode
214 configuration change to import and export mode
215 overload - occurrence
216 overload - restoration

A9 Non roll over event
251 Meter cover openning - occurrence

A10 Control event
301 load switch status - disconnected
302 load switch status - connected

*/

#define PQ_NO_CODE 0
#define PQ_OVER_V1 (PQ_NO_CODE+1)				/*1*/
#define PQ_OVER_V1R (PQ_OVER_V1+1)				/*2*/
#define PQ_UNDER_V1 (PQ_OVER_V1R+1)				/*3*/
#define PQ_UNDER_V1R (PQ_UNDER_V1+1)			/*4*/
#define PQ_REVERSAL (PQ_UNDER_V1R+1)			/*5*/
#define PQ_REVERSALR (PQ_REVERSAL+1)			/*6*/
#define PQ_OVERCURRENT (PQ_REVERSALR+1)			/*7*/
#define PQ_OVERCURRENTR (PQ_OVERCURRENT+1)		/*8*/
#define PQ_EARTH_LOAD (PQ_OVERCURRENTR+1)		/*9*/
#define PQ_EARTH_LOADR (PQ_EARTH_LOAD+1)		/*10*/
#define PQ_POWER_DOWN (PQ_EARTH_LOADR+1)		/*11*/
#define PQ_POWER_UP (PQ_POWER_DOWN+1)			/*12*/
#define PQ_CLOCK0 (PQ_POWER_UP+1)				/*13*/
#define PQ_DEMAND_PERIOD (PQ_CLOCK0+1)			/*14*/
#define PQ_PROFILE_PERIOD (PQ_DEMAND_PERIOD+1)	/*15*/
#define PQ_SGL_BILL (PQ_PROFILE_PERIOD+1)		/*16*/
#define PQ_SGL_INST (PQ_SGL_BILL+1)				/*17*/
#define PQ_SGL_IMAGE (PQ_SGL_INST+1)			/*18*/
#define PQ_ACAL_ZONE (PQ_SGL_IMAGE+1)			/*19*/
#define PQ_FIRM_ACTIVATE (PQ_ACAL_ZONE+1)		/*20*/
#define PQ_LOAD_LIMIT (PQ_FIRM_ACTIVATE+1)		/*21*/
#define PQ_LIMIT_ENABLE (PQ_LOAD_LIMIT+1)		/*22*/
#define PQ_LIMIT_DISABLE (PQ_LIMIT_ENABLE+1)	/*23*/
#define PQ_LLS_MR (PQ_LIMIT_DISABLE+1)			/*24*/
#define PQ_HLS_US (PQ_LLS_MR+1)					/*25*/
#define PQ_HLS_FW (PQ_HLS_US+1)					/*26*/
#define PQ_GLOBAL (PQ_HLS_FW+1)					/*27*/
#define PQ_ESWF (PQ_GLOBAL+1)					/*28*/
#define PQ_MAXDMD_RESET (PQ_ESWF+1)				/*29*/
#define PQ_RELAY_CFG (PQ_MAXDMD_RESET+1)		/*30*/
#define PQ_VA_LEAD_LAG (PQ_RELAY_CFG+1)			/*31*/
#define PQ_VA_LAG_ONLY (PQ_VA_LEAD_LAG+1)		/*32*/
#define PQ_MAGNET (PQ_VA_LAG_ONLY+1)			/*33*/
#define PQ_MAGNETR (PQ_MAGNET+1)				/*34*/
#define PQ_DISTURB (PQ_MAGNETR+1)				/*35*/
#define PQ_DISTURBR (PQ_DISTURB+1)				/*36*/
#define PQ_NEUTRAL (PQ_DISTURBR+1)				/*37*/
#define PQ_NEUTRALR (PQ_NEUTRAL+1)				/*38*/
#define PQ_PLUGIN (PQ_NEUTRALR+1)				/*39*/
#define PQ_PLUGINR (PQ_PLUGIN+1)				/*40*/
#define PQ_POST_PAID (PQ_PLUGINR+1)				/*41*/
#define PQ_PRE_PAID (PQ_POST_PAID+1)			/*42*/
#define PQ_FOWARED_MODE (PQ_PRE_PAID+1)			/*43*/
#define PQ_IMP_EXP_MODE (PQ_FOWARED_MODE+1)		/*44*/
#define PQ_OVERLOAD (PQ_IMP_EXP_MODE+1)			/*45*/
#define PQ_OVERLOADR (PQ_OVERLOAD+1)			/*46*/
#define PQ_COVER (PQ_OVERLOADR+1)				/*47*/
#define PQ_LOAD_DISCON (PQ_COVER+1)				/*48*/
#define PQ_LOAD_CON (PQ_LOAD_DISCON+1)			/*49*/
#define PQ_UNEXPECTED_RESET (PQ_LOAD_CON+1)		/*50*/
#define PQ_NIC_COVER (PQ_UNEXPECTED_RESET+1)	/*51*/
#define PQ_NIC_COVERR (PQ_NIC_COVER+1)			/*52*/
#define PQ_CUR_MIS (PQ_NIC_COVERR+1)			/*53*/
#define PQ_CURRENT_MISMATCHR (PQ_CUR_MIS+1)		/*54*/
#define PQ_TEMPERTURE (PQ_CURRENT_MISMATCHR+1)	/*55*/
#define PQ_TEMPERTURER (PQ_TEMPERTURE+1)		/*56*/
#define PQ_RELAY_TEMP (PQ_TEMPERTURER+1)		/*57*/
#define PQ_RELAY_TEMPR (PQ_RELAY_TEMP+1)		/*58*/
#define PQ_ESD (PQ_RELAY_TEMPR+1)				/*59*/
#define PQ_ESDR (PQ_ESD+1)						/*60*/
#define PQ_JAMMER (PQ_ESDR+1)					/*61*/
#define PQ_JAMMERR (PQ_JAMMER+1)				/*62*/
#define PQ_MELT (PQ_JAMMERR+1)					/*63*/
#define PQ_MELTR (PQ_MELT+1)					/*64*/
#define PQ_VTHD (PQ_MELTR+1)					/*65*/
#define PQ_VTHDR (PQ_VTHD+1)					/*66*/
#define PQ_ATHD (PQ_VTHDR+1)					/*67*/
#define PQ_ATHDR (PQ_ATHD+1)					/*68*/
#define PQ_ATDD (PQ_ATHDR+1)					/*69*/
#define PQ_ATDDR (PQ_ATDD+1)					/*70*/
#define PQ_LWPF (PQ_ATDDR+1)					/*71*/
#define PQ_LWPFR (PQ_LWPF+1)					/*72*/
#define PQ_PERIOD (PQ_LWPFR+1)					/*73 	817,Instant Push Destination IP Changed*/
#define PQ_ALERT (PQ_PERIOD+1)					/*74 	819, 	Alert Push Destination IP Changed*/
#define PQ_SGL_SELF (PQ_ALERT+1)				/*75 	823, 	Two way push Action schedule Changed*/
#define PQ_SGL_LOAD (PQ_SGL_SELF+1)				/*76 	846, 	Block Load Profile Push schedule change*/
#define PQ_SGL_DAILY (PQ_SGL_LOAD+1)			/*77 	845, 	Daily Load Profile Push schedule change*/
#define PQ_SGL_TAMP (PQ_SGL_DAILY+1)			/*78 	808, 	Tamper data Push schedule change*/
#define PQ_TAMP_IP (PQ_SGL_TAMP+1)				/*79 	811, 	Tamper Push Destination IP Changed*/
#define PQ_SELF_IP (PQ_TAMP_IP+1)				/*80 	818, 	Two Way push Destination IP Changed*/
#define PQ_BILL_IP (PQ_SELF_IP+1)				/*81 	820, 	Bill Push Destination IP Changed*/
#define PQ_DAILY_IP (PQ_BILL_IP+1)				/*82 	821, 	Daily Energy Push Destination IP Changed*/
#define PQ_LOAD_IP (PQ_DAILY_IP+1)				/*83 	822, 	Load Survey Push Destination IP Changed*/
#define PQ_GPRS (PQ_LOAD_IP+1)					/*84 	805, 	APN configuration*/
#define PQ_BILL_RND (PQ_GPRS+1)					/*85 	945, 	Randomization Delay Interval Bill*/
#define PQ_DAILY_RND (PQ_BILL_RND+1)			/*86 	946, 	Randomization Delay Interval DE*/
#define PQ_LOAD_RND (PQ_DAILY_RND+1)			/*87 	947, 	Randomization Delay Interval LS*/
#define PQ_PERIOD_RND (PQ_LOAD_RND+1)			/*88 	948, 	Randomization Delay Interval Instant*/
#define PQ_LRC_ENABLE (PQ_PERIOD_RND+1)			/*89 	675, 	LRC enabled / disabled*/
#define PQ_LRC_CLOSE (PQ_LRC_ENABLE+1)			/*90 	674, 	LRCF triggered and relay closed*/
#define PQ_MODE0 (PQ_LRC_CLOSE+1)				/*91*/
#define PQ_MODE4 (PQ_MODE0+1)					/*92*/
#define PQ_LOWBAT (PQ_MODE4+1)					/*93 	679,*/

//#define	ESW_MISSING_VOLT1		0
//#define	ESW_MISSING_VOLT2		1
//#define	ESW_MISSING_VOLT3		2
#define	ESW_OVER_VOLTAGE		3	/*ook*/
#define	ESW_LOW_VOLTAGE			4	/*ook*/
//#define	ESW_VOLTAGE_UNBALANCE	5
//#define	ESW_CURRENT_REVERSAL1	6	/*ook*/
//#define	ESW_CURRENT_REVERSAL2	7
//#define	ESW_CURRENT_REVERSAL3	8
//#define	ESW_CURRENT_UNBALANCE	9
//#define	ESW_CURRENT_BYPASS		10
#define	ESW_OVER_CURRENT		11	/*ook*/
//#define	ESW_LOW_PF				12	/*ok*/
#define	ESW_EARTH_LOADING		51	/*ook*/
//#define	ESW_RELAY_WELD			79	/*ook*/
#define	ESW_MAGNET				81	/*ook*/
#define	ESW_NEUTRAL_DISTURB		82	/*ook*/
#define	ESW_METER_COVER			83	/*ook*/
#define ESW_METER_LOAD			84	/*ook*/
#define ESW_LAST_GASP			85	/*ook*/
#define ESW_FIRST_BREATH		86	/*ook*/
#define ESW_BILLING_COUNTER		87	/*ook*/
//#define ESW_NIC_REMOVAL			89	/*ok*/
//#define ESW_RTC_BAD				90	/*ok*/
//#define ESW_NVM_BAD				91	/*ok*/
//#define ESW_FIRMWARE_UPGRADE	93	/*ook*/
//#define ESW_TEMPERATURE_RIZE	116	/*ok*/
//#define ESW_CURRENT_MISMATCH	117	/*ok*/
#define ESW_LRC_CLOSE			118	/*ook*/
//#define ESW_BATT_LO				119 /*ok*/
//#define ESW_ESD					120	/*ok*/

//#define ESW_UNEXPECTED_RESET	100	/*G*/
/*==========================================================================*/
/* 多ﾊﾞｲﾄﾃﾞｰﾀ型																*/
/*==========================================================================*/
typedef	union	tagDAT2							/* ２ﾊﾞｲﾄ領域	*/
{
	ui16	hf;
	ui08	by[2];
} DAT2;
typedef	DAT2* PDAT2;
/*--------------------------------------------------------------------------*/
typedef	union	tagDAT4							/* ４ﾊﾞｲﾄ領域	*/
{
	ui32	wd;
	ui16	hf[2];
	ui08	by[4];
} DAT4;
typedef	DAT4* PDAT4;
/*--------------------------------------------------------------------------*/
typedef	union	tagDAT8							/* ８ﾊﾞｲﾄ領域	*/
{
	ui64	ll;
	ui32	wd[2];
	ui16	hf[4];
	ui08	by[8];
} DAT8;
typedef	DAT8* PDAT8;

typedef	union	tagDAT16						/* 12ﾊﾞｲﾄ領域	*/
{
	ui64	ll[2];
	ui32	wd[4];
	ui16	hf[8];
	ui08	by[16];
} DAT16;
typedef	DAT16* PDAT16;

typedef	union	tagFNCFLG
{
	struct {
		ui16 forward : 1;	/*BIT0*/
		ui16 pripaid : 1;	/*BIT1*/
		ui16 lite : 1;		/*BIT2*/
		ui16 laglead : 1;	/*BIT3*//*0:lag+lead, 1:lag*/
		ui16 reserve4 : 1;	/*BIT4*/
		ui16 reserve5 : 1;	/*BIT5*/
		ui16 reserve6 : 1;	/*BIT6*/
		ui16 reserve7 : 1;	/*BIT7*/
		ui16 reserve8 : 1;	/*BIT8*/
		ui16 current : 1;	/*BIT9*/
		ui16 first : 1;		/*BIT10*/
		ui16 quick : 1;		/*BIT11*/
		ui16 relay_off : 1;	/*BIT12*/
		ui16 bis : 1;		/*BIT13*/
		ui16 power_user : 1;/*BIT14*/
		ui16 non_module : 1;/*BIT15*/
	} bt;
	ui08	by[2];
	ui16	hf;
} FNFG;
typedef	FNFG* PFNFG;

/*--------------------------------------------------------------------------*/
typedef	struct	tagTIM32
{
	ui16	tim32H;
	ui16	tim32L;
} TIM32;
/*--------------------------------------------------------------------------*/

/* 内部拡張RAM																*/
/*==========================================================================*/
/* ｱﾌﾟﾘｹｰｼｮﾝ層　変数														*/
/*==========================================================================*/
typedef	struct	tagDS
{
	ui08	status;								/* 起動モード、デジタル封印、	*/
} DS;
typedef	DS* PDS;
/*--------------------------------------------------------------------------*/
typedef	struct	tagDTTM
{
	ui08	byo;								/* 秒ﾃﾞｰﾀ		(BCD: 0x00 ~ 0x59)		*/
	ui08	fun;								/* 分ﾃﾞｰﾀ		(BCD: 0x00 ~ 0x59)		*/
	ui08	ji;									/* 時ﾃﾞｰﾀ		(BCD: 0x00 ~ 0x23)		*/
	ui08	yoo;								/* 曜日ﾃﾞｰﾀ		(BCD: 0x00固定)			*/
	ui08	hi;									/* 日ﾃﾞｰﾀ		(BCD: 0x01 ~ 0x31)		*/
	ui08	gatu;								/* 月ﾃﾞｰﾀ		(BCD: 0x01 ~ 0x12)		*/
	ui08	nen;								/* 年下位ﾃﾞｰﾀ	(BCD: 0x00 ~ 0x99)		*/
	ui08	dmy;
} DTTM;
typedef	DTTM* PDTTM;

typedef	struct	tagTM {
	ui08	byo;								/* 秒ﾃﾞｰﾀ		(BCD: 0x00 ~ 0x59)		*/
	ui08	fun;								/* 分ﾃﾞｰﾀ		(BCD: 0x00 ~ 0x59)		*/
	ui08	ji;									/* 時ﾃﾞｰﾀ		(BCD: 0x00 ~ 0x23)		*/
}TM;
typedef	TM* PTM;

typedef	union	tagYOGA
{
	struct {
		ui08 gatu : 5;
		ui08 yoo : 3;
	} bt;
	ui08	by;
} YOGA;

typedef	struct	tagDT3 {
	ui08	hi;									/* 日ﾃﾞｰﾀ		(BCD: 0x01 ~ 0x31)		*/
	YOGA	yoga;								/* 月ﾃﾞｰﾀ		(BCD: 0x01 ~ 0x12)		*/
	ui08	nen;								/* 年下位ﾃﾞｰﾀ	(BCD: 0x00 ~ 0x99)		*/
}DT3;

typedef	DT3* PDT3;
typedef	struct	tagDT4 {
	ui08	yoo;								/* 曜日ﾃﾞｰﾀ		(BCD: 0x00固定)			*/
	ui08	hi;									/* 日ﾃﾞｰﾀ		(BCD: 0x01 ~ 0x31)		*/
	ui08	gatu;								/* 月ﾃﾞｰﾀ		(BCD: 0x01 ~ 0x12)		*/
	ui08	nen;								/* 年下位ﾃﾞｰﾀ	(BCD: 0x00 ~ 0x99)		*/
}DT4;

typedef struct	tagDATETIME {
	ui08	year_h;								/* 年上位ﾃﾞｰﾀ	(HEX: 0x07 ~ 0x08)		*/
	ui08	year_l;								/* 年下位ﾃﾞｰﾀ	(HEX: 0xDA ~ 0x33)		*/
	ui08	mon;								/* 月ﾃﾞｰﾀ		(HEX: 0x01 ~ 0x0C)		*/
	ui08	day;								/* 日ﾃﾞｰﾀ		(HEX: 0x01 ~ 0x1F)		*/
	ui08	week;								/* 週ﾃﾞｰﾀ		(HEX: 0xFF固定)			*/
	ui08	hour;								/* 時ﾃﾞｰﾀ		(HEX: 0x00 ~ 0x17)		*/
	ui08	min;								/* 分ﾃﾞｰﾀ		(HEX: 0x00 ~ 0x3B)		*/
	ui08	sec;								/* 秒ﾃﾞｰﾀ		(HEX: 0x00 ~ 0x3B)		*/
	ui08	msec;								/* 100分の秒ﾃﾞｰﾀ(HEX: 0xFF固定)			*/
	ui08	lag_h;								/* 時差上位ﾃﾞｰﾀ	(HEX: 0x80固定)			*/
	ui08	lag_l;								/* 時差下位ﾃﾞｰﾀ	(HEX: 0x00固定)			*/
	ui08	sts;
} DATETIME;
typedef DATETIME* PDATETIME;

typedef struct	tagDATE
{
	ui08	year_h;								/* 年上位ﾃﾞｰﾀ	(HEX: 0x07 ~ 0x08)		*/
	ui08	year_l;								/* 年下位ﾃﾞｰﾀ	(HEX: 0xDA ~ 0x33)		*/
	ui08	mon;								/* 月ﾃﾞｰﾀ		(HEX: 0x01 ~ 0x0C)		*/
	ui08	day;								/* 日ﾃﾞｰﾀ		(HEX: 0x01 ~ 0x1F)		*/
	ui08	week;								/* 週ﾃﾞｰﾀ		(HEX: 0xFF固定)			*/
} DATE;
typedef DATE* PDATE;

typedef	struct	tagTIME {
	ui08	hour;
	ui08	min;
	ui08	sec;
	ui08	handredths;
}TIME;
typedef	TIME* PTIME;

/*--------------------------------------------------------------------------*/
typedef	struct	tagLCD
{
	ui08	mode;								/* 画面ﾓｰﾄﾞ					*/
	ui08	pos;
	ui08	sub;
	ui08	focs;								/* 全点灯処理分割№			*/
} LCD;
typedef	LCD* PLCD;
/*--------------------------------------------------------------------------*/
typedef	struct tagSEGDAT
{
	ui08	by[20];
} SEGDAT;
typedef	SEGDAT* PSEGDAT;

typedef	struct	tagLCDDAT
{
	ui08	by[10];
	ui08	dmy;
} SUBSEGDAT;
typedef	SUBSEGDAT* PSUBSEGDAT;

/*--------------------------------------------------------------------------*/
typedef	struct	tagEEP
{
	ui16	req1;								/* ﾀｽｸ要求１				*/
	ui16	req2;								/* ﾀｽｸ要求２				*/
	ui16	req3;								/* ﾀｽｸ要求３				*/
	ui16	exc1;								/* ﾀｽｸ実行１				*/
	ui16	exc2;								/* ﾀｽｸ実行２				*/
	ui16	exc3;								/* ﾀｽｸ実行３				*/
	ui16	focs;								/* ﾀｽｸ内分割No			*/
	ui08	retry[7];							/* ダミー				*/
	ui08	tsk;								/* ﾀｽｸNo					*/
	ui08	sub;								/* ダミー				*/
	ui08	idx;								/* ダミー				*/
} EEP;
typedef	EEP* PEEP;
/*--------------------------------------------------------------------------*/
typedef	struct	tagKEICTL
{
	ui08	tsk;								/* 平均値算出用ﾀｽｸ番号		*/
	ui08	pos;								/* 最新実効値ﾃﾞｰﾀﾎﾟｲﾝﾀ		*/
	ui08	req;
	ui08	rsv;
} KEICTL;
typedef	KEICTL* PKEICTL;
/*--------------------------------------------------------------------------*/
typedef	struct	tagKEICOS
{
	ui16	dem1;								/* 分周値(分周比分)			*/
	ui08	bun;								/* 計量値更新分周値(1/2)	*/
	ui08	cnt;								/* 36分周ｶｳﾝﾀ				*/
} KEICOS;
typedef	KEICOS* PKEICOS;
/* ↓ '10-08-23 J.Matsuda 変更 ↓ */
typedef	struct	tagTESTCTL
{
	ui08	index;								/* ﾃｽﾄﾌﾟﾛｸﾞﾗﾑ項目Ｎo.		*/
	ui08	dat1;
	ui16	dat2;
} TESTCTL;
typedef	TESTCTL* PTESTCTL;

typedef	struct	tagTEST
{
	TESTCTL	lcd;
	TESTCTL	kei;
	TESTCTL	eep;
	TESTCTL	rtc;
	TESTCTL	key;
	TESTCTL	bat;
	TESTCTL	rly;
	TESTCTL	mag;
	ui08	sw[4];
	ui08	lcd_wr;								/* LCDﾄﾞﾗｲﾊﾞ 要求ﾌﾗｸﾞ		*/
	ui08	rsv;
} FTEST;
typedef	FTEST* PFTEST;

typedef	struct	tagTESTREQ
{
	ui08	req;
	ui08	ret;
} TESTFREQ;

/*--------------------------------------------------------------------------*/
typedef	struct	tgTHREAD
{
	ui08	task;
	ui08	focs;
	ui08	error;
	ui08	tmp;
} BASIC;
typedef	BASIC* PBASIC;

typedef	struct	tgRELAY
{
	ui08	output;
	ui08	req;
	ui08	order;
	ui08	mode;
	ui08	state;
	ui08	task;
	ui16	looping;
} RELAY;

typedef void(*LMMVAL)(DAT4* out, cui08 dot);

typedef struct tgRECMAP {
	ui32	addr;
	ui16	unit;
	ui32	size;
}RECMAP;

typedef struct tgRECPTR {
	ui32	rno;								/* 最新ﾎﾟｲﾝﾀ				*/
	ui16	recpt;							/* 最新ﾎﾟｲﾝﾀ				*/
	ui16	one;
	ui08	ring;								/* ﾚｺｰﾄﾞ一周ﾌﾗｸﾞ			*/
	ui08	active;							/*0:レコード状態、1:初期化完了、2:初期化中、3:不定(FLAG不正)*/
}RECPTR;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
typedef ui16(*DECTIM)(pui08 out, const DAT4* in);
typedef void(*DECDAT)(cpui08 in, cui32 start);
typedef	struct	tagRECSET
{
	RECPTR	__far* ptr;
	RECMAP 	__far* map;
	DECTIM	tim;
	DECDAT	__far* dat;
	ui16	cnt;
	ui16	max;
	ui16	sub;
	FNC		__far* fnc;
} RECSET;
typedef	RECSET* PRECSET;

typedef	struct	tagSACTL
{
	ui16	from;
	ui16	to;
	ui16	appable;
	ui16	eepcnt;
	ui08	ret;
	ui08	rsv;
} SACTL;
typedef	SACTL* PSACTL;

typedef	struct	tagSEARCHCTL {
	RECSET	cfg;
	DAT4	st;									/* 検索範囲開始位置				*/
	DAT4	ed;									/* 検索範囲終了位置				*/
	ui16	rec_code;							/* 検索対象ﾚｺｰﾄﾞ情報			*/
	ui16	rec_sel;							/* ﾚｺｰﾄﾞ検索方法情報			*/
	ui16	find;						/* 検索範囲内該当ﾚｺｰﾄﾞ数		*/
	ui16	read;							/* 読出し済み該当ﾚｺｰﾄﾞ数		*/
	ui16	entry;
	ui16	recpt;								/* 検索開始時の最新ﾎﾟｲﾝﾀ位置	*/
	ui16	recpt_next;							/* 次の検索開始ﾎﾟｲﾝﾀ位置		*/
	ui16	one;
	ui08	ring;
	ui08	idx;
} SRCHCTL;

typedef struct tgMEMRD {
	DAT4	adr;
	DAT4	siz;
	DAT4	cur;
	DAT2	stock;
	DAT2	upper;
}MEMRD;

typedef	struct	tagVI4
{
	DAT4	v1;							/* 1側電圧1秒実効値				*/
	DAT4	v2;							/* 1側電圧1秒実効値				*/
	DAT4	v3;							/* 3側電圧1秒実効値				*/
	DAT4	i1;							/* 1側電流1秒実効値				*/
	DAT4	i2;							/* 1側電流1秒実効値				*/
	DAT4	i3;							/* 2側電流1秒実効値				*/
} VI4;

typedef	struct	tagXYZ4
{
	DAT4	ac_x;
	DAT4	ac_y;
	DAT4	ac_z;
	DAT4	dc_x;
	DAT4	dc_y;
	DAT4	dc_z;
} JIKI;

typedef	struct	tagKW4
{
	DAT4	lft;
	DAT4	rgt;
	DAT4	imp;
	DAT4	exp;
	DAT4	fwd;
	DAT4	bak;
} KW4;

typedef KW4* PKW4;

typedef	struct	tagTOTAL
{
	ui32	samp;
	DAT4	val;
} TOTAL;

typedef	struct	tagDISPCHG
{
	ui08		power;
	ui08		mode;
	ui08		sel;
} DISPCHG;

/*--------------------------------------------------------------------------*/
typedef	struct	tagLSICTL
{
	ui08	quad;					/* 計量象限					*/
	ui08	coscrp;					/* COSｸﾘｰﾌﾟﾌﾗｸﾞ				*/
	ui08	sincrp;					/* SINｸﾘｰﾌﾟﾌﾗｸﾞ				*/
	ui08	cosdir;					/* COS潮流方向ﾌﾗｸﾞ			*/
	ui08	sindir;					/* SIN潮流方向ﾌﾗｸﾞ			*/
	ui08	modulo;
	ui16	pos;
	ui16	neg;
} LSICTL;
typedef	LSICTL* PLSICTL;

/*--------------------------------------------------------------------------*/
typedef	struct	tagHRDBUN
{
	ui08	enable;								/* ﾊﾟﾙｽ出力許可/禁止ﾌﾗｸﾞ	*/
	ui08	dosflg;								/* 動作ﾏｰｸﾌﾗｸﾞ				*/
	ui08	doscnt;								/* 動作ﾏｰｸ分周ｶｳﾝﾀ			*/
	ui08	plscnt;								/* 動作ﾏｰｸ分周ｶｳﾝﾀ			*/
	ui08	keicnt;								/* 計量36分周ｶｳﾝﾀ			*/
	ui08	kei;								/* 計量値更新回数ｶｳﾝﾀ		*/
} HRDBUN;
typedef	HRDBUN* PHRDBUN;
/*--------------------------------------------------------------------------------------*/
typedef	struct	tagVCTMP
{
	DAT4	vc_volt1w;									/* １側電圧実効値			*/
	DAT4	vc_volt2w;									/* １側電圧実効値			*/
	DAT4	vc_volt3w;									/* ３側電圧実効値			*/
	DAT4	vc_curr1w;									/* １側電流実効値			*/
	DAT4	vc_curr2w;									/* １側電流実効値			*/
	DAT4	vc_curr3w;									/* ３側電流実効値			*/
	ui08	vc_dir0w;									/* １側電流方向				*/
	ui08	vc_dir1w;									/* １側電流方向				*/
	ui08	vc_dir2w;									/* １側電流方向				*/
	ui08	vc_dir3w;									/* ３側電流方向				*/
} VCTMP;
typedef	VCTMP* PVCTMP;
/*--------------------------------------------------------------------------------------*/
typedef	struct	tagVIKAK
{
	DAT4	vc_volt1;									/* １側電圧実効値			*/
	DAT4	vc_volt2;									/* ２側電圧実効値			*/
	DAT4	vc_volt3;									/* ３側電圧実効値			*/

	DAT4	vc_curr1;									/* １側電流実効値			*/
	DAT4	vc_curr2;									/* ２側電流実効値			*/
	DAT4	vc_curr3;									/* ３側電流実効値			*/

	ui08	vc_dir0;									/* １側電流方向				*/
	ui08	vc_dir1;									/* １側電流方向				*/
	ui08	vc_dir2;									/* ２側電流方向				*/
	ui08	vc_dir3;									/* ３側電流方向				*/

	DAT8	vi_rd1v;									/* １側電圧二乗和積算今回値	*/
	DAT8	vi_rd2v;									/* ２側電圧二乗和積算今回値	*/
	DAT8	vi_rd3v;									/* ３側電圧二乗和積算今回値	*/

	DAT8	vi_rd1i;									/* １側電流二乗和積算今回値	*/
	DAT8	vi_rd2i;									/* ２側電流二乗和積算今回値	*/
	DAT8	vi_rd3i;									/* ３側電流二乗和積算今回値	*/

	DAT8	vi_rd1dir;									/* １側方向算出用電力量今回値	*/
	DAT8	vi_rd2dir;									/* ２側方向算出用電力量今回値	*/
	DAT8	vi_rd3dir;									/* ３側方向算出用電力量今回値	*/

	DAT8	vi_rd1vold;									/* １側電圧二乗和積算前回値	*/
	DAT8	vi_rd2vold;									/* ２側電圧二乗和積算前回値	*/
	DAT8	vi_rd3vold;									/* ３側電圧二乗和積算前回値	*/

	DAT8	vi_rd1iold;									/* １側電流二乗和積算前回値	*/
	DAT8	vi_rd2iold;									/* ２側電流二乗和積算前回値	*/
	DAT8	vi_rd3iold;									/* ３側電流二乗和積算前回値	*/

	DAT8	vi_rd1dirold;								/* １側方向算出用電力量前回値	*/
	DAT8	vi_rd2dirold;								/* ２側方向算出用電力量前回値	*/
	DAT8	vi_rd3dirold;								/* ３側方向算出用電力量前回値	*/
} VIKAK;
typedef	VIKAK* PVIKAK;
/*--------------------------------------------------------------------------------------*/
typedef	struct	tagHOSEI
{
	DAT4	el1;											/* １側軽負荷補正計数		*/
	ui16	kb1;											/* １側 ﾊﾞﾗﾝｽ調整ﾃﾞｰﾀ		*/
	ui16	vb1;											/* １側 ﾊﾞﾗﾝｽ調整ﾃﾞｰﾀ		*/

	DAT4	el2;											/* １側軽負荷補正計数		*/
	ui16	kb2;											/* １側 ﾊﾞﾗﾝｽ調整ﾃﾞｰﾀ		*/
	ui16	vb2;											/* １側 ﾊﾞﾗﾝｽ調整ﾃﾞｰﾀ		*/

	DAT4	el3;											/* ３側軽負荷補正計数		*/
	ui16	kb3;											/* ３側 ﾊﾞﾗﾝｽ調整ﾃﾞｰﾀ		*/
	ui16	vb3;											/* ３側 ﾊﾞﾗﾝｽ調整ﾃﾞｰﾀ		*/

	ui16	fq1;											/* １側周波数補正ﾃﾞｰﾀ		*/
	ui16	fq2;											/* １側周波数補正ﾃﾞｰﾀ		*/
	ui16	fq3;											/* ３側周波数補正ﾃﾞｰﾀ		*/
} HOSEI;
typedef	HOSEI* PHOSEI;
/*--------------------------------------------------------------------------------------*/
typedef	struct	tagELC
{
	ui08	req;											/* 有効電力量算出要求		*/
	ui08	alert;											/* ｱﾗｰﾄ検出ﾌﾗｸﾞ				*/

	DAT4	rd1;											/* １側有効電力量積算今回値	*/
	DAT4	rd1old;											/* １側有効電力量積算前回値	*/

	DAT4	rd2;											/* １側有効電力量積算今回値	*/
	DAT4	rd2old;											/* １側有効電力量積算前回値	*/

	DAT4	rd3;											/* ３側有効電力量積算今回値	*/
	DAT4	rd3old;											/* ３側有効電力量積算前回値	*/
} ELC;
typedef	ELC* PELC;

typedef struct tgELM {
	ui08	wf;
	ui08	vf;
	DAT8	a;
	DAT8	w;
	DAT8	v;
}ELM;

typedef struct tgCUM {
	DAT8	p;
	DAT8	m;
}CUM;

/*--------------------------------------------------------------------------------------*/
typedef	struct	tagPLS
{
	ui16	cnt0;											/* 有効電力量パルス数(正)	*/
	ui16	cnt1;											/* 有効電力量パルス数(逆)	*/
} PLS;
typedef	PLS* PPLS;
/*--------------------------------------------------------------------------------------*/
typedef	struct	tagVI
{
	ui08	rdok;											/* 電圧電流値読出しﾌﾗｸﾞ		*/
	ui08	stage;											/* 処理ﾌｪｰｽﾞ				*/
	ui16	kiadd1[3];										/* １側電流軽負荷補正値		*/
	ui16	kiadd2[3];										/* ２側電流軽負荷補正値		*/
	ui16	kiadd3[3];										/* ３側電流軽負荷補正値		*/
} VI;
typedef	VI* PVI;
/*--------------------------------------------------------------------------------------*/
typedef	struct	tagENGADD
{
	DAT8	add1;									/* 1側電力量加算確定値		*/
	DAT8	add2;									/* 1側電力量加算確定値		*/
	DAT8	add3;									/* 3側電力量加算確定値		*/

	DAT8	wk1;									/* 1側電力量加算値 ﾜｰｸ		*/
	DAT8	wk2;									/* 1側電力量加算値 ﾜｰｸ		*/
	DAT8	wk3;									/* 3側電力量加算値 ﾜｰｸ		*/

	ui08	sts;									/* 分割処理番号				*/
	ui08	cnt;									/* 加算回数ｶｳﾝﾄ値			*/
	ui08	tim;									/* 加算回数					*/
	ui08	dmy;									/* ﾊﾞｳﾝﾀﾞﾘ合わせのﾀﾞﾐｰ		*/
} ENGADD;

typedef	struct	tagLSIWK
{
	DAT8	wreg20;											/* ﾜｰｸﾚｼﾞｽﾀ20				*/
	ENGADD	eng[2];											/* 電力量加算用				*/
	/* [0]:有効 / [1]:無効		*/
} LSIWK;
typedef	LSIWK* PLSIWK;

/*--------------------------------------------------------------------------------------*/
typedef struct tgRAW41 {

	DAT4	ram;
	DAT4	old;
	DAT4	snf;
	DAT8	ssq;

	DAT4	old1;
	DAT4	snf1;
	//	DAT4	max;
}RAW41;

typedef struct tgRAW42 {

	DAT4	ram;	//0
	DAT4	old0;	//4
	DAT4	snf1;	//8
	DAT4	old1;	//12
	DAT4	snf2;	//16
	DAT4	old2;	//20
	DAT8	ssq;
	DAT4	snf3;	//16
	DAT4	old3;	//20
}RAW42;

typedef struct tgRAW43 {

	DAT4	ram;
	DAT4	old0;
	DAT4	snf1;
	DAT4	old1;
	DAT4	snf2;
	DAT4	old2;
	DAT4	snf3;
	DAT8	ssq;
}RAW43;

typedef struct tgRAW81 {

	DAT8	ram;
	DAT8	old;
	DAT8	snf;
	DAT8	ssq;
}RAW81;

/*--------------------------------------------------------------------------*/
typedef	struct	tagDTCCTL
{
	ui08	dtccr;
	ui08	dtbls;
	ui08	dtcct;
	ui08	dtrld;
	ui16	dtsar;
	ui16	dtdar;
} DTCCTL;

typedef	struct	tagDTC
{
	ui08	vct[40];
	ui08	rsv[24];							/* 予約領域					*/
	DTCCTL	ctl[24];							/* DTCｺﾝﾄﾛｰﾙﾃﾞｰﾀ領域		*/
} DTC;

typedef struct tgUNIT_SCAL {
	ui08	mode;
	eUNIT	unit;
	char	scale;
}UNIT_SCAL;
/*==========================================================================*/
/* 不揮発性ﾃﾞｰﾀ																*/
/*==========================================================================*/
typedef	struct	tagTYOSEI
{
	ui16	sp_b;								/* sp_b					0	*/
	ui16	sp_i;								/* sp_i					2	*/
	ui16	sp_u;								/* sp_u					4	*/
	ui16	sp_ca_1;							/* sp_ca_1				6	*/
	ui16	sp_ca_2;							/* sp_ca_2				8	*/
	ui16	sp_ca_3;							/* sp_ca_3				10	*/
	ui16	sp_hp_u1;							/* sp_hp_u1				12	*/
	ui16	sp_hp_u2;							/* sp_hp_u2				14	*/
	ui16	sp_hp_u3;							/* sp_hp_u3				16	*/
	ui16	sp_pthres;							/* sp_pthres			18	*/
	ui16	sp_pvalue;							/* sp_pvalue			20	*/
	ui08	rsv;								/*						22  */
	ui08	sp_freq;							/* sp_freq				23	*/
	ui16	sp_v_h;								/* sp_v_h				24	*/
	ui16	sp_v_l1;							/* sp_v_l1				26	*/
	ui16	sp_v_l2;							/* sp_v_l2				28	*/
	ui16	sp_v_l3;							/* sp_v_l3				30	*/
	ui16	sp_i_h;								/* sp_i_h				32	*/
	ui16	sp_i_l1;							/* sp_i_l1				34	*/
	ui16	sp_i_l2;							/* sp_i_l2				36	*/
	ui16	sp_i_l3;							/* sp_i_l3				38	*/
	ui16	sp_i_add1;							/* sp_i_add1			40	*/
	ui16	sp_i_add2;							/* sp_i_add2			42	*/
	ui16	sp_i_add3;							/* sp_i_add3			44	*/
	ui08	fmark;								/* Fﾏｰｸ					46	*/
	ui08	chksum;								/* ﾁｪｯｸｻﾑ				48	*/
} TYOSEI;
typedef	TYOSEI* PTYOSEI;
typedef struct tgPHASE_ADJ {
	ui08	adj0;
	ui08	adj1;
	ui08	adj2;
	ui08	adj3;
}PHASE_ADJ;
typedef struct tgCALB_RTC
{
	ui16	slope;			/* 温度計温度傾斜係数		*/
	ui16	score;			/* 中心温度センサー値		*/
	ui16	ondo;			/* 中心温度					*/
	ui16	offset;			/* オフセット周波数			*/
	ui08	unit;			/* 模倣周期倍数				*/
	ui08	base;			/**/
	ui16	tbl[28];		/**/
	ui16	freq;
}CALB_RTC;

typedef struct tgCALB_JIK
{
	ui08	axis_x;
	ui08	axis_y;
	ui08	axis_z;
	ui08	temp;
}CALB_JIK;

typedef struct tgCALB_LIV
{
	ui16	ratio;
	ui08	temp;
	ui08	reserve;
}CALB_LIV;

typedef struct tgCALB {

	CALB_RTC rtc;
	CALB_JIK jik;
	CALB_LIV liv;
	ui08	fmark;			/* Fﾏｰｸ						*/
	ui08	chksum;			/* ﾁｪｯｸｻﾑ					*/
}CALB;

typedef	struct	tagSIGNITURE
{
	ui32	cputime;
	ui08	original[24];
	ui08	current[24];
	ui08	fmark;							/* Fﾏｰｸ							*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ						*/
} SIGN;
typedef	SIGN* PSIGNITURE;

/*--------------------------------------------------------------------------*/
typedef	struct	tagMETER
{
	ui08	id1[12];						/* Serial */
	ui08	id2[8];							/* Model */
	ui08	id3[4];							/* Category */
	ui08	id4[4];							/* Meter type*/
	ui08	id5[10];						/* Current rating*/
	ui08	id6[30];						/* Manufacture */
	ui08	id7[4];							/* Meter year */
	ui08	id8[16];						/* Version */
	ui08	id9[10];						/* */
	ui08	id10[20];						/* BLUETOOTH*/
	ui08	id11[20];						/* */
	ui08	id12[10];						/* */
	ui16	wakesec;
	ui16	energizing;						/* 計量分周比 */
	ui08	fmark;							/* Fﾏｰｸ						*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ					*/
} METER;
typedef	METER __far * PMETER;

typedef	union	tagLOPT
{
	struct {
		ui16 fil : 1;
		ui16 kir : 3;
		ui16 rsv : 4;
		ui16 num : 4;
		ui16 dig : 4;
	} bt;
	ui16	hf;
} LOPT;

typedef	union	tagLSUB
{
	struct {
		ui16 rsv : 4;
		ui16 idx : 12;
	} bt;
	ui16	hf;
} LSUB;

typedef	union	tagLMOD
{
	struct {
		ui08 pre : 1;
		ui08 net : 1;
		ui08 hi : 1;
		ui08 rsv : 3;
		ui08 off : 1;
		ui08 on : 1;
	} bt;
	ui08	by;
} LMOD;

typedef	union	tagLIDX
{
	struct {
		ui16 tou : 1;
		ui16 div : 2;
		ui16 rgx : 1;
		ui16 prv : 2;
		ui16 idx : 10;
	} bt;
	ui16	hf;
} LIDX;

typedef struct tgDISSET {
	ui08	period;		/*7-4bit:マニュアル時間 3-0bit:サイクリック時間*/
	LMOD	automate;
	DAT2	option;		/*15-8bit:桁上げ、7bit:0埋め 6-4bit:整数桁、3-0bit有効桁*/
	DAT2	serial;		/*上位4ビットはモーション*/
	LIDX	lidx;		/*上位4ビットはインデックス*/
}DISSET;

typedef struct tgDETSET {
	ui08	threshold0;	/*検出閾値*/
	ui08	threshold1;	/*解除閾値*/
	ui08	threshold2;	/*開始条件閾値*/
	ui08	interval;	/**/
	ui08	period0;	/*検出時間*/
	ui08	period1;	/*解除時間*/
	ui08	fmark;		/* Fﾏｰｸ							*/
	ui08	chksum;		/* ﾁｪｯｸｻﾑ						*/
}DETSET;

typedef struct tgRLYCFG {
	ui32	interval;
	ui32	interval0;
	ui32	locktime;
	ui32	locktime0;
	ui08	reconnect;
	ui08	reconnect0;
	DTTM	dttm;
}RLYCFG;

typedef	struct	tagDEFSET {
	RLYCFG	brkcfg;
	ui16	adjsec;
	ui16	div;
	ui08	period1;
	ui08	period2;
	ui08	period3;
	ui08	period4;
	ui08	unit_ryo[4];
	ui08	unit_dmd[4];
	ui08	unit_rto[4];
	ui08	unit_eff[4];
	ui08	unit_oth[4];					/*その他単位*/
	ui08	fmark;							/* Fﾏｰｸ							*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ						*/
} DEFSET;
typedef	DEFSET* PDEFSET;

typedef struct tgTSNCFG {
	ui08	baud;
	ui08	reserved;
	ui08	win_size_rcv;
	ui08	win_size_snd;
	ui16	info_len_rcv;
	ui16	info_len_snd;
	ui16	inter_timeout;
	ui16	inact_timeout;
}TSNCFG;

typedef struct tgMTRCFG {

	FNFG	userflag;						/* 機能フラグ */
	ui08	vref;
	ui08	ib;
	ui08	imax;
	ui08	freq;
	ui08	battlv;
	ui08	vrtc;
	ui08	vbat;
	ui08	match;
	ui08	mode;
	ui08	puls;
	ui08	source;
	ui08	thrd0;	/*10*/
	ui08	thrd1;	/*8*/
	ui08	thrd2;	/*36*/
	ui08	thrd3;	/*72*/
	ui08	thrd4;	/*250*/
	ui16	width;	/*30000*/
	ui16	dropp;
	ui16	dropm;
	ui16	ndhalf;
	ui08	val0;
	ui08	val1;
	ui08	val2;
	ui08	val3;
	ui08	val4;
}MTRCFG;

typedef struct tgPQCODES {
	ui16	code[92];
}PQCODES;

typedef	struct	tagESWEEP {
	ui08	reg[16];
	ui08	fil[16];
	ui08	dsc[16];
} ESWEEP;

typedef	struct	tagESW {
	DAT16	reg;
	DAT16	fil;
	DAT16	cur;
} ESW;

typedef	ESW* PESW;

typedef struct tgSCRIPT {
	ui16 inst;
	ui16 ident;
}SCRIPT;

typedef struct tgSGLACT1 {
	SCRIPT	scr;
	DTTM	sch;
}SINGLE1;

typedef struct tgSGLACT6 {
	SCRIPT	scr;
	DTTM	sch[6];
}SINGLE6;

typedef	struct	tagCUMCNT {
	ui32	timestamp;
	ui16	cumcnt;
	ui08	fmark;							/* Fﾏｰｸ							*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ						*/
}CUMCNT;

typedef	struct	tagUSERSET1 {
	MTRCFG	mtrcfg;
	TSNCFG	tsncfg[2];
	ui08	fmark;						/* Fﾏｰｸ						*/
	ui08	chksum;						/* ﾁｪｯｸｻﾑ					*/
} USERSET1;
typedef	USERSET1* PUSERSET1;

typedef	struct	tagUSERSET2 {
	ESWEEP	esw;
	RELAY	relay;
	SINGLE1	single1[8];
	SINGLE6	single6;
	ui32	pwroff;
	ui32	pwron;
	ui32	billpwron;
	ui32	recharge_amount;
	ui32	recharge_time;
	ui32	total_amount;
	ui32	current_balance_amount;
	ui32	current_balance_time;
	ui08	fmark;						/* Fﾏｰｸ						*/
	ui08	chksum;						/* ﾁｪｯｸｻﾑ					*/
} USERSET2;
typedef	USERSET2* PUSERSET2;

typedef struct tgOTHIF {
	ui08 id;
	ui08 size;
	ui08 value[50];
}OTHIF;

typedef union	tagCMST {
	struct {
		ui08 get0 : 1;
		ui08 get1 : 1;
		ui08 bit2 : 1;
		ui08 bit3 : 1;
		ui08 bit4 : 1;
		ui08 bit5 : 1;
		ui08 bit6 : 1;
		ui08 bit7 : 1;
	} bt;
	ui08	by;
}CMST;

typedef struct tgCOMSTS {
	CMST update;
	ui08 strength;
	DAT4 status;
}COMSTS;

typedef struct tgNICINF {
	ui08 rsrp;
	ui08 rsrq;
	ui08 sinr;
	ui08 rssi;
	ui08 rat;
	ui08 band;
	DAT4 cellid;
	DAT4 status;
}NICINF;
/*--------------------------------------------------------------------------*/
typedef	struct	tagLCD8
{
	ui08	dt0;
	ui08	dt1;
	ui08	dt2;
	ui08	dt3;
	ui08	dt4;
	ui08	dt5;
	ui08	dt6;
	ui08	dt7;
} LCD8;
typedef	LCD8* PLCD8;
typedef	struct	tagLCD6
{
	ui08	dt0;
	ui08	dt1;
	ui08	dt2;
	ui08	dt3;
	ui08	dt4;
	ui08	dt5;
} LCD6;
typedef	LCD6* PLCD6;
/*--------------------------------------------------------------------------*/
typedef	struct	tagBCD10
{
	ui08	dt01;								/* 電力量１，２桁目			*/
	ui08	dt23;								/* 電力量３，４桁目			*/
	ui08	dt45;								/* 電力量５，６桁目			*/
	ui08	dt67;								/* 電力量７，８桁目			*/
	ui08	dt89;								/* 電力量９，10桁目			*/
} BCD10;
typedef	BCD10* PBCD10;

/*--------------------------------------------------------------------------*/
typedef	struct	tagRYO
{
	DAT4	add[10];		/*VA(2),WAT(2),VARQ(4),FAN(2)*/
	DAT4	tou[48];		/*VA(8+8),WAT(8+8),VAR(8+8)*/
	KEICOS	keicos[8];
	DAT4	defraud;
	DAT4	apfm[36];
	DAT4	apfd[4];
	DAT4	apfb[4];
	DAT4	defra;
	DAT4	defra_tou[8];
	ui08	fmark;							/* Fﾏｰｸ						*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ					*/
} RYO;
typedef	RYO	*PKEI;

/*--------------------------------------------------------------------------*/
typedef	struct	tagRECLOG
{
	ui08	code1;
	ui08	code2;
} RECLOG;
typedef	RECLOG* PRECLOG;

typedef	struct	tagCAPLOG
{
	ui08	code1;
	ui08	code2;
} CAPLOG;
typedef	CAPLOG* PCAPLOG;

/*--------------------------------------------------------------------------*/
typedef	struct	tagREC08CTL
{
	ui08	ptr;								/* 最新ﾎﾟｲﾝﾀ					*/
	ui08	t_ptr;								/* 退避ﾎﾟｲﾝﾀ					*/
	ui08	ring;
	ui08	dmy;
} REC08P;
typedef	REC08P* PREC08CTL;
/*--------------------------------------------------------------------------*/
typedef	struct	tagJGN4
{
	ui08	date[4];
} JGN4;
typedef	JGN4* PJGN4;
/*--------------------------------------------------------------------------*/
typedef	struct	tagJGN3
{
	ui08	date[3];
} JGN3;
typedef	JGN3* PJGN3;
/*--------------------------------------------------------------------------*/
typedef	struct	tagJGN2
{
	ui08	date[2];
} JGN2;
typedef	JGN2* PJGN2;
/*--------------------------------------------------------------------------*/
typedef	struct	tagRECEVE
{
	ui16	code;
	ui08	volt1[2];
	ui08	ampc[2];
	ui08	ampg[2];
	ui08	pf[2];
	ui08	wh[4];
	ui08	vah[4];
	ui08	recordno[4];
} RECEVE;
typedef	RECEVE*	PRECEVE;

typedef	struct	tagCAPEVE
{
	ui16	code;
	ui08	volt1[2];
	ui08	ampc[2];
	ui08	ampg[2];
	ui08	pf[2];
	ui08	wh[4];
	ui08	vah[4];
	ui08	recordno[4];
	ui08	serialno[8];
} CAPEVE;
typedef	CAPEVE*	PCAPEVE;

typedef	struct	tagCAPOPE
{
	ui16	code;
	ui08	recordno[4];
	ui08	serialno[8];
} CAPOPE;
typedef	CAPOPE*	PCAPOPE;

typedef struct tgUB4 {
	ui08	u[4];
}UB4;

typedef	struct	tagRECRLY {
	UB4	impWh[9];
	UB4	expWh;
	UB4	impVAh[9];
	UB4	expVAh;
	UB4	varh[4];
} RECRLY;

typedef	struct	tagCAPRLY {
	UB4	impWh[9];
	UB4	expWh;
	UB4	impVAh[9];
	UB4	expVAh;
	UB4	varh[4];
} CAPRLY;

typedef	RECRLY* PRECRLY;
typedef	CAPRLY* PCAPRLY;

/*----------------------Billing Capture Objects-------------------------------------*/

typedef struct tgBLDM {
	ui08	max[4];
	ui08	date[4];
}BLDM;

typedef struct tgBLD {
	ui08	cum[4];
	ui08	max[4];
	ui08	date[4];
}BLD;

typedef struct	tagRECBLD1 {
	ui08	apfp[18];
	ui08	apfm[18];
	ui08	billpwron[4];
	UB4		net[2];
	ui08	tampsum[2];
	ui08	tampmon[2];
} RECBLD1;

typedef struct	tagRECBLD2 {
	BLD		imp[9];
	BLD		exp[9];
	BLDM	max24[2];
} RECBLD2;	/*WAT*/

typedef struct	tagRECBLD3 {
	BLD		imp[9];
	BLD		exp[9];
	BLDM	max24[2];
	UB4		varh[4];
} RECBLD3; /*VA*/

typedef struct	tagRECBLD4 {
	UB4		impVAh[9];
	UB4		expVAh[9];
	BLDM	max24[2];
	UB4		impWh[9];	/*PrePaid*/
	UB4		expWh[9];
} RECBLD4;

typedef struct	tagRECBLD5 {
	ui08	relay[640];
} RECBLD5;

typedef	RECBLD1* PRECBLD1;
typedef	RECBLD2* PRECBLD2;
typedef	RECBLD3* PRECBLD3;
typedef	RECBLD4* PRECBLD4;
typedef	RECBLD5* PRECBLD5;
typedef struct	tagCAPBLD1 {
	ui08	billpwron[4];
	ui08	apf[2];
} CAPBLD1;

typedef struct	tagCAPBLD2 {
	UB4		impWh[9];
	UB4		expWh;
	UB4		imp;
	ui08	date[12];
} CAPBLD2;

typedef struct	tagCAPBLD3 {
	UB4		impVAh[9];
	UB4		expVAh;
	UB4		imp;
	ui08	date[12];
} CAPBLD3;

typedef	CAPBLD1* PCAPBLD1;
typedef	CAPBLD2* PCAPBLD2;
typedef	CAPBLD3* PCAPBLD3;

/*--------------------------------------------------------------------------*/
typedef struct	tagRECLOP {
	ui08	volt1[2];
	ui08	impWh[4];
	ui08	impVAh[4];
	ui08	expWh[4];
	ui08	expVAh[4];
	ui08	ampc[2];
	ui08	ampg[2];
	ui08	varq1[4];
	ui08	varq2[4];
	ui08	varq3[4];
	ui08	varq4[4];
} RECLOP;

typedef struct	tagCAPLOP {
	ui08	volt1[2];
	ui08	impWh[4];
	ui08	impVAh[4];
	ui08	expWh[4];
	ui08	expVAh[4];
	ui08	ampc[2];
	ui08	ampg[2];
	ui08	varq1[4];
	ui08	varq2[4];
	ui08	varq3[4];
	ui08	varq4[4];
} CAPLOP;
typedef	RECLOP*	PRECLOP;
typedef	CAPLOP*	PCAPLOP;

/*--------------------------------------------------------------------------*/
typedef struct	tagRECDAY {
	ui08	expWh[4];
	ui08	expVAh[4];
	ui08	impWh[4];
	ui08	impWh1[4];
	ui08	impWh2[4];
	ui08	impWh3[4];
	ui08	impWh4[4];
	ui08	impWh5[4];
	ui08	impWh6[4];
	ui08	impWh7[4];
	ui08	impWh8[4];
	ui08	impVAh[4];
	ui08	fund[4];
	ui08	maxW[4];
	ui08	dateW[4];
	ui08	maxVA[4];
	ui08	dateVA[4];
	ui08	varq1[4];
	ui08	varq2[4];
	ui08	varq3[4];
	ui08	varq4[4];
} RECDAY;

typedef struct	tagCAPDAY {
	ui08	expWh[4];
	ui08	expVAh[4];
	ui08	impWh[4];
	ui08	impWh1[4];
	ui08	impWh2[4];
	ui08	impWh3[4];
	ui08	impWh4[4];
	ui08	impWh5[4];
	ui08	impWh6[4];
	ui08	impWh7[4];
	ui08	impWh8[4];
	ui08	impVAh[4];
	ui08	fund[4];
	ui08	maxW[4];
	ui08	dateW[12];
	ui08	maxVA[4];
	ui08	dateVA[12];
	ui08	varq1[4];
	ui08	varq2[4];
	ui08	varq3[4];
	ui08	varq4[4];
} CAPDAY;
typedef	RECDAY*	PRECDAY;
typedef	CAPDAY*	PCAPDAY;

/*IST_PUSHD_HES*/
typedef struct	tagRECPSH {
	ui08	data[0x100];
} RECPSH;
typedef	RECPSH* PRECPSH;

typedef	struct	tagRECFLG
{
	RECPTR		recptr[RECORD];
	ui08		fmark;							/* Fﾏｰｸ							*/
	ui08		chksum;							/* ﾁｪｯｸｻﾑ						*/
} RECFLG;

/*--------------------------------------------------------------------------*/
/*==========================================================*/
/*==========================================================*/
typedef	struct	tagREG2EX
{
	DAT4	date;
	DAT2	reg;
} REG2EX;

typedef	struct	tagREG4EX
{
	DAT4	date;
	DAT4	reg;
} REG4EX;

typedef	struct	tagMINMAX
{
	REG2EX	min[4];
	REG4EX	max[40];		/*8+8+8+8+8*/
	REG4EX	max24[8];
	ui32	cum[8];
	ui08	fmark;							/* Fﾏｰｸ							*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ						*/
} MINMAX;
typedef	MINMAX* PMINMAX;

typedef	struct	tagEVEDAT
{
	ui16	cnt;
	ui08	mode;
	ui08	opt;
	DAT4	time;
} EVEDAT;
typedef	EVEDAT* PEVEDAT;

typedef	struct	tagEVESTS
{
	EVEDAT	cap[32];
	DAT4	act;
	DAT4	prv;
	DAT4	fault;
	DAT2	sub[32];
	ui16	pf0;
	ui16	v1;
	ui16	i0;
	ui16	i1;
	ui16	i2;
	ui16	vthd;
	ui16	athd;
	ui16	atdd;
	ui08	fg0;
	ui08	fg1;
	ui08	fg2;
	ui08	mode;								/* 分割処理番号			*/
	ui08	fmark;								/* Fﾏｰｸ					*/
	ui08	chksum;								/* ﾁｪｯｸｻﾑ				*/
} EVESTS;
typedef	EVESTS* PEVESTS;
/*==========================================================*/
/*==========================================================*/
typedef	struct	tagSLEEP
{
	ui32	cum;
	ui32	sec;
	DTTM	date;
	ui08	fmark;							/* Fﾏｰｸ							*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ						*/
} SLEEP;
typedef	SLEEP* PSLEEP;

typedef	struct	tagJGNDAT
{
	DAT4	present;
	DAT4	current;
	ui16	startnen;						/* 初期年					*/
	ui16	finalnen;						/* 終了年					*/
	ui16	oneday;
	ui08	base;
	ui08	term;							/*分単位*/
	ui08	timer;							/*分単位*/
	ui08	reset;							/* 時計ﾘｾｯﾄﾌﾗｸﾞ				*/
	ui08	fmark;							/* Fﾏｰｸ							*/
	ui08	chksum;							/* ﾁｪｯｸｻﾑ						*/
} JGNDAT;
typedef	JGNDAT* PJGNDAT;

typedef	union	tagHRD
{
	struct {
		ui16 supp : 1;
		ui16 v1 : 1;
		ui16 lvi : 1;
		ui16 cvr : 1;
		ui16 mag : 1;
		ui16 eep : 1;
		ui16 nic : 1;
		ui16 dv1 : 1;
		ui16 zero : 1;
		ui16 scv : 1;
		ui16 rsv : 7;
	} bt;
	ui08	by[2];
	ui16	hf;
} HRD;
typedef	HRD* PHRD;

typedef	union	tagIMOP
{
	struct {
		ui08 no_backup : 1;	/* backup before update */
		ui08 rsv1 : 1;
		ui08 rsv2 : 1;
		ui08 restart : 1;
		ui08 mem_clr : 1;
		ui08 zero_skip : 1;
		ui08 without_backup : 1;	/*update after backup*/
		ui08 skip_backup : 1;
	} bt;
	ui08	by;
} IMOP;

typedef	union	tagFAULT
{
	struct {
		ui08 rsv : 1;
		ui08 vrtc : 1;
		ui08 vbat : 1;
		ui08 lcd : 1;
		ui08 rst : 1;
		ui08 mtr : 1;
		ui08 eep : 1;
		ui08 rtc : 1;
	} bt;
	ui08	by;
} FAULT;

/*--------------------------------------------------------------------------*/
typedef	union	tagPORTS
{
	ui08	by[2];
	ui16	hf;
} PORTS;
typedef	PORTS* PPORTS;
/*--------------------------------------------------------------------------*/
typedef	union	tagTM1 {
	struct {
		ui08 tmr : 6, drv : 1, up : 1;
	} bt;
	ui08 by;
} TM1;
typedef	TM1* PTM1;

typedef	union	tagTM2 {
	struct {
		ui16 tmr : 14, drv : 1, up : 1;
	} bt;
	ui16 hf;
} TM2;
typedef	TM2* PTM2;

typedef	union	tagLongInvokeIdAndPriority {
	struct {
		ui32 lii : 24;
		ui32 rsv : 4;
		ui32 self : 1;
		ui32 option : 1;
		ui32 service : 1;
		ui32 priority : 1;
	} bt;
	ui08	by[4];
	ui32	wd;
} LIIAP;
typedef	LIIAP* PLIIAP;
/*--------------------------------------------------------------------------*/
typedef	struct	tagCAL
{
	DAT4	apt;				/* 有効電力(HEX)			*/
	DAT4	wat;				/* 有効電力(HEX)			*/
	DAT4	var;				/* 無効電力(HEX)			*/
	DAT8	aptaddwk;			/* 有効電力加算用			*/
	DAT8	wataddwk;			/* 有効電力加算用			*/
	DAT8	varaddwk;			/* 無効電力加算用			*/
	ui08	aptreq;				/* 有効電力生成要求用ﾌﾗｸﾞ		*/
	ui08	watreq;				/* 有効電力生成要求用ﾌﾗｸﾞ		*/
	ui08	varreq;				/* 無効電力生成要求用ﾌﾗｸﾞ		*/
	ui08	aptaddcnt;			/* KW 計算回数ｶｳﾝﾀ			*/
	ui08	wataddcnt;			/* KW 計算回数ｶｳﾝﾀ			*/
	ui08	varaddcnt;			/* ELC 計算回数ｶｳﾝﾀ			*/
	ui08	sts;				/* 有効/無効 電力, 力率処理状態用ﾌﾗｸﾞ	*/
	ui08	dmy;
}CAL;

typedef	struct	tagRIK
{
	CAL		cal;
	DAT4	cos;				/* 力率(HEX)				*/
	DAT4	apt;				/* 有効電力(HEX)			*/
	DAT4	wat;				/* P(有効電力)ﾜｰｸ		*/
	DAT4	var;				/* Q(無効電力)ﾜｰｸ		*/
	//	DAT4	amp;				/* P×1000 or P*1		*/
	//	DAT4	wat_var;			/* P/Q					*/
	DAT4	wat2_var2;			/* (P/Q)^2				*/
	DAT4	sqrt;				/* √(1 + (P/Q)^2)		*/
	//	DAT4	hex;				/* 1/√(1 + (P/Q)^2)		*/
	ui08	req;				/* 力率計算要求			*/
	ui08	task;				/* ﾀｽｸNo					*/
	ui08	sign;
	ui08	times;
} RIK;
typedef	RIK* PRIK;

typedef	struct	tagBTN
{
	ui08	zenkai;							/* ｽｲｯﾁ状態ﾎﾟｰﾄ前回値			*/
	ui08	konkai;							/* ｽｲｯﾁ状態ﾎﾟｰﾄ今回値			*/
	ui08	edgeon;							/* ｽｲｯﾁ状態ﾎﾟｰﾄ立上りｴｯｼﾞ確定値	*/
	ui08	edgeoff;						/* ｽｲｯﾁ状態ﾎﾟｰﾄ立下りｴｯｼﾞ確定値	*/
	ui08	latch;							/* ｽｲｯﾁ状態ﾎﾟｰﾄ立上りﾗｯﾁ情報	*/
	ui08	swon1;							/* ｽｲｯﾁ押下情報1(第1押下釦)		*/
	ui08	swon2;							/* ｽｲｯﾁ押下情報2(第2押下釦)		*/
	ui08	swon3;							/* ｽｲｯﾁ押下情報3(第3押下釦)		*/
	ui08	swon4;							/* ｽｲｯﾁ押下情報4(第4押下釦)		*/
	ui08	only;							/* ｽｲｯﾁ単独短押下ｴｯｼﾞ情報		*/
	ui08	lonly;							/* ｽｲｯﾁ単独長押下ｴｯｼﾞ情報		*/
	ui08	ptime;							/* 経過時間ｶｳﾝﾀ					*/
} BTN;
typedef	BTN* PSW;

typedef struct tgRSVLOG {
	elogs cde;
	ui08 opt;
}RSVLOG;

typedef	struct	tagXDLMS_RAND
{
	ui08		rand[4 * 6 + 16];
	LIIAP		liiap;
	ui08		fmark;						/* Fﾏｰｸ							*/
	ui08		chksum;						/* ﾁｪｯｸｻﾑ						*/
} XDLMS_RAND;
typedef	XDLMS_RAND* PXDLMS_RAND;
typedef	struct	tagLIFE
{
	ui16		count0;		/* cpu:count0x65535		*/
	ui16		count1;		/* cpu:count1x65535		*/
	ui08		count2;		/* cpu:500ms 			*/
	ui08		count3;		/* cpu:2ms				*/
	ui16		count4;		/* switch               */
	ui16		count5;		/* cover                */
	ui16		count6;		/* 		*/
	ui16		count7;		/*		*/
	ui08		fmark;		/* Fﾏｰｸ					*/
	ui08		chksum;		/* ﾁｪｯｸｻﾑ				*/
} LIFE;

typedef struct tagPULSCTL {
	ui08 sts;
	ui08 offbit;
	DAT4 cur;
	DAT4 max;
}PULSCTL;


typedef	union tagOCTL
{
	struct {
		ui08 idx : 7, four : 1;
	} bt;
	ui08 by;
} OCTL;
typedef OCTL __far* POCTL;
typedef	union	tagTABLE1
{
	struct {
		ui08 data : 5, act : 1, set : 1, get : 1;
	} bt;
	ui08 by;
} ACC1;

typedef	union	tagTABLE4
{
	struct {
		ui32 data : 29, act : 1, set : 1, get : 1;
	} bt;
	ui08 by[4];
	ui16 hf[2];
	ui32 wd;
} ACC4;

typedef struct tgSYS {
	ui08		sys[8];
	ui08		sc;
	ui08		maddr;
}SYS;

typedef struct tgACC {
	ui08		acc1[32];
	ui32		acc4[6];
}ACC;
typedef ACC __far* PACC;

typedef struct tagKEY {
	ui08		len[2];
	ui08		dat[32];
}KEY;

typedef	struct	tagUSER
{
	SYS			sys;
	ACC			acc;
} USER;
typedef USER* PUSER;
typedef void(*INIDAT)(USER* out, cui08 area);

typedef union	tagBIT
{
	struct {
		ui08 bit0 : 1;
		ui08 bit1 : 1;
		ui08 bit2 : 1;
		ui08 bit3 : 1;
		ui08 bit4 : 1;
		ui08 bit5 : 1;
		ui08 bit6 : 1;
		ui08 bit7 : 1;
	} bt;
	ui08	by;
}BIT8;

typedef struct tgSOJUN {
	ui16 base;
	ui16 sou_a;
	ui16 sou_b;
	ui16 sou_c;
	ui08 diff;
	ui08 err;
}SOJUN;

typedef struct tgSCHEDULE {
	DTTM	start;
	ui32	sec;
	FNC		call;
}SCHEDULE;	/*16*/

typedef struct tgTOU_CAL {
	ui08	calender[16];
	DTTM	dttm;
	ui08	fmark;				/* Fﾏｰｸ							*/
	ui08	chksum;				/* ﾁｪｯｸｻﾑ						*/
}TOU_CAL;

typedef struct tgATERM {
	DTTM	start;
	ui08	season[16];
	ui08	weekidx;
	ui08	inf;
}ATERM;	/*6*/

typedef struct tgTOU_TERM {
	ATERM	tout[4];
	ui08	fmark;				/* Fﾏｰｸ							*/
	ui08	chksum;				/* ﾁｪｯｸｻﾑ						*/
}TOU_TERM;

typedef struct tgAWEEK {
	ui08	week[6];
	ui08	dayid[7];
	ui08	inf;
}AWEEK;		/*8*/

typedef struct tgTOU_WEEK {
	AWEEK	touw[4];			/* 8x10 */
	ui08	fmark;				/* Fﾏｰｸ							*/
	ui08	chksum;				/* ﾁｪｯｸｻﾑ						*/
}TOU_WEEK;

typedef struct tgASCR {
	TM		start;
	ui08	arg;
}ASCR;		/*6*/

typedef struct tgADAY {
	ASCR	scr[8];
}ADAY;		/*24*/

typedef struct tgTOU_DAY {
	ADAY	toud[4];			/*24x10*/
	ui08	fmark;				/* Fﾏｰｸ							*/
	ui08	chksum;				/* ﾁｪｯｸｻﾑ						*/
}TOU_DAY;

typedef struct tgTOU_DAT {
	TOU_CAL		cal;
	TOU_TERM	term;
	TOU_WEEK	week;
	TOU_DAY		day;
}TOU_DAT;

typedef struct tgTRGID {
	ui08	now;
	ui08	nxt;
}TRGID;		/*2*/

typedef struct tgSPDAY {
	DT3		start;
	ui08	day;
}TOU_SPD;

typedef struct tgTOUS {
	TOU_SPD		tous[7];
	ui08		fmark;				/* Fﾏｰｸ							*/
	ui08		chksum;				/* ﾁｪｯｸｻﾑ						*/
}TOUS;

typedef void(*SCHEDULER)(SCHEDULE* out, const ui32 cur);
typedef ui08(*DETFNC)(cui08 index);

typedef struct tgCALLING {
	ui16	arg;
	ui16	inst;
	ui08	order;
	ui08	port;
	ui08	userid;
	ui08	active;
}CALLING;

typedef struct tgIO
{
	uint	len;
	uint	pos;
	pui08	buf;
}IO;
typedef IO* PIO;

typedef struct tgRAW {

	pui08	buf;	/*ダイレクトバッファポインタ*/
	ui16	len;	/*第1レイヤー受送信数(確定)*/
	ui16	siz;	/*第2レイヤー確保サイズ*/
	DAT2	adr2;	/*アドレス2バイト領域*/
	ui08	posW;	/*現在ウインドウ*/
	ui08	maxW;	/*最大ウインドウ数*/
	ui08	seg;	/*セグメンテーションフラグﾞ*/
	ui08	pfb;	/*P/Fビット(対話)ﾞ*/
	ui08	cmd;	/*コマンド*/
	ui08	ns;		/*送信シーケンス番号N(S)*/
	ui08	nr;		/*受信シーケンス番号N(R)*/
	ui08	rsv;
}RAW;

typedef struct tgSEQ_CTL {
	RAW		req;
	RAW		res;
	DAT4	adr4;	/*アドレス4バイト領域*/
	ui16	pos;	/*第1レイヤーオフセット位置*/
	ui16	inf;	/*第2レイヤー受送信数*/
	ui16	oct;	/*inter octet timeout*/
	ui16	act;	/*inactivity timeout*/
	ui16	tmr;	/*2ミリタイマー*/
	ui16	cks;	/*チェックサム*/
	ui16	crc;	/*チェックサム*/
	ui08	phs;	/*処理0:len,1:addr,2:addr,3;cmd,4;data*/
	ui08	mod;	/*受送信モード　STX、DLMS*/
	ui08	ads;	/*アドレスサイズ*/
	ui08	cur;	/*アドレスサイズ*/
	ui08	neg;
	ui08	rsv;
}SEQ;

typedef struct tgPIPE
{
	IO		req;	/*第3レイヤー*/
	IO		res;	/*第3レイヤー*/
	ui08	prt;	/*第3レイヤー*/
	ui08	sta;	/*第3レイヤー*/
	ui08	cmd;	/*コマンドインデックス*/
	ui08	ctl;	/*第3レイヤー*/
	ui08	tsk;	/*分割処理番号*/
	ui08	err;	/*エラー*/
	ui08	inv;	/*inversion*/
	ui08	notify;	/*reserve*/
	ui16	cnt;
}PIPE;
typedef PIPE* PPIP;
typedef void(*FNC_HDLC)(PPIP);

typedef struct tgPEEK {
	ui16 ctl;
	ui16 m;
	ui16 p;
	ui16 peek;
	ui16 cnt;
	ui16 sts;
	ui32 first;
	ui16 change;
	ui32 sum0;
	ui32 sum1;
}PEEK;

typedef struct tgPEEK32 {
	ui16 ctl;
	DAT4 cur;
	ui32 m;
	ui32 p;
}PEEK32;

typedef struct tgIMAGE {
	IMOP ctl0;
	ui08 src;
	ui08 done;
	ui08 err;
	ui08 enable;		/*0:blockno 1: 2:enable 3:status */
	ui08 status;
	ui32 from;
	ui32 to;
	ui16 blockno;
	ui32 cntw[2];
	ui32 cntr[2];
	ui16 crci[2];
	ui16 crcw[2];
	ui16 crcr[2];
	ui32 buildNO;
	ui16 crc;
}IMAGE;

typedef struct tgHOT {
	ui16 val0;	/*CPU*/
	ui16 val1;	/*MAGNET*/
}HOT;

typedef struct tgRATING {
	DAT8	energy;
	ui16	current;
}PRATING;

/*==========================================================================*/
/* FFT用																	*/
/*==========================================================================*/
typedef	union	tagFFTFLG
{
	struct {
		ui08 currentctl : 1;	/* 1側2側電流切替ﾌﾗｸﾞ	*/
		ui08 low_current : 1;	/* 電流LOW				*/
		ui08 hokan1 : 1;		/* 補間手法(線形、Sinc)　切替え		*/
		ui08 hokan2 : 1;		/* 補間手法(線形、64ﾃﾞｰﾀ正規化）切替え */
		ui08 thd_v_4 : 1;		/* thd_vが4%以上 */
		ui08 thd_i_4 : 1;		/* thd_iが4%以上 */
		ui08 reserve5 : 1;
		ui08 reserve6 : 1;
	} bt;
	ui08	by;
} FFTFLG;
typedef	FFTFLG* PFFTFLG;

typedef	struct	tagNILMCTL
{
	ui08	posv;			/* ｻﾝﾌﾟﾘﾝｸﾞﾃﾞｰﾀﾊﾞｯﾌｧﾎﾟｲﾝﾀ	*/
	ui08	confirm;		/* ｻﾝﾌﾟﾘﾝｸﾞﾃﾞｰﾀﾊﾞｯﾌｧﾎﾟｲﾝﾀ	*/
	ui08	ctl_step;		/* 正規化～thd ｽﾃｯﾌﾟ		*/
	ui08	sub_step;		/* 正規化ｽﾃｯﾌﾟ				*/
} NILMCTL;

typedef	union	tagNICSTS
{
	struct {
		ui32 gprs : 1;
		ui32 port : 1;
		ui32 inf0 : 1;
		ui32 inf1 : 1;
		ui32 inf2 : 1;
		ui32 inf3 : 1;
		ui32 inf4 : 1;
		ui32 inf5 : 1;
		ui32 inf6 : 1;
		ui32 inf7 : 1;
		ui32 inf8 : 1;
		ui32 inf9 : 1;
		ui32 adr0 : 1;
		ui32 adr1 : 1;
		ui32 adr2 : 1;
		ui32 adr3 : 1;
		ui32 adr4 : 1;
		ui32 adr5 : 1;
		ui32 adr6 : 1;
		ui32 adr7 : 1;
		ui32 adr8 : 1;
		ui32 adr9 : 1;
		ui32 rsv : 1;
		ui32 sts : 1;
		ui32 cnt : 8;
	} bt;
	ui08	by[4];
	ui16	hf[2];
	ui32	wd;
} NICSTS;

typedef	union	tagNISTS
{
	struct {
		ui08 rsrp : 1;
		ui08 rsrq : 1;
		ui08 sinr : 1;
		ui08 rssi : 1;
		ui08 status : 1;
		ui08 cellid : 1;
		ui08 rat : 1;
		ui08 band : 1;
	} bt;
	ui08	by;
} NISTS;

typedef	union	tagSLFDIAG
{
	struct {
		ui08 clock : 1;
		ui08 nvm : 1;
		ui08 batm : 1;
		ui08 batr : 1;
		ui08 relay : 1;
		ui08 meter : 1;
		ui08 rsv : 2;
	} bt;
	ui08	by;
} SLFDIAG;

typedef struct tgPSHCFG{
	ui08	retry;
	ui08	rsv;
	ui16	rand;
	ui16	delay;
	ui08	fmark;				/* Fﾏｰｸ							*/
	ui08	chksum;				/* ﾁｪｯｸｻﾑ						*/
}PSHCFG;

typedef struct tgLCRCFG {
	ui08	enable;
	ui08	timer0;
	ui08	timer1;
	ui08	mon;
	ui08	fmark;				/* Fﾏｰｸ							*/
	ui08	chksum;				/* ﾁｪｯｸｻﾑ						*/
}LCRCFG;

#endif /*#ifndef _DATA_H_*/
