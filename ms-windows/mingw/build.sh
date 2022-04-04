#!/bin/bash
# Script to build QGIS inside the qgis-build-deps-mingw.dockerfile Docker container
# Run from QGIS root directory with:
# docker run --rm -w /QGIS -v $(pwd):/QGIS elpaso/qgis-deps-mingw:latest /QGIS/ms-windows/mingw/build.sh


#!/bin/sh

# shellcheck disable=SC2086,SC2035,SC2035,SC2046,SC2044,SC2012,SC2155

arch=${1:-x86_64}
DEBUG=false
if [ "$2" == "debug" ]; then
  DEBUG=true
fi

njobs=${3:-$(($(grep -c ^processor /proc/cpuinfo) * 3 / 2))}


if [ "$arch" == "i686" ]; then
    bits=32
elif [ "$arch" == "x86_64" ]; then
    bits=64
else
    echo "Error: unrecognized architecture $arch"
    exit 1
fi

# Do copies instead of links if building inside container
if [ -f /.dockerenv ]; then
    lnk() {
        cp -aL "$1" "$2"
    }
else
    lnk() {
        ln -sf "$1" "$2"
    }
fi

# Note: This script is written to be used with the Fedora mingw environment
MINGWROOT=/usr/$arch-w64-mingw32/sys-root/mingw

if $DEBUG; then
  optflags="-O0 -g1 -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4 -fno-omit-frame-pointer"
  buildtype="Debug"
else
  optflags="-O2 -g1 -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4 -fno-omit-frame-pointer"
  buildtype="RelWithDebugInfo"
fi
pyver=$(mingw${bits}-python3 -c "import sys; print('.'.join(list(map(str, sys.version_info))[0:2]))")

# Halt on errors
set -e

export MINGW32_CFLAGS="$optflags"
export MINGW32_CXXFLAGS="$optflags"
export MINGW64_CFLAGS="$optflags"
export MINGW64_CXXFLAGS="$optflags"

SRCDIR="$(readlink -f "$(dirname "$(readlink -f "$0")")/../..")"

if $DEBUG; then
  BUILDDIR="$SRCDIR/build_mingw${bits}_debug"
else
  BUILDDIR="$SRCDIR/build_mingw${bits}"
fi
installroot="$BUILDDIR/dist"
installprefix="$installroot/usr/$arch-w64-mingw32/sys-root/mingw"

# Cleanup
rm -rf "$installroot"

# Build
echo "::group::cmake"
mkdir -p "$BUILDDIR"
(
  CRSSYNC_BIN=$(readlink -f "$SRCDIR")/build/output/bin/crssync
  cd "$BUILDDIR"
  mingw$bits-cmake \
    -DCMAKE_CROSS_COMPILING=1 \
    -DUSE_CCACHE=ON \
    -DCMAKE_BUILD_TYPE=$buildtype \
    -DNATIVE_CRSSYNC_BIN="$CRSSYNC_BIN" \
    -DBUILD_TESTING=OFF \
    -DENABLE_TESTS=OFF \
    -DQGIS_BIN_SUBDIR=bin \
    -DQGIS_CGIBIN_SUBDIR=bin \
    -DQGIS_LIB_SUBDIR=lib \
    -DQGIS_LIBEXEC_SUBDIR=lib/qgis \
    -DQGIS_DATA_SUBDIR=share/qgis \
    -DQGIS_PLUGIN_SUBDIR=lib/qgis/plugins \
    -DQGIS_INCLUDE_SUBDIR=include/qgis \
    -DQGIS_SERVER_MODULE_SUBDIR=lib/qgis/server \
    -DQGIS_QML_SUBDIR=lib/qt5/qml \
    -DBINDINGS_GLOBAL_INSTALL=ON \
    -DSIP_GLOBAL_INSTALL=ON \
    -DWITH_SERVER=ON \
    -DWITH_SERVER_LANDINGPAGE_WEBAPP=ON \
    -DTXT2TAGS_EXECUTABLE= \
    ..
)
echo "::endgroup::"


