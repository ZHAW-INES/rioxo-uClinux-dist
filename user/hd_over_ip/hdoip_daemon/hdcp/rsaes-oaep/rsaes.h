#ifndef _RSAES_H
#define _RSAES_H

void rsaes_encrypt(char* kpubrx,char* kpubrx_e,char* message, char* ciphertext, char* seed);
void rsaes_decrypt(char* ciphertext, char* km, char* p, char* q, char* dp, char* dq, char* qInv);
void mgf1(char* seed, char* T);

#endif /* _RSAES_H */
