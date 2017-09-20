/***************************************************************************
    qgsappbrowserproviders.cpp
    ---------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappbrowserproviders.h"
#include "qgisapp.h"

//
// QgsQlrDataItem
//

QgsQlrDataItem::QgsQlrDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsLayerItem( parent, name, path, path, QgsLayerItem::NoType, QStringLiteral( "qlr" ) )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( ":/images/icons/qgis-icon-16x16.png" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsQlrDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsQlrDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "qlr" );
  u.name = name();
  u.uri = path();
  return u;
}

//
// QgsQlrDataItemProvider
//

QString QgsQlrDataItemProvider::name()
{
  return QStringLiteral( "QLR" );
}

int QgsQlrDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsQlrDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QStringLiteral( "qlr" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsQlrDataItem( parentItem, fileInfo.fileName(), path );
  }
  return nullptr;
}

QString QgsQlrDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "qlr" );
}

void QgsQlrDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->openLayerDefinition( path );
}
