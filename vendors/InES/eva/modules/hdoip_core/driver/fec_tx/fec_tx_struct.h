/*
 * fec_tx_struct.h
 *
 *  Created on: 18.10.2010
 *      Author: buan
 */

#ifndef FEC_TX_STRUCT_H_
#define FEC_TX_STRUCT_H_


typedef struct
{
    uint8_t     video_enable;
    uint8_t     video_l;
    uint8_t     video_d;
    uint8_t     video_interleaving;
    uint8_t     video_col_only;
    uint8_t     audio_enable;
    uint8_t     audio_l;
    uint8_t     audio_d;
    uint8_t     audio_interleaving;
    uint8_t     audio_col_only;
} t_fec_setting;

#endif /* FEC_TX_STRUCT_H_ */
