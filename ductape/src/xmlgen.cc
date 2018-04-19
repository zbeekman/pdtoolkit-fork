/** LANL:license
 * -------------------------------------------------------------------------
 * This SOFTWARE has been authored by an employee or employees of the
 * University of California, operator of the Los Alamos National Laboratory
 * under Contract No. W-7405-ENG-36 with the U.S. Department of Energy.
 * The U.S. Government has rights to use, reproduce, and distribute this
 * SOFTWARE.  The public may copy, distribute, prepare derivative works and
 * publicly display this SOFTWARE without charge, provided that this Notice
 * and any statement of authorship are reproduced on all copies.  Neither
 * the Government nor the University makes any warranty, express or implied,
 * or assumes any liability or responsibility for the use of this SOFTWARE.
 * If SOFTWARE is modified to produce derivative works, such modified
 * SOFTWARE should be clearly marked, so as not to confuse it with the
 * version available from LANL.
 * -------------------------------------------------------------------------
 * LANL:license
 * -------------------------------------------------------------------------
 */
/**
 * xmlgen : XML generator for component creation tools based on PDToolkit
 *
 * Written by Matthew Sottile (matt@lanl.gov)
 *
 * Contact: Matthew Sottile, MS B287
 *          Advanced Computing Laboratory,
 *          Los Alamos National Laboratory,
 *          Los Alamos, NM 87545
 *
 * Conforms to DTD specified in mapping.dtd included with this code.
 */
#ifdef _MSC_VER
 #include "getopt.h"
#else
 #include <unistd.h>
#endif
#include "pdb.h"
#include "pdbAll.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

//
// enumeration for which language we're dealing with.  I'd rather not
// depend on pdbTypes:: enumerations for this kind of thing, so we have our
// own here with JUST the languages currently supported.
//
enum language_t { LANG_FORTRAN=1, 
		  LANG_C=2, 
		  LANG_CXX=3 };

// 
// global flag with the language
//
language_t libraryLanguage;

//
// global vector of fully-qualified names of subroutines that have
// been output already.  since ductape doesn't have a way to ask for
// globals, we have to make sure that modules/namespaces/classes have
// been dumped FIRST, and then go and figure out what stuff hasn't
// come out yet and see if they qualify as valid globals.
//
vector<string> dumpedRoutines;
vector<string> dumpedTypes;
vector<string> dumpedClasses;

//
// map cache for determining overloaded functions in inheritance trees
//
map< string, map< string, int > > overload_cache;

//
// this is used to remember the file we are looking for data about, so
// we can check the location a pdb object came from and verify it is
// from the right place (the good location).
//
const pdbFile *goodLoc;

//
// flag whether or not statement level information is to be dumped.
//
bool dumpingStatements;
bool dumpingLocations;

//
// since C++ templates use angle brackets, we need to clean up names that
// contain them so XML doesn't get confused (grrrr....)
//
string sanitizeForXML(string instring) {
  string outstring = "";

  for (unsigned int i = 0; i < instring.length(); i++) {
    switch (instring[i]) {
    case '<':
      outstring += "&lt;";
      break;
    case '>':
      outstring += "&gt; ";
      break;
    default:
      outstring += instring[i];
    }
  }

  return outstring;
}

//
// macro for indentation
//
#define indent(os,depth) { for (int i=0;i<depth;i++) os << "  "; }

//
// given a class, traverse up and down the class hierarchy to figure out
// which methods are overloaded or not.
//
map<std::string, int> determineOverloadedNames(const pdbClass *c) {
  map<std::string, int> occurrances;
  vector<std::string> classes;

  //
  // lookup this class in the cache
  //
  if (overload_cache.find(c->fullName()) != overload_cache.end())
    return overload_cache[c->fullName()];

  //
  // this hierarchy hasn't been traversed yet - do it now
  //
  classes.push_back(c->fullName());

  //
  // iterate over own methods first
  //
  const pdbClass::methodvec cmv = c->methods();
  
  for (pdbClass::methodvec::const_iterator m = cmv.begin();
       m != cmv.end();
       ++m) {

    pdbMethod *mm = static_cast<pdbMethod *>(*m);
      
    occurrances[mm->func()->name()] += 1;
  }

  //
  // iterate over derived classes
  //
  pdbClass::classvec dv = c->derivedClasses();
  for (pdbClass::classvec::const_iterator d = dv.begin();
       d != dv.end();
       ++d) {

    //
    // iterate over class methods
    //
    pdbClass *dc = static_cast<pdbClass *>(*d);

    classes.push_back(dc->fullName());

    const pdbClass::methodvec mv = dc->methods();

    for (pdbClass::methodvec::const_iterator m = mv.begin();
         m != mv.end();
         ++m) {

      pdbMethod *mm = static_cast<pdbMethod *>(*m);

      occurrances[mm->func()->name()] += 1;
    }

  }
  
  // 
  // same, but over parents
  //
  const pdbClass::basevec bases = c->baseClasses();
  for (pdbClass::basevec::const_iterator bi = bases.begin();
       bi != bases.end();
       ++bi) {

    pdbBase *base = static_cast<pdbBase *>(*bi);

    if (base->isVirtual() == false) {
      const pdbClass *bc = base->base();

      classes.push_back(bc->fullName());

      const pdbClass::methodvec mv = bc->methods();

      for (pdbClass::methodvec::const_iterator m = mv.begin();
           m != mv.end();
           ++m) {
        
        pdbMethod *mm = static_cast<pdbMethod *>(*m);
        
        //
        // do NOT count virtual things as overloaded.
        //
        if (mm->func()->virtuality() == pdbItem::VI_NO)
          occurrances[mm->func()->name()] += 1;
      }
      
    }

  }

  // 
  // now populate the cache with the occurrance map for each class we touched.
  //
  for (vector<std::string>::iterator vi = classes.begin();
       vi != classes.end();
       ++vi)
    overload_cache[static_cast<std::string>(*vi)] = occurrances;

  return occurrances;
}

//
// predefine these guys since we need them and there's a bit of a 
// circular dependency with outputTypeKinds.
//
int outputArray(const pdbType *ty, ostream& os, int depth);
int outputIndirect(const pdbType *ty, ostream& os, int depth);

