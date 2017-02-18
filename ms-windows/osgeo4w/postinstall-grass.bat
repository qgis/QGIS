textreplace -std -t bin\@package@-grass@grassmajor@.bat
textreplace -std -t bin\@package@-browser-grass@grassmajor@.bat

if "%OSGEO4W_DESKTOP%"=="" set OSGEO4W_DESKTOP=~$folder.common_desktop$

copy bin\@package@-bin.exe bin\@package@-bin-grass@grassmajor@.exe
copy bin\@package@-bin.vars bin\@package@-bin-grass@grassmajor@.vars
"%OSGEO4W_ROOT%\bin\@package@-grass@grassmajor@.bat" --exit

if not %OSGEO4W_MENU_LINKS%==0 mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_DESKTOP_LINKS%==0 mkdir "%OSGEO4W_DESKTOP%"

if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\@package@-bin-grass@grassmajor@.exe" "%OSGEO4W_STARTMENU%" "QGIS Desktop @version@ with GRASS @grassversion@"
if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\@package@-bin-grass@grassmajor@.exe" "%OSGEO4W_DESKTOP%" "QGIS Desktop @version@ with GRASS @grassversion@"

if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "QGIS Browser @version@ with GRASS @grassversion@" "exec hide %OSGEO4W_ROOT%\bin\@package@-browser-grass@grassmajor@.bat" "%OSGEO4W_ROOT%\apps\@package@\icons\browser.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_DESKTOP%" "QGIS Browser @version@ with GRASS @grassversion@" "exec hide %OSGEO4W_ROOT%\bin\@package@-browser-grass@grassmajor@.bat" "%OSGEO4W_ROOT%\apps\@package@\icons\browser.ico"
