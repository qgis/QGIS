export DEBIAN_FRONTEND=noninteractive

pushd ${HOME}

curl -L https://github.com/opengisch/osgeo4travis/archive/qt4bin.tar.gz | tar -xzC /home/travis --strip-components=1
curl -L https://cmake.org/files/v3.5/cmake-3.5.0-Linux-x86_64.tar.gz | tar --strip-components=1 -zxC /home/travis/osgeo4travis

popd
pip install --user autopep8 nose2 pyyaml mock future
