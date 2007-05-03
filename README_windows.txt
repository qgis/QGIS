       Procedure for setup of a windows build environment for QGIS
                    Tim Sutton and Godofredo Contreras 2006
                    CMake additions Magnus Homann 2007
  With thanks to Tisham Dhar for preparing the initial msys environment
---------------------------------------------------------------------------------

This document describes the process you need to follow to get QGIS built directly
under windows (rather than using a cross compiler under linux).

Note: The process for building under windows is still being sorted out so
      check this document regularly for updates.

MSYS:
-----
get this: 

http://qgis.org/uploadfiles/msys/msys.tar.gz

and unpack to c:\msys

The file is compressed as gzipped tarball - you can get a free windows application
for creating and decompressing files here:

http://www.7-zip.org/


Qt4.2:
------

Download qt4.2 opensource precompiled edition exe and install (including the 
download and install of mingw) from here:

http://www.trolltech.com/developer/downloads/qt/windows

Edit C:\Qt\4.2.3\bin\qtvars.bat and add the following line:

set PATH=%PATH%;C:\msys\local\bin;c:\msys\local\lib

If you plan to do some debugging, you'll need to compile debug version of Qt:
C:\Qt\4.2.3\bin\qtvars.bat compile_debug


Python stuff: (optional)
-------------

Follow this section in case you would like to use Python bindings for QGIS.

*) Download and install Python - use Windows installer
   (It doesn't matter to what folder you'll install it)

   http://python.org/download/

*) Download and install PyQt4 - use binary package for Windows
   (Binary package includes also SIP - no need to compile it manually)

   http://riverbankcomputing.co.uk/pyqt/download.php


QGIS:
-----

*) Start a cmd.exe window ( Start -> Run -> cmd.exe )

*) Create development directory and move into it

   > md c:\dev\cpp
   > cd c:\dev\cpp
   
*) Check out sources from SVN

   > svn co https://svn.qgis.org/repos/qgis/trunk/qgis

Currently you need to put it into that location in case you plan to create
the NSIS installer because so far it contains hardcoded paths for this location.


Compiling:
----------

As a background read http://wiki.qgis.org/qgiswiki/Building_with_CMake

*) Start a cmd.exe window ( Start -> Run -> cmd.exe ) if you don't have one already.

*) Add paths to compiler and our MSYS environment:
   
   > c:\Qt\4.2.3\bin\qtvars.bat

*) Create build directory and set it as current directory:

   > cd c:\dev\cpp\qgis
   > md build
   > cd build

*) Configuration

   > cmakesetup ..

   Click 'Configure' button.
   When asked, you should choose 'MinGW Makefiles' as generator.
   
   There's a problem with MinGW Makefiles on Win2K. If you're compiling on this
   platform, use 'MSYS Makefiles' generator instead.

   All dependencies should be picked up automatically, if you have set
   up the Paths correctly. The only thing you need to change is the
   installation destination (CMAKE_INSTALL_PREFIX) and/or set 'Debug'.

   When configuration is done, click 'OK' to exit the setup utility.

*) Compilation and installation
 
   > make
   > make install

*) Run qgis.exe from the directory where it's installed (CMAKE_INSTALL_PREFIX)

   Make sure to copy all .dll:s needed to the same directory as the
   qgis.exe binary is installed to, if not already done so, otherwise
   QGIS will complain about missing libraries when started.
   
   Other possibility is to run qgis.exe when your path contains
   c:\msys\local\bin and c:\msys\local\lib directories, so the DLLs
   will be used from that place.


Create the installation package: (optional)
--------------------------------

[ TO BE UPDATED FOR 0.9 RELEASE ]

Downlad and install NSIS from (http://nsis.sourceforge.net/Main_Page)

Download both the QGIS debug and release installer packages from 

http://qgis.org/uploadfiles/testbuilds/

and install them. Now copy the installation dirs from C:\Program Files\QGIS* into 
c:\dev\cpp\qgis\qgis-debug and c:\dev\cpp\qgis\qgis-release respectively. After
making these copies uninstall the release and debug versions of QGIS from 
your c:\Program Files directories using the provided uninstaller. Double check
that both dirs are complete gone under program files afterwards.

Now using windows explorer, enter the c:\dev\cpp\qgis\win_build directory and right 
click on qgis.nsi and choose the option 'Compile NSIS Script'. Do the same 
for qgis-debug.nsi. Congratulations you should have two installable qgis 
setup files in the win_build directory now..

