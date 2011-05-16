/*
 * hdcp.c
 *
 *  Created on: 28.04.2011
 *      Author: itin
 */
#include "hdcp.h"

/* HDCP SERVER FUNCTIONS */

/*hdcp server session key exchange*/
int hdcp_ske_s(t_rscp_media* media, t_rscp_req_hdcp* m, t_rscp_connection* rsp)
{
    report(INFO "ID: %s CONTENT: %s ###########",m->id, m->content);
    int i;
    char enc_km[257];
	int message = atoi(m->id);
	printf("The converted string: %d",message);
	char null[] ="";
	char id0[] = "00";
	char id2[] = "02";
	char id4[] = "04";
	char id9[] = "09";
	char id11[] = "11";
	char HL[65];
	char Edkey_ks[49];
	char kd[65]="";  //replace kd
	char ret[100];
	uint32_t keys[6];
	char number2[]="1745bb8bd04bc83c79578f90c914b8938055aa4ac1fa80393827975af6622de43808dcd5d90b83cb3d89eb00d0944f43f5fabb9c4c996ef78b58f6977b47d08149c81a08f041fa088e120c7344a49356599cf5319f0c68176055cb9deddab3db092a1234f0c71304278f655aebd36258e250d4e5e8e776a60e3c1e9eecd2b9e186397d4e67501000100001d0a61eaabf8a82b0269a134fd91ac2bf28f348bd484fa62bc014a4aa2b214bfb5f4dfac80930d13ec9ce5d83470519a6680ebbecc7e45f0e6396384c9b98e8caf9ca9d40eeb9a572a1741ca97f31996b55d0f30a384e573a2ed05697a22ce841f3e399e2876c9bc895b70b17bf4edb67412ab482964ce6c6004eba97aa215a6589aad32c75339e5fef037a7a0c5ffecd9b005bb2513a0a4c70b2a5dc68f5111cb36ed5c177e2220c3eb408c67bb1cd247b0e0bde74ccd5dd52312f83b1d913bf3c760ea902448e592216cf6d95e768d2b86a67c16aea83608a037141ad703e14031ca6c95e010b043cfb7e03005b9acb70868cd7e11472a033beb74c819628b2f1191b6064fe02a44204329131fddd04a116c0e83bf22623bebecd77628ba648842c873a79e4a693ab20c4b3ad950db7c51ee15e06b2c63a69157ddbf174723ad15cbb991180b518ff91c5167c10b78f5d955dc48e4c083a5df75e2dc88d2c6dddf1f379035f6fddae0043269c1afd9f911c5aa7458321c71aaa714fb231722";
	switch (message) {
	        case HDCP_START:
	        	hdcp_generate_random_nr(ret);
	        	strcpy(hdoipd.hdcp.rtx, ret);
	        	//TODO: set and check state
	            rscp_response_hdcp(rsp, &media->sessionid, id0, ret); //respond to setup message
	            report(INFO "hdcp.rtx1: %s", hdoipd.hdcp.rtx);
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_CERT:
	        	//check certificate
	        	//TEST with static certificate
	            if (hdcp_check_certificate(number2, &hdoipd.hdcp.repeater, &hdoipd.hdcp.km, enc_km)!=1)
	        	//if (check_certificate(m->content, &hdoipd.hdcp.repeater, &hdoipd.hdcp.km, enc_km)!=1)
	        		return RSCP_HDCP_ERROR;
	            rscp_response_hdcp(rsp, &media->sessionid, id4, enc_km); //respond message to setup message
	            report(INFO "repeater: %d enc_km: %s", hdoipd.hdcp.repeater, enc_km);
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_RRX:
	        	//save rrx
	        	strcpy(hdoipd.hdcp.rrx, m->content);
	            rscp_response_hdcp(rsp, &media->sessionid, id0, null); //respond message to setup message
	            report(INFO "Received rrx: %s", hdoipd.hdcp.rrx);
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_AKE_SEND_HPRIME:
	        	//check if H is valid
	        	report(INFO "rtx: %s", hdoipd.hdcp.rtx);
	        	report(INFO "rep: %d", hdoipd.hdcp.repeater);
	        	report(INFO "km: %s", hdoipd.hdcp.km);
	        	hdcp_calculate_h(&hdoipd.hdcp.rtx, &hdoipd.hdcp.repeater, HL, &hdoipd.hdcp.km, kd);  //calculate own H
	        	strcpy(hdoipd.hdcp.kd, kd);  //save kd
	        	report(INFO "kd: %s", hdoipd.hdcp.kd);
	        	report(INFO "Own H: %s", HL);
	        	report(INFO "Received H: %s", m->content);
	        	if(strcmp(HL,&m->content) != 0) {							 	//compare received and own H
	        		report(INFO "HMAC of H is wrong!");
	        		return RSCP_HDCP_ERROR;
	        	}
	        	hdcp_generate_random_nr(&ret);
	            strcpy(hdoipd.hdcp.rn, ret);
	            rscp_response_hdcp(rsp, &media->sessionid, id9, ret); 			//respond message LC_Init
	            return RSCP_SUCCESS;
	        break;
	        case HDCP_LC_SEND_LPRIME:
	            report(INFO "rn: %s", hdoipd.hdcp.rn);
	            report(INFO "rrx: %s", hdoipd.hdcp.rrx);
	            report(INFO "kd: %s", hdoipd.hdcp.kd);
	        	hdcp_calculate_l(&hdoipd.hdcp.rn, &hdoipd.hdcp.rrx, &hdoipd.hdcp.kd, HL);
	        	report(INFO "Own L: %s", HL);
	        	report(INFO "Received L: %s", m->content);
	        	if(strcmp(HL,&m->content) != 0) {							 	//compare received and own L
	        		report(INFO "HMAC of L is wrong!");
	        		return RSCP_HDCP_ERROR;
	        	}
	        	/* Encrypt session key and send it back */
	        	hdcp_ske_enc_ks(&hdoipd.hdcp.rn, &hdoipd.hdcp.ks, Edkey_ks);
	        	/* generate riv */
	        	hdcp_generate_random_nr(&ret);
	            strcpy(hdoipd.hdcp.riv, ret);
	            report(INFO "RIV: %s", hdoipd.hdcp.riv);
	        	/* concatenate and send enc session key and riv back */
	            strcat(Edkey_ks,hdoipd.hdcp.riv);
	        	rscp_response_hdcp(rsp, &media->sessionid, id11, Edkey_ks); 	//respond message SKE_Send_Eks
	        	report(INFO "THE SESSION KEY: %s", hdoipd.hdcp.ks);
	        	/* write keys to HW and enable encryption*/
	        	hdcp_convert_sk_char_int(&hdoipd.hdcp.ks, &hdoipd.hdcp.riv, keys);
	        	for (i=0;i<6;i++){
	        		report(INFO "THE KEYS: %08x", keys[i]);
	        	}
	        	hoi_drv_hdcp(keys); //write keys to kernel
		    	/*hoi_drv_hdcp_viden_eto();*/
		    	report(INFO "Video encryption enabled (eto)!");
		    	/*hoi_drv_hdcp_auden_eto();*/
		    	report(INFO "Audio encryption enabled (eto)!");
	            return RSCP_SUCCESS;
	        break;
	}

    //rscp_response_hdcp(rsp, media->sessionid, m->id, m->content); //respond message to setup message
    //return RSCP_SUCCESS;
}


