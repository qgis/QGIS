FROM      ubuntu:18.04
MAINTAINER Denis Rouzaud <denis@opengis.ch>

LABEL Description="Docker container with QGIS dependencies" Vendor="QGIS.org" Version="1.0"

# && echo "deb http://ppa.launchpad.net/ubuntugis/ubuntugis-unstable/ubuntu xenial main" >> /etc/apt/sources.list \
# && echo "deb-src http://ppa.launchpad.net/ubuntugis/ubuntugis-unstable/ubuntu xenial main" >> /etc/apt/sources.list \
# && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 314DF160 \

RUN  apt-get update \
  && apt-get install -y software-properties-common \
  && apt-get update \
  && apt-get install -y \
    bison \
    ca-certificates \
    ccache \
    clang \
    cmake \
    curl \
    dh-python \
    flex \
    gdal-bin \
    git \
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
    libqt53drender5 \
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
    python3-termcolor \
    python3-yaml \
    qt3d5-dev \
    qt3d-assimpsceneimport-plugin \
    qt3d-defaultgeometryloader-plugin \
    qt3d-gltfsceneio-plugin \
    qt3d-scene2d-plugin \
    qt5keychain-dev \
    qtbase5-dev \
    qtpositioning5-dev \
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
  && apt-get autoremove -y python3-pip python2.7 sip-dev \
  && apt-get clean

  RUN bash -c "echo $(sip -V)"
  RUN bash -c "echo $(which sip)"

WORKDIR /root
RUN curl -s -S -O https://www.riverbankcomputing.com/hg/sip/archive/tip.tar.gz \
 && tar xzf tip.tar.gz \
 && mkdir /root/sip419 \
 && mv  $(find -type d -iname 'sip-*')/* sip419
WORKDIR /root/sip419
RUN python3 build.py prepare \
 && python3 configure.py \
 && make \
 && make install

 RUN bash -c "echo $(sip -V)"
 RUN bash -c "echo $(which sip)"

# WORKDIR /root
# RUN curl -s -S -O https://svwh.dl.sourceforge.net/project/pyqt/PyQt5/PyQt-5.9.2/PyQt5_gpl-5.9.2.tar.gz \
#  && tar xzf PyQt5_gpl-5.9.2.tar.gz
# WORKDIR /root/PyQt5_gpl-5.9.2
# RUN apt-get install -y qt5-qmake \
#  && python3 configure.py --confirm-license --qmake /usr/lib/x86_64-linux-gnu/qt5/bin/qmake --qsci-api --enable QtSql --enable QtSvg && make && make install
#
# WORKDIR /root
# RUN curl -s -S -O https://cfhcable.dl.sourceforge.net/project/pyqt/QScintilla2/QScintilla-2.10.3/QScintilla_gpl-2.10.3.tar.gz
#  && tar xzf QScintilla_gpl-2.10.3.tar.gz
# WORKDIR /root/QScintilla_gpl-2.10.3/Python
# RUN python3 configure.py --pyqt=PyQt5
#  && make
#  && make install

WORKDIR /root

RUN echo "alias python=python3" >> ~/.bash_aliases

ENV CC=/usr/lib/ccache/clang
ENV CXX=/usr/lib/ccache/clang++
ENV QT_SELECT=5
ENV LANG=C.UTF-8
ENV PATH="/usr/local/bin:${PATH}"

CMD /root/QGIS/.ci/travis/linux/docker-build-test.sh
