/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/* University of Oregon, Department of Computer and Information Science  */
/*************************************************************************/

/*??? #include <bits/types.h>*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#ifdef _OLD_HEADER_
# include <fstream.h>
# include <set.h>
# include <algo.h>
#else
# include <fstream>
   using std::ifstream;
   using std::ofstream;
# include <set>
   using std::set;
# include <algorithm>
   using std::sort;
#endif
#include "pdbAll.h"

//----------------------------------------------------------------------
#define NUMBER_OF_KEYWORDS 73

static const char *keywords[NUMBER_OF_KEYWORDS] = {
   "and", "and_eq", "asm", "auto", 
   "bitand", "bitor", "bool", "break",
   "case", "catch", "char", "class", "compl", "const", "const_cast", "continue", 
   "default", "delete", "do", "double", "dynamic_cast", 
   "else", "enum", "explicit", "extern", 
   "false", "float", "for", "friend", 
   "goto",
   "if", "inline", "int", 
   "long", 
   "mutable", 
   "namespace", "new", "not", "not_eq", 
   "operator", "or", "or_eq", 
   "private", "protected", "public",
   "register", "reinterpret_cast", "return", 
   "short", "signed", "sizeof", "static", "static_cast", "struct", "switch", 
   "template", "this", "throw", "true", "try", "typedef", "typeid", "typename", 
   "union", "unsigned", "using", 
   "virtual", "void", "volatile", 
   "wchar_t", "while", 
   "xor", "xor_eq"
};

//----------------------------------------------------------------------
struct ltstr {
   bool operator()(const char* s1, const char* s2) const {
      return strcmp(s1, s2) < 0;
   }
};

//----------------------------------------------------------------------
static bool itemCmp(const pdbSimpleItem* i1, const pdbSimpleItem* i2) {
   return i1->name() < i2->name();
}

//----------------------------------------------------------------------
static void out(ostream& ostr, char ch) {
   switch( ch ) {
      case '<': ostr << "&lt;";  break;
      case '>': ostr << "&gt;";  break;
      case '&': ostr << "&amp;"; break;
      default : ostr << ch;      break;
   }
}

//----------------------------------------------------------------------
static void outStr(ostream& ostr, const string& str) {
   for (unsigned i=0; i<str.size(); ++i) out(ostr, str[i]);
}

//----------------------------------------------------------------------
struct filenamedata {
   filenamedata(string pth) : path(pth) {
      path = pth;
   }
   filenamedata(string orig, string nm, bool dup, string pth, 
                string incform, string htmlnm) : 
                original(orig), name(nm), dupname(dup), path(pth), 
               includeform(incform), htmlname(htmlnm)  {
      original = orig;
      name = nm;
      dupname = dup;
      path = pth;
      includeform = incform;
      htmlname = htmlnm;
   }
   string original;
   string name;
   bool dupname;
   string path;
   string includeform;
   string htmlname;
};

//----------------------------------------------------------------------
bool filenamedataCmp( filenamedata* f1, filenamedata* f2) {
   if( f1->name == f2->name ) {
      return f1->original < f2->original;
   } else {
      return f1->name < f2->name;
   }
}


//----------------------------------------------------------------------
void exclude_this( string which, vector<filenamedata*>& rcsysfiles, 
                     string rcline ) {
   string::size_type path_location;

   if( ( which == "string" ) 
         || ( string::npos == rcline.find_first_of( "/", 0 ) ) ) {
      path_location = rcline.find_last_of( " \t" );
      if( path_location == string::npos ) return;
      ++path_location;
      if( path_location == string::npos ) return;
      string thing = rcline.substr(path_location);
      rcsysfiles.push_back(new filenamedata(rcline,thing,false,thing,
                                             thing,thing) );
   } else {
      path_location = rcline.find_first_of( "/", 0 );

      if( path_location == string::npos ) return;

      string name("");
      string path = rcline.substr(path_location);
      path_location = path.find_last_of( "/" );
      if( ++path_location != string::npos ) {
         name = path.substr(path_location);
      }
      rcsysfiles.push_back(new filenamedata(rcline,name,false,path,
                                             path,name) );
   } 
      
}


//----------------------------------------------------------------------
const int INBUF_SIZE = 2048;
const int STYLE_SIZE = 32;
const int COLOR_SIZE = 7;

//----------------------------------------------------------------------
struct run_control {
   run_control() {}

   vector<filenamedata *> exclude_files;
   vector<filenamedata *> exclude_strings;

   vector<filenamedata *> read_src_files_from;
   filenamedata * write_html_files_to;
   filenamedata * find_doc_files_in;


   bool add_line_numbers;
   bool just_build_pdbhtmlrc_html;
   bool add_backtoindex_footer;
   bool list_methods_in_classes;
   bool add_doc_files;
   bool exit_on_missing_src_file;
   bool print_error_messages;
   bool print_warning_messages;

   char slash_replacement;
   char dot_replacement;

   char style_text[STYLE_SIZE];
   char style_link[STYLE_SIZE];
   char style_alink[STYLE_SIZE];
   char style_vlink[STYLE_SIZE];
   char style_comments[STYLE_SIZE];
   char style_constants[STYLE_SIZE];
   char style_preprocessor[STYLE_SIZE];
   char style_linenumbers[STYLE_SIZE];
   char style_target[STYLE_SIZE];
   char style_keyword[STYLE_SIZE];

   char color_bgcolor[COLOR_SIZE+1];
   char color_text[COLOR_SIZE+1];
   char color_link[COLOR_SIZE+1];
   char color_alink[COLOR_SIZE+1];
   char color_vlink[COLOR_SIZE+1];
   char color_comments[COLOR_SIZE+1];
   char color_constants[COLOR_SIZE+1];
   char color_preprocessor[COLOR_SIZE+1];
   char color_linenumbers[COLOR_SIZE+1];
   char color_target[COLOR_SIZE+1];
   char color_keyword[COLOR_SIZE+1];

};

//----------------------------------------------------------------------
void linenumbers( ostream& ostr, struct run_control * rc , int line,
                  bool inComment, bool inPreStmt, bool * inPreFont ) {
   if( rc->add_line_numbers == true ) {

      if ( inComment ) {
         ostr << "</FONT>";
         if ( strlen( rc->style_comments ) > 0 ) {
            ostr << "</" << rc->style_comments << ">";
         }
      }
      if ( strlen( rc->style_linenumbers ) > 0 ) {
         ostr << "<" << rc->style_linenumbers << ">";
      }
      ostr << "<FONT COLOR=" << rc->color_linenumbers << ">"
           << line << ": </FONT>";
      if ( strlen( rc->style_linenumbers ) > 0 ) {
         ostr << "</" << rc->style_linenumbers << ">";
      }
      if ( inComment && strlen( rc->style_comments ) > 0 ) {
         ostr << "<" << rc->style_comments << ">";
      }
      if( inComment ) {
         ostr << "<FONT COLOR=" << rc->color_comments << ">";
      }
      if ( inPreStmt && strlen( rc->style_preprocessor ) > 0 ) {
         ostr << "<" << rc->style_preprocessor << ">";
      }
      if( inPreStmt ) {
         ostr << "<FONT COLOR=" << rc->color_preprocessor << ">";
         *inPreFont = true;
      }
   }
}

//----------------------------------------------------------------------
void build_pdbhtmlrc_html( struct run_control * rc ) {

   string writehere;
   if( rc->write_html_files_to ) {
      writehere = rc->write_html_files_to->path + "/" + "pdbhtmlrc.html";
   } else {
      writehere = "pdbhtmlrc.html";
   }

   ofstream html(writehere.c_str());
   if (!html) {
      cerr << "Error: Cannot open 'pdbhtmlrc.html' for output." << endl;
      return;
   }

   html << "<HTML>\n<HEAD>\n<TITLE> pdbhtmlrc </TITLE>\n</HEAD>\n"
          << "<BODY BGCOLOR=" << rc->color_bgcolor
          << " TEXT=" << rc->color_text
          << " LINK=" << rc->color_link
          << " ALINK=" << rc->color_alink
          << " VLINK=" << rc->color_vlink
          << ">" << endl;

   html << "<h1 align=center> Pdbhtmlrc Values and Example </h1><hr>" << endl;


   html << "<table border=0>" << endl;
   html << "<tr> <td colspan=2> </td> </tr>" << endl;
   html << "<tr> <th>";
   html << "<h2 valign=top> Values </h2> " << endl;
   html << "</th> <th>";
   html << "<h2 valign=top> Example </h2> " << endl;

   html << "</th> </tr>";
   html << "<tr> <td>";
   html << "NOTE:<ul>";
   html << "<li>The example will always display linenumbers.<br>" << endl;
   html << "<li>Example links will be stale.<br>" << endl;
   html <<"<li>color_bgcolor is the background color for this and all other pages.";

   html << endl << "</ul><hr><pre>";



   if( 0 < strlen( rc->style_link ) ) {
      html << "<" << rc->style_link << ">";
   }
   html << "<FONT COLOR=" << rc->color_link << "> " << rc->color_link  << " "
        << rc->style_link << " color_link" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_link ) ) {
      html << "</" << rc->style_link << ">";
   }

   html << "<BR>";
   if( 0 < strlen( rc->style_alink ) ) {
      html << "<" << rc->style_alink << ">";
   }
   html << "<FONT COLOR=" << rc->color_alink << ">" << rc->color_alink << " " 
        << rc->style_alink << " color_alink" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_alink ) ) {
      html << "</" << rc->style_alink << ">";
   }

   html << "<BR>";
   if( 0 < strlen( rc->style_vlink ) ) {
      html << "<" << rc->style_vlink << ">";
   }
   html << "<FONT COLOR=" << rc->color_vlink << ">" << rc->color_vlink  << " " 
        << rc->style_vlink << " color_vlink" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_vlink ) ) {
      html << "</" << rc->style_vlink << ">";
   }

   html << "<BR>";
   if( 0 < strlen( rc->style_comments ) ) {
      html << "<" << rc->style_comments << ">";
   }
   html << "<FONT COLOR=" << rc->color_comments << ">" << rc->color_comments  << " " 
        << rc->style_comments << " color_comments" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_comments ) ) {
      html << "</" << rc->style_comments << ">";
   }

   html << "<BR>";
   if( 0 < strlen( rc->style_constants ) ) {
      html << "<" << rc->style_constants << ">";
   }
   html << "<FONT COLOR=" << rc->color_constants<< ">" << rc->color_constants  << " " 
        << rc->style_constants << " color_constants" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_constants ) ) {
      html << "</" << rc->style_constants << ">";
   }

   html << "<BR>";
   if( 0 < strlen( rc->style_preprocessor ) ) {
      html << "<" << rc->style_preprocessor << ">";
   }
   html << "<FONT COLOR=" << rc->color_preprocessor<< ">" << rc->color_preprocessor  << " " 
        << rc->style_preprocessor << " color_preprocessor" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_preprocessor ) ) {
      html << "</" << rc->style_preprocessor << ">";
   }

   html << "<BR>";
   if( 0 < strlen( rc->style_linenumbers ) ) {
      html << "<" << rc->style_linenumbers << ">";
   }
   html << "<FONT COLOR=" << rc->color_linenumbers<< ">" << rc->color_linenumbers  << " " 
        << rc->style_linenumbers << " color_linenumbers" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_linenumbers ) ) {
      html << "</" << rc->style_linenumbers << ">";
   }

   if( 0 < strlen( rc->style_target ) ) {
      html << "<" << rc->style_target << ">";
   }
   html << "<FONT COLOR=" << rc->color_target<< "><A NAME=\"xx1\">" << rc->color_target  << " " 
        << rc->style_target << " color_target</A>" << "</FONT>";
   if( 0 < strlen( rc->style_target ) ) {
      html << "</" << rc->style_target << ">";
   }


   html << "<BR>";
   if( 0 < strlen( rc->style_keyword ) ) {
      html << "<" << rc->style_keyword << ">";
   }
   html << "<FONT COLOR="<<rc->color_keyword<< ">" << rc->color_keyword  << " " 
        << rc->style_keyword << " color_keyword" << "</FONT>" << endl;
   if( 0 < strlen( rc->style_keyword ) ) {
      html << "</" << rc->style_keyword << ">";
   }

   html << "</pre>";
   html << "<hr>";


   html << "<table border=0>" << endl;
   html << "<tr> <td colspan=2> </td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Add_line_numbers</td><td>" ;
   if( 1 == rc->add_line_numbers ) {
      html << "True"<< endl;
   } else {
      html << "False"<< endl;
   } 
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Add_backtoindex_footer</td><td>" ;
   if( 1 == rc->add_backtoindex_footer ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>List_methods_in_classes</td><td>" ;
   if( 1 == rc->list_methods_in_classes ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Just_build_pdbhtmlrc_html</td><td>" ;
   if( 1 == rc->just_build_pdbhtmlrc_html ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Add_doc_files</td><td>" ;
   if( 1 == rc->add_doc_files ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Exit_on_missing_src_file</td><td>" ;
   if( 1 == rc->exit_on_missing_src_file ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Print_error_messages</td><td>" ;
   if( 1 == rc->print_error_messages ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;

   html << "<tr> <td>";
   html << "<td>Print_warning_messages</td><td>" ;
   if( 1 == rc->print_warning_messages ) {
      html << " True"<< endl;
   } else {
      html << " False"<< endl;
   }
   html << "</td> </tr>" << endl;


   html << "</table>" << endl;


   html << "<hr>";

   html << "<table border=0>" << "<tr> <td colspan=2> </td> </tr><tr>" << endl;
   html << "<td>slash_replacement</td><td>" << rc->slash_replacement << "</td>" << endl;
   html << "</tr><tr>";
   html << "<td>dot_replacement</td><td> " << rc->dot_replacement << "</td>" << endl;
   html << "</tr>" << "</table>" << endl;

   html << "<hr>";

   html << "Exclude these files:" << endl;
   vector<filenamedata*>::iterator vi;

   if( rc->exclude_files.empty() ) {
      html << "<BR> No files are specified.<BR>" << endl;
   } else {
      for( vi = rc->exclude_files.begin(); vi != rc->exclude_files.end(); vi++ ) {
         html << "<BR>" << (*vi)->name.c_str() ;
      }
      html << "<BR>";
   }


   html << "<BR>Exclude these strings:" << endl;
   if( rc->exclude_strings.empty() ) {
      html << "<BR> No strings are specified.<BR>" << endl;
   } else {
      for( vi=rc->exclude_strings.begin(); vi != rc->exclude_strings.end(); vi++ ) {
         html << "<BR>" << (*vi)->name.c_str() ;
      }
      html << "<BR>";
   }

   html << "<hr>";
   html << "<BR>Read source files from:" << endl;
   if( rc->read_src_files_from.empty() ) {
      html << "<BR> No directory paths are specified.<BR>" << endl;
   } else {
      for( vi=rc->read_src_files_from.begin(); 
           vi != rc->read_src_files_from.end(); vi++ ) {
         html << "<BR>" << (*vi)->path.c_str() ;
      }
      html << "<BR>";
   }

   html << "<BR>Write html files to" << endl;
   if( rc->write_html_files_to ) {
      html << "<BR>" << rc->write_html_files_to->path.c_str() << "<BR>";
   } else {
      html << "<BR> No directory paths are specified.<BR>" << endl; 
   }

   html << "<BR>Find doc files in" << endl;
   if( rc->find_doc_files_in ) {
      html << "<BR>" << rc->find_doc_files_in->path.c_str() << "<BR>";
   } else {
      html << "<BR> No directory paths are specified.<BR>" << endl; 
   }


   html << "</td> " << endl;
   html << "<td>" << endl;

/*********************************************************************************/



