
#ifndef EVENT_H_
#define EVENT_H_

#define E_CNT                   (E_HOI_CNT+E_ADV7441A_CNT+E_VSI_CNT)

/* ADV7441A events */
#define E_ADV7441A_ID		    (0x10000000) /* ADV7441A event ID */
#define E_ADV7441A_CNT          7
#define E_ADV7441A_NC		    (E_ADV7441A_ID + 0x00000001)  /* no HDMI connected */
#define E_ADV7441A_CONNECT      (E_ADV7441A_ID + 0x00000008)  /* HDMI connected */
#define E_ADV7441A_NEW_RES      (E_ADV7441A_ID + 0x00000002)	/* Resolution change */
#define E_ADV7441A_NEW_AUDIO    (E_ADV7441A_ID + 0x00000003)  /* new audio content received */ 
#define E_ADV7441A_NO_AUDIO     (E_ADV7441A_ID + 0x00000004)  /* no audio avaible */
#define E_ADV7441A_HDCP         (E_ADV7441A_ID + 0x00000005)  /* HDCP encrypted content */
#define E_ADV7441A_NO_HDCP      (E_ADV7441A_ID + 0x00000006)  /* no HDCP encrypted content */
#define E_ADV7441A_HDCP_KEY_ERR (E_ADV7441A_ID + 0x00000007)  /* HDCP key error (not read or error) */
/* HDCP events */
#define E_HDCP_STREAMING_ERROR (E_ADV7441A_ID + 0x00000009)  /* HDCP Error, streaming without permission */

/* ETO events */
#define E_ADV9889_ID            (0x90000000)
#define E_ADV9889_CNT           3
#define E_ADV9889_CABLE_ON      (E_ADV9889_ID + 0x00000001)  /* HDMI connected */
#define E_ADV9889_CABLE_OFF     (E_ADV9889_ID + 0x00000002)  /* no HDMI connected */
#define E_ADV9889_NEXT_EDID     (E_ADV9889_ID + 0x00000003)  /* next edid segment ready */

/* VSI events */
#define E_VSI_ID		        (0x20000000)
#define E_VSI_CNT		        2
#define E_VSI_FIFO2_FULL	    (E_VSI_ID + 0x00000001)
#define E_VSI_CDFIFO_FULL	    (E_VSI_ID + 0x00000002)

/* VSO events */
#define E_VSO_ID    	        (0x40000000)
#define E_VSO_CNT               5
#define E_VSO_CHOKED            (E_VSO_ID + 0x00000001)
#define E_VSO_PAYLOAD_EMPTY     (E_VSO_ID + 0x00000002)
#define E_VSO_PAYLOAD_MTIMEOUT  (E_VSO_ID + 0x00000003)
#define E_VSO_TIMESTAMP_ERROR   (E_VSO_ID + 0x00000004)
#define E_VSO_ST_ACTIVE         (E_VSO_ID + 0x00000005)

/* ASI events */
#define E_ASI_ID                (0x50000000)
#define E_ASI_CNT               1
#define E_ASI_RBF_ERROR         (E_ASI_ID + 0x00000001)
#define E_ASI_NEW_FS            (E_ASI_ID + 0x00000002)

/* ASO events */
#define E_ASO_ID                (0x60000000)
#define E_ASO_CNT               4
#define E_ASO_SIZE_ERROR        (E_ASO_ID + 0x00000001)
#define E_ASO_FIFO_EMPTY        (E_ASO_ID + 0x00000002)
#define E_ASO_FIFO_FULL         (E_ASO_ID + 0x00000003)
#define E_ASO_RAM_FULL          (E_ASO_ID + 0x00000004)

/* ETI events */
#define E_ETI_ID                (0x70000000)
#define E_ETI_CNT               6
#define E_ETI_LINK_UP           (E_ETI_ID + 0x00000001)
#define E_ETI_LINK_DOWN         (E_ETI_ID + 0x00000002)
#define E_ETI_VIDEO_ON          (E_ETI_ID + 0x00000003)
#define E_ETI_VIDEO_OFF         (E_ETI_ID + 0x00000004)
#define E_ETI_AUDIO_ON          (E_ETI_ID + 0x00000005)
#define E_ETI_AUDIO_OFF         (E_ETI_ID + 0x00000006)

/* ETO events */
#define E_ETO_ID                (0x80000000)
#define E_ETO_CNT               6
#define E_ETO_LINK_UP           (E_ETO_ID + 0x00000001)
#define E_ETO_LINK_DOWN         (E_ETO_ID + 0x00000002)
#define E_ETO_VIDEO_ON          (E_ETO_ID + 0x00000003)
#define E_ETO_VIDEO_OFF         (E_ETO_ID + 0x00000004)
#define E_ETO_AUDIO_ON          (E_ETO_ID + 0x00000005)
#define E_ETO_AUDIO_OFF         (E_ETO_ID + 0x00000006)


/* VIO events */
#define E_VIO_ID                (0x30000000)
#define E_VIO_CNT               15
#define E_VIO_JD0ENC_OOS        (E_VIO_ID + 1)
#define E_VIO_JD1ENC_OOS        (E_VIO_ID + 2)
#define E_VIO_JD2ENC_OOS        (E_VIO_ID + 3)
#define E_VIO_JD3ENC_OOS        (E_VIO_ID + 4)
#define E_VIO_PLL_OOS           (E_VIO_ID + 5)
#define E_VIO_PLL_SNC           (E_VIO_ID + 6)
#define E_VIO_RES_CHANGE        (E_VIO_ID + 7)
// 4x for each ADV (0,1,2,3) [8..15]
#define E_VIO_ADV212_CFERR      (E_VIO_ID + 8)
#define E_VIO_ADV212_CFTH       (E_VIO_ID + 12)

#endif /* EVENT_H_ */ 

