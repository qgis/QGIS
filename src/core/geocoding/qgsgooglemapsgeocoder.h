/***************************************************************************
  qgsgooglemapsgeocoder.h
  ---------------
  Date                 : November 2020
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

#ifndef QGSGOOGLEMAPSGEOCODER_H
#define QGSGOOGLEMAPSGEOCODER_H

#include "qgis_core.h"
#include "qgsgeocoder.h"

#include <QMutex>

/**
 * \ingroup core
 * \brief A geocoder which uses the Google Map geocoding API to retrieve results.
 *
 * This geocoder utilizes the Google Maps "geocoding" API in order to geocode
 * strings. The Google Maps service is not publicly available, and accordingly
 * an API key must be first obtained from Google and specified when constructing
 * this class.
 *
 * \warning The user is responsible for managing their Google Maps API key, and ensuring
 * that the use of this geocoder does not exceed their usage limits! Excessive use
 * of the Google Maps geocoder API can result in charges being applied to the API key
 * holder.
 *
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsGoogleMapsGeocoder : public QgsGeocoderInterface
{

  public:

    /**
     * Constructor for QgsGoogleMapsGeocoder.
     *
     * The \a apiKey argument must specify a valid Google Maps API key. All use of this
     * geocoder will be associated with the specified key for Google's billing purposes!
     *
     * Optionally, a \a regionBias can be specified to prioritize results in a certain region.
     * The \a regionBias argument must be set to a two letter country code top-level domain value,
     * e.g. "gb" for Great Britain.
     */
    QgsGoogleMapsGeocoder( const QString &apiKey, const QString &regionBias = QString() );

    Flags flags() const override;
    QgsFields appendedFields() const override;
    Qgis::WkbType wkbType() const override;
    QList< QgsGeocoderResult > geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback *feedback = nullptr ) const override;

    /**
     * Returns the URL generated for geocoding the specified \a address.
     */
    QUrl requestUrl( const QString &address, const QgsRectangle &bounds = QgsRectangle() ) const;

    /**
     * Converts a JSON result returned from the Google Maps service to a geocoder result object.
     */
    QgsGeocoderResult jsonToResult( const QVariantMap &json ) const;

    /**
     * Sets a specific API \a endpoint to use for requests. This is for internal testing purposes only.
     */
    void setEndpoint( const QString &endpoint );

    /**
     * Returns the API key which will be used when accessing the Google Maps API.
     *
     * \see setApiKey()
     */
    QString apiKey() const;

    /**
     * Sets the API \a key to use when accessing the Google Maps API.
     *
     * All use of this geocoder will be associated with the specified key for Google's billing purposes!
     *
     * \see apiKey()
     */
    void setApiKey( const QString &key );

    /**
     * Returns the optional region bias which will be used to prioritize results in a certain region.
     *
     * \see setRegion()
     */
    QString region() const;

    /**
     * Sets the optional \a region bias which will be used to prioritize results in a certain region.
     *
     * The \a region argument must be set to a two letter country code top-level domain value,
     * e.g. "gb" for Great Britain.
     *
     * \see region()
     */
    void setRegion( const QString &region );

  private:

    QString mApiKey;
    QString mRegion;
    QString mEndpoint;

    static QReadWriteLock sMutex;

};

#endif // QGSGOOGLEMAPSGEOCODER_H
