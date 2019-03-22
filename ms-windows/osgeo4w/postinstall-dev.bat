setlocal enabledelayedexpansion

textreplace -std -t bin\@package@-designer.bat
textreplace -std -t bin\python-@package@.bat

if "%OSGEO4W_DESKTOP%"=="" set OSGEO4W_DESKTOP=~$folder.common_desktop$

if not %OSGEO4W_MENU_LINKS%==0 mkdir "%OSGEO4W_STARTMENU%"
if not %OSGEO4W_DESKTOP_LINKS%==0 mkdir "%OSGEO4W_DESKTOP%"

for %%g in (@grassversions@) do (
        for /f "usebackq tokens=1" %%a in (`%%g --config version`) do set gv=%%a
	for /F "delims=." %%i in ("%%gv") do set v=%%i

	copy "%OSGEO4W_ROOT%\bin\@package@-bin.exe" "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe"
	copy "%OSGEO4W_ROOT%\bin\@package@-bin.vars" "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.vars"
	textreplace -std -map @grassmajor@ !v! -t bin\@package@-g!v!.bat
	call "%OSGEO4W_ROOT%\bin\@package@-g!v!.bat" --postinstall

	if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe" "%OSGEO4W_STARTMENU%" "QGIS Desktop @version@ with GRASS %%g (Nightly)"
	if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe" "%OSGEO4W_DESKTOP%" "QGIS Desktop @version@ with GRASS %%g (Nightly)"
)

if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "Qt Designer with QGIS @version@ custom widgets (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-designer.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_DESKTOP%" "Qt Designer with QGIS @version@ custom widgets (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-designer.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
set OSGEO4W_ROOT=%O4W_ROOT%

REM Do not register extensions if release is installed
if not exist "%O4W_ROOT%\apps\qgis\bin\qgis.reg" nircmd elevate "%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call qt5_env.bat
path %PATH%;%OSGEO4W_ROOT%\apps\@package@\bin
set QGIS_PREFIX_PATH=%OSGEO4W_ROOT:\=/%/apps/@package@
"%OSGEO4W_ROOT%\apps\@package@\crssync"

del /s /q "%OSGEO4W_ROOT%\apps\@package@\*.pyc"
exit /b 0

endlocal
