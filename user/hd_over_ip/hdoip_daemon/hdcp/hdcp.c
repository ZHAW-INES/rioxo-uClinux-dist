/*
 * hdcp.c
 *
 * HDCP SERVER FUNCTIONS (needed for hdcp session key exchange)
 *
 * Created on: 28.04.2011
 * Author: itin
 */
#include <debug.h>
#include "hdcp.h"
#include "rsaes-oaep/rsaes.h"
#include "rsassa_pkcs1/rsassa.h"
#include "aes128/aes_encrypt.h"
#include "sha256/sha256.h"
#include "protocol/protocol.h"
#include "bcrypt/b_main.h"
#include "hdoipd_fsm.h"
#include "hoi_drv_user.h"

/** State machine for session key exchange (server side)
 *  Responds to client requests. If successful, client and server share
 *  the same session key.
 *
 * */
int hdcp_ske_s(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp)
{
    report(INFO "ID: %s CONTENT: %s",m->id, m->content);
    char enc_km[257];
    int i;
	int message = atoi(m->id);
	char null[] ="";
	char *id[16] = {"00","01","02","03","04","05","06","07","08","09","10","11","12","13","14","15"};
	char HL[65];
	char Edkey_ks[49];
	char temp[17];

	switch (message) {
	        case HDCP_START:
	        	media->hdcp_state = HDCP_SEND_RTX;
	            hdcp_decrypt_flash_keys();/* Get the encrypted HDCP keys from flash and decrypt them*/
	        	hdcp_generate_random_nr(media->hdcp_var.rtx);
	            rscp_response_hdcp(rsp, media->sessionid, id[2], media->hdcp_var.rtx); //respond to setup message
	            report(INFO "RTX: %s", media->hdcp_var.rtx);
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_CERT:	/* check certificate */
	        	if (media->hdcp_state != HDCP_SEND_RTX) return RSCP_HDCP_ERROR;
	        	report(INFO "CERTIFICATE: %s", m->content);
	            if (hdcp_check_certificate(m->content, &media->hdcp_var.repeater, media->hdcp_var.km, enc_km)!=1)
	        		return RSCP_HDCP_ERROR;
	            rscp_response_hdcp(rsp, media->sessionid, id[4], enc_km); //respond message to setup message
	            report(INFO "repeater: %d enc_km: %s", media->hdcp_var.repeater, enc_km);
	            media->hdcp_state = HDCP_VERIFY_CERT;
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_RRX:		/* receive rrx and acknowledge */
	        	if (media->hdcp_state != HDCP_VERIFY_CERT) return RSCP_HDCP_ERROR;
	        	strcpy(media->hdcp_var.rrx, m->content); //save rrx
	            rscp_response_hdcp(rsp, media->sessionid, id[0], null); //respond message to setup message
	            report(INFO "Received rrx: %s", media->hdcp_var.rrx);
	            media->hdcp_state = HDCP_RECEIVE_RRX;
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_HPRIME: /* compare received and generated H */
	        	if (media->hdcp_state != HDCP_RECEIVE_RRX) return RSCP_HDCP_ERROR;
	        	//check if H is valid
	        	hdcp_calculate_h(media->hdcp_var.rtx, &media->hdcp_var.repeater, HL, media->hdcp_var.km, media->hdcp_var.kd);  //calculate own H
	        	//report(INFO "kd: %s", media->hdcp_var.kd);
	        	//report(INFO "Own H: %s", HL);
	        	report(INFO "Received H: %s", m->content);
	        	if(strcmp(HL,m->content) != 0) {					//compare received and own H
	        		report(INFO "HMAC of H is wrong!");
	        		return RSCP_HDCP_ERROR;
	        	}
	        	hdcp_generate_random_nr(media->hdcp_var.rn);
	            rscp_response_hdcp(rsp, media->sessionid, id[9], media->hdcp_var.rn); 	//respond message LC_Init
	            media->hdcp_state = HDCP_RECEIVE_H;
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_LC_SEND_LPRIME: /* start locality check */
	        	if (media->hdcp_state != HDCP_RECEIVE_H) return RSCP_HDCP_ERROR;
	            report(INFO "rn: %s", media->hdcp_var.rn);
	            report(INFO "rrx: %s", media->hdcp_var.rrx);
	        	hdcp_calculate_l(media->hdcp_var.rn, media->hdcp_var.rrx, media->hdcp_var.kd, HL);
	        	//report(INFO "Own L: %s", HL);
	        	report(INFO "Received L: %s", m->content);
	        	if(strcmp(HL,m->content) != 0) {		//compare received and own L
	        		report(INFO "HMAC of L is wrong!");
	        		return RSCP_HDCP_ERROR;
	        	}

	        	/*For multicast: riv and ks are static for VTB, check if generated previously*/
	        	if (hdoipd.hdcp.ske_executed != HDCP_SKE_EXECUTED){

	        	    hdcp_generate_random_nr(hdoipd.hdcp.riv);	/* generate riv */

		        	/* Generate random number for session key */
		        	for (i=0;i<2;i++){
		        	    if (generate_random_nr(temp)==1){
		        	    	report(INFO "Couldn't generate TRN for ks! Try PRNG!");
		        	        if (pseudo_random_nr(temp)==1) {
		        	        	report(INFO "Couldn't generate PRN for ks! EXIT!");
		        	        	return RSCP_HDCP_ERROR;
		        	        }
		        	    }
		        	    strcat(hdoipd.hdcp.ks,temp);
		        	}
	        	}

	        	/* Encrypt session key and send it back */
	        	hdcp_ske_enc_ks(media->hdcp_var.rn, media->hdcp_var.km, media->hdcp_var.rrx, media->hdcp_var.rtx, hdoipd.hdcp.ks, Edkey_ks);

	            report(INFO "RIV: %s", hdoipd.hdcp.riv);
	        	/* concatenate and send enc session key and riv back */
	            strcat(Edkey_ks,hdoipd.hdcp.riv);
	        	rscp_response_hdcp(rsp, media->sessionid, id[11], Edkey_ks); 	//respond message SKE_Send_Eks
	        	//report(INFO "THE SESSION KEY: %s", hdoipd.hdcp.ks); //SECRET VALUE, SHOW ONLY TO DEBUG
	        	/* xor session key with lc128 */
	        	xor_strings(hdoipd.hdcp.ks, hdoipd.hdcp.lc128, hdoipd.hdcp.ks_x_lc128,32);
	        	/* write keys to HW and enable encryption*/
	        	hdcp_convert_sk_char_int(hdoipd.hdcp.ks_x_lc128, hdoipd.hdcp.riv, hdoipd.hdcp.keys);
	        	/*for (i=0;i<6;i++){  //SECRET VALUES, SHOW ONLY TO DEBUG
	        		report(INFO "THE KEYS: %08x",  hdoipd.hdcp.keys[i]);
	        	}*/
	        	report(INFO "SESSION KEY EXCHANGE SUCCESSFUL!")
	        	hdoipd.hdcp.ske_executed = HDCP_SKE_EXECUTED;
		    	media->hdcp_state = HDCP_SEND_KS;
	            return RSCP_SUCCESS;
	        break;
	}
	return RSCP_HDCP_ERROR;
}

