

FROM      ubuntu:16.04
MAINTAINER Denis Rouzaud <denis.rouzaud@gmail.com>

LABEL Description="Docker container with QGIS dependencies" Vendor="QGIS.org" Version="1.0"

ENV CORES 8

RUN  apt-get update \
  && apt-get install -y software-properties-common \
  && add-apt-repository ppa:ubuntugis/ubuntugis-unstable \
  && apt-get update \
  && apt-get install -y \
    bison \
    ca-certificates \
    ccache \
    clang \
    cmake \
    dh-python \
    flex \
    gdal-bin \
    graphviz \
    grass-dev \
    libexpat1-dev \
    libfcgi-dev \
    libgdal-dev \
    libgeos-dev \
    libgsl-dev \
    libpq-dev \
    libproj-dev \
    libqca-qt5-2-dev \
    libqca-qt5-2-plugins \
    libqt5opengl5-dev \
    libqt5scintilla2-dev \
    libqt5sql5-sqlite \
    libqt5svg5-dev \
    libqt5webkit5-dev \
    libqt5xmlpatterns5-dev \
    libqwt-qt5-dev \
    libspatialindex-dev \
    libspatialite-dev \
    libsqlite3-dev \
    libsqlite3-mod-spatialite \
    libzip-dev \
    lighttpd \
    locales \
    ninja-build \
    pkg-config \
    poppler-utils \
    postgresql-client \
    pyqt5-dev \
    pyqt5-dev-tools \
    pyqt5.qsci-dev \
    python3-all-dev \
    python3-dev \
    python3-future \
    python3-gdal \
    python3-mock \
    python3-nose2 \
    python3-pip \
    python3-psycopg2 \
    python3-pyqt5 \
    python3-pyqt5.qsci \
    python3-pyqt5.qtsql \
    python3-pyqt5.qtsvg \
    python3-sip \
    python3-sip-dev \
    python3-termcolor \
    python3-yaml \
    qt5keychain-dev \
    qtbase5-dev \
    qtpositioning5-dev \
    qtscript5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    spawn-fcgi \
    txt2tags \
    xauth \
    xfonts-100dpi \
    xfonts-75dpi \
    xfonts-base \
    xfonts-scalable \
    xvfb \
  && pip3 install \
    psycopg2 \
    numpy \
    nose2 \
    pyyaml \
    mock \
    future \
    termcolor \
  && apt-get autoremove -y python3-pip python2.7 \
  && apt-get clean

RUN echo "alias python=python3" >> ~/.bash_aliases

ENV CC=/usr/lib/ccache/clang
ENV CXX=/usr/lib/ccache/clang++
ENV QT_SELECT=5

# RUN cmake --version
# RUN ${CC} --version
# RUN ${CXX} --version

RUN apt-get install -y git

CMD /root/QGIS/.ci/travis/linux/docker-build-test.sh
