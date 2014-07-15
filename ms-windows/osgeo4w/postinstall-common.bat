call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
path %PATH%;%OSGEO4W_ROOT%\apps\@package@\bin
set QGIS_PREFIX_PATH=%OSGEO4W_ROOT:\=/%/apps/@package@
"%OSGEO4W_ROOT%\apps\@package@\crssync"
