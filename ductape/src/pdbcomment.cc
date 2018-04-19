/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/* University of Oregon, Department of Computer and Information Science  */
/*************************************************************************/

#ifdef _OLD_HEADER_
# include <fstream.h>
# include <algo.h>
# include <vector.h>
#else
# include <fstream>
  using std::ifstream;
  using std::ofstream;
# include <algorithm>
  using std::transform;
#include <vector>
  using std::vector;
# include <string>
  using std::getline;
#endif

extern void pdb_ERROR(const char *msg, const char *item, int val);
#ifdef _MSC_VER
 #include "getopt.h"
#else
 #include <unistd.h>
#endif
#include <ctype.h>
#include "pdbAll.h"
#include "pdbTraits.h"
#include "pdbTdefs.h"

//---------------------------------------------------------------------------
namespace {
  int pragma_id = 0;
  int comment_id = 0;
  bool gen_comments = true;
  bool gen_pragmas = true;
  vector<string> directives;
  char dComChar = 'c';

  struct fo_tolower : public std::unary_function<int,int> {
    int operator()(int x) const {
      return tolower(x);
    }
  };

  /*
   * find_[cf]_next_word:  find next word in preprocessor statement
   *                  IN:  preStmt   :  lines of preprocessor statement 
   *                  IN:  size      :  number of lines
   *                  IN:  pline, pos:  start location for search
   *                 OUT:  pline, pos:  start location for next statement
   *                 OUT:  wbeg      :  start position of word
   */
  string find_c_next_word(vector<string>& preStmt, unsigned size,
                          unsigned& pline, string::size_type& ppos,
                          string::size_type& wbeg) {
    while ( pline < size ) {
      wbeg = preStmt[pline].find_first_not_of(" \t", ppos);
      if ( preStmt[pline][wbeg] == '\\' || wbeg == string::npos ) {
        ++pline;
        if ( pline < size ) { ppos = 0; } else { return ""; }
      } else {
        ppos = preStmt[pline].find_first_of(" \t()", wbeg);
        return preStmt[pline].substr(wbeg,
                                     ppos==string::npos ? ppos : ppos-wbeg);
      }
    }
    return "";
  }

  string find_f_next_word(vector<string>& cStmt, unsigned size,
                          const string& sentinel,
                          unsigned& pline, string::size_type& ppos) {
    while ( pline < size ) {
      string::size_type wbeg = cStmt[pline].find_first_not_of(" \t", ppos);
      if ( cStmt[pline][wbeg] == '&' ) {
	ppos = wbeg+1;
      } else if ( wbeg == string::npos ) {
        ++pline;
        if ( pline < size ) {
	  ppos = cStmt[pline].find(sentinel) + sentinel.size() + 1;
        } else {
          return "";
        }
      } else if ( cStmt[pline][wbeg] == '(' || cStmt[pline][wbeg] == ')' ) {
        ppos = wbeg+1;
        return string(1, cStmt[pline][wbeg]);
      } else {
        ppos = cStmt[pline].find_first_of(" \t()", wbeg);
        return cStmt[pline].substr(wbeg, ppos==string::npos ? ppos : ppos-wbeg);
      }
    }
    return "";
  }

  /*
   * find_[cf]_kind:  find next word in preprocessor statement
   *             IN:  preStmt    :  lines of preprocessor statement 
   *             IN:  size       :  number of lines
   *             IN:  name       :  name of pragma
   *             IN:  kline, kpos:  start location for search
   */
  string find_c_kind(vector<string>& preStmt, unsigned size, const string& name,
                     unsigned kline, string::size_type kpos) {
    string::size_type kbeg = 0;
    string kind = find_c_next_word(preStmt, size, kline, kpos, kbeg);

    // special omp handling
    if ( name == "omp" ) {
      string w1 = kind, w2;

      if ( kind == "end" ) {
	w1 = find_c_next_word(preStmt, size, kline, kpos, kbeg);
	kind += w1;
      }
      if ( w1 == "parallel" ) {
	  string w2 = find_c_next_word(preStmt, size, kline, kpos, kbeg);
	  if ( w2 == "do" || w2 == "for" ||
	       w2 == "sections" || w2 == "workshare" ) kind += w2;
      }
    }
    return kind;
  }

