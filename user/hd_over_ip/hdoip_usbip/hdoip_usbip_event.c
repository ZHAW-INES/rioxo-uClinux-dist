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

#include <linux/input.h>

#include "hdoip_usbip.h"
#include "util.h"

#define EV_BUF_SIZE		32
#define KEY_REPORT_SIZE		8
#define MOUSE_REPORT_SIZE	3

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

static const char keys_mod_map[] = {
	[0 ... KEY_MAX] = INVALID,
	[KEY_LEFTCTRL] = 0x01,	[KEY_RIGHTCTRL] = 0x10,
	[KEY_LEFTSHIFT] = 0x02,	[KEY_RIGHTSHIFT] = 0x20,
	[KEY_LEFTALT] = 0x04,	[KEY_RIGHTALT] = 0x40,
};

/* Map input event code to report bit for USB HID gadget */
static const char keys_val_map[] = {
	[0 ... KEY_MAX] = INVALID,
	/* Keyboard */
	[KEY_RESERVED] = INVALID,		[KEY_ESC] = 0x29,
	[KEY_1] = 0x1E,				[KEY_2] = 0x1F,
	[KEY_3] = 0x20,				[KEY_4] = 0x21,
	[KEY_5] = 0x22,				[KEY_6] = 0x23,
	[KEY_7] = 0x24,				[KEY_8] = 0x25,
	[KEY_9] = 0x26,				[KEY_0] = 0x27,
	[KEY_MINUS] = 0x2d,			[KEY_EQUAL] = 0x2e,
	[KEY_BACKSPACE] = 0x2a,			[KEY_TAB] = 0x2b,
	[KEY_Q] = X('q'),			[KEY_W] = X('w'),
	[KEY_E] = X('e'),			[KEY_R] = X('r'),
	[KEY_T] = X('t'),			[KEY_Y] = X('y'),
	[KEY_U] = X('u'),			[KEY_I] = X('i'),
	[KEY_O] = X('o'),			[KEY_P] = X('p'),
	[KEY_LEFTBRACE] = 0x2f,			[KEY_RIGHTBRACE] = 0x30,
	[KEY_ENTER] = 0x28,
	[KEY_A] = X('a'),			[KEY_S] = X('s'),
	[KEY_D] = X('d'),			[KEY_F] = X('f'),
	[KEY_G] = X('g'),			[KEY_H] = X('h'),
	[KEY_J] = X('j'),			[KEY_K] = X('k'),
	[KEY_L] = X('l'),			[KEY_SEMICOLON] = 0x33,
	[KEY_APOSTROPHE] = 0x34,		[KEY_GRAVE] = 0x35,

	[KEY_Z] = X('z'),			[KEY_X] = X('x'),
	[KEY_C] = X('c'),			[KEY_V] = X('v'),
	[KEY_B] = X('b'),			[KEY_N] = X('n'),
	[KEY_M] = X('m'),			[KEY_COMMA] = 0x36,
	[KEY_DOT] = 0x37,			[KEY_SLASH] = 0x38,
	[KEY_LEFTALT] = INVALID,		[KEY_SPACE] = 0x2c,
	[KEY_CAPSLOCK] = INVALID,		[KEY_F1] = 0x3a,
	[KEY_F2] = 0x3b,			[KEY_F3] = 0x3c,
	[KEY_F4] = 0x3d,			[KEY_F5] = 0x3e,
	[KEY_F6] = 0x3f,			[KEY_F7] = 0x40,
	[KEY_F8] = 0x41,			[KEY_F9] = 0x42,
	[KEY_F10] = 0x43,			[KEY_F11] = 0x44,
	[KEY_F12] = 0x45,

	/* Keypad */
	[KEY_KPSLASH] = 0x54,			[KEY_KPASTERISK] = 0x55,
	[KEY_KPMINUS] = 0x56,			[KEY_KPPLUS] = 0x57,
	[KEY_KPENTER] = 0x58,			[KEY_KP1] = 0x59,
	[KEY_KP2] = 0x5a,			[KEY_KP3] = 0x5b,
	[KEY_KP4] = 0x5c,			[KEY_KP5] = 0x5d,
	[KEY_KP6] = 0x5e,			[KEY_KP7] = 0x5f,
	[KEY_KP8] = 0x60,			[KEY_KP9] = 0x61,
	[KEY_KP0] = 0x62,			[KEY_KPDOT] = 0x63,
						[KEY_KPEQUAL] = 0x67,

	/* Mouse buttons */
	[BTN_LEFT] = 0x01,			[BTN_RIGHT] = 0x02,
	[BTN_MIDDLE] = 0x03,
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
	case EV_KEY:
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
	case EV_KEY:
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

static int prepare_key_event(struct input_event *ev, char *report,
			     unsigned int *idx, int *hold)
{
	int ret;

	/* index too large */
	if (*idx + 1 > KEY_REPORT_SIZE)
		return RET_INVALID;

	switch (ev->type) {
	case EV_SYN:
		ret = RET_SEND;
		break;
	case EV_KEY:
		if (ev->value == 0) {
			*hold = 0;
		} else {
			/* do we have a modifier key? */
			if (keys_mod_map[ev->code] != INVALID) {
				report[0] |= keys_mod_map[ev->code];
				*hold = (ev->value == 2);
			} else if (keys_val_map[ev->code] != INVALID) {
				report[2 + *idx] = keys_val_map[ev->code];
				(*idx)++;
				*hold = (ev->value == 2);
			}
		}

		ret = RET_CONTINUE;
		break;
	default:
		ret = RET_INVALID;	/* event not properly filtered */
	}

	return 0;
}

static int prepare_mouse_event(struct input_event *ev, char *report,
			       unsigned int __unused *idx, int *hold)
{
	int ret;

	switch (ev->type) {
	case EV_SYN:
		ret = RET_SEND;
		break;
	case EV_REL:
		/*
		 * the stack will make sure we only get values in the proper
		 * range, so we can safely cast it here.
		 */
		if (ev->code > 1) {
			warn("invalid code value in input device event\n");
			ret = RET_INVALID;
		} else {
			report[1 + ev->code] = (char) ev->value;
			ret = RET_CONTINUE;
		}
		break;
	case EV_KEY:
		report[0] |= keys_val_map[ev->code];
		ret = RET_CONTINUE;
		break;
	default:
		ret = RET_INVALID;	/* event not properly filtered */
	}

	return ret;
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
	int hold;
	int report_sent;
	int (*filter_event)(struct input_event *);
	int (*prepare_event)(struct input_event *, char *, unsigned int *, int *);

	ev = malloc(EV_BUF_SIZE * sizeof(struct input_event));
	if (!ev) {
		err("failed to allocate input event buffer\n");
		return -1;
	}

	if (type[0] == 'k') {
		report_size = KEY_REPORT_SIZE;
		filter_event = filter_key_event;
		prepare_event = prepare_key_event;
	} else {
		report_size = MOUSE_REPORT_SIZE;
		filter_event = filter_mouse_event;
		prepare_event = prepare_mouse_event;
	}

	hold = 0;
	report_sent = 0;

	while (!hdoip_usbipd_exit) {
		/* TODO: do a select() here */

		rd = read(efd, ev, EV_BUF_SIZE * sizeof(struct input_event));
		if (rd < (ssize_t) sizeof(struct input_event)) {
			err("expected %zu bytes, got %zd\n", sizeof(struct input_event), rd);
			rc = -1;
			break;
		}

		for (i = 0; i < rd / sizeof(struct input_event); i++) {
			/* Do we care about the event? */
			if (filter_event(&ev[i]))
				continue;

#if 0
			dbg("Event: time %ld.%06ld, type %d (%s), code %d, value %d\n",
					ev[i].time.tv_sec, ev[i].time.tv_usec,
					ev[i].type, event_names[ev[i].type],
					ev[i].code, ev[i].value);
#endif

			ret = prepare_event(&ev[i], report, &idx, &hold);
			switch (ret) {
			case RET_CONTINUE:
				/* continue filling the report */
				report_sent = 0;
				break;
			case RET_SEND:
				/* write the report to the gadget */
				if (write(gfd, report, report_size) != report_size) {
					err("failed to write report to gadget\n");
					return -1;
				}
				/* FALLTHROUGH */
			default:
				/* error, start a new report */
				report_sent = (ret == RET_SEND) ? 1 : 0;
				idx = 0;
				memset(report, 0, sizeof(report));
				break;
			}
		}

		if (!report_sent)
			continue;

		report_sent = 0;
		idx = 0;
		memset(report, 0, sizeof(report));

		if (!hold) {
			if (write(gfd, report, report_size) != report_size) {
				err("failed to write report to gadget\n");
				rc = -1;
				break;
			}
		}
	}

	info("shutting down...\n");
	free(ev);
	return rc;
}
