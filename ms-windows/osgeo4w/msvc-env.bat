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

set ARCH=%1
if not "%ARCH%"=="x86" if not "%ARCH%"=="x86_64" (
	goto usage
)

if "%OSGEO4W_ROOT%"=="" (
	if "%ARCH%"=="x86" (
		set OSGEO4W_ROOT=C:\OSGeo4W
		set VCARCH=x86
	) else (
		set OSGEO4W_ROOT=C:\OSGeo4W64
		set VCARCH=amd64
	)
)

if not exist "%OSGEO4W_ROOT%\bin\o4w_env.bat" (echo o4w_env.bat not found & goto error)
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call "%OSGEO4W_ROOT%\bin\py3_env.bat"
call "%OSGEO4W_ROOT%\bin\qt5_env.bat"

if not "%PROGRAMFILES(X86)%"=="" set PF86=%PROGRAMFILES(X86)%
if "%PF86%"=="" set PF86=%PROGRAMFILES%
if "%PF86%"=="" (echo PROGRAMFILES not set & goto error)

set VS140COMNTOOLS=%PF86%\Microsoft Visual Studio 14.0\Common7\Tools\
call "%PF86%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %VCARCH%
path %path%;%PF86%\Microsoft Visual Studio 14.0\VC\bin

set GRASS7=
if exist %OSGEO4W_ROOT%\bin\grass72.bat set GRASS7=%OSGEO4W_ROOT%\bin\grass72.bat
if exist %OSGEO4W_ROOT%\bin\grass74.bat set GRASS7=%OSGEO4W_ROOT%\bin\grass74.bat
if "%GRASS7%"=="" (echo GRASS7 not found & goto error)
for /f "usebackq tokens=1" %%a in (`%GRASS7% --config path`) do set GRASS_PREFIX=%%a

set PYTHONPATH=
if exist "%PROGRAMFILES%\CMake\bin" path %PATH%;%PROGRAMFILES%\CMake\bin
if exist "%PF86%\CMake\bin" path %PATH%;%PF86%\CMake\bin
if exist c:\cygwin64\bin path %PATH%;c:\cygwin64\bin
if exist c:\cygwin\bin path %PATH%;c:\cygwin\bin
path

set LIB=%LIB%;%OSGEO4W_ROOT%\apps\Qt5\lib;%OSGEO4W_ROOT%\lib
set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\apps\Qt5\include;%OSGEO4W_ROOT%\include

goto end

:usage
echo usage: %0 arch
echo sample: %0 x86_64
exit /b 1

:error
echo ENV ERROR %ERRORLEVEL%: %DATE% %TIME%
exit /b 1

:end
