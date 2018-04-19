======================
pdbsql : December 2009
======================

1.0 OVERVIEW
------------

The pdbsql package includes two important pieces:

 - An SQLite schema that represents PDB files as a relational database.
 - A Perl script for converting PDB 3.0 files to the SQLite form.
 - A Perl script for generating code to map PDB integer values to symbolic
   types for use in languages from which SQLite will be accessed.

The goal of this package is to allow users to write code that consumes
data contained within PDB files in a wider set of languages than
currently provided by the C++ Ductape API alone.  Any language that
has a binding to SQLite can use this method of accessing PDB data.
Furthermore, the use of SQL to construct queries on the data removes
the need for the user to explicitly code up the query by combining STL
data structures, iterators, and query-specific logic.  This also means
that general purpose user interface tools for accessing the data in
the database can traverse the PDB data using the standard SQL language.

2.0 GETTING STARTED
-------------------

The current version of pdbsql does not include scripts to wrap up the
process of generating .PDB files from source code and populating the
SQLite database.  The process must currently be performed by hand.
Fortunately, it is fairly simple.  First, generate a PDB file as you would
for normal PDT applications.

  % cxxparse foo.cxx
  % cparse bar.c
  % gfparse ffoo.f90

These yield .pdb files containing the PDB data for the given source code.
If you require the PDB files to contain comment information, you must
post-process the PDB file with the command "pdbcomment".

  % pdbcomment foo.pdb -o foo_comments.pdb

This is necessary for applications such as the CCA code generation tools
that use comments to carry annotation information.  Now, we can create
the SQLite database.  First, you must initialize the database with the
provided schema (contained in schema.sql).  We will assume you wish to
store the SQLite database in the file "myprogram.db".

Before proceeding, ensure that you have sqlite3 installed on your
system.  If you do not, skip to section 2.1 before proceeding.

  % sqlite3 myprogram.db < schema.sql

This yields the file myprogram.db containing an empty database structured
based on the schema.  Now, we must populate it.  To avoid requiring
additional, non-standard Perl modules to be installed prior to using
this code, the Perl conversion script does not directly work with the
raw database file.  Instead, it currently emits a sequence of SQL commands
as a single atomic transaction operation which can be piped to the sqlite3
command, or stored in a file.

  % pdb_to_sql.pl foo_comments.pdb | sqlite3 myprogram.db

This will populate the contents of myprogram.db.  The performance is
tolerable for demonstration and prototyping purposes.  Later, we will
be streamlining this process of moving from source code to a fully
populated SQLite3 database.  In the meantime, we are instead focusing on
working out the kinks of the schema design and conversion process,
and will worry about auxiliary tools later.

2.1 INSTALLING THE BUNDLED SQLITE3
----------------------------------

Fortunately, SQLite is in the public domain, so we can carry the
source distribution (which is small) along with the pdbsql work with
no trouble in the event that you, the user, don't have it installed
already and do not have permissions to do so through the system
package management system.  It is easy to build and install for
use somewhere in your home directory.

Assuming that we want to compile and build the code in the current
directory, we first unpack the tar file.

  % tar xf sqlite-3.5.6.tar.gz

Note that if that file is not found, the documentation is probably
out of date and a newer version with a slightly different filename
is in the directory in it's place.

Go into the directory that was created from the tar file, and
create a local installation directory and execute configure.

  % cd sqlite-3.5.6
  % mkdir BUILD
  % ./configure --prefix=`pwd`/BUILD --disable-tcl --disable-shared

The arguments to disable TCL and shared libraries are merely there to
decrease the likelihood of configure failing on some systems (or
worse, succeeding but creating makefiles that fail - this has been
observed on MacOSX 10.5 due to Tcl).  Now, we can build and install
the code:

  % make
  % make install

The result will be three directories being created in the BUILD
directory created earlier : bin, include, and lib.  For now, we don't
care about the include or lib directories.  If you add the bin
directory to your path though, the sqlite3 command used in section 2.0
above will now be available.

  % export PATH=$PATH:`pwd`/BUILD/bin

3.0 ACCESSING SQLITE3 DATA
--------------------------

Given an SQLite database containing PDB information, the next step is to
write programs that access it.  Any language with an SQLite3 binding will
do.  For common ones, we provide a helper script that automatically generates
functions and data types for mapping integer values used to represent
enumerations in PDB to symbolic representations for easier programming.
This is performed by enum_gen.pl.

Currently enum_gen.pl supports five target languages:

 * OCaml
 * Haskell
 * Python
 * Ruby
 * C++

Executing enum_gen.pl without arguments lists the flags for each supported
language.  Note that only one language can be generated at a time - if you
want to generate for multiple languages, you must run the script once per
language.  The contents of the automatically generated code are printed to
STDOUT by the script, so they must be redirected to a file.  For example,
to generate the Haskell bindings, we say:

 % ./enum_gen.pl --haskell > PDT.hs

Note in some cases (such as Haskell) the filename matters as it corresponds
to the name of a module contained within the generated code.

Please refer to your favorite search engine for bindings from your favorite
language to sqlite3.  If you would like to have a language added to the
enum_gen.pl script, please send an e-mail request to Matt.

4.0 GETTING HELP
----------------

Contact the author, Matt Sottile (mjsottile@computer.org).

5.0 SQLITE3 LICENSE
-------------------

The following text was taken from http://www.sqlite.org/.

SQLite is in the Public Domain

All of the deliverable code in SQLite has been dedicated to the public
domain by the authors. All code authors, and representatives of the
companies they work for, have signed affidavits dedicating their
contributions to the public domain and originals of those signed
affidavits are stored in a firesafe at the main offices of
Hwaci. Anyone is free to copy, modify, publish, use, compile, sell, or
distribute the original SQLite code, either in source code form or as
a compiled binary, for any purpose, commercial or non-commercial, and
by any means.

The previous paragraph applies to the deliverable code in SQLite -
those parts of the SQLite library that you actually bundle and ship
with a larger application. Portions of the documentation and some code
used as part of the build process might fall under other licenses. The
details here are unclear. We do not worry about the licensing of the
documentation and build code so much because none of these things are
part of the core deliverable SQLite library.

All of the deliverable code in SQLite has been written from
scratch. No code has been taken from other projects or from the open
internet. Every line of code can be traced back to its original
author, and all of those authors have public domain dedications on
file. So the SQLite code base is clean and is uncontaminated with
licensed code from other projects.
