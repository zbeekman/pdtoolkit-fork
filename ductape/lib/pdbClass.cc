/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbType.h"
#include "pdbClass.h"
#include "pdbGroupField.h"
#include "pdbCRoutine.h"
#include "pdbNamespace.h"
#include "pdbTemplate.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbClass.inl"
#endif

ostream& operator<<(ostream& ostr, const pdbBase& b) {
  ostr << pdbItem::toName(b.virtuality())
       << " " << pdbItem::toName(b.access())
       << " gr#" << b.base()->id() << " ";
  return b.printLoc(ostr);
}
ostream& operator<<(ostream& ostr, const pdbFriendFunc& f) {
  ostr << "ro#" << f.friendFunc()->id() << " ";
  return f.printLoc(ostr);
}

ostream& operator<<(ostream& ostr, const pdbFriendClass& c) {
  ostr << "gr#" << c.friendClass()->id() << " ";
  return c.printLoc(ostr);
}

ostream& operator<<(ostream& ostr, const pdbMethod& f) {
  ostr << "ro#" << f.func()->id() << " ";
  return f.printLoc(ostr);
}

pdbClass::~pdbClass() {
  for (basevec::const_iterator bt=base.begin(); bt!=base.end(); ++bt)
     delete *bt;
  for (methodvec::const_iterator mt=fmem.begin(); mt!=fmem.end(); ++mt)
     delete *mt;
  for (friendfuncvec::const_iterator ft=fro.begin(); ft!=fro.end(); ++ft)
     delete *ft;
  for (friendclassvec::const_iterator ct=fcl.begin(); ct!=fcl.end(); ++ct)
     delete *ct;
}

pdbSimpleItem::dupl_t pdbClass::findDuplicate(pdbSimpleItem* r) {
  pdbClass* rhs = dynamic_cast<pdbClass*> (r);
  if ( fullName() == rhs->fullName() ) {
    int mysize = methods().size() + dataMembers().size();
    int rhssize = rhs->methods().size() + rhs->dataMembers().size();
    if ( mysize < rhssize )
      return OLDDUPL;
    else
      return NEWDUPL;
  } else {
      return NODUPL;
  }
}

void pdbClass::process(PDB *p) {
  // ??? name is fullName, because of bug in frontend
  // ??? to fix this bug, try to correct name
  size_t pos;
  if ( !isTemplate() ) {
    if ( (pos = name().rfind("::")) != STR_NPOS ) {
      name(name().substr(pos+2, STR_NPOS));
    }
    pdbItem::process(p);
  } else {
    fullName(name());
  }
  // the same for type members
  const pdbType* ty;
  for( fieldvec::iterator cit = dmem.begin(); cit!=dmem.end(); ++cit) {
    if ( (*cit)->kind() == M_TYPE ) {
      ty = (*cit)->type();
      if ( ty->kind()==TY_ENUM || ty->kind()==TY_GROUP ||
           (ty->kind()==TY_TREF && ty->qualifiers().size()==0) ) {
        if ( (pos = (*cit)->name().rfind("::")) != STR_NPOS ) {
          (*cit)->name((*cit)->name().substr(pos+2, STR_NPOS));
        }
      }
    }
    // make sure all data members are "processed" too
    (*cit)->process(p);
  }

  // if class has no base classes, add it to the toplevel classes
  if ( base.size() == 0 ) {
    p->classTree()->addDerivedClass(this);
  }

  // determine if class is Abstract Base Class
  if ( kind() == GR_CLASS || kind() == GR_STRUCT ) {
    const pdbClass::methodvec f = methods();
    for (pdbClass::methodvec::const_iterator ft=f.begin(); ft!=f.end(); ++ft) {
      if ( const pdbCRoutine *cr =
           static_cast<const pdbCRoutine*>((*ft)->func()) ) {
        if ( cr->virtuality() == VI_PURE ) {
          abc = true;
          break;
        }
      }
    }
  }
}

