#ifdef _OLD_HEADER_
# include <iomanip.h>
#else
# include <iomanip>
  using std::setw;
#endif
#include "pdb.h"
#include "pdbFile.h"

static void printFileTree(const pdbFile *file, int level) {
  pdbFile::incvec f = file->includes();
  for (pdbFile::incvec::iterator i=f.begin(); i!=f.end(); ++i) {
    cout << setw(level*4) << "" << (*i)->name() << endl;
    printFileTree(*i, level+1);
  }
}
int main(int argc, char *argv[]) {
  PDB pdb(argv[1]);
  if ( !pdb ) return 1;

  printFileTree(pdb.fileTree(), 0);
  return 0;
}
