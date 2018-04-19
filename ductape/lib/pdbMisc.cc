/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdb.h"
#include "pdbRoutine.h"
#include "pdbClass.h"

#ifdef NO_INLINE
#  define inline
#  include "pdb.inl"
#endif

extern void pdb_ERROR(const char *msg, const char *val);

PDB::PDB(char *fname) {
  if (!attrsAreInit) {
    initAttrs();
    attrsAreInit = true;
  }
  versNum = 1;
  lang    = LA_NA;
  topRoutine = new pdbRoutine("$TOP$", pdbSimpleItem::UNIQUE);
  topClass = new pdbClass("$TOP$", pdbSimpleItem::UNIQUE);
  topFile = new pdbFile("$TOP$", pdbSimpleItem::UNIQUE);
  status = read(fname);
}

PDB::~PDB() {
  for (itemvec::iterator it = itemVec.begin(); it!=itemVec.end(); ++it) {
    if ( (*it)->newId() == pdbSimpleItem::NOTDEL ) {
      // do not delete: item used in other PDB; but reset
      (*it)->newId(pdbSimpleItem::UNIQUE);
    } else {
      delete *it;
    }
  }
  delete topRoutine;
  delete topClass;
  delete topFile;
}

PDB::attrmap PDB::attr;
PDB::namemap PDB::name;
bool PDB::attrsAreInit = false;

