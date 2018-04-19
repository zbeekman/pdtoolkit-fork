/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbType.h"
#include "pdbCRoutine.h"
#include "pdbFRoutine.h"
#include "pdbClass.h"
#include <stdio.h>

#ifdef NO_INLINE
#  define inline
#  include "pdbRoutine.inl"
#endif

extern void pdb_ERROR(const char *msg, const char *val);

const char *pdbStmt::toName(stmt_t v) {
  switch (v) {
  case ST_SWITCH         : return "switch";
  case ST_CASE           : return "case";
  case ST_INIT           : return "init";
  case ST_RETURN         : return "return";
  case ST_IF             : return "if";
  case ST_EMPTY          : return "empty";
  case ST_FOR            : return "for";
  case ST_GOTO           : return "goto";
  case ST_BREAK          : return "break";
  case ST_LABEL          : return "label";
  case ST_BLOCK          : return "block";
  case ST_ASM            : return "asm";
  case ST_EXPR           : return "expr";
  case ST_ASSIGN         : return "assign";
  case ST_THROW          : return "throw";
  case ST_WHILE          : return "while";
  case ST_DO             : return "do";
  case ST_TRY            : return "try";
  case ST_CATCH          : return "catch";
  case ST_DECL           : return "decl";
  case ST_CONTINUE       : return "continue";
  case ST_SET_VLA_SIZE   : return "set_vla_size";
  case ST_VLA_DECL       : return "vla_decl";
  case ST_VLA_DEALLOC    : return "vla_dealloc";
  case ST_FALLOCATE      : return "fallocate";
  case ST_FASSIGN        : return "fassign";
  case ST_FIO            : return "fio";
  case ST_FDO            : return "fdo";
  case ST_FDEALLOCATE    : return "fdeallocate";
  case ST_FRETURN        : return "freturn";
  case ST_FIF            : return "fif";
  case ST_FCONTINUE      : return "fcontinue";
  case ST_FSINGLE_IF     : return "fsingle_if";
  case ST_FGOTO          : return "fgoto";
  case ST_FSTOP          : return "fstop";
  case ST_FLABEL         : return "flabel";
  case ST_FEXIT          : return "fexit";
  case ST_FCYCLE         : return "fcycle";
  case ST_FARITHIF       : return "farithif";
  case ST_FENTRY         : return "fentry";
  case ST_FPAUSE         : return "fpause";
  case ST_FLABELASSIGN   : return "flabelassign";
  case ST_FPOINTERASSIGN : return "fpointerassign";
  case ST_FSELECT        : return "fselect";
  case ST_FCASE          : return "fcase";
  case ST_FWHERE         : return "fwhere";
  case ST_FFORALL        : return "fforall";
  case ST_FCALL          : return "fcall";
  case ST_FFLUSH         : return "fflush";
  case ST_FWAIT          : return "fwait";
  case ST_FLOCK          : return "flock";
  case ST_FUNLOCK        : return "funlock";
  case ST_FSYNC_ALL      : return "fsync_all";
  case ST_FSYNC_MEMORY   : return "fsync_memory";
  case ST_FSYNC_IMAGES   : return "fsync_images";
  case ST_FSELECT_TYPE   : return "fselect_type";
  case ST_FBLOCK         : return "fblock";
  case ST_FCALL_PPC      : return "fcall_ppc";
  case ST_FASSIGN_CALL   : return "fassign_call";
  case ST_FCOMPCALL      : return "fcompcall";
  case ST_FERROR_STOP    : return "ferror_stop";
  case ST_FEND_BLOCK     : return "fend_block";
  // OMP support
  case ST_FOMP_PARALLEL           : return "fomp_parallel";
  case ST_FOMP_PARALLEL_SECTIONS  : return "fomp_parallel_sections";
  case ST_FOMP_SECTIONS           : return "fomp_sections";
  case ST_FOMP_ORDERED            : return "fomp_ordered";
  case ST_FOMP_CRITICAL           : return "fomp_critical";
  case ST_FOMP_MASTER             : return "fomp_master";
  case ST_FOMP_SINGLE             : return "fomp_single";
  case ST_FOMP_TASK               : return "fomp_task";
  case ST_FOMP_DO                 : return "fomp_do";
  case ST_FOMP_PARALLEL_DO        : return "fomp_parallel_do";
  case ST_FOMP_BARRIER            : return "fomp_barrier";
  case ST_FOMP_FLUSH              : return "fomp_flush";
  case ST_FOMP_WORKSHARE          : return "fomp_workshare";
  case ST_FOMP_PARALLEL_WORKSHARE : return "fomp_parallel_workshare";
  case ST_FOMP_ATOMIC             : return "fomp_atomic";
  case ST_FOMP_TASKWAIT           : return "fomp_taskwait";
  case ST_FOMP_TASKYIELD          : return "fomp_taskyield";
  // UPC support
  case ST_UPC_FORALL     : return "upc_forall";
  case ST_UPC_BARRIER    : return "upc_barrier";
  case ST_UPC_FENCE      : return "upc_fence";
  case ST_UPC_NOTIFY     : return "upc_notify";
  case ST_UPC_WAIT       : return "upc_wait";
  case ST_NA             :
  default                : return "NA";
  }
}

