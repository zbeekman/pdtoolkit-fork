#!/usr/bin/env perl
############################################################################
## PDB -> SQLite3 enumeration generator
## Matthew Sottile / matt@cs.uoregon.edu
##
## Change history:
##  Original version : Feb. 2008
##  Python addition  : Jul. 2008
##  Ocaml addition   : Jun. 2009
##  Haskell addition : Dec. 2009
############################################################################

use strict;
use pdbSql;
use Getopt::Long;

############################################################################
## hash mappers for each language target

# python
sub hash_to_pythondict {
    my ($name, %hash) = @_;
    my $str = "$name = {\n";
    my $key = "";
    foreach $key (keys(%hash)) {
	$str .= "  $hash{$key}: '$key',\n";
    }
    chop($str);
    chop($str);
    $str .= "\n}\n";
    return $str;
}

# ruby
sub hash_to_rubyhash {
  my ($name, %hash) = @_;

  my $str ="$name = {\n";
  my $key = "";
  foreach $key (keys(%hash)) {
    $str .= "  '$hash{$key}' => '$key',\n";
  }
  chop($str);
  chop($str);
  $str .= "\n}\n";
  return $str;
}

# ocaml
sub hash_to_ocaml {
  my ($name, $pfx, %hash) = @_;
  
  my $typestr = "type ".$name."_type =\n";
  my $typefun = "let ".$name."_num_to_type x =\n  match x with\n";
  my $key = "";
  my $first = 0;
  foreach $key (keys(%hash)) {
      my $namekey = $pfx."_".$key;
      $typestr .= " | $namekey\n";
      if ($first == 0) {
          $typefun .= "      $hash{$key} -> $namekey\n";          
          $first++;
      } else {
          $typefun .= "    | $hash{$key} -> $namekey\n";
      }
  }
  $typefun .= "    | _ -> raise (PDBException \"$name bad value\")\n";
  chop($typestr);
  chop($typefun);
  $typestr =~ s/c\+\+/cxx/g;
  $typefun =~ s/c\+\+/cxx/g;
  $typestr =~ s/C\+\+/CXX/g;
  $typefun =~ s/C\+\+/CXX/g;


  $typestr .= ";;\n\n";
  $typefun .= ";;\n\n";
  return $typestr.$typefun;
}

# haskell
sub hash_to_haskell {
  my ($name, $pfx, %hash) = @_;
  
  my $typestr = "data ".ucfirst($name)." =\n";
  my $fun_name = $name."_num_to_type x ";
  my $typefun = $name."_num_to_type :: Int -> ".ucfirst($name)."\n";
  my $key = "";
  my $first = 0;
  foreach $key (keys(%hash)) {
      my $namekey = $pfx."_".$key;
      if ($first == 0) {
	  $typestr .= "    $namekey\n";
	  $first++;
      } else {
	  $typestr .= "  | $namekey\n";
      }
      $typefun .= $fun_name."| x == $hash{$key} = $namekey\n";
  }
  $typefun .= $fun_name."| otherwise = error \"bad PDT type for $name\"\n\n";
  chop($typestr);
  chop($typefun);
  $typestr =~ s/c\+\+/cxx/g;
  $typefun =~ s/c\+\+/cxx/g;
  $typestr =~ s/C\+\+/CXX/g;
  $typefun =~ s/C\+\+/CXX/g;
  
  $typestr .= "\n  deriving Show\n\n";
  $typefun .= "\n\n";

  return $typestr.$typefun;
}

# C++
sub hash_to_cxx_enum {
  my ($name, $pfx, %hash) = @_;

  my $str = "  enum $name {\n";

  $pfx = uc $pfx;

  my $first = 0;
  my $key = "";
  foreach $key (keys(%hash)) {
      my $val = $hash{$key};
      $key =~ s/\+\+/xx/g;
      $key = uc $key;
      if ($first == 0) {
	  $str .= "     $pfx"."_$key = $val\n";
	  $first++;
      } else {
	  $str .= "   , $pfx"."_$key = $val\n";
      }
  }
  $str .= "  };\n";
  return $str;
}

