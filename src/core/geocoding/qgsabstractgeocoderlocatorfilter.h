/***************************************************************************
  qgsabstractgeocoderlocatorfilter.h
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

#ifndef QGSABSTRACTGEOCODERLOCATORFILTER_H
#define QGSABSTRACTGEOCODERLOCATORFILTER_H

#include "qgis_core.h"
#include "qgslocatorfilter.h"

class QgsGeocoderInterface;
class QgsGeocoderResult;

/**
 * \ingroup core
 * \brief An abstract base class which implements a locator filter populated from a QgsGeocoderInterface.
 *
 * This base class implements the required logic to bridge a class which implements the
 * QgsGeocoderInterface interface to a QgsLocatorFilter. It allows easy creation of a locator
 * filter from a geocoder.
 *
 * \note This is a low-level API, designed for use by client applications which do not
 * utilize the QGIS gui library. Usually the concrete class QgsGeocoderLocatorFilter from
 * the gui library should be used instead.
 *
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsAbstractGeocoderLocatorFilter : public QgsLocatorFilter SIP_ABSTRACT
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAbstractGeocoderLocatorFilter.
     *
     * The \a name argument specifies a unique name for the filter. This should be
     * an untranslated string identifying the filter.
     *
     * The \a displayName argument must specify a translated, user-friendly name for the filter.
     *
     * The \a prefix argument specifies the prefix character(s) for this filter. Prefixing a search
     * with these characters will restrict the locator search to only include results from this filter.
     * The \a prefix must consist of at least three characters.
     *
     * The \a geocoder must specify an instance of a class which implements the QgsGeocoderInterface
     * interface. Ownership of \a geocoder is not transferred, and the caller must ensure that \a geocoder
     * exists for the lifetime of this filter.
     *
     * The \a boundingBox argument specifies the geographic bounding box, in WGS84, covered by the
     * filter.
     */
    QgsAbstractGeocoderLocatorFilter( const QString &name, const QString &displayName,
                                      const QString &prefix,
                                      QgsGeocoderInterface *geocoder,
                                      const QgsRectangle &boundingBox = QgsRectangle() );

    QString name() const override;
    QString displayName() const override;
    QString prefix() const override;
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

    /**
     * Returns the geocoder attached to the filter.
     */
    QgsGeocoderInterface *geocoder() const;

    /**
     * Returns the WGS84 bounding box attached to the filter.
     */
    const QgsRectangle boundingBox() { return mBoundingBox; }

    /**
     * Converts a locator \a result to a geocoder result.
     *
     * \see geocoderResultToLocatorResult()
     */
    QgsGeocoderResult locatorResultToGeocoderResult( const QgsLocatorResult &result ) const;

    /**
     * Converts a geocoder \a result to a locator result.
     *
     * \see locatorResultToGeocoderResult()
     */
    QgsLocatorResult geocoderResultToLocatorResult( const QgsGeocoderResult &result );

  private:

    /**
     * Called when a geocode \a result was triggered by the filter.
     */
    virtual void handleGeocodeResult( const QgsGeocoderResult &result ) = 0;

    QString mName;
    QString mDisplayName;
    QString mPrefix;
    QgsGeocoderInterface *mGeocoder = nullptr;
    QgsRectangle mBoundingBox;

};

#endif // QGSABSTRACTGEOCODERLOCATORFILTER_H
