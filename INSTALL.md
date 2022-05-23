Building QGIS from source - step by step

<!-- Table of contents generated with https://freelance-tech-writer.github.io/table-of-contents-generator/index.html -->

# Table of Contents
- [1. Introduction](#1-introduction)
- [2. Overview](#2-overview)
- [3. Building on GNU/Linux](#3-building-on-gnulinux)
  - [3.1. Building QGIS with Qt 5.x](#31-building-qgis-with-qt-5x)
  - [3.2. Prepare apt](#32-prepare-apt)
  - [3.3. Install build dependencies](#33-install-build-dependencies)
  - [3.4. Setup ccache (Optional, but recommended)](#34-setup-ccache-optional-but-recommended)
  - [3.5. Prepare your development environment](#35-prepare-your-development-environment)
  - [3.6. Check out the QGIS Source Code](#36-check-out-the-qgis-source-code)
  - [3.7. Starting the compile](#37-starting-the-compile)
  - [3.8. Compiling with 3D](#38-compiling-with-3d)
    - [3.8.1. Compiling with 3D on Debian based distributions](#381-compiling-with-3d-on-debian-based-distributions)
  - [3.9. Building different branches](#39-building-different-branches)
  - [3.10. Building Debian packages](#310-building-debian-packages)
  - [3.11. On Fedora Linux](#311-on-fedora-linux)
    - [3.11.1. Install build dependencies](#3111-install-build-dependencies)
    - [3.11.2. Suggested system tweaks](#3112-suggested-system-tweaks)
- [4. Building on Windows](#4-building-on-windows)
  - [4.1. Building with Microsoft Visual Studio](#41-building-with-microsoft-visual-studio)
    - [4.1.1. Visual Studio 2015 Community Edition](#411-visual-studio-2015-community-edition)
    - [4.1.2. Other tools and dependencies](#412-other-tools-and-dependencies)
    - [4.1.3. Clone the QGIS Source Code](#413-clone-the-qgis-source-code)
    - [4.1.4. Configure and build with CMake from command line](#414-configure-and-build-with-cmake-from-command-line)
      - [4.1.4.1 Using configonly.bat to create the MSVC solution file](#4141-using-configonlybat-to-create-the-msvc-solution-file)
      - [4.1.4.2 Compiling QGIS with MSVC](#4142-compiling-qgis-with-msvc)
    - [4.1.5 Old alternative method that might still work using cmake-gui](#415-old-alternative-method-that-might-still-work-using-cmake-gui)
    - [4.1.6. Packaging](#416-packaging)
    - [4.1.7. Packaging your own build of QGIS](#417-packaging-your-own-build-of-qgis)
    - [4.1.8. Osgeo4w packaging](#418-osgeo4w-packaging)
  - [4.2. Building on Linux with mingw64](#42-building-on-linux-with-mingw64)
    - [4.2.1. Building with Docker](#421-building-with-docker)
      - [4.2.1.1. Initial setup](#4211-initial-setup)
      - [4.2.1.2. Building the dependencies](#4212-building-the-dependencies)
      - [4.2.1.3. Cross-Building QGIS](#4213-cross-building-qgis)
    - [4.2.2. Testing QGIS](#422-testing-qgis)
- [5. Building on MacOS X](#5-building-on-macos-x)
  - [5.1. Install Developer Tools](#51-install-developer-tools)
  - [5.2. Install CMake and other build tools](#52-install-cmake-and-other-build-tools)
  - [5.3. Install Qt5 and QGIS-Deps](#53-install-qt5-and-qgis-deps)
  - [5.4. QGIS source](#54-qgis-source)
  - [5.5. Configure the build](#55-configure-the-build)
  - [5.6. Building](#56-building)
- [6. Setting up the WCS test server on GNU/Linux](#6-setting-up-the-wcs-test-server-on-gnulinux)
  - [6.1. Preparation](#61-preparation)
  - [6.2. Setup mapserver](#62-setup-mapserver)
  - [6.3. Create a home page](#63-create-a-home-page)
  - [6.4. Now deploy it](#64-now-deploy-it)
  - [6.5. Debugging](#65-debugging)
- [7. Setting up a Jenkins Build Server](#7-setting-up-a-jenkins-build-server)
- [8. Debug output and running tests](#8-debug-output-and-running-tests)
- [9. Authors and Acknowledgments](#9-authors-and-acknowledgments)

# 1. Introduction

This document is the original installation guide of the described software
QGIS. The software and hardware descriptions named in this
document are in most cases registered trademarks and are therefore subject
to the legal requirements. QGIS is subject to the GNU General Public
License. Find more information on the QGIS Homepage:
https://qgis.org

The details, that are given in this document have been written and verified
to the best of knowledge and responsibility of the editors. Nevertheless,
mistakes concerning the content are possible. Therefore, all data are not
liable to any duties or guarantees. The editors and publishers do not take
any responsibility or liability for failures and their consequences. You are
always welcome for indicating possible mistakes.

Because the code of QGIS evolves from release to release, These instructions are
regularly updated to match the corresponding release. Instructions for the current
master branch are available at https://github.com/qgis/QGIS/blob/master/INSTALL.md.
If you wish to build another version of QGIS, ensure to checkout the appropriate
release branch. The QGIS source code can be found [in the repository](https://github.com/qgis/QGIS).

Please visit https://qgis.org for information on joining our mailing lists
and getting involved in the project further.

**Note to document writers:** Please use this document as the central
place for describing build procedures. Please do not remove this notice.

# 2. Overview

QGIS, like a number of major projects (e.g., KDE 4.0),
uses [CMake](https://www.cmake.org) for building from source.

Following a summary of the required dependencies for building:

Required build tools:

* CMake >= 3.12.0
* Flex >= 2.5.6
* Bison >= 2.4
* Python >= 3.7

Required build dependencies:

* Qt >= 5.12.0
* Proj >= 4.9.3
* GEOS >= 3.4
* Sqlite3 >= 3.0.0
* SpatiaLite >= 4.2.0
* libspatialindex
* GDAL/OGR >= 2.1
* Qwt >= 5.0 & (< 6.1 with internal QwtPolar)
* expat >= 1.95
* QScintilla2
* QCA
* qtkeychain (>= 0.5)
* libzip

Optional dependencies:

* for GRASS providers and plugin - GRASS >= 7.0.0.
* for georeferencer - GSL >= 1.8
* for PostGIS support - PostgreSQL >= 8.0.x
* for gps plugin - gpsbabel
* for mapserver export and PyQGIS - Python >= 3.6
* for python support - SIP >= 4.12, PyQt >= 5.3 must match Qt version, Qscintilla2
* for qgis mapserver - FastCGI
* for oracle provider - Oracle OCI library

Indirect dependencies:

Some proprietary formats (e.g., ECW and MrSid) supported by GDAL require
proprietary third party libraries.  QGIS doesn't need any of those itself to
build, but will only support those formats if GDAL is built accordingly.  Refer
to [format list](https://gdal.org/index.html) for instructions how to include
those formats in GDAL.

# 3. Building on GNU/Linux

## 3.1. Building QGIS with Qt 5.x

**Requires:** Ubuntu / Debian derived distro

**Note:** Refer to the section Building Debian packages for building
debian packages. Unless you plan to develop on QGIS, that is probably the
easiest option to compile and install QGIS.

These notes are for Ubuntu - other versions and Debian derived distros may
require slight variations in package names.

These notes are for if you want to build QGIS from source. One of the major
aims here is to show how this can be done using binary packages for ***all***
dependencies - building only the core QGIS stuff from source. I prefer this
approach because it means we can leave the business of managing system packages
to apt and only concern ourselves with coding QGIS!

This document assumes you have made a fresh install and have a 'clean' system.
These instructions should work fine if this is a system that has already been
in use for a while, you may need to just skip those steps which are irrelevant
to you.

## 3.2. Prepare apt

The packages QGIS depends on to build are available in the "universe" component
of Ubuntu. This is not activated by default, so you need to activate it:

1. Edit your `/etc/apt/sources.list` file.
2. Uncomment all the lines starting with "deb"

Also you will need a recent enough distribution in order for all dependencies
to be met. The supported distributions are listed in the following section.

Now update your local sources database:

```bash
sudo apt-get update
```

## 3.3. Install build dependencies

|Distribution|Install command for packages|
|------------|----------------------------|
| bullseye | ``apt-get install bison ca-certificates ccache cmake cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpdal-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pdal pkg-config poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dateutil python3-dev python3-future python3-gdal python3-httplib2 python3-jinja2 python3-lxml python3-markupsafe python3-mock python3-nose2 python3-owslib python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtpositioning python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-requests python3-sip python3-sip-dev python3-six python3-termcolor python3-tz python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qt5keychain-dev qtbase5-dev qtbase5-private-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools saga spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| focal | ``apt-get install bison ca-certificates ccache cmake cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpdal-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pdal pkg-config poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dateutil python3-dev python3-future python3-gdal python3-httplib2 python3-jinja2 python3-lxml python3-markupsafe python3-mock python3-nose2 python3-owslib python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtpositioning python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-requests python3-sip python3-sip-dev python3-six python3-termcolor python3-tz python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qt5-default qt5keychain-dev qtbase5-dev qtbase5-private-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools saga spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| hirsute | ``apt-get install bison ca-certificates ccache cmake cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpdal-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pdal pkg-config poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dateutil python3-dev python3-future python3-gdal python3-httplib2 python3-jinja2 python3-lxml python3-markupsafe python3-mock python3-nose2 python3-owslib python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtpositioning python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-requests python3-sip python3-sip-dev python3-six python3-termcolor python3-tz python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qt5keychain-dev qtbase5-dev qtbase5-private-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools saga spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| impish | ``apt-get install bison ca-certificates ccache cmake cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpdal-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pdal pkg-config poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dateutil python3-dev python3-future python3-gdal python3-httplib2 python3-jinja2 python3-lxml python3-markupsafe python3-mock python3-nose2 python3-owslib python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtpositioning python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-requests python3-sip python3-six python3-termcolor python3-tz python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qt5keychain-dev qtbase5-dev qtbase5-private-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools saga sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| jammy | ``apt-get install bison ca-certificates ccache cmake cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpdal-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pdal pkg-config poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dateutil python3-dev python3-future python3-gdal python3-httplib2 python3-jinja2 python3-lxml python3-markupsafe python3-mock python3-nose2 python3-owslib python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtpositioning python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-requests python3-sip python3-six python3-termcolor python3-tz python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qt5keychain-dev qtbase5-dev qtbase5-private-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools saga sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| sid | ``apt-get install bison ca-certificates ccache cmake cmake-curses-gui dh-python doxygen expect flex flip gdal-bin git graphviz grass-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build ocl-icd-opencl-dev opencl-headers pandoc pkg-config poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dateutil python3-dev python3-future python3-gdal python3-httplib2 python3-jinja2 python3-lxml python3-markupsafe python3-mock python3-nose2 python3-owslib python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtpositioning python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-requests python3-sip python3-six python3-termcolor python3-tz python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qt5keychain-dev qtbase5-dev qtbase5-private-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools saga sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |

(extracted from the control.in file in `debian/`)

See [debian-ubuntu](https://qgis.org/en/site/forusers/alldownloads.html#debian-ubuntu) for
currently supported distributions (plain xenial's GDAL for instance is too old
and we build with GDAL2 from ubuntugis).

To build [QGIS server landing page/catalog webapp](https://docs.qgis.org/latest/en/docs/server_manual/services.html#qgis-server-catalog) additional dependencies are required:

Node.js (current LTS recommended): https://nodejs.org/en/download/<br>
Yarn Package Manager: https://yarnpkg.com/getting-started/install

Additionally, the cmake flag `WITH_SERVER_LANDINGPAGE_WEBAPP` needs to be turned on.

## 3.4. Setup ccache (Optional, but recommended)

You should also setup ccache to speed up compile times:

```bash
cd /usr/local/bin
sudo ln -s /usr/bin/ccache gcc
sudo ln -s /usr/bin/ccache g++
```

or simply add `/usr/lib/ccache` to your `PATH`.

## 3.5. Prepare your development environment

As a convention I do all my development work in $HOME/dev/<language>, so in
this case we will create a work environment for C++ development work like
this:

```bash
mkdir -p ${HOME}/dev/cpp
cd ${HOME}/dev/cpp
```

This directory path will be assumed for all instructions that follow.

## 3.6. Check out the QGIS Source Code

There are two ways the source can be checked out. Use the anonymous method
if you do not have edit privileges for the QGIS source repository, or use
the developer checkout if you have permissions to commit source code
changes.

1. Anonymous Checkout

```bash
cd ${HOME}/dev/cpp
git clone git://github.com/qgis/QGIS.git
```

2. Developer Checkout

```bash
cd ${HOME}/dev/cpp
git clone git@github.com:qgis/QGIS.git
```

## 3.7. Starting the compile

I compile my development version of QGIS into my ~/apps directory to avoid
conflicts with Ubuntu packages that may be under /usr. This way for example
you can use the binary packages of QGIS on your system along side with your
development version. I suggest you do something similar:

```bash
mkdir -p ${HOME}/apps
```

Now we create a build directory and run ccmake:

```bash
cd QGIS
mkdir build-master
cd build-master
ccmake ..
```

When you run ccmake (note the .. is required!), a menu will appear where
you can configure various aspects of the build:

* If you want QGIS to have debugging capabilities then set `CMAKE_BUILD_TYPE` to `Debug`.
* If you do not have root access or do not want to overwrite existing QGIS
  installs (by your package manager for example), set the `CMAKE_INSTALL_PREFIX`
  to somewhere you have write access to (For example `${HOME}/apps`).

Now press 'c' to configure, 'e' to dismiss any error messages that may appear.
and 'g' to generate the make files. Note that sometimes 'c' needs to
be pressed several times before the 'g' option becomes available.
After the 'g' generation is complete, press 'q' to exit the ccmake
interactive dialog.

**Warning:** Make sure that your build directory is completely empty when you
enter the command. Do never try to "re-use" an existing **Qt5** build directory.
If you want to use `ccmake` or other interactive tools, run the command in
the empty build directory once before starting to use the interactive tools.

Now on with the build:
```bash
make -jX
```

where X is the number of available cores. Depending on your platform,
this can speed up the build time considerably.

Then you can directly run from the build directory:
```bash
./output/bin/qgis
```
Another option is to install to your system:
```bash
make install
```

After that you can try to run QGIS:
```bash
$HOME/apps/bin/qgis
```
If all has worked properly the QGIS application should start up and appear
on your screen. If you get the error message "error while loading shared libraries",
execute this command in your shell.
```bash
sudo ldconfig
```
If that doesn't help add the install path to LD_LIBRARY_PATH:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/apps/lib/
```
Optionally, if you already know what aspects you want in your custom build
then you can skip the interactive ccmake .. part by using the cmake -D
option for each aspect, e.g.:
```bash
cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=${HOME}/apps ..
```
Also, if you want to speed your build times, you can easily do it with ninja,
an alternative to make with similar build options.

For example, to configure your build you can do either one of:
```bash
ccmake -G Ninja ..
cmake -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=${HOME}/apps ..
```
Build and install with ninja:
```bash
ninja   # (uses all cores by default; also supports the above described -jX option)
ninja install
```
To build even faster, you can build just the targets you need using, for example:
```bash
ninja qgis
ninja pycore
# if it's on desktop related code only:
ninja qgis_desktop
```

## 3.8. Compiling with 3D

In the cmake, you need to enable:
```bash
WITH_3D=True
```

### 3.8.1. Compiling with 3D on Debian based distributions

QGIS 3D requires Qt53DExtras. These headers have been removed
from Qt upstream on Debian based distributions. A copy has been made in the
QGIS repository in `external/qt3dextra-headers`.
To compile with 3D enabled, you need to add some cmake options:

```bash
CMAKE_PREFIX_PATH={path to QGIS Git repo}/external/qt3dextra-headers/cmake
QT5_3DEXTRA_INCLUDE_DIR={path to QGIS Git repo}/external/qt3dextra-headers
QT5_3DEXTRA_LIBRARY=/usr/lib/x86_64-linux-gnu/libQt53DExtras.so
Qt53DExtras_DIR={path to QGIS Git repo}/external/qt3dextra-headers/cmake/Qt53DExtras
```

## 3.9. Building different branches

By using `git worktree`, you can switch between different branches to use
several sources in parallel, based on the same Git configuration.
We recommend you to read the documentation about this Git command:

```bash
git commit
git worktree add ../my_new_functionality
cd ../my_new_functionality
git fetch qgis/master
git rebase -i qgis/master
# only keep the commits to be pushed
git push -u my_own_repo my_new_functionality
```

## 3.10. Building Debian packages

Instead of creating a personal installation as in the previous step you can
also create debian package. This is done from the QGIS root directory, where
you'll find a debian directory.

First you need to install the debian packaging tools once:

```bash
apt-get install build-essential
```

First you need to create an changelog entry for your distribution. For example
for Ubuntu Precise:

```bash
dch -l ~precise --force-distribution --distribution precise "precise build"
```

The QGIS packages will be created with:

```bash
dpkg-buildpackage -us -uc -b
```

**Note:** Install `devscripts` to get `dch`.

**Note:** If `dpkg-buildpackage` complains about unmet build dependencies
you can install them using `apt-get` and re-run the command.

**Note:** If you have `libqgis1-dev` installed, you need to remove it first
using `dpkg -r libqgis1-dev`.  Otherwise `dpkg-buildpackage` will complain about a
build conflict.

**Note:** By default tests are run in the process of building and their
results are uploaded to https://cdash.orfeo-toolbox.org/index.php?project=QGIS.
You can turn the tests off using `DEB_BUILD_OPTIONS=nocheck` in front of the
build command. The upload of results can be avoided with `DEB_TEST_TARGET=test`.

The packages are created in the parent directory (ie. one level up).
Install them using `dpkg`.  E.g.:

```bash
sudo debi
```

## 3.11. On Fedora Linux

We assume that you have the source code of QGIS ready and created a
new subdirectory called `build` or `build-qt5` in it.

### 3.11.1. Install build dependencies
|Distribution|Install command for packages|
|------------|----------------------------|
| Fedora 35 Workstation | ``dnf install qt5-qtbase-private-devel qt5-qtwebkit-devel qt5-qtlocation-devel qt5-qttools-static qca-qt5-devel qca-qt5-ossl qt5-qt3d-devel python3-qt5-devel python3-qscintilla-qt5-devel qscintilla-qt5-devel python3-qscintilla-qt5 clang flex bison geos-devel gdal-devel sqlite-devel libspatialite-devel qt5-qtsvg-devel spatialindex-devel expat-devel proj-devel qwt-qt5-devel gsl-devel postgresql-devel cmake python3-future gdal-python3 python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt5-ossl qwt-qt5-devel qtkeychain-qt5-devel qwt-devel libzip-devel exiv2-devel python3-sip-devel protobuf-lite protobuf-lite-devel libzstd-devel qt5-qtserialport-devel`` |
| older versions | ``dnf install qt5-qtbase-private-devel qt5-qtwebkit-devel qt5-qtlocation-devel qt5-qttools-static qca-qt5-devel qca-qt5-ossl qt5-qt3d-devel python3-qt5-devel python3-qscintilla-qt5-devel qscintilla-qt5-devel python3-qscintilla-devel python3-qscintilla-qt5 clang flex bison geos-devel gdal-devel sqlite-devel libspatialite-devel qt5-qtsvg-devel spatialindex-devel expat-devel proj-devel qwt-qt5-devel gsl-devel postgresql-devel cmake python3-future gdal-python3 python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt5-ossl qwt-qt5-devel qtkeychain-qt5-devel qwt-devel sip-devel libzip-devel exiv2-devel`` |

To build QGIS server additional dependencies are required:

```bash
dnf install fcgi-devel
```

And for building [QGIS server landing page/catalog webapp](https://docs.qgis.org/latest/en/docs/server_manual/services.html#qgis-server-catalog):

```bash
dnf install nodejs yarnpkg
```

Additionally, the cmake flag `WITH_SERVER_LANDINGPAGE_WEBAPP` needs to be turned on.

Make sure that your build directory is completely empty when you enter the
following command. Do never try to "re-use" an existing Qt5 build directory.
If you want to use `ccmake` or other interactive tools, run the following
command in the empty build directory once before starting to use the interactive
tools.

```bash
cmake ..
```

If everything went OK you can finally start to compile. (As usual append a `-jX`
where X is the number of available cores option to make to speed up your build
process)

```bash
make
```

Run from the build directory

```bash
./output/bin/qgis
```

Or install to your system

```bash
make install
```

### 3.11.2. Suggested system tweaks

By default Fedora disables debugging calls from Qt applications. This prevents
the useful debug output which is normally printed when running the unit tests.

To enable debug prints for the current user, execute:

```bash
cat > ~/.config/QtProject/qtlogging.ini << EOL
[Rules]
default.debug=true
EOL
```

# 4. Building on Windows

## 4.1. Building with Microsoft Visual Studio

This section describes how to build QGIS using Visual Studio (MSVC) 2015 on Windows.
This is currently also how the binary QGIS packages are made (earlier versions used MinGW).

This section describes the setup required to allow Visual Studio to be used to
build QGIS.

### 4.1.1. Visual Studio 2015 Community Edition

Download the [free (as in free beer) Community installer](https://download.microsoft.com/download/D/2/3/D23F4D0F-BA2D-4600-8725-6CCECEA05196/vs_community_ENU.exe)

Select "Custom" install and add the following packages:

* "Common Tools for Visual C++ 2015" under "Visual C++"
* "Tools (1.4.1) and Windows 10 SDK (10.0.14393)" under "Universal Windows App Development Tools".

### 4.1.2. Other tools and dependencies

Download and install following packages:

* [CMake](https://cmake.org/files/v3.12/cmake-3.12.3-win64-x64.msi)
* GNU flex, GNU bison and GIT with cygwin [32bit](https://cygwin.com/setup-x86.exe) or [64bit](https://cygwin.com/setup-x86_64.exe)
* OSGeo4W [32bit](https://download.osgeo.org/osgeo4w/osgeo4w-setup-x86.exe) or [64bit](https://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe)
* [ninja](https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip): Copy the `ninja.exe` to `C:\OSGeo4W64\bin\`

For the QGIS build you need to install following packages from cygwin:

* bison
* flex
* git (even if you already have Git for Windows installed)

and from OSGeo4W (select *Advanced Install*):

* qgis-dev-deps

  * This will also select packages the above packages depend on.

  * Note: If you install other packages, this might cause issues. Particularly, make sure
    **not** to install the msinttypes package. It installs a stdint.h file in
    OSGeo4W[64]\include, that conflicts with Visual Studio own stdint.h, which for
    example breaks the build of the virtual layer provider.

Earlier versions of this document also covered how to build all above
dependencies.  If you're interested in that, check the history of this page in the Wiki
or the SVN repository.

### 4.1.3. Clone the QGIS Source Code

Choose a directory to store the QGIS source code.
For example, to put it in the OSGeo4W64 install, navigate there:

```cmd
cd C:\OSGeo4W64
```

This directory will be assumed for all instructions
that follow.

On the command prompt clone the QGIS source from
git to the source directory `QGIS`:

```cmd
git clone git://github.com/qgis/QGIS.git
```

This requires Git. If you have Git for Windows on your PATH already,
you can do this from a normal command prompt. If you do not, you can
use the Git package that was installed as part of Cygwin by opening
a Cygwin[64] Terminal

And, to avoid Git in Windows reporting changes to files not actually modified:

```cmd
cd QGIS
git config core.filemode false
```

### 4.1.4. Configure and build with CMake from command line

**Note:** Consider this section as example.  It tends to outdate, when OSGeo4W and
SDKs move on.  `ms-windows/osgeo4w/package-nightly.cmd` is used for the
nightly builds and constantly updated and hence might contain necessary
updates that are not yet reflected here.

To start a command prompt with an environment that both has the VC++ and the OSGeo4W
variables create the following batch file (assuming the above packages were
installed in the default locations):

```cmd
@echo off
call C:\OSGeo4W64\QGIS\ms-windows\osgeo4w\msvc-env.bat x86_64
@cmd
```

Save the batch file as `C:\OSGeo4W64\OSGeo4W-dev.bat` and run it.

#### 4.1.4.1 Using configonly.bat to create the MSVC solution file
We will be using the file `ms-windows/osgeo4w/configonly.bat` to create an MSVC solution file.
There are two supported CMake generators for creating a solution file: Ninja, and native MSVC.
The advantage of using native MSVC solution is that you can find the root of build problems much more easily.
configonly.bat is meant to create a configured build directory with a MSVC solution file:

```cmd
cd C:\OSGeo4W64\QGIS\ms-windows\osgeo4w
configonly.bat
```

#### 4.1.4.2 Compiling QGIS with MSVC
We will need to run MSVC with all the environment variables set, thus we will run it as follows:
* Run the batch file OSGeo4W-dev.bat you created before.
* On the command prompt run `call gdal-dev-env.bat` to add the release gdal and proj libraries to your PATH.
* On the command prompt run `devenv` to open MSVC.
* From MSVC, open the solution file `C:\OSGeo4W64\QGIS\ms-windows\osgeo4w\build-qgis-test-x86_64\qgis.sln`.
* Try to build the solution (go grab a cup of tea, it may take a while).
* If it fails, run it again and again until there are (hopefully) no errors.

Running QGIS from within MSVC:
* Edit the properties of the project ALL_BUILD to include the path to the executable:
* Debugging -> Command -> `C:\OSGeo4W64\QGIS\ms-windows\osgeo4w\build-qgis-test-x86_64\output\bin\RelWithDebInfo\qgis.exe`.
* To run, use the menu commands: Debug -> Start Debugging (F5) or Start Without Debugging (Ctrl+F5).
* Ignore the "These projects are out of date" message, it appears even if no files were changed.

### 4.1.5 Old alternative method that might still work using cmake-gui
Create a 'build' directory somewhere. This will be where all the build output
will be generated.

Now run `cmake-gui` (still from `cmd`) and in the *Where is the source code:*
box, browse to the top level QGIS directory.

In the *Where to build the binaries:* box, browse to the `build` directory you
created.

If the path to bison and flex contains blanks, you need to use the short name
for the directory (i.e. `C:\Program Files` should be rewritten to
`C:\Progra~n`, where `n` is the number as shown in `dir /x C:\`).

Verify that the `BINDINGS_GLOBAL_INSTALL` option is not checked, so that python
bindings are placed into the output directory when you run the `INSTALL` target.

Hit `Configure` to start the configuration and select `Visual Studio 9 2008`
and keep `native compilers` and click `Finish`.

The configuration should complete without any further questions and allow you to
click `Generate`.

Now close `cmake-gui` and continue on the command prompt by starting
`vcexpress`.  Use File / Open / Project/Solutions and open the
qgis-x.y.z.sln File in your project directory.

Change `Solution Configuration` from `Debug` to `RelWithDebInfo` (Release
with Debug Info)  or `Release` before you build QGIS using the `ALL_BUILD`
target (otherwise you need debug libraries that are not included).

After the build completed you should install QGIS using the `INSTALL` target.

Install QGIS by building the `INSTALL` project. By default this will install to
`C:\Program Files\qgis<version>` (this can be changed by changing the
`CMAKE_INSTALL_PREFIX` variable in `cmake-gui`).

You will also either need to add all the dependency DLLs to the QGIS install
directory or add their respective directories to your `PATH`.

### 4.1.6. Packaging

To create a standalone installer there is a perl script named `creatensis.pl`
in `qgis/ms-windows/osgeo4w`.  It downloads all required packages from OSGeo4W
and repackages them into an installer using NSIS.

The script can be run on both Windows and Linux.

On Debian/Ubuntu you can just install the `nsis` package.

NSIS for Windows can be downloaded at:

https://nsis.sourceforge.io/Main_Page

And Perl for Windows (including other requirements like `wget`, `unzip`, `tar`
and `bzip2`) is available at:

https://cygwin.com

### 4.1.7. Packaging your own build of QGIS

Assuming you have completed the above packaging step, if you want to include
your own hand built QGIS executables, you need to copy them in from your
windows installation into the ms-windows file tree created by the creatensis
script.

```cmd
cd ms-windows/
rm -rf osgeo4w/unpacked/apps/qgis/*
cp -r /tmp/qgis1.7.0/* osgeo4w/unpacked/apps/qgis/
```

Now create a package.

```cmd
./quickpackage.sh
```

After this you should now have a nsis installer containing your own build
of QGIS and all dependencies needed to run it on a windows machine.

### 4.1.8. Osgeo4w packaging

The actual packaging process is currently not documented, for now please take a
look at `ms-windows/osgeo4w/package.cmd`.


## 4.2. Building on Linux with mingw64

With this approach you can cross build a Windows binary on Linux using mingw64
in a Docker container.

To build on Linux from your QGIS sources directory, launch:

```cmd
ms-windows/mingw/build.sh
```

After a successful build, you will find two packages in the QGIS sources
directory:

- qgis-portable-win64.zip (QGIS for Windows 64bit)
- qgis-portable-win64-debugsym.zip (debug symbols)

This method is also used in the continuous integrations process. After each pull
request the two packages mentioned above are stored as GitHub actions artifacts
and are available for download making it possible to quickly test changes on Windows.

### 4.2.1. Building with Docker

This is the simplest way, but you need to have Docker installed
on your system.

You can use a Docker image to cross build QGIS by calling
the script ms-windows/mxe/build.sh from the root directory of QGIS repository.

=== Building without Docker ====

This requires to install mxe toolchain on your system and build
all dependencies by yourself.

#### 4.2.1.1. Initial setup

Please follow the instructions on mxe website to setup your building toolchain http://mxe.cc/,
take note of the path where you have installed mxe.

#### 4.2.1.2. Building the dependencies

Please see README.md under ms-windows/mxe for detailed instructions and for the
list of dependencies that need to be built in mxe before attempting to build QGIS.

#### 4.2.1.3. Cross-Building QGIS

Edit the build-mxe.sh script and optionally adjust the path where your mxe installation is located, you
can also change the build and release directories.

### 4.2.2. Testing QGIS

Copy and unzip on the Windows machine package produced by the build and launch the qgis binary: no installation
is required.

# 5. Building on MacOS X

If you want to test QGIS, easiest option is to download and install all-in-one self-containing bundle directly from

https://qgis.org/downloads/macos

On the other hand, if you want to build or develop QGIS on your own, you need a set of dependencies and tools.
These instructions will use the same set of dependencies that are used for all-in-one QGIS bundle,
but you can build QGIS with Homebrew, MacPorts or Conda dependencies too.

https://github.com/qgis/QGIS-Mac-Packager

Included are notes for building on latest Mac OS X with latest updates installed.
The build uses clang compiler.

Parallel Compilation: On multiprocessor/multicore Macs, it's possible to
speed up compilation, but it's not automatic.  Whenever you type "make" (but
NOT "make install"), instead type:

```bash
make -j [#cpus]
```

Replace [#cpus] with the number of cores and/or processors your Mac has.
To find out how many CPUs you have available, run the following in Terminal:

```bash
/usr/sbin/sysctl -n hw.ncpu
```

## 5.1. Install Developer Tools

Developer tools are not a part of a standard OS X installation.
As minimum you require command line tools

```bash
sudo xcode-select --install
```

but installation of Xcode from the App Store is recommended too.

## 5.2. Install CMake and other build tools

For example install Homebrew

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

and these development/build tools

```bash
brew install git cmake ninja pkg-config wget bash-completion curl gnu-sed coreutils ccache libtool astyle help2man autoconf automake pandoc
```

if you have these tools installed from MacPorts or Conda, it is the same, but we will need to be able to
run `cmake` and others from Terminal in the following steps

## 5.3. Install Qt5 and QGIS-Deps

To build QGIS, we need Qt5 and FOSS dependencies on hand. The Qt5 version ideally should match the version that was
used to build dependency package.

Download the latest QGIS-Deps install script, qt package and QGIS-Deps packages from

https://qgis.org/downloads/macos/deps

You should have one bash script and two tar archive in your download folder.
Run the install script to install Qt and QGIS-Deps to `/opt/` area. You need
root privileges or have write access to `/opt/Qt` and `/opt/QGIS`.

Alternatively you can download and install Qt Open Source for MacOS from the

https://www.qt.io/

in the same version as referenced in the install script. It must be installed in `/opt/Qt`

Note that the QGIS-Deps package is not yet signed, so you may need to add Terminal
to System Preferences -> Security & Privacy -> Privacy -> Developer Tools or manually accept usage
of the libraries when asked by system.

## 5.4. QGIS source

Unzip the QGIS source to a working folder of your choice.
If you are reading this from the source, you've already done this.

If you want to experiment with the latest development sources, go to the github
QGIS project page:

http://github.com/qgis/QGIS

It should default to the master branch.  Click the Downloads button and
select Download .tar.gz. Double-click the tarball to unzip it.

*Alternatively*, use git and clone the repository by
```bash
git clone git://github.com/qgis/QGIS.git
```

## 5.5. Configure the build

CMake supports out of source build so we will create a 'build' dir for the
build process. OS X uses ${HOME}/Applications as a standard user app folder (it
gives it the system app folder icon).  If you have the correct permissions you
may want to build straight into your /Applications folder. The instructions
below assume you are building into a ${HOME}/Applications directory.

In a Terminal cd to the qgis source folder previously downloaded, then:

```bash
cd ..
mkdir build
cd build

QGIS_DEPS_VERSION=0.9;\
QT_VERSION=5.15.2;\
PATH=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/bin:$PATH;\
cmake \
  -DCMAKE_INSTALL_PREFIX=~/Applications \
  -DCMAKE_BUILD_TYPE=Release \
  -DQGIS_MAC_DEPS_DIR=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage \
  -DCMAKE_PREFIX_PATH=/opt/Qt/${QT_VERSION}/clang_64 \
  ../QGIS
```

Note: Don't forget the `../QGIS` on the last line, which tells CMake to look for the source files.

Note: Double check on the screen output that all libraries are picked from QGIS-Deps `/opt/QGIS`
and not from system `/usr/lib` or Homebrew's `/usr/local/` or system Frameworks `/Library/Frameworks/`.
Especially check Proj, GDAL, sqlite3 and Python paths.

After the initial Terminal configure, you can use ccmake to make further changes:

```bash
cd build
ccmake ../QGIS
```

## 5.6. Building

Now we can start the build process (remember the parallel compilation note at
the beginning, this is a good place to use it, if you can):

```bash
make -j [#cpus]
```

Now you can run the QGIS from build directory by `./output/bin/QGIS.app/Contents/MacOS/QGIS`

If all built without errors you can then install it:

```bash
make install
```

or, for an /Applications build:

```bash
sudo make install
```

For running the installed QGIS, you need to keep the dependencies in `/opt/` folder in place.
If you want to create bundle that runs without these dependencies, please read the documentation in project

https://github.com/qgis/QGIS-Mac-Packager

# 6. Setting up the WCS test server on GNU/Linux

**Requires:** Ubuntu / Debian derived distro

These notes are for Ubuntu - other versions and Debian derived distros may
require slight variations in package names.

## 6.1. Preparation

Note the git repo below will change to the default QGIS repo once this work
is integrated into master.

```bash
git remote add blazek git://github.com/blazek/Quantum-GIS.git
git fetch blazek
git branch --track wcs2 blazek/wcs2
git checkout wcs2
cd /var/www/
sudo mkdir wcs
sudo chown timlinux wcs
cd wcs/
mkdir cgi-bin
cd cgi-bin/
```

## 6.2. Setup mapserver

```bash
sudo apt-get install cgi-mapserver
```

Set the contents of `/var/www/wcs/cgi-bin/wcstest-1.9.0` to:

```bash
#! /bin/sh
MS_MAPFILE=/var/www/wcs/testdata/qgis-1.9.0/raster/wcs.map
export MS_MAPFILE
/usr/lib/cgi-bin/mapserv
```

Then do:

```bash
chmod +x var/www/wcs/cgi-bin/wcstest-1.9.0
mkdir -p /var/www/wcs/testdata/qgis-1.9.0/raster/
cd /var/www/wcs/testdata/qgis-1.9.0/raster/
cp -r /home/timlinux/QGIS/tests/testdata/raster/* .
```

Edit `/var/www/wcs/testdata/qgis-1.9.0/raster/wcs.map` and set the shapepath to this:

```bash
SHAPEPATH "/var/www/wcs/testdata/qgis-1.9.0/raster"
```

Then create `/var/www/wcs/7-wcs.example.com.conf` setting the contents to this:

```bash
<VirtualHost*:80>
ServerName wcs.example.com
ServerAdmin wcs-admin@example.com

LogLevel warn
LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" \"%{forensic-id}n\"" combined
CustomLog /var/log/apache2/wcs_example.com/access.log combined
ErrorLog /var/log/apache2/wcs_example.com/error.log

DocumentRoot /var/www/wcs/html

ScriptAlias /cgi-bin/ /var/www/wcs/cgi-bin/
<Directory "/var/www/wcs/cgi-bin">
	AllowOverride None
	Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
	Order allow,deny
	Allow from all
</Directory>

RewriteEngine on
RewriteRule /1.9.0/wcs /cgi-bin/wcstest-1.9.0 [PT]

</VirtualHost>
```


## 6.3. Create a home page

```bash
mkdir html
vim html/index.html
```

Set the contents to:
```
This is the test platform for QGIS' wcs client. You can use these services
from QGIS directly (to try out WCS for example) by pointing your QGIS to:
http://wcs.example.com/1.9.0/wcs
```

## 6.4. Now deploy it

```bash
sudo mkdir /var/log/apache2/wcs_example.com
sudo chown www-data /var/log/apache2/wcs_example.com
cd /etc/apache2/sites-available/
sudo ln -s /var/www/wcs/7-wcs.example.com.conf .
cd /var/www/wcs/
sudo a2ensite 7-wcs.example.com.conf
sudo /etc/init.d/apache2 reload
```

## 6.5. Debugging

```bash
sudo tail -f /var/log/apache2/wcs_example.com/error.log
```

# 7. Setting up a Jenkins Build Server

**Assumption:** You know how to make a working build environment and want to
deploy it under Jenkins for continuous integration testing now.

These notes are terse, I will expand on them later as the need arises. The
procedure is:

* Install Jenkins and get it configured according to your own preferences
* Make sure you have the git, github, junit etc plugins installed. A complete
list of the plugins I have installed follows (note that you almost certainly
don't need evey plugin listed here):
  * External Monitor Job Type Plugin
  * LDAP Plugin
  * pam-auth
  * javadoc
  * ant
  * Jenkins Subversion Plug-in
  * Git Plugin
  * Maven 2 Project Plugin
  * Jenkins SLOCCount Plug-in
  * Jenkins Sounds plugin
  * Jenkins Translation Assistance plugin
  * ruby-runtime
  * Jenkins CVS Plug-in
  * Coverage/Complexity Scatter Plot PlugIn
  * Status Monitor Plugin
  * Git Parameter Plug-In
  * github-api
  * GitHub plugin
  * Jenkins Violations plugin
  * git-notes Plugin
  * Twitter plugin
  * Jenkins Cobertura Plugin
  * Jenkins Gravatar plugin
  * Jenkins SSH Slaves plugin
* Create a Job called 'QGIS'
* Use the following options for your job:
  * Job Name: QGIS
  * Job Type: Build a free-style software project
  * Tick enable project based security (you need to elsewhere configure your
Jenkins security to per project settings)
  * Allow Anonymous user Read and Discover access
  * Set the github project to https://github.com/qgis/QGIS/
  * Set source code management to Git
  * Set repository url to git://github.com/qgis/QGIS.git
  * In advanced repository url settings set refspec to `+refs/heads/master:refs/remotes/origin/master`
  * Set branch to build to master
  * Repository Browser: Auto
  * Build triggers: set to Poll SCM and set schedule to `*****` (polls every minute)
  * Build - Execute shell and set shell script to:

```bash
cd build
cmake ..
xvfb-run --auto-servernum --server-num=1 \
  --server-args="-screen 0 1024x768x24" \
  make Experimental || true
if [ -f Testing/TAG ] ; then
  xsltproc ../tests/ctest2junix.xsl \
    Testing/`head -n 1 < Testing/TAG`/Test.xml > \
    CTestResults.xml
fi
```

  * Add Junit post build action and set 'Publish Junit test result report' to:
`build/CTestResults.xml`
  * Email notification: Send separate e-mails to individuals who broke the build
  * Jenkins sounds - set up sounds for Failure, Success and Unstable.
  * Save

Now open the Job dash board and push something to QGIS and wait a minute to
validate automated builds work.

**Note:** You will need to log in to the Jenkins user account and go to
/var/lib/jenkins/jobs/QGIS/workspace, then make a `build` directory and run
the initial cmake setup and then do test build. This process is the same as
described elsewhere in this doc.

I based some of the set up from this nice blog article here:

http://alexott.blogspot.com/2012/03/jenkins-cmakectest.html

# 8. Debug output and running tests

If you are interested in seeing embedded debug output, change the following
CMake option:

```bash
-D CMAKE_BUILD_TYPE=DEBUG  # (or RELWITHDEBINFO)
```

This will flood your terminal or system log with lots of useful output from
QgsDebugMsg() calls in source code.

If you would like to run the test suite, you will need to do so from the build
directory, as it will not work with the installed/bundled app. First set the
CMake option to enable tests:

```bash
-D ENABLE_TESTS=TRUE
```

Then run all tests from build directory:

```bash
cd build
make test
```

To run all tests and report to http://cdash.orfeo-toolbox.org/index.php?project=QGIS

```bash
cd build
make Experimental
```

You can define the host name reported via 'make Experimental' by setting a CMake
option:

```bash
-D SITE="my.domain.org"
```

To run specific test(s) (see 'man ctest'):

```bash
cd build
# show listing of tests, without running them
ctest --show-only

# run specific C++ or Python test(s) matching a regular expression
ctest --verbose --tests-regex SomeTestName
```

# 9. Authors and Acknowledgments

The following people have contributed to this document:

* Windows MINGW Section
  * Tim Sutton, Godofredo Contreras 2006
  * CMake additions Magnus Homann 2007
  * Python additions Martin Dobias 2007
  * With thanks to Tisham Dhar for preparing the initial msys environment

* Windows MSVC Section (Detailed install)
  * David Willis 2007
  * MSVC install additions Tim Sutton 2007
  * PostgreSQL, Qt compile, SIP, Python, AutoExp additions Juergen Fischer 2007

* Windows MSVC Section (Simplified install)
  * Tim Sutton 2007
  * Juergen Fischer 2007
  * Florian Hillen 2010

* OSX Section
  * Tim Sutton 2007
  * With special thanks to Tom Elwertowski and William Kyngesburye
  * Larry Shaffer 2012
  * Peter Petrik 2020

* GNU/Linux Section
  * Tim Sutton 2006
  * Debian package section: Juergen Fischer 2008

* WCS Test Server Section
  * Tim Sutton, Radim Blazek 2012

* Jenkins CI Configuration
  * Tim Sutton 2012

* Latex Generator
  * Tim Sutton 2011

* Debug Output/Tests Section
  * Larry Shaffer 2012, by way of 'Test Friday' Tim Sutton

* MXE/Mingw64 section
  * Alessandro Pasotti (2018-2021)