//
// code to print type kind info -- based on code in pdbType.cc::printType
//
int outputTypeKinds(const pdbType *_ty, ostream& os, int depth) {
  const pdbType *ty = _ty;

  // 
  // we shouldn't see this.
  // if we do, dump a comment to mark the spot in the output and
  // send the error to STDERR
  //
  if (ty == NULL) {
    os << "<!-- OUTPUTTYPEKINDS ERROR OCCURRED HERE --> " << endl;
    cerr << "ERROR: attempting to print null type." << endl;
    return 1;
  }

  //
  // kind is dumped as is -- specialized types are conditionally
  // printed below
  //
  // NOTE: examine group to see if kind is actually a user defined
  //       type
  //

  bool isConst = false;

  /** 
   ** check if this type is const, and follow the tref one level.
   ** also see if the parent is an enum, and follow it up one level
   ** in that case too.  The enum case came up when wrapping C code.
   **/
  if (ty != NULL && ty->kind() == pdbItem::TY_TREF) {

    /**
     * const-ness
     */
    const pdbType::qualvec qv = ty->qualifiers();
    for (pdbType::qualvec::const_iterator q = qv.begin();
	 q != qv.end();
	 ++q) {
      pdbItem::qual_t qt = static_cast<pdbItem::qual_t>(*q);
      if (qt == pdbItem::QL_CONST) {
        ty = ty->baseType();    
        isConst = true;
        break;
      }
    }

    /**
     * enum-ness
     */
    const pdbType *pty = ty->baseType();
    if (pty != NULL) {
      if (pty->kind() == pdbItem::TY_ENUM)
        ty = pty;
    }
    
  }

  const pdbGroup *cd = ty->isGroup();

  indent(os,depth);
  os << "<type ";

  // 
  // if ty is a group, then we treat it as a usertype and dump the
  // appropriate name.  otherwise, dump the ductape provided name.
  //
  if (cd == NULL) {
    if (ty->kind() != pdbItem::TY_TREF)
      os << "kind=\"" << ty->toName(ty->kind()) << "\" ";
    else
      //
      // only call fullName on C++ stuff to get the fully qualified
      // name including namespace.
      //
      if (libraryLanguage == LANG_CXX)
        os << "kind=\"usertype\" usertype=\"" << sanitizeForXML(ty->fullName()) << "\" ";
      else
        os << "kind=\"usertype\" usertype=\"" << ty->name() << "\" ";
  } else {
    //
    // if the parent group is not null, we can find out where the
    // type is defined.  For example, if this is a fortran user
    // defined type, then the parent group is the module in which
    // the type is declared.
    //
    if (cd->parentGroup() != NULL) {
      if (libraryLanguage == LANG_CXX)
        os << "kind=\"usertype\" usertype=\"" 
           << sanitizeForXML(cd->fullName()) << "\" "
           << "context=\"" << cd->parentGroup()->name() << "\" ";
      else
        os << "kind=\"usertype\" usertype=\"" 
           << sanitizeForXML(cd->name()) << "\" "
           << "context=\"" << cd->parentGroup()->name() << "\" ";
    } else {
      if (libraryLanguage == LANG_CXX)
        os << "kind=\"usertype\" usertype=\"" 
           << sanitizeForXML(cd->fullName()) << "\" ";
      else
        os << "kind=\"usertype\" usertype=\"" 
           << sanitizeForXML(cd->name()) << "\" ";
    }
  }

  //
  // see printType method mentioned above for a very similar switch
  //
  switch(ty->kind()) {
  case pdbItem::TY_INT:
  case pdbItem::TY_BOOL:
  case pdbItem::TY_WCHAR:
  case pdbItem::TY_FINT:
  case pdbItem::TY_FLOGIC:
  case pdbItem::TY_FCHAR:
  case pdbItem::TY_ENUM:
    os << "ikind=\"" << ty->toName(ty->integerType())
       << "\" ";
    break;
  case pdbItem::TY_FLOAT:
  case pdbItem::TY_FFLOAT:
  case pdbItem::TY_FCMPLX:
    os << "fkind=\"" << ty->toName(ty->floatType()) << "\" ";
    break;
  case pdbItem::TY_TPARAM:
    //
    // template junk
    //
    os << "genericname=\"" << ty->name() << "\" ";
    break;
  default:
    break;
  }

  //
  // we have a string, so dump the length
  // I think this only applies to FCHAR
  //
  if (ty->kind() == pdbItem::TY_FCHAR) {
    if (ty->numElements() == -1)
      os << "clen=\"*\" ";
    else
      os << "clen=\"" << ty->numElements() << "\" ";
  }

  //
  // dump enum name if we need to, and the base
  //
  if (ty->kind() == pdbItem::TY_ENUM) {
    os << "enumname=\"" << ty->fullName() << "\" ";

    int enumbase = 0; // default to zero

    //
    // determine the lowest value the enumeration can take, emit this
    // as enumbase.
    //
    pdbType::enumvec ev = ty->enumeration();
    for (pdbType::enumvec::const_iterator evi = ev.begin();
         evi != ev.end();
         ++evi) {
      pdbEnum pe = static_cast<pdbEnum>(*evi);
      if (enumbase > pe.val()) enumbase = pe.val();
    }

    os << "enumbase=\"" << enumbase << "\" ";
  }

  //
  // check if the type was const
  //
  if (isConst == true) {
    os << "const=\"yes\" ";
  }

  os << ">" << endl;

  //
  // if the type is an array or indirection, we need to emit those elements
  // in the context of the currently open TYPE
  //
  outputArray(ty,os,depth+1);
  outputIndirect(ty,os,depth+1);
  
  //
  // close tag
  //
  indent(os,depth);
  os << "</type>" << endl;
  return 0;
}

//
// code to deal with pointers and references
//
int outputIndirect(const pdbType *ty, ostream& os, int depth) {
  int ptrLevels = 0;

  if ((ty->kind() == pdbItem::TY_PTR) ||
      (ty->kind() == pdbItem::TY_FPTR)) {
    //
    // deal with pointers
    //
    ptrLevels++;

    const pdbType *ptype = ty->pointedToType();
    
    //
    // iterate down the pointedToType chain to figure out how deep the
    // pointers go.
    //
    while (ptype != NULL && (ptype->kind() == pdbItem::TY_PTR ||
                             ptype->kind() == pdbItem::TY_FPTR)) {
      ptrLevels++;
      ptype = ptype->pointedToType();
    }

    indent(os,depth);
    os << "<indirect depth=\"" << ptrLevels << "\" kind=\"pointer\">" << endl; 

    //
    // dump the type that this pointer points at
    //
    outputTypeKinds(ptype,os,depth+1);

    indent(os,depth);
    os << "</indirect>" << endl;
  } else if (ty->kind() == pdbItem::TY_REF) {
    //
    // deal with references
    //
    ptrLevels++;

    const pdbType *rtype = ty->referencedType();
    
    indent(os,depth);
    os << "<indirect depth=\"" << ptrLevels << "\" kind=\"reference\">" 
       << endl;

    outputTypeKinds(rtype,os,depth+1);

    indent(os,depth);
    os << "</indirect>" << endl;
  }

  return 0;
}

//
// code to print array elements
//
int outputArray(const pdbType *ty, ostream& os, int depth) {
  bool explicitShape = false;
  bool isFortran = false;

  if ((ty->kind() == pdbItem::TY_ARRAY) || 
      (ty->kind() == pdbItem::TY_FARRAY)) {
    //
    // check shape
    //
    if (ty->shape() == pdbItem::SH_EXPLICIT) {
      explicitShape = true;
    } 

    //
    // print array element: element kind, rank, language
    // the rank only seems to work for FARRAY
    //
    indent(os,depth);
    os << "<array ";
    if (ty->kind() == pdbItem::TY_FARRAY) {
      os << "lang=\"fortran\" ";
      os << "rank=\"" << ty->rank() << "\">" << endl;
      isFortran = true;
    } else {
      //
      // CRank is 1.  Looking at rank() is bogus
      //
      os << "lang=\"c\" rank=\"1\">" << endl;
    }
    
    outputTypeKinds(ty->elementType(),os,depth+1);

    // 
    // c-arrays are really braindead - they don't even show
    // dimension info.  Grrr.
    //
    if (ty->kind() == pdbItem::TY_ARRAY) {
      indent(os,depth+1);
      os << "<dimension extent=\"*\"/>" << endl;
    }

    //
    // iterate over dimensions
    // eventually we will probably have better information from
    // the pdbBounds class, and can then stop the INT_MAX stupidity.
    //
    const pdbType::boundvec bv = ty->bounds();
    for (pdbType::boundvec::const_iterator b = bv.begin();
	 b != bv.end();
	 ++b) {
      const pdbBounds curb = static_cast<const pdbBounds>(*b);
      indent(os,depth+1);

      //
      // if upper == INT_MAX, upper is undefined.  Very dumb hack IMHO.
      // if upper bound was NA, then the array should not be tagged
      // as having explicit shape (right?) -mjs 7/24/02
      //
      if ((&curb)->upper() == INT_MAX)
        explicitShape = false;
        
      //
      // if explicitShape, we know extent and lower_bound (if non-zero or one)
      // otherwise, no lower bound specified and the extent is set to
      // "*".
      //
      if (explicitShape) {
        os << "<dimension extent=\"" << (&curb)->upper()-(&curb)->lower()+1
           << "\" ";
        if (( (&curb)->lower() != 0 && !isFortran) ||
            ( (&curb)->lower() != 1 &&  isFortran) )
          os << "lower_bound=\"" << (&curb)->lower() << "\"";
        os << "/>" << endl;
      } else {
        os << "<dimension extent=\"*\"/>" << endl;
      }
    }

    //
    // emit close tag
    //
    indent(os,depth);
    os << "</array>" << endl;
  }

  return 0;
}

