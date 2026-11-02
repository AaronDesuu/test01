/****************************************************************************/
/*                                                                          */
/*  システム名      :   DLMS                                                */
/*                                                                          */
/*  サブシステム名  :   AES-GCM                                             */
/*                                                                          */
/*  内容            :   DLMS暗号、復号処理                                  */
/*                                                                          */
/*  ファイル名      :   aes_gcm.c                                           */
/*                                                                          */
/*  作成日          :   2013-04-05  担当: T.Kuroda                          */
/*                                                                          */
/*  変更履歴        :                                                       */
/*      2016-01-20 T.Kuroda PC用マクロ削除                                  */
/*      2019-01-18 T.Kuroda HW-AESGCM対応                                   */
/*                                                                          */
/****************************************************************************/
#include "def.h"
#include "data.h"
#include "ram.h"
#include "devtmr.h"
#include "hrdtushin.h"
#include "apleeptask.h"			/* define宣言					*/
#include "aplrecord.h"			/* define宣言					*/
#include "aplevent.h"			/* define宣言					*/
#include "sub.h"
#include "dlms.h"
#include "conf.h"
#include "acse.h"
#include "aes_gcm.h"
#include "xdlms.h"
#if (0==SW_AES)
#include "iodef_aes.h"
#if DB_AES
ui08	debbuf[64];
#endif
#endif

/* macro for xtime^2 table generate */
#define xtime2(x) ((x)&0x80?((ui08)((x)<<1)^0x1b):((ui08)((x)<<1)))

/* macro for s-box table */
#define sb_data(w) {\
    w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
    w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
    w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
    w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
    w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
    w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
    w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
    w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
    w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
    w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
    w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
    w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
    w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
    w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
    w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
    w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
    w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
    w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
    w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
    w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
    w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
    w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
    w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
    w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
    w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
    w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
    w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
    w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
    w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
    w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
    w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
    w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16) }

/* macro for inverse s-box table */
#define isb_data(w) {\
    w(0x52), w(0x09), w(0x6a), w(0xd5), w(0x30), w(0x36), w(0xa5), w(0x38),\
    w(0xbf), w(0x40), w(0xa3), w(0x9e), w(0x81), w(0xf3), w(0xd7), w(0xfb),\
    w(0x7c), w(0xe3), w(0x39), w(0x82), w(0x9b), w(0x2f), w(0xff), w(0x87),\
    w(0x34), w(0x8e), w(0x43), w(0x44), w(0xc4), w(0xde), w(0xe9), w(0xcb),\
    w(0x54), w(0x7b), w(0x94), w(0x32), w(0xa6), w(0xc2), w(0x23), w(0x3d),\
    w(0xee), w(0x4c), w(0x95), w(0x0b), w(0x42), w(0xfa), w(0xc3), w(0x4e),\
    w(0x08), w(0x2e), w(0xa1), w(0x66), w(0x28), w(0xd9), w(0x24), w(0xb2),\
    w(0x76), w(0x5b), w(0xa2), w(0x49), w(0x6d), w(0x8b), w(0xd1), w(0x25),\
    w(0x72), w(0xf8), w(0xf6), w(0x64), w(0x86), w(0x68), w(0x98), w(0x16),\
    w(0xd4), w(0xa4), w(0x5c), w(0xcc), w(0x5d), w(0x65), w(0xb6), w(0x92),\
    w(0x6c), w(0x70), w(0x48), w(0x50), w(0xfd), w(0xed), w(0xb9), w(0xda),\
    w(0x5e), w(0x15), w(0x46), w(0x57), w(0xa7), w(0x8d), w(0x9d), w(0x84),\
    w(0x90), w(0xd8), w(0xab), w(0x00), w(0x8c), w(0xbc), w(0xd3), w(0x0a),\
    w(0xf7), w(0xe4), w(0x58), w(0x05), w(0xb8), w(0xb3), w(0x45), w(0x06),\
    w(0xd0), w(0x2c), w(0x1e), w(0x8f), w(0xca), w(0x3f), w(0x0f), w(0x02),\
    w(0xc1), w(0xaf), w(0xbd), w(0x03), w(0x01), w(0x13), w(0x8a), w(0x6b),\
    w(0x3a), w(0x91), w(0x11), w(0x41), w(0x4f), w(0x67), w(0xdc), w(0xea),\
    w(0x97), w(0xf2), w(0xcf), w(0xce), w(0xf0), w(0xb4), w(0xe6), w(0x73),\
    w(0x96), w(0xac), w(0x74), w(0x22), w(0xe7), w(0xad), w(0x35), w(0x85),\
    w(0xe2), w(0xf9), w(0x37), w(0xe8), w(0x1c), w(0x75), w(0xdf), w(0x6e),\
    w(0x47), w(0xf1), w(0x1a), w(0x71), w(0x1d), w(0x29), w(0xc5), w(0x89),\
    w(0x6f), w(0xb7), w(0x62), w(0x0e), w(0xaa), w(0x18), w(0xbe), w(0x1b),\
    w(0xfc), w(0x56), w(0x3e), w(0x4b), w(0xc6), w(0xd2), w(0x79), w(0x20),\
    w(0x9a), w(0xdb), w(0xc0), w(0xfe), w(0x78), w(0xcd), w(0x5a), w(0xf4),\
    w(0x1f), w(0xdd), w(0xa8), w(0x33), w(0x88), w(0x07), w(0xc7), w(0x31),\
    w(0xb1), w(0x12), w(0x10), w(0x59), w(0x27), w(0x80), w(0xec), w(0x5f),\
    w(0x60), w(0x51), w(0x7f), w(0xa9), w(0x19), w(0xb5), w(0x4a), w(0x0d),\
    w(0x2d), w(0xe5), w(0x7a), w(0x9f), w(0x93), w(0xc9), w(0x9c), w(0xef),\
    w(0xa0), w(0xe0), w(0x3b), w(0x4d), w(0xae), w(0x2a), w(0xf5), w(0xb0),\
    w(0xc8), w(0xeb), w(0xbb), w(0x3c), w(0x83), w(0x53), w(0x99), w(0x61),\
    w(0x17), w(0x2b), w(0x04), w(0x7e), w(0xba), w(0x77), w(0xd6), w(0x26),\
    w(0xe1), w(0x69), w(0x14), w(0x63), w(0x55), w(0x21), w(0x0c), w(0x7d) }

/* rc con table */
#define rc_data(w) {\
    w(0x01), w(0x02), w(0x04), w(0x08), w(0x10),w(0x20), w(0x40), w(0x80),\
    w(0x1b), w(0x36) }

/* table for ShiftRow */		/*  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 */
static cuint shiftrow_idx[] = { ((uint)0), ((uint)5), ((uint)10), ((uint)15),
((uint)4), ((uint)9), ((uint)14), ((uint)3),
((uint)8), ((uint)13), ((uint)2), ((uint)7),
((uint)12), ((uint)1), ((uint)6), ((uint)11) };
/* table for inverse ShiftRow*/
static cuint ishiftrow_idx[] = { ((uint)0), ((uint)13), ((uint)10), ((uint)7),
((uint)4), ((uint)1), ((uint)14), ((uint)11),
((uint)8), ((uint)5), ((uint)2), ((uint)15),
((uint)12), ((uint)9), ((uint)6), ((uint)3) };
/* rcon table for Key Expansion */
static cui08 rcon[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd, 0xab, 0x00 };
/* xtime^2 table */
static cui08  xt2[] = DATA256(xtime2);
/* s-box table for SubByte */
static cui08  sbox[] = sb_data(UI8_VAL);
/* Inverse s-box table for inverse SubByte */
static cui08  isbox[] = isb_data(UI8_VAL);

typedef struct tgmux
{
	uint    len;    /*暗号/復号電文長*/
	uint    mlo;    /*余り*/
	uint    pos;    /*暗号/復号位置*/
	pui08   cde;    /*暗号/復号電文*/
	ui08    tmp[LEN_BLOCK]; /*余り用*/
	ui08    cb[LEN_BLOCK];  /*initial counter block*/
	ui08    ath[LEN_BLOCK]; /*認証タグ*/
}MUX;
FNC     SECURITY;
GCM     gcm;                /* Global key */
static  MUX     *mux;
static  AES		tmp_key;                /* Master key*/
static  PGP     enc, dec;
static  GP      gp1;                /* encode aad */
#if (0==SW_AES)
static  ui16    sts_key;
static  ui16    sts_first;
uint    calc_max;
#define	DECODE	0x0008
#else
static  GP      gp2;                /* decode aad */
static  ui16 carry, gcm_t16;
#endif

static  pui08   secret;
static  pui08   secretQ;            /* Current secret0 */
static  pui08   scy_buf;            /* temporary buffer */
static  pui08   system_title;       /* iv(client) */
ui08    secret0[LEN_SECRET];        /* Master secret0 */
static  ui08    order_plane;

