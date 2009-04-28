/* vi: set sw=4 ts=4: */
/*
 * adduser - add users to /etc/passwd and /etc/shadow
 *
 * Copyright (C) 1999 by Lineo, inc. and John Beppu
 * Copyright (C) 1999,2000,2001 by John Beppu <beppu@codepoet.org>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */
#include "libbb.h"

/* #define OPT_HOME           (1 << 0) */ /* unused */
/* #define OPT_GECOS          (1 << 1) */ /* unused */
#define OPT_SHELL          (1 << 2)
#define OPT_GID            (1 << 3)
#define OPT_DONT_SET_PASS  (1 << 4)
#define OPT_SYSTEM_ACCOUNT (1 << 5)
#define OPT_DONT_MAKE_HOME (1 << 6)
#define OPT_UID            (1 << 7)

/* We assume UID_T_MAX == INT_MAX */
/* remix */
/* recoded such that the uid may be passed in *p */
static void passwd_study(struct passwd *p)
{
	int max = UINT_MAX;

	if (getpwnam(p->pw_name)) {
		bb_error_msg_and_die("%s '%s' in use", "user", p->pw_name);
		/* this format string is reused in adduser and addgroup */
	}

	if (!(option_mask32 & OPT_UID)) {
		if (option_mask32 & OPT_SYSTEM_ACCOUNT) {
			p->pw_uid = 100; /* FIRST_SYSTEM_UID */
			max = 999;       /* LAST_SYSTEM_UID */
		} else {
			p->pw_uid = 1000; /* FIRST_UID */
			max = 64999;      /* LAST_UID */
		}
	}
	/* check for a free uid (and maybe gid) */
	while (getpwuid(p->pw_uid) || (p->pw_gid == (gid_t)-1 && getgrgid(p->pw_uid))) {
		if (option_mask32 & OPT_UID) {
			/* -u N, cannot pick uid other than N: error */
			bb_error_msg_and_die("%s '%s' in use", "uid", itoa(p->pw_uid));
			/* this format string is reused in adduser and addgroup */
		}
		if (p->pw_uid == max) {
			bb_error_msg_and_die("no %cids left", 'u');
		}
		p->pw_uid++;
	}

	if (p->pw_gid == (gid_t)-1) {
		p->pw_gid = p->pw_uid; /* new gid = uid */
		if (getgrnam(p->pw_name)) {
			bb_error_msg_and_die("%s '%s' in use", "group", p->pw_name);
			/* this format string is reused in adduser and addgroup */
		}
	}
}

static void addgroup_wrapper(struct passwd *p)
{
	char *cmd;

	cmd = xasprintf("addgroup -g %u '%s'", (unsigned)p->pw_gid, p->pw_name);
	system(cmd);
	free(cmd);
}

static void passwd_wrapper(const char *login) NORETURN;

static void passwd_wrapper(const char *login)
{
	static const char prog[] ALIGN1 = "passwd";

	BB_EXECLP(prog, prog, login, NULL);
	bb_error_msg_and_die("can't execute %s, you must set password manually", prog);
}

#if ENABLE_FEATURE_ADDUSER_LONG_OPTIONS
static const char adduser_longopts[] ALIGN1 =
		"home\0"                Required_argument "h"
		"gecos\0"               Required_argument "g"
		"shell\0"               Required_argument "s"
		"ingroup\0"             Required_argument "G"
		"disabled-password\0"   No_argument       "D"
		"empty-password\0"      No_argument       "D"
		"system\0"              No_argument       "S"
		"no-create-home\0"      No_argument       "H"
		"uid\0"                 Required_argument "u"
		;
#endif

/*
 * adduser will take a login_name as its first parameter.
 * home, shell, gecos:
 * can be customized via command-line parameters.
 */
