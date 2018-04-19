INC = ../inc

.SUFFIXES: .cc .o .pdb .h

CCFLAGS = -I$(INC)

#CXXPARSE = cxxparse
#CXXPFLAGS = -I$(INC)
CXXPARSE = true

PDBOBJS = pdbRead.o pdbMisc.o pdbItem.o pdbGroup.o pdbClass.o pdbModule.o \
	  pdbRoutine.o pdbCRoutine.o pdbFRoutine.o pdbTemplate.o \
	  pdbGroupField.o pdbWrite.o pdbMacro.o pdbNamespace.o pdbType.o \
	  pdbFile.o pdbSimpleItem.o pdbTemplateItem.o pdbFatItem.o \
	  pdbTemplateArg.o pdbPragma.o 

all : libpdb.a

libpdb.a: $(PDBOBJS)
	$(PRELINK)
	$(CXXAR) libpdb.a $(PDBOBJS) $(EXTRAOBJS)
	$(RANLIB) 

.cc.o:
	$(CXX) $(CCFLAGS) -c $<
	$(CXXPARSE) $< $(CXXPFLAGS)

clean:
	rm -rf core *.o $(PROG) libpdb.a $(EXTRAOBJS) *~
	rm -rf a.out *.ti *.ii ii_files ti_files

# ===========================================================================
# Dependencies generated with:
#   g++ -M -I../inc *.cc | sed -e 's,/opt[^ ]* ,,g' -e 's,/usr[^ ]* ,,g'
#                              -e '/^  \\$/d' -e 's,../inc,$(INC),g'
#
pdbClass.o: pdbClass.cc $(INC)/pdbType.h $(INC)/pdbItem.h \
  $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbType.inl \
  $(INC)/pdbClass.h $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbClass.inl \
  $(INC)/pdbGroupField.h $(INC)/pdbGroupField.inl $(INC)/pdbNamespace.h \
  $(INC)/pdbNamespace.inl $(INC)/pdbTemplate.h $(INC)/pdbTemplate.inl
pdbCRoutine.o: pdbCRoutine.cc \
  $(INC)/pdbType.h $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbType.inl \
  $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbClass.h \
  $(INC)/pdbGroup.h $(INC)/pdbGroup.inl $(INC)/pdbClass.inl
pdbFatItem.o: pdbFatItem.cc $(INC)/pdbFatItem.h $(INC)/pdbItem.h \
  $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl
pdbFile.o: pdbFile.cc $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl
pdbFRoutine.o: pdbFRoutine.cc $(INC)/pdbFRoutine.h \
  $(INC)/pdbRoutine.h $(INC)/pdbTemplateItem.h $(INC)/pdbFatItem.h \
  $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbRoutine.inl $(INC)/pdbFRoutine.inl
pdbGroup.o: pdbGroup.cc $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbGroup.inl $(INC)/pdbGroupField.h \
  $(INC)/pdbGroupField.inl
pdbGroupField.o: pdbGroupField.cc $(INC)/pdbClass.h $(INC)/pdbGroup.h \
  $(INC)/pdbTemplateItem.h $(INC)/pdbFatItem.h $(INC)/pdbItem.h \
  $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h \
  $(INC)/pdbRoutine.h $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl \
  $(INC)/pdbClass.inl $(INC)/pdbGroupField.h $(INC)/pdbGroupField.inl \
  $(INC)/pdbType.h $(INC)/pdbType.inl
pdbItem.o: pdbItem.cc $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbClass.h \
  $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h $(INC)/pdbFatItem.h \
  $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h \
  $(INC)/pdbRoutine.h $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl \
  $(INC)/pdbClass.inl $(INC)/pdbModule.h $(INC)/pdbFRoutine.h \
  $(INC)/pdbFRoutine.inl $(INC)/pdbModule.inl $(INC)/pdbNamespace.h \
  $(INC)/pdbNamespace.inl
pdbMacro.o: pdbMacro.cc $(INC)/pdbMacro.h $(INC)/pdbItem.h $(INC)/pdb.h \
  $(INC)/pdb.inl $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl \
  $(INC)/pdbFile.h $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbMacro.inl
pdbMisc.o: pdbMisc.cc $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbRoutine.h $(INC)/pdbTemplateItem.h $(INC)/pdbFatItem.h \
  $(INC)/pdbItem.h $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl \
  $(INC)/pdbFile.h $(INC)/pdbFile.inl $(INC)/pdbItem.inl \
  $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbRoutine.inl $(INC)/pdbClass.h \
  $(INC)/pdbGroup.h $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h \
  $(INC)/pdbCRoutine.inl $(INC)/pdbClass.inl
pdbModule.o: pdbModule.cc $(INC)/pdbModule.h $(INC)/pdbGroup.h \
  $(INC)/pdbTemplateItem.h $(INC)/pdbFatItem.h $(INC)/pdbItem.h $(INC)/pdb.h \
  $(INC)/pdb.inl $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl \
  $(INC)/pdbFile.h $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbGroup.inl $(INC)/pdbFRoutine.h \
  $(INC)/pdbRoutine.h $(INC)/pdbRoutine.inl $(INC)/pdbFRoutine.inl \
  $(INC)/pdbModule.inl $(INC)/pdbGroupField.h $(INC)/pdbGroupField.inl \
  $(INC)/pdbType.h $(INC)/pdbType.inl
