@echo off
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

if not "%PROGRAMFILES(X86)%"=="" set PF86=%PROGRAMFILES(X86)%
if "%PF86%"=="" set PF86=%PROGRAMFILES%
if "%PF86%"=="" (echo PROGRAMFILES not set & goto error)

if "%VCSDK%"=="" set VCSDK=10.0.18362.0

set ARCH=%1
if "%ARCH%"=="x86" goto x86
if "%ARCH%"=="x86_64" goto x86_64
goto usage

:x86
set VCARCH=x86
set SETUPAPI_LIBRARY=%PF86%\Windows Kits\10\Lib\%VCSDK%\um\x86\SetupAPI.Lib
set DBGHLP_PATH=%PF86%\Windows Kits\10\Debuggers\x86
goto archset

:x86_64
set VCARCH=amd64
set SETUPAPI_LIBRARY=%PF86%\Windows Kits\10\Lib\%VCSDK%\um\x64\SetupAPI.Lib
set DBGHLP_PATH=%PF86%\Windows Kits\10\Debuggers\x64

:archset
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

if "%CC%"=="" set CC=cl.exe
if "%CXX%"=="" set CXX=cl.exe

if "%OSGEO4W_ROOT%"=="" (
	if "%ARCH%"=="x86" (
		set OSGEO4W_ROOT=C:\OSGeo4W
	) else (
		set OSGEO4W_ROOT=C:\OSGeo4W64
	)
)

if not exist "%OSGEO4W_ROOT%\bin\o4w_env.bat" (echo o4w_env.bat not found & goto error)
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call "%OSGEO4W_ROOT%\bin\py3_env.bat"
call "%OSGEO4W_ROOT%\bin\qt5_env.bat"

for %%e in (Community Professional Enterprise) do if exist "%PF86%\Microsoft Visual Studio\2019\%%e" set vcdir=%PF86%\Microsoft Visual Studio\2019\%%e
if "%vcdir%"=="" (echo Visual C++ not found & goto error)

set VS160COMNTOOLS=%vcdir%\Common7\Tools
call "%vcdir%\VC\Auxiliary\Build\vcvarsall.bat" %VCARCH%
path %path%;%vcdir%\VC\bin

set GRASS7=
if exist %OSGEO4W_ROOT%\bin\grass74.bat set GRASS7=%OSGEO4W_ROOT%\bin\grass74.bat
if exist %OSGEO4W_ROOT%\bin\grass76.bat set GRASS7=%OSGEO4W_ROOT%\bin\grass76.bat
if exist %OSGEO4W_ROOT%\bin\grass78.bat set GRASS7=%OSGEO4W_ROOT%\bin\grass78.bat
if "%GRASS7%"=="" (echo GRASS7 not found & goto error)
for /f "usebackq tokens=1" %%a in (`%GRASS7% --config path`) do set GRASS_PREFIX=%%a

set PYTHONPATH=
if exist "%PROGRAMFILES%\CMake\bin" path %PATH%;%PROGRAMFILES%\CMake\bin
if exist "%PF86%\CMake\bin" path %PATH%;%PF86%\CMake\bin
if exist c:\cygwin64\bin path %PATH%;c:\cygwin64\bin
if exist c:\cygwin\bin path %PATH%;c:\cygwin\bin

set LIB=%LIB%;%OSGEO4W_ROOT%\apps\Qt5\lib;%OSGEO4W_ROOT%\lib
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\apps\Qt5\include;%OSGEO4W_ROOT%\include

goto end

:usage
echo usage: %0 [x86^|x86_64]
echo sample: %0 x86_64
exit /b 1

:error
echo ENV ERROR %ERRORLEVEL%: %DATE% %TIME%
exit /b 1

:end
