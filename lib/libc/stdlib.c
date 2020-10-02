#include <stdlib.h>

extern int	atoi(const char *str)
{
	int neg = 1;
	int nbr = 0;
	int i = 0;

	nbr = 0;
	while (str[i] <= ' ') {
		i++;
	}
	if (str[i] == '-' || str[i] == '+') {
		if (str[i] == '-') {
			neg *= -1;
		}
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9') {
		nbr = nbr * 10 + (str[i] - 48);
		i++;
	}
	return (nbr * neg);
}

extern int	chtoi(const char c)
{
	return ((int)c - 48);
}