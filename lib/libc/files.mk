SRCS_LIBC_C = 	string.c\
				ftlib.c\
				stdlib.c

SRCS_LIBC_A =

HEADERS += string.h

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_LIBC_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_LIBC_A)))