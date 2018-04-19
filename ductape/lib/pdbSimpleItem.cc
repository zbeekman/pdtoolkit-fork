/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbSimpleItem.h"
#include "pdbFile.h"
#include "pdbRoutine.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbSimpleItem.inl"
#endif

#ifndef _MSC_VER
const int pdbSimpleItem::UNIQUE ;
const int pdbSimpleItem::NOTDEL ;
#endif

ostream& operator<<(ostream& ostr, const pdbSimpleItem& i) {
  return i.print(ostr);
}

const char *pdbSimpleItem::attrPrefix() const {
  static char pre[2];

  pre[0] = desc()[0];
  pre[1] = '\0';

  return pre;
}

ostream& pdbLoc::printLoc(ostream& ostr) const {
  if ( const pdbFile *f = file() )
    ostr << "so#" << f->id();
  else
    ostr << "NULL";
  return ostr << " " << line() << " " << col();
}

ostream& operator<<(ostream& ostr, const pdbLoc& l) {
  return l.printLoc(ostr);
}

