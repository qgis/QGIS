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
#include <QDesktopServices>

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

//
// QgsQptDataItemProvider
//

QString QgsQptDataItemProvider::name()
{
  return QStringLiteral( "QPT" );
}

int QgsQptDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsQptDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QStringLiteral( "qpt" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsQptDataItem( parentItem, fileInfo.fileName(), path );
  }
  return nullptr;
}

//
// QgsQptDropHandler
//

QString QgsQptDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "qpt" );
}

void QgsQptDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->openTemplate( path );
}

bool QgsQptDropHandler::handleFileDrop( const QString &file )
{
  QFileInfo fi( file );
  if ( fi.completeSuffix().compare( QStringLiteral( "qpt" ), Qt::CaseInsensitive ) == 0 )
  {
    QgisApp::instance()->openTemplate( file );
    return true;
  }
  return false;
}

//
// QgsQptDataItem
//

QgsQptDataItem::QgsQptDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( ":/images/icons/qgis-icon-16x16.png" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsQptDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsQptDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "qpt" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsQptDataItem::handleDoubleClick()
{
  QgisApp::instance()->openTemplate( path() );
  return true;
}

QList<QAction *> QgsQptDataItem::actions( QWidget *parent )
{
  QAction *newLayout = new QAction( tr( "New Layout from Template" ), parent );
  connect( newLayout, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->openTemplate( path() );
  } );
  return QList<QAction *>() << newLayout;
}

//
// QgsPyDataItem
//

QgsPyDataItem::QgsPyDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( ":/images/icons/qgis-icon-16x16.png" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsPyDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsPyDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "py" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsPyDataItem::handleDoubleClick()
{
  QgisApp::instance()->runScript( path() );
  return true;
}

QList<QAction *> QgsPyDataItem::actions( QWidget *parent )
{
  QAction *runScript = new QAction( tr( "&Run Script" ), parent );
  connect( runScript, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->runScript( path() );
  } );
  QAction *editScript = new QAction( tr( "Open in External &Editor" ), this );
  connect( editScript, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( path() ) );
  } );
  return QList<QAction *>() << runScript << editScript;
}

//
// QgsPyDataItemProvider
//

QString QgsPyDataItemProvider::name()
{
  return QStringLiteral( "py" );
}

int QgsPyDataItemProvider::capabilities()
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsPyDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QStringLiteral( "py" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsPyDataItem( parentItem, fileInfo.fileName(), path );
  }
  return nullptr;
}

//
// QgsPyDropHandler
//

QString QgsPyDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "py" );
}

void QgsPyDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->runScript( path );
}

bool QgsPyDropHandler::handleFileDrop( const QString &file )
{
  QFileInfo fi( file );
  if ( fi.completeSuffix().compare( QStringLiteral( "py" ), Qt::CaseInsensitive ) == 0 )
  {
    QgisApp::instance()->runScript( file );
    return true;
  }
  return false;
}
