#!/bin/bash
# Build a Linux x86_64 AppImage of QGIS_AI.
# Designed to run on Ubuntu 22.04 (LTS = broad glibc compatibility).
# Output: ./QGIS_AI-${QGISAI_VERSION}-x86_64.AppImage
#
# Required env: QGISAI_VERSION (e.g. "0.1.0")

set -euo pipefail

VERSION="${QGISAI_VERSION:-dev}"
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
  ocl-icd-opencl-dev \
  fuse \
  libfuse2t64 \
  wget \
  file \
  desktop-file-utils \
  gdal-data \
  proj-data

echo "==> Relaxing Qt6 minimum version 6.6.0 -> 6.4.0 (noble ships 6.4.2)"
# Noble's qt6-base-dev is 6.4.2; QGIS upstream requires >= 6.6.0. We patch
# the minimum here only for the AppImage CI build — kept out of the
# committed CMakeLists.txt so upstream merges stay clean.
sed -i 's|set(QT_MIN_VERSION 6\.6\.0)|set(QT_MIN_VERSION 6.4.0)|' CMakeLists.txt

echo "==> Configuring CMake (Release, ENABLE_AI_ASSISTANT=ON)"
# WITH_PDAL=OFF: noble does not ship libpdal-dev where cmake expects it,
# and PDAL (point cloud reading) is optional. Users who need point cloud
# providers can install QGIS_AI from source.
# WITH_AUTH=OFF: QCA Qt6 (libqca-qt6-2-dev) is not in noble core repo and
# would require adding the QGIS apt repository. Auth disabled for AppImage
# only — desktop installs from source still get full auth support.
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${APPDIR}/usr" \
  -DENABLE_AI_ASSISTANT=ON \
  -DWITH_BINDINGS=ON \
  -DWITH_DESKTOP=ON \
  -DWITH_3D=ON \
  -DWITH_QTWEBENGINE=ON \
  -DWITH_PDAL=OFF \
  -DWITH_AUTH=OFF \
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
  -e 's|@QGIS_APP_NAME@|qgis|g' \
  linux/org.qgis.qgis.desktop.in > "${APPDIR}/usr/share/applications/com.francemazzi.qgisai.desktop"

# Top-level desktop file (linuxdeploy needs one in AppDir root).
cp "${APPDIR}/usr/share/applications/com.francemazzi.qgisai.desktop" "${APPDIR}/com.francemazzi.qgisai.desktop"

# Icon (use the upstream QGIS icon for M1).
ICON_SRC="images/icons/qgis-icon-512x512.png"
if [ ! -f "${ICON_SRC}" ]; then
  ICON_SRC="$(find images/icons -maxdepth 3 -name 'qgis-icon-*.png' | sort -r | head -n1)"
fi
mkdir -p "${APPDIR}/usr/share/icons/hicolor/512x512/apps"
cp "${ICON_SRC}" "${APPDIR}/usr/share/icons/hicolor/512x512/apps/qgis.png"
cp "${ICON_SRC}" "${APPDIR}/qgis.png"

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
export OUTPUT="QGIS_AI-${VERSION}-x86_64.AppImage"

# Allow running AppImage tools in CI containers (which lack FUSE).
export APPIMAGE_EXTRACT_AND_RUN=1

.tools/linuxdeploy-x86_64.AppImage \
  --appdir "${APPDIR}" \
  --plugin qt \
  --output appimage \
  --desktop-file "${APPDIR}/com.francemazzi.qgisai.desktop" \
  --icon-file "${APPDIR}/qgis.png"

echo "==> Done"
ls -lh QGIS_AI-*.AppImage
