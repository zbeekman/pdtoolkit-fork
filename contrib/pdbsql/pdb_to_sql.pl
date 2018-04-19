#!/usr/bin/env perl
############################################################################
## PDB -> SQLite3 converter
## Matthew Sottile / matt@cs.uoregon.edu
##
## Assumes PDB 3.0.
############################################################################

use strict;
use pdbSql;

############################################################################
## main program

my $pdb_name = $ARGV[0];

if ($#ARGV != 0) {
  print "Usage: pdb_to_sql.pl [PDB FILENAME]\n\n";
  exit(1);
}

## open files
open(PDBFILE, "< $pdb_name") || die "Error opening $pdb_name\n";

## read a single line, stripping any CR or LF characters from it.  provides
## the $reuse_line variable to cause a line to be pushed back into the queue
## of lines to be returned on the next call instead of popping a new line from
## the file.
my $reuse_flag = 0;
my $reuse_line = "";
sub read_line {
  my $line;
  if ($reuse_flag != 0) {
    $reuse_flag = 0;
    $line = $reuse_line;
    $reuse_line = "";
    return $line;
  }
  $line = <PDBFILE>;
  $line =~ s/(\r|\n)//g;
  return $line;
}

sub push_line {
  my ($line) = @_;
  $reuse_line = $line;
  $reuse_flag = 1;
}

## empty sql sequence
my @sqlseq = ();

##
## header
##

# version magic
my $line = &read_line;
my $vnum;
if ($line =~ /<PDB (\d).(\d)>/) {
  $vnum = $1; # ignore minor version $2 for now
} else {
  die "Invalid PDB file\n";
}

# language
my ($junk,$lang) = split(/ /,&read_line);

my $sql = "INSERT INTO PDB (version,language) VALUES ($vnum, $pdbSql::languages{$lang});";
push(@sqlseq,$sql);

# Absorb empty line
&read_line;

#
# TODO: replace this with an autogenerated key from the database
#
my $pdb_id = 1;

##
## subroutine to take an associative array of key-value pairs and produce
## a SQL string inserting the values values corresponding with each key as
## a single new row in the specified table.
##
sub kvpairs_to_sql {
  my ($tablename, %kvs) = @_;

  my $keys = "";
  my $values = "";
  foreach my $k (keys(%kvs)) {
    $keys = $keys."$k,";
    $values = $values.$kvs{$k}.",";
  }
  chop($keys);
  chop($values);
  my $sql = "INSERT INTO $tablename ($keys) VALUES ($values);";
  push(@sqlseq,$sql);
}

