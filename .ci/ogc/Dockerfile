FROM ubuntu:latest
MAINTAINER Paul Blottiere <blottiere.paul@gmail.com>
RUN  export DEBIAN_FRONTEND=noninteractive
ENV  DEBIAN_FRONTEND noninteractive

RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y \
  ccache \
  cmake \
  ninja-build \
  clang \
  flex \
  bison \
  libgeos-dev \
  libgdal-dev \
  libzip-dev \
  libprotobuf-dev \
  qtbase5-dev \
  libqt5svg5-dev \
  libqt5serialport5-dev \
  qttools5-dev \
  protobuf-compiler \
  qt5-qmake \
  qtbase5-dev-tools \
  qtchooser \
  qtpositioning5-dev \
  libqt5webkit5-dev \
  libqca-qt5-2-dev \
  libgsl-dev \
  libspatialindex-dev \
  qt5keychain-dev \
  libexiv2-dev \
  libfcgi-dev \
  libqt5scintilla2-dev \
  libqwt-qt5-dev \
  pyqt5-dev \
  python3-pyqt5 \
  python3-pyqt5.qsci \
  python3-all-dev \
  python3-dev \
  python3-sip-dev \
  pyqt5-dev-tools \
  spawn-fcgi

ADD qgis_mapserv.sh /root/qgis_mapserv.sh
CMD ["sh", "/root/qgis_mapserv.sh"]