pdbStmt::stmt_t pdbStmt::toStmt(const char *v) {
  if ( strcmp ("switch",         v) == 0 ) return ST_SWITCH;
  if ( strcmp ("case",           v) == 0 ) return ST_CASE;
  if ( strcmp ("init",           v) == 0 ) return ST_INIT;
  if ( strcmp ("return",         v) == 0 ) return ST_RETURN;
  if ( strcmp ("if",             v) == 0 ) return ST_IF;
  if ( strcmp ("empty",          v) == 0 ) return ST_EMPTY;
  if ( strcmp ("for",            v) == 0 ) return ST_FOR;
  if ( strcmp ("goto",           v) == 0 ) return ST_GOTO;
  if ( strcmp ("break",          v) == 0 ) return ST_BREAK;
  if ( strcmp ("label",          v) == 0 ) return ST_LABEL;
  if ( strcmp ("block",          v) == 0 ) return ST_BLOCK;
  if ( strcmp ("asm",            v) == 0 ) return ST_ASM;
  if ( strcmp ("expr",           v) == 0 ) return ST_EXPR;
  if ( strcmp ("assign",         v) == 0 ) return ST_ASSIGN;
  if ( strcmp ("throw",          v) == 0 ) return ST_THROW;
  if ( strcmp ("while",          v) == 0 ) return ST_WHILE;
  if ( strcmp ("do",             v) == 0 ) return ST_DO;
  if ( strcmp ("try",            v) == 0 ) return ST_TRY;
  if ( strcmp ("catch",          v) == 0 ) return ST_CATCH;
  if ( strcmp ("decl",           v) == 0 ) return ST_DECL;
  if ( strcmp ("continue",       v) == 0 ) return ST_CONTINUE;
  if ( strcmp ("set_vla_size",   v) == 0 ) return ST_SET_VLA_SIZE;
  if ( strcmp ("vla_decl",       v) == 0 ) return ST_VLA_DECL;
  if ( strcmp ("vla_dealloc",    v) == 0 ) return ST_VLA_DEALLOC;
  if ( strcmp ("fallocate",      v) == 0 ) return ST_FALLOCATE;
  if ( strcmp ("fassign",        v) == 0 ) return ST_FASSIGN;
  if ( strcmp ("fio",            v) == 0 ) return ST_FIO;
  if ( strcmp ("fdo",            v) == 0 ) return ST_FDO;
  if ( strcmp ("fdeallocate",    v) == 0 ) return ST_FDEALLOCATE;
  if ( strcmp ("freturn",        v) == 0 ) return ST_FRETURN;
  if ( strcmp ("fif",            v) == 0 ) return ST_FIF;
  if ( strcmp ("fsingle_if",     v) == 0 ) return ST_FSINGLE_IF;
  if ( strcmp ("fgoto",          v) == 0 ) return ST_FGOTO;
  if ( strcmp ("fstop",          v) == 0 ) return ST_FSTOP;
  if ( strcmp ("flabel",         v) == 0 ) return ST_FLABEL;
  if ( strcmp ("fexit",          v) == 0 ) return ST_FEXIT;
  if ( strcmp ("fcycle",         v) == 0 ) return ST_FCYCLE;
  if ( strcmp ("farithif",       v) == 0 ) return ST_FARITHIF;
  if ( strcmp ("fentry",         v) == 0 ) return ST_FENTRY;
  if ( strcmp ("fpause",         v) == 0 ) return ST_FPAUSE;
  if ( strcmp ("flabelassign",   v) == 0 ) return ST_FLABELASSIGN;
  if ( strcmp ("fpointerassign", v) == 0 ) return ST_FPOINTERASSIGN;
  if ( strcmp ("fselect",        v) == 0 ) return ST_FSELECT;
  if ( strcmp ("fcase",          v) == 0 ) return ST_FCASE;
  if ( strcmp ("fwhere",         v) == 0 ) return ST_FWHERE;
  if ( strcmp ("fforall",        v) == 0 ) return ST_FFORALL;
  if ( strcmp ("fcall",          v) == 0 ) return ST_FCALL;
  if ( strcmp ("fflush",         v) == 0 ) return ST_FFLUSH;
  if ( strcmp ("fwait",          v) == 0 ) return ST_FWAIT;
  if ( strcmp ("flock",          v) == 0 ) return ST_FLOCK;
  if ( strcmp ("funlock",        v) == 0 ) return ST_FUNLOCK;
  if ( strcmp ("fsync_all",      v) == 0 ) return ST_FSYNC_ALL;
  if ( strcmp ("fsync_images",   v) == 0 ) return ST_FSYNC_IMAGES;
  if ( strcmp ("fsync_memory",   v) == 0 ) return ST_FSYNC_MEMORY;
  if ( strcmp ("fselect_type",   v) == 0 ) return ST_FSELECT_TYPE;
  if ( strcmp ("fblock",         v) == 0 ) return ST_FBLOCK;
  if ( strcmp ("fassign_call",   v) == 0 ) return ST_FASSIGN_CALL;
  if ( strcmp ("fcompcall",      v) == 0 ) return ST_FCOMPCALL;
  if ( strcmp ("fcall_ppc",      v) == 0 ) return ST_FCALL_PPC;
  if ( strcmp ("ferror_stop",    v) == 0 ) return ST_FERROR_STOP;
  if ( strcmp ("fend_block",     v) == 0 ) return ST_FEND_BLOCK;
  if ( strcmp ("fcontinue",      v) == 0 ) return ST_FCONTINUE;
  // OMP support
  if ( strcmp ("fomp_parallel",           v) == 0) return ST_FOMP_PARALLEL;
  if ( strcmp ("fomp_parallel_sections",  v) == 0) return ST_FOMP_PARALLEL_SECTIONS;
  if ( strcmp ("fomp_sections",           v) == 0) return ST_FOMP_SECTIONS;
  if ( strcmp ("fomp_ordered",            v) == 0) return ST_FOMP_ORDERED;
  if ( strcmp ("fomp_critical",           v) == 0) return ST_FOMP_CRITICAL;
  if ( strcmp ("fomp_master",             v) == 0) return ST_FOMP_MASTER;
  if ( strcmp ("fomp_single",             v) == 0) return ST_FOMP_SINGLE;
  if ( strcmp ("fomp_task",               v) == 0) return ST_FOMP_TASK;
  if ( strcmp ("fomp_do",                 v) == 0) return ST_FOMP_DO;
  if ( strcmp ("fomp_parallel_do",        v) == 0) return ST_FOMP_PARALLEL_DO;
  if ( strcmp ("fomp_barrier",            v) == 0) return ST_FOMP_BARRIER;
  if ( strcmp ("fomp_flush",              v) == 0) return ST_FOMP_FLUSH;
  if ( strcmp ("fomp_workshare",          v) == 0) return ST_FOMP_WORKSHARE;
  if ( strcmp ("fomp_parallel_workshare", v) == 0) return ST_FOMP_PARALLEL_WORKSHARE;
  if ( strcmp ("fomp_atomic",             v) == 0) return ST_FOMP_ATOMIC;
  if ( strcmp ("fomp_taskwait",           v) == 0) return ST_FOMP_TASKWAIT;
  if ( strcmp ("fomp_taskyield",          v) == 0) return ST_FOMP_TASKYIELD;
  // UPC support
  if ( strcmp ("upc_forall",     v) == 0 ) return ST_UPC_FORALL;
  if ( strcmp ("upc_barrier",    v) == 0 ) return ST_UPC_BARRIER;
  if ( strcmp ("upc_fence",      v) == 0 ) return ST_UPC_FENCE;
  if ( strcmp ("upc_notify",     v) == 0 ) return ST_UPC_NOTIFY;
  if ( strcmp ("upc_wait",       v) == 0 ) return ST_UPC_WAIT;



  pdb_ERROR("Unknown statement kind ", v);
  return ST_NA;
}

