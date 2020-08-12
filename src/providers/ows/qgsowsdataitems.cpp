/***************************************************************************
    qgsowsdataitems.cpp
    ---------------------
    begin                : May 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsproviderregistry.h"
#include "qgsowsdataitems.h"
#include "qgsowsprovider.h"
#include "qgslogger.h"
#include "qgsowsconnection.h"
#include "qgsdataitemprovider.h"

#include "qgsapplication.h"

#include <QFileInfo>

// ---------------------------------------------------------------------------
QgsOWSConnectionItem::QgsOWSConnectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "OWS" ) )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsOWSConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QHash<QgsDataItem *, QString> serviceItems; // service/provider key

  int layerCount = 0;
  // Try to open with WMS,WFS,WCS
  Q_FOREACH ( const QString &key, QStringList() << "wms" << "WFS" << "wcs" )
  {
    QgsDebugMsg( "Add connection for provider " + key );
    const QList<QgsDataItemProvider *> providerList = QgsProviderRegistry::instance()->dataItemProviders( key );
    if ( providerList.isEmpty() )
    {
      QgsDebugMsg( key + " does not have dataItemProviders" );
      continue;
    }

    QString path = key.toLower() + ":/" + name();
    QgsDebugMsg( "path = " + path );

    QVector<QgsDataItem *> items;
    for ( QgsDataItemProvider *pr : providerList )
    {
      if ( !pr->name().startsWith( key ) )
        continue;

      items = pr->createDataItems( path, this );
      if ( !items.isEmpty() )
      {
        break;
      }
    }

    if ( items.isEmpty() )
    {
      QgsDebugMsg( key + " does not have any data items" );
      continue;
    }

    for ( QgsDataItem *item : qgis::as_const( items ) )
    {
      item->populate( true ); // populate in foreground - this is already run in a thread

      layerCount += item->rowCount();
      if ( item->rowCount() > 0 )
      {
        QgsDebugMsg( "Add new item : " + item->name() );
        serviceItems.insert( item, key );
      }
      else
      {
        //delete item;
      }
    }
  }

  for ( auto it = serviceItems.constBegin(); it != serviceItems.constEnd(); ++it )
  {
    QgsDataItem *item = it.key();
    QgsDebugMsg( QStringLiteral( "serviceItems.size = %1 layerCount = %2 rowCount = %3" ).arg( serviceItems.size() ).arg( layerCount ).arg( item->rowCount() ) );
    QString providerKey = it.value();
    if ( serviceItems.size() == 1 || layerCount <= 30 || item->rowCount() <= 10 )
    {
      // Add layers directly to OWS connection
      const auto constChildren = item->children();
      for ( QgsDataItem *subItem : constChildren )
      {
        item->removeChildItem( subItem );
        subItem->setParent( this );
        replacePath( subItem, providerKey.toLower() + ":/", QStringLiteral( "ows:/" ) );
        children.append( subItem );
      }
      delete item;
    }
    else // Add service
    {
      replacePath( item, item->path(), path() + '/' + providerKey.toLower() );
      children.append( item );
    }
  }

  return children;
}

// reset path recursively
void QgsOWSConnectionItem::replacePath( QgsDataItem *item, QString before, QString after )
{
  item->setPath( item->path().replace( before, after ) );
  const auto constChildren = item->children();
  for ( QgsDataItem *subItem : constChildren )
  {
    replacePath( subItem, before, after );
  }
}

bool QgsOWSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsOWSConnectionItem *o = dynamic_cast<const QgsOWSConnectionItem *>( other );
  return ( o && mPath == o->mPath && mName == o->mName );
}


// ---------------------------------------------------------------------------


QgsOWSRootItem::QgsOWSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "OWS" ) )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconOws.svg" );
  populate();
}

QVector<QgsDataItem *> QgsOWSRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  // Combine all WMS,WFS,WCS connections
  QStringList connNames;
  Q_FOREACH ( const QString &service, QStringList() << "WMS" << "WFS" << "WCS" )
  {
    Q_FOREACH ( const QString &connName, QgsOwsConnection::connectionList( service ) )
    {
      if ( !connNames.contains( connName ) )
      {
        connNames << connName;
      }
    }
  }
  const auto constConnNames = connNames;
  for ( const QString &connName : constConnNames )
  {
    QgsDataItem *conn = new QgsOWSConnectionItem( this, connName, "ows:/" + connName );
    connections.append( conn );
  }
  return connections;
}


// ---------------------------------------------------------------------------

static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QString QgsOwsDataItemProvider::name()
{
  return QStringLiteral( "OWS" );
}

int QgsOwsDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsOwsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsOWSRootItem( parentItem, QStringLiteral( "OWS" ), QStringLiteral( "ows:" ) );
  }
  return nullptr;
}
