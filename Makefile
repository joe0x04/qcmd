
CC ?= gcc
WINDRES ?= windres
STRIP ?= strip
RM ?= rm -f

CCX = i686-w64-mingw32-gcc
LIBX = /usr/i686-w64-mingw32/sys-root/mingw/lib

# Define V=1 to show command line.
ifdef V
    Q :=
    E := @true
else
    Q := @
    E := @echo
endif

all: 
	$(E) [CC] qcmd
	$(Q)$(CC) -o qcmd qcmd.c

static:
	$(E) [CC] qcmd
	$(Q)$(CC) -o qcmd qcmd.c -static -static-libgcc
	
windows:
	$(E) [CC] qcmd.exe
	$(Q)$(CCX) -o qcmd.exe qcmd.c -static -static-libgcc $(LIBX)/libws2_32.a

strip: 
	$(E) [STRIP]
	$(Q)$(STRIP) qcmd

clean:
	$(E) [CLEAN]
	$(Q)$(RM) qcmd
	
default: all