ostream& operator<<(ostream& ostr, const pdbStmt& s) {
  const pdbStmt* s1;

  ostr << "st#" << s.id() << " " << pdbStmt::toName(s.kind())
       << " " << s.stmtBegin() << " " << s.stmtEnd();
  if ((s1 = s.nextStmt()) != NULL)
    ostr << " st#" << s1->id();
  else
    ostr << " NA";
  if ((s1 = s.downStmt()) != NULL)
    ostr << " st#" << s1->id();
  else
    ostr << " NA";
  if ((s1 = s.extraStmt()) != NULL)
    ostr << " st#" << s1->id();
  else if ( s.kind() == pdbStmt::ST_DECL ||
            s.kind() == pdbStmt::ST_IF   ||
            s.kind() == pdbStmt::ST_CASE ||
	        s.kind() == pdbStmt::ST_FOR  ||
            s.kind() == pdbStmt::ST_UPC_FORALL)
    ostr << " NA";
  if ((s1 = s.affinityStmt()) != NULL) 
    ostr << " st#" << s1->id();
  else if ( s.kind() == pdbStmt::ST_UPC_FORALL)
    ostr << " NA";
  return ostr;
}

ostream& operator<<(ostream& ostr, const pdbCallee& c) {
  ostr << "ro#" << c.call()->id() << " "
       << pdbItem::toName(c.virtuality()) << " ";
  return c.printLoc(ostr);
}

