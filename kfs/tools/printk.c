#include <kernel.h>
#include <stdint.h>
#include <string.h>
#include <ftlib.h>
#include <vga.h>
#include <stdlib.h>

struct options {
	uint8_t		b_signed;
	uint8_t		b_zpadded;
	uint8_t		b_formatted;
	uint8_t		b_rightpadded;
	size_t		precision;
	size_t		width;
	uint8_t		size;
};



/* statics */

static void		init_options(struct options *opts);

static size_t	printk_int(int arg, struct options opts);

static size_t	printk_uint(unsigned int arg, struct options opts);

static size_t	printk_oct(unsigned int arg, struct options opts);

static size_t	printk_hex(unsigned int arg, struct options opts, uint8_t uppercase);

static size_t	printk_bin(unsigned int arg, struct options opts);

static size_t	printk_char(char c);
static size_t	printk_string(char *str);

static void		pad(size_t len, char c);
static uint8_t	print_log_prefix(char c);


static uint8_t	print_log_prefix(char c)
{
	if (c == *KERN_NONE) {
		return (1);
	}
	if (c == *KERN_DEBUG) {
		putstring("LOG DEBUG: ");
		return (1);
	}
	else if (c == *KERN_INFO) {\
		putstring("LOG INFO: ");
		return (1);
	}
	else if (c == *KERN_NOTICE) {
		textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREEN);
		putstring("NOTICE: ");
	}
	else if (c == *KERN_WARNING) {
		textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_BROWN);
		putstring("WARNING: ");
	}
	else if (c == *KERN_ERR) {
		textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_MAGENTA);
		putstring("ERROR: ");
	}
	else if (c == *KERN_CRIT) {
		textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_MAGENTA);
		putstring("CRITICAL: ");
	}
	else if (c == *KERN_ALERT) {
		textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED);
		putstring("ALERT: ");
	}
	else if (c == *KERN_EMERG) {
		textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_RED);
		putstring("EMERGENCY: ");
	} else {
		return (0);
	}
	textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
	return (1);
}


static void		init_options(struct options *opts)
{
	opts->b_formatted = 0;
	opts->b_rightpadded = 0;
	opts->b_signed = 0;
	opts->b_zpadded = 0;
	opts->precision = 0;
	opts->width = 0;
	opts->size = TYPE_DEFAULT;
}

static void		pad(size_t len, char c)
{
	for (;len > 0;len--) {
		putchar(c);
	}
}

static void		left_pad(size_t len, struct options opts)
{
	if (opts.width && !opts.b_rightpadded) {
		if (opts.precision && opts.width > opts.precision && opts.precision > len) {
			opts.b_zpadded ? pad(opts.width - opts.precision, '0') :  pad(opts.width - opts.precision, ' ');
		}
		else {
			opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
		}
	}
	if (opts.precision && opts.precision > len  && (opts.width > opts.precision || !opts.width)) {
		pad(opts.precision - len, '0');
	}
}

static size_t	printk_int(int arg, struct options opts)
{
	size_t		len = nbrlen(arg);

	if (opts.width && !opts.b_rightpadded) {
		if (opts.b_zpadded && opts.b_signed) {
			if (arg > 0) {
				putchar('+');
			}
		}
		if (opts.precision && opts.width > opts.precision && opts.precision > len) {
			opts.b_zpadded ? pad(opts.width - opts.precision, '0') :  pad(opts.width - opts.precision, ' ');
		}
		else {
			opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
		}
	}
	if (opts.b_signed && (!opts.b_zpadded || !opts.width)) {
		if (arg > 0) {
			putchar('+');
		}
	}
	if (opts.precision && opts.precision > len  && (opts.width > opts.precision || !opts.width)) {
		pad(opts.precision - len, '0');
	}
	putnbr(arg);
	if (opts.width && opts.b_rightpadded) {	
		opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
	}
	if (opts.width > len && opts.width > opts.precision) {
		return (opts.width);
	}
	if (opts.precision > len) {
		return (opts.precision);
	}
	return (len);
}

