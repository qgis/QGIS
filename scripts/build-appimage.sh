#!/bin/bash
# Build a Linux x86_64 AppImage of Strata.
# Designed to run on Ubuntu 22.04 (LTS = broad glibc compatibility).
# Output: ./Strata-${STRATA_VERSION}-x86_64.AppImage
#
# Required env: STRATA_VERSION (e.g. "0.1.0")

set -euo pipefail

VERSION="${STRATA_VERSION:-dev}"
APPDIR="${PWD}/AppDir"
NPROC="$(nproc)"

echo "==> Installing build dependencies (apt) — Ubuntu 24.04 noble"
sudo apt-get update
# Build deps for QGIS Qt6 + extras needed for AppImage packaging.
# Package names verified for Ubuntu 24.04 (noble).
# Qwt is intentionally omitted — noble has no libqwt-qt6-dev and QGIS
# falls back to the bundled copy at external/qwt-6.3.0.
# PDAL (point cloud) is intentionally omitted — cmake auto-disables the
# feature when libpdal-dev is missing.
sudo apt-get install -y --no-install-recommends \
  build-essential \
  bison \
  flex \
  cmake \
  ninja-build \
  ccache \
  pkg-config \
  python3-dev \
  python3-pip \
  python3-pyqt6 \
  python3-pyqt6.qsci \
  python3-pyqt6.qtwebengine \
  python3-pyqt6.qtmultimedia \
  python3-pyqt6.qtpositioning \
  python3-sip-dev \
  python3-pyqtbuild \
  pyqt6-dev \
  pyqt6-dev-tools \
  sip-tools \
  libqt6core5compat6-dev \
  qt6-base-dev \
  qt6-base-dev-tools \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-l10n-tools \
  qt6-multimedia-dev \
  qt6-positioning-dev \
  qt6-3d-dev \
  qt6-webengine-dev \
  libqt6svg6-dev \
  libqt6serialport6-dev \
  libqt6sql6-sqlite \
  libqt6opengl6-dev \
  libqt6qml6 \
  libqt6quick6 \
  qml6-module-qtcharts \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts \
  libqscintilla2-qt6-dev \
  qtkeychain-qt6-dev \
  libgdal-dev \
  libproj-dev \
  libgeos-dev \
  libsqlite3-dev \
  libspatialite-dev \
  libspatialindex-dev \
  libpq-dev \
  libexpat1-dev \
  libzip-dev \
  libzstd-dev \
  libprotobuf-dev \
  protobuf-compiler \
  libexiv2-dev \
  libfcgi-dev \
  libdraco-dev \
  libgsl-dev \
  libssl-dev \
  ocl-icd-opencl-dev \
  fuse \
  libfuse2t64 \
  wget \
  file \
  desktop-file-utils \
  gdal-data \
  proj-data

echo "==> Building QCA Qt6 from source (not packaged on Ubuntu noble)"
# QCA (Qt Cryptographic Architecture) Qt6 build is required by QGIS auth
# (find_package(QCA REQUIRED) when WITH_AUTH=ON). Noble's repo only has
# Qt5 variants (libqca-qt5-2-dev) and the QGIS apt repo does not yet
# publish a Qt6 build for noble. Compile from source — small library
# (~5 min build with ninja + parallel jobs) and ships with the AppImage.
QCA_SRC=/tmp/qca-src
QCA_BUILD=/tmp/qca-build
if [ ! -d "$QCA_SRC" ]; then
  git clone --depth 1 --branch v2.3.10 https://invent.kde.org/libraries/qca.git "$QCA_SRC"
fi
cmake -S "$QCA_SRC" -B "$QCA_BUILD" -G Ninja \
  -DBUILD_WITH_QT6=ON \
  -DBUILD_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DWITH_ossl_PLUGIN=yes
cmake --build "$QCA_BUILD" -j "$NPROC"
sudo cmake --install "$QCA_BUILD"
sudo ldconfig

echo "==> Relaxing Qt6 minimum version 6.6.0 -> 6.4.0 (noble ships 6.4.2)"
# Noble's qt6-base-dev is 6.4.2; QGIS upstream requires >= 6.6.0. We patch
# the minimum here only for the AppImage CI build — kept out of the
# committed CMakeLists.txt so upstream merges stay clean.
sed -i 's|set(QT_MIN_VERSION 6\.6\.0)|set(QT_MIN_VERSION 6.4.0)|' CMakeLists.txt

