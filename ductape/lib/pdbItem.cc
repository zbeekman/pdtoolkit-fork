/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbItem.h"
#include "pdbClass.h"
#include "pdbModule.h"
#include "pdbRoutine.h"
#include "pdbNamespace.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbItem.inl"
#endif

extern void pdb_ERROR(const char *msg, const char *val);
extern void pdb_ERROR(const char *msg, const char *item, int val);

void pdbItem::process(PDB*) {
  string fn = name();
  const pdbItem *iptr = this, *jptr;
  
  do {
    if ( iptr->name().rfind("::") != STR_NPOS ) break; // ??? parser bug
    if ( (jptr = iptr->parentGroup()) )
      fn = jptr->name() + "::" + fn;
    else if ( (jptr = iptr->parentNSpace()) )
      fn = jptr->name() + "::" + fn;
    iptr = jptr;
  } while (iptr);
      
  fullName(fn);
}

pdbSimpleItem::dupl_t pdbItem::findDuplicate(pdbSimpleItem* r) {
  pdbItem* rhs = dynamic_cast<pdbItem*> (r);
  //return ( fullName() == rhs->fullName() ? NEWDUPL : NODUPL );

  bool res = fullName() == rhs->fullName();
  if (res) {
    pdbLoc ll = location();
    pdbLoc rl = rhs->location();
    if ( ll.file() && rl.file() &&
       ( (ll.file()->name() != rl.file()->name()) ||
         (ll.line() != rl.line()) || (ll.col() != rl.col()) ) ) {
    cerr << fullName() << " == " << rhs->fullName() << endl;
    cerr << "*** " << ll.file()->name() << " ??? " << rl.file()->name() << endl;
    cerr << "*** " << ll.line() << " ??? " << rl.line() << endl;
    cerr << "*** " << ll.col() << " ??? " << rl.col() << endl;

    }
    return NEWDUPL;
  }
  return NODUPL;
}

ostream& pdbItem::print(ostream& ostr) const {
  access_t a;

  const char *X = attrPrefix();
  ostr << desc()[0] << desc()[1] << "#" << id();
  if ( ! (name() == "$NA$") ) ostr << " " << name();
  ostr << "\n";
  if ( location().file() ) ostr << X << "loc " << location() << "\n";
  if ( const pdbGroup* gptr = parentGroup() )
    ostr << X << "group gr#" << gptr->id() << "\n";
  if ( (a = access()) != AC_NA )
    ostr << X << "acs " << toName(a) << "\n";
  if ( const pdbNamespace* nptr = parentNSpace() ) {
    ostr << X << "nspace na#" << nptr->id() << "\n";
  }
  return ostr;
}

void pdbItem::adjustPtrs(PDB* p) {
  const pdbFile* f;
  if ( (f=location().file()) && (f->newId() > UNIQUE) )
    location().file(p->getFileMap()[f->newId()]);
  if ( gptr && (gptr->newId() > UNIQUE) ) {
    if ( p->language() & PDB::LA_C_or_CXX ) {
      gptr = p->getClassMap()[gptr->newId()];
    } else if ( p->language() & PDB::LA_FORTRAN ) {
      gptr = p->getModuleMap()[gptr->newId()];
    }
  }
  if ( nptr && (nptr->newId() > UNIQUE) )
    nptr = p->getNamespaceMap()[nptr->newId()];
}

const char *pdbItem::toName(access_t v) {
  switch (v) {
  case AC_PUB : return "pub";
  case AC_PRIV: return "priv";
  case AC_PROT: return "prot";
  case AC_NA  :
  default     : return "NA";
  }
}

const char *pdbItem::toName(routine_t v) {
  switch (v) {
  case RO_ASM    : return "asm";
  case RO_AUTO   : return "auto";
  case RO_EXT    : return "ext";
  case RO_STAT   : return "stat";
  case RO_TPROTO : return "tproto";

  case RO_FEXT   : return "fext";
  case RO_FPROG  : return "fprog";
  case RO_FBLDAT : return "fbldat";
  case RO_FINTRIN: return "fintrin";
  case RO_FINT   : return "fint";
  case RO_FSTFN  : return "fstfn";
  case RO_FMPROC : return "fmproc";
  case RO_FUNSPEC: return "funspec";
  case RO_FALIAS : return "falias";

  case RO_NA     :
  default        : return "NA";
  }
}

