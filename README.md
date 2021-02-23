# About QGIS

[![Tests](https://github.com/qgis/QGIS/workflows/QGIS%20tests/badge.svg)](https://github.com/qgis/QGIS/actions/workflows/run-tests.yml?query=branch%3Amaster+event%3Apush)
[![Docker Status](https://img.shields.io/docker/automated/qgis/qgis.svg)](https://hub.docker.com/r/qgis/qgis/tags)
[![Build Status](https://dev.azure.com/qgis/QGIS/_apis/build/status/qgis.QGIS?branchName=master)](https://dev.azure.com/qgis/QGIS/_build/latest?definitionId=1&branchName=master)
[![Windows cross build](https://github.com/qgis/QGIS/workflows/MingW64%20Windows%2064bit%20Build/badge.svg)](https://github.com/qgis/QGIS/actions/workflows/mingw64.yml?query=branch%3Amaster+event%3Apush)

*Note: This is an in-progress draft. Formatting, especially images, will be improved once feedback on structure and main text is received.*

QGIS is a full-featured, user-friendly, free-and-open-source (FOSS) geographical 
information system (GIS) that runs on Unix platforms, Windows, and MacOS.

Features include

<img align="right" src="https://www.qgis.org/en/_images/qgisdesktopscreenshot.jpg" width="250" alt="example layer display"><div style="min-height:240;">

**1. Flexible visualization and editing of geospatial data**
* Raster, vector, mesh layers in a range of industry-standard formats
* Local files, webserver, database/PostGIS, tiled...
* Large variety of rendering options; 2D and 3D
* Reprojection between coordinate reference systems (CSR) on the fly
* Visual and numerical and digitizing and editing
* Temporal support

</div><img align="right" src="https://live.staticflickr.com/65535/50870685936_e1ae8c29bd_k.jpg" width="180" height="180" style="margin:0 25px 0 25px;" alt="example image"><div style="min-height:300;">
<!-- Having trouble clipping the image, and also the div min-height seems to be ignored by github --> 
  
**2. Beautiful cartography**
* Fine control over symbology, labeling, legends and additional graphical elements for beautifully rendered maps
* Create and update maps with specified scale, extent, style, and decorations via saved layouts
* Generate a group of maps with same style and layout via atlases
* Flexible output direct to printer, or as image (raster), PDF, or SVG for further customization 
* On-the-fly rendering enhancement via geometry generator symbol layers, e.g. callouts, dimensions, oriented/scaled markers

</div><img align="right" src="https://docs.qgis.org/3.16/en/_images/models_model.png" width="250" alt="example modeler"><div style="min-height:180;">

**3. Advanced GIS analysis**
* Geospatial database engine (filters, joins, relations, forms, etc.), as close to datasource- and format-independent as possible 
* 150+ geoprocessing algorithms
* Immediate visualization of geospatial query and geoprocessing results
* Graphical modeler and scripting
* Access to additional algorithms via GDAL, GRASS, SAGA, etc.

</div><img align="right" src="https://docs.qgis.org/3.16/en/_images/python_console_editor.png" width="250" alt="example image"><div style="min-height:220;">

**4. Wide customization**
* Fully customizable user interface
* Powerful expression engine, plus scripting via Python
* Rich ecosystem of plugins, data connectors, advanced analysis and charting tools, in-the-field data capture
* Python and C++ API for standalone applications

</div><img align="right" src="https://docs.qgis.org/3.16/en/_images/server_wfs3_feature.png" width="250" alt="server image"><div style="min-height:160;">

**5. QGIS server**
* Headless map server, running on Windows, Mac OSX, Linux or in a docker container, that shares the same code base as QGIS
* Industry-standard protocols (WMS, WFS, WFS3/OGC API for Features and WCS) allow plug-n-play with any software stack
* Works with any web server (Apache, nginx, etc) or standalone
* All QGIS beautiful cartography is supported. Best in class support for printing
* Fully customizable, Python scripting support

</div>

## Under the hood

QGIS is developed using the [Qt toolkit](https://qt.io) and C++, since 2002, and has a pleasing, easy to use graphical
user interface with multilingual support. It is maintained by an active developer team and supported by vibrant 
community of GIS professionals and enthusiasts as well as geospatial data publishers and endusers. Releases follow 
a time-based schedule, with a new release every four months, interim bug-fix releases monthly, and a stable long-term release (LTR) once a year.

QGIS is released under the GNU Public License (GPL) Version 2 or above.
Developing QGIS under this license means that you can (if you want to) inspect
and modify the source code and guarantees that you, our happy user will always
have access to a GIS program that is free of cost and can be freely
modified.

QGIS is part of the Open-Source Geospatial Foundation ([OSGeo](https://www.osgeo.org/)), offering a range of complementary open-source GIS software projects.

## Supported formats

Supported raster formats include GeoTIFF, GRASS, ArcInfo binary and ASCII grids, ERDAS Imagine SDTS, WMS, WCS, PostgreSQL/PostGIS, MBTiles, and [other GDAL supported formats](https://gdal.org/drivers/raster/index.html).

Supported vector formats include GeoPackage, ESRI Shapefiles, GRASS, SpatiaLite, PostgreSQL/PostGIS, MSSQL, Oracle, WFS, and [other OGR supported formats](http://www.gdal.org/ogr_formats.html).

Supported mesh formats include NetCDF, GRIB, 2DM, and [other MDAL supported formats](https://github.com/lutraconsulting/MDAL#supported-formats).

## Installing and using QGIS

Precompiled binaries for QGIS are available at [the QGIS.org download page](https://www.qgis.org/en/site/forusers/download.html).
Please follow the installation instructions carefully.

A range of 
[documentation](https://qgis.org/en/docs/index.html) is available, including a full manual as well as a gentle introduction to GIS.

The [building guide](INSTALL.md) can be used to get started with building QGIS from source.

For installation of QGIS Server, see its [getting started documentation](https://docs.qgis.org/testing/en/docs/server_manual/getting_started.html).

Please **help us by submitting bug reports** using the [QGIS bug tracker](https://github.com/qgis/QGIS/issues/).

## Support
You can get support in the following ways:

 -  Using the QGIS community site at https://qgis.org
 -  Joining the [qgis-users mailing list](https://lists.osgeo.org/mailman/listinfo/qgis-user)
 -  Chatting with us real-time.
    Please wait around for a response to your question as many folks
    on the channel are doing other things and it may take a while for
    them to notice your question.
    The following paths all take you to the same chat room:
     - Using an IRC client and joining the [#qgis](http://webchat.freenode.net/?channels=#qgis) channel on irc.freenode.net.
     - Using a Matrix client and joining the [#qgis:matrix.org](http://matrix.to/#/#qgis:matrix.org) room.
     - Using [Gitter](https://gitter.im/qgis/QGIS?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) chat.
 - At the [GIS stackexchange](https://gis.stackexchange.com/) or [r/QGIS reddit](https://www.reddit.com/r/QGIS/), which are not maintained by the QGIS team, but where the QGIS and broader GIS community provides lots of advice.

## Contribute

QGIS is on GitHub at https://github.com/qgis/QGIS. If you wish to contribute
patches you can [fork the project](https://help.github.com/forking/), make your changes, commit to your
repository, and then [create a pull request](https://help.github.com/articles/creating-a-pull-request-from-a-fork/). The development team can then review your contribution and commit it upstream as appropriate.

If you commit a new feature, add [FEATURE] to your commit message AND give a clear description of the new feature. A webhook will automatically create an issue on the QGIS-Documentation repo to tell people to write documentation about it.

If you are not a developer, there are many other possibilities which do not require programming skills to help QGIS to evolve. Check our [project homepage for more information](http://qgis.org/en/site/getinvolved/index.html).
