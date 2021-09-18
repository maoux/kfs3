#include <string.h>

/**		static functions		**/

static char		c_is_in_str(const char *s, char c);

/*********************************/

extern void		*memchr(const void *str, int c, size_t n)
{
	unsigned char	*cstr = (unsigned char *)str;
	unsigned char	cc = (unsigned char)c;

	for (size_t i =0; i < n; i++) {
		if (*(cstr + i) == cc) {
			return ((void *)(cstr + i));
		}
	}
	return (NULL);
}

extern int		memcmp(const void *str1, const void *str2, size_t n)
{
	unsigned char	*cstr1 = (unsigned char *)str1;
	unsigned char	*cstr2 = (unsigned char *)str2;

	for (size_t i = 0; i < n; i++) {
		if (*(cstr1 + i) != *(cstr2 + i)) {
			return (*(cstr1 + i) - *(cstr2 + i));
		}
	}
	return (0);
}

extern void		*memcpy(void *dest, const void *src, size_t n)
{
	unsigned char	*cdest = (unsigned char *)dest;
	unsigned char	*csrc = (unsigned char *)src;

	for (size_t i = 0; i < n; i++) {
		*(cdest + i) = *(csrc + i);
	}
	return dest;
}

extern void		*memmove(void *dest, const void *src, size_t n)
{
	unsigned char	buffer[n];
	unsigned char	*cdest = (unsigned char *)dest;
	unsigned char	*csrc = (unsigned char *)src;

	for (size_t i = 0; i < n; i++) {
		buffer[i] = *(csrc + i);
	}
	for (size_t i = 0; i < n; i++) {
		*(cdest + i) = buffer[i];
	}
	return dest;
}

extern void		*memset(void *str, int c, size_t n)
{
	unsigned char	*cstr = (unsigned char *)str;
	unsigned char	cc = (unsigned char)c;

	for (size_t i = 0; i < n; i++) {
		*(cstr + i) = cc;
	}
	return (str);
}

extern void		*memsetw(void *str, int c, size_t n)
{
	size_t	*cstr = (size_t *)str;
	size_t	cc = (size_t)c;

	for (size_t i = 0; i < n; i++) {
		*(cstr + i) = cc;
	}
	return (str);
}


extern char		*strcat(char *dest, const char *src)
{
	size_t		destlen = strlen(dest);

	for (size_t i = 0; *(src + i); i++) {
		*(dest + destlen + i) = *(src + i);
		if (*(src + i + 1) == '\0') {
			*(dest + destlen + i + 1) = '\0';
		}
	}
	return (dest);
}

extern char		*strncat(char *dest, const char *src, size_t n)
{
	size_t		destlen = strlen(dest);

	for (size_t i = 0; i < n && *(src + i); i++) {
		*(dest + destlen + i) = *(src + i);
		if (*(src + i + 1) == '\0') {
			*(dest + destlen + i + 1) = '\0';
		}
	}
	return (dest);
}

extern char		*strchr(const char *str, int c)
{
	unsigned char	cc = (unsigned char)c;

	for (size_t i = 0; *(str + i); i++) {
		if (*(str + i) == cc) {
			return (((char *)str) + i);
		}
	}
	return (NULL);
}

extern int		strcmp(const char *str1, const char *str2)
{
	size_t		i;

	for (i = 0; str1[i] && str2[i]; i++) {
		if (*(str1 + i) != *(str2 + i)) {
			break ;
		}
	}
	return (*(str1 + i) - *(str2 + i));
}

extern int		strncmp(const char *str1, const char *str2, size_t n)
{
	size_t		i;

	for (i = 0; str1[i] && str2[i] && i < n; i++) {
		if (*(str1 + i) != *(str2 + i)) {
			break ;
		}
	}
	return (*(str1 + i) - *(str2 + i));
}


extern char		*strcpy(char *dest, const char *src)
{
	size_t		i = 0;

	while (*(src + i)) {
		*(dest + i) = *(src + i);
		i++;
	}
	*(dest + i) = '\0';
	return (dest);
}

extern char		*strncpy(char *dest, const char *src, size_t n)
{
	size_t		i = 0;

	while (*(src + i) && i < n) {
		*(dest + i) = *(src + i);
		i++;
	}
	*(dest + i) = '\0';
	return (dest);
}

static char		c_is_in_str(const char *s, char c)
{
	for (size_t i = 0; s[i]; i++) {
		if (s[i] == c) {
			return (1);
		}
	}
	return (0);
}

extern size_t	strcspn(const char *str1, const char *str2)
{
	size_t		i = 0;
	size_t		len = 0;

	while (str1[i] && c_is_in_str(str2, str1[i])) {
		i++;
	}
	while (str1[i] && !c_is_in_str(str2, str1[i])) {
		i++;
		len++;
	}
	return (len);
}

extern size_t	strlen(const char *s)
{
	for (size_t i = 0;; i++) {
		if (s[i] == '\0') {
			return (i);
		}
	}
}

extern char		*strpbrk(const char *str1, const char *str2)
{
	for (size_t i = 0; str1[i]; i++) {
		if (c_is_in_str(str2, str1[i])) {
			return (((char *)str1) + i);
		}
	}
	return (NULL);
}

extern char		*strrchr(const char *str, int c)
{
	unsigned char	cc = (unsigned char)c;

	for (size_t i = strlen(str) - 1; i > 0; i--) {
		if (str[i] == cc) {
			return (((char *)str) + i);
		}
	}
	if (str[0] == cc) {
		return ((char *)str);
	}
	return (NULL);
}

extern size_t	strspn(const char *str1, const char *str2)
{
	size_t		i = 0;
	size_t		len = 0;

	while (str1[i] && !c_is_in_str(str2, str1[i])) {
		i++;
	}
	while (str1[i] && c_is_in_str(str2, str1[i])) {
		i++;
		len++;
	}
	return (len);
}

extern char		*strstr(const char *haystack, const char *needle)
{
	for (size_t i = 0; haystack[i]; i++) {
		for (size_t j = 0; needle[j]; j++) {
			if (needle[j + 1] == '\0' && haystack[i + j] != needle[j]) {
				return (((char *)haystack) + i);
			}
			if (haystack[i + j] != needle[j]) {
				break ;
			}
		}
	}
	return (NULL);
}