/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifdef _OLD_HEADER_
# include <fstream.h>
#else
# include <fstream>
  using std::ofstream;
#endif
#ifdef _MSC_VER
 #include "getopt.h"
#else
 #include <unistd.h>
#endif
#include <stdlib.h>
#include "pdbAll.h"

static bool Aflag = false;
static bool Gflag = false;
static bool Mflag = false;
static bool Nflag = false;
static bool Pflag = false;
static bool Rflag = false;
static bool Sflag = false;
static bool Tflag = false;
static bool Yflag = false;
static bool checkonly = false;

static char toUpper(const char ch) { return (ch - 'a' + 'A'); }

static const char *toName(pdbItem::access_t v) {
  switch (v) {
  case pdbItem::AC_PUB : return "public";
  case pdbItem::AC_PRIV: return "private";
  case pdbItem::AC_PROT: return "protected";
  case pdbItem::AC_NA  :
  default              : return "NA";
  }
}

static const char *toName(pdbItem::routine_t v) {
  switch (v) {
  case pdbItem::RO_ASM    : return "c/c++Assembler";
  case pdbItem::RO_AUTO   : return "c/c++Automatic";
  case pdbItem::RO_EXT    : return "c/c++Extern";
  case pdbItem::RO_STAT   : return "c/c++Static";
  case pdbItem::RO_TPROTO : return "c/c++PrototypeTemplateInstantiation";

  case pdbItem::RO_FEXT   : return "fortranExtern";
  case pdbItem::RO_FPROG  : return "fortranProgram";
  case pdbItem::RO_FBLDAT : return "fortranBlockData";
  case pdbItem::RO_FINTRIN: return "fortranIntrinsic";
  case pdbItem::RO_FINT   : return "fortranInternal";
  case pdbItem::RO_FSTFN  : return "fortranStmtFunction";
  case pdbItem::RO_FMPROC : return "fortranModuleProcedure";
  case pdbItem::RO_FUNSPEC: return "fortranUnspecified";
  case pdbItem::RO_FALIAS : return "fortranAlias";

  case pdbItem::RO_NA  :
  default              : return "NA";
  }
}

static const char *toName(pdbItem::rspec_t v) {
  switch (v) {
  case pdbItem::RS_CONV: return "conversionOperator";
  case pdbItem::RS_CTOR: return "constructor";
  case pdbItem::RS_DTOR: return "destructor";
  case pdbItem::RS_OP  : return "operator";
  case pdbItem::RS_NA  :
  default              : return "NA";
  }
}

static const char *toName(pdbItem::virt_t v) {
  switch (v) {
  case pdbItem::VI_PURE: return "pureVirtual";
  case pdbItem::VI_VIRT: return "virtual";
  case pdbItem::VI_NO  :
  default              : return "no";
  }
}

static const char *toName(pdbItem::macro_t v) {
  switch (v) {
  case pdbItem::MA_DEF  : return "define";
  case pdbItem::MA_UNDEF: return "undef";
  case pdbItem::MA_NA   :
  default               : return "NA";
  }
}

static const char *toName(pdbItem::templ_t v) {
  switch (v) {
  case pdbItem::TE_CLASS   : return "class";
  case pdbItem::TE_FUNC    : return "function";
  case pdbItem::TE_MEMCLASS: return "memberClass";
  case pdbItem::TE_MEMFUNC : return "memberFunction";
  case pdbItem::TE_STATMEM : return "staticDataMember";
  case pdbItem::TE_TPARAM  : return "templateParameter";
  case pdbItem::TE_NA      :
  default                  : return "NA";
  }
}

static const char *toName(pdbItem::float_t v) {
  switch (v) {
  case pdbItem::FL_FLOAT  : return "float";
  case pdbItem::FL_DBL    : return "double";
  case pdbItem::FL_LONGDBL: return "longDouble";
  case pdbItem::FL_NA     :
  default                 : return "NA";
  }
}

static const char *toName(pdbItem::int_t v) {
  switch (v) {
  case pdbItem::I_CHAR     : return "character";
  case pdbItem::I_SCHAR    : return "signedCharacter";
  case pdbItem::I_UCHAR    : return "unsignedCharacter";
  case pdbItem::I_SHORT    : return "short";
  case pdbItem::I_USHORT   : return "unsignedShort";
  case pdbItem::I_INT      : return "integer";
  case pdbItem::I_UINT     : return "unsignedInteger";
  case pdbItem::I_LONG     : return "long";
  case pdbItem::I_ULONG    : return "uunsignedLong";
  case pdbItem::I_LONGLONG : return "longLong";
  case pdbItem::I_ULONGLONG: return "unsignedLongLong";
  case pdbItem::I_WCHAR    : return "wideCharacter";
  case pdbItem::I_NA       :
  default                  : return "NA";
  }
}