# Compile native crssync
# mkdir -p $BUILDDIR/native_crssync
# (
# cd $BUILDDIR/native_crssync
# echo "Building native crssync..."
# moc-qt5 $SRCDIR/src/core/qgsapplication.h > moc_qgsapplication.cpp
# g++ $optflags -fPIC -o crssync $SRCDIR/src/crssync/main.cpp $SRCDIR/src/crssync/qgscrssync.cpp moc_qgsapplication.cpp $SRCDIR/src/core/qgsapplication.cpp -DCORE_EXPORT= -DCOMPILING_CRSSYNC -I$SRCDIR/src/core/ -I$SRCDIR/src/core/geometry -I$BUILDDIR $(pkg-config --cflags --libs Qt5Widgets gdal sqlite3 proj)
# )
# crssync needs X at runtime
# Xvfb :99 &
# export DISPLAY=:99

echo "::group::compile QGIS"
mingw$bits-make -C"$BUILDDIR" -j"$njobs" DESTDIR="${installroot}" install VERBOSE=1
echo "::endgroup::"

#echo "ccache statistics"
ccache -s

# Remove plugins with missing dependencies
rm -rf "${installroot}/share/qgis/python/plugins/{MetaSearch,processing}"

# Strip debuginfo
binaries=$(find "$installprefix" -name '*.exe' -or -name '*.dll' -or -name '*.pyd')
for f in $binaries
do
    case $(mingw-objdump -h "$f" 2>/dev/null | grep -E -o '(debug[\.a-z_]*|gnu.version)') in
        *debuglink*) continue ;;
        *debug*) ;;
        *gnu.version*)
        echo "WARNING: $(basename "$f") is already stripped!"
        continue
        ;;
        *) continue ;;
    esac

    echo extracting debug info from "$f"
    mingw-objcopy --only-keep-debug "$f" "$f.debug" || :
    pushd $(dirname "$f")
    keep_symbols=$(mktemp)
    mingw-nm $f.debug --format=sysv --defined-only | awk -F \| '{ if ($4 ~ "Function") print $1 }' | sort > "$keep_symbols"
    mingw-objcopy --add-gnu-debuglink=$(basename "$f.debug") --strip-unneeded $(basename "$f") --keep-symbols="$keep_symbols" || :
    rm -f "$keep_symbols"
    popd
done

# Collect dependencies
function isnativedll {
    # If the import library exists but not the dynamic library, the dll ist most likely a native one
    local lower=${1,,}
    [ ! -e $MINGWROOT/bin/$1 ] && [ -f $MINGWROOT/lib/lib${lower/%.*/.a} ] && return 0;
    return 1;
}

function linkDep {
# Link the specified binary dependency and it's dependencies
    local indent=$3
    local destdir="$installprefix/${2:-bin}"
    local name="$(basename $1)"
    test -e "$destdir/$name" && return 0
    test -e "$destdir/qgisplugins/$name" && return 0
    [[ "$1" == *api-ms-win* ]] || [[ "$1" == *MSVCP*.dll ]] || [[ "$1" == *VCRUNTIME*.dll ]] && return 0
    echo "${indent}${1}"
    [ ! -e "$MINGWROOT/$1" ] && echo "Error: missing $MINGWROOT/$1" && return 1
    mkdir -p "$destdir" || return 1
    lnk "$MINGWROOT/$1" "$destdir/$name" || return 1
    echo "${2:-bin}/$name: $(rpm -qf "$MINGWROOT/$1")" >> $installprefix/origins.txt
    autoLinkDeps "$destdir/$name" "${indent}  " || return 1
    [ -e "/usr/lib/debug${MINGWROOT}/$1.debug" ] && lnk "/usr/lib/debug${MINGWROOT}/$1.debug" "$destdir/$name.debug" || :
    [ -e "$MINGWROOT/$1.debug" ] && lnk "$MINGWROOT/$1.debug" "$destdir/$name.debug" || :
    return 0
}

function autoLinkDeps {
# Collects and links the dependencies of the specified binary
    for dep in $(mingw-objdump -p "$1" | grep "DLL Name" | awk '{print $3}'); do
        if ! isnativedll "$dep"; then
            # HACK fix incorrect libpq case
            dep=${dep/LIBPQ/libpq}
            linkDep bin/$dep bin "$2" || return 1
        fi
    done
    return 0
}

