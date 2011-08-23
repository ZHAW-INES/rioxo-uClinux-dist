#ifndef _UTIL_H_
#define _UTIL_H_

#define BITS_PER_LONG		(sizeof(long) * 8)
#define NBITS(x)		((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)			((x) % BITS_PER_LONG)
#define BIT(x)			(1UL << OFF(x))
#define LONG(x)			((x) / BITS_PER_LONG)
#define test_bit(bit, arr)	((arr[LONG(bit)] >> OFF(bit)) & 1)

#endif /* _UTIL_H_ */