static size_t	printk_uint(unsigned int arg, struct options opts)
{
	size_t		len = unbrlen(arg);

	left_pad(len, opts);
	putunbr(arg);
	if (opts.width && opts.b_rightpadded) {	
		opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
	}
	if (opts.width > len && opts.width > opts.precision) {
		return (opts.width);
	}
	if (opts.precision > len) {
		return (opts.precision);
	}
	return (len);
}


static size_t	printk_oct(unsigned int arg, struct options opts)
{
	size_t len = unbrlen_base(arg, 8);

	if (opts.b_formatted) {
		putchar('0');
		len++;
	}
	left_pad(len, opts);
	putunbr_base(arg, 8, 0);
	if (opts.width && opts.b_rightpadded) {	
		opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
	}
	if (opts.width > len && opts.width > opts.precision) {
		return (opts.width);
	}
	if (opts.precision > len) {
		return (opts.precision);
	}
	return (len);
}

static size_t	printk_hex(unsigned int arg, struct options opts, uint8_t uppercase)
{
	size_t		len = unbrlen_base(arg, 16);

	if (opts.b_formatted) {
		uppercase ? putstring("0X") : putstring("0x");
		len += 2;
	}
	left_pad(len, opts);
	putunbr_base(arg, 16, uppercase);
	if (opts.width && opts.b_rightpadded) {
		opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
	}
	if (opts.width > len && opts.width > opts.precision) {
		return (opts.width);
	}
	if (opts.precision > len) {
		return (opts.precision);
	}
	return (len);
}

static size_t	printk_bin(unsigned int arg, struct options opts)
{
	size_t		len = unbrlen_base(arg, 2);

	if (opts.b_formatted) {
		putstring("0b");
		len += 2;
	}
	left_pad(len, opts);
	putunbr_base(arg, 2, 0);
	if (opts.width && opts.b_rightpadded) {
		opts.b_zpadded ? pad(opts.width - len, '0') :  pad(opts.width - len, ' ');
	}
	if (opts.width > len && opts.width > opts.precision) {
		return (opts.width);
	}
	if (opts.precision > len) {
		return (opts.precision);
	}
	return (len);
}

static size_t	printk_char(char c)
{
	putchar(c);
	return (1);
}

static size_t	printk_string(char *str)
{
	putstring((const char *)str);
	return (strlen(str));
}