static const char *toName(pdbItem::type_t v) {
  switch (v) {
  case pdbItem::TY_BOOL  : return "c/c++Boolean";
  case pdbItem::TY_ENUM  : return "c/c++Enumeration";
  case pdbItem::TY_ERR   : return "c/c++Error";
  case pdbItem::TY_FUNC  : return "c/c++FunctionSignature";
  case pdbItem::TY_VOID  : return "c/c++Void";
  case pdbItem::TY_INT   : return "c/c++Integer";
  case pdbItem::TY_FLOAT : return "c/c++Float";
  case pdbItem::TY_PTR   : return "c/c++Pointer";
  case pdbItem::TY_REF   : return "c/c++Reference";
  case pdbItem::TY_ARRAY : return "c/c++Array";
  case pdbItem::TY_TREF  : return "c/c++TypeReference";
  case pdbItem::TY_PTRMEM: return "c/c++PointerToMember";
  case pdbItem::TY_TPARAM: return "c/c++TemplateParameter";
  case pdbItem::TY_WCHAR : return "c/c++WideCharacter";

  case pdbItem::TY_FERR  : return "fortranError";
  case pdbItem::TY_FVOID : return "fortranVoid";
  case pdbItem::TY_FINT  : return "fortranInteger";
  case pdbItem::TY_FLOGIC: return "fortranLogical";
  case pdbItem::TY_FFLOAT: return "fortranFloat";
  case pdbItem::TY_FFUNC : return "fortranFunctionSignature";
  case pdbItem::TY_FCHAR : return "fortranCharacter";
  case pdbItem::TY_FARRAY: return "fortranArray";
  case pdbItem::TY_FCMPLX: return "fortranComplex";
  case pdbItem::TY_FUNSPECFUNC: return "fortranUnspecifiedFunc";
  case pdbItem::TY_FBLDAT: return "fortranBlockData";
  case pdbItem::TY_FMOD  : return "fortranModule";
  case pdbItem::TY_FPTR  : return "fortranPointer";

  case pdbItem::TY_GROUP : return "groupType";

  case pdbItem::TY_NA    :
  default                : return "NA";
  }
}

static const char *toName(pdbItem::qual_t v) {
  switch (v) {
  case pdbItem::QL_CONST   : return "const";
  case pdbItem::QL_VOLATILE: return "volatile";
  case pdbItem::QL_RESTRICT: return "restrict";
  default                  : return "NA";
  }
}

static const char *toName(pdbItem::group_t v) {
  switch (v) {
  case pdbItem::GR_CLASS   : return "c/c++Class";
  case pdbItem::GR_STRUCT  : return "c/c++Struct";
  case pdbItem::GR_UNION   : return "c/c++Union";
  case pdbItem::GR_TPROTO  : return "c/c++PrototypeTemplateInstantiation";
  case pdbItem::GR_FDERIVED: return "fortranDerivedType";
  case pdbItem::GR_FMODULE : return "fortranModule";
  case pdbItem::GR_NA      : 
  default                  : return "NA";
  }
}

static const char *toName(pdbItem::link_t v) {
  switch (v) {
  case pdbItem::LK_INTERNAL: return "cInternal";
  case pdbItem::LK_CXX     : return "C++";
  case pdbItem::LK_C       : return "C";
  case pdbItem::LK_FINT    : return "fortranInternal";
  case pdbItem::LK_F90     : return "fortran90";
  case pdbItem::LK_NA      :
  default                  : return "no";
  }
}

static const char *toName(pdbItem::mem_t v) {
  switch (v) {
  case pdbItem::M_VAR    : return "variable";
  case pdbItem::M_STATVAR: return "staticVariable";
  case pdbItem::M_TYPE   : return "type";
  case pdbItem::M_TEMPL  : return "template";
  case pdbItem::M_NA     :
  default                : return "NA";
  }
}

static const char *toName(pdbItem::shape_t v) {
  switch (v) {
  case pdbItem::SH_EXPLICIT : return "explicit";
  case pdbItem::SH_ASIZE    : return "assumedSize";
  case pdbItem::SH_ASHAPE   : return "assumedShape";
  case pdbItem::SH_DEFERRED : return "deferred";
  case pdbItem::SH_NA       :
  default                   : return "NA";
  }
}

static const char *toName(pdbArg::arg_t v) {
  switch (v) {
  case pdbArg::ARG_IN   : return "intentIn";
  case pdbArg::ARG_OUT  : return "intentOut";
  case pdbArg::ARG_INOUT: return "intentInOut";
  case pdbArg::ARG_OPT  : return "optional";
  case pdbArg::ARG_DEF  : return "default";
  case pdbArg::ARG_NA   :
  default               : return "NA";
  }
}

