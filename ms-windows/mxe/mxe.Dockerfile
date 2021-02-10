# Build deps for QGIS Windows binaries using MXE cross compile environment
# Author: Alessandro Pasotti

FROM ubuntu:focal

RUN chown root:root /tmp && chmod ugo+rwXt /tmp
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    dirmngr \
    software-properties-common \
    lsb-release \
    gpg-agent \
    python3-distutils \
    ccache

RUN apt-key adv \
    --keyserver keyserver.ubuntu.com \
    --recv-keys 86B72ED9 && \
    add-apt-repository \
    "deb [arch=amd64] https://pkg.mxe.cc/repos/apt `lsb_release -sc` main" && \
    apt-get update

RUN DEBIAN_FRONTEND=noninteractive apt install -y \
    mxe-x86-64-w64-mingw32.shared-gdal \
    mxe-x86-64-w64-mingw32.shared-qca \
    mxe-x86-64-w64-mingw32.shared-qtlocation \
    mxe-x86-64-w64-mingw32.shared-qscintilla2 \
    mxe-x86-64-w64-mingw32.shared-qwt \
    mxe-x86-64-w64-mingw32.shared-qtkeychain \
    mxe-x86-64-w64-mingw32.shared-qtserialport \
    mxe-x86-64-w64-mingw32.shared-qtwebkit \
    mxe-x86-64-w64-mingw32.shared-qtwinextras \
    mxe-x86-64-w64-mingw32.shared-exiv2 \
    mxe-x86-64-w64-mingw32.shared-protobuf \
    mxe-x86-64-w64-mingw32.shared-zlib \
    mxe-x86-64-w64-mingw32.shared-libzip \
    mxe-x86-64-w64-mingw32.shared-libspatialindex \
    mxe-x86-64-w64-mingw32.shared-gsl \
    mxe-x86-64-w64-mingw32.shared-zstd

# For QT SQL driver installation as an unprivileged user
RUN chmod -R 777 /usr/lib/mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/
