HEADERS	+=	kernel.h\
			io.h

VGADIR		:= $(KERNELDIR)/vga
IODIR		:= $(KERNELDIR)/io
TOOLSDIR	:= $(KERNELDIR)/tools
MEMDIR		:= $(KERNELDIR)/mem

VPATH	+= $(VGADIR) $(IODIR) $(TOOLSDIR)

include $(VGADIR)/files.mk
include $(IODIR)/files.mk
include $(TOOLSDIR)/files.mk
include $(MEMDIR)/files.mk