#!/bin/sh
# Copy supportibng libraries (except Qt) to openModeller bundle
# and make search paths for them relative to bundle


APP_PREFIX=/Applications/qgis0.11.0.app
BUNDLE_DIR=${APP_PREFIX}/Contents/MacOS
DEPS_BASE=/usr/local/qgis_universal_deps
LIB_DIR=${DEPS_BASE}/lib
#set -x
cp -r ${LIB_DIR}/libexpat.dylib  ${BUNDLE_DIR}/lib
cp -r ${LIB_DIR}/libgdal.1.dylib  ${BUNDLE_DIR}/lib
cp -r ${LIB_DIR}/libgeos.dylib  ${BUNDLE_DIR}/lib
cp -r ${LIB_DIR}/libproj.dylib  ${BUNDLE_DIR}/lib
cp -r ${LIB_DIR}/libgsl.dylib  ${BUNDLE_DIR}/lib
cp -r ${LIB_DIR}/libgslcblas.dylib  ${BUNDLE_DIR}/lib
#cp -r ${LIB_DIR}/libsqlite3.0.dylib  ${BUNDLE_DIR}
pushd .
cd ${BUNDLE_DIR}/lib
ln -s libgdal.1.dylib libgdal.dylib
ln -s libexpat.dylib libexpat.1.dylib
ln -s libgsl.dylib libgsl.0.dylib
ln -s libgslcblas.dylib libgslcblas.0.dylib
#ln -s libsqlite3.0.dylib libsqlite3.dylib
popd

set -x
install_name_tool -change ${LIB_DIR}/libgdal.1.dylib \
                   @executable_path/lib/libgdal.1.dylib \
                   ${BUNDLE_DIR}/qgis
install_name_tool -change ${LIB_DIR}/libproj.dylib \
                   @executable_path/lib/libproj.dylib \
                   ${BUNDLE_DIR}/qgis

set +x

LIBS="lib/libqgis_core.dylib \
      lib/libqgis_gui.dylib \
      lib/libexpat.1.dylib \
      lib/libgsl.0.dylib \
      lib/libgslcblas.0.dylib \
      lib/libexpat.dylib \
      lib/libgdal.dylib \
      lib/libgeos.dylib \
      lib/libproj.dylib"
#
# Next we work through all the libs except Qt libs 
# and update the library ids and paths for these libs
#
for LIB in $LIBS
do
  install_name_tool -id @executable_path/$LIB ${BUNDLE_DIR}/${LIB}
  #echo "install_name_tool -id @executable_path/$LIB ${BUNDLE_DIR}/${LIB}"
  # for debugging only
  for LIBPATH in `otool -L ${BUNDLE_DIR}/${LIB} \
                  | sed 's/(\([a-zA-Z0-9\., ]*\))//g' \
                  | grep  $LIB_DIR \
                  | grep -v framework` #frameworks (in particular qt frameworks) 
                                       #get dealt with in another script
  do 
    #echo $LIBPATH 
    BASELIB=`basename "$LIBPATH"`
    #echo $BASELIB
    install_name_tool -change ${LIBPATH} \
                      @executable_path/lib/${BASELIB} \
                      ${BUNDLE_DIR}/${LIB}
  done
  #otool -L ${BUNDLE_DIR}/${LIB}
  echo ${LIB}
done

#
# Next sort out the qgis plugins....
#
LIBS="libcopyrightlabelplugin.so      
      libgpxprovider.so               
      libscalebarplugin.so
      libdelimitedtextplugin.so       
      libgridmakerplugin.so           
      libwfsplugin.so
      libdelimitedtextprovider.so     
      libnortharrowplugin.so          
      libwfsprovider.so
      libgeorefplugin.so              
      libogrprovider.so               
      libwmsprovider.so
      libgpsimporterplugin.so         
      libquickprintplugin.so
      libevis.so"