/* Generate random number
 * Try HW true random number generator first, if that fails,
 * user Linux pseudo random number generator
 *
 * @param[out] pointer to generated number
 * @return error code
 * */
int hdcp_generate_random_nr(char* number){
  if (generate_random_nr(number)==1) {
	report(INFO "Couldn't generate TRN! Try PRNG!");
    if (pseudo_random_nr(number)==1) {
      report(INFO "Couldn't generate PRN! EXIT!");
      return RSCP_HDCP_ERROR;
    }
  }
  return RSCP_SUCCESS;
}

/* Convert the string with the session key and riv to 6 integers
 *
 * @param[in] pointer to ks (string with session key, 128bit)
 * @param[in] pointer to riv (string with riv, 64bit)
 * @param[out] pointer to uint32_t[6] array,
 * @return error code
 * */
int hdcp_convert_sk_char_int(char* ks, char* riv, uint32_t* keys){

	int i, j;
	char temp[9];
	//convert the session key to an integer array
	for (j=0;j<4;j++){
	    for (i=0;i<8;i++){
	        temp[i]=ks[i+j*8];
	    }
	    temp[8]='\0';
	    keys[j]=strtoul(temp, NULL, 16);
	}
	//convert riv to an integer array
	for (j=0;j<2;j++){
	    for (i=0;i<8;i++){
	        temp[i]=riv[i+j*8];
	    }
	    temp[8]='\0';
	    keys[j+4]=strtoul(temp, NULL, 16);
	}
	return RSCP_SUCCESS;
}

