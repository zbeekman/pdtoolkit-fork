USEROPT=-O2

EXTRAOPT=-D_XOPEN_SOURCE
PRELINK=echo Building libpdb.a
PDT_RANLIB=echo Built libpdb.a
#SOL2#EXTRAOPT=-D__EXTENSIONS__#EOC#
#CXX#EXTRAOPT=-D__USE_STD_IOSTREAM#EOC#
#CRAY#PDT_RANLIB= #EOC#
#PDT_APPLE_BUGFIX#EXTRAOPT= #EOC#

# -- SGI MipsPro
#CC#CXX = CC $(USEROPT) -LANG:std -ptused -OPT:Olimit=0#EOC#
#CC#CXXAR = CC $(USEROPT) -LANG:std -OPT:Olimit=0 -ar -o#EOC#


#SOLARIS2-64#SUN_CC_OPTIONS=-xarch=v9 #EOC#
#SOLARIS2-64#SUN_GNU_OPTIONS=-mcpu=v9 -m64 #EOC#
#SUNX86_64#SUN_CC_OPTIONS=-xarch=amd64 #EOC#
#SUNX86_64#SUN_GNU_OPTIONS=-m64 #EOC#

# -- CRAY CC
#CRAYCC#CXX = CC -g -h conform,instantiate=used #EOC#
#CRAYCC#CXXAR = ar rc#EOC#

# -- Pathscale C++
#PATHCC#CXX = pathCC $(USEROPT) -w $(EXTRAOPT) #EOC#
#PATHCC#CXXAR = ar cr#EOC#

# -- Open64 ORCC C++
#ORCC#CXX = orCC $(USEROPT) -w $(EXTRAOPT) #EOC#
#ORCC#CXXAR = ar cr#EOC#

# -- SiCortex scg++ C++
#MIPS32LINUX#SC_GNU_OPTIONS=-mabi=n32 #EOC#
#SCGCC#CXX = scg++ $(USEROPT) -w $(EXTRAOPT) $(SC_GNU_OPTIONS) #EOC#
#SCGCC#CXXAR = scar cr#EOC#
#SCGCC#PDT_RANLIB=scranlib libpdb.a#EOC#

# -- Apple c++
#CPP#CXX = c++ $(USEROPT) -w -Wall -ansi $(EXTRAOPT) #EOC#
#CPP#CXXAR = ar cr#EOC#

#CLANG#CXX = clang++ $(USEROPT) -w -Wall -ansi $(EXTRAOPT) #EOC#
#CLANG#CXXAR = ar cr#EOC#

# -- Compaq Tru64 cxx
#CXX#CXX = cxx $(USEROPT) $(EXTRAOPT) #EOC#
#CXX#CXXAR = ar cr#EOC#
#CXX#EXTRAOBJS = cxx_repository/*.o #EOC#

# -- IBM AIX xlC
#IBM64#IBM_XLC_OPTIONS=-q64 #EOC#
#IBM64#IBM_GNU_OPTIONS=-maix64 #EOC#

#IBM64LINUX#IBM_XLC_OPTIONS=-q64 #EOC#
#IBM64LINUX#IBM_GNU_OPTIONS=-m64 #EOC#


#GXX#PLATFORM_CXX_OPTS=$(IBM_GNU_OPTIONS) $(SUN_GNU_OPTIONS) $(SC_GNU_OPTIONS) #EOC#
#IA32LINUX#PLATFORM_CXX_OPTS=-m32 #EOC#
#XLC#PLATFORM_CXX_OPTS=$(IBM_XLC_OPTIONS) #EOC#
#SOLCC#PLATFORM_CXX_OPTS=$(SUN_CC_OPTIONS) #EOC#

#XLC#IBM_XLC_CXX = xlC $(USEROPT) $(EXTRAOPT) -qlanglvl=extended $(PLATFORM_CXX_OPTS) #EOC#
#PPCLINUX#IBM_XLC_CXX = xlC $(USEROPT) $(EXTRAOPT) $(PLATFORM_CXX_OPTS) #EOC#
#DARWIN#IBM_XLC_CXX = xlC $(USEROPT) $(EXTRAOPT) $(PLATFORM_CXX_OPTS) #EOC#
#IBM64LINUX#IBM_XLC_CXX = xlC $(USEROPT) $(EXTRAOPT) $(PLATFORM_CXX_OPTS) #EOC#
#XLC#CXX = $(IBM_XLC_CXX) #EOC#