# Install python libs
(
cd $MINGWROOT
SAVEIFS=$IFS
IFS=$(echo -en "\n\b")
for file in $(find lib/python${pyver} -type f); do
    mkdir -p "$installprefix/$(dirname $file)"
    lnk "$MINGWROOT/$file" "$installprefix/$file"
done
IFS=$SAVEIFS
)

# Gdal plugins
mkdir -p "$installprefix/lib/"
cp -a "$MINGWROOT/lib/gdalplugins" "$installprefix/lib/gdalplugins"

echo "Linking dependencies..."
binaries=$(find "$installprefix" -name '*.exe' -or -name '*.dll' -or -name '*.pyd')
for binary in $binaries; do
    autoLinkDeps $binary
done
linkDep bin/gdb.exe
linkDep bin/python3.exe
linkDep bin/python3w.exe

linkDep $(ls "$MINGWROOT/bin/libssl-"*.dll | sed "s|$MINGWROOT/||")
linkDep $(ls "$MINGWROOT/bin/libcrypto-"*.dll | sed "s|$MINGWROOT/||")
linkDep lib/mod_spatialite.dll bin

# Additional dependencies
linkDep lib/qt5/plugins/imageformats/qgif.dll  bin/imageformats
linkDep lib/qt5/plugins/imageformats/qicns.dll bin/imageformats
linkDep lib/qt5/plugins/imageformats/qico.dll  bin/imageformats
linkDep lib/qt5/plugins/imageformats/qjp2.dll  bin/imageformats
linkDep lib/qt5/plugins/imageformats/qjpeg.dll bin/imageformats
linkDep lib/qt5/plugins/imageformats/qtga.dll  bin/imageformats
linkDep lib/qt5/plugins/imageformats/qtiff.dll bin/imageformats
linkDep lib/qt5/plugins/imageformats/qwbmp.dll bin/imageformats
linkDep lib/qt5/plugins/imageformats/qwebp.dll bin/imageformats
linkDep lib/qt5/plugins/imageformats/qsvg.dll  bin/imageformats
linkDep lib/qt5/plugins/platforms/qwindows.dll bin/platforms
linkDep lib/qt5/plugins/printsupport/windowsprintersupport.dll bin/printsupport
linkDep lib/qt5/plugins/styles/qwindowsvistastyle.dll bin/styles
linkDep lib/qt5/plugins/audio/qtaudio_windows.dll bin/audio
linkDep lib/qt5/plugins/mediaservice/dsengine.dll bin/mediaservice
linkDep lib/qt5/plugins/mediaservice/qtmedia_audioengine.dll bin/mediaservice
linkDep lib/qt5/plugins/sqldrivers/qsqlite.dll bin/sqldrivers
linkDep lib/qt5/plugins/sqldrivers/qsqlodbc.dll bin/sqldrivers
linkDep lib/qt5/plugins/sqldrivers/qsqlpsql.dll bin/sqldrivers

linkDep lib/qt5/plugins/crypto/libqca-gcrypt.dll bin/crypto
linkDep lib/qt5/plugins/crypto/libqca-logger.dll bin/crypto
linkDep lib/qt5/plugins/crypto/libqca-softstore.dll bin/crypto
linkDep lib/qt5/plugins/crypto/libqca-gnupg.dll bin/crypto
linkDep lib/qt5/plugins/crypto/libqca-ossl.dll bin/crypto

linkDep lib/ossl-modules/legacy.dll lib/ossl-modules

mkdir -p "$installprefix/share/qt5/translations/"
#cp -a "$MINGWROOT/share/qt5/translations/qt_"*.qm  "$installprefix/share/qt5/translations"
#cp -a "$MINGWROOT/share/qt5/translations/qtbase_"*.qm  "$installprefix/share/qt5/translations"

# Data files
mkdir -p "$installprefix/share/"
cp -a /usr/share/gdal "$installprefix/share/gdal"
cp -a /usr/share/proj "$installprefix/share/proj"

# Sort origins file
sort "$installprefix/origins.txt" | uniq > "$installprefix/origins.new" && mv "$installprefix/origins.new" "$installprefix/origins.txt"
