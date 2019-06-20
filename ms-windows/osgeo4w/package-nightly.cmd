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
set SHA=%5
set SITE=%6
if "%VERSION%"=="" goto usage
if "%PACKAGE%"=="" goto usage
if "%PACKAGENAME%"=="" goto usage
if "%ARCH%"=="" goto usage
if not "%SHA%"=="" set SHA=-%SHA%
if "%SITE%"=="" set SITE=qgis.org

set BUILDDIR=%CD%\build-%PACKAGENAME%-%ARCH%
if not exist "%BUILDDIR%" mkdir %BUILDDIR%
if not exist "%BUILDDIR%" (echo could not create build directory %BUILDDIR% & goto error)

call msvc-env.bat %ARCH%

set O4W_ROOT=%OSGEO4W_ROOT:\=/%
set LIB_DIR=%O4W_ROOT%

if "%ARCH%"=="x86" goto cmake_x86
goto cmake_x86_64

:cmake_x86
set CMAKE_COMPILER_PATH=%PF86%\Microsoft Visual Studio 14.0\VC\bin
set SETUPAPI_LIBRARY=%PF86%\Windows Kits\10\Lib\10.0.14393.0\um\x86\SetupAPI.Lib
if not exist "%SETUPAPI_LIBRARY%" set SETUPAPI_LIBRARY=%PF86%\Windows Kits\8.0\Lib\win8\um\x86\SetupAPI.Lib
if not exist "%SETUPAPI_LIBRARY%" (echo SETUPAPI_LIBRARY not found & goto error)

set CMAKE_OPT=^
	-D CMAKE_CXX_FLAGS_RELWITHDEBINFO="/MD /ZI /MP /Od /D NDEBUG" ^
	-D CMAKE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO=%BUILDDIR%\apps\%PACKAGENAME%\pdb ^
	-D SPATIALINDEX_LIBRARY=%O4W_ROOT%/lib/spatialindex-32.lib
goto cmake

:cmake_x86_64
set CMAKE_COMPILER_PATH=%PF86%\Microsoft Visual Studio 14.0\VC\bin\amd64
set SETUPAPI_LIBRARY=%PF86%\Windows Kits\10\Lib\10.0.14393.0\um\x64\SetupAPI.Lib
if not exist "%SETUPAPI_LIBRARY%" set SETUPAPI_LIBRARY=%PF86%\Windows Kits\8.0\Lib\win8\um\x64\SetupAPI.Lib
if not exist "%SETUPAPI_LIBRARY%" (echo SETUPAPI_LIBRARY not found & goto error)

set CMAKE_OPT=^
	-D SPATIALINDEX_LIBRARY=%O4W_ROOT%/lib/spatialindex-64.lib ^
	-D CMAKE_CXX_FLAGS_RELWITHDEBINFO="/MD /Zi /MP /Od /D NDEBUG" ^
	-D CMAKE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO=%BUILDDIR%\apps\%PACKAGENAME%\pdb ^
	-D SETUPAPI_LIBRARY="%SETUPAPI_LIBRARY%" ^
	-D CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS=TRUE

:cmake
for %%i in ("%GRASS_PREFIX%") do set GRASS7_VERSION=%%~nxi
set GRASS_VERSIONS=%GRASS7_VERSION%

set TAR=tar.exe
if exist "c:\cygwin\bin\tar.exe" set TAR=c:\cygwin\bin\tar.exe
if exist "c:\cygwin64\bin\tar.exe" set TAR=c:\cygwin64\bin\tar.exe

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

if exist CMakeCache.txt if exist skipcmake goto skipcmake

touch %SRCDIR%\CMakeLists.txt

echo CMAKE: %DATE% %TIME%
if errorlevel 1 goto error

if "%CMAKEGEN%"=="" set CMAKEGEN=Ninja

for %%i in (%PYTHONHOME%) do set PYVER=%%~ni

