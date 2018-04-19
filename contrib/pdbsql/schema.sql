-- =========================================================================
--
-- PDB--->SQLite schema
-- Version 0.1
-- matt@cs.uoregon.edu
--
-- Notes:
--   * this is largely based on the contents of converter.txt in v3.11.1
--     of PDToolkit.
--   * although SQLite currently ignores FOREIGN KEY information, I have
--     made sure to indicate it wherever appropriate to make the table
--     relationships clearer.
--
-- =========================================================================

----------------------------------------------------------------------------
-- Table for main entry for each PDB file.
----------------------------------------------------------------------------
CREATE TABLE PDB (
  ID INTEGER PRIMARY KEY,

  version INTEGER,

  -- Treat language as an enumeration with the following values.
  --
  -- LANGUAGE: C=0x01 C++=0x02 C_OR_C++=0x03 Fortran=0x04 Java=0x08
  --           multi=0x0f
  language INTEGER(1)
);

----------------------------------------------------------------------------
-- Table for source files.
----------------------------------------------------------------------------
CREATE TABLE SOURCE (
  ID INTEGER PRIMARY KEY,
  pdbID INTEGER,
  --  FOREIGN KEY(pdbID) REFERENCES PDB(ID),
  filename TEXT,

  -- boolean : 0=false, 1=true
  issys INTEGER(1) DEFAULT 0
);

----------------------------------------------------------------------------
-- Table to deal with include relationships
----------------------------------------------------------------------------
CREATE TABLE SOURCE_INCLUDES (
  includerID INTEGER,
  --  FOREIGN KEY(includerID) REFERENCES SOURCE(ID)
  includedID INTEGER
  --  FOREIGN KEY(includedID) REFERENCES SOURCE(ID)
);

----------------------------------------------------------------------------
-- Table for comments
----------------------------------------------------------------------------
CREATE TABLE COMMENTS (
  ID INTEGER,

  fileID INTEGER,

  --  FOREIGN KEY(fileID) REFERENCES SOURCE(ID),

  -- Treat language as an enumeration with the following values.
  --
  -- LANGUAGE: C=0x01 C++=0x02 C_OR_C++=0x03 Fortran=0x04 Java=0x08
  --           multi=0x0f
  language INTEGER(1),
  
  beginLine INTEGER,
  beginColumn INTEGER,
  endLine INTEGER,
  endColumn INTEGER,

  value TEXT,

  PRIMARY KEY (ID, fileID)

);

----------------------------------------------------------------------------
-- Table of statement-level information
----------------------------------------------------------------------------
CREATE TABLE STATEMENTS (
  ID INTEGER PRIMARY KEY,
  routineID INTEGER,
  --  FOREIGN KEY(routineID) REFERENCES ROUTINES(ID),

  -- valid kind values:
  --   switch=1, case=2, init=3, return=4, if=5, empty=6, for=7, goto=8, 
  --   continue=9, break=10, label=11, block=12, asm=13, expr=14, assign=15, 
  --   throw=16, while=17, do=18, try=19, catch=20, decl=21, 
  --   set_vla_size=22, vla_decl=23, vla_dealloc=24
  --
  --   (Fortran only)
  --
  --   fallocate=25, fassign=26, fio=27, fdo=28, fdeallocate=29, freturn=30, 
  --   fif=31, fsingle_if=32, fgoto=33, fstop=34, flabel=35, fexit=36, 
  --   fcycle=37, farithif=38, fentry=39, fpause=40, flabelassign=41, 
  --   fpointerassign=42, fselect=43, fcase=44, fwhere=45, fforall=46, 
  --   fcall=47
  --
  kind INTEGER(1),

  fileID INTEGER,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  beginLine INTEGER,
  beginColumn INTEGER,
  endLine INTEGER,
  endColumn INTEGER,

  nextStatementID INTEGER,
  downStatementID INTEGER,

  -- valid extra type values:
  --   for_init = 1, else_stmt = 2, catch_stmt = 3, target_stmt = 4,
  --   break_stmt = 5, goto_target = 6, for_stmt = 7
  --
  exraType INTEGER(1) DEFAULT NULL,

  -- statement ID for extra.  optional
  extraID INTEGER DEFAULT NULL
);