static const char *toName(pdbTemplateArg::targ_t v) {
  switch (v) {
  case pdbTemplateArg::TA_TYPE    : return "typename";
  case pdbTemplateArg::TA_NONTYPE : return "nontype ";
  case pdbTemplateArg::TA_TEMPL   : return "template";
  case pdbTemplateArg::TA_NA      :
  default                         : return "NA";
  }
}

static void printType(ostream& ostr, const char *attr, const pdbType *ty,
                      const char *extra = "\n") {
  const pdbGroup *gr;

  if ( (gr = ty->isGroup()) != 0 )
    ostr << attr << "GR#" << gr->id() << " " << gr->name() << extra;
  else
    ostr << attr << "TY#" << ty->id() << " " << ty->name() << extra;
}

void printPos(ostream& ostr, const pdbLoc& l) {
  if ( l.file() ) {
    ostr << " SO#"
         << l.file()->id() << " " << l.file()->name()
         << " (" << l.line() << "," << l.col() << ")";
  } else {
    ostr << " [UNKNOWN]";
  }
}

void printRange(ostream& ostr,const pdbLoc& l1, const pdbLoc& l2) { 
  printPos(ostr, l1);
  if ( l2.file() && l1.file() == l2.file() )
    ostr << " -> (" << l2.line() << "," << l2.col() << ")";
  else
    printPos(ostr, l2);
}

void printPositions(ostream& ostr, const pdbFatItem* fi) {
  ostr << "headPosition:      ";
  printRange(ostr, fi->headBegin(), fi->headEnd());
  ostr << "\n";
  ostr << "bodyPosition:      ";
  printRange(ostr, fi->bodyBegin(), fi->bodyEnd());
  ostr << "\n";
}

void printStmt(ostream& ostr, const pdbStmt& s) {
  const pdbStmt* s1;

  ostr << "ST#" << s.id() << " " << pdbStmt::toName(s.kind());
  printRange(ostr, s.stmtBegin(), s.stmtEnd());
  if ((s1 = s.nextStmt()) != NULL)
    ostr << " ST#" << s1->id();
  else
    ostr << " NA";
  if ((s1 = s.downStmt()) != NULL)
    ostr << " ST#" << s1->id();
  else
    ostr << " NA";
  if ((s1 = s.extraStmt()) != NULL)
    ostr << " ST#" << s1->id();
}

void printLo(ostream& ostr, const pdbLoc& l) {
  ostr << "location:          ";
  printPos(ostr, l);
  ostr << "\n";
}

void printComment(ostream& ostr, const pdbComment& c) {
  ostr << "CO#" << c.id() << " " << PDB::toName(c.kind());
  printRange(ostr, c.cmtBegin(), c.cmtEnd());
  ostr << " " << c.text();

}

void printSo(ostream& ostr, const pdbFile *f) {
  ostr << "SO#" << f->id() << " " << f->name() << "\n";
  if ( f->isSystemFile() )
    ostr << "isSystemInclude:    TRUE\n";
  for (pdbFile::incvec::const_iterator it=f->includes().begin();
       it!=f->includes().end(); ++it)
    ostr << "includes:           SO#"
         << (*it)->id() << " " << (*it)->name() << "\n";
  for (pdbFile::cmtvec::const_iterator ct=f->comments().begin();
       ct!=f->comments().end(); ++ct) {
    ostr << "comment:            ";
    printComment(ostr, **ct);
    ostr << "\n";
  }
  ostr << endl;
}

void printItem(ostream& ostr, const pdbItem *i) {
  ostr << toUpper(i->desc()[0]) << toUpper(i->desc()[1]) << "#" << i->id();
  if ( i->name() == "$NA$" )
    ostr << " [NONAME]\n";
  else
    ostr << " " << i->fullName() << "\n";
  printLo(ostr, i->location());
  if ( const pdbGroup* gptr = i->parentGroup() ) {
    ostr << "group:              GR#"
         << gptr->id() << " " << gptr->name() << "\n";
    ostr << "access:             " << ::toName(i->access()) << "\n";
  }
  if ( const pdbNamespace* nptr = i->parentNSpace() ) {
    ostr << "namespace:          NA#"
         << nptr->id() << " " << nptr->name() << "\n";
  }
}

