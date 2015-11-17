
=======================================================================
=                                                                     =
=                                                                     =
=   metapath - The universal Explorer-like Plugin                     =
=                                                                     =
=                                                                     =
=                                                   metapath 4.0.13   =
=                                      (c) Florian Balmer 1996-2011   =
=                                       http://www.flos-freeware.ch   =
=                                                                     =
=                                                                     =
=======================================================================


The metapath Source Code

  This package contains the full source code of metapath 4.0.13 for
  Windows. Project files for Visual C++ 7.0 are included. Chances are
  that metapath can be rebuilt with other development tools, including
  the free Visual C++ Express Edition, but I haven't tested this.


Creating a Compact Executable Program File

  Linking to the system CRT slightly improves disk footprint, memory
  usage and startup because the pages for the system CRT are already
  loaded and shared in memory. To achieve this, the release version of
  metapath.exe is built using the Windows Driver Kit (WDK) 7.1.0 tools,
  available as a free download from Microsoft. The appropriate build
  scripts can be found in the "wdkbuild" subdirectory. Set %WDKBASEDIR%
  to the directory where the WDK tools are located on your system.


Copyright

  See License.txt for details about distribution and modification.

  If you have any comments or questions, please drop me a note:
  florian.balmer@gmail.com

  (c) Florian Balmer 1996-2011
  http://www.flos-freeware.ch

###