/** Check receiver certificate, encrypt km if certificate is valid
 *
 *  @param pointer to reciever certificate
 *  @param pointer to repeater value (true or false)
 *  @param pointer to masterkey
 *  @param pointer to encrypted km
 *  @return error code
 */
int hdcp_check_certificate(char* certificate, uint32_t* repeater, char* km, char* enc_km){
	//kpub_dcp (3072-bit public key with exponent)
	//char n[]="A2C7555754CBAAA77A2792C31A6DC231CF12C224BF897246A48D2083B2DD04DA7E01A919EF7E8C4754C859725C8960629F39D0E480CAA8D41E91E30E2C77556D58A89E3EF2DA783EBAD1053707F288740CBCFB68A47A27AD63A51F67F1458516498AE6341C6E80F5FF1372855DC1DE5F0186558671E8103314702A5F157B5C653C463A1779ED546AA6C9DFEB2A812A802A46A206DBFDD5F3CF74BB665648D77C6A03141E5556E4B6FA382B5DFB879F9E782187C00C633E8D0FE2A719109B15E11187493349B86632287C87F5D22EC5F3662F79EF405AD41485745F064350CDDE84E73C7D8E8A49CC5ACF73A18A13FF37133DAD57D85122D6321FC0684CA05BDD5F78C89F2D3AA2B81E4AE408556405E694FBEB036A0ABE831894D4B6C3F2589C7A24DDD13AB73AB0BBE5D128ABAD2454720E76D28932EA46D378D0A96778C12D18B033DEDB27CCB07CC9A4BDDF2B64103244068121B3BACF3385491E864CBDF23D34EFD6237A9F2CDA84F08383717DDA6E4496CD1D05DE30F61E2F9C999C6007";
    char n[]="b0e9aa45f129ba0a1cbe175728eb2b4e8fd0c06aad79980f8d438d4704b82bf4152156190140013bd09190629e89c2278ecfb6dbce3f721050938c2329837b8064a759e861674cbcd858b8f1d4f82c379816260e4ef94eee24deccd14b4bc5067afb4965e6c00083481e8e422a53a0f537292b5af973c59aa1b5b5747c06dc7b7cdc6c6e826b4988d41b25e0eed179bd3985fa4f25ec701923c1b9a6d97e3eda48a958e318141e9f307f4ca8ae5322662bbe24cb4766fc83cf5c2d1e3aabab06be05aa1a9b2db7a654f3632b97bf93bec1af2139490ce93190ccc2bb3c02c4e2bdbd2f84639bd2dd783e90c6c5ac16772e696c77fded8a4d6a8ca3a9256c21fdb2940c84aa07292646f79b3a1987e09feb30a8f564eb07f1e9dbf9af2c8b697e2e67393ff3a6e5cdda249ba27872f0a227c3e025b4a1046a598027b5dab4b453973b2899acf496270f7f300c4aafcb9ed87128243ebc3515be13ebaf4301bd612454349f733eb5109fc9fc80e84de332968f88102325f3d33e6e6dbbdc2966eb";	
    char e[]="03";
	char message[277];
	char signature[769];
	int i, ret;
	char temp[17];
	char seed[65]="";
	char kpubrx_n[257];
	char kpubrx_e[7];

	km[0]='\0';

    //check if repeater
	if (certificate[0]=='1') *repeater=1;
	else *repeater=0;

	// divide the certificate into message and signature
	for (i=0;i<276;i++){
	    message[i]=certificate[i+1];
	}
	message[276]='\0';
	report(INFO "Certificate message: %s",message);
	for (i=0;i<768;i++){
	  signature[i]=certificate[i+277];
	}
	signature[768]='\0';
	report(INFO "Certificate signature: %s",signature);

	//check signature, return if fail
	ret = rsassa_verify(n, e, message, signature);
	if (ret != 1) return ret;

	/* if signature ok, encrypt km */
	//get receiver public key out of the certificate
	for (i=0;i<256;i++){kpubrx_n[i]=message[i+10];}
	kpubrx_n[256]='\0';
	for (i=0;i<6;i++){kpubrx_e[i]=message[i+266];}
	kpubrx_e[6]='\0';
	//generate the random numbers
	for (i=0;i<2;i++){
	    if (generate_random_nr(temp)==1){
	    	report(INFO "Couldn't generate TRN for km! Try PRNG!");
	        if (pseudo_random_nr(temp)==1) {
	    	    report(INFO "Couldn't generate PRN for km! EXIT!");
	            return 0;
	        }
	    }
	    strcat(km,temp);
	}
	for (i=0;i<4;i++){
	    if (generate_random_nr(temp)==1){
	    	report(INFO "Couldn't generate TRN for seed! Try PRNG!");
	        if (pseudo_random_nr(temp)==1) {
	    	    report(INFO "Couldn't generate PRN for seed! EXIT!");
	            return 0;
	        }
	    }
	    strcat(seed,temp);
	}
	report(INFO "KM: %s",km); //SECRET VALUE, SHOW ONLY TO DEBUG
	rsaes_encrypt(kpubrx_n,kpubrx_e,km,enc_km,seed);

	return ret;
}

