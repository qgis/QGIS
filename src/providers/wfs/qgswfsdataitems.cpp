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
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifcollection.h"
#include "qgsoapifprovider.h"
#include "qgswfsconstants.h"
#include "qgswfsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsdataitems.h"
#include "moc_qgswfsdataitems.cpp"
#include "qgswfsdatasourceuri.h"
#include "qgswfsprovider.h"
#include "qgssettings.h"

#ifdef HAVE_GUI
#include "qgswfssourceselect.h"
#endif

#include <QCoreApplication>
#include <QEventLoop>

//
// QgsWfsLayerItem
//

QgsWfsLayerItem::QgsWfsLayerItem( QgsDataItem *parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString, const QString &providerKey )
  : QgsLayerItem( parent, title.isEmpty() ? featureType : title, parent->path() + '/' + name, QString(), Qgis::BrowserLayerType::Vector, providerKey )
{
  const QgsSettings settings;
  const bool useCurrentViewExtent = settings.value( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), true ).toBool();
  mUri = QgsWFSDataSourceURI::build( uri.uri( false ), featureType, crsString, QString(), QString(), useCurrentViewExtent );
  setState( Qgis::BrowserItemState::Populated );
  mIconName = QStringLiteral( "mIconWfs.svg" );
  mBaseUri = uri.param( QStringLiteral( "url" ) );
}

//
// QgsWfsConnectionItem
//

QgsWfsConnectionItem::QgsWfsConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "WFS" ) )
  , mUri( uri )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}


QVector<QgsDataItem *> QgsWfsConnectionItem::createChildrenOapif()
{
  QVector<QgsDataItem *> layers;
  const QgsDataSourceUri uri( mUri );
  const bool synchronous = true;
  const bool forceRefresh = false;

  QgsOapifLandingPageRequest landingPageRequest( uri );
  if ( landingPageRequest.request( synchronous, forceRefresh ) && landingPageRequest.errorCode() == QgsBaseNetworkRequest::NoError )
  {
    QString url = landingPageRequest.collectionsUrl();
    while ( !url.isEmpty() )
    {
      QgsOapifCollectionsRequest collectionsRequest( uri, url );
      url.clear();
      if ( collectionsRequest.request( synchronous, forceRefresh ) && collectionsRequest.errorCode() == QgsBaseNetworkRequest::NoError )
      {
        for ( const auto &collection : collectionsRequest.collections() )
        {
          QgsWfsLayerItem *layer = new QgsWfsLayerItem(
            this, mName, uri, collection.mId, collection.mTitle,
            QString(), QgsOapifProvider::OAPIF_PROVIDER_KEY
          );
          layers.append( layer );
        }
        url = collectionsRequest.nextUrl();
      }
    }
  }

  return layers;
}

QVector<QgsDataItem *> QgsWfsConnectionItem::createChildren()
{
  const QgsDataSourceUri uri( mUri );
  QgsDebugMsgLevel( "mUri = " + mUri, 2 );

  const bool synchronous = true;
  const bool forceRefresh = false;
  const auto version = QgsWFSDataSourceURI( mUri ).version();
  if ( version == QLatin1String( "OGC_API_FEATURES" ) )
  {
    return createChildrenOapif();
  }
  else
  {
    QgsWfsCapabilities capabilities( mUri );
    if ( version == QgsWFSConstants::VERSION_AUTO )
    {
      capabilities.setLogErrors( false ); // as this might be a OAPIF server
    }
    capabilities.requestCapabilities( synchronous, forceRefresh );

    QVector<QgsDataItem *> layers;
    if ( capabilities.errorCode() == QgsWfsCapabilities::NoError )
    {
      const auto featureTypes = capabilities.capabilities().featureTypes;
      for ( const QgsWfsCapabilities::FeatureType &featureType : featureTypes )
      {
        QgsWfsLayerItem *layer = new QgsWfsLayerItem(
          this, mName, uri, featureType.name, featureType.title,
          !featureType.crslist.isEmpty() ? featureType.crslist.first() : QString(),
          QgsWFSProvider::WFS_PROVIDER_KEY
        );
        layers.append( layer );
      }
    }
    else if ( version == QgsWFSConstants::VERSION_AUTO )
    {
      return createChildrenOapif();
    }

    return layers;
  }
}


//
// QgsWfsRootItem
//

QgsWfsRootItem::QgsWfsRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "WFS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconWfs.svg" );
  populate();
}

QVector<QgsDataItem *> QgsWfsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList list = QgsWfsConnection::connectionList();
  for ( const QString &connName : list )
  {
    const QgsWfsConnection connection( connName );
    const QString path = "wfs:/" + connName;
    QgsDataItem *conn = new QgsWfsConnectionItem( this, connName, path, connection.uri().uri( false ) );
    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI

QWidget *QgsWfsRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsWFSSourceSelect::connectionsChanged, this, &QgsWfsRootItem::onConnectionsChanged );
  return select;
}

void QgsWfsRootItem::onConnectionsChanged()
{
  refresh();
}

#endif


//
// QgsWfsDataItemProvider
//

QString QgsWfsDataItemProvider::name()
{
  return QStringLiteral( "WFS" );
}

QString QgsWfsDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "WFS" );
}

Qgis::DataItemProviderCapabilities QgsWfsDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsWfsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "WFS path = " + path, 4 );
  if ( path.isEmpty() )
  {
    return new QgsWfsRootItem( parentItem, QObject::tr( "WFS / OGC API - Features" ), QStringLiteral( "wfs:" ) );
  }

  // path schema: wfs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wfs:/" ) ) )
  {
    const QString connectionName = path.split( '/' ).last();
    if ( QgsWfsConnection::connectionList().contains( connectionName ) )
    {
      const QgsWfsConnection connection( connectionName );
      return new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, connection.uri().uri( false ) );
    }
  }

  return nullptr;
}


bool QgsWfsConnectionItem::layerCollection() const
{
  return true;
}
