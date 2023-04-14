/***************************************************************************
  qgsnominatimgeocoder.h
  ---------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNOMINATIMGEOCODER_H
#define QGSNOMINATIMGEOCODER_H

#include "qgis_core.h"
#include "qgsgeocoder.h"

#include <QMutex>

/**
 * \ingroup core
 * \brief A geocoder which uses the Nominatim geocoding API to retrieve results.
 *
 * This geocoder utilizes the Nominatim geocoding API in order to geocode
 * strings.
 *
 * \warning The user is responsible for respecting the usage policy when
 * using the default OpenStreetMap-run server.
 *
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsNominatimGeocoder : public QgsGeocoderInterface
{

  public:

    /**
     * Constructor for QgsNominatimGeocoder.
     *
     * Optionally, \a countryCodes can be specified to restrict results to one or more countries. The codes
     * must be in  ISO 3166-1alpha2 code and comma-separated.
     *
     * The optional \a endpoint argument can be used to specify a non-default endpoint to use for request.
     */
    QgsNominatimGeocoder( const QString &countryCodes = QString(), const QString &endpoint = QString() );

    Flags flags() const override;
    QgsFields appendedFields() const override;
    Qgis::WkbType wkbType() const override;
    QList< QgsGeocoderResult > geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback *feedback = nullptr ) const override;

    /**
     * Returns the URL generated for geocoding the specified \a address.
     */
    QUrl requestUrl( const QString &address, const QgsRectangle &bounds = QgsRectangle() ) const;

    /**
     * Converts a JSON result returned from the Nominatim service to a geocoder result object.
     */
    QgsGeocoderResult jsonToResult( const QVariantMap &json ) const;

    /**
     * Returns the API endpoint used for requests.
     *
     * \see setEndpoint()
     */
    QString endpoint() const;

    /**
     * Sets a specific API \a endpoint to use for requests. This is for internal testing purposes only.
     *
     * \see endpoint()
     */
    void setEndpoint( const QString &endpoint );

    /**
     * Returns the number of requests per seconds to the endpoint.
     *
     * \see setRequestsPerSecond()
     */
    double requestsPerSecond() const { return mRequestsPerSecond; }

    /**
     * Sets the \a number of request per seconds to the endpoint.
     *
     * \see requestsPerSecond()
     * \warning Setting this to a value > 1 violates the nomatim terms of service. Only change this value if you are using a self-hosted nomatim service.
     */
    void setRequestsPerSecond( double number ) { mRequestsPerSecond = number; }

    /**
     * Returns the optional region bias which will be used to prioritize results in a certain region.
     *
     * \see setCountryCodes()
     */
    QString countryCodes() const;

    /**
     * Sets the optional \a region bias which will be used to prioritize results in a certain region.
     *
     * The \a region argument must be set to a two letter country code top-level domain value,
     * e.g. "gb" for Great Britain.
     *
     * \see countryCodes()
     */
    void setCountryCodes( const QString &countryCodes );

  private:

    QString mCountryCodes;
    QString mEndpoint;
    double mRequestsPerSecond = 1;

    static QMutex sMutex;

    static qint64 sLastRequestTimestamp;

};

#endif // QGSNOMINATIMGEOCODER_H
