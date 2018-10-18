/***************************************************************************
      qgsafsdataitems.cpp
      -------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslogger.h"
#include "qgsowsconnection.h"
#include "qgsafsdataitems.h"
#include "qgsafsprovider.h"
#include "qgsarcgisrestutils.h"

#ifdef HAVE_GUI
#include "qgsnewhttpconnection.h"
#include "qgsafssourceselect.h"
#endif

#include <QMessageBox>
#include <QCoreApplication>
#include <QSettings>
#include <QUrl>


QgsAfsRootItem::QgsAfsRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconAfs.svg" );
  populate();
}

QVector<QgsDataItem *> QgsAfsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList connectionList = QgsOwsConnection::connectionList( "ARCGISFEATURESERVER" );
  for ( const QString &connName : connectionList )
  {
    const QString path = QStringLiteral( "afs:/" ) + connName;
    connections.append( new QgsAfsConnectionItem( this, connName, path, connName ) );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsAfsRootItem::actions( QWidget *parent )
{
  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsAfsRootItem::newConnection );
  return QList<QAction *>() << actionNew;
}

QWidget *QgsAfsRootItem::paramWidget()
{
  QgsAfsSourceSelect *select = new QgsAfsSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsArcGisServiceSourceSelect::connectionsChanged, this, &QgsAfsRootItem::onConnectionsChanged );
  return select;
}

void QgsAfsRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsAfsRootItem::newConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ) );
  nc.setWindowTitle( tr( "Create a New ArcGIS Feature Server Connection" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAfsConnectionItem::QgsAfsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName )
  : QgsDataCollectionItem( parent, name, path )
  , mConnName( connectionName )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsAfsConnectionItem::createChildren()
{
  const QgsOwsConnection connection( QStringLiteral( "ARCGISFEATURESERVER" ), mConnName );
  const QString url = connection.uri().param( QStringLiteral( "url" ) );
  const QString authcfg = connection.uri().param( QStringLiteral( "authcfg" ) );

  QVector<QgsDataItem *> layers;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( url, authcfg, errorTitle, errorMessage );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = qgis::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      layers.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return layers;
  }
  const QString authid = QgsArcGisRestUtils::parseSpatialReference( serviceData.value( QStringLiteral( "spatialReference" ) ).toMap() ).authid();

  const QVariantList layerInfoList = serviceData[QStringLiteral( "layers" )].toList();
  for ( const QVariant &layerInfo : layerInfoList )
  {
    const QVariantMap layerInfoMap = layerInfo.toMap();
    if ( !layerInfoMap.value( QStringLiteral( "subLayerIds" ) ).toList().empty() )
    {
      // group layer - do not show as it is not possible to load
      // TODO - show nested groups
      continue;
    }
    const QString id = layerInfoMap.value( QStringLiteral( "id" ) ).toString();
    QgsAfsLayerItem *layer = new QgsAfsLayerItem( this, mName, url + '/' + id, layerInfoMap.value( QStringLiteral( "name" ) ).toString(), authid, authcfg );
    layers.append( layer );
  }

  return layers;
}

bool QgsAfsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsAfsConnectionItem *o = qobject_cast<const QgsAfsConnectionItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

#ifdef HAVE_GUI
QList<QAction *> QgsAfsConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionRefresh = new QAction( tr( "Refresh" ), parent );
  connect( actionRefresh, &QAction::triggered, this, &QgsAfsConnectionItem::refreshConnection );
  lst.append( actionRefresh );

  QAction *separator = new QAction( parent );
  separator->setSeparator( true );
  lst.append( separator );

  QAction *actionEdit = new QAction( tr( "Edit Connection…" ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsAfsConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete Connection" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsAfsConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsAfsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), mName );
  nc.setWindowTitle( tr( "Modify ArcGIS Feature Server Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    refresh();
    if ( mParent )
      mParent->refreshConnections();
  }
}

void QgsAfsConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgisfeatureserver" ), mName );

  // the parent should be updated
  if ( mParent )
    mParent->refreshConnections();
}

void QgsAfsConnectionItem::refreshConnection()
{
  refresh();
  // the parent should be updated
  if ( mParent )
    mParent->refreshConnections();
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAfsLayerItem::QgsAfsLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &title, const QString &authid, const QString &authcfg )
  : QgsLayerItem( parent, title, parent->path() + "/" + name, QString(), QgsLayerItem::Vector, QStringLiteral( "arcgisfeatureserver" ) )
{
  mUri = QStringLiteral( "crs='%1' url='%2'" ).arg( authid, url );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );
  setState( Populated );
  mIconName = QStringLiteral( "mIconAfs.svg" );
}