void printTeArg(ostream& ostr, const pdbTemplateArg& ta) {
  const pdbType* ty;
  const pdbTemplate *te;

  ostr << ::toName(ta.kind());
  if ( ta.kind() == pdbTemplateArg::TA_TYPE ) {
    if ( ta.isSpecialization() ) {
      printType(ostr, " ", ta.type(), "");
    } else {
      ostr << " TY#" << ta.type()->id() << " " << ta.type()->name();
      if ( (ty = ta.defaultType()) ) printType(ostr, " ", ty, "");
    }
  } else if ( ta.kind() == pdbTemplateArg::TA_NONTYPE ) {
    if ( ta.isSpecialization() ) {
      ostr << " " << ta.value();
    } else {
      ostr << " TY#" << ta.type()->id() << " " << ta.type()->name()
           << " " << ta.name();
      if ( ta.defaultValue() != "" ) ostr << " " << ta.defaultValue();
    }
  } else if ( ta.kind() == pdbTemplateArg::TA_TEMPL ) {
    ostr << " TE#" << ta.templateArg()->id() << " "
         << ta.templateArg()->name();
    if ( (te = ta.defaultTemplateArg()) )
      ostr << " TE#" << te->id() << " " << te->name();
  }
  ostr << "\n";
}

void printTemplItem(ostream& ostr, const pdbTemplateItem *ti, const char *pre) {
  if ( const pdbTemplate* tptr = ti->isTemplate() ) {
    if ( tptr == pdbTemplateItem::TE_UNKNOWN )
      ostr << pre << "isTemplate:         TRUE\n";
    else
      ostr << pre << "instantiatedFrom:   TE#"
           << tptr->id() << " " << tptr->name() << "\n";
    if ( ti->isSpecialized() ) ostr << pre << "isSpecialized:      TRUE\n";
    const pdbTemplateItem::targvec& s = ti->speclArguments();
    for (pdbTemplateItem::targvec::const_iterator it=s.begin();
                                                 it!=s.end(); ++it) {
      ostr << "specializationParameter: ";
      printTeArg(ostr, *it);
    }
  }
}

void printTy(ostream& ostr, const pdbType *t) {
  if ( t->isGroup() ) return;

  pdbType::qualvec q;
  pdbType::boundvec b;

  printItem(ostr, t);
  ostr << "kind:               " << ::toName(t->kind()) << "\n";
  switch ( t->kind() ) {
  case pdbItem::TY_INT:
  case pdbItem::TY_BOOL:
  case pdbItem::TY_WCHAR:
  case pdbItem::TY_FINT:
  case pdbItem::TY_FLOGIC:
  case pdbItem::TY_FCHAR:
    ostr << "integerKind:        " << ::toName(t->integerType()) << "\n";
    if ( t->isSigned() ) ostr << "explicitlySigned:   TRUE" << "\n";
    if ( t->kind() == pdbItem::TY_FCHAR ) {
      int n = t->numElements();
      if ( n == -1 ) 
        ostr << "characterLength:    *\n";
      else if ( n > 0 )
        ostr << "characterLength:    " << n << "\n";
    }
    break;
  case pdbItem::TY_ENUM: {
    ostr << "integerKind:        " << ::toName(t->integerType()) << "\n";
    for (pdbType::enumvec::const_iterator eit=t->enumeration().begin();
         eit!=t->enumeration().end(); ++eit)
      ostr << "enumerationValue:   " << (*eit).id()
           << "=" << (*eit).val() << "\n";
    }
    break;
  case pdbItem::TY_FLOAT:
  case pdbItem::TY_FFLOAT:
  case pdbItem::TY_FCMPLX:
    ostr << "floatKind:          " << ::toName(t->floatType()) << "\n";
    break;
  case pdbItem::TY_PTR:
  case pdbItem::TY_FPTR:
    printType(ostr, "pointedToType:      ", t->pointedToType());
    break;
  case pdbItem::TY_REF:
    printType(ostr, "referencedType:     ", t->referencedType());
    break;
  case pdbItem::TY_ARRAY:
    printType(ostr, "elementType:        ", t->elementType());
    ostr << "numElements:       " << t->numElements() << "\n";
    if ( t->isStaticArray() ) ostr << "isStaticC99Array:   TRUE" << "\n";
    break;
  case pdbItem::TY_FARRAY:
    printType(ostr, "elementType:        ", t->elementType());
    ostr << "shape:              " << ::toName(t->shape()) << "\n";
    ostr << "rank:               " << t->rank() << "\n";
    b = t->bounds();
    if ( b.size() ) {
      int dim = 1;
      for (pdbType::boundvec::iterator it=b.begin(); it!=b.end(); ++it, ++dim) {
        ostr << "dimension:         ";
        if ( it->lower() == pdbBounds::UNKNOWN ) {
          ostr << " NA";
        } else {
          ostr << " " << it->lower();
        }
        if ( it->upper() == pdbBounds::UNKNOWN ) {
          if ( t->shape() == pdbItem::SH_ASIZE &&
               dim == t->rank() ) ostr << " *";
          else ostr << " NA";
        } else {
          ostr << " " << it->upper();
        }
        ostr << "\n";
      }
    }
    break;
  case pdbItem::TY_TREF:
    printType(ostr, "baseType:           ", t->baseType());
    q = t->qualifiers();
    if ( q.size() ) {
      ostr << "qualifiers:        ";
      for (pdbType::qualvec::iterator it=q.begin(); it!=q.end(); ++it)
        ostr << " " << ::toName(*it);
      ostr << "\n";
    }
    break;
  case pdbItem::TY_FUNC:
  case pdbItem::TY_FFUNC:
    printType(ostr, "returnType:         ", t->returnType());
    for (pdbType::argvec::const_iterator ait=t->arguments().begin();
         ait!=t->arguments().end(); ++ait) {
      printType(ostr, "argument:           ", ait->type(), "");
      ostr << " " << ait->name();
      printPos(ostr, *ait);
      if ( ait->intentIn()   ) {ostr << " " << ::toName(pdbArg::ARG_IN);}
      if ( ait->intentOut()  ) {ostr << " " << ::toName(pdbArg::ARG_OUT);}
      if ( ait->isOptional() ) {ostr << " " << ::toName(pdbArg::ARG_OPT);}
      if ( ait->hasDefault() ) {ostr << " " << ::toName(pdbArg::ARG_DEF);}
      ostr << '\n';
    }
    if ( t->kind() == pdbItem::TY_FUNC ) {
      if ( t->hasEllipsis() ) ostr << "hasEllipsis:        TRUE" << "\n";
      if ( t->oldStyleDecl() ) ostr << "oldStyleDecl:       TRUE" << "\n";
      if ( t->isConst() ) ostr << "qualifiers:        const\n";
      for (pdbType::typevec::const_iterator xit=t->exceptionSpec().begin();
           xit!=t->exceptionSpec().end(); ++xit) {
        if ( (*xit) == pdbType::NULLEXCEP )
          ostr << "exceptionSpec:      NULL\n";
        else
          printType(ostr, "exceptionSpec:      ", *xit, "\n");
      }
    }
    break;
  case pdbItem::TY_PTRMEM:  
    ostr << "memberPointerClass: GR#" << t->memberPointerClass()->id()
         << " " << t->memberPointerClass()->name() << "\n";
    printType(ostr, "memberPointerType:  ", t->memberPointerType());
    break;
  default:
    // nothing
    break;
  }
  ostr << endl;
}

