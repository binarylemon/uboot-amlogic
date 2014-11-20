#ifndef _LINUX_UNALIGNED_BE_BYTESHIFT_H
#define _LINUX_UNALIGNED_BE_BYTESHIFT_H

#include <linux/types.h>

static inline u16 __get_unaligned_be16(const u8 *p)
{
#ifdef CONFIG_M8B
	u16	tmp;
	asm volatile (
		"ldrh	%[tmp], [%[p]]					\n"
		"rev16	%[tmp], %[tmp]					\n"
		:[tmp] "=r" (tmp)
		:[p] "r" (p)
		:"memory", "cc"
	);
	return tmp;
#else
	return p[0] << 8 | p[1];
#endif
}

static inline u32 __get_unaligned_be32(const u8 *p)
{
#ifdef CONFIG_M8B 
	u32 tmp;
	asm volatile (
		"ldr	%[tmp], [%[p]]					\n"
		"rev	%[tmp], %[tmp]					\n"
		:[tmp] "=r" (tmp)
		:[p] "r" (p)
		:"memory", "cc"
	);
	return tmp;
#else
	return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
#endif
}

static inline u64 __get_unaligned_be64(const u8 *p)
{
	return (u64)__get_unaligned_be32(p) << 32 |
	       __get_unaligned_be32(p + 4);
}

static inline void __put_unaligned_be16(u16 val, u8 *p)
{
#ifdef CONFIG_M8B
	asm volatile (
		"rev16	%[tmp], %[tmp]					\n"
		"strh	%[tmp], [%[p]], #2				\n"
		:
		:[p] "r" (p), [tmp] "r" (val)
		:"memory", "cc"
	);
#else
	*p++ = val >> 8;
	*p++ = val;
#endif
}

static inline void __put_unaligned_be32(u32 val, u8 *p)
{
#ifdef CONFIG_M8B
	asm volatile (
		"rev	%[tmp], %[tmp]					\n"
		"str	%[tmp], [%[p]], #4				\n"
		:
		:[p] "r" (p), [tmp] "r" (val)
		:"memory", "cc"
	);
#else
	__put_unaligned_be16(val >> 16, p);
	__put_unaligned_be16(val, p + 2);
#endif
}

static inline void __put_unaligned_be64(u64 val, u8 *p)
{
	__put_unaligned_be32(val >> 32, p);
	__put_unaligned_be32(val, p + 4);
}

static inline u16 get_unaligned_be16(const void *p)
{
	return __get_unaligned_be16((const u8 *)p);
}

static inline u32 get_unaligned_be32(const void *p)
{
	return __get_unaligned_be32((const u8 *)p);
}

static inline u64 get_unaligned_be64(const void *p)
{
	return __get_unaligned_be64((const u8 *)p);
}

static inline void put_unaligned_be16(u16 val, void *p)
{
	__put_unaligned_be16(val, p);
}

static inline void put_unaligned_be32(u32 val, void *p)
{
	__put_unaligned_be32(val, p);
}

static inline void put_unaligned_be64(u64 val, void *p)
{
	__put_unaligned_be64(val, p);
}

#endif /* _LINUX_UNALIGNED_BE_BYTESHIFT_H */
