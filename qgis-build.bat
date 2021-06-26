@echo off

set VS150COMNTOOLS =D:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools
call "D:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64

set INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include
set LIB=%LIB%;C:\Program Files (x86)\Windows Kits\10\Lib

set OSGEO4W_ROOT=C:\OSGeo4W64
 
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call "%OSGEO4W_ROOT%\bin\py3_env.bat"
call "%OSGEO4W_ROOT%\bin\qt5_env.bat"

set O4W_ROOT=%OSGEO4W_ROOT:\=/%
set LIB_DIR=%O4W_ROOT%

echo %LIB_DIR%

path %PATH%;C:\Program Files\CMake\bin
 
@set GRASS_PREFIX=%OSGEO4W_ROOT%/apps/grass/grass78

@set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
@set LIB=%LIB%;%OSGEO4W_ROOT%\lib;%OSGEO4W_ROOT%\lib
 
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\apps\Qt5\include;%OSGEO4W_ROOT%\include
set LIB=%LIB%;%OSGEO4W_ROOT%\apps\Qt5\lib;%OSGEO4W_ROOT%\lib
 
@cmd
