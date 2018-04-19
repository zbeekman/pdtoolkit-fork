/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbGroup.h"
#include "pdbGroupField.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbGroup.inl"
#endif

pdbGroup::~pdbGroup() {
  for (fieldvec::const_iterator dt=dmem.begin(); dt!=dmem.end(); ++dt)
     delete *dt;
}

ostream& pdbGroup::print(ostream& ostr) const {
  pdbItem::print(ostr);
  ostr << "gkind " << toName(kind()) << "\n";
  pdbTemplateItem::print(ostr);
  const fieldvec d = dataMembers();
  for (fieldvec::const_iterator dt=d.begin(); dt!=d.end(); ++dt)
     ostr << **dt;
  pdbFatItem::print(ostr);
  return ostr;
}
