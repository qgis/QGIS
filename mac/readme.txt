Mac Notes

The 'cmake' folder scripts handle bundling dependent libraries in the QGIS
application package and fixing up the library paths.  It is automatic during
installation.  There are 2 levels currently, specified with the cmake config
option QGIS_MACAPP_BUNDLE, and one that always occurs:

0 = (default) fixup the library paths for all QGIS libraries if @loader_path
    is available in the system (OS X 10.5+)
1 = bundle Qt, PyQt, PyQwt and OSG/osgEarth
2 = additionally, bundle libraries, but not frameworks

A third level that is not finished will additionally bundle frameworks.
This would create the "standalone" QGIS.

There is also a configure option to set a user bundle script,
QGIS_MACAPP_BUNDLE_USER.  This specifies the path to a cmake bundle script
similar to the built-in bundle scripts for the defined levels. This script is
always run independent of and after the QGIS_MACAPP_BUNDLE level specified.