html << "<PRE>" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">1:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_comments<< "><FONT COLOR=" << rc->color_comments<< ">";
html << "/*************************************************************************/" 
     << "</FONT></" << rc->style_comments<< ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">2:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_comments<< "><FONT COLOR=" << rc->color_comments<< ">";
html << "/* DUCTAPE Version 1.0                                                   */" 
     << "</FONT></" << rc->style_comments<< ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">3:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_comments<< "><FONT COLOR=" << rc->color_comments<< ">";
html << "/* Copyright (C) 2001                                                    */" 
     << "</FONT></" << rc->style_comments<< ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">4:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_comments<< "><FONT COLOR=" << rc->color_comments<< ">";
html << "/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */" 
     << "</FONT></" << rc->style_comments<< ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">5:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_comments<< "><FONT COLOR=" << rc->color_comments<< ">";
html << "/* University of Oregon, Department of Computer and Information Science  */" 
     << "</FONT></" << rc->style_comments<< ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">6:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_comments<< "><FONT COLOR=" << rc->color_comments<< ">";
html << "/*************************************************************************/" 
     << "</FONT></" << rc->style_comments<< ">" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">7:</FONT></" << rc->style_linenumbers << ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">8:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_preprocessor<< "><FONT COLOR=" << rc->color_preprocessor 
     << ">#ifndef __PDB_ROUTINE_H__" << "</FONT></"
         << rc->style_preprocessor << ">" << endl;



html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">9:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_preprocessor << "><FONT COLOR=" << rc->color_preprocessor 
     << ">#define __PDB_ROUTINE_H__" << "</FONT></" << rc->style_preprocessor << ">" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">10:</FONT></" << rc->style_linenumbers << ">" << endl;



html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">11:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_preprocessor << "><FONT COLOR=" << rc->color_preprocessor 
     << ">#include \"<A HREF=\"pdbTemplateItem.h.html\">pdbTemplateItem.h</A>\"" 
     << "</FONT></" << rc->style_preprocessor << ">" << endl;



html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">12:</FONT></" << rc->style_linenumbers << ">" << endl;
html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">13:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_preprocessor << "><FONT COLOR=" << rc->color_preprocessor 
     << ">#define  THIS    1" << "</FONT></" << rc->style_preprocessor << ">" << endl;
html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">14:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_preprocessor << "><FONT COLOR=" << rc->color_preprocessor 
     << ">#define  THAT    2" << "</FONT></" << rc->style_preprocessor << ">" << endl;
html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">15:</FONT></" << rc->style_linenumbers << ">" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">16:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << ">class</FONT> </" << rc->style_keyword << ">";
html << "<" << rc->style_target << "><FONT COLOR=" << rc->color_target 
     << "><A NAME=\"cl2755\">pdbRoutine</A>";
html << "</FONT></" << rc->style_target << "> : " ;
html << "<" << rc->style_keyword<< "> <FONT COLOR=" << rc->color_keyword
     << ">public</FONT> </" << rc->style_keyword << ">" ;
html << "<A HREF=\"pdbTemplateItem.h.html#cl2761\">pdbTemplateItem</A> {" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">17:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << ">public</FONT></" << rc->style_keyword << ">:" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">18:</FONT></" << rc->style_linenumbers << ">  "  ;
html << "<" << rc->style_target << "><FONT COLOR=" << rc->color_target 
     << "><A NAME=\"ro416\">pdbRoutine</A>";
html << "</FONT></" << rc->style_target << ">(" ;

html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << "> const</FONT></" << rc->style_keyword << "> string&amp; name, " ;
html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << "> int</FONT></" << rc->style_keyword << "> id) : " ;

html << "<A HREF=\"pdbTemplateItem.h.html#ro410\">pdbTemplateItem</A>(name, id)" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">19:</FONT></" << rc->style_linenumbers << ">    {<A HREF=\"pdbTemplateItem.h.html#ro411\">}</A>" << endl;
html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">20:</FONT></" << rc->style_linenumbers << ">"  ;

