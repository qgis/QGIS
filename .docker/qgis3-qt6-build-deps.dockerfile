FROM fedora:34 as single
MAINTAINER Matthias Kuhn <matthias@opengis.ch>

RUN dnf -y install \
    bison \
    ccache \
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
    python3-termcolor \
    qt6-qt3d-devel \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qttools-static \
    qt6-qtsvg-devel \
    qt6-qt5compat-devel \
    spatialindex-devel \
    sqlite-devel \
    unzip \
    xorg-x11-server-Xvfb \
    util-linux \
    wget \
    openssl-devel \
    libsecret-devel \
    make \
    automake \
    gcc \
    gcc-c++ \
    kernel-devel \
    ninja-build

RUN cd /usr/src \
  && wget https://github.com/KDE/qca/archive/refs/heads/master.zip \
  && unzip master.zip \
  && rm master.zip \
  && mkdir build \
  && cd build \
  && cmake -DQT6=ON -GNinja ../qca-master \
  && ninja install

RUN cd /usr/src \
  && wget https://github.com/frankosterfeld/qtkeychain/archive/refs/heads/master.zip \
  && unzip master.zip \
  && rm master.zip \
  && cd qtkeychain-master \
  && cmake -DBUILD_WITH_QT6=ON -GNinja \
  && ninja install

RUN cd /usr/src \
  && wget https://sourceforge.net/projects/qwt/files/qwt/6.2.0/qwt-6.2.0.zip/download \
  && unzip download \
  && cd qwt-6.2.0 \
  && qmake6 qwt.pro \
  && make -j4 \
  && make install
  