/*Generate the random number*/
int hdcp_generate_random_nr(char* number){
  if (generate_random_nr(number)==1) {
	report(INFO "Couldn't generate TRN! Try PRNG!");
    if (pseudo_random_nr(number)==1) {
      report(INFO "Couldn't generate PRN! EXIT!");
      return 0;
    }
  }
}

/*Convert the string with the session key to integers*/
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
}

/** Check receiver certificate, encrypt km if certificate is valid
 *
 *  @param pointer to reciever certificate
 *  @param pointer to repeater value (true or false)
 *  @param pointer to masterkey
 *  @param pointer to encrypted km
 *  @return error code
 */
int hdcp_check_certificate(char* certificate, int* repeater, char* km, char* enc_km){
	//kpub_dcp (3072-bit public key with exponent)
	char n[]="A2C7555754CBAAA77A2792C31A6DC231CF12C224BF897246A48D2083B2DD04DA7E01A919EF7E8C4754C859725C8960629F39D0E480CAA8D41E91E30E2C77556D58A89E3EF2DA783EBAD1053707F288740CBCFB68A47A27AD63A51F67F1458516498AE6341C6E80F5FF1372855DC1DE5F0186558671E8103314702A5F157B5C653C463A1779ED546AA6C9DFEB2A812A802A46A206DBFDD5F3CF74BB665648D77C6A03141E5556E4B6FA382B5DFB879F9E782187C00C633E8D0FE2A719109B15E11187493349B86632287C87F5D22EC5F3662F79EF405AD41485745F064350CDDE84E73C7D8E8A49CC5ACF73A18A13FF37133DAD57D85122D6321FC0684CA05BDD5F78C89F2D3AA2B81E4AE408556405E694FBEB036A0ABE831894D4B6C3F2589C7A24DDD13AB73AB0BBE5D128ABAD2454720E76D28932EA46D378D0A96778C12D18B033DEDB27CCB07CC9A4BDDF2B64103244068121B3BACF3385491E864CBDF23D34EFD6237A9F2CDA84F08383717DDA6E4496CD1D05DE30F61E2F9C999C6007";
	char e[]="03";
	char message[277];
	char signature[769];
	int i, ret;
	char temp[17];
	char seed[65];
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
	report(INFO "KM: %s",km);
	rsaes_encrypt(kpubrx_n,kpubrx_e,km,enc_km,seed);

	return ret;
}

