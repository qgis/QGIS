/***************************************************************************
    qgsowsprovider.cpp  -  OWS meta provider for WMS,WFS,WCS in browser
                         -------------------
    begin                : 4/2012
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsowsprovider.h"

#include "qgslogger.h"
#include "qgsconfig.h"
#include "qgsowsdataitems.h"

#include <QString>

static QString PROVIDER_KEY = QStringLiteral( "ows" );
static QString PROVIDER_DESCRIPTION = QStringLiteral( "OWS meta provider" );

QgsOwsProvider::QgsOwsProvider( const QString &uri, const ProviderOptions &options )
  : QgsDataProvider( uri, options )
{
}

QString QgsOwsProvider::name() const
{
  return PROVIDER_KEY;
}

QString QgsOwsProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}


QgsOwsProviderMetadata::QgsOwsProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QgsOwsProvider *QgsOwsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsOwsProvider( uri, options );
}

QList<QgsDataItemProvider *> QgsOwsProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsOwsDataItemProvider;
  return providers;
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsOwsProviderMetadata();
}

