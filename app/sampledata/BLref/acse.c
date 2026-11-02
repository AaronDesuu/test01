/****************************************************************************/
/*                                                                          */
/*  システム名      :   DLMS                                                */
/*                                                                          */
/*  サブシステム名  :   ACSE                                                */
/*                                                                          */
/*  内　容          :   セッション確立、開放処理(BER処理)                   */
/*                                                                          */
/*  ファイル名      :   acse.c                                              */
/*                                                                          */
/*  作成日          :   2013-04-05  担当: T.Kuroda                          */
/*                                                                          */
/*  変更履歴        :                                                       */
/*      2016-01-20 T.Kuroda PC用マクロ削除                                  */
/*                                                                          */
/****************************************************************************/
#include "def.h"				/* define宣言					*/
#include "data.h"				/* define宣言					*/
#include "aplevent.h"			/* define宣言					*/
#include "sub.h"
#include "dlms.h"
#include "conf.h"
#include "acse.h"
#include "aes_gcm.h"
#include "xdlms.h"

#define SERVICE_USER		1
#define SERVICE_PROVIDER	2
#define TAG_Y	0x01	/* 必須 */
#define TAG_N	0x00	/* 不要 */

/* PUBクライアント用*/
static cui08 client0[] = { TAG_Y,TAG_N,TAG_N,TAG_N,TAG_N,TAG_Y };
/* PEAクライアント用*/
static cui08 client1[] = { TAG_Y,TAG_N,TAG_Y,TAG_Y,TAG_Y,TAG_Y };
/* COMクライアント用*/
static cui08 client2[] = { TAG_Y,TAG_Y,TAG_Y,TAG_Y,TAG_Y,TAG_Y };
static BER		ber0;							/*タグ0デコード/エンコード*/
static BER		ber1;							/*タグ1デコード/エンコード*/
static uint		acse_res_len;					/*ACSE応答電文長*/
static uint		ber1_offset;					/*デコード/エンコード長*/
static ui08		I(15, 6, asso_lnQ);				/* authentication_mechanism_name */
static pui08	application_contextQ;
static ui08		I(64,4,security_setupQ)[LEN_SYSTEM_TITLE];	/* 4.client_system_title */
static ui08		result_source_diagnostic[3];	/*AARE app03*/
static ui08		result_acse;					/*ACSE処理結果*/
static ui08		association_result;				/*AARE app02*/
static ui08		I(64,2,security_setupQ);		/* 2.security_policy */


/****************************************************************/
/*                                                              */
/*  関数名      : get_system_titleQ                             */
/*                                                              */
/*  内　容      : 接続要求クライアントのシステムタイトルを返す. */
/*                                                              */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : システムタイトル(8バイト)                     */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
pui08 get_system_titleQ( void ) {
	return I(64, 4, security_setupQ);
}

void init_system_titleQ(void) {
	ramset(I(64, 4, security_setupQ), 0, sizeof(I(64, 4, security_setupQ)));
}
/****************************************************************/
/*                                                              */
/*  関数名      :security_policyQ                           */
/*                                                              */
/*  内　容      :接続要求クライアントのセキュリティポリシーを返す.*/
/*                                                              */
/*                                                              */
/*  引　数      :なし                                           */
/*                                                              */
/*  戻り値      :セキュリティポリシー(1バイト)                  */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 security_policyQ( void ) {
	return I(64, 2, security_setupQ);
}

/****************************************************************/
/*                                                              */
/*  関数名      : set_security_policyQ                          */
/*                                                              */
/*  内　容      : 接続要求クライアントのセキュリティポリシーを設定する.*/
/*                                                              */
/*                                                              */
/*  引　数      : セキュリティポリシー(1バイト)                 */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void set_security_policyQ( cui08 in ) {
	I(64, 2, security_setupQ) = in;
}

/****************************************************************/
/*                                                              */
/*  関数名      :get_mechanism_name                           */
/*                                                              */
/*  内　容      :接続要求クライアントのセキュリティポリシーを返す.*/
/*                                                              */
/*                                                              */
/*  引　数      :なし                                           */
/*                                                              */
/*  戻り値      :セキュリティポリシー(1バイト)                  */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 get_mechanism_name_idxQ(void)
{
	return I(15, 6, asso_lnQ);
}

