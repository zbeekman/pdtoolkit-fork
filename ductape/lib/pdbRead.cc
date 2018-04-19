/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifdef _OLD_HEADER_
# include <fstream.h>
# include <algo.h>
#else
# include <fstream>
  using std::ifstream;
  using std::ios;
# include <algorithm>
  using std::remove_if;
  using std::find_if;
#endif
#include <stdlib.h>
#include "pdbAll.h"

static const char *L1 = "~cggg";
static const char *L2 = "~lrrr";
static const char *MSG[] = { "", "Bad classID ", "Bad groupID ",
                             "Bad groupID ", "Bad groupID " };

static const char *fname = 0;
static int line = -1;
static bool ok = true;

void pdb_ERROR(const char *msg, const char *val) {
  cerr << "ERROR: " << fname << ":" << line << ": " << msg << val << endl;
  ok = false;
}

void pdb_ERROR(const char *msg, const char *item, int val) {
  cerr << "ERROR: " << fname << ":" << line << ": "
       << msg << " " << item << " "
       << item[0] << item[1] << "#" << val << endl;
  ok = false;
}

#include "pdbTraits.h"
#include "pdbTdefs.h"

static bool isDuplVec(const pdbSimpleItem* s) {
  return s->newId() > pdbSimpleItem::UNIQUE;
}

static bool isRoDuplMap(std::map<int, pdbCRoutine*>::value_type& r) {
  return (r.second)->newId() > pdbSimpleItem::UNIQUE;
}

static bool isClDuplMap(std::map<int, pdbClass*>::value_type& r) {
  return (r.second)->newId() > pdbSimpleItem::UNIQUE;
}


bool PDB::merge(char *other_fname) {
  {
    if ( ! (language() & LA_C_or_CXX) ) {
      cerr << "ERROR: " << fname <<  ": can only merge C or C++" << endl;
      return false;
    }

    PDB other(other_fname);
    if (!other) return false;

    if ( ! (other.language() & LA_C_or_CXX) ) {
      cerr << "ERROR: " << other_fname <<  ": can only merge C or C++" << endl;
      return false;
    }

    markDuplicates(typeTag(), other);
    markDuplicates(fileTag(), other);
    markDuplicates(classTag(), other);
    markDuplicates(croutineTag(), other);
    markDuplicates(templateTag(), other);
    markDuplicates(macroTag(), other);
    markDuplicates(pragmaTag(), other);
    markDuplicates(namespaceTag(), other);

    // update/merge callTree / fileTree / classTree
    for( pdbRoutine::callvec::iterator
         orit = other.topRoutine->callees().begin();
         orit!=other.topRoutine->callees().end(); ++orit) {
      const pdbRoutine *r = (*orit)->call();
      if ( r->newId() <= pdbSimpleItem::UNIQUE )
        topRoutine->addCallee(r, pdbItem::VI_NO, 0, 0, 0);
    }
    for( pdbFile::incvec::iterator ofit = other.topFile->includes().begin();
         ofit!=other.topFile->includes().end(); ++ofit) {
      if ( (*ofit)->newId() <= pdbSimpleItem::UNIQUE )
        topFile->addInclude(*ofit);
    }
    for( pdbClass::classvec::iterator
         ocit = other.topClass->derivedClasses().begin();
         ocit!=other.topClass->derivedClasses().end(); ++ocit) {
      if ( (*ocit)->newId() <= pdbSimpleItem::UNIQUE )
        topClass->addDerivedClass(*ocit);
    }

    // adjust pointer in items which ID didn't change
    for (itemvec::iterator it=itemVec.begin(); it!=itemVec.end(); ++it) {
      if ( (*it)->newId() <= pdbSimpleItem::UNIQUE ) (*it)->adjustPtrs(this);
    }

    // adjust pointers in callTree / fileTree / classTree too
    for( pdbRoutine::callvec::iterator rit = topRoutine->callees().begin();
         rit!=topRoutine->callees().end(); ++rit) {
      const pdbRoutine *r = (*rit)->call();
      if ( r->newId() > pdbSimpleItem::UNIQUE )
        (*rit)->call(getCRoutineMap()[r->newId()]);
    }
    for( pdbFile::incvec::iterator fit = topFile->includes().begin();
         fit!=topFile->includes().end(); ++fit) {
      if ( (*fit)->newId() > pdbSimpleItem::UNIQUE )
        *fit = getFileMap()[(*fit)->newId()];
    }
    for( pdbClass::classvec::iterator cit = topClass->derivedClasses().begin();
         cit!=topClass->derivedClasses().end(); ++cit) {
      if ( (*cit)->newId() > pdbSimpleItem::UNIQUE )
        *cit = getClassMap()[(*cit)->newId()];
    }

    // remove duplicates (in "this" PDB)
    // only need to do it for routines and classes
    itemVec.erase(remove_if(itemVec.begin(), itemVec.end(), isDuplVec),
                  itemVec.end());
    croutineVec.erase(remove_if(croutineVec.begin(), croutineVec.end(),
                                isDuplVec),
                     croutineVec.end());
    classVec.erase(remove_if(classVec.begin(), classVec.end(), isDuplVec),
                   classVec.end());

    croutinemap::iterator rrit = croutineMap.begin(), rnit;
    while ( (rrit = find_if(rrit, croutineMap.end(), isRoDuplMap))
            != croutineMap.end() ) {
      rnit = rrit;
      ++rrit;
      croutineMap.erase(rnit);
    }
    classmap::iterator ccit = classMap.begin(), cnit;
    while ( (ccit = find_if(ccit, classMap.end(), isClDuplMap))
            != classMap.end() ) {
      cnit = ccit;
      ++ccit;
      classMap.erase(cnit);
    }
  }

  // reset newId
  for (itemvec::iterator it=itemVec.begin(); it!=itemVec.end(); ++it) {
    (*it)->newId(pdbSimpleItem::UNIQUE);
  }

  return true;
}

