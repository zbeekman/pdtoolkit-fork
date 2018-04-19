#include "pdb.h"
#include "pdbCRoutine.h"
#include "pdbClass.h"
#include "pdbClassField.h"

int main(int argc, char *argv[]) {
  PDB p(argv[1]); if ( !p ) return 1;
  for (PDB::croutinevec::iterator r=p.getCRoutineVec().begin();
      r!=p.getCRoutineVec().end(); ++r)
    if ((*r)->isTemplate()) cout << (*r)->fullName() << endl;
  for (PDB::classvec::iterator c=p.getClassVec().begin();
      c!=p.getClassVec().end(); ++c) {
    if ((*c)->isTemplate()) cout << (*c)->fullName() << endl;
    pdbClass::fieldvec f=(*c)->dataMembers();
    for   (pdbClass::fieldvec::iterator m=f.begin(); m!=f.end(); ++m)
      if ((*m)->isTemplate()) cout << (*m)->fullName() << endl;
  }
  return 0;
}
