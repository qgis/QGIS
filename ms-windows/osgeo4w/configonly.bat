@echo off
REM ***************************************************************************
REM    configonly.cmd
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
if "%ARCH%"=="x86" (
	set CMAKEGEN=Visual Studio 14 2015
) else (
	set CMAKEGEN=Visual Studio 14 2015 Win64
	set ARCH=x86_64
)

set CONFIGONLY=1

setlocal enabledelayedexpansion

for /f "tokens=*" %%L in (..\..\CMakeLists.txt) do (
        set L=%%L
        set V=!L:SET(CPACK_PACKAGE_VERSION_=!
        if not !V!==!L! (
                set V=!V:"=!
                set V=!V:^)=!
                set _major=!V:MAJOR =!
                set _minor=!V:MINOR =!
                set _patch=!V:PATCH =!
                if not !_major!==!V! set MAJOR=!_major!
                if not !_minor!==!V! set MINOR=!_minor!
                if not !_patch!==!V! set PATCH=!_patch!
        )
)

package-nightly.cmd %MAJOR%.%MINOR%.%PATCH% 99 qgis-test %ARCH%

endlocal
