@ECHO OFF
SET CREATOR=%APPDATA%\QtProject\qtcreator
SET WIZARDS=%CREATOR%\templates\wizards
SET CODESTYLES=%CREATOR%\codestyles\cpp
IF NOT EXIST "%WIZARDS%" (
   mkdir "%WIZARDS%"
)

IF NOT EXIST "%WIZARDS%\qgis" (
  mklink /J  "%WIZARDS%\qgis" %CD%\templates\wizards\qgis
)

IF NOT EXIST "%CODESTYLES%" (
   mkdir "%CODESTYLES%"
)

IF NOT EXIST "%CODESTYLES%\qgis_code_style.xml" (
  mklink "%CODESTYLES%\qgis_code_style.xml" %CD%\qgis_code_style.xml
)
