@echo off
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

setlocal enabledelayedexpansion

set VERSION=%1
set PACKAGE=%2
set PACKAGENAME=%3
set ARCH=%4
set SHA=%5
set SITE=%6
if "%VERSION%"=="" goto usage
if "%PACKAGE%"=="" goto usage
if "%PACKAGENAME%"=="" goto usage
if "%ARCH%"=="" goto usage
if not "%SHA%"=="" set SHA=-%SHA%
if "%SITE%"=="" set SITE=qgis.org

set BUILDDIR=%CD%\build-%ARCH%

if "%OSGEO4W_ROOT%"=="" (
	if "%ARCH%"=="x86" (
		set OSGEO4W_ROOT=C:\OSGeo4W
	) else (
		set OSGEO4W_ROOT=C:\OSGeo4W64
	)
)

if not exist "%BUILDDIR%" mkdir %BUILDDIR%
if not exist "%BUILDDIR%" (echo could not create build directory %BUILDDIR% & goto error)

if not exist "%OSGEO4W_ROOT%\bin\o4w_env.bat" (echo o4w_env.bat not found & goto error)
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

set O4W_ROOT=%OSGEO4W_ROOT:\=/%
set LIB_DIR=%O4W_ROOT%

if not "%PROGRAMFILES(X86)%"=="" set PF86=%PROGRAMFILES(X86)%
if "%PF86%"=="" set PF86=%PROGRAMFILES%
if "%PF86%"=="" (echo PROGRAMFILES not set & goto error)

if "%ARCH%"=="x86" goto devenv_x86
goto devenv_x86_64

:devenv_x86
set GRASS_VERSIONS=6.4.4 7.0.1RC1
call "%PF86%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
if exist "c:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.Cmd" call "c:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.Cmd" /x86 /Release
path %path%;%PF86%\Microsoft Visual Studio 10.0\VC\bin

set CMAKE_OPT=^
	-G "Visual Studio 10" ^
	-D SIP_BINARY_PATH=%O4W_ROOT%/apps/Python27/sip.exe ^
	-D QWT_LIBRARY=%O4W_ROOT%/lib/qwt.lib ^
	-D WITH_GRASS=TRUE ^
	-D WITH_GRASS7=TRUE ^
	-D GRASS_PREFIX=%O4W_ROOT%/apps/grass/grass-6.4.4 ^
	-D GRASS_PREFIX7=%O4W_ROOT%/apps/grass/grass-7.0.1RC1
goto devenv

:devenv_x86_64
set GRASS_VERSIONS=6.4.3
call "%PF86%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
if exist "c:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.Cmd" call "c:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.Cmd" /x64 /Release
path %path%;%PF86%\Microsoft Visual Studio 10.0\VC\bin

set SETUPAPI_LIBRARY=%PF86%\Microsoft SDKs\Windows\v7.0A\Lib\x64\SetupAPI.Lib
if not exist "%SETUPAPI_LIBRARY%" set SETUPAPI_LIBRARY=%PROGRAMFILES%\Microsoft SDKs\Windows\v7.1\Lib\x64\SetupAPI.lib
if not exist "%SETUPAPI_LIBRARY%" (echo SETUPAPI_LIBRARY not found & goto error)

set CMAKE_OPT=^
	-G "Visual Studio 10 Win64" ^
	-D SPATIALINDEX_LIBRARY=%O4W_ROOT%/lib/spatialindex-64.lib ^
	-D WITH_GRASS=TRUE ^
	-D WITH_GRASS7=FALSE ^
	-D GRASS_PREFIX=%O4W_ROOT%/apps/grass/grass-6.4.3 ^
	-D SIP_BINARY_PATH=%O4W_ROOT%/bin/sip.exe ^
	-D QWT_LIBRARY=%O4W_ROOT%/lib/qwt5.lib ^
	-D SETUPAPI_LIBRARY="%SETUPAPI_LIBRARY%" ^
	-D CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS=TRUE

:devenv
set PYTHONPATH=
path %PF86%\CMake\bin;%PATH%;c:\cygwin\bin

PROMPT qgis%VERSION%$g 

set BUILDCONF=Release

cd ..\..
set SRCDIR=%CD%

if "%BUILDDIR:~1,1%"==":" %BUILDDIR:~0,2%
cd %BUILDDIR%

