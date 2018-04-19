/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbClass.h"
#include "pdbGroupField.h"
#include "pdbType.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbGroupField.inl"
#endif

ostream& pdbGroupField::print(ostream& ostr) const {
  const pdbGroup *gr;

  ostr << "gmem " << name() << "\n";
  if ( location().file() )ostr << "gmloc " << location() << "\n";
  if ( const pdbGroup* gptr = parentGroup() ) {
    //gmgroup useless for C/C++
    if ( gptr->kind() == GR_FMODULE && !location().file() )
      ostr << "gmgroup gr#" << gptr->id() << "\n";
    ostr << "gmacs " << toName(access()) << "\n";
  }
  ostr << "gmkind " << toName(kind()) << "\n";
  if ( type() ) {
    if ( (gr = type()->isGroup()) != 0 )
      ostr << "gmtype " << "gr#" << gr->id() << "\n";
    else
      ostr << "gmtype " << "ty#" << type()->id() << "\n";
  }
  pdbTemplateItem::print(ostr);
  if ( isStaticConst() ) ostr << "gmconst T\n";
  if ( isBitField() ) ostr << "gmisbit T\n";
  if ( isMutable() ) ostr << "gmmut T\n";
  return ostr;
}
