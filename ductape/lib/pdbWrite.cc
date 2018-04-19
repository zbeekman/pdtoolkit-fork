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
#include "pdbAll.h"

void PDB::write(ostream& ostr) {
  lang_t l;

  ostr << "<PDB 3.0>\n";
  if ( (l = language()) != LA_NA )
    ostr << "lang " << toName(l) << "\n";
  ostr << "\n";

  // One can use the following more simpler implementation
  // if printings all items in order is not necessary
  //
  // itemvec items = getItemVec();
  // for (itemvec::iterator it = items.begin(); it != items.end(); ++it)
  //   ostr << **it;

  filevec& f = getFileVec();
  for (filevec::iterator fi = f.begin(); fi != f.end(); ++fi)
    ostr << **fi;

  typevec& t = getTypeVec();
  for (typevec::iterator ty = t.begin(); ty != t.end(); ++ty)
    if ( !(*ty)->isGroup() ) ostr << **ty;

  classvec& c = getClassVec();
  for (classvec::iterator cl = c.begin(); cl != c.end(); ++cl)
    ostr << **cl;

  modulevec& m = getModuleVec();
  for (modulevec::iterator ml = m.begin(); ml != m.end(); ++ml)
    ostr << **ml;

  croutinevec& cr = getCRoutineVec();
  for (croutinevec::iterator cro = cr.begin(); cro != cr.end(); ++cro)
    ostr << **cro;

  froutinevec& fr = getFRoutineVec();
  for (froutinevec::iterator fro = fr.begin(); fro != fr.end(); ++fro)
    ostr << **fro;

  templatevec& u = getTemplateVec();
  for (templatevec::iterator te = u.begin(); te != u.end(); ++te)
    ostr << **te;

  namespacevec& n = getNamespaceVec();
  for (namespacevec::iterator na = n.begin(); na != n.end(); ++na)
    ostr << **na;

  macrovec& ma = getMacroVec();
  for (macrovec::iterator mi = ma.begin(); mi != ma.end(); ++mi)
    ostr << **mi;

  pragmavec& pr = getPragmaVec();
  for (pragmavec::iterator pi = pr.begin(); pi != pr.end(); ++pi)
    ostr << **pi;
}

bool PDB::write(char *fname) {
  ofstream of(fname);
  if (!of) {
    cerr << "Error: Cannot open '" << fname << "'" << endl;
    return false;
  }
  write(of);
  return true;
}