void printGf(ostream& ostr, const pdbGroupField *g) {
  ostr << "dataMember:         " << g->name() << "\n";
  ostr << "  "; printLo(ostr, g->location());
  if ( const pdbGroup* gptr = g->parentGroup() ) {
    if ( gptr->kind() == pdbItem::GR_FMODULE && !g->location().file() )
      ostr << "  group:              GR#"
           << gptr->id() << " " << gptr->name() << "\n";
    ostr << "  access:             " << ::toName(g->access()) << "\n";
  }
  ostr << "  kind:               " << ::toName(g->kind()) << "\n";
  if ( g->type() ) printType(ostr, "  type:               ", g->type());
  printTemplItem(ostr, g, "  ");
  if ( g->isStaticConst() ) ostr << "  isStaticConst:      TRUE\n";
  if ( g->isBitField() ) ostr << "  isBitfield:         TRUE\n";
  if ( g->isMutable() ) ostr << "  isMutable:          TRUE\n";
}

void printGroup(ostream& ostr, const pdbGroup *g) {
  printItem(ostr, g);
  ostr << "kind                " << ::toName(g->kind()) << "\n";
  printTemplItem(ostr, g, "");
  const pdbGroup::fieldvec& d = g->dataMembers();
  for (pdbGroup::fieldvec::const_iterator dt=d.begin(); dt!=d.end(); ++dt)
     printGf(ostr, *dt);
  printPositions(ostr, g);
}

