SRCS_MEM_C :=	pmm_bootstrap.c\
				vmm.c\
				pmm.c\
				pde.c\
				pte.c\
				vmalloc.c

SRCS_MEM_A :=

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_MEM_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_MEM_A)))