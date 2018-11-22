setlocal enabledelayedexpansion

for %%g in (@grassversions@) do (
	for /F "delims=." %%i in ("%%g") do set v=%%i

	del "%OSGEO4W_STARTMENU%\QGIS Desktop @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_STARTMENU%\QGIS Browser @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_DESKTOP%\QGIS Desktop @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_DESKTOP%\QGIS Browser @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_ROOT%\bin\@package@-g!v!.bat"
	del "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.exe"
	del "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.env"
	del "%OSGEO4W_ROOT%\bin\@package@-bin-g!v!.vars"
)

del "%OSGEO4W_STARTMENU%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk"
rmdir "%OSGEO4W_STARTMENU%"
del "%OSGEO4W_DESKTOP%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk"
rmdir "%OSGEO4W_DESKTOP%"

del "%OSGEO4W_ROOT%\bin\@package@-bin.env"
del "%OSGEO4W_ROOT%\bin\@package@-designer.bat"
del "%OSGEO4W_ROOT%\bin\python-@package@.bat"
del "%OSGEO4W_ROOT%\apps\@package@\python\qgis\qgisconfig.py"
del "%OSGEO4W_ROOT%\apps\@package@\bin\qgis.reg"
del /s /q "%OSGEO4W_ROOT%\apps\@package@\*.pyc"

endlocal
