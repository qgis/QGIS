# MXE cross build

Scripts to cross build a windows QGIS binary from Linux using MXE:
(M cross environment) http://mxe.cc/

The product is a zip file that contains a Windows build of QGIS,
ready to be unzipped on a Windows machine, it does not require
installation, just run the `qgis` binary.

Unfortunately it has some...

## Limitations

- No Python support
- No OpenCL support
- No support for the new native Windows overrides (notifications etc.)

## The easy way (requires docker)

From the main directory of QGIS repo issue the following command:

```
ms-windows/mxe/build.sh
```

## The hard way

Follow the instructions on the website to prepare the mxe environment, you
will need to build all required dependencies for QGIS (or see `mxe.Dockerfile` to get an idea).

The following command will select the posix threads enabled target and install
the dependencies required by QGIS:

```
make MXE_TARGETS=i686-w64-mingw32.shared.posix -j 16 \
    qca \
    qtlocation  \
    qscintilla2  \
    qwt  \
    gdal  \
    qtkeychain  \
    qtserialport  \
    qtwebkit \
    qtwinextras \
    libzip \
    gsl \
    libspatialindex
```

When done, you can check and edit the `build-mxe.sh` script and set the `MXE` path to your mxe installation directory, `MXE` can also be passed as an environment variable.

