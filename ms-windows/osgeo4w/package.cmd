@echo off
set GRASS_VERSION=6.4.0
set SVNVERSION=c:/cygwin/bin/svnversion

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
if "%PACKAGENAME%"=="" set PACKAGENAME=qgis-dev

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

cmake -G "Visual Studio 9 2008" ^
	-D PEDANTIC=TRUE ^
	-D WITH_SPATIALITE=TRUE ^
	-D WITH_MAPSERVER=TRUE ^
	-D WITH_INTERNAL_SPATIALITE=TRUE ^
	-D BINDINGS_GLOBAL_INSTALL=FALSE ^
	-D CMAKE_BUILD_TYPE=%BUILDCONF% ^
	-D CMAKE_CONFIGURATION_TYPES=%BUILDCONF% ^
	-D GEOS_LIBRARY=%OSGEO4W_ROOT%/lib/geos_c_i.lib ^
	-D PYTHON_EXECUTABLE=%O4W_ROOT%/bin/python.exe ^
	-D PYTHON_INCLUDE_PATH=%O4W_ROOT%/apps/Python25/include ^
	-D PYTHON_LIBRARY=%O4W_ROOT%/apps/Python25/libs/python25.lib ^
	-D SIP_BINARY_PATH=%O4W_ROOT%/apps/Python25/sip.exe ^
	-D GRASS_PREFIX=%O4W_ROOT%/apps/grass/grass-%GRASS_VERSION% ^
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
	-D SVNVERSION="%SVNVERSION%" ^
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

echo PACKAGE: %DATE% %TIME%>>%LOG% 2>&1

cd ..
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' postinstall.bat >%OSGEO4W_ROOT%\etc\postinstall\%PACKAGENAME%.bat
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' preremove.bat >%OSGEO4W_ROOT%\etc\preremove\%PACKAGENAME%.bat
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' qgis.bat.tmpl >%OSGEO4W_ROOT%\bin\%PACKAGENAME%.bat.tmpl
sed -e 's/@package@/%PACKAGENAME%/g' -e 's/@version@/%VERSION%/g' qgis.reg.tmpl >%OSGEO4W_ROOT%\apps\%PACKAGENAME%\bin\qgis.reg.tmpl

REM sed -e 's/%OSGEO4W_ROOT:\=\\\\\\\\%/@osgeo4w@/' %OSGEO4W_ROOT%\apps\%PACKAGENAME%\python\qgis\qgisconfig.py >%OSGEO4W_ROOT%\apps\%PACKAGENAME%\python\qgis\qgisconfig.py.tmpl
REM if errorlevel 1 goto error

REM del %OSGEO4W_ROOT%\apps\%PACKAGENAME%\python\qgis\qgisconfig.py

touch exclude

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	--exclude "apps/%PACKAGENAME%/themes/classic/grass" ^
	--exclude "apps/%PACKAGENAME%/themes/default/grass" ^
	--exclude "apps/%PACKAGENAME%/themes/qgis/grass" ^
	--exclude "apps/%PACKAGENAME%/grass" ^
	--exclude "apps/%PACKAGENAME%/bin/qgisgrass.dll" ^
	--exclude "apps/%PACKAGENAME%/plugins/grassrasterprovider.dll" ^
	--exclude "apps/%PACKAGENAME%/plugins/grassplugin.dll" ^
	--exclude "apps/%PACKAGENAME%/plugins/grassprovider.dll" ^
	apps/%PACKAGENAME% ^
	bin/%PACKAGENAME%.bat.tmpl ^
	etc/postinstall/%PACKAGENAME%.bat ^
	etc/preremove/%PACKAGENAME%.bat ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

tar -C %OSGEO4W_ROOT% -cjf %PACKAGENAME%-grass-plugin-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude-from exclude ^
	--exclude "*.pyc" ^
	"apps/%PACKAGENAME%/themes/classic/grass" ^
	"apps/%PACKAGENAME%/themes/default/grass" ^
	"apps/%PACKAGENAME%/themes/gis/grass" ^
	"apps/%PACKAGENAME%/grass" ^
	"apps/%PACKAGENAME%/bin/qgisgrass.dll" ^
	"apps/%PACKAGENAME%/plugins/grassrasterprovider.dll" ^
	"apps/%PACKAGENAME%/plugins/grassplugin.dll" ^
	"apps/%PACKAGENAME%/plugins/grassprovider.dll" ^
	>>%LOG% 2>&1
if errorlevel 1 goto error

goto end

:error
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%>>%LOG% 2>&1
if exist %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-%VERSION%-%PACKAGE%.tar.bz2
if exist %PACKAGENAME%-grass-plugin-%VERSION%-%PACKAGE%.tar.bz2 del %PACKAGENAME%-grass-plugin-%VERSION%-%PACKAGE%.tar.bz2

:end
echo FINISHED: %DATE% %TIME% >>%LOG% 2>&1
