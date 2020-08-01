setlocal enabledelayedexpansion

textreplace -std -t bin\@package@.bat
textreplace -std -t bin\@package@-designer.bat
textreplace -std -t bin\python-@package@.bat
textreplace -std -t bin\qgis_process-@package@.bat

if not defined OSGEO4W_DESKTOP for /F "tokens=* USEBACKQ" %%F IN (`getspecialfolder Desktop`) do set OSGEO4W_DESKTOP=%%F
for /F "tokens=* USEBACKQ" %%F IN (`getspecialfolder Documents`) do set DOCUMENTS=%%F

call "%OSGEO4W_ROOT%\bin\@package@.bat" --postinstall
echo on

if not %OSGEO4W_MENU_LINKS%==0 if not exist "%OSGEO4W_STARTMENU%" mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_DESKTOP_LINKS%==0 if not exist "%OSGEO4W_DESKTOP%" mkdir "%OSGEO4W_DESKTOP%"

if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\QGIS Desktop @version@ (Nightly).lnk" "%OSGEO4W_ROOT%\bin\@package@-bin.exe" "" "%DOCUMENTS%"
if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%OSGEO4W_DESKTOP%\QGIS Desktop @version@ (Nightly).lnk" "%OSGEO4W_ROOT%\bin\@package@-bin.exe" "" "%DOCUMENTS%"

for %%g in (@grassversions@) do (
	set gv=
	for /f "usebackq tokens=1" %%a in (`%%g --config version`) do set gv=%%a
	if not "!gv!"=="" (
		for /F "delims=." %%i in ("!gv!") do set v=%%i

		copy "%OSGEO4W_ROOT%\bin\@package@-bin.exe" "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe"
		copy "%OSGEO4W_ROOT%\bin\@package@-bin.vars" "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.vars"
		textreplace -std -map @grassmajor@ !v! -t bin\@package@-g!v!.bat
		call "%OSGEO4W_ROOT%\bin\@package@-g!v!.bat" --postinstall

		if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\QGIS Desktop @version@ with GRASS !gv! (Nightly).lnk" "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe" "" "%DOCUMENTS%"
		if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%OSGEO4W_DESKTOP%\QGIS Desktop @version@ with GRASS !gv! (Nightly).lnk" "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe" "" "%DOCUMENTS%"
	)
)

if not %OSGEO4W_MENU_LINKS%==0 xxmklink "%OSGEO4W_STARTMENU%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk" "%OSGEO4W_ROOT%\bin\bgspawn.exe" "\"%OSGEO4W_ROOT%\bin\@package@-designer.bat\"" "%DOCUMENTS%" "" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 xxmklink "%OSGEO4W_DESKTOP%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk" "%OSGEO4W_ROOT%\bin\bgspawn.exe" "\"%OSGEO4W_ROOT%\bin\@package@-designer.bat\"" "%DOCUMENTS%" "" 1 "%O4W_ROOT%\apps\@package@\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
set OSGEO4W_ROOT=%O4W_ROOT%

REM Do not register extensions if release is installed
if not exist "%O4W_ROOT%\apps\qgis\bin\qgis.reg" "%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call qt5_env.bat
call gdal-dev-env.bat
path %PATH%;%OSGEO4W_ROOT%\apps\@package@\bin
set QGIS_PREFIX_PATH=%OSGEO4W_ROOT:\=/%/apps/@package@
"%OSGEO4W_ROOT%\apps\@package@\crssync"

del /s /q "%OSGEO4W_ROOT%\apps\@package@\*.pyc"

endlocal

exit /b 0
