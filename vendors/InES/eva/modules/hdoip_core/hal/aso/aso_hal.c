
#include "aso_hal.h"


void aso_set_aud_params(void* p, struct hdoip_aud_params* aud_params) 
{ 
    HOI_WR32(p, ASO_OFF_CH_CNT_LEFT, aud_params->ch_cnt_left);    
    HOI_WR32(p, ASO_OFF_CH_CNT_RIGHT, aud_params->ch_cnt_right);   
    HOI_WR32(p, ASO_OFF_CONTAINER, aud_bits_to_container(aud_params->sample_width)); 
}

void aso_get_aud_params(void* p, struct hdoip_aud_params* aud_params)
{
    aud_params->ch_cnt_left = HOI_RD32(p, ASO_OFF_CH_CNT_LEFT);
    aud_params->ch_cnt_right = HOI_RD32(p, ASO_OFF_CH_CNT_RIGHT);
    aud_params->sample_width = aud_container_to_bits(HOI_RD32(p, ASO_OFF_CONTAINER));
}

uint8_t aso_set_aud_cfg(void* p, uint8_t ch_cnt_l, uint8_t ch_cnt_r, uint8_t bits) 
{
    uint32_t container = aud_bits_to_container(bits);

    HOI_WR32(p, ASO_OFF_CH_CNT_LEFT, ch_cnt_l);    
    HOI_WR32(p, ASO_OFF_CH_CNT_RIGHT, ch_cnt_r);   
    HOI_WR32(p, ASO_OFF_CONTAINER, container); 

    return 0;
} 

void aso_get_aud_cfg(void* p, uint8_t* ch_cnt_l, uint8_t* ch_cnt_r, uint8_t* bits) 
{
    *ch_cnt_l = HOI_RD32(p, ASO_OFF_CH_CNT_LEFT);
    *ch_cnt_r = HOI_RD32(p, ASO_OFF_CH_CNT_RIGHT);
    *bits = aud_container_to_bits(HOI_RD32(p, ASO_OFF_CONTAINER));
}

