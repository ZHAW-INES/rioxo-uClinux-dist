/*
 * hdcp_drv.c
 *
 *  Created on: 26.05.2011
 *      Author: itin
 */

#include "hdcp_drv.h"

/** Get HDMI HDCP status, get HDCP enabled status and get Audio/Video stream enabled status
 *  Compare the states and turn off audio and video stream if they do not match
 *
 * @param all necessary handle pointer
 * @return
 */
int hdcp_drv_handler(t_eti* h_eti, t_eto* h_eto, t_adv7441a* h_adv7441a, t_adv9889* h_adv9889, t_vsi* h_vsi ,t_vso* h_vso, t_asi* h_asi, t_aso* h_aso, uint32_t* h_drivers, t_queue* event_queue)
{
	//get aes status
	uint32_t audio_enc_en_eto, video_enc_en_eto, audio_enc_en_eti, video_enc_en_eti;
	eto_drv_aes_stat(h_eto, &audio_enc_en_eto, &video_enc_en_eto);
	eti_drv_aes_stat(h_eti, &audio_enc_en_eti, &video_enc_en_eti);

    if (*h_drivers & DRV_ADV7441) {  				//HDMI receiver?
    	//check if video encryption status is okay, otherwise stop audio and send black picture
    	if ((h_adv7441a->status & ADV7441A_STATUS_ENCRYPTED)  && ((!(video_enc_en_eto) && (h_vsi->status & VSI_DRV_STATUS_ACTIV)) || (!(audio_enc_en_eto) && (h_asi->stream_status[AUD_STREAM_NR_EMBEDDED] & ASI_DRV_STREAM_STATUS_ACTIV))))
    	{
    	    //vsi_drv_stop(h_vsi);	 				//stop transmitting video
    	    REPORT(INFO, "HDMI AUDIO/VIDEO SOURCE CHANGED, HDCP ERROR!");
    	    queue_put(event_queue, E_HDCP_STREAMING_ERROR);
    	}
    }

    if (*h_drivers & DRV_ADV9889) {  //HDMI transmitter?
    	//check if video encryption status is okay, otherwise stop video
    	if (!((h_adv9889->hdcp_state & HDCP_VID_ACTIVE_LOOP) || (h_adv9889->hdcp_state & HDCP_VID_ACTIVE)) && (video_enc_en_eti) && (h_vso->status & VSO_DRV_STATUS_ACTIV))
    	{
    	   	vso_drv_stop(h_vso);    //stop transmitting video
    	   	REPORT(INFO, "HDMI VIDEO SINK CHANGED, HDCP ERROR!");
            queue_put(event_queue, E_HDCP_STREAMING_ERROR);
    	}
    	//check if audio encryption status is okay, otherwise stop audio
  //  	if (!((h_adv9889->hdcp_state & HDCP_VID_ACTIVE_LOOP) || (h_adv9889->hdcp_state & HDCP_VID_ACTIVE))  && (audio_enc_en_eti) && (h_aso->stream_status & ASO_DRV_STATUS_ACTIV))
  //  	{
  //  	   	aso_drv_stop(h_aso);    //stop transmitting video
  //  	   	REPORT(INFO, "HDMI AUDIO SINK CHANGED, HDCP ERROR!");
  //          queue_put(event_queue, E_HDCP_STREAMING_ERROR);
  //  	}
    }

    return ERR_HDCP_SUCCESS;
}
