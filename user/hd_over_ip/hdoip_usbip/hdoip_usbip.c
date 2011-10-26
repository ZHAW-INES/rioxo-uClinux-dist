/*
 * Copyright (C) 2011 Institute of Embedded Systems,
 *                    Zurich University of Applied Sciences
 * Copyright (C) 2011 Tobias Klauser <klto@zhaw.ch>
 *
 * Licensed under GPL v2.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "hdoip_usbip.h"
#include "hdoip_usbip_monitor.h"

struct command {
	const char *name;
	int (*cmd)(int argc, char **argv);
	const char *help;
	void (*usage)(void);
};

static struct command cmds[];

static void hdoip_usbip_usage(void)
{
	const struct command *cmd;

	fprintf(stdout, "usage: hdoip_usbip [--debug] COMMAND [COMMAND OPTIONS]\n");
	for (cmd = cmds; cmd->name; cmd++)
		if (cmd->help)
			fprintf(stdout, "  %-12s %s\n", cmd->name, cmd->help);
	fprintf(stdout, "\n");
}

static int hdoip_usbip_help(int argc, char **argv)
{
	const struct command *cmd;
	int ret = 0;

	if (argc > 1) {
		argv++;
		for (cmd = cmds; cmd->name; cmd++)
			if (!strcmp(cmd->name, argv[0]) && cmd->usage) {
				cmd->usage();
				return 0;
			}
		ret = -1;
	}

	hdoip_usbip_usage();

	return ret;
}

static struct command cmds[] = {
	{
		.name	= "help",
		.cmd	= hdoip_usbip_help,
		.help	= "show command help",
		.usage	= NULL,
	},
	{
		.name	= "monitor",
		.cmd	= hdoip_usbip_monitor,
		.help	= "listen to USB pluggin events",
		.usage	= hdoip_usbip_monitor_help,
	},
	{ NULL, NULL, NULL },
};

int main(int argc, char **argv)
{
	return 0;
}