/** Generate session key and encrypt it
 *
 *  @param[in] pointer to rn (random number from locality check)
 *  @param[in] pointer to km (master key)
 *  @param[in] pointer to rrx (random number)
 *  @param[in] pointer to rtx (random number)
 *  @param[in] pointer to ks (session key)
 *  @param[out] pointer to encrypted session key
 *  @return error code
 */
int hdcp_ske_enc_ks(char* rn, char* km, char* rrx, char* rtx, char* ks, char* Edkey_ks){

	char plaintext3[33]="";
	char key2[33];
	char dkey2[33];
	char xor_val[65];

	/* Generate random number for session key */
	/*for (i=0;i<2;i++){
	    if (generate_random_nr(temp)==1){
	    	report(INFO "Couldn't generate TRN for ks! Try PRNG!");
	        if (pseudo_random_nr(temp)==1) {
	        	report(INFO "Couldn't generate PRN for ks! EXIT!");
	        	return RSCP_HDCP_ERROR;
	        }
	    }
	    strcat(ks,temp);
	}*/

	/* Generate encrypted session key */
	strcat(plaintext3, rtx);
	strcat(plaintext3,"0000000000000002");
	xor_strings(rn,km,key2,32);
	aes_128(key2,plaintext3,dkey2);
	xor_strings(dkey2, rrx, xor_val, 64);
	xor_strings(ks, xor_val,Edkey_ks,32);
	return RSCP_SUCCESS;
}