//
// dump a single pdbArg out to os
//
int outputArgument(const pdbArg *arg, ostream& os, int depth) {
  //
  // comment block
  //
  os << endl;
  indent(os,depth);
  os << "<!-- ARGUMENT : " << arg->name() << " -->" << endl;

  //
  // begin argument
  //
  indent(os,depth);
  os << "<arg name=\"" << arg->name() << "\" ";
  
  //
  // dump intent
  //  
  if (arg->intentOut()) {
    if (arg->intentIn()) {
      os << "intent=\"inout\">";
    } else {
      os << "intent=\"out\">";
    }
  } else {
    os << "intent=\"in\">";
  }

  //
  // if it is an array, we dump that here.
  //
  os << endl;

  //
  // print specific type kind information
  //
  outputTypeKinds(arg->type(),os,depth+1);

  //
  // end arg tag
  //
  indent(os,depth);
  os << "</arg>" << endl;

  return 0;
}

//
// Output statements
//
int outputStatements(const pdbStmt *stmt, ostream& os, int depth) {
  if (stmt == NULL) return 1; /* bail out on null statements */

  const pdbStmt *s;
  int currentID;

  /* loop over the current statement list, recursively descending
     the statement tree as necessary for dealing with nested blocks.
     the loop will also have the ID counter to preserve the order of
     statements in the XML. 
   */
  for (s = stmt, currentID = 1;
       s != NULL;
       s = s->nextStmt(), currentID++) {
    /*
     * deal with blocks
     */
    if (s->kind() == pdbStmt::ST_BLOCK) {
      indent(os,depth);
      os << "<stmtblock id=\"" << currentID << "\" type=\"block\">" 
         << endl;

      outputStatements(s->downStmt(),os,depth+1);

      indent(os,depth);
      os << "</stmtblock>" << endl;
    } else {

      // dump the statement
      indent(os,depth);
      os << "<statement id=\"" << currentID << "\" kind=\"" <<
        pdbStmt::toName(s->kind()) << "\"";

      // figure out if this statement element will contain anything else
      //
      if (s->extraStmt() == NULL && s->downStmt() == NULL &&
          dumpingLocations == false)
        os << "/>" << endl;
      else
        os << ">" << endl;

      // if we're dumping locations, dump them here
      //
      if (dumpingLocations) {
        indent(os,depth+1);
        os << "<location filename=\"" << s->stmtBegin().file()->name() <<
          "\" startline=\"" << s->stmtBegin().line() << 
          "\" startcol=\"" << s->stmtBegin().col() << 
          "\" endline=\"" << s->stmtEnd().line() <<
          "\" endcol=\"" << s->stmtEnd().col() << "\"/>" << endl;
      }

      //
      // dump extra statements - these are like the bits of stuff that
      // appear in the control structure for a for loop.
      //
      if (s->extraStmt() != NULL) {
        indent(os,depth+1);
        os << "<stmtblock id=\"0\" type=\"extra\">" << endl;

        outputStatements(s->extraStmt(),os,depth+2);
      
        indent(os,depth+1);
        os << "</stmtblock>" << endl;        
      }

      //
      // if we have a downStmt, recurse.
      //
      if (s->downStmt() != NULL)
        outputStatements(s->downStmt(),os,depth+1);

      //
      // check if we had stuff in a statement element to close it off.
      // otherwise, it got closed earlier
      //
      if (s->extraStmt() != NULL || s->downStmt() != NULL ||
          dumpingLocations == true) {
        indent(os,depth);
        os << "</statement>" << endl;
      }
    }
  }

  // done
  //
  return 1;
}

//
// Output a single fortran routine
//
int outputFRoutine(const pdbFRoutine *cur, ostream& os, int depth) {
  const pdbType *sig = cur->signature();

  //
  // note: not sure what sig==NULL implies about the function itself.
  //
  if (sig != NULL) {
    const pdbLoc ploc = cur->firstExecStmtLocation();

    // 
    // if we have no known implementation, this is 0 and we shouldn't generate
    // a signature.  Wrapper generators aren't guaranteed an implementation
    // exists, so we don't want to lead them into believing they should
    // worry about wrapper code
    //
    if (ploc.line() == 0) {
      dumpedRoutines.push_back(cur->fullName());
      return 0;
    }

    //
    // dump the routine
    //
    indent(os,depth);
    
    // 
    // comment block
    //
    os << endl;
    indent(os,depth);
    os << "<!-- " << endl;
    indent(os,depth);
    os << "  -  METHOD: " << cur->fullName() << endl;
    indent(os,depth);
    os << "  -->" << endl;
    indent(os,depth);
    dumpedRoutines.push_back(cur->fullName());
    
    //
    // begin method
    //
    os << "<method name=\"" << sanitizeForXML(cur->name()) << "\" ";

    const pdbFRoutine *isAlias = cur->isAliasFor();
    if (isAlias != NULL) {
      os << "alias=\"" << isAlias->fullName() << "\" ";
    }

    //
    // aliases (methods from INTERFACE blocks) are considered abstract.
    // otherwise, fortran subroutines and functions are considered static.
    //
    if (cur->kind() == pdbType::RO_FALIAS) {
      os << "kind=\"abstract\">" << endl;
    } else {
      os << "kind=\"static\">" << endl;
    }

    //
    // dump each argument
    //
    const pdbType::argvec av = sig->arguments();
    for (pdbType::argvec::const_iterator a = av.begin();
	 a != av.end();
	 ++a) {
      const pdbArg cur = static_cast<const pdbArg>(*a);
      outputArgument(&cur,os,depth+1);
    }

    //
    // dump the return value
    //
    const pdbType *ret = sig->returnType();
    os << endl;
    indent(os,depth+1);
    os << "<!-- return value -->" << endl;
    indent(os,depth+1);
    os << "<return>" << endl;

    outputTypeKinds(ret,os,depth+2);
    
    indent(os,depth+1);
    os << "</return>" << endl;
    
    //
    // if we're supposed to dump statement level information, do so here
    //
    if (dumpingStatements == true) {
      const pdbStmt* bdy = cur->body();
      os << endl;
      indent(os,depth+1);
      os << "<!-- statement level info -->" << endl;
      outputStatements(bdy, os, depth+1);
      os << endl;
    }

    //
    // close method
    //
    indent(os,depth);
    os << "</method>" << endl;
  } else {
    //
    // SIG == NULL means we might be dealing with a function alias here
    // 
    dumpedRoutines.push_back(cur->fullName());

    if (cur->kind() == pdbItem::RO_FALIAS) {
      // 
      // comment block
      //
      os << endl;
      indent(os,depth);
      os << "<!-- " << endl;
      indent(os,depth);
      os << "  -  PROCEDUREMAP: " << cur->fullName() << endl;
      indent(os,depth);
      os << "  -->" << endl;
      indent(os,depth);
      
      os << "<proceduremap name=\"" << cur->fullName() << "\">" << endl;
      //
      // dump the implementations of the generic as concrete elements
      //
      const pdbFRoutine::implvec iv = cur->implementedBy();
      for (pdbFRoutine::implvec::const_iterator i = iv.begin();
           i != iv.end();
           ++i) {
        pdbFRoutine *fr = static_cast<pdbFRoutine *>(*i);
        indent(os,depth+1);
        os << "<concrete name=\"" << fr->name() << "\"/>" << endl;
      }

      indent(os,depth);
      os << "</proceduremap>" << endl << endl;

    } else {
      cerr << "METHOD: " << cur->fullName() << " has null signature." << endl;
      cerr << "        method not emitted to XML." << endl;
    }
  }

  return 0;
}

