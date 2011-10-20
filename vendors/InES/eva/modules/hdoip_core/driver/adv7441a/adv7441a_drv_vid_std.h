#ifndef ADV7441A_DRV_VID_STD_H_
#define ADV7441A_DRV_VID_STD_H_

typedef struct {
	uint32_t		fs;
	uint8_t			bit_width;
	uint8_t			channel_cnt;
	uint8_t			mute;
} t_adv7441a_aud_st;

typedef struct {
	uint8_t			vsync_pol;
	uint8_t			hsync_pol;
    uint8_t         field_pol;
	uint16_t		pixel_clk;    
    uint8_t         interlaced;

    uint32_t        h_total_line_width;
    uint32_t        h_line_width;
    uint32_t        h_front_porch_width;
    uint32_t        h_sync_width;
    uint32_t        h_back_porch_width;

    uint32_t        f0_total_height;
    uint32_t        f0_height;
    uint32_t        f0_front_porch_width;
    uint32_t        f0_vs_pulse_width;
    uint32_t        f0_back_porch_width;

    uint32_t        f1_total_height;
    uint32_t        f1_height;
    uint32_t        f1_front_porch_width;
    uint32_t        f1_vs_pulse_width;
    uint32_t        f1_back_porch_width;

} t_adv7441a_vid_st;


#endif /* ADV7441A_DRV_VID_STD_H_ */