/** Calculate H (HMAC value to check if km was successfully decrypted)
 *
 *  @param[in]  pointer to rtx (random number)
 *  @param[in]  pointer to repeater (value to indicate if client is repeater or not)
 *  @param[out] pointer to H (the calculated HMAC value)
 *  @param[in]  pointer to km (master key)
 *  @param[in]  pointer to kd (derived from km)
 *  @return error code
 * */
int hdcp_calculate_h(char* rtx, uint32_t* repeater, char* H, char* km, char* kd){

	int i;
	char plaintext1[33];
	char plaintext2[33];
	char dkey0[33];
	char dkey1[33];
	char repeater_s[3]="";

    kd[0]='\0';

	if (*repeater) strcat(repeater_s,"01"); //convert int to string
    else strcat(repeater_s,"00");

	for (i=0;i<17;i++){
		plaintext1[i]=rtx[i];
		plaintext2[i]=rtx[i];
	}
	strcat(plaintext1,"0000000000000000");
	strcat(plaintext2,"0000000000000001");
	aes_128(km,plaintext1,dkey0);
	aes_128(km,plaintext2,dkey1);
    strcat(kd,dkey0);
    strcat(kd,dkey1);
    hmac(rtx,kd,H,repeater_s);
    return RSCP_SUCCESS;
}

/** Calculate L (HMAC value for locality check)
 *
 *  @param[in]  pointer to rn (received random number)
 *  @param[in]  pointer to rrx (random number)
 *  @param[in]  pointer to kd (derived from km)
 *  @param[out] pointer to L (the calculated HMAC value)
 *  @return error code
 * */
int hdcp_calculate_l(char* rn, char* rrx, char* kd, char* L){

	char key_l[65];
	char rrx_pad[65]="000000000000000000000000000000000000000000000000";
	char no_repeater[]="00"; /* Parameter not used here, therefore always zero*/

    strcat(rrx_pad,rrx);
    xor_strings(rrx_pad, kd, key_l, 64);
    hmac(rn,key_l,L,no_repeater);
    return RSCP_SUCCESS;
}

/** Decrypt session key (client function)
 *
 *  @param[in]  pointer to rn (random number from locality check)
 *  @param[out] pointer to ks (decrypted session key)
 *  @param[in]	pointer to Edkey_ks (encrypted session key)
 *  @param[in]  pointer to rtx (random number)
 *  @param[in]  pointer to rrx (random number)
 *  @param[in]  pointer to km (master key)
 *  @return error code
 */
int hdcp_ske_dec_ks(char* rn, char* ks, char* Edkey_ks, char* rtx, char* rrx, char* km){

	char plaintext3[33]="";
	char key2[33];
	char dkey2[33];
	char xor_val[65];

	strcat(plaintext3, rtx);
	strcat(plaintext3,"0000000000000002");
	xor_strings(rn,km,key2,32);
	aes_128(key2,plaintext3,dkey2);
	xor_strings(dkey2, rrx, xor_val, 64);
	xor_strings(Edkey_ks, xor_val, ks, 32);  //ks is now the encrypted session key
	return RSCP_SUCCESS;
}

/** Get the encrypted HDCP keys from flash and decrypt them
 *  No parameter to pass to function, the decrypted values are
 *  directly written to global variables. Key to decrypt the data
 *  and the path can be edited directly in the corresponding functions
 *
 *  @return error code
 * */