#IBM64#IBM_AR64_OPTS = -X64 #EOC#
#IBM64#PLATFORM_AR_OPTS = $(IBM_AR64_OPTS) #EOC#

#XLC#CXXAR = ar $(PLATFORM_AR_OPTS) cr #EOC#
#XLC#EXTRAOPT= #EOC#

# -- PGI pgCC 
#PGI#EXTRAOPT=  #EOC#
#PGI#CXX = pgCC $(USEROPT) $(EXTRAOPT) #EOC#
#PGI#CXXAR = ar $(PLATFORM_AR_OPTS) cr#EOC#
#PGI#PRELINK = pgCC --prelink_objects *.o #EOC#

# -- PGI6+ pgCC 
# PGI 6 is used without --prelink_objects option 
#PGI6#EXTRAOPT=  #EOC#
#PGI6#CXX = pgc++ $(USEROPT) $(EXTRAOPT) #EOC#
#PGI6#CXXAR = ar $(PLATFORM_AR_OPTS) cr#EOC#
#PGI6#PRELINK = #EOC#

# -- PGI Apple OS X Intel Macs
#APPLEPGI#EXTRAOPT= -D_XOPEN_SOURCE #EOC#
#APPLEPGI#CXX = pgcpp $(USEROPT) $(EXTRAOPT) #EOC#
#APPLEPGI#CXXAR = ar $(PLATFORM_AR_OPTS) cr#EOC#
#APPLEPGI#PRELINK =  #EOC#

# -- INTEL icpc (IA32) 
#ICPC#EXTRAOPT= #EOC#
#ICPC#CXX = icpc $(USEROPT) $(EXTRAOPT) #EOC#
#ICPC#CXXAR = ar $(PLATFORM_AR_OPTS) cr#EOC#

# -- INTEL ecpc (IA64) 
#ECPC#EXTRAOPT= #EOC#
#ECPC#CXX = ecpc $(USEROPT) $(EXTRAOPT) #EOC#
#ECPC#CXXAR = ar $(PLATFORM_AR_OPTS) cr#EOC#

# -- egcs
#EGXX#CXX = eg++ $(USEROPT) -w -Wall -ansi $(EXTRAOPT) #EOC#
#EGXX#CXXAR = ar $(PLATFORM_AR_OPTS) cr#EOC#

# -- g++
PDT_GXX=g++ 
#IBM64LINUX##PDT_GXX=powerpc64-linux-g++ #EOC#

# -- g++4
#GXX4#PDT_GXX=g++4 #EOC#
#GXX4#CXX = $(PDT_GXX) $(USEROPT) -w -Wall -ansi $(EXTRAOPT) $(PLATFORM_CXX_OPTS) #EOC#
#GXX4#CXXAR = ar $(PLATFORM_AR_OPTS) cr #EOC#

#GXX#CXX = $(PDT_GXX) $(USEROPT) -w -Wall -ansi $(EXTRAOPT) $(PLATFORM_CXX_OPTS) #EOC#
#GXX#CXXAR = ar $(PLATFORM_AR_OPTS) cr #EOC#
#CRAYXMT#CXXAR = /usr/bin/ar $(PLATFORM_AR_OPTS) cr #EOC#
#DARWIN#PDT_RANLIB=ranlib libpdb.a#EOC#


# -- SUN CC
#SOLCC#CXX = CC $(USEROPT) $(PLATFORM_CXX_OPTS) #EOC#
#SOLCC#CXXAR = CC $(USEROPT) -xar -o#EOC#


# -- KAI KCC
#KCC#EXTRAOPT=  #EOC#
#KCC#CXX = KCC $(USEROPT) $(EXTRAOPT) --one_instantiation_per_object#EOC#
#KCC#CXXAR = KCC $(USEROPT) -o#EOC#

all :
	(cd lib; $(MAKE) "MAKE=$(MAKE)" all CXX="$(CXX)" CXXAR="$(CXXAR)" RANLIB="$(PDT_RANLIB)" PRELINK="$(PRELINK)" EXTRAOBJS="$(EXTRAOBJS)")
	(cd src; $(MAKE) "MAKE=$(MAKE)" all CXX="$(CXX)")

install:
	(cd src; $(MAKE) "MAKE=$(MAKE)" install)

clean :
	(cd lib; $(MAKE) "MAKE=$(MAKE)" clean)
	(cd src; $(MAKE) "MAKE=$(MAKE)" clean)

