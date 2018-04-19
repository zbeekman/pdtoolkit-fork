PDToolkit AKA PDT
=================

The purpose of this fork is to track work & patches that can be
submitted upstream since I do not have access to the [UO CS dept.]
[CVS] server. Even if I had access to the server, I would rather do
many unpleasant things rather than trying to remind myself how to use
[CVS].


N.B.!!!
-------

__*USE THIS VERSIO OF PDT AT YOUR OWN RISK!*__

This is a fork of [University of Oregon] Performance Research Lab's
([PRL]) PDToolkit found at
https://www.cs.uoregon.edu/research/pdt/home.php

__I have stripped out the [EDG] closed source components.__ Therfore
this is not a fully functional version of PDT, and you should obtain
an [official version].

Some of the parsers are derived from GCC, which are licensed under the
[GPL] license. For compliance with the GCC [GPL] license, here are
links to the GCC sources:

* [gfortran-pdt.tar.gz]
* [gfortran48-pdt.tar.gz]


What's the point?
-----------------

This repo is for my (mostly reactionary) hacks to PDT and it's
components as they relate to parsing Fortran codes and allowing [TAU]
to insert automatically generated source based instrumentation. There
are some quirks/bugs that I have discovered as to how [PDT] works with
Fortran code `>=` Fortran 90. (Mostly having to do with---you geussed
it---everyone's favorite compilation-order-dependency-inducing,
compiler-specific `.mod` files.)

---

<div align="center">

![Not the repo you want][obi-wan]
</div>


[PDT]: https://www.cs.uoregon.edu/research/pdt/home.php
[UO CS dept.]: https://cs.uoregon.edu/
[University of Oregon]: https://www.uoregon.edu/
[PRL]: https://nic.uoregon.edu/prl/home.php
[CVS]: https://www.nongnu.org/cvs/
[EDG]: http://www.edg.com/
[official version]: http://tau.uoregon.edu/pdt.tgz
[GPL]: https://opensource.org/licenses/gpl-license
[gfortran-pdt.tar.gz]: http://www.cs.uoregon.edu/research/paracomp/pdtoolkit/Download/gfortran-pdt.tar.gz
[gfortran48-pdt.tar.gz]: http://www.cs.uoregon.edu/research/paracomp/pdtoolkit/Download/gfortran48-pdt.tar.gz
[obi-wan]: https://camo.githubusercontent.com/62df93030477a40e0c221a8e5f4aec15dab445ad/687474703a2f2f692e696d6775722e636f6d2f516347784b43562e6a7067
