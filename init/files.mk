SRCS_INIT_C :=	main.c\
				gdt_setup.c\
				mem_setup.c\
				multiboot.c\
				system.c

SRCS_INIT_S :=

HEADERS += 
OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_INIT_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_INIT_S)))
