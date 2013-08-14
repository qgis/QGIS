textreplace -std -t bin\@package@.bat
textreplace -std -t bin\@package@-browser.bat

REM get short path without blanks
for %%i in ("%OSGEO4W_ROOT%") do set O4W_ROOT=%%~fsi

if not %OSGEO4W_MENU_LINKS%==0 mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\QGIS Desktop (@version@).lnk"       "%O4W_ROOT%\bin\@package@.bat" " " \ "QGIS - Desktop GIS (@version@)" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\QGIS Browser (@version@).lnk"       "%O4W_ROOT%\bin\@package@-browser.bat" " " \ "QGIS - Desktop GIS (@version@)" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"

if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%ALLUSERSPROFILE%\Desktop\QGIS Desktop (@version@).lnk" "%O4W_ROOT%\bin\@package@.bat" " " \ "QGIS - Desktop GIS (@version@)" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%ALLUSERSPROFILE%\Desktop\QGIS Browser (@version@).lnk" "%O4W_ROOT%\bin\@package@-browser.bat" " " \ "QGIS - Desktop GIS (@version@)" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"

set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
