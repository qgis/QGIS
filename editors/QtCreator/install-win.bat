@ECHO OFF
SET CREATOR=%APPDATA%\QtProject\qtcreator
SET WIZARDS=%CREATOR%\templates\wizards
SET CODESTYLES=%CREATOR%\codestyles\cpp
IF NOT EXIST "%WIZARDS%" (
   ECHO Making wizard folder..
   mkdir "%WIZARDS%"
)

IF NOT EXIST "%WIZARDS%\qgis" (
  ECHO Linking code wizards from source..
  mklink /J  "%WIZARDS%\qgis" %CD%\templates\wizards\qgis
) ELSE (
  ECHO Wizard folders already linked
)

IF NOT EXIST "%CODESTYLES%" (
   ECHO Making code style folder..
   mkdir "%CODESTYLES%"
)

IF NOT EXIST "%CODESTYLES%\qgis_code_style.xml" (
  ECHO Linking code styles from source..
  mklink "%CODESTYLES%\qgis_code_style.xml" %CD%\qgis_code_style.xml
) ELSE (
  ECHO Code styles already linked..
)

pause