void printClass(ostream& ostr, const pdbClass *c) {
  printGroup(ostr, c);
  if ( c->isABC() ) ostr << "isAbstractBaseClass:  TRUE\n";
  const pdbClass::basevec& b = c->baseClasses();
  for (pdbClass::basevec::const_iterator bt=b.begin(); bt!=b.end(); ++bt) {
     ostr << "baseClass:          "
          << ::toName((*bt)->virtuality())
          << " " << ::toName((*bt)->access())
          << " GR#" << (*bt)->base()->id() << " " << (*bt)->base()->name()
          << " ";
     printPos(ostr, **bt);
     ostr << "\n";
  }
  const pdbClass::classvec& s = c->derivedClasses();
  for (pdbClass::classvec::const_iterator st=s.begin(); st!=s.end(); ++st)
     ostr << "derivedClass:       GR#"
          << (*st)->id() << " "  << (*st)->name() << "\n";
  const pdbClass::friendclassvec& fc = c->friendClasses();
  for (pdbClass::friendclassvec::const_iterator fct=fc.begin();
       fct!=fc.end(); ++fct) {
     ostr << "friendClass:        GR#" << (*fct)->friendClass()->id()
          << " " << (*fct)->friendClass()->name() << " ";
     printPos(ostr, **fct);
     ostr << "\n";
  }
  const pdbClass::friendfuncvec& fr = c->friendRoutines();
  for (pdbClass::friendfuncvec::const_iterator frt=fr.begin();
       frt!=fr.end(); ++frt) {
     ostr << "friendRoutine:      RO#" << (*frt)->friendFunc()->id()
          << " " << (*frt)->friendFunc()->name() << " ";
     printPos(ostr, **frt);
     ostr << "\n";
  }
  const pdbClass::methodvec& f = c->methods();
  for (pdbClass::methodvec::const_iterator ft=f.begin(); ft!=f.end(); ++ft) {
     ostr << "method:             RO#"
          << (*ft)->func()->id() << " " << (*ft)->func()->name() << " ";
     printPos(ostr, **ft);
     ostr << "\n";
  }
  ostr << endl;
}

void printModule(ostream& ostr, const pdbModule *m) {
  printGroup(ostr, m);
  const pdbModule::modfuncvec& f = m->funcMembers();
  for (pdbModule::modfuncvec::const_iterator ft=f.begin(); ft!=f.end(); ++ft) {
     ostr << "moduleFunc:         RO#"
          << (*ft)->func()->id() << " " << (*ft)->func()->name() << " ";
     printPos(ostr, **ft);
     ostr << "\n";
  }
  ostr << endl;
}

void printRo(ostream& ostr, const pdbRoutine *r) {
  pdbItem::rspec_t s;
  pdbItem::routine_t k;
  pdbItem::link_t l;
  const pdbStmt* st;

  printItem(ostr, r);
  if ( r->signature() )
    ostr << "signature:          TY#" << r->signature()->id() << " "
                          << r->signature()->name() << "\n";
  if ( (l = r->linkage()) != pdbItem::LK_NA )
    ostr << "linkage:            " << ::toName(l) << "\n";
  if ( (k = r->kind()) != pdbItem::RO_NA )
    ostr << "kind:               " << ::toName(k) << "\n";
  if ( (s = r->specialKind()) != pdbItem::RS_NA )
    ostr << "specialKind:        " << ::toName(s) << "\n";
  printTemplItem(ostr, r, "");
  const pdbRoutine::callvec& c = r->callees();
  for (pdbRoutine::callvec::const_iterator ct=c.begin(); ct!=c.end(); ++ct) {
    const pdbCallee *func = *ct;
    ostr << "calls:              RO#" << func->call()->id() << " "
         << func->call()->name() << " " << ::toName(func->virtuality()) << " ";
    printPos(ostr, *func);
    ostr << "\n";
  }
  printPositions(ostr, r);
  if ( (st = r->body()) )
    ostr << "routineBody:        ST#" << st->id() << "\n";
  const pdbRoutine::stmtvec& stmts = r->statements();
  for (int i=0; i<stmts.size(); ++i) {
    if ( stmts[i] ) {
      ostr << "bodyStatement:      ";
      printStmt(ostr, *(stmts[i]));
      ostr << "\n";
    }
  }
  if ( r->firstExecStmtLocation().file() ) {
    ostr << "firstExecStmt:     ";
    printPos(ostr, r->firstExecStmtLocation());
    ostr << "\n";
  }
  const pdbRoutine::locvec& lr = r->returnLocations();
  for (pdbRoutine::locvec::const_iterator lt=lr.begin(); lt!=lr.end(); ++lt) {
    ostr << "returnStmt:        ";
    printPos(ostr, **lt);
    ostr << "\n";
  }
}

void printCRo(ostream& ostr, const pdbCRoutine *r) {
  pdbItem::virt_t v;

  printRo(ostr, r);
  if ( (v = r->virtuality()) != pdbItem::VI_NO )
    ostr << "virtuality:         " << ::toName(v) << "\n";
  if ( r->covariantReturnVirtualOverride() )
    ostr << "covariantReturnVirtualOverride:  TRUE\n";
  if ( r->isCompilerGenerated() ) ostr << "isCompilerGenerated:  TRUE\n";
  if ( r->isExplicitCtor() ) ostr << "isExplicitCtor:     TRUE\n";
  if ( r->isInline() ) ostr << "isInline:           TRUE\n";
  if ( r->isStatic() ) ostr << "isStatic:           TRUE\n";
  ostr << endl;
}

