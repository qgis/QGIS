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

#include "qgsapplication.h"
#include "qgsgrassrasterprovider.h"
#include "qgsprovidermetadata.h"

#include <QIcon>
#include <QString>

static const QString PROVIDER_KEY = u"grassraster"_s;
static const QString PROVIDER_DESCRIPTION = u"GRASS %1 raster provider"_s.arg( GRASS_VERSION_MAJOR );

class QgsGrassRasterProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsGrassRasterProviderMetadata()
      : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION ) {}
    QgsGrassRasterProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override
    {
      Q_UNUSED( options );
      Q_UNUSED( flags );
      return new QgsGrassRasterProvider( uri );
    }
    QList<Qgis::LayerType> supportedLayerTypes() const override
    {
      return { Qgis::LayerType::Raster };
    }
    QIcon icon() const override
    {
      return QgsApplication::getThemeIcon( u"providerGrass.svg"_s );
    }
};


QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGrassRasterProviderMetadata();
}
