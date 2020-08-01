textreplace -std -t bin\@package@.bat
textreplace -std -t bin\@package@-designer.bat
textreplace -std -t bin\python-@package@.bat
textreplace -std -t bin\qgis_process-@package@.bat

REM get short path without blanks
for %%i in ("%OSGEO4W_ROOT%") do set O4W_ROOT=%%~fsi
if not defined OSGEO4W_DESKTOP for /F "tokens=* USEBACKQ" %%F IN (`getspecialfolder Desktop`) do set OSGEO4W_DESKTOP=%%F
for /F "tokens=* USEBACKQ" %%F IN (`getspecialfolder Documents`) do set DOCUMENTS=%%F

call "%OSGEO4W_ROOT%\bin\@package@.bat" --postinstall

if not %OSGEO4W_MENU_LINKS%==0 if not exist "%OSGEO4W_STARTMENU%" mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_DESKTOP_LINKS%==0 if not exist "%OSGEO4W_DESKTOP%" mkdir "%OSGEO4W_DESKTOP%"

if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\QGIS Desktop @version@.lnk" "%O4W_ROOT%\bin\@package@-bin.exe" "" "%DOCUMENTS%"
if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%OSGEO4W_DESKTOP%\QGIS Desktop @version@.lnk" "%O4W_ROOT%\bin\@package@-bin.exe" "" "%DOCUMENTS%"

if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\Qt Designer with QGIS @version@ custom widgets" "%OSGEO4W_ROOT%\bin\bgspawn.exe" "\"%O4W_ROOT%\bin\@package@-designer.bat\"" "%DOCUMENTS%" "" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%OSGEO4W_DESKTOP%\Qt Designer with QGIS @version@ custom widgets" "%OSGEO4W_ROOT%\bin\bgspawn.exe" "\"%O4W_ROOT%\bin\@package@-designer.bat\"" "%DOCUMENTS%" "" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"

set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
"%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
del /s /q "%OSGEO4W_ROOT%\apps\@package@\python\*.pyc"
exit /b 0
