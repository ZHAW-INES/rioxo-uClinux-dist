#ifndef _RTCP_H_
#define _RTCP_H_

#include <hdoip/hdoip.h>

enum {
	RTCP_SR		= 200,
	RTCP_RR		= 201,
	RTCP_SDES	= 202,
	RTCP_BYE	= 203,
	RTCP_APP	= 204,
};

struct rtcp_header {
#if defined(HDOIP_ENDIAN_BIG)
	uint8_t version:2;
	uint8_t padding:1;
	uint8_t rc:5;
	uint8_t packet_type;
#else
	uint8_t rc:5;
	uint8_t padding:1;
	uint8_t version:2;
	uint8_t packet_type;
#endif
	uint16_t length;
	uint32_t ssrc;
#define csrc	ssrc;
} __packed;

/* SR or RR packet */

struct rtcp_sr_rr_sender_info {
	uint32_t ntp_ts_msw;
	uint32_t ntp_ts_lsw;
	uint32_t rtp_ts;
	uint32_t sender_packet_count;
	uint32_t sender_octet_count;
} __packed;

struct rtcp_sr_rr_report_block {
	uint32_t ssrc;
	uint8_t fraction_lost;
	uint32_t cum_num_packet_lost:24;
	uint32_t ext_high_seq_num_rx;
	uint32_t interarrival_jitter;
	uint32_t last_sr;
	uint32_t delay_since_last_sr;
} __packed;

/* SDES packet */

enum {
	RTCP_SDES_END	= 0,
	RTCP_SDES_CNAME	= 1,
	RTCP_SDES_NAME	= 2,
	RTCP_SDES_EMAIL	= 3,
	RTCP_SDES_PHONE	= 4,
	RTCP_SDES_LOC	= 5,
	RTCP_SDES_TOOL	= 6,
	RTCP_SDES_NOTE	= 7,
	RTCP_SDES_PRIV	= 8,
	RTCP_SDES_MAX	= 9,
};

/* BYE packet */

struct rtcp_bye_reason {
	uint8_t len;
} __packed;

/* APP packet */

/* The RC field in the RTCP header is used as subtype in APP packets */
#define subtype		rc

#endif /* _RTCP_H_ */