#if (0==SW_AES)
/****************************************************************/
/*																*/
/*	関数名	: aesclr											*/
/*																*/
/*	内容	: HW-AESGCM専用クリア関数							*/
/*																*/
/*	引　数1	: 初期化対象ポインタ(16ビット)						*/
/*																*/
/*	引　数2	: 個数(16ビット)									*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2019-01-19	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void aesclr(pui16 out, cui08 cnt)
{
	ui08 i;

	for (i = 0; i < cnt; i++)
	{
		*(out++) = 0x0000;
	}
}

/****************************************************************/
/*																*/
/*	関数名	: aescpy											*/
/*																*/
/*	内容	: HW-AESGCM専用コピー関数(16-16)					*/
/*																*/
/*	引　数1	: コピー先ポインタ(16ビット)						*/
/*																*/
/*	引　数2	: コピー元ポインタ(16ビット)						*/
/*																*/
/*	引　数3	: 個数(16ビット)									*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2019-01-19	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void aescpy(pui16 out, cpui16f in, cui08 cnt)
{
	ui08 i, pos;

	pos = cnt;
	for (i = 0; i < cnt; i++)
	{
		pos--;
		out[pos] = in[i];
	}
}

/****************************************************************/
/*																*/
/*	関数名	: aes8cpy											*/
/*																*/
/*	内容	: HW-AESGCM専用コピー関数(16-8)						*/
/*																*/
/*	引　数1	: コピー先ポインタ(16ビット)						*/
/*																*/
/*	引　数2	: コピー元ポインタ(8ビット)							*/
/*																*/
/*	引　数3	: 個数(16ビット)									*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2019-01-19	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void aes8cpy(pui16 out, cpui08 in, cui08 cnt)
{
	ui08 i, pos;
	union {
		ui16	s;
		ui08	c[2];
	}dat;

	pos = cnt;
	for (i = 0; i < cnt; i++)
	{
		dat.c[0] = *(in++);
		dat.c[1] = *(in++);
		out[--pos] = dat.s;
	}
}

static void aes8cpyf(pui16 out, cui08 __far * __far in, cui08 cnt)
{
	ui08 i, pos;
	union {
		ui16	s;
		ui08	c[2];
	}dat;

	pos = cnt;
	for (i = 0; i < cnt; i++)
	{
		dat.c[0] = *(in++);
		dat.c[1] = *(in++);
		out[--pos] = dat.s;
	}
}
/****************************************************************/
/*																*/
/*	関数名	: aes8cpy											*/
/*																*/
/*	内容	: HW-AESGCM専用コピー関数(8-16)						*/
/*																*/
/*	引　数1	: コピー先ポインタ(8ビット)							*/
/*																*/
/*	引　数2	: コピー元ポインタ(16ビット)						*/
/*																*/
/*	引　数3	: 個数(16ビット)									*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2019-01-19	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void aes16cpy(pui08 out, cpui16f in, cui08 cnt)
{
	ui08 i, pos;
	union {
		ui16	s;
		ui08	c[2];
	}dat;

	pos = cnt;
	for (i = 0; i < cnt; i++)
	{
		dat.s = in[--pos];
		*(out++) = dat.c[0];
		*(out++) = dat.c[1];
	}
}

static void calc_aesgcm(cui16 cnt)
{
	uint i;
	AESDCNTL.WORD = cnt;
	for (i = 0; AESSTSL.BIT.odata_valid == 0 || AESSTSL.BIT.first_odata == 0 || AESSTSL.BIT.busy == 1; i++);
	if (i > calc_max)
	{
		calc_max = i;
	}
}
#endif
/******************************************************/
/********************* AES ***************************/
/******************************************************/
/****************************************************************/
/*																*/
/*	関数名	: SubBytes_ShiftRows								*/
/*																*/
/*	内容	: s-box置き換え+行シフト							*/
/*																*/
/*	引　数1	: 出力ブロック(16バイト)							*/
/*																*/
/*	引　数2	: 入力ブロック(16バイト)							*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
/* SubBytes and ShiftRow Function*/
void SubBytes_ShiftRows(pui08 out, cpui08 in)
{
	ui08	i, tmp[LEN_BLOCK];

	ramcpy(tmp, in, LEN_BLOCK);
	for (i = 0; i < LEN_BLOCK; i++)
	{
		*(out++) = sbox[tmp[shiftrow_idx[i]]];
	}
}

/****************************************************************/
/*																*/
/*	関数名	: iSubBytes_iShiftRows								*/
/*																*/
/*	内容	: 逆s-box置き換え+逆行シフト						*/
/*																*/
/*	引　数1	: 出力ブロック(16バイト)							*/
/*																*/
/*	引　数2	: 入力ブロック(16バイト)							*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void iShiftRows_iSubBytes(pui08 out, cpui08 in)
{
	ui08	i, tmp[LEN_BLOCK];

	ramcpy(tmp, in, LEN_BLOCK);
	for (i = 0; i < LEN_BLOCK; i++)
	{
		*(out++) = isbox[tmp[ishiftrow_idx[i]]];
	}
}

/****************************************************************/
/*																*/
/*	関数名	: RotWord											*/
/*																*/
/*	内容	: 列ローテーション	1								*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void RotWord(pui08 out, cpui08 in)
{
	out[0] = in[1];
	out[1] = in[2];
	out[2] = in[3];
	out[3] = in[0];
}

/****************************************************************/
/*																*/
/*	関数名	: RotWord2											*/
/*																*/
/*	内容	: 列ローテーション2									*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void RotWord2(pui08 out, cpui08 in)
{
	out[0] = in[2];
	out[1] = in[3];
	out[2] = in[0];
	out[3] = in[1];
}
#if 0
/****************************************************************/
/*																*/
/*	関数名	: iRotWord											*/
/*																*/
/*	内容	: 列逆ローテーション1								*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void iRotWord(pui08 out, cpui08 in)
{
	out[0] = in[3];
	out[1] = in[0];
	out[2] = in[1];
	out[3] = in[2];
}
#endif
/****************************************************************/
/*																*/
/*	関数名	: MixColumns										*/
/*																*/
/*	内容	: 列混ぜ											*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void MixColumns(pui08 out, cpui08 in)
{
	ui08 xor, rot[4];

	out[0] = xt2[in[0]];
	out[1] = xt2[in[1]];
	out[2] = xt2[in[2]];
	out[3] = xt2[in[3]];

	RotWord(rot, out);

	xor = in[0] ^ in[1] ^ in[2] ^ in[3];

	out[0] ^= rot[0] ^ in[0] ^ xor;
	out[1] ^= rot[1] ^ in[1] ^ xor;
	out[2] ^= rot[2] ^ in[2] ^ xor;
	out[3] ^= rot[3] ^ in[3] ^ xor;
}
/****************************************************************/
/*																*/
/*	関数名	: iMixColums										*/
/*																*/
/*	内容	: 列逆混ぜ											*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void iMixColums(pui08 out, cpui08 in)
{
	ui08	i;
	ui08	t[2][4], rot[4], xt81 = 0;

	for (i = 0; i < 4; i++)
	{
		t[0][i] = xt2[in[i]];				/* xt2 */
		t[1][i] = xt2[t[0][i]];			/* xt4 */
		xt81 ^= xt2[t[1][i]] ^ in[i];
	}

	RotWord(rot, t[0]);
	out[0] = t[0][0] ^ rot[0] ^ in[0] ^ xt81;
	out[1] = t[0][1] ^ rot[1] ^ in[1] ^ xt81;
	out[2] = t[0][2] ^ rot[2] ^ in[2] ^ xt81;
	out[3] = t[0][3] ^ rot[3] ^ in[3] ^ xt81;

	RotWord2(rot, t[1]);
	out[0] ^= t[1][0] ^ rot[0];
	out[1] ^= t[1][1] ^ rot[1];
	out[2] ^= t[1][2] ^ rot[2];
	out[3] ^= t[1][3] ^ rot[3];
	/* 0x0e, 0x0b, 0x0d, 0x09 */
}