void printFRo(ostream& ostr, const pdbFRoutine *r) {
  printRo(ostr, r);
  const pdbRoutine::locvec& lr = r->stopLocations();
  for (pdbRoutine::locvec::const_iterator lt=lr.begin(); lt!=lr.end(); ++lt) {
    ostr << "stopStmt:           ";
    printPos(ostr, **lt);
    ostr << "\n";
  }
  if ( r->parentRoutine() ) {
    ostr << "parentRoutine:      RO#" << r->parentRoutine()->id() << " "
         << r->parentRoutine()->name() << "\n";
  }
  if ( r->isAliasFor() ) {
    ostr << "aliasFor:           RO#" << r->isAliasFor()->id() << " "
         << r->isAliasFor()->name() << "\n";
  }
  const pdbFRoutine::implvec& im = r->implementedBy();
  for (pdbFRoutine::implvec::const_iterator it=im.begin(); it!=im.end(); ++it)
    ostr << "implementedBy:      RO#" << (*it)->id() << " "
         << (*it)->name() << "\n";
  if ( r->hasExplicitInterface() )
    ostr << "explicitInterface:  TRUE" << "\n";
  if ( r->isRecursive() ) ostr << "isRecursive:        TRUE" << "\n";
  if ( r->isElemental() ) ostr << "isElemental:        TRUE" << "\n";
  ostr << endl;
}

void printTe(ostream& ostr, const pdbTemplate *t) {
  const pdbType* ty;
  const pdbTemplate *te;

  printItem(ostr, t);
  ostr << "kind:               " << ::toName(t->kind()) << "\n";
  if ( !(t->text() == "") ) ostr << "text:               " << t->text() << "\n";
  if ( (te = t->declaration()) )
    ostr << "declaration:        TE#" << te->id() << " " << te->name() << "\n";
  if ( (te = t->definition()) )
    ostr << "definition:         TE#" << te->id() << " " << te->name() << "\n";

  const pdbTemplate::targvec& a = t->arguments();
  for (pdbTemplate::targvec::const_iterator it=a.begin(); it!=a.end(); ++it) {
    ostr << "parameter:          ";
    printTeArg(ostr, *it);
  }
  const pdbTemplate::targvec& s = t->speclArguments();
  for (pdbTemplate::targvec::const_iterator st=s.begin(); st!=s.end(); ++st) {
    ostr << "specializationParameter: ";
    printTeArg(ostr, *st);
  }

  if ( const pdbCRoutine* ro = t->funcProtoInst() ) {
    ostr << "prototypeInstance:  RO#" << ro->id() << " " << ro->name() << "\n";
  } else if ( const pdbClass* cl = t->classProtoInst() ) {
    ostr << "prototypeInstance:  GR#" << cl->id() << " " << cl->name() << "\n";
  } else if ( (ty = t->statMemType()) ) {
    printType(ostr, "ttype ", ty);
  }

  printPositions(ostr, t);
  ostr << endl;
}

void printNa(ostream& ostr, const pdbNamespace *n) {
  printItem(ostr, n);
  if ( n->isAlias() )
    ostr << "aliasFor:           NA#"
         << n->isAlias()->id() << " " << n->isAlias()->name() << "\n";
  for (pdbNamespace::memvec::const_iterator it = n->members().begin();
      it!=n->members().end(); ++it) {
    ostr << "member:             "
         << toupper((*it)->desc()[0]) << toupper((*it)->desc()[1]) << "#"
         << (*it)->id() << " " << (*it)->name() << "\n";
  }
  printPositions(ostr, n);
  ostr << endl;
}

void printMa(ostream& ostr, const pdbMacro *m) {
  printItem(ostr, m);
  ostr << "kind:               " << ::toName(m->kind()) << "\n";
  ostr << "text:               " << m->text() << "\n";
  ostr << endl;
}

void printPr(ostream& ostr, const pdbPragma *p) {
  printItem(ostr, p);
  ostr << "kind:               " << p->kind() << "\n";
  ostr << "position:          ";
  printRange(ostr, p->prBegin(), p->prEnd());
  ostr << "\n";
  ostr << "text:               " << p->text() << "\n";
  ostr << endl;
}