##
## handler for source files
##
sub so_handler {
   my ($id, $fname) = @_;

   my %kvs = ( "ID",       $id,
	       "pdbID",    $pdb_id,
	       "filename", "\'$fname\'",
	       "issys",    "0" );
    
   my $line = &read_line;

   while (length($line) > 0) {
       my ($tag,@vals) = split(/ /,$line);
       
       if ($tag eq "ssys") {
	   $kvs{"issys"} = "1";
       } elsif ($tag eq "sinc") {
	   my ($junk, $incid) = split(/\#/,$vals[0]);
	   my $sql = "INSERT INTO SOURCE_INCLUDES (includerID, includedID) VALUES ($id, $incid);";
	   push(@sqlseq,$sql);
       } elsif ($tag eq "scom") {
    #	   %languages

	   my ($junk, $cID) = split(/\#/,shift(@vals));
	   my $clang = $pdbSql::languages{shift(@vals)};
	   my ($junk, $cFile) = split(/\#/,shift(@vals));
	   my $cbLine = shift(@vals);
	   my $cbCol = shift(@vals);
	   shift(@vals);
	   my $ceLine = shift(@vals);
	   my $ceCol = shift(@vals);
	   my $cval = "";
	   foreach my $vs (@vals) {
	       $vs =~ s/\'/\'\'/g;
	       $cval .= $vs." ";
	   }
	   chop($cval);
	   my $sql = "INSERT INTO COMMENTS (ID, fileID, language, beginLine, beginColumn, endLine, endColumn, value) VALUES ($cID, $cFile, $clang, $cbLine, $cbCol, $ceLine, $ceCol, \'$cval\');";
	   push(@sqlseq, $sql);
       } else {
	   die "UNKNOWN SOURCE TAG $tag\n";
       }
       
       $line = &read_line;
   }
   
   &kvpairs_to_sql("SOURCE", %kvs);
}

##
## handler for macros
##
sub ma_handler {
   my ($id, $mname) = @_;

   my %kvs = ( "ID",       $id,
	       "name",     "\'$mname\'",
	       "kind",     "",
	       "text",     "");
    
   my $line = &read_line;

   while (length($line) > 0) {
       my ($tag,@vals) = split(/ /,$line);
       
       if ($tag eq "mkind") {
	   $kvs{"kind"} = $pdbSql::macrokinds{$vals[0]};
       } elsif ($tag eq "mtext") {
	   my $vstr = "";
	   foreach my $vs (@vals) {
	       $vstr .= $vs." ";
	   }
	   chop($vstr);
	   $vstr =~ s/\'/\'\'/g;
	   $kvs{"text"} = "\'$vstr\'";
       } elsif ($tag eq "mloc") {
	   my $soid;
	   my $junk;
	   ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	   ($junk, $kvs{"fileID"}) = split(/\#/,$soid);
       } else {
	   die "UNKNOWN MACRO TAG $tag\n";
       }
       
       $line = &read_line;
   }
   
   &kvpairs_to_sql("MACROS",%kvs);
}

##
## handler for namespaces
##
sub na_handler {
   my ($id, $nname) = @_;

   my %kvs = ( "ID",       $id,
	       "name",     "\'$nname\'");
    
   my $line = &read_line;

   while (length($line) > 0) {
       my ($tag,@vals) = split(/ /,$line);
       
       if ($tag eq "nloc") {
	   my $soid;
	   my $junk;
	   ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	   ($junk, $kvs{"fileID"}) = split(/\#/,$soid);
       } elsif ($tag eq "nnspace") {
	   my ($junk, $nid) = split(/\#/,$vals[0]);
	   $kvs{"parentID"} = $nid;
       } elsif ($tag eq "nmem") {
	   my ($mtype, $mid) = split(/\#/,$vals[0]);
	   my %mem_kvs;

	   $mem_kvs{"nspaceID"} = $id;
	   $mem_kvs{"type"} = $pdbSql::nsmemtype{$mtype};
	   
	   if ($mtype eq "ty") {
	       $mem_kvs{"typeID"} = $mid;
	   } elsif ($mtype eq "ro") {
	       $mem_kvs{"routineID"} = $mid;
	   } elsif ($mtype eq "gr") {
	       $mem_kvs{"groupID"} = $mid;
	   } elsif ($mtype eq "te") {
	       $mem_kvs{"templateID"} = $mid;
	   } elsif ($mtype eq "na") {
	       $mem_kvs{"namespaceID"} = $mid;
	   } else {
	       die "UNKNOWN NMEM $mtype\n";
	   }
	   &kvpairs_to_sql("NAMESPACE_MEMBERS",%mem_kvs);
       } elsif ($tag eq "npos") {
	   # TODO: ignoring
       } else {
	   die "UNKNOWN NAMESPACE TAG $tag\n";
       }
       
       $line = &read_line;
   }
   
   &kvpairs_to_sql("NAMESPACES",%kvs);
}

##
## type handler
##
sub ty_handler {
    my ($id, @tname_parts) = @_;

    # reassemble type name
    my $tname = "";
    foreach my $tp (@tname_parts) {
	$tname = $tname.$tp." ";
    }
    chop($tname);

    my %kvs = ( "ID",              $id,
		"name",            "\'$tname\'",
		"line",            0,
		"col",             0,
		"fileID",          0,
		"access",          0,
		"kind",            "NULL",
		"trefIsType",      0,
		"trefTypeID",      "NULL",
		"trefGroupID",     "NULL",
		"qualifiers",      0,
		"indirectIsType",  0,
		"indirectTypeID",  "NULL",
		"indirectGroupID", "NULL",
		"intKind",         "NULL",
		"elementIsType",   0,
		"elementTypeID",   "NULL",
		"elementGroupID",  "NULL",
		"nelem",           0,
		"rettIsType",      0,
		"rettTypeID",      "NULL",
		"rettGroupID",     "NULL",
		"hasEllip",        0,
		"isSigned",        0
	);

    my $arg_pos = 0;

    my $line = &read_line;
    while (length($line) > 0) {
	my ($tag,@vals) = split(/ /,$line);
	
	if ($tag eq "ykind") {
	    $kvs{"kind"} = $pdbSql::typekind{$vals[0]};
	} elsif ($tag eq "yacs") {
	    $kvs{"access"} = $pdbSql::access{$vals[0]};
	} elsif ($tag eq "ytref") {
	    my ($ty, $rid) = split(/\#/,$vals[0]);
	    if ($ty eq "ty") {
		$kvs{"trefIsType"} = 1;
		$kvs{"trefTypeID"} = $rid;
	    } elsif ($ty eq "gr") {
		$kvs{"trefIsType"} = 0;
		$kvs{"trefGroupID"} = $rid;
	    } else {
		die "BOGUS ytref $ty\n";
	    }
	} elsif ($tag eq "ygroup") {
	    my ($junk, $gid) = split(/\#/,$vals[0]);
	    $kvs{"groupID"} = $gid;
	} elsif ($tag eq "ynspace") {
	    my ($junk, $nid) = split(/\#/,$vals[0]);
	    $kvs{"nspaceID"} = $nid;
	} elsif ($tag eq "yqual") {
	    my $qualval = 0;
	    foreach my $qv (@vals) {
		$qualval = $qualval | $pdbSql::quals{$qv};
	    }
	    $kvs{"qualifiers"} = $qualval;
	} elsif ($tag eq "yptr" || $tag eq "yref") {
	    my ($ty, $rid) = split(/\#/,$vals[0]);
	    if ($ty eq "ty") {
		$kvs{"indirectIsType"} = 1;
		$kvs{"indirectTypeID"} = $rid;
	    } elsif ($ty eq "gr") {
		$kvs{"indirectIsType"} = 0;
		$kvs{"indirectGroupID"} = $rid;
	    } else {
		die "BOGUS yptr\n";
	    }
	} elsif ($tag eq "yelem") {
	    my ($ty, $rid) = split(/\#/,$vals[0]);
	    if ($ty eq "ty") {
		$kvs{"elementIsType"} = 1;
		$kvs{"elementTypeID"} = $rid;
	    } elsif ($ty eq "gr") {
		$kvs{"elementIsType"} = 0;
		$kvs{"elementGroupID"} = $rid;
	    } else {
		die "BOGUS yelem\n";
	    }
  } elsif ($tag eq "yshape") {
    $kvs{"shape"} = $pdbSql::tyfshape{$vals[0]};
  } elsif ($tag eq "yrank") {
    $kvs{"rank"} = $vals[0];
	} elsif ($tag eq "ynelem") {
	    $kvs{"nelem"} = $vals[0];
	} elsif ($tag eq "yrett") {
	    my ($ty, $rid) = split(/\#/,$vals[0]);
	    if ($ty eq "ty") {
		$kvs{"rettIsType"} = 1;
		$kvs{"rettTypeID"} = $rid;
	    } elsif ($ty eq "gr") {
		$kvs{"rettIsType"} = 0;
		$kvs{"rettGroupID"} = $rid;
	    } else {
		die "BOGUS yrett\n";
	    }	    
	} elsif ($tag eq "yloc") {
	    my $soid;
	    my $junk;
	    ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	    ($junk, $kvs{"fileID"}) = split(/\#/,$soid);
	} elsif ($tag eq "yikind") {
	    $kvs{"intKind"} = $pdbSql::intkinds{$vals[0]};
	} elsif ($tag eq "yfkind") {
	    $kvs{"fKind"} = $pdbSql::floatkinds{$vals[0]};
	} elsif ($tag eq "yexcep") {
	    ## TODO: ignoring for now
        } elsif ($tag eq "yold") {
	    ## TODO: ignoring for now
	} elsif ($tag eq "yellip") {
	    if ($vals[0] eq "T") {
		$kvs{"hasEllip"} = 1;
	    }
	} elsif ($tag eq "ysigned") {
	    if ($vals[0] eq "T") {
		$kvs{"isSigned"} = 1;
	    }	    
	} elsif ($tag eq "yargt") {
	    my ($ty, $rid) = split(/\#/,$vals[0]);

	    my $isType = 0;
	    my $typeID = "NULL";
	    my $groupID = "NULL";

	    if ($ty eq "ty") {
		$isType = 1;
		$typeID = $rid;
	    } elsif ($ty eq "gr") {
		$isType = 0;
		$groupID = $rid;
	    }

	    my %yargt_kvs = ("itemID",$id,
			     "name","\'$vals[1]\'",
			     "position",$arg_pos,
			     "isType",$isType,
			     "typeID",$typeID,
			     "groupID",$groupID,
			     "intent",0);
	    if (length($vals[3]) > 0) {
		$yargt_kvs{"line"} = $vals[3];
	    }
	    if (length($vals[4]) > 0) {
		$yargt_kvs{"col"} = $vals[4];
	    }

	    ## TODO : not handling intent right now
	    &kvpairs_to_sql("TYPES_ARGS",%yargt_kvs);
	    $arg_pos++;
	} elsif ($tag eq "yenum") {
	    my $sql = "INSERT INTO TYPES_ENUMS (itemID, name, value) VALUES ($id, \'".$vals[0]."\', \'".$vals[1]."\');";
	    push(@sqlseq,$sql);
	} else {
	    die "TYPE TAG UNSUPPORTED : $tag\n";
	}
	
	$line = &read_line;
    }

    &kvpairs_to_sql("TYPES", %kvs);
}

##
## gmem handler
##
sub gmem_handler {
    my ($gmem_name,$parentID) = @_;

    my %kvs = ( "name",          "\'$gmem_name\'",
		"line",          "NULL",
		"col",           "NULL",
		"parentGroupID", $parentID,
		"access",        0,
		"kind",          0,
		"isType",        0,
		"typeID",        "NULL",
		"groupID",       "NULL",
		"isBit",         0,
		"isConst",       0,
		"isMut",         0
	);

    my $line = &read_line;
    while (length($line) > 0) {
	my ($tag, @vals) = split(/ /,$line);
	if ($tag !~ /^gm/ || $tag eq "gmem") {
	    &push_line($line);
	    &kvpairs_to_sql("GROUP_MEMBERS",%kvs);
	    return;
	} elsif ($tag eq "gmloc") {
	    my $soid;
	    my $junk;
	    
	    ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	} elsif ($tag eq "gmtempl") {
	    my ($junk, $tid) = split(/\#/,$vals[0]);
	    $kvs{"templID"} = $tid;
	} elsif ($tag eq "gmgroup") {
	    my $junk;
	    ($junk, $kvs{"parentGroupID"}) = split(/\#/,$vals[0]);
	} elsif ($tag eq "gmacs") {
	    $kvs{"access"} = $pdbSql::access{$vals[0]};
	} elsif ($tag eq "gmkind") {
	    $kvs{"kind"} = $pdbSql::gmkinds{$vals[0]};
	} elsif ($tag eq "gmtype") {
	    my ($lbl, $tid) = split(/\#/,$vals[0]);
	    if ($lbl eq "ty") {
		$kvs{"isType"} = 1;
		$kvs{"typeID"} = $tid;
	    } elsif ($lbl eq "gr") {
		$kvs{"isType"} = 0;
		$kvs{"groupID"} = $tid;
	    } else {
		die "Bad gmtype\n";
	    }
	} elsif ($tag eq "gmmut") {
	    if ($vals[0] eq "T") {
		$kvs{"isMut"} = 1;
	    }
	} elsif ($tag eq "gmconst") {
	    if ($vals[0] eq "T") {
		$kvs{"isConst"} = 1;
	    }
	} elsif ($tag eq "gmisbit") {
	    if ($vals[0] eq "T") {
		$kvs{"isBit"} = 1;
	    }
	} else {
	    die "Bad GMEM tag: $tag\n";
	}

	$line = &read_line;
    } 

    &push_line($line);

    &kvpairs_to_sql("GROUP_MEMBERS",%kvs);
}

##
## group handler
##
sub gr_handler {
#    my ($id, $grname) = @_;

    my ($id, @grname_parts) = @_;
    my $grname = "";
    foreach my $gp (@grname_parts) {
        $grname = $grname.$gp." ";
    }
    #print STDERR $grname;
    chop($grname);


    my %kvs = ( "ID",   $id,
		"name", "\'$grname\'",
		"fileID",      "NULL",
		"line",        "NULL",
		"col",         "NULL",
		"kind",        0,
		"parentID",    "NULL",
		"isSpecl",     "0",
		"access",      0
	);

    my $line = &read_line;
    while (length($line) > 0) {
	my ($tag,@vals) = split(/ /,$line);
	
	if ($tag eq "gloc") {
	    my $soid;
	    my $junk;
	    
	    ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	    ($junk, $kvs{"fileID"}) = split(/\#/,$soid);
	} elsif ($tag eq "gkind") {
	    $kvs{"kind"} = $pdbSql::groupkinds{$vals[0]};
	} elsif ($tag eq "gfunc") {
	    # note that location is thrown out since it is
	    # redundant with the routine table
	    my ($junk, $rnum) = split(/\#/,$vals[0]);
	    my $sql = "INSERT INTO GROUP_FUNCTIONS (routineID,isFriend,groupID) VALUES ($rnum,0,$id);";
	    push(@sqlseq,$sql);	    
	} elsif ($tag eq "gmem") {
	    &gmem_handler($vals[0],$id);
	} elsif ($tag eq "gnspace") {
	    my ($junk, $nid) = split(/\#/,$vals[0]);
	    $kvs{"nspaceID"} = $nid;
	} elsif ($tag eq "gpos") {
	    # TODO: ignoring
	} elsif ($tag eq "gbase") {
	    my $sql = "INSERT INTO GROUP_INHERITANCE (ID,DirectBaseID,isVirtual,access,fileID,line,col) VALUES (";
	    my $vert = 0;
	    if ($vals[0] eq "virt") {
		$vert = 1;
	    }
	    my $access = $pdbSql::access{$vals[1]};
	    my ($junk, $gid) = split(/\#/,$vals[2]);
	    my ($junk, $fid) = split(/\#/,$vals[3]);
	    $sql .= "$id,$gid,$vert,$access,$fid,$vals[4],$vals[5]);";
	    push(@sqlseq,$sql);
	} elsif ($tag eq "gtempl") {
	    my ($junk, $tid) = split(/\#/,$vals[0]);
	    $kvs{"templID"} = $tid;
	} elsif ($tag eq "ggroup") {
	    my ($junk, $pid) = split(/\#/,$vals[0]);
	    $kvs{"parentID"} = $pid;
	} elsif ($tag eq "gspecl") {
	    if ($vals[0] eq "T") {
		$kvs{"isSpecl"} = 1;
	    }
	} elsif ($tag eq "gacs") {
	    $kvs{"access"} = $pdbSql::access{$vals[0]};
	} elsif ($tag eq "gfrgroup") {
	    my ($junk,$gid) = split(/\#/,$vals[0]);
	    my $sql = "INSERT INTO GROUP_FRIENDS (grID,friendID,isFriendGroup) VALUES ($id,$gid,1);";
	    push(@sqlseq,$sql);
	} elsif ($tag eq "gfrfunc") {
	    my ($junk,$rid) = split(/\#/,$vals[0]);
	    my $sql = "INSERT INTO GROUP_FRIENDS (grID,friendID,isFriendGroup) VALUES ($id,$rid,0);";
	    push(@sqlseq,$sql);	    
	} elsif ($tag eq "gsparam") {
	    my $ptype = $vals[0];
	    my %param_kvs;

	    $param_kvs{"grID"} = $id;
	    $param_kvs{"isSpecialization"} = 1; # TODO: is this redundant?

	    $param_kvs{"type"} = "'".$ptype."'";

	    if ($ptype eq "type") {
		my ($junk, $tid) = split(/\#/,$vals[1]);

		if ($junk eq "gr") {
		    $param_kvs{"isType"} = 0;
		    $param_kvs{"groupID"} = $tid;
		} elsif ($junk eq "ty") {
		    $param_kvs{"isType"} = 1;
		    $param_kvs{"typeID"} = $tid;
		} else {
		    die "BAD gsparam type\n";
		}
	    } elsif ($ptype eq "ntype") {
		$param_kvs{"constant"} = "'".$vals[1]."'";
	    } elsif ($ptype eq "templ") {
		my ($junk, $tid) = split(/\#/,$vals[1]);
		$param_kvs{"templID"} = $tid;
	    } else {
		die "GROUP PARAMETER TYPE UNKNOWN $ptype\n";
	    }
	    &kvpairs_to_sql("GROUP_PARAMS",%param_kvs);
	} else {
	    print "$line\n";
	    die "GROUP TAG UNSUPPORTED : $tag\n";
	}

	$line = &read_line;
    }

    &kvpairs_to_sql("GROUPS",%kvs);
}

##
## template handler
##
sub te_handler {
    my ($id, $tename) = @_;

    my %kvs = ( "ID",   $id,
		"name", "\'$tename\'",
		"fileID",      "NULL",
		"line",        "NULL",
		"col",         "NULL",
		"groupID",     "NULL",
		"access",      1,
		"kind",      0
	);

    my $line = &read_line;
    while (length($line) > 0) {
	my ($tag,@vals) = split(/ /,$line);
	
	if ($tag eq "tloc") {
	    my $soid;
	    my $junk;
	    
	    ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	    ($junk, $kvs{"fileID"}) = split(/\#/,$soid);
	} elsif ($tag eq "tgroup") {
	    my ($junk, $pid) = split(/\#/,$vals[0]);
	    $kvs{"groupID"} = $pid;
	} elsif ($tag eq "tacs") {
	    $kvs{"access"} = $pdbSql::access{$vals[0]};
	} elsif ($tag eq "tkind") {
	    $kvs{"kind"} = $pdbSql::tekinds{$vals[0]};
	} elsif ($tag eq "tpos") {
	    ## TODO: this is currently ignored.
	} elsif ($tag eq "tdef") {
	    my ($junk, $tid) = split(/\#/,$vals[0]);
	    $kvs{"defID"} = $tid;
	} elsif ($tag eq "tdecl") {
	    my ($junk, $tid) = split(/\#/,$vals[0]);
	    $kvs{"declID"} = $tid;
	} elsif ($tag eq "tnspace") {
	    my ($junk, $nid) = split(/\#/,$vals[0]);
	    $kvs{"nspaceID"} = $nid;
	} elsif ($tag eq "tsparam") {
	    my $ptype = $vals[0];
	    my %param_kvs;

	    $param_kvs{"templateID"} = $id;
	    $param_kvs{"partialSpec"} = 1;
	    $param_kvs{"type"} = "'".$ptype."'";

	    if ($ptype eq "type") {
		my ($junk, $tid) = split(/\#/,$vals[1]);
		if ($junk eq "gr") {
		    $param_kvs{"groupID"} = $tid;
		    $param_kvs{"typeIsGroup"} = 1;
		} else {
		    $param_kvs{"typeID"} = $tid;
		    $param_kvs{"typeIsGroup"} = 0;
		}
	    } elsif ($ptype eq "ntype") {
		$param_kvs{"name"} = "'".$vals[1]."'";
	    } elsif ($ptype eq "templ") {
		my ($junk, $tid) = split(/\#/,$vals[1]);
		$param_kvs{"templID"} = $tid;
	    } else {
		die "TEMPLATE SPARAMETER TYPE UNKNOWN $ptype\n";
	    }
	    &kvpairs_to_sql("TEMPLATE_PARAMS",%param_kvs);
	} elsif ($tag eq "tparam") {
	    my $ptype = $vals[0];
	    my %param_kvs;

	    $param_kvs{"templateID"} = $id;
	    $param_kvs{"partialSpec"} = 0;
	    $param_kvs{"type"} = "'".$ptype."'";

	    if ($ptype eq "type") {
		my ($junk, $tid) = split(/\#/,$vals[1]);
		$param_kvs{"typeID"} = $tid;
		$param_kvs{"typeIsGroup"} = 0;

		if (length(@vals) == 2) {
		    my ($junk, $pid) = split(/\#/,$vals[2]);
		    if ($junk eq "gr") {
			$param_kvs{"defaultIsGroup"} = 1;
			$param_kvs{"defGroupID"} = $pid;
		    } elsif ($junk eq "ty") {
			$param_kvs{"defaultIsGroup"} = 0;
			$param_kvs{"defTypeID"} = $pid;
		    } else {
			die "INVALID TEMPL PARAM TYPE DEFAULT $vals[2]\n";
		    }
		}
	    } elsif ($ptype eq "ntype") {
		my ($junk, $tid) = split(/\#/,$vals[1]);
		$param_kvs{"typeID"} = $tid;
		$param_kvs{"name"} = "'".$vals[2]."'";
		if (length(@vals) > 3) {
		    $param_kvs{"defaultValue"} = $vals[3];
		}
	    } elsif ($ptype eq "templ") {
		my ($junk, $tid) = split(/\#/,$vals[1]);
		$param_kvs{"templID"} = $tid;
		if (length(@vals) > 2) {
		    my ($junk, $tid) = split(/\#/,$vals[2]);
		    $param_kvs{"defaultTemplID"} = $tid;
		}
	    } else {
		die "TEMPLATE PARAMETER TYPE UNKNOWN $ptype\n";
	    }
	    &kvpairs_to_sql("TEMPLATE_PARAMS",%param_kvs);
	} else {
	    die "TEMPLATE TAG UNSUPPORTED : $tag\n";
	}

	$line = &read_line;
    }

    &kvpairs_to_sql("TEMPLATES",%kvs);
}


##
## routine handler
##
sub ro_handler {
    my ($id, $roname) = @_;

    my %kvs = ( "ID",            $id,
		"name",          "\'$roname\'",
		"fileID",        "NULL",
		"line",    "NULL",
		"col",     "NULL",
		"endLine",      "NULL",
		"endCol",       "NULL",
		"groupID",       "NULL",
		"access",        "NULL",
		"sigTypeID",     "NULL",
		"linkage",       "0",
		"kind",          "NULL",
		"specialkind",   "NULL",
		"virt",          "0",
		"isInline",      "0",
		"isCgen",        "0",
		"isExpl",        "0",
		"isSpecl",       "0",
		"rtypeLine",     0,
		"rtypeCol",      0,
		"ltokLine",      0,
		"ltokCol",       0,
		"isStatic",      0,
		"bodyID",        "NULL",
		"firstStmtLine", "NULL",
		"firstStmtCol",  "NULL"
	);
    
    my $line = &read_line;
    while (length($line) > 0) {
	my ($tag,@vals) = split(/ /,$line);
	
	if ($tag eq "rloc") {
	    my $soid;
	    my $junk;

	    ($soid, $kvs{"line"}, $kvs{"col"}) = @vals;
	    ($junk, $kvs{"fileID"}) = split(/\#/,$soid);
	} elsif ($tag eq "rstart") {
	    $kvs{"firstStmtLine"} = $vals[1];
	    $kvs{"firstStmtCol"} = $vals[2];
	} elsif ($tag eq "rgroup") {
	    my $junk;
	    ($junk, $kvs{"groupID"}) = split(/\#/,$vals[0]);
	} elsif ($tag eq "racs") {
	    $kvs{"access"} = $pdbSql::access{$vals[0]};
	} elsif ($tag eq "rsig") {
	    my $junk;
	    ($junk, $kvs{"sigTypeID"}) = split(/\#/,$vals[0]);
	} elsif ($tag eq "rlink") {
	    $kvs{"linkage"} = $pdbSql::linkage{$vals[0]};
	} elsif ($tag eq "rkind") {
	    $kvs{"kind"} = $pdbSql::kind{$vals[0]};
	} elsif ($tag eq "rskind") {
	    $kvs{"specialkind"} = $pdbSql::skind{$vals[0]};
	} elsif ($tag eq "rvirt") {
	    $kvs{"virt"} = $pdbSql::virt{$vals[0]};
	} elsif ($tag eq "rnspace") {
	    my ($junk, $nid) = split(/\#/,$vals[0]);
	    $kvs{"nspaceID"} = $nid;
	} elsif ($tag eq "rspecl") {
	    if ($vals[0] eq "T") {
		$kvs{"isSpecl"} = 1;
	    }
	} elsif ($tag eq "rinline") {
	    if ($vals[0] eq "T") {
		$kvs{"isInline"} = 1;
	    }
	} elsif ($tag eq "rexpl") {
	    if ($vals[0] eq "T") {
		$kvs{"isExpl"} = 1;
	    }
	} elsif ($tag eq "rcgen") {
	    if ($vals[0] eq "T") {
		$kvs{"isCgen"} = 1;
	    }
	} elsif ($tag eq "rpos") {
	    my $junk;
	    my @junk2;

	    ($junk, $kvs{"rtypeLine"}, $kvs{"rtypeCol"},
	     $junk, $kvs{"ltokLine"}, $kvs{"ltokCol"},
	     $junk, $junk, $junk,
	     $junk, $kvs{"endLine"}, $kvs{"endCol"}) = @vals;
	} elsif ($tag eq "rstatic") {
	    if ($vals[0] eq "T") {
		$kvs{"isStatic"} = 1;
	    }
	} elsif ($tag eq "rtempl") {
	    my ($junk, $tid) = split(/\#/,$vals[0]);
	    $kvs{"templID"} = $tid;
	} elsif ($tag eq "rstmt") {
	    ## TODO: ignoring
	} elsif ($tag eq "rcall") {
	    ## TODO: done
		my %cgkv;
                $cgkv{"srcroID"}=$id;
                my($junk,$did) = split(/\#/,$vals[0]);
                $cgkv{"destroID"}=$did;
                $cgkv{"virt"}="\'$vals[1]\'";
                my($junk,$sid) = split(/\#/,$vals[2]);
                $cgkv{"sourceID"}=$sid;
                $cgkv{"line"}=$vals[3];
                $cgkv{"col"}=$vals[4];
                #create the query here
                &kvpairs_to_sql("CALLGRAPH",%cgkv);

	} elsif ($tag eq "rbody") {
	    ($junk, $kvs{"bodyID"}) = split(/\#/, $vals[0]);
	} else {
	    die "ROUTINE TAG UNSUPPORTED : $tag\n";
	}
	
	$line = &read_line;
    }

    &kvpairs_to_sql("ROUTINES",%kvs);
}

##
## main program loop that invokes the appropriate handler for each PDB
## entry.
##
while (!eof(PDBFILE)) {
    my ($entity,$rest) = split(/\#/,&read_line);

    if ($entity eq "so") {
	##
	## source files
	##
	my ($idnum,$fname) = split(/ /,$rest);
	&so_handler($idnum,$fname);
    } elsif ($entity eq "ro") {
	##
	## routines
	## 
	my ($idnum, $roname) = split(/ /,$rest);
	&ro_handler($idnum, $roname);
    } elsif ($entity eq "ty") {
	## 
	## types
	##
	my ($idnum,@tname) = split(/ /,$rest);
	&ty_handler($idnum, @tname);
    } elsif ($entity eq "gr") {
	##
	## groups
	##
	my ($idnum,@gname) = split(/ /,$rest);
	&gr_handler($idnum, @gname);
    } elsif ($entity eq "ma") {
	##
	## macros
	##
	my ($idnum,$mname) = split(/ /,$rest);
	&ma_handler($idnum, $mname);
    } elsif ($entity eq "na") {
	##
	## namespaces
	##
	my ($idnum, $nname) = split(/ /,$rest);
	&na_handler($idnum, $nname);
    } elsif ($entity eq "te") {
	## 
	## C++ templates
	##
	my ($idnum, $tename) = split(/ /,$rest);
	&te_handler($idnum, $tename);
    } elsif ($entity eq "pr") {
	##
        ## pragmas.
	## TODO: Add support for pragmas.  Currently we don't use them
	##       in the CCA work, so they're safe to just ignore.
        ##
        my $line = &read_line;
        while (length($line) > 0) {
            $line = &read_line;
        }
    } else {
	# unsupported.
	die "UNSUPPORTED ENTITY TYPE: $entity\n";
    }
}

## close files
close(PDBFILE);


## emit SQL
print "BEGIN TRANSACTION;\n";
foreach my $sql (@sqlseq) {
    print $sql."\n";
}
print "COMMIT;\n";

