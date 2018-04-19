/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include <ctype.h>
#include "pdbFRoutine.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbFRoutine.inl"
#endif

pdbFRoutine::~pdbFRoutine() {
  for (locvec::const_iterator ct=stops.begin(); ct!=stops.end(); ++ct)
    delete *ct;
}

void pdbFRoutine::process(PDB *p) {
  // build up full name:
  // do it like for pdbRoutine, but add signature
  pdbRoutine::process(p);

  /*???*/
}

ostream& pdbFRoutine::print(ostream& ostr) const {
  pdbRoutine::print(ostr);
  const pdbRoutine::locvec le = stopLocations();
  for (pdbRoutine::locvec::const_iterator lt=le.begin(); lt!=le.end(); ++lt)
    ostr << "rstop " << **lt << "\n";
  if ( parentRoutine() )
    ostr << "rroutine ro#" << parentRoutine()->id() << '\n';
  if ( isAliasFor() )
    ostr << "ralias ro#" << isAliasFor()->id() << '\n';
  const pdbFRoutine::implvec im = implementedBy();
  for (pdbFRoutine::implvec::const_iterator it=im.begin(); it!=im.end(); ++it)
    ostr << "rimpl ro#" << (*it)->id() << '\n';
  if ( hasExplicitInterface() ) ostr << "rarginfo T\n";
  if ( isRecursive() ) ostr << "rrec T\n";
  if ( isElemental() ) ostr << "riselem T\n";
  return ostr << endl;
}

void pdbFRoutine::adjustPtrs(PDB* p) {
  const pdbFile* fi;

  if ( parentRoutine()->newId() > UNIQUE )
    parentRoutine(p->getFRoutineMap()[parentRoutine()->newId()]);
  if ( isAliasFor()->newId() > UNIQUE )
    isAliasFor(p->getFRoutineMap()[isAliasFor()->newId()]);
  for( implvec::iterator it = impls.begin(); it!=impls.end(); ++it) {
    if ( (*it)->newId() > UNIQUE )
      *it = p->getFRoutineMap()[(*it)->newId()];
  }
  for( locvec::iterator lt = stops.begin(); lt!=stops.end(); ++lt) {
    if ( (fi=(*lt)->file()) && (fi->newId() > UNIQUE) )
      (*lt)->file(p->getFileMap()[fi->newId()]);
  }
}
