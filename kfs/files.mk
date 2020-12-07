HEADERS	+=	kernel.h\
			io.h\
			mem.h

VGADIR		:= $(KERNELDIR)/vga
IODIR		:= $(KERNELDIR)/io
TOOLSDIR	:= $(KERNELDIR)/tools
MEMDIR		:= $(KERNELDIR)/mem

VPATH	+= $(VGADIR) $(IODIR) $(TOOLSDIR) $(MEMDIR)

include $(VGADIR)/files.mk
include $(IODIR)/files.mk
include $(TOOLSDIR)/files.mk
include $(MEMDIR)/files.mk