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
#include "pdbClass.h"
#include "pdbGroupField.h"
#include "pdbType.h"

const int INACTIVE = 0, ACTIVE = 1;

static void printFuncTree(const pdbRoutine *r, int level) {
  r->flag(ACTIVE);
  pdbRoutine::callvec c = r->callees();
  for (pdbRoutine::callvec::iterator it=c.begin(); it!=c.end(); ++it) {
    const pdbRoutine *rr = (*it)->call();
    if ( level != 0 || rr->callees().size() ) {
      cout << setw((level-1)*5) << "";
      if ( level ) cout << "`--> ";
      cout << rr->fullName();
      //if ( rr->virtuality()==pdbItem::VI_VIRT ||
      //     rr->virtuality()==pdbItem::VI_PURE ) cout << " (VIRTUAL)";
      if ( (*it)->isVirtual() ) cout << " (VIRTUAL)";
      if ( rr->flag() == ACTIVE ) {
        cout << " ..." << endl;
      } else {
        cout << endl;
        printFuncTree(rr, level+1);
      }
    }
  }
  r->flag(INACTIVE);
}

static const char *toName(pdbItem::access_t v) {
  switch (v) {
  case pdbItem::AC_PUB : return "public";
  case pdbItem::AC_PRIV: return "private";
  case pdbItem::AC_PROT: return "protected";
  case pdbItem::AC_NA  :
  default              : return "NA";
  }
}

static void printAccess(const pdbClass* base, const pdbClass* drvd) {
  pdbClass::basevec b = drvd->baseClasses();
  for (pdbClass::basevec::iterator it=b.begin(); it!=b.end(); ++it) {
    if ( (*it)->base() == base ) {
      if ( (*it)->isVirtual() ) cout << "virtual ";
      cout << ::toName((*it)->access()) << " ";
      if ( (*it)->access() == pdbItem::AC_PUB ) cout << "(IS-A) ";
      break;
    }
  }
}

static const pdbClass *containsClass(const pdbType* ty) {
  switch ( ty->kind() ) {
  case pdbItem::TY_GROUP: return static_cast<const pdbClass*>(ty->isGroup());
  case pdbItem::TY_PTR  : return containsClass(ty->pointedToType());
  case pdbItem::TY_REF  : return containsClass(ty->referencedType());
  case pdbItem::TY_ARRAY: return containsClass(ty->elementType());
  case pdbItem::TY_TREF : return containsClass(ty->baseType());
  default               : return 0;
  }
}

static void printClassTree(const pdbClass *c, int level) {
  pdbClass::classvec i = c->derivedClasses();
  for (pdbClass::classvec::iterator cit=i.begin(); cit!=i.end(); ++cit) {
    cout << setw((level-1)*5) << "";
    if ( level ) cout << "^--- ";
    printAccess(c, (*cit));
    cout << (*cit)->fullName() << endl;
    printClassTree(*cit, level+1);
  }

  pdbClass::fieldvec f = c->dataMembers();
  for (pdbClass::fieldvec::iterator fit=f.begin(); fit!=f.end(); ++fit) {
    if ( (*fit)->kind() == pdbItem::M_VAR ||
         (*fit)->kind() == pdbItem::M_STATVAR ) {
      const pdbClass *cl = containsClass((*fit)->type());
      if ( cl ) {
        cout << setw((level-1)*5) << "" << "`--> (HAS-A) ";
        if ( (*fit)->type()->isGroup() )
          cout << cl->fullName() << endl;
        else
          cout << (*fit)->type()->fullName() << endl;
      }
    }
  }
}

static void printFileTree(const pdbFile *f, int level) {
  pdbFile::incvec i = f->includes();
  for (pdbFile::incvec::iterator it=i.begin(); it!=i.end(); ++it) {
    cout << setw((level-1)*5) << "";
    if ( level ) cout << "`--> ";
    cout << (*it)->name() << endl;
    printFileTree(*it, level+1);
  }
}

int main(int argc, char *argv[]) {
  bool Aflag = false;
  bool Cflag = false;
  bool Rflag = false;
  bool Sflag = false;
  bool errflag = argc < 2;
  int ch;

  while ( (ch = getopt(argc, argv, "CRS")) != -1 ) {
    switch (ch) {
    case 'C': Cflag = true;     break;
    case 'R': Rflag = true;     break;
    case 'S': Sflag = true;     break;
    case '?': errflag = true;   break;
    }
  }
  if ( errflag ) {
    cerr << "usage: " << argv[0] << " [-CRS] pdbfile" << endl;
    cerr << "pdbtree   : Prints the source file inclusion tree, class hierarchy (IS-A +\n"
            "HAS-A), and function call graph. Usage:\n\n"

            "  pdbtree [-CRS] pdbfile\n\n"

            "-C            Print only the (C)lass hierarchy\n\n"

            "-R            Print only the (R)outine call graph\n\n"

            "-S            Print only the (S)ource file inclusion tree\n\n"

            "Note: Class hierarchy is a DAG, not a tree, and therefore display\n"
            "is bad.";

    return 1;
  }
  Aflag = !Rflag && !Cflag && !Sflag;

  PDB pdb(argv[optind]);

  if ( pdb ) {
    if ( Aflag || Rflag ) {
      pdbRoutine *topFunc = pdb.callTree();
      printFuncTree(topFunc, 0);
      cout << endl;
    }

    if ( Aflag || Cflag ) {
      pdbClass *topClass = pdb.classTree();
      printClassTree(topClass, 0);
      cout << endl;
    }

    if ( Aflag || Sflag ) {
      pdbFile *topFile = pdb.fileTree();
      printFileTree(topFile, 0);
      cout << endl;
    }
    return 0;
  }
  return 1;
}