/**
 * This method outputs a user-defined fortran type.
 */
int outputFUserType(const pdbGroup *gr, ostream& os, int depth) {
  os << endl;
  indent(os,depth);
  os << "<!--" << endl;
  indent(os,depth);
  os << "  - User defined type : " << gr->name() << endl;
  indent(os,depth);
  os << "  -->" << endl;

  dumpedTypes.push_back(gr->fullName());

  indent(os,depth);
  os << "<structure name=\"" << gr->name() << "\" kind=\"ftype\">" << endl;

  //
  // iterate over fields
  //
  pdbGroup::fieldvec typefields = gr->dataMembers();
  for (pdbGroup::fieldvec::const_iterator tf = typefields.begin();
       tf != typefields.end();
       ++tf) {
    const pdbGroupField *tfield =
      static_cast<const pdbGroupField*>(*tf);

    os << endl;
    indent(os,depth+1);
    os << "<!-- FIELD: " << tfield->name() << " -->" << endl;
    indent(os,depth+1);
    os << "<field name=\"" << tfield->name() << "\">" << endl;
    
    outputTypeKinds(tfield->type(),os,depth+2);
    
    indent(os,depth+1);
    os << "</field>" << endl;
  }
  
  indent(os,depth);
  os << "</structure>" << endl;

  return 0;
}

//
// Output a single C routine.  Return values are in enum.
//

typedef enum { OCR_NONE=0, OCR_NORMAL=1, OCR_CTOR=2, OCR_DTOR=3 } ocr_res;

ocr_res outputCRoutine(const pdbCRoutine *cur, ostream& os, int depth, 
                       bool ol) {
  const pdbType *sig = cur->signature();
  ocr_res retval = OCR_NORMAL;

  //
  // bail out on compiler generated stuff
  //
  if (cur->isCompilerGenerated()) return OCR_NONE;

  //
  // note: not sure what sig==NULL implies about the function itself.
  //
  if (sig != NULL) {

    //
    // dump the routine
    //
    indent(os,depth);
    
    // 
    // comment block
    //
    os << endl;
    indent(os,depth);
    os << "<!-- " << endl;
    indent(os,depth);
    os << "  -  METHOD: " << cur->fullName() << endl;
    indent(os,depth);
    os << "  -->" << endl;
    indent(os,depth);
    dumpedRoutines.push_back(cur->fullName());

    //
    // begin method
    //
    os << "<method name=\"" << sanitizeForXML(cur->name()) << "\" ";
    
    if (cur->isVirtual()) {
      if (cur->virtuality() == pdbItem::VI_PURE)
        os << "kind=\"purevirtual\"";
      else
        os << "kind=\"virtual\"";
    } else if (cur->isStatic())
      os << "kind=\"static\"";
    else
      os << "kind=\"final\"";

    //
    // check constructor/destructor, operator overloading, etc...
    //
    bool isCDTOR = false;
    bool isOperator = false;

    switch(cur->specialKind()) {
    case pdbItem::RS_NA:
      break;

    case pdbItem::RS_DTOR:
      os << " dtor=\"yes\"";
      isCDTOR = true;
      retval = OCR_DTOR;
      break;
     
    case pdbItem::RS_CTOR:
      os << " ctor=\"yes\"";
      isCDTOR = true;
      retval = OCR_CTOR;
      break;

    case pdbItem::RS_OP:
      os << " op=\"yes\"";
      isOperator = true;
      break;
      
    default:
      cerr << "WARNING: UNSUPPORTED SPECIAL KIND ON METHOD " << cur->name() << endl;
    }

    // 
    // check method access.  only indicate protected or private, since public
    // is the default.
    //
    switch (cur->access()) {
    case pdbItem::AC_PRIV:
      os << " access=\"priv\"";
      break;
    case pdbItem::AC_PROT:
      os << " access=\"prot\"";
      break;
    default:
      break;
    }

    // 
    // if it's overloaded, dump this attribute
    //
    if (ol == true)
      os << " overloaded=\"yes\"";

    //
    // check if method is inline
    //
    if (cur->isInline())
      os << " inlined=\"yes\"";
    
    os << ">" << endl;

    //
    // warning related to operator overloading
    //
    if (isOperator == true) {
      indent(os,depth+1);
      os << "<!-- OVERLOADED OPERATOR METHOD: DO NOT TREAT LIKE " << 
        "NORMAL -->" << endl;
    }

    //
    // dump each argument
    //
    const pdbType::argvec av = sig->arguments();
    for (pdbType::argvec::const_iterator a = av.begin();
	 a != av.end();
	 ++a) {
      const pdbArg cur = static_cast<const pdbArg>(*a);
      outputArgument(&cur,os,depth+1);
    }

    //
    // dump the return value
    //
    if (isCDTOR == true) {
      indent(os,depth+1);
      os << "<!-- THIS IS A CTOR/DTOR METHOD: THIS IS TO BE IGNORED. -->" 
         << endl;
    }

    const pdbType *ret = sig->returnType();
    os << endl;
    indent(os,depth+1);
    os << "<!-- return type (name="<< ret->name() << ") -->" << endl;
    indent(os,depth+1);
    os << "<return>" << endl;
    outputTypeKinds(ret,os,depth+2);
    indent(os,depth+1);
    os << "</return>" << endl;

    //
    // dump exceptions raised by this method
    //

    bool excepComment = false;
    for (pdbType::typevec::const_iterator ex = sig->exceptionSpec().begin();
         ex!=sig->exceptionSpec().end();
         ++ex) {
      if ((*ex) != pdbType::NULLEXCEP) {
        const pdbGroup *gr = (*ex)->isGroup();

        // 
        // we only want this comment if the method throws one or more
        // real exceptions.
        //
        if (excepComment == false) {
          excepComment = true;
          os << endl;
          indent(os,depth+1);
          os << "<!-- exceptions thrown by this method -->" << endl;
        }

        indent(os,depth+1);
        if (gr != NULL)
          os << "<exception name=\"" << gr->fullName() << "\"/>" << endl;
        else
          os << "<exception name=\"" << (*ex)->fullName() << "\"/>" << endl;
      }
    }

    //
    // if we're supposed to dump statement level information, do so here
    //
    if (dumpingStatements == true) {
      const pdbStmt* bdy = cur->body();
      os << endl;
      indent(os,depth+1);
      os << "<!-- statement level info -->" << endl;
      outputStatements(bdy, os, depth+1);
      os << endl;
    }

    //
    // close method
    //
    indent(os,depth);
    os << "</method>" << endl;
  } else {
    //
    // back to this sig==NULL weirdness.  Currently, do NOT dump, just warn.
    //
    dumpedRoutines.push_back(cur->fullName());

    cerr << "METHOD: " << cur->fullName() << " has null signature." << endl;
    cerr << "        method not emitted to XML." << endl;
  }

  return retval;
}

/**
 * this method outputs C/C++ routines not contained in classes or other
 * containers.
 */
