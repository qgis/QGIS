FROM fedora:rawhide
MAINTAINER Matthias Kuhn <matthias@opengis.ch>

RUN dnf -y install \
    bison \
    clang \
    clazy \
    exiv2-devel \
    fcgi-devel \
    flex \
    gdal-devel \
    geos-devel \
    gsl-devel \
    libpq-devel \
    libspatialite-devel \
    libzip-devel \
    libzstd-devel \
    ninja-build \
    proj-devel \
    protobuf-devel \
    protobuf-lite-devel \
    qt6-qt3d-devel \
    qt6-qtbase-devel \
    qt6-qttools-static \
    qt6-qtsvg-devel \
    qt6-qt5compat-devel \
    spatialindex-devel \
    sqlite-devel \
    unzip


RUN dnf -y install wget openssl-devel && cd /usr/src \
  && wget https://github.com/KDE/qca/archive/refs/heads/qt6.zip \
  && unzip qt6.zip \
  && cd qca-qt6 \
  && cmake -DCMAKE_INSTALL_PREFIX=/usr -GNinja \
  && ninja install

RUN dnf -y install libsecret-devel && cd /usr/src \
  && wget https://github.com/frankosterfeld/qtkeychain/archive/refs/heads/master.zip \
  && unzip master.zip \
  && cd qtkeychain-master \
  && cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_WITH_QT6=ON -GNinja \
  && ninja install