html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << ">  virtual</FONT></" << rc->style_keyword << "><A NAME=\"ro1098\"> ~pdbRoutine</A>();" ; 


html << "   <" << rc->style_comments << "><FONT COLOR=" << rc->color_comments
     << ">// dtor</FONT></" << rc->style_comments << ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">21:</FONT></" << rc->style_linenumbers << ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">22:</FONT></" << rc->style_linenumbers << ">"  ;

html << "<" << rc->style_keyword << "><FONT COLOR=" << rc->color_keyword
     << ">  bool </FONT></" << rc->style_keyword;
html << "><" << rc->style_target << "><FONT COLOR=" << rc->color_target 
     << "><A NAME=\"ro417\">isVirtual</A></FONT></" << rc->style_target << "() ";
html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << "> const</FONT></" << rc->style_keyword << "> {" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">23:</FONT></" << rc->style_linenumbers << ">"    ;
html << "<" << rc->style_keyword << "><FONT COLOR=" << rc->color_keyword 
     << ">   return</FONT></" << rc->style_keyword << "> v == ";
html << "<" << rc->style_constants << "><FONT COLOR=" << rc->color_constants 
     << ">0 </FONT></" << rc->style_constants << ">|| v == " ;
html << "<" << rc->style_constants << "><FONT COLOR=" << rc->color_constants 
     << ">7 </FONT></" << rc->style_constants << ">|| v == " ;
html << "<" << rc->style_constants << "><FONT COLOR=" << rc->color_constants 
     << ">THAT </FONT></" << rc->style_constants << ">;" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">24:</FONT></" << rc->style_linenumbers << ">  }" << endl;
html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">25:</FONT></" << rc->style_linenumbers << ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">26:</FONT></" << rc->style_linenumbers << ">" ;
html << "<" << rc->style_keyword << "><FONT COLOR=" << rc->color_keyword
     << ">  virtual </FONT></" << rc->style_keyword << ">";
html << "<" << rc->style_keyword << "><FONT COLOR=" << rc->color_keyword
     << ">void </FONT></" << rc->style_keyword;
html << "><" << rc->style_target << "><FONT COLOR=" << rc->color_target 
     << "><A NAME=\"ro1101\">process</A></FONT></" << rc->style_target << ">(";
html << "<A HREF=\"pdb.h.html#cl2091\">PDB</A> *p);" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">27:</FONT></" << rc->style_linenumbers << ">" << endl   ;
html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">28:</FONT></" << rc->style_linenumbers << ">" ;
html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << ">private</FONT></" << rc->style_keyword << ">:" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">29:</FONT></" << rc->style_linenumbers << ">" ;
html << "<" << rc->style_keyword<< "><FONT COLOR=" << rc->color_keyword
     << ">  const </FONT></" << rc->style_keyword << ">" ;
html << "<A HREF=\"pdb.h.html#cl2048\">pdbType</A>     *<";
html << rc->style_target << "><FONT COLOR=" << rc->color_target 
     << "><A NAME=\"cl-1\">sig</A></FONT></" << rc->style_target << ">;" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">30:</FONT></" << rc->style_linenumbers << ">" ;
html << "  <A HREF=\"pdbItem.h.html#ty409\">virt_t</A>             v" << endl;


html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">31:</FONT></" << rc->style_linenumbers << ">" ;
html << "<" << rc->style_keyword << "><FONT COLOR=" << rc->color_keyword
     << ">  bool </FONT></" << rc->style_keyword << ">              crvo;" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">32:</FONT></" << rc->style_linenumbers << ">" ;
html << "<" << rc->style_keyword << "><FONT COLOR=" << rc->color_keyword
     << ">  int </FONT></" << rc->style_keyword << ">               n;" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">33:</FONT></" << rc->style_linenumbers << ">};" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">34:</FONT></" << rc->style_linenumbers << ">" << endl;

html << "<" << rc->style_linenumbers << "><FONT COLOR=" << rc->color_linenumbers 
     << ">34:</FONT></" << rc->style_linenumbers << ">";
html << "<" << rc->style_preprocessor << "><FONT COLOR=" << rc->color_preprocessor 
     << ">#endif" << "</FONT></" << rc->style_preprocessor << ">" << endl;
html << "<HR>\n";
html << "Back to <A HREF=\"index.html\">Index</A>\n";
html << "<BR><A HREF=\"pdbhtmlrc.html\">Key to Colors and Styles</A>\n";




   html << "</td> </tr>" << endl;

   html << "</table>" << endl;
   html << "<HR align=center>\n";

   html << "</BODY>\n</HTML>\n";

   html.close();
}


//----------------------------------------------------------------------
struct run_control * readpdbhtmlrc() {

   struct run_control * rc = new run_control();

   rc->find_doc_files_in = 0;
   rc->write_html_files_to = 0;
   rc->add_line_numbers = false;
   rc->just_build_pdbhtmlrc_html = false;
   rc->add_backtoindex_footer = true;
   rc->list_methods_in_classes = true;
   rc->add_doc_files = false;
   rc->exit_on_missing_src_file = false;
   rc->print_error_messages = false;
   rc->print_warning_messages = false;


   rc->slash_replacement = '_';
   rc->dot_replacement = '_';

   strcpy( rc->style_text, "strong" );
   strcpy( rc->style_vlink, "tt" );
   strcpy( rc->style_link, "em" );
   strcpy( rc->style_alink, "tt" );
   strcpy( rc->style_comments, "em" );
   strcpy( rc->style_constants, "tt" );
   strcpy( rc->style_preprocessor, "sup" );
   strcpy( rc->style_linenumbers, "tt" );
   strcpy( rc->style_target, "big" );
   strcpy( rc->style_keyword, "strong" );

   strcpy( rc->color_bgcolor, "#ffffff" );
   strcpy( rc->color_text, "#111111" );
   strcpy( rc->color_link, "#000ff0" );
   strcpy( rc->color_alink, "#3300ff" );
   strcpy( rc->color_vlink, "#3388dd" );
   strcpy( rc->color_comments, "#9911bb" );
   strcpy( rc->color_constants, "#ff3f00" );
   strcpy( rc->color_preprocessor, "#408000" );
   strcpy( rc->color_linenumbers, "#904000" );
   strcpy( rc->color_target, "#c00000" );
   strcpy( rc->color_keyword, "#881088" );




   ifstream pdbhtmlrc("pdbhtmlrc");

   if (!pdbhtmlrc) {
      cerr << "Error: Cannot open 'pdbhtmlrc'" << endl;
      return rc;
   }

   
   static char inbuf[INBUF_SIZE];
   string rcline("");
   string parm("");


   while ( pdbhtmlrc.getline(inbuf, INBUF_SIZE) ) {


      const string ignore_letters(" \t");
      const string first_letters("tf");

      string::size_type parm_location;

      rcline = inbuf;


         // Colors
         if( 0 == rcline.find( "color_bgcolor" ) ) {
            strcpy( rc->color_bgcolor, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_text" ) ) {
            strcpy( rc->color_text, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_vlink" ) ) {
            strcpy( rc->color_vlink, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_link" ) ) {
            strcpy( rc->color_link, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_alink" ) ) {
            strcpy( rc->color_alink, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_comments" ) ) {
            strcpy( rc->color_comments, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_constants" ) ) {
            strcpy( rc->color_constants, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_preprocessor" ) ) {
            strcpy( rc->color_preprocessor, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_linenumbers" ) ) {
            strcpy( rc->color_linenumbers, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         } else if ( 0 == rcline.find( "color_keyword" ) ) {
            strcpy( rc->color_keyword, 
               rcline.substr(rcline.find('#'), COLOR_SIZE ).c_str() );

         // Styles
         } else if ( 0 == rcline.find( "style_text" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_text") );
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_text, rcline.substr(parm_location).c_str() );
         
         } else if ( 0 == rcline.find( "style_vlink" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_vlink") );
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_vlink, rcline.substr(parm_location).c_str() );
         
         } else if ( 0 == rcline.find( "style_link" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_link") );
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_link, rcline.substr(parm_location).c_str() );
         
         } else if ( 0 == rcline.find( "style_alink" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_alink") );
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_alink, rcline.substr(parm_location).c_str() );
         
         } else if ( 0 == rcline.find( "style_comments" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_comments") );
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_comments, rcline.substr(parm_location).c_str() );
         
         } else if ( 0 == rcline.find( "style_constants" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_constants") );
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_constants, rcline.substr(parm_location).c_str() );
         
         } else if ( 0 == rcline.find( "style_preprocessor" ) ) {
            parm_location=rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_preprocessor" ));
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_preprocessor, rcline.substr(parm_location).c_str() );

         } else if ( 0 == rcline.find( "style_linenumbers" ) ) {
            parm_location=rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_linenumbers" ));
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_linenumbers, rcline.substr(parm_location).c_str() );

         } else if ( 0 == rcline.find( "style_target" ) ) {
            parm_location=rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_target" ));
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_target, rcline.substr(parm_location).c_str() );

         } else if ( 0 == rcline.find( "style_keyword" ) ) {
            parm_location=rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("style_keyword" ));
            if( parm_location == string::npos ) continue;
            strcpy( rc->style_keyword, rcline.substr(parm_location).c_str() );


         // Generics
         } else if ( 0 == rcline.find( "add_line_numbers" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                                 sizeof("add_line_numbers") );
            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->add_line_numbers = true;
            } else {
               rc->add_line_numbers = false;
            }
         
         } else if ( 0 == rcline.find( "list_methods_in_classes" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                                 sizeof("list_methods_in_classes") );

            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->list_methods_in_classes = true;
            } else {
               rc->list_methods_in_classes = false;
            }

         } else if ( 0 == rcline.find( "add_backtoindex_footer" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                                 sizeof("add_backtoindex_footer") );

            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->add_backtoindex_footer = true;
            } else {
               rc->add_backtoindex_footer = false;
            }

         } else if ( 0 == rcline.find( "just_build_pdbhtmlrc_html" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                        sizeof("just_build_pdbhtmlrc_html") );

            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->just_build_pdbhtmlrc_html = true;
            } else {
               rc->just_build_pdbhtmlrc_html = false;
            }

         } else if ( 0 == rcline.find( "exit_on_missing_src_file" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                        sizeof("exit_on_missing_src_file") );

            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->exit_on_missing_src_file = true;
            } else {
               rc->exit_on_missing_src_file = false;
            }

         } else if ( 0 == rcline.find( "print_error_messages" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                        sizeof("print_error_messages") );

            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->print_error_messages = true;
            } else {
               rc->print_error_messages = false;
            }

         } else if ( 0 == rcline.find( "print_warning_messages" ) ) {
            parm_location = rcline.find_first_of( first_letters, 
                                        sizeof("print_warning_messages") );

            if( parm_location == string::npos ) continue;

            if( rcline.substr(parm_location,4) == "true" ) {
               rc->print_warning_messages = true;
            } else {
               rc->print_warning_messages = false;
            }

         } else if ( 0 == rcline.find( "slash_replacement" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                                 sizeof("slash_replacement") );

            if( parm_location == string::npos ) continue;

            rc->slash_replacement = rcline.at(parm_location);

         } else if ( 0 == rcline.find( "dot_replacement" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                               sizeof("dot_replacement") );

            if( parm_location == string::npos ) continue;

            rc->dot_replacement = rcline.at(parm_location);

         } else if ( 0 == rcline.find( "exclude_this_file" ) ) {
            exclude_this( "file", rc->exclude_files, rcline );

         } else if ( 0 == rcline.find( "exclude_this_string" ) ) {
            exclude_this( "string", rc->exclude_strings, rcline );

         } else if( 0 == rcline.find( "read_src_files_from" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                               sizeof("read_src_files_from") );
            string path = rcline.substr(parm_location);
            rc->read_src_files_from.push_back(new filenamedata(path));

         } else if( 0 == rcline.find( "write_html_files_to" ) ) {
            parm_location = rcline.find_first_not_of( ignore_letters, 
                                           sizeof("write_html_files_to") );
            string path = rcline.substr(parm_location);
            rc->write_html_files_to = new filenamedata(path);

         } else if( 0 == rcline.find( "find_doc_files_in" ) ) {


            parm_location = rcline.find_first_not_of( ignore_letters, 
                                           sizeof("find_doc_files_in") );
            string path = rcline.substr(parm_location);
            rc->find_doc_files_in = new filenamedata(path);


         }

