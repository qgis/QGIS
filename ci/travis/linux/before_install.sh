export DEBIAN_FRONTEND=noninteractive
sudo add-apt-repository ppa:ubuntugis/ppa -y
sudo add-apt-repository ppa:ubuntugis/ubuntugis-unstable -y # For postgresql-9.1-postgis-2.1
sudo add-apt-repository ppa:grass/grass-stable -y
sudo add-apt-repository ppa:smspillaz/cmake-3.0.2 -y
sudo add-apt-repository ppa:kedazo/doxygen-updates-precise -y # For doxygen 1.8.8
sudo apt-get update -qq
sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
        bison \
        cmake \
        cmake-data \
        doxygen \
        flex \
        git \
        graphviz \
        grass-dev \
        grass7-dev \
        libexpat1-dev \
        libfcgi-dev \
        libgdal1-dev \
        libgeos++-dev \
        libgeos-dev \
        libgsl0-dev \
        libpq-dev \
        libproj-dev \
        libqscintilla2-dev \
        libqtwebkit-dev \
        libqwt-dev \
        libspatialindex-dev \
        libspatialite-dev \
        libsqlite3-dev \
        lighttpd \
        pkg-config \
        poppler-utils \
        python \
        python-dev \
        python-sip \
        python-sip-dev \
        spawn-fcgi \
        txt2tags \
        xauth \
        xfonts-100dpi \
        xfonts-75dpi \
        xfonts-base \
        xfonts-scalable \
        xvfb \
        postgresql-9.1-postgis-2.1/precise # from ubuntugis-unstable, not pgdg

if [ ${QT} == 5 ]; then
  sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
    libqwt-qt5-dev \
    libqt5scintilla2-dev \
    libqca-qt5-2-dev \
    libqca-qt5-2-plugins \
    libqt5xmlpatterns5-dev \
    libqt5svg5-dev
else
  sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
    libqca2-dev \
    libqca2-plugin-ossl \
    libqt4-dev \
    libqt4-opengl-dev \
    libqt4-sql-sqlite \
    python-qt4 \
    python-qt4-dev \
    python-qt4-sql \
    pyqt4-dev-tools \
fi

cmake --version
clang --version
