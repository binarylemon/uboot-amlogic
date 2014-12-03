/*
 *  LZO1X Decompressor from MiniLZO
 *
 *  Copyright (C) 1996-2005 Markus F.X.J. Oberhumer <markus@oberhumer.com>
 *
 *  The full LZO package can be found at:
 *  http://www.oberhumer.com/opensource/lzo/
 *
 *  Changed for kernel use by:
 *  Nitin Gupta <nitingupta910@gmail.com>
 *  Richard Purdie <rpurdie@openedhand.com>
 */

#include <common.h>
#include <linux/lzo.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include "lzodefs.h"

#define HAVE_IP(x)      ((size_t)(ip_end - ip) >= (size_t)(x))
#define HAVE_OP(x)      ((size_t)(op_end - op) >= (size_t)(x))
#define NEED_IP(x)      if (!HAVE_IP(x)) goto input_overrun
#define NEED_OP(x)      if (!HAVE_OP(x)) goto output_overrun
#define TEST_LB(m_pos)  if ((m_pos) < out) goto lookbehind_overrun

#define CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS 1
#define COPY4(dst, src)	\
		* (u32 *) (void *) (dst) = * (const u32 *) (const void *) (src)

#define COPY8(dst, src)	\
		COPY4(dst, src); COPY4((dst) + 4, (src) + 4)

static const unsigned char lzop_magic[] = {
	0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a
};

#define HEADER_HAS_FILTER	0x00000800L

static inline const unsigned char *parse_header(const unsigned char *src)
{
	u8 level = 0;
	u16 version;
	int i;

	/* read magic: 9 first bytes */
	for (i = 0; i < ARRAY_SIZE(lzop_magic); i++) {
		if (*src++ != lzop_magic[i])
			return NULL;
	}
	/* get version (2bytes), skip library version (2),
	 * 'need to be extracted' version (2) and
	 * method (1) */
	version = get_unaligned_be16(src);
	src += 7;
	if (version >= 0x0940)
		level = *src++;
	if (get_unaligned_be32(src) & HEADER_HAS_FILTER)
		src += 4; /* filter info */

	/* skip flags, mode and mtime_low */
	src += 12;
	if (version >= 0x0940)
		src += 4;	/* skip mtime_high */

	i = *src++;
	/* don't care about the file name, and skip checksum */
	src += i + 4;

	return src;
}

