/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbTemplateItem.h"
#include "pdbTemplate.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbTemplateItem.inl"
#endif

const pdbTemplate* pdbTemplateItem::TE_UNKNOWN = (const pdbTemplate*)1;

ostream& pdbTemplateItem::print(ostream& ostr) const {
  const char *X = attrPrefix();

  if ( const pdbTemplate* tptr = isTemplate() ) {
    if ( tptr == TE_UNKNOWN )
      ostr << X << "templ T\n";
    else
      ostr << X << "templ te#" << tptr->id() << "\n";
    if ( isSpecialized() ) ostr << X << "specl T\n";
    const targvec& s = speclArguments();
    for (targvec::const_iterator it=s.begin(); it!=s.end(); ++it) {
      ostr << "gsparam " << *it << "\n";
    }
  }
  return ostr;
}

void pdbTemplateItem::adjustPtrs(PDB* p) {
  pdbFatItem::adjustPtrs(p);
  if ( tmpl && (tmpl != TE_UNKNOWN) && (tmpl->newId() > UNIQUE) )
    tmpl = p->getTemplateMap()[tmpl->newId()];
  targvec& s = sargs;
  for (targvec::iterator it=s.begin(); it!=s.end(); ++it) it->adjustPtrs(p);
}
