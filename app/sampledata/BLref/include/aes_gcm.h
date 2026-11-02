#ifndef _GCM_H_
#define _GCM_H_

extern ui08	secret0[LEN_SECRET];
extern	FNC		SECURITY;
ui08	confirm_password(cpui08 in, cui08 len);
void	gcm_hash0(cpui08 in, cui08 size);
void	gcm_hash0f(cpui08f in, cui08 size);
void	gcm_hash1(cpui08 in, cui08 size);
void	gcm_hash1f(cpui08f in, cui08 size);
void	gcm_hash2(pui16 hash);
void	install_dedicated_key(void);
void	install_sign_key(void);
void	delete_gcm(void);
void	set_BtoA(cpui08f in, cui08 len);
pui08	get_AtoB(void);
void	fixed_secret(void);
void	set_aad_key_e(void);
void	set_aad_key_d(void);
void	set_aad_gmac_a(void);
void	set_aad_gmac_b(void);
ui08	install_global_key(void);
ui08	hls_secret(pui08 out, cui08 pos);
ui08	unwrap_key(pui08 io, cpui08 init);
void	set_ive(cpui08 inve, cuint len);
ui08	iAES_GCM_arr(pui08 cde, cuint len, cui08 len_tag);
void	AES_GCM_arr(pui08 cde, cuint len, cui08 len_tag);
void	clear_security(void);

void	init_SECURITY(void);
void	aes_ini(void);
void	aes_stop(void);
void	AESGCM(void);
#if 0
ui08	AESVECT(pui08 out, cpui08 in, cuint size);
#else
void	AESVECT(void);
#endif

#endif