/****************************************************************/
/*																*/
/*	関数名	: SubBytes_ShiftRows_MixColums						*/
/*																*/
/*	内容	: ibox置き換え+行シフト+列混ぜ						*/
/*																*/
/*	引　数1	: 出力ブロック(16バイト)							*/
/*																*/
/*	引　数2	: 入力ブロック(16バイト)							*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void SubBytes_ShiftRows_MixColums(pui08 out, cpui08 in)
{
	ui08	i;
	ui08	tmp[4];

	for (i = 0; i < LEN_BLOCK; i += 4)
	{
		tmp[0] = sbox[in[shiftrow_idx[i + 0]]];
		tmp[1] = sbox[in[shiftrow_idx[i + 1]]];
		tmp[2] = sbox[in[shiftrow_idx[i + 2]]];
		tmp[3] = sbox[in[shiftrow_idx[i + 3]]];
		MixColumns(&out[i], tmp);
	}
}
/****************************************************************/
/*																*/
/*	関数名	: SubWord											*/
/*																*/
/*	内容	: s-box置き換え										*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void SubWord(pui08 out, cpui08 in)
{
	out[0] = sbox[in[0]];
	out[1] = sbox[in[1]];
	out[2] = sbox[in[2]];
	out[3] = sbox[in[3]];
}
#if 0
/****************************************************************/
/*																*/
/*	関数名	: iSubWord											*/
/*																*/
/*	内容	: s-box逆置き換え									*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void iSubWord(pui08 out, cpui08 in)
{
	out[0] = isbox[in[0]];
	out[1] = isbox[in[1]];
	out[2] = isbox[in[2]];
	out[3] = isbox[in[3]];
}
#endif
/****************************************************************/
/*																*/
/*	関数名	: KeyExpansion										*/
/*																*/
/*	内容	: 拡張鍵生成										*/
/*																*/
/*	引　数1	: AES鍵												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void KeyExpansion(PAES aes)
{
	uint	i, j;
	ui08	a[4], b[4];

	for (i = 0; i < aes->nk * 4; i++)
	{
		aes->ek[i] = aes->ky[i];
	}
	for (i = aes->nk; i < (aes->nr + 1) * 4; i++)
	{
		j = (i - 1) * 4;
		if (0 == (i % aes->nk))
		{
			RotWord(a, &aes->ek[j]);
			SubWord(b, a);
			b[0] ^= rcon[i / aes->nk - 1];
		}
		else if ((6 < aes->nk) && (4 == (i%aes->nk)))
		{
			SubWord(b, &aes->ek[j]);
		}
		else
		{
			b[0] = aes->ek[j + 0];
			b[1] = aes->ek[j + 1];
			b[2] = aes->ek[j + 2];
			b[3] = aes->ek[j + 3];
		}
		j = (i - aes->nk) * 4;
		aes->ek[i * 4 + 0] = aes->ek[j + 0] ^ b[0];
		aes->ek[i * 4 + 1] = aes->ek[j + 1] ^ b[1];
		aes->ek[i * 4 + 2] = aes->ek[j + 2] ^ b[2];
		aes->ek[i * 4 + 3] = aes->ek[j + 3] ^ b[3];
	}
}

/****************************************************************/
/*																*/
/*	関数名	: xor_block											*/
/*																*/
/*	内容	: ブロック加算										*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void xor_block(pui16 out, cpui16f a, cpui16f b)
{
	ui08	i;

	for (i = 0; i < LEN_BLOCK / sizeof(ui16); i++)
	{
		*(out++) = (*(a++)) ^ (*(b++));
	}
}

/****************************************************************/
/*																*/
/*	関数名	: set_aes											*/
/*																*/
/*	内容	: AES用鍵の初期設定									*/
/*																*/
/*	引　数1	: AES鍵												*/
/*																*/
/*	引　数2	: 鍵												*/
/*																*/
/*	引　数3	: 鍵長												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void set_aes(PAES aes, cpui08 key_in, cui08 len)
{
	/* 鍵長から、ブロック数とラウンド回数の保存 */
	/* 128bit = 10, 192bit = 12, 256bit = 14 */
	aes->nk = (len >> 2);		/* 鍵のブロック数	nk = 4個/6個/8個 */
	aes->nr = aes->nk + 6;	/* ラウンド回数	nr = 4個=16回/6個=24回/8個=32回 */
							/* 鍵のコピー */
	aes->ky = (pui08)key_in;
	/* 拡張鍵の生成 */
	KeyExpansion(aes);
}
/****************************************************************/
/*																*/
/*	関数名	: Encrypt											*/
/*																*/
/*	内容	: AES暗号											*/
/*																*/
/*	引　数1	: 入出力ブロック(16バイト)							*/
/*																*/
/*	引　数2	: AES鍵												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void Encrypt(pui16 data, CPAES aes)
{
	uint	i;
	ui16	ZTMP[LEN_BLOCK / sizeof(ui16)], VTMP[LEN_BLOCK / sizeof(ui16)];

	xor_block(ZTMP, data, (pui16)&aes->ek[0 * 16]);
	for (i = 1; i < aes->nr; i++)
	{
		SubBytes_ShiftRows_MixColums((pui08)VTMP, (pui08)ZTMP);
		xor_block(ZTMP, VTMP, (pui16)&aes->ek[i * 16]);
	}
	SubBytes_ShiftRows((pui08)VTMP, (pui08)ZTMP);
	xor_block(data, VTMP, (pui16)&aes->ek[i * 16]);
}

/****************************************************************/
/*																*/
/*	関数名	: Decrypt											*/
/*																*/
/*	内容	: AES復号											*/
/*																*/
/*	引　数1	: 入出力ブロック									*/
/*																*/
/*	引　数2	: AES鍵												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void Decrypt(pui16 data, CPAES aes)
{
	uint	i;
	ui08	a[LEN_BLOCK], b[LEN_BLOCK];

	xor_block((pui16)a, data, (pui16)&aes->ek[aes->nr * 16]);
	for (i = aes->nr - 1; i > 0; i--)
	{
		iShiftRows_iSubBytes(b, a);
		xor_block((pui16)b, (pui16)b, (pui16)&aes->ek[i * 16]);
		iMixColums(&a[0], &b[0]);
		iMixColums(&a[4], &b[4]);
		iMixColums(&a[8], &b[8]);
		iMixColums(&a[12], &b[12]);
	}
	iShiftRows_iSubBytes(b, a);
	xor_block(data, (pui16)b, (pui16)aes->ek);
}

/******************************************************/
/******************* AES GCM *************************/
/******************************************************/
/****************************************************************/
/*																*/
/*	関数名	: gcm_inc_128_32									*/
/*																*/
/*	内容	: 列インクリメント									*/
/*																*/
/*	引　数1	: 入力列(4バイト)									*/
/*																*/
/*	戻り値	: 出力列(4バイト)									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void gcm_inc_128_32(pui08 out, cpui08 in)
{
	ui32	inc;

	inc = UI32L(in);
	inc++;
	ENDCPY4(out, inc);
}


#if (0==SW_AES)
/****************************************************************/
/*																*/
/*	関数名	: get_aescnt										*/
/*																*/
/*	内容	: HW-AESGCMデータコントローラ						*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: 指令値											*/
/*																*/
/*	作成日	: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当:							*/
/*																*/
/****************************************************************/
static ui16 get_aescnt(void)
{
	ui16	aescnt;

	aescnt = 0x0000;
	aescnt |= sts_key;
	aescnt |= sts_first;
	sts_key = 0x0000;
	sts_first = 0x0000;

	return aescnt;
}
#else	/* (0==SW_AES)*/
/****************************************************************/
/*																*/
/*	関数名	: gcm_shift_block									*/
/*																*/
/*	内容	: ブロックシフト									*/
/*																*/
/*	引　数1	: 出力列											*/
/*																*/
/*	引　数2	: 入力列											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void gcm_shift_block(pui16 io)
{
	ui08	i;

	carry = 0;
	for (i = 0; i < LEN_BLOCK / sizeof(ui16); i++)
	{
		gcm_t16 = (io[i] << 8) + (io[i] >> 8);
		gcm_t16 >>= 1;
		if (0 < carry)
		{
			gcm_t16 |= 0x8000;		/*桁あふれ処理*/
		}
		carry = io[i] & 0x0100;	/*下位ビット取得*/
		io[i] = (gcm_t16 << 8) + (gcm_t16 >> 8);
	}
}

/****************************************************************/
/*																*/
/*	関数名	: gcm_mul128										*/
/*																*/
/*	内容	: ブロック乗算										*/
/*																*/
/*	引　数1	: 出力列											*/
/*																*/
/*	引　数2	: 入力列											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void gcm_mul128(pui08 out, cpui08 hash0)
{
	ui08	i;
	ui08	p;
	ui16	ZTMP[LEN_BLOCK / sizeof(ui16)], VTMP[LEN_BLOCK / sizeof(ui16)];
	pui08	q;

	ramcpy((pui08)VTMP, hash0, LEN_BLOCK);	/*copy_byte16( &in2->e[0], &VTMP.e[0]);*/
	ramset((pui08)ZTMP, 0, LEN_BLOCK);		/*BLOCK_INIT(&ZTMP);*/
	q = out;

	for (i = 0; i < 128; i++, p <<= 1)
	{
		if (0 == (i % 8))
		{
			p = *q;
			q++;
		}
		if (0 < (p & 0x80))
		{
			xor_block(ZTMP, ZTMP, VTMP);	/* ZTMPとVTMPのXOR */
		}
		else
		{
			/*何もしない*/
		}
		if (0 < (VTMP[7] & 0x0100))
		{
			gcm_shift_block(VTMP);
			VTMP[0] ^= 0x00e1; /*11100001 || [0]120.*/
		}
		else
		{
			gcm_shift_block(VTMP);
		}
	}
	ramcpy(out, (pui08)ZTMP, LEN_BLOCK);
}
#endif 	/* (0==SW_AES)*/
/****************************************************************/
/*																*/
/*	関数名	: gcm_gctr											*/
/*																*/
/*	内容	: カウンター合成								*/
/*																*/
/*	引　数1	: 出力列											*/
/*																*/
/*	引　数2	: 出力列長											*/
/*																*/
/*	引　数3	: カウンターブロック								*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
#if SW_AES
static void gcm_gctr(pui08 ctr, cui08 len, cpui08 cb)	/* initial counter block */
{
	ui16	cipher_cb[LEN_BLOCK / sizeof(ui16)], tmp[LEN_BLOCK / sizeof(ui16)];

	ramcpy((pui08)tmp, ctr, len);
	ramcpy((pui08)cipher_cb, cb, LEN_BLOCK);

	/* cipher counter Block */
	Encrypt(cipher_cb, &gcm.ae);

	/* cipher ctr with icb */
	xor_block(tmp, tmp, cipher_cb);

	ramcpy(ctr, (pui08)tmp, len);
}
#else
static void gcm_gctr(pui08 ctr, cuint len, cpui08 cb)		/* initial counter block */
{
	ui16	tmp[8], aescnt;

	aesclr(tmp, 8);
	ramcpy((pui08)tmp, ctr, len);
	aescpy((pui16)AESIDAT.BLOCK, tmp, 8);
	aescnt = get_aescnt();
	if ((mux->pos + len) >= mux->len)
	{
		aescnt |= 0x0033;
	}
	else
	{
		aescnt |= 0x0023;
	}
	if (len != 16)
	{
		aescnt += len << 11;
	}
	calc_aesgcm(aescnt);
	aescpy(tmp, (pui16)AESODAT.BLOCK, 8);
	ramcpy(ctr, (pui08)tmp, len);
}
#endif
/****************************************************************/
/*																*/
/*	関数名	: gcm_hash											*/
/*																*/
/*	内容	: GHASH計算											*/
/*																*/
/*	引　数1	: ハッシュブロック(16バイト)						*/
/*																*/
/*	引　数2	: 入力ブロック(16バイト)							*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
#if SW_AES
static void gcm_hash(pui08 hash, cpui08 in)
{
	ui16	tmp[LEN_BLOCK / sizeof(ui16)];

	ramcpy((pui08)tmp, in, 16);

	/* xor */
	xor_block((pui16)hash, (pui16)hash, tmp);

	/* mul */
	gcm_mul128(hash, gcm.h0);
}
#else
static void gcm_hash(pui16 hash, cpui08 in)
{
	aes8cpy((pui16)AESIDAT.BLOCK, in, 8);
	calc_aesgcm(0x0003);
	aescpy(hash, (pui16)AESODAT.BLOCK, 8);
}

