/* vi: set sw=4 ts=4: */
/*
 * Mini df implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * based on original code by (I think) Bruce Perens <bruce@pixar.com>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 _NOT_ compliant -- option -t missing. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/df.html */

/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Size reduction.  Removed floating point dependency.  Added error checking
 * on output.  Output stats on 0-sized filesystems if specifically listed on
 * the command line.  Properly round *-blocks, Used, and Available quantities.
 *
 * Aug 28, 2008      Bernhard Reutner-Fischer
 *
 * Implement -P and -B; better coreutils compat; cleanup
 */

#include <mntent.h>
#include <sys/vfs.h>
#include "libbb.h"

#if !ENABLE_FEATURE_HUMAN_READABLE
static unsigned long kscale(unsigned long b, unsigned long bs)
{
	return (b * (unsigned long long) bs + 1024/2) / 1024;
}
#endif

int df_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int df_main(int argc, char **argv)
{
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	unsigned long df_disp_hr = 1024;
	int status = EXIT_SUCCESS;
	unsigned opt;
	FILE *mount_table;
	struct mntent *mount_entry;
	struct statfs s;
	static const char ignored_mounts[] ALIGN1 = "rootfs\0";

	enum {
		OPT_KILO  = (1 << 0),
		OPT_POSIX = (1 << 1),
		OPT_ALL   = (1 << 2) * ENABLE_FEATURE_DF_FANCY,
		OPT_INODE = (1 << 3) * ENABLE_FEATURE_DF_FANCY,
		OPT_BSIZE = (1 << 4) * ENABLE_FEATURE_DF_FANCY,
		OPT_HUMAN = (1 << (2 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
		OPT_MEGA  = (1 << (3 + 3*ENABLE_FEATURE_DF_FANCY)) * ENABLE_FEATURE_HUMAN_READABLE,
	};
	const char *disp_units_hdr = NULL;
	char *chp;

#if ENABLE_FEATURE_HUMAN_READABLE && ENABLE_FEATURE_DF_FANCY
	opt_complementary = "k-mB:m-Bk:B-km";
#elif ENABLE_FEATURE_HUMAN_READABLE
	opt_complementary = "k-m:m-k";
#endif
	opt = getopt32(argv, "kP"
			IF_FEATURE_DF_FANCY("aiB:")
			IF_FEATURE_HUMAN_READABLE("hm")
			IF_FEATURE_DF_FANCY(, &chp));
	if (opt & OPT_MEGA)
		df_disp_hr = 1024*1024;

	if (opt & OPT_BSIZE)
		df_disp_hr = xatoul_range(chp, 1, ULONG_MAX); /* disallow 0 */

	/* From the manpage of df from coreutils-6.10:
	   Disk space is shown in 1K blocks by default, unless the environment
	   variable POSIXLY_CORRECT is set, in which case 512-byte blocks are used.
	*/
	if (getenv("POSIXLY_CORRECT")) /* TODO - a new libbb function? */
		df_disp_hr = 512;

	if (opt & OPT_HUMAN) {
		df_disp_hr = 0;
		disp_units_hdr = "     Size";
	}
	if (opt & OPT_INODE)
		disp_units_hdr = "   Inodes";

	if (disp_units_hdr == NULL) {
#if ENABLE_FEATURE_HUMAN_READABLE
		disp_units_hdr = xasprintf("%s-blocks",
			make_human_readable_str(df_disp_hr, 0, !!(opt & OPT_POSIX)));
#else
		disp_units_hdr = xasprintf("%lu-blocks", df_disp_hr);
#endif
	}
	printf("Filesystem           %-15sUsed Available %s Mounted on\n",
			disp_units_hdr, (opt & OPT_POSIX) ? "Capacity" : "Use%");

	mount_table = NULL;
	argv += optind;
	if (optind >= argc) {
		mount_table = setmntent(bb_path_mtab_file, "r");
		if (!mount_table)
			bb_perror_msg_and_die(bb_path_mtab_file);
	}

	while (1) {
		const char *device;
		const char *mount_point;

		if (mount_table) {
			mount_entry = getmntent(mount_table);
			if (!mount_entry) {
				endmntent(mount_table);
				break;
			}
		} else {
			mount_point = *argv++;
			if (!mount_point)
				break;
			mount_entry = find_mount_point(mount_point, bb_path_mtab_file);
			if (!mount_entry) {
				bb_error_msg("%s: can't find mount point", mount_point);
 set_error:
				status = EXIT_FAILURE;
				continue;
			}
		}

		device = mount_entry->mnt_fsname;
		mount_point = mount_entry->mnt_dir;

		if (statfs(mount_point, &s) != 0) {
			bb_simple_perror_msg(mount_point);
			goto set_error;
		}

		if ((s.f_blocks > 0) || !mount_table || (opt & OPT_ALL)) {
			if (opt & OPT_INODE) {
				s.f_blocks = s.f_files;
				s.f_bavail = s.f_bfree = s.f_ffree;
				s.f_bsize = 1;

				if (df_disp_hr)
					df_disp_hr = 1;
			}
			blocks_used = s.f_blocks - s.f_bfree;
			blocks_percent_used = 0;
			if (blocks_used + s.f_bavail) {
				blocks_percent_used = (blocks_used * 100ULL
						+ (blocks_used + s.f_bavail)/2
						) / (blocks_used + s.f_bavail);
			}

			/* GNU coreutils 6.10 skip certain mounts, try to be compatible.  */
			if (index_in_strings(device, ignored_mounts) != -1)
				continue;

#ifdef WHY_WE_DO_IT_FOR_DEV_ROOT_ONLY
/* ... and also this is the only user of find_block_device */
			if (strcmp(device, "/dev/root") == 0) {
				/* Adjusts device to be the real root device,
				* or leaves device alone if it can't find it */
				device = find_block_device("/");
				if (!device) {
					goto set_error;
				}
			}
#endif

			if (printf("\n%-20s" + 1, device) > 20)
				    printf("\n%-20s", "");
#if ENABLE_FEATURE_HUMAN_READABLE
			printf(" %9s ",
				make_human_readable_str(s.f_blocks, s.f_bsize, df_disp_hr));

			printf(" %9s " + 1,
				make_human_readable_str((s.f_blocks - s.f_bfree),
						s.f_bsize, df_disp_hr));

			printf("%9s %3u%% %s\n",
					make_human_readable_str(s.f_bavail, s.f_bsize, df_disp_hr),
					blocks_percent_used, mount_point);
#else
			printf(" %9lu %9lu %9lu %3u%% %s\n",
					kscale(s.f_blocks, s.f_bsize),
					kscale(s.f_blocks - s.f_bfree, s.f_bsize),
					kscale(s.f_bavail, s.f_bsize),
					blocks_percent_used, mount_point);
#endif
		}
	}

	return status;
}