/****************************************************************/
/*                                                              */
/*  関数名      : set_construct_construct                       */
/*                                                              */
/*  内　容      : Construct型をConstruct型で応答電文にエンコードする.*/
/*                                                              */
/*  引　数1     : クラスID                                      */
/*                                                              */
/*  引　数2     : Constructで包むタグのタグID                   */
/*                                                              */
/*  引　数3     : Constructで包むタグID                         */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void set_construct( cuint id0 ) {
	ber0.c	= eCL_context_specific;
	ber0.p	= ePC_constructed;
	ber0.i	= id0;
	ber0.l	= ber1_offset+((ber1.v== NULL)?ber1.l:0);	/*ber1の値が無い場合はber1の長さを使用*/
	ber0.v	= g_xdlms_io;
	encode( g_cosem_res.buf, &g_cosem_res.pos, &ber0 );
}

/****************************************************************/
/*                                                              */
/*  関数名      : set_construct_construct                       */
/*                                                              */
/*  内　容      : Construct型をConstruct型で応答電文にエンコードする.*/
/*                                                              */
/*  引　数1     : クラスID                                      */
/*                                                              */
/*  引　数2     : Constructで包むタグのタグID                   */
/*                                                              */
/*  引　数3     : Constructで包むタグID                         */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void set_construct_construct( const eCLASS cls, cuint id1, cuint id0 ) {
	ber1.c	= cls;
	ber1.p	= ePC_constructed;
	ber1.i	= id1;
	ber1_offset = 0U;
	encode( g_xdlms_io, &ber1_offset, &ber1 );
	set_construct( id0 );
}

/****************************************************************/
/*                                                              */
/*  関数名      : set_construct_primitive                       */
/*                                                              */
/*  内　容      : Primitive型をConstruct型で応答電文にエンコードする.*/
/*                                                              */
/*  引　数1     : クラスID                                      */
/*                                                              */
/*  引　数2     : Primitive型で包むタグのタグID                 */
/*                                                              */
/*  引　数3     : Constructで包むタグID                         */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void set_construct_primitive( const eCLASS cls, cuint id1, cuint id0 )
{
	ber1.c	= cls;
	ber1.p	= ePC_primitive;
	ber1.i	= id1;
	ber1_offset = 0U;
	encode( g_xdlms_io, &ber1_offset, &ber1 );
	set_construct( id0 );
}

/****************************************************************/
/*                                                              */
/*  関数名      : get_construct_primitive                       */
/*                                                              */
/*  内　容      : Construct型で包まれたPrimitive型のタグをデコードする.*/
/*                                                              */
/*  引　数1     : クラスID                                      */
/*                                                              */
/*  引　数2     : Primitive型のタグID                           */
/*                                                              */
/*  戻り値      : 成功(1),失敗(0)                               */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 get_construct_primitive( const eCLASS cls, eUNIVERSAL id ) {

	if( !(ePC_constructed == ber0.p) ) {
		/* Error : incorrect ber0 p/c */
		return FALSE;
	}
	ber1_offset = 0U;
	decode( &ber1, &ber1_offset, ber0.v );
	if( !( ber1_offset == ber0.l ) ) {
		/* Error : incorrect ber1 length */
		return FALSE;
	}
	if( !(cls == ber1.c) ) {
		/* Error : incorrect ber1 class */
		return FALSE;
	}
	if( !(ePC_primitive == ber1.p) ) {
		/* Error : incorrect ber1 p/c */
		return FALSE;
	}
	if( !(id == ber1.i) ) {
		/* Error : incorrect ber1 id */
		return FALSE;
	}
	return TRUE;
}

static cui08 protocol_ver[]={ 0x02, 0x84};  
ui08 request_protocol_version( void ){

	if( !(ePC_primitive == ber0.p) ) {
		/* Error : incorrect ber0 p/c */
		result_acse = ERROR_ACSE_INVALID_VERSION;
		return FALSE;
	}
	if( !( 2 ==  ber0.l ) ) {
		/* Error : incorrect length */
		return FALSE;
	}
	if(0 == ramcmp(protocol_ver,ber0.v,2)){
		return TRUE;
	}
	else{
		result_acse = ERROR_ACSE_INVALID_VERSION;
		return FALSE;
	}
}