int outputCRoutines(PDB::croutinevec& crv, ostream& os, int depth) {
  bool dumped = false;
  bool printedScope = false;

  //
  // iterate over each routine, outputting the XML for each
  //
  for (PDB::croutinevec::const_iterator c = crv.begin();
       c != crv.end();
       ++c) {
    const pdbCRoutine *cur = static_cast<const pdbCRoutine*>(*c);

    //
    // check if it has already been dumped...
    //
    for (unsigned int i = 0; i < dumpedRoutines.size(); i++) {
      if ((string)dumpedRoutines[i] == cur->fullName()) {
        dumped = true;
        break;
      }
    }

    //
    // if a method has a null signature, we definitely don't dump it.
    // so pretend it has already been dumped.
    //
    const pdbType *sig = cur->signature();
    if (sig == NULL)
      dumped = true;

    //
    // compiler generated functions don't get dumped either.  (things
    // like operator= and other junk.)  I hate C++!
    //
    if (cur->isCompilerGenerated() == true)
      dumped = true;

    //
    // make sure this method comes from the file we care about
    //
    if (cur->location().file() != goodLoc)
      dumped = true;

    if (dumped == false) {
      if (printedScope == false) {
        indent(os,depth);
        os << "<scope name=\"CXXGlobal\" kind=\"global\">" <<
          endl;
        printedScope = true;
      }
      outputCRoutine(cur,os,depth+1,false);
      dumpedRoutines.push_back(cur->fullName());
    }

    dumped = false;
  }

  if (printedScope == true) {
    indent(os,depth);
    os << "</scope>" << endl;
  }
  
  return 0;
}

/**
 * dump template info for debug info.
 */
void dumpTemplate(const pdbTemplate *ctempl) {
  if (ctempl == NULL) return;

  if (ctempl->location().file() != goodLoc) return;

  /*
  cerr << "DUMPING TEMPLATE:" << endl;
  cerr << "desc() = " << ctempl->desc() << endl;
  cerr << "text() = " << ctempl->text() << endl;
  cerr << "name() = " << ctempl->name() << endl;

  cerr << "==>declaration" << endl;
  */
  dumpTemplate(ctempl->declaration());

  /*
  cerr << "==>definition" << endl;
  */
  dumpTemplate(ctempl->definition());

  /*
  cerr << "---->template arguments" << endl;
  */
  const pdbTemplate::targvec &tv = ctempl->arguments();
  for (pdbTemplate::targvec::const_iterator ta = tv.begin();
       ta != tv.end();
       ++ta) {
    const pdbTemplateArg targ =
      static_cast<const pdbTemplateArg>(*ta);

    /*
    cerr << "-->name() = " << targ.name() << endl;
    cerr << "-->kind() = " << targ.toName(targ.kind()) << endl;
    cerr << "-->value() = " << targ.value() << endl;
    cerr << "-->defaultValue() = " << targ.defaultValue() << endl;
    cerr << "-->isSpecialization() = " << targ.isSpecialization() << endl;

    cerr << "---->TEMPLATEARG()" << endl;
    */
    dumpTemplate(targ.templateArg());
    /*
    cerr << "---->DEFAULTTEMPLATEARG()" << endl;
    */
    dumpTemplate(targ.defaultTemplateArg());
    
    /*
    const pdbType *t = targ.type();
    cerr << "---->TYPE() name=" << t->name() << endl;
    cerr << "---->       kind=" << t->toName(t->kind()) << endl;
    */    
  }

}

/**
 * This method outputs a C++ class or C struct. 
 */
