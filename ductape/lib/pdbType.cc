/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbType.h"
#include "pdbClass.h"
#include "pdbModule.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbType.inl"
#endif

#if defined(__GNUC__) && !defined(__SPSL__)
#  include <limits.h>
  const int pdbBounds::UNKNOWN = INT_MAX;
#else
# include <limits>
  using std::numeric_limits;
  const int pdbBounds::UNKNOWN = numeric_limits<int>::max();
#endif

extern void pdb_ERROR(const char *msg, const char *val);

const char *pdbArg::toName(arg_t v) {
  switch (v) {
  case ARG_IN   : return "in";
  case ARG_OUT  : return "out";
  case ARG_INOUT: return "inout";
  case ARG_OPT  : return "opt";
  case ARG_DEF  : return "def";
  case ARG_NA   :
  default       : return "NA";
  }
}

pdbArg::arg_t pdbArg::toArg(const char* v) {
  if ( strcmp ("in",    v) == 0 ) return ARG_IN;
  if ( strcmp ("out",   v) == 0 ) return ARG_OUT;
  if ( strcmp ("inout", v) == 0 ) return ARG_INOUT;
  if ( strcmp ("opt",   v) == 0 ) return ARG_OPT;
  if ( strcmp ("def",   v) == 0 ) return ARG_DEF;
  pdb_ERROR("Unknown argument kind ", v);
  return ARG_NA;
}

const pdbType* pdbType::NULLEXCEP = (const pdbType*)1;

void pdbType::process(PDB *p) {
  // ??? name is fullName, because of bug in frontend
  // ??? to fix this bug, try to correct name
  // but only for enums and typedefs
  size_t pos;
  if ( kind()==TY_ENUM ||
      (kind()==TY_TREF && qualifiers().size()==0
                       && name()[name().size()-1]!='>') ) {
    if ( (pos = name().rfind("::")) != STR_NPOS ) {
      name(name().substr(pos+2, STR_NPOS));
    }
  }
  pdbItem::process(p);

  // is Const?
  const qualvec& q = qualifiers();
  if ( q.size() ) {
    for (qualvec::const_iterator it=q.begin(); it!=q.end(); ++it) {
      if ( (*it) == QL_CONST ) {
        isConst(true);
        break;
      }
    }
  }
}

static void printType(ostream& ostr, char *attr, const pdbType* ty,
                      const char *extra = "\n") {
  const pdbGroup *gr;

  if ( (gr = ty->isGroup()) != 0 )
    ostr << attr << "gr#" << gr->id() << extra;
  else
    ostr << attr << "ty#" << ty->id() << extra;
}

ostream& pdbType::print(ostream& ostr) const {
  qualvec q;
  boundvec b;

  pdbItem::print(ostr);
  ostr << "ykind " << toName(kind()) << "\n";
  switch ( kind() ) {
  case TY_INT:
  case TY_BOOL:
  case TY_WCHAR:
  case TY_FINT:
  case TY_FLOGIC:
    ostr << "yikind " << toName(integerType()) << "\n";
    // fall through
  case TY_FCHAR:
    if ( isSigned() ) ostr << "ysigned T" << "\n";
    if ( kind() == TY_FCHAR ) {
      int n = numElements();
      if ( n == -1 ) ostr << "yclen *\n";
      else if ( n > 0 ) ostr << "yclen " << n << "\n";
    }
    break;
  case TY_ENUM: {
    ostr << "yikind " << toName(integerType()) << "\n";
    for (enumvec::const_iterator eit=enums.begin(); eit!=enums.end(); ++eit)
      ostr << "yenum " << (*eit).id() << " " << (*eit).val() << "\n";
    }
    break;
  case TY_FLOAT:
  case TY_FFLOAT:
  case TY_FCMPLX:
    ostr << "yfkind " << toName(floatType()) << "\n";
    break;
  case TY_PTR:
  case TY_FPTR:
    printType(ostr, "yptr ", pointedToType());
    break;
  case TY_REF:
    printType(ostr, "yref ", referencedType());
    break;
  case TY_ARRAY:
    printType(ostr, "yelem ", elementType());
    ostr << "ynelem " << numElements() << "\n";
    if ( isStaticArray() ) ostr << "ystat T" << "\n";
    break;
  case TY_FARRAY:
    printType(ostr, "yelem ", elementType());
    ostr << "yshape " << toName(shape()) << "\n";
    ostr << "yrank " << rank() << "\n";
    b = bounds();
    if ( b.size() ) {
      int dim = 1;
      for (boundvec::iterator it=b.begin(); it!=b.end(); ++it, ++dim) {
        ostr << "ydim";
        if ( it->lower() == pdbBounds::UNKNOWN ) {
          ostr << " NA";
        } else {
          ostr << " " << it->lower();
        }
        if ( it->upper() == pdbBounds::UNKNOWN ) {
          if ( shape() == SH_ASIZE && dim == rank() ) ostr << " *";
          else ostr << " NA";
        } else {
          ostr << " " << it->upper();
        }
        ostr << "\n";
      }
    }
    break;
  case TY_TREF:
    printType(ostr, "ytref ", baseType());
    q = qualifiers();
    if ( q.size() ) {
      ostr << "yqual";
      for (qualvec::iterator it=q.begin(); it!=q.end(); ++it)
        ostr << " " << toName(*it);
      ostr << "\n";
    }
    break;
  case TY_FUNC:
  case TY_FFUNC:
    printType(ostr, "yrett ", returnType());
    for (argvec::const_iterator ait=args.begin(); ait!=args.end(); ++ait) {
      printType(ostr, "yargt ", ait->type(), "");
      ostr << " " << ait->name() << " ";
      ait->printLoc(ostr);
      if ( ait->intentIn()   ) {ostr << " " << pdbArg::toName(pdbArg::ARG_IN);}
      if ( ait->intentOut()  ) {ostr << " " << pdbArg::toName(pdbArg::ARG_OUT);}
      if ( ait->isOptional() ) {ostr << " " << pdbArg::toName(pdbArg::ARG_OPT);}
      if ( ait->hasDefault() ) {ostr << " " << pdbArg::toName(pdbArg::ARG_DEF);}
      ostr << '\n';
    }

    if ( kind() == TY_FUNC ) {
      if ( hasEllipsis() ) ostr << "yellip T\n";
      if ( oldStyleDecl() ) ostr << "yold T\n";
      if ( isConst() ) ostr << "yqual const\n";
      for (typevec::const_iterator xit=exSpec.begin(); xit!=exSpec.end(); ++xit)
        if ( (*xit) == NULLEXCEP )
          ostr << "yexcep NULL\n";
        else
          printType(ostr,  "yexcep ", *xit, "\n");
    }
    break;
  case TY_PTRMEM:
    ostr << "ympgroup gr#" << memberPointerClass()->id() << "\n";
    printType(ostr, "ymptype ", memberPointerType());
    break;
  default:
    // nothing
    break;
  }

  if(isShared()) {
    ostr << "yshared T\n";
    if(blockSize() >= 0) {
        ostr << "yblocksize " << blockSize() << "\n";
    }
    if(isRelaxed()) {
        ostr << "yrelaxed T\n";
    }
    if(isStrict()) {
        ostr << "ystrict T\n";
    }
  }

  return ostr << endl;
}