ui08 request_application_name( void )	/* ePC_constructed */
{
	ui08 ret;
	ui08 rank;

	ret = FALSE;
	if( FALSE == get_construct_primitive( eCL_universal, eOctetidentifier ) )
	{
		result_acse = ERROR_ACSE_INVALID_APPLI_NAME;
		return ret;
	}
	if( !(LEN_CONTEXT_NAME == ber1.l )  )
	{
		/* Error : incorrect length */
		result_acse = ERROR_ACSE_INVALID_APPLI_NAME;
		return ret;
	}
	application_contextQ = ber1.v;
	if( !(0 == ramcmp(application_contextQ , application_name[0], 6 )))
	{
		/* Error : incorrect application context identifier */
		result_acse = ERROR_ACSE_INVALID_APPLI_NAME;
		return ret;
	}
	if ( !(LN_NO_CIPHER == ber1.v[6]) && !(LN_CIPHER == ber1.v[6]) )
	{
		/* Error : incorrect application name */
		result_acse = ERROR_ACSE_INVALID_APPLI_NAME_VAL;
		return ret;
	}
	rank = Rank(g_userQ);
	if (rank == COSEM_PUBLIC) {
		if (LN_NO_CIPHER == ber1.v[6]) {
			ret = TRUE;
			set_security_policyQ(SECURITY_POLICY_NN);
			result_source_diagnostic[2] = 0x00;
		}
	}
	if (rank == COSEM_POWER || rank == COSEM_READER) {
		if (LN_CIPHER == ber1.v[6]) {
			ret = TRUE;
			set_security_policyQ(SECURITY_POLICY_NE);
			result_source_diagnostic[2] = 0x00;
		}
	}
	if (rank == COSEM_ADMIN) {
		if (LN_CIPHER == ber1.v[6]) {
			ret = TRUE;
			set_security_policyQ(SECURITY_POLICY_NE);
			result_source_diagnostic[2] = 0x0e;
		}
	}
	if (ret == TRUE) {
		association_result = 0x00;
		result_source_diagnostic[0] = eInteger;
		result_source_diagnostic[1] = 0x01;
		encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);	/*エコーバック*/
		ber1.l = 1;
		ber1.v = &association_result;
		set_construct_primitive(eCL_universal, eInteger, 2);
		ber1.l = 3;
		ber1.v = result_source_diagnostic;
		set_construct_construct(eCL_context_specific, SERVICE_USER, 3);
	}
	return ret;
}

ui08 request_called_AP_title(void)/*A2*/
{
	ui08 ret;

	ret = get_construct_primitive(eCL_universal, eOctetstring);

	return ret;
}

ui08 request_called_AE_qualifier(void)/*A3*/
{
	ui08 ret;

	ret = get_construct_primitive(eCL_universal, eOctetstring);

	return ret;
}

ui08 request_called_AP_invocation_id(void)/*A4*/
{
	ui08 ret;

	ret = get_construct_primitive(eCL_universal, eInteger);

	return ret;
}

ui08 request_called_AE_invocation_id(void)/*A5*/
{
	ui08 ret;

	ret = get_construct_primitive(eCL_universal, eInteger);

	return ret;
}


ui08 request_application_title( void )
{
	ui08 ret, ok;

	ret = FALSE;
	if (FALSE == get_construct_primitive(eCL_universal, eOctetstring)) {
		result_acse = ERROR_ACSE_INVALID_APPLI_TITLE;
		return ret;
	}
	if( !(LEN_SYSTEM_TITLE == ber1.l ) ) {
		/* Error : incorrect length */
		result_acse = ERROR_ACSE_INVALID_APPLI_TITLE;
		return ret;
	}
	if (g_userQ == 0) {
		if (0 == romcmp(ber1.v, USERS[g_userQ].sys.sys, ber1.l)) {
			ret = TRUE;
		}
	}
	else {
		ret = TRUE;
	}
	if (ret == TRUE) {
		ramcpy(I(64, 4, security_setupQ), ber1.v, ber1.l);	/*システムタイトルの設定*/
		ber1.v = get_system_title();
		ber1_offset = 0U;
		encode(g_xdlms_io, &ber1_offset, &ber1);
		ber0.i = 4;
		ber0.l = ber1_offset;
		ber0.v = g_xdlms_io;
		encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);
	}
	return ret;
}