         rcline = "";
   }   


   pdbhtmlrc.close();


   build_pdbhtmlrc_html( rc ) ;

   return rc;

}


//----------------------------------------------------------------------
static bool isExcludedFile( struct run_control * rc, filenamedata * thisfile ) {

   if( (filenamedata*) 0 == thisfile ) { return 1; }

   vector<filenamedata*>::iterator vi;

   vector<filenamedata*>& excludefile  = rc->exclude_files;
   for( vi = excludefile.begin(); vi != excludefile.end(); vi++ ) {
      if( string::npos != thisfile->original.find((*vi)->name.c_str()) ) {

         return 1;
      }
   }

   vector<filenamedata*>& excludestring  = rc->exclude_strings;
   for( vi = excludestring.begin(); vi != excludestring.end(); vi++ ) {
      if( string::npos != thisfile->original.find((*vi)->name.c_str()) ) {

         return 1;
      }
   }

   return 0;
}


//----------------------------------------------------------------------
const char *accessToName(pdbItem::access_t v) {
  switch (v) {
  case pdbItem::AC_PUB : return " public ";
  case pdbItem::AC_PRIV: return " private ";
  case pdbItem::AC_PROT: return " protected ";
  case pdbItem::AC_NA  :
  default     : return " NA ";
  }
}

//----------------------------------------------------------------------
filenamedata * getFileNameData( string fname, vector<filenamedata*> usefiles ) {
   vector<filenamedata*>::iterator vi;

   for( vi = usefiles.begin(); vi != usefiles.end(); vi++ ) {

      if ( (*vi)->name == fname ) {

         return( *vi );
      }
   }

   return( (filenamedata *) 0 );
}


//----------------------------------------------------------------------
string getSourceLine( struct filenamedata * names, int line ) {

   const char * fileName = names->path.c_str();

   ifstream grab;
   grab.open(fileName);

   if (!grab) {
      cerr << "Error: Cannot open !!! for output." << endl;
      return "Yikes! I can't open this file! Really Weird!" ;
   }
   
   int lineinfile = 0;
   char c;
   string returnMe;

   --line;

   while( lineinfile != line ) {
      grab.get(c);
      if( c == '\n' ) ++lineinfile;
   }

   char lastc = '?';

   do {
      grab.get(c);
      if( c == '\n' ) continue;
      if( ( c == ' ' || c == '\t' ) && ( lastc == ' ' || lastc == '\t') ) continue;
      lastc = c;
      returnMe += c;
   } while( ( c != ';') && (c != '{' ) );

   
   return returnMe;
}

//----------------------------------------------------------------------
static void generateIndex( PDB& pdb, ostream& ostr, const char *pdbfile,
                           struct run_control * rc, 
                           vector<filenamedata*> usefiles,
                           const map<const char *, const pdbItem *, ltstr>& isItem ) {
   bool printed;
   bool foundit;

   cout << "Generating index.html ..." << endl;

   ostr << "<HTML>\n<HEAD>\n<TITLE>" << pdbfile << " Index</TITLE>\n</HEAD>\n"
          << "<BODY BGCOLOR=" << rc->color_bgcolor
          << " TEXT=" << rc->color_text
          << " LINK=" << rc->color_link
          << " ALINK=" << rc->color_alink
          << " VLINK=" << rc->color_vlink
          << ">\n<H1 ALIGN=\"CENTER\">Index of " << pdbfile << "</H1>\n"
          << "<center><A HREF=\"pdbhtmlrc.html\">Key to Colors and Styles</A>"
          << "</center><HR>\n";

//--------------------------------------------
   ostr << "<H2>Files:</H2>\n<UL>\n";


   sort(usefiles.begin(), usefiles.end(), filenamedataCmp );
   vector<filenamedata*>::iterator it;
   for( it=usefiles.begin(); it!=usefiles.end(); ++it) {
   
      if ( isExcludedFile( rc, *it) ) continue;
      ostr << "<LI><A HREF=\"";

      if( (*it)->dupname ) {
         ostr << ((*it)->htmlname + ".html").c_str();
      } else {
         ostr << ((*it)->name + ".html").c_str();
      }

      ostr << "\">" << (*it)->original << "</A>\n";
   }
   ostr << "</UL>\n";


//--------------------------------------------
   if( rc->list_methods_in_classes == 1 ) {
      ostr << "<H2>Classes and Methods:</H2>\n<UL>\n";
   } else {
      ostr << "<H2>Classes:</H2>\n<UL>\n";
   }

   PDB::classvec classes_so_far;;
   PDB::classvec classes = pdb.getClassVec();
   sort(classes.begin(), classes.end(), itemCmp);
   for (PDB::classvec::const_iterator cit=classes.begin();
          cit!=classes.end(); ++cit) {

      struct filenamedata * names;
      if ( isExcludedFile( rc, names = getFileNameData( 
                         (*cit)->location().file()->name(), usefiles )) ) {
         continue;
      }

      if ( isItem.find( (*cit)->name().c_str() ) == isItem.end() )  {
         continue;
      }


      char fname[128];
      strcpy( fname, names->htmlname.c_str() );

      if(   // if it's a union...
          ( 0 == strncmp( "union", pdbClass::toName((*cit)->kind()), 5 ) 
       &&   // ...and it has no name, i.e. it's anonymous...
          ( 0 != strstr( (*cit)->fullName().c_str(), "unnamed" ) ) 
        ) ) {  

         continue;  // don't list it
      }

      foundit = false;
      if( classes_so_far.size() > 0 ) {
         for( PDB::classvec::const_iterator uit=classes_so_far.begin();
                uit!=classes_so_far.end(); ++uit) {
            if( ! (*cit)->name().compare( (*uit)->name() ) ) {
               foundit = true;
               break;
            }
         }
      }

      if( ! foundit ) {
         classes_so_far.push_back( *cit );

         ostr << "<LI><A HREF=\"";
         if( names->dupname ) {
            ostr << names->htmlname;
         } else {
            ostr << names->name;
         }
         ostr << ".html#gr" << (*cit)->id() << "\">";
         outStr(ostr, (*cit)->fullName());
         ostr << "</A>\n";

         if( rc->list_methods_in_classes == 1 ) {

            pdbClass::methodvec fmem = (*cit)->methods();
            if ( fmem.size() ) {

               ostr << "<UL>\n";
               for (pdbClass::methodvec::iterator mit = fmem.begin();
                      mit!=fmem.end(); ++mit) {
                  const pdbCRoutine* mfunc = (*mit)->func();

                  if ( mfunc->isTemplate() ) { 

                     PDB::templatevec templates = pdb.getTemplateVec();
                     sort(templates.begin(), templates.end(), itemCmp);
                     for (PDB::templatevec::const_iterator tit=templates.begin();
                            tit!=templates.end(); ++tit) {

                        names = getFileNameData( 
                          (*tit)->location().file()->name(), usefiles );

                        if( names != (filenamedata*) 0 ) {

                           if( ( (*tit)->location().file()->name() == names->name ) 
                            && ( mfunc->name() == (*tit)->name() ) ) {

                             ostr <<"<LI>"<< accessToName(mfunc->access());

                              ostr << "<A HREF=\"" ;
                              if( names->dupname ) {

                                 ostr << names->htmlname;
                              } else {

                                 ostr << names->name;
                              }
                              ostr << ".html#te" << (*tit)->id() << "\">";
                              outStr(ostr, mfunc->fullName());
                              ostr << "</A>\n";
                              break;
                           }
                        }

                     }

                  } else if ( ! mfunc->isCompilerGenerated() ) {
                  // don't list compiler generated functions like ctor, dtor

                     ostr << "<LI>" << accessToName(mfunc->access()) ;

                     names = getFileNameData( 
                          (*mit)->func()->location().file()->name(), usefiles );

                     if( names != (filenamedata*) 0 ) {
                        ostr << "<A HREF=\"";
                        if( names->dupname ) {
                           ostr << names->htmlname;
                        } else {
                           ostr << names->name;
                        }
                        ostr  << ".html#ro" << mfunc->id() << "\">";
                        outStr(ostr, mfunc->fullName());
                        ostr << "</A>\n";
                     } else {
                        outStr(ostr, mfunc->fullName());
                     }
                  }
               } // end of for (pdbClass::methodvec::iterator mit
               ostr << "</UL>\n";

            }

         } // end of if( rc->list_methods_in_classes == 1 


      }  // end of if( ! foundit )   

   } 
   ostr << "</UL>\n";




//--------------------------------------------
   printed = false;

   PDB::templatevec templates = pdb.getTemplateVec();
   sort(templates.begin(), templates.end(), itemCmp);
   for (PDB::templatevec::const_iterator tit=templates.begin();
          tit!=templates.end(); ++tit) {
      struct filenamedata * names;


      if ( isExcludedFile( rc, names = getFileNameData( 
                                (*tit)->location().file()->name(), usefiles)) ) {
         continue;
      }

      if( !printed ) {
         ostr << "<H2>Templates:</H2>\n<UL>\n";
         printed = true;
      }

      ostr << "<LI><A HREF=\"" ;
      if( names->dupname ) {
         ostr << names->htmlname;
      } else {
         ostr << names->name;
      }

      ostr << ".html#te" << (*tit)->id() << "\">";
      outStr(ostr, (*tit)->location().file()->name());

      string sourceLine = getSourceLine( names,(*tit)->location().line() );

      ostr << ":&nbsp;&nbsp;&nbsp;";
      outStr(ostr, sourceLine );

      ostr << "</A>\n";

   }

   if( printed ) {
      ostr << "</UL>\n";
   }



//--------------------------------------------
   printed = false;

   PDB::croutinevec routines = pdb.getCRoutineVec();
   sort(routines.begin(), routines.end(), itemCmp);

   for (PDB::croutinevec::const_iterator rit=routines.begin();
          rit!=routines.end(); ++rit) {

      if ( ! (*rit)->parentGroup() ) {

         if ( ! (*rit)->isCompilerGenerated() ) {
            struct filenamedata * names;

            if ( isExcludedFile( rc, names = getFileNameData( 
                                (*rit)->location().file()->name(), usefiles )) ) {
               continue;
            }

            if( !printed ) {
               ostr << "<H2>Functions:</H2>\n<UL>\n";
               printed = true;
            }

            ostr << "<LI><A HREF=\"";
            if( names->dupname ) {
               ostr << names->htmlname;
            } else {
               ostr << names->name;
            }
            ostr << ".html#ro" << (*rit)->id() << "\">";

            outStr(ostr, (*rit)->location().file()->name());
            ostr << ":&nbsp;&nbsp;&nbsp;";
            outStr(ostr, (*rit)->fullName());

            ostr << "</A>\n";

         }
      }
   }

   ostr << "</UL>\n";

//--------------------------------------------

   printed = false;

   PDB::namespacevec namespaces = pdb.getNamespaceVec();

   if( ! pdb.getNamespaceVec().empty() ) {

      sort(namespaces.begin(), namespaces.end(), itemCmp);

      for (PDB::namespacevec::const_iterator nit=namespaces.begin();
             nit!=namespaces.end(); ++nit) {
         const pdbLoc nullpdbLoc;
         struct filenamedata * names;

         if( (nullpdbLoc == (*nit)->location()) ) { continue; }

         if ( isExcludedFile( rc, names = getFileNameData( 
                              (*nit)->location().file()->name(), usefiles )) ) {
            continue;
         }

         if( !printed ) {
            ostr << "<H2>Namespaces:</H2>\n<UL>\n";
            printed = true;
         }

         ostr << "<LI><A HREF=\"" ;
         if( names->dupname ) {
            ostr << names->htmlname;
         } else {
            ostr << names->name;
         }
         ostr << ".html#na" << (*nit)->id() << "\">";
         outStr(ostr, (*nit)->fullName());
         ostr << "</A>\n";
      }

      if( printed ) {
         ostr << "</UL>\n";
      }
   } 

   ostr << "<HR>\n</BODY>\n</HTML>\n";
}

