#ifndef __STRING_H__
# define __STRING_H__

# include <stddef.h>

extern void		*memchr(const void *str, int c, size_t n);
extern int		memcmp(const void *str1, const void *str2, size_t n);
extern void		*memcpy(void *dest, const void *src, size_t n);
extern void		*memmove(void *dest, const void *src, size_t n);
extern void		*memset(void *str, int c, size_t n);
extern void		*memsetw(void *str, int c, size_t n);
extern char		*strcat(char *dest, const char *src);
extern char		*strncat(char *dest, const char *src, size_t n);
extern char		*strchr(const char *str, int c);
extern int		strcmp(const char *str1, const char *str2);
extern int		strncmp(const char *str1, const char *str2, size_t n);
//extern int		strcoll(const char *str1, const char *str2);
extern char		*strcpy(char *dest, const char *src);
extern char		*strncpy(char *dest, const char *src, size_t n);
extern size_t	strcspn(const char *str1, const char *str2);
//extern char		*strerror(int errnum);
extern size_t	strlen(const char *str);
extern char		*strpbrk(const char *str1, const char *str2);
extern char		*strrchr(const char *str, int c);
extern size_t	strspn(const char *str1, const char *str2);
extern char		*strstr(const char *haystack, const char *needle);
//extern char		*strtok(char *str, const char *delim);
//extern size_t	strxfrm(char *dest, const char *src, size_t n);

#endif