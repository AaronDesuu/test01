#ifndef _ACSE_H_
#define _ACSE_H_

void		clear_acse_req(void);
void		init_ACSE(void);
void		ACSE(void);
void		set_confirm_error_aare0(cui08 code, cui08 len);
void		set_confirm_error_aare1(cui08 code, cui08 len, eCLASS data);
void		set_confirm_error_aare2(cui08 code, cui08 len);
void		set_confirm_error_aare(cui08 code, cui08 len);
ui08		security_policyQ(void);
void		set_security_policyQ(cui08 in);
void		init_system_titleQ(void);
pui08		get_system_titleQ(void);
ui08		get_mechanism_name_idxQ(void);
void		encode_release_reason(pui08 out, puint offset, cui08 reason);

#endif  /* _ACSE_H_*/
