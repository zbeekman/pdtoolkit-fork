/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                   */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbMacro.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbMacro.inl"
#endif

ostream& pdbMacro::print(ostream& ostr) const {
  pdbItem::print(ostr);
  ostr << "mkind " << toName(kind()) << "\n";
  ostr << "mtext " << text() << "\n";
  return ostr << endl;
}

pdbSimpleItem::dupl_t pdbMacro::findDuplicate(pdbSimpleItem* r) {
  pdbMacro* rhs = dynamic_cast<pdbMacro*> (r);
  if ( fullName() == rhs->fullName() ) {
    pdbLoc ll = location();
    pdbLoc rl = rhs->location();
    if ( (ll.file() && rl.file()) && (ll.file()->name() == rl.file()->name()) &&
         (ll.line() == rl.line()) && (ll.col() == rl.col()) )
      return NEWDUPL;
    else if ( !ll.file() && !rl.file() ) 
      return NEWDUPL;
    else
      return NODUPL;
  } else {
    return NODUPL;
  }
}
