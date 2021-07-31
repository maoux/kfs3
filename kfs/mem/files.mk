SRCS_MEM_C :=	pmm_bootstrap.c\
				vmm.c\
				pmm.c\
				pde.c\
				pte.c\
				vmalloc.c\
				panic.c\
				kmalloc.c\
				cache.c\
				mem_test.c

SRCS_MEM_A :=

OBJS += $(addprefix $(OBJSDIR)/, $(SRCS_MEM_C:.c=.o) $(patsubst %.asm, %.o, $(SRCS_MEM_A)))