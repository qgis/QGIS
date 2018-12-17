# About QGIS

[![Build Status](https://travis-ci.org/qgis/QGIS.svg?branch=master)](https://travis-ci.org/qgis/QGIS)
[![Docker Status](https://img.shields.io/docker/automated/qgis/qgis.svg)](https://cloud.docker.com/app/qgis/repository/docker/qgis/qgis/general)

QGIS is an Open Source Geographic Information System. The project was born in
May of 2002 and was established as a project on SourceForge in June of the same
year. We've worked hard to make GIS software (which is traditionally expensive
commercial software) a viable prospect for anyone with basic access to a
Personal Computer.  QGIS currently runs on most Unix platforms (macOS/OS X included)
and Windows. QGIS is developed using the [Qt toolkit](https://qt.io) and C++.  This
means that QGIS feels snappy to use and has a pleasing, easy to use graphical
user interface.

QGIS aims to be an easy to use GIS, providing common functions and
features. The initial goal was to provide a GIS data viewer. QGIS has
reached that point in its evolution and is being used by many for their
daily GIS data viewing and editing needs. QGIS supports a number of raster
, vector and mesh data formats, with new support easily added using the plugin
architecture.

QGIS is released under the GNU Public License (GPL) Version 2 or above.
Developing QGIS under this license means that you can (if you want to) inspect
and modify the source code and guarantees that you, our happy user will always
have access to a GIS program that is free of cost and can be freely
modified.

## Supported raster formats include:

 * GRASS
 * USGS DEM
 * ArcInfo binary grid
 * ArcInfo ASCII grid
 * ERDAS Imagine
 * SDTS
 * GeoTiff
 * Tiff with world file
 * WMS, WCS

## Supported vector formats include:

 * ESRI Shapefiles
 * PostgreSQL/PostGIS
 * GRASS
 * GeoPackage
 * SpatiaLite
 * [Other OGR supported formats](http://www.gdal.org/ogr_formats.html)
 * MSSQL
 * Oracle
 * WFS

## Supported mesh formats include:

 * NetCDF
 * GRIB
 * 2DM 
 * [Other MDAL supported formats](https://github.com/lutraconsulting/MDAL#supported-formats)

## Note

Please follow the installation instructions carefully.
After extracting the distribution, you can find the HTML version of the
installation document in `qgis/doc/index.html`. The installation document is
also available as PDF in the same directory.

## Help us
Please submit bug reports using the [QGIS bug tracker](https://issues.qgis.org/).
When reporting a bug, either login or, if you don't have a qgis trac, provide
an email address where we can request additional information.

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

## Contribute

QGIS is on GitHub at https://github.com/qgis/QGIS. If you wish to contribute
patches you can [fork the project](https://help.github.com/forking/), make your changes, commit to your
repository, and then [create a pull request](https://help.github.com/articles/creating-a-pull-request-from-a-fork/). The development team can then
review your contribution and commit it upstream as appropriate.
If you commit a new feature, add [FEATURE] to your commit message AND give a clear description of the new feature. A webhook will automatically create an issue on the QGIS-Documentation repo to tell people to write documentation about it.

If you are not a developer, there are many other possibilities which do not require programming skills to help QGIS to evolve. Check our [project homepage for more information](http://qgis.org/en/site/getinvolved/index.html).
