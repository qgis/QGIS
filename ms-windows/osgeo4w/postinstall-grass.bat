textreplace -std -t bin\@package@-grass@grassmajor@.bat

if not defined OSGEO4W_DESKTOP for /F "tokens=* USEBACKQ" %%F IN (`getspecialfolder Desktop`) do set OSGEO4W_DESKTOP=%%F

copy bin\@package@-bin.exe bin\@package@-bin-g@grassmajor@.exe
copy bin\@package@-bin.vars bin\@package@-bin-g@grassmajor@.vars
call "%OSGEO4W_ROOT%\bin\@package@-grass@grassmajor@.bat" --postinstall

if not %OSGEO4W_MENU_LINKS%==0 if not exist "%OSGEO4W_STARTMENU%" mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_DESKTOP_LINKS%==0 if not exist "%OSGEO4W_DESKTOP%" mkdir "%OSGEO4W_DESKTOP%"

if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\QGIS Desktop @version@ with GRASS @grassversion@.lnk" "%OSGEO4W_ROOT%\bin\@package@-bin-g@grassmajor@.exe" "" "%DOCUMENTS%"
if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%OSGEO4W_DESKTOP%\QGIS Desktop @version@ with GRASS @grassversion@.lnk" "%OSGEO4W_ROOT%\bin\@package@-bin-g@grassmajor@.exe" "" "%DOCUMENTS%"