//----------------------------------------------------------------------
struct itemRef {
   itemRef(const pdbItem *i, bool isT) : item(i), isTarget(isT) {
      line = i->location().line();
      col   = i->location().col();
   }
   itemRef(const pdbItem *i, bool isT, int l, int c)
             : item(i), isTarget(isT), line(l), col(c) {}
   const pdbItem *item;
   bool       isTarget;
   int         line;
   int         col;
};

//----------------------------------------------------------------------
static bool locCmp(const itemRef* r1, const itemRef* r2) {
   if ( r1->line == r2->line )
      return r1->col < r2->col;
   else
      return r1->line < r2->line;
}

//----------------------------------------------------------------------
void getReferences(vector<itemRef *>& itemvec, PDB& pdb, string file) {

   const pdbLoc nullpdbLoc;

   for( PDB::classvec::const_iterator it=pdb.getClassVec().begin();
        it!=pdb.getClassVec().end(); ++it) {

      if( nullpdbLoc == (*it)->location()) { continue; }

      if ( (*it)->location().file()->name() == file ) {
         itemvec.push_back(new itemRef(*it, true));

         pdbClass::fieldvec::const_iterator dit;
         for ( dit=(*it)->dataMembers().begin();
               dit!=(*it)->dataMembers().end(); ++dit) {
            if( (*dit)->location().file() 
             && (*dit)->kind()!=pdbItem::M_TYPE
             && !((*dit)->type()->isGroup()) )
               itemvec.push_back(new itemRef(*dit, true));
         }

      }
   }

   PDB::croutinevec routines = pdb.getCRoutineVec();
   for (PDB::croutinevec::const_iterator rit=routines.begin();
          rit!=routines.end(); ++rit) {

      if( nullpdbLoc == (*rit)->location()) { continue; }

      if ( (*rit)->location().file()->name() == file 
         && !(*rit)->isCompilerGenerated()) 
         {
         itemvec.push_back(new itemRef(*rit, true));
         for (pdbRoutine::callvec::iterator cit=(*rit)->callees().begin();
                cit!=(*rit)->callees().end(); ++cit) {
            if ( (*cit)->line() > 0 )
               itemvec.push_back( new itemRef((*cit)->call(), false
                                   , (*cit)->line(), (*cit)->col()));
         }
      }
   }

   PDB::templatevec templates = pdb.getTemplateVec();
   for (PDB::templatevec::const_iterator tit=templates.begin();
          tit!=templates.end(); ++tit) {

      if( nullpdbLoc == (*tit)->location()) { continue; }

      if ( (*tit)->location().file()->name() == file ) {
         itemvec.push_back(new itemRef(*tit, true));
      }
   }

   PDB::typevec types = pdb.getTypeVec();
   for (PDB::typevec::const_iterator yit=types.begin();
          yit!=types.end(); ++yit) {

      if( nullpdbLoc == (*yit)->location()) { continue; }

      if ( (*yit)->location().file()->name() == file 
       && (((*yit)->kind()==pdbItem::TY_TREF && (*yit)->qualifiers().size()==0)
       || (*yit)->kind()==pdbItem::TY_ENUM) ) {
         itemvec.push_back(new itemRef(*yit, true));
      }
   }

   PDB::namespacevec namespaces = pdb.getNamespaceVec();
   for (PDB::namespacevec::const_iterator nit=namespaces.begin();
          nit!=namespaces.end(); ++nit) {
      if( nullpdbLoc == (*nit)->location()) { continue; }
      if ( (*nit)->location().file()->name() == file ) {
         itemvec.push_back(new itemRef(*nit, true));
      }
   }
   sort(itemvec.begin(), itemvec.end(), locCmp);
}

//----------------------------------------------------------------------
static const pdbItem* setNextItem(vector<itemRef *>& itemvec,
                                        int& curr, int& nextLine, int& nextCol) {
   ++curr;
   if ( curr >= itemvec.size() ) {
      nextLine = -1;
      nextCol   = -1;
      return 0;
   } else {
      nextLine = itemvec[curr]->line;
      nextCol   = itemvec[curr]->col;
      return itemvec[curr]->item;
   }
}

//----------------------------------------------------------------------
static string getQualifiedName(const pdbItem* item) {
   string fn = item->name();
   const pdbGroup *gptr = item->parentGroup();
   while ( gptr ) {
      fn = gptr->name() + "::" + fn;
      gptr = gptr->parentGroup();
   }
   const pdbNamespace *nptr = item->parentNSpace();
   while ( nptr ) {
      fn = nptr->name() + "::" + fn;
      nptr = nptr->parentNSpace();
   }
   return fn;
}

//----------------------------------------------------------------------
static int nextMultiple = -1;


