textreplace -std -t bin\qgis-dev.bat
textreplace -std -t apps\qgis-dev\python\qgis\qgisconfig.py

mkdir "%OSGEO4W_STARTMENU%"
xxmklink "%OSGEO4W_STARTMENU%\Quantum GIS (trunk).lnk"       "%OSGEO4W_ROOT%\bin\qgis-dev.bat" " " \ "Quantum GIS - Desktop GIS (trunk)" 1 "%OSGEO4W_ROOT%\apps\qgis-dev\icons\QGIS.ico"
xxmklink "%ALLUSERSPROFILE%\Desktop\Quantum GIS (trunk).lnk" "%OSGEO4W_ROOT%\bin\qgis-dev.bat" " " \ "Quantum GIS - Desktop GIS (trunk)" 1 "%OSGEO4W_ROOT%\apps\qgis-dev\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\qgis-dev\bin\qgis-dev.reg"
"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\qgis-dev\bin\qgis-dev.reg"
