/***************************************************************************
    qgsmbtilesvectortileguiprovider.cpp
     --------------------------------------
    Date                 : March 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmbtilesvectortileguiprovider.h"
///@cond PRIVATE

#include <QList>
#include <QAction>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>

#include "qgsmaplayer.h"
#include "qgsmbtilesvectortilesourcewidget.h"
#include "qgsproviderregistry.h"
#include "qgsmbtilesvectortiledataprovider.h"


//
// QgsMbtilesVectorTileGuiProviderMetadata
//

QgsMbtilesVectorTileSourceWidgetProvider::QgsMbtilesVectorTileSourceWidgetProvider()
{
}

QString QgsMbtilesVectorTileSourceWidgetProvider::providerKey() const
{
  return QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY;
}

bool QgsMbtilesVectorTileSourceWidgetProvider::canHandleLayer( QgsMapLayer *layer ) const
{
  if ( layer->providerType() != QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY )
    return false;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(
    QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY,
    layer->source()
  );
  if ( parts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
    return false;

  return true;
}

QgsProviderSourceWidget *QgsMbtilesVectorTileSourceWidgetProvider::createWidget( QgsMapLayer *layer, QWidget *parent )
{
  if ( layer->providerType() != QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY )
    return nullptr;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(
    QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY,
    layer->source()
  );

  if ( parts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
    return nullptr;

  return new QgsMbtilesVectorTileSourceWidget( parent );
}


//
// QgsMbtilesVectorTileGuiProviderMetadata
//
QgsMbtilesVectorTileGuiProviderMetadata::QgsMbtilesVectorTileGuiProviderMetadata()
  : QgsProviderGuiMetadata( QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY )
{
}

QList<QgsProviderSourceWidgetProvider *> QgsMbtilesVectorTileGuiProviderMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsMbtilesVectorTileSourceWidgetProvider();
  return providers;
}

///@endcond
