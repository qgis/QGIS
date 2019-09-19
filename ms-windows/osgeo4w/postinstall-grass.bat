textreplace -std -t bin\@package@-grass@grassmajor@.bat

if "%OSGEO4W_DESKTOP%"=="" set OSGEO4W_DESKTOP=~$folder.common_desktop$

copy bin\@package@-bin.exe bin\@package@-bin-g@grassmajor@.exe
copy bin\@package@-bin.vars bin\@package@-bin-g@grassmajor@.vars
call "%OSGEO4W_ROOT%\bin\@package@-grass@grassmajor@.bat" --postinstall

if not %OSGEO4W_MENU_LINKS%==0 mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_DESKTOP_LINKS%==0 mkdir "%OSGEO4W_DESKTOP%"

if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\@package@-bin-g@grassmajor@.exe" "%OSGEO4W_STARTMENU%" "QGIS Desktop @version@ with GRASS @grassversion@" "" "" "" "" "~$folder.mydocuments$"
if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\@package@-bin-g@grassmajor@.exe" "%OSGEO4W_DESKTOP%" "QGIS Desktop @version@ with GRASS @grassversion@" "" "" "" "" "~$folder.mydocuments$"
