# MinGW build environment for QGIS / KADAS Albireo

FROM fedora:rawhide

MAINTAINER Sandro Mani <manisandro@gmail.com>

RUN \
echo all > /etc/rpm/macros.image-language-conf && \
dnf install -y 'dnf-command(config-manager)' && \
dnf config-manager --add-repo https://copr.fedorainfracloud.org/coprs/smani/mingw-extras/repo/fedora-rawhide/smani-mingw-extras-fedora-rawhide.repo && \
dnf install -y \
  mingw64-dlfcn \
  mingw64-exiv2 \
  mingw64-gcc-c++ \
  mingw64-gdal \
  mingw64-gdb \
  mingw64-GdbCrashHandler \
  mingw64-GeographicLib \
  mingw64-geos \
  mingw64-gsl \
  mingw64-libgomp \
  mingw64-libzip \
  mingw64-osgearth-qt5 \
  mingw64-postgresql \
  mingw64-proj \
  mingw64-python3-OWSLib \
  mingw64-python3-pyyaml \
  mingw64-python3-qscintilla-qt5 \
  mingw64-python3-qt5 \
  mingw64-qca-qt5 \
  mingw64-qscintilla-qt5 \
  mingw64-qt5-qmake \
  mingw64-qt5-qtactiveqt \
  mingw64-qt5-qtbase \
  mingw64-qt5-qtimageformats \
  mingw64-qt5-qtlocation \
  mingw64-qt5-qtmultimedia \
  mingw64-qt5-qtscript \
  mingw64-qt5-qtserialport \
  mingw64-qt5-qtsvg \
  mingw64-qt5-qttools \
  mingw64-qt5-qttranslations \
  mingw64-qt5-qtwebkit \
  mingw64-qt5-qtxmlpatterns \
  mingw64-qtkeychain-qt5 \
  mingw64-quazip-qt5 \
  mingw64-qwt-qt5 \
  mingw64-sip \
  mingw64-spatialindex \
  mingw64-sqlite \
  mingw64-svg2svgt \
  bison \
  cmake \
  findutils \
  flex \
  gcc-c++ \
  gdal-devel \
  git \
  make \
  proj-devel \
  python-qt5 \
  qt5-linguist \
  qt5-qtbase-devel \
  sqlite-devel \
  wget \
  xorg-x11-server-Xvfb \
  zip

RUN wget https://pkg.sourcepole.ch/kadas/mingw64-librsvg2-2.40.11-1.fc28.noarch.rpm
RUN dnf install -y mingw64-librsvg2-2.40.11-1.fc28.noarch.rpm

WORKDIR /workspace
VOLUME ["/workspace"]