const char *pdbItem::toName(rspec_t v) {
  switch (v) {
  case RS_CONV: return "conv";
  case RS_CTOR: return "ctor";
  case RS_DTOR: return "dtor";
  case RS_OP  : return "op";
  case RS_NA  :
  default     : return "NA";
  }
}

const char *pdbItem::toName(virt_t v) {
  switch (v) {
  case VI_PURE: return "pure";
  case VI_VIRT: return "virt";
  case VI_NO  :
  default     : return "no";
  }
}

const char *pdbItem::toName(macro_t v) {
  switch (v) {
  case MA_DEF  : return "def";
  case MA_UNDEF: return "undef";
  case MA_NA   :
  default      : return "NA";
  }
}

const char *pdbItem::toName(templ_t v) {
  switch (v) {
  case TE_CLASS   : return "class";
  case TE_FUNC    : return "func";
  case TE_MEMCLASS: return "memclass";
  case TE_MEMFUNC : return "memfunc";
  case TE_STATMEM : return "statmem";
  case TE_TPARAM  : return "ttparam";
  case TE_NA      :
  default         : return "NA";
  }
}

const char *pdbItem::toName(float_t v) {
  switch (v) {
  case FL_FLOAT  : return "float";
  case FL_DBL    : return "dbl";
  case FL_LONGDBL: return "longdbl";
  case FL_NA     :
  default        : return "NA";
  }
}

const char *pdbItem::toName(int_t v) {
  switch (v) {
  case I_CHAR     : return "char";
  case I_SCHAR    : return "schar";
  case I_UCHAR    : return "uchar";
  case I_SHORT    : return "short";
  case I_USHORT   : return "ushort";
  case I_INT      : return "int";
  case I_UINT     : return "uint";
  case I_LONG     : return "long";
  case I_ULONG    : return "ulong";
  case I_LONGLONG : return "longlong";
  case I_ULONGLONG: return "ulonglong";
  case I_WCHAR    : return "wchar";
  case I_NA       :
  default         : return "NA";
  }
}

const char *pdbItem::toName(type_t v) {
  switch (v) {
  case TY_BOOL  : return "bool";
  case TY_ENUM  : return "enum";
  case TY_ERR   : return "err";
  case TY_FUNC  : return "func";
  case TY_VOID  : return "void";
  case TY_INT   : return "int";
  case TY_FLOAT : return "float";
  case TY_PTR   : return "ptr";
  case TY_REF   : return "ref";
  case TY_ARRAY : return "array";
  case TY_TREF  : return "tref";
  case TY_PTRMEM: return "ptrmem";
  case TY_TPARAM: return "tparam";
  case TY_WCHAR : return "wchar";

  case TY_FERR  : return "ferr";
  case TY_FVOID : return "fvoid";
  case TY_FINT  : return "fint";
  case TY_FLOGIC: return "flogic";
  case TY_FFLOAT: return "ffloat";
  case TY_FFUNC : return "ffunc";
  case TY_FCHAR : return "fchar";
  case TY_FARRAY: return "farray";
  case TY_FCMPLX: return "fcmplx";
  case TY_FUNSPECFUNC: return "funspecfunc";
  case TY_FBLDAT: return "fbldat";
  case TY_FMOD  : return "fmod";
  case TY_FPTR  : return "fptr";

  case TY_GROUP : return "group";
  case TY_NA    :
  default       : return "NA";
  }
}

const char *pdbItem::toName(qual_t v) {
  switch (v) {
  case QL_CONST   : return "const";
  case QL_VOLATILE: return "volatile";
  case QL_RESTRICT: return "restrict";
  case QL_NA      : 
  default         : return "NA";
  }
}