int lzo1x_decompress_safe(const unsigned char *in, size_t in_len,
			  unsigned char *out, size_t *out_len)
{
	unsigned char *op;
	const unsigned char *ip;
	size_t t, next;
	size_t state = 0;
	const unsigned char *m_pos;
	const unsigned char * const ip_end = in + in_len;
	unsigned char * const op_end = out + *out_len;
	op = out;
	ip = in;
	if (unlikely(in_len < 3))
		goto input_overrun;
	if (*ip > 17) {
		t = *ip++ - 17;
		if (t < 4) {
			next = t;
			goto match_next;
		}
		goto copy_literal_run;
	}
	for (;;) {
		t = *ip++;
		if (t < 16) {
			if (likely(state == 0)) {
				if (unlikely(t == 0)) {
					while (unlikely(*ip == 0)) {
						t += 255;
						ip++;
						NEED_IP(1);
					}
					t += 15 + *ip++;
				}
				t += 3;
copy_literal_run:
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
				if (likely(HAVE_IP(t + 15) && HAVE_OP(t + 15))) {
					const unsigned char *ie = ip + t;
					unsigned char *oe = op + t;
					do {
						COPY8(op, ip);
						op += 8;
						ip += 8;
#  if !defined(__arm__)
						COPY8(op, ip);
						op += 8;
						ip += 8;
#  endif
					} while (ip < ie);
					ip = ie;
					op = oe;
				} else
#endif
				{
					NEED_OP(t);
					NEED_IP(t + 3);
					do {
						*op++ = *ip++;
					} while (--t > 0);
				}
				state = 4;
				continue;
			} else if (state != 4) {
				next = t & 3;
				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;
				TEST_LB(m_pos);
				NEED_OP(2);
				op[0] = m_pos[0];
				op[1] = m_pos[1];
				op += 2;
				goto match_next;
			} else {
				next = t & 3;
				m_pos = op - (1 + M2_MAX_OFFSET);
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;
				t = 3;
			}
		} else if (t >= 64) {
			next = t & 3;
			m_pos = op - 1;
			m_pos -= (t >> 2) & 7;
			m_pos -= *ip++ << 3;
			t = (t >> 5) - 1 + (3 - 1);
		} else if (t >= 32) {
			t = (t & 31) + (3 - 1);
			if (unlikely(t == 2)) {
				while (unlikely(*ip == 0)) {
					t += 255;
					ip++;
					NEED_IP(1);
				}
				t += 31 + *ip++;
				NEED_IP(2);
			}
			m_pos = op - 1;
			next = get_unaligned_le16(ip);
			ip += 2;
			m_pos -= next >> 2;
			next &= 3;
		} else {
			m_pos = op;
			m_pos -= (t & 8) << 11;
			t = (t & 7) + (3 - 1);
			if (unlikely(t == 2)) {
				while (unlikely(*ip == 0)) {
					t += 255;
					ip++;
					NEED_IP(1);
				}
				t += 7 + *ip++;
				NEED_IP(2);
			}
			next = get_unaligned_le16(ip);
			ip += 2;
			m_pos -= next >> 2;
			next &= 3;
			if (m_pos == op)
				goto eof_found;
			m_pos -= 0x4000;
		}
		TEST_LB(m_pos);
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
		if (op - m_pos >= 8) {
			unsigned char *oe = op + t;
			if (likely(HAVE_OP(t + 15))) {
				do {
					COPY8(op, m_pos);
					op += 8;
					m_pos += 8;
#  if !defined(__arm__)
					COPY8(op, m_pos);
					op += 8;
					m_pos += 8;
#  endif
				} while (op < oe);
				op = oe;
				if (HAVE_IP(6)) {
					state = next;
					COPY4(op, ip);
					op += next;
					ip += next;
					continue;
				}
			} else {
				NEED_OP(t);
				do {
					*op++ = *m_pos++;
				} while (op < oe);
			}
		} else
#endif
		{
			unsigned char *oe = op + t;
			NEED_OP(t);
			op[0] = m_pos[0];
			op[1] = m_pos[1];
			op += 2;
			m_pos += 2;
			do {
				*op++ = *m_pos++;
			} while (op < oe);
		}
match_next:
		state = next;
		t = next;
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
		if (likely(HAVE_IP(6) && HAVE_OP(4))) {
			COPY4(op, ip);
			op += t;
			ip += t;
		} else
#endif
		{
			NEED_IP(t + 3);
			NEED_OP(t);
			while (t > 0) {
				*op++ = *ip++;
				t--;
			}
		}
	}
eof_found:
	*out_len = op - out;
	return (t != 3       ? LZO_E_ERROR :
		ip == ip_end ? LZO_E_OK :
		ip <  ip_end ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN);
input_overrun:
	*out_len = op - out;
	return LZO_E_INPUT_OVERRUN;
output_overrun:
	*out_len = op - out;
	return LZO_E_OUTPUT_OVERRUN;
lookbehind_overrun:
	*out_len = op - out;
	return LZO_E_LOOKBEHIND_OVERRUN;
}

int lzop_decompress(const unsigned char *src, size_t src_len,
		    unsigned char *dst, size_t *dst_len)
{
	unsigned char *start = dst;
	const unsigned char *send = src + src_len;
	u32 slen, dlen;
	size_t tmp;
	int r;

	src = parse_header(src);
	if (!src)
		return LZO_E_ERROR;

	while (src < send) {
		/* read uncompressed block size */
		dlen = get_unaligned_be32(src);
		src += 4;

		/* exit if last block */
		if (dlen == 0) {
			*dst_len = dst - start;
			return LZO_E_OK;
		}

		/* read compressed block size, and skip block checksum info */
		slen = get_unaligned_be32(src);
		src += 8;

		if (slen <= 0 || slen > dlen)
			return LZO_E_ERROR;

		/* decompress */
		tmp = dlen;
		r = lzo1x_decompress_safe((u8 *) src, slen, dst, &tmp);

		if (r != LZO_E_OK)
			return r;

		if (dlen != tmp)
			return LZO_E_ERROR;

		src += slen;
		dst += dlen;
	}

	return LZO_E_INPUT_OVERRUN;
}