void gcm_hash0(cpui08 in, cui08 size)
{
	ui16	tmp[8];

	if (size == 16) {
		aes8cpy((pui16)AESIDAT.BLOCK, in, 8);
	}
	else {
		aesclr(tmp, 8);
		ramcpy((pui08)tmp, in, size);
		aescpy((pui16)AESIDAT.BLOCK, tmp, 8);
	}
	calc_aesgcm(0x000f);
}
void gcm_hash0f(cpui08f in, cui08 size)
{
	ui16	tmp[8];

	if (size == 16) {
		aes8cpyf((pui16)AESIDAT.BLOCK, in, 8);
	}
	else {
		aesclr(tmp, 8);
		romcpy((pui08)tmp, (cpui08f)in, size);
		aescpy((pui16)AESIDAT.BLOCK, tmp, 8);
	}
	calc_aesgcm(0x000f);
}

void gcm_hash1(cpui08 in, cui08 size)
{
	ui16	tmp[8];

	if (size == 16) {
		aes8cpy((pui16)AESIDAT.BLOCK, in, 8);
	}
	else {
		aesclr(tmp, 8);
		ramcpy((pui08)tmp, in, size);
		aescpy((pui16)AESIDAT.BLOCK, tmp, 8);
	}
	calc_aesgcm(0x0003);
}

void gcm_hash1f(cpui08f in, cui08 size)
{
	ui16	tmp[8];

	if (size == 16) {
		aes8cpyf((pui16)AESIDAT.BLOCK, in, 8);
	}
	else {
		aesclr(tmp, 8);
		romcpy((pui08)tmp, (cpui08f)in, size);
		aescpy((pui16)AESIDAT.BLOCK, tmp, 8);
	}
	calc_aesgcm(0x0003);
}

void gcm_hash2(pui16 hash)
{
	aescpy(hash, (pui16)AESODAT.BLOCK, 8);
}

#endif

/****************************************************************/
/*																*/
/*	関数名	: gcm_hash_arr										*/
/*																*/
/*	内容	: GHASH生成											*/
/*																*/
/*	引　数1	: ハッシュ値(16バイト)								*/
/*																*/
/*	引　数2	: 入力列											*/
/*																*/
/*	引　数3	: 入力列長											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
#if SW_AES
void gcm_hash_arr(pui08 hash, cpui08 in, cuint len)
{
	uint	i, cnt;
	ui08	tmp[LEN_BLOCK];

	cnt = len >> 4;
	/* 初期認証タグの生成 */
	for (i = 0; i < cnt; i++)
	{
		gcm_hash(hash, &in[i*LEN_BLOCK]);
	}

	if (0 < (len%LEN_BLOCK))
	{
		ramset(tmp, 0, LEN_BLOCK);
		ramcpy(tmp, &in[i*LEN_BLOCK], UI8_VAL(len%LEN_BLOCK));
		gcm_hash(hash, tmp);
	}
}
#else
void gcm_hash_arr(pui16 hash, cpui08 in, cuint len)
{
	uint	i, remain;
	ui16	tmp[8], aescnt;

	/* 初期認証タグの生成 */
	for (i = 0; i < len;)
	{
		remain = len - i;
		if (remain > LEN_BLOCK)
		{
			remain = LEN_BLOCK;
		}
		aesclr(tmp, 8);
		ramcpy((pui08)tmp, &in[i], remain);
		aescpy((pui16)AESIDAT.BLOCK, tmp, 8);
		if (i == 0)
		{
			aescnt = 0x000f;
		}
		else
		{
			aescnt = 0x0003;
		}
		calc_aesgcm(aescnt);
		i += remain;
	}
	if (hash != NULL)
	{
		aescpy(hash, (pui16)AESODAT.BLOCK, 8);
	}
}
#endif

/****************************************************************/
/*																*/
/*	関数名	: delete_gcm										*/
/*																*/
/*	内容	: GCM鍵削除											*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
void delete_gcm(void)
{
#if SW_AES
	ramset((pui08)&gcm.ae, 0, sizeof(AES));		/*initialize master key*/
	ramset((pui08)&gcm.h0, 0, LEN_BLOCK);
#else
	AESPRR2.BIT.AESRES = 1;
	for (AESCNT.BLOCK[1] = 0x8000; AESCNT.BLOCK[1] == 0x8000;);
	AESPRR2.BIT.AESRES = 0;
#endif
	ramset((pui08)&gcm.j0, 0, LEN_BLOCK);
}

#if SW_AES
/****************************************************************/
/*																*/
/*	関数名	: set_gcm											*/
/*																*/
/*	内容	: GCM鍵の初期設定									*/
/*																*/
/*	引　数1	: GCM鍵												*/
/*																*/
/*	引　数2	: 鍵	列											*/
/*																*/
/*	引　数3	: 鍵列長											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void set_gcm(PGCM key, cpui08 key_in, cui08 key_len)
{

	set_aes(&key->ae, key_in, key_len);
	/*hash 0 の生成 */
	ramset((pui08)key->h0, 0, LEN_BLOCK);
	Encrypt((pui16)key->h0, &key->ae);
}
#endif
/****************************************************************/
/*																*/
/*	関数名	: set_aad											*/
/*																*/
/*	内容	: AAD認証タグ初期設定								*/
/*																*/
/*	引　数1	: 認証タグ											*/
/*																*/
/*	引　数2	: AAD列												*/
/*																*/
/*	引　数3	: AAD列長											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void set_aad(PGP gp, cpui08 ath, cuint len)
{
	ui32	len_bit;

	ramset((pui08)gp, 0, sizeof(GP));
#if (SW_AES == 0)
	sts_first = 0x0004;
#endif
	if (0 < len)
	{
		len_bit = len << 3;					/* byte to bit */
#if SW_AES
											/*ハッシュの生成*/
		gcm_hash_arr(gp->h, ath, len);
		/* 認証子のビット長さを保存 */
		ENDCPY4((pui08)&gp->l[4], len_bit);			/*ビット長に変換 */
#else
											/*ハッシュの生成*/
		gcm_hash_arr(NULL, ath, len);
		sts_first = 0x0000;
		ENDCPY4((pui08)&gp->l[2], len_bit);			/*ビット長に変換 */
#endif
	}
	else
	{
		/* Fatal : invalid pass */
	}
}

/****************************************************************/
/*																*/
/*	関数名	: set_aad_key										*/
/*																*/
/*	内容	: AAD認証タグ生成									*/
/*																*/
/*	引　数1	: 認証タグ											*/
/*																*/
/*	引　数2	: AADオフセット										*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static void set_aad_key(PGP gp, cuint pos) {
	uint	l = 0;
	ui08	len;
	ui08	key[LEN_KEY_INF];

	len = read_auth_key(key, g_userQ);

	ramset(scy_buf, 0, 32);
	/*SCの保存*/
	scy_buf[0] = security_control;
	l += LEN_SC;
	/*AKの保存*/
	ramcpy(&scy_buf[l], key, len);
	l += len;
	/* AAD-ghashの生成 */
	set_aad(gp, scy_buf, l);
}

/****************************************************************/
/*																*/
/*	関数名	: set_aad_key_e										*/
/*																*/
/*	内容	: 暗号用AAD認証タグ生成								*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 201x-xx-xx  	担当: 								*/
/*																*/
/****************************************************************/
void set_aad_key_e(void)
{
	set_aad_key(enc, OFFSET_ATOB);
}

/****************************************************************/
/*																*/
/*	関数名	: set_aad_key_d										*/
/*																*/
/*	内容	: 復号用AAD認証タグ生成								*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_aad_key_d(void)
{
	set_aad_key(dec, OFFSET_BTOA);
}

/****************************************************************/
/*																*/
/*	関数名	: set_AtoB											*/
/*																*/
/*	内容	: チャレンジコード保存								*/
/*																*/
/*	引　数1	: チャレンジコード列								*/
/*																*/
/*	引　数2	: チャレンジコード長								*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_AtoB(cpui08 in, cui08 len)
{
	secretQ[OFFSET_ATOB] = len;
	ramcpy(&secretQ[OFFSET_ATOB + 1], in, len);
}

/****************************************************************/
/*																*/
/*	関数名	: get_graphic_char									*/
/*																*/
/*	内容	: GraphicString変換									*/
/*																*/
/*	引　数1	: 入力値											*/
/*																*/
/*	戻り値	: 出力値											*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
static ui08 get_graphic_char(cuint in)
{
	uint res;

	if (in < 0x0080)
	{
		res = in % 0x5e;
		res += 0x20;
	}
	else
	{
		res = in - 0x80;
		res %= 0x3e;
		res += 0xa1;
	}
	return ((ui08)res);
}

/****************************************************************/
/*																*/
/*	関数名	: set_BtoA											*/
/*																*/
/*	内容	: チャレンジ保存生成								*/
/*																*/
/*	引　数1	: チャレンジコード列								*/
/*																*/
/*	引　数2	: チャレンジコード長								*/
/*														 		*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_BtoA(cpui08f in, cui08 len)
{
	uint i;
	ui08 j;

	secretQ[OFFSET_BTOA] = len;
	romcpy(&secretQ[OFFSET_BTOA + 1], in, len);
	secretQ[OFFSET_ATOB] = len;
	for (i = 0; i < len;) {
		challenge(&scy_buf[i]);
		for (j = 0; j<16; j++)
		{
			scy_buf[i] = get_graphic_char(scy_buf[i]);
			i++;
		}
	}
	ramcpy(&secretQ[OFFSET_ATOB + 1], scy_buf, len);
}

/****************************************************************/
/*																*/
/*	関数名	: get_AtoB											*/
/*																*/
/*	内容	: チャレンジコード取得								*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: チャレンジコード									*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
pui08 get_AtoB(void)
{
	return &secretQ[OFFSET_ATOB + 1];
}

/****************************************************************/
/*																*/
/*	関数名	: set_aad_gmac										*/
/*																*/
/*	内容	: GMAC初期設定										*/
/*																*/
/*	引　数1	: チャレンジコードオフセット						*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_aad_gmac(cuint pos)
{
	uint	l;
	ui08	len, key[LEN_KEY_INF];

	len = read_auth_key(key, g_userQ);

	ramset(scy_buf, 0, 32);
#if (0==SW_AES)
	AESCNT.BLOCK[0] &= (~DECODE);
#endif
	l = 0;
	scy_buf[l] = 0x10;
	l += LEN_SC;
	/*AKの保存*/
	ramcpy(&scy_buf[l], key, len);
	l += len;
	ramcpy(&scy_buf[l], &secret[pos + 1], secret[pos]);
	l += secret[pos];
	/* hash */
	set_aad(enc, scy_buf, l);
}

