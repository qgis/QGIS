textreplace -std -t bin\@package@.bat
textreplace -std -t bin\@package@-browser.bat

mkdir "%OSGEO4W_STARTMENU%"
xxmklink "%OSGEO4W_STARTMENU%\Quantum GIS Desktop (@version@).lnk"       "%OSGEO4W_ROOT%\bin\@package@.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
xxmklink "%ALLUSERSPROFILE%\Desktop\Quantum GIS Desktop (@version@).lnk" "%OSGEO4W_ROOT%\bin\@package@.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
xxmklink "%OSGEO4W_STARTMENU%\Quantum GIS Browser (@version@).lnk"       "%OSGEO4W_ROOT%\bin\@package@-browser.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
xxmklink "%ALLUSERSPROFILE%\Desktop\Quantum GIS Browser (@version@).lnk" "%OSGEO4W_ROOT%\bin\@package@-browser.bat" " " \ "Quantum GIS - Desktop GIS (@version@)" 1 "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
set OSGEO4W_ROOT=%O4W_ROOT%

"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"

call "%OSGEO4W_ROOT%"\bin\o4w_env.bat
path %PATH%;%OSGEO4W_ROOT%\apps\@package@\bin
set QGIS_PREFIX_PATH=%OSGEO4W_ROOT:\=/%/apps/@package@
%OSGEO4W_ROOT%\apps\@package@\crssync
