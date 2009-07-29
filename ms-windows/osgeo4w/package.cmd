@echo off
set GRASS_VERSION=6.4.0svn

path %SYSTEMROOT%\system32;%SYSTEMROOT%;%SYSTEMROOT%\System32\Wbem;%PROGRAMFILES%\CMake 2.6\bin
set PYTHONPATH=

set VS90COMNTOOLS=%PROGRAMFILES%\Microsoft Visual Studio 9.0\Common7\Tools\
call "%PROGRAMFILES%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86

set OSGEO4W_ROOT=%PROGRAMFILES%\OSGeo4W
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

set LIB_DIR=%OSGEO4W_ROOT%

set FLEX=%PROGRAMFILES%\GnuWin32\bin\flex.exe
set BISON=%PROGRAMFILES%\GnuWin32\bin\bison.exe

set VERSION=%1
set PACKAGE=%2

PROMPT qgis%VERSION%$g 

set BUILDCONF=RelWithDebInfo
REM set BUILDCONF=Release

if not exist build mkdir build
if not exist build goto error

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
set LOG=%CD%\build.log

cd build

echo Logging to %LOG%
echo BEGIN: %DATE% %TIME%>>%LOG% 2>&1
if errorlevel 1 goto error

set >buildenv.log

if exist CMakeCache.txt goto skipcmake

echo CMAKE: %DATE% %TIME%>>%LOG% 2>&1
if errorlevel 1 goto error

cmake -G "Visual Studio 9 2008" ^
        -D PEDANTIC=TRUE ^
        -D WITH_SPATIALITE=TRUE ^
        -D WITH_INTERNAL_SPATIALITE=TRUE ^
	-D CMAKE_CONFIGURATION_TYPE=%BUILDCONF% ^
	-D CMAKE_BUILDCONFIGURATION_TYPES=%BUILDCONF% ^
	-D FLEX_EXECUTABLE=%FLEX% ^
	-D BISON_EXECUTABLE=%BISON% ^
	-D GDAL_INCLUDE_DIR=%OSGEO4W_ROOT%\apps\gdal-16\include -D GDAL_LIBRARY=%OSGEO4W_ROOT%\apps\gdal-16\lib\gdal_i.lib ^
	-D PYTHON_EXECUTABLE=%OSGEO4W_ROOT%\bin\python.exe ^
	-D PYTHON_INCLUDE_DIR=%OSGEO4W_ROOT%\apps\Python25\include -D PYTHON_LIBRARY=%OSGEO4W_ROOT%\apps\Python25\libs\python25.lib ^
	-D SIP_BINARY_PATH=%OSGEO4W_ROOT%\apps\Python25\sip.exe ^
	-D GRASS_PREFIX=%OSGEO4W_ROOT%\apps\grass\grass-%GRASS_VERSION% ^
	-D QT_BINARY_DIR=%OSGEO4W_ROOT%\bin -D QT_LIBRARY_DIR=%OSGEO4W_ROOT%\lib ^
	-D QT_HEADERS_DIR=%OSGEO4W_ROOT%\include\qt4 ^
	-D QT_ZLIB_LIBRARY=%OSGEO4W_ROOT%\lib\zlib.lib ^
	-D QT_PNG_LIBRARY=%OSGEO4W_ROOT%\lib\libpng13.lib ^
	-D CMAKE_INSTALL_PREFIX=%OSGEO4W_ROOT%\apps\qgis-dev ^
	../../..>>%LOG% 2>&1
if errorlevel 1 goto error

:skipcmake

echo ZERO_CHECK: %DATE% %TIME%>>%LOG% 2>&1
devenv qgis%VERSION%.sln /Project ZERO_CHECK /Build %BUILDCONF%>>%LOG% 2>&1
if errorlevel 1 goto error 

echo ALL_BUILD: %DATE% %TIME%>>%LOG% 2>&1
devenv qgis%VERSION%.sln /Project ALL_BUILD /Build %BUILDCONF%>>%LOG% 2>&1
if errorlevel 1 goto error 

echo INSTALL: %DATE% %TIME%>>%LOG% 2>&1
devenv qgis%VERSION%.sln /Project INSTALL /Build %BUILDCONF%>>%LOG% 2>&1
if errorlevel 1 goto error

echo PACKAGE: %DATE% %TIME%>>%LOG% 2>&1

cd ..
copy postinstall.bat %OSGEO4W_ROOT%\etc\postinstall\qgis-dev.bat
copy preremove.bat %OSGEO4W_ROOT%\etc\preremove\qgis-dev.bat
copy qgis-dev.bat.tmpl %OSGEO4W_ROOT%\bin\qgis-dev.bat.tmpl

tar -C %OSGEO4W_ROOT% -cjf qgis-dev-%VERSION%-%PACKAGE%.tar.bz2 ^
	--exclude "apps/qgis-dev/plugins/EDBSQuery.dll" ^
	apps/qgis-dev ^
	bin/qgis-dev.bat.tmpl ^
	etc/postinstall/qgis-dev.bat ^
	etc/preremove/qgis-dev.bat>>%LOG% 2>&1
if errorlevel 1 goto error

goto end

:error
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%
echo BUILD ERROR %ERRORLEVEL%: %DATE% %TIME%>>%LOG% 2>&1
if exist qgis-dev-%VERSION%-%PACKAGE%.tar.bz2 del qgis-dev-%VERSION%-%PACKAGE%.tar.bz2

:end
echo FINISHED: %DATE% %TIME% >>%LOG% 2>&1
