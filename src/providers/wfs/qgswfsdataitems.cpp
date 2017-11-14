/***************************************************************************
    qgswfsdataitems.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QClipboard>
#include <QMenu>

#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgswfsconstants.h"
#include "qgswfsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsdataitems.h"
#include "qgswfsdatasourceuri.h"
#include "qgssettings.h"
#include "qgsgeonodeconnection.h"
#include "qgsgeonoderequest.h"
#include "qgsstyle.h"

#ifdef HAVE_GUI
#include "qgsnewhttpconnection.h"
#include "qgswfssourceselect.h"
#endif

#include <QCoreApplication>
#include <QEventLoop>

//
// QgsWfsLayerItem
//

QgsWfsLayerItem::QgsWfsLayerItem( QgsDataItem *parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString )
  : QgsLayerItem( parent, title, parent->path() + '/' + name, QString(), QgsLayerItem::Vector, QStringLiteral( "WFS" ) )
{
  QgsSettings settings;
  bool useCurrentViewExtent = settings.value( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), true ).toBool();
  mUri = QgsWFSDataSourceURI::build( uri.uri( false ), featureType, crsString, QString(), useCurrentViewExtent );
  setState( Populated );
  mIconName = QStringLiteral( "mIconConnect.png" );
  mBaseUri = uri.param( QStringLiteral( "url" ) );
}

QList<QMenu *> QgsWfsLayerItem::menus( QWidget *parent )
{
  QList<QMenu *> menus;

  if ( mPath.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QMenu *menuStyleManager = new QMenu( tr( "Styles" ), parent );

    QAction *actionCopyStyle = new QAction( tr( "Copy Style" ), menuStyleManager );
    connect( actionCopyStyle, &QAction::triggered, this, &QgsWfsLayerItem::copyStyle );

    menuStyleManager->addAction( actionCopyStyle );
    menus << menuStyleManager;
  }

  return menus;
}

void QgsWfsLayerItem::copyStyle()
{
  std::unique_ptr< QgsGeoNodeConnection > connection;
  const QStringList connections = QgsGeoNodeConnectionUtils::connectionList();
  for ( const QString &connName : connections )
  {
    connection.reset( new QgsGeoNodeConnection( connName ) );
    if ( mBaseUri.contains( connection->uri().param( QStringLiteral( "url" ) ) ) )
      break;
    else
      connection.reset( nullptr );
  }

  if ( !connection )
  {
#ifdef QGISDEBUG
    QString errorMsg( QStringLiteral( "Cannot get style for layer %1" ).arg( this->name() ) );
    QgsDebugMsg( " Cannot get style: " + errorMsg );
#endif
#if 0
    // TODO: how to emit message from provider (which does not know about QgisApp)
    QgisApp::instance()->messageBar()->pushMessage( tr( "Cannot copy style" ),
        errorMsg,
        QgsMessageBar::CRITICAL, messageTimeout() );
#endif
    return;
  }

  QString url( connection->uri().encodedUri() );
  QgsGeoNodeRequest geoNodeRequest( url.replace( QStringLiteral( "url=" ), QString() ), true );
  QgsGeoNodeStyle style = geoNodeRequest.fetchDefaultStyleBlocking( this->name() );
  if ( style.name.isEmpty() )
  {
#ifdef QGISDEBUG
    QString errorMsg( QStringLiteral( "Cannot get style for layer %1" ).arg( this->name() ) );
    QgsDebugMsg( " Cannot get style: " + errorMsg );
#endif
#if 0
    // TODO: how to emit message from provider (which does not know about QgisApp)
    QgisApp::instance()->messageBar()->pushMessage( tr( "Cannot copy style" ),
        errorMsg,
        QgsMessageBar::CRITICAL, messageTimeout() );
#endif
    return;
  }

  QClipboard *clipboard = QApplication::clipboard();

  QMimeData *mdata = new QMimeData();
  mdata->setData( QGSCLIPBOARD_STYLE_MIME, style.body.toByteArray() );
  mdata->setText( style.body.toString() );
  // Copies data in text form as well, so the XML can be pasted into a text editor
  if ( clipboard->supportsSelection() )
    clipboard->setMimeData( mdata, QClipboard::Selection );
  clipboard->setMimeData( mdata, QClipboard::Clipboard );
  // Enables the paste menu element
  // actionPasteStyle->setEnabled( true );
}

//
// QgsWfsConnectionItem
//

QgsWfsConnectionItem::QgsWfsConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path )
  , mUri( uri )
{
  mIconName = QStringLiteral( "mIconWfs.svg" );
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsWfsConnectionItem::createChildren()
{
  QgsDataSourceUri uri( mUri );
  QgsDebugMsg( "mUri = " + mUri );

  QgsWfsCapabilities capabilities( mUri );

  const bool synchronous = true;
  const bool forceRefresh = false;
  capabilities.requestCapabilities( synchronous, forceRefresh );

  QVector<QgsDataItem *> layers;
  if ( capabilities.errorCode() == QgsWfsCapabilities::NoError )
  {
    QgsWfsCapabilities::Capabilities caps = capabilities.capabilities();
    Q_FOREACH ( const QgsWfsCapabilities::FeatureType &featureType, caps.featureTypes )
    {
      //QgsWFSLayerItem* layer = new QgsWFSLayerItem( this, mName, featureType.name, featureType.title );
      QgsWfsLayerItem *layer = new QgsWfsLayerItem( this, mName, uri, featureType.name, featureType.title, featureType.crslist.first() );
      layers.append( layer );
    }
  }
  else
  {
    //layers.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
  }

  return layers;
}

#ifdef HAVE_GUI
QList<QAction *> QgsWfsConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit..." ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsWfsConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsWfsConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsWfsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS, mName );
  nc.setWindowTitle( tr( "Modify WFS Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refreshConnections();
  }
}

void QgsWfsConnectionItem::deleteConnection()
{
  QgsWfsConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refreshConnections();
}
#endif


//
// QgsWfsRootItem
//

QgsWfsRootItem::QgsWfsRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconWfs.svg" );
  populate();
}

QVector<QgsDataItem *> QgsWfsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsWfsConnection::connectionList() )
  {
    QgsWfsConnection connection( connName );
    QString path = "wfs:/" + connName;
    QgsDataItem *conn = new QgsWfsConnectionItem( this, connName, path, connection.uri().uri() );
    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsWfsRootItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection..." ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsWfsRootItem::newConnection );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsWfsRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( nullptr, 0, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsWFSSourceSelect::connectionsChanged, this, &QgsWfsRootItem::onConnectionsChanged );
  return select;
}

void QgsWfsRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsWfsRootItem::newConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS );
  nc.setWindowTitle( tr( "Create a New WFS Connection" ) );

  if ( nc.exec() )
  {
    refreshConnections();
  }
}
#endif


//
// QgsWfsDataItemProvider
//

QgsDataItem *QgsWfsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "WFS path = " + path, 4 );
  if ( path.isEmpty() )
  {
    return new QgsWfsRootItem( parentItem, QStringLiteral( "WFS" ), QStringLiteral( "wfs:" ) );
  }

  // path schema: wfs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wfs:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsWfsConnection::connectionList().contains( connectionName ) )
    {
      QgsWfsConnection connection( connectionName );
      return new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, connection.uri().uri( false ) );
    }
  }
  else if ( path.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsGeoNodeConnectionUtils::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection connection( connectionName );

      QString url = connection.uri().param( QStringLiteral( "url" ) );
      QgsGeoNodeRequest geonodeRequest( url, true );

      QgsWFSDataSourceURI sourceUri( geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WFS" ) )[0] );

      QgsDebugMsgLevel( QString( "WFS full uri: '%1'." ).arg( QString( sourceUri.uri() ) ), 4 );

      return new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, sourceUri.uri( false ) );
    }
  }

  return nullptr;
}

QVector<QgsDataItem *> QgsWfsDataItemProvider::createDataItems( const QString &path, QgsDataItem *parentItem )
{
  QVector<QgsDataItem *> items;
  if ( path.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsGeoNodeConnectionUtils::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection connection( connectionName );

      QString url = connection.uri().param( QStringLiteral( "url" ) );
      QgsGeoNodeRequest geonodeRequest( url, true );

      const QStringList encodedUris( geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WFS" ) ) );

      if ( !encodedUris.isEmpty() )
      {
        for ( const QString &encodedUri : encodedUris )
        {
          QgsWFSDataSourceURI uri( encodedUri );
          QgsDebugMsgLevel( QStringLiteral( "WFS full uri: '%1'." ).arg( uri.uri( false ) ), 4 );

          QgsDataItem *item = new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, uri.uri( false ) );
          if ( item )
          {
            items.append( item );
          }
        }
      }
    }
  }

  return items;
}


#ifdef HAVE_GUI
QGISEXTERN QgsWFSSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsWFSSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN QList<QgsDataItemProvider *> *dataItemProviders()
{
  QList<QgsDataItemProvider *> *providers = new QList<QgsDataItemProvider *>();

  *providers << new QgsWfsDataItemProvider;

  return providers;
}
