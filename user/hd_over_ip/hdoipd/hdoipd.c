/*
 * Copyright (c) 2010, Institute of Embedded Systems,
 *                     Zurich University of Applied Sciences
 *
 * Authors:
 *   Tobias Klauser <klto@zhaw.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

#include <hdoip/hdoip.h>
#include <hdoip/rtcp.h>

#define PROGRAM			"hdoipd"

#define PORT			9005
#define DEST_MCAST_ADDR		"226.90.0.4"

#define SEND_INTERVAL_SEC	1
#define SEND_INTERVAL_USEC	MSEC_TO_USEC(0)

static bool running = true;
static bool verbose = false;

static void usage_and_exit(int rc) __noreturn;

static void sighandler(int signo __unused)
{
	running = false;
}

/**
 * Display program usage and exit program.
 *
 * This function does not return.
 *
 * \param rc the return value of the program returned to the calling process,
 *           usually either EXIT_FAILURE or EXIT_SUCCESS
 */
static void usage_and_exit(int rc)
{
	fprintf(stderr, "usage: %s [OPTION]...\n\n"
			" -i <interface>  select interface to bind to\n"
			" -p <port>       destination port for the RCTP messages\n"
			" -v              verbose mode\n"
			" -h              display this help\n"
	                "", PROGRAM);
	exit(rc);
}

