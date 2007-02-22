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

