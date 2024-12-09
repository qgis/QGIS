/***************************************************************************
  qgsgeocoderlocatorfilter.h
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

#ifndef QGSGEOCODERLOCATORFILTER_H
#define QGSGEOCODERLOCATORFILTER_H

#include "qgis_gui.h"
#include "qgsabstractgeocoderlocatorfilter.h"

class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief An adapter class which implements a locator filter populated from a QgsGeocoderInterface.
 *
 * This class implements the required logic to bridge a class which implements the
 * QgsGeocoderInterface interface to a QgsLocatorFilter. It allows easy creation of a locator
 * filter from a geocoder.
 *
 * \since QGIS 3.18
*/
class GUI_EXPORT QgsGeocoderLocatorFilter : public QgsAbstractGeocoderLocatorFilter
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsGeocoderLocatorFilter.
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
    QgsGeocoderLocatorFilter( const QString &name, const QString &displayName, const QString &prefix, QgsGeocoderInterface *geocoder, QgsMapCanvas *canvas, const QgsRectangle &boundingBox = QgsRectangle() );

    QgsLocatorFilter *clone() const override SIP_FACTORY;

  private:
    void handleGeocodeResult( const QgsGeocoderResult &result ) override;

    QgsMapCanvas *mCanvas = nullptr;
};

#endif // QGSGEOCODERLOCATORFILTER_H