extern int	printk(const char *fmt, ...)
{
	char	**args = (char **) &fmt;
	struct options opts = {0, 0, 0, 0, 0, 0, 0};
	int ret_value;

	ret_value = 0;
	args++;
	if (!fmt) {
		return (0);
	}
	fmt += print_log_prefix(*fmt);
	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			do {
				if (*fmt == 'l') {
					opts.size = TYPE_LONG;
					fmt++;
					if (*fmt == 'l') {
						opts.size = TYPE_LONG_LONG;
						fmt++;
					}
				}
				if (*fmt == 'h') {
					opts.size = TYPE_SHORT;
					fmt++;
					if (*fmt == 'h') {
						opts.size = TYPE_CHAR;
						fmt++;
					}
				}
				if (*fmt == '0') {
					opts.b_zpadded = 1;
					fmt++;
				}
				if (*fmt == '#') {
					opts.b_formatted = 1;
					fmt++;
				}
				if (*fmt== '-') {
					opts.b_rightpadded = 1;
					fmt++;
				}
				if (*fmt == '.') {
					fmt++;
					while (*fmt >= '0' && *fmt <= '9') {
						opts.precision = opts.precision * 10 + chtoi(*fmt);
						fmt++;
					}
				}
				if (*fmt > '0' && *fmt <= '9') {
					while (*fmt >= '0' && *fmt <= '9') {
						opts.width = opts.width * 10 + chtoi(*fmt);
						fmt++;
					}
				}
				if (*fmt == '+') {
					opts.b_signed = 1;
					fmt++;
				}
				if (*fmt == 'd' || *fmt == 'i') {
					if (opts.size == TYPE_CHAR) {
						ret_value += printk_int(*((char *) args++), opts);
					}
					else if (opts.size == TYPE_SHORT) {
						ret_value += printk_int(*((short int *) args++), opts);
					}
					else if (opts.size == TYPE_DEFAULT || opts.size == TYPE_LONG) {
						ret_value += printk_int(*((int *) args++), opts);
					}
					else if (opts.size == TYPE_LONG_LONG) {
						ret_value += printk_int(*((long long int *) args++), opts);
					}
				}
				else if (*fmt == 'u') {
					if (opts.size == TYPE_CHAR) {
						ret_value += printk_uint(*((unsigned char *) args++), opts);
					}
					else if (opts.size == TYPE_SHORT) {
						ret_value += printk_uint(*((unsigned short int *) args++), opts);
					}
					else if (opts.size == TYPE_DEFAULT || opts.size == TYPE_LONG) {
						ret_value += printk_uint(*((unsigned int *) args++), opts);
					}
					else if (opts.size == TYPE_LONG_LONG) {
						ret_value += printk_uint(*((unsigned long long int *) args++), opts);
					}
				}
				else if (*fmt == '%') {
					ret_value++;
					putchar('%');
				}
				else if (*fmt == 'o') {
					if (opts.size == TYPE_CHAR) {
						ret_value += printk_oct(*((unsigned char *) args++), opts);
					}
					else if (opts.size == TYPE_SHORT) {
						ret_value += printk_oct(*((unsigned short int *) args++), opts);
					}
					else if (opts.size == TYPE_DEFAULT || opts.size == TYPE_LONG) {
						ret_value += printk_oct(*((unsigned int *) args++), opts);
					}
					else if (opts.size == TYPE_LONG_LONG) {
						ret_value += printk_oct(*((unsigned long long int *) args++), opts);
					}
				}
				else if (*fmt == 'x' || *fmt == 'X') {
					if (opts.size == TYPE_CHAR) {
						ret_value += printk_hex(*((unsigned char *) args++), opts, *fmt == 'X');
					}
					else if (opts.size == TYPE_SHORT) {
						ret_value += printk_hex(*((unsigned short int *) args++), opts, *fmt == 'X');
					}
					else if (opts.size == TYPE_DEFAULT || opts.size == TYPE_LONG) {
						ret_value += printk_hex(*((unsigned int *) args++), opts, *fmt == 'X');
					}
					else if (opts.size == TYPE_LONG_LONG) {
						ret_value += printk_hex(*((unsigned long long int *) args++), opts, *fmt == 'X');
					}
				}
				else if (*fmt == 'b') {
					if (opts.size == TYPE_CHAR) {
						ret_value += printk_bin(*((unsigned char *) args++), opts);
					}
					else if (opts.size == TYPE_SHORT) {
						ret_value += printk_bin(*((unsigned short int *) args++), opts);
					}
					else if (opts.size == TYPE_DEFAULT || opts.size == TYPE_LONG) {
						ret_value += printk_bin(*((unsigned int *) args++), opts);
					}
					else if (opts.size == TYPE_LONG_LONG) {
						ret_value += printk_bin(*((unsigned long long int *) args++), opts);
					}
				}
				else if (*fmt == 's') {
					if (*args == NULL) {
						args++;
						ret_value += printk_string("(null)");
					}
					else {
						ret_value += printk_string(*args++);
					}
				}
				else if (*fmt == 'c') {
					ret_value += printk_char((char)(*((int *) args++)));
				}
			} while (strchr("diuoxXbcs%", *fmt) == NULL);
			init_options(&opts);
		}
		else {
			ret_value++;
			putchar(*fmt);
		}
		*fmt != '\0' ? fmt++ : 0;
	}
	return (ret_value);
}