set PKGDIR=%OSGEO4W_ROOT%\apps\%PACKAGENAME%

if exist repackage goto package

if not exist build.log goto build

REM
REM try renaming the logfile to see if it's locked
REM

if exist build.tmp del build.tmp
if exist build.tmp (echo could not remove build.tmp & goto error)

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
echo BEGIN: %DATE% %TIME%

set >buildenv.log

if exist qgsversion.h del qgsversion.h

if exist CMakeCache.txt if exist skipcmake goto skipcmake

touch %SRCDIR%\CMakeLists.txt

echo CMAKE: %DATE% %TIME%
if errorlevel 1 goto error

set LIB=%LIB%;%OSGEO4W_ROOT%\lib
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include

cmake %CMAKE_OPT% ^
	-D BUILDNAME="%PACKAGENAME%-%VERSION%%SHA%-Release-VC10-%ARCH%" ^
	-D SITE="%SITE%" ^
	-D PEDANTIC=TRUE ^
	-D WITH_QSPATIALITE=TRUE ^
	-D WITH_SERVER=TRUE ^
	-D SERVER_SKIP_ECW=TRUE ^
	-D WITH_GLOBE=TRUE ^
	-D WITH_TOUCH=TRUE ^
	-D WITH_ORACLE=TRUE ^
	-D WITH_CUSTOM_WIDGETS=TRUE ^
	-D CMAKE_CXX_FLAGS_RELEASE="/MD /MP /O2 /Ob2 /D NDEBUG" ^
	-D CMAKE_BUILD_TYPE=%BUILDCONF% ^
	-D CMAKE_CONFIGURATION_TYPES=%BUILDCONF% ^
	-D GEOS_LIBRARY=%O4W_ROOT%/lib/geos_c.lib ^
	-D SQLITE3_LIBRARY=%O4W_ROOT%/lib/sqlite3_i.lib ^
	-D SPATIALITE_LIBRARY=%O4W_ROOT%/lib/spatialite_i.lib ^
	-D PYTHON_EXECUTABLE=%O4W_ROOT%/bin/python.exe ^
	-D PYTHON_INCLUDE_PATH=%O4W_ROOT%/apps/Python27/include ^
	-D PYTHON_LIBRARY=%O4W_ROOT%/apps/Python27/libs/python27.lib ^
	-D QT_BINARY_DIR=%O4W_ROOT%/bin ^
	-D QT_LIBRARY_DIR=%O4W_ROOT%/lib ^
	-D QT_HEADERS_DIR=%O4W_ROOT%/include/qt4 ^
	-D QWT_INCLUDE_DIR=%O4W_ROOT%/include/qwt ^
	-D CMAKE_INSTALL_PREFIX=%O4W_ROOT%/apps/%PACKAGENAME% ^
	-D FCGI_INCLUDE_DIR=%O4W_ROOT%/include ^
	-D FCGI_LIBRARY=%O4W_ROOT%/lib/libfcgi.lib ^
	-D WITH_INTERNAL_JINJA2=FALSE ^
	-D WITH_INTERNAL_MARKUPSAFE=FALSE ^
	-D WITH_INTERNAL_PYGMENTS=FALSE ^
	-D WITH_INTERNAL_DATEUTIL=FALSE ^
	-D WITH_INTERNAL_PYTZ=FALSE ^
	-D WITH_INTERNAL_SIX=FALSE ^
	%SRCDIR%
if errorlevel 1 (echo cmake failed & goto error)

:skipcmake
if exist noclean (echo skip clean & goto skipclean)
echo CLEAN: %DATE% %TIME%
cmake --build %BUILDDIR% --target clean --config %BUILDCONF%
if errorlevel 1 (echo clean failed & goto error)

:skipclean
echo ALL_BUILD: %DATE% %TIME%
cmake --build %BUILDDIR% --config %BUILDCONF%
if errorlevel 1 cmake --build %BUILDDIR% --config %BUILDCONF%
if errorlevel 1 (echo build failed twice & goto error)

if exist ..\skiptests goto skiptests

echo RUN_TESTS: %DATE% %TIME%

set oldpath=%PATH%
for %%g IN (%GRASS_VERSIONS%) do (
	set path=!path!;%OSGEO4W_ROOT%\apps\grass\grass-%%g\lib
	set GISBASE=%OSGEO4W_ROOT%\apps\grass\grass-%%g
)
PATH %path%;%BUILDDIR%\output\plugins\%BUILDCONF%

