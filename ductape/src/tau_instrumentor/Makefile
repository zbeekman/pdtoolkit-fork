
INC=../../inc
CCFLAGS=-I$(INC)
LINK=$(CXX)
LIB=../../lib/libpdb.a

PLATFORM=
COMPDIR=
PDTDIR=

BIN=$(PDTDIR)/$(PLATFORM)/$(COMPDIR)/bin

PROG=
INSTALL=

OBJS=tau_instrument.o tau_instrumentor.o tau_selective.o

all: $(PROG)

tau_instrument.o: tau_instrument.cpp
	$(CXX) $(CCFLAGS) -c tau_instrument.cpp

tau_selective.o: tau_selective.cpp
	$(CXX) $(CCFLAGS) -c tau_selective.cpp

tau_instrumentor.o: tau_instrumentor.cpp
	$(CXX) $(CCFLAGS) -c tau_instrumentor.cpp

tau_instrumentor: $(OBJS)
	$(LINK) $(CCFLAGS) $(OBJS) -o tau_instrumentor $(LIB)

install: $(PROG)
	$(INSTALL)

remove:
	rm -rf *.cpp *.h
clean:
	rm -rf core *.o tau_instrumentor libpdb.a *~
	rm -rf a.out *.ti *.ii ii_files ti_files
