/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbNamespace.h"
#include "pdbClass.h"
#include "pdbType.h"
#include "pdbCRoutine.h"
#include "pdbTemplate.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbNamespace.inl"
#endif

ostream& pdbNamespace::print(ostream& ostr) const {
  pdbItem::print(ostr);
  if ( isAlias() ) ostr << "nalias na#" << isAlias()->id() << "\n";
  for (memvec::const_iterator it = members().begin();
       it!=members().end(); ++it) {
    ostr << "nmem " << (*it)->desc()[0] << (*it)->desc()[1] << "#"
         << (*it)->id() << "\n";
  }
  pdbFatItem::print(ostr);
  return ostr << endl;
}

void pdbNamespace::adjustPtrs(PDB* p) {
  pdbFatItem::adjustPtrs(p);
  if ( isAlias() && isAlias()->newId() > UNIQUE )
    isAlias(p->getNamespaceMap()[isAlias()->newId()]);
  for( memvec::iterator it = mems.begin(); it!=mems.end(); ++it) {
    if ( (*it)->newId() > UNIQUE ) {
      if ( dynamic_cast<pdbCRoutine*>(*it) )
        *it = p->getCRoutineMap()[(*it)->newId()];
      else if ( dynamic_cast<pdbClass*>(*it) )
        *it = p->getClassMap()[(*it)->newId()];
      else if ( dynamic_cast<pdbType*>(*it) )
        *it = p->getTypeMap()[(*it)->newId()];
      else if ( dynamic_cast<pdbTemplate*>(*it) )
        *it = p->getTemplateMap()[(*it)->newId()];
      else if ( dynamic_cast<pdbNamespace*>(*it) )
        *it = p->getNamespaceMap()[(*it)->newId()];
    }
  }
}

pdbSimpleItem::dupl_t pdbNamespace::findDuplicate(pdbSimpleItem* r) {
  pdbNamespace* rhs = dynamic_cast<pdbNamespace*> (r);
  if ( fullName() == rhs->fullName() ) {
    if ( name() == "$NA$" )         // unnamed namespace
      return NODUPL;
    else
      return NEWDUPL;
  } else {
      return NODUPL;
  }
}