echo "==> Configuring CMake (Release, ENABLE_AI_ASSISTANT=ON)"
# WITH_PDAL=OFF: noble does not ship libpdal-dev where cmake expects it,
# and PDAL (point cloud reading) is optional. Users who need point cloud
# providers can install Strata from source.
# WITH_PYTHON=OFF + WITH_BINDINGS=OFF: noble ships sip6 which rejects the
# /Movable/ annotation in python/PyQt6/core/conversions.sip:1249 (the
# %If(MOVABLE_MAPPED_TYPE) guard does not gate parsing in sip6). Disabling
# WITH_BINDINGS alone is not enough because CMakeLists.txt:321 tests
# WITH_PYTHON to add dependencies on python targets (pycore, pygui,
# pyanalysis, pyplugin-installer, qgispython, staged-plugins) that only
# exist when WITH_BINDINGS=ON. Both are disabled together. PyQGIS plugin
# support is lost in the AppImage; source builds still get it.
# WITH_AUTH=ON (default): QCA Qt6 built from source above and installed
# under /usr/local. CMake's FindQCA picks it up automatically.
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${APPDIR}/usr" \
  -DQGIS_APP_NAME=Strata \
  -DSTRATA_VERSION="${VERSION}" \
  -DENABLE_AI_ASSISTANT=ON \
  -DWITH_PYTHON=OFF \
  -DWITH_BINDINGS=OFF \
  -DWITH_DESKTOP=ON \
  -DWITH_3D=ON \
  -DWITH_QTWEBENGINE=ON \
  -DWITH_PDAL=OFF \
  -DENABLE_TESTS=OFF \
  -DUSE_CCACHE=ON \
  -DENABLE_UNITY_BUILDS=ON \
  -DBUILD_WITH_QT6=ON

echo "==> Building (this takes ~30-60 min)"
cmake --build build -j "${NPROC}"

echo "==> Installing into ${APPDIR}"
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}"
DESTDIR="" cmake --install build

echo "==> Staging AppDir metadata"
mkdir -p "${APPDIR}/usr/share/applications"

# Use the existing .desktop template; render @QGIS_APP_NAME@ etc minimally.
sed \
  -e 's|@QGIS_APP_NAME@|Strata|g' \
  linux/org.qgis.qgis.desktop.in > "${APPDIR}/usr/share/applications/com.francemazzi.strata.desktop"

# Top-level desktop file (linuxdeploy needs one in AppDir root).
cp "${APPDIR}/usr/share/applications/com.francemazzi.strata.desktop" "${APPDIR}/com.francemazzi.strata.desktop"

# Icon.
ICON_SRC="images/icons/strata-icon-512x512.png"
if [ ! -f "${ICON_SRC}" ]; then
  ICON_SRC="images/icons/qgis-icon-512x512.png"
fi
mkdir -p "${APPDIR}/usr/share/icons/hicolor/512x512/apps"
cp "${ICON_SRC}" "${APPDIR}/usr/share/icons/hicolor/512x512/apps/strata.png"
cp "${ICON_SRC}" "${APPDIR}/strata.png"

# AppRun launcher.
cp scripts/AppRun.sh "${APPDIR}/AppRun"
chmod +x "${APPDIR}/AppRun"

echo "==> Downloading linuxdeploy"
mkdir -p .tools
cd .tools
[ -f linuxdeploy-x86_64.AppImage ] || wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
[ -f linuxdeploy-plugin-qt-x86_64.AppImage ] || wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage
cd ..

# Bundle GDAL/PROJ data so the AppImage is self-contained.
mkdir -p "${APPDIR}/usr/share/gdal" "${APPDIR}/usr/share/proj"
GDAL_SHARE="$(find /usr/share/gdal -maxdepth 2 -name '*.csv' -o -name '*.wkt' 2>/dev/null | head -n1 | xargs -r dirname || true)"
if [ -n "${GDAL_SHARE}" ]; then
  cp -r "${GDAL_SHARE}/." "${APPDIR}/usr/share/gdal/"
fi
PROJ_SHARE="/usr/share/proj"
if [ -d "${PROJ_SHARE}" ]; then
  cp -r "${PROJ_SHARE}/." "${APPDIR}/usr/share/proj/"
fi

echo "==> Running linuxdeploy with Qt plugin"
export QMAKE=/usr/lib/qt6/bin/qmake6
export EXTRA_QT_MODULES="quick;quickcontrols2;svg;sql;multimedia;positioning;3dcore;3drender;3dinput;3dlogic;3dextras;webengine;webenginewidgets;serialport"
export OUTPUT="Strata-${VERSION}-x86_64.AppImage"

# Allow running AppImage tools in CI containers (which lack FUSE).
export APPIMAGE_EXTRACT_AND_RUN=1

# linuxdeploy resolves dependencies of the qgis binary by name; libqgis_app.so
# and the other QGIS shared libs install under ${APPDIR}/usr/lib/qgis (a
# subdir of usr/lib that linuxdeploy does not search by default). Make them
# discoverable via LD_LIBRARY_PATH and pass them as explicit --library args.
export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${APPDIR}/usr/lib/qgis:${APPDIR}/usr/lib/x86_64-linux-gnu:/usr/local/lib:${LD_LIBRARY_PATH:-}"
LIBRARY_ARGS=()
while IFS= read -r -d '' lib; do
  LIBRARY_ARGS+=( "--library" "$lib" )
done < <(find "${APPDIR}/usr/lib" -maxdepth 3 -name 'libqgis_*.so*' -print0)

.tools/linuxdeploy-x86_64.AppImage \
  --appdir "${APPDIR}" \
  --plugin qt \
  --output appimage \
  --desktop-file "${APPDIR}/com.francemazzi.strata.desktop" \
  --icon-file "${APPDIR}/strata.png" \
  "${LIBRARY_ARGS[@]}"

echo "==> Done"
ls -lh Strata-*.AppImage
