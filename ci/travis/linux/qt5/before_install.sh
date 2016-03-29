export DEBIAN_FRONTEND=noninteractive
export CORES=2

##################################################
#
# Get precompiled dependencies
#
##################################################

pushd ${HOME}

curl -L https://github.com/opengisch/osgeo4travis/raw/binary-py33/osgeo4travis.tar.xz | tar -JxC /home/travis
curl -L https://cmake.org/files/v3.5/cmake-3.5.0-Linux-x86_64.tar.gz | tar --strip-components=1 -zxC /home/travis/osgeo4travis
popd

pip install psycopg2 numpy nose2 pyyaml mock future
