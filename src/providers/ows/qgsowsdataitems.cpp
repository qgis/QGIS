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

#ifdef HAVE_GUI
#include "qgsnewhttpconnection.h"
#include "qgsowssourceselect.h"
#endif

#include "qgsapplication.h"

#include <QFileInfo>

// ---------------------------------------------------------------------------
QgsOWSConnectionItem::QgsOWSConnectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
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
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
    {
      QgsDebugMsg( "Cannot get provider " + key );
      continue;
    }

    dataItem_t *dItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
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
      QgsDebugMsg( QStringLiteral( "Connection not found by provider" ) );
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

  for ( auto it = serviceItems.constBegin(); it != serviceItems.constEnd(); ++it )
  {
    QgsDataItem *item = it.key();
    QgsDebugMsg( QStringLiteral( "serviceItems.size = %1 layerCount = %2 rowCount = %3" ).arg( serviceItems.size() ).arg( layerCount ).arg( item->rowCount() ) );
    QString providerKey = it.value();
    if ( serviceItems.size() == 1 || layerCount <= 30 || item->rowCount() <= 10 )
    {
      // Add layers directly to OWS connection
      Q_FOREACH ( QgsDataItem *subItem, item->children() )
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
  Q_FOREACH ( QgsDataItem *subItem, item->children() )
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

#ifdef HAVE_GUI
QList<QAction *> QgsOWSConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit…" ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsOWSConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsOWSConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsOWSConnectionItem::editConnection()
{
#if 0
  QgsNewHttpConnection nc( 0, "qgis/connections-ows/", mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refreshConnections();
  }
#endif
}

void QgsOWSConnectionItem::deleteConnection()
{
#if 0
  QgsOWSConnection::deleteConnection( "OWS", mName );
  // the parent should be updated
  mParent->refreshConnections();
#endif
}
#endif


// ---------------------------------------------------------------------------


QgsOWSRootItem::QgsOWSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
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
  Q_FOREACH ( const QString &connName, connNames )
  {
    QgsDataItem *conn = new QgsOWSConnectionItem( this, connName, "ows:/" + connName );
    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsOWSRootItem::actions( QWidget *parent )
{
  Q_UNUSED( parent );
  QList<QAction *> lst;

#if 0
  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );
#endif

  return lst;
}


QWidget *QgsOWSRootItem::paramWidget()
{
#if 0
  QgsOWSSourceSelect *select = new QgsOWSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
#endif
  return nullptr;
}
void QgsOWSRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsOWSRootItem::newConnection()
{
#if 0
  QgsNewHttpConnection nc( 0, "qgis/connections-ows/" );

  if ( nc.exec() )
  {
    refreshConnections();
  }
#endif
}
#endif


// ---------------------------------------------------------------------------

static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsOWSRootItem( parentItem, QStringLiteral( "OWS" ), QStringLiteral( "ows:" ) );
  }
  return nullptr;
}

//QGISEXTERN QgsOWSSourceSelect * selectWidget( QWidget * parent, Qt::WindowFlags fl )
QGISEXTERN QDialog *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  Q_UNUSED( parent );
  Q_UNUSED( fl );
  Q_UNUSED( widgetMode );
  //return new QgsOWSSourceSelect( parent, fl, widgetMode );
  return nullptr;
}