############################################################################
## language printers below

# python
sub emitPython {
  print &hash_to_pythondict("quals",%pdbSql::quals);
  print &hash_to_pythondict("gmkinds",%pdbSql::gmkinds);
  print &hash_to_pythondict("nsmemtype",%pdbSql::nsmemtype);
  print &hash_to_pythondict("linkage",%pdbSql::linkage);
  print &hash_to_pythondict("kind",%pdbSql::kind);
  print &hash_to_pythondict("skind",%pdbSql::skind);
  print &hash_to_pythondict("virt",%pdbSql::virt);
  print &hash_to_pythondict("floatkinds",%pdbSql::floatkinds);
  print &hash_to_pythondict("intkinds",%pdbSql::intkinds);
  print &hash_to_pythondict("tekinds",%pdbSql::tekinds);  
  print &hash_to_pythondict("macrokinds",%pdbSql::macrokinds);
  print &hash_to_pythondict("groupkinds",%pdbSql::groupkinds);
  print &hash_to_pythondict("typekind",%pdbSql::typekind);
  print &hash_to_pythondict("access",%pdbSql::access);
  print &hash_to_pythondict("languages",%pdbSql::languages);
}

# ruby
sub emitRuby {
  print &hash_to_rubyhash("quals",%pdbSql::quals);
  print &hash_to_rubyhash("gmkinds",%pdbSql::gmkinds);
  print &hash_to_rubyhash("nsmemtype",%pdbSql::nsmemtype);
  print &hash_to_rubyhash("linkage",%pdbSql::linkage);
  print &hash_to_rubyhash("kind",%pdbSql::kind);
  print &hash_to_rubyhash("skind",%pdbSql::skind);
  print &hash_to_rubyhash("virt",%pdbSql::virt);
  print &hash_to_rubyhash("floatkinds",%pdbSql::floatkinds);
  print &hash_to_rubyhash("intkinds",%pdbSql::intkinds);
  print &hash_to_rubyhash("tekinds",%pdbSql::tekinds);  
  print &hash_to_rubyhash("macrokinds",%pdbSql::macrokinds);
  print &hash_to_rubyhash("groupkinds",%pdbSql::groupkinds);
  print &hash_to_rubyhash("typekind",%pdbSql::typekind);
  print &hash_to_rubyhash("access",%pdbSql::access);
  print &hash_to_rubyhash("languages",%pdbSql::languages);
}

# C++
sub emitCxx {
  print "class pdbSql {\npublic:\n";
  print &hash_to_cxx_enum("quals","QUAL",%pdbSql::quals);
  print &hash_to_cxx_enum("gmkinds","GMK",%pdbSql::gmkinds);
  print &hash_to_cxx_enum("nsmemtype","NS",%pdbSql::nsmemtype);
  print &hash_to_cxx_enum("linkage","LINK",%pdbSql::linkage);
  print &hash_to_cxx_enum("kind","KIND",%pdbSql::kind);
  print &hash_to_cxx_enum("skind","SK",%pdbSql::skind);
  print &hash_to_cxx_enum("virt","VIRT",%pdbSql::virt);
  print &hash_to_cxx_enum("floatkinds","FK",%pdbSql::floatkinds);
  print &hash_to_cxx_enum("intkinds","IK",%pdbSql::intkinds);
  print &hash_to_cxx_enum("tekinds","TEK",%pdbSql::tekinds);  
  print &hash_to_cxx_enum("macrokinds","MK",%pdbSql::macrokinds);
  print &hash_to_cxx_enum("groupkinds","GK",%pdbSql::groupkinds);
  print &hash_to_cxx_enum("typekind","TK",%pdbSql::typekind);
  print &hash_to_cxx_enum("access","AC",%pdbSql::access);
  print &hash_to_cxx_enum("languages","LA",%pdbSql::languages);
  print "};\n";
}

