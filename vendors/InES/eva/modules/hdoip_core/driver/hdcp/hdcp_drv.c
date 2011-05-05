
#include "hdcp_drv.h"
//#include "hoi_drv.h"

/** Get HDMI HDCP status, get HDCP enabled status and get Audio/Video stream enabled status
 *  Compare the states and turn of audio and video stream if they do not match (STOP vsi and vso)
 *
 * @param handle pointer
 * @return error code
 */
int hdcp_drv_handler(t_eti* h_eti, t_eto* h_eto, t_adv7441a* h_adv7441a, t_adv9889* h_adv9889, t_vsi* h_vsi ,t_vso* h_vso, t_asi* h_asi, t_aso* h_aso, uint32_t* h_drivers)
{
	//get aes status
    uint32_t audio_enc_en_eto, video_enc_en_eto, audio_enc_en_eti, video_enc_en_eti;
	eto_drv_aes_stat(h_eto, &audio_enc_en_eto, &video_enc_en_eto);
	eti_drv_aes_stat(h_eti, &audio_enc_en_eti, &video_enc_en_eti);

    if (*h_drivers & DRV_ADV7441) {  //HDMI receiver?
    	//check if video encryption status is okay, otherwise stop video
    	if ((h_adv7441a->status & ADV7441A_STATUS_ENCRYPTED)  && !(video_enc_en_eto) && (h_vsi->status & VSI_DRV_STATUS_ACTIV))
    	{
    	    vsi_drv_stop(h_vsi);	 //stop transmitting video
    	    REPORT(INFO, "HDMI VIDEO SOURCE CHANGED, HDCP ERROR!");
    	}
    	//check if audio encryption status is okay, otherwise stop audio
    	if ((h_adv7441a->status & ADV7441A_STATUS_ENCRYPTED)  && !(audio_enc_en_eto) && (h_asi->status & ASI_DRV_STATUS_ACTIV))
    	{
    	    asi_drv_stop(h_asi);	 //stop transmitting audio
    	    REPORT(INFO, "HDMI AUDIO SOURCE CHANGED, HDCP ERROR!");
    	}
    }
    //NOT YET ENABLED BECAUSE NO KEYS AVAILABLE FOR AD9889
    if (*h_drivers & DRV_ADV9889) {  //HDMI transmitter?
    	//check if video encryption status is okay, otherwise stop video
   /* 	if (!(h_adv9889->hdcp_state & HDCP_VID_ACTIVE_LOOP)  && (video_enc_en_eti) && (h_vso->status & VSO_DRV_STATUS_ACTIV))
    	{
    	   	vso_drv_stop(h_vso);    //stop transmitting video
    	   	REPORT(INFO, "HDMI VIDEO SINK CHANGED, HDCP ERROR!");
    	}
    	//check if audio encryption status is okay, otherwise stop audio
    	if (!(h_adv9889->hdcp_state & HDCP_VID_ACTIVE_LOOP)  && (audio_enc_en_eti) && (h_aso->status & ASO_DRV_STATUS_ACTIV))
    	{
    	   	aso_drv_stop(h_aso);    //stop transmitting video
    	   	REPORT(INFO, "HDMI AUDIO SINK CHANGED, HDCP ERROR!");
    	}*/
    }
    return ERR_HDCP_SUCCESS;
}




/*int hdcp_drv_handler(t_hoi* handle)
//int hdcp_drv_handler()
{
	//get aes status
    uint32_t audio_enc_en_eto, video_enc_en_eto, audio_enc_en_eti, video_enc_en_eti;
	eto_drv_aes_stat(handle->eto, &audio_enc_en_eto, &video_enc_en_eto);
	eti_drv_aes_stat(handle->eti, &audio_enc_en_eti, &video_enc_en_eti);

    if (handle->drivers & DRV_ADV7441) {  //HDMI receiver?
    	//check if video encryption status is okay, otherwise stop video
    	if ((handle->adv7441a.status & ADV7441A_STATUS_ENCRYPTED)  && !(video_enc_en_eto) && (handle->vsi.status & VSI_DRV_STATUS_ACTIV))
    	{
    	    vsi_drv_stop(&handle->vsi);	 //stop transmitting video
    	    REPORT(INFO, "HDMI VIDEO SOURCE CHANGED, HDCP ERROR!");
    	}
    	//check if audio encryption status is okay, otherwise stop audio
    	if ((handle->adv7441a.status & ADV7441A_STATUS_ENCRYPTED)  && !(audio_enc_en_eto) && (handle->asi.status & ASI_DRV_STATUS_ACTIV))
    	{
    	    asi_drv_stop(&handle->asi);	 //stop transmitting audio
    	    REPORT(INFO, "HDMI AUDIO SOURCE CHANGED, HDCP ERROR!");
    	}
    }
    if (handle->drivers & DRV_ADV9889) {  //HDMI transmitter?
    	//check if video encryption status is okay, otherwise stop video
    	if (!(handle->adv9889.hdcp_state & HDCP_VID_ACTIVE_LOOP)  && (video_enc_en_eti) && (handle->vso.status & VSO_DRV_STATUS_ACTIV))
    	{
    	   	vso_drv_stop(&handle->vso);    //stop transmitting video
    	   	REPORT(INFO, "HDMI VIDEO SINK CHANGED, HDCP ERROR!");
    	}
    	//check if audio encryption status is okay, otherwise stop audio
    	if (!(handle->adv9889.hdcp_state & HDCP_VID_ACTIVE_LOOP)  && (audio_enc_en_eti) && (handle->aso.status & ASO_DRV_STATUS_ACTIV))
    	{
    	   	aso_drv_stop(&handle->aso);    //stop transmitting video
    	   	REPORT(INFO, "HDMI AUDIO SINK CHANGED, HDCP ERROR!");
    	}
    }
    return ERR_HDCP_SUCCESS;
}*/





//int hdcp_drv_handler(t_eti* handle_eti, t_eto* handle_eto, t_adv7441a* handle_adv7441a, t_vsi* handle_vsi ,t_vso* handle_vso, uint32_t* handle_drivers, t_adv9889* handle, t_queue* event_queue)
//get aes status
/*uint32_t audio_enc_en_eto, video_enc_en_eto, audio_enc_en_eti, video_enc_en_eti;
eto_drv_aes_stat(handle_eto, &audio_enc_en_eto, &video_enc_en_eto); //TODO: check return value
eti_drv_aes_stat(handle_eti, &audio_enc_en_eti, &video_enc_en_eti); //TODO: check return value
*/

/*    if ((handle_adv7441a->status & ADV7441A_STATUS_ENCRYPTED)  && !(audio_enc_en_eto||video_enc_en_eto||audio_enc_en_eti||video_enc_en_eti) && (handle_vsi->status & VSI_DRV_STATUS_ACTIV))
    {
        vsi_drv_stop(handle_vsi);	 //stop transmitting audio and video
        vso_drv_stop(handle_vso);    //stop transmitting audio and video
        REPORT(INFO, "SOURCE CHANGED, HDCP ERROR!");
    }*/

