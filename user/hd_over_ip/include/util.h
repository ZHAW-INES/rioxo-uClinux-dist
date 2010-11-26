#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <net/if.h>
#include <net/ethernet.h>

#define err(fmt, args...)	fprintf(stderr, "Error: " fmt, ##args)
#ifdef DEBUG
# define dbg(fmt, args...)	fprintf(stdout, fmt, ##args)
#endif

#define MAC_FMT		"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARR(x)	x[0], x[1], x[2], x[3], x[4], x[5]

struct interface {
	int sock;
	char ifname[IF_NAMESIZE];
	uint8_t hwaddr[ETHER_ADDR_LEN];
};

/**
 * Check whether two strings match exactly.
 *
 * @param ctx  the reference string (usually a defined constant or string under
 *             the programs control)
 * @param arg  the string to match against the reference string (usually the
 *             user-supplied string)
 * @return     true if the strings match
 */
static inline int xstrmatch(const char *ctx, const char *arg)
{
	size_t len = strlen(ctx);
	return (strlen(arg) == len && strncmp(arg, ctx, len) == 0);
}

#endif /* _UTIL_H_ */
