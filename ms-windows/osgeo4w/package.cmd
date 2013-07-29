REM ***************************************************************************
REM    package.cmd
REM    ---------------------
REM    begin                : July 2009
REM    copyright            : (C) 2009 by Juergen E. Fischer
REM    email                : jef at norbit dot de
REM ***************************************************************************
REM *                                                                         *
REM *   This program is free software; you can redistribute it and/or modify  *
REM *   it under the terms of the GNU General Public License as published by  *
REM *   the Free Software Foundation; either version 2 of the License, or     *
REM *   (at your option) any later version.                                   *
REM *                                                                         *
REM ***************************************************************************
@echo off
set GRASS_VERSION=6.4.3

set BUILDDIR=%CD%\build
REM set BUILDDIR=%TEMP%\qgis_unstable
set LOG=%BUILDDIR%\build.log

if not exist "%BUILDDIR%" mkdir %BUILDDIR%
if not exist "%BUILDDIR%" goto error

set VERSION=%1
set PACKAGE=%2
set PACKAGENAME=%3
if "%VERSION%"=="" goto error
if "%PACKAGE%"=="" goto error
if "%PACKAGENAME%"=="" set PACKAGENAME=qgis

path %SYSTEMROOT%\system32;%SYSTEMROOT%;%SYSTEMROOT%\System32\Wbem;%PROGRAMFILES%\CMake 2.8\bin
set PYTHONPATH=

set VS90COMNTOOLS=%PROGRAMFILES%\Microsoft Visual Studio 9.0\Common7\Tools\
call "%PROGRAMFILES%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86

if "%OSGEO4W_ROOT%"=="" set OSGEO4W_ROOT=%PROGRAMFILES%\OSGeo4W
if not exist "%OSGEO4W_ROOT%\bin\o4w_env.bat" goto error

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

set O4W_ROOT=%OSGEO4W_ROOT:\=/%
set LIB_DIR=%O4W_ROOT%

set DEVENV=
if exist "%DevEnvDir%\vcexpress.exe" set DEVENV=vcexpress
if exist "%DevEnvDir%\devenv.exe" set DEVENV=devenv
if "%DEVENV%"=="" goto error

PROMPT qgis%VERSION%$g 

set BUILDCONF=RelWithDebInfo
REM set BUILDCONF=Release


cd ..\..
set SRCDIR=%CD%

if "%BUILDDIR:~1,1%"==":" %BUILDDIR:~0,2%
cd %BUILDDIR%

if exist repackage goto package

if not exist build.log goto build

REM
REM try renaming the logfile to see if it's locked
REM

if exist build.tmp del build.tmp
if exist build.tmp goto error

ren build.log build.tmp
if exist build.log goto locked
if not exist build.tmp goto locked

ren build.tmp build.log
if exist build.tmp goto locked
if not exist build.log goto locked

goto build

:locked
echo Logfile locked
if exist build.tmp del build.tmp
goto error

:build
echo Logging to %LOG%
echo BEGIN: %DATE% %TIME%>>%LOG% 2>&1
if errorlevel 1 goto error

set >buildenv.log

if exist CMakeCache.txt goto skipcmake

echo CMAKE: %DATE% %TIME%>>%LOG% 2>&1
if errorlevel 1 goto error

set LIB=%LIB%;%OSGEO4W_ROOT%\lib
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
set GRASS_PREFIX=%O4W_ROOT%/apps/grass/grass-%GRASS_VERSION%