pdbRoutine::~pdbRoutine() {
  for (callvec::const_iterator ct=calls.begin(); ct!=calls.end(); ++ct)
    delete *ct;
  for (locvec::const_iterator rt=retrns.begin(); rt!=retrns.end(); ++rt)
    delete *rt;
  for (stmtvec::const_iterator st=stmts.begin(); st!=stmts.end(); ++st)
    delete *st;
}

void pdbRoutine::incrCalled() const {
  pdbRoutine *self = const_cast<pdbRoutine*>(this);
  self->numc++;
}

pdbStmt* pdbRoutine::addStmt(int id) {
  if ( id < 0 ) return 0;
  if ( id >= stmts.size() ) stmts.resize(id+1, 0);
  if ( ! stmts[id] ) stmts[id] = new pdbStmt(id);
  return stmts[id];
}

void pdbRoutine::process(PDB *p) {
  // if routine wasn't called, add it to the toplevel functions
  if ( numCalled() == 0 ) {
    p->callTree()->addCallee(this, pdbItem::VI_NO, 0, 0, 0);
  }

  int lastid=-1;
  // recreate PDB 2.0 return location information from statement info
  for (stmtvec::const_iterator st=statements().begin(); st!=statements().end(); ++st) {
    if (*st == NULL) {
      fprintf (stderr, "Error: invalid PDB file, last was st#%d\n",lastid);
    } else {
      lastid = (*st)->id();
      if ( (*st)->kind() == pdbStmt::ST_RETURN || (*st)->kind() == pdbStmt::ST_FRETURN ) {
	const pdbLoc& loc = (*st)->stmtBegin();
	if ( loc.file() ) addReturnLocation(loc.file(), loc.line(), loc.col());
      }
      if ( (*st)->kind() == pdbStmt::ST_FSTOP ) {
	const pdbLoc& loc = (*st)->stmtBegin();
	if ( loc.file() ) {
	  pdbFRoutine *f = dynamic_cast<pdbFRoutine *> (this);
	  f->addStopLocation(const_cast<pdbFile*>(loc.file()), loc.line(), loc.col());
	}
      }
    }
  }

  pdbItem::process(p);
}