# Ocaml
sub emitOcaml {
  print "(* This file is automatically generated.  Do not modify by hand. *)\n\n";
  print "exception PDBException of string;;\n\n";
  print &hash_to_ocaml("quals","QUAL",%pdbSql::quals);
  print &hash_to_ocaml("gmkinds","GMK",%pdbSql::gmkinds);
  print &hash_to_ocaml("nsmemtype","NS",%pdbSql::nsmemtype);
  print &hash_to_ocaml("linkage","LINK",%pdbSql::linkage);
  print &hash_to_ocaml("kind","KIND",%pdbSql::kind);
  print &hash_to_ocaml("skind","SK",%pdbSql::skind);
  print &hash_to_ocaml("virt","VIRT",%pdbSql::virt);
  print &hash_to_ocaml("floatkinds","FK",%pdbSql::floatkinds);
  print &hash_to_ocaml("intkinds","IK",%pdbSql::intkinds);
  print &hash_to_ocaml("tekinds","TEK",%pdbSql::tekinds);  
  print &hash_to_ocaml("macrokinds","MK",%pdbSql::macrokinds);
  print &hash_to_ocaml("groupkinds","GK",%pdbSql::groupkinds);
  print &hash_to_ocaml("typekind","TK",%pdbSql::typekind);
  print &hash_to_ocaml("access","AC",%pdbSql::access);
  print &hash_to_ocaml("languages","LA",%pdbSql::languages);
}

# Haskell
sub emitHaskell {
  print "-- This file is automatically generated.  Do not modify by hand.\n\n";
  print "module PDT where\n\n";
  print &hash_to_haskell("quals","QUAL",%pdbSql::quals);
  print &hash_to_haskell("gmkinds","GMK",%pdbSql::gmkinds);
  print &hash_to_haskell("nsmemtype","NS",%pdbSql::nsmemtype);
  print &hash_to_haskell("linkage","LINK",%pdbSql::linkage);
  print &hash_to_haskell("kind","KIND",%pdbSql::kind);
  print &hash_to_haskell("skind","SK",%pdbSql::skind);
  print &hash_to_haskell("virt","VIRT",%pdbSql::virt);
  print &hash_to_haskell("floatkinds","FK",%pdbSql::floatkinds);
  print &hash_to_haskell("intkinds","IK",%pdbSql::intkinds);
  print &hash_to_haskell("tekinds","TEK",%pdbSql::tekinds);  
  print &hash_to_haskell("macrokinds","MK",%pdbSql::macrokinds);
  print &hash_to_haskell("groupkinds","GK",%pdbSql::groupkinds);
  print &hash_to_haskell("typekind","TK",%pdbSql::typekind);
  print &hash_to_haskell("access","AC",%pdbSql::access);
  print &hash_to_haskell("languages","LA",%pdbSql::languages);
}


############################################################################
## main program

# default options
my $doOcaml = '';
my $doHaskell = '';
my $doCxx = '';
my $doPython = '';
my $doRuby = '';
my $count = 0;

GetOptions ('ocaml' => sub { $doOcaml = 1; $count++ },
	    'haskell' => sub { $doHaskell = 1; $count++ },
	    'cxx' => sub { $doCxx = 1; $count++ },
	    'python' => sub { $doPython = 1; $count++ },
	    'ruby' => sub { $doRuby = 1; $count++ });

# sanity checks
if ($count == 0) {
    print "ERROR: You must specify a language target.\n";
    print "  Valid targets: --python | --ruby | --cxx | --ocaml | --haskell\n\n";
    exit(1);
}

if ($count > 1) {
    print "ERROR: You may specify only one language target.\n";
    exit(1);
}

# emit chosen target language
if ($doOcaml) {
    emitOcaml;
}

if ($doCxx) {
    emitCxx;
}

if ($doPython) {
    emitPython;
}

if ($doRuby) {
    emitRuby;
}

if ($doHaskell) {
    emitHaskell;
}
