/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbItem.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbFile.inl"
#endif

ostream& operator<<(ostream& ostr, const pdbComment& c) {
  ostr << "co#" << c.id() << " " << PDB::toName(c.kind())
       << " " << c.cmtBegin() << " " << c.cmtEnd();
  return ostr << " " << c.text();
}

pdbComment* pdbFile::addComment(int id) {
  if ( id < 0 ) return 0;
  if ( id >= cmts.size() ) cmts.resize(id+1, 0);
  if ( ! cmts[id] ) cmts[id] = new pdbComment(id);
  return cmts[id];
}

pdbFile::~pdbFile() {
  for (int i=0; i<cmts.size(); ++i)
    delete cmts[i];
}

void pdbFile::process(PDB *p) {
  // if file wasn't included, add it to the toplevel files
  if ( numCalled() == 0 ) {
    p->fileTree()->addInclude(this);
  }
}

ostream& pdbFile::print(ostream& ostr) const {
  ostr << "so#" << id() << " " << name() << "\n";
  if ( isSystemFile() ) ostr << "ssys T\n";
  for (pdbFile::incvec::const_iterator it=incls.begin(); it!=incls.end(); ++it)
    ostr << "sinc so#" << (*it)->id() << "\n";
  for (int i=0; i<cmts.size(); ++i)
    if ( cmts[i] ) ostr << "scom " << *(cmts[i]) << "\n";

  return ostr << endl;
}

void pdbFile::adjustPtrs(PDB* p) {
  const pdbFile* fi;

  for (incvec::iterator it=incls.begin(); it!=incls.end(); ++it) {
    if ( (*it)->newId() > UNIQUE ) (*it) = p->getFileMap()[(*it)->newId()];
  }
  for( cmtvec::iterator ct = cmts.begin(); ct!=cmts.end(); ++ct) {
    if ( *ct ) {
      if ( (fi=(*ct)->bg.file()) && (fi->newId() > UNIQUE) )
        (*ct)->bg.file(p->getFileMap()[fi->newId()]);
      if ( (fi=(*ct)->ed.file()) && (fi->newId() > UNIQUE) )
        (*ct)->ed.file(p->getFileMap()[fi->newId()]);
    }
  }
}
