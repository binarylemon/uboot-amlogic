#ifndef _LINUX_UNALIGNED_ACCESS_OK_H
#define _LINUX_UNALIGNED_ACCESS_OK_H

#include <asm/byteorder.h>

static inline u16 get_unaligned_le16(const void *p)
{
#ifdef CONFIG_M8B
	u16 tmp;
	asm volatile (
		"ldrh	%[tmp], [%[p]]					\n"
		:[tmp] "=r" (tmp)
		:[p] "r" (p)
		:"memory", "cc"
	);
	return tmp;
#else
	return le16_to_cpup((__le16 *)p);
#endif
}

static inline u32 get_unaligned_le32(const void *p)
{
#ifdef CONFIG_M8B 
	u32 tmp;
	asm volatile (
		"ldr	%[tmp], [%[p]]					\n"
		:[tmp] "=r" (tmp)
		:[p] "r" (p)
		:"memory", "cc"
	);
	return tmp;
#else
	return le32_to_cpup((__le32 *)p);
#endif
}

static inline u64 get_unaligned_le64(const void *p)
{
	return le64_to_cpup((__le64 *)p);
}

static inline u16 get_unaligned_be16(const void *p)
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
	return be16_to_cpup((__be16 *)p);
#endif
}

static inline u32 get_unaligned_be32(const void *p)
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
	return be32_to_cpup((__be32 *)p);
#endif
}

static inline u64 get_unaligned_be64(const void *p)
{
	return be64_to_cpup((__be64 *)p);
}

static inline void put_unaligned_le16(u16 val, void *p)
{
#ifdef CONFIG_M8B
	asm volatile (
		"strh	%[tmp], [%[p]], #2				\n"
		:
		:[p] "r" (p), [tmp] "r" (val)
		:"memory", "cc"
	);
#else
	*((__le16 *)p) = cpu_to_le16(val);
#endif
}

static inline void put_unaligned_le32(u32 val, void *p)
{
#ifdef CONFIG_M8B
	asm volatile (
		"str	%[tmp], [%[p]], #2				\n"
		:
		:[p] "r" (p), [tmp] "r" (val)
		:"memory", "cc"
	);
#else
	*((__le32 *)p) = cpu_to_le32(val);
#endif
}

static inline void put_unaligned_le64(u64 val, void *p)
{
	*((__le64 *)p) = cpu_to_le64(val);
}

static inline void put_unaligned_be16(u16 val, void *p)
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
	*((__be16 *)p) = cpu_to_be16(val);
#endif
}

static inline void put_unaligned_be32(u32 val, void *p)
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
	*((__be32 *)p) = cpu_to_be32(val);
#endif
}

static inline void put_unaligned_be64(u64 val, void *p)
{
	*((__be64 *)p) = cpu_to_be64(val);
}

#endif /* _LINUX_UNALIGNED_ACCESS_OK_H */
