INC = ../inc

CCFLAGS = -I$(INC)

LINK = $(CXX) #--link_command_prefix purify

LIB = ../lib/libpdb.a

PLATFORM=
COMPDIR=
PDTDIR=

BIN=$(PDTDIR)/$(PLATFORM)/$(COMPDIR)/bin

PDBHTMLEXE=pdbhtml
#CRAYX1#PDBHTMLEXE=#EOC#
#CRAY#PDBHTMLEXE=#EOC#

PDBXMLGEN=xmlgen
#APPLEPGI#PDBXMLGEN=#EOC#

PROG = pdbconv pdbtree pdbmerge pdbcomment pdbstmt $(PDBXMLGEN) $(PDBHTMLEXE)

all: $(PROG) tau_instrumentor_prog

tau_instrumentor_prog:
	(cd tau_instrumentor; $(MAKE) "MAKE=$(MAKE)" CXX="$(CXX)")

pdbconv: pdbconv.o $(LIB)
	$(LINK) $(CCFLAGS) pdbconv.o -o pdbconv $(LIB)

pdbconv.o: pdbconv.cc $(INC)/pdbAll.h
	$(CXX) $(CCFLAGS) -c pdbconv.cc

pdbtree: pdbtree.o $(LIB)
	$(LINK) $(CCFLAGS) pdbtree.o -o pdbtree $(LIB)

pdbtree.o: pdbtree.cc $(INC)/pdb.h $(INC)/pdbRoutine.h $(INC)/pdbClass.h \
	$(INC)/pdbGroupField.h $(INC)/pdbType.h
	$(CXX) $(CCFLAGS) -c pdbtree.cc

pdbmerge: pdbmerge.o $(LIB)
	$(LINK) $(CCFLAGS) pdbmerge.o -o pdbmerge $(LIB)

pdbmerge.o: pdbmerge.cc $(INC)/pdb.h
	$(CXX) $(CCFLAGS) -c pdbmerge.cc

xmlgen: xmlgen.o $(LIB)
	$(LINK) $(CCFLAGS) xmlgen.o -o xmlgen $(LIB)

xmlgen.o: xmlgen.cc $(INC)/pdbAll.h
	$(CXX) $(CCFLAGS) -c xmlgen.cc

pdbcomment: pdbcomment.o $(LIB)
	$(LINK) $(CCFLAGS) pdbcomment.o -o pdbcomment $(LIB)

pdbcomment.o: pdbcomment.cc $(INC)/pdbAll.h
	$(CXX) $(CCFLAGS) -c pdbcomment.cc

pdbstmt: pdbstmt.o $(LIB)
	$(LINK) $(CCFLAGS) pdbstmt.o -o pdbstmt $(LIB)

pdbstmt.o: pdbstmt.cc $(INC)/pdb.h $(INC)/pdbCRoutine.h $(INC)/pdbFRoutine.h \
	$(INC)/pdbType.h
	$(CXX) $(CCFLAGS) -c pdbstmt.cc

pdbhtml: pdbhtml.o $(LIB)
	$(LINK) $(CCFLAGS) pdbhtml.o -o pdbhtml $(LIB)

pdbhtml.o: pdbhtml.cc $(INC)/pdbAll.h
	$(CXX) $(CCFLAGS) -c pdbhtml.cc

install: $(PROG)
	cp $(PROG) $(BIN)
	(cd tau_instrumentor; $(MAKE) "MAKE=$(MAKE)" install CXX="$(CXX)")

clean:
	cd tau_instrumentor ; $(MAKE) "MAKE=$(MAKE)" clean
	rm -rf core *.o $(PROG) libpdb.a *~
	rm -rf a.out *.ti *.ii ii_files ti_files
