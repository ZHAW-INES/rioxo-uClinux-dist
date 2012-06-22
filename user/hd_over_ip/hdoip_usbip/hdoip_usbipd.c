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
#include <linux/input.h>
#include <stdbool.h>
#include "hdoip_usbip.h"
#include "hdoip_usbip_event.h"

#define VERSION	"0.0"

int hdoip_usbipd_exit = 0;
int debug = 0;
int readonly = 0;

#define OPTSTRING "drhV"
#define BITS_PER_LONG           (sizeof(long) * 8)
#define NBITS(x)                ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)                  ((x) % BITS_PER_LONG)
#define LONG(x)                 ((x) / BITS_PER_LONG)
#define test_bit(bit, arr)      ((arr[LONG(bit)] >> OFF(bit)) & 1)

#define FILE_EVENT_LIST         "/tmp/used_event_list"

static const struct option options[] = {
	{ "debug", no_argument, NULL, 'd' },
	{ "readonly", no_argument, NULL, 'r' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
};

static void usage_and_exit(int rc)
{
	fprintf(stdout, "usage: hdoip_usbipd [OPTION...] TYPE EVDEV GDEV\n\n"
			"TYPE is either 'keyboard' or 'mouse (not in use -> should be removed)'\n"
			"EVDEV is the input event device node (/dev/input/eventN)\n"
			"GDEV is the HID gadget device node (/dev/hidgN) (not in use -> should be removed)\n\n"
			"options:\n"
			" -d, --debug     enable debugging output\n"
			" -r, --readonly  only read input events, don't forward them\n"
			" -h, --help      show this help and exit\n"
			" -V, --version   show version and exit\n");
	exit(rc);
}

static void sig_handler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		hdoip_usbipd_exit = 1;
}

int write_to_file(char *event, bool set)
{
    int fd;
    int tmp;
    char buff[100];
    bool available[2] = {false, false};
    bool used[2] = {false, false};

    // read file
    fd = open(FILE_EVENT_LIST, O_CREAT|O_RDWR, 0600);
    if(fd != -1) {
        tmp = read(fd, buff, 100);
        if (tmp >=0) {
            // if first event is used
            if (buff[0] == '/') {
                available[0] = true;
                // if new event is already used
                if (strcmp(event, buff) == 0) {
                    used[0] = true;
                    if (set)
                        return (1);
                }
                // if second event is used
                if (buff[strlen(buff)+1] == '/') {
                    available[1] = true;
                    // if new event is already used
                    if (strcmp(event, &buff[strlen(buff)+1]) == 0) {
                        used[1] = true;
                        if (set)
                            return (1);
                    }
                }
            }
        }
    }
    close(fd);

    // clear file
    fd = open(FILE_EVENT_LIST, O_CREAT|O_RDWR, 0600);
    if(fd != -1) { 
        tmp = write(fd, "                                                                                                    ", 100);
        if (tmp <= 0)
            err("cant write to file: %s", FILE_EVENT_LIST);
    } else {
        err("cant open file: %s", FILE_EVENT_LIST);
    }
    close(fd);

    // write file
    fd = open(FILE_EVENT_LIST, O_CREAT|O_RDWR, 0600);
    if(fd != -1) {
        if (set) {
            // write already used event to file, if available
            if (available[0]) {
                tmp = write(fd, buff, (strlen(buff)+1));
                if (tmp <= 0)
                    err("cant write to file: %s", FILE_EVENT_LIST);
            }
            // write new event to file
            tmp = write(fd, event, (strlen(event)+1));
            if (tmp <= 0)
                err("cant write to file: %s", FILE_EVENT_LIST);
        } else {
            // if event not should be deleted
            if (available[0] && !used[0]) {
                tmp = write(fd, buff, (strlen(buff)+1));
                if (tmp <= 0)
                    err("cant write to file: %s", FILE_EVENT_LIST);
            }
            // if event not should be deleted
            if (available[1] && !used[1]) {
                tmp = write(fd, &buff[strlen(buff)+1], (strlen(&buff[strlen(buff)+1])+1));
                if (tmp <= 0)
                    err("cant write to file: %s", FILE_EVENT_LIST);
            }
        }
    } else {
        err("cant open file: %s", FILE_EVENT_LIST);
    }
    close(fd);

    return 0;
}

int detect_keyboard_or_mouse(char *type, char* gdev, int efd)
{
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

    if (ioctl(efd, EVIOCGBIT(EV_KEY, sizeof(bit[EV_KEY])), bit[EV_KEY]) >= 0) {
        // Check for some common keyboard keys to be present
        if (test_bit(KEY_ESC, bit[EV_KEY]) ||
            test_bit(KEY_Q, bit[EV_KEY]) ||
            test_bit(KEY_W, bit[EV_KEY]) ||
            test_bit(KEY_E, bit[EV_KEY]) ||
            test_bit(KEY_R, bit[EV_KEY]) ||
            test_bit(KEY_T, bit[EV_KEY]) ||
            test_bit(KEY_ENTER, bit[EV_KEY])) {
            strcpy(gdev, "/dev/hidg0");
            strcpy(type, "keyboard");
            return 0;
        }
    }

    if (ioctl(efd, EVIOCGBIT(EV_REL, sizeof(bit[EV_REL])), bit[EV_REL]) >= 0) {
        // Check for the X and Y mouse axes to be present
        if (test_bit(REL_X, bit[EV_REL]) ||
            test_bit(REL_Y, bit[EV_REL])) {
            strcpy(gdev, "/dev/hidg1");
            strcpy(type, "mouse");
            return 0;
        }
    }

    return -1;
}

int main(int argc, char **argv)
{
	int rc = -1;
	int o, i;
	struct sigaction act;
	sigset_t mask;
	char *evdev;
    char type[20];
    char gdev[20];
	int efd = -1, gfd = -1;
    bool active = false;

	while ((o = getopt_long(argc, argv, OPTSTRING, options, NULL)) != -1) {
		switch (o) {
		case 'd':
			debug = 1;
			break;
		case 'r':
			readonly = 1;
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

	evdev = argv[optind + 1];
	if (!evdev)
		usage_and_exit(EXIT_FAILURE);

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

    if (write_to_file(evdev, true))
		goto out;

    active = true;

    // poll to open device
    for (i=0; i<8; i++){
    	info("Loop %i\n",i);
      	efd = open(evdev, O_RDONLY);
  		if (efd < 0) {
  			if (i == 7){
  				err("failed to open event device %s: %s\n", evdev, strerror(errno));
  				goto out;
  			} else{
  				usleep(500000);
  			}
  		}else{
  	    	info("Tried to open event dev for %i ms\n",i*500);
  			break;  //opening file was successful, go on
  		}
  	}

    if (detect_keyboard_or_mouse(type, gdev, efd) == -1)
        goto out;

	info("type: %s\n", type);
	info("event device: %s\n", evdev);
	info("gadget device: %s\n", gdev);

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
    if (active)
        write_to_file(evdev, false);
	if (gfd >= 0)
		close(gfd);
	if (efd >= 0)
		close(efd);
	return rc;
}
