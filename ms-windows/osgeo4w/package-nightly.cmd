@echo off
REM ***************************************************************************
REM    package-nightly.cmd
REM    ---------------------
REM    begin                : January 2011
REM    copyright            : (C) 2011 by Juergen E. Fischer
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
if "%VERSION%"=="" goto usage
if "%PACKAGE%"=="" goto usage
if "%PACKAGENAME%"=="" goto usage
if "%ARCH%"=="" goto usage

set BUILDDIR=%CD%\build-nightly-%ARCH%

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
	-D GRASS_PREFIX7=%O4W_ROOT%/apps/grass/grass-7.0.1RC1 ^
	-D CMAKE_CXX_FLAGS_RELWITHDEBINFO="/MD /ZI /MP /Od /D NDEBUG /D QGISDEBUG" ^
	-D CMAKE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO=%BUILDDIR%\apps\%PACKAGENAME%\pdb
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
	-D CMAKE_CXX_FLAGS_RELWITHDEBINFO="/MD /Zi /MP /Od /D NDEBUG /D QGISDEBUG" ^
	-D CMAKE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO=%BUILDDIR%\apps\%PACKAGENAME%\pdb ^
	-D SETUPAPI_LIBRARY="%SETUPAPI_LIBRARY%" ^
	-D CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS=TRUE

:devenv
set PYTHONPATH=
path %PF86%\CMake\bin;%PF86%\CMake 2.8\bin;%PATH%;c:\cygwin\bin

PROMPT qgis%VERSION%$g 

set BUILDCONF=RelWithDebInfo

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

if exist CMakeCache.txt goto skipcmake

echo CMAKE: %DATE% %TIME%
if errorlevel 1 goto error

set LIB=%LIB%;%OSGEO4W_ROOT%\lib
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include

cmake %CMAKE_OPT% ^
	-D BUILDNAME="%PACKAGENAME%-%VERSION%-Nightly-VC10-%ARCH%" ^
	-D SITE="qgis.org" ^
	-D PEDANTIC=TRUE ^
	-D WITH_QSPATIALITE=TRUE ^
	-D WITH_SERVER=TRUE ^
	-D SERVER_SKIP_ECW=TRUE ^
	-D WITH_ASTYLE=TRUE ^
	-D WITH_GLOBE=TRUE ^
	-D WITH_TOUCH=TRUE ^
	-D WITH_ORACLE=TRUE ^
	-D WITH_CUSTOM_WIDGETS=TRUE ^
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

cmake --build %BUILDDIR% --target Nightly --config %BUILDCONF%
if errorlevel 1 echo TESTS WERE NOT SUCCESSFUL.

PATH %oldpath%

:skiptests

if exist %PKGDIR% (
	echo REMOVE: %DATE% %TIME%
	rmdir /s /q %PKGDIR%
)

echo INSTALL: %DATE% %TIME%
cmake --build %BUILDDIR% --target INSTALL --config %BUILDCONF%
if errorlevel 1 (echo INSTALL failed & goto error)

:package
echo PACKAGE: %DATE% %TIME%

cd ..

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversions@/%GRASS_VERSIONS%/g' postinstall-dev.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%.bat
if errorlevel 1 (echo creation of desktop postinstall failed & goto error)
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversions@/%GRASS_VERSIONS%/g' preremove-dev.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%.bat
if errorlevel 1 (echo creation of desktop preremove failed & goto error)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' designer.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-designer.bat.tmpl
if errorlevel 1 (echo creation of designer template failed & goto error)
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' qgis.reg.tmpl >%PKGDIR%\bin\qgis.reg.tmpl
if errorlevel 1 (echo creation of registry template & goto error)

set batches=
for %%g IN (%GRASS_VERSIONS%) do (
	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%%g/g' qgis.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-g%%g.bat.tmpl
	if errorlevel 1 (echo creation of desktop template failed & goto error)
	set batches=!batches! bin/%PACKAGENAME%-g%%g.bat.tmpl

	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grassversion@/%%g/g' browser.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-browser-g%%g.bat.tmpl
	if errorlevel 1 (echo creation of browser template & goto error)
	set batches=!batches! bin/%PACKAGENAME%-browser-g%%g.bat.tmpl
)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' python.bat.tmpl >%OSGEO4W_ROOT%\bin\python-%PACKAGENAME%.bat.tmpl
if errorlevel 1 (echo creation of python wrapper failed & goto error)

REM sed -e 's/%OSGEO4W_ROOT:\=\\\\\\\\%/@osgeo4w@/' %PKGDIR%\python\qgis\qgisconfig.py >%PKGDIR%\python\qgis\qgisconfig.py.tmpl
REM if errorlevel 1 (echo creation of qgisconfig.py.tmpl failed & goto error)

REM del %PKGDIR%\python\qgis\qgisconfig.py

touch exclude

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
	apps/%PACKAGENAME% ^
	bin/%PACKAGENAME%-bin.exe ^
	bin/%PACKAGENAME%-browser-bin.exe ^
	%batches% ^
	bin/%PACKAGENAME%-designer.bat.tmpl ^
	bin/python-%PACKAGENAME%.bat.tmpl ^
	etc/postinstall/%PACKAGENAME%.bat ^
	etc/preremove/%PACKAGENAME%.bat
if errorlevel 1 (echo tar failed & goto error)

if not exist %ARCH%\release\qgis\%PACKAGENAME%-pdb mkdir %ARCH%\release\qgis\%PACKAGENAME%-pdb
tar -C %BUILDDIR% -cjf %ARCH%/release/qgis/%PACKAGENAME%-pdb/%PACKAGENAME%-pdb-%VERSION%-%PACKAGE%.tar.bz2 ^
	apps/%PACKAGENAME%/pdb
if errorlevel 1 (echo tar failed & goto error)

goto end

:usage
echo usage: %0 version package packagename arch
echo sample: %0 2.1.0 38 qgis-dev x86_64
exit

:error
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%
if exist %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2

:end
echo FINISHED: %DATE% %TIME%

endlocal
