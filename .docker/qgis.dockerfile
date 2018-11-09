
# CACHE_TAG is provided by Docker cloud
# see https://docs.docker.com/docker-cloud/builds/advanced/
# using ARG in FROM requires min v17.05.0-ce
ARG DOCKER_TAG=latest
ARG CACHE_DIR

FROM  qgis/qgis3-build-deps:${DOCKER_TAG}
MAINTAINER Denis Rouzaud <denis@opengis.ch>

ENV CC=/usr/lib/ccache/clang
ENV CXX=/usr/lib/ccache/clang++
ENV QT_SELECT=5
ENV LANG=C.UTF-8

COPY . /usr/src/QGIS

COPY ${CACHE_DIR} /root/.ccache
ENV CCACHE_DIR=/root/.ccache
RUN ccache -M 1G

WORKDIR /usr/src/QGIS/build

RUN cmake \
  -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DWITH_DESKTOP=OFF \
  -DWITH_SERVER=ON \
  -DWITH_3D=ON \
  -DWITH_BINDINGS=ON \
  -DBINDINGS_GLOBAL_INSTALL=ON \
  -DWITH_STAGED_PLUGINS=ON \
  -DWITH_GRASS=ON \
  -DSUPPRESS_QT_WARNINGS=ON \
  -DDISABLE_DEPRECATED=ON \
  -DENABLE_TESTS=OFF \
  -DWITH_QSPATIALITE=ON \
  -DWITH_APIDOC=OFF \
  -DWITH_ASTYLE=OFF \
 .. \
 && ninja install \
 && rm -rf /usr/src/QGIS

WORKDIR /