cmake -G "Visual Studio 9 2008" ^
	-D PEDANTIC=TRUE ^
	-D WITH_QSPATIALITE=TRUE ^
	-D WITH_MAPSERVER=TRUE ^
	-D MAPSERVER_SKIP_ECW=TRUE ^
	-D WITH_GLOBE=TRUE ^
	-D WITH_TOUCH=TRUE ^
	-D WITH_ORACLE=TRUE ^
	-D CMAKE_BUILD_TYPE=%BUILDCONF% ^
	-D CMAKE_CONFIGURATION_TYPES=%BUILDCONF% ^
	-D GEOS_LIBRARY=%O4W_ROOT%/lib/geos_c.lib ^
	-D SQLITE3_LIBRARY=%O4W_ROOT%/lib/sqlite3_i.lib ^
	-D SPATIALITE_LIBRARY=%O4W_ROOT%/lib/spatialite_i.lib ^
	-D PYTHON_EXECUTABLE=%O4W_ROOT%/bin/python.exe ^
	-D PYTHON_INCLUDE_PATH=%O4W_ROOT%/apps/Python27/include ^
	-D PYTHON_LIBRARY=%O4W_ROOT%/apps/Python27/libs/python27.lib ^
	-D SIP_BINARY_PATH=%O4W_ROOT%/apps/Python27/sip.exe ^
	-D QT_BINARY_DIR=%O4W_ROOT%/bin ^
	-D QT_LIBRARY_DIR=%O4W_ROOT%/lib ^
	-D QT_HEADERS_DIR=%O4W_ROOT%/include/qt4 ^
	-D QT_ZLIB_LIBRARY=%O4W_ROOT%/lib/zlib.lib ^
	-D QT_PNG_LIBRARY=%O4W_ROOT%/lib/libpng13.lib ^
	-D QWT_INCLUDE_DIR=%O4W_ROOT%/include/qwt ^
	-D QWT_LIBRARY=%O4W_ROOT%/lib/qwt5.lib ^
	-D CMAKE_INSTALL_PREFIX=%O4W_ROOT%/apps/%PACKAGENAME% ^
	-D CMAKE_CXX_FLAGS_RELWITHDEBINFO="/MD /ZI /Od /D NDEBUG" ^
	-D FCGI_INCLUDE_DIR=%O4W_ROOT%/include ^
	-D FCGI_LIBRARY=%O4W_ROOT%/lib/libfcgi.lib ^
	%SRCDIR%>>%LOG% 2>&1
if errorlevel 1 goto error

REM bail out if python or grass was not found
grep -Eq "^(Python not being built|Could not find GRASS)" %LOG%
if not errorlevel 1 goto error

:skipcmake

echo ZERO_CHECK: %DATE% %TIME%>>%LOG% 2>&1
%DEVENV% qgis%VERSION%.sln /Project ZERO_CHECK /Build %BUILDCONF% /Out %LOG%>>%LOG% 2>&1
if errorlevel 1 goto error

echo ALL_BUILD: %DATE% %TIME%>>%LOG% 2>&1
%DEVENV% qgis%VERSION%.sln /Project ALL_BUILD /Build %BUILDCONF% /Out %LOG%>>%LOG% 2>&1
if errorlevel 1 goto error

echo INSTALL: %DATE% %TIME%>>%LOG% 2>&1
%DEVENV% qgis%VERSION%.sln /Project INSTALL /Build %BUILDCONF% /Out %LOG%>>%LOG% 2>&1
if errorlevel 1 goto error

:package
echo PACKAGE: %DATE% %TIME%>>%LOG% 2>&1

cd ..
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' postinstall-common.bat >%OSGEO4W_ROOT%\etc\postinstall\\%PACKAGENAME%-common.bat

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' postinstall-desktop.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%.bat
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' preremove-desktop.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%.bat
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' qgis.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%.bat.tmpl
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' browser.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-browser.bat.tmpl
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' qgis.reg.tmpl >%OSGEO4W_ROOT%\apps\%PACKAGENAME%\bin\qgis.reg.tmpl

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' postinstall-server.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%-server.bat
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' preremove-server.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%-server.bat
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%GRASS_VERSION%/g' httpd.conf.tmpl >%OSGEO4W_ROOT%\httpd.d\httpd_%PACKAGENAME%.conf.tmpl

REM sed -e 's/%OSGEO4W_ROOT:\=\\\\\\\\%/@osgeo4w@/' %OSGEO4W_ROOT%\apps\%PACKAGENAME%\python\qgis\qgisconfig.py >%OSGEO4W_ROOT%\apps\%PACKAGENAME%\python\qgis\qgisconfig.py.tmpl
REM if errorlevel 1 goto error

REM del %OSGEO4W_ROOT%\apps\%PACKAGENAME%\python\qgis\qgisconfig.py