//----------------------------------------------------------------------
static bool processFile(PDB& pdb, filenamedata* f,
            const set<const char *, ltstr>& isKeyword, 
            const map<const char *, const pdbItem *, ltstr>& isItem,
            struct run_control * rc, vector<filenamedata*> usefiles ) {

   string file(f->name);
   if( f->dupname ) {
      file = f->htmlname;
   } 

   static char inbuf[INBUF_SIZE];
   int line = 1;
   int pos = 0;
   bool inComment = false;
   bool inPreStmt = false;
   bool inPreFont = false;


   // do not process excluded files
   if ( isExcludedFile( rc, f) ) return true;

   // open source file
   ifstream istr(f->original.c_str());
   if (!istr) {
      cerr << "Error: Cannot open '" << file << "'" << endl;
      return false;
   }
   cout << endl << "Processing " << file ;

   // open outfile for HTML version of source file
   string writehere;
   if( rc->write_html_files_to ) {
      writehere = rc->write_html_files_to->path + "/" + file + ".html";
   } else {
      writehere = file + ".html";
   }
   ofstream ostr(writehere.c_str());

   if (!ostr) {
      cerr << "Error: Cannot open '" << writehere << "'" << endl;
      return false;
   }

   // initialize reference vector
   vector<itemRef *> itemvec;
   getReferences(itemvec, pdb, f->name );


   int curr = -1;
   int nextLine = -1;
   int nextCol   = -1;
   const pdbItem* nextItem = 0;
   nextItem = setNextItem(itemvec, curr, nextLine, nextCol);

   // write header
   ostr << "<HTML>\n<HEAD>\n<TITLE>" << f->name
          << "</TITLE>\n</HEAD>\n"
          << "<BODY BGCOLOR=" << rc->color_bgcolor
          << " TEXT=" << rc->color_text
          << " LINK=" << rc->color_link
          << " ALINK=" << rc->color_alink
          << " VLINK=" << rc->color_vlink
          << "> <H1>File: " << f->name << "</H1>\n<HR>\n<PRE>\n";

   // parse file
   while ( istr.getline(inbuf, INBUF_SIZE) ) {

      linenumbers( ostr, rc, line, inComment, inPreStmt, &inPreFont );

      if ( !inComment && inbuf[0] == '#' ) {   /*??? ERROR # first non-white*/
         // preprocessor statement

         bool isInc = false;
         bool isIncRef = false;
         bool inPreStComment = false;

         if ( strlen( rc->style_preprocessor ) > 0 ) {
            ostr << "<" << rc->style_preprocessor << ">";
         }
         ostr << "<FONT COLOR=" << rc->color_preprocessor << ">";
         inPreFont = true;

         while ( inbuf[pos] ) {

            if ( inbuf[pos] == 'i' && strncmp(inbuf+pos, "include", 7)==0 ) {
               isInc = true;
               ostr << 'i';


            } else if ( isIncRef && (inbuf[pos]=='\"' || inbuf[pos]=='>') ) {
               ostr << "</A>" << inbuf[pos];


            } else if ( isInc && (inbuf[pos]=='\"' || inbuf[pos]=='<') ) {

               out(ostr, inbuf[pos]);
               int len = 0;
               while ( inbuf[pos+len+1]!='\"' && inbuf[pos+len+1]!='>' ) ++len;


               vector<filenamedata*>::iterator it;
               for( it=usefiles.begin(); it!=usefiles.end(); ++it) {

                  char fname[128];
                  
                  if( 0 == strchr( inbuf+pos, '/' ) ) {
                     strcpy( fname, (*it)->name.c_str() );
                  } else {
                     strcpy( fname, (*it)->includeform.c_str() );
                  } 

                  if ( strncmp(inbuf+pos+1, fname, len)==0 ) {

                     ostr << "<A HREF=\"";
                     if( (*it)->dupname ) {
                        ostr << (*it)->htmlname;
                     } else {
                        ostr << (*it)->name;
                     }
                     ostr << ".html\">";
                     isIncRef = true;
                     break;
                  }

               }
            } else {

               if( inbuf[pos] == '/' 
                && ( inbuf[pos+1] == '*' || inbuf[pos+1] == '/' ) ) {

                  ostr << "</FONT>";
                  inPreFont = false;
                  if ( strlen( rc->style_preprocessor ) > 0 ) {
                     ostr << "</" << rc->style_preprocessor <<  ">";
                  }
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "<" <<  rc->style_comments <<  ">";
                  }
                  ostr << "<FONT COLOR=" << rc->color_comments <<  ">";
                  inPreStComment = true;
               }

               if( inPreStComment 
                  && ( inbuf[pos] == '*' && inbuf[pos+1] == '/' ) ) {

                  out(ostr, inbuf[pos++]);
                  out(ostr, inbuf[pos++]);
                  ostr << "</FONT>";
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "</" <<  rc->style_comments <<  ">";
                  }
                  inPreStComment = false;

                  if( inbuf[pos] != '\0' ) {
                     if ( strlen( rc->style_preprocessor ) > 0 ) {
                        ostr << "<" <<  rc->style_preprocessor <<  ">";
                     }
                     ostr << "<FONT COLOR=" << rc->color_preprocessor <<  ">";
                     inPreFont = true;

                  }
               } else if( inPreStComment && inbuf[pos+1] == '\0'  ) {

                  out(ostr, inbuf[pos]);
                  ostr << "</FONT>";
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "</" <<  rc->style_comments <<  ">";
                  }
                  inComment = false;
                  inPreStComment = false;

               } else {

                  out(ostr, inbuf[pos]);
               }
            } // end of else

            ++pos;

         } // end of while( inbuf[pos] ) {


         if( inPreFont ) {
            ostr << "</FONT>";
            inPreFont = false;
            if ( strlen( rc->style_preprocessor ) > 0 ) {
               ostr << "</" <<  rc->style_preprocessor <<  ">";
            }
         }

         if ( inbuf[pos-1] == '\\' ) {
            inPreStmt = true;
         } else {
            inPreStmt = false;
         }

      } else if ( inPreStmt ) {

         // preprocessor statement continuation
         while ( inbuf[pos] ) out(ostr, inbuf[pos++]);   

         if ( inbuf[pos-1] == '\\' ) {
            inPreStmt = true;
         } else {
            inPreStmt = false;
         }
         if( inPreFont ) {
            ostr << "</FONT>";
            inPreFont = false;
            if ( strlen( rc->style_preprocessor ) > 0 ) {
               ostr << "</" <<  rc->style_preprocessor <<  ">";
            }
         }
      } else {

         while ( inbuf[pos] ) {
            if ( inComment ) {

               /* look for comment end */
               if ( inbuf[pos] == '*' && inbuf[pos+1] == '/' ) {
                  ostr << "*/</FONT>";
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "</" <<  rc->style_comments << ">";
                  }
                  inComment = false;
                  pos += 2;

               } else {
                  out(ostr, inbuf[pos++]);
               }

            } else if ( inbuf[pos] == '/' ) {
               pos++;
               if ( inbuf[pos] == '/' ) {
                  // c++ comments
                  pos++;
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "<" <<  rc->style_comments << ">";
                  }
                  ostr << "<FONT COLOR=" << rc->color_comments <<   ">//";

                  while ( inbuf[pos] ) { out(ostr, inbuf[pos++]); }
                  ostr << "</FONT>";
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "</" <<  rc->style_comments << ">";
                  }
               } else if ( inbuf[pos] == '*' ) {
                  /* c comment start */
                  pos++;
                  if ( strlen( rc->style_comments ) > 0 ) {
                     ostr << "<" <<  rc->style_comments << ">";
                  }
                  ostr << "<FONT COLOR=" << rc->color_comments <<   ">/*";
                  inComment = true;
               } else {
                  ostr << '/';
               }

            } else if ( inbuf[pos] == '\"' ) {
               // character string constant

               if( strlen( rc->style_constants ) > 0 ) {
                  ostr << "<" <<  rc->style_constants << ">";
               }
               ostr << "<FONT COLOR=" << rc->color_constants <<  ">\"";
               do {
                  pos++;
                  if ( inbuf[pos] == '\\' ) {
                     ostr << '\\';
                     pos++;
                     if ( inbuf[pos] == '\"' ) {
                        ostr << '\"';
                        pos++;
                     }
                  }
                  out(ostr, inbuf[pos]);
               }
               while ( inbuf[pos] != '\"' );
               ostr << "</FONT>";
               if( strlen( rc->style_constants ) > 0 ) {
                  ostr << "</" <<  rc->style_constants <<  ">";
               }
               pos++;
            } else if ( inbuf[pos] == '\'' ) {
               // character constant

               if( strlen( rc->style_constants ) > 0 ) {
                  ostr << "<" <<  rc->style_constants <<  ">";
               }
               ostr << "<FONT COLOR=" << rc->color_constants <<   ">\'";
               do {
                  pos++;
                  if ( inbuf[pos] == '\\' ) {
                     ostr << '\\';
                     pos++;
                     if ( inbuf[pos] == '\'' ) {
                        ostr << '\'';
                        pos++;
                     }
                  }
                  out(ostr, inbuf[pos]);
               }
               while ( inbuf[pos] != '\'' );
               ostr << "</FONT>";
               if( strlen( rc->style_constants ) > 0 ) {
                  ostr << "</" <<  rc->style_constants <<  ">";
               }
               pos++;
            } else {

               if ( nextLine == line && nextCol == (pos+1) ) {
                  // hyperlink item

                  // ignore multiple entries of the same thing
                  while ( curr < (itemvec.size()-1) &&
                              nextItem == itemvec[curr+1]->item &&
                              nextLine == itemvec[curr+1]->line &&
                              nextCol   == itemvec[curr+1]->col ) {

                     nextItem = setNextItem(itemvec, curr, nextLine, nextCol);
                  }

                  // multiple target or link?
                  bool isMult = false;
                  if ( curr < (itemvec.size()-1) &&

                         nextItem != itemvec[curr+1]->item &&
                         nextLine == itemvec[curr+1]->line &&
                         nextCol   == itemvec[curr+1]->col ) {
                     isMult = true;
                  }

                  if ( itemvec[curr]->isTarget ) {

                     int numAnchor = 0;
                     const string& itemName = nextItem->name();
                     const char *qname = getQualifiedName(nextItem).c_str();
                     int len;

                     if( ! ( strncmp(inbuf+pos, 
                             itemName.c_str(), strlen(itemName.c_str()))
                      && strncmp(inbuf+pos, qname, strlen(qname))) ) {

                        if ( strncmp(inbuf+pos, qname, strlen(qname)) == 0) {
                           len = strlen(qname);
                        } else {
                           len = itemName.size();
                        }
    
                        if ( strlen( rc->style_target ) > 0 ) {
                           ostr << "<" <<  rc->style_target <<  ">";
                        }
                        ostr << "<FONT COLOR=" << rc->color_target <<  ">";

                        do {
                           ostr << "<A NAME=\""
                                  << nextItem->desc()[0] << nextItem->desc()[1]
                                  << nextItem->id() << "\">";
                           ++numAnchor;
                           nextItem = setNextItem(itemvec, curr, nextLine, nextCol);
                        } while ( isMult &&
                                       nextItem != itemvec[curr-1]->item &&
                                       nextLine == itemvec[curr-1]->line &&
                                       nextCol   == itemvec[curr-1]->col );

                        for (int i=0; i<len; ++i) {
                           out(ostr, inbuf[pos++]);
                        }
                        while ( numAnchor-- ) ostr << "</A>";
                        ostr << "</FONT>";
                        if ( strlen( rc->style_target ) > 0 ) {
                           ostr << "</" <<  rc->style_target <<  ">";
                        }
                     } else {
                        while( (*(inbuf+pos) != ' ' )
                          &&   ( *(inbuf+pos) != '\n') 
                          &&   ( *(inbuf+pos) != '\0') ) {
                           out(ostr, inbuf[pos++]);
                        }
                     }


                  } else {
                     int len;
                     char multFile[1024];

                     if ( isMult ) {

                        // multiple targets for the same anchor
                        // -> generate auxillary target table file
                        // generate link to auxillary target table file

                        sprintf(multFile, 
                            "%s__mult%d.html", file.c_str(), ++nextMultiple);
                        ostr << "<A HREF=\"" << multFile << "\">";

                     } else {

                        struct filenamedata * names;
                        bool printed_href;

                        printed_href = false;
                        if( nextItem->location().file() && 
                          ! isExcludedFile( rc, names = getFileNameData( 
                           nextItem->location().file()->name(), usefiles )) ) {
   
                           PDB::templatevec templates = pdb.getTemplateVec();
                           sort(templates.begin(), templates.end(), itemCmp);

                           PDB::templatevec::const_iterator tit;
                           for (tit=templates.begin();tit!=templates.end(); ++tit) {

                              if( ( (*tit)->location().file()->name() 
                                                         == names->name ) 
                               && ( (*tit)->name() == nextItem->name() ) ) {

                                 ostr << names->htmlname;
                                 if( names->dupname ) {
                                    ostr << names->htmlname;
                                 } else {
                                    ostr << names->name;
                                 }
                                 ostr << ".html#te" << (*tit)->id() << "\">";
                                 printed_href = true;
                                 break;
                              }
                           }
                           if( !printed_href ) {
                              ostr << "<A HREF=\"" ;
                              if( names->dupname ) {
                                 ostr << names->htmlname;
                              } else {
                                 ostr << names->name;
                              }
                              ostr << ".html#" << nextItem->desc()[0] 
                               << nextItem->desc()[1]
                               << nextItem->id() << "\">";
                           }
                        }
                     }

                     if ( strncmp(nextItem->name().c_str(), "operator", 8) == 0 ) {

                        if( inbuf[pos] != 'o' && inbuf[pos+1] != 'p' ) {
                           const char *opname = nextItem->name().c_str() + 8;

                           while ( *opname == ' ' ) ++opname;

                           if ( *opname == '[' || *opname == '(' )
                              len = 1;
                           else
                              len = strlen(opname);
                        } else {
                           len=10;
                        }

                     } else {

                        len = nextItem->name().size();
                        if ( strncmp(inbuf+pos, nextItem->name().c_str(), len) 
                                != 0) {
                           // name doesn't match: try qualified name

                           const char *qname = getQualifiedName(nextItem).c_str();
                           if ( strncmp(inbuf+pos, qname, strlen(qname)) == 0) {
                              len = strlen(qname);

                           } else {

                              len = 1;
                              while( inbuf[pos+len] != ' ' ) len++;

                              /*??? qualified name with template class*/
                              if( rc->print_warning_messages == 1 ) {
                                 cerr << endl << "WARNING: non-matching name: " 
                                      << nextItem->fullName() << " at " 
                                      << nextLine << " " << nextCol;
                                 cerr << endl << "WARNING: non-matching name: " 
                                      << nextItem->name() << " at " << nextLine 
                                      << " " << nextCol << endl;
                              }
                           }
                        }
                     }

                     for (int i=0; i<len; ++i) {
                        if( isprint( inbuf[pos] ) )  {
                           out(ostr, inbuf[pos++]);
                        } 
                     }

                     ostr << "</A>";

                     if ( isMult ) {
                        char writemultFile[1024];

                        // create auxillary target table file
                        if( rc->write_html_files_to ) {
                           sprintf(writemultFile, "%s/%s",
                                rc->write_html_files_to->path.c_str(), 
                                     multFile );

                        } else   {
                           strcpy( writemultFile, multFile );

                        }
                        ofstream mul(writemultFile);

                        if (!mul) {
                           cerr << "Error: Can't open '" << multFile << "'" << endl;
                           return false;
                        }

                        mul << "<HTML>\n<HEAD>\n<TITLE>"
                              << multFile << "</TITLE>\n</HEAD>\n"
                              << "<BODY BGCOLOR=" << rc->color_bgcolor
                              << " TEXT=" << rc->color_text
                              << " LINK=" << rc->color_link
                              << " ALINK=" << rc->color_alink
                              << " VLINK=" << rc->color_vlink;
                        mul << ">\n<H1>Target Table for " 
                            << nextItem->name() << "</H1>\n";

                        // add targets to auxillary target table file

                        mul << "<UL>\n";

                        do {

                           struct filenamedata * names;
                           if( nextItem->location().file() && 
                             ! isExcludedFile( rc, names = getFileNameData( 
                              nextItem->location().file()->name(), usefiles )) ) {

                              mul << "<LI><A HREF=\"" ;
                              if( names->dupname ) {
                                 mul << names->htmlname;
                              } else {
                                 mul << names->name;
                              }
                              mul << ".html#" 
                                 << nextItem->desc()[0] << nextItem->desc()[1]
                                 << nextItem->id() << "\">" ;

                              if( 0 != nextItem->fullName().size() ) {
                                 outStr(mul, nextItem->fullName());
                              } else {
                                 outStr(mul, nextItem->location().file()->name() );
                                 outStr(mul, ".html" );
                              }

                              mul << "</A>\n" << endl;
                           } else {
                              mul <<  "<LI>";
                              outStr(mul, nextItem->fullName());
                              mul <<  endl;
                           }

                           nextItem = setNextItem(itemvec, curr, nextLine, nextCol);

                        } while ( nextItem != itemvec[curr-1]->item &&
                                       nextLine == itemvec[curr-1]->line &&
                                       nextCol   == itemvec[curr-1]->col );
                        // end of do/while

                        mul << "</UL>\n";
                        mul << "</BODY>\n</HTML>\n";

                     } else {
                        nextItem = setNextItem(itemvec, curr, nextLine, nextCol);
                     }
                  }

               } else if ( isalpha(inbuf[pos]) 
                         || inbuf[pos] == '_' ) {

                  /*???*/
                  if ( nextItem && ((nextLine == line && nextCol < (pos+1)) ||
                                                   (nextLine < line)) ) {
                     if( rc->print_error_messages == 1 ) {
                        cerr << "ERROR: skipping " << nextItem->desc() << "#"
                         << nextItem->id() << " " << nextItem->name() << " at "
                         << nextLine << ":" <<nextCol << " -> " << line << ":"
                         << (pos+1) << endl;
                     }
                     nextItem = setNextItem(itemvec, curr, nextLine, nextCol);
                  }

                  // identifier   /*??? _identifier?*/
                  map<const char *, const pdbItem *, ltstr>::const_iterator it;
                  const pdbItem *item;

                  int startPos = pos;
                  while ( isalnum(inbuf[pos]) || inbuf[pos]=='_' ) pos++;
                  char saveCh = inbuf[pos];
                  inbuf[pos] = '\0';


                  if ( static_cast<set<const char *, ltstr>::const_iterator>(isKeyword.find(inbuf+startPos)) != isKeyword.end() ) {
                     // keyword

                     if ( strlen( rc->style_keyword ) > 0 ) {
                        ostr << "<" <<  rc->style_keyword <<  ">";
                     }
                       
                     ostr << "<FONT COLOR=" << rc->color_keyword <<  ">";
                     ostr << inbuf+startPos ;
                     ostr << "</FONT>";

                     if ( strlen( rc->style_keyword ) > 0 ) {
                        ostr << "</" <<  rc->style_keyword <<  ">";
                     }

                  } else if ( (it = isItem.find(inbuf+startPos)) != isItem.end() 
                                && (item = (*it).second) != 0) {

                     // class, enum, or enum value name
                     struct filenamedata * names;
                     const pdbLoc nullpdbLoc;

                     if( !(nullpdbLoc == item->location()) &&
                       ! isExcludedFile( rc, names = getFileNameData( 
                        item->location().file()->name(), usefiles )) ) {

                        if( names->original == item->location().file()->name()
                         && line == item->location().line()
                         && startPos+1 == item->location().col() ) {

                           ostr << "<A NAME=\"";
                        } else {

                           ostr << "<A HREF=\"" ;
                           if( names->dupname ) {
                              ostr << names->htmlname;
                           } else {
                              ostr << names->name;
                           }
                           ostr << ".html#";

                        }
                        ostr << item->desc()[0] << item->desc()[1]
                             << item->id() << "\">";
                        outStr(ostr, item->name() );
                        ostr << "</A>";

                     } else {

                        outStr(ostr, item->name());
                     }

                  } else {

                     ostr << inbuf+startPos;
                  }
                  inbuf[pos] = saveCh;
               } else if ( isdigit(inbuf[pos]) ||
                           ( ( inbuf[pos] == '-' || inbuf[pos] == '+' ) 
                              &&
                              isdigit(inbuf[pos+1]) )
                          ) {
                  // numerical constant?

                  char *eptr1, *eptr2;
                  strtod(inbuf+pos, &eptr1);       // decimal, octal, float?
                  strtol(inbuf+pos, &eptr2, 16); // hexadecimal?
                  if ( eptr1 != (inbuf+pos) || eptr2 != (inbuf+pos) ) {

                     int len = (eptr1 < eptr2) ? eptr2-(inbuf+pos) 
                                               : eptr1-(inbuf+pos);
                     if( strlen( rc->style_constants ) > 0 ) {
                        ostr << "<" << rc->style_constants <<  ">";
                     }
                     ostr << "<FONT COLOR=" << rc->color_constants <<  ">";
                     for (int i=0; i<len; ++i) out(ostr, inbuf[pos++]);
                     ostr << "</FONT>";
                     if( strlen( rc->style_constants ) > 0 ) {
                        ostr << "</" << rc->style_constants <<  ">";
                     }

                   } else {

                     out(ostr, inbuf[pos++]);
                  }
               } else {


                  /*???*/
                  if ( nextItem && ((nextLine == line && nextCol < (pos+1)) ||
                                                   (nextLine < line)) ) {
                     if( rc->print_error_messages == 1 ) {
                        cerr << "ERROR: skipping " << nextItem->desc() << "#"
                         << nextItem->id() << " " << nextItem->name() << " at "
                         << nextLine << ":" <<nextCol << " -> " << line << ":"
                         << (pos+1) << endl;
                     }
                     nextItem = setNextItem(itemvec, curr, nextLine, nextCol);
                  }
                  out(ostr, inbuf[pos++]);
               }
            }
         }
      }  // end of loop

      ostr << endl;
      ++line;
      pos = 0;
      memset( (void*) inbuf, (int) 0 , (size_t) INBUF_SIZE );


   } // end of line


   // write footer
   ostr << "</PRE>\n";
   if( rc->add_backtoindex_footer == 1 ) {   
      ostr << "<HR>\n";
      ostr << "Back to <A HREF=\"index.html\">Index</A>\n";
      ostr << "<BR><A HREF=\"pdbhtmlrc.html\">Key to Colors and Styles</A>\n";
   }
   ostr << "</BODY>\n</HTML>\n";

   for (curr=0; curr<itemvec.size(); ++curr) delete itemvec[curr];
   return true;
}

