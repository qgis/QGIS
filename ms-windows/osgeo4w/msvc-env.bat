REM @echo off
REM ***************************************************************************
REM    msvc-env.cmd
REM    ---------------------
REM    begin                : June 2018
REM    copyright            : (C) 2018 by Juergen E. Fischer
REM    email                : jef at norbit dot de
REM ***************************************************************************
REM *                                                                         *
REM *   This program is free software; you can redistribute it and/or modify  *
REM *   it under the terms of the GNU General Public License as published by  *
REM *   the Free Software Foundation; either version 2 of the License, or     *
REM *   (at your option) any later version.                                   *
REM *                                                                         *
REM ***************************************************************************

if defined PROGRAMFILES(X86) set PF86=%PROGRAMFILES(X86)%
if not defined PF86 set PF86=%PROGRAMFILES%
if not defined PF86 (echo PROGRAMFILES not set & goto error)

if not defined VCSDK set VCSDK=10.0.19041.0

set VCARCH=amd64
set VCARCH=amd64
set CMAKE_COMPILER_PATH=%PF86%\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64
set SETUPAPI_LIBRARY=%PF86%\Windows Kits\10\Lib\%VCSDK%\um\x64\SetupAPI.Lib
set DBGHLP_PATH=%PF86%\Windows Kits\10\Debuggers\x64

if not exist "%SETUPAPI_LIBRARY%" (
  echo SETUPAPI_LIBRARY not found
  dir /s /b "%PF86%\setupapi.lib"
  goto error
)

if not exist "%DBGHLP_PATH%\dbghelp.dll" (
  echo dbghelp.dll not found
  dir /s /b "%PF86%\dbghelp.dll" "%PF86%\symsrv.dll"
  goto error
)

if not defined CC set CC=cl.exe
if not defined CXX set CXX=cl.exe
if not defined OSGEO4W_ROOT set OSGEO4W_ROOT=C:\OSGeo4W

if not exist "%OSGEO4W_ROOT%\bin\o4w_env.bat" (echo o4w_env.bat not found & goto error)
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

for %%e in (Community Professional Enterprise) do if exist "%PF86%\Microsoft Visual Studio\2019\%%e" set vcdir=%PF86%\Microsoft Visual Studio\2019\%%e
if not defined vcdir (echo Visual C++ not found & goto error)

set VS160COMNTOOLS=%vcdir%\Common7\Tools
call "%vcdir%\VC\Auxiliary\Build\vcvarsall.bat" %VCARCH% %VCSDK%
path %path%;%vcdir%\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64
path %path%;C:\OSGeo4W\apps\Qt5\bin;C:\OSGeo4W\apps\gdal-dev\bin;C:\OSGeo4W\apps\proj-dev\bin

set GRASS=
if exist %OSGEO4W_ROOT%\bin\grass78.bat set GRASS=%OSGEO4W_ROOT%\bin\grass78.bat
if not defined GRASS (echo GRASS not found & goto error)
for /f "usebackq tokens=1" %%a in (`%GRASS% --config path`) do set GRASS_PREFIX=%%a

set PYTHONPATH=
if exist "%PROGRAMFILES%\CMake\bin" path %PROGRAMFILES%\CMake\bin;%PATH%
if exist "%PF86%\CMake\bin" path %PF86%\CMake\bin;%PATH%
if exist c:\cygwin64\bin path %PATH%;c:\cygwin64\bin
if exist c:\cygwin\bin path %PATH%;c:\cygwin\bin

set LIB=%LIB%;%OSGEO4W_ROOT%\apps\Qt5\lib;%OSGEO4W_ROOT%\lib
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\apps\Qt5\include;%OSGEO4W_ROOT%\include

goto end

:usage
echo usage: %0
exit /b 1

:error
echo ENV ERROR %ERRORLEVEL%: %DATE% %TIME%
exit /b 1

:end