/** Generate session key and encrypt it
 *
 *  @param pointer to rn (random number)
 *  @param pointer to session key
 *  @param pointer to encrypted session key
 *  @return error code
 */
int hdcp_ske_enc_ks(char* rn, char* ks, char* Edkey_ks){

	int i;
	char plaintext3[33]="";
	char key2[33];
	char dkey2[33];
	char xor_val[65];
	char temp[17];

	/* Generate random number for session key*/
	for (i=0;i<2;i++){
	    if (generate_random_nr(temp)==1){
	    	report(INFO "Couldn't generate TRN for ks! Try PRNG!");
	        if (pseudo_random_nr(temp)==1) {
	        	report(INFO "Couldn't generate PRN for ks! EXIT!");
	            return 0;
	        }
	    }
	    strcat(ks,temp);
	}

	/* Generate encrypted session key*/
	strcat(plaintext3, hdoipd.hdcp.rtx);
	strcat(plaintext3,"0000000000000002");
	xor_strings(rn,hdoipd.hdcp.km,key2,32);
	aes_128(key2,plaintext3,dkey2);
	xor_strings(dkey2, hdoipd.hdcp.rrx, xor_val, 64);
	xor_strings(ks, xor_val,Edkey_ks,32);

	//TODO: xor session key with lc128
}

int hdcp_calculate_h(char* rtx, int* repeater, char* H, char* km, char* kd){

	int i;
	char plaintext1[33];
	char plaintext2[33];
	char dkey0[33];
	char dkey1[33];
	char repeater_s[3]="";

	if (repeater) strcat(repeater_s,"01"); //convert int to string
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
}

int hdcp_calculate_l(char* rn, char* rrx, char* kd, char* L){

	char key_l[65];
	char rrx_pad[65]="000000000000000000000000000000000000000000000000";
	char no_repeater[]="00"; //TODO: make this value adjustable

    strcat(rrx_pad,rrx);
    xor_strings(rrx_pad, kd, key_l, 64);
    hmac(rn,key_l,L,no_repeater);
}

/* HDCP CLIENT FUNCTIONS */

/** Decrypt session key
 *
 *  @param
 *  @param
 *  @param pointer to
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

	//TODO: xor session key with lc128
}

/** Open HDCP socket, bind socket but do not accept connections yet
 *
 *  @param pointer to hdcp struct
 *  @return error code
 */
