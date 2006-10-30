       Procedure for setup of a windows build environment for QGIS
                    Tim Sutton and Godofredo Contreras 2005
  With thanks to Tisham Dhar for preparing the initial msys environment
---------------------------------------------------------------------------------

This document describes the process you need to follow to get QGIS built directly
under windows (rather than using a cross compiler under linux).

Note: The process for building under windows is still being sorted out so
      check this document regularly for updates.

Msys:
-------------
get this : 

http://qgis.org/uploadfiles/mingw/msys.zip 

and unpack to c:\msys

The file is compressed using zip - you can get a free windows application for creating and decompressing zip files here:

http://www.filzip.com/

Qt4.2:
-------------
Download qt4.2 opensource precompiled edition exe and install (including the 
download and install of mingw).

Edit C:\Qt\4.2.0\bin\qtvars.bat and add the following line (the second is only 
needed if you like vim in your shell):

set PATH=%PATH%;C:\msys\local\bin
set PATH=%PATH%;"c:\Program Files\Vim\vim70\


NSIS:
-------------
Downlad and install NSIS from (http://nsis.sourceforge.net/Main_Page)

GDB:
-------------
Download and install gdb-6.3.2.exe from 

http://sourceforge.net/project/showfiles.php?group_id=2435 

and install into c:\mingw


QGIS:
-------------
Check out to c:\dev\cpp\qgis
Currently you need to put it into that location I think - if you try to build
somewhere else you *may* run into problems - particularly with the NSIS installer 
as I havent checked all paths are relative yet.

Next you need to shift some files around. Note I expect this requirement to go away
once the windows build process is refined a bit.


Core:
-------------
 - moved qgsspatialrefsys* and qgscoordinatetransform* into core dir from gui
 - qgsspatialrefsys.cpp - commented out lines using qgsproject as it depends on gui stuff
 - qgsdistancearea.cpp - commented out lines using qgsproject as it depends on 
   gui stuff and moved to core
 - qgscsexception.h  into core from gui (only tested on mac)

I have created a small archive which includes the above changes, which I will make available.

Gui:
-------------
 Temporarily disable postgres support until we heve resolved issues
 - added #undef HAVE_POSTGRESQL to qgisapp.cpp around line 144
 - added #undef HAVE_POSTGRESQL to qgsvectorlayerproperties.cpp around line 32

 - moved qgspluginregistry.* to gui from core

Building:
-------------

Now open the qt command shell

cd c:\dev\cpp\qgis
qmake
make


Create the installation package:
---------------------------------

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
