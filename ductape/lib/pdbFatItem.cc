/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#include "pdbFatItem.h"

#ifdef NO_INLINE
#  define inline
#  include "pdbFatItem.inl"
#endif

ostream& pdbFatItem::print(ostream& ostr) const {
  const char *X = attrPrefix();

  if ( (pos[0].line()+pos[0].col()+pos[1].line()+pos[1].col()+
        pos[2].line()+pos[2].col()+pos[3].line()+pos[3].col()) != 0 ) {
    ostr << X << "pos "
         << pos[0] << " " << pos[1] << " " << pos[2] << " " << pos[3] << "\n";
  }
  return ostr;
}

void pdbFatItem::adjustPtrs(PDB* p) {
  pdbItem::adjustPtrs(p);

  for (int i=0; i<4; ++i) {
    if ( pos[i].file() && (pos[i].file()->newId() > UNIQUE) )
      pos[i].file(p->getFileMap()[pos[i].file()->newId()]);
  }
}
