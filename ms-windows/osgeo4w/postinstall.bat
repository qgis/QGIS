textreplace -std -t bin\@package@.bat
textreplace -std -t apps\@package@\python\qgis\qgisconfig.py

mkdir "%OSGEO4W_STARTMENU%"
xxmklink "%OSGEO4W_STARTMENU%\Quantum GIS (@version@).lnk"       "%OSGEO4W_ROOT%\bin\@package@.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
xxmklink "%ALLUSERSPROFILE%\Desktop\Quantum GIS (@version@).lnk" "%OSGEO4W_ROOT%\bin\@package@.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
