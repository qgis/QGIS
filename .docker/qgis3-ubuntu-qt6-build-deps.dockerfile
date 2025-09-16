
ARG DISTRO_VERSION=25.04
ARG PDAL_VERSION=2.8.4

# Oracle Docker image is too large, so we add as less dependencies as possible
# so there is enough space on GitHub runner
FROM      ubuntu:${DISTRO_VERSION} AS binary-for-oracle
LABEL org.opencontainers.image.authors="Denis Rouzaud <denis@opengis.ch>"

LABEL Description="Docker container with QGIS dependencies" Vendor="QGIS.org" Version="1.0"

ARG PDAL_VERSION

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
    git \
    gdal-bin \
    gnupg \
    gpsbabel \
    graphviz \
    'libaio1|libaio1t64' \
    'libdraco4|libdraco8' \
    libexiv2-28 \
    'libfcgi0ldbl|libfcgi0t64' \
    libgsl28 \
    'libprotobuf-lite17|libprotobuf-lite23|libprotobuf-lite32t64' \
    libqca-qt6-plugins \
    libqt63dextras6 \
    libqt63drender6 \
    libqt6concurrent6 \
    libqt6keychain1 \
    libqt6positioning6 \
    libqt6multimedia6 \
    libqt6multimediawidgets6 \
    libqt6qml6 \
    libqt6quick6 \
    libqt6quickcontrols2-6 \
    libqt6quickwidgets6 \
    libqt6serialport6 \
    libqt6sql6-odbc \
    libqt6sql6-sqlite \
    libqt6uitools6 \
    libqt6xml6 \
    libqt6pdf6 \
    libqt6webengine6-data \
    libqt6webenginecore6 \
    libqt6webenginecore6-bin \
    libqt6webenginewidgets6 \
    libsqlite3-mod-spatialite \
    'libzip4|libzip5|libzip4t64' \
    lighttpd \
    locales \
    poppler-utils \
    pyqt6-dev \
    pyqt6-dev-tools \
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
    python3-requests \
    python3-shapely  \
    python3-sphinx \
    python3-six \
    python3-termcolor \
    python3-yaml \
    qpdf \
    qt6-3d-assimpsceneimport-plugin \
    qt6-3d-defaultgeometryloader-plugin \
    qt6-3d-gltfsceneio-plugin \
    qt6-3d-scene2d-plugin \
    qt6-image-formats-plugins \
    saga \
    libsfcgal2 \
    supervisor \
    unzip \
    xauth \
    xfonts-100dpi \
    xfonts-75dpi \
    xfonts-base \
    xfonts-scalable \
    xvfb \
    ocl-icd-libopencl1
RUN  pip3 install --break-system-packages \
    future \
    capturer \
    hdbcli
RUN  apt-get clean

# Node.js and Yarn for server landingpage webapp
RUN mkdir -p /etc/apt/keyrings
RUN curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg
RUN echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_22.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list
RUN apt-get update
RUN apt-get install -y nodejs
RUN corepack enable

# Oracle : client side
RUN curl https://download.oracle.com/otn_software/linux/instantclient/2116000/instantclient-basic-linux.x64-21.16.0.0.0dbru.zip > instantclient-basic-linux.x64-21.16.0.0.0dbru.zip
RUN curl https://download.oracle.com/otn_software/linux/instantclient/2116000/instantclient-sdk-linux.x64-21.16.0.0.0dbru.zip > instantclient-sdk-linux.x64-21.16.0.0.0dbru.zip
RUN curl https://download.oracle.com/otn_software/linux/instantclient/2116000/instantclient-sqlplus-linux.x64-21.16.0.0.0dbru.zip > instantclient-sqlplus-linux.x64-21.16.0.0.0dbru.zip

RUN unzip -n instantclient-basic-linux.x64-21.16.0.0.0dbru.zip
RUN unzip -n instantclient-sdk-linux.x64-21.16.0.0.0dbru.zip
RUN unzip -n instantclient-sqlplus-linux.x64-21.16.0.0.0dbru.zip

