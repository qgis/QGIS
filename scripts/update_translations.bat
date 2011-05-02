echo Creating qmake project file
qmake -project -o qgis_ts.pro
echo Updating translation files
lupdate -verbose qgis_ts.pro
lrelease -verbose qgis_ts.pro
echo Removing qmake project file
del qgis_ts.pro
copy i18n\*.qm qgis-release\share\qgis\i18n\
