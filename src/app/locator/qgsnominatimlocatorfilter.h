/***************************************************************************
                         qgsnominatimlocatorfilters.h
                         --------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNOMINATIMLOCATORFILTERS_H
#define QGSNOMINATIMLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgsgeocoderlocatorfilter.h"


class APP_EXPORT QgsNominatimLocatorFilter : public QgsGeocoderLocatorFilter
{
    Q_OBJECT

  public:
    QgsNominatimLocatorFilter( QgsGeocoderInterface *geocoder, QgsMapCanvas *canvas );
    QgsNominatimLocatorFilter *clone() const override SIP_FACTORY;

    void triggerResult( const QgsLocatorResult &result ) override;
    bool hasConfigWidget() const override { return true; }
    void openConfigWidget( QWidget *parent ) override;
};

#endif // QGSNOMINATIMLOCATORFILTERS_H
