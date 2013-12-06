/*
 * fec_rx_struct.h
 *
 *  Created on: 12.02.2013
 *      Author: buan
 */

#ifndef FEC_RX_STRUCT_H_
#define FEC_RX_STRUCT_H_


typedef struct
{
    uint32_t    valid_packets_vid;
    uint32_t    valid_packets_aud_emb;
    uint32_t    valid_packets_aud_board;
    uint32_t    missing_packets_vid;
    uint32_t    missing_packets_aud_emb;
    uint32_t    missing_packets_aud_board;
    uint32_t    fixed_packets_vid;
    uint32_t    fixed_packets_aud_emb;
    uint32_t    fixed_packets_aud_board;
} t_fec_rx;

#endif /* FEC_RX_STRUCT_H_ */
