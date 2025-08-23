从源代码构建QGIS - 分步指南

<!-- TOC start (generated with https://bitdowntoc.derlin.ch/) -->

- [1. 介绍](#1-介绍)
- [2. 概述](#2-概述)
- [3. 在GNU/Linux上构建](#3-在gnulinux上构建)
   * [3.1. 使用Qt 5.x构建QGIS](#31-使用qt-5x构建qgis)
   * [3.2. 准备apt](#32-准备apt)
   * [3.3. 安装构建依赖](#33-安装构建依赖)
   * [3.4. 设置ccache (可选，但推荐)](#34-设置ccache-可选但推荐)
   * [3.5. 准备开发环境](#35-准备开发环境)
   * [3.6. 检出QGIS源代码](#36-检出qgis源代码)
   * [3.7. 开始编译](#37-开始编译)
      + [3.7.1 可用的编译标志](#371-可用的编译标志)
   * [3.8. 使用3D编译](#38-使用3d编译)
      + [3.8.1. 在旧的基于Debian的发行版上使用3D编译](#381-在旧的基于debian的发行版上使用3d编译)
   * [3.9. 构建不同的分支](#39-构建不同的分支)
   * [3.10. 构建Debian软件包](#310-构建debian软件包)
      + [3.10.1. 构建支持Oracle的软件包](#3101-构建支持oracle的软件包)
   * [3.11. 在Fedora Linux上](#311-在fedora-linux上)
      + [3.11.1. 安装构建依赖](#3111-安装构建依赖)
      + [3.11.2. 建议的系统调优](#3112-建议的系统调优)
      + [3.11.3. QGIS开发的附加工具](#3113-qgis开发的附加工具)
      + [3.11.4. 使用Fedora Rawhide的QT6实验性构建](#3114-使用fedora-rawhide的qt6实验性构建)
   * [3.12. 在Linux上使用vcpkg构建](#312-在linux上使用vcpkg构建)
- [4. 在Windows上构建](#4-在windows上构建)
   * [4.1. 使用Microsoft Visual Studio构建](#41-使用microsoft-visual-studio构建)
      + [4.1.1. Visual Studio 2022 Community Edition](#411-visual-studio-2022-community-edition)
      + [4.1.2. 其他工具和依赖](#412-其他工具和依赖)
      + [4.1.3. 克隆QGIS源代码](#413-克隆qgis源代码)
      + [4.1.4. OSGeo4W](#414-osgeo4w)
   * [4.2. 在Linux上使用mingw64构建](#42-在linux上使用mingw64构建)
      + [4.2.1. 使用Docker构建](#421-使用docker构建)
         - [4.2.1.1. 初始设置](#4211-初始设置)
         - [4.2.1.2. 构建依赖](#4212-构建依赖)
         - [4.2.1.3. 交叉编译QGIS](#4213-交叉编译qgis)
      + [4.2.2. 测试QGIS](#422-测试qgis)
   * [4.3 在Windows上使用vcpkg构建](#43-在windows上使用vcpkg构建)
      + [4.3.1 安装构建工具](#431-安装构建工具)
      + [4.3.2 构建QGIS](#432-构建qgis)
         - [4.3.2.1 使用SDK构建](#4321-使用sdk构建)
         - [4.3.2.1 本地构建所有依赖](#4321-本地构建所有依赖)
- [5. 在MacOS X上构建](#5-在macos-x上构建)
   * [5.1. 使用Mac Packager构建](#51-使用mac-packager构建)
     + [5.1.1. 安装开发者工具](#511-安装开发者工具)
     + [5.1.2. 安装CMake和其他构建工具](#512-安装cmake和其他构建工具)
     + [5.1.3. 安装Qt5和QGIS-Deps](#513-安装qt5和qgis-deps)
     + [5.1.4. QGIS源代码](#514-qgis源代码)
     + [5.1.5. 配置构建](#515-配置构建)
     + [5.1.6. 构建](#516-构建)
   * [5.2. 使用vcpkg构建](#52-使用vcpkg构建)
- [6. 在GNU/Linux上设置WCS测试服务器](#6-在gnulinux上设置wcs测试服务器)
   * [6.1. 准备工作](#61-准备工作)
   * [6.2. 设置mapserver](#62-设置mapserver)
   * [6.3. 创建主页](#63-创建主页)
   * [6.4. 现在部署它](#64-现在部署它)
   * [6.5. 调试](#65-调试)
- [7. 设置Jenkins构建服务器](#7-设置jenkins构建服务器)
- [8. 调试输出和运行测试](#8-调试输出和运行测试)
- [9. 作者和致谢](#9-作者和致谢)

<!-- TOC end -->

# 1. 介绍

本文档是所述软件QGIS的原始安装指南。本文档中提及的软件和硬件描述在大多数情况下都是注册商标，因此受到法律要求的约束。QGIS受GNU通用公共许可证约束。在QGIS主页上查找更多信息：
https://qgis.org

本文档中给出的详细信息已经根据编辑者的最佳知识和责任编写和验证。尽管如此，仍可能存在关于内容的错误。因此，所有数据不承担任何义务或保证。编辑者和出版商对故障及其后果不承担任何责任或义务。欢迎您指出可能的错误。

由于QGIS的代码在每个版本之间都在演进，这些说明会定期更新以匹配相应的版本。当前master分支的说明可在https://github.com/qgis/QGIS/blob/master/INSTALL.md获得。如果您希望构建QGIS的其他版本，请确保检出相应的发布分支。QGIS源代码可以在[仓库](https://github.com/qgis/QGIS)中找到。

请访问https://qgis.org获取有关加入我们邮件列表和进一步参与项目的信息。

**文档编写者注意：**请将此文档作为描述构建过程的中心位置。请不要删除此通知。

# 2. 概述

QGIS，像许多主要项目（例如KDE 4.0）一样，使用[CMake](https://www.cmake.org)从源代码构建。

以下是构建所需依赖的摘要：

必需的构建工具：

* CMake >= 3.12.0
* Flex >= 2.5.6
* Bison >= 2.4
* Python >= 3.11

必需的构建依赖：

* Qt >= 5.15.2
* Proj >= 8.1.0
* GEOS >= 3.9
* Sqlite3 >= 3.0.0
* SpatiaLite >= 4.2.0
* libspatialindex
* GDAL/OGR >= 3.2.0
* Qwt >= 5.0 & (< 6.1 with internal QwtPolar)
* expat >= 1.95
* QScintilla2
* QCA
* qtkeychain (>= 0.5)
* libzip
* protobuf

可选依赖：

* 用于GRASS提供者和插件 - GRASS >= 7.0.0.
* 用于地理配准器 - GSL >= 1.8
* 用于PostGIS支持 - PostgreSQL >= 8.0.x
* 用于gps插件 - gpsbabel
* 用于mapserver导出和PyQGIS - Python >= 3.6
* 用于python支持 - SIP >= 4.12, PyQt >= 5.3 必须匹配Qt版本, Qscintilla2
* 用于qgis mapserver - FastCGI
* 用于oracle提供者 - Oracle OCI库

间接依赖：

GDAL支持的一些专有格式（例如，ECW和MrSid）需要专有的第三方库。QGIS本身构建时不需要任何这些库，但只有在GDAL相应构建时才会支持这些格式。请参考[格式列表](https://gdal.org/index.html)了解如何在GDAL中包含这些格式的说明。

# 3. 在GNU/Linux上构建

## 3.1. 使用Qt 5.x构建QGIS

**要求：** Ubuntu / Debian派生发行版

**注意：** 请参阅[构建Debian软件包](#310-构建debian软件包)部分了解构建debian软件包的信息。除非您计划开发QGIS，否则这可能是编译和安装QGIS的最简单选项。

这些说明适用于Ubuntu - 其他版本和Debian派生发行版可能在软件包名称上需要略微不同。

这些说明是针对您想要从源代码构建QGIS的情况。这里的一个主要目标是展示如何使用***所有***依赖项的二进制包来完成此操作 - 只从源代码构建核心QGIS内容。我更喜欢这种方法，因为这意味着我们可以将管理系统包的业务留给apt，只专注于编码QGIS！

本文档假设您进行了全新安装并拥有一个"干净"的系统。如果这是一个已经使用了一段时间的系统，这些说明应该可以正常工作，您可能只需要跳过那些与您无关的步骤。

## 3.2. 准备apt

您需要一个足够新的发行版以满足所有依赖项。支持的发行版在以下部分中列出。

现在更新您的本地源数据库：

```bash
sudo apt-get update
```

## 3.3. 安装构建依赖

|发行版|软件包安装命令|
|------|------------|
| bookworm | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| trixie | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| jammy | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpdal-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pdal pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| kinetic | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| lunar | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| mantic | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| noble | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| oracular | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqt5.qtwebkit python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| plucky | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |
| sid | ``apt-get install bison build-essential ca-certificates ccache cmake cmake-curses-gui dh-python expect flex flip gdal-bin git graphviz grass-dev libdraco-dev libexiv2-dev libexpat1-dev libfcgi-dev libgdal-dev libgeos-dev libgsl-dev libpq-dev libproj-dev libprotobuf-dev libqca-qt5-2-dev libqca-qt5-2-plugins libqscintilla2-qt5-dev libqt5opengl5-dev libqt5serialport5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5xmlpatterns5-dev libqwt-qt5-dev libspatialindex-dev libspatialite-dev libsqlite3-dev libsqlite3-mod-spatialite libyaml-tiny-perl libzip-dev libzstd-dev lighttpd locales ninja-build nlohmann-json3-dev ocl-icd-opencl-dev opencl-headers pandoc pkgconf poppler-utils protobuf-compiler pyqt5-dev pyqt5-dev-tools pyqt5.qsci-dev python3-all-dev python3-autopep8 python3-dev python3-gdal python3-jinja2 python3-lxml python3-mock python3-nose2 python3-owslib python3-packaging python3-plotly python3-psycopg2 python3-pygments python3-pyproj python3-pyqt5 python3-pyqt5.qsci python3-pyqt5.qtmultimedia python3-pyqt5.qtpositioning python3-pyqt5.qtserialport python3-pyqt5.qtsql python3-pyqt5.qtsvg python3-pyqtbuild python3-sip python3-termcolor python3-yaml qt3d-assimpsceneimport-plugin qt3d-defaultgeometryloader-plugin qt3d-gltfsceneio-plugin qt3d-scene2d-plugin qt3d5-dev qtbase5-dev qtbase5-private-dev qtkeychain-qt5-dev qtmultimedia5-dev qtpositioning5-dev qttools5-dev qttools5-dev-tools sip-tools spawn-fcgi xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-scalable xvfb`` |

（从`debian/`中的control.in文件提取）

请参阅[debian-ubuntu](https://qgis.org/resources/installation-guide/#debianubuntu)了解当前支持的发行版（例如，普通xenial的GDAL版本太旧，我们使用来自ubuntugis的GDAL2构建）。

要构建[QGIS服务器登录页面/目录Web应用](https://docs.qgis.org/latest/en/docs/server_manual/catalog.html)，需要额外的依赖项：

* Node.js（推荐当前LTS）: https://nodejs.org/en/download/
* Yarn包管理器: https://yarnpkg.com/getting-started/install

此外，需要打开cmake标志`WITH_SERVER_LANDINGPAGE_WEBAPP`。

## 3.4. 设置ccache（可选，但推荐）

您还应该设置ccache来加速编译时间：

```bash
cd /usr/local/bin
sudo ln -s /usr/bin/ccache gcc
sudo ln -s /usr/bin/ccache g++
```

或者简单地将`/usr/lib/ccache`添加到您的`PATH`中。

## 3.5. 准备您的开发环境

按照惯例，我在$HOME/dev/<语言>中进行所有开发工作，所以在这种情况下，我们将为C++开发工作创建一个工作环境，如下所示：

```bash
mkdir -p ${HOME}/dev/cpp
cd ${HOME}/dev/cpp
```

以下所有说明都将假设此目录路径。

## 3.6. 检出QGIS源代码

有两种方式可以检出源代码。如果您没有QGIS源代码仓库的编辑权限，请使用匿名方法，或者如果您有提交源代码更改的权限，请使用开发者检出。

1. 匿名检出

```bash
cd ${HOME}/dev/cpp
git clone https://github.com/qgis/QGIS.git
```

2. 开发者检出

```bash
cd ${HOME}/dev/cpp
git clone git@github.com:qgis/QGIS.git
```

## 3.7. 开始编译

我将我的QGIS开发版本编译到我的~/apps目录中，以避免与/usr下可能存在的Ubuntu包发生冲突。这样，例如，您可以在系统上使用QGIS的二进制包，同时使用您的开发版本。我建议您也做类似的事情：

```bash
mkdir -p ${HOME}/apps
```

现在我们创建一个构建目录并运行ccmake：

```bash
cd QGIS
mkdir build-master
cd build-master
ccmake ..
```

当您运行ccmake时（注意..是必需的！），会出现一个菜单，您可以在其中配置构建的各个方面：

* 如果您希望QGIS具有调试功能，则将`CMAKE_BUILD_TYPE`设置为`Debug`。
* 如果您没有root访问权限或不想覆盖现有的QGIS安装（例如，由您的包管理器），将`CMAKE_INSTALL_PREFIX`设置为您有写入权限的地方（例如`${HOME}/apps`）。

现在按'c'配置，'e'关闭可能出现的任何错误消息，按'g'生成make文件。请注意，有时需要按几次'c'才能使'g'选项变为可用。在'g'生成完成后，按'q'退出ccmake交互式对话框。

**警告：** 确保您输入命令时构建目录完全为空。永远不要尝试"重用"现有的**Qt5**构建目录。如果您想使用`ccmake`或其他交互式工具，请在开始使用交互式工具之前在空的构建目录中运行一次命令。

现在继续构建：
```bash
make -jX
```

其中X是可用核心的数量。根据您的平台，这可以大大加快构建时间。

然后您可以直接从构建目录运行：
```bash
./output/bin/qgis
```
另一个选项是安装到您的系统：
```bash
make install
```

之后您可以尝试运行QGIS：
```bash
$HOME/apps/bin/qgis
```
如果一切正常，QGIS应用程序应该启动并出现在您的屏幕上。如果您收到"加载共享库时出错"的错误消息，请在shell中执行此命令。
```bash
sudo ldconfig
```
如果这没有帮助，请将安装路径添加到LD_LIBRARY_PATH：
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HOME}/apps/lib/
```
或者，如果您已经知道自定义构建中需要哪些方面，那么您可以通过为每个方面使用cmake -D选项来跳过交互式ccmake ..部分，例如：
```bash
cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=${HOME}/apps ..
```
此外，如果您想加快构建时间，可以轻松使用ninja，这是make的替代品，具有类似的构建选项。

例如，要配置构建，您可以执行以下任一操作：
```bash
ccmake -G Ninja ..
cmake -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=${HOME}/apps ..
```
使用ninja构建和安装：
```bash
ninja   # (默认使用所有核心；也支持上述描述的-jX选项)
ninja install
```
要构建得更快，您可以只构建所需的目标，例如：
```bash
ninja qgis
ninja pycore
# 如果只是桌面相关代码：
ninja qgis_desktop
```

### 3.7.1 可用的编译标志

QGIS构建可根据您的需求进行调整。有许多标志可用于激活或停用某些功能，以下是一个子集：

* `BUILD_WITH_QT6`: 启用（实验性）Qt6支持
* `WITH_3D`: 确定是否应构建QGIS 3D库
* `WITH_ANALYSIS`: 确定是否应构建QGIS分析库
* `WITH_AUTH`: 确定是否应构建QGIS身份验证方法
* `WITH_BINDINGS`: 确定是否应构建Python绑定
* `WITH_COPC`: 确定是否应构建云优化点云(COPC)支持
* `WITH_DESKTOP`: 确定是否应构建QGIS桌面
* `WITH_EPT`: 确定是否应构建Entwine点云(EPT)支持
* `WITH_GRASS${GRASS_CACHE_VERSION}`: 确定是否应构建GRASS ${GRASS_SEARCH_VERSION}插件
* `WITH_GUI`: 确定是否应构建QGIS GUI库（以及在其之上构建的所有内容）
* `WITH_HANA`: 确定是否应构建SAP HANA Spatial支持
* `WITH_ORACLE`: 确定是否应构建Oracle支持
* `WITH_PDAL`: 确定是否应构建PDAL支持
* `WITH_POSTGRESQL`: 确定是否应构建POSTGRESQL支持
* `WITH_PYTHON`: 确定是否应构建Python支持（禁用它将特别禁用处理）
* `WITH_QGIS_PROCESS`: 确定是否应构建独立的\"qgis_process\"工具
* `WITH_QSPATIALITE`: 确定是否应构建QSPATIALITE sql驱动程序
* `WITH_SERVER`: 确定是否应构建QGIS服务器
* `WITH_SPATIALITE`: 确定是否应构建Spatialite支持（spatialite、虚拟、wfs提供者所需）

完整列表可以使用以下命令行从源代码中提取：

```bash
cmake .. -N -LH | grep -B1 WITH_
```

## 3.8. 使用3D编译

在cmake中，您需要启用：
```bash
WITH_3D=True
```

### 3.8.1. 在旧的基于Debian的发行版上使用3D编译

QGIS 3D需要Qt53DExtras。这些头文件已从Debian Buster和Ubuntu focal (20.04)基础发行版的Qt上游中删除。在QGIS仓库的`external/qt3dextra-headers`中已制作了副本。要在这些发行版上启用3D编译，您需要添加一些cmake选项：

```bash
CMAKE_PREFIX_PATH={QGIS Git仓库路径}/external/qt3dextra-headers/cmake
QT5_3DEXTRA_INCLUDE_DIR={QGIS Git仓库路径}/external/qt3dextra-headers
QT5_3DEXTRA_LIBRARY=/usr/lib/x86_64-linux-gnu/libQt53DExtras.so
Qt53DExtras_DIR={QGIS Git仓库路径}/external/qt3dextra-headers/cmake/Qt53DExtras
```

上述说明不适用于更新版本的Debian和Ubuntu。

## 3.9. 构建不同的分支

通过使用`git worktree`，您可以在不同分支之间切换，以并行使用基于相同Git配置的多个源代码。我们建议您阅读关于此Git命令的文档：

```bash
git commit
git worktree add ../my_new_functionality
cd ../my_new_functionality
git fetch qgis/master
git rebase -i qgis/master
# 只保留要推送的提交
git push -u my_own_repo my_new_functionality
```

## 3.10. 构建Debian软件包

您也可以创建debian包，而不是像前面步骤中那样创建个人安装。这需要在QGIS根目录中完成，在那里您会找到一个debian目录。

首先为您的发行版设置一个变更日志条目。例如对于Debian Bookworm：

```bash
dch -l ~bookworm --force-distribution --distribution bookworm "bookworm build"
```

您还需要安装[构建依赖](#33-安装构建依赖)。或者使用：

```bash
debian/rules templates
sudo mk-build-deps -i
```

QGIS包将通过以下命令创建：

```bash
dpkg-buildpackage -us -uc -b
```

**注意：** 安装`devscripts`以获得`dch`和`mk-build-deps`。

**注意：** 如果您安装了`libqgis1-dev`，您需要首先使用`dpkg -r libqgis1-dev`删除它。否则`dpkg-buildpackage`会抱怨构建冲突。

**注意：** 默认情况下，测试在构建过程中运行，其结果会上传到https://cdash.orfeo-toolbox.org/index.php?project=QGIS。您可以在构建命令前使用`DEB_BUILD_OPTIONS=nocheck`关闭测试。可以使用`DEB_TEST_TARGET=test`避免上传结果。

包在父目录中创建（即上一级）。使用`dpkg`安装它们。例如：

```bash
sudo debi
```

### 3.10.1. 构建支持Oracle的软件包

要构建支持Oracle的软件包，您需要Oracle库（当前为21.11）作为额外的构建依赖：

```bash
curl -JLO https://download.oracle.com/otn_software/linux/instantclient/2111000/oracle-instantclient-devel-21.11.0.0.0-1.el8.x86_64.rpm
curl -JLO https://download.oracle.com/otn_software/linux/instantclient/2111000/oracle-instantclient-basiclite-21.11.0.0.0-1.el8.x86_64.rpm
sudo apt install alien
fakeroot alien oracle-instantclient-devel-21.11.0.0.0-1.el8.x86_64.rpm oracle-instantclient-basiclite-21.11.0.0.0-1.el8.x86_64.rpm
sudo dpkg -i oracle-instantclient-devel_21.11.0.0.0-2_amd64.deb oracle-instantclient-basiclite_21.11.0.0.0-2_amd64.deb
```

（如果客户端版本更改，需要相应调整`debian/rules`中的`ORACLE_INCLUDEDIR`和`ORACLE_LIBDIR`）

如果发行版包含`-oracle`，打包文件会启用Oracle支持：

```bash
dch -l ~sid~oracle --force-distribution --distribution sid-oracle "sid build with oracle"
dpkg-buildpackage -us -uc -b
```

## 3.11. 在Fedora Linux上

我们假设您已经准备好QGIS的源代码，并在其中创建了一个名为`build`或`build-qt5`的新子目录。

### 3.11.1. 安装构建依赖

|发行版|软件包安装命令|
|------|------------|
| Fedora 38 Workstation | ``dnf install qt5-qtbase-private-devel qt5-qtwebkit-devel qt5-qtlocation-devel qt5-qtmultimedia-devel qt5-qttools-static qca-qt5-devel qca-qt5-ossl qt5-qt3d-devel python3-qt5-devel python3-qscintilla-qt5-devel qscintilla-qt5-devel python3-qscintilla-qt5 clang flex bison geos-devel gdal  gdal-devel hdf5-devel sqlite-devel libspatialite-devel qt5-qtsvg-devel spatialindex-devel expat-devel netcdf-devel proj-devel qwt-qt5-devel gsl-devel PDAL PDAL-devel postgresql-devel cmake python3-gdal gdal-python-tools python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt5-ossl qwt-qt5-devel qtkeychain-qt5-devel libzip-devel exiv2-devel  PyQt-builder protobuf-lite protobuf-lite-devel libzstd-devel qt5-qtserialport-devel draco-devel python3-devel`` |
| Fedora 37 Workstation | ``dnf install qt5-qtbase-private-devel qt5-qtwebkit-devel qt5-qtlocation-devel qt5-qtmultimedia-devel qt5-qttools-static qca-qt5-devel qca-qt5-ossl qt5-qt3d-devel python3-qt5-devel python3-qscintilla-qt5-devel qscintilla-qt5-devel python3-qscintilla-qt5 clang flex bison geos-devel gdal  gdal-devel hdf5-devel sqlite-devel libspatialite-devel qt5-qtsvg-devel spatialindex-devel expat-devel netcdf-devel proj-devel qwt-qt5-devel gsl-devel PDAL PDAL-devel postgresql-devel cmake python3-gdal gdal-python-tools python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt5-ossl qwt-qt5-devel qtkeychain-qt5-devel qwt-devel libzip-devel exiv2-devel python3-sip-devel protobuf-lite protobuf-lite-devel libzstd-devel qt5-qtserialport-devel draco-devel`` |
| Fedora 35/36 Workstation | ``dnf install qt5-qtbase-private-devel qt5-qtwebkit-devel qt5-qtlocation-devel qt5-qtmultimedia-devel qt5-qttools-static qca-qt5-devel qca-qt5-ossl qt5-qt3d-devel python3-qt5-devel python3-qscintilla-qt5-devel qscintilla-qt5-devel python3-qscintilla-qt5 clang flex bison geos-devel gdal gdal-devel hdf5-devel sqlite-devel libspatialite-devel qt5-qtsvg-devel spatialindex-devel expat-devel netcdf-devel proj-devel qwt-qt5-devel gsl-devel PDAL PDAL-devel postgresql-devel cmake gdal-python3 gdal-python-tools python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt5-ossl qwt-qt5-devel qtkeychain-qt5-devel qwt-devel libzip-devel exiv2-devel python3-sip-devel protobuf-lite protobuf-lite-devel libzstd-devel qt5-qtserialport-devel draco-devel`` |
| 旧版本 | ``dnf install qt5-qtbase-private-devel qt5-qtwebkit-devel qt5-qtlocation-devel qt5-qtmultimedia-devel qt5-qttools-static qca-qt5-devel qca-qt5-ossl qt5-qt3d-devel python3-qt5-devel python3-qscintilla-qt5-devel qscintilla-qt5-devel python3-qscintilla-devel python3-qscintilla-qt5 clang flex bison geos-devel gdal gdal-devel sqlite-devel libspatialite-devel qt5-qtsvg-devel qt5-qtbase-tds qt5-qtbase-odbc spatialindex-devel expat-devel proj-devel qwt-qt5-devel gsl-devel postgresql-devel cmake gdal-python3 python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt5-ossl qwt-qt5-devel qtkeychain-qt5-devel qwt-devel sip-devel libzip-devel exiv2-devel draco-devel`` |

要构建QGIS服务器，需要额外的依赖：

```bash
dnf install fcgi-devel
```

要构建[QGIS服务器登录页面/目录Web应用](https://docs.qgis.org/latest/en/docs/server_manual/services.html#qgis-server-catalog)：

```bash
dnf install nodejs yarnpkg
```

此外，需要打开cmake标志`WITH_SERVER_LANDINGPAGE_WEBAPP`。

确保在输入以下命令时您的构建目录完全为空。永远不要尝试"重用"现有的Qt5构建目录。如果您想使用`ccmake`或其他交互式工具，请在开始使用交互式工具之前在空的构建目录中运行一次以下命令。

```bash
cmake ..
```

如果一切顺利，您最终可以开始编译。（像往常一样，添加`-jX`选项，其中X是可用核心数，以加速构建过程）

```bash
make
```

从构建目录运行

```bash
./output/bin/qgis
```

或安装到您的系统

```bash
make install
```

### 3.11.2. 建议的系统调优

默认情况下，Fedora禁用来自Qt应用程序的调试调用。这阻止了通常在运行单元测试时打印的有用调试输出。

要为当前用户启用调试打印，执行：

```bash
cat > ~/.config/QtProject/qtlogging.ini << EOL
[Rules]
default.debug=true
EOL
```

### 3.11.3. QGIS开发的附加工具

如果您要在Fedora系统上开发QGIS，各种QGIS源代码格式化和准备脚本需要以下额外的包。

```bash
dnf install ag ccache expect ninja-build astyle python3-autopep8 python3-mock python3-nose2 perl-YAML-Tiny
```

### 3.11.4. 使用Fedora Rawhide的QT6实验性构建

这需要最新的QGIS master（>= 2024年1月25日）。

为了节省约700MB的空间，如果您想在不安装可选网格的情况下安装PROJ，请首先执行：

```bash
dnf5 install -y --setopt=install_weak_deps=False proj-devel
```

安装所有所需的构建依赖：

|发行版|软件包安装命令|
|------|------------|
| Fedora 40 Workstation | ``dnf install qt6-qtbase-private-devel qt6-qtlocation-devel qt6-qtmultimedia-devel qt6-qttools-static qca-qt6-devel qca-qt6-ossl qt6-qt3d-devel qt6-qtwebengine-devel python3-pyqt6-devel python3-qscintilla-qt6-devel qscintilla-qt6-devel python3-qscintilla-qt6 clang flex bison geos-devel gdal gdal-devel hdf5-devel sqlite-devel libspatialite-devel qt6-qtsvg-devel spatialindex-devel expat-devel netcdf-devel proj-devel qwt-qt6-devel gsl-devel PDAL PDAL-devel postgresql-devel cmake python3-gdal gdal-python-tools python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt6-ossl qwt-qt6-devel qtkeychain-qt6-devel libzip-devel exiv2-devel PyQt-builder protobuf-lite protobuf-lite-devel libzstd-devel qt6-qtserialport-devel draco-devel python3-devel qt6-qt5compat-devel python3-pyqt6-webengine`` |
| Fedora 39 Workstation | ``dnf install qt6-qtbase-private-devel qt6-qtwebkit-devel qt6-qtlocation-devel qt6-qtmultimedia-devel qt6-qttools-static qca-qt6-devel qt6-qtwebengine-devel qca-qt6-ossl qt6-qt3d-devel python3-qt6-devel python3-qscintilla-qt6-devel qscintilla-qt6-devel python3-qscintilla-qt6 clang flex bison geos-devel gdal gdal-devel hdf5-devel sqlite-devel libspatialite-devel qt6-qtsvg-devel spatialindex-devel expat-devel netcdf-devel proj-devel qwt-qt6-devel gsl-devel PDAL PDAL-devel postgresql-devel cmake python3-gdal gdal-python-tools python3-psycopg2 python3-PyYAML python3-pygments python3-jinja2 python3-OWSLib qca-qt6-ossl qwt-qt6-devel qtkeychain-qt6-devel libzip-devel exiv2-devel PyQt-builder protobuf-lite protobuf-lite-devel libzstd-devel qt6-qtserialport-devel draco-devel python3-devel qt6-qt5compat-devel`` |


要构建，使用：

```bash
cmake .. -DBUILD_WITH_QT6=ON -DWITH_QTWEBKIT=OFF -DWITH_QTWEBENGINE=ON
```

## 3.12. 在Linux上使用vcpkg构建

使用[vcpkg](https://github.com/microsoft/vcpkg/)，您可以在Linux系统上使用Qt6开发QGIS。

首先，[安装和初始化vcpkg](https://github.com/microsoft/vcpkg-tool/blob/main/README.md#installuseremove)。

获取QGIS源代码：

```sh
git clone git@github.com:qgis/QGIS.git
```

配置：

```sh
cmake -S . \
      -B ./build-x64-linux \
      -GNinja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DWITH_VCPKG=ON \
      -DBUILD_WITH_QT6=ON \
      -DWITH_QTWEBKIT=OFF \
      -DWITH_BINDINGS=ON \
      -DVCPKG_TARGET_TRIPLET=x64-linux-dynamic-release \
      -DVCPKG_HOST_TRIPLET=x64-linux-dynamic-release
```

构建：

```sh
cmake --build ./build-x64-linux
```

# 4. 在Windows上构建

## 4.1. 使用Microsoft Visual Studio构建

本节描述如何在Windows上使用Visual Studio (MSVC) 2022构建QGIS。官方Windows包是使用OSGeo4W构建的。

本节描述允许使用Visual Studio构建QGIS所需的设置。

### 4.1.1. Visual Studio 2022 Community Edition

下载并安装[免费的Community安装程序](https://c2rsetup.officeapps.live.com/c2r/downloadVS.aspx?sku=community&channel=Release&version=VS2022&source=VSLandingPage&cid=2030:7851336a02d44ba38a548acc719002df)

选择"使用C++的桌面开发"

### 4.1.2. 其他工具和依赖

下载并安装以下包：

* [CMake](https://github.com/Kitware/CMake/releases/download/v3.31.4/cmake-3.31.4-windows-x86_64.msi)
* 使用[cygwin 64bit](https://cygwin.com/setup-x86_64.exe)安装GNU flex、GNU bison和GIT
* [OSGeo4W](https://download.osgeo.org/osgeo4w/v2/osgeo4w-setup.exe)
* [ninja](https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip)（版本 >= 1.10）：将`ninja.exe`复制到`C:\OSGeo4W\bin\`

对于QGIS构建，您需要从cygwin安装以下包：

* bison
* flex
* git（即使您已经安装了Git for Windows）

从OSGeo4W安装（选择*高级安装*）：

* qgis-dev-deps

  * 这也会选择上述包所依赖的包。

  * 注意：如果您安装其他包，可能会导致问题。特别是，确保**不要**安装msinttypes包。它在`OSGeo4W\include`中安装了一个stdint.h文件，与Visual Studio自己的stdint.h冲突，例如会破坏虚拟图层提供者的构建。

如果您打算构建所有依赖项，可以参考[OSGeo4W仓库](https://github.com/jef-n/OSGeo4W)。

### 4.1.3. 克隆QGIS源代码

选择一个目录来存储QGIS源代码。例如，要将其放在OSGeo4W安装目录中，导航到那里：

```cmd
cd C:\OSGeo4W
```

以下所有说明都假设使用此目录。

在命令提示符中从git克隆QGIS源代码到源目录`QGIS`：

```cmd
git clone git://github.com/qgis/QGIS.git
```

这需要Git。如果您的PATH中已经有Git for Windows，您可以从普通命令提示符执行此操作。如果没有，您可以使用作为Cygwin一部分安装的Git包，通过打开Cygwin[64]终端。

为了避免Windows中的Git报告实际上未修改的文件更改：

```cmd
cd QGIS
git config core.filemode false
```

### 4.1.4. OSGeo4W

官方Windows包是在OSGeo4W中制作的。独立的MSI安装程序也是使用OSGeo4W的包制作的。

master的每日构建（包qgis-dev）是使用[OSGeo4W构建配方](https://github.com/jef-n/OSGeo4W/blob/master/src/qgis-dev/osgeo4w/package.sh)制作的。

还有用于长期发布构建（qgis-ltr）、下一个长期点发布的每日构建（qgis-ltr-dev）、最新发布（qgis）、下一个点发布的每日构建（qgis-rel-dev）的其他配方。所有这些当前都基于Qt5。master与Qt6的构建配方可作为qgis-qt6-dev使用。

要设置构建环境（如果尚未安装则包括Visual C++）并构建每日构建，在命令行（cmd）中运行：

```cmd
mkdir osgeo4w-build
cd osgeo4w-build
curl -JLO https://raw.githubusercontent.com/jef-n/OSGeo4W/refs/heads/master/bootstrap.cmd
curl -JLO https://raw.githubusercontent.com/jef-n/OSGeo4W/refs/heads/master/bootstrap.sh
bootstrap.cmd qgis-dev
```

要构建其他包，您可以简单地将qgis-dev替换为其他包。调用不带任何参数的`bootstrap.cmd`将构建所有包（包括所有依赖项）。

构建成功后，OSGeo4W包将在`x86_64/`中。

要安装包，您可以在OSGeo4W安装程序中添加`osgeo4w-build`目录作为`用户URL`并选择您的`qgis-dev`包。

MSI是使用`scripts\msis.sh`（实际上是`scripts/creatmsi.pl`）创建的。

## 4.2. 在Linux上使用mingw64构建

使用这种方法，您可以在Linux上使用Docker容器中的mingw64交叉构建Windows二进制文件。

要从您的QGIS源目录在Linux上构建，启动：

```cmd
ms-windows/mingw/build.sh
```

成功构建后，您将在QGIS源目录中找到两个包：

- qgis-portable-win64.zip（用于Windows 64位的QGIS）
- qgis-portable-win64-debugsym.zip（调试符号）

这种方法也用于持续集成过程。在每个拉取请求后，上述两个包作为GitHub操作工件存储，可供下载，使得在Windows上快速测试更改成为可能。

### 4.2.1. 使用Docker构建

这是最简单的方法，但您需要在系统上安装Docker。

您可以通过从QGIS仓库的根目录调用脚本ms-windows/mxe/build.sh来使用Docker镜像交叉构建QGIS。

==== 不使用Docker构建 ====

这需要在您的系统上安装mxe工具链并自己构建所有依赖项。

#### 4.2.1.1. 初始设置

请按照mxe网站上的说明设置您的构建工具链 http://mxe.cc/，记下您安装mxe的路径。

#### 4.2.1.2. 构建依赖

请参见ms-windows/mxe下的README.md以获取详细说明，以及在尝试构建QGIS之前需要在mxe中构建的依赖项列表。

#### 4.2.1.3. 交叉编译QGIS

编辑build-mxe.sh脚本，可选地调整mxe安装位置的路径，您也可以更改构建和发布目录。

### 4.2.2. 测试QGIS

将构建产生的包复制并解压到Windows机器上，然后启动qgis二进制文件：不需要安装。

## 4.3 在Windows上使用vcpkg构建

Vcpkg是一个免费开源的跨平台库生态系统。它提供对所使用依赖项版本的精确控制。

### 4.3.1 安装构建工具

1. 下载并安装免费的[Microsoft Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022)。
2. 获取[winflexbison](https://github.com/lexxmark/winflexbison/releases)。

### 4.3.2 构建QGIS

有两种使用vcpkg构建的方法。您可以下载SDK，这允许您重用已构建的依赖项。这通常更容易，也是推荐的。也可以自己构建所有依赖项，这给您对依赖项更多的控制。

#### 4.3.2.1 使用SDK构建

1. 下载并解压SDK。获取最新的[QGIS master SDK](https://nightly.link/qgis/QGIS/workflows/windows-qt6/master/qgis-sdk-x64-windows.zip)。
2. 构建

我们现在将配置QGIS。

打开_Developer PowerShell for VS 2022_

```ps
# 我们假设您有QGIS源代码的副本
# 并已将工作目录更改到其中

# 配置
cmake -S . `
      -B build `
      -DSDK_PATH="path/to/vcpkg-export-[date]" `
      -DBUILD_WITH_QT6=ON `
      -DWITH_QTWEBKIT=OFF `
      -DVCPKG_TARGET_TRIPLET=x64-windows-release `
      -DFLEX_EXECUTABLE="path/to/flex-executable" `
      -DBISON_EXECUTABLE="path/to/bison-executable"
```

这将为您提供一个配置好的项目。您可以直接从命令行构建它。

```ps
# 构建
# 注意：您也可以使用RelWithDebInfo在构建中包含调试符号
cmake --build build --config Release
```

或者您可以使用Visual Studio打开`build`文件夹中生成的`.sln`文件。这将允许您使用此IDE提供的所有工具。

#### 4.3.2.1 本地构建所有依赖

也可以本地构建所有依赖项。这将需要一些时间、CPU和磁盘空间。

```ps
# 我们假设您有QGIS源代码的副本
# 并已将工作目录更改到其中

# 配置
cmake -S . `
      -B build `
      -D WITH_VCPKG=ON `
      -D BUILD_WITH_QT6=ON `
      -D WITH_QTWEBKIT=OFF `
      -D VCPKG_TARGET_TRIPLET=x64-windows-release `
      -D VCPKG_HOST_TRIPLET=x64-windows-release
```

**管理依赖版本**

依赖项在文件`vcpkg/vcpkg.json`中定义。此文件定义使用哪些版本的注册表。要更新到最新版本，您可以使用命令`vcpkg x-update-baseline --x-manifest-root=vcpkg`。如果您想修补或更新特定的依赖项，可以将该依赖项的端口复制到文件夹`vcpkg/ports`中。每当重新配置构建时，它将检查需要重新构建的依赖项。

# 5. 在MacOS X上构建

如果您想测试QGIS，最简单的选择是直接从以下地址下载并安装一体化自包含包：

https://download.qgis.org/downloads/macos/

如果您想自己构建或开发QGIS，您需要一组依赖项和工具。您可以使用：

- 用于一体化QGIS包的同一组依赖项
- 基于vcpkg的依赖项，用于使用Qt6构建
- 使用Homebrew、MacPorts或Conda依赖项（本指南不涵盖）

## 5.1. 使用Mac Packager构建

https://github.com/qgis/QGIS-Mac-Packager

包括在安装了最新更新的最新Mac OS X上构建的说明。构建使用clang编译器。

并行编译：在多处理器/多核Mac上，可以加速编译，但这不是自动的。每当您键入"make"（但不是"make install"）时，请键入：

```bash
make -j [#cpus]
```

用您的Mac拥有的核心和/或处理器数量替换[#cpus]。要找出您有多少可用的CPU，请在终端中运行以下命令：

```bash
/usr/sbin/sysctl -n hw.ncpu
```

### 5.1.1. 安装开发者工具

开发者工具不是标准OS X安装的一部分。至少您需要命令行工具：

```bash
sudo xcode-select --install
```

但也建议从App Store安装Xcode。

### 5.1.2. 安装CMake和其他构建工具

例如安装Homebrew：

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

和这些开发/构建工具：

```bash
brew install git cmake ninja pkg-config wget bash-completion curl gnu-sed coreutils ccache libtool astyle help2man autoconf automake pandoc draco
```

如果您从MacPorts或Conda安装了这些工具，也是一样的，但我们需要能够在以下步骤中从终端运行`cmake`和其他工具。

### 5.1.3. 安装Qt5和QGIS-Deps

要构建QGIS，我们需要手头有Qt5和FOSS依赖项。Qt5版本理想情况下应与用于构建依赖包的版本匹配。

从以下地址下载最新的QGIS-Deps安装脚本、qt包和QGIS-Deps包：

https://download.qgis.org/downloads/macos/deps

您的下载文件夹中应该有一个bash脚本和两个tar存档。运行安装脚本将Qt和QGIS-Deps安装到`/opt/`区域。您需要root权限或对`/opt/Qt`和`/opt/QGIS`有写访问权限。

或者，您可以从以下地址下载并安装MacOS的Qt开源版本：

https://www.qt.io/

版本与安装脚本中引用的版本相同。它必须安装在`/opt/Qt`中。

请注意，QGIS-Deps包尚未签名，因此您可能需要将终端添加到系统偏好设置 -> 安全性与隐私 -> 隐私 -> 开发者工具，或在系统询问时手动接受库的使用。

### 5.1.4. QGIS源代码

将QGIS源代码解压到您选择的工作文件夹。如果您正在从源代码阅读此文档，您已经完成了此操作。

如果您想尝试最新的开发源代码，请转到github QGIS项目页面：

http://github.com/qgis/QGIS

它应该默认为master分支。点击下载按钮并选择下载.tar.gz。双击tarball解压。

*或者*，使用git并通过以下方式克隆仓库：
```bash
git clone git://github.com/qgis/QGIS.git
```

### 5.1.5. 配置构建

CMake支持源外构建，所以我们将为构建过程创建一个'build'目录。OS X使用`${HOME}/Applications`作为标准用户应用文件夹（它给它系统应用文件夹图标）。如果您有正确的权限，您可能想直接构建到您的`/Applications`文件夹中。以下说明假设您正在构建到`${HOME}/Applications`目录。

在终端中cd到之前下载的qgis源文件夹，然后：

```bash
cd ..
mkdir build
cd build

QGIS_DEPS_VERSION=0.9;\
QT_VERSION=5.15.2;\
PATH=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/bin:$PATH;\
cmake \
  -D CMAKE_INSTALL_PREFIX=~/Applications \
  -D CMAKE_BUILD_TYPE=Release \
  -D QGIS_MAC_DEPS_DIR=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage \
  -D CMAKE_PREFIX_PATH=/opt/Qt/${QT_VERSION}/clang_64 \
  ../QGIS
```

注意：不要忘记最后一行的`../QGIS`，它告诉CMake查找源文件。

注意：在屏幕输出上双重检查所有库都从QGIS-Deps `/opt/QGIS`中获取，而不是从系统`/usr/lib`或Homebrew的`/usr/local/`或系统框架`/Library/Frameworks/`中获取。特别检查Proj、GDAL、sqlite3和Python路径。

在初始终端配置后，您可以使用ccmake进行进一步更改：

```bash
cd build
ccmake ../QGIS
```

### 5.1.6. 构建

现在我们可以开始构建过程（记住开始时的并行编译说明，这是使用它的好地方，如果可以的话）：

```bash
make -j [#cpus]
```

现在您可以通过`./output/bin/QGIS.app/Contents/MacOS/QGIS`从构建目录运行QGIS

如果全部构建无错误，您可以安装它：

```bash
make install
```

或者，对于/Applications构建：

```bash
sudo make install
```

要运行已安装的QGIS，您需要保持`/opt/`文件夹中的依赖项。如果您想创建无需这些依赖项即可运行的包，请阅读项目中的文档：

https://github.com/qgis/QGIS-Mac-Packager

## 5.2. 使用vcpkg构建

使用[vcpkg](https://github.com/microsoft/vcpkg/)，您可以使用Qt6开发QGIS。

安装和初始化vcpkg：

```sh
. <(curl https://aka.ms/vcpkg-init.sh -L)
```

获取QGIS源代码：

```sh
git clone git@github.com:qgis/QGIS.git
```

配置：

```sh
cmake -S . \
      -B ./build-arm64-osx \
      -GNinja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DWITH_VCPKG=ON \
      -DBUILD_WITH_QT6=ON \
      -DWITH_QTWEBKIT=OFF \
      -DWITH_BINDINGS=ON \
      -DVCPKG_TARGET_TRIPLET=arm64-osx-dynamic-release \
      -DVCPKG_HOST_TRIPLET=arm64-osx-dynamic-release
```

构建：

```sh
cmake --build ./build-arm64-osx
```

现在可以运行：

```sh
./build-arm64-osx/output/bin/QGIS.app/Contents/MacOS/QGIS
```

# 9. 作者和致谢

本文档由以下人员撰写：

- Tim Sutton
- Radim Blazek  
- Godofredo Contreras
- Otto Dassau
- Marco Hugentobler
- Magnus Homann
- Gary Sherman
- Jürgen E. Fischer

感谢所有为QGIS项目做出贡献的开发者和社区成员。