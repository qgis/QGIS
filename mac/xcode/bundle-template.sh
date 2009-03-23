#!/bin/sh

# bundle-template.sh
# Qgis
#
# Created by William Kyngesburye on 2009-03-22.

# Extra bundling operations.  Use for extras that are not a part of the
# standard Qgis build.  For example, PyQt extras like Qwt5.

# To use, duplicate this file with the name 'bundle.sh'.  It will not be
# overwritten in an update so you can use it for future Qgis versions.
# Just make sure that what you do here doesn't conflict with updates.
# And make sure to check this template for any changes.

# Qgis paths to use:

# bin: "$QGIS_BUILD_PATH/MacOS/$QGIS_BIN_SUBDIR/"
# lib: "$QGIS_BUILD_PATH/MacOS/$QGIS_LIB_SUBDIR/"
# plugins: "$QGIS_BUILD_PATH/MacOS/$QGIS_PLUGIN_SUBDIR/"
# data: "$QGIS_BUILD_PATH/MacOS/$QGIS_DATA_SUBDIR/"
# frameworks: "$QGIS_BUILD_PATH/Frameworks"

# If Qt components are used in any bundled extras, they must be processed
# with install_name_tool to use the bundled Qt frameworks.
# Do this *after* copying.

#for q in $QTLISTQG
#do
#	qq="$q.framework/Versions/$QT_FWVER/$q"
#	install_name_tool -change $qq @executable_path/$QGIS_FW_SUBDIR/$qq \
#	"/path/to/bundled/binary"
#done

# If a library or framework is bundled along with a plugin or python module
# that uses it, install_name_tool must be used on the plugin or module
# so that it can find the library.  Everything is relative to the Qgis
# executable.  ie:

# install_name_tool -change /current/library/path \
# @executable_path/$QGIS_LIB_SUBDIR/bundled_lib_name

# The current library path can be found in the script with (fill in the
# file basename for 'libname'):

# currlib=`otool -L "/path/to/libname.dylib" | grep -E -m 1 "libname.+ " | cut -d \( -f 1 | sed -E -e 's/^[[:space:]]//' -e 's/[[:space:]]$//'`

# See the Qgis target bundle Qt and bundle extras script phases for examples.

##### Put your bundling command here: #####


