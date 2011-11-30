/*
 * Copyright (C) 2011 Institute of Embedded Systems,
 *                    Zurich University of Applied Sciences
 * Copyright (C) 2011 Tobias Klauser <klto@zhaw.ch>
 *
 * Input event handling extracted from the Event device test program (evtest):
 *
 * Copyright (c) 1999-2000 Vojtech Pavlik
 * Copyright (c) 2009-2011 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <linux/input.h>

#include "hdoip_usbip.h"
#include "util.h"

extern int readonly;

#define EV_BUF_SIZE		64
#define KEY_REPORT_SIZE		8
#define MOUSE_REPORT_SIZE	4

#define INVALID (-1)
#define X(v)	((v) - ('a' - 0x04))

static const char *event_names[EV_MAX + 1] = {
	[0 ... EV_MAX] = NULL,
        [EV_SYN] = "Sync",		[EV_KEY] = "Key",
	[EV_REL] = "Relative",		[EV_ABS] = "Absolute",
	[EV_MSC] = "Misc",		[EV_LED] = "LED",
	[EV_SND] = "Sound",		[EV_REP] = "Repeat",
	[EV_FF] = "ForceFeedback",	[EV_PWR] = "Power",
	[EV_FF_STATUS] = "ForceFeedbackStatus",
};

static const unsigned char scancode_mod_map[] = {
	[0 ... 0xFF] = 0,
	[0xE0] = 0x01,             // CTRL left
	[0xE1] = 0x02,             // Shift left
	[0xE2] = 0x04,             // Alt left
	[0xE3] = 0x08,             // GUI left
	[0xE4] = 0x01,             // CTRL right
	[0xE5] = 0x02,             // Shift right
	[0xE6] = 0x04,             // Alt right
	[0xE7] = 0x08              // GUI right
};


int hdoip_usbip_event_print_info(int efd)
{
	int version;
	unsigned short id[4];
	char name[256];

	if (ioctl(efd, EVIOCGVERSION, &version) < 0) {
		err("failed to get input event driver version\n");
		return -1;
	}

	info("Input event driver version: %d.%d.%d\n",
			version >> 16, (version >> 8) & 0xff, version & 0xff);

	if (ioctl(efd, EVIOCGID, id) < 0) {
		err("failed to get input device IDs\n");
		return -1;
	}

	info("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
			id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

	if (ioctl(efd, EVIOCGNAME(sizeof(name)), name) < 0) {
		err("failed to get input device name\n");
		return -1;
	}

	info("Input device name: '%s'\n", name);

	return 0;
}

int hdoip_usbip_event_check_supported(int efd, const char *type)
{
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

	if (ioctl(efd, EVIOCGBIT(0, EV_MAX), bit[0]) < 0)
		return -1;

	if (!strcmp("keyboard", type)) {
		if (!test_bit(EV_KEY, bit[0]) ||
		    !test_bit(EV_MSC, bit[0])) {
			dbg("1st test failed\n");
			goto err_out;
		}

		if (ioctl(efd, EVIOCGBIT(EV_KEY, sizeof(bit[EV_KEY])), bit[EV_KEY]) < 0)
			goto err_out;

		/* Check for some common keyboard keys to be present */
		if (!test_bit(KEY_ESC, bit[EV_KEY]) ||
		    !test_bit(KEY_Q, bit[EV_KEY]) ||
		    !test_bit(KEY_W, bit[EV_KEY]) ||
		    !test_bit(KEY_E, bit[EV_KEY]) ||
		    !test_bit(KEY_R, bit[EV_KEY]) ||
		    !test_bit(KEY_T, bit[EV_KEY]) ||
		    !test_bit(KEY_ENTER, bit[EV_KEY])) {
			dbg("2nd test failed\n");
			goto err_out;
		}
	} else if (!strcmp("mouse", type)) {
		if (!test_bit(EV_REL, bit[0]) ||
		    !test_bit(EV_KEY, bit[0]) ||
		    !test_bit(EV_MSC, bit[0]))
			goto err_out;

		if (ioctl(efd, EVIOCGBIT(EV_REL, sizeof(bit[EV_REL])), bit[EV_REL]) < 0)
			goto err_out;

		/* Check for the X and Y mouse axes to be present */
		if (!test_bit(REL_X, bit[EV_REL]) ||
		    !test_bit(REL_Y, bit[EV_REL]))
			goto err_out;

		if (ioctl(efd, EVIOCGBIT(EV_KEY, sizeof(bit[EV_KEY])), bit[EV_KEY]) < 0)
			goto err_out;

		/* Check for at least one mouse button to be present */
		if (!test_bit(BTN_MOUSE, bit[EV_KEY]))
			goto err_out;
	} else {
		err("invalid event device type '%s'\n", type);
		return -1;
	}

	return 0;

err_out:
	err("event device is not of device type '%s'\n", type);
	return -1;
}

static int filter_key_event(struct input_event *ev)
{
	switch (ev->type) {
	case EV_SYN:
	case EV_MSC:
		return 0;
	default:
		return 1;
	}
}

static int filter_mouse_event(struct input_event *ev)
{
	switch (ev->type) {
	case EV_SYN:
	case EV_REL:
	case EV_MSC:
		return 0;
	default:
		return 1;
	}
}