---------------------------------------------------------------------------
-- Table for call graph
----------------------------------------------------------------------------
CREATE TABLE CALLGRAPH
(
  -- NOTE: routine IDs below refer to the routine table

  -- source routine: the caller
  srcroID INTEGER,
  -- destination routine: the callee
  destroID INTEGER,

  -- source ID containing the caller
  sourceID  INTEGER,

  --line number of this call
  line INTEGER,
  -- column number of this call
  col INTEGER,

  --flag indicating virtual or not
  virt CHAR(5),

  -- primary key is a tuple of the src/dest ID, the line, column,
  -- and source of the call.  This is to distinguish multiple
  -- callee/caller pairs within a single file.  src/dest are not
  -- sufficient alone to uniquely identify individual call sites.
  PRIMARY KEY (srcroID, destroID, line, col, sourceID)
);



----------------------------------------------------------------------------
-- Table of routines
----------------------------------------------------------------------------
CREATE TABLE ROUTINES (
  ID INTEGER PRIMARY KEY,

  name TEXT,

  fileID INTEGER,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  line INTEGER,
  col INTEGER,

  groupID INTEGER,
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID),

  -- access: valid values are:
  --   pub = 1, prot = 2, priv = 3
  --
  --   NOTE: prot not valid for Fortran code.
  access INTEGER(1),

  -- C++ only
  nspaceID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(nspaceID) REFERENCES NAMESPACES(ID),

  -- used only in F90 or later
  parentRoutineID INTEGER,

  -- alias reference (F90)
  routineAliasID INTEGER,

  sigTypeID INTEGER,
  --  FOREIGN KEY(sigTypeID) REFERENCES TYPES(ID),

  -- linkage: valid values are:
  --   no = 0, internal = 1, C++ = 2, C = 3, fint = 4, f90 = 5
  --
  linkage INTEGER(1),

  -- storage class kind: valid values are:
  --   ext = 1, stat = 2, auto = 3, NA = 4, asm = 5, tproto = 6, fext = 7, 
  --   fprog = 8, fbldat = 9, fintrin = 10, fint = 11, fstfn = 12, 
  --   fmproc = 13, funspec = 14, falias = 15
  --
  kind INTEGER(1),

  -- kind=falias parameter
  implRoutineID INTEGER,

  isStatic INTEGER(1) DEFAULT 0,

  -- valid special kinds are:
  --   ctor = 1, dtor = 2, conv = 3, op = 4
  specialkind INTEGER(1),

  -- virtuality valid values are:
  --   no = 0, virt = 1, pure = 2
  virt INTEGER(1) DEFAULT 0,

  isCrvo INTEGER(1) DEFAULT 0,
  isInline INTEGER(1) DEFAULT 0,
  isCgen INTEGER(1) DEFAULT 0,
  isExpl INTEGER(1) DEFAULT 0,

  -- if template instance, the template ID
  templID INTEGER,
  --  FOREIGN KEY(tempID) REFERENCES TEMPLATES(ID),

  isSpecl INTEGER(1) DEFAULT 0,
  isArginfo INTEGER(1) DEFAULT 0,
  isRec INTEGER(1) DEFAULT 0,
  isElem INTEGER(1) DEFAULT 0,

  firstStmtLine INTEGER DEFAULT NULL,
  firstStmtCol INTEGER DEFAULT NULL,

  -- BIG NOTE: the rret and rstop fields have been eliminated in favor of
  -- using statements with the kind=freturn or kind=fstop.

  -- also note: not including rpos since I can't see how it differs from
  -- the location at the beginning of the table...

  --    rpos related stuff here.
  -- beginning location of return type
  rtypeLine INTEGER,
  rtypeCol INTEGER,
  -- end of last token before {
  ltokLine INTEGER,
  ltokCol INTEGER,

  -- end of routine at }
  endLine INTEGER,
  endCol INTEGER,

  bodyID INTEGER
  --  FOREIGN KEY(bodyID) REFERENCES STATEMENTS(ID)
);

----------------------------------------------------------------------------
-- Table for groups
----------------------------------------------------------------------------
CREATE TABLE GROUPS (
  ID INTEGER PRIMARY KEY,

  name TEXT,
  
  fileID INTEGER,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  line INTEGER,
  col INTEGER,

  parentID INTEGER DEFAULT NULL,

  -- access: valid values:
  --   pub = 1, prot = 2, priv = 3
  access INTEGER(1),

  nspaceID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(nspaceID) REFERENCES NAMESPACES(ID),

  -- kind, valid values:
  --   class = 1, struct = 2, union = 3, tproto = 4, fderived = 5,
  --   fmodule = 6
  --
  kind INTEGER(1),

  templID INTEGER,
  --  FOREIGN KEY(templID) REFERENCES TEMPLATES(ID),

  isSpecl INTEGER(1) DEFAULT 0
);