touch exclude

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-common-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/bin/Microsoft.VC90.CRT.manifest" ^
	"apps/%PACKAGENAME%/bin/msvcm90.dll" ^
	"apps/%PACKAGENAME%/bin/msvcp90.dll" ^
	"apps/%PACKAGENAME%/bin/msvcr90.dll" ^
	"apps/%PACKAGENAME%/bin/qgispython.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_analysis.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_networkanalysis.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_core.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_gui.dll" ^
	"apps/%PACKAGENAME%/doc/" ^
	"apps/%PACKAGENAME%/plugins/delimitedtextprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/diagramoverlay.dll" ^
	"apps/%PACKAGENAME%/plugins/gdalprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/gpxprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/memoryprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/mssqlprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/ogrprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/owsprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/postgresprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/qgissqlanyconnection.dll" ^
	"apps/%PACKAGENAME%/plugins/spatialiteprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/sqlanywhereprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/wcsprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/wfsprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/wmsprovider.dll" ^
	"apps/%PACKAGENAME%/resources/qgis.db" ^
	"apps/%PACKAGENAME%/resources/spatialite.db" ^
	"apps/%PACKAGENAME%/resources/srs.db" ^
	"apps/%PACKAGENAME%/resources/symbology-ng-style.db" ^
	"apps/%PACKAGENAME%/svg/" ^
	"apps/%PACKAGENAME%/cpt-city-qgis-min/" ^
	"apps/%PACKAGENAME%/crssync.exe" ^
	"etc/postinstall/%PACKAGENAME%-common.bat" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-server-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/bin/qgis_mapserv.fcgi.exe" ^
	"apps/%PACKAGENAME%/bin/admin.sld" ^
	"apps/%PACKAGENAME%/bin/wms_metadata.xml" ^
	"httpd.d/httpd_%PACKAGENAME%.conf.tmpl" ^
	"etc/postinstall/%PACKAGENAME%-server.bat" ^
	"etc/preremove/%PACKAGENAME%-server.bat" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

move %OSGEO4W_ROOT%\apps\%PACKAGENAME%\bin\qgis.exe %OSGEO4W_ROOT%\bin\%PACKAGENAME%-bin.exe
move %OSGEO4W_ROOT%\apps\%PACKAGENAME%\bin\qbrowser.exe %OSGEO4W_ROOT%\bin\%PACKAGENAME%-browser-bin.exe
tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"bin/%PACKAGENAME%-browser-bin.exe" ^
	"bin/%PACKAGENAME%-bin.exe" ^
	"apps/%PACKAGENAME%/bin/qgis.reg.tmpl" ^
	"apps/%PACKAGENAME%/i18n/" ^
	"apps/%PACKAGENAME%/icons/" ^
	"apps/%PACKAGENAME%/images/" ^
	"apps/%PACKAGENAME%/plugins/coordinatecaptureplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/delimitedtextplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/dxf2shpconverterplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/evis.dll" ^
	"apps/%PACKAGENAME%/plugins/georefplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/gpsimporterplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/heatmapplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/interpolationplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/offlineeditingplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/oracleplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/rasterterrainplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/roadgraphplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/spatialqueryplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/spitplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/sqlanywhereplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/topolplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/zonalstatisticsplugin.dll" ^
	"apps/%PACKAGENAME%/qgis_help.exe" ^
        "apps/qt4/plugins/sqldrivers/qsqlspatialite.dll" ^
	"apps/%PACKAGENAME%/python/" ^
	"apps/%PACKAGENAME%/resources/context_help/" ^
	"apps/%PACKAGENAME%/resources/function_help/" ^
	"apps/%PACKAGENAME%/resources/customization.xml" ^
	"apps/%PACKAGENAME%/resources/qgis_help.db" ^
	"bin/%PACKAGENAME%.bat.tmpl" ^
	"bin/%PACKAGENAME%-browser.bat.tmpl" ^
	"etc/postinstall/%PACKAGENAME%.bat" ^
	"etc/preremove/%PACKAGENAME%.bat" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-grass-plugin-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/grass" ^
	"apps/%PACKAGENAME%/bin/qgisgrass.dll" ^
	"apps/%PACKAGENAME%/plugins/grassrasterprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/grassplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/grassprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/libgrass_gis.%GRASS_VERSION%.dll" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-globe-plugin-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/globe" ^
	"apps/%PACKAGENAME%/plugins/globeplugin.dll" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-oracle-provider-%VERSION%-%PACKAGE%.tar.bz2 ^
	"apps/%PACKAGENAME%/plugins/oracleprovider.dll" ^
        apps/qt4/plugins/sqldrivers/qsqlocispatial.dll ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-devel-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/FindQGIS.cmake" ^
	"apps/%PACKAGENAME%/include/" ^
	"apps/%PACKAGENAME%/lib/" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

goto end

:error
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%>>%LOG% 2>&1
if exist %PACKAGENAME%-common-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-common-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-server-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-server-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-devel-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-devel-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-grass-plugin-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-grass-plugin-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-globe-plugin-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-globe-plugin-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-oracle-provider-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-oracle-provider-%VERSION%-%PACKAGE%.tar.bz2

:end
echo FINISHED: %DATE% %TIME% >>%LOG% 2>&1
