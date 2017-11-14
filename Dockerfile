
# CACHE_TAG is provided by Docker cloud
# see https://docs.docker.com/docker-cloud/builds/advanced/
# using ARG in FROM requires min v17.05.0-ce
ARG  CACHE_TAG=latest

FROM  qgis/qgis3-build-deps:${CACHE_TAG}
MAINTAINER Denis Rouzaud <denis.rouzaud@gmail.com>

ENV CC=/usr/lib/ccache/clang
ENV CXX=/usr/lib/ccache/clang++
ENV QT_SELECT=5
ENV LANG=C.UTF-8

COPY . /usr/src/QGIS

WORKDIR /usr/src/QGIS/build

RUN cmake \
 -GNinja \
 -DCMAKE_INSTALL_PREFIX=/usr \
 -DBINDINGS_GLOBAL_INSTALL=ON \
 -DWITH_STAGED_PLUGINS=ON \
 -DWITH_GRASS=ON \
 -DSUPPRESS_QT_WARNINGS=ON \
 -DENABLE_TESTS=OFF \
 -DWITH_QSPATIALITE=ON \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_DESKTOP=ON \
 -DWITH_BINDINGS=ON \
 -DDISABLE_DEPRECATED=ON \
 .. \
 && ninja install \
 && rm -rf /usr/src/QGIS

WORKDIR /
