@echo off
call %OSGEO4W_ROOT%\bin\o4w_env.bat
call py3_env.bat
call qt5_env.bat
call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
path %PATH%;c:\cygwin\bin;c:\program files\cmake\bin
%OSGEO4W_ROOT%\bin\ninja -j4 -C ..\build-qgis-dev-x86_64