PDB::attr_t PDB::toAttr(const char *v) {
  attrmap::iterator it = attr.find(v);
  if ( it == attr.end() ) {
    pdb_ERROR("Unknown attribute ", v);
    return NOATTR;
  }
  return (*it).second;
}

pdbType *PDB::getTypeOrGroup(const char *value) {
  if ( value[0] == 't' && value[1] == 'y' ) {
    return findItem(typeTag(), atoi(value+3));
  } else if ( value[0] == L1[version()] && value[1] == L2[version()] ) {
    pdbType *t = findItem(typeTag(), -atoi(value+3));
    t->name(value);
    t->kind(pdbItem::TY_GROUP);
    if ( language() & LA_C_or_CXX ) {
      t->isGroup(findItem(classTag(), atoi(value+3)));
    } else if ( language() & LA_FORTRAN ) {
      t->isGroup(findItem(moduleTag(), atoi(value+3)));
    }
    return t;
  } else {
    pdb_ERROR("Bad typeID ", value);
    return 0;
  }
}

bool PDB::getLocation(const char *value, char **ptr,
                      pdbFile*& file, int& line, int& col) {

  // skip leading blanks
  while ( *value == ' ' ) ++value;

  if ( value[0] == 's' && value[1] == 'o' ) {
    // looks like valid location
    int f = strtol(value+3, ptr, 10);
    line = strtol((*ptr)+1, ptr, 10);
    col  = strtol((*ptr)+1, ptr, 10);
    file = findItem(fileTag(), f);
  } else if (  value[0]=='N' && value[1]=='U' && value[2]=='L' ) {
    // looks like valid NUL location
    *ptr = const_cast<char*>(value) + 8;
    file = NULL;
    line = col = 0;
  } else if ( value[0] == '\0' ) {
    // nothing there => handle like NUL location
    file = NULL;
    line = col = 0;
  } else {
    // everything else is an error
    pdb_ERROR("Bad fileID ", value);
    return false;
  }
  return true;
}

bool PDB::getStmt(const char *value, char **ptr, int& st) {
  // skip leading blanks
  while ( *value == ' ' ) ++value;

  if ( value[0] == 's' && value[1] == 't' ) {
    // looks like valid statement
    st = strtol(value+3, ptr, 10);
  } else if (  value[0]=='N' && value[1]=='A' ) {
    // looks like valid NA statement
    *ptr = const_cast<char*>(value) + 2;
    st = -1;
  } else if ( value[0] == '\0' ) {
    // nothing there => handle like NA statement
    st = -1;
  } else {
    // everything else is an error
    pdb_ERROR("Bad statementID ", value);
    return false;
  }
  return true;
}

const int INBUF_SIZE=2048;