ENV PATH="/instantclient_21_16:${PATH}"
ENV LD_LIBRARY_PATH="/instantclient_21_16"
# workaround noble libaio SONAME issue -- see https://bugs.launchpad.net/ubuntu/+source/libaio/+bug/2067501
RUN if [ -e /usr/lib/x86_64-linux-gnu/libaio.so.1t64 ] ; then ln -sf /usr/lib/x86_64-linux-gnu/libaio.so.1t64 /usr/lib/x86_64-linux-gnu/libaio.so.1 ; fi

# Avoid sqlcmd termination due to locale -- see https://github.com/Microsoft/mssql-docker/issues/163
RUN echo "nb_NO.UTF-8 UTF-8" > /etc/locale.gen
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
RUN locale-gen

RUN echo "alias python=python3" >> ~/.bash_aliases

# PDAL is not available in ubuntu 24.04
# Install it from source
# PDAL dependencies
RUN  apt-get update \
     && DEBIAN_FRONTEND=noninteractive apt-get install -y \
     ninja-build \
     libgdal-dev \
     libproj-dev
# download PDAL and compile it
RUN curl -L https://github.com/PDAL/PDAL/releases/download/${PDAL_VERSION}/PDAL-${PDAL_VERSION}-src.tar.gz --output PDAL-${PDAL_VERSION}-src.tar.gz \
    && mkdir pdal \
    && tar zxf PDAL-${PDAL_VERSION}-src.tar.gz -C pdal --strip-components=1 \
    && rm -f PDAL-${PDAL_VERSION}-src.tar.gz \
    && mkdir -p pdal/build \
    && cd pdal/build \
    && cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_TESTS=OFF .. \
    && ninja \
    && ninja install

# download spatialindex and compile it
RUN curl -L https://github.com/libspatialindex/libspatialindex/releases/download/2.0.0/spatialindex-src-2.0.0.tar.gz --output spatialindex-src-2.0.0.tar.gz \
    && mkdir spatialindex \
    && tar zxf spatialindex-src-2.0.0.tar.gz -C spatialindex --strip-components=1 \
    && rm -f spatialindex-src-2.0.0.tar.gz \
    && mkdir -p spatialindex/build \
    && cd spatialindex/build \
    && cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && ninja \
    && ninja install

RUN 
FROM binary-for-oracle AS binary-only

RUN  apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    grass \
    iproute2 \
    postgresql-client \
    spawn-fcgi \
  && pip3 install --break-system-packages \
    psycopg2 \
  && apt-get clean

# HANA: client side
# Install hdbsql tool
RUN curl -j -k -L -H "Cookie: eula_3_2_agreed=tools.hana.ondemand.com/developer-license-3_2.txt" https://tools.hana.ondemand.com/additional/hanaclient-latest-linux-x64.tar.gz --output hanaclient-latest-linux-x64.tar.gz \
  && tar -xvf hanaclient-latest-linux-x64.tar.gz \
  && mkdir /usr/sap \
  && ./client/hdbinst -a client --sapmnt=/usr/sap \
  && rm -rf client \
  && rm hanaclient*
ENV PATH="/usr/sap/hdbclient:${PATH}"

# MSSQL: client side
RUN curl -sSL -O https://packages.microsoft.com/ubuntu/24.10/prod/pool/main/p/packages-microsoft-prod/packages-microsoft-prod_1.1-ubuntu24.10_all.deb
RUN dpkg -i packages-microsoft-prod_1.1-ubuntu24.10_all.deb
RUN rm packages-microsoft-prod_1.1-ubuntu24.10_all.deb
RUN apt-get update
RUN ACCEPT_EULA=Y apt-get install -y --allow-unauthenticated msodbcsql18 mssql-tools18
ENV PATH="/opt/mssql-tools18/bin:${PATH}"

FROM binary-only

RUN  apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    bison \
    ccache \
    clang \
    cmake \
    flex \
    mold \
    grass-dev \
    libdraco-dev \
    libexiv2-dev \
    libexpat1-dev \
    libfcgi-dev \
    libgeos-dev \
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
    pyqt6.qsci-dev \
    python3-pyqt6.sip \
    python3-all-dev \
    python3-dev \
    python3-sipbuild \
    python3-pyqtbuild \
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
  && apt-get clean

ENV PATH="/usr/local/bin:${PATH}"

# environment variables shall be located in .docker/docker-variables.env