int outputClass(const pdbClass *c, ostream &os, int depth, bool isInner,
                bool isCStruct) {
  //
  // check to see if the class contains anything - if it is a simple forward
  // declaration of a class, then it will be empty and we don't want to
  // emit anything
  //

  if (libraryLanguage == LANG_CXX) {
    const pdbGroup::fieldvec fv = ((const pdbGroup *)c)->dataMembers();
    int field_count = 0;
    for (pdbGroup::fieldvec::const_iterator tf = fv.begin();
         tf != fv.end();
         ++tf) {
      field_count++;
    }

    //
    // no fields, so check for methods.  otherwise, fall through and emit the
    // class.
    //
    if (field_count == 0) {
      int method_count = 0;
      const pdbClass::methodvec mv = c->methods();
      for (pdbClass::methodvec::const_iterator m = mv.begin();
           m != mv.end();
           ++m) {
        const pdbMethod *meth =
          static_cast<const pdbMethod *>(*m);
        if (meth->func()->isCompilerGenerated() == false)
          method_count++;
      }
      // 
      // if method and field counts are zero, the class is empty so don't
      // dump it.
      //
      if (method_count == 0)
        return 0;

    }
  }

  // 
  // at this point we know it contains something...
  //
  os << endl;
  indent(os,depth);
  os << "<!--" << endl;
  indent(os,depth);
  if (libraryLanguage == LANG_C || isCStruct == true)
    os << "   - STRUCTURE: " << c->fullName() << endl;
  else
    os << "   - CLASS: " << c->fullName() << endl;
  indent(os,depth);
  os << "  -->" << endl;

  dumpedClasses.push_back(c->fullName());

  //
  // print open tag for class structure
  //
  indent(os,depth);
  if (libraryLanguage == LANG_CXX && isCStruct == false) {
    os << "<structure kind=\"cxxclass\" name=\"" << sanitizeForXML(c->name()) << "\" ";

    // 
    // emit inner class flag yes/no (no is implicit if this isn't
    // printed.)
    //
    if (isInner == true)
      os << "innerclass=\"yes\" ";

    //
    // get the base class vector and iterate over the parents of this
    // class.  this wasn't in older version of xmlgen, but we need
    // to support multiple inheritance (even though it is dumb.).
    //
    const pdbClass::basevec &bv = c->baseClasses();
    if (bv.size() != 0) {
      os << "parent=\"";

      int pcount = 0;
      for (pdbClass::basevec::const_iterator b = bv.begin();
           b != bv.end();
           ++b) { pcount++; }

      int pnum = 0;
      for (pdbClass::basevec::const_iterator b = bv.begin();
           b != bv.end();
           ++b) {
        pnum++;
        pdbBase *base = static_cast<pdbBase *>(*b);
        const pdbClass *baseclass = base->base();
        if (pnum < pcount)
          os << baseclass->fullName() << " ";
        else
          os << baseclass->fullName();
      }
      os << "\" ";
    }

  } else
    os << "<structure kind=\"cstruct\" name=\"" << c->name() << "\" ";
  
  switch (c->access()) {
  case pdbItem::AC_PRIV:
    os << "access=\"priv\" ";
    break;
  case pdbItem::AC_PROT:
    os << "access=\"prot\" ";
    break;
  default:
    break;
  }

  const pdbTemplate *ctempl = c->isTemplate();

  //
  // ... so its templated.
  //
  if (ctempl != NULL) {
    // 
    // switch on the different template types
    //
    switch (ctempl->kind()) {
    case pdbItem::TE_CLASS:
      os << " templated=\"yes\" ";
      dumpTemplate(ctempl);

      //os << "<!-- " << ctempl->text() << " --> " << endl;
      //      const pdbTemplateItem::targvec *targs = c->speclArguments();
      //for (pdbTemplateItem::targvec::const_iterator ta = targs->begin();
      //     ta != targs->end();
      //     ++ta) {
      //  const pdbTemplateArg ti =
      //    static_cast<const pdbTemplateArg>(*ta);
      //  
      //  os << "<!-- template arg: " << ti.name() << " -->" << endl;
      //}
      
      break;
    default:
      cerr << "Unsupported template type found in class '" <<
        c->name() << "'" << endl;
    }
  }

  //
  // close element open tag
  //
  os << ">" << endl;

  // now go through fields and methods
  // fields first.
  const pdbGroup::fieldvec fv = ((const pdbGroup *)c)->dataMembers();
  for (pdbGroup::fieldvec::const_iterator tf = fv.begin();
       tf != fv.end();
       ++tf) {
    const pdbGroupField *tfield =
      static_cast<const pdbGroupField*>(*tf);

    switch (tfield->kind()) {
    case pdbItem::M_VAR:
      os << endl;
      indent(os,depth+1);
      os << "<!-- FIELD: " << tfield->name() << " -->" << endl;
      indent(os,depth+1);
      os << "<field name=\"" << tfield->name() << "\" ";

      switch (tfield->access()) {
      case pdbItem::AC_PROT:
        os << "access=\"prot\"";
        break;
      case pdbItem::AC_PRIV:
        os << "access=\"priv\"";
        break;
      default:
        break;
      }

      os << ">" << endl;

      outputTypeKinds(tfield->type(),os,depth+2);
      
      indent(os,depth+1);
      os << "</field>" << endl;
      break;
      
    case pdbItem::M_TYPE:
      //
      // if we come across a member type that is a group, then we have
      // an inner class or structure within a class.
      //
      if (tfield->type()->isGroup() != NULL) {
        const pdbGroup *innerGr = tfield->type()->isGroup();

        // kind() is group_t on pdbGroup objects
        switch (innerGr->kind()) {
        case pdbItem::GR_CLASS:
          outputClass((const pdbClass *)innerGr, os, depth+1, true, false);
          dumpedClasses.push_back(((const pdbClass *)innerGr)->fullName());
          break;

        case pdbItem::GR_STRUCT:
          outputClass((const pdbClass *)innerGr, os, depth+1, true, true);
          dumpedClasses.push_back(((const pdbClass *)innerGr)->fullName());
          break;

        case pdbItem::GR_UNION:
        case pdbItem::GR_TPROTO:
          cerr << "Group within class " << c->fullName() << " is unsupported "
               << "kind() - ask someone to fix." << endl;
          break;

        case pdbItem::GR_FDERIVED:
        case pdbItem::GR_FMODULE:
          cerr << "Group within class " << c->fullName() << " contains " <<
               "Fortran structures : likely PDB bug." << endl;
          break;

        case pdbItem::GR_NA:
          cerr << "Group within class " << c->fullName() << " has NA kind." <<
            endl;
          break;
          
        default:
          cerr << "Group within class " << c->fullName() << " has unknown " <<
            "kind." << endl;
          break;
        }

      } else {
        os << endl;
        indent(os,depth+1);
        os << "<!-- TYPEALIAS: " << tfield->name() << " -->" << endl;
        indent(os,depth+1);
        os << "<typealias name=\"" << tfield->name() << "\">" << endl;

        outputTypeKinds(tfield->type()->baseType(),os,depth+2);

        dumpedTypes.push_back(tfield->fullName());

        indent(os,depth+1);
        os << "</typealias>" << endl;
      }

      break;

    default:
      cerr << "Field kind() not supported.\n" << endl;
    }
  }

  // build list of overloaded methods for this class and it's parents and
  // children.
  map<std::string, int> overloaded = determineOverloadedNames(c);

  // then methods
  bool gotCTOR = false;
  const pdbClass::methodvec mv = c->methods();
  for (pdbClass::methodvec::const_iterator m = mv.begin();
       m != mv.end();
       ++m) {
    const pdbMethod *meth =
      static_cast<const pdbMethod *>(*m);
    ocr_res ocrr;

    if (overloaded[meth->func()->name()] > 1) 
      ocrr = outputCRoutine(meth->func(),os,depth+1,true);
    else
      ocrr = outputCRoutine(meth->func(),os,depth+1,false);

    if (ocrr == OCR_CTOR) gotCTOR = true;
  }  

  //
  // if we haven't gotten a constructor then we should emit one special here
  //
  if (gotCTOR == false && isCStruct == false) {
    os << endl;
    indent(os,depth+1);
    os << "<!-- default constructor -->" << endl;

    indent(os,depth+1);
    os << "<method name=\"" << sanitizeForXML(c->name()) << "\" kind=\"final\" ctor=\"yes\">"
       << endl;

    indent(os,depth+2);
    os << "<return>" << endl;
    indent(os,depth+3);
    os << "<type kind=\"ref\">" << endl;
    indent(os,depth+4);
    os << "<indirect depth=\"1\" kind=\"reference\">" << endl;
    indent(os,depth+5);
    os << "<type kind=\"usertype\" usertype=\"" << sanitizeForXML(c->name()) << 
      "\"/>" << endl;
    indent(os,depth+4);
    os << "</indirect>" << endl;
    indent(os,depth+3);
    os << "</type>" << endl;
    indent(os,depth+2);
    os << "</return>" << endl;

    indent(os,depth+1);
    os << "</method>" << endl;
  }

  //
  // close structure
  //
  indent(os,depth);
  os << "</structure>" << endl;

  return 0;
}

/**
 * This method outputs all C++ namespaces
 */
int outputNamespaces(PDB::namespacevec& nv, ostream& os, int depth) {
  for (PDB::namespacevec::const_iterator n = nv.begin();
       n != nv.end();
       ++n) {
    const pdbNamespace *cur = static_cast<const pdbNamespace*>(*n);
    pdbNamespace::memvec mv = cur->members();

    // 
    // make sure the namespace isn't empty - those are useless and stupid.
    // also make sure we are coming from the file we care about.
    //
    if (mv.begin() != mv.end() && cur->location().file() == goodLoc) {
      //
      // comment the XML
      //
      os << endl;
      indent(os,depth);
      os << "<!--" << endl;
      indent(os,depth);
      os << "   - Namespace: " << cur->fullName() << endl;
      indent(os,depth);
      os << "  -->" << endl << endl;

      //
      // begin scope tag
      //
      indent(os,depth);
      os << "<scope name=\"" << cur->fullName() << "\" kind=\"namespace\"" <<
	">" << endl;

      //
      // iterate over namespace members
      // (how one does this is not obvious...  more reason PDT is
      //  lightyears from user friendly...)
      //
      for (pdbNamespace::memvec::const_iterator m = mv.begin();
	   m != mv.end();
	   ++m) {
	const pdbItem *memitem = static_cast<const pdbItem *>(*m);

	//
	// i think this has got to be the absolute worst thing in PDT
	// so far.  Nothing *NOTHING* gives any hint that we need to
	// cast across from pdbItem to pdbGroup and pdbClass.  In fact,
	// nothing allows us to look at a pdbItem and figure out what
	// it should be cast to.  This is bad.  very bad.
	//
	if (((const pdbGroup *)memitem)->kind() == pdbItem::GR_CLASS) {
	  outputClass((const pdbClass *)memitem,os,depth+1,false,false);
	}
      }
      
      //
      // close scope tag
      //
      indent(os,depth);
      os << "</scope>" << endl;
    }    
  }

  return 0;
}

/**
 * This method outputs all fortran modules
 */