ui08 request_calling_AP_invocation_id(void)/*A8*/
{
	ui08 ret;

	ret = get_construct_primitive(eCL_universal, eInteger);

	return ret;
}

ui08 request_calling_AE_invocation_id(void)	/*A9*/
{
	ui08 ret;

	ret = get_construct_primitive(eCL_universal, eInteger);

	return ret;
}

/****************************************************************/
/*                                                              */
/*  関数名      : request_acse_requirements                     */
/*                                                              */
/*  内　容      : acse requirementsを応答電文にエンコードする.  */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : 成功(1),失敗(0)                               */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 request_acse_requirements( void )
{
	if( !(ePC_primitive == ber0.p) )
	{
		/* Error : incorrect ber0 p/c */
		return FALSE;
	}
	if( !( 2 ==  ber0.l ) )
	{
		/* Error : incorrect length */
		return FALSE;
	}
	if( !(0x07 == ber0.v[0]) )
	{
		/* Error : incorrect bit ignore length */
		return FALSE;
	}
	if( !(0x80 == (ber0.v[1]&0x80)) )
	{
		/* Error : incorrect bit */
		return FALSE;
	}
	if (SECURITY_POLICY_NE == security_policyQ()) {
		ber0.i = 8;
		encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);
	}
	return TRUE;
}

/****************************************************************/
/*                                                              */
/*  関数名      : request_mechanism_name                        */
/*                                                              */
/*  内　容      : mechanism nameを応答電文にエンコードする.     */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : 成功(1),失敗(0)                               */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 request_mechanism_name( void )
{
	ui08 rank;

	if (!(ePC_primitive == ber0.p)) {
		/* Error : incorrect ber0 p/c */
		return FALSE;
	}
	if (!(LEN_CONTEXT_NAME == ber0.l)) {
		/* Error : incorrect length */
		return FALSE;
	}
	if (!(0 == ramcmp(ber0.v, mechanism_name[0], 6))) {
		/* Error : incorrect object identifier */
		return FALSE;
	}
	rank = Rank(g_userQ);
	if (MECHA_LLS_PASS == ber0.v[6]) {
		if (rank == COSEM_POWER || rank == COSEM_READER) {
			ber0.i = 9;
			encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);
			I(15, 6, asso_lnQ) = MECHA_LLS_PASS;
			set_security_policyQ(SECURITY_POLICY_NE);
			setCounter(g_userQ);
			return TRUE;
		}
	}
	else {
		if (rank == COSEM_ADMIN) {
			if (MECHA_HLS == ber0.v[6]) {
				ber0.i = 9;
				encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);
				I(15, 6, asso_lnQ) = MECHA_HLS;
				set_security_policyQ(SECURITY_POLICY_AE);
				setCounter(g_userQ);
				return TRUE;
			}
			if (MECHA_HLS_GMAC == ber0.v[6]) {
				ber0.i = 9;
				encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);
				I(15, 6, asso_lnQ) = MECHA_HLS_GMAC;
				set_security_policyQ(SECURITY_POLICY_AE);
				setCounter(0);
				return TRUE;
			}
		}
	}
	return FALSE;
}

/****************************************************************/
/*                                                              */
/*  関数名      : request_authentication_value                  */
/*                                                              */
/*  内　容      : application valueを応答電文にエンコードする.  */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : 成功(1),失敗(0)                               */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 request_authentication_value( void )
{
	ui08 ret;

	ret = get_construct_primitive(eCL_context_specific, eReserved0);
	if (ret == TRUE)
	{
		switch (I(15, 6, asso_lnQ)) {
		case MECHA_HLS_GMAC:
		case MECHA_HLS:
			set_BtoA(ber1.v, ber1.l);		/*チャレンジキーの設定*/
			ber1.v = get_AtoB();
			ber1_offset = 0U;
			encode(g_xdlms_io, &ber1_offset, &ber1);

			ber0.i = 10;
			ber0.l = ber1_offset;
			ber0.v = g_xdlms_io;
			encode(g_cosem_res.buf, &g_cosem_res.pos, &ber0);
			ret = TRUE;
			break;

		case MECHA_LLS_PASS:
			/* タグ応答電文は生成しない */
			ret = confirm_password(ber1.v, ber1.l);
			break;
		}
	}
	return ret;
}

