#ifndef __STDINT_H__
# define __STDINT_H__

typedef char					int8_t;
typedef short					int16_t;
typedef int						int32_t;
typedef unsigned char			uint8_t;
typedef unsigned short			uint16_t;
typedef unsigned int			uint32_t;

# define INT8_MIN		-127
# define INT8_MAX		128
# define UINT8_MAX		255		

# define INT16_MIN		-32767
# define INT16_MAX		32768
# define UINT16_MAX		65536

# define INT32_MIN		-2147483647
# define INT32_MAX		2147483648
# define UINT32_MAX		4294967296

# define INT_MIN		-2147483647
# define INT_MAX		2147483648

#endif