/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include <ctype.h>
#include "pdbType.h"
#include "pdbCRoutine.h"
#include "pdbClass.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbCRoutine.inl"
#endif

void pdbCRoutine::process(PDB *p) {
  // routines without location or the same location as the parent class
  // are computer generated ???
  if ( !location().file() ||
       (parentGroup() && parentGroup()->location() == location()) ) {
    if ( !isCGen ) { 
      //cerr << "**** FOUND ANOTHER CGEN" << endl;
      //cerr << "ro#" << id() << " " << name() << " file=" << (location().file()==0) << endl;
      isCGen = true;
    }
  }

  // build up full name:
  // do it like for pdbRoutine, but add signature
  pdbRoutine::process(p);

  // check for signature types which are actually typedefs for function type
  const pdbType *currSig = 0;
  if ( signature()->kind() == TY_FUNC )
    currSig = signature();
  else if ( signature()->kind() == TY_TREF &&
            signature()->baseType()->kind() == TY_FUNC ) {
    currSig = signature()->baseType();
  }
  const pdbType *retType = currSig->returnType();;

  // Implementation A:
  // build it out of return type and argument types
  string other = retType->isGroup() ? retType->isGroup()->fullName()
                                    : retType->fullName();
  string leftover = "";
  if ( isalnum(other[other.size()-1]) || other[other.size()-1]=='>' )
    other += " ";
  if ( retType->kind() == TY_PTR &&
       retType->pointedToType()->kind() == TY_FUNC ) {
    // return type is function pointer
    const char *ret = other.c_str();
    const char *ptr = strstr(ret, "(*");
    int idx = (ptr - ret) + 2;
    int len = other.size() - 1;
    leftover = other.substr(idx, len-idx+1);
    other = other.substr(0,idx) + fullName() + "(";
  } else {
    other += fullName() + "(";
  }
  if ( !currSig->oldStyleDecl() ) {
    for (pdbType::argvec::const_iterator ait=currSig->arguments().begin();
         ait!=currSig->arguments().end(); ++ait) {
      if ( ait!=currSig->arguments().begin() ) other += ", ";
      other += ait->type()->isGroup() ?
               ait->type()->isGroup()->fullName() : ait->type()->fullName();
    }
    if ( currSig->hasEllipsis() ) other += ", ...";
    if ( currSig->arguments().size() == 0 && p->language() == PDB::LA_C )
      other += "void";
  }
  other += ")" + leftover;
  if ( currSig->isConst() ) other += " const";
  if ( linkage() == LK_C ) other += " C";

  // Implementation B:
  // use signature
  const string strsig = currSig->name();
  int idx = strsig.size() - 1;
  int len = idx;
  if ( retType->kind() == TY_PTR &&
       retType->pointedToType()->kind() == TY_FUNC ) {
    // return type is function pointer
    const char *sig = strsig.c_str();
    const char *ptr = strstr(sig, "(*");
    idx = (ptr - sig) + 2;
  } else {
    // normal function
    int paren = 0;
    while ( idx >= 0 ) {
      if ( strsig[idx] == ')' ) {
        ++paren;
      } else if ( strsig[idx] == '(' ) {
        --paren;
        if ( paren == 0 ) break;
      }
      --idx;
    }
  }
  if ( idx >= 0 ) {
    if ( linkage() == LK_C && strsig[len-1]!=' ' && strsig[len]!='C' ) {
      // in C mode, edg doesn't add linkage specifiers
      fullName(strsig.substr(0,idx) + fullName()
                                    + strsig.substr(idx, len-idx+1) + " C");
    } else
      fullName(strsig.substr(0,idx) + fullName()
                                    + strsig.substr(idx, len-idx+1));
  }

  // Compare A vs. B
  /* PDT: Suppress error. It is annoying.  
  if ( other != fullName() ) {
    cerr << "other *** '" << other << "'" << endl;
    cerr << "full  *** '" << fullName() << "'" << endl;
  }
  */

  // Calculate location of first executable statement out of statement info
  const pdbStmt* s = body();
  
  if ( s ) {
    if ( s->kind() == pdbStmt::ST_TRY ) {
      // special case: C++ function try block
      firstExecStmtLocation(s->stmtBegin());
      return;
    } else {
      s = s->downStmt();
    }
  } else {
    // no body available
    return;
  }

  // skip declarations
  while ( s && s->kind() == pdbStmt::ST_DECL ) s = s->nextStmt();

  // if empty body or implicit return statement, choose end-of-main-body
  if ( !s || ( s->kind() == pdbStmt::ST_RETURN && !s->stmtBegin().file() ) )
    firstExecStmtLocation(body()->stmtEnd());
  else
    firstExecStmtLocation(s->stmtBegin());

}

ostream& pdbCRoutine::print(ostream& ostr) const {
  pdbRoutine::print(ostr);
  ostr << "rvirt " << toName(virtuality()) << "\n";
  if ( covariantReturnVirtualOverride() ) ostr << "rcrvo T\n";
  if ( isInline() ) ostr << "rinline T\n";
  if ( isCompilerGenerated() ) ostr << "rcgen T\n";
  if ( isExplicitCtor() ) ostr << "rexpl T\n";
  if ( isStatic() ) ostr << "rstatic T\n";
  return ostr << endl;
}
