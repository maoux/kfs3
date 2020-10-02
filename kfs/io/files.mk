SRCS_IO_C :=	io.c\
				keyboard.c\
				ps2_controller.c

SRCS_IO_A :=

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_IO_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_IO_A)))