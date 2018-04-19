##############################################################################
##
## FILE OF ENUMERATIONS USED BY PDBSQL
##
## These are enumerations intended to be used within the SQLite PDB
## representation.  These currently do not necessarily match the values
## inside the Ductape pdbItem class since those are not explicitly assigned
## values in the class definition.  This is easily rectified later though
## if it ever becomes necessary.
##
## matt@cs.uoregon.edu / Feb. 2008
##
##############################################################################

package pdbSql;

use strict;

# languages
our %languages = (
  "c",        0x01,
  "c++",      0x02,
  "c_or_c++", 0x03,
  "fortran",  0x04,
  "java",     0x08,
  "multi",    0x0f);

# access
our %access = (
  "NA",   0,
  "pub",  1,
  "prot", 2,
  "priv", 3);

# namespace members
our %nsmemtype = (
  "ty", 1,
  "ro", 2,
  "gr", 3,
  "te", 4,
  "na", 5);

# linkage
our %linkage = (
  "no",       0,
  "internal", 1,
  "C++",      2,
  "C",        3,
  "fint",     4,
  "f90",      5);

# kinds for functions
our %kind = (
  "ext",     1, 
  "stat",    2, 
  "auto",    3, 
  "NA",      4, 
  "asm",     5, 
  "tproto",  6, 
  "fext",    7, 
  "fprog",   8, 
  "fbldat",  9, 
  "fintrin", 10, 
  "fint",    11,
  "fstfn",   12, 
  "fmproc",  13, 
  "funspec", 14, 
  "falias",  15);

# specialized kinds
our %skind = (
  "ctor", 1, 
  "dtor", 2, 
  "conv", 3, 
  "op",   4 );

# virtual interfaces
our %virt = ( 
  "no",   0,
  "virt", 1,
  "pure", 2 );

# kinds for types
our %typekind = (
  "NA",          0,
  "err",         1,
  "void",        2,
  "int",         3,
  "float",       4,
  "ptr",         5,
  "ref",         6,
  "func",        7,
  "array",       8,
  "tref",        9,       
  "ptrmem",      10,
  "tparam",      11,
  "enum",        12,
  "wchar",       13,
  "bool",        14,
  "ferr",        15,
  "fvoid",       16,           
  "fint",        17,
  "flogic",      18,
  "ffloat",      19,
  "ffunc",       20,
  "fchar",       21,
  "farray",      22,                 
  "fcmplx",      23,
  "funspecfunc", 24,
  "fbldat",      25,
  "fmod",        26,
  "fptr",        27);

# type qualifiers
our %quals = (
  "const",    1,
  "volatile", 2,
  "restrict", 4);

# floating-point types
our %floatkinds = ( 
  "float",   1,
  "dbl",     2,
  "longdbl", 3);

# integer types
our %intkinds = ( 
  "char",      1,
  "schar",     2,
  "uchar",     3,
  "wchar",     4,
  "short",     5,
  "ushort",    6,
  "int",       7,
  "uint",      8,
  "long",      9,
  "ulong",     10,
  "longlong",  11,
  "ulonglong", 12);

# template kinds
our %tekinds = ( 
  "none",     0,
  "class",    1,
  "func",     2,
  "memfunc",  3,
  "statmem",  4,
  "memclass", 5,
  "ttparam",  6);
  
# fortran array shapes
our %tyfshape = (
  "explicit",  1,
  "asmdsize",  2,
  "asmdshape", 3,
  "deferred",  4);

# group member kinds
our %gmkinds = ( 
  "type",    1, 
  "statvar", 2, 
  "var",     3, 
  "templ",   4 );

# group kinds
our %groupkinds = ( 
  "class",    1, 
  "struct",   2, 
  "union",    3, 
  "tproto",   4, 
  "fderived", 5,
  "fmodule",  6);

# macro kinds
our %macrokinds = ( 
  "def",   1,
  "undef", 2);

1;