/****************************************************************/
/*																*/
/*	関数名	: set_aad_gmac_a									*/
/*																*/
/*	内容	: A側GMAC初期設定									*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_aad_gmac_a(void)
{
	set_aad_gmac(OFFSET_ATOB);
}

/****************************************************************/
/*																*/
/*	関数名	: set_aad_gmac_b									*/
/*																*/
/*	内容	: B側GMAC初期設定									*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_aad_gmac_b(void)
{
	set_aad_gmac(OFFSET_BTOA);
}

/****************************************************************/
/*																*/
/*	関数名	: set_ive											*/
/*																*/
/*	内容	: IV設定											*/
/*																*/
/*	引　数1	: IV配列											*/
/*																*/
/*	引　数2	: IV長												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void set_ive(cpui08 ive, cuint len)
{
	ui32	len_bit;

	ramset((pui08)gcm.j0, 0, LEN_BLOCK);
	if (0 < len)
	{
		/*Define a block, J0, as follows:*/
		if (12 == len)
		{
			ramcpy((pui08)gcm.j0, ive, len);
#if SW_AES
			/*If len(IV)=96, then let J0 = IV || 031 ||1.*/
			gcm.j0[12] = 0x00;	/* j0のセット */
			gcm.j0[13] = 0x00;	/* j0のセット */
			gcm.j0[14] = 0x00;	/* j0のセット */
			gcm.j0[15] = 0x01;	/* j0のセット */
#else /* SW_AES */
			/*If len(IV)=96, then let J0 = IV || 031 ||1.*/
			gcm.j0[6] = 0x0000;	/* j0のセット */
			gcm.j0[7] = 0x0100;	/* j0のセット */
#endif /* SW_AES */
		}
		else
		{
			/* If len(IV) not equal 96, then let s = 128 [len(IV)/128]-len(IV), and let */
			/* J0=GHASHH(IV||0s+64||[len(IV)]64). */
			gcm_hash_arr(gcm.j0, ive, len);			/* */
			ramset(scy_buf, 0, LEN_SCY_BUF);
			len_bit = len << 3;
			ENDCPY4(&scy_buf[12], len_bit);	/*ビット長に変換 */
			gcm_hash(gcm.j0, scy_buf);				/* j0のセット */
		}
	}
	else
	{
		/* Fatal : invalid pass */
	}
	/* 初期カウンタブロックの保存 */
	ramcpy(mux->cb, (pui08)gcm.j0, LEN_BLOCK);
	/* カウンタブロックのインクリメント */
	gcm_inc_128_32(&mux->cb[12], &mux->cb[12]);
#if (0==SW_AES)
	aescpy((pui16)AESIV.BLOCK, (pui16)mux->cb, 8);
#endif
}


/****************************************************************/
/*																*/
/*	関数名	: start_AES_GCM										*/
/*																*/
/*	内容	: AES-GCM初期設定									*/
/*																*/
/*	引　数1	: 認証タグ											*/
/*																*/
/*	引　数2	: 暗号/復号対象列									*/
/*																*/
/*	引　数3	: 配列長											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void start_AES_GCM(PGP gp, pui08 data, cuint len)
{
	ui32	len_bit;

	mux->len = len;
	mux->pos = 0;
	mux->cde = data;
	mux->mlo = len%LEN_BLOCK;
	if (0 < mux->mlo) {
		ramset(mux->tmp, 0, LEN_BLOCK);
	}
	len_bit = mux->len << 3;
#if SW_AES
	ENDCPY4((pui08)&gp->l[12], len_bit);		/*ビット長に変換 */
												/* 初期認証タグの保存 */
	ramcpy(mux->ath, gp->h, LEN_BLOCK);
#else
	ENDCPY4((pui08)&gp->l[6], len_bit);		/*ビット長に変換 */
#endif
}

/****************************************************************/
/*																*/
/*	関数名	: finish_iAES_GCM									*/
/*																*/
/*	内容	: AES-GCM復号後処理									*/
/*																*/
/*	引　数1	: 認証タグ											*/
/*																*/
/*	引　数2	: 認証タグ長											*/
/*																*/
/*	戻り値	: 認証タグ一致(TRUE)/不一致(FALSE)						*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void finish_iAES_GCM(void)
{
#if SW_AES
	gcm_hash(mux->ath, dec->l);
	/* 認証タグと初期カウンタブロックとのxor */
	gcm_gctr(mux->ath, 16, gcm.j0);
#else
	ui16	aescnt;

	aescnt = get_aescnt();
	aescnt |= 0x0043;
	aescpy((pui16)AESIV.BLOCK, gcm.j0, 8);
	aescpy((pui16)AESIDAT.BLOCK, dec->l, 8);
	calc_aesgcm(aescnt);
	aes16cpy(mux->ath, (pui16)AESODAT.BLOCK, 8);
#endif
}

/****************************************************************/
/*																*/
/*	関数名	: finish_AES_GCM									*/
/*																*/
/*	内容	: AES-GCM暗号後処理									*/
/*																*/
/*	引　数1	: 認証タグ											*/
/*																*/
/*	引　数2	: 認証タグ長											*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void finish_AES_GCM(void)
{

#if SW_AES
	gcm_hash(mux->ath, enc->l);
	/* 認証タグと初期カウンタブロックとのxor */
	gcm_gctr(mux->ath, 16, gcm.j0);
#else
	ui16	aescnt;

	aescnt = get_aescnt();
	aescnt |= 0x0043;
	aescpy((pui16)AESIV.BLOCK, gcm.j0, 8);
	aescpy((pui16)AESIDAT.BLOCK, enc->l, 8);
	calc_aesgcm(aescnt);
	aes16cpy(mux->ath, (pui16)AESODAT.BLOCK, 8);
#endif
}

/****************************************************************/
/*																*/
/*	関数名	: iAES_GCM											*/
/*																*/
/*	内容	: AES-GCM復号										*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void iAES_GCM(void)
{
	if ((mux->pos + LEN_BLOCK) <= mux->len)
	{
#if SW_AES
		/* GHASH */
		gcm_hash(mux->ath, &mux->cde[mux->pos]);
#endif
		/* 復号 */
		gcm_gctr(&mux->cde[mux->pos], LEN_BLOCK, mux->cb);
		mux->pos += LEN_BLOCK;
	}
	else
	{
#if SW_AES
		/* GHASH */
		ramcpy(mux->tmp, &mux->cde[mux->pos], mux->mlo);
		gcm_hash(mux->ath, mux->tmp);
#endif
		/* 復号 */
		gcm_gctr(&mux->cde[mux->pos], mux->mlo, mux->cb);
		mux->mlo = (mux->mlo == 0) ? 1 : mux->mlo;
		mux->pos += mux->mlo;
	}
#if SW_AES
	/* カウンタブロックのインクリメント */
	gcm_inc_128_32(&mux->cb[12], &mux->cb[12]);
#endif
}

/****************************************************************/
/*																*/
/*	関数名	: AES_GCM											*/
/*																*/
/*	内容	: AES-GCM暗号										*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void AES_GCM(void) {
	if ((mux->pos + LEN_BLOCK) <= mux->len) {
		/* 暗号 */
		gcm_gctr(&mux->cde[mux->pos], LEN_BLOCK, mux->cb);
#if SW_AES
		/* GHASH */
		gcm_hash(mux->ath, &mux->cde[mux->pos]);
#endif
		mux->pos += LEN_BLOCK;
	}
	else {
		/* 暗号 */
		gcm_gctr(&mux->cde[mux->pos], mux->mlo, mux->cb);
#if SW_AES
		/* GHASH */
		ramcpy(mux->tmp, &mux->cde[mux->pos], mux->mlo);
		gcm_hash(mux->ath, mux->tmp);
#endif
		mux->mlo = (mux->mlo == 0) ? 1 : mux->mlo;
		mux->pos += mux->mlo;
	}
#if SW_AES
	/* カウンタブロックのインクリメント */
	gcm_inc_128_32(&mux->cb[12], &mux->cb[12]);
#endif
}

static void stream_AES_GCM(void) {

	if (mux->pos < g_xdlms_res_len) {
		gcm_gctr(&mux->cde[mux->pos], LEN_BLOCK, mux->cb);
		stream1_write(&mux->cde[mux->pos], LEN_BLOCK);
		mux->pos += LEN_BLOCK;
	}
	else {
		if ((mux->pos + LEN_BLOCK) <= mux->len) {
			stream0_read(&mux->cde[g_xdlms_res_len], LEN_BLOCK);
			gcm_gctr(&mux->cde[g_xdlms_res_len], LEN_BLOCK, mux->cb);
			stream1_write(&mux->cde[g_xdlms_res_len], LEN_BLOCK);
			mux->pos += LEN_BLOCK;
		}
		else {
			/* 暗号 */
			stream0_read(&mux->cde[g_xdlms_res_len], mux->mlo);
			gcm_gctr(&mux->cde[g_xdlms_res_len], mux->mlo, mux->cb);
			stream1_write(&mux->cde[g_xdlms_res_len], mux->mlo);
			mux->mlo = (mux->mlo == 0) ? 1 : mux->mlo;
			mux->pos += mux->mlo;
		}
	}
}
/****************************************************************/
/*																*/
/*	関数名	: iAES_GCM_arr										*/
/*																*/
/*	内容	: 一括AES-GCM復号									*/
/*																*/
/*	引　数1	: 復号対象列											*/
/*																*/
/*	引　数2	: 復号対象列長										*/
/*																*/
/*	引　数3	: 認証タグ長											*/
/*																*/
/*	戻り値	: 認証タグ一致(TRUE)/不一致(FALSE)						*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
ui08 iAES_GCM_arr(pui08 cde, cuint len, cui08 len_tag)
{
	ui08 res = TRUE;

	start_AES_GCM(dec, cde, len);
	for (; mux->pos < mux->len; ) iAES_GCM();
	finish_iAES_GCM();
	if ((security_control & 0x10) > 0) {
		if (ramcmp(mux->ath, &mux->cde[mux->len], len_tag))
		{
			/* Error : incorrect authenticate tag */
			res = FALSE;
		}
	}
	return res;
}

