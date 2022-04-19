
ARG DISTRO_VERSION=20.04

# Oracle Docker image is too large, so we add as less dependencies as possible
# so there is enough space on GitHub runner
FROM      ubuntu:${DISTRO_VERSION} as binary-for-oracle
MAINTAINER Denis Rouzaud <denis@opengis.ch>

LABEL Description="Docker container with QGIS dependencies" Vendor="QGIS.org" Version="1.0"

# && echo "deb http://ppa.launchpad.net/ubuntugis/ubuntugis-unstable/ubuntu xenial main" >> /etc/apt/sources.list \
# && echo "deb-src http://ppa.launchpad.net/ubuntugis/ubuntugis-unstable/ubuntu xenial main" >> /etc/apt/sources.list \
# && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 314DF160 \

RUN  apt-get update \
  && apt-get install -y software-properties-common \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    apt-transport-https \
    ca-certificates \
    clazy \
    cmake \
    curl \
    dh-python \
    gdal-bin \
    gpsbabel \
    graphviz \
    libaio1 \
    libexiv2-27 \
    libfcgi0ldbl \
    'libgsl23|libgsl23' \
    'libprotobuf-lite17|libprotobuf-lite23' \
    libqca-qt5-2-plugins \
    libqt53dextras5 \
    libqt53drender5 \
    libqt5concurrent5 \
    libqt5keychain1 \
    libqt5positioning5 \
    libqt5qml5 \
    libqt5quick5 \
    libqt5quickcontrols2-5 \
    libqt5quickwidgets5 \
    libqt5serialport5 \
    libqt5sql5-odbc \
    libqt5sql5-sqlite \
    libqt5xml5 \
    libqt5webkit5 \
    libqwt-qt5-6 \
    libspatialindex6 \
    libsqlite3-mod-spatialite \
    'libzip4|libzip5' \
    lighttpd \
    locales \
    pdal \
    poppler-utils \
    python3-future \
    python3-gdal \
    python3-mock \
    python3-nose2 \
    python3-owslib \
    python3-pip \
    python3-psycopg2 \
    python3-pyproj \
    python3-pyqt5 \
    python3-pyqt5.qsci \
    python3-pyqt5.qtsql \
    python3-pyqt5.qtsvg \
    python3-pyqt5.qtwebkit \
    python3-pyqt5.qtpositioning \
    python3-sip \
    python3-termcolor \
    python3-yaml \
    qpdf \
    qt3d-assimpsceneimport-plugin \
    qt3d-defaultgeometryloader-plugin \
    qt3d-gltfsceneio-plugin \
    qt3d-scene2d-plugin \
    qt5-image-formats-plugins \
    saga \
    supervisor \
    unzip \
    xauth \
    xfonts-100dpi \
    xfonts-75dpi \
    xfonts-base \
    xfonts-scalable \
    xvfb \
    ocl-icd-libopencl1 \
  && pip3 install \
    numpy \
    nose2 \
    pyyaml \
    mock \
    future \
    termcolor \
    oauthlib \
    pyopenssl \
    pep8 \
    pexpect \
    capturer \
    sphinx \
    requests \
    six \
    hdbcli \
  && apt-get clean

# Node.js and Yarn for server landingpage webapp
RUN curl -fsSL https://deb.nodesource.com/setup_lts.x | bash -
RUN apt-get install -y nodejs
RUN corepack enable

# Oracle : client side
RUN curl https://download.oracle.com/otn_software/linux/instantclient/199000/instantclient-basic-linux.x64-19.9.0.0.0dbru.zip > instantclient-basic-linux.x64-19.9.0.0.0dbru.zip
RUN curl https://download.oracle.com/otn_software/linux/instantclient/199000/instantclient-sdk-linux.x64-19.9.0.0.0dbru.zip > instantclient-sdk-linux.x64-19.9.0.0.0dbru.zip
RUN curl https://download.oracle.com/otn_software/linux/instantclient/199000/instantclient-sqlplus-linux.x64-19.9.0.0.0dbru.zip > instantclient-sqlplus-linux.x64-19.9.0.0.0dbru.zip