void pdbType::adjustPtrs(PDB* pdb) {
  const pdbFile* fi;

  pdbItem::adjustPtrs(pdb);
  if ( elemTy && (elemTy->newId() > UNIQUE) )
    elemTy = pdb->getTypeMap()[elemTy->newId()];
  if ( ptrTy && (ptrTy->newId() > UNIQUE) )
    ptrTy = pdb->getTypeMap()[ptrTy->newId()];
  if ( baseTy && (baseTy->newId() > UNIQUE) )
    baseTy = pdb->getTypeMap()[baseTy->newId()];
  if ( retTy && (retTy->newId() > UNIQUE) )
    retTy = pdb->getTypeMap()[retTy->newId()];
  if ( memPtrTy && (memPtrTy->newId() > UNIQUE) )
    memPtrTy = pdb->getTypeMap()[memPtrTy->newId()];
  if ( memPtrCl && (memPtrCl->newId() > UNIQUE) )
    memPtrCl = pdb->getClassMap()[memPtrCl->newId()];
  if ( groupTy && (groupTy->newId() > UNIQUE) ) {
    if ( pdb->language() & PDB::LA_C_or_CXX )
      groupTy = pdb->getClassMap()[groupTy->newId()];
    else if ( pdb->language() & PDB::LA_FORTRAN )
      groupTy = pdb->getModuleMap()[groupTy->newId()];
  }
  for (argvec::iterator ait=args.begin(); ait!=args.end(); ++ait) {
    if ( ait->type()->newId() > UNIQUE )
      ait->type(pdb->getTypeMap()[ait->type()->newId()]);
    if ( (fi=ait->file()) && (fi->newId() > UNIQUE) )
      ait->file(pdb->getFileMap()[fi->newId()]);
  }
  for (typevec::iterator xit=exSpec.begin(); xit!=exSpec.end(); ++xit) {
    if ( ((*xit) != NULLEXCEP) && ((*xit)->newId() > UNIQUE) )
      *xit = pdb->getTypeMap()[(*xit)->newId()];
  }
}

pdbSimpleItem::dupl_t pdbType::findDuplicate(pdbSimpleItem* t) {
  pdbType* rhs = dynamic_cast<pdbType*> (t);
  if ( isGroup() && rhs->isGroup() ) 
    // if both are group types, compare group names
    return ( isGroup()->fullName() == rhs->isGroup()->fullName() ?
             NEWDUPL : NODUPL );
  else if ( isGroup() || rhs->isGroup() )
    // if only one of them is group type, they are different
    return NODUPL;
  else
    // otherwise compare full names as usual
    return ( fullName() == rhs->fullName() ? NEWDUPL : NODUPL );
}