int main(int argc, char **argv)
{
	int ret = EXIT_FAILURE;
	int optn;
	int sock_send, sock_recv;
	char *ifname = NULL;
	struct in_addr mcast_addr;
	struct sockaddr_in si_send, si_recv;
	size_t slen = sizeof(struct sockaddr_in);
	struct ip_mreq imr;
	unsigned char send_buf[512], recv_buf[512];
	struct rtcp_header *h;
	struct rtcp_sender_info *si;
	struct rtcp_report_block *rb;
	int val;
	int nfds;
	unsigned int i;
	unsigned int n_sent, n_recvd;
	struct timeval next;
	bool timer_expired;

	for (optn = 1; optn < argc && argv[optn][0] == '-'; optn++) {
		switch (argv[optn][1]) {
		case 'i':
			ifname = argv[++optn];
			if (!ifname) {
				err("No interface specified for option -i\n");
				usage_and_exit(EXIT_FAILURE);
			}
			break;
		case 'p':
//			port = strtoul(argv[++optn], NULL, 10);
			break;
		case 'v':
			verbose = true;
			break;
		case 'h':
			usage_and_exit(EXIT_SUCCESS);
			break;	/* never reached */
		default:
			err("unknown option: -%c\n", argv[optn][1]);
			usage_and_exit(EXIT_FAILURE);
		}
	}

	sock_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_recv < 0) {
		err("Failed to create socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	sock_send = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_send < 0) {
		close(sock_recv);
		err("Failed to create socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* TODO: Check multicast ability of interface -> IFF_MULTICAST */

	val = 1;
	if (setsockopt(sock_send, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		err("Failed to set SO_REUSEADDR: %s\n", strerror(errno));
		goto out_close;
	}

	if (!inet_aton(DEST_MCAST_ADDR, &mcast_addr)) {
		err("Failed to convert address\n");
		goto out_close;
	}

	memset(&si_recv, 0, sizeof(si_recv));
	si_recv.sin_family = AF_INET;
	si_recv.sin_port = htons(PORT);
	si_recv.sin_addr.s_addr = mcast_addr.s_addr;
	if (bind(sock_recv, (struct sockaddr *) &si_recv, sizeof(si_recv)) < 0) {
		err("Failed to bind to socket: %s\n", strerror(errno));
		goto out_close;
	}

	/* Join multicast group */
	if (!inet_aton(DEST_MCAST_ADDR, &imr.imr_multiaddr)) {
		err("Failed to convert address\n");
		goto out_close;
	}
	imr.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(sock_recv, IPPROTO_IP, IP_ADD_MEMBERSHIP,
	               &imr, sizeof(imr))) {
		err("Failed to join multicast group: %s\n", strerror(errno));
		goto out_close;
	}

	memset(&si_send, 0, sizeof(si_send));
	si_send.sin_family = AF_INET;
	si_send.sin_port = htons(PORT);
	if (!inet_aton(DEST_MCAST_ADDR, &si_send.sin_addr)) {
		err("Failed to convert address\n");
		goto out_close;
	}

	/* Set the interface to send multicast frames on */
	if (ifname) {
		struct ifreq ifr;
		struct ip_mreqn imrn;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
		if (ioctl(sock_send, SIOCGIFINDEX, &ifr) < 0) {
			err("Failed to get address for interface %s: %s\n",
			    ifname, strerror(errno));
			goto out_close;
		}

		memset(&imrn, 0, sizeof(imrn));
		imrn.imr_ifindex = ifr.ifr_ifindex;
		if (setsockopt(sock_send, SOL_IP, IP_MULTICAST_IF,
		               &imrn, sizeof(imrn))) {
			err("Failed to set interface for multicast send\n");
			goto out_close;
		}
	}

	/* Disable loopback of sent multicast packets */
	val = 0;
	if (setsockopt(sock_send, IPPROTO_IP, IP_MULTICAST_LOOP,
	               &val, sizeof(val))) {
		err("Failed to disable loopback: %s\n", strerror(errno));
		goto out_close;
	}

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	/* Build the RTCP SR packet */
	memset(&send_buf, 0, sizeof(send_buf));
	h = (struct rtcp_header *) &send_buf;
	rtcp_header_set_default_header(h);
	rtcp_header_set_packet_type(h, RTCP_SR);
	rtcp_header_set_ssrc(h, 0x12345678);
	h->rc = 1;
	/* Fill sender info */
	si = rtcp_sr_get_sender_info(send_buf);
	si->ntp_ts_msw = 0xbbbbbbbb;
	si->ntp_ts_lsw = 0xcccccccc;
	si->rtp_ts = 0xaaaaaaaa;
	si->sender_packet_count = htonl(42);
	si->sender_octet_count = htonl(42 * 120);
	/* Fill report block */
	rb = rtcp_sr_get_report_block(send_buf);
	rb->ssrc = htonl(0xaabbccdd);
	rb->fraction_lost = 0;
	rb->cum_num_packet_lost = 0;
	rb->ext_high_seq_num_rx = htonl(42);
	rb->interarrival_jitter = htonl(23);
	rb->last_sr = htonl(12);
	rb->delay_since_last_sr = htonl(5);

	rtcp_sr_set_length(h, 1);

	n_sent = n_recvd = 0;
	nfds = max(sock_send, sock_recv);
	gettimeofday(&next, NULL);
	timer_expired = false;

	while (running) {
		fd_set rfds, wfds;
		struct timeval timeout;
		struct timeval now;

		FD_ZERO(&rfds);
		FD_SET(sock_recv, &rfds);
		FD_ZERO(&wfds);
		FD_SET(sock_send, &wfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = MSEC_TO_USEC(100);

		/* Replacement for timerfd, for now */
		gettimeofday(&now, NULL);

		if (timercmp(&now, &next, >=)) {
			struct timeval tmp;

			timer_expired = true;

			/* Reset the timer */
			tmp.tv_sec = SEND_INTERVAL_SEC;
			tmp.tv_usec = SEND_INTERVAL_USEC;
			timeradd(&now, &tmp, &next);
		}

		ret = select(nfds + 1, &rfds, &wfds, NULL, &timeout);
		if (ret < 0) {
			err("Failed to select(): %s\n", strerror(errno));
			running = false;
			continue;
		} else if (ret == 0) {
			/* timeout occured */
			continue;
		}

		if (timer_expired && FD_ISSET(sock_send, &wfds)) {
			if (verbose)
				info("Sending packet to %s\n", DEST_MCAST_ADDR);
			if (sendto(sock_send, send_buf, ntohs(h->length), 0,
			           (struct sockaddr *) &si_send, slen) < 0)
				warn("Failed to send packet\n");
			else
				++n_sent;

			timer_expired = false;
		}

		if (FD_ISSET(sock_recv, &rfds)) {
			struct sockaddr_in sa;
			socklen_t len;
			ssize_t r;

			if (verbose)
				info("Receiving packet\n");
			if ((r = recvfrom(sock_recv, recv_buf, sizeof(recv_buf), 0,
			             (struct sockaddr *) &sa, &len)) < 0)
				warn("Failed to receive packet\n");
			else
				++n_recvd;

			for (i = 0; i < (unsigned int) r; i++) {
				if (i % 16 == 0 && i > 0)
					info("\n");
				if (i % 8 == 0 && i % 16 != 0 && i > 0)
					info(" ");
				info(" %02x", recv_buf[i]);
			}
			info("\n");
		}
	}

	info("\nIn: %d packets, Out: %d packets\n", n_recvd, n_sent);
	ret = EXIT_SUCCESS;

out_close:
	close(sock_send);
	close(sock_recv);

	exit(ret);
}
