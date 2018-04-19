/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifdef _MSC_VER
 #include "getopt.h"
#else
 #include <unistd.h>
#endif
#include "pdb.h"

int main(int argc, char *argv[]) {
  int ch;
  char *outfile = 0;
  bool errflag = argc < 4;
  bool verbose = false;

  while ( (ch = getopt(argc, argv, "o:v")) != -1 ) {
    switch (ch) {
    case 'o': outfile = optarg;
              break;
    case 'v': verbose = true;
              break;
    case '?': errflag = true;
              break;
    }
  }
  if ( errflag ) {
    cerr << "usage: " << argv[0] << " [[-v] -o outfile] pdbfile..." << endl;
    cerr << "pdbmerge  : Takes a set of program database files and merges them into one,\n"
            "eliminating duplicate items from different files, in the process.\n"
            "Usage:\n\n"

              "pdbmerge [ [-v] -o <outfile>] <pdbfiles>...\n\n"

            "-v            Verbose.\n\n"

            "-o <outfile>  Write merged database to file <outfile> instead of\n"
                          "cout.\n\n"

            "Note:  Namespace definitions spread over several files are\n"
            "not merged correctly yet.\n";

    
    return 1;
  }

  if ( outfile && verbose )
    cout << "opening " << argv[optind] << " ..." << endl;
  PDB pdb(argv[optind++]);

  if ( pdb ) {
    for (; optind<argc; ++optind) {
      if ( outfile && verbose )
        cout << "merging " << argv[optind] << " ..." << endl;
      if ( ! pdb.merge(argv[optind]) ) return 1;
    }
  }
  if ( outfile ) {
    if ( verbose ) cout << "writing " << outfile << " ..." << endl;
    pdb.write(outfile);
  } else
    pdb.write(cout);
  return 0;
}