void printPDB(PDB& pdb, ostream& ostr) {
  if ( Aflag || Sflag ) {
    PDB::filevec& f = pdb.getFileVec();
    for (PDB::filevec::iterator fi = f.begin(); fi != f.end(); ++fi)
      printSo(ostr, *fi);
  }

  if ( Aflag || Yflag ) {
    PDB::typevec& t = pdb.getTypeVec();
    for (PDB::typevec::iterator ty = t.begin(); ty != t.end(); ++ty)
      printTy(ostr, *ty);
  }

  if ( Aflag || Gflag ) {
    if ( pdb.language() & PDB::LA_C_or_CXX ) {
      PDB::classvec& c = pdb.getClassVec();
      for (PDB::classvec::iterator cl = c.begin(); cl != c.end(); ++cl)
        printClass(ostr, *cl);
    } else if ( pdb.language() & PDB::LA_FORTRAN ) {
      PDB::modulevec& m = pdb.getModuleVec();
      for (PDB::modulevec::iterator mo = m.begin(); mo != m.end(); ++mo)
        printModule(ostr, *mo);
    }
  }

  if ( Aflag || Rflag ) {
    if ( pdb.language() & PDB::LA_C_or_CXX ) {
      PDB::croutinevec& r = pdb.getCRoutineVec();
      for (PDB::croutinevec::iterator ro = r.begin(); ro != r.end(); ++ro)
        printCRo(ostr, *ro);
    } else if ( pdb.language() & PDB::LA_FORTRAN ) {
      PDB::froutinevec& r = pdb.getFRoutineVec();
      for (PDB::froutinevec::iterator ro = r.begin(); ro != r.end(); ++ro)
        printFRo(ostr, *ro);
    }
  }

  if ( Aflag || Tflag ) {
    PDB::templatevec& u = pdb.getTemplateVec();
    for (PDB::templatevec::iterator te = u.begin(); te != u.end(); ++te)
      printTe(ostr, *te);
  }

  if ( Aflag || Nflag ) {
    PDB::namespacevec& n = pdb.getNamespaceVec();
    for (PDB::namespacevec::iterator na = n.begin(); na != n.end(); ++na)
      printNa(ostr, *na);
  }

  if ( Aflag || Mflag ) {
    PDB::macrovec& m = pdb.getMacroVec();
    for (PDB::macrovec::iterator ma = m.begin(); ma != m.end(); ++ma)
      printMa(ostr, *ma);
  }

  if ( Aflag || Pflag ) {
    PDB::pragmavec& p = pdb.getPragmaVec();
    for (PDB::pragmavec::iterator pr = p.begin(); pr != p.end(); ++pr)
      printPr(ostr, *pr);
  }
}

int main(int argc, char *argv[]) {
  int ch;
  bool errflag = argc < 2;
  char *outfile = 0;

  while ( (ch = getopt(argc, argv, "AGMNPRSTYco:")) != -1 ) {
    switch (ch) {
    case 'A': Aflag = true;     break;
    case 'G': Gflag = true;     break;
    case 'M': Mflag = true;     break;
    case 'N': Nflag = true;     break;
    case 'P': Pflag = true;     break;
    case 'R': Rflag = true;     break;
    case 'S': Sflag = true;     break;
    case 'T': Tflag = true;     break;
    case 'Y': Yflag = true;     break;
    case 'c': checkonly = true; break;
    case 'o': outfile = optarg; break;
    case '?': errflag = true;   break;
    }
  }
  if ( errflag ) {
    cerr << "usage: " << argv[0]
         << "pdbconv   : Simple tool that checks the consistency/correctness of a PDB file\n";
    cerr << "and converts it to a more verbose, human-readable format.\n\n"
      
	    "  pdbconv [-c | -o <outfile>] [-AGMNPRSTY] <pdbfile>\n\n"
    
	  "Called without any options, pdbconv reads the PDB file <pdbfile>\n"
          "checks it for correctness, and prints it out again in a standard\n"
          "form.  The following options are available:\n\n"

          "-c            Check for correctness only\n\n"

          "-o <outfile>  Write output to file <outfile>\n\n"

          "-A            Convert (A)ll item output to verbose format\n\n"

          "-G            Print only (G)roup items (in verbose format)\n\n"
    
          "-M            Print only (M)acro items (in verbose format)\n\n"

          "-N            Print only (N)amespace items (in verbose format)\n\n"

          "-P            Print only (P)ragma items (in verbose format)\n\n"

          "-R            Print only (R)outine items (in verbose format)\n\n"

          "-S            Print only (S)ource file items (in verbose format)\n\n"

          "-T            Print only (T)emplate items (in verbose format)\n\n"

          "-Y            Print only t(Y)pe items (in verbose format)\n\n";


    return 1;
  }

  PDB pdb(argv[optind]);
  if ( ! pdb ) return 1;
  if ( checkonly ) return 0;
 
  ostream* ostr;
  ofstream of;
  if ( outfile ) {
    of.open(outfile);
    ostr = &of;
  } else 
    ostr = &cout;

  if ( Aflag || Gflag || Mflag || Nflag || Pflag ||
       Rflag || Sflag || Tflag || Yflag )
    printPDB(pdb, *ostr);
  else
    pdb.write(*ostr);
  return 0;
}