cmake -G "%CMAKEGEN%" ^
	-D CMAKE_CXX_COMPILER="%CMAKE_COMPILER_PATH:\=/%/cl.exe" ^
	-D CMAKE_C_COMPILER="%CMAKE_COMPILER_PATH:\=/%/cl.exe" ^
	-D CMAKE_LINKER="%CMAKE_COMPILER_PATH:\=/%/link.exe" ^
	-D BUILDNAME="%PACKAGENAME%-%VERSION%%SHA%-Nightly-VC14-%ARCH%" ^
	-D SITE="%SITE%" ^
	-D PEDANTIC=TRUE ^
	-D WITH_QSPATIALITE=TRUE ^
	-D WITH_SERVER=TRUE ^
	-D SERVER_SKIP_ECW=TRUE ^
	-D WITH_GRASS=TRUE ^
	-D WITH_3D=TRUE ^
	-D WITH_GRASS7=TRUE ^
	-D GRASS_PREFIX7=%GRASS_PREFIX:\=/% ^
	-D WITH_GLOBE=FALSE ^
	-D WITH_ORACLE=TRUE ^
	-D WITH_CUSTOM_WIDGETS=TRUE ^
	-D CMAKE_BUILD_TYPE=%BUILDCONF% ^
	-D CMAKE_CONFIGURATION_TYPES=%BUILDCONF% ^
	-D GEOS_LIBRARY=%O4W_ROOT%/lib/geos_c.lib ^
	-D SQLITE3_LIBRARY=%O4W_ROOT%/lib/sqlite3_i.lib ^
	-D SPATIALITE_LIBRARY=%O4W_ROOT%/lib/spatialite_i.lib ^
	-D PYTHON_EXECUTABLE=%O4W_ROOT%/bin/python3.exe ^
	-D SIP_BINARY_PATH=%PYTHONHOME:\=/%/sip.exe ^
	-D PYTHON_INCLUDE_PATH=%PYTHONHOME:\=/%/include ^
	-D PYTHON_LIBRARY=%PYTHONHOME:\=/%/libs/%PYVER%.lib ^
	-D QT_LIBRARY_DIR=%O4W_ROOT%/lib ^
	-D QT_HEADERS_DIR=%O4W_ROOT%/apps/qt5/include ^
	-D CMAKE_INSTALL_PREFIX=%O4W_ROOT%/apps/%PACKAGENAME% ^
	-D FCGI_INCLUDE_DIR=%O4W_ROOT%/include ^
	-D FCGI_LIBRARY=%O4W_ROOT%/lib/libfcgi.lib ^
	-D QCA_INCLUDE_DIR=%OSGEO4W_ROOT%\apps\Qt5\include\QtCrypto ^
	-D QCA_LIBRARY=%OSGEO4W_ROOT%\apps\Qt5\lib\qca-qt5.lib ^
	-D QSCINTILLA_LIBRARY=%OSGEO4W_ROOT%\apps\Qt5\lib\qscintilla2.lib ^
	%CMAKE_OPT% ^
	%SRCDIR:\=/%
if errorlevel 1 (echo cmake failed & goto error)

if "%CONFIGONLY%"=="1" (echo Exiting after configuring build directory: %CD% & goto end)

:skipcmake
if exist ..\noclean (echo skip clean & goto skipclean)
echo CLEAN: %DATE% %TIME%
cmake --build %BUILDDIR% --target clean --config %BUILDCONF%
if errorlevel 1 (echo clean failed & goto error)

:skipclean
if exist ..\skipbuild (echo skip build & goto skipbuild)
echo ALL_BUILD: %DATE% %TIME%
cmake --build %BUILDDIR% --target NightlyBuild --config %BUILDCONF%
if errorlevel 1 cmake --build %BUILDDIR% --target NightlyBuild --config %BUILDCONF%
if errorlevel 1 (
	cmake --build %BUILDDIR% --target NightlySubmit --config %BUILDCONF%
	if errorlevel 1 echo SUBMITTING BUILD ERRORS WAS NOT SUCCESSFUL.
	echo build failed twice
	goto error
)

:skipbuild
if exist ..\skiptests goto skiptests

echo RUN_TESTS: %DATE% %TIME%

reg add "HKCU\Software\Microsoft\Windows\Windows Error Reporting" /v DontShow /t REG_DWORD /d 1 /f

set oldtemp=%TEMP%
set oldtmp=%TMP%
set oldpath=%PATH%

set TEMP=%TEMP%\%PACKAGENAME%-%ARCH%
set TMP=%TEMP%
if exist "%TEMP%" rmdir /s /q "%TEMP%"
mkdir "%TEMP%"

for %%g IN (%GRASS_VERSIONS%) do (
	set path=!path!;%OSGEO4W_ROOT%\apps\grass\%%g\lib
	set GISBASE=%OSGEO4W_ROOT%\apps\grass\%%g
)
PATH %path%;%BUILDDIR%\output\plugins
set QT_PLUGIN_PATH=%BUILDDIR%\output\plugins;%OSGEO4W_ROOT%\apps\qt5\plugins

cmake --build %BUILDDIR% --target NightlyTest --config %BUILDCONF%
if errorlevel 1 echo TESTS WERE NOT SUCCESSFUL.

:skiptests