  string find_f_kind(vector<string>& cStmt, unsigned size, const string& name,
                     unsigned kline, string::size_type kpos) {
    string kind = find_f_next_word(cStmt, size, name, kline, kpos);

    // special omp handling
    if ( name == "$omp" ) {
      string w1 = kind, w2;

      if ( kind == "end" ) {
	w1 = find_f_next_word(cStmt, size, name, kline, kpos);
	kind += w1;
      }
      if ( w1 == "parallel" ) {
	  string w2 = find_f_next_word(cStmt, size, name, kline, kpos);
	  if ( w2 == "do" || w2 == "for" ||
	       w2 == "sections" || w2 == "workshare" ) kind += w2;
      }
    }
    return kind;
  }

  /*
   * process_preStmt:  determine if preprocessor statement is pragma
   *              IN:  preStmt     :  lines of preprocessor statement 
   *              IN:  lineno, ppos:  location of '#'
   *              IN:          file:  file pointer
   *              IN:           pdb:  program database
   */
  bool process_preStmt(vector<string>& preStmt, int lineno,
                       string::size_type ppos, pdbFile* file, PDB& pdb) {
    unsigned s = preStmt.size();
    bool inComment = false;
    string ctext;
    int cli = 0, cco = 0;

    // remove comments
    for (unsigned i=0; i<s; ++i) {
      string::size_type pos = 0;
      string& line = preStmt[i];
      while ( pos < line.size() ) {
        if ( inComment ) {
          // look for comment end
          if ( line[pos] == '*' && line[pos+1] == '/' ) {
	    ctext += "*/";
            line[pos++] = ' ';

	    if ( gen_comments ) {
	      pdbComment* com = file->addComment(comment_id++);
	      com->kind(PDB::LA_C);
	      com->cmtBegin(pdbLoc(file, cli, cco));
	      com->cmtEnd(pdbLoc(file, lineno+i, pos+1));
	      com->text(ctext);
	    }
            ctext = "";
            inComment = false;
          } 

	  // otherwise copy but handle end of line correctly
	  if ( pos == (line.size()-1) ) {
            ctext += "\\n";
            line[pos++] = '\\';
	  } else {
	    ctext += line[pos];
            line[pos++] = ' ';
	  }
        } else if ( line[pos] == '/' ) {
          pos++;
          if ( line[pos] == '/' ) {
            // c++ comments
	    cli = lineno + i;
	    cco = pos;
	    ctext = line.substr(pos-1);
            line[pos-1] = ' ';
            line[pos++] = ' ';
            while ( pos < line.size() ) { line[pos++] = ' '; }

	    if ( gen_comments ) {
	      pdbComment* com = file->addComment(comment_id++);
	      com->kind(PDB::LA_CXX);
	      com->cmtBegin(pdbLoc(file, cli, cco));
	      com->cmtEnd(pdbLoc(file, lineno+i, pos));
	      com->text(ctext);
	    }
            ctext = "";
          } else if ( line[pos] == '*' ) {
            // c comment start
	    cli = lineno + i;
	    cco = pos;
	    ctext = "/*";
            line[pos-1] = ' ';
            line[pos++] = ' ';
            inComment = true;
          }
        } else {
          pos++;
        }
      }
    }

    // pragma?
    unsigned nline = 0;
    string::size_type npos = ppos;
    string::size_type nbeg = 0;
    if ( find_c_next_word(preStmt, s, nline, npos, nbeg) == "pragma" ) {
      // determine name (= next word after pragma)
      string name = find_c_next_word(preStmt, s, nline, npos, nbeg);

      // determine kind (= next word after name)
      string kind = find_c_kind(preStmt, s, name, nline, npos);

      // generate pragma text the PDB way
      string ptext;
      for (unsigned i=0; i<s; ++i) {
	if ( i )
          ptext += preStmt[i];
	else
          ptext += preStmt[i].substr(ppos-1);
        if ( i != (s-1) ) ptext += "n";
      }

      if ( gen_pragmas ) {
        pdbPragma* pr = pdb.findItem(PDB::pragmaTag(), name, ++pragma_id);
	pr->kind(kind);
        pr->location(pdbLoc(file, lineno+nline, nbeg+1));
        pr->prBegin(pdbLoc(file, lineno, ppos));
        pr->prEnd(pdbLoc(file, lineno+s-1, preStmt[s-1].size()));
        pr->text(ptext);
      }
    }
    return inComment;
  }