/****************************************************************/
/*                                                              */
/*  関数名      : request_user_information                      */
/*                                                              */
/*  内　容      : user informationを応答電文にエンコードする.   */
/*                                                              */
/*  引　数      : なし                                          */
/*                                                              */
/*  戻り値      : 成功(1),失敗(0)                               */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
ui08 request_user_information( void )
{
	ui08	order;

	if( FALSE == get_construct_primitive( eCL_universal, eOctetstring ) )
	{
		return FALSE;
	}
	acse_res_len = 0;
	order = ber1.v[0];
	if( CMD_INITIATE_REQUEST == order ) {
		if (SECURITY_POLICY_NN == security_policyQ()) {
			g_xdlms_req.buf = ber1.v;
			g_xdlms_req.len = ber1.l;
			acse_res_len = LEN_INITIATE_RESPONSE + 1;
		}
		else
		{
			result_acse = ERROR_ACSE_INVALID_INITIATE;
		}
	}
	if( CMD_GLO_INITIATE_REQUEST == order ) {
		if (SECURITY_POLICY_AE == security_policyQ()) {
			g_gcm_req.buf = ber1.v;
			g_gcm_req.len = ber1.l;
			acse_res_len = LEN_INITIATE_RESPONSE + LEN_AUTH_TAG + 8;/* 8=gloQ/ded0(1)+len(1)+SC(1)+FC(4)+tag(1)*/
		}
		else {
			if (SECURITY_POLICY_NE == security_policyQ()) {
				g_gcm_req.buf = ber1.v;
				g_gcm_req.len = ber1.l;
				acse_res_len = LEN_INITIATE_RESPONSE + 8;			/* 8=gloQ/ded0(1)+len(1)+SC(1)+FC(4)+tag(1)*/
			}
			else {
				result_acse = ERROR_SECURITY_INVALID_AUTH;
			}
		}
	}
	if( 0 < acse_res_len )
	{
		ber1.l	= acse_res_len;
		ber1.v	= NULL;
		ber1_offset	= 0U;
		encode( g_xdlms_io, &ber1_offset, &ber1 );
		ber0.l = ber1_offset + ber1.l;
		ber0.v = g_xdlms_io;
		encode( g_cosem_res.buf, &g_cosem_res.pos, &ber0 );

		g_cosem_res.pos -= acse_res_len;
		g_cosem_res.buf[ g_cosem_res.pos ] = order + CONV_REQ_RES_SN;
		g_cosem_res.pos++;
		return TRUE;
	}
	/* Error : incorrect command */
	return FALSE;
}

/****************************************************************/
/*                                                              */
/*  関数名      : encode_release_reason                         */
/*                                                              */
/*  内　容      : release reasonを応答電文にエンコードする.     */
/*                                                              */
/*  引　数1     : 出力先バッファ                                */
/*                                                              */
/*  引　数2     : 出力先バッファのオフセット                    */
/*                                                              */
/*  引　数3     : 理由                                          */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void	encode_release_reason( pui08 out, puint offset, cui08 reason )
{
	ber0.c = eCL_context_specific;
	ber0.p = ePC_primitive;
	ber0.i = 0;
	ber0.l = 1;
	ber0.v = (pui08)&reason;
	encode( out, offset, &ber0 );
}

