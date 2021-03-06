/* vi: set sw=4 ts=4: */
/* uname -- print system information
 * Copyright (C) 1989-1999 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/uname.html */

/* Option		Example
 * -s, --sysname	SunOS
 * -n, --nodename	rocky8
 * -r, --release	4.0
 * -v, --version
 * -m, --machine	sun
 * -a, --all		SunOS rocky8 4.0  sun
 *
 * The default behavior is equivalent to '-s'.
 *
 * David MacKenzie <djm@gnu.ai.mit.edu>
 *
 * GNU coreutils 6.10:
 * Option:                      struct   Example(s):
 *                              utsname
 *                              field:
 * -s, --kernel-name            sysname  Linux
 * -n, --nodename               nodename localhost.localdomain
 * -r, --kernel-release         release  2.6.29
 * -v, --kernel-version         version  #1 SMP Sun Jan 11 20:52:37 EST 2009
 * -m, --machine                machine  x86_64   i686
 * -p, --processor              (none)   x86_64   i686
 * -i, --hardware-platform      (none)   x86_64   i386
 *      NB: vanilla coreutils reports "unknown" -p and -i,
 *      x86_64 and i686/i386 shown above are Fedora's inventions.
 * -o, --operating-system       (none)   GNU/Linux
 * -a, --all: all of the above, in the order shown.
 *      If -p or -i is not known, don't show them
 */

/* Busyboxed by Erik Andersen
 *
 * Before 2003: Glenn McGrath and Manuel Novoa III
 *  Further size reductions.
 * Mar 16, 2003: Manuel Novoa III (mjn3@codepoet.org)
 *  Now does proper error checking on i/o.  Plus some further space savings.
 * Jan 2009:
 *  Fix handling of -a to not print "unknown", add -o and -i support.
 */

#include <sys/utsname.h>
#include "libbb.h"

typedef struct {
	struct utsname name;
	char processor[sizeof(((struct utsname*)NULL)->machine)];
	char platform[sizeof(((struct utsname*)NULL)->machine)];
	char os[sizeof("GNU/Linux")];
} uname_info_t;

static const char options[] ALIGN1 = "snrvmpioa";
static const unsigned short utsname_offset[] = {
	offsetof(uname_info_t, name.sysname), /* -s */
	offsetof(uname_info_t, name.nodename), /* -n */
	offsetof(uname_info_t, name.release), /* -r */
	offsetof(uname_info_t, name.version), /* -v */
	offsetof(uname_info_t, name.machine), /* -m */
	offsetof(uname_info_t, processor), /* -p */
	offsetof(uname_info_t, platform), /* -i */
	offsetof(uname_info_t, os), /* -o */
};

int uname_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int uname_main(int argc UNUSED_PARAM, char **argv)
{
#if ENABLE_GETOPT_LONG
	static const char longopts[] ALIGN1 =
		/* name, has_arg, val */
		"all\0"               No_argument       "a"
		"kernel-name\0"       No_argument       "s"
		"nodename\0"          No_argument       "n"
		"kernel-release\0"    No_argument       "r"
		"release\0"           No_argument       "r"
		"kernel-version\0"    No_argument       "v"
		"machine\0"           No_argument       "m"
		"processor\0"         No_argument       "p"
		"hardware-platform\0" No_argument       "i"
		"operating-system\0"  No_argument       "o"
	;
#endif
	uname_info_t uname_info;
#if defined(__sparc__) && defined(__linux__)
	char *fake_sparc = getenv("FAKE_SPARC");
#endif
	const char *unknown_str = "unknown";
	const char *fmt;
	const unsigned short *delta;
	unsigned toprint;

	IF_GETOPT_LONG(applet_long_options = longopts);
	toprint = getopt32(argv, options);

	if (argv[optind]) { /* coreutils-6.9 compat */
		bb_show_usage();
	}

	if (toprint & (1 << 8)) { /* -a => all opts on */
		toprint = (1 << 8) - 1;
		unknown_str = ""; /* -a does not print unknown fields */
	}

	if (toprint == 0) { /* no opts => -s (sysname) */
		toprint = 1;
	}

	uname(&uname_info.name); /* never fails */

#if defined(__sparc__) && defined(__linux__)
	if (fake_sparc && (fake_sparc[0] | 0x20) == 'y') {
		strcpy(uname_info.name.machine, "sparc");
	}
#endif
	strcpy(uname_info.processor, unknown_str);
	strcpy(uname_info.platform, unknown_str);
	strcpy(uname_info.os, "GNU/Linux");
#if 0
	/* Fedora does something like this */
	strcpy(uname_info.processor, uname_info.name.machine);
	strcpy(uname_info.platform, uname_info.name.machine);
	if (uname_info.platform[0] == 'i'
	 && uname_info.platform[1]
	 && uname_info.platform[2] == '8'
	 && uname_info.platform[3] == '6'
	) {
		uname_info.platform[1] = '3';
	}
#endif

	delta = utsname_offset;
	fmt = " %s" + 1;
	do {
		if (toprint & 1) {
			const char *p = (char *)(&uname_info) + *delta;
			if (p[0]) {
				printf(fmt, p);
				fmt = " %s";
			}
		}
		++delta;
	} while (toprint >>= 1);
	bb_putchar('\n');

	fflush_stdout_and_exit(EXIT_SUCCESS); /* coreutils-6.9 compat */
}