int outputModules(PDB::modulevec& mv, ostream& os, int depth) {
  //
  // we're handed a module vector, so go through it and dump each module
  //
  for (PDB::modulevec::const_iterator m = mv.begin();
       m != mv.end();
       ++m) {
    const pdbModule *cur = static_cast<const pdbModule*>(*m);
    pdbModule::modfuncvec mfv = cur->funcMembers();

    //
    // make sure the module isn't empty.  we don't like
    // empty scopes.
    //
    if (mfv.begin() != mfv.end()) {

      //
      // comment block
      //
      os << endl;
      indent(os,depth);
      os << "<!--" << endl;
      indent(os,depth);
      os << "  - Module: " << cur->fullName() << endl;
      indent(os,depth);
      os << "  -->" << endl;
      
      //
      // begin scope, we know TYPE is fmodule
      //
      indent(os,depth);
      os << "<scope name=\"" << cur->fullName() << "\" kind=\"module\">"
         << endl;

      //
      // dump out user defined types
      //
      pdbGroup::fieldvec gfv = cur->dataMembers();
      for (pdbGroup::fieldvec::const_iterator f = gfv.begin();
           f != gfv.end();
           ++f) {
        const pdbGroupField *c = static_cast<const pdbGroupField*>(*f);

        // get the type of the group field
        const pdbType *ct = c->type();
        if (ct != NULL) {
          //
          // now, get the group for this type.  If it is NULL, then
          // this is not a user defined type.  If it is non-null, we
          // process it.
          //
          const pdbGroup *cg = ct->isGroup();
          if (cg != NULL)
            outputFUserType(cg,os,depth+1);
         }
      }

      // go through the module function members
      for (pdbModule::modfuncvec::const_iterator f = mfv.begin();
           f != mfv.end();
           ++f) {
        const pdbModFunc *cur = static_cast<const pdbModFunc*>(*f);
        
        //
        // dump the routine
        //
        outputFRoutine(cur->func(),os,depth+1);
      }

      //
      // close scope
      //
      indent(os,depth);
      os << "</scope>" << endl;
    }
  }
  
  return 0;
}

/**
 * This method outputs all fortran types
 */
int outputFTypes(PDB::typevec& tv, ostream& os, int depth) {
  bool dumped = false;
  bool printedScope = false;

  for (PDB::typevec::const_iterator t = tv.begin();
       t != tv.end();
       ++t) {
    const pdbType *cur = static_cast<const pdbType*>(*t);
    //
    // check if already dumped
    //
    for (unsigned int i = 0; i < dumpedTypes.size(); i++) {

      // we only deal with ones that isGroup() != NULL
      if (cur->isGroup() != NULL) {
        if ((string)dumpedTypes[i] ==
            cur->isGroup()->fullName()) {
          dumped = true;
          break;
        }
      } else {
        if ((string)dumpedTypes[i] == cur->fullName()) {
          dumped = true;
          break;
        }
      }
    }

    //
    // don't accidentally fall into printing scope if all we know about
    // are intrinsic types...
    //
    if (cur->isGroup() == NULL) dumped = true;

    // 
    // dumped == false means we really have at least one non-trivial
    // user defined type, so we dump it.
    //
    if (dumped == false) {
      if (printedScope == false) {
        indent(os,depth);
	//
	// NOTE: maybe here interface is implicit?
	//
        os << "<scope name=\"types_not_in_modules\" kind=\"global\">" <<
          endl;
        printedScope = true;
      }
      
      const pdbGroup *cg = cur->isGroup();

      //
      // this if should never fail thanks to the IF above.
      //
      if (cg != NULL)
        outputFUserType(cg,os,depth+1);

      // 
      // remember this type
      //
      dumpedTypes.push_back(cur->fullName());
    }

    // reset flag
    dumped = false;
  }

  //
  // close scope element if we opened it.
  //
  if (printedScope == true) {
    indent(os,depth);
    os << "</scope>" << endl;
  }

  return 0;
}

/**
 * This method outputs all fortran routines
 */
int outputFRoutines(PDB::froutinevec& frv, ostream& os, int depth) {
  bool dumped = false;
  bool printedScope = false;

  // iterate over each routine, outputting the XML for each
  for (PDB::froutinevec::const_iterator f = frv.begin();
       f != frv.end();
       ++f) {
    const pdbFRoutine *cur = static_cast<const pdbFRoutine*>(*f);

    //
    // check if it has already been dumped...
    //
    for (unsigned int i = 0; i < dumpedRoutines.size(); i++) {
      if ((string)dumpedRoutines[i] == cur->fullName()) {
        dumped = true;
        break;
      }
    }

    //
    // if a method has a null signature, we definitely don't dump it.
    // so pretend it has already been dumped.
    //
    const pdbType *sig = cur->signature();
    if (sig == NULL)
      dumped = true;

    if (dumped == false) {
      if (printedScope == false) {
        indent(os,depth);
        os << "<scope name=\"F90Global\" kind=\"global\">" <<
          endl;
        printedScope = true;
      }
      outputFRoutine(cur,os,depth+1);
      dumpedRoutines.push_back(cur->fullName());
    }

    dumped = false;
  }

  if (printedScope == true) {
    indent(os,depth);
    os << "</scope>" << endl;
  }
  
  return 0;
}

/**
 * dump types from the given PDB::typevec that have not been dumped yet.
 */
int outputCTypes(PDB::typevec &iv, ostream &os, int depth) {
  bool dumped = false;
  bool printedScope = false;

  // iterate over each type
  for (PDB::typevec::const_iterator i = iv.begin();
       i != iv.end();
       ++i) {

    const pdbType *cur = static_cast<const pdbType*>(*i);

    for (unsigned int i = 0; i < dumpedTypes.size(); i++) {
      if ((string)dumpedTypes[i] == cur->fullName()) {
        dumped = true;
        break;
      }
    }

    if (cur->location().file() != goodLoc) dumped = true;

    // 
    // now we know what types haven't been dumped yet
    //
    if (dumped == false) {
      if (printedScope == false) {
        indent(os,depth);
        os << "<scope name=\"GlobalTypes\" kind=\"global\">" <<
          endl;
        printedScope = true;
      }

      if (cur->isGroup() != NULL) {
        const pdbGroup *innerGr = cur->isGroup();

        // kind() is group_t on pdbGroup objects
        switch (innerGr->kind()) {
        case pdbItem::GR_CLASS:
          outputClass((const pdbClass *)innerGr, os, depth+1, true, false);
          dumpedClasses.push_back(((const pdbClass *)innerGr)->fullName());
          break;

        case pdbItem::GR_STRUCT:
          outputClass((const pdbClass *)innerGr, os, depth+1, true, true);
          dumpedClasses.push_back(((const pdbClass *)innerGr)->fullName());
          break;

        case pdbItem::GR_UNION:
        case pdbItem::GR_TPROTO:
        case pdbItem::GR_FDERIVED:
        case pdbItem::GR_FMODULE:
        case pdbItem::GR_NA:
        default:
          cerr << "Group has unknown kind." << endl;
          break;
        }

      } else {
        os << endl;
        indent(os,depth+1);
        os << "<!-- TYPEALIAS: " << cur->name() << " -->" << endl;
        indent(os,depth+1);
        os << "<typealias name=\"" << cur->name() << "\">" << endl;

        outputTypeKinds(cur->baseType(),os,depth+2);

        indent(os,depth+1);
        os << "</typealias>" << endl;
      }

      dumpedTypes.push_back(cur->fullName());
    }

    //
    // reset dumped flag
    //
    dumped = false;
  }

  if (printedScope == true) {
    indent(os,depth);
    os << "</scope>" << endl;
  }

  return 0;
}

/**
 * dump classes from the given PDB::classvec that have not been dumped
 * yet.
 */
