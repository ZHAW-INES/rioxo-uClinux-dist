#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "hdoip_usbip.h"

enum {
	MONITOR_GROUP_NONE,
	MONITOR_GROUP_KERNEL,
};

static int hdoip_usbip_exit;

static void sighandler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		hdoip_usbip_exit = 1;
}

void hdoip_usbip_monitor_help(void)
{
}

int hdoip_usbip_monitor(int argc, char **argv)
{
	struct sigaction act;
	sigset_t mask;
	int sock, ret;
	struct sockaddr_nl snl;

	/* set signal handler */
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = sighandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock < 0) {
		err("failed to open uevent socket: %s\n", strerror(errno));
		return -1;
	}

	snl.nl_family = AF_NETLINK;
	snl.nl_groups = MONITOR_GROUP_KERNEL;

	ret = bind(sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
	if (ret < 0) {
		err("failed to bind socket: %s\n", strerror(errno));
		close(sock);
		return -1;
	}

	while (!hdoip_usbip_exit) {
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		ret = select(sock + 1, &rfds, NULL, NULL, NULL);
		if (ret < 0) {
			if (errno != EINTR)
				err("failed to select on socket: %s\n", strerror(errno));
			continue;
		}

		if (FD_ISSET(sock, &rfds)) {

		}
	}

	close(sock);

	return 0;
}