RUN unzip instantclient-basic-linux.x64-19.9.0.0.0dbru.zip
RUN unzip instantclient-sdk-linux.x64-19.9.0.0.0dbru.zip
RUN unzip instantclient-sqlplus-linux.x64-19.9.0.0.0dbru.zip

ENV PATH="/instantclient_19_9:${PATH}"
ENV LD_LIBRARY_PATH="/instantclient_19_9:${LD_LIBRARY_PATH}"

# Avoid sqlcmd termination due to locale -- see https://github.com/Microsoft/mssql-docker/issues/163
RUN echo "nb_NO.UTF-8 UTF-8" > /etc/locale.gen
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
RUN locale-gen

RUN echo "alias python=python3" >> ~/.bash_aliases

FROM binary-for-oracle as binary-only

RUN  apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    grass \
    iproute2 \
    postgresql-client \
    spawn-fcgi \
  && pip3 install \
    psycopg2 \
  && apt-get clean

# HANA: client side
# Install hdbsql tool
RUN curl -v -j -k -L -H "Cookie: eula_3_1_agreed=tools.hana.ondemand.com/developer-license-3_1.txt" https://tools.hana.ondemand.com/additional/hanaclient-latest-linux-x64.tar.gz --output hanaclient-latest-linux-x64.tar.gz \
  && tar -xvf hanaclient-latest-linux-x64.tar.gz \
  && mkdir /usr/sap \
  && ./client/hdbinst -a client --sapmnt=/usr/sap \
  && rm -rf client \
  && rm hanaclient*
ENV PATH="/usr/sap/hdbclient:${PATH}"

# MSSQL: client side
RUN curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
RUN curl https://packages.microsoft.com/config/ubuntu/19.04/prod.list | tee /etc/apt/sources.list.d/msprod.list
RUN apt-get update
RUN ACCEPT_EULA=Y apt-get install -y --allow-unauthenticated msodbcsql17 mssql-tools

# OTB: download and install otb packages for QGIS tests
RUN curl -k https://www.orfeo-toolbox.org/packages/archives/OTB/OTB-7.1.0-Linux64.run -o /tmp/OTB-Linux64.run && sh /tmp/OTB-Linux64.run --target /opt/otb
ENV OTB_INSTALL_DIR=/opt/otb


FROM binary-only

RUN  apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    bison \
    ccache \
    clang \
    cmake \
    flex \
    git \
    grass-dev \
    libexiv2-dev \
    libexpat1-dev \
    libfcgi-dev \
    libgdal-dev \
    libgeos-dev \
    libgsl-dev \
    libpdal-dev \
    libpq-dev \
    libproj-dev \
    libprotobuf-dev \
    libqca-qt5-2-dev \
    libqt5opengl5-dev \
    libqt5scintilla2-dev \
    libqt5svg5-dev \
    libqt5webkit5-dev \
    libqt5serialport5-dev \
    libqwt-qt5-dev \
    libspatialindex-dev \
    libspatialite-dev \
    libsqlite3-dev \
    libsqlite3-mod-spatialite \
    libzip-dev \
    libzstd-dev \
    ninja-build \
    protobuf-compiler \
    pyqt5-dev \
    pyqt5-dev-tools \
    pyqt5.qsci-dev \
    python3-all-dev \
    python3-dev \
    python3-sip-dev \
    qt3d5-dev \
    qt5keychain-dev \
    qtbase5-dev \
    qtdeclarative5-dev-tools \
    qtpositioning5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    qtbase5-private-dev \
    opencl-headers \
    ocl-icd-opencl-dev \
  && apt-get clean

ENV PATH="/usr/local/bin:${PATH}"

# environment variables shall be located in .docker/docker-variables.env
