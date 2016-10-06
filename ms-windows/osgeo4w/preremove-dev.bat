for %%g in (@grassversions@) do (
	del "%OSGEO4W_STARTMENU%\QGIS Desktop @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_STARTMENU%\QGIS Browser @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_DESKTOP%\QGIS Desktop @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_DESKTOP%\QGIS Browser @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_ROOT%\bin\@package@-g%%g.bat"
	del "%OSGEO4W_ROOT%\bin\@package@-browser-g%%g.bat"
)

del "%OSGEO4W_STARTMENU%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk"
rmdir "%OSGEO4W_STARTMENU%"
del "%OSGEO4W_DESKTOP%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk"
rmdir "%OSGEO4W_DESKTOP%"

del "%OSGEO4W_ROOT%\bin\@package@-designer.bat"
del "%OSGEO4W_ROOT%\bin\python-@package@.bat"
del "%OSGEO4W_ROOT%\apps\@package@\python\qgis\qgisconfig.py"
del "%OSGEO4W_ROOT%\apps\@package@\bin\qgis.reg"
