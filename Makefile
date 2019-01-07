
CC ?= gcc
WINDRES ?= windres
STRIP ?= strip
RM ?= rm -f

# Define V=1 to show command line.
ifdef V
    Q :=
    E := @true
else
    Q := @
    E := @echo
endif

all: 
	$(E) [CC] qcmd.c
	$(Q)$(CC) -o qcmd qcmd.c

strip: 
	$(E) [STRIP]
	$(Q)$(STRIP) qcmd

clean:
	$(E) [CLEAN]
	$(Q)$(RM) qcmd
	
default: all