CREATE TABLE GROUP_FUNCTIONS (
  -- table subsumes role of gfrfunc and gfunc elementsrm 
  ID INTEGER PRIMARY KEY,
  routineID INTEGER,
  --  FOREIGN KEY(routineID) REFERENCES ROUTINES(ID),
  isFriend INTEGER(1) DEFAULT 0,
  groupID INTEGER
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID)
  -- location inherited from routine table
);

CREATE TABLE GROUP_INHERITANCE (
	ID INTEGER,
	DirectBaseID INTEGER,
	isVirtual INTEGER(1) DEFAULT 0,
	access INTEGER(2) DEFAULT 0,
	fileID INTEGER,
	line INTEGER,
	col INTEGER
);

CREATE TABLE GROUP_HIERARCHY (
  childID INTEGER,
  --  FOREIGN KEY(childID) REFERENCES GROUPS(ID),
  parentID INTEGER
  --  FOREIGN KEY(parentID) REFERENCES GROUPS(ID),
);

CREATE TABLE GROUP_FRIENDS (
  -- ID of the group being befriended
  grID INTEGER,

  -- ID of the friend
  friendID INTEGER,

  -- flag whether or not to interpret the friendID as a group
  -- ID or routine ID
  isFriendGroup INTEGER(1) DEFAULT 0

  -- currently ignoring fileID/col/line for friends
);

CREATE TABLE GROUP_PARAMS (
  grID INTEGER,
  isSpecialization INTEGER(1) DEFAULT 0,

  type TEXT,

  isType INTEGER(1) DEFAULT 0,
  -- isType = 0
  typeID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(typeID) REFERENCES TYPES(ID),
  -- isType = 1
  groupID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID),

  -- template arguments.  first one is... weird
--  ntype TEXT,
  constant TEXT,

  -- second is also weird
--  templ TEXT,
  templID INTEGER
  --  FOREIGN KEY(templID) REFERENCES TEMPLATES(ID)
);

CREATE TABLE GROUP_MEMBERS (
  ID INTEGER PRIMARY KEY,
  name TEXT,

  -- note: assume file is inherited from parent group, so we
  --       only need line and column information.  
  line INTEGER,
  col  INTEGER,

  parentGroupID INTEGER,
  --  FOREIGN KEY(parentGroupID) REFERENCES GROUPS(ID),

  -- access: valid values:
  --   pub = 1, prot = 2, priv = 3
  access INTEGER(1),

  -- kind: valid values:
  --   type = 1, statvar = 2, var = 3, templ = 4
  kind INTEGER(1),

  -- type of member (valid for kind=type,statvar, and var)
  isType INTEGER(1) DEFAULT 0,

  -- isType=true
  typeID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(typeID) REFERENCES TYPES(ID),
  -- isType = false
  groupID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID),

  -- statvar and templ kinds have templID
  templID INTEGER,
  --  FOREIGN KEY(templID) REFERENCES TEMPLATES(ID),
  
  -- for statvar
  isSpecl INTEGER(1) DEFAULT 0,
  isConst INTEGER(1) DEFAULT 0,

  -- for var
  isBit INTEGER(1) DEFAULT 0,
  isMut INTEGER(1) DEFAULT 0
  -- isConst used for statvar and var

  -- pos?  not currently here.
);

