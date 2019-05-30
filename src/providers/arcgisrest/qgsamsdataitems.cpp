/***************************************************************************
      qgsamsdataitems.cpp
      -------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsamsdataitems.h"
#include "qgsarcgisrestutils.h"
#include "qgsowsconnection.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"

#ifdef HAVE_GUI
#include "qgsamssourceselect.h"
#include "qgsnewhttpconnection.h"
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#endif

#include <QImageReader>

QgsAmsRootItem::QgsAmsRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast | Collapse;
  mIconName = QStringLiteral( "mIconAms.svg" );
  populate();
}

QVector<QgsDataItem *> QgsAmsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList connectionList = QgsOwsConnection::connectionList( QStringLiteral( "ARCGISMAPSERVER" ) );
  for ( const QString &connName : connectionList )
  {
    QString path = "ams:/" + connName;
    connections.append( new QgsAmsConnectionItem( this, connName, path, connName ) );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsAmsRootItem::actions( QWidget *parent )
{
  QAction *actionNew = new QAction( tr( "New Connection…" ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsAmsRootItem::newConnection );
  return QList<QAction *>() << actionNew;
}


QWidget *QgsAmsRootItem::paramWidget()
{
  QgsAmsSourceSelect *select = new QgsAmsSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsArcGisServiceSourceSelect::connectionsChanged, this, &QgsAmsRootItem::onConnectionsChanged );
  return select;
}

void QgsAmsRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsAmsRootItem::newConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgismapserver/" ), QString(), QgsNewHttpConnection::FlagShowHttpSettings );
  nc.setWindowTitle( tr( "Create a New ArcGIS Map Server Connection" ) );

  if ( nc.exec() )
  {
    refreshConnections();
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAmsConnectionItem::QgsAmsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName )
  : QgsDataCollectionItem( parent, name, path )
  , mConnName( connectionName )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
}

QVector<QgsDataItem *> QgsAmsConnectionItem::createChildren()
{
  const QgsOwsConnection connection( QStringLiteral( "ARCGISMAPSERVER" ), mConnName );
  const QString url = connection.uri().param( QStringLiteral( "url" ) );
  const QString authcfg = connection.uri().param( QStringLiteral( "authcfg" ) );
  const QString referer = connection.uri().param( QStringLiteral( "referer" ) );
  QgsStringMap headers;
  if ( ! referer.isEmpty() )
    headers[ QStringLiteral( "Referer" )] = referer;

  QVector<QgsDataItem *> layers;
  QString errorTitle, errorMessage;

  QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( url, authcfg, errorTitle,  errorMessage, headers );
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

  QString authid = QgsArcGisRestUtils::parseSpatialReference( serviceData[QStringLiteral( "spatialReference" )].toMap() ).authid();
  QString format = QStringLiteral( "jpg" );
  bool found = false;
  const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
  const QStringList supportedImageFormatTypes = serviceData.value( QStringLiteral( "supportedImageFormatTypes" ) ).toString().split( ',' );
  for ( const QString &encoding : supportedImageFormatTypes )
  {
    for ( const QByteArray &fmt : supportedFormats )
    {
      if ( encoding.startsWith( fmt, Qt::CaseInsensitive ) )
      {
        format = encoding;
        found = true;
        break;
      }
    }
    if ( found )
      break;
  }

  const QVariantList layersList = serviceData.value( QStringLiteral( "layers" ) ).toList();
  for ( const QVariant &layerInfo : layersList )
  {
    QVariantMap layerInfoMap = layerInfo.toMap();
    QString id = layerInfoMap[QStringLiteral( "id" )].toString();
    QgsAmsLayerItem *layer = new QgsAmsLayerItem( this, mName, url, id, layerInfoMap[QStringLiteral( "name" )].toString(), authid, format, authcfg, headers );
    layers.append( layer );
  }

  return layers;
}

bool QgsAmsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsAmsConnectionItem *o = qobject_cast<const QgsAmsConnectionItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

QString QgsAmsConnectionItem::url() const
{
  const QgsOwsConnection connection( QStringLiteral( "ARCGISMAPSERVER" ), mConnName );
  return connection.uri().param( QStringLiteral( "url" ) );
}

#ifdef HAVE_GUI
QList<QAction *> QgsAmsConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit…" ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsAmsConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsAmsConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsAmsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-arcgismapserver/" ), mName, QgsNewHttpConnection::FlagShowHttpSettings );
  nc.setWindowTitle( tr( "Modify ArcGIS Map Server Connection" ) );

  if ( nc.exec() )
  {
    mParent->refreshConnections();
  }
}

void QgsAmsConnectionItem::deleteConnection()
{
  QgsOwsConnection::deleteConnection( QStringLiteral( "arcgismapserver" ), mName );
  mParent->refreshConnections();
}
#endif

///////////////////////////////////////////////////////////////////////////////

QgsAmsLayerItem::QgsAmsLayerItem( QgsDataItem *parent, const QString &, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format, const QString &authcfg, const QgsStringMap &headers )
  : QgsLayerItem( parent, title, url + '/' + id, QString(), QgsLayerItem::Raster, QStringLiteral( "arcgismapserver" ) )
{
  mUri = QStringLiteral( "crs='%1' format='%2' layer='%3' url='%4'" ).arg( authid, format, id, url );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );
  if ( !headers.value( QStringLiteral( "Referer" ) ).isEmpty() )
    mUri += QStringLiteral( " referer='%1'" ).arg( headers.value( QStringLiteral( "Referer" ) ) );
  setState( Populated );
  mIconName = QStringLiteral( "mIconAms.svg" );
  setToolTip( mPath );
}

//
// QgsAmsDataItemProvider
//

QgsDataItem *QgsAmsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsAmsRootItem( parentItem, QStringLiteral( "ArcGisMapServer" ), QStringLiteral( "arcgismapserver:" ) );
  }

  // path schema: ams:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "ams:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "arcgismapserver" ) ).contains( connectionName ) )
    {
      return new QgsAmsConnectionItem( parentItem, QStringLiteral( "ArcGisMapServer" ), path, connectionName );
    }
  }

  return nullptr;
}

#ifdef HAVE_GUI

//
// QgsAmsItemGuiProvider
//

QString QgsAmsItemGuiProvider::name()
{
  return QStringLiteral( "ams_items" );
}

void QgsAmsItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsAmsConnectionItem *connectionItem = qobject_cast< QgsAmsConnectionItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( connectionItem->url() ) );
    } );
    menu->addAction( viewInfo );
  }
  else if ( QgsAmsLayerItem *layerItem = qobject_cast< QgsAmsLayerItem * >( item ) )
  {
    QAction *viewInfo = new QAction( tr( "View Service Info" ), menu );
    connect( viewInfo, &QAction::triggered, this, [ = ]
    {
      QDesktopServices::openUrl( QUrl( layerItem->path() ) );
    } );
    menu->addAction( viewInfo );
    menu->addSeparator();
  }
}

#endif