pdbNamespace.o: pdbNamespace.cc $(INC)/pdbNamespace.h $(INC)/pdbFatItem.h \
  $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbNamespace.inl $(INC)/pdbClass.h $(INC)/pdbGroup.h \
  $(INC)/pdbTemplateItem.h $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbClass.inl \
  $(INC)/pdbType.h $(INC)/pdbType.inl $(INC)/pdbTemplate.h \
  $(INC)/pdbTemplate.inl
pdbPragma.o: pdbPragma.cc $(INC)/pdbPragma.h $(INC)/pdbItem.h $(INC)/pdb.h \
  $(INC)/pdb.inl $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl \
  $(INC)/pdbFile.h  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbPragma.inl
pdbRead.o: pdbRead.cc $(INC)/pdbAll.h $(INC)/pdbType.h \
  $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbType.inl \
  $(INC)/pdbClass.h $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbClass.inl \
  $(INC)/pdbModule.h $(INC)/pdbFRoutine.h $(INC)/pdbFRoutine.inl \
  $(INC)/pdbModule.inl $(INC)/pdbGroupField.h $(INC)/pdbGroupField.inl \
  $(INC)/pdbTemplate.h $(INC)/pdbTemplate.inl $(INC)/pdbMacro.h \
  $(INC)/pdbMacro.inl $(INC)/pdbPragma.h $(INC)/pdbPragma.inl \
  $(INC)/pdbNamespace.h $(INC)/pdbNamespace.inl $(INC)/pdbTraits.h
pdbRoutine.o: pdbRoutine.cc $(INC)/pdbType.h $(INC)/pdbItem.h \
  $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbType.inl \
  $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbFRoutine.h \
  $(INC)/pdbFRoutine.inl $(INC)/pdbClass.h $(INC)/pdbGroup.h \
  $(INC)/pdbGroup.inl $(INC)/pdbClass.inl
pdbSimpleItem.o: pdbSimpleItem.cc $(INC)/pdbSimpleItem.h $(INC)/pdb.h \
  $(INC)/pdb.inl $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h $(INC)/pdbFile.inl \
  $(INC)/pdbRoutine.h $(INC)/pdbTemplateItem.h $(INC)/pdbFatItem.h \
  $(INC)/pdbItem.h $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbRoutine.inl
pdbTemplateArg.o: pdbTemplateArg.cc $(INC)/pdbTemplate.h \
  $(INC)/pdbFatItem.h $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplate.inl $(INC)/pdbType.h $(INC)/pdbType.inl \
  $(INC)/pdbClass.h $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h \
  $(INC)/pdbRoutine.h $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl \
  $(INC)/pdbClass.inl
pdbTemplate.o: pdbTemplate.cc $(INC)/pdbTemplate.h $(INC)/pdbFatItem.h \
  $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplate.inl $(INC)/pdbType.h $(INC)/pdbType.inl \
  $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl \
  $(INC)/pdbClass.h $(INC)/pdbGroup.h $(INC)/pdbGroup.inl \
  $(INC)/pdbClass.inl
pdbTemplateItem.o: pdbTemplateItem.cc $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbFatItem.inl \
  $(INC)/pdbTemplateArg.h $(INC)/pdbTemplateArg.inl \
  $(INC)/pdbTemplateItem.inl $(INC)/pdbTemplate.h $(INC)/pdbTemplate.inl
pdbType.o: pdbType.cc $(INC)/pdbType.h $(INC)/pdbItem.h $(INC)/pdb.h \
  $(INC)/pdb.inl $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl \
  $(INC)/pdbFile.h $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbType.inl \
  $(INC)/pdbClass.h $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbClass.inl \
  $(INC)/pdbModule.h $(INC)/pdbFRoutine.h $(INC)/pdbFRoutine.inl \
  $(INC)/pdbModule.inl
pdbWrite.o: pdbWrite.cc $(INC)/pdbAll.h $(INC)/pdbType.h \
  $(INC)/pdbItem.h $(INC)/pdb.h $(INC)/pdb.inl \
  $(INC)/pdbSimpleItem.h $(INC)/pdbSimpleItem.inl $(INC)/pdbFile.h \
  $(INC)/pdbFile.inl $(INC)/pdbItem.inl $(INC)/pdbType.inl \
  $(INC)/pdbClass.h $(INC)/pdbGroup.h $(INC)/pdbTemplateItem.h \
  $(INC)/pdbFatItem.h $(INC)/pdbFatItem.inl $(INC)/pdbTemplateArg.h \
  $(INC)/pdbTemplateArg.inl $(INC)/pdbTemplateItem.inl \
  $(INC)/pdbGroup.inl $(INC)/pdbCRoutine.h $(INC)/pdbRoutine.h \
  $(INC)/pdbRoutine.inl $(INC)/pdbCRoutine.inl $(INC)/pdbClass.inl \
  $(INC)/pdbModule.h $(INC)/pdbFRoutine.h $(INC)/pdbFRoutine.inl \
  $(INC)/pdbModule.inl $(INC)/pdbGroupField.h $(INC)/pdbGroupField.inl \
  $(INC)/pdbTemplate.h $(INC)/pdbTemplate.inl $(INC)/pdbMacro.h \
  $(INC)/pdbMacro.inl $(INC)/pdbPragma.h $(INC)/pdbPragma.inl \
  $(INC)/pdbNamespace.h $(INC)/pdbNamespace.inl
