IDIR =/home/pi/
CC=g++
CFLAGS=-I$(IDIR) -Wall -g -fPIC

INSTALLEXEDIR=/usr/local/bin
INSTALLLIBDIR=/usr/local/lib
INSTALLINCDIR=/usr/local/include

EXENAME=bbsbsend

LDIR =
LIBS=-lcxxtools -lcxxtools-json -lcxxtools-http -lwiringPi -lrt -lm  -pthread 
LIBHEADER=eventNotifier.h
DEPS=$(LIBHEADER)

EXEOBJS=bbsb.o bbsbSendMain.o

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 

%.o: %.cpp $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 

$(EXENAME): $(EXEOBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) 

.PHONY: clean clobber all install

clean:
	rm -f $(EXEOBJS) 

clobber:
	rm -f $(EXEOBJS) $(EXENAME)

# sudo make install
install:
	sudo cp -f $(EXENAME) $(INSTALLEXEDIR)
	
all: $(EXENAME) $(LIBNAME)