const char *pdbItem::toName(group_t v) {
  switch (v) {
  case GR_CLASS   : return "class";
  case GR_STRUCT  : return "struct";
  case GR_UNION   : return "union";
  case GR_TPROTO  : return "tproto";
  case GR_FDERIVED: return "fderived";
  case GR_FMODULE : return "fmodule";
  case GR_NA      : 
  default         : return "NA";
  }
}

const char *pdbItem::toName(link_t v) {
  switch (v) {
  case LK_INTERNAL: return "internal";
  case LK_CXX     : return "C++";
  case LK_C       : return "C";
  case LK_FINT    : return "fint";
  case LK_F90     : return "f90";
  case LK_NA      :
  default         : return "no";
  }
}

const char *pdbItem::toName(mem_t v) {
  switch (v) {
  case M_VAR    : return "var";
  case M_STATVAR: return "statvar";
  case M_TYPE   : return "type";
  case M_TEMPL  : return "templ";
  case M_NA     :
  default       : return "NA";
  }
}

const char *pdbItem::toName(shape_t v) {
  switch (v) {
  case SH_EXPLICIT : return "explicit";
  case SH_ASIZE    : return "asmdsize";
  case SH_ASHAPE   : return "asmdshape";
  case SH_DEFERRED : return "deferred";
  case SH_NA       :
  default          : return "NA";
  }
}

pdbItem::access_t pdbItem::toAccess(const char *v) {
  if ( strcmp("pub",  v) == 0 ) return AC_PUB;
  if ( strcmp("priv", v) == 0 ) return AC_PRIV;
  if ( strcmp("prot", v) == 0 ) return AC_PROT;
  if ( strcmp("NA",   v) == 0 ) return AC_NA;
  pdb_ERROR("Unknown access mode ", v);
  return AC_NA;
}

pdbItem::fprefix_t pdbItem::toFPrefix(const char *v) {
  if ( strcmp("pure",  v) == 0 ) return FP_PURE;
  if ( strcmp("elem", v) == 0 ) return FP_ELEM;
  pdb_ERROR("Unknown prefix ", v);
  return FP_NA;
}


pdbItem::routine_t pdbItem::toRoutine(const char *v) {
  if ( strcmp("ext",     v) == 0 ) return RO_EXT;
  if ( strcmp("stat",    v) == 0 ) return RO_STAT;
  if ( strcmp("auto",    v) == 0 ) return RO_AUTO;
  if ( strcmp("asm",     v) == 0 ) return RO_ASM;
  if ( strcmp("tproto",  v) == 0 ) return RO_TPROTO;
  if ( strcmp("NA",      v) == 0 ) return RO_NA;

  if ( strcmp("fext",    v) == 0 ) return RO_FEXT;
  if ( strcmp("fprog",   v) == 0 ) return RO_FPROG;
  if ( strcmp("fbldat",  v) == 0 ) return RO_FBLDAT;
  if ( strcmp("fintrin", v) == 0 ) return RO_FINTRIN;
  if ( strcmp("fint",    v) == 0 ) return RO_FINT;
  if ( strcmp("fstfn",   v) == 0 ) return RO_FSTFN;
  if ( strcmp("fmproc",  v) == 0 ) return RO_FMPROC;
  if ( strcmp("funspec", v) == 0 ) return RO_FUNSPEC;
  if ( strcmp("falias",  v) == 0 ) return RO_FALIAS;

  pdb_ERROR("Unknown storage mode ", v);
  return RO_NA;
}

pdbItem::rspec_t pdbItem::toRSpecial(const char *v) {
  if ( strcmp("ctor", v) == 0 ) return RS_CTOR;
  if ( strcmp("dtor", v) == 0 ) return RS_DTOR;
  if ( strcmp("op",   v) == 0 ) return RS_OP;
  if ( strcmp("conv", v) == 0 ) return RS_CONV;
  if ( strcmp("NA",   v) == 0 ) return RS_NA;
  pdb_ERROR("Unknown routine kind ", v);
  return RS_NA;
}

