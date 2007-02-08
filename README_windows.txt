       Procedure for setup of a windows build environment for QGIS
                    Tim Sutton and Godofredo Contreras 2006
                    CMake additions Magnus Homann 2007
  With thanks to Tisham Dhar for preparing the initial msys environment
---------------------------------------------------------------------------------

This document describes the process you need to follow to get QGIS built directly
under windows (rather than using a cross compiler under linux).

Note: The process for building under windows is still being sorted out so
      check this document regularly for updates.

Msys:
-----
get this: 

http://qgis.org/uploadfiles/msys/msys.tar.bz2

and unpack to c:\msys

The file is compressed using bzip2 - you can get a free windows application
for creating and decompressing files here:

http://www.7-zip.org/

Qt4.2:
------

Download qt4.2 opensource precompiled edition exe and install (including the 
download and install of mingw) from here:

http://www.trolltech.com/developer/downloads/qt/windows

Edit C:\Qt\4.2.0\bin\qtvars.bat and add the following line (the second is only 
needed if you like vim in your shell):

set PATH=%PATH%;C:\msys\local\bin;c:\msys\local\lib
set PATH=%PATH%;"c:\Program Files\Vim\vim70\

If you plan to do some debugging, you'll need to complie debug version of Qt:
C:\Qt\4.2.0\bin\qtvars.bat compile_debug


QGIS:
-----
Check out to c:\dev\cpp\qgis:

cd c:\dev\cpp
svn co https://svn.qgis.org/repos/qgis/trunk/qgis

Currently you need to put it into that location in case you plan to create
the NSIS installer because so far it contains hardcoded paths for this location.


USING CMAKE :
-----------

Instead of shifting around files, you could use CMake. CMake compiles
raster, compaser, legend, gui and core libraries into one core
library. So it is not 100% compatible with 'normal' 0.8 Makefiles.

Below are the steps to configure and make the source. The building
takes plac in a separate directory from the source. If you have built
the source with 'normal' Makefiels first, please do a make clean (or
remove and check out everything). Previoussly made intermediate files
can disturb the CMake process.

As a background read http://wiki.qgis.org/qgiswiki/Building_with_CMake

*) Make sure %QTDIR%\bin;c:\msys\local\bin;c:\msys\bin;c;\msys\mingw\bin is in your Path

*) Start a cmd.exe window ( Start -> Run -> cmd.exe ) if you don't have one already.

*) > mkdir build

*) > cd build

*) > cmakesetup ..

   If asked, you should chose 'MSYS Makefiles' as generator.

   All dependencies should be picked up automatically, if you have set
   up the Paths correctly. The only thing you need to change is the
   installation destination and/or set 'Debug'.


*) Now, start sh.exe and run 'make.exe install' from within that shell

   It should now start compiling. The reason for this is that we use
   the mingw compiler included in the msys tar, but that compiler is
   not found from within sh.exe. (MSYS magic). So, if you delete
   CMakeCache, you have to generate it from cmd.exe.

   Why not run make from cmd.exe? Because creating 'qgssvnversion.h'
   requires the 'mv' command...

*) Make sure to copy all .dll:s needed to the same directory as the
   qgis.exe binary is installed to, if not already done so.

Create the installation package: (optional)
--------------------------------

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