cmake --build %BUILDDIR% --target Experimental --config %BUILDCONF%
if errorlevel 1 echo TESTS WERE NOT SUCCESSFUL.

PATH %oldpath%

:skiptests
:package

if exist %PKGDIR% (
	echo REMOVE: %DATE% %TIME%
	rmdir /s /q %PKGDIR%
)

echo INSTALL: %DATE% %TIME%
cmake --build %BUILDDIR% --target INSTALL --config %BUILDCONF%
if errorlevel 1 (echo INSTALL failed & goto error)

echo PACKAGE: %DATE% %TIME%

cd ..
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' postinstall-common.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%-common.bat
if errorlevel 1 (echo creation of common postinstall failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' postinstall-desktop.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%.bat
if errorlevel 1 (echo creation of desktop postinstall failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' preremove-desktop.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%.bat
if errorlevel 1 (echo creation of desktop preremove failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' qgis.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%.bat.tmpl
if errorlevel 1 (echo creation of desktop template failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' browser.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-browser.bat.tmpl
if errorlevel 1 (echo creation of browser template & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' designer.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-designer.bat.tmpl
if errorlevel 1 (echo creation of designer template failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' python.bat.tmpl >%OSGEO4W_ROOT%\bin\python-%PACKAGENAME%.bat.tmpl
if errorlevel 1 (echo creation of python wrapper template failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' qgis.reg.tmpl >%PKGDIR%\bin\qgis.reg.tmpl
if errorlevel 1 (echo creation of registry template & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' postinstall-server.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%-server.bat
if errorlevel 1 (echo creation of server postinstall failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' preremove-server.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%-server.bat
if errorlevel 1 (echo creation of server preremove failed & goto error)

if not exist %OSGEO4W_ROOT%\httpd.d mkdir %OSGEO4W_ROOT%\httpd.d
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' httpd.conf.tmpl >%OSGEO4W_ROOT%\httpd.d\httpd_%PACKAGENAME%.conf.tmpl
if errorlevel 1 (echo creation of httpd.conf template failed & goto error)

set packages="" "-common" "-server" "-devel" "-globe-plugin" "-oracle-provider" "-grass-plugin-common"

for %%g IN (%GRASS_VERSIONS%) do (
	for /F "delims=." %%i in ("%%g") do set v=%%i
	set w=!v!
	if !v!==6 set w=

	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%%g/g' -e 's/@grassmajor@/!v!/g' postinstall-grass.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%-grass-plugin!w!.bat
	if errorlevel 1 (echo creation of grass desktop postinstall failed & goto error)
	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%%g/g' -e 's/@grassmajor@/!v!/g' preremove-grass.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%-grass-plugin!w!.bat
	if errorlevel 1 (echo creation of grass desktop preremove failed & goto error)
	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%%g/g' -e 's/@grassmajor@/!v!/g' qgis-grass.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-grass!v!.bat.tmpl
	if errorlevel 1 (echo creation of grass desktop template failed & goto error)
	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%%g/g' -e 's/@grassmajor@/!v!/g' browser-grass.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-browser-grass!v!.bat.tmpl
	if errorlevel 1 (echo creation of grass browser template & goto error)

	set packages=!packages! "-grass-plugin!w!"
)

touch exclude

for %%i in (%packages%) do (
	if not exist %ARCH%\release\qgis\%PACKAGENAME%%%i mkdir %ARCH%\release\qgis\%PACKAGENAME%%%i
)

tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-common/%PACKAGENAME%-common-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/bin/qgispython.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_analysis.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_networkanalysis.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_core.dll" ^
	"apps/%PACKAGENAME%/bin/qgis_gui.dll" ^
	"apps/%PACKAGENAME%/doc/" ^
	"apps/%PACKAGENAME%/plugins/delimitedtextprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/gdalprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/gpxprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/memoryprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/mssqlprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/ogrprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/owsprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/postgresprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/spatialiteprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/wcsprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/wfsprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/wmsprovider.dll" ^
	"apps/%PACKAGENAME%/resources/qgis.db" ^
	"apps/%PACKAGENAME%/resources/spatialite.db" ^
	"apps/%PACKAGENAME%/resources/srs.db" ^
	"apps/%PACKAGENAME%/resources/symbology-ng-style.db" ^
	"apps/%PACKAGENAME%/resources/cpt-city-qgis-min/" ^
	"apps/%PACKAGENAME%/svg/" ^
	"apps/%PACKAGENAME%/crssync.exe" ^
	"etc/postinstall/%PACKAGENAME%-common.bat"
if errorlevel 1 (echo tar common failed & goto error)

tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-server/%PACKAGENAME%-server-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/bin/qgis_mapserv.fcgi.exe" ^
	"apps/%PACKAGENAME%/bin/qgis_server.dll" ^
	"apps/%PACKAGENAME%/bin/admin.sld" ^
	"apps/%PACKAGENAME%/bin/wms_metadata.xml" ^
	"apps/%PACKAGENAME%/bin/schemaExtension.xsd" ^
	"apps/%PACKAGENAME%/python/qgis/_server.pyd" ^
	"apps/%PACKAGENAME%/python/qgis/_server.lib" ^
	"apps/%PACKAGENAME%/python/qgis/server/" ^
	"httpd.d/httpd_%PACKAGENAME%.conf.tmpl" ^
	"etc/postinstall/%PACKAGENAME%-server.bat" ^
	"etc/preremove/%PACKAGENAME%-server.bat"
if errorlevel 1 (echo tar server failed & goto error)

move %PKGDIR%\bin\qgis.exe %OSGEO4W_ROOT%\bin\%PACKAGENAME%-bin.exe
if errorlevel 1 (echo move of desktop executable failed & goto error)
move %PKGDIR%\bin\qbrowser.exe %OSGEO4W_ROOT%\bin\%PACKAGENAME%-browser-bin.exe
if errorlevel 1 (echo move of browser executable failed & goto error)

if not exist %PKGDIR%\qtplugins\sqldrivers mkdir %PKGDIR%\qtplugins\sqldrivers
move %OSGEO4W_ROOT%\apps\qt4\plugins\sqldrivers\qsqlocispatial.dll %PKGDIR%\qtplugins\sqldrivers
if errorlevel 1 (echo move of oci sqldriver failed & goto error)
move %OSGEO4W_ROOT%\apps\qt4\plugins\sqldrivers\qsqlspatialite.dll %PKGDIR%\qtplugins\sqldrivers
if errorlevel 1 (echo move of spatialite sqldriver failed & goto error)

if not exist %PKGDIR%\qtplugins\designer mkdir %PKGDIR%\qtplugins\designer
move %OSGEO4W_ROOT%\apps\qt4\plugins\designer\qgis_customwidgets.dll %PKGDIR%\qtplugins\designer
if errorlevel 1 (echo move of customwidgets failed & goto error)

if not exist %PKGDIR%\python\PyQt4\uic\widget-plugins mkdir %PKGDIR%\python\PyQt4\uic\widget-plugins
move %OSGEO4W_ROOT%\apps\Python27\Lib\site-packages\PyQt4\uic\widget-plugins\qgis_customwidgets.py %PKGDIR%\python\PyQt4\uic\widget-plugins
if errorlevel 1 (echo move of customwidgets binding failed & goto error)

if not exist %ARCH%\release\qgis\%PACKAGENAME% mkdir %ARCH%\release\qgis\%PACKAGENAME%
tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%/%PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	--exclude "apps/%PACKAGENAME%/python/qgis/_server.pyd" ^
	--exclude "apps/%PACKAGENAME%/python/qgis/_server.lib" ^
	--exclude "apps/%PACKAGENAME%/python/qgis/server" ^
	"bin/%PACKAGENAME%-browser-bin.exe" ^
	"bin/%PACKAGENAME%-bin.exe" ^
	"apps/%PACKAGENAME%/bin/qgis.reg.tmpl" ^
	"apps/%PACKAGENAME%/i18n/" ^
	"apps/%PACKAGENAME%/icons/" ^
	"apps/%PACKAGENAME%/images/" ^
	"apps/%PACKAGENAME%/plugins/coordinatecaptureplugin.dll" ^
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
	"apps/%PACKAGENAME%/plugins/topolplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/zonalstatisticsplugin.dll" ^
	"apps/%PACKAGENAME%/qgis_help.exe" ^
	"apps/%PACKAGENAME%/qtplugins/sqldrivers/qsqlspatialite.dll" ^
	"apps/%PACKAGENAME%/qtplugins/designer/" ^
	"apps/%PACKAGENAME%/python/" ^
	"apps/%PACKAGENAME%/resources/customization.xml" ^
	"bin/%PACKAGENAME%.bat.tmpl" ^
	"bin/%PACKAGENAME%-browser.bat.tmpl" ^
	"bin/%PACKAGENAME%-designer.bat.tmpl" ^
	"etc/postinstall/%PACKAGENAME%.bat" ^
	"etc/preremove/%PACKAGENAME%.bat"
if errorlevel 1 (echo tar desktop failed & goto error)

tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-grass-plugin-common/%PACKAGENAME%-grass-plugin-common-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.d.rast6.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.d.rast7.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.g.info6.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.g.info7.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.r.in6.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.r.in7.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.v.in6.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/modules/qgis.v.in7.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/bin/qgis.g.browser6.exe" ^
	--exclude "apps/%PACKAGENAME%/grass/bin/qgis.g.browser7.exe" ^
	"apps/%PACKAGENAME%/grass"
if errorlevel 1 (echo tar grass-plugin failed & goto error)

for %%g IN (%GRASS_VERSIONS%) do (
	for /F "delims=." %%i in ("%%g") do set v=%%i
	set w=!v!
	if !v!==6 set w=

	set files="apps/%PACKAGENAME%/bin/qgisgrass!v!.dll" ^
		"apps/%PACKAGENAME%/grass/modules/qgis.d.rast!v!.exe" ^
		"apps/%PACKAGENAME%/grass/modules/qgis.g.info!v!.exe" ^
		"apps/%PACKAGENAME%/grass/modules/qgis.r.in!v!.exe" ^
		"apps/%PACKAGENAME%/grass/modules/qgis.v.in!v!.exe" ^
		"apps/%PACKAGENAME%/plugins/grassrasterprovider!v!.dll" ^
		"apps/%PACKAGENAME%/plugins/grassprovider!v!.dll" ^
		"bin/%PACKAGENAME%-grass!v!.bat.tmpl" ^
		"bin/%PACKAGENAME%-browser-grass!v!.bat.tmpl" ^
		"etc/postinstall/%PACKAGENAME%-grass-plugin!w!.bat" ^
		"etc/preremove/%PACKAGENAME%-grass-plugin!w!.bat"

	if !v!==6 set files=!files! ^
		"apps/%PACKAGENAME%/plugins/grassplugin!v!.dll" ^
		"apps/%PACKAGENAME%/grass/bin/qgis.g.browser!v!.exe"

	tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-grass-plugin!w!/%PACKAGENAME%-grass-plugin!w!-%VERSION%-%PACKAGE%.tar.bz2 ^
		!files!
	if errorlevel 1 (echo tar grass-plugin!w! failed & goto error)
)

tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-globe-plugin/%PACKAGENAME%-globe-plugin-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/globe" ^
	"apps/%PACKAGENAME%/plugins/globeplugin.dll"
if errorlevel 1 (echo tar globe-plugin failed & goto error)

tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-oracle-provider/%PACKAGENAME%-oracle-provider-%VERSION%-%PACKAGE%.tar.bz2 ^
	"apps/%PACKAGENAME%/plugins/oracleprovider.dll" ^
	"apps/%PACKAGENAME%/qtplugins/sqldrivers/qsqlocispatial.dll"
if errorlevel 1 (echo tar oracle-provider failed & goto error)

tar -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%-devel/%PACKAGENAME%-devel-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/FindQGIS.cmake" ^
	"apps/%PACKAGENAME%/include/" ^
	"apps/%PACKAGENAME%/lib/"
if errorlevel 1 (echo tar devel failed & goto error)

goto end

:usage
echo usage: %0 version package packagename arch [sha [site]]
echo sample: %0 2.0.1 3 qgis x86 f802808
exit

:error
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%
for %%i in ("" "-common" "-server" "-devel" "-grass-plugin" "-globe-plugin" "-oracle-provider") do (
	if exist %ARCH%\release\qgis\%PACKAGENAME%%%i\%PACKAGENAME%%%i-%VERSION%-%PACKAGE%.tar.bz2 del %ARCH%\release\qgis\%PACKAGENAME%%%i\%PACKAGENAME%%%i-%VERSION%-%PACKAGE%.tar.bz2
)

:end
echo FINISHED: %DATE% %TIME%

endlocal
