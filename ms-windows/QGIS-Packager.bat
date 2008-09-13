@echo off

rem ----------------------------------------------------------------------------------------------------------
rem Set the script variables; change the absolute paths if needed
rem ----------------------------------------------------------------------------------------------------------

set GRASS_PREFIX=c:\msys\local\grass-6.3.0
set PYTHON_PREFIX=c:\DevTools\Python
set QT_PREFIX=c:\DevTools\Qt-OpenSource

set QGIS_DEV_PACKAGE_DIR=.\QGIS-Dev-Package
set QGIS_RELEASE_PACKAGE_DIR=.\QGIS-Release-Package

set QGIS_DEV_INSTALL_FOLDER=c:\msys\local\qgis-dev
set QGIS_RELEASE_INSTALL_FOLDER=c:\msys\local\qgis-0.11.0

rem ----------------------------------------------------------------------------------------------------------
rem  Do not modify the following lines
rem ----------------------------------------------------------------------------------------------------------

@echo -------------------------------------------------------------------------------------------------------
@echo Self Contained Quantum GIS + GRASS Automated Packager
@echo -------------------------------------------------------------------------------------------------------
@echo Quantum GIS Version: Current SVN Development or Release Version
@echo GRASS Version: 6.3.0
@echo.
@echo Edited by: Marco Pasetti
@echo Last Update: 21 August 2008
@echo -------------------------------------------------------------------------------------------------------
@echo.
@echo Select if you want to create a Development Package from the Current Development Trunk Build
@echo or a Release Package from the Current Release Branch Build
@echo.
@echo 1. Current QGIS Development Trunk Build
@echo.
@echo 2. Current QGIS Release Branch Build
@echo.

set /p UPDATE_TYPE=Enter your selection (1/2):

if %UPDATE_TYPE%==1 (
set PACKAGE_DIR=%QGIS_DEV_PACKAGE_DIR%
set QGIS_PREFIX=%QGIS_DEV_INSTALL_FOLDER%
)

if %UPDATE_TYPE%==2 (
set PACKAGE_DIR=%QGIS_RELEASE_PACKAGE_DIR%
set QGIS_PREFIX=%QGIS_RELEASE_INSTALL_FOLDER%
)

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Remove the previous SVN Selected Package and create a new package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

pause

if exist %PACKAGE_DIR% rmdir /S/Q %PACKAGE_DIR%
mkdir %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the QGIS build install content to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

xcopy %QGIS_PREFIX% %PACKAGE_DIR% /S/V/F

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the Python Dynamic Library to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

copy C:\WINDOWS\system32\python25.dll %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the Qt-OpenSource Dynamic Libraries to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

copy %QT_PREFIX%\bin\mingwm10.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\Qt3Support4.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\QtCore4.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\QtGui4.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\QtNetwork4.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\QtSql4.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\QtXml4.dll %PACKAGE_DIR%
copy %QT_PREFIX%\bin\QtSvg4.dll %PACKAGE_DIR%

mkdir %PACKAGE_DIR%\plugins\imageformats
copy %QT_PREFIX%\plugins\imageformats\*.dll %PACKAGE_DIR%\plugins\imageformats

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the Python Files to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

xcopy %PYTHON_PREFIX%\sip\PyQt4 %PACKAGE_DIR%\python\PyQt4 /S/V/F/I

xcopy %PYTHON_PREFIX%\Lib\site-packages\PyQt4 %PACKAGE_DIR%\python\PyQt4 /S/V/F/I

copy %PYTHON_PREFIX%\Lib\site-packages\*.py %PACKAGE_DIR%\python
copy %PYTHON_PREFIX%\Lib\site-packages\*.pyd %PACKAGE_DIR%\python
copy %PYTHON_PREFIX%\Lib\site-packages\*.pth %PACKAGE_DIR%\python
copy %PYTHON_PREFIX%\Lib\site-packages\*.pyc %PACKAGE_DIR%\python
copy %PYTHON_PREFIX%\Lib\site-packages\*.pyo %PACKAGE_DIR%\python

copy %PYTHON_PREFIX%\Lib\* %PACKAGE_DIR%\python

xcopy %PYTHON_PREFIX%\Lib\bsddb %PACKAGE_DIR%\python\bsddb /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\compiler %PACKAGE_DIR%\python\compiler /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\ctypes %PACKAGE_DIR%\python\ctypes /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\curses %PACKAGE_DIR%\python\curses /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\distutils %PACKAGE_DIR%\python\distutils /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\email %PACKAGE_DIR%\python\email /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\encodings %PACKAGE_DIR%\python\encodings /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\hotshot %PACKAGE_DIR%\python\hotshot /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\idlelib %PACKAGE_DIR%\python\idlelib /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\lib-tk %PACKAGE_DIR%\python\lib-tk /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\logging %PACKAGE_DIR%\python\logging /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\msilib %PACKAGE_DIR%\python\msilib /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\sqlite3 %PACKAGE_DIR%\python\sqlite3 /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\wsgiref %PACKAGE_DIR%\python\wsgiref /S/V/F/I
xcopy %PYTHON_PREFIX%\Lib\xml %PACKAGE_DIR%\python\xml /S/V/F/I

copy %PYTHON_PREFIX%\DLLs\*.pyd %PACKAGE_DIR%\python

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the GRASS build install content to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

xcopy %GRASS_PREFIX% %PACKAGE_DIR%\grass /S/V/F

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Move the GRASS dynamic libraries from %PACKAGE_DIR%\grass\lib to %PACKAGE_DIR%
@echo -------------------------------------------------------------------------------------------------------
@echo.

move %PACKAGE_DIR%\grass\lib\*.dll %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the Extra-libraries to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

copy C:\msys\local\bin\*.dll %PACKAGE_DIR%
copy C:\msys\local\pgsql\lib\libpq.dll %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the AVCE00 and E00compr binaries to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

copy C:\msys\local\bin\avcexport.exe %PACKAGE_DIR%
copy C:\msys\local\bin\avcimport.exe %PACKAGE_DIR%
copy C:\msys\local\bin\e00conv.exe %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the SQLite dynamic library to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

copy C:\msys\local\sqlite\bin\*.dll %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the GPSBABEL executable and dll to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

copy c:\msys\local\gpsbabel\gpsbabel.exe %PACKAGE_DIR%
copy c:\msys\local\gpsbabel\libexpat.dll %PACKAGE_DIR%

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the shared PROJ.4 files to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

xcopy C:\msys\local\share\proj %PACKAGE_DIR%\grass\proj /S/V/F/I

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Copy the MSYS files to the package folder
@echo -------------------------------------------------------------------------------------------------------
@echo.

mkdir %PACKAGE_DIR%\msys

copy c:\msys\* %PACKAGE_DIR%\msys

xcopy c:\msys\bin %PACKAGE_DIR%\msys\bin /S/V/F/I
xcopy c:\msys\doc %PACKAGE_DIR%\msys\doc /S/V/F/I
xcopy c:\msys\etc %PACKAGE_DIR%\msys\etc /S/V/F/I
xcopy c:\msys\info %PACKAGE_DIR%\msys\info /S/V/F/I
xcopy c:\msys\lib %PACKAGE_DIR%\msys\lib /S/V/F/I
xcopy c:\msys\man %PACKAGE_DIR%\msys\man /S/V/F/I

@echo.
@echo -------------------------------------------------------------------------------------------------------
@echo Packaging Completed
@echo -------------------------------------------------------------------------------------------------------
@echo.
pause