/*
 * Copyright (C) 2011 Institute of Embedded Systems,
 *                    Zurich University of Applied Sciences
 * Copyright (C) 2011 Tobias Klauser <klto@zhaw.ch>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "hdoip_usbip.h"
#include "hdoip_usbip_event.h"

#define VERSION	"0.0"

int hdoip_usbipd_exit = 0;
int debug = 0;

#define OPTSTRING "dhV"

static const struct option options[] = {
	{ "debug", no_argument, NULL, 'd' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static void usage_and_exit(int rc)
{
	fprintf(stdout, "usage: hdoip_usbipd [OPTION...] TYPE EVDEV GDEV\n\n"
			"TYPE is either 'keyboard' or 'mouse'\n"
			"EVDEV is the input event device node (/dev/input/eventN)\n"
			"GDEV is the HID gadget device node (/dev/hidgN)\n\n"
			"options:\n"
			" -d, --debug    enable debugging output\n"
			" -h, --help     show this help and exit\n"
			" -V, --version  show version and exit\n");
	exit(rc);
}

static void sig_handler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		hdoip_usbipd_exit = 1;
}

int main(int argc, char **argv)
{
	int rc = -1;
	int o;
	struct sigaction act;
	sigset_t mask;
	char *type, *evdev, *gdev;
	int efd = -1, gfd = -1;

	while ((o = getopt_long(argc, argv, OPTSTRING, options, NULL)) != -1) {
		switch (o) {
		case 'd':
			debug = 1;
			break;
		case 'V':
			fprintf(stdout, "%s\n", VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
			usage_and_exit(EXIT_SUCCESS);
		default:
			usage_and_exit(EXIT_FAILURE);
		}
	}

	type = argv[optind];
	if (!type || (strcmp("keyboard", type) && strcmp("mouse", type)))
		usage_and_exit(EXIT_FAILURE);

	evdev = argv[optind + 1];
	if (!evdev)
		usage_and_exit(EXIT_FAILURE);

	gdev = argv[optind + 2];
	if (!gdev)
		usage_and_exit(EXIT_FAILURE);

	info("type: %s\n", type);
	info("event device: %s\n", evdev);
	info("gadget device: %s\n", gdev);

	/* set signal handler */
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = sig_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	efd = open(evdev, O_RDONLY);
	if (efd < 0) {
		err("failed to open event device %s: %s\n", evdev, strerror(errno));
		goto out;
	}

	gfd = open(gdev, O_RDWR, 0666);
	if (gfd < 0) {
		err("failed to open gadget device %s: %s\n", gdev, strerror(errno));
		goto out;
	}

	if (hdoip_usbip_event_print_info(efd) < 0)
		goto out;

	if (hdoip_usbip_event_check_supported(efd, type) < 0)
		goto out;

	/* select loop on efd */
	rc = hdoip_usbip_event_loop(efd, gfd, type);

out:
	if (gfd >= 0)
		close(gfd);
	if (efd >= 0)
		close(efd);
	return rc;
}