/****************************************************************/
/*																*/
/*	関数名	: AES_GCM_arr										*/
/*																*/
/*	内容	: 一括AES-GCM暗号									*/
/*																*/
/*	引　数1	: 暗号対象列											*/
/*																*/
/*	引　数2	: 暗号対象列長										*/
/*																*/
/*	引　数3	: 認証タグ長											*/
/*																*/
/*	戻り値	: なし						*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
void AES_GCM_arr(pui08 cde, cuint len, cui08 len_tag)
{

	start_AES_GCM(enc, cde, len);
	for (; mux->pos < mux->len; ) AES_GCM();
	/* 認証タグの長さ調整 */
	finish_AES_GCM();
	if ((security_control & 0x10) > 0) {
		ramcpy(&mux->cde[mux->len], mux->ath, len_tag);
	}
}

/****************************************************************/
/*																*/
/*	関数名	: decode_start										*/
/*																*/
/*	内容	: 復号準備処理										*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static ui08 decode_start(void)
{
	uint	len;
	ui08	iv[LEN_INITIAL_VECTOR];

	if (NULL == system_title) {
		/* Error : no system title */
		return FALSE;
	}
#if (0==SW_AES)
	AESCNT.BLOCK[0] |= DECODE;
#endif
	/* 長さの取得 + offset移動 */
	len = decode_len(&g_gcm_req.pos, g_gcm_req.buf);
	if (!((len + g_gcm_req.pos) == g_gcm_req.len)) {
		/* Error : incorrect length */
		return FALSE;
	}

	/* セキュリティコントロールのCheck*/
	if((g_gcm_req.buf[g_gcm_req.pos] & 0x30) == 0) {
		/* Error : incorrect SC */
		return FALSE;
	}
	security_control = g_gcm_req.buf[g_gcm_req.pos];

	if ((security_control & 0x10) > 0) {
		if (len < (LEN_SC + LEN_FRAME_COUNTER + LEN_AUTH_TAG)) {
			/* Error : request length is too short */
			return FALSE;
		}
	}
	else {
		if (len < (LEN_SC + LEN_FRAME_COUNTER)) {
			/* Error : request length is too short */
			return FALSE;
		}
	}
	g_gcm_req.pos++;
	/* 初期ベクタの保存 */
	ramset(iv, 0, sizeof(iv));
	ramcpy(&iv[0], system_title, LEN_SYSTEM_TITLE);
	ramcpy(&iv[8], &g_gcm_req.buf[g_gcm_req.pos], LEN_FRAME_COUNTER);
	g_gcm_req.pos += LEN_FRAME_COUNTER;

	/* initial vectorのセット */
	set_ive(iv, LEN_INITIAL_VECTOR);
	if ((security_control & 0x10) > 0) {
		len -= (LEN_SC + LEN_FRAME_COUNTER + LEN_AUTH_TAG);
	}
	else {
		len -= (LEN_SC + LEN_FRAME_COUNTER);
	}
	if (0 == len) {
		/* Error : no xdlms */
		return FALSE;
	}

#if (0==SW_AES)
	set_aad_key_d();
#endif
	start_AES_GCM(dec, &g_gcm_req.buf[g_gcm_req.pos], len);
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名	: encode_start										*/
/*																*/
/*	内容	: 暗号準備処理										*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void encode_start(void)
{
	uint	offset_data, tmp;
	ui08	iv[LEN_INITIAL_VECTOR], *start;

#if (0==SW_AES)
	AESCNT.BLOCK[0] &= (~DECODE);
#endif
	/* gloQ/ded0 - length 長さの保存 */
	switch(order_cipher){
		case CMD_GLO_INITIATE_REQUEST:
		case CMD_DED_INITIATE_REQUEST:
			order_plane += CONV_REQ_RES_SN;
			break;
		case 0xff:
			order_cipher = CMD_GENERAL_GLO_CALL;
			order_plane  = CMD_NOTIFICATION_CALL;
		case CMD_GENERAL_GLO:
			g_gcm_res.buf[g_gcm_res.pos] = LEN_SYSTEM_TITLE;
			g_gcm_res.pos++;
			ramcpy(&g_gcm_res.buf[g_gcm_res.pos], get_system_title(), LEN_SYSTEM_TITLE);
			g_gcm_res.pos += LEN_SYSTEM_TITLE;
		default:			
			order_plane += CONV_REQ_RES_LN;
			break;
	}
	if ((security_control & 0x10) > 0) {
		g_gcm_res.len += LEN_SC + LEN_FRAME_COUNTER + 1 + LEN_AUTH_TAG; /*SC(1)+FC(4)+ORDER(1)*/
	}
	else {
		g_gcm_res.len += LEN_SC + LEN_FRAME_COUNTER + 1;				/*SC(1)+FC(4)+ORDER(1)*/
	}
	encode_len(g_gcm_res.buf, &g_gcm_res.pos, g_gcm_res.len);
	/* System SC-AEの保存 */
	g_gcm_res.buf[g_gcm_res.pos] = security_control;
	g_gcm_res.pos++;

	/* 初期ベクタの保存 */
	ramcpy(&iv[0], get_system_title(), LEN_SYSTEM_TITLE);

	/* System FCの保存 */
	Counter(&iv[8]);
	ramcpy(&g_gcm_res.buf[g_gcm_res.pos], &iv[8], 4);
	g_gcm_res.pos += 4;

	/* initial vectorのセット */
	set_ive(iv, LEN_INITIAL_VECTOR);

#if (0==SW_AES)
	set_aad_key_e();
#endif
	/*暗号化の準備　暗号化する先頭アドレスのオフセットを保存 */
	offset_data = g_gcm_res.pos;
	g_gcm_res.buf[g_gcm_res.pos] = order_plane;
	g_gcm_res.pos++;
	/* 生データのコピー */
	if (0 < g_xdlms_res_len) {
		ramcpy(&g_gcm_res.buf[g_gcm_res.pos], g_xdlms_res, g_xdlms_res_len);
		g_gcm_res.pos += g_xdlms_res_len;
	}
	if (0 < g_raw_len) {
		if (stream == 0) {
			ramcpy(&g_gcm_res.buf[g_gcm_res.pos], g_raw, g_raw_len);
			g_gcm_res.pos += g_raw_len;
			start_AES_GCM(enc, &g_gcm_res.buf[offset_data], g_gcm_res.pos - offset_data);
		}
		else {
			if ((g_gcm_res.pos + g_raw_len) < (apdu_size - 32)) {
				stream0_read(&g_gcm_res.buf[g_gcm_res.pos], g_raw_len);
				g_gcm_res.pos += g_raw_len;
				start_AES_GCM(enc, &g_gcm_res.buf[offset_data], g_gcm_res.pos - offset_data);
			}
			else {
				order_cipher = CMD_GENERAL_GLO;
				stream1_write(&order_cipher, 1);
				tmp = g_gcm_res.pos - offset_data;
				stream1_write(g_gcm_res.buf, offset_data);
				ramcpy(g_raw, &g_gcm_res.buf[offset_data], tmp);
				stream0_read(&g_raw[tmp], 16 - (tmp % 16));
				g_xdlms_res_len = tmp + 16 - (tmp % 16);
				stream++;
				g_gcm_res.pos += g_raw_len;
				start_AES_GCM(enc, g_raw, g_gcm_res.pos - offset_data);
			}
		}
	}
	else {
		start_AES_GCM(enc, &g_gcm_res.buf[offset_data], g_gcm_res.pos - offset_data);
	}
}

/****************************************************************/
/*																*/
/*	関数名	: decode_finish										*/
/*																*/
/*	内容	: 復号後処理										*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: 成功(TRUE)/失敗(FALSE)								*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static ui08 decode_finish(void)
{
	ui08 res;

	res = TRUE;
	finish_iAES_GCM();

	if ((security_control & 0x10) > 0) {
		if (ramcmp(mux->ath, &mux->cde[mux->len], LEN_AUTH_TAG)) {
			/* Error : incorrect authenticate tag */
			res = FALSE;
		}
	}
	if (res == TRUE) {
		order_plane = mux->cde[0];
		g_xdlms_req.len = mux->len;	/*要求電文長を保存*/
		g_xdlms_req.buf = mux->cde;
		res = TRUE;
	}
	return res;
}