ostream& pdbRoutine::print(ostream& ostr) const {
  rspec_t k;
  link_t l;
  const pdbStmt* s;
 
  pdbItem::print(ostr);
  if ( signature() ) ostr << "rsig ty#" << signature()->id() << "\n";
  if ( (l = linkage()) != LK_NA )
    ostr << "rlink " << toName(l) << "\n";
  ostr << "rkind " << toName(kind()) << "\n";
  if ( (k = specialKind()) != RS_NA )
    ostr << "rskind " << toName(k) << "\n";
  pdbTemplateItem::print(ostr);
  const pdbRoutine::callvec c = callees();
  for (pdbRoutine::callvec::const_iterator ct=c.begin(); ct!=c.end(); ++ct)
    ostr << "rcall " << **ct << "\n";
  pdbFatItem::print(ostr);
  if ( (s = body()) ) {
    ostr << "rbody st#" << s->id() << "\n";
    for (int i=0; i<stmts.size(); ++i)
      if ( stmts[i] && stmts[i]->kind() != pdbStmt::ST_NA ) ostr << "rstmt " << *(stmts[i]) << "\n";
  } else {
    if ( firstExecStmtLocation().file() )
      ostr << "rstart " << firstExecStmtLocation() << "\n";
    const pdbRoutine::locvec lr = returnLocations();
    for (pdbRoutine::locvec::const_iterator lt=lr.begin(); lt!=lr.end(); ++lt)
      ostr << "rret " << **lt << "\n";
  }
 
  return ostr;
}

void pdbRoutine::adjustPtrs(PDB* p) {
  const pdbRoutine* r;
  const pdbFile* fi;

  pdbTemplateItem::adjustPtrs(p);
  if ( sig->newId() > UNIQUE ) sig = p->getTypeMap()[sig->newId()];
  for( callvec::iterator it = calls.begin(); it!=calls.end(); ++it) {
    r = (*it)->call();
    if ( r->newId() > UNIQUE )
      if ( p->language() & PDB::LA_C_or_CXX ) {
        (*it)->call(p->getCRoutineMap()[r->newId()]);
      } else if ( p->language() & PDB::LA_FORTRAN ) {
        (*it)->call(p->getFRoutineMap()[r->newId()]);
      }
    if ( (fi=(*it)->file()) && (fi->newId() > UNIQUE) )
      (*it)->file(p->getFileMap()[fi->newId()]);
  }
  if ( (fi=firstExecStmtLocation().file()) && (fi->newId() > UNIQUE) )
    festmt.file(p->getFileMap()[fi->newId()]);
  for( locvec::iterator rt = retrns.begin(); rt!=retrns.end(); ++rt) {
    if ( (fi=(*rt)->file()) && (fi->newId() > UNIQUE) )
      (*rt)->file(p->getFileMap()[fi->newId()]);
  }
  for( stmtvec::iterator st = stmts.begin(); st!=stmts.end(); ++st) {
    if ( *st ) {
      if ( (fi=(*st)->bg.file()) && (fi->newId() > UNIQUE) )
        (*st)->bg.file(p->getFileMap()[fi->newId()]);
      if ( (fi=(*st)->ed.file()) && (fi->newId() > UNIQUE) )
        (*st)->ed.file(p->getFileMap()[fi->newId()]);
    }
  }
}

pdbSimpleItem::dupl_t pdbRoutine::findDuplicate(pdbSimpleItem* r) {
  pdbRoutine* rhs = dynamic_cast<pdbRoutine*> (r);
  if ( fullName() == rhs->fullName() ) {
    if ( kind() == RO_EXT && rhs->kind() != RO_EXT )
      return OLDDUPL;
    else
      return NEWDUPL;
  } else {
      return NODUPL;
  }
}
