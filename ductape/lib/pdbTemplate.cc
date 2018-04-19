/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include <string.h>
#include "pdbTemplate.h"
#include "pdbType.h"
#include "pdbCRoutine.h"
#include "pdbClass.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbTemplate.inl"
#endif

ostream& pdbTemplate::print(ostream& ostr) const {
  const pdbType* ty;
  const pdbGroup *gr;
  const pdbTemplate *te;

  pdbItem::print(ostr);
  ostr << "tkind " << toName(kind()) << "\n";
  if ( ! (text() == "") ) ostr << "ttext " << text() << "\n";
  if ( (te = declaration()) ) ostr << "tdecl te#" << te->id() << "\n";
  if ( (te = definition()) ) ostr << "tdef te#" << te->id() << "\n";
  const targvec& a = arguments();
  for (targvec::const_iterator ait=a.begin(); ait!=a.end(); ++ait) {
    ostr << "tparam " << *ait << "\n";
  }
  const targvec& s = speclArguments();
  for (targvec::const_iterator it=s.begin(); it!=s.end(); ++it) {
    ostr << "tsparam " << *it << "\n";
  }
  if ( const pdbCRoutine* ro = funcProtoInst() ) {
    ostr << "tproto ro#" << ro->id() << "\n";
  } else if ( const pdbClass* cl = classProtoInst() ) {
    ostr << "tproto gr#" << cl->id() << "\n";
  } else if ( (ty = statMemType()) ) {
    if ( (gr = ty->isGroup()) != 0 )
      ostr << "ttype gr#" << gr->id() << "\n";
    else
      ostr << "ttype ty#" << ty->id() << "\n";
  }
  pdbFatItem::print(ostr);
  return ostr << endl;
}

void pdbTemplate::process(PDB *p) {
  pdbItem::process(p);

  // add template param /*???*/
  const string& strtext = text();
  if ( ! (strtext == "") ) {
    int start = 9;
    int end = start+1;
    int cnt = 1;
    while ( strtext[end] ) {
      if ( strtext[end] == '<' )
        ++cnt;
      else if ( strtext[end] == '>' ) {
        --cnt;
        if ( cnt == 0 ) break;
      }
      ++end;
    }
    string targ = text().substr(start, end-start+1);

    // remove possible line breaks
    bool blank = false;
    unsigned i, j = 0;
    for (i=0; i<targ.size(); ++i) {
      if ( targ[i] == '\\' ) {
        targ[j++] = ' ';
        i += 3;
        blank = true;
      } else if ( targ[i] == ' ' ) {
        if (!blank) targ[j++] = ' ';
        blank = true;
      } else {
        targ[j++] = targ[i];
        blank = false;
      }
    }
    if ( j < i) targ.resize(j);
    fullName(fullName() + targ);
  }

  // set template argument names
  targvec& a = args;
  for (targvec::iterator it=a.begin(); it!=a.end(); ++it) {
    if ( it->kind() == pdbTemplateArg::TA_TYPE )
      it->name(it->type()->name());
    else if ( it->kind() == pdbTemplateArg::TA_TEMPL )
      it->name(it->templateArg()->name());
  }
}

pdbSimpleItem::dupl_t pdbTemplate::findDuplicate(pdbSimpleItem* r) {
  pdbTemplate* rhs = dynamic_cast<pdbTemplate*> (r);
  // ??? for function templates, should actually compare "signatures"
  // ??? but this information is not available yet
  if ( fullName() == rhs->fullName() ) {
    pdbLoc ll = location();
    pdbLoc rl = rhs->location();
    if ( (ll.file() && rl.file()) && (ll.file()->name() == rl.file()->name()) &&
         (ll.line() == rl.line()) && (ll.col() == rl.col()) ) {
      return NEWDUPL;
    } else
      return NODUPL;
  } else {
    return NODUPL;
  }
}

void pdbTemplate::adjustPtrs(PDB* pdb) {
  pdbFatItem::adjustPtrs(pdb);
  targvec& a = args;
  for (targvec::iterator it=a.begin(); it!=a.end(); ++it) it->adjustPtrs(pdb);
  targvec& s = sargs;
  for (targvec::iterator st=s.begin(); st!=s.end(); ++st) st->adjustPtrs(pdb);
  if ( funcProtoInst() && (funcProtoInst()->newId() > UNIQUE) )
    funcProtoInst(pdb->getCRoutineMap()[funcProtoInst()->newId()]);
  else if ( classProtoInst() && (classProtoInst()->newId() > UNIQUE) )
    classProtoInst(pdb->getClassMap()[classProtoInst()->newId()]);
  else if ( statMemType() && (statMemType()->newId() > UNIQUE) )
    statMemType(pdb->getTypeMap()[statMemType()->newId()]);
  if ( definition() && (definition()->newId() > UNIQUE) )
    definition(pdb->getTemplateMap()[definition()->newId()]);
  if ( declaration() && (declaration()->newId() > UNIQUE) )
    declaration(pdb->getTemplateMap()[declaration()->newId()]);
}
