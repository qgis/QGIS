textreplace -std -t bin\@package@-designer.bat
textreplace -std -t bin\python-@package@.bat

for %%g in (@grassversions@) do (
	textreplace -std -t bin\@package@-g%%g.bat
	textreplace -std -t bin\@package@-browser-g%%g.bat

	if not %OSGEO4W_MENU_LINKS%==0 mkdir "%OSGEO4W_STARTMENU%"
	if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "QGIS Desktop @version@ with GRASS %%g (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-g%%g.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
	if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "QGIS Browser @version@ with GRASS %%g (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-browser-g%%g.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\browser.ico"

	if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "~$folder.desktop$" "QGIS Desktop @version@ with GRASS %%g (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-g%%g.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
	if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "~$folder.desktop$" "QGIS Browser @version@ with GRASS %%g (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-browser-g%%g.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\browser.ico"
)

if not %OSGEO4W_MENU_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "%OSGEO4W_STARTMENU%" "Qt Designer with QGIS @version@ custom widgets (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-designer.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"
if not %OSGEO4W_DESKTOP_LINKS%==0 nircmd shortcut "%OSGEO4W_ROOT%\bin\nircmd.exe" "~$folder.desktop$" "Qt Designer with QGIS @version@ custom widgets (Nightly)" "exec hide """%OSGEO4W_ROOT%\bin\@package@-designer.bat"" "%OSGEO4W_ROOT%\apps\@package@\icons\QGIS.ico"

set O4W_ROOT=%OSGEO4W_ROOT%
set OSGEO4W_ROOT=%OSGEO4W_ROOT:\=\\%
textreplace -std -t "%O4W_ROOT%\apps\@package@\bin\qgis.reg"
set OSGEO4W_ROOT=%O4W_ROOT%

REM Do not register extensions if release is installed
if not exist "%O4W_ROOT%\apps\qgis\bin\qgis.reg" nircmd elevate "%WINDIR%\regedit" /s "%O4W_ROOT%\apps\@package@\bin\qgis.reg"

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
path %PATH%;%OSGEO4W_ROOT%\apps\@package@\bin
set QGIS_PREFIX_PATH=%OSGEO4W_ROOT:\=/%/apps/@package@
"%OSGEO4W_ROOT%\apps\@package@\crssync"
