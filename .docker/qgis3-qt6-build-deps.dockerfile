FROM fedora:rawhide
MAINTAINER Matthias Kuhn <matthias@opengis.ch>

RUN dnf -y install \
    bison \
    clang \
    clazy \
    exiv2-devel \
    fcgi-devel \
    flex \
    gdal-devel \
    geos-devel \
    gsl-devel \
    libpq-devel \
    libspatialite-devel \
    libzip-devel \
    libzstd-devel \
    ninja-build \
    proj-devel \
    protobuf-devel \
    protobuf-lite-devel \
    qt6-qt3d-devel \
    qt6-qtbase-devel \
    qt6-qttools-static \
    spatialindex-devel \
    sqlite-devel \
    unzip
