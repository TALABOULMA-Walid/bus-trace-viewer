# $Id: Makefile,v davewang Exp $

HOSTNAME = $(shell hostname)

CC = gcc

OPT = -O3 

INCLUDEPATH = -I/usr/local/include -I/usr/include -I/usr/openwin/include
LIBDIRS = -L/usr/local/lib \
        -L/usr/X11R6/lib \
        -L/usr/lib
LIBS =  -ltk8.4 -ltcl8.4 -lX11 -lXmu -lm 

CFLAGS = $(OPT) $(INCLUDEPATH)
LDFLAGS = $(OPT)

BTVOBJ = btvInit.o files.o btvUtils.o btvWin.o drawings.o stats.o

STATOBJ = files.o stats.o tracestat.o 

MERGESTATOBJ = files.o stats.o mergestat.o

MAKESSOBJ = makess.o

btvWish: $(BTVOBJ)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(BTVOBJ) $(LIBDIRS) -o $@ $(LIBS)
	./btv

clean: 
	rm *.o
	rm tracestat
	rm btvWish
	rm makess

tracestat: $(STATOBJ)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(STATOBJ) $(LIBDIRS) -o $@ $(LIBS)

mergestat: $(MERGESTATOBJ)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(MERGESTATOBJ) $(LIBDIRS) -o $@ $(LIBS)

makess: $(MAKESSOBJ)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(MAKESSOBJ)  -o $@ 

BTV = btv_src

tar:
	cd .. ; \
	tar -cvf $(BTV).tar src ; \
	gzip $(BTV).tar 

files.o: btv.h
btvInit.o: btv.h
btvUtils.o: btv.h
btvWin.o: btv.h
drawings.o: btv.h
stats.o: btv.h
makess.o: makess.h