  /*
   * get_fcom_prefix:  determine fortran comment prefix
   *              IN:  line:  comment line
   */
  string get_fcom_prefix(const string& line) {
    for (int i=0; i<directives.size(); ++i) {
      string& dir = directives[i];
#if (__GNUC__ == 2)
      if ( strncmp(line.c_str(), dir.c_str(), dir.size()) == 0) return dir; 
#else
      if ( line.compare(1, dir.size(), dir) == 0 ) return dir;
#endif /* GNU bug */
    }
    return line.substr(0,1);
  }

   /*
    * has_no_cont:  determine whether f90 directive has continuation line
    *               => last non-blank, non-comment character is '&'
    *          IN:  line:  comment line
    *          IN:   pos:  starting position for possible inline comment
    */
  bool has_no_cont(const string& line, int pos) {
    string::size_type com = line.find("!", pos);
    if ( com != string::npos ) --com;
    string::size_type amp = line.find_last_not_of(" \t", com);
    return ( line[amp] != '&' );
  }

  /*
   * process_fcom:  process fortran comment block
   *           IN:  cStmt      :  lines of fortran comment block
   *           IN:  sline, scol:  start location
   *           IN:         ecol:  end   location
   *           IN:         file:  file pointer
   *           IN:          pdb:  program database
   *           IN:       prefix:  comment prefix
   */
  void process_fcom(vector<string>& cStmt, int sline, int scol, int ecol,
                    pdbFile* file, PDB& pdb, const string& prefix) {
    printf ("prefix=%s\n", prefix.c_str());
    if ( prefix.size() > 1 ) {
      // directive/pragma
      if ( gen_pragmas ) {
        // process pragma text
        string ptext;
        unsigned s = cStmt.size();
        for (unsigned i=0; i<s; ++i) {
	  string& lowline = cStmt[i];
	  string line = lowline;
          transform(lowline.begin(), lowline.end(),
	            lowline.begin(), fo_tolower());

          string::size_type pos = string::npos;
	  pos = lowline.find(prefix) + prefix.size() + 1;
	  if ( (pos = lowline.find("!", pos)) != string::npos ) {
	    // inline comment; erase from pragma text
	    if ( gen_comments ) {
	      pdbComment* com = file->addComment(comment_id++);
	      com->kind(PDB::LA_FORTRAN);
	      com->cmtBegin(pdbLoc(file, sline+i, pos+1));
	      com->cmtEnd(pdbLoc(file, sline+i, cStmt[i].size()));
	      com->text(line.substr(pos));
	    }
	    lowline.erase(pos);
	  }
          ptext += lowline;
          if ( i != (s-1) ) ptext += "\\n";
        }

        // determine kind (= next word after name)
        string kind = find_f_kind(cStmt, s, prefix, 0, scol+prefix.size()+1);

	// generate pragma item
        pdbPragma* pr = pdb.findItem(PDB::pragmaTag(),
	                             prefix=="$omp"?"omp":prefix, ++pragma_id);
	pr->kind(kind);
        pr->location(pdbLoc(file, sline, scol+1));
        pr->prBegin(pdbLoc(file, sline, scol));
        pr->prEnd(pdbLoc(file, sline+s-1, ecol));
        pr->text(ptext);
      }
    } else {
      // plain comment
      if ( gen_comments ) {
        // generate comment text the PDB way
        string ctext;
        unsigned s = cStmt.size();
        for (unsigned i=0; i<s; ++i) {
          ctext += cStmt[i];
          if ( i != (s-1) ) ctext += "\\n";
        }
	
	// generate comment entry
        pdbComment* com = file->addComment(comment_id++);
        com->kind(PDB::LA_FORTRAN);
        com->cmtBegin(pdbLoc(file, sline, scol));
        com->cmtEnd(pdbLoc(file, sline+s-1, ecol));
        com->text(ctext);
      }
    }
    cStmt.clear();
  }

