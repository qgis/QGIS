
# see https://docs.docker.com/docker-cloud/builds/advanced/
# using ARG in FROM requires min v17.05.0-ce
ARG DOCKER_DEPS_TAG=latest

FROM  qgis/qgis3-build-deps:${DOCKER_DEPS_TAG} AS BUILDER
MAINTAINER Denis Rouzaud <denis@opengis.ch>

LABEL Description="Docker container with QGIS" Vendor="QGIS.org" Version="1.1"

# build timeout in seconds, so no timeout by default
ARG BUILD_TIMEOUT=360000

ARG CC=/usr/lib/ccache/gcc
ARG CXX=/usr/lib/ccache/g++
ENV LANG=C.UTF-8

COPY . /QGIS

# If this directory is changed, also adapt script.sh which copies the directory
# if ccache directory is not provided with the source
RUN mkdir -p /QGIS/.ccache_image_build
ENV CCACHE_DIR=/QGIS/.ccache_image_build
RUN ccache -M 1G
RUN ccache -s

RUN echo "ccache_dir: "$(du -h --max-depth=0 ${CCACHE_DIR})

WORKDIR /QGIS/build

RUN SUCCESS=OK \
  && cmake \
  -GNinja \
  -DUSE_CCACHE=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DWITH_DESKTOP=ON \
  -DWITH_SERVER=ON \
  -DWITH_3D=ON \
  -DWITH_BINDINGS=ON \
  -DWITH_CUSTOM_WIDGETS=ON \
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
  && ninja install || SUCCESS=FAILED \
  && echo "$SUCCESS" > /QGIS/build_exit_value

# Additional run-time dependencies
RUN pip3 install jinja2 pygments pexpect && apt install -y expect

################################################################################
# Python testing environment setup

# Add QGIS test runner
COPY .docker/qgis_resources/test_runner/qgis_* /usr/bin/

# Make all scripts executable
RUN chmod +x /usr/bin/qgis_*

# Add supervisor service configuration script
COPY .docker/qgis_resources/supervisor/ /etc/supervisor

# Python paths are for
# - kartoza images (compiled)
# - deb installed
# - built from git
# needed to find PyQt wrapper provided by QGIS
ENV PYTHONPATH=/usr/share/qgis/python/:/usr/share/qgis/python/plugins:/usr/lib/python3/dist-packages/qgis:/usr/share/qgis/python/qgis

WORKDIR /

# Run supervisor
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/supervisord.conf"]
