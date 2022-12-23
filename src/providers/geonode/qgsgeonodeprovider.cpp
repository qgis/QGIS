/***************************************************************************
                              qgsgeonodeprovider.cpp
                              ----------------------
    begin                : September 2017
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

#include <QList>

#include "qgsgeonodeprovider.h"

static const QString PROVIDER_KEY = QStringLiteral( "geonode" );
static const QString PROVIDER_DESCRIPTION = QStringLiteral( "GeoNode provider" );

QgsGeoNodeProviderMetadata::QgsGeoNodeProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsGeoNodeProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconGeonode.svg" ) );
}

QList<QgsDataItemProvider *> QgsGeoNodeProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsGeoNodeDataItemProvider();
  return providers;
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGeoNodeProviderMetadata();
}