int hdcp_decrypt_flash_keys(){

	unsigned long i;
	unsigned long sz;
	char *input;
	char *str;
	char *certrx;
	char *p;
	char *q;
	char *dp;
	char *dq;
	char *qInv;
	char *lc128;

	//get the decrypted data
	bcrypt_main(&sz, &input);
	//alloc a string and copy the data to the string
	str = (char*)calloc(sz, sizeof(char));

	//copy input to a string
	for (i=0;i<sz;i++){
		str[i]=*(input+i);
	}

	//get the keys out of the string
	certrx=&str[LENGHT_OFFSET];
	str[LENGHT_OFFSET+LENGHT_CERTRX]='\0';
	strcpy(hdoipd.hdcp.certrx, certrx);
	report(INFO "CERT_RX: %s", hdoipd.hdcp.certrx);

	p=&str[2*LENGHT_OFFSET+LENGHT_CERTRX+1];
	str[2*LENGHT_OFFSET+LENGHT_CERTRX+LENGHT_PRIVATE_KEY+1]='\0';
	strcpy(hdoipd.hdcp.p, p);
	//report(INFO "P: %s", hdoipd.hdcp.p);  //SECRET VALUE, SHWO ONLY TO DEBUG

	q=&str[3*LENGHT_OFFSET+LENGHT_CERTRX+LENGHT_PRIVATE_KEY+2];
	str[3*LENGHT_OFFSET+LENGHT_CERTRX+2*LENGHT_PRIVATE_KEY+2]='\0';
	strcpy(hdoipd.hdcp.q, q);
	//report(INFO "Q: %s", hdoipd.hdcp.q); //SECRET VALUE, SHWO ONLY TO DEBUG

	dp=&str[4*LENGHT_OFFSET+LENGHT_CERTRX+2*LENGHT_PRIVATE_KEY+3];
	str[4*LENGHT_OFFSET+LENGHT_CERTRX+3*LENGHT_PRIVATE_KEY+3]='\0';
	strcpy(hdoipd.hdcp.dp, dp);
	//report(INFO "DP: %s", hdoipd.hdcp.dp); //SECRET VALUE, SHWO ONLY TO DEBUG

	dq=&str[5*LENGHT_OFFSET+LENGHT_CERTRX+3*LENGHT_PRIVATE_KEY+4];
	str[5*LENGHT_OFFSET+LENGHT_CERTRX+4*LENGHT_PRIVATE_KEY+4]='\0';
	strcpy(hdoipd.hdcp.dq, dq);
	//report(INFO "DQ: %s", hdoipd.hdcp.dq); //SECRET VALUE, SHWO ONLY TO DEBUG

	qInv=&str[6*LENGHT_OFFSET+LENGHT_CERTRX+4*LENGHT_PRIVATE_KEY+5];
	str[6*LENGHT_OFFSET+LENGHT_CERTRX+5*LENGHT_PRIVATE_KEY+5]='\0';
	strcpy(hdoipd.hdcp.qInv, qInv);
	//report(INFO "qInv: %s", hdoipd.hdcp.qInv);

	lc128=&str[7*LENGHT_OFFSET+ +LENGHT_CERTRX+5*LENGHT_PRIVATE_KEY+6];
	str[7*LENGHT_OFFSET+LENGHT_CERTRX+5*LENGHT_PRIVATE_KEY+LENGHT_LC128+6]='\0';
	strcpy(hdoipd.hdcp.lc128, lc128);
	//report(INFO "lc128: %s", hdoipd.hdcp.lc128); //SECRET VALUE, SHWO ONLY TO DEBUG

	free(str);
	return RSCP_SUCCESS;
}

/** Check if hdcp is enabled in HW (audio or video)
 *
 *  @return hdcp status bits of eti/eto audio/video
 */
int get_hdcp_status(){

	uint32_t eti_audio_en,eti_video_en,eto_audio_en,eto_video_en;
	uint32_t temp=0;

	/* get status from HW */
	hoi_drv_hdcpstat(&eto_audio_en,&eto_video_en,&eti_audio_en,&eti_video_en);

	if (eti_video_en) temp = temp | HDCP_ETI_VIDEO_EN;
	if (eti_audio_en) temp = temp | HDCP_ETI_AUDIO_EN;
	if (eto_video_en) temp = temp | HDCP_ETO_VIDEO_EN;
	if (eto_audio_en) temp = temp | HDCP_ETO_AUDIO_EN;
	report(INFO "HDCP HW Audio/Video status: %08x",temp);
	return temp;
}
