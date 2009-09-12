textreplace -std -t bin\qgis-unstable.bat
textreplace -std -t apps\qgis-unstable\python\qgis\qgisconfig.py

mkdir "%OSGEO4W_STARTMENU%"
xxmklink "%OSGEO4W_STARTMENU%\Quantum GIS (1.3).lnk"       "%OSGEO4W_ROOT%\bin\qgis-unstable.bat" " " \ "Quantum GIS - Desktop GIS (1.3)" 1 "%OSGEO4W_ROOT%\apps\qgis-unstable\icons\QGIS.ico"
xxmklink "%ALLUSERSPROFILE%\Desktop\Quantum GIS (1.3).lnk" "%OSGEO4W_ROOT%\bin\qgis-unstable.bat" " " \ "Quantum GIS - Desktop GIS (1.3)" 1 "%OSGEO4W_ROOT%\apps\qgis-unstable\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\qgis-unstable\bin\qgis-unstable.reg"
"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\qgis-unstable\bin\qgis-unstable.reg"
