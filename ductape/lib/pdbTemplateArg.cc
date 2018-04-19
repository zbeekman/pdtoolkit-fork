/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbTemplate.h"
#include "pdbType.h"
#include "pdbClass.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbTemplateArg.inl"
#endif

extern void pdb_ERROR(const char *msg, const char *val);

const char *pdbTemplateArg::toName(targ_t v) {
  switch (v) {
  case TA_TYPE    : return "type";
  case TA_NONTYPE : return "ntype";
  case TA_TEMPL   : return "templ";
  case TA_NA      :
  default         : return "NA";
  }
}

pdbTemplateArg::targ_t pdbTemplateArg::toTarg(const char* v) {
  if ( strcmp ("type",  v) == 0 ) return TA_TYPE;
  if ( strcmp ("ntype", v) == 0 ) return TA_NONTYPE;
  if ( strcmp ("templ", v) == 0 ) return TA_TEMPL;
  pdb_ERROR("Unknown template argument kind ", v);
  return TA_NA;
}

ostream& operator<<(ostream& ostr, const pdbTemplateArg& ta) {
  const pdbType* ty;
  const pdbGroup *gr;
  const pdbTemplate *te;

  ostr << pdbTemplateArg::toName(ta.kind());
  if ( ta.kind() == pdbTemplateArg::TA_TYPE ) {
    if ( ta.isSpecialization() ) {
      ty = ta.type();
      if ( (gr = ty->isGroup()) != 0 )
        ostr << " gr#" << gr->id();
      else
        ostr << " ty#" << ty->id();
    } else {
      ostr << " ty#" << ta.type()->id();
      if ( (ty = ta.defaultType()) )
        if ( (gr = ty->isGroup()) != 0 )
          ostr << " gr#" << gr->id();
        else
          ostr << " ty#" << ty->id();
    }
  } else if ( ta.kind() == pdbTemplateArg::TA_NONTYPE ) {
    if ( ta.isSpecialization() ) {
      ostr << " " << ta.value();
    } else {
      ostr << " ty#" << ta.type()->id() << " " << ta.name();
      if ( ta.defaultValue() != "" ) ostr << " " << ta.defaultValue();
    }
  } else if ( ta.kind() == pdbTemplateArg::TA_TEMPL ) {
    ostr << " te#" << ta.templateArg()->id();
    if ( (te = ta.defaultTemplateArg()) )
      ostr << " te#" << te->id();
  }
  return ostr;
}

void pdbTemplateArg::adjustPtrs(PDB* pdb) {
  if ( kind() == pdbTemplateArg::TA_TYPE ) {
    if ( type() && (type()->newId() > pdbSimpleItem::UNIQUE) )
      type(pdb->getTypeMap()[type()->newId()]);
    if ( defaultType() && (defaultType()->newId() > pdbSimpleItem::UNIQUE) )
      defaultType(pdb->getTypeMap()[defaultType()->newId()]);
  } else if ( kind() == pdbTemplateArg::TA_NONTYPE ) {
    if ( type() && (type()->newId() > pdbSimpleItem::UNIQUE) )
      type(pdb->getTypeMap()[type()->newId()]);
  } else if ( kind() == pdbTemplateArg::TA_TEMPL ) {
    if ( templateArg() && (templateArg()->newId() > pdbSimpleItem::UNIQUE) )
      templateArg(pdb->getTemplateMap()[templateArg()->newId()]);
    if ( defaultTemplateArg() &&
         (defaultTemplateArg()->newId() > pdbSimpleItem::UNIQUE) )
      defaultTemplateArg(pdb->getTemplateMap() [defaultTemplateArg()->newId()]);
  }
}
