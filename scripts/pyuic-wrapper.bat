@echo off
set PYUIC4=%1
set PATH=%2;%PATH%
set PYTHONPATH=%3;%PYTHONPATH%
set PYTHON=%4
rem %PYTHON% %~dp0\pyuic-wrapper.py %5 %6 %7 %8 %9
%PYTHON% "C:\OSGeo4W64\apps\Python27\lib\site-packages\PyQt4\uic\pyuic.py" %5 %6 %7 %8 %9