  /*
   * find_[fc]_comments_in:  find comments and pragmas in file 'name'
   *                    IN:  file:  file pointer
   *                    IN:  pdb :  program database
   */
  void find_c_comments_in(pdbFile* file, PDB& pdb) {
    const string& name = file->name();
    string line;
    bool inComment = false;
    bool preContLine = false;
    int lineno = 1;
    string::size_type pos = 0;
    string::size_type lstart = string::npos;
    vector<string> preStmt;
    string ctext;
    int cli = 0, cco = 0;

    ifstream is(name.c_str());
    if ( !is ) {
      cerr << "ERROR: cannot open " << name << endl;
      return;
    }

    comment_id = 0;

    while ( getline(is, line) ) {
      if ( preContLine ) {
        /*
         * preprocessor directive continuation
         */
        preStmt.push_back(line);
        if ( line[line.size()-1] != '\\' ) {
          preContLine = false;
          inComment = process_preStmt(preStmt, lineno-preStmt.size()+1,
	                              lstart+1, file, pdb);
          preStmt.clear();
        }

      } else if ( !inComment && 
           ((lstart = line.find_first_not_of(" \t")) != string::npos) &&
           line[lstart] == '#' ) {
        /*
         * preprocessor directive
         */
        preStmt.push_back(line);
        if ( line[line.size()-1] == '\\' ) {
          preContLine = true;
        } else {
          inComment = process_preStmt(preStmt, lineno, lstart+1, file, pdb);
          preStmt.clear();
        }
  
      } else {
        /*
         * regular line
         */
        while ( pos < line.size() ) {
          if ( inComment ) {
            // look for comment end
            if ( line[pos] == '*' && line[pos+1] == '/' ) {
              ctext += "*/";
              inComment = false;
              pos += 2;
	      if ( gen_comments ) {
	        pdbComment* com = file->addComment(comment_id++);
	        com->kind(PDB::LA_C);
	        com->cmtBegin(pdbLoc(file, cli, cco));
	        com->cmtEnd(pdbLoc(file, lineno, pos));
	        com->text(ctext);
	      }
              ctext = "";
            } else {
              ctext += line[pos++];
            }

          } else if ( line[pos] == '/' ) {
            pos++;
            if ( line[pos] == '/' ) {
              // c++ comments
              pos++;
              cli = lineno;
              cco = pos-1;
              ctext += "//";
              while ( pos < line.size() ) { ctext += line[pos++]; }
	      if ( gen_comments ) {
	        pdbComment* com = file->addComment(comment_id++);
	        com->kind(PDB::LA_CXX);
	        com->cmtBegin(pdbLoc(file, cli, cco));
	        com->cmtEnd(pdbLoc(file, lineno, pos));
	        com->text(ctext);
	      }
              ctext = "";
            } else if ( line[pos] == '*' ) {
              // c comment start
              pos++;
              ctext += "/*";
              inComment = true;
              cli = lineno;
              cco = pos-1;
            }

          } else if ( line[pos] == '\"' ) {
            // character string constant
            do {
              pos++;
              if ( line[pos] == '\\' ) {
                pos++;
                if ( line[pos] == '\"' ) { pos++; }
              }
            }
            while ( line[pos] != '\"' && pos < line.size());
            pos++;

          } else if ( line[pos] == '\'' ) {
            // character constant
            do {
              pos++;
              if ( line[pos] == '\\' ) {
                pos++;
                if ( line[pos] == '\'' ) { pos++; }
              }
            }
            while ( line[pos] != '\'' && pos < line.size() );
            pos++;
          } else {
            pos++;
          }
        }
      }
      if ( inComment ) ctext += "\\n";
      ++lineno;
      pos = 0;
    }
  }

