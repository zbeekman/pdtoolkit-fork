/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbModule.h"
#include "pdbGroupField.h"
#include "pdbFRoutine.h"
#include "pdbType.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbModule.inl"
#endif

ostream& operator<<(ostream& ostr, const pdbModFunc& f) {
  ostr << "ro#" << f.func()->id() << " ";
  return f.printLoc(ostr);
}

pdbModule::~pdbModule() {
  for (modfuncvec::const_iterator mt=fmem.begin(); mt!=fmem.end(); ++mt)
     delete *mt;
}

pdbSimpleItem::dupl_t pdbModule::findDuplicate(pdbSimpleItem* r) {
  pdbModule* rhs = dynamic_cast<pdbModule*> (r);
  if ( fullName() == rhs->fullName() ) {
    int mysize = funcMembers().size() + dataMembers().size();
    int rhssize = rhs->funcMembers().size() + rhs->dataMembers().size();
    if ( mysize < rhssize )
      return OLDDUPL;
    else
      return NEWDUPL;
  } else {
      return NODUPL;
  }
}

void pdbModule::process(PDB *p) {
  pdbItem::process(p);
  for( fieldvec::iterator cit = dmem.begin(); cit!=dmem.end(); ++cit) {
    // make sure all data members are "processed" too
    (*cit)->process(p);
  }
}

ostream& pdbModule::print(ostream& ostr) const {
  pdbGroup::print(ostr);
  const modfuncvec f = funcMembers();
  for (modfuncvec::const_iterator ft=f.begin(); ft!=f.end(); ++ft)
     ostr << "gfunc " << (**ft) << "\n";
  return ostr << endl;
}

void pdbModule::adjustPtrs(PDB* p) {
  pdbGroupField* fd;
  const pdbFile* fi;

  pdbGroup::adjustPtrs(p);
  for( modfuncvec::iterator fit = fmem.begin(); fit!=fmem.end(); ++fit) {
    if ( (*fit)->func()->newId() > UNIQUE )
      (*fit)->func(p->getFRoutineMap()[(*fit)->func()->newId()]);
    if ( (fi=(*fit)->file()) && (fi->newId() > UNIQUE) )
      (*fit)->file(p->getFileMap()[fi->newId()]);
  }
  for( fieldvec::iterator cit = dmem.begin(); cit!=dmem.end(); ++cit) {
    fd = (*cit);
    if ( fd->parentGroup()->newId() > UNIQUE )
      fd->parentGroup(p->getModuleMap()[fd->parentGroup()->newId()]);
    if ( fd->type()->newId() > UNIQUE )
      fd->type(p->getTypeMap()[fd->type()->newId()]);
    if ( (fi=fd->location().file()) && (fi->newId() > UNIQUE) )
      fd->location().file(p->getFileMap()[fi->newId()]);
  }
}
