/***************************************************************************
    qgsvtpkvectortileguiprovider.cpp
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

#include "qgsvtpkvectortileguiprovider.h"
///@cond PRIVATE

#include <QList>
#include <QAction>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>

#include "qgsmaplayer.h"
#include "qgsvtpkvectortilesourcewidget.h"
#include "qgsproviderregistry.h"
#include "qgsvtpkvectortiledataprovider.h"


//
// QgsVtpkVectorTileSourceWidgetProvider
//

QgsVtpkVectorTileSourceWidgetProvider::QgsVtpkVectorTileSourceWidgetProvider()
{
}

QString QgsVtpkVectorTileSourceWidgetProvider::providerKey() const
{
  return QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY;
}

bool QgsVtpkVectorTileSourceWidgetProvider::canHandleLayer( QgsMapLayer *layer ) const
{
  if ( layer->providerType() != QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY )
    return false;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(
    QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY,
    layer->source()
  );
  if ( parts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
    return false;

  return true;
}

QgsProviderSourceWidget *QgsVtpkVectorTileSourceWidgetProvider::createWidget( QgsMapLayer *layer, QWidget *parent )
{
  if ( layer->providerType() != QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY )
    return nullptr;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(
    QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY,
    layer->source()
  );

  if ( parts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
    return nullptr;

  return new QgsVtpkVectorTileSourceWidget( parent );
}


//
// QgsVtpkVectorTileGuiProviderMetadata
//
QgsVtpkVectorTileGuiProviderMetadata::QgsVtpkVectorTileGuiProviderMetadata()
  : QgsProviderGuiMetadata( QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY )
{
}

QList<QgsProviderSourceWidgetProvider *> QgsVtpkVectorTileGuiProviderMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsVtpkVectorTileSourceWidgetProvider();
  return providers;
}

///@endcond