int adduser_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int adduser_main(int argc UNUSED_PARAM, char **argv)
{
	struct passwd pw;
	const char *usegroup = NULL;
	char *p;
	unsigned opts;

#if ENABLE_FEATURE_ADDUSER_LONG_OPTIONS
	applet_long_options = adduser_longopts;
#endif

	/* got root? */
	if (geteuid()) {
		bb_error_msg_and_die(bb_msg_perm_denied_are_you_root);
	}

	pw.pw_gecos = (char *)"Linux User,,,";
	pw.pw_shell = (char *)DEFAULT_SHELL;
	pw.pw_dir = NULL;

	/* exactly one non-option arg */
	/* disable interactive passwd for system accounts */
	opt_complementary = "=1:SD:u+";
	if (sizeof(pw.pw_uid) == sizeof(int)) {
		opts = getopt32(argv, "h:g:s:G:DSHu:", &pw.pw_dir, &pw.pw_gecos, &pw.pw_shell, &usegroup, &pw.pw_uid);
	} else {
		unsigned uid;
		opts = getopt32(argv, "h:g:s:G:DSHu:", &pw.pw_dir, &pw.pw_gecos, &pw.pw_shell, &usegroup, &uid);
		if (opts & OPT_UID) {
			pw.pw_uid = uid;
		}
	}
	argv += optind;

	/* fill in the passwd struct */
	pw.pw_name = argv[0];
	die_if_bad_username(pw.pw_name);
	if (!pw.pw_dir) {
		/* create string for $HOME if not specified already */
		pw.pw_dir = xasprintf("/home/%s", argv[0]);
	}
	pw.pw_passwd = (char *)"x";
	if (opts & OPT_SYSTEM_ACCOUNT) {
		if (!usegroup) {
			usegroup = "nogroup";
		}
		if (!(opts & OPT_SHELL)) {
			pw.pw_shell = (char *) "/bin/false";
		}
	}
	pw.pw_gid = usegroup ? xgroup2gid(usegroup) : -1; /* exits on failure */

	/* make sure everything is kosher and setup uid && maybe gid */
	passwd_study(&pw);

	p = xasprintf("x:%u:%u:%s:%s:%s",
			(unsigned) pw.pw_uid, (unsigned) pw.pw_gid,
			pw.pw_gecos, pw.pw_dir, pw.pw_shell);
	if (update_passwd(bb_path_passwd_file, pw.pw_name, p, NULL) < 0) {
		return EXIT_FAILURE;
	}
	if (ENABLE_FEATURE_CLEAN_UP)
		free(p);

#if ENABLE_FEATURE_SHADOWPASSWDS
	p = xasprintf("!:%u:0:99999:7:::", (unsigned)(time(NULL) / 86400)); /* sp->sp_lstchg */
	/* ignore errors: if file is missing we suppose admin doesn't want it */
	update_passwd(bb_path_shadow_file, pw.pw_name, p, NULL);
	if (ENABLE_FEATURE_CLEAN_UP)
		free(p);
#endif

	/* add to group */
	/* addgroup should be responsible for dealing w/ gshadow */
	/* if using a pre-existing group, don't create one */
	if (!usegroup)
		addgroup_wrapper(&pw);

	/* clear the umask for this process so it doesn't
	 * screw up the permissions on the mkdir and chown. */
	umask(0);
	if (!(opts & OPT_DONT_MAKE_HOME)) {
		/* set the owner and group so it is owned by the new user,
		 * then fix up the permissions to 2755. Can't do it before
		 * since chown will clear the setgid bit */
		if (mkdir(pw.pw_dir, 0755)
		 || chown(pw.pw_dir, pw.pw_uid, pw.pw_gid)
		 || chmod(pw.pw_dir, 02755) /* set setgid bit on homedir */
		) {
			bb_simple_perror_msg(pw.pw_dir);
		}
	}

	if (!(opts & OPT_DONT_SET_PASS)) {
		/* interactively set passwd */
		passwd_wrapper(pw.pw_name);
	}

	return 0;
}
