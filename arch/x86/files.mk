SRCS_ARCH_C	:=

SRCS_ARCH_S	:=	boot.asm\
				GDT.asm\
				tlb.asm

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_ARCH_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_ARCH_S)))