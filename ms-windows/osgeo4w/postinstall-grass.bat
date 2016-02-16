textreplace -std -t bin\@package@-grass@grassmajor@.bat
textreplace -std -t bin\@package@-browser-grass@grassmajor@.bat

if not %OSGEO4W_MENU_LINKS%==0 mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "QGIS Desktop @version@ with GRASS @grassversion@" "exec hide %OSGEO4W_ROOT%\bin\@package@-grass@grassmajor@.bat" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "QGIS Browser @version@ with GRASS @grassversion@" "exec hide %OSGEO4W_ROOT%\bin\@package@-browser-grass@grassmajor@.bat" "%OSGEO4W_ROOT%\apps\@package@\icons\browser.ico"

if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "~$folder.desktop$" "QGIS Desktop @version@ with GRASS @grassversion@" "exec hide %OSGEO4W_ROOT%\bin\@package@-grass@grassmajor@.bat" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "~$folder.desktop$" "QGIS Browser @version@ with GRASS @grassversion@" "exec hide %OSGEO4W_ROOT%\bin\@package@-browser-grass@grassmajor@.bat" "%OSGEO4W_ROOT%\apps\@package@\icons\browser.ico"
