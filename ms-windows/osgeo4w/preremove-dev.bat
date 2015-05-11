for %%g in (@grassversions@) do (
	del "%OSGEO4W_STARTMENU%\QGIS Desktop @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_STARTMENU%\QGIS Browser @version@ with GRASS %%g (Nightly).lnk"
	del "%ALLUSERSPROFILE%\Desktop\QGIS Desktop @version@ with GRASS %%g (Nightly).lnk"
	del "%ALLUSERSPROFILE%\Desktop\QGIS Browser @version@ with GRASS %%g (Nightly).lnk"
	del "%OSGEO4W_ROOT%"\bin\@package@-g%%g.bat
	del "%OSGEO4W_ROOT%"\bin\@package@-browser-g%%g.bat
)

del "%OSGEO4W_STARTMENU%\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk"
del "%ALLUSERSPROFILE%\Desktop\Qt Designer with QGIS @version@ custom widgets (Nightly).lnk"

del "%OSGEO4W_ROOT%"\bin\designer-@package@.bat
del "%OSGEO4W_ROOT%"\bin\python-@package@.bat
del "%OSGEO4W_ROOT%"\apps\@package@\python\qgis\qgisconfig.py
del "%OSGEO4W_ROOT%"\apps\@package@\bin\qgis.reg