int hdcp_open_socket(t_rscp_hdcp* m, int* sockfd, struct sockaddr_in* cli_addr ) // sockfd, (struct sockaddr *) &cli_addr
{
    struct sockaddr_in serv_addr;

    //set hdcp bit in registry if necessary
    if (m->hdcp_on==1){
    	reg_set("hdcp-force", "on");
    }

    //check if hdcp is forced by HDMI or over RSCP
    if (reg_test("hdcp-force", "on") || hdoipd_rsc(RSC_VIDEO_IN_HDCP)) {
    	m->hdcp_on = 1;
    	//if (!(get_hdcp_status())){
    	if (reg_test("hdcp-done", "off")){
    	    //only start listener if HDCP is needed and not enabled yet
    	    *sockfd = socket(AF_INET, SOCK_STREAM, 0);  //open a socket
    	    if (sockfd < 0) error("ERROR opening socket");
    	    bbzero((char *) &serv_addr, sizeof(serv_addr));
    	    serv_addr.sin_family = AF_INET;
    	    serv_addr.sin_addr.s_addr = INADDR_ANY;
    	    serv_addr.sin_port = htons(m->port_nr);
    	    if (bind(*sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
                printf("error on binding\n");
      		    return RSCP_HDCP_ERROR;
      	    }
    	    listen(*sockfd,3); //get ready for a client to connect
    	    reg_set("hdcp-done", "on");
    	}
    }
    return RSCP_SUCCESS;
}

/** Check if hdcp is enabled in HW (audio or video)
 *
 *  @return hdcp status bits of eti/eto  audio/video
 */
int get_hdcp_status(){

	uint32_t eti_audio_en,eti_video_en,eto_audio_en,eto_video_en;
	uint32_t temp=0;

	hoi_drv_hdcpstat(&eto_audio_en,&eto_video_en,&eti_audio_en,&eti_video_en);

	if (eti_video_en) temp = temp | HDCP_ETI_VIDEO_EN;
	if (eti_audio_en) temp = temp | HDCP_ETI_AUDIO_EN;
	if (eto_video_en) temp = temp | HDCP_ETO_VIDEO_EN;
	if (eto_audio_en) temp = temp | HDCP_ETO_AUDIO_EN;
	printf("HDCP HW Audio/Video status: %08x\n",temp);
	return temp;

	/*if ((eti_audio_en==1)||(eti_video_en==1)||(eto_audio_en==1)||(eto_video_en==1)){
		return 1;  //true, hdcp is already enabled
	}
	else{
		return 0;  //false, hdcp is NOT enabled
	}*/
}

/** Check if HDCP is needed, start HDCP server and start session key exchange (ske)
 *  If the ske was successful, write the new keys to the hardware
 *
 *  @param pointer to hdcp struct
 *  @return error code
 */
int hdcp_ske_server(t_rscp_hdcp* m, int* sockfd, struct sockaddr_in* cli_addr, char* media_type){
	int newsockfd;
	socklen_t clilen;
	char ip; //Not necessary in transmitter function
	char riv[17];
	char session_key[33];
	uint32_t hdcp_keys[6];

	if (m->hdcp_on == 1){//if hdcp is needed...
	    if (get_hdcp_status()){  //session key exchange previously done
	    	if (!(strcmp(media_type, "video")))	{
	    		hoi_drv_hdcp_viden_eto();
	    		printf("Video encryption enabled (eto)!\n"); //... audio encryption is already enabled
	    	}
	    	if (!(strcmp(media_type, "audio")))	{
	    		hoi_drv_hdcp_auden_eto();
	    		printf("Audio encryption enabled (eto)!\n"); //... video encryption is already enabled
	    	}
	    }
	    else {
	        //no hdcp encryption running, start SKE
	    	clilen = sizeof(cli_addr);
	    	newsockfd = accept(*sockfd, (struct sockaddr*) cli_addr, &clilen);//the new socket (file descriptor) for the client
	    	if (newsockfd < 0) error("ERROR on accept");
		    if (transmitter(newsockfd,ip,session_key,riv,hdcp_keys)){
		        close(newsockfd);
			    hoi_drv_hdcp(hdcp_keys); //write keys to kernel
			    if (!(strcmp(media_type, "video")))	{
			    	hoi_drv_hdcp_viden_eto();
			        printf("Video encryption enabled (eto)!\n");
			    }
			    if (!(strcmp(media_type, "audio")))	{
			    	hoi_drv_hdcp_auden_eto();
			        printf("Audio encryption enabled (eto)!\n");
			    }
		    }
		    else{
		        close(newsockfd);
			    return RSCP_HDCP_ERROR; //abort if SKE was not successful
		    }
	    }
	}
	return RSCP_SUCCESS;

}

/** On client side: Check if HDCP is needed and start session key exchange (ske)
 *  If the ske was successful, write the new keys to the hardware
 *
 *  @param pointer to hdcp struct
 *  @param pointer to media_type (audio or video)
 *  @return error code
 */
int hdcp_ske_client(t_rscp_hdcp* m, char* media_type){

	if (m->hdcp_on == 1){                                  //if hdcp is needed...
        if (get_hdcp_status()){                        //only start SKE if hdcp is still disabled
		    if (!(strcmp(media_type, "video")))	{
		        hoi_drv_hdcp_viden_eti();
		    	printf("Video encryption enabled (eti)!\n"); //... audio encryption is already enabled
		    }
		    if (!(strcmp(media_type, "audio")))	{
		    	hoi_drv_hdcp_auden_eti();
		    	printf("Audio encryption enabled (eti)!\n"); //... video encryption is already enabled
		    }
		}
		else {
		    //no hdcp encryption running, start SKE
		    char port_nr_string[10];
		    char riv[17];
		    char session_key[33];
		    char *ip;
		 	uint32_t hdcp_keys[6];
		    ip=reg_get("remote-uri");
		  	ip += 7;
		   	printf("Remote-uri: %s\n",ip);
		    sprintf(port_nr_string, "%d",m->port_nr);
		    if (receiver(port_nr_string,ip,session_key,riv,hdcp_keys)){
		        hoi_drv_hdcp(hdcp_keys); 				//write keys to kernel
			    if (!(strcmp(media_type, "video")))	{
//DISABLED -> NO KEYS		    	//hoi_drv_hdcp_adv9889en();   		//enable hdcp encryption in AD9889
			        hoi_drv_hdcp_viden_eti();
			    	printf("Video encryption enabled (eti)!\n");
			    }
			    if (!(strcmp(media_type, "audio")))	{
//DISABLED -> NO KEYS			    //hoi_drv_hdcp_adv9889en();			//enable hdcp encryption in AD9889
			    	hoi_drv_hdcp_auden_eti();
			    	printf("Audio encryption enabled (eti)!\n");
			    }
		    }
		    else{
		    	return RSCP_HDCP_ERROR; //abort if SKE was not successful
		    }
        }
    }
	return RSCP_SUCCESS;
}
