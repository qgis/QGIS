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
#include "qgsdatasourceuri.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"

#include "qgsapplication.h"

#include <QFileInfo>

// ---------------------------------------------------------------------------
QgsOWSConnectionItem::QgsOWSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconConnect.png";
}

QgsOWSConnectionItem::~QgsOWSConnectionItem()
{
}

QVector<QgsDataItem*> QgsOWSConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> children;
  QMap<QgsDataItem*, QString> serviceItems; // service/provider key

  int layerCount = 0;
  // Try to open with WMS,WFS,WCS
  Q_FOREACH ( const QString& key, QStringList() << "wms" << "WFS" << "wcs" )
  {
    QgsDebugMsg( "Add connection for provider " + key );
    QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( key );
    if ( !library )
    {
      QgsDebugMsg( "Cannot get provider " + key );
      continue;
    }

    dataItem_t * dItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    QString path = key.toLower() + ":/" + name();
    QgsDebugMsg( "path = " + path );
    QgsDataItem *item = dItem( path, this );  // empty path -> top level
    if ( !item )
    {
      QgsDebugMsg( "Connection not found by provider" );
      continue;
    }

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

  Q_FOREACH ( QgsDataItem* item, serviceItems.keys() )
  {
    QgsDebugMsg( QString( "serviceItems.size = %1 layerCount = %2 rowCount = %3" ).arg( serviceItems.size() ).arg( layerCount ).arg( item->rowCount() ) );
    QString providerKey = serviceItems.value( item );
    if ( serviceItems.size() == 1 || layerCount <= 30 || item->rowCount() <= 10 )
    {
      // Add layers directly to OWS connection
      Q_FOREACH ( QgsDataItem* subItem, item->children() )
      {
        item->removeChildItem( subItem );
        subItem->setParent( this );
        replacePath( subItem, providerKey.toLower() + ":/", "ows:/" );
        children.append( subItem );
      }
      delete item;
    }
    else // Add service
    {
      replacePath( item, item->path(), path() + "/" + providerKey.toLower() );
      children.append( item );
    }
  }

  return children;
}

// reset path recursively
void QgsOWSConnectionItem::replacePath( QgsDataItem* item, QString before, QString after )
{
  item->setPath( item->path().replace( before, after ) );
  Q_FOREACH ( QgsDataItem* subItem, item->children() )
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

QList<QAction*> QgsOWSConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsOWSConnectionItem::editConnection()
{
#if 0
  QgsNewHttpConnection nc( 0, "/Qgis/connections-ows/", mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
#endif
}

void QgsOWSConnectionItem::deleteConnection()
{
#if 0
  QgsOWSConnection::deleteConnection( "OWS", mName );
  // the parent should be updated
  mParent->refresh();
#endif
}


// ---------------------------------------------------------------------------


QgsOWSRootItem::QgsOWSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = "mIconOws.svg";
  populate();
}

QgsOWSRootItem::~QgsOWSRootItem()
{
}

QVector<QgsDataItem*> QgsOWSRootItem::createChildren()
{
  QgsDebugMsg( "Entered" );
  QVector<QgsDataItem*> connections;
  // Combine all WMS,WFS,WCS connections
  QStringList connNames;
  Q_FOREACH ( const QString& service, QStringList() << "WMS" << "WFS" << "WCS" )
  {
    Q_FOREACH ( const QString& connName, QgsOWSConnection::connectionList( service ) )
    {
      if ( !connNames.contains( connName ) )
      {
        connNames << connName;
      }
    }
  }
  Q_FOREACH ( const QString& connName, connNames )
  {
    QgsDataItem * conn = new QgsOWSConnectionItem( this, connName, "ows:/" + connName );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsOWSRootItem::actions()
{
  QList<QAction*> lst;

#if 0
  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );
#endif

  return lst;
}


QWidget * QgsOWSRootItem::paramWidget()
{
#if 0
  QgsOWSSourceSelect *select = new QgsOWSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
#endif
  return 0;
}
void QgsOWSRootItem::connectionsChanged()
{
  refresh();
}

void QgsOWSRootItem::newConnection()
{
#if 0
  QgsNewHttpConnection nc( 0, "/Qgis/connections-ows/" );

  if ( nc.exec() )
  {
    refresh();
  }
#endif
}


// ---------------------------------------------------------------------------

static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( thePath.isEmpty() )
  {
    return new QgsOWSRootItem( parentItem, "OWS", "ows:" );
  }
  return 0;
}

//QGISEXTERN QgsOWSSourceSelect * selectWidget( QWidget * parent, Qt::WindowFlags fl )
QGISEXTERN QDialog * selectWidget( QWidget * parent, Qt::WindowFlags fl )
{
  Q_UNUSED( parent );
  Q_UNUSED( fl );
  //return new QgsOWSSourceSelect( parent, fl );
  return 0;
}
