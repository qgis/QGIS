
# CACHE_TAG is provided by Docker cloud
# see https://docs.docker.com/docker-cloud/builds/advanced/
# using ARG in FROM requires min v17.05.0-ce
ARG DOCKER_TAG=latest

FROM  qgis/qgis3-build-deps:${DOCKER_TAG}
MAINTAINER Denis Rouzaud <denis@opengis.ch>

LABEL Description="Docker container with QGIS" Vendor="QGIS.org" Version="1.1"

ENV CC=/usr/lib/ccache/clang
ENV CXX=/usr/lib/ccache/clang++
ENV QT_SELECT=5
ENV LANG=C.UTF-8

COPY . /usr/src/QGIS

# If this directory is changed, also adapt script.sh which copies the directory
RUN mkdir -p /usr/src/.ccache_image_build
ENV CCACHE_DIR=/usr/src/.ccache_image_build
RUN ccache -M 1G
RUN ccache -s

WORKDIR /usr/src/QGIS/build

RUN cmake \
  -GNinja \
  -DUSE_CCACHE=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DWITH_DESKTOP=ON \
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
  -DQT5_3DEXTRA_LIBRARY="/usr/lib/x86_64-linux-gnu/libQt53DExtras.so" \
  -DQT5_3DEXTRA_INCLUDE_DIR="/usr/src/QGIS/external/qt3dextra-headers" \
  -DCMAKE_PREFIX_PATH="/usr/src/QGIS/external/qt3dextra-headers/cmake" \
 .. \
 && ninja install \
 && rm -rf /usr/src/QGIS

################################################################################
# Python testing environment setup

# Add QGIS test runner
COPY .docker/qgis_resources/test_runner/qgis_* /usr/bin/

# Make all scripts executable
RUN chmod +x /usr/bin/qgis_*

# Add supervisor service configuration script
COPY .docker/qgis_resources/supervisor/supervisord.conf /etc/supervisor/
COPY .docker/qgis_resources/supervisor/supervisor.xvfb.conf /etc/supervisor/supervisor.d/

# Python paths are for
# - kartoza images (compiled)
# - deb installed
# - built from git
# needed to find PyQt wrapper provided by QGIS
ENV PYTHONPATH=/usr/share/qgis/python/:/usr/share/qgis/python/plugins:/usr/lib/python3/dist-packages/qgis:/usr/share/qgis/python/qgis


WORKDIR /

# Run supervisor
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/supervisord.conf"]
