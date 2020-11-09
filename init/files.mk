SRCS_INIT_C :=	main.c\
				gdt_setup.c\
				multiboot.c\
				system.c\
				mem_setup.c

SRCS_INIT_S :=

HEADERS += 
OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_INIT_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_INIT_S)))
