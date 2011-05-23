textreplace -std -t bin\@package@.bat

mkdir "%OSGEO4W_STARTMENU%"
xxmklink "%OSGEO4W_STARTMENU%\Quantum GIS (@version@).lnk"       "%OSGEO4W_ROOT%\bin\@package@.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
xxmklink "%ALLUSERSPROFILE%\Desktop\Quantum GIS (@version@).lnk" "%OSGEO4W_ROOT%\bin\@package@.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"

path %PATH%;%OSGEO4W_ROOT%\apps\@package@\bin
call "%OSGEO4W_ROOT%"\bin\o4w_env.bat
start "CRS synchronization" /wait %OSGEO4W_ROOT%\apps\@package@\crssync