pdbItem::virt_t pdbItem::toVirt(const char *v) {
  if ( strcmp("no",   v) == 0 ) return VI_NO;
  if ( strcmp("virt", v) == 0 ) return VI_VIRT;
  if ( strcmp("pure", v) == 0 ) return VI_PURE;
  pdb_ERROR("Unknown virtuality mode ", v);
  return VI_NO;
}

pdbItem::macro_t pdbItem::toMacro(const char *v) {
  if ( strcmp("def",   v) == 0 ) return MA_DEF;
  if ( strcmp("undef", v) == 0 ) return MA_UNDEF;
  if ( strcmp("NA",    v) == 0 ) return MA_NA;
  pdb_ERROR("Unknown macro kind ", v);
  return MA_NA;
}

pdbItem::templ_t pdbItem::toTempl(const char *v) {
  if ( strcmp("class",    v) == 0 ) return TE_CLASS;
  if ( strcmp("func",     v) == 0 ) return TE_FUNC;
  if ( strcmp("memclass", v) == 0 ) return TE_MEMCLASS;
  if ( strcmp("memfunc",  v) == 0 ) return TE_MEMFUNC;
  if ( strcmp("statmem",  v) == 0 ) return TE_STATMEM;
  if ( strcmp("ttparam",  v) == 0 ) return TE_TPARAM;
  if ( strcmp("none",     v) == 0 ) return TE_NA;
  pdb_ERROR("Unknown template kind ", v);
  return TE_NA;
}

pdbItem::float_t pdbItem::toFloat(const char* v) {
  if ( strcmp("float",   v) == 0 ) return FL_FLOAT;
  if ( strcmp("dbl",     v) == 0 ) return FL_DBL;
  if ( strcmp("longdbl", v) == 0 ) return FL_LONGDBL;
  pdb_ERROR("Unknown float kind ", v);
  return FL_NA;
}

pdbItem::int_t pdbItem::toInt(const char* v) {
  if ( strcmp("char",      v) == 0 ) return I_CHAR;
  if ( strcmp("schar",     v) == 0 ) return I_SCHAR;
  if ( strcmp("uchar",     v) == 0 ) return I_UCHAR;
  if ( strcmp("short",     v) == 0 ) return I_SHORT;
  if ( strcmp("ushort",    v) == 0 ) return I_USHORT;
  if ( strcmp("int",       v) == 0 ) return I_INT;
  if ( strcmp("uint",      v) == 0 ) return I_UINT;
  if ( strcmp("long",      v) == 0 ) return I_LONG;
  if ( strcmp("ulong",     v) == 0 ) return I_ULONG;
  if ( strcmp("longlong",  v) == 0 ) return I_LONGLONG;
  if ( strcmp("ulonglong", v) == 0 ) return I_ULONGLONG;
  if ( strcmp("wchar",     v) == 0 ) return I_WCHAR;
  pdb_ERROR("Unknown integer kind ", v);
  return I_NA;
}

pdbItem::type_t pdbItem::toType(const char* v) {
  if ( v[0] == 'f' ) {
    if ( strcmp("func",   v) == 0 ) return TY_FUNC;
    if ( strcmp("float",  v) == 0 ) return TY_FLOAT;
    if ( strcmp("ferr",   v) == 0 ) return TY_FERR  ;
    if ( strcmp("fvoid",  v) == 0 ) return TY_FVOID ;
    if ( strcmp("fint",   v) == 0 ) return TY_FINT  ;
    if ( strcmp("flogic", v) == 0 ) return TY_FLOGIC;
    if ( strcmp("ffloat", v) == 0 ) return TY_FFLOAT;
    if ( strcmp("ffunc",  v) == 0 ) return TY_FFUNC ;
    if ( strcmp("fchar",  v) == 0 ) return TY_FCHAR ;
    if ( strcmp("farray", v) == 0 ) return TY_FARRAY;
    if ( strcmp("fcmplx", v) == 0 ) return TY_FCMPLX;
    if ( strcmp("funspecfunc", v) == 0 ) return TY_FUNSPECFUNC;
    if ( strcmp("fbldat", v) == 0 ) return TY_FBLDAT;
    if ( strcmp("fmod",   v) == 0 ) return TY_FMOD  ;
    if ( strcmp("fptr",   v) == 0 ) return TY_FPTR  ;
  } else {
    if ( strcmp("bool",   v) == 0 ) return TY_BOOL;
    if ( strcmp("enum",   v) == 0 ) return TY_ENUM;
    if ( strcmp("err",    v) == 0 ) return TY_ERR;
    if ( strcmp("void",   v) == 0 ) return TY_VOID;
    if ( strcmp("int",    v) == 0 ) return TY_INT;
    if ( strcmp("ptr",    v) == 0 ) return TY_PTR;
    if ( strcmp("ref",    v) == 0 ) return TY_REF;
    if ( strcmp("array",  v) == 0 ) return TY_ARRAY;
    if ( strcmp("tref",   v) == 0 ) return TY_TREF;
    if ( strcmp("ptrmem", v) == 0 ) return TY_PTRMEM;
    if ( strcmp("tparam", v) == 0 ) return TY_TPARAM;
    if ( strcmp("wchar",  v) == 0 ) return TY_WCHAR;
    if ( strcmp("NA",     v) == 0 ) return TY_NA;
  }
  pdb_ERROR("Unknown type kind ", v);
  return TY_NA;
}

