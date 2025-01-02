<img src="images/README-md/main_logo.png" width="300">

[![🧪 QGIS tests](https://github.com/qgis/QGIS/actions/workflows/run-tests.yml/badge.svg)](https://github.com/qgis/QGIS/actions/workflows/run-tests.yml?query=branch%3Amaster+event%3Apush)
[![Docker Status](https://img.shields.io/docker/automated/qgis/qgis.svg)](https://hub.docker.com/r/qgis/qgis/tags)
[![Build Status](https://dev.azure.com/qgis/QGIS/_apis/build/status/qgis.QGIS?branchName=master)](https://dev.azure.com/qgis/QGIS/_build/latest?definitionId=1&branchName=master)
[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/qgis/QGIS/badge)](https://securityscorecards.dev/viewer/?uri=github.com/qgis/QGIS)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/1581/badge)](https://www.bestpractices.dev/projects/1581)
[![🪟 MingW64 Windows 64bit Build](https://github.com/qgis/QGIS/actions/workflows/mingw64.yml/badge.svg)](https://github.com/qgis/QGIS/actions/workflows/mingw64.yml?query=branch%3Amaster+event%3Apush)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5869837.svg)](https://doi.org/10.5281/zenodo.5869837)

QGIS is a full-featured, user-friendly, free-and-open-source (FOSS) geographical information system (GIS) that runs on Unix platforms, Windows, and MacOS.

<!-- TOC generated with https://freelance-tech-writer.github.io/table-of-contents-generator/index.html -->

- [Features](#features)
  - [1. Flexible and powerful spatial data management](#1-flexible-and-powerful-spatial-data-management)
  - [2. Beautiful cartography](#2-beautiful-cartography)
  - [3. Advanced and robust geospatial analysis](#3-advanced-and-robust-geospatial-analysis)
  - [4. Powerful customization and extensibility](#4-powerful-customization-and-extensibility)
  - [5. QGIS Server](#5-qgis-server)
- [Under the hood](#under-the-hood)
  - [Versions and release cycle](#versions-and-release-cycle)
  - [Free and Open Source](#free-and-open-source)
- [Installing and using QGIS](#installing-and-using-qgis)
  - [Documentation](#documentation)
  - [Help and support channels](#help-and-support-channels)
- [Get involved with the community](#get-involved-with-the-community)

## Features

### 1. Flexible and powerful spatial data management

- Support for raster, vector, mesh, and point cloud data in a range of industry-standard formats
    - *Raster formats include*: GeoPackage, GeoTIFF, GRASS, ArcInfo binary and ASCII grids, ERDAS Imagine SDTS, WMS, WCS, PostgreSQL/PostGIS, and [other GDAL supported formats](https://gdal.org/drivers/raster/index.html).
    - *Vector formats include*: GeoPackage, ESRI shapefiles, GRASS, SpatiaLite, PostgreSQL/PostGIS, MSSQL, Oracle, WFS, Vector Tiles and [other OGR supported formats](https://www.gdal.org/ogr_formats.html).
    - *Mesh formats include*: NetCDF, GRIB, 2DM, and [other MDAL supported formats](https://github.com/lutraconsulting/MDAL#supported-formats).
    - *Point-cloud format*: LAS/LAZ and EPT datasets.
- Data abstraction framework, with local files, spatial databases (PostGIS, SpatiaLite, SQL Server, Oracle, SAP HANA), and web services (WMS, WCS, WFS, ArcGIS REST) all accessed through a unified data model and browser interface, and as flexible layers in user-created projects
- Spatial data creation via visual and numerical digitizing and editing, as well as georeferencing of raster and vector data
- On-the-fly reprojection between coordinate reference systems (CRS)
- Nominatim (OpenStreetMap) geocoder access
- Temporal support

*Example: Temporal animation*

![Example: Temporal animation](images/README-md/icebergs.gif "Temporal animation")

*Example: 3D map view*

![Example: 3D map view](https://docs.qgis.org/latest/en/_images/3dmapview.png "3D map view")

### 2. Beautiful cartography
- Large variety of rendering options in 2D and 3D
- Fine control over symbology, labeling, legends and additional graphical elements for beautifully rendered maps
- Respect for embedded styling in many spatial data sources (e.g. KML and TAB files, Mapbox-GL styled vector tiles)
- In particular, near-complete replication (and significant extension) of symbology options that are available in proprietary software by ESRI
- Advanced styling using data-defined overrides, blending modes, and draw effects
- 500+ built-in color ramps (cpt-city, ColorBrewer, etc.)
- Create and update maps with specified scale, extent, style, and decorations via saved layouts
- Generate multiple maps (and reports) automatically using QGIS Atlas and QGIS Reports
- Display and export elevation profile plots with flexible symbology
- Flexible output direct to printer, or as image (raster), PDF, or SVG for further customization
- On-the-fly rendering enhancements using geometry generators (e.g. create and style new geometries from existing features)
- Preview modes for inclusive map making (e.g. monochrome, color blindness)

*[Example: Map of Bogota, Colombia in the style of Starry Starry Night, by Andrés Felipe Lancheros Sánchez](https://flic.kr/p/2jFfGJP)*

![Map of Bogota, Colombia in the style of Starry Starry Night](https://live.staticflickr.com/65535/50327326323_3da28f0d86_b.jpg "Map of Bogota, Colombia in the style of Starry Starry Night")

For more maps created with QGIS, visit the [QGIS Map Showcase Flickr Group](https://www.flickr.com/groups/2244553@N22/pool/with/50355460063/).

![QGIS Map Showcase](images/README-md/qgis_map_showcase.png "QGIS Map Showcase")

### 3. Advanced and robust geospatial analysis
- Powerful processing framework with 200+ native processing algorithms
- Access to 1000+ processing algorithms via providers such as GDAL, SAGA, GRASS, OrfeoToolbox, as well as custom models and processing scripts
- Geospatial database engine (filters, joins, relations, forms, etc.), as close to datasource- and format-independent as possible
- Immediate visualization of geospatial query and geoprocessing results
- Model designer and batch processing

*Example: Travel isochrones*

![Example: Travel isochrones](images/README-md/network_analysis_2.png "Travel isochrones")

*Example: Model designer*

![Example: model designer](https://docs.qgis.org/latest/en/_images/models_model.png "Model designer")

### 4. Powerful customization and extensibility

- Fully customizable user experience, including user interface and application settings that cater to power-users and beginners alike
- Rich [expression engine](https://docs.qgis.org/testing/en/docs/user_manual/working_with_vector/expression.html) for maximum flexibility in visualization and processing
- Broad and varied [plugin ecosystem](https://plugins.qgis.org/) that includes data connectors, digitizing aids, advanced analysis and charting tools,
in-the-field data capture, conversion of ESRI style files, etc.
- Style manager for creating, storing, and managing styles
- [QGIS style hub](https://plugins.qgis.org/styles/) for easy sharing of styles
- Python and C++ API for standalone (headless) applications as well as in-application comprehensive scripting (PyQGIS)

*Example: Style manager*

![Example: Style manager](https://docs.qgis.org/latest/en/_images/stylemanager.png "Style Manager")

*Example: Plugins*

![Example: Plugins](images/README-md/plugins_1.png "Plugins")

<!-- Kill this one for now, since the example provided is Python2 not 3
Example: Python console

![Example: Python console](https://docs.qgis.org/latest/en/_images/python_console_editor.png "Python console")
-->

### 5. QGIS Server

Headless map server -- running on Linux, macOS, Windows, or in a docker container -- that shares the same code base as QGIS.

- Industry-standard protocols (WMS, WFS, WFS3/OGC API for Features and WCS) allow plug-n-play with any software stack
- Works with any web server (Apache, nginx, etc) or standalone
- All beautiful QGIS cartography is supported with best-in-class support for printing
- Fully customizable with Python scripting support

*Example: QGIS server WMS response*

![Example: QGIS Server response to a WMS request](https://docs.qgis.org/latest/en/_images/server_selection_parameter.png "QGIS Server response to a WMS request")

*Example: QGIS server WFS response*

![Example: QGIS Server response to a WFS Feature request](https://docs.qgis.org/latest/en/_images/server_wfs3_feature.png "QGIS Server response to a WFS Feature request")

## Under the hood

QGIS is developed using the [Qt toolkit](https://qt.io) and C++, since 2002, and has a pleasing, easy to use graphical
user interface with multilingual support. It is maintained by an active developer team and supported by vibrant
community of GIS professionals and enthusiasts as well as geospatial data publishers and end-users.

### Versions and release cycle

QGIS development and releases follow a [time based schedule/roadmap](https://www.qgis.org/en/site/getinvolved/development/roadmap.html). There are three main branches of QGIS that users can install. These are the **Long Term Release (LTR)** branch, the **Latest Release (LR)** branch, and the **Development (Nightly)** branch.

Every month, there is a **Point Release** that provides bug-fixes to the LTR and LR.

### Free and Open Source

QGIS is released under the GNU Public License (GPL) Version 2 or any later version.
Developing QGIS under this license means that you can (if you want to) inspect
and modify the source code and guarantees that you, our happy user will always
have access to a GIS program that is free of cost and can be freely
modified.

QGIS is part of the Open-Source Geospatial Foundation ([OSGeo](https://www.osgeo.org/)), offering a range of complementary open-source GIS software projects.

## Installing and using QGIS

Precompiled binaries for QGIS are available at [the QGIS.org download page](https://www.qgis.org/en/site/forusers/download.html).
Please follow the installation instructions carefully.

The [building guide](INSTALL.md) can be used to get started with building QGIS from source.

For installation of QGIS Server, see its [getting started documentation](https://docs.qgis.org/testing/en/docs/server_manual/getting_started.html).

### Documentation

A range of [documentation](https://qgis.org/resources/hub/#documentation) is available. This includes:

- [Training Manual](https://docs.qgis.org/latest/en/docs/training_manual/index.html)
- [QGIS User Guide](https://docs.qgis.org/latest/en/docs/user_manual/index.html)
- [QGIS Server Guide](https://docs.qgis.org/latest/en/docs/server_manual/index.html)
- [Visual Changelog](https://qgis.org/project/visual-changelogs/)
- [Documentation Guidelines](https://docs.qgis.org/latest/en/docs/documentation_guidelines/index.html)
- [QGIS Python (PyQGIS) Cookbook](https://docs.qgis.org/latest/en/docs/pyqgis_developer_cookbook/index.html)
- [QGIS Python (PyQGIS) API](https://qgis.org/pyqgis/)
- [QGIS C++ API](https://qgis.org/api/)
- [Developers Guide](https://docs.qgis.org/latest/en/docs/developers_guide/index.html)

### Help and support channels

There are several channels where you can find help and support for QGIS:

- Using the [QGIS community site](https://qgis.org)
- Joining the [qgis-users mailing list](https://lists.osgeo.org/mailman/listinfo/qgis-user)
- Chatting with other users real-time. *Please wait around for a response to your question as many folks on the channel are doing other things and it may take a while for them to notice your question. The following paths all take you to the same chat room:*
    - Using an IRC client and joining the
      [#qgis](https://web.libera.chat/?channels=#qgis) channel on irc.libera.chat.
    - Using a Matrix client and joining the [#qgis:osgeo.org](https://matrix.to/#/#qgis:osgeo.org) room.
 - At the [GIS stackexchange](https://gis.stackexchange.com/) or [r/QGIS reddit](https://www.reddit.com/r/QGIS/), which are not maintained by the QGIS team, but where the QGIS and broader GIS community provides lots of advice
- [Other support channels](https://qgis.org/resources/support/)

## Get involved with the community

[Contribution guidelines for this project](CONTRIBUTING.md)
