 Notes for using NSIS installer scripts.

             Tim Sutton 2006
---------------------------------------

The installer script has an optional data
download part. This requires an NSIS 
plugin available here:

http://nsis.sourceforge.net/ZipDLL
or
http://nsis.sourceforge.net/ZipDLL_plug-in

Follow the instructions on that page before
attempting to run the installer.

NOTE: the zipdll.nsh at the download was 
broken at time of writing this. Use the 
one provided in this directory instead
(still copy it to your nsis include dir 
though).

The NSIS installer now expects the qgis 
binaries to be packaged to exist in 
c:\Program Files\qgis<version> (the old
c:\dev\cpp\qgis\qgs-release path is deprecated)

Set the PRODUCT_VERSION_NUMBER in qgis.nsis 
before running. This will allow having multiple 
development versions on the same machine and 
being able to package them without issue.

Unattended Install:
===================

The QGIS installer supports silent installation now. 
Run from the command line like this

qgis_setup0.8.1.exe /S

There are a few things to note:
-  after running the above command the installer will 
   fork to the background and  immediately return 
   you to the prompt.
- the installer uses all default options that means 
  that it will try to go onto the internet and 
  retrieve the two sample datasets of around 20mb each.
- if you wish to customise the datasets etc that are 
  installed, you can quite easily modify the nsis 
  installer - its in svn 0.8 branch under win_build.

Customising the installer:
==========================

The procedure for creating your custom installer is 
(more or less)

- Install qgis 0.8.1 preview (and later final 
  release) into c:\Program Files\qgis0.8.1
- Install nulsoft NSIS installer application
- Check out the win_build dir from qgis 0.8 branch:

  https://svn.qgis.org/repos/qgis/branches/Release-0_8_0/win_build

- Follow the notes pertaining to fixing auto unzip 
  issues in the source in higher up in this document
- edit qgis.nsi and add remove sample data etc.
- Run nsis on the nsi file
- Feel good and be happy :-)

