HEADERS	+=	kernel.h\
			io.h

VGADIR		:= $(KERNELDIR)/vga
IODIR		:= $(KERNELDIR)/io
TOOLSDIR	:= $(KERNELDIR)/tools

VPATH	+= $(VGADIR) $(IODIR) $(TOOLSDIR)

include $(VGADIR)/files.mk
include $(IODIR)/files.mk
include $(TOOLSDIR)/files.mk