/****************************************************************/
/*                                                              */
/*  関数名      : set_confirm_service_error_aare                */
/*                                                              */
/*  内　容      : confirm service errorのacseを応答電文にエンコードする.*/
/*                                                              */
/*  引　数1     : application context name                      */
/*                                                              */
/*  引　数2     : result_source_diagnosticの値                  */
/*                                                              */
/*  引　数3     : user informationの電文長                      */
/*                                                              */
/*  戻り値      : なし                                          */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void set_confirm_error_aare0(cui08 code, cui08 type) {
	g_cosem_res.pos += 2;	/* AARQ(1) + LENGTH(1)*/

	ber1.l = LEN_CONTEXT_NAME;
	ber1.v = (pui08)application_contextQ;
	set_construct_primitive(eCL_universal, eOctetidentifier, 1);

	association_result = code;
	ber1.l = 1;
	ber1.v = &association_result;
	set_construct_primitive(eCL_universal, type, 0x02);
}

void set_confirm_error_aare1(cui08 code, cui08 id, eCLASS type) {
	g_cosem_res.pos += 2;	/* AARQ(1) + LENGTH(1)*/

	ber1.l = LEN_CONTEXT_NAME;
	ber1.v = (pui08)application_contextQ;
	set_construct_primitive(eCL_universal, eOctetidentifier, 1);

	association_result = 0x01;
	ber1.l = 1;
	ber1.v = &association_result;
	set_construct_primitive(eCL_universal, id, 0x02);
	result_source_diagnostic[0] = eInteger;
	result_source_diagnostic[1] = 0x01;
	result_source_diagnostic[2] = code;
	ber1.l = 3;
	ber1.v = result_source_diagnostic;
	set_construct_construct(eCL_context_specific, type, 3);
}

void set_confirm_error_aare2(cui08 code, cui08 type) {
	g_cosem_res.pos += 2;	/* AARQ(1) + LENGTH(1)*/

	ber1.l = LEN_CONTEXT_NAME;
	ber1.v = (pui08)application_contextQ;
	set_construct_primitive(eCL_universal, eOctetidentifier, 1);

	association_result = 0x01;
	ber1.l = 1;
	ber1.v = &association_result;
	set_construct_primitive(eCL_universal, eInteger, 0x02);
	result_source_diagnostic[0] = eInteger;
	result_source_diagnostic[1] = 0x01;
	result_source_diagnostic[2] = code;
	ber1.l = 3;
	ber1.v = result_source_diagnostic;
	set_construct_construct(eCL_context_specific, SERVICE_USER, 3);
}

void set_confirm_error_aare(cui08 code, cui08 len )
{
	g_cosem_res.pos	+=2;	/* AARQ(1) + LENGTH(1)*/

	ber1.l	= LEN_CONTEXT_NAME;
	ber1.v	= (pui08)application_contextQ;
	set_construct_primitive(  eCL_universal, eOctetidentifier , 1 );

	association_result	= 0x01;
	ber1.l	= 1;
	ber1.v	= &association_result;
	set_construct_primitive( eCL_universal, eInteger, 0x02 );
	result_source_diagnostic[0] = eInteger;
	result_source_diagnostic[1] = 0x01;
	result_source_diagnostic[2] = code;
	ber1.l	= 3;
	ber1.v	= result_source_diagnostic;
	set_construct_construct( eCL_context_specific, SERVICE_USER, 3 );
	ber1.l	= LEN_SYSTEM_TITLE;
	ber1.v	= get_system_title( );
	set_construct_primitive( eCL_universal, eOctetstring, 4 );

	ber1.l	= len;
	ber1.v	= NULL;
	set_construct_primitive( eCL_universal, eOctetstring, 30 );
	g_cosem_res.pos -= len;
}

