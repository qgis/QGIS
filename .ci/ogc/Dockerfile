FROM ubuntu:25.10
RUN  export DEBIAN_FRONTEND=noninteractive
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y \
    bison \
    ccache \
    clang \
    cmake \
    ninja-build \
    flex \
    locales \
    mold \
    libdraco-dev \
    libexiv2-dev \
    libexpat1-dev \
    libfcgi-dev \
    libgeographiclib-dev \
    libgeos-dev \
    libgdal-dev \
    libgsl-dev \
    libpq-dev \
    libprotobuf-dev \
    libqca-qt6-dev \
    libqt6opengl6-dev \
    libqscintilla2-qt6-dev \
    libqt6svg6-dev \
    libspatialite-dev \
    libsqlite3-dev \
    libsqlite3-mod-spatialite \
    libzip-dev \
    libzstd-dev \
    protobuf-compiler \
    pyqt6-dev-tools \
    pyqt6.qsci-dev \
    python3-all-dev \
    python3-dev \
    python3-gdal \
    python3-mock \
    python3-nose2 \
    python3-numpy \
    python3-oauthlib \
    python3-openssl \
    python3-owslib \
    python3-pep8 \
    python3-pexpect \
    python3-pip \
    python3-psycopg2 \
    python3-pyproj \
    python3-pyqt6 \
    python3-pyqt6.qsci \
    python3-pyqt6.qtsvg \
    python3-pyqt6.qtpositioning \
    python3-pyqt6.qtmultimedia \
    python3-pyqt6.qtserialport \
    python3-pyqt6.qtwebengine \
    python3-pyqt6.sip \
    python3-pyqtbuild \
    python3-requests \
    python3-shapely  \
    python3-geopandas \
    python3-sphinx \
    python3-sipbuild \
    python3-six \
    python3-termcolor \
    python3-yaml \
    libsfcgal-dev \
    sip-tools \
    qmake6 \
    qt6-3d-dev \
    qtkeychain-qt6-dev \
    qt6-base-dev \
    qt6-declarative-dev-tools \
    qt6-positioning-dev \
    qt6-multimedia-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-base-private-dev \
    qt6-5compat-dev \
    qt6-webengine-dev \
    qt6-pdf-dev \
    qt6-serialport-dev \
    opencl-headers \
    ocl-icd-opencl-dev \
    spawn-fcgi

RUN echo "C UTF-8" > /etc/locale.gen
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
RUN locale-gen

ADD qgis_mapserv.sh /root/qgis_mapserv.sh
CMD ["sh", "/root/qgis_mapserv.sh"]
