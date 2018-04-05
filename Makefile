.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -g
LDFLAGS =

# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions
#LIBS += -lsocket -lnsl -lrt
LIBS += -lpthread

INCLUDE = csapp.h get_ftp.c ls_ftp.c
OBJS = csapp.o get_ftp.o ls_ftp.o
INCLDIR = -I.

PROGS = client serveur_esclave serveur_maitre



all: $(PROGS)

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<

%: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f $(PROGS) *.o
