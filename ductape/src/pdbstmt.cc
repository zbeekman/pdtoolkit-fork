/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifdef _OLD_HEADER_
# include <iomanip.h>
#else
# include <iomanip>
  using std::setw;
#endif

#ifdef _MSC_VER
 #include "getopt.h"
#else
 #include <unistd.h>
#endif
#include "pdb.h"
#include "pdbRoutine.h"
#include "pdbCRoutine.h"
#include "pdbFRoutine.h"
#include "pdbType.h"

void printLoc(ostream& ostr, const pdbLoc& l) {
  if ( l.file() )
    ostr << " (" << setw(3) << l.line() << "," << setw(2) << l.col() << ")";
  else
    ostr << " (---,--)";
}

void printStmt(ostream& ostr, const pdbStmt *s, int level) {
  if ( !s ) return;
  pdbStmt::stmt_t k = s->kind();

  // prefix
#if (__GNUC__ == 2)
  ostr << "st#" << setw(2) << s->id() ;
#else
  ostr << "st#" << std::left << setw(2) << s->id() << std::right;
#endif /* GNUC */
  printLoc(ostr, s->stmtBegin());
  if ( k == pdbStmt::ST_BLOCK )
    ostr << "         ";
  else
    printLoc(ostr, s->stmtEnd());
  ostr << ":  ";
  ostr << setw(level*2) << "";

  // stmt kind
  if ( k == pdbStmt::ST_BLOCK ) ostr << "{\n";
  else {
    ostr << pdbStmt::toName(k);
    if ( k == pdbStmt::ST_GOTO || k == pdbStmt::ST_BREAK ||
         k == pdbStmt::ST_CONTINUE || k == pdbStmt::ST_FOR || k == pdbStmt::ST_UPC_FORALL) {
      if ( s->extraStmt() ) ostr << " -> st#" << s->extraStmt()->id();
    }
    ostr << "\n";
    if ( k == pdbStmt::ST_FOR ) printStmt(ostr, s->extraStmt(), level+4);
  }

  // down
  printStmt(ostr, s->downStmt(), level+1);

  // extra
  switch ( k ) {
  case pdbStmt::ST_GOTO:
  case pdbStmt::ST_BREAK:
  case pdbStmt::ST_CONTINUE:
  case pdbStmt::ST_FOR:
    // already processed above
    break;
  case pdbStmt::ST_BLOCK:
    ostr << "              ";
    printLoc(ostr, s->stmtEnd());
    ostr << ":  ";
    ostr << setw(level*2) << "" << "}\n";
    break;
#ifdef PDB_FORTRAN_EXTENDED_STATEMENTS_LEVEL_1
  case pdbStmt::ST_FGOTO:
    break; /* don't go into extraStmt for Fortran GOTO to avoid looping */
#endif /* PDB_FORTRAN_EXTENDED_STATEMENTS_LEVEL_1 */
#ifdef PDB_FORTRAN_EXTENDED_STATEMENTS_LEVEL_2
  case pdbStmt::ST_FEXIT:
  case pdbStmt::ST_FCYCLE:
  case pdbStmt::ST_FWHERE:
    break; /* don't go into extraStmt for Fortran EXIT to avoid looping */
#endif /* PDB_FORTRAN_EXTENDED_STATEMENTS_LEVEL_1 */

  default:
    printStmt(ostr, s->extraStmt(), level+1);
    break;
  }

  // next
  printStmt(ostr, s->nextStmt(), level);
}

void printReturns(ostream& ostr, const pdbRoutine* r) {
  const pdbRoutine::locvec&l = r->returnLocations();
  if ( l.size() ) {
    ostr << "--------------- \n";
    for (pdbRoutine::locvec::const_iterator lo = l.begin(); lo!=l.end(); ++lo) {
      ostr << "return ";
      printLoc(ostr, **lo);
      ostr << "\n";
    }
    ostr << "\n";
  }
}

int main(int argc, char *argv[]) {
  const pdbStmt* s;

  if ( argc != 2 ) {
    cerr << "usage: " << argv[0] << " pdbfile" << endl;
    return 1;
  }

  PDB pdb(argv[optind]);

  if ( pdb ) {
    if ( pdb.language() & PDB::LA_C_or_CXX ) {
      PDB::croutinevec& r = pdb.getCRoutineVec();
      for (PDB::croutinevec::iterator ro = r.begin(); ro != r.end(); ++ro) {
        if ( (s = (*ro)->body()) ) {
          cout << "--------------- " << (*ro)->fullName() << ":\n";
          printStmt(cout, s, 0);
          cout << "\n";
        }
	//printReturns(cout, *ro);
      }
    } else if ( pdb.language() & PDB::LA_FORTRAN ) {
      PDB::froutinevec& r = pdb.getFRoutineVec();
      for (PDB::froutinevec::iterator ro = r.begin(); ro != r.end(); ++ro) {
        if ( (s = (*ro)->body()) ) {
          cout << "--------------- " << (*ro)->fullName() << ":\n";
          printStmt(cout, s, 0);
          cout << "\n";
        }
	//printReturns(cout, *ro);
      }
    }

    return 0;
  }
  return 1;
}
