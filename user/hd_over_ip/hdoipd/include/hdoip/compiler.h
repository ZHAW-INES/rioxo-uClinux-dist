#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifdef __GNUC__
# define __packed	__attribute__ ((packed))
# define __unused	__attribute__ ((unused))
# define likely		__builtin_expect(!!(x), 1)
# define unlikely	__builtin_expect(!!(x), 0)
#else
# define __packed
# define __unused
# define likely(x)	(x)
# define unlikely(x)	(x)
#endif /* __GNUC__ */

#undef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)

#undef container_of
#define container_of(ptr, type, member) ({				\
		const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
		(type *)( (char *)__mptr - offsetof(type, member) );})

#endif /* _COMPILER_H_ */
