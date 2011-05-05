/*
 * hdcp.c
 *
 *  Created on: 28.04.2011
 *      Author: itin
 */
#include "hdcp.h"


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
    	if (!(get_hdcp_status())){
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

	    //only start SKE if hdcp is still disabled
	    if (get_hdcp_status()){
	    	if (!(strcmp(media_type, "video")))	{
	    		hoi_drv_hdcp_viden();
	    		printf("Video encryption enabled!\n"); //... audio encryption is already enabled
	    	}
	    	if (!(strcmp(media_type, "audio")))	{
	    		hoi_drv_hdcp_auden();
	    		printf("Audio encryption enabled!\n"); //... video encryption is already enabled
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
			    	hoi_drv_hdcp_viden();
			        printf("Video encryption enabled!\n");
			    }
			    if (!(strcmp(media_type, "audio")))	{
			    	hoi_drv_hdcp_auden();
			        printf("Audio encryption enabled!\n");
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
		        hoi_drv_hdcp_viden();
		    	printf("Video encryption enabled!\n"); //... audio encryption is already enabled
		    }
		    if (!(strcmp(media_type, "audio")))	{
		    	hoi_drv_hdcp_auden();
		    	printf("Audio encryption enabled!\n"); //... video encryption is already enabled
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
			        hoi_drv_hdcp_viden();
			    	printf("Video encryption enabled!\n");
			    }
			    if (!(strcmp(media_type, "audio")))	{
//DISABLED -> NO KEYS			    //hoi_drv_hdcp_adv9889en();			//enable hdcp encryption in AD9889
			    	hoi_drv_hdcp_auden();
			    	printf("Audio encryption enabled!\n");
			    }
		    }
		    else{
		    	return RSCP_HDCP_ERROR; //abort if SKE was not successful
		    }
        }
    }
	return RSCP_SUCCESS;
}
