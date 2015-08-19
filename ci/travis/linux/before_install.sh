export DEBIAN_FRONTEND=noninteractive
sudo add-apt-repository ppa:ubuntugis/ppa -y
sudo add-apt-repository ppa:ubuntugis/ubuntugis-unstable -y # For postgresql-9.1-postgis-2.1
sudo add-apt-repository ppa:grass/grass-stable -y
sudo add-apt-repository ppa:smspillaz/cmake-3.0.2 -y
sudo add-apt-repository ppa:kedazo/doxygen-updates-precise -y # For doxygen 1.8.8
if [ ${QT} == 5 ]; then
  sudo add-apt-repository ppa:ubuntu-sdk-team/ppa -y
fi

sudo apt-get update -qq

sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
                         bison cmake cmake-data doxygen flex git graphviz \
                         grass-dev grass7-dev libexpat1-dev libfcgi-dev \
                         libgdal1-dev libgeos-dev libgsl0-dev libpq-dev \
                         libproj-dev \
                         libspatialindex-dev libspatialite-dev \
                         libsqlite3-dev lighttpd pkg-config poppler-utils \
                         python python-dev \
                         python-qt4-dev python-sip python-sip-dev spawn-fcgi \
                         txt2tags xauth xfonts-100dpi xfonts-75dpi xfonts-base \
                         xfonts-scalable xvfb \
                         postgresql-9.1-postgis-2.1/precise # postgis one from ubuntugis-unstable, not pgdg

if [ ${QT} == 5 ]; then
  sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
    qtbase5-dev qtdeclarative5-dev libqt5webkit5-dev qt5-default \
    qttools5-dev-tools libqt5svg5-dev libqt5gui5-dev libqt5widgets5-dev libqt5network5-dev \
    libqt5xml5-dev libqt5concurrent5-dev libqt5printsupport5-dev libqt5positioning5-dev

  # Download build dependencies
  git clone https://github.com/osakared/qwt.git
  wget http://sourceforge.net/projects/pyqt/files/QScintilla2/QScintilla-2.9/QScintilla-gpl-2.9.tar.gz
  tar -xvf QScintilla-gpl-2.9.tar.gz
else
  sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
    libqt4-dev libqt4-opengl-dev libqt4-sql-sqlite libqtwebkit-dev \
    pyqt4-dev-tools python-qt4 python-qt4-dev libqscintilla2-dev libqwt-dev 
fi

cmake --version
clang --version
