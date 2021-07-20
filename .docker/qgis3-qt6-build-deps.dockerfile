FROM fedora:rawhide
MAINTAINER Matthias Kuhn <matthias@opengis.ch>

RUN dnf -y install \
    bison \
    clang \
    clazy \
    exiv2-devel \
    expat-devel \
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
    python3-devel \
    qt6-qt3d-devel \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qttools-static \
    qt6-qtsvg-devel \
    qt6-qt5compat-devel \
    spatialindex-devel \
    sqlite-devel \
    unzip


RUN dnf -y install wget openssl-devel && cd /usr/src \
  && wget https://github.com/KDE/qca/archive/refs/tags/v2.3.3.zip \
  && unzip v2.3.3.zip \
  && cd qca-2.3.3 \
  && cmake -DCMAKE_INSTALL_PREFIX=/usr -DQT6=ON -GNinja \
  && ninja install

RUN dnf -y install libsecret-devel && cd /usr/src \
  && wget https://github.com/frankosterfeld/qtkeychain/archive/refs/heads/master.zip \
  && unzip master.zip \
  && cd qtkeychain-master \
  && cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_WITH_QT6=ON -GNinja \
  && ninja install

RUN cd /usr/src \
  && wget https://sourceforge.net/projects/qwt/files/qwt/6.2.0/qwt-6.2.0.zip/download \
  && unzip download \
  && cd qwt-6.2.0 \
  && qmake6 qwt.pro \
  && make -j4 \
  && make install

