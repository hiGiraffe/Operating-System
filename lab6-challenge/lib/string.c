#include <types.h>
// void *memmove(void *dest, const void *src, size_t n)
// {
// 	unsigned char *d = (unsigned char *)dest;
// 	const unsigned char *s = (const unsigned char *)src;

// 	if (s < d)
// 	{
// 		for (size_t i = n; i > 0; --i)
// 		{
// 			d[i - 1] = s[i - 1];
// 		}
// 	}
// 	else
// 	{
// 		for (size_t i = 0; i < n; ++i)
// 		{
// 			d[i] = s[i];
// 		}
// 	}

// 	return dest;
// }

void *memcpy(void *dst, const void *src, size_t n)
{
	void *dstaddr = dst;
	void *max = dst + n;

	if (((u_long)src & 3) != ((u_long)dst & 3))
	{
		while (dst < max)
		{
			*(char *)dst++ = *(char *)src++;
		}
		return dstaddr;
	}

	while (((u_long)dst & 3) && dst < max)
	{
		*(char *)dst++ = *(char *)src++;
	}

	// copy machine words while possible
	while (dst + 4 <= max)
	{
		*(uint32_t *)dst = *(uint32_t *)src;
		dst += 4;
		src += 4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max)
	{
		*(char *)dst++ = *(char *)src++;
	}
	return dstaddr;
}

void *memset(void *dst, int c, size_t n)
{
	void *dstaddr = dst;
	void *max = dst + n;
	u_char byte = c & 0xff;
	uint32_t word = byte | byte << 8 | byte << 16 | byte << 24;

	while (((u_long)dst & 3) && dst < max)
	{
		*(u_char *)dst++ = byte;
	}

	// fill machine words while possible
	while (dst + 4 <= max)
	{
		*(uint32_t *)dst = word;
		dst += 4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max)
	{
		*(u_char *)dst++ = byte;
	}
	return dstaddr;
}

size_t strlen(const char *s)
{
	int n;

	for (n = 0; *s; s++)
	{
		n++;
	}

	return n;
}

char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++) != 0)
	{
	}

	return ret;
}

const char *strchr(const char *s, int c)
{
	for (; *s; s++)
	{
		if (*s == c)
		{
			return s;
		}
	}
	return 0;
}

int strcmp(const char *p, const char *q)
{
	while (*p && *p == *q)
	{
		p++, q++;
	}

	if ((u_int)*p < (u_int)*q)
	{
		return -1;
	}

	if ((u_int)*p > (u_int)*q)
	{
		return 1;
	}

	return 0;
}