  void find_f_comments_in(pdbFile* file, PDB& pdb) {
    const string& name = file->name();
    string line;
    int lineno = 0;
    int cli = 0, cco = 0, cstart = 0, cend = 0;
    bool inString1 = false, inString2 = false;
    vector<string> cStmt;
    string currComPrefix, lastComPrefix;

    ifstream is(name.c_str());
    if ( !is ) {
      cerr << "ERROR: cannot open " << name << endl;
      return;
    }

    comment_id = 0;

    while ( getline(is, line) ) {
      ++lineno;
      string lowline(line);
      transform(line.begin(), line.end(), lowline.begin(), fo_tolower());

      if ( line.size() &&
           (lowline[0] == 'c' || lowline[0] == '*' ||
	    lowline[0] == dComChar ) ) {
	/*
         * fix form comment
         */
	currComPrefix = get_fcom_prefix(lowline);
	if ( cStmt.empty() ) {
	  // new comment block
          cli = lineno;
          cco = 1;
	  lastComPrefix = currComPrefix;
	} else if ( lastComPrefix != currComPrefix ) {
	  // new comment block because different prefix
	  process_fcom(cStmt, cli, cco, cend, file, pdb, lastComPrefix);
          cli = lineno;
          cco = cstart+1;
	  lastComPrefix = currComPrefix;
	} else if ( currComPrefix.size() > 1 &&
	            (line[5] == ' ' || line[5] == '0') ) {
	  // new comment block because start of new directive
	  // (no directive continuation line)
	  process_fcom(cStmt, cli, cco, cend, file, pdb, lastComPrefix);
          cli = lineno;
          cco = 1;
	}
	cStmt.push_back(line);
	cend = line.size();
      } else if ( line.size() &&
        (cstart = lowline.find_first_not_of(" \t")) != string::npos &&
	lowline[cstart] == '!') {
	/*
         * free form full line comment
         */
	currComPrefix = get_fcom_prefix(lowline.substr(cstart));
	if ( cStmt.empty() ) {
	  // new comment block
          cli = lineno;
          cco = cstart+1;
	  lastComPrefix = currComPrefix;
	} else if ( lastComPrefix != currComPrefix ) {
	  // new comment block because different prefix
	  process_fcom(cStmt, cli, cco, cend, file, pdb, lastComPrefix);
          cli = lineno;
          cco = cstart+1;
	  lastComPrefix = currComPrefix;
	}
	
	cStmt.push_back(line);
	cend = line.size();

	if ( currComPrefix.size() > 1 && has_no_cont(line, cstart+1) ) {
	  // end of pragma (no directive continuation line)
	  process_fcom(cStmt, cli, cco, cend, file, pdb, lastComPrefix);
	}
      } else {
        /*
         * normal line
         */
	// commit comment block if not done already
	if ( cStmt.size() ) {
	  process_fcom(cStmt, cli, cco, cend, file, pdb, lastComPrefix);
        }

	// scan line for inline comments ignoring character constants
	int pos=0;
        while ( pos < line.size() ) {
	  if ( inString1 ) {
	    if ( line[pos] == '\'' ) {
              pos++;
	      // ' is actually continuation character?
	      if ( pos == 6 && line.find_first_not_of(" \t") == 5 ) continue;
	      // only single ' does end string
	      if ( line[pos] != '\'' ) {
		inString1 = false;
              }
	    }
          } else if ( inString2 ) {
	    if ( line[pos] == '\"' ) {
              pos++;
	      // " is actually continuation character?
	      if ( pos == 6 && line.find_first_not_of(" \t") == 5 ) continue;
	      // only single " does end string
	      if ( line[pos] != '\"' ) {
		inString2 = false;
              }
	    }
	  } else if ( line[pos] == '\'' ) {
	    inString1 = true;
	  } else if ( line[pos] == '\"' ) {
	    inString2 = true;
	  } else if ( line[pos] == '!' ) {
	    if ( gen_comments ) {
	      pdbComment* com = file->addComment(comment_id++);
	      com->kind(PDB::LA_FORTRAN);
	      com->cmtBegin(pdbLoc(file, lineno, pos+1));
	      com->cmtEnd(pdbLoc(file, lineno, line.size()));
	      com->text(line.substr(pos));
	    }
	    break;
	  }
	  pos++;
	}
      }
    }
    // commit comment block at end of file
    if ( cStmt.size() ) {
      process_fcom(cStmt, cli, cco, cend, file, pdb, lastComPrefix);
    }
  }
}

