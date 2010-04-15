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
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <hdoip/hdoip.h>
#include <hdoip/rtcp.h>

#define PORT			9004
#define DEST_MCAST_ADDR		"226.90.0.4"

#define SEND_INTERVAL_SEC	1
#define SEND_INTERVAL_USEC	MSEC_TO_USEC(0)

static bool running = true;

static void sighandler(int signo __unused)
{
	running = false;
}

int main(int argc, char **argv)
{
	int ret = EXIT_FAILURE;
	int sock_send, sock_recv;
	struct in_addr mcast_addr, local_addr;
	struct sockaddr_in si_send, si_recv;
	size_t slen = sizeof(struct sockaddr_in);
	struct ip_mreq imr;
	int val;
	unsigned char send_buf[512], recv_buf[512];
	struct rtcp_header *h;
	int nfds;
	unsigned int i;
	unsigned int n_sent, n_recvd;
	struct timeval next;
	bool timer_expired;

	if (argc < 2) {
		fprintf(stderr, "usage: hdoipd <local IP>\n");
		exit(EXIT_FAILURE);
	}

	if (!inet_aton(argv[1], &local_addr)) {
		err("Failed to convert address\n");
		exit(EXIT_FAILURE);
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
	               (const char *) &imr, sizeof(imr))) {
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

	/* Disable loopback */
	val = 0;
	if (setsockopt(sock_send, IPPROTO_IP, IP_MULTICAST_LOOP,
	               (const char *) &val, sizeof(val))) {
		err("Failed to disable loopback: %s\n", strerror(errno));
		goto out_close;
	}

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	/* Build the RTCP APP packet */
	memset(&send_buf, 0, sizeof(send_buf));
	h = (struct rtcp_header *) &send_buf;
	h->version = 2;
	h->padding = 0;
	h->subtype = 0;
	h->packet_type = 204;
	/* length will be set later */
	h->ssrc = 0x12345678;
	strncpy((char *) (send_buf + sizeof(struct rtcp_header)), "HELO", 4);
	for (i = sizeof(struct rtcp_header) + 4; i < sizeof(struct rtcp_header) + 4 + 32; i++)
		send_buf[i] = i;
	h->length = sizeof(struct rtcp_header) + 4 + 32;

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
			info("Sending packet\n");
			if (sendto(sock_send, send_buf, sizeof(send_buf), 0,
			           (struct sockaddr *) &si_send, slen) < 0)
				warn("Failed to send packet\n");
			++n_sent;
			timer_expired = false;
		}

		if (FD_ISSET(sock_recv, &rfds)) {
			struct sockaddr_in sa;
			socklen_t len;
			ssize_t r;

			info("Receiving packet\n");
			if ((r = recvfrom(sock_recv, recv_buf, sizeof(recv_buf), 0,
			             (struct sockaddr *) &sa, &len)) < 0)
				warn("Failed to receive packet\n");
			for (i = 0; i < (unsigned int) r; i++) {
				if (i % 16 == 0 && i > 0)
					info("\n");
				if (i % 8 == 0 && i % 16 != 0 && i > 0)
					info(" ");
				info(" %02x", recv_buf[i]);
			}
			info("\n");

			++n_recvd;
		}
	}

	info("In: %d packets, Out: %d packets\n", n_recvd, n_sent);
	ret = EXIT_SUCCESS;

out_close:
	close(sock_send);
	close(sock_recv);

	exit(ret);
}
