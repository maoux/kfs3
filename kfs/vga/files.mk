SRCS_VGA_C :=	screen.c\
				putnbr.c

SRCS_VGA_A :=

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_VGA_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_VGA_A)))