/****************************************************************/
/*                                                              */
/*  関数名      :AARE                                           */
/*                                                              */
/*  内　容      :AARQに対するAARE処理をする.                    */
/*                                                              */
/*  引　数      :なし                                           */
/*                                                              */
/*  戻り値      :なし                                           */
/*                                                              */
/*  作成日      : 2013-04-05    担当: T.Kuroda                  */
/*                                                              */
/*  変更履歴    : 201x-xx-xx    担当:                           */
/*                                                              */
/****************************************************************/
void AARE( void )
{
	uint	len, cur_id;
	uint	prv_tag = 0, prv_idx=0;
	ui08	check[6], cnt = 0, rank;
	pui08	client;

	I(15, 6, asso_lnQ) = MECHA_NONE;

	len = decode_len( &g_acse_req.pos, g_acse_req.buf );
	if( !((len + g_acse_req.pos) == g_acse_req.len) )
	{
		result_acse = ERROR_ACSE_INVALID_AARQ;
		/* Error : invalid length */
		return;
	}
	ramset( check, 0, 6 );
	result_acse = ERROR_ACSE_INVALID_APPLICATION_TAG;
	for( cnt=0 ; (g_acse_req.pos < g_acse_req.len) && (cnt < 31) ; cnt++ )
	{
		decode( &ber0, &g_acse_req.pos, g_acse_req.buf );
		if( 0 == ber0.l )
		{
			/* Error : invalid ber0 */
			return;
		}
		if( g_acse_req.pos > g_acse_req.len )
		{
			/* Error : incorrect ber0 length */
			return;
		}
		if( !(eCL_context_specific == ber0.c) )
		{
			/* Error : incorrect ber0 class */
			return;
		}
		cur_id = ber0.i;
		switch ( cur_id )
		{
			case 0:
				if( FALSE == request_protocol_version( ) ) {
					return;
				}
				break;

			case 1:
				if(1 <= prv_tag)
				{
					/* Error : incorrect app01 order */
					return;
				}
				if( FALSE == request_application_name( ) )
				{
					return;
				}
				check[0]++;prv_idx=cur_id;
				break;

			case 2:
				if (FALSE == request_called_AP_title()) {
					return;
				}
				break;

			case 3:
				if (FALSE == request_called_AE_qualifier()) {
					return;
				}
				break;

			case 4:
				if (FALSE == request_called_AP_invocation_id()) {
					return;
				}
				break;

			case 5:
				if (FALSE == request_called_AE_invocation_id()) {
					return;
				}
				break;

			case 6:
				if(6 <= prv_tag)
				{
					/* Error : incorrect app06 order */
					return;
				}
				if( FALSE == request_application_title( ) )
				{
					return;
				}
				check[1]++;prv_idx=cur_id;
				break;

			case 8:
				if (FALSE == request_calling_AP_invocation_id()) {
					return;
				}
				break;

			case 9:
				if (FALSE == request_calling_AE_invocation_id()) {
					return;
				}
				break;

			case 10:
				if (10 <= prv_tag)
				{
					/* Error : incorrect app10 order */
					return;
				}
				if( FALSE == request_acse_requirements( ) )
				{
					return;
				}
				check[2]++;prv_idx=cur_id;
				break;

			case 11:
				if(11 <= prv_tag)
				{
					/* Error : incorrect app11 order */
					return;
				}
				if( FALSE == request_mechanism_name( ) )
				{
					return;
				}
				check[3]++;prv_idx=cur_id;
				break;
			case 12:
				if(12 <= prv_tag)
				{
					/* Error : incorrect app12 order */
					return;
				}
				if( FALSE == request_authentication_value( ) )
				{
					result_acse = ERROR_ACSE_INVALID_APPLI_VALUE;
					return;
				}
				check[4]++;prv_idx=cur_id;
				break;
			case 30:
				if( 30 <= prv_tag )
				{
					/* Error : incorrect app30 order */
					return;
				}
				if( FALSE == request_user_information( ) )
				{
					return;
				}
				check[5]++;prv_idx=cur_id;
				break;

			default:
				/* Error : found unknown tag */
				return;
		};
		prv_tag = cur_id;
	}
	client = NULL;
	rank = Rank(g_userQ);

	switch (I(64, 2, security_setupQ)) {
	case  SECURITY_POLICY_NN:
		if (COSEM_PUBLIC == rank) {
			client = client0;
		}
		break;
	case SECURITY_POLICY_NE:
		if (COSEM_POWER == rank || COSEM_READER == rank) {
			client = client2;
		}
		break;
	case SECURITY_POLICY_AE:
		if (COSEM_POWER == rank || COSEM_ADMIN == rank) {
			client = client2;
		}
		break;
	default:
		break;
	}
	if (client != NULL) {
		if (0 == ramcmp(check, client, 6)) {
			result_acse = DLMS_SUCCESS;
		}
	}
	/* Error : ACSE tag compornents are incorrect */
}


/****************************************************************/
/*                                                              */
/*  関数名      : request_release_reason                        */
/*                                                              */
/*  内　容      : RLREのrelease reasonを応答電文にエンコードする.*/
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
ui08 request_release_reason( void )
{
	uint	len;

	acse_res_len = 1;
	len = decode_len( &g_acse_req.pos, g_acse_req.buf );
	if( !(3 == len) )
	{
		/* Error : incorrect RLRQ length */
		return FALSE;
	}
	if( !((len+g_acse_req.pos) == g_acse_req.len) )
	{
		/* Error : incorrect request length */
		return FALSE;
	}
	decode( &ber0, &g_acse_req.pos, g_acse_req.buf );
	if( !(eCL_context_specific == ber0.c) )
	{
		/* Error : incorrect ber1 class */
		return FALSE;
	}
	if( !(ePC_primitive == ber0.p) )
	{
		/* Error : incorrect ber1 p/c */
		return FALSE;
	}
	if( !( 1 == ber0.l ) )
	{
		/* Error : incorrect ber1 p/c */
		return FALSE;
	}
	if( !( 0 == ber0.i ) )
	{
		/* Error : incorrect ber1 id */
		return FALSE;
	}
	if( !(RL_NORMAL == ber0.v[0]) )
	{
		/* Error : incorrect rlrq normal */
		return FALSE;
	}
	return TRUE;
}

/****************************************************************/
/*                                                              */
/*  関数名      : RLRE                                          */
/*                                                              */
/*  内　容      : RLRQに対するRLRE処理をする.                   */
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
void RLRE( void )
{
	ui08 rank;
	if( 0 <request_release_reason( ) ) {
		encode_release_reason( g_cosem_res.buf, &g_cosem_res.pos, RL_NORMAL );
		set_non_associated(3);
		delete_gcm( );
	}
	else {
		encode_release_reason( g_cosem_res.buf, &g_cosem_res.pos, RL_NOT_FINISHED );
	}
}

/****************************************************************/
/*                                                              */
/*  関数名      : clear_acse_req                                */
/*                                                              */
/*  内　容      : ACSE要求電文パラメータ初期化.                 */
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
void clear_acse_req( void )
{
	ramset( (pui08)&g_acse_req, 0, sizeof(IO) );
}

/****************************************************************/
/*                                                              */
/*  関数名      : init_ACSE                                     */
/*                                                              */
/*  内　容      : ACSE初期化.                                   */
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
void init_ACSE( void )
{
	clear_acse_req( );
}

/****************************************************************/
/*                                                              */
/*  関数名      : ACSE                                          */
/*                                                              */
/*  内　容      : ACSEに対する要求電文を処理する.               */
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
void ACSE( void )
{
	uint	offset;
	ui08	order_acse;				/*ACSE指示内容*/

	if ( g_acse_req.len > 0 )		/* 要求電文 */
	{
		result_acse	= DLMS_SUCCESS;
		offset = g_cosem_res.pos;
		g_cosem_res.pos++;		/* 応答電文のlength位置に移動  */
		order_acse = g_acse_req.buf[g_acse_req.pos];
		g_acse_req.pos++;
		if (Rank(g_userQ) == COSEM_PUBLIC) {
			application_contextQ = (pui08)application_name[1];
		}
		else {
			application_contextQ = (pui08)application_name[3];
		}
		switch ( order_acse )
		{
			case CMD_AARQ:
				AARE( );
				break;
			default:
				/*未実装: 非対応コマンドに対する処理*/
			case CMD_RLRQ:
				RLRE( );
				break;
		};
		if( !(DLMS_SUCCESS == result_acse ) )
		{
			ConfirmServiceError( result_acse );
			return;
		}
		acse_res_len += g_cosem_res.pos;
		acse_res_len -= offset;
		acse_res_len -= 2;
		encode_len( g_cosem_res.buf, &offset, acse_res_len );
		if( !(CMD_AARQ == order_acse) )
		{
			g_cosem_res.len = g_cosem_res.pos;
		}
		clear_acse_req( );
	}
}
