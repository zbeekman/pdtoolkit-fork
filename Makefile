PDTDIR=

PLATFORM=
COMPDIR=

BIN=$(PDTDIR)/$(PLATFORM)/$(COMPDIR)/bin
LIB=$(PDTDIR)/$(PLATFORM)/$(COMPDIR)/lib

PDT_RANLIB=echo "*****DONE*****"
#DARWIN#PDT_RANLIB=ranlib $(LIB)/libpdb.a#EOC#

all : clean
	(cd ductape; $(MAKE) "MAKE=$(MAKE)" all)

install: all 
	/bin/rm -f $(LIB)/libpdb.a
	(cd ductape; $(MAKE) "MAKE=$(MAKE)" install)
	cp ./ductape/lib/libpdb.a $(LIB)
	$(PDT_RANLIB) 

clean :
	(cd ductape; $(MAKE) "MAKE=$(MAKE)" clean)
