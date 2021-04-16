SRCS_MEM_C :=	pages.c

SRCS_MEM_A :=

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_MEM_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_MEM_A)))