//----------------------------------------------------------------------
int main(int argc, char *argv[]) {
   if ( argc != 2 ) {
      cerr << "usage: " << argv[0] << " pdbfile" << endl;
      cerr << "pdbhtml   : Produces \"htmlized\" versions of all source and header files\n"
            "contained in the program database file <pdbfile>. It also\n"
            "produces an HTML index of Classes, Templates, Namespaces,\n"
            "and Functions called \"index.html\". Usage:\n\n"

            "  pdbhtml <pdbfile>\n\n"

            "Only works for C and C++ sources for now.";

      return 1;
   }


   // open and initialize program database
   PDB pdb(argv[1]);
   if ( !pdb ) return 1;

   // can only handle C/C++ for now
   if ( ! (pdb.language() & PDB::LA_C_or_CXX) ) {
     cerr << "Error: can only handle C and C++ files" << endl;
     return 1;
   }

   // set defaults, reset from init file pdbhtmlrc, write pdbhtmlrc.html
   struct run_control * rc = readpdbhtmlrc();


   if( rc->just_build_pdbhtmlrc_html == 1 ) {
      return 0;
   }


   // initialize keyword and item lookup tables
   set<const char *, ltstr> isKeyword(keywords, keywords+NUMBER_OF_KEYWORDS);
   map<const char *, const pdbItem *, ltstr> isItem;
   map<const char *, const pdbItem *, ltstr>::iterator iit;

   PDB::classvec& c = pdb.getClassVec();

   for (PDB::classvec::const_iterator cit=c.begin(); cit!=c.end(); ++cit) {

      if ( !(*cit)->isTemplate() ) {
         if ( (iit = isItem.find((*cit)->name().c_str())) != isItem.end() ) {

            (*iit).second = 0;

         } else {
            isItem[(*cit)->name().c_str()] = (*cit);

         }
      }

   }

   PDB::typevec& t = pdb.getTypeVec();
   for (PDB::typevec::const_iterator tit=t.begin(); tit!=t.end(); ++tit) {

      if ( ((*tit)->kind()==pdbItem::TY_TREF && (*tit)->qualifiers().size()==0)
          || (*tit)->kind()==pdbItem::TY_ENUM ) {
         if ( (iit = isItem.find((*tit)->name().c_str())) != isItem.end() ) {

            (*iit).second = 0;

         } else {
            isItem[(*tit)->name().c_str()] = (*tit);

         }
      }

   }
   PDB::namespacevec& n = pdb.getNamespaceVec();
   for (PDB::namespacevec::const_iterator nit=n.begin(); nit!=n.end(); ++nit) {
      if ( (*nit)->location().file() ) {
         if ( (iit = isItem.find((*nit)->name().c_str())) != isItem.end() ) {

            (*iit).second = 0;

         } else {
            isItem[(*nit)->name().c_str()] = (*nit);

         }
      }
   }

   // build file list
   PDB::filevec startfiles = pdb.getFileVec();
   vector<filenamedata*> augmentedfiles;
   vector<filenamedata*> usefiles;
   string::size_type path_end;
   PDB::filevec::const_iterator sfi;
   bool foundit;
   bool dup;

   for( sfi=startfiles.begin(); sfi!=startfiles.end(); ++sfi) {
      string pdbfile = (*sfi)->name();

      path_end = pdbfile.find_last_of( "/" );
      if( path_end == string::npos ) {
         // No directory path in name

         ifstream fileisopen(pdbfile.c_str());
         if( fileisopen ) {

            fileisopen.close();
            augmentedfiles.push_back(new filenamedata(pdbfile, pdbfile, dup,
                            pdbfile, pdbfile, pdbfile ) );

           } else {

            if( rc->read_src_files_from.empty() ) {

               if( rc->exit_on_missing_src_file ) {

                  cerr << "Error: Cannot open:" << pdbfile << endl;
                  return 1;
               }

            } else {

               vector<filenamedata*>::iterator vi;
               string trythispath;
               bool foundit = false;
               for( vi=rc->read_src_files_from.begin(); 
                    vi != rc->read_src_files_from.end(); vi++ ) {
                  trythispath = (*vi)->path + "/" + pdbfile ;

                  ifstream fileisopen(trythispath.c_str());
                  if( fileisopen ) {

                     fileisopen.close();
                     foundit = true;
                     augmentedfiles.push_back(new filenamedata(trythispath, 
                           pdbfile, dup, trythispath, pdbfile, pdbfile ) );
                     break;
                  }
               }

               if( !foundit && rc->exit_on_missing_src_file ) {

                  cerr << "Error: Cannot open:" << pdbfile << endl;
                  return 1;
               }
            }
         }

      } else {   

         if( ++path_end == string::npos ) {
            cerr << "File listed in pdb file ends in '/' " << endl;
            continue;
         } else {
            string path     = pdbfile.substr(0,path_end);
            string filename = pdbfile.substr(path_end);
            string incform = filename;

            if( string::npos != path.find("/usr/include",0) ) {
               incform = pdbfile.substr(sizeof("/usr/include") );
            }   
              
            char htmlname[128];
            memset( (void*)htmlname, (int) 0, (size_t) 128 );

            strcpy( htmlname, pdbfile.c_str() );

            int i = 0;
            while( *(htmlname + i) != '\0' ) {
               if( htmlname[i]  == '/' ) { 
                  htmlname[i] = rc->slash_replacement; 
               } else if( htmlname[i]  == '.' ) {
                  htmlname[i] = rc->dot_replacement;
               }
               ++i;
            }
            augmentedfiles.push_back(new filenamedata(pdbfile, 
                              filename, dup, path, incform, htmlname) );
         }
      }
   }


   // remove duplicates

   vector<filenamedata*>::iterator afi;
   for( afi=augmentedfiles.begin(); afi!=augmentedfiles.end(); ++afi) {

      if ( isExcludedFile( rc, *afi) ) continue;

      foundit = false;
      if( usefiles.size() > 0 ) {
         vector<filenamedata*>::iterator ufi;

         for( ufi=usefiles.begin(); ufi!=usefiles.end(); ++ufi) {
            if( ! (*afi)->name.compare( (*ufi)->name ) ) {   
               (*afi)->dupname = true;
               (*ufi)->dupname = true;
               if( ! (*afi)->original.compare( (*ufi)->original ) ) {   
                  foundit = true; 
                  break;
               }
            }
         }
         if( foundit ) continue;
      }
      usefiles.push_back((*afi));
   }
   

// generate index files
   {
      string writehere;
      if( rc->write_html_files_to ) {
         writehere = rc->write_html_files_to->path + "/index.html";
      } else {
         writehere =  "index.html";
      } 

      ofstream ostr(writehere.c_str());

      if (!ostr) {
         cerr << "Error: Cannot open 'index.html'" << endl;
         return 1;
      }

      generateIndex(pdb, ostr, argv[1], rc, usefiles, isItem );
   }



   // generate HTML version of every source file
   vector<filenamedata*>::iterator it;
   for( it=usefiles.begin(); it!=usefiles.end(); ++it) {
      if ( ! processFile(pdb, *it, isKeyword, isItem, rc, usefiles) ) return 1;
   }

   cout << endl << "All Done." << endl;   

   return 0;
}