//---------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  int ch;
  bool errflag = argc < 2;
  bool verbose = false;
  char *outfile = 0;
  directives.push_back("$omp");

  while ( (ch = getopt(argc, argv, "cdo:pvD:")) != -1 ) {
    switch (ch) {
    case 'c': gen_comments = true;
	      gen_pragmas  = false;
	      break;
    case 'p': gen_pragmas  = true;
	      gen_comments = false;
	      break;
    case 'd': dComChar = 'd';
	      break;
    case 'v': verbose = true;
	      break;
    case 'o': outfile = optarg;
	      break;
    case 'D': {
		string dir = optarg;
                transform(dir.begin(), dir.end(), dir.begin(), fo_tolower());
		directives.push_back(dir);
	      }
	      break;
    case '?': errflag = true;
	      break;
    }
  }
  if ( errflag ) {
    cerr << "usage: " << argv[0]
         << " [-o outfile] [-v] [-c|-p] [-d] [-D directive] pdbfile" << "\n\n";

    cerr << "Scans all (non-system) source files related to a PDB file \n"
      "for C, C++, Fortran comments, C/C++ pragmas, and Fortran\n"
      "directives and prints out a new enhanced PDB file containing\n"
      "this additional information.\n"
      "\n"
      "Options:\n\n"
      "     -o <outfile>  Write output to file <outfile>\n\n"
      "     -c            Only scan for comments (ignore pragmas)\n\n"
      "     -p            Only scan for pragmas (ignore comments)\n\n"
      "     -d            Fortran only: Consider lines with a 'D'\n"
      "                   in the first column also as comments\n\n"
      "     -v            Verbose mode\n\n"
      "     -D string     Fortran only: Scan also for directives\n"
      "                   which are marked with the sentinel 'string'.\n"
      "                   This option can be specified several times,\n"
      "                   once for each directive sentinel to scan for.\n"
      "                   Case does NOT matter when specifiying 'string'.\n"
      "                   pdbcomment recognizes OpenMP (sentinel $omp)\n"
      "                   by default.\n\n";





    return 1;
  }

  // open and initialize program database
  PDB pdb(argv[optind]);
  if ( !pdb ) return 1;

  // build file list
  PDB::filevec& files = pdb.getFileVec();

  for(PDB::filevec::iterator fi=files.begin(); fi!=files.end(); ++fi) {
    if ( ! (*fi)->isSystemFile() ) {
      if ( verbose ) cout << "Processing " << (*fi)->name() << " ..." << endl;
      if ( pdb.language() & PDB::LA_C_or_CXX ) {
        find_c_comments_in(*fi, pdb);
      } else {
        find_f_comments_in(*fi, pdb);
      }
    } else {
      if ( verbose ) cout << "Ignoring " << (*fi)->name() << " ..." << endl;
    }
  }

  // commit pragma items
  if ( gen_pragmas ) pdb.finalCheck(PDB::pragmaTag());

  // preare output
  if ( outfile ) {
    if ( verbose ) cout << "Writing " << outfile << " ..." << endl;
    pdb.write(outfile);
  } else {
    pdb.write(cout);
  }

  return 0;
}


