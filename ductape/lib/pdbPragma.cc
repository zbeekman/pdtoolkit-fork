/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbPragma.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbPragma.inl"
#endif


ostream& pdbPragma::print(ostream& ostr) const {
  pdbItem::print(ostr);
  if ( kind() != "" ) ostr << "pkind " << kind() << "\n";
  ostr << "ppos " << prBegin() << " " << prEnd() << "\n";
  ostr << "ptext " << text() << "\n";
  return ostr << endl;
}

pdbSimpleItem::dupl_t pdbPragma::findDuplicate(pdbSimpleItem* r) {
  pdbPragma* rhs = dynamic_cast<pdbPragma*> (r);
  if ( name() == rhs->name() && kind() == rhs->kind() ) {
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