int outputClasses(PDB::classvec &cv, ostream &os, int depth) {
  bool dumped = false;
  bool printedScope = false;

  // iterate over each class, outputting the XML for each
  for (PDB::classvec::const_iterator c = cv.begin();
       c != cv.end();
       ++c) {
    const pdbClass *cur = static_cast<const pdbClass*>(*c);

    //
    // check if it has already been dumped...
    //
    for (unsigned int i = 0; i < dumpedClasses.size(); i++) {
      if ((string)dumpedClasses[i] == cur->fullName()) {
        dumped = true;
        break;
      }
    }

    //
    // make sure the class comes from the file we care about
    //
    if (cur->location().file() != goodLoc) dumped = true;

    if (dumped == false) {
      if (printedScope == false) {
        indent(os,depth);
        os << "<scope name=\"GlobalClasses\" kind=\"global\">" <<
          endl;
        printedScope = true;
      }
      
      if (libraryLanguage == LANG_C)
        outputClass(cur,os,depth+1,false,true);
      else
        outputClass(cur,os,depth+1,false,false);

      dumpedClasses.push_back(cur->fullName());
    }

    dumped = false;
  }

  if (printedScope == true) {
    indent(os,depth);
    os << "</scope>" << endl;
  }
  
  return 0;
}

/**
 * XML Generator Main : start things up.
 */
int main(int argc, char *argv[]) {
  bool    errflag = argc < 3;
  filebuf outfb;
  bool    forceC = false;

  //
  // default behaviour
  //
  dumpingStatements = false;
  dumpingLocations = false;

  //
  // really braindead argument processing going on here...
  if (errflag) {
    cerr << "usage: " << argv[0] << " [-c] [-l] [-s] outfile.xml " <<
      "[file1.pdb file2.pdb ... filen.pdb]" << endl << endl;
    cerr << "       -s : optional flag.  causes generator to emit statement level info." << endl;
    cerr << "       -l : optional flag.  causes generator to emit locations with -s." << endl;
    cerr << "       -c : optional flag.  forces generator to assume C, not C++." << endl << endl;
    return 0;
  }
  
  int startArg = 1;
  bool keepGoing = true;
  while (keepGoing) {
    if (startArg > argc) {
      keepGoing = false;
    } else if (strcmp(argv[startArg],"-c") == 0) {
      startArg++;
      forceC = true;
    } else if (strcmp(argv[startArg],"-s") == 0) {
      dumpingStatements = true;
      startArg++;
    } else if (strcmp(argv[startArg],"-l") == 0) {
      dumpingLocations = true;
      startArg++;
    } else {
      keepGoing = false;
    }
  }

  // open output XML file
  outfb.open(argv[startArg],ios::out);
  ostream os(&outfb);
  ostream *osp = &os;

  startArg++;

  // doctype
  os << "<?xml version=\"1.0\"?>" << endl;
  os << "<!DOCTYPE library SYSTEM \"mapping.dtd\">" << endl;
  os << endl;

  //
  // iterate over all PDB files, each being considered a library of its
  // own.
  //
  for (int i = startArg; i < argc; i++) {    
    // create a new ductape PDB object
    PDB pdb(argv[i]);

    PDB::filevec fvv = pdb.getFileVec();

    //
    // assume the last file in a PDB is the one we care about and that
    // the rest are before it since in general #includes go at the TOP and
    // get processed before the contents of the file doing the including.
    // this isn't the slickest, cleanest or sanest way to do it - as ben
    // pointed out, it's easy to mess it up and we don't know if it is 
    // always the case that the file we want will be the last in the vector.
    // later maybe some form of dependency analysis of the files from the
    // compiler can help us figure out which one is the right one.
    //
    for (PDB::filevec::const_iterator f = fvv.begin();
         f != fvv.end();
         ++f) {
      goodLoc = static_cast<pdbFile *>(*f);
    }

    // reset the language flag
    libraryLanguage = (language_t)0;

    // a little header comment in the XML file telling source info
    // and creation info.
    os << "<!-- " << argv[startArg-1] << " : automatically generated XML -->" 
       << endl;
    os << "<!-- Source PDB file : " << argv[i] << " -->" << endl;
    
    // output the outermost tag for the library
    os << "<library ";
    
    //
    // determine the language.  based on this we will call different
    // routines later to dump the XML.
    //
    if (forceC) {
      libraryLanguage = LANG_C;
      os << "lang=\"c\" ";
    } else {
      switch (pdb.language()) {
      case PDB::LA_C:
        libraryLanguage = LANG_C;
        os << "lang=\"c\" ";
        break;
      case PDB::LA_CXX:
      case PDB::LA_C_or_CXX:
        libraryLanguage = LANG_CXX;
        os << "lang=\"cxx\" ";
        break;
      case PDB::LA_FORTRAN:
        libraryLanguage = LANG_FORTRAN;
        os << "lang=\"fortran\" ";
        break;
      default:
        os << "lang=\"unknown\" ";
        cerr << "Error: unknown language in PDB file (" << pdb.language() <<
          ")" << endl;
      }
    }
    
    // 
    // if language wasn't set, something is wrong so we bail out.  the OS
    // file will contain a partially completed <LIBRARY tag, but it was
    // obviously messed up from the beginning anyways - so who cares?
    //
    if (libraryLanguage == 0) {
      cerr << "Error: PDB file did not contain a valid language identifier."
	   << endl;
      exit(0);
    }

    //
    // get version from pdb object -- I think this is PDT version, but ...
    //
    os << "version=\"" << pdb.version() << "\" ";
    
    //
    // name the library after the first filename
    //
    os << "name=\"" << goodLoc->name() << "\">" << endl << endl;

    //
    // assuming this is the file, try to figure out includes
    //
    os << "<!-- include files that the topmost file includes -->" << endl;
    const pdbFile::incvec ivec = goodLoc->includes();
    for (pdbFile::incvec::const_iterator iv = ivec.begin();
         iv != ivec.end();
         ++iv) {
      const pdbFile *ivf = static_cast<pdbFile *>(*iv);
      indent(os,1);
      os << "<include filename=\"" << ivf->name() << "\" />" << endl;
    }
    
    // ************************
    //  DEAL WITH FORTRAN 
    // ************************

    if (libraryLanguage == LANG_FORTRAN) {
      // dump fortran modules
      //
      os << "<!-- fortran module scopes -->" << endl;
      
      outputModules(pdb.getModuleVec(),*osp,1);
      
      // 
      // dump routines not in modules
      //
      os << "<!-- functions NOT contained in a scope -->" << endl;
      outputFRoutines(pdb.getFRoutineVec(),*osp,1);
      os << endl;
      
      os << "<!-- types NOT contained in a scope -->" << endl;
      outputFTypes(pdb.getTypeVec(),*osp,1);
      os << endl;
    } else if (libraryLanguage == LANG_C || libraryLanguage == LANG_CXX) {
      // ************************
      //  DEAL WITH C and C++
      // ************************

      if (libraryLanguage == LANG_CXX) {
	// first deal with the C++ side of this inbred language family...
	//
	os << endl << "<!-- c++ namespaces and classes -->" << endl;

	outputNamespaces(pdb.getNamespaceVec(),*osp,1);
      }
      
      // now classes NOT in namespaces
      // (this means structs too...)
      os << endl << "<!-- c++ classes not contained in namespaces "
         << "and C structs -->" 
         << endl;
      
      outputClasses(pdb.getClassVec(),*osp,1);

      // what is left is the global C++ stuff outside classes and such, and
      // the C side of things.

      os << endl << "<!-- C and C++ routines not contained elsewhere --> "
         << endl;
      outputCRoutines(pdb.getCRoutineVec(),*osp,1);

      //
      // now we need to go over global variables and typedefs
      //
      os << endl << "<!-- C and C++ global typedefs and variables --> " 
         << endl;
      outputCTypes(pdb.getTypeVec(),*osp,1);

    } else {
      // we should never get here.
      //
      cerr << "Big ol' fatal error: This should REALLY not happen.\n" << endl;
      exit(0);
    }

    // close the library tag
    os << endl << "</library>" << endl << endl;
  }

  // close output XML file
  outfb.close();

  return 0;
}
