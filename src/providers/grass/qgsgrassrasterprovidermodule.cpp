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
#include <QIcon>

#include "qgsgrassrasterprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsapplication.h"

static const QString PROVIDER_KEY = QStringLiteral( "grassraster" );
static const QString PROVIDER_DESCRIPTION = QStringLiteral( "GRASS %1 raster provider" ).arg( GRASS_VERSION_MAJOR );

class QgsGrassRasterProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsGrassRasterProviderMetadata(): QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION ) {}
    QgsGrassRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override
    {
      Q_UNUSED( options );
      Q_UNUSED( flags );
      return new QgsGrassRasterProvider( uri );
    }
    QList< QgsMapLayerType > supportedLayerTypes() const override
    {
      return { QgsMapLayerType::RasterLayer };
    }
    QIcon icon() const override
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "providerGrass.svg" ) );
    }
};


QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGrassRasterProviderMetadata();
}

