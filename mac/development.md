
# (THIS DOCUMENT IS NOT UP TO DATE)

# Developing QGIS using Homebrew dependencies 

In addition to using this tap to install [QGIS development formulae](../Formula), you can also use it to fully set up a development environment for an externally built QGIS from a clone of the current [development (master) branch](https://github.com/qgis/QGIS) of the source code tree.

> Note: This setup, though heavily tested, is currently _experimental_ and may change.

For the terminally lazy, who are already comfortable with Homebrew, [jump to sample Terminal session](#terminal).

## Development Tools

This tutorial is based upon the following software:
* [Qt Creator](http://qt-project.org/downloads) for CMake/C++ development of ([core source](https://github.com/qgis/QGIS/tree/master/src) and [plugins](https://github.com/qgis/QGIS/tree/master/src/plugins))
* [PyCharm Community Edition](http://www.jetbrains.com/pycharm/download/) for Python development of ([PyQGIS plugins/apps](http://docs.qgis.org/testing/en/docs/pyqgis_developer_cookbook/), [Python unit tests](https://github.com/qgis/QGIS/tree/master/tests/src/python), [reStructuredText for documentation](https://github.com/qgis/QGIS-Documentation)).
* macOS XCode (download via Mac App Store and _launch at least once_) and Xcode Command Line Tools (run `xcode-select --install` after installing Xcode), for Homebrew and building QGIS source. QGIS's [CMake](http://www.cmake.org) build process uses generated build files for building QGIS source directly with the `clang` compiler, _not via Xcode project files_.

## Homebrew

See http://brew.sh and [Homebrew docs](https://github.com/Homebrew/brew/tree/master/docs) for info on installing Homebrew.

After Homebrew is installed run the following and fix everything that it mentions, if you can:
```sh
brew doctor
```

### Homebrew configuration

Homebrew now defaults to _auto-updating_ itself (runs `brew update`) upon *every* `brew install` invocation. While this can be handy for keeping up with the latest changes, it can also quickly break an existing build's linked-to libraries. Consider setting the `HOMEBREW_NO_AUTO_UPDATE` environment variable to turn this off, thereby forcing manual running of `brew update`:

```sh
export HOMEBREW_NO_AUTO_UPDATE=1
```

See end of `man brew` for other environment variables.

### Homebrew prefix

While all of the formulae and scripts support building QGIS using Homebrew installed to a non-standard prefix, e.g. `/opt/homebrew`, **do yourself a favor** (especially if you are new to Homebrew) and [install in the default directory of `/usr/local`](https://github.com/Homebrew/brew/blob/master/docs/Installation.md). QGIS has many dependencies which are available as ["bottles"](https://github.com/Homebrew/brew/blob/master/docs/Bottles.md) (pre-built binary installs) from the Homebrew project. Installing Homebrew to a non-standard prefix will force many of the bottled formulae to be built from source, since many of the available bottles are built specific to `/usr/local`. Such unnecessary building from source can comparatively take hours and hours more, depending upon your available CPU cores.

If desired, this setup supports QGIS builds where the dependencies are in a non-standard Homebrew location, e.g. `/opt/homebrew`, instead of `/usr/local`. This allows for multiple build scenarios, though often requires a more meticulous CMake configuration of QGIS.

You can programmatically find the prefix with `brew --prefix`. In formulae code, it is denoted with the HOMEBREW_PREFIX environment variable.

### Homebrew formula install prefixes

By default, Homebrew installs to a versioned prefix in the 'Cellar', e.g. `/usr/local/Cellar/gdal2/2.1.2/`. Using the versioned path for dependencies _might_ lead to it being hardcoded into the linking phase of your builds. This is an issue because the version can change often with formula changes that are unrelated to the actual dependency's version, e.g. `2.1.2_1`. When defining paths for CMake, always try the formula's 'opt prefix' first, e.g. `/usr/local/opt/gdal2`, which is a symbolic link that _always_ points to the latest versioned install prefix of the formula. This often avoids the issue, though some CMake find modules _may_ resolve the symbolic link back to the versioned install prefix.

### Install some basic formulae

Always read the 'Caveats' section output at the end of `brew info <formula>` or `brew install <formula>`. 

```sh
brew install bash-completion
brew install git
```

## Python Dependencies

### Select an interpreter

The first important decision to make is regarding whether to use Homebrew's or another Python 3.x. Currently macOS does not ship with Python 3, and QGIS 3 should be built against Python 3.x.

> Note: The more Homebrew formulae you install from bottles, the higher likelihood you will end up running into a formulae that requires installing Homebrew's Python 3, since bottles are always built against that Python.

If using Homebrew Python 3.x, install with:

```sh
# review options
brew info python3
brew install python3 [--with-option ...]
```

Regardless of which Python interpreter you use, always ensure it is the first found `python3` on PATH in your Terminal session where you run `brew` commands, i.e. `which python3` points to the correct Python 3 you wish to use when building formulae.

### Install required Python packages

Use [pip3](https://pypi.python.org/pypi/pip/) that was installed against the same Python 3 you are using when installing formulae. You can also tap [homebrew/python](https://github.com/Homebrew/homebrew-python) for some more complex package installs.

Reference `pip3 --help` for info on usage (usually just `pip install <package>`).

* [future](https://pypi.python.org/pypi/future)
* [numpy](https://pypi.python.org/pypi/numpy)
* [psycopg2](https://pypi.python.org/pypi/psycopg2)
* [matplotlib](https://pypi.python.org/pypi/matplotlib)
* [pyparsing](https://pypi.python.org/pypi/pyparsing)
* [requests](https://pypi.python.org/pypi/requests)
* [mock](https://pypi.python.org/pypi/mock)
* [pyyaml](https://pypi.python.org/pypi/PyYAML)
* [nose2](https://pypi.python.org/pypi/nose2)

Other Python packages **automatically installed** by Homebrew from QGIS dependencies:

* [sip](https://github.com/Homebrew/homebrew-core/blob/master/Formula/sip.rb)
* [PyQt5](https://github.com/Homebrew/homebrew-core/blob/master/Formula/pyqt5.rb)
* [QScintilla2](https://github.com/Homebrew/homebrew-core/blob/master/Formula/qscintilla2.rb)
* [pyspatialite](https://github.com/OSGeo/homebrew-osgeo4mac/blob/master/Formula/pyspatialite.rb) (deprecated)
* [`osgeo.gdal` and `osgeo.ogr`, etc.](https://github.com/OSGeo/homebrew-osgeo4mac/blob/master/Formula/gdal2-python.rb)

## Install Build and Linked Library Dependencies

### Install dependencies from QGIS formulae

> Note: substitute **`qgis3-xx`** for whatever QGIS formula you are using for dependencies, e.g. `qgis3-dev`.

```sh
# add this tap
brew tap qgis/qgisdev
# review options
brew info qgis3-xx
# see what dependencies will be included
brew deps --tree qgis3-xx [--with[out]-some-option ...]
# install dependencies, but not QGIS
brew install qgis3-xx --only-dependencies [--with[out]-some-option ...]
```

You do not have to actually do `brew install qgis3-xx` unless you also want that version installed. If you do have QGIS 3 formulae installed, and are planning on _installing_ your development build (not just running from the build directory), you should unlink the formulae installs, e.g.:

```sh
brew unlink qgis3-xx
```

This will ensure the `qgis.core`, etc. Python modules of the formula(e) installs are not overwritten by the development build upon `make install`. All `qgis-xx` formulae QGIS applications will run just fine from their Cellar keg install directory. _Careful_, though, as multiple QGIS installs will probably all share the same application preference files; so, don't run them concurrently.

### Optional External Dependencies

The [Processing framework](http://docs.qgis.org/testing/en/docs/user_manual/processing/index.html) of QGIS can leverage many external geospatial applications and utilities, which _do not_ need to be built as dependencies prior to building QGIS:

* [`grass7`](https://github.com/OSGeo/homebrew-osgeo4mac/blob/master/Formula/grass7.rb) (`--with-grass` option) - [GRASS 7](http://grass.osgeo.org), which is also used by the GRASS core plugin in QGIS
* [`orfeo5`](https://github.com/OSGeo/homebrew-osgeo4mac/blob/master/Formula/orfeo5.rb) (`--with-orfeo5` option) - [Orfeo Toolbox](http://orfeo-toolbox.org/otb/)
* [`r`](http://www.r-project.org/) (`--with-r` option) - [R Project](http://www.r-project.org/)
* [`saga-gis`](https://github.com/OSGeo/homebrew-osgeo4mac/blob/master/Formula/saga-gis.rb) (`--with-saga-gis` option) - [System for Automated Geoscientific Analyses](http://www.saga-gis.org)
* [`taudem`](https://github.com/OSGeo/homebrew-osgeo4mac/blob/master/Formula/taudem.rb) - [Terrain Analysis Using Digital Elevation Models](http://hydrology.usu.edu/taudem/taudem5/index.html).

The `gpsbabel` formula can be installed as a dependency, though you may have to define the path to its binary when using QGIS's [GPS Tools](http://docs.qgis.org/testing/en/docs/user_manual/working_with_gps/plugins_gps.html).

> Note: if you install Processing external utilities _after_ installing a QGIS formula or building your own QGIS, you may need to configure the individual utility's paths in Processing's options dialog. 

## Clone QGIS Source

See the QGIS [INSTALL](https://github.com/qgis/QGIS/blob/master/INSTALL) document for information on using git to clone the source tree.

QGIS's build setup uses CMake, which supports 'out-of-source' build directories. It is recommended to create a separate build directory, either within the source tree, or outside of it. Since the (re)build process can generate _many_ files, consider creating a separate partition on which to place the build directory. Such a setup can significantly reduce fragmentation on your main startup drive. Use of Solid State Disks is recommended.

## Customize Build Scripts

This tap offers several convenience scripts for use in Qt Creator, or wrapper build scripts, to aid in building/installing QGIS, located at:

```sh
$(brew --repository qgis/qgisdev)/scripts
```

> Note: **Copy the directory elsewhere** and use it from there. It's important to not edit the scripts where they are located, in the tap, because it is a git repository. You should keep that working tree clean so that `brew update` always works.

The scripts will be used when configuring/building/installing the QGIS project in Qt Creator, or can be used independent of Qt Creator.

### Open and review scripts

**Note:** scripts expect the HOMEBREW_PREFIX environment variable to be set, e.g. in your `.bash_profile`:
 
```sh
# after prepending `brew --prefix` to PATH (not needed for default /usr/local Homebrew)
export HOMEBREW_PREFIX=$(brew --prefix)
```

* [qgis-cmake-setup.sh](../scripts/qgis-cmake-setup.sh) - For generating CMake option string for use in Qt Creator (or build scripts) when built off dependencies from this and other taps. Edit CMake options to suit your build needs. Note, the current script usually has CMake options for building QGIS with *most* core options that the current `qgis3-xx` Homebrew formula supports, which may not include things like Oracle support, etc. You will probably want to edit it and (un)comment out such lines for an initial build. 

* [qgis-set-app-env.py](../scripts/qgis-set-app-env.py) - For setting env vars in dev build and installed QGIS.app, to ensure they are available on double-click run. _Needs to stay in the same directory as the next scripts._ Generally, you will not need to edit this script.

* [qgis-dev-build.sh](../scripts/qgis-dev-build.sh) - Sets up the build environ and ensures the QGIS.app in the build directory can find resources, so it can run from there.

* [qgis-dev-install.sh](../scripts/qgis-dev-install.sh) - Installs the app and ensures QGIS.app has proper environment variables, so it can be moved around on the filesystem. Currently, QGIS.app bundling beyond [QGIS_MACAPP_BUNDLE=0](https://github.com/qgis/QGIS/tree/master/mac) is not supported. Since all dependencies are in your `HOMEBREW_PREFIX`, _no complex bundling is necessary_, unless you intend to relocate the built app to another Mac (which is a planned feature).

## <a name="terminal"></a>Configure/build/install QGIS in a Terminal.app session

**Example** Terminal.app session for cloning and building QGIS from scratch, based off of `qgis-3-dev` formula dependencies and assuming Xcode.app, Xcode Command Line Tools, and Homebrew are _already installed_. BASH used here.

```sh
# Setup environment variables
export HOMEBREW_PREFIX=$(brew --prefix)
export HOMEBREW_NO_AUTO_UPDATE=1

# Optionally update Homebrew (recommended)
brew update

# Install some handy base formulae
brew install bash-completion
brew install git
brew install cmake

# Decide to use recommended Homebrew's Python3
# (could use Anaconda's Python 3, etc. instead, though bottles may not work)
brew install python3

# Install some Python dependencies
# NOTE: may require `sudo` if Python 3 is installed in a root-owned location 
pip3 install future numpy psycopg2 matplotlib pyparsing requests pyyaml mock nose2

# Add some useful Homebrew taps
# NOTE: try to avoid tapping homebrew/boneyard
brew tap homebrew/science
brew tap homebrew/python
brew tap qgis/qgisdev
brew tap osgeo/osgeo4mac

# Make sure deprecated Qt4 formulae are not linked
brew unlink qt
brew unlink pyqt

# Older txt2tags generator for INSTALL doc breaks build and chokes on Python 3
brew unlink txt2tags

# Install and verify GDAL/OGR with decent driver support
# Do NOT install `gdal` (1.11.x) formula, unless you truly need it otherwise
# NOTE: keg-only, e.g. only available from HOMEBREW_PREFIX/opt/gdal2 prefix
brew install osgeo/osgeo4mac/gdal2 --with-complete --with-libkml
brew test osgeo/osgeo4mac/gdal2

# If failure, review any .dylib errors when loading drivers (scroll to top of output)
$HOMEBREW_PREFIX/opt/gdal2/bin/gdalinfo --formats
$HOMEBREW_PREFIX/opt/gdal2/bin/ogrinfo --formats

# Add the Python 3 bindings for GDAL/OGR
brew install osgeo/osgeo4mac/gdal2-python --with-python3
brew test osgeo/osgeo4mac/gdal2-python

# Optionally add and verify Processing framework extra utilities
brew install osgeo/osgeo4mac/grass7
brew install osgeo/osgeo4mac/gdal2-grass7
brew test osgeo/osgeo4mac/grass7
brew test osgeo/osgeo4mac/gdal2-grass7

brew install osgeo/osgeo4mac/saga-gis --with-app
brew test osgeo/osgeo4mac/saga-gis

# This one's huge, bringing in large dependencies
brew install orfeo5
brew test orfeo5

# Install remaining dependencies for qgis3-dev formula, but not QGIS
# This may take a loooong time if there are missing bottles, which need built
brew install qgis3-dev --only-dependencies [--with-other-options]

# Base directory path of src, install and build directories
BASE_DIR=$HOME/src
mkdir -p $BASE_DIR
cd $BASE_DIR

# Create and save a directory to install a final QGIS.app
QGIS_INSTALL=$BASE_DIR/QGIS_install
mkdir -p $QGIS_INSTALL

# Clone QGIS source tree
QGIS_SRC=$BASE_DIR/QGIS
# This may take a looong time, depending upon connection speed
git clone https://github.com/qgis/QGIS.git $QGIS_SRC
cd $QGIS_SRC

# Setup out-of-source build directory, inside of QGIS tree
BUILD_DIR=$BASE_DIR/QGIS/build
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Where you have copied the build scripts (here using the defaults, as is)
BUILD_SCRIPTS=$(brew --repository qgis/qgisdev)/scripts

# Configure CMake and generate build files
# usage: qgis-cmake-setup.sh 'source directory' 'build directory' 'install directory'
#        (directories need to absolute paths)
$BUILD_SCRIPTS/qgis-cmake-setup.sh $QGIS_SRC $BUILD_DIR $QGIS_INSTALL

# Review or edit what was configured (almost all dependencies should be from Homebrew)
ccmake $BUILD_DIR

# Build QGIS
# usage: qgis-dev-build.sh 'absolute path to build directory'
$BUILD_SCRIPTS/qgis-dev-build.sh $BUILD_DIR

# Run QGIS test suite from build directory
# source environment and run tests inside of a bash subshell, 
#   so your shell environment is not polluted
( source $BUILD_SCRIPTS/qgis-dev.env $BUILD_DIR && ctest )

# Install QGIS.app
# note: app bundle is moveable about the filesystem, but not to another Mac
$BUILD_SCRIPTS/qgis-dev-install.sh $BUILD_DIR
```

## Configure/build/install QGIS in Qt Creator.app

These steps assume you are starting from a QGIS source tree clone, having done none of the above in Terminal.app. However, using an existing Terminal-based build is possible, if you assign the existing build directory upon loading your QGIS project.

**Important:** This configuration uses ``/usr/local`` as the HOMEBREW_PREFIX, which is the default. Substitute if yours is different. 

### Install QtCreator 4.2 or better

It is recommended to use Qt Creator >= 4.2.0, as the CMake support much better than previous versions. Newer versions are slated to offer even more CMake integration, so update often.

* Grab _just_ the Qt Creator installer from https://www.qt.io/download-open-source/
  * Click **View All Downloads** scroll down to Qt Creator section
* Open the DMG and drag **Qt Creator.app** wherever you like 
* Open **Qt Creator.app**

### Make a "Build & Run" kit for Homebrew

Go to ``Preferences -> Build & Run``. All further settings for building a kit are contained in tabs of that Preferences section.

**Note:** You can click ``Apply`` button, at any time, to apply your settings without closing the dialog. 

#### Define a CMake executable

Under ``CMake`` tab, click ``Add``

* **Name:** Brew CMake
* **Path:** /usr/local/bin/cmake

#### Optionally define a compiler cache

Generally, the default macOS `clang` compiler is very quick, but subsequent compilations can be accelerated with a compiler cache. QGIS developers often use and have heavily tested ``ccache`` for this purpose:

```sh
brew install ccache
```

Compiler symlinks are **not available** in ``/usr/local/bin``, but can be found in:

```sh
/usr/local/opt/ccache/libexec
```

Under ``Compilers`` tab...

* Click ``Add -> Clang -> C`` and configure:

  * **Name:** Clang (x86 64bit) - Brew ccache
  * **Compiler path:** _Paste_ in ``/usr/local/opt/ccache/libexec/clang``
  * **ABI:** x86, darwin, generic, mach_o, 64bit

* Click ``Add -> Clang -> C++`` and configure:

  * **Name:** Clang (x86 64bit) - Brew ccache
  * **Compiler path:** _Paste_ in ``/usr/local/opt/ccache/libexec/clang++``
  * **ABI:** x86, darwin, generic, mach_o, 64bit

**Note:** If you browse to one of the symlinked compiler paths, you will end up with an unwanted "Cellar"-based path.

#### Define a Qt Version
Under ``Qt Versions``, click ``Add``.

* **Version name:** Brew Qt5
* **qmake location:** Browse to the executable ``/usr/local/opt/qt5/bin/qmake``

**Note:** Homebrew's Qt5 `qmake` is not currently linked into `HOMEBREW_PREFIX/bin`, though this may change. To check, see if `/usr/local/bin/qmake` exists and is from Homebrew's Qt5. If so, use that path instead; otherwise, continue with next step.

The **qmake** location for `/usr/local/opt/qt5/bin/qmake` will be saved as a versioned "Cellar" path. We need to adjust this to its "opt prefix" alternate path, the one we originally browsed. This keeps incremental Homebrew upgrades of Qt5 packages from breaking our kit in the future.

##### Fix qmake location

* Click ``OK`` for Preferences dialog and **quit** Qt Creator
* Open this file in a text editor:

  ``~/.config/QtProject/qtcreator/qtversion.xml``

* Find the same versioned "Cellar" ``qmake`` path (associated with the element whose attribute is ``key="QMakePath"``) and change it to:

  ``/usr/local/opt/qt5/bin/qmake``

* Save file and relaunch Qt Creator, then go to:

  ``Preferences -> Build & Run -> Qt Versions``

When you select the ``Brew Qt5`` version, it should now show the "opt prefix" path.

#### Define a kit

Under ``Kits`` tab, click ``Add``, then configure with:

* **Name:** QGIS Build Kit - Qt5
* **File system name:** _blank_
* **Device type:** Desktop
* **Device:** Local PC (default for Desktop)
* **Sysroot:** _blank_
* **Compiler:** (if using ccache, substitute the compilers you defined)
  * **C:** Clang (x86_64bin in /usr/bin)
  * **C++:** Clang (x86_64bin in /usr/bin)
* **Environment:** click ``Change...``  and add the following variables:

  ```
  HOMEBREW_PREFIX=/usr/local
  PATH=/usr/local/opt/ccache/libexec:/usr/local/bin:/usr/local/sbin:/usr/bin:/bin:/usr/sbin:/sbin
  ```

  **Notes:**
  * The kit's Qt5 `bin` dir is always prepended to PATH: `/usr/local/opt/qt5/bin`, but it will be resolved to it's "Cellar" path, e.g. `/usr/local/Cellar/qt5/5.7.1_1/bin`, which is not an issue since it is dynamically prepended.
  * `HOMEBREW_PREFIX` is used by the custom build and install scripts.

* **Debugger:** System LLDB at `/Applications/Xcode.app/Contents/Developer/usr/bin/lldb` (or `/usr/bin/lldb`, if using the Command Line Tools)
* **Qt Version:** Brew Qt5 (which you previously created)
* **Qt mkspec:** leave blank
* **CMake Tool:** Brew CMake (which you previously created)
* **CMake Generator:** CodeBlocks - Unix Makefiles
* **CMake Configuration:** Press the `Change...` button and paste this into the box provided:

  ```
  CMAKE_CXX_COMPILER:STRING=%{Compiler:Executable}
  CMAKE_C_COMPILER:STRING=%{Compiler:Executable:C}
  QT_QMAKE_EXECUTABLE:STRING=%{Qt:qmakeExecutable}
  CMAKE_BUILD_TYPE:STRING=RelWithDebInfo
  CMAKE_FIND_FRAMEWORK:STRING=LAST 
  CMAKE_PREFIX_PATH:STRING='/usr/local/opt/qt5;/usr/local/opt/qt5-webkit;/usr/local/opt/qscintilla2;/usr/local/opt/qwt;/usr/local/opt/qwtpolar;/usr/local/opt/qca;/usr/local/opt/gdal2;/usr/local/opt/gsl;/usr/local/opt/geos;/usr/local/opt/proj;/usr/local/opt/libspatialite;/usr/local/opt/spatialindex;/usr/local/opt/fcgi;/usr/local/opt/expat;/usr/local/opt/sqlite;/usr/local/opt/flex;/usr/local/opt/bison'
  ```
  
  **Notes:**
  * If using `ccache`, you must also define `CMAKE_C_COMPILER` (as above), or Apple's default C compiler will be used.
  * `CMAKE_PREFIX_PATH` is **critical** as it tells CMake to search _first_ in Homebrew install prefixes that are not linked into the HOMEBREW_PREFIX, e.g. `/usr/local`. Otherwise, CMake can not find them. Often, these packages are important to building QGIS or are newer versions overriding packages already installed on the base macOS.
  * The `CMAKE_PREFIX_PATH` value can be taken from `$(brew --repository qgis/qgisdev)/scripts/qgis-cmake-setup.sh`. You can comment out the last `eval` line of the aforementioned script to get the equivalent value above.
  * Do not place QGIS-specific CMake options here. Those should be defined in the CMake section of a specific QGIS source tree when loaded into Qt Creator as a project.

Optionally, once you have your kit defined, you can make it the default by selecting it and clicking the `Make Default` button.

### Load QGIS project

**Note:** This describes loading a CMake project in Qt Creator 4.2 (recommended minimum version). Other versions may vary.

When opening a project, the build directory needs defined. You can set a default build directory templated path in the `General` tab's `Default build directory:` option. If you wish to have the build directory default to _inside_ your project source tree directory, you can use the following template path:

```
%{CurrentProject:Path}/build_%{CurrentBuild:Type}
```

The generated build paths based upon this template can still be overridden in the next step. 

Load QGIS project and configure/generate its build files:

* Select `File -> Open File or Project...` menu action
* Open the `CMakeLists.txt` in the root of your cloned QGIS source tree directory
* In the resulting `Configure Project` panel:

  * Check only the `QGIS Build Kit - Qt5` you previously created, and uncheck **all** other kits
  * Click `Details` of your kit to reveal build directory options
    * Uncheck all options except "Release with Debug Information" (recommended - you can always add other types later as additional build configurations)
    * If you wish to use the templated directory path, and want to create it now, the easiest way is to copy the path and run `mkdir <path>` in Terminal.app.
    * If you wish to override the templated build directory path, click `Choose...` (create an empty build folder and select it)
  * Click `Configure Project`

**Note:** If you did not create the build directory, CMake will _configure_ the project, but not _create_ the build directory or _generate_ the build files until you choose to build the project.

### Configuring QGIS CMake options

**Tip:** You can choose the `Build -> Run CMake` menu action to reconfigure and generate build files for your project at any time. This is helpful for when Qt Creator does not update its GUI and give you the option to do so.

You can configure CMake options for your project in the `Projects` section of the main window, under `Active Project -> Your project` then under `Build & Run -> Your kit -> Build`.

* **Note:** When you `Add` or `Edit` options here, they are stored in `CMakeLists.txt.user` in the root of your QGIS source tree, under the this XML element:
 
  ```
  <valuelist type="QVariantList" key="CMake.Configuration">
  ```
  If you have problems with Qt Creator picking up automatic configuration changes, check the child elements of that `valuelist` to see if there are stored option elements that need deleted (if so, quit Creator first). This is necessary until Qt adds a `Delete` action for CMake options to Creator. Optionally, you can also delete `CMakeLists.txt.user`, though you lose considerably more project configurations.

#### Configure for running from build directory

This will stage the core Python plugins so they are available at runtime from the build directory:

```
WITH_STAGED_PLUGINS=TRUE   (Add as Boolean)
```

#### Configure QwtPolar

To avoid building the internal QwtPolar, and use Homebrew's:

```
WITH_QWTPOLAR=TRUE             (Add as Boolean)
WITH_INTERNAL_QWTPOLAR=FALSE   (Add as Boolean)
```

#### Configure for code styling

To ensure the `astyle` binary is built for checking your code with `prepare-commit.sh` script before committing to PR, etc.

```
WITH_ASTYLE=TRUE   (Add as Boolean)
```

#### Configure for install

If you intend to install QGIS.app, and not just run it from the build directory:

```
CMAKE_INSTALL_PREFIX=<directory-path>
```

This defaults to `/usr/local`, which is probably not what you want. Suggested:

```
CMAKE_INSTALL_PREFIX=$HOME/Applications/QGIS   (Add as Directory)
```

Unless you intend to install a fully bundled, standalone `QGIS.app` (not yet supported), then you will want to set:

```
QGIS_MACAPP_BUNDLE=0   (Add as String)
```

#### Configure GRASS

If using `osgeo/osgeo4mac/grass7`:

```
WITH_GRASS7=TRUE                                  (Add as Boolean)
GRASS_PREFIX7=/usr/local/opt/grass7/grass-base"   (Add as Directory)
```

#### Fix some CMake module search results

To ensure your build is isolated as much as possible from incidental Homebrew updates, prefer package "opt prefix" paths for CMake modules that find versioned prefix by default:

```
GDAL_LIBRARY=/usr/local/opt/gdal2/lib/libgdal.dylib        (Add as File)
GEOS_LIBRARY=/usr/local/opt/geos/lib/libgeos_c.dylib       (Add as File)
GSL_CONFIG=/usr/local/opt/gsl/bin/gsl-config               (Add as File)
GSL_INCLUDE_DIR=/usr/local/opt/gsl/include                 (Add as Directory)
GSL_LIBRARIES='-L/usr/local/opt/gsl/lib -lgsl -lgslcblas'  (Add as String)
```

# TODO: WIP from here to end

### Building QGIS

TODO: notes on `qgis-set-app-env.py` and `qgis-dev-build.sh`

### Cleaning the build

If you want to start with a clean setup in Qt Creator you should:

* Quit Qt Creator
* Remove the `CMakeLists.txt.user` file at the root of the QGIS source tree
* Optionally, remove the _contents_ of any existing build directory or just its `CMakeCache.txt` file
* Restart Qt Creator
* Load project as noted above

### Running/debugging QGIS.app from build directory

You may need to do this:

	•	add /usr/local/opt/gdal2/bin/ to the raster/gdal tool settings in qgis
	•	add gdal to your path : export PATH=/usr/local/opt/gdal2/bin/:$PATH

When debugging / running from Qt-Creator, ensure that GDAL python packages are in your path by adding this to your run environment:

```
PYTHONPATH set to $PYTHONPATH:/usr/local/opt/gdal2-python/lib/python3.5/site-packages
```

Alternatively, add the gdal2 packages to your environment directly in QGIS like this:

![screen shot 2017-03-19 at 8 37 54 pm](https://cloud.githubusercontent.com/assets/178003/24084521/6d5345ec-0cf4-11e7-9d86-e8af713a5b76.png)



### Installing QGIS.app

### Qt Creator tweaks

* Import QGIS code style rules
  * Under `Preferences -> C++`, click `Import`
  * Import `qtcreator_code_style.xml` from `QGIS/doc` folder in QGIS source tree
  * A "QGIS" code style is imported, which you can assign as the code style for your loaded project (or import just for your project, under `Project Settings` for your project in the `Projects` section of the main window)

### Useful Qt Creator shortcuts

* **Ctrl-K** - pops up quick search for a class
* **:spacebar** in the search popup will search for symbols
* **Ctrl-K** - then type 'git blame' and it will give git blame for currently open file
* **F2** - jump to symbol / definition under cursor
* **Alt-Enter** - refactoring you can automatically implement stubs for a method in a header
* **Alt-Enter** - refactoring you can generate getter and setter for a private member in a header
* **Alt-Enter** - general refactoring
* **Cmd-shift R** - refactor symbol name under the cursor
* **Cmd-B** - Build
* **Cmd-R** - Run w/o debugger
* **F5** - debug

### Troubleshooting

**Problem:**
/usr/local/Cellar/cmake/3.5.2/share/cmake/Modules/CMakeTestCCompiler.cmake:61: error: The C compiler "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc" is not able to compile a simple test program. It fails with the following output: Change Dir: /Users/timlinux/dev/cpp/QGIS/build/CMakeFiles/CMakeTmp

**Resolution:**

Close Qt Creator, remove CMakeLists.txt.user from your source tree and start again, see if that helps.

**Problem:**

CMAKE CODEBLOCKS GENERATOR not found

**Resolution:**

Manually set this to 'make' (or try ninja)

## Configure/build/install QGIS in CLion

[CLion](https://www.jetbrains.com/clion/) is a cross platform C++ IDE developed by JetBrains. Usage generally requires purchasing a commercial license, though they do provide free licenses for Open Source projects (subject to some criteria).

![screen shot 2017-03-19 at 10 39 28 pm](https://cloud.githubusercontent.com/assets/178003/24084554/f67a9b40-0cf4-11e7-8351-49324dc154ab.png)
Image above: An example session running CLion.

CLion is much simpler to set up as a build environment for QGIS compared to QtCreator and my be a good alternative for those used to developing with PyCharm for their Python Work (see PyCharm section below), since both PyCharm and CLion are build on the same 'engine' and have constencies in features, key bindings etc.

* Open project
* Open the CMakeLists.txt in QGIS Source tree
* Open CLion preferences
* Go to Build, execution, deployment
* Go to Toolchain
* CMake Executable: Custom: /usr/local/Cellar/cmake/3.6.1/bin/cmake
* CMake:
  * Generation group box
  * Configuration: RelWithDebInfo
  * CMake options:

```
-DWITH_BINDINGS=ON
-G
"Ninja"
-DCMAKE_CODEBLOCKS_EXECUTABLE:PATH='/usr/local/bin/ninja'
-DCMAKE_INSTALL_PREFIX:PATH='/Users/timlinux/Applications/'
-DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
-DCMAKE_FIND_FRAMEWORK:STRING=LAST
-DCMAKE_PREFIX_PATH:STRING='/usr/local/opt/qt5;/usr/local/opt/qt5-webkit-qt@5.7;/usr/local/opt/qscintilla2;/usr/local/opt/qwt;/usr/local/opt/qwtpolar;/usr/local/opt/qca-qt@5.7;/usr/local/opt/gdal2;/usr/local/opt/gsl;/usr/local/opt/geos;/usr/local/opt/proj;/usr/local/opt/libspatialite;/usr/local/opt/spatialindex;/usr/local/opt/fcgi;/usr/local/opt/expat;/usr/local/opt/sqlite;/usr/local/opt/flex;/usr/local/opt/bison;'
-DENABLE_MODELTEST:BOOL=FALSE
-DENABLE_TESTS:BOOL=TRUE
-DGDAL_LIBRARY:FILEPATH=/usr/local/opt/gdal2/lib/libgdal.dylib
-DGEOS_LIBRARY:FILEPATH=/usr/local/opt/geos/lib/libgeos_c.dylib
-DGSL_CONFIG:FILEPATH=/usr/local/opt/gsl/bin/gsl-config
-DGSL_INCLUDE_DIR:PATH=/usr/local/opt/gsl/include
-DGSL_LIBRARIES="-L/usr/local/opt/gsl/lib -lgsl -lgslcblas"
-DWITH_QWTPOLAR:BOOL=TRUE
-DWITH_INTERNAL_QWTPOLAR:BOOL=FALSE
-DWITH_GRASS:BOOL=FALSE
-DWITH_GRASS7:BOOL=TRUE
-DGRASS_PREFIX7:PATH=/usr/local/opt/grass7/grass-base
-DWITH_APIDOC:BOOL=FALSE
-DWITH_ASTYLE:BOOL=TRUE
-DWITH_CUSTOM_WIDGETS:BOOL=TRUE
-DWITH_GLOBE:BOOL=FALSE
-DWITH_ORACLE:BOOL=FALSE
-DWITH_QSCIAPI:BOOL=FALSE
-DWITH_QSPATIALITE:BOOL=FALSE
-DWITH_QTWEBKIT:BOOL=TRUE
-DWITH_SERVER:BOOL=TRUE
-DWITH_STAGED_PLUGINS:BOOL=TRUE
-DQGIS_MACAPP_DEV_PREFIX:PATH=/Users/timlinux/Applications/qgis-dev
-DQGIS_MACAPP_INSTALL_DEV:BOOL=TRUE
-DQGIS_MACAPP_BUNDLE:STRING=0
'/Users/timlinux/dev/cpp/QGIS'
-DCMAKE_CXX_COMPILER=/usr/local/opt/ccache/libexec/clang++
-DCMAKE_C_COMPILER=/usr/local/opt/ccache/libexec/clang
```

In our testing we found that with the above options, QGIS will compile but crash at runtime just after the splash screen. Adding the options below will resolve this issue - we will update this list to remove redundant entries in the near future.

```
-DCMAKE_AR:FILEPATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar
-DCMAKE_CXX_COMPILER:FILEPATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
-DCMAKE_C_COMPILER:FILEPATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc
-DCMAKE_LINKER:FILEPATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld
-DCMAKE_PREFIX_PATH:STRING=/usr/local/opt/qt5;/usr/local/opt/qt5-webkit-qt@5.7;/usr/local/opt/qscintilla2;/usr/local/opt/qwt;/usr/local/opt/qwtpolar;/usr/local/opt/qca-qt@5.7;/usr/local/opt/gdal2;/usr/local/opt/gsl;/usr/local/opt/geos;/usr/local/opt/proj;/usr/local/opt/libspatialite;/usr/local/opt/spatialindex;/usr/local/opt/fcgi;/usr/local/opt/expat;/usr/local/opt/sqlite;/usr/local/opt/flex;/usr/local/opt/bison;
-DCMAKE_RANLIB:FILEPATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib
-DCMAKE_STRIP:FILEPATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip
-DQCA_INCLUDE_DIR:PATH=/usr/local/opt/qca/lib/qca-qt5.framework/Headers
-DQCA_LIBRARY:FILEPATH=/usr/local/opt/qca/lib/qca-qt5.framework
-DQGIS_MACAPP_INSTALL_DEV:BOOL=FALSE
-DQt5WebKitWidgets_DIR:PATH=/usr/local/opt/qt5-webkit/lib/cmake/Qt5WebKitWidgets
-DQt5WebKit_DIR:PATH=/usr/local/opt/qt5-webkit/lib/cmake/Qt5WebKit
-DCMAKE_EXTRA_GENERATOR:INTERNAL=
```

Make sure to review the options - espectially paths near the bottom of the list to ensure
they are correct for your home directory.

For the most part these options were generated by calling the 

```
$(brew --repository qgis/qgisdev)/scripts/qgis-cmake-setup.sh
```

utility script provided with homebrew.


Generation path: Set to your home applications dir e.g.:
```
/Users/timlinux/dev/cpp/QGIS-CLion-Build
```

When debugging / running from CLion, ensure that GDAL python packages are in your path by adding this to your run environment:

```
PYTHONPATH set to $PYTHONPATH:/usr/local/opt/gdal2-python/lib/python3.5/site-packages
```

Alternatively, add the gdal2 packages to your environment directly in QGIS like this:

![screen shot 2017-03-19 at 8 37 54 pm](https://cloud.githubusercontent.com/assets/178003/24084521/6d5345ec-0cf4-11e7-9d86-e8af713a5b76.png)



## PyCharm

* Using paths in PyCharm and PyCharm test defaults:
* Using hand built QGIS:

Set your interpreter so add these python paths:

```
/usr/local/lib/python3.5/site-packages/
/Users/timlinux/Applications/QGIS.app/Contents/Resources/python/
/Users/timlinux/Applications/QGIS.app/Contents/Resources/python/plugins
```
￼

(Adjust these paths if you used a different install dir)

```
QGIS_PREFIX_PATH=/Users/timlinux/dev/cpp/QGIS/build/output/bin/QGIS.app/contents/MacOS;

PYTHONPATH=$PYTHONPATH:/Users/timlinux/Applications/QGIS.app/contents/Resources/python:/Users/timlinux/Applications/QGIS.app/contents/Resources/python/plugins/:/usr/local/lib/python3.5/site-packages/
```

###  For tests in PyCharm:

```
$PYTHONPATH:/Users/timlinux/Applications/QGIS.app/contents/Resources/python:/Users/timlinux//Applications/QGIS.app/Contents/Resources/python/plugins/
```

### Commit some changes to QGIS

Before committing your changes to QGIS, you need to run `scripts/prepare-commit.sh`. You need to install some tools first:
```
brew install gnu-sed
pip3 install autopep8
```