void PDB::initAttrs() {

  // Common Attributes 
  attr["cacs"]     = ACS;       name[ACS]       = "acs";
  attr["cmacs"]    = ACS;
  attr["gacs"]     = ACS;
  attr["gmacs"]    = ACS;
  attr["racs"]     = ACS;
  attr["tacs"]     = ACS;
  attr["yacs"]     = ACS;
  attr["cclass"]   = CLASS;     name[CLASS]     = "class";
  attr["rclass"]   = CLASS;
  attr["tclass"]   = CLASS;
  attr["yclass"]   = CLASS;
  attr["ggroup"]   = GROUP;     name[GROUP]     = "group";
  attr["gmgroup"]  = GROUP;
  attr["rgroup"]   = GROUP;
  attr["tgroup"]   = GROUP;
  attr["ygroup"]   = GROUP;
  attr["cmloc"]    = LOC;       name[LOC]       = "loc";
  attr["cloc"]     = LOC;
  attr["gmloc"]    = LOC;
  attr["gloc"]     = LOC;
  attr["mloc"]     = LOC;
  attr["nloc"]     = LOC;
  attr["ploc"]     = LOC;
  attr["rloc"]     = LOC;
  attr["tloc"]     = LOC;
  attr["yloc"]     = LOC;
  attr["cnspace"]  = NSPACE;    name[NSPACE]    = "nspace";
  attr["gnspace"]  = NSPACE;
  attr["nnspace"]  = NSPACE;
  attr["rnspace"]  = NSPACE;
  attr["tnspace"]  = NSPACE;
  attr["ynspace"]  = NSPACE;
  attr["rpos"]     = POS;       name[POS]       = "pos";
  attr["cpos"]     = POS;
  attr["gpos"]     = POS;
  attr["tpos"]     = POS;
  attr["npos"]     = POS;

  // Common Template Item Attributes
  attr["cmspecl"]  = SPECL;     name[SPECL]     = "specl";
  attr["cspecl"]   = SPECL;
  attr["gmspecl"]  = SPECL;
  attr["gspecl"]   = SPECL;
  attr["rspecl"]   = SPECL;
  attr["cmtempl"]  = TEMPL;     name[TEMPL]     = "templ";
  attr["ctempl"]   = TEMPL;
  attr["gmtempl"]  = TEMPL;
  attr["gtempl"]   = TEMPL;
  attr["rtempl"]   = TEMPL;
  attr["gsparam"]  = GSPARAM;   name[GSPARAM]   = "gsparam";

  // Item Specific Attributes
  attr["cbase"]    = CBASE;     name[CBASE]     = "cbase";
  attr["cfrclass"] = CFRCLASS;  name[CFRCLASS]  = "cfrclass";
  attr["cfrfunc"]  = CFRFUNC;   name[CFRFUNC]   = "cfrfunc";
  attr["cfunc"]    = CFUNC;     name[CFUNC]     = "cfunc";
  attr["ckind"]    = CKIND;     name[CKIND]     = "ckind";
  attr["cmem"]     = CMEM;      name[CMEM]      = "cmem";
  attr["cmconst"]  = CMCONST;   name[CMCONST]   = "cmconst";
  attr["cmisbit"]  = CMISBIT;   name[CMISBIT]   = "cmisbit";
  attr["cmkind"]   = CMKIND;    name[CMKIND]    = "cmkind";
  attr["cmmut"]    = CMMUT;     name[CMMUT]     = "cmmut";
  attr["cmtype"]   = CMTYPE;    name[CMTYPE]    = "cmtype";
  attr["gbase"]    = GBASE;     name[GBASE]     = "gbase";
  attr["gfrgroup"] = GFRGROUP;  name[GFRGROUP]  = "gfrgroup";
  attr["gfrfunc"]  = GFRFUNC;   name[GFRFUNC]   = "gfrfunc";
  attr["gfunc"]    = GFUNC;     name[GFUNC]     = "gfunc";
  attr["gkind"]    = GKIND;     name[GKIND]     = "gkind";
  attr["gmem"]     = GMEM;      name[GMEM]      = "gmem";
  attr["gmconst"]  = GMCONST;   name[GMCONST]   = "gmconst";
  attr["gmisbit"]  = GMISBIT;   name[GMISBIT]   = "gmisbit";
  attr["gmkind"]   = GMKIND;    name[GMKIND]    = "gmkind";
  attr["gmmut"]    = GMMUT;     name[GMMUT]     = "gmmut";
  attr["gmtype"]   = GMTYPE;    name[GMTYPE]    = "gmtype";
  attr["lang"]     = LANG;      name[LANG]      = "lang";
  attr["mkind"]    = MKIND;     name[MKIND]     = "mkind";
  attr["mtext"]    = MTEXT;     name[MTEXT]     = "mtext";
  attr["nalias"]   = NALIAS;    name[NALIAS]    = "nalias";
  attr["nmem"]     = NMEM;      name[NMEM]      = "nmem";
  attr["pkind"]    = PKIND;     name[PKIND]     = "pkind";
  attr["ppos"]     = PPOS;      name[PPOS]      = "ppos";
  attr["ptext"]    = PTEXT;     name[PTEXT]     = "ptext";
  attr["ralias"]   = RALIAS;    name[RALIAS]    = "ralias";
  attr["rarginfo"] = RARGINFO;  name[RARGINFO]  = "rarginfo";
  attr["rbody"]    = RBODY;     name[RBODY]     = "rbody";
  attr["rcall"]    = RCALL;     name[RCALL]     = "rcall";
  attr["rcatch"]   = RCATCH;    name[RCATCH]    = "rcatch";
  attr["rcgen"]    = RCGEN;     name[RCGEN]     = "rcgen";
  attr["rcrvo"]    = RCRVO;     name[RCRVO]     = "rcrvo";
  attr["rexpl"]    = REXPL;     name[REXPL]     = "rexpl";
  attr["rimpl"]    = RIMPL;     name[RIMPL]     = "rimpl";
  attr["rinline"]  = RINLINE;   name[RINLINE]   = "rinline";
  attr["riselem"]  = RISELEM;   name[RISELEM]   = "riselem";
  attr["rkind"]    = RKIND;     name[RKIND]     = "rkind";
  attr["rfprefix"] = RFPREFIX;  name[RFPREFIX]  = "rfprefix";
  attr["rlink"]    = RLINK;     name[RLINK]     = "rlink";
  attr["rrec"]     = RREC;      name[RREC]      = "rrec";
  attr["rret"]     = RRET;      name[RRET]      = "rret";
  attr["rroutine"] = RROUT;     name[RROUT]     = "rroutine";
  attr["rsig"]     = RSIG;      name[RSIG]      = "rsig";
  attr["rskind"]   = RSKIND;    name[RSKIND]    = "rskind";
  attr["rstart"]   = RSTART;    name[RSTART]    = "rstart";
  attr["rstatic"]  = RSTAT;     name[RSTAT]     = "rstatic";
  attr["rstmt"]    = RSTMT;     name[RSTMT]     = "rstmt";
  attr["rstop"]    = RSTOP;     name[RSTOP]     = "rstop";
  attr["rstore"]   = RSTORE;    name[RSTORE]    = "rstore";
  attr["rvirt"]    = RVIRT;     name[RVIRT]     = "rvirt";
  attr["scom"]     = SCOM;      name[SCOM]      = "scom";
  attr["sinc"]     = SINC;      name[SINC]      = "sinc";
  attr["ssys"]     = SSYS;      name[SSYS]      = "ssys";
  attr["tdecl"]    = TDECL;     name[TDECL]     = "tdecl";
  attr["tdef"]     = TDEF;      name[TDEF]      = "tdef";
  attr["tkind"]    = TKIND;     name[TKIND]     = "tkind";
  attr["tparam"]   = TPARAM;    name[TPARAM]    = "tparam";
  attr["tproto"]   = TPROTO;    name[TPROTO]    = "tproto";
  attr["tsparam"]  = TSPARAM;   name[TSPARAM]   = "tsparam";
  attr["ttext"]    = TTEXT;     name[TTEXT]     = "ttext";
  attr["ttype"]    = TTYPE;     name[TTYPE]     = "ttype";
  attr["yargt"]    = YARGT;     name[YARGT]     = "yargt";
  attr["yclen"]    = YCLEN;     name[YCLEN]     = "yclen";
  attr["ydim"]     = YDIM;      name[YDIM]      = "ydim";
  attr["yelem"]    = YELEM;     name[YELEM]     = "yelem";
  attr["yellip"]   = YELLIP;    name[YELLIP]    = "yellip";
  attr["yenum"]    = YENUM;     name[YENUM]     = "yenum";
  attr["yexcep"]   = YEXCEP;    name[YEXCEP]    = "yexcep";
  attr["yfkind"]   = YFKIND;    name[YFKIND]    = "yfkind";
  attr["yikind"]   = YIKIND;    name[YIKIND]    = "yikind";
  attr["ykind"]    = YKIND;     name[YKIND]     = "ykind";
  attr["ympclass"] = YMPGROUP;
  attr["ympgroup"] = YMPGROUP;  name[YMPGROUP]  = "ympgroup";
  attr["ymptype"]  = YMPTYPE;   name[YMPTYPE]   = "ymptype";
  attr["ynelem"]   = YNELEM;    name[YNELEM]    = "ynelem";
  attr["yold"]     = YOLD;      name[YOLD]      = "yold";
  attr["yptr"]     = YPTR;      name[YPTR]      = "yptr";
  attr["yqual"]    = YQUAL;     name[YQUAL]     = "yqual";
  attr["yrank"]    = YRANK;     name[YRANK]     = "yrank";
  attr["yref"]     = YREF;      name[YREF]      = "yref";
  attr["yrett"]    = YRETT;     name[YRETT]     = "yrett";
  attr["yshape"]   = YSHAPE;    name[YSHAPE]    = "yshape";
  attr["ysigned"]  = YSIGNED;   name[YSIGNED]   = "ysigned";
  attr["ystat"]    = YSTAT;     name[YSTAT]     = "ystat";
  attr["ytref"]    = YTREF;     name[YTREF]     = "ytref";
  // UPC support
  attr["yshared"]    = YSHARED;    name[YSHARED]    = "yshared";
  attr["yblocksize"] = YBLOCKSIZE; name[YBLOCKSIZE] = "yblocksize";
  attr["yrelaxed"]   = YRELAXED;   name[YRELAXED]   = "yrelaxed";
  attr["ystrict"]    = YSTRICT;    name[YSTRICT]    = "ystrict";
}

const char *PDB::toName(lang_t v) {
  switch (v) {
  case LA_C        : return "c";
  case LA_CXX      : return "c++";
  case LA_C_or_CXX : return "c_or_c++";
  case LA_FORTRAN  : return "fortran";
  case LA_JAVA     : return "java";
  case LA_MULTI    : return "multi";
  case LA_UPC      : return "upc";
  case LA_NA       :
  default          : return "NA";
  }
}

PDB::lang_t PDB::toLang(const char *v) {
  if ( strcmp("c",        v) == 0 ) return LA_C;
  if ( strcmp("c++",      v) == 0 ) return LA_CXX;
  if ( strcmp("c_or_c++", v) == 0 ) return LA_C_or_CXX;
  if ( strcmp("fortran",  v) == 0 ) return LA_FORTRAN;
  if ( strcmp("java",     v) == 0 ) return LA_JAVA;
  if ( strcmp("multi",    v) == 0 ) return LA_MULTI;
  if ( strcmp("upc",      v) == 0 ) return LA_UPC;
  if ( strcmp("NA",       v) == 0 ) return LA_NA;
  pdb_ERROR("Unknown language ", v);
  return LA_NA;
}