for LIB in $LIBS
do
  install_name_tool -id @executable_path/lib/qgis/${LIB} ${BUNDLE_DIR}/lib/qgis/${LIB}
  # for debugging only
  for LIBPATH in `otool -L ${BUNDLE_DIR}/lib/qgis/${LIB} \
                  | sed 's/(\([a-zA-Z0-9\., ]*\))//g' \
                  | grep  $LIB_DIR \
                  | grep -v framework` #frameworks (in particular qt frameworks) get
                                       #dealt with in another script
  do 
    #echo "------------"
    #echo $LIBPATH 
    #echo "------------"
    BASELIB=`basename "$LIBPATH"`
    #echo $BASELIB
    install_name_tool -change ${LIBPATH} @executable_path/lib/${BASELIB} ${BUNDLE_DIR}/lib/qgis/${LIB}
  done
  echo $LIB
  #otool -L ${BUNDLE_DIR}/lib/qgis/${LIB}
  #echo "----------------------------------"
done

# Python libs need some special care
LIBS="share/qgis/python/qgis/core.so
      share/qgis/python/qgis/gui.so
      lib/libqgispython.dylib"
for LIB in $LIBS
do
  install_name_tool -id @executable_path/${LIB} ${BUNDLE_DIR}/${LIB}
  # for debugging only
  for LIBPATH in `otool -L ${BUNDLE_DIR}/${LIB} \
                  | sed 's/(\([a-zA-Z0-9\., ]*\))//g' \
                  | grep  $LIB_DIR \
                  | grep -v framework` #frameworks (in particular qt frameworks) get
                                       #dealt with in another script
  do 
    #echo "------------"
    #echo $LIBPATH 
    #echo "------------"
    BASELIB=`basename "$LIBPATH"`
    #echo $BASELIB
    install_name_tool -change ${LIBPATH} @executable_path/lib/${BASELIB} ${BUNDLE_DIR}/${LIB}
  done
  # Change the search path for qgis libs in python libs

  CORELIBPATH=/`otool -L ${BUNDLE_DIR}/${LIB} |grep -o "\b[/A-Za-z0-9]*libqgis_core.[0-9.]*.dylib\b"`
  CORELIB=`echo "${CORELIBPATH}" | grep -o "libqgis_core.[0-9.]*.dylib"`
  install_name_tool -change ${CORELIBPATH} @executable_path/lib/${CORELIB} ${BUNDLE_DIR}/${LIB}
  GUILIBPATH=/`otool -L ${BUNDLE_DIR}/${LIB} |grep -o "\b[/A-Za-z0-9]*libqgis_gui.[0-9.]*.dylib\b"`
  GUILIB=`echo "${CORELIBPATH}" | grep -o "libqgis_gui.[0-9.]*.dylib"`
  install_name_tool -change ${GUILIBPATH} @executable_path/lib/${GUILIB} ${BUNDLE_DIR}/${LIB}
  echo $LIB
  #otool -L ${BUNDLE_DIR}/lib/qgis/${LIB}
  #echo "----------------------------------"
done


/Users/timlinux/dev/cpp/qgis/build/src/core/libqgis_core.0.11.dylib

#
# Strip binaries - disable for debugging
#
#pushd .
#cd ${APP_PREFIX}
#for FILE in `find . -name *.dylib`; do echo "Stripping $FILE"; strip -x $FILE;  done 
#for FILE in `find . -name *.so`; do echo "Stripping $FILE"; strip -x $FILE;  done
#strip -x ${APP_PREFIX}/Contents/MacOS/qgis
#popd

#
# Install GDAL and Proj support files
# 
cp -r ${DEPS_BASE}/share/proj ${APP_PREFIX}/Contents/MacOS/share/
cp -r ${DEPS_BASE}/share/*.wkt ${APP_PREFIX}/Contents/MacOS/share/
cp -r ${DEPS_BASE}/share/*.csv ${APP_PREFIX}/Contents/MacOS/share/
cp -r ${DEPS_BASE}/share/*.dgn ${APP_PREFIX}/Contents/MacOS/share/