ostream& pdbClass::print(ostream& ostr) const {
  pdbGroup::print(ostr);
  const pdbClass::basevec b = baseClasses();
  for (pdbClass::basevec::const_iterator bt=b.begin(); bt!=b.end(); ++bt)
     ostr << "gbase " << (**bt) << "\n";
  const pdbClass::friendclassvec fc = friendClasses();
  for (pdbClass::friendclassvec::const_iterator fct=fc.begin();
       fct!=fc.end(); ++fct)
     ostr << "gfrgroup " << (**fct) << "\n";
  const pdbClass::friendfuncvec fr = friendRoutines();
  for (pdbClass::friendfuncvec::const_iterator frt=fr.begin();
       frt!=fr.end(); ++frt)
     ostr << "gfrfunc " << (**frt) << "\n";
  const pdbClass::methodvec f = methods();
  for (pdbClass::methodvec::const_iterator ft=f.begin(); ft!=f.end(); ++ft)
     ostr << "gfunc " << (**ft) << "\n";
  return ostr << endl;
}

void pdbClass::adjustPtrs(PDB* p) {
  pdbGroupField* fd;
  const pdbFile* fi;
  const pdbTemplate *tm;

  pdbGroup::adjustPtrs(p);
  for( basevec::iterator bit = base.begin(); bit!=base.end(); ++bit) {
    if ( (*bit)->base()->newId() > UNIQUE )
      (*bit)->base(p->getClassMap()[(*bit)->base()->newId()]);
    if ( (fi=(*bit)->file()) && (fi->newId() > UNIQUE) )
      (*bit)->file(p->getFileMap()[fi->newId()]);
  }
  for( friendclassvec::iterator fcit = fcl.begin(); fcit!=fcl.end(); ++fcit) {
    if ( (*fcit)->friendClass()->newId() > UNIQUE )
      (*fcit)->friendClass(p->getClassMap()[(*fcit)->friendClass()->newId()]);
    if ( (fi=(*fcit)->file()) && (fi->newId() > UNIQUE) )
      (*fcit)->file(p->getFileMap()[fi->newId()]);
  }
  for( friendfuncvec::iterator frit = fro.begin(); frit!=fro.end(); ++frit) {
    if ( (*frit)->friendFunc()->newId() > UNIQUE )
      (*frit)->friendFunc(p->getCRoutineMap()[(*frit)->friendFunc()->newId()]);
    if ( (fi=(*frit)->file()) && (fi->newId() > UNIQUE) )
      (*frit)->file(p->getFileMap()[fi->newId()]);
  }
  for( classvec::iterator dit = drvd.begin(); dit!=drvd.end(); ++dit) {
    if ( (*dit)->newId() > UNIQUE )
      (*dit) = p->getClassMap()[(*dit)->newId()];
  }
  for( methodvec::iterator fit = fmem.begin(); fit!=fmem.end(); ++fit) {
    if ( (*fit)->func()->newId() > UNIQUE )
      (*fit)->func(p->getCRoutineMap()[(*fit)->func()->newId()]);
    if ( (fi=(*fit)->file()) && (fi->newId() > UNIQUE) )
      (*fit)->file(p->getFileMap()[fi->newId()]);
  }
  for( fieldvec::iterator cit = dmem.begin(); cit!=dmem.end(); ++cit) {
    fd = (*cit);
    // if ( fd->parentNSpace() && fd->parentNSpace()->newId() > UNIQUE )
    //   fd->parentNSpace(p->getNamespaceMap()[fd->parentNSpace()->newId()]);
    if ( fd->parentGroup()->newId() > UNIQUE )
      fd->parentGroup(p->getClassMap()[fd->parentGroup()->newId()]);
    if ( fd->type() && fd->type()->newId() > UNIQUE )
      fd->type(p->getTypeMap()[fd->type()->newId()]);
    if ( (fi=fd->location().file()) && (fi->newId() > UNIQUE) )
      fd->location().file(p->getFileMap()[fi->newId()]);
    if ( (tm=fd->isTemplate()) && (tm != TE_UNKNOWN) && (tm->newId() > UNIQUE) )
      fd->isTemplate(p->getTemplateMap()[tm->newId()]);
  }
}
