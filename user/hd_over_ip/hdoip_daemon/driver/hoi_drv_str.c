/*
 * hoi_drv_str.c
 *
 *  Created on: 10.12.2010
 *      Author: alda
 */
#include "hoi_drv_user.h"

const char* event_str(uint32_t e)
{
    switch (e) {
        case E_ADV7441A_NC: return "no HDMI connected";
        case E_ADV7441A_CONNECT: return "HDMI connected";
        case E_ADV7441A_NEW_HDMI_RES: return "HDMI resolution changed";
        case E_ADV7441A_NEW_AUDIO: return "new audio content received";
        case E_ADV7441A_NO_AUDIO: return "no audio available";
        case E_ADV7441A_HDCP: return "HDCP encrypted content";
        case E_ADV7441A_NO_HDCP: return "no HDCP encrypted content";
        case E_ADV7441A_NEW_VGA_RES: return "VGA resolution changed";
        case E_ADV7441A_ACTIVITY_ON_SYNC: return "activity on video in sync detected";

        case E_GS2971_VIDEO_ON: return "SDI video input on";
        case E_GS2971_VIDEO_OFF: return "SDI video input off";
        case E_GS2971_LOOP_ON: return "SDI loopback on";
        case E_GS2971_LOOP_OFF: return "SDI loopback off";

        case E_ADV9889_CABLE_ON: return "HDMI sink connected";
        case E_ADV9889_CABLE_OFF: return "HDMI sink disconnected";
        case E_ADV9889_HDCP_NOT_OK: return "HDCP Link is not OK";

        case E_VSI_FIFO2_FULL: return "vsi fifo 2 full";
        case E_VSI_CDFIFO_FULL: return "vsi code fifo full";

        case E_VSO_CHOKED: return "vso choked";
        case E_VSO_PAYLOAD_EMPTY: return "vso payload empty";
        case E_VSO_PAYLOAD_MTIMEOUT: return "vso marker timeout";
        case E_VSO_TIMESTAMP_ERROR: return "vso timestamp error";
        case E_VSO_ST_ACTIVE: return "vso stream active";

        case E_ASI_NEW_FS: return "asi detected new audio sampling rate";

        case E_ASO_EMB_TS_ERROR: return "audio_emb: aso timestamp error";
        case E_ASO_BOARD_TS_ERROR: return "audio_board: aso timestamp error";
        case E_ASO_EMB_FIFO_EMPTY: return "audio_emb: aso fifo underflow";
        case E_ASO_BOARD_FIFO_EMPTY: return "audio_board: aso fifo underflow";
        case E_ASO_EMB_FIFO_FULL: return "audio_emb: aso fifo overflow";
        case E_ASO_BOARD_FIFO_FULL: return "audio_board: aso fifo overflow";

        case E_ETI_LINK_UP: return "Ethernet Input Link UP";
        case E_ETI_LINK_DOWN: return "Ethernet Input Link DOWN";
        case E_ETO_LINK_UP: return "Ethernet Output Link UP";
        case E_ETO_LINK_DOWN: return "Ethernet Output Link DOWN";
        case E_ETO_VIDEO_ON: return "Ethernet Video stream out started";
        case E_ETO_VIDEO_OFF: return "Ethernet Video stream out stopped";

        case E_ETO_AUDIO_EMB_ON: return "Ethernet Audio embedded stream out started";
        case E_ETO_AUDIO_EMB_OFF: return "Ethernet Audio embedded stream out stopped";
        case E_ETO_AUDIO_BOARD_ON: return "Ethernet Audio board stream out started";
        case E_ETO_AUDIO_BOARD_OFF: return "Ethernet Audio board stream out stopped";
        case E_ETI_VIDEO_ON: return "Ethernet Video stream in started";
        case E_ETI_VIDEO_OFF: return "Ethernet Video stream in stopped";
        case E_ETI_AUDIO_EMB_ON: return "Ethernet Audio embedded stream in started";
        case E_ETI_AUDIO_EMB_OFF: return "Ethernet Audio embedded stream in stopped";
        case E_ETI_AUDIO_BOARD_ON: return "Ethernet Audio board stream in started";
        case E_ETI_AUDIO_BOARD_OFF: return "Ethernet Audio board stream in stopped";

        case E_VIO_JD0ENC_OOS: return "ADV212[0] out of sync";
        case E_VIO_JD1ENC_OOS: return "ADV212[1] out of sync";
        case E_VIO_JD2ENC_OOS: return "ADV212[2] out of sync";
        case E_VIO_JD3ENC_OOS: return "ADV212[3] out of sync";
        case E_VIO_PLL_OOS: return "video PLL out of sync";
        case E_VIO_PLL_SNC: return "video PLL sync";
        case E_VIO_RES_CHANGE: return "vio resolution change";
        case E_VIO_ADV212_CFERR: return "ADV212[0] code fifo error";
        case E_VIO_ADV212_CFERR+1: return "ADV212[1] code fifo error";
        case E_VIO_ADV212_CFERR+2: return "ADV212[2] code fifo error";
        case E_VIO_ADV212_CFERR+3: return "ADV212[3] code fifo error";
        case E_VIO_ADV212_CFTH: return "ADV212[0] code fifo threshold";
        case E_VIO_ADV212_CFTH+1: return "ADV212[1] code fifo threshold";
        case E_VIO_ADV212_CFTH+2: return "ADV212[2] code fifo threshold";
        case E_VIO_ADV212_CFTH+3: return "ADV212[3] code fifo threshold";

        case E_BDT_RESET_BUTTON: return "reset button was pressed";
    }
    return "unknown";
}