----------------------------------------------------------------------------
-- Table for types
----------------------------------------------------------------------------
CREATE TABLE TYPES (
  ID INTEGER PRIMARY KEY,

  name TEXT,

  fileID INTEGER,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  line INTEGER,
  col INTEGER,  

  groupID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID),

  -- access: valid values:
  --   pub=1, prot=2, priv=3
  --   NOTE: prot not valid for Fortran
  access INTEGER(1),

  nspaceID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(nspaceID) REFERENCES NAMESPACES(ID),

  -- kind: valid values:
  --   NA=0,err=1,void=2,int=3,float=4,ptr=5,ref=6,func=7,array=8,tref=9,
  --   ptrmem=10,tparam=11,enum=12,wchar=13,bool=14,ferr=15,fvoid=16,
  --   fint=17,flogic=18,ffloat=19,ffunc=20,fchar=21,farray=22,
  --   fcmplx=23,funspecfunc=24,fbldat=25,fmod=26,fptr=27
  kind INTEGER(1),

  -- kind = int,enum,wchar,bool,fint,flogic,fchar
  -- intKind: valid values:
  --   char=1,schar=2,uchar=3,wchar=4,short=5,ushort=6,int=7,uint=8,
  --   long=9,ulong=10,longlong=11,ulonglong=12
  intKind INTEGER(1),
  isSigned INTEGER(1) DEFAULT 0,

  -- fchar: NOTE: NULL is used in place of * to mean unspecified.
  clen INTEGER DEFAULT NULL,

  -- kind = float,ffloat,fcmplx
  -- fKind: valid values:
  --   float=1,dbl=2,longdbl=3
  fKind INTEGER(1),

  -- ptr, fptr, ref
  indirectIsType INTEGER(1) DEFAULT 0,
  indirectTypeID INTEGER,
  --  FOREIGN KEY(indirectTypeID) REFERENCES TYPES(ID),
  indirectGroupID INTEGER,
  --  FOREIGN KEY(indirectGroupID) REFERENCES GROUPS(ID),

  -- func, ffunc
  rettIsType INTEGER(1) DEFAULT 0,
  rettTypeID INTEGER,
  --  FOREIGN KEY(rettTypeId) REFERENCES TYPES(ID),
  rettGroupID INTEGER,
  --  FOREIGN KEY(rettGroupId) REFERENCES GROUPS(ID),
  
  hasEllip INTEGER(1) DEFAULT 0,
  qualifier TEXT,
  
  -- array
  elementIsType INTEGER(1) DEFAULT 0,
  elementTypeID INTEGER,
  --  FOREIGN KEY (elementTypeID) REFERENCES TYPES(ID),
  elementGroupID INTEGER,
  --  FOREIGN KEY (elementGroupID) REFERENCES GROUPS(ID),
  isStatic INTEGER(1) DEFAULT 0,
  nelem INTEGER,

  -- farray
  -- elementType from array reused

  -- shape valid values:
  --   explicit=1,asmdsize=2,asmdshape=3,deferred=4
  shape INTEGER(1),
  rank INTEGER(1),

  -- tref
  trefIsType INTEGER(1) DEFAULT 0,
  trefTypeID INTEGER,
  --  FOREIGN KEY (trefTypeID) REFERENCES TYPES(ID),
  trefGroupID INTEGER,
  --  FOREIGN KEY (trefGroupID) REFERENCES GROUPS(ID),
  -- qualifier valid values
  --   const=0x01,volatile=0x02,restrict=0x04
  --   combinations can be OR'd together
  qualifiers INTEGER(1),

  -- ptrmem
  mpgroup INTEGER,
  --  FOREIGN KEY(mpgroup) REFERENCES GROUPS(ID),
  mpIsType INTEGER(1) DEFAULT 0,
  mpTypeID INTEGER,
  --  FOREIGN KEY (mpTypeID) REFERENCES TYPES(ID),
  mpGroupID INTEGER
  --  FOREIGN KEY (mpGroupID) REFERENCES GROUPS(ID)
  
);

CREATE TABLE TYPES_ARGS (
  itemID INTEGER,

  name TEXT,

  position INTEGER,

  --  FOREIGN KEY (itemID) REFERENCES TYPES(ID),
  isType INTEGER(1) DEFAULT 0,
  typeID INTEGER,
  --  FOREIGN KEY (typeID) REFERENCES TYPES(ID),
  groupID INTEGER,
  --  FOREIGN KEY (groupID) REFERENCES GROUPS(ID),
  -- valid intent values:
  --   def=0,in=1,out=2,opt=3
  intent INTEGER(1),

  -- note: location inherits file from parent item.
  --       just need to store more precise line, column info here.
  line INTEGER,
  col INTEGER
);

CREATE TABLE TYPE_FDIMS (
  itemID INTEGER,
  --  FOREIGN KEY (itemID) REFERENCES TYPES(ID),
  lower INTEGER,
  upper INTEGER  
);

CREATE TABLE TYPE_EXCEPTIONS (
  itemID INTEGER,
  --  FOREIGN KEY (itemID) REFERENCES TYPES(ID),
  isType INTEGER(1),
  typeID INTEGER,
  --  FOREIGN KEY (typeID) REFERENCES TYPES(ID),
  groupID INTEGER
  --  FOREIGN KEY (groupID) REFERENCES GROUPS(ID),
);

CREATE TABLE TYPES_ENUMS (
  itemID INTEGER,
  --  FOREIGN KEY (itemID) REFERENCES TYPES(ID),
  name TEXT,
  value TEXT
);

