Mac Notes

The 'cmake' folder scripts handle bundling dependent libraries in the QGIS
application package and fixing up the library paths.  It is automatic during
installation.  There are 2 levels currently, specified with the cmake config
option QGIS_MACAPP_BUNDLE, and one that always occurs:

0 = (default) fixup the library paths for all QGIS libraries if @loader_path
    is available in the system (OS X 10.5+)
1 = bundle Qt, PyQt and PyQwt
2 = additionally, bundle libraries, but not frameworks

A third level that is not finished will additionally bundle frameworks.
This would create the "standalone" QGIS.


The 'extras' folder scripts are for packaging the old v1.0 build.  All the
bundling is now handled in the cmake install, so these scripts are
essentially deprecated.


The Xcode project is now deprecated since the bundling now happens in the
cmake build, including making sure library paths are all correct.
