textreplace -std ^
	-map @windir@ "%WINDIR%" ^
	-map @temp@ "%TEMP%" ^
	-map @GDAL_DRIVER_PATH@ "%GDAL_DRIVER_PATH%" ^
	-map @GDAL_DATA@ "%GDAL_DATA%" ^
	-map @PROJ_LIB@ "%PROJ_LIB%" ^
	-map @addpath@ "" ^
	-t httpd.d\httpd_@package@.conf
del httpd.d\httpd_@package@.conf.tmpl
