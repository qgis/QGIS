/***************************************************************************
  qgisexiftools.h
  ---------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXIFTOOLS_H
#define QGSEXIFTOOLS_H

#include "qgis_core.h"
#include "qgspointxy.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

/**
 * \brief Contains utilities for working with EXIF tags in images.
 * \ingroup analysis
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsExifTools
{
    Q_GADGET

  public:

    /**
     * Returns a map object containing all exif tags stored in the image at \a imagePath.
     * \since QGIS 3.22
     */
    static QVariantMap readTags( const QString &imagePath );

    /**
     * Returns the value of of an exif tag \a key stored in the image at \a imagePath.
     * \since QGIS 3.22
     */
    static QVariant readTag( const QString &imagePath, const QString &key );

    /**
     * Returns the geotagged coordinate stored in the image at \a imagePath.
     *
     * If a geotag was found, \a ok will be set to TRUE.
     *
     * If the image contains an elevation tag then the returned point will contain
     * the elevation as a z value.
     *
     * \see geoTagImage()
     */
    static QgsPoint getGeoTag( const QString &imagePath, bool &ok SIP_OUT );

    /**
     * Returns TRUE if the image at \a imagePath contains a valid geotag.
     *
     * \see getGeoTag()
     */
    Q_INVOKABLE static bool hasGeoTag( const QString &imagePath );

    /**
     * \brief Extended image geotag details.
     * \ingroup analysis
     * \since QGIS 3.6
     */
    class GeoTagDetails
    {
      public:

        GeoTagDetails()
          : elevation( std::numeric_limits< double >::quiet_NaN() )
        {
        }

        /**
         * GPS elevation, or NaN if elevation is not available.
         */
        double elevation = 0;
    };

    /**
     * Writes geotags to the image at \a imagePath.
     *
     * The \a location argument indicates the GPS location to write to the image, as a WGS84 latitude/longitude coordinate.
     *
     * If desired, extended GPS tags (such as elevation) can be specified via the \a details argument.
     *
     * Returns TRUE if writing was successful.
     *
     * \see getGeoTag()
     */
    static bool geoTagImage( const QString &imagePath, const QgsPointXY &location, const GeoTagDetails &details = QgsExifTools::GeoTagDetails() );

    /**
     * Writes a tag to the image at imagePath.
     * \param imagePath the image path
     * \param tag the exif tag name
     * \param value the exif tag value
     * \returns TRUE if writing was successful.
     * \since QGIS 3.30
     */
    static bool tagImage( const QString &imagePath, const QString &tag, const QVariant &value );

};

#endif // QGSEXIFTOOLS_H
