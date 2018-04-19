.SUFFIXES: .h.pdb .h

PDBS = pdb.h.pdb pdbCRoutine.h.pdb pdbClass.h.pdb pdbFRoutine.h.pdb \
	pdbFatItem.h.pdb pdbFile.h.pdb pdbGroup.h.pdb pdbGroupField.h.pdb \
	pdbItem.h.pdb pdbMacro.h.pdb pdbModule.h.pdb pdbNamespace.h.pdb \
	pdbRoutine.h.pdb pdbSimpleItem.h.pdb pdbTemplate.h.pdb \
	pdbTemplateItem.h.pdb pdbType.h.pdb

../html/index.html: ductape.pdb
	../src/pdbhtml ductape.pdb

ductape.pdb: $(PDBS)
	../src/pdbmerge -v -o ductape.pdb $(PDBS)

clean:
	rm *.pdb

.h.h.pdb:
	cxxparse $*.h -DNO_INLINE

pdb.h.pdb: pdb.h
pdbCRoutine.h.pdb: pdbCRoutine.h
pdbClass.h.pdb: pdbClass.h
pdbFRoutine.h.pdb: pdbFRoutine.h
pdbFatItem.h.pdb: pdbFatItem.h
pdbFile.h.pdb: pdbFile.h
pdbGroup.h.pdb: pdbGroup.h
pdbGroupField.h.pdb: pdbGroupField.h
pdbItem.h.pdb: pdbItem.h
pdbMacro.h.pdb: pdbMacro.h
pdbModule.h.pdb: pdbModule.h
pdbNamespace.h.pdb: pdbNamespace.h
pdbRoutine.h.pdb: pdbRoutine.h
pdbSimpleItem.h.pdb: pdbSimpleItem.h
pdbTemplate.h.pdb: pdbTemplate.h
pdbTemplateItem.h.pdb: pdbTemplateItem.h
pdbType.h.pdb: pdbType.h