pdbItem::qual_t pdbItem::toQual(const char* v) {
  if ( strcmp("const",        v) == 0 ) return QL_CONST;
  if ( strcmp("volatile",     v) == 0 ) return QL_VOLATILE;
  if ( strcmp("restrict",     v) == 0 ) return QL_RESTRICT;
  if ( strcmp("__restrict",   v) == 0 ) return QL_RESTRICT;
  if ( strcmp("__restrict__", v) == 0 ) return QL_RESTRICT;
  pdb_ERROR("Unknown qualifier ", v);
  return QL_NA;
}

pdbItem::group_t pdbItem::toGroup(const char* v) {
  if ( strcmp ("class",    v) == 0 ) return GR_CLASS;
  if ( strcmp ("struct",   v) == 0 ) return GR_STRUCT;
  if ( strcmp ("union",    v) == 0 ) return GR_UNION;
  if ( strcmp ("tproto",   v) == 0 ) return GR_TPROTO;
  if ( strcmp ("fderived", v) == 0 ) return GR_FDERIVED;
  if ( strcmp ("fmodule",  v) == 0 ) return GR_FMODULE;
  pdb_ERROR("Unknown group kind ", v);
  return GR_NA;
}

pdbItem::link_t pdbItem::toLink(const char* v) {
  if ( strcmp ("internal", v) == 0 ) return LK_INTERNAL;
  if ( strcmp ("C++",      v) == 0 ) return LK_CXX;
  if ( strcmp ("C",        v) == 0 ) return LK_C;
  if ( strcmp ("fint",     v) == 0 ) return LK_FINT;
  if ( strcmp ("f90",      v) == 0 ) return LK_F90;
  if ( strcmp ("no",       v) == 0 ) return LK_NA;
  pdb_ERROR("Unknown linkage kind ", v);
  return LK_NA;
}

pdbItem::mem_t pdbItem::toMem(const char* v) {
  if ( strcmp ("var",     v) == 0 ) return M_VAR;
  if ( strcmp ("statvar", v) == 0 ) return M_STATVAR;
  if ( strcmp ("type",    v) == 0 ) return M_TYPE;
  if ( strcmp ("templ",   v) == 0 ) return M_TEMPL;
  pdb_ERROR("Unknown class member type ", v);
  return M_NA;
}

pdbItem::shape_t pdbItem::toShape(const char* v) {
  if ( strcmp ("explicit",  v) == 0 ) return SH_EXPLICIT;
  if ( strcmp ("asmdsize",  v) == 0 ) return SH_ASIZE;
  if ( strcmp ("asmdshape", v) == 0 ) return SH_ASHAPE;
  if ( strcmp ("deferred",  v) == 0 ) return SH_DEFERRED;
  if ( strcmp ("NA",        v) == 0 ) return SH_NA;
  pdb_ERROR("Unknown shape kind ", v);
  return SH_NA;
}
