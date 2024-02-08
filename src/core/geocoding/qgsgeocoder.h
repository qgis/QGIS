/***************************************************************************
  qgsgeocoder.h
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOCODER_H
#define QGSGEOCODER_H

#include "qgis_core.h"
#include "qgsgeocoderresult.h"
#include "qgsgeometry.h"
#include "qgsfields.h"

class QgsFeature;
class QgsGeocoderContext;

/**
 * \ingroup core
 * \brief Interface for geocoders.
 *
 * QgsGeocoderInterface implementations are able to take either a QgsFeature or a free-form string
 * and calculate the corresponding geometry of the feature.
 *
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsGeocoderInterface
{

  public:

    //! Capability flags for the geocoder.
    enum class Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      GeocodesStrings = 1 << 0, //!< Can geocode string input values
      GeocodesFeatures = 1 << 1, //!< Can geocode QgsFeature input values
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    virtual ~QgsGeocoderInterface() = default;

    /**
     * Returns the geocoder's capability flags.
     */
    virtual Flags flags() const = 0;

    /**
     * Geocodes a \a feature.
     *
     * If implemented by the geocoder (i.e. flags() returns the QgsGeocoderInterface::Flag::GeocodesFeatures flag), a list of matching results will be returned.
     *
     * The optional \a feedback argument can be used to provider cancellation support.
     */
    virtual QList< QgsGeocoderResult > geocodeFeature( const QgsFeature &feature, const QgsGeocoderContext &context, QgsFeedback *feedback = nullptr ) const;

    /**
     * Returns a set of newly created fields which will be appended to existing features during the geocode
     * operation.
     *
     * These fields will include any extra content returned by the geocoder, such as fields for accuracy of the
     * match or correct attribute values.
     */
    virtual QgsFields appendedFields() const;

    /**
     * Returns the WKB type of geometries returned by the geocoder.
     *
     * If this is not known in advance then QgsWkbTypes::Unknown should be returned (e.g.
     * in the case that a geocoder may return different geometry types depending on the
     * quality of the match).
     */
    virtual Qgis::WkbType wkbType() const;

    /**
     * Geocodes a \a string.
     *
     * If implemented by the geocoder (i.e. flags() returns the QgsGeocoderInterface::Flag::GeocodesStrings flag), a list of matching results will be returned.
     *
     * The optional \a feedback argument can be used to provider cancellation support.
     */
    virtual QList< QgsGeocoderResult > geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback *feedback = nullptr ) const;

};

#endif // QGSGEOCODER_H
