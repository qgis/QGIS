/***************************************************************************
                    qgsgrassrasterprovidermodule.cpp
                     -------------------
    begin                : October, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>

#include "qgsgrassrasterprovider.h"
#include "qgsprovidermetadata.h"

static const QString PROVIDER_KEY = QStringLiteral( "grassraster" );
static const QString PROVIDER_DESCRIPTION = QStringLiteral( "GRASS %1 raster provider" ).arg( GRASS_VERSION_MAJOR );

class QgsGrassRasterProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsGrassRasterProviderMetadata(): QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION ) {}
    QgsGrassRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options ) override
    {
      Q_UNUSED( options );
      return new QgsGrassRasterProvider( uri );
    }
};

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGrassRasterProviderMetadata();
}

