@echo off
set PYUIC4=%1
set PATH=%2;%PATH%
set PYTHONPATH=%3;%PYTHONPATH%
%PYUIC4% %4 %5 %6 %7 %8 %9
