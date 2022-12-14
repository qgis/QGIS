ARG DISTRO_VERSION=36

FROM fedora:${DISTRO_VERSION} as single
MAINTAINER Matthias Kuhn <matthias@opengis.ch>

RUN dnf -y --refresh install \
    bison \
    ccache \
    clang \
    clazy \
    curl \
    exiv2-devel \
    expat-devel \
    fcgi-devel \
    flex \
    git \
    gdal-devel \
    geos-devel \
    gpsbabel \
    grass \
    grass-devel \
    gsl-devel \
    libpq-devel \
    libspatialite-devel \
    libxml2-devel \
    libzip-devel \
    libzstd-devel \
    netcdf-devel \
    ninja-build \
    ocl-icd-devel \
    PDAL \
    PDAL-libs \
    PDAL-devel \
    proj-devel \
    protobuf-devel \
    protobuf-lite-devel \
    python3-devel \
    python3-termcolor \
    qt6-qt3d-devel \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qttools-static \
    qt6-qtserialport-devel \
    qt6-qtsvg-devel \
    qt6-qtpositioning-devel \
    qt6-qtdeclarative-devel \
    qt6-qt5compat-devel \
    qt6-qtmultimedia-devel \
    spatialindex-devel \
    sqlite-devel \
    unzip \
    unixODBC-devel \
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
    ninja-build \
    patch \
    dos2unix

RUN cd /usr/src \
  && wget https://github.com/KDE/qca/archive/refs/heads/master.zip \
  && unzip master.zip \
  && rm master.zip \
  && mkdir build \
  && cd build \
  && cmake -DQT6=ON -DBUILD_TESTS=OFF -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local ../qca-master \
  && ninja install

RUN cd /usr/src \
  && wget https://github.com/frankosterfeld/qtkeychain/archive/841f31c7ca177e45647fd705200d7fcbeee056e5/master.zip \
  && unzip master.zip \
  && rm master.zip \
  && cd qtkeychain-841f31c7ca177e45647fd705200d7fcbeee056e5 \
  && cmake -DBUILD_WITH_QT6=ON -DBUILD_TRANSLATIONS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local -GNinja \
  && ninja install

RUN cd /usr/src \
  && wget https://sourceforge.net/projects/qwt/files/qwt/6.2.0/qwt-6.2.0.zip/download \
  && unzip download \
  && cd qwt-6.2.0 \
  && dos2unix qwtconfig.pri \
  && printf '140c140\n< QWT_CONFIG     += QwtExamples\n---\n> #QWT_CONFIG     += QwtExamples\n151c151\n< QWT_CONFIG     += QwtPlayground\n---\n> #QWT_CONFIG     += QwtPlayground\n158c158\n< QWT_CONFIG     += QwtTests\n---\n> #QWT_CONFIG     += QwtTests\n' | patch qwtconfig.pri \
  && qmake6 qwt.pro \
  && make -j4 \
  && make install


RUN cd /usr/src \
  && wget https://www.riverbankcomputing.com/static/Downloads/QScintilla/2.13.3/QScintilla_src-2.13.3.zip \
  && unzip QScintilla_src-2.13.3.zip \
  && rm QScintilla_src-2.13.3.zip \
  && cd QScintilla_src-2.13.3 \
  && qmake6 src/qscintilla.pro \
  && make -j4 \
  && make install

# Oracle : client side
RUN curl https://download.oracle.com/otn_software/linux/instantclient/199000/instantclient-basic-linux.x64-19.9.0.0.0dbru.zip > instantclient-basic-linux.x64-19.9.0.0.0dbru.zip
RUN curl https://download.oracle.com/otn_software/linux/instantclient/199000/instantclient-sdk-linux.x64-19.9.0.0.0dbru.zip > instantclient-sdk-linux.x64-19.9.0.0.0dbru.zip
RUN curl https://download.oracle.com/otn_software/linux/instantclient/199000/instantclient-sqlplus-linux.x64-19.9.0.0.0dbru.zip > instantclient-sqlplus-linux.x64-19.9.0.0.0dbru.zip

RUN unzip instantclient-basic-linux.x64-19.9.0.0.0dbru.zip
RUN unzip instantclient-sdk-linux.x64-19.9.0.0.0dbru.zip
RUN unzip instantclient-sqlplus-linux.x64-19.9.0.0.0dbru.zip

ENV PATH="/instantclient_19_9:${PATH}"
ENV LD_LIBRARY_PATH="/instantclient_19_9:${LD_LIBRARY_PATH}"