/****************************************************************/
/*																*/
/*	関数名	: encode_finish										*/
/*																*/
/*	内容	: 暗号後処理											*/
/*																*/
/*	引　数1	: なし												*/
/*																*/
/*	戻り値	: なし								*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
static void encode_finish(void)
{
	finish_AES_GCM();
	if ((security_control & 0x10) > 0) {
		if (stream == 2) {
			stream0_write(mux->ath, LEN_AUTH_TAG);
		}
		else {
			ramcpy(&mux->cde[mux->len], mux->ath, LEN_AUTH_TAG);
		}
		g_gcm_res.pos += LEN_AUTH_TAG;
	}
	g_cosem_res.pos += g_gcm_res.pos;
	g_cosem_res.len = g_cosem_res.pos;
}
/****************************************************************/
/*																*/
/*	関数名	: install_unwrap_key								*/
/*																*/
/*	内容	: マスター鍵準備									*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 								*/
/*																*/
/****************************************************************/
static ui08 install_secret_key(PAES key)
{
	ui08 len, ret;

	ret = FALSE;
	ramset(scy_buf, 0, 16);		/*0埋め*/
	len = read_auth_val(scy_buf, g_user0);
	if (len > 0) {
		set_aes(key, scy_buf, 16);	/*16バイト固定*/
		ret = TRUE;
	}
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名	: install_unwrap_key								*/
/*																*/
/*	内容	: マスター鍵準備									*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 								*/
/*																*/
/****************************************************************/
static ui08 install_unwrap_key(PAES key)
{
	ui08 len, ret;

	ret = FALSE;
	len = read_master_key(scy_buf, g_user0);
	if (len > 0) {
		set_aes(key, scy_buf, len);
		ret = TRUE;
	}
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名	: install_global_key								*/
/*																*/
/*	内容	: グローバル鍵準備									*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
ui08 install_global_key(void)
{
	ui08 len;

	if(g_userQ==0){
		len = read_master_key(scy_buf, g_userQ);
		if (jgndat.reset == 0) {
			record_log(LOG_SUPER, 0);
		}
	}
	else {
		len = read_global_key(scy_buf, g_userQ);
	}
	if (len == 0) {
		return FALSE;
	}
	delete_gcm();
#if SW_AES
	set_gcm(&gcm, scy_buf, len);
	set_aad_key_e();
	set_aad_key_d();
#else
	sts_key = 0x0008;
	sts_first = 0x0004;
	len >>= 1;
	aes8cpy((pui16)AESKEY.BLOCK, &scy_buf[OFFSET_KEY_BLK], len);
	AESCNT.BLOCK[0] = 0x00A0;
	if (len > 8) {
		len >>= 2;
		len &= 0xfe;
		AESCNT.BLOCK[0] |= len;
	}
#endif
	return TRUE;
}

/****************************************************************/
/*																*/
/*	関数名	: confirm_password									*/
/*																*/
/*	内容	: パスワード確認											*/
/*																*/
/*	引　数1	: 確認パスワード										*/
/*																*/
/*	引　数2	: 確認長さ											*/
/*																*/
/*	戻り値	: TRUE(1)/FALSE(0)									*/
/*																*/
/*	作成日	: 2016-06-09 	担当: T.Kuroda						*/
/*																*/
/*	変更履歴														*/
/*		20yy-mm-dd	担当:	理由:								*/
/*																*/
/****************************************************************/
ui08 confirm_password(cpui08 in, cui08 len)
{
	ui08 ret, l;

	l = read_auth_val(scy_buf, g_userQ);
	if (len != l) {
		ret = FALSE;
		if (g_portQ == 2) {
			comsts.status.wd |= 0x00000800;
		}
	}
	else {
		if (0 != ramcmp(in, scy_buf, len)) {
			ret = FALSE;
			if (g_portQ == 2) {
				comsts.status.wd |= 0x00000800;
			}
		}
		else {
			/* パスワードチェック */
			ret = TRUE;
			if (g_portQ == 2) {
				comsts.status.wd &= ~0x00000800;
			}
		}
	}
	ramset(scy_buf, 0, sizeof(scy_buf));
	return ret;
}

/****************************************************************/
/*																*/
/*	関数名	: install_dedicated_key								*/
/*																*/
/*	内容	: Dedicated鍵準備									*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
void install_dedicated_key(void)
{
	ui08 len;

	order_plane = DLMS_INVALID;

	len = get_dedicated_key(scy_buf);
	if (0 == len)	/*AARE済クライアント*/
	{
		return;
	}

#if SW_AES
	set_gcm(&gcm, scy_buf, len);
	set_aad_key_e();
	set_aad_key_d();
#else
	sts_key = 0x0008;
	sts_first = 0x0004;
	len >>= 1;
	aes8cpy((pui16)AESKEY.BLOCK, scy_buf, len);
	AESCNT.BLOCK[0] = 0x00A0;
	if (len > 8) {
		len >>= 2;
		len &= 0xfe;
		AESCNT.BLOCK[0] |= len;
	}
#endif
}

void install_sign_key(void)
{
	ramset(scy_buf, 0x00, 16);
#if SW_AES
	set_gcm(&gcm, scy_buf, 16);
	set_aad_key_e();
	set_aad_key_d();
#else
	sts_key = 0x0008;
	sts_first = 0x0004;
	aes8cpy((pui16)AESKEY.BLOCK, scy_buf, 8);
	AESCNT.BLOCK[0] = 0x00A0;
#endif
}

/****************************************************************/
/*																*/
/*	関数名	: unwrap_key									*/
/*																*/
/*	内容	: AES鍵開封											*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
ui08 unwrap_key(pui08 io, cpui08 init)
{
	pui08	iv, r1, r2, in, tp;
	ui08	t;
	AES		key;

	if (FALSE == install_unwrap_key(&tmp_key)) {
		return DLMS_INVALID;
	}
	in = scy_buf;
	tp = &scy_buf[24];
	ramcpy(in, io, 24);
	iv = &in[0];
	r1 = &in[8];
	r2 = &in[16];
	for (t = 12; t > 0; t--)
	{
		iv[7] ^= t;
		if (t % 2)	/*11,9,7,5,3,1*/
		{
			/*odd*/
			Decrypt((pui16)iv, &tmp_key);
		}
		else	/*12,10,8,6,4,2*/
		{
			/*even*/
			ramcpy(tp, r1, 8);	/*r1退避*/
			ramcpy(r1, r2, 8);	/*r1にr2をコピー*/
			Decrypt((pui16)iv, &tmp_key);
			ramcpy(r2, r1, 8);	/*r2にr1をコピー*/
			ramcpy(r1, tp, 8);	/*退避したr1をコピー*/
		}
	}
	ramset((pui08)&tmp_key, 0, sizeof(AES));		/*initialize master key*/
	if (!(0 == ramcmp(iv, init, 8)))
	{
		return DLMS_ERROR;
	}
	ramcpy(io, in, 24);
	return DLMS_SUCCESS;
}

ui08 hls_secret(pui08 out, cui08 pos) {

	if (FALSE == install_secret_key(&tmp_key)) {
		return DLMS_INVALID;
	}
	ramcpy(&scy_buf[0], &secret[pos + 1], secret[pos]);
	Encrypt((pui16)&scy_buf[0], &tmp_key);
	ramset((pui08)&tmp_key, 0, sizeof(AES));		/*initialize master key*/
	ramcpy(out, &scy_buf[0], 16);

	return DLMS_SUCCESS;
}
/****************************************************************/
/*																*/
/*	関数名	: clear_security									*/
/*																*/
/*	内容	: securityパラメータ初期化							*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void clear_security(void)
{
	ramset((pui08)&g_gcm_req, 0, sizeof(IO));
	ramset((pui08)&g_gcm_res, 0, sizeof(IO));
	ramset((pui08)mux, 0, sizeof(MUX));
}

/****************************************************************/
/*																*/
/*	関数名	: init_SECURITY										*/
/*																*/
/*	内容	: Security初期化									*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void init_SECURITY(void) {
		
	uint offset;
	
	offset = MAX_XDLMS_BUF - 2;

	offset -= sizeof(MUX);
	mux = (MUX*)&g_xdlms_io[offset];
	offset -= LEN_SECRET;
	secretQ = &g_xdlms_io[offset];
	offset -= LEN_SCY_BUF;
	scy_buf = &g_xdlms_io[offset];

	dec = &gp1;
#if (0==SW_AES)
	enc = &gp1;
	calc_max = 0;
#else
	enc = &gp2;
#endif
	clear_security();
}

/****************************************************************/
/*																*/
/*	関数名	: AESGCM											*/
/*																*/
/*	内容	: DLMS-AESGCMメイン									*/
/*																*/
/*	引　数1	: 													*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/****************************************************************/
/* 要求電文の復号化 */
/* [Command]         [Len]               [SC-AE]   [FC] [invCipherText]  [Tag]*/
/* gloQ/ded0(1)  (g_gcm_req.len-1-lenoflen) 0x30(1)  (4)  (g_gcm_req.len)   (12)*/
void AESGCM(void)
{
	if (0 < g_gcm_req.len)	/* 要求電文 */
	{
		if (0 == g_gcm_req.pos)	/*初期処理*/
		{
			system_title = NULL;
			secret = NULL;
			order_cipher = g_gcm_req.buf[g_gcm_req.pos];	/* コマンドの取得 */
			g_gcm_req.pos++;
			g_authQ = FALSE;
			security_control = USERS[g_userQ].sys.sc;

			switch (order_cipher) {
			case CMD_GLO_INITIATE_REQUEST:				/*常に受け付ける。*/
				secret = secretQ;
				install_global_key();
				system_title = get_system_titleQ();	/*未接続、未認証*/
				break;
			case CMD_GENERAL_GLO:			/**/
				install_global_key();
				system_title = get_system_title0();		/*接続済、未認証*/
//				ramcpy(system_title, &g_gcm_req.buf[g_gcm_req.pos + 1], g_gcm_req.buf[g_gcm_req.pos]);
				g_gcm_req.pos+= g_gcm_req.buf[g_gcm_req.pos];
				g_gcm_req.pos++;
				break;
			case CMD_GLO_GET_REQUEST:			/**/
			case CMD_GLO_SET_REQUEST:			/**/
			case CMD_GLO_ACT_REQUEST:			/**/
				install_global_key();
				secret = secret0;
				system_title = get_system_title0();		/*接続済、未認証*/
				break;
			case CMD_DED_GET_REQUEST:					/*相互認証クライアントであれば受け付ける。*/
			case CMD_DED_SET_REQUEST:
			case CMD_DED_ACT_REQUEST:
				install_dedicated_key();
				secret = secret0;
				system_title = get_system_title0();		/*接続済、認証済*/
				break;
			default:
				/* Fatal : unknown request */
				break;
			};
			if (FALSE == decode_start())
			{
				ConfirmServiceError(ERROR_SECURITY_INVALID_LEN_SC);
				return;
			}
		}
		if (mux->pos < mux->len)
		{
			iAES_GCM();
		}
		if (mux->pos == mux->len)
		{
			if (FALSE == decode_finish())
			{
				ConfirmServiceError(ERROR_SECURITY_INVALID_AUTH);
				if (g_portQ == 2) {
					comsts.status.wd |= 0x00000800;
				}
				return;
			}
			else {
				if (g_portQ == 2) {
					comsts.status.wd &= ~0x00000800;
				}
			}
			g_authQ = TRUE;
			ramset((pui08)&g_gcm_req, 0, sizeof(IO));
		}
		if (mux->pos > MAX_XDLMS_BUF)
		{
			ConfirmServiceError(ERROR_HDLC_INVALID_ORDER);
			return;
		}
	}
	if (0 < g_gcm_res.len) {		/* 暗号化する電文長と、暗号化する電文の先頭アドレスが入ってくる。*/

		if (0 == g_gcm_res.pos) {
			encode_start();
		}
		if (stream == 2) {
			stream_AES_GCM();
		}
		else {
			AES_GCM();
		}
		if (mux->pos == mux->len) {
			encode_finish();
			clear_security();
		}
		if (mux->pos > MAX_XDLMS_BUF)
		{
			ConfirmServiceError(ERROR_HDLC_INVALID_ORDER);
		}
	}
}