CREATE TABLE TEMPLATES (
  ID INTEGER PRIMARY KEY,

  name TEXT,

  fileID INTEGER,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  line INTEGER,
  col INTEGER,

  groupID INTEGER,
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID),
  -- valid access values
  --   pub=1,prot=2,priv=3
  access INTEGER,

  nspaceID INTEGER,
  --  FOREIGN KEY(nspaceID) REFERENCES NAMESPACES(ID),
  declID INTEGER, -- template ID
  defID INTEGER, -- template ID

  -- valid kind values
  --   none=0,class=1,func=2,memfunc=3,statmem=4,memclass=5,
  --   ttparam=6
  kind INTEGER(1),

  -- kind=func or memfunc
  protoRoutineID INTEGER,
  --  FOREIGN KEY(protoRoutineID) REFERENCES ROUTINES(ID),

  -- kind=class or memclass
  protoGroupID INTEGER,
  --  FOREIGN KEY(protoGroupID) REFERENCES GROUPS(ID),

  -- kind=statmem
  isStatmemType INTEGER(1) DEFAULT 0,
  statmemTypeID INTEGER,
  --  FOREIGN KEY (statmemTypeID) REFERENCES TYPES(ID),
  statmemGroupID INTEGER,
  --  FOREIGN KEY (statmemGroupID) REFERENCES GROUPS(ID),  

  text TEXT
);

CREATE TABLE TEMPLATE_PARAMS (
  templateID INTEGER,
  --  FOREIGN KEY(templateID) references TEMPLATES(ID),

  -- to merge param and sparam into one table
  partialSpec INTEGER(1) DEFAULT 0,

  type TEXT,

  -- for type = type
  -- note: typeID shared with ntype too
  typeIsGroup INTEGER(1) DEFAULT 0,
  typeID INTEGER DEFAULT NULL,
  groupID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(typeID) REFERENCES TYPES(ID),
  defaultIsGroup INTEGER(1) DEFAULT 0,
  defTypeID INTEGER DEFAULT NULL,
  defGroupID INTEGER DEFAULT NULL,

  -- for type = ntype
  name TEXT DEFAULT NULL,
  defaultValue TEXT DEFAULT NULL,

  -- for type = templ
  templID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(templID) REFERENCES TEMPLATES(ID),
  defaultTemplID INTEGER DEFAULT NULL
  --  FOREIGN KEY(defaultTemplID) REFERENCES TEMPLATES(ID)
);

CREATE TABLE PRAGMAS (
  ID INTEGER PRIMARY KEY,
  name TEXT,

  fileID INTEGER,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),

  kind TEXT,

  beginLine INTEGER,
  beginCol INTEGER,
  endLine INTEGER,
  endCol INTEGER,

  ptext TEXT
);

CREATE TABLE NAMESPACES (
  ID INTEGER PRIMARY KEY,

  name TEXT NOT NULL,

  fileID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  line INTEGER DEFAULT NULL, 
  col INTEGER DEFAULT NULL,

  parentID INTEGER DEFAULT NULL,
  aliasName TEXT DEFAULT NULL
);

CREATE TABLE NAMESPACE_MEMBERS (
  nspaceID INTEGER,
  --  FOREIGN KEY(nspaceID) REFERENCES NAMESPACES(ID),
  -- type valid values:
  --   type=1,routine=2,group=3,template=4,namespace=5
  type INTEGER(2),

  typeID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(typeID) REFERENCES TYPES(ID),
  routineID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(routineID) REFERENCES ROUTINES(ID),
  groupID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(groupID) REFERENCES GROUPS(ID),
  templateID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(templateID) REFERENCES TEMPLATES(ID),
  namespaceID INTEGER DEFAULT NULL
  --  FOREIGN KEY(namespaceID) REFERENCES NAMESPACES(ID)
);

CREATE TABLE MACROS (
  ID INTEGER PRIMARY KEY,
  name TEXT NOT NULL,

  -- these get null defaults since some macros are compiler macros with
  -- no real home.
  fileID INTEGER DEFAULT NULL,
  --  FOREIGN KEY(fileID) REFERENCES SOURCES(ID),
  line INTEGER DEFAULT NULL, 
  col INTEGER DEFAULT NULL,

  -- def or undef : def = 1, undef = 2
  kind INTEGER(1) NOT NULL,
  text TEXT NOT NULL
);