/* possible return values for the event prepare functions */
enum {
	RET_INVALID = -1,	/* error condition */
	RET_SEND,		/* send the report */
	RET_CONTINUE,		/* contninue adding data to the report */
};

static int prepare_key_event(struct input_event *ev, char *report, const unsigned char *new)
{
	unsigned char scancode;
	unsigned char i;
	unsigned char tag;

	dbg("prepare_key_event\n");

	if ( ev->type == EV_SYN ) return RET_SEND;

	if ( ev->type != EV_MSC ) return RET_INVALID;
	if ( ev->code != MSC_SCAN ) return RET_INVALID;
	if ( (unsigned short)(ev->value>>8) != 0x0700 ) return RET_INVALID;

	scancode = (unsigned char)ev->value;

	tag = scancode_mod_map[ scancode ];
	if ( tag != 0 ) {
		// Modifier
		dbg("modifier\n");
		report[0] ^= tag;
	} else {
		// Key
		dbg("key\n");
		tag = 0;
		for ( i=2; i<KEY_REPORT_SIZE; i++ ) {
			dbg("i = %i, report[i] = %X\n", i, report[i]);
			if ( report[i] == scancode ) {
				// Remove key
				dbg("clear report %i\n", i);
				memmove( &report[i], &report[i+1], (KEY_REPORT_SIZE-i-1) );
				report[KEY_REPORT_SIZE-1] = 0;
				tag = 1;
				break;
			} else if ( report[i] == 0 ) {
				// Add key
				dbg("report[%i] = %X\n", i, scancode);
				report[i] = scancode;
				tag = 2;
				break;				
			}
		}
		if ( tag == 0 ) return RET_INVALID;
	}

	return RET_CONTINUE;
}

static int prepare_mouse_event(struct input_event *ev, char *report, const unsigned char *new)
{
	unsigned char tag;

	dbg("prepare_mouse_event\n");

	if ( *new == 1 ) memset(&report[1], 0, sizeof(report)-1);

	if ( ev->type == EV_SYN ) return RET_SEND;

	if ( ( ev->type == EV_REL ) && ( ev->code == REL_X || ev->code == REL_Y ) ) {

		report[1 + ev->code] = (char) ev->value;
		return RET_CONTINUE;

	}

	if ( ( ev->type == EV_REL ) && ev->code == REL_WHEEL ) {

		report[3] = (char) ev->value;
		return RET_CONTINUE;

	}

	if ( ev->type == EV_MSC ) {
		tag = (unsigned char)ev->value;
		if ( tag == 3 ) tag = 4;
		report[0] ^= tag;
	}

	return RET_INVALID;
}

int hdoip_usbip_event_loop(int efd, int gfd, const char *type)
{
	int rc = 0;
	ssize_t rd;
	struct input_event *ev;
	unsigned int i;
	char report[KEY_REPORT_SIZE];
	ssize_t report_size;
	unsigned int idx;
	int ret;
	unsigned char new_report;
	int (*filter_event)(struct input_event *);
	int (*prepare_event)(struct input_event *, char *, const unsigned char *);

	ev = malloc(EV_BUF_SIZE * sizeof(struct input_event));
	if (!ev) {
		err("failed to allocate input event buffer\n");
		return -1;
	}

	if (type[0] == 'k') {
		report_size = KEY_REPORT_SIZE;
		prepare_event = prepare_key_event;
		filter_event = filter_key_event;
	} else {
		report_size = MOUSE_REPORT_SIZE;
		prepare_event = prepare_mouse_event;
		filter_event = filter_mouse_event;
	}

	memset(report, 0, sizeof(report));
	new_report = 1;

	while (!hdoip_usbipd_exit) {
		fd_set rfds;
		struct timeval timeout;
		size_t n_events;

		FD_ZERO(&rfds);
		FD_SET(efd, &rfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;

		ret = select(efd + 1, &rfds, NULL, NULL, &timeout);
		if (ret < 0) {
			if (errno == EINTR)
				continue;	/* interrupted by signal */

			err("select() on input event device failed: %s\n", strerror(errno));
			rc = -1;
			break;
		} else if (ret == 0)
			continue;	/* timeout */

		if (!FD_ISSET(efd, &rfds))
			continue;	/* no event data */

		rd = read(efd, ev, EV_BUF_SIZE * sizeof(struct input_event));
		if (rd < (ssize_t) sizeof(struct input_event)) {
			err("expected %zu bytes, got %zd\n", sizeof(struct input_event), rd);
			rc = -1;
			break;
		}

		n_events = rd / sizeof(struct input_event);

		for (i = 0; i < n_events; i++) {
			/* Do we care about the event? */
			if (filter_event(&ev[i]))
				continue;

			dbg("Input event #%d: time %ld.%06ld type %d (%s), code %d, value %d\n",
					i + 1, ev[i].time.tv_sec, ev[i].time.tv_usec,
					ev[i].type, event_names[ev[i].type],
					ev[i].code, ev[i].value);

			ret = prepare_event( &ev[i], report, &new_report );
			new_report = 0;

			if ( ret == RET_SEND ) {
				dbg( "Report: %X %X %X %X\n", report[0], report[1], report[2], report[3] );
				if (!readonly) {
					if (write(gfd, report, report_size) != report_size) {
						err("failed to write report to gadget\n");
						return -1;
					}
				}
				new_report = 1;
			}

		}

	}

	info("shutting down...\n");
	free(ev);
	return rc;
}