/****************************************************************/
/*																*/
/*	関数名	: AESVECT											*/
/*																*/
/*	内容	: AESGCMテストベクタメイン							*/
/*																*/
/*	引　数	: なし												*/
/*																*/
/*	戻り値	: 指令値											*/
/*																*/
/*	作成日	: 2019-01-18	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当:							*/
/*																*/
/****************************************************************/
typedef void(*CIPHER)(pui16 data, CPAES aes);
static const CIPHER cipher[] = { Decrypt, Encrypt };
static const FNC crypt[] = { iAES_GCM, AES_GCM, finish_iAES_GCM, finish_AES_GCM };
#if SW_AES
static const PGP codec[] = { &gp1,&gp2 };
#else
static const PGP codec[] = { &gp1,&gp1 };
#endif

void AESVECT(void)
{
	uint	len, len_in;
	pui08	data;
	ui16	reg;
	PAES	key;

	if (0 < g_vect_req.len)
	{
		key = (PAES)g_xdlms_io;
		ramset((pui08)mux, 0, sizeof(MUX));

		order_cipher = g_vect_req.buf[g_vect_req.pos];	/* コマンドの取得 */
		g_vect_req.pos++;

		/**************** 鍵設定 *******************/
		/* 長さの取得 + offset移動 */
		len = decode_len(&g_vect_req.pos, g_vect_req.buf);
		switch (order_cipher)
		{
		case 0:	/*AESGCM復号*/
		case 1: /*AESGCM暗号*/
#if (0==SW_AES)
		case 4:	/*AESMCT復号*/
		case 5: /*AESMCT暗号*/
#endif
#if SW_AES
			set_gcm(&gcm, &g_vect_req.buf[g_vect_req.pos], len);
#else
			AESPRR2.BIT.AESRES = 1;
			for (AESCNT.BLOCK[1] = 0x8000; AESCNT.BLOCK[1] == 0x8000;);
			AESPRR2.BIT.AESRES = 0;
			if (order_cipher < 2)
			{
				reg = 0x00A0;
			}
			else
			{
				reg = 0x0010;
			}
			if (order_cipher == 0 || order_cipher == 4)
			{
				reg |= 0x0008;
			}
			if (len > 16)
			{
				reg |= (len >> 3);
			}
			AESCNT.BLOCK[0] = reg;
			aes8cpy((pui16)AESKEY.BLOCK, &g_vect_req.buf[g_vect_req.pos], len >> 1);
			sts_key = 0x0008;
			sts_first = 0x0004;
#endif
			break;
		default:
			set_aes(key, &g_vect_req.buf[g_vect_req.pos], len);
			break;
		};
		g_vect_req.pos += len;
		reg = order_cipher % 2;
		/************ 初期ベクタの保存 *************/
		/* 長さの取得 + offset移動 */
		len = decode_len(&g_vect_req.pos, g_vect_req.buf);
		if (order_cipher < 2)
		{
			set_ive(&g_vect_req.buf[g_vect_req.pos], len);
		}
		else
		{
			ramcpy(mux->cb, &g_vect_req.buf[g_vect_req.pos], len);
		}
		g_vect_req.pos += len;

		/************ データの保存 *************/
		len_in = decode_len(&g_vect_req.pos, g_vect_req.buf);
		data = &g_vect_req.buf[g_vect_req.pos];
		g_vect_req.pos += len_in;

		switch (order_cipher)
		{
		case 0:	/*AESGCM復号*/
		case 1: /*AESGCM暗号*/
				/* AAD設定 */
				/* 長さの取得 + offset移動 */
			len = decode_len(&g_vect_req.pos, g_vect_req.buf);
			set_aad(codec[reg], &g_vect_req.buf[g_vect_req.pos], len);
			g_vect_req.pos += len;
			/* 長さの取得 + offset移動 */
			start_AES_GCM(codec[reg], data, len_in);
			for (; mux->pos < mux->len; wdtclr())
			{
				crypt[reg]();
			}
			crypt[reg + 2]();
			encode_len(g_cosem_res.buf, &g_cosem_res.pos, 16);
			ramcpy(&g_cosem_res.buf[g_cosem_res.pos], mux->ath, 16);
			g_cosem_res.pos += 16;
			break;
		case 2: /*aes-ecb復号*/
		case 3: /*aes-ecb暗号*/
			mux->cde = data;
			mux->len = len_in;
			for (; mux->pos < mux->len; wdtclr())
			{
				ramcpy(scy_buf, &mux->cde[mux->pos], 16);
				cipher[reg]((pui16)scy_buf, key);
				ramcpy(&mux->cde[mux->pos], scy_buf, 16);
				mux->pos += 16;
			}
			break;
		case 4: /*aes-ecb-mct復号*/
		case 5: /*aes-ecb-mct暗号*/
			mux->cde = data;
			mux->len = len_in;
#if SW_AES
			ramcpy(scy_buf, &mux->cde[mux->pos], 16);
			for (mux->mlo = 0; mux->mlo < 1000; wdtclr(), mux->mlo++)
			{
				cipher[reg]((pui16)scy_buf, key);
			}
#else
			aes8cpy((pui16)AESIDAT.BLOCK, &mux->cde[mux->pos], 8);
			AESDCNTL.WORD = 0x000b;
			for (; AESSTSL.BIT.odata_valid == 0 || AESSTSL.BIT.busy == 1;);
			for (mux->mlo = 1; mux->mlo < 1000; wdtclr(), mux->mlo++)
			{
				AESIDAT.BLOCK[0] = AESODAT.BLOCK[0];
				AESIDAT.BLOCK[1] = AESODAT.BLOCK[1];
				AESIDAT.BLOCK[2] = AESODAT.BLOCK[2];
				AESIDAT.BLOCK[3] = AESODAT.BLOCK[3];
				AESIDAT.BLOCK[4] = AESODAT.BLOCK[4];
				AESIDAT.BLOCK[5] = AESODAT.BLOCK[5];
				AESIDAT.BLOCK[6] = AESODAT.BLOCK[6];
				AESIDAT.BLOCK[7] = AESODAT.BLOCK[7];
				AESDCNTL.WORD = 0x0003;
				for (; AESSTSL.BIT.odata_valid == 0 || AESSTSL.BIT.busy == 1;);
			}
			aescpy((pui16)scy_buf, (pui16)AESODAT.BLOCK, 8);
#endif
			ramcpy(&mux->cde[mux->pos], scy_buf, 16);
			mux->pos += 16;
			break;
		case 6: /*aes-cfb復号*/
			break;
		case 7: /*aes-cfb暗号*/
			break;
		case 8: /*aes-ofb復号*/
			break;
		case 9: /*aes-ofb暗号*/
			break;
		default:
			break;
		};
		encode_len(g_cosem_res.buf, &g_cosem_res.pos, mux->len);
		ramcpy(&g_cosem_res.buf[g_cosem_res.pos], mux->cde, mux->len);
		g_cosem_res.pos += mux->len;
		g_cosem_res.len = g_cosem_res.pos;
		ramset((pui08)&g_vect_req, 0, sizeof(IO));
	}
}

/****************************************************************/
/*																*/
/*	関数名	: fixed_secret										*/
/*																*/
/*	内容	: GMAC初期設定										*/
/*																*/
/*	引　数1	: チャレンジコードオフセット						*/
/*																*/
/*	戻り値	: なし												*/
/*																*/
/*	作成日	: 2013-04-05	担当: T.Kuroda						*/
/*																*/
/*	変更履歴	: 201x-xx-xx  	担当: 							*/
/*																*/
/****************************************************************/
void fixed_secret(void) {
	ramcpy(secret0, secretQ, LEN_SECRET);
}

void aes_ini(void) {
#if (0==SW_AES)
	AESPER2.BIT.AESEN = 1;
	AESPRR2.BIT.AESRES = 1;
	for (AESCNT.BLOCK[1] = 0x8000; AESCNT.BLOCK[1] == 0x8000;);
	AESPRR2.BIT.AESRES = 0;
#endif
}

void aes_stop(void) {
#if (0==SW_AES)
	AESPRR2.BIT.AESRES = 1;
	for (AESCNT.BLOCK[1] = 0x8000; AESCNT.BLOCK[1] == 0x8000;);
	AESPRR2.BIT.AESRES = 0;
	AESPER2.BIT.AESEN = 0;
#endif
}