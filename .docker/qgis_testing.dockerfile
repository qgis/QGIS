
ARG DOCKER_TAG=latest
FROM  qgis/qgis3-build-deps:${DOCKER_TAG}
MAINTAINER Denis Rouzaud <denis@opengis.ch>

LABEL Description="Docker container with QGIS build for testing" Vendor="QGIS.org" Version="1.1"

ENV CC=/usr/lib/ccache/clang
ENV CXX=/usr/lib/ccache/clang++
ENV QT_SELECT=5
ENV LANG=C.UTF-8
############################
# Setup the (c)test environment
ENV LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so
ENV SEGFAULT_SIGNALS="abrt segv"
ENV CTEST_BUILD_COMMAND="/usr/bin/ninja"
ENV CTEST_PARALLEL_LEVEL=1


##############################
# Copy source
COPY . /usr/src/QGIS
WORKDIR /usr/src/QGIS/build

##############################
# ccache
ENV CCACHE_DIR=/usr/src/QGIS/.ccache
RUN ccache -M 1G
RUN ccache -s
# Temporarily uncomment to debug ccache issues
# export CCACHE_LOGFILE=/tmp/cache.debug
RUN ccache -z


###########
# Configure
RUN echo "travis_fold:start:cmake"
RUN echo "Running cmake..."
RUN cmake \
 -GNinja \
 -DUSE_CCACHE=OFF \
 -DWITH_QUICK=ON \
 -DWITH_3D=ON \
 -DWITH_STAGED_PLUGINS=ON \
 -DWITH_GRASS=OFF \
 -DSUPPRESS_QT_WARNINGS=ON \
 -DENABLE_MODELTEST=ON \
 -DENABLE_PGTEST=ON \
 -DENABLE_MSSQLTEST=ON \
 -DWITH_QSPATIALITE=ON \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_DESKTOP=ON \
 -DWITH_BINDINGS=ON \
 -DWITH_SERVER=ON \
 -DDISABLE_DEPRECATED=ON \
 -DPYTHON_TEST_WRAPPER="timeout -sSIGSEGV 55s"\
 -DCXX_EXTRA_FLAGS="${CLANG_WARNINGS}" \
 -DWERROR=TRUE \
 -DQT5_3DEXTRA_LIBRARY="/usr/lib/x86_64-linux-gnu/libQt53DExtras.so" \
 -DQT5_3DEXTRA_INCLUDE_DIR="/root/QGIS/external/qt3dextra-headers" \
 -DCMAKE_PREFIX_PATH="/root/QGIS/external/qt3dextra-headers/cmake" \
 ..
RUN echo "travis_fold:end:cmake"

#######
# Build
#######
# Calculate the timeout for building.
# The tests should be aborted before travis times out, in order to allow uploading
# the ccache and therefore speedup subsequent e builds.
#
# Travis will kill the job after approx 150 minutes, we subtract 5 minutes for
# uploading and subtract the bootstrapping time from that.
# Hopefully clocks are in sync :)
ENV TRAVIS_TIME=150
ENV UPLOAD_TIME=5
RUN export CURRENT_TIME=$(date +%s) \
 && export TIMEOUT=$((( TRAVIS_TIME - UPLOAD_TIME ) * 60 - CURRENT_TIME + TRAVIS_TIMESTAMP)) \
 && export TIMEOUT=$(( TIMEOUT < 300 ? 300 : TIMEOUT )) \
 && echo "Timeout: ${TIMEOUT}s (started at ${TRAVIS_TIMESTAMP}, current: ${CURRENT_TIME})"

# echo "travis_fold:start:ninja-build.1"
RUN echo "Building QGIS..."
RUN timeout ${TIMEOUT}s ${CTEST_BUILD_COMMAND} && rv=$? && \
  if [ $rv -eq 124 ] ; then printf '\n\nBuild and test timeout. Please restart the build for meaningful results.\n'; exit #$rv; fi


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