bool PDB::read(char *filename) {
  ifstream istr(filename);
  static char inbuf[INBUF_SIZE];
  bool expectNew = true;
  char *value;
  char currItem = '\0';
  long num;
  int l, c;
  pdbFile *f = 0;
  pdbItem *iptr = 0;
  pdbGroup *gptr = 0;
  pdbClass *cptr = 0;
  pdbModule *moptr = 0;
  pdbGroupField *gmptr = 0;
  pdbRoutine *rptr = 0;
  pdbCRoutine *crptr = 0;
  pdbFRoutine *frptr = 0;
  pdbTemplate *tptr = 0;
  pdbTemplateItem *tiptr = 0;
  pdbMacro *mptr = 0;
  pdbPragma *pptr = 0;
  pdbType *yptr = 0;
  pdbNamespace *nptr = 0;
  pdbFile *fptr = 0;
  pdbFatItem *fiptr = 0;

  fname = filename;
  line = 3;
  ok = true;
  inbuf[INBUF_SIZE-1] = '\n';

  if (!istr) {
    if ( fname )
      cerr << "ERROR: Cannot open '" << fname << "'" << endl;
    else
      cerr << "ERROR: Cannot open ''" << endl;
    return false;
  }

  // check magic number
  if ( !istr.getline(inbuf, INBUF_SIZE) ||
       ((strcmp(inbuf, "<PDB 1.0>")!=0) &&
	(strcmp(inbuf, "<PDB 2.0>")!=0)  &&
	(strcmp(inbuf, "<PDB 3.0>")!=0) &&
	(strcmp(inbuf, "<PDB 4.0>")!=0)) ) {
    cerr << "ERROR: Wrong magic number" << endl;
    return false;
  } else {
    versNum = inbuf[5] - '0';

    // read optional language specifier and skip empty line
    istr.getline(inbuf, INBUF_SIZE);
    if ( strncmp(inbuf, "lang", 4) == 0 ) {
      line = 2;
      lang = toLang(inbuf+5);
      istr.getline(inbuf, INBUF_SIZE);
      line = 4;
    } else if ( versNum == 1 ) {
      // Version 1 only supported C/C++
      lang = LA_C_or_CXX;
    }
  }
  
  while ( istr.getline(inbuf, INBUF_SIZE) || istr.gcount() ) {
    if ( inbuf[0] == '\0' ) {
      // empty line -> end of item
      // complete / check item
      expectNew = true;
      iptr = 0;
      cptr = 0;
      gmptr = 0;
      rptr = 0;
      tptr = 0;
      mptr = 0;
      pptr = 0;
      nptr = 0;
      fptr = 0;
      yptr = 0;
      tiptr = 0;
      fiptr = 0;

    } else if ( expectNew ) {
      // New Item
      char fst = currItem = inbuf[0];
      char snd = inbuf[1];
      num = strtol(inbuf+3, &value, 10);
      string name( (*value == '\0' || *(value+1) == '\0') ? "$NA$" : ++value);

      // check for very long lines
      while ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE) {
        inbuf[INBUF_SIZE-1] = '\n';
        if (istr.fail()) istr.clear(istr.rdstate() & ~ios::failbit);     
        istr.getline(inbuf, INBUF_SIZE);
        name += inbuf;
      }



      if ( fst=='r' && snd=='o' ) {
        if ( language() & LA_C_or_CXX ) {
          iptr = fiptr = tiptr = rptr = crptr =
                 findItem(croutineTag(), name, num);
        } else if ( language() & LA_FORTRAN ) {
          iptr = fiptr = tiptr = rptr = frptr =
                 findItem(froutineTag(), name, num);
        } else {
          cerr << "ERROR: Cannot determine source language" << endl;
          return false;
        }
      } else if ( fst=='s' && snd=='o' ) {
        fptr = findItem(fileTag(), name, num);
        
        // initialize language if necessary
        //   name *.[fF]*   => Fortran
        //   name *.h       => C or C++ (preset)
        //   name *.c       => C
        //   name *.[cC]*   => C++
        //   name *.[jJ]*   => Java
        //   name *.[uU]*   => UPC
        if ( language() == LA_NA || language() == LA_C_or_CXX ) {
          int dot = name.rfind('.');
          if ( dot != STR_NPOS  && (dot+1) != name.length() ) {
            switch ( name[dot+1] ) {
            case 'f': case 'F': lang = LA_FORTRAN;
                                break;
            case 'c': case 'C': if ( (dot+2) == name.length() ) {
                                  lang = ( name[dot+1] == 'c' ) ? LA_C : LA_CXX;
                                } else {
                                  lang = LA_CXX;
                                }
                                break;
            case 'j': case 'J': lang = LA_JAVA;
                                break;
            case 'h': lang = LA_C_or_CXX;
                                break;
            case 'u': case 'U': lang = LA_UPC;
            }
          }
        }
      } else if ( fst==L1[version()] && snd==L2[version()] ) {
        if ( language() & LA_C_or_CXX ) {
          iptr = fiptr = tiptr = gptr = cptr =
                 findItem(classTag(), name, num);
        } else if ( language() & LA_FORTRAN ) {
          iptr = fiptr = tiptr = gptr = moptr =
                 findItem(moduleTag(), name, num);
        } else {
          cerr << "ERROR: Cannot determine source language" << endl;
          return false;
        }
      } else if ( fst=='t' && snd=='y' ) {
        iptr = yptr = findItem(typeTag(), name, num);
        currItem = 'y';
      } else if ( fst=='t' && snd=='e' ) {
        iptr = fiptr = tptr = findItem(templateTag(), name, num);
        if ( language() == LA_C_or_CXX ) lang = LA_CXX;
      } else if ( fst=='m' && snd=='a' ) {
        iptr = mptr = findItem(macroTag(), name, num);
      } else if ( fst=='p' && snd=='r' ) {
        iptr = pptr = findItem(pragmaTag(), name, num);
      } else if ( fst=='n' && snd=='a' ) {
        iptr = fiptr = nptr = findItem(namespaceTag(), name, num);
        if ( language() == LA_C_or_CXX ) lang = LA_CXX;
      } else {
        pdb_ERROR("Unknown item ", inbuf);
        currItem = '\0';
      }
      expectNew = false;

    } else {
      // Attributes
      char *ptr = strchr(inbuf, ' ');
      *ptr = '\0';
      value = ptr+1;
      attr_t a = toAttr(inbuf);
      if ( currItem != inbuf[0] ) {
        pdb_ERROR("Non-matching attribute ", inbuf);
      } else {
        switch (a) {
        // ==== Common Attributes =======================================
        case ACS:      // ---- Access
          if (iptr) iptr->access(pdbItem::toAccess(value));
          break;
        case CLASS:    // ---- Parent Class
	  if ( value[0] == 'c' && value[1] == 'l' ) {
            if (iptr) iptr->parentGroup(findItem(classTag(), atoi(value+3)));
	  } else pdb_ERROR("Bad classID ", value);
          break;
        case GROUP:    // ---- Parent Group
	  if ( value[0] == 'g' && value[1] == 'r' ) {
            if ( language() & LA_C_or_CXX ) {
              if (iptr) iptr->parentGroup(findItem(classTag(), atoi(value+3)));
            } else if ( language() & LA_FORTRAN ) {
              if (iptr) iptr->parentGroup(findItem(moduleTag(), atoi(value+3)));
            }
	  } else pdb_ERROR("Bad groupID ", value);
          break;
        case LOC:      // ---- Location: fileId Line Column
          if ( getLocation(value, &ptr, f, l, c) && iptr )
            iptr->location(pdbLoc(f, l, c));
          break;
        case POS: {    // ---- Position: loc1 loc2 loc3 loc4
                       // ----           locN := file line col
            if ( getLocation(value, &ptr, f, l, c) )
              fiptr->headBegin(pdbLoc(f, l, c));
            if ( getLocation(ptr, &ptr, f, l, c) )
              fiptr->headEnd  (pdbLoc(f, l, c));
            if ( getLocation(ptr, &ptr, f, l, c) )
              fiptr->bodyBegin(pdbLoc(f, l, c));
            if ( getLocation(ptr, &ptr, f, l, c) )
              fiptr->bodyEnd  (pdbLoc(f, l, c));
          }
          break;
        case NSPACE:   // ---- Parent Namespace
	  if ( value[0] == 'n' && value[1] == 'a' ) {
            if (iptr)
              iptr->parentNSpace(findItem(namespaceTag(), atoi(value+3)));
	  } else pdb_ERROR("Bad namespaceID ", value);
          break;

        // ==== Common Template Item Attributes =========================
        case TEMPL:    // ---- Template
	  if ( value[0] == 't' && value[1] == 'e' ) {
            if (tiptr)
              tiptr->isTemplate(findItem(templateTag(), atoi(value+3)));
	  } else if ( value[0] == 'T' ) {
            if (tiptr)
              tiptr->isTemplate(pdbTemplateItem::TE_UNKNOWN);
          } else pdb_ERROR("Bad templateID ", value);
          break;
        case SPECL:    // ---- Is Specialized?
          if (tiptr) tiptr->isSpecialized(value[0] == 'T');
          break;
        case GSPARAM:  // ---- Template Specialization Parameter (Classes only)
          if (tiptr) {
            ptr = strtok(value, " ");
            pdbTemplateArg::targ_t t = pdbTemplateArg::toTarg(value);
            pdbTemplateArg arg(t, true);
            ptr = strtok(NULL, " ");
            switch ( t ) {
            case pdbTemplateArg::TA_TYPE:
              arg.type(getTypeOrGroup(ptr));
              break;
            case pdbTemplateArg::TA_NONTYPE:
              arg.value(ptr);
              break;
            case pdbTemplateArg::TA_TEMPL:
	      if ( ptr[0] == 't' && ptr[1] == 'e' ) {
                int r = strtol(ptr+3, &ptr, 10);
                arg.templateArg(findItem(templateTag(), r));
	      } else pdb_ERROR("Bad templateID ", ptr);
              break;
            case pdbTemplateArg::TA_NA:
              // ignore
              break;
            }
            tiptr->addSpeclArgument(arg);
          }
          break;

        // ==== Class (1.0) / Group (2.0) Attributes ====================
        case GBASE:
        case CBASE: {  // ---- Direct Base Class
          ptr = strtok(value, " ");
          pdbItem::virt_t v = pdbItem::toVirt(ptr);
          ptr = strtok(NULL, " ");
          pdbItem::access_t a = pdbItem::toAccess(ptr);
          ptr = strtok(NULL, "");
	  if ( ptr[0] == L1[version()] && ptr[1] == L2[version()] ) {
            if (cptr) {
              int b = strtol(ptr+3, &ptr, 10);
              if ( getLocation(ptr, &ptr, f, l, c) ) {
                pdbClass *cb = findItem(classTag(), b);
                cptr->addBaseClass(v, a, cb, f, l, c);
                cb->addDerivedClass(cptr);
              }
            }
	  } else pdb_ERROR(MSG[version()], ptr);
          }
          break;
        case GFUNC:    // ---- Member or Module Function (2.0)
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            if (cptr || moptr) {
              int r = strtol(value+3, &ptr, 10);
              if ( getLocation(ptr, &ptr, f, l, c) ) {
                if ( language() & LA_C_or_CXX ) {
                  if (cptr)
                    cptr->addMethod(findItem(croutineTag(), r), f, l, c);
                } else if ( language() & LA_FORTRAN ) {
                  if (moptr)
                    moptr->addFuncMember(findItem(froutineTag(), r), f, l, c);
                }
              }
            }
	  } else pdb_ERROR("Bad routineID ", value);
          break;
        case CFUNC:    // ---- Member Function (1.0)
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            if (cptr) {
              int r = strtol(value+3, &ptr, 10);
              if ( getLocation(ptr+1, &ptr, f, l, c) ) {
                cptr->addMethod(findItem(croutineTag(), r), f, l, c);
              }
            }
	  } else pdb_ERROR("Bad routineID ", value);
          break;
        case GKIND:    // ---- Group Kind
        case CKIND:    // ---- Class Kind
          if (gptr) {
            gptr->kind(pdbItem::toGroup(value));
            if ( language() == LA_C_or_CXX &&
                 gptr->kind() == pdbItem::GR_CLASS ) lang = LA_CXX;
          }
          break;
        case GMEM:
        case CMEM: {   // ---- Data Member
          string name(value);

          // check for very long lines
          while ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE) {
            inbuf[INBUF_SIZE-1] = '\n';
            if (istr.fail()) istr.clear(istr.rdstate() & ~ios::failbit);     
            istr.getline(inbuf, INBUF_SIZE);
            name += inbuf;
          }

          iptr = tiptr = gmptr = new pdbGroupField(name);
          if (gptr) {
            gptr->addDataMember(gmptr);
            gmptr->parentGroup(gptr);
          }
          }
          break;
       case GFRGROUP:
       case CFRCLASS: // ---- Friend Class
	  if ( value[0] == L1[version()] && value[1] == L2[version()] ) {
            if (cptr) {
              int b = strtol(value+3, &ptr, 10);
              if ( getLocation(ptr, &ptr, f, l, c) ) {
                cptr->addFriendClass(findItem(classTag(), b), f, l, c);
              }
            }
	  } else pdb_ERROR(MSG[version()], value);
          break;
        case GFRFUNC:
        case CFRFUNC:  // ---- Friend Function
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            if (cptr) {
              int b = strtol(value+3, &ptr, 10);
              if ( getLocation(ptr, &ptr, f, l, c) ) {
                cptr->addFriendRoutine(findItem(croutineTag(), b), f, l, c);
              }
            }
	  } else pdb_ERROR(MSG[version()], value);
          break;

        // ==== Class (1.0) / Group (2.0) Field Attributes ==============
        case GMKIND:   // ---- Group Field Kind
        case CMKIND:   // ---- Class Field Kind
          if (gmptr) gmptr->kind(pdbItem::toMem(value));
          break;
        case GMISBIT:
        case CMISBIT:  // ---- Is Bitfield?
          if (gmptr) gmptr->isBitField(value[0] == 'T');
          break;
        case GMTYPE:
        case CMTYPE:   // ---- type
          if (gmptr) gmptr->type(getTypeOrGroup(value));
          break;
        case GMCONST:
        case CMCONST:  // ---- Is Static Constant?
          if (gmptr) gmptr->isStaticConst(value[0] == 'T');
          break;
        case GMMUT:
        case CMMUT:    // ---- Is Mutable?
          if (gmptr) gmptr->isMutable(value[0] == 'T');
          break;

        // ==== File Attributes =========================================
        case SCOM:     // ---- comment
	  if ( value[0] == 'c' && value[1] == 'o' ) {
	    pdbComment* cmt = fptr->addComment(strtol(value+3, &ptr, 10));
	    ptr = strtok(ptr+1, " ");
	    cmt->kind(PDB::toLang(ptr));
	    ptr = strtok(NULL, "");
	    if ( getLocation(ptr, &ptr, f, l, c) ) {
              cmt->cmtBegin(pdbLoc(f, l, c));
              if ( getLocation(ptr, &ptr, f, l, c) ) {
                cmt->cmtEnd(pdbLoc(f, l, c));
	      }
	      string txt(ptr+1);
	      while ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE ) {
                inbuf[INBUF_SIZE-1] = '\n';
                if (istr.fail()) istr.clear(istr.rdstate() & ~ios::failbit);
                istr.getline(inbuf, INBUF_SIZE);
                txt += inbuf;
              }
              cmt->text(txt);
	    }
	  } else pdb_ERROR("Bad commentID ", value);
	  break;
        case SINC:     // ---- File Include
	  if ( value[0] == 's' && value[1] == 'o' ) {
            if (fptr) {
              fptr->addInclude(findItem(fileTag(), atoi(value+3)));
            }
	  } else pdb_ERROR("Bad fileID ", value);
          break;
        case SSYS:     // ---- is System File?
          if (fptr) fptr->isSystemFile(value[0] == 'T');
          break;

        // ==== Macro Attributes ========================================
        case MKIND:    // ---- Macro Kind
          if (mptr) mptr->kind(pdbItem::toMacro(value));
          break;
        case MTEXT:    // ---- Macro Text
          if (mptr) {
            string txt(value);
            while ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE ) {
              inbuf[INBUF_SIZE-1] = '\n';
              if (istr.fail()) istr.clear(istr.rdstate() & ~ios::failbit);     
              istr.getline(inbuf, INBUF_SIZE);
              txt += inbuf;
            }
            mptr->text(txt);
          }
          break;

        // ==== Pragma Attributes =======================================
        case PKIND:    // ---- Pragma Kind
          if (pptr) pptr->kind(value);
          break;
        case PPOS:     // ---- Pragma Position
          if (pptr) {
            if ( getLocation(value, &ptr, f, l, c) ) {
              pptr->prBegin(pdbLoc(f, l, c));
              if ( getLocation(ptr, &ptr, f, l, c) )
                pptr->prEnd(pdbLoc(f, l, c));
	    }
	  }
          break;
        case PTEXT:    // ---- Pragma Text
          if (pptr) {
            string txt(value);
            while ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE ) {
              inbuf[INBUF_SIZE-1] = '\n';
              if (istr.fail()) istr.clear(istr.rdstate() & ~ios::failbit);     
              istr.getline(inbuf, INBUF_SIZE);
              txt += inbuf;
            }
            pptr->text(txt);
          }
          break;

	
        // ==== Routine Attributes ======================================
        case RBODY:    // ---- Statement which makes up routine body
	  if ( value[0] == 's' && value[1] == 't' ) {
            if (rptr) rptr->body(rptr->addStmt(strtol(value+3, &ptr, 10)));
	  } else pdb_ERROR("Bad statementID ", value);
          break;
        case RSTMT:    // ---- Routine Statement
	  if ( value[0] == 's' && value[1] == 't' ) {
            pdbStmt* s = rptr->addStmt(strtol(value+3, &ptr, 10));
            ptr = strtok(ptr+1, " ");
            s->kind(pdbStmt::toStmt(ptr));
            ptr = strtok(NULL, "");
            if ( getLocation(ptr, &ptr, f, l, c) ) {
              s->stmtBegin(pdbLoc(f, l, c));
              if ( getLocation(ptr, &ptr, f, l, c) ) {
                s->stmtEnd(pdbLoc(f, l, c));
                int sno = -1;
                if ( getStmt(ptr, &ptr, sno) ) {
                  s->nextStmt(rptr->addStmt(sno));
                  if ( getStmt(ptr, &ptr, sno) ) {
                    s->downStmt(rptr->addStmt(sno));
                    if ( getStmt(ptr, &ptr, sno) ) {
                      s->extraStmt(rptr->addStmt(sno));
                      if ( getStmt(ptr, &ptr, sno) ) {
                        s->affinityStmt(rptr->addStmt(sno));
                      }
                    }
                  }
                }
              }
            }
	  } else pdb_ERROR("Bad statementID ", value);
          break;
        case RCALL:    // ---- Calls Routine
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            int r = strtol(value+3, &ptr, 10);
            ptr = strtok(ptr+1, " ");
            pdbItem::virt_t v = pdbItem::toVirt(ptr);
            ptr = strtok(NULL, "");
            if ( getLocation(ptr, &ptr, f, l, c) && rptr ) {
              if ( language() & LA_C_or_CXX ) {
                rptr->addCallee(findItem(croutineTag(), r), v, f, l, c);
              } else if ( language() & LA_FORTRAN ) {
                rptr->addCallee(findItem(froutineTag(), r), v, f, l, c);
              }
            }
	  } else pdb_ERROR("Bad routineID ", value);
          break;
        case RCRVO:    // ---- Covariant Return Virtual Override?
          if (crptr) crptr->covariantReturnVirtualOverride(value[0] == 'T');
          break;
        case RINLINE:  // ---- Is Inline?
          if (crptr) crptr->isInline(value[0] == 'T');
          break;
        case RKIND:    // ---- Special Kind PDB 1.0
                       // ---- Kind PDB 2.0
          if ( version() < 2 ) {
            if (rptr) rptr->specialKind(pdbItem::toRSpecial(value));
          } else {
            if (rptr) rptr->kind(pdbItem::toRoutine(value));
          }
          break;
	case RFPREFIX:
	  if (rptr) rptr->fprefix(pdbItem::toFPrefix(value));
	  break;
        case RSKIND:   // ---- Special Kind PDB 2.0
          if (rptr) rptr->specialKind(pdbItem::toRSpecial(value));
          break;
        case RSIG:     // ---- Signature
	  if ( value[0] == 't' && value[1] == 'y' ) {
            if (rptr) rptr->signature(findItem(typeTag(), atoi(value+3)));
	  } else pdb_ERROR("Bad typeID ", value);
          break;
        case RLINK:    // ---- Linkage
          if (rptr) rptr->linkage(pdbItem::toLink(value));
          break;
        case RSTAT:  // ---- Is Static?
          if (crptr) crptr->isStatic(value[0] == 'T');
          break;
        case RSTORE:   // ---- Storage Class PDB 1.0 (now Kind)
          if (rptr) rptr->kind(pdbItem::toRoutine(value));
          break;
        case RVIRT:    // ---- Virtuality
          if (crptr) crptr->virtuality(pdbItem::toVirt(value));
          break;
        case RCGEN:    // ---- Is Compiler Generated?
          if (crptr) crptr->isCompilerGenerated(value[0] == 'T');
          break;
        case REXPL:    // ---- Is Explicit Constructor?
          if (crptr) crptr->isExplicitCtor(value[0] == 'T');
          break;
        case RSTART:   // ---- First Executable Stmt Location
          if ( getLocation(value, &ptr, f, l, c) && rptr )
            rptr->firstExecStmtLocation(pdbLoc(f, l, c));
          break;
        case RRET:     // ---- Return Stmt Location
          if ( getLocation(value, &ptr, f, l, c) && rptr )
            rptr->addReturnLocation(f, l, c);
          break;

        case RSTOP:    // ---- Stop Stmt Loctaion (f90)
          if ( getLocation(value, &ptr, f, l, c) && frptr )
            frptr->addStopLocation(f, l, c);
          break;
        case RROUT:    // ---- Parent Routine (f90)
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            if (frptr) {
              int r = strtol(value+3, &ptr, 10);
              frptr->parentRoutine(findItem(froutineTag(), r));
            }
	  } else pdb_ERROR("Bad routineID ", value);
          break;
        case RALIAS:   // ---- Alias (f90)
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            if (frptr) {
              int r = strtol(value+3, &ptr, 10);
              frptr->isAliasFor(findItem(froutineTag(), r));
            }
	  } else pdb_ERROR("Bad routineID ", value);
          break;
        case RIMPL: // ---- Implements (f90)
	  if ( value[0] == 'r' && value[1] == 'o' ) {
            if (frptr) {
              int r = strtol(value+3, &ptr, 10);
              frptr->addImplementation(findItem(froutineTag(), r));
            }
	  } else pdb_ERROR("Bad routineID ", value);
          break;
        case RREC:     // ---- Is Recursive? (f90)
          if (frptr) frptr->isRecursive(value[0] == 'T');
          break;
        case RISELEM:  // ---- Is Elemental? (f90)
          if (frptr) frptr->isElemental(value[0] == 'T');
          break;
        case RARGINFO: // ---- Has Explicit Interface? (f90)
          if (frptr) frptr->hasExplicitInterface(value[0] == 'T');
          break;
        case RCATCH: { // ---- FunctionTry Block Position: loc1 loc2 loc3 loc4
                       // ----                      locN := file line col
            if ( getLocation(value, &ptr, f, l, c) )
              ; // ignore for now
            if ( getLocation(ptr, &ptr, f, l, c) )
              ; // ignore for now
            if ( getLocation(ptr, &ptr, f, l, c) )
              ; // ignore for now
            if ( getLocation(ptr, &ptr, f, l, c) )
              ; // ignore for now
          }
          break;

        // ==== Template Attributes =====================================
        case TKIND:    // ---- Template Kind
          if (tptr) tptr->kind(pdbItem::toTempl(value));
          break;
        case TDECL:    // ---- Template Declaration
	  if ( value[0] == 't' && value[1] == 'e' ) {
            if (tptr)
              tptr->declaration(findItem(templateTag(), atoi(value+3)));
          } else pdb_ERROR("Bad templateID ", value);
          break;
        case TDEF:     // ---- Template Definition
	  if ( value[0] == 't' && value[1] == 'e' ) {
            if (tptr)
              tptr->definition(findItem(templateTag(), atoi(value+3)));
          } else pdb_ERROR("Bad templateID ", value);
          break;
        case TTEXT:    // ---- Template Text
          if (tptr) {
            string txt(value);
            while ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE) {
              inbuf[INBUF_SIZE-1] = '\n';
              if (istr.fail()) istr.clear(istr.rdstate() & ~ios::failbit);     
              istr.getline(inbuf, INBUF_SIZE);
              txt += inbuf;
            }
            tptr->text(txt);
          }
          break;
        case TPARAM:   // ---- Template Parameter
          if (tptr) {
            ptr = strtok(value, " ");
            pdbTemplateArg::targ_t t = pdbTemplateArg::toTarg(value);
            pdbTemplateArg arg(t);
            ptr = strtok(NULL, " ");
            switch ( t ) {
            case pdbTemplateArg::TA_TYPE: {
	      if ( ptr[0] == 't' && ptr[1] == 'y' ) {
                int r = strtol(ptr+3, &ptr, 10);
                arg.type(findItem(typeTag(), r));
	      } else pdb_ERROR("Bad typeID ", ptr);
              if ( (ptr = strtok(NULL, " ")) )
                arg.defaultType(getTypeOrGroup(ptr));
              }
              break;
            case pdbTemplateArg::TA_NONTYPE: {
	      if ( ptr[0] == 't' && ptr[1] == 'y' ) {
                int r = strtol(ptr+3, &ptr, 10);
                arg.type(findItem(typeTag(), r));
	      } else pdb_ERROR("Bad typeID ", ptr);
              ptr = strtok(NULL, " ");
              arg.name(ptr);
              if ( (ptr = strtok(NULL, " ")) ) arg.defaultValue(ptr);
              }
              break;
            case pdbTemplateArg::TA_TEMPL: {
	      if ( ptr[0] == 't' && ptr[1] == 'e' ) {
                int r = strtol(ptr+3, &ptr, 10);
                arg.templateArg(findItem(templateTag(), r));
	      } else pdb_ERROR("Bad templateID ", ptr);
              if ( (ptr = strtok(NULL, " " )) ) {
	        if ( ptr[0] == 't' && ptr[1] == 'e' ) {
                  int r = strtol(ptr+3, &ptr, 10);
                  arg.defaultTemplateArg(findItem(templateTag(), r));
	        } else pdb_ERROR("Bad templateID ", ptr);
              }
              }
              break;
            case pdbTemplateArg::TA_NA:
              // ignore
              break;
            }
            tptr->addArgument(arg);
          }
          break;
        case TSPARAM:  // ---- Template Specialization Parameter
          if (tptr) {
            ptr = strtok(value, " ");
            pdbTemplateArg::targ_t t = pdbTemplateArg::toTarg(value);
            pdbTemplateArg arg(t, true);
            ptr = strtok(NULL, " ");
            switch ( t ) {
            case pdbTemplateArg::TA_TYPE:
              arg.type(getTypeOrGroup(ptr));
              break;
            case pdbTemplateArg::TA_NONTYPE:
              arg.value(ptr);
              break;
            case pdbTemplateArg::TA_TEMPL:
	      if ( ptr[0] == 't' && ptr[1] == 'e' ) {
                int r = strtol(ptr+3, &ptr, 10);
                arg.templateArg(findItem(templateTag(), r));
	      } else pdb_ERROR("Bad templateID ", ptr);
              break;
            case pdbTemplateArg::TA_NA:
              // ignore
              break;
            }
            tptr->addSpeclArgument(arg);
          }
          break;
        case TPROTO:   // ---- Template Prototype Instantiation
          if (tptr && !(value[0]=='N' && value[1]=='U' && value[2]=='L')) {
            if (tptr->kind() == pdbItem::TE_FUNC ||
                tptr->kind() == pdbItem::TE_MEMFUNC ) {
	      if ( value[0] == 'r' && value[1] == 'o' ) {
                int r = strtol(value+3, &ptr, 10);
                tptr->funcProtoInst(findItem(croutineTag(), r));
	      } else pdb_ERROR("Bad routineID ", value);
            } else if (tptr->kind() == pdbItem::TE_CLASS ||
                       tptr->kind() == pdbItem::TE_MEMCLASS ) {
	      if ( value[0] == 'g' && value[1] == 'r' ) {
                int r = strtol(value+3, &ptr, 10);
                tptr->classProtoInst(findItem(classTag(), r));
	      } else pdb_ERROR("Bad groupID ", value);
            } else pdb_ERROR("invalid attribute tproto", value);
          }
          break;
        case TTYPE:    // ---- Template Static Member Type
          if (tptr) {
            tptr->statMemType(getTypeOrGroup(value));
          }
          break;

        // ==== Namespace Attributes ====================================
        case NALIAS:   // ---- Alias Name
	  if ( value[0] == 'n' && value[1] == 'a' ) {
            if (nptr) nptr->isAlias(findItem(namespaceTag(), atoi(value+3)));
	  } else pdb_ERROR("Bad namespaceID ", value);
          break;
        case NMEM:     // ---- Namespace Member
          if ( value[0]=='r' && value[1]=='o' ) {
            if (nptr) nptr->addMember(findItem(croutineTag(), atoi(value+3)));
          } else if ( value[0]==L1[version()] && value[1]==L2[version()] ) {
            if (nptr) nptr->addMember(findItem(classTag(), atoi(value+3)));
          } else if ( value[0]=='t' && value[1]=='y' ) {
            if (nptr) nptr->addMember(findItem(typeTag(), atoi(value+3)));
          } else if ( value[0]=='t' && value[1]=='e' ) {
            if (nptr) nptr->addMember(findItem(templateTag(), atoi(value+3)));
          } else if ( value[0]=='n' && value[1]=='a' ) {
            if (nptr) nptr->addMember(findItem(namespaceTag(), atoi(value+3)));
          } else {
            pdb_ERROR("Unknown item ", value);
          }
          break;

        // ==== Type Attributes =========================================
        case YELEM:    // ---- Element Type (if Array)
          if (yptr) yptr->elementType(getTypeOrGroup(value));
          break;
        case YNELEM:   // ---- Number of Elements (if Array)
          if (yptr) yptr->numElements(atoi(value));
          break;
        case YSTAT:    // ---- is Static Array (C99)
          if (yptr) yptr->isStaticArray(value[0] == 'T');
          break;
        case YFKIND:   // ---- Float Kind (if Float)
          if (yptr) yptr->floatType(pdbItem::toFloat(value));
          break;
        case YIKIND:   // ---- Integer Kind (if Integer)
          if (yptr) yptr->integerType(pdbItem::toInt(value));
          break;
        case YKIND:    // ---- Kind of Type
          if (yptr) yptr->kind(pdbItem::toType(value));
          break;
        case YQUAL:    // ---- Type Qualifiers
          ptr = strtok(value, " ");
          while ( ptr != NULL ) {
            if (yptr) yptr->addQualifier(pdbItem::toQual(ptr));
            ptr = strtok(NULL, " ");
          }
          break;
        case YPTR:     // ---- Type pointed to (if Pointer)
          if (yptr) yptr->pointedToType(getTypeOrGroup(value));
          break;
        case YREF:     // ---- Type referenced (if Reference)
          if (yptr) yptr->referencedType(getTypeOrGroup(value));
          break;
        case YOLD:     // ---- pre-ANSI "C" Declaration Style?
          if (yptr) yptr->oldStyleDecl(value[0] == 'T');
          break;
        case YSIGNED:  // ---- is explicitly Signed?
          if (yptr) yptr->isSigned(value[0] == 'T');
          break;
        case YTREF:    // ---- Base Type (if Typedef or Qualified Type)
          if (yptr) yptr->baseType(getTypeOrGroup(value));
          break;
        case YRETT:    // ---- Return Type
          if (yptr) yptr->returnType(getTypeOrGroup(value));
          break;
        case YARGT:    // ---- Argument Type
          if (yptr) {
            ptr = strtok(value, " ");
            if ( version() < 2 ) {
              ptr = strtok(NULL, " ");
              yptr->addArgument(getTypeOrGroup(value), "-", NULL, 0, 0,
                       (ptr && ptr[0]=='T') ? pdbArg::ARG_DEF : pdbArg::ARG_NA);
            } else {
              ptr = strtok(NULL, " ");
              string name(ptr);
              if ( ((ptr = strtok(NULL, "")) != NULL) &&
                   (ptr[0] == 's' || ptr[0] == 'N')) {
                getLocation(ptr, &ptr, f, l, c);
              } else {
                f = 0;
                l = c = 0;
              }
              int flg = pdbArg::ARG_NA;
              char* p = ptr;
              while ( (ptr = strtok(p, " ")) != NULL ) {
                flg |= pdbArg::toArg(ptr);
                p = NULL;
              }
              yptr->addArgument(getTypeOrGroup(value), name, f, l, c, flg);
            }
          }
          break;
        case YELLIP:   // ---- Has Ellipsis?
          if (yptr) yptr->hasEllipsis(value[0] == 'T');
          break;
        case YMPGROUP: // ---- Member Pointer Parent Class
	  if ( value[0] == L1[version()] && value[1] == L2[version()] ) {
            if (yptr)
              yptr->memberPointerClass(findItem(classTag(), atoi(value+3)));
	  } else pdb_ERROR(MSG[version()], value);
          break;
        case YMPTYPE:  // ---- Member Pointer Type
          if (yptr) yptr->memberPointerType(getTypeOrGroup(value));
          break;
        case YEXCEP:   // ---- Exception Specification
          if (yptr) {
            if ( strcmp(value, "NULL") == 0 ) {
              yptr->addExceptionSpec(pdbType::NULLEXCEP);
	    } else {
              yptr->addExceptionSpec(getTypeOrGroup(value));
	    }
	  }
          break;
        case YENUM:    // ---- Enumeration Value
          ptr = strtok(value, " ");
          ptr = strtok(NULL, " ");
          *(ptr-1) = '\0';
          yptr->addEnumeration(pdbEnum(value, strtol(ptr, 0, 0)));
          break;

        case YCLEN:    // ---- Number of Characters (if f90 fchar)
          if (yptr) {
            if ( value[0] == '*' )
              yptr->numElements(-1);
            else
              yptr->numElements(atoi(value));
          }
          break;
        case YSHAPE:   // ---- Shape (if f90 array)
          if (yptr) yptr->shape(pdbItem::toShape(value));
          break;
        case YRANK:    // ---- Rank (if f90 array)
          if (yptr) yptr->rank(atoi(value));
          break;
        case YDIM:     // ---- bounds of Dimension (if f90 array)
          if (yptr) {  /*??? correct handlin of '*' and 'NA' */
            int l, u;
            if ( value[0] == '*' ) {
              // lower bound cannot be '*' - code here just for security
              l=pdbBounds::UNKNOWN; ptr=value+2;
            } else if ( value[0] == 'N' && value[1] == 'A' ) {
              l=pdbBounds::UNKNOWN; ptr=value+3;
            } else {
              l = strtol(value, &ptr, 10); ++ptr;
            }
            if ( ptr[0] == '*' ) {
              u=pdbBounds::UNKNOWN;
            } else if ( ptr[0] == 'N' && ptr[1] == 'A' ) {
              u=pdbBounds::UNKNOWN;
            } else u = strtol(ptr, &ptr, 10);
            yptr->addBound(l, u);
            if ( yptr->bounds().size() > yptr->rank() ) {
              pdb_ERROR("Too many bounds", "");
            }
          }
          break;

        // UPC support
        case YSHARED:
          if(yptr) {
            yptr->isShared(value[0] == 'T');  
          }
          break;
        case YBLOCKSIZE: 
          if(yptr) {
            yptr->blockSize(atoi(value));
          }
          break;
        case YRELAXED:
          if(yptr) {
            yptr->isRelaxed(value[0] == 'T');
          }
          break;
        case YSTRICT:
          if(yptr) {
            yptr->isStrict(value[0] == 'T');
          }
          break;
        // ==== Invalid Attributes= =====================================
        case NOATTR:
          // ignore
          break;

        default:
          pdb_ERROR("*** INTERNAL PROGRAM ERROR *** at ", value);
          exit(1);
        }
      }
    }
    if ( inbuf[INBUF_SIZE-1] != '\n' && istr.gcount() != INBUF_SIZE) {
      pdb_ERROR("*** LINE TOO LONG ***", "");
      cerr << inbuf << endl;
      exit(1);
    }
    line++;
    inbuf[INBUF_SIZE-1] = '\n';
  }

  // If still not clear whether source language is C or C++, set to C
  if ( language() == LA_C_or_CXX ) lang = LA_C;

  // Are items missing?
  // - build full names and other item specific finalization
  finalCheck(typeTag());
  finalCheck(fileTag());
  finalCheck(classTag());
  finalCheck(moduleTag());
  finalCheck(templateTag());
  finalCheck(macroTag());
  finalCheck(pragmaTag());
  finalCheck(namespaceTag());
  // in case of errors above stop here because these often crash routine checks
  if (!ok) return false;
  finalCheck(croutineTag());
  finalCheck(froutineTag());
  return ok;
}