set TEMP=%oldtemp%
set TMP=%oldtmp%
PATH %oldpath%

cmake --build %BUILDDIR% --target NightlySubmit --config %BUILDCONF%
if errorlevel 1 echo TEST SUBMISSION WAS NOT SUCCESSFUL.

if exist "%PKGDIR%" (
	echo REMOVE: %DATE% %TIME%
	rmdir /s /q "%PKGDIR%"
)

echo INSTALL: %DATE% %TIME%
cmake --build %BUILDDIR% --target install --config %BUILDCONF%
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
	for /f "usebackq tokens=1" %%a in (`%%g --config version`) do set gv=%%a
	for /F "delims=." %%i in ("!gv!") do set v=%%i

	sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' -e 's/@grasspath@/%%g/g' -e 's/@grassversion@/!gv!/g' qgis-grass.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%-g!v!.bat.tmpl
	if errorlevel 1 (echo creation of desktop template failed & goto error)
	set batches=!batches! bin/%PACKAGENAME%-g!v!.bat.tmpl
)

sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' python.bat.tmpl >%OSGEO4W_ROOT%\bin\python-%PACKAGENAME%.bat.tmpl
if errorlevel 1 (echo creation of python wrapper template failed & goto error)


touch exclude
if exist ..\skipbuild (echo skip build & goto skipbuild)

move %PKGDIR%\bin\qgis.exe %OSGEO4W_ROOT%\bin\%PACKAGENAME%-bin.exe
if errorlevel 1 (echo move of desktop executable failed & goto error)
copy qgis.vars %OSGEO4W_ROOT%\bin\%PACKAGENAME%-bin.vars
if errorlevel 1 (echo copy of desktop executable vars failed & goto error)

if not exist %PKGDIR%\qtplugins\sqldrivers mkdir %PKGDIR%\qtplugins\sqldrivers
move %OSGEO4W_ROOT%\apps\qt5\plugins\sqldrivers\qsqlocispatial.dll %PKGDIR%\qtplugins\sqldrivers
if errorlevel 1 (echo move of oci sqldriver failed & goto error)
move %OSGEO4W_ROOT%\apps\qt5\plugins\sqldrivers\qsqlspatialite.dll %PKGDIR%\qtplugins\sqldrivers
if errorlevel 1 (echo move of spatialite sqldriver failed & goto error)

if not exist %PKGDIR%\qtplugins\designer mkdir %PKGDIR%\qtplugins\designer
move %OSGEO4W_ROOT%\apps\qt5\plugins\designer\qgis_customwidgets.dll %PKGDIR%\qtplugins\designer
if errorlevel 1 (echo move of customwidgets failed & goto error)

if not exist %PKGDIR%\python\PyQt5\uic\widget-plugins mkdir %PKGDIR%\python\PyQt5\uic\widget-plugins
move %PYTHONHOME%\Lib\site-packages\PyQt5\uic\widget-plugins\qgis_customwidgets.py %PKGDIR%\python\PyQt5\uic\widget-plugins
if errorlevel 1 (echo move of customwidgets binding failed & goto error)

if not exist %ARCH%\release\qgis\%PACKAGENAME% mkdir %ARCH%\release\qgis\%PACKAGENAME%
%TAR% -C %OSGEO4W_ROOT% -cjf %ARCH%/release/qgis/%PACKAGENAME%/%PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	apps/%PACKAGENAME% ^
	bin/%PACKAGENAME%-bin.exe ^
	bin/%PACKAGENAME%-bin.vars ^
	%batches% ^
	bin/%PACKAGENAME%-designer.bat.tmpl ^
	bin/python-%PACKAGENAME%.bat.tmpl ^
	etc/postinstall/%PACKAGENAME%.bat ^
	etc/preremove/%PACKAGENAME%.bat
if errorlevel 1 (echo tar failed & goto error)

if not exist %ARCH%\release\qgis\%PACKAGENAME%-pdb mkdir %ARCH%\release\qgis\%PACKAGENAME%-pdb
%TAR% -C %BUILDDIR% -cjf %ARCH%/release/qgis/%PACKAGENAME%-pdb/%PACKAGENAME%-pdb-%VERSION%-%PACKAGE%.tar.bz2 ^
	apps/%PACKAGENAME%/pdb
if errorlevel 1 (echo tar failed & goto error)

goto end

:usage
echo usage: %0 version package packagename arch [sha [site]]
echo sample: %0 2.11.0 38 qgis-dev x86_64 339dbf1 qgis.org
exit /b 1

:error
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%
if exist %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2
exit /b 1

:end
echo FINISHED: %DATE% %TIME%

endlocal
