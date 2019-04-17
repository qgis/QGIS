/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsexiftools.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsExifTools
{
%Docstring
Contains utilities for working with EXIF tags in images.

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsexiftools.h"
%End
  public:


    static QgsPoint getGeoTag( const QString &imagePath, bool &ok /Out/ );
%Docstring
Returns the geotagged coordinate stored in the image at ``imagePath``.

If a geotag was found, ``ok`` will be set to ``True``.

If the image contains an elevation tag then the returned point will contain
the elevation as a z value.

.. seealso:: :py:func:`geoTagImage`
%End

    class GeoTagDetails
{
%Docstring
Extended image geotag details.

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsexiftools.h"
%End
      public:

        GeoTagDetails();

        double elevation;
    };

    static bool geoTagImage( const QString &imagePath, const QgsPointXY &location, const GeoTagDetails &details = QgsExifTools::GeoTagDetails() );
%Docstring
Writes geotags to the image at ``imagePath``.

The ``location`` argument indicates the GPS location to write to the image, as a WGS84 latitude/longitude coordinate.

If desired, extended GPS tags (such as elevation) can be specified via the ``details`` argument.

Returns ``True`` if writing was successful.

.. seealso:: :py:func:`getGeoTag`
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsexiftools.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
