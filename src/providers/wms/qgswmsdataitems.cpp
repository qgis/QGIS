/***************************************************************************
    qgswmsdataitems.cpp
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
#include "qgswmsdataitems.h"

#include "qgslogger.h"

#include "qgsdataitemproviderregistry.h"
#include "qgsdatasourceuri.h"
#include "qgswmscapabilities.h"
#include "qgswmsconnection.h"
#include "qgsxyzconnection.h"

#ifdef HAVE_GUI
#include "qgswmssourceselect.h"
#endif
#include "qgsgeonodeconnection.h"
#include "qgsgeonoderequest.h"
#include "qgssettings.h"

// ---------------------------------------------------------------------------
QgsWMSConnectionItem::QgsWMSConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path )
  , mUri( uri )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
  mCapabilitiesDownload = new QgsWmsCapabilitiesDownload( false );
}

QgsWMSConnectionItem::~QgsWMSConnectionItem()
{
  delete mCapabilitiesDownload;
}

void QgsWMSConnectionItem::deleteLater()
{
  if ( mCapabilitiesDownload )
  {
    mCapabilitiesDownload->abort();
  }
  QgsDataCollectionItem::deleteLater();
}

QVector<QgsDataItem *> QgsWMSConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  QgsDataSourceUri uri;
  uri.setEncodedUri( mUri );

  QgsDebugMsgLevel( "mUri = " + mUri, 2 );

  QgsWmsSettings wmsSettings;
  if ( !wmsSettings.parseUri( mUri ) )
  {
    children.append( new QgsErrorItem( this, tr( "Failed to parse WMS URI" ), mPath + "/error" ) );
    return children;
  }

  bool res = mCapabilitiesDownload->downloadCapabilities( wmsSettings.baseUrl(), wmsSettings.authorization() );

  if ( !res )
  {
    children.append( new QgsErrorItem( this, tr( "Failed to download capabilities" ), mPath + "/error" ) );
    return children;
  }

  QgsWmsCapabilities caps;
  if ( !caps.parseResponse( mCapabilitiesDownload->response(), wmsSettings.parserSettings() ) )
  {
    children.append( new QgsErrorItem( this, tr( "Failed to parse capabilities" ), mPath + "/error" ) );
    return children;
  }

  // Attention: supportedLayers() gives tree leafs, not top level
  QVector<QgsWmsLayerProperty> layerProperties = caps.supportedLayers();
  if ( !layerProperties.isEmpty() )
  {
    QgsWmsCapabilitiesProperty capabilitiesProperty = caps.capabilitiesProperty();
    const QgsWmsCapabilityProperty &capabilityProperty = capabilitiesProperty.capability;

    for ( const QgsWmsLayerProperty &layerProperty : qgis::as_const( capabilityProperty.layers ) )
    {
      // Attention, the name may be empty
      QgsDebugMsgLevel( QString::number( layerProperty.orderId ) + ' ' + layerProperty.name + ' ' + layerProperty.title, 2 );
      QString pathName = layerProperty.name.isEmpty() ? QString::number( layerProperty.orderId ) : layerProperty.name;

      QgsWMSLayerItem *layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + '/' + pathName, capabilitiesProperty, uri, layerProperty );

      children << layer;
    }
  }

  QStringList styleIdentifiers;
  QStringList linkIdentifiers;

  QList<QgsWmtsTileLayer> tileLayers = caps.supportedTileLayers();
  if ( !tileLayers.isEmpty() )
  {
    QHash<QString, QgsWmtsTileMatrixSet> tileMatrixSets = caps.supportedTileMatrixSets();

    const auto constTileLayers = tileLayers;
    for ( const QgsWmtsTileLayer &l : constTileLayers )
    {
      QString title = l.title.isEmpty() ? l.identifier : l.title;
      QgsDataItem *layerItem = l.styles.size() == 1 ? this : new QgsDataCollectionItem( this, title, mPath + '/' + l.identifier );
      if ( layerItem != this )
      {
        layerItem->setCapabilities( layerItem->capabilities2() & ~QgsDataItem::Fertile );
        layerItem->setState( QgsDataItem::Populated );
        layerItem->setToolTip( title );
        children << layerItem;
      }

      for ( const QgsWmtsStyle &style : qgis::as_const( l.styles ) )
      {
        QString styleName = style.title.isEmpty() ? style.identifier : style.title;
        if ( layerItem == this )
          styleName = title;  // just one style so no need to display it

        // Ensure style path is unique
        QString stylePathIdentifier { style.identifier };
        int i = 0;
        while ( styleIdentifiers.contains( stylePathIdentifier ) )
        {
          stylePathIdentifier = QStringLiteral( "%1_%2" ).arg( style.identifier ).arg( ++i );
        }
        styleIdentifiers.push_back( stylePathIdentifier );

        QgsDataItem *styleItem = l.setLinks.size() == 1 ? layerItem : new QgsDataCollectionItem( layerItem, styleName, layerItem->path() + '/' + stylePathIdentifier );
        if ( styleItem != layerItem )
        {
          styleItem->setCapabilities( styleItem->capabilities2() & ~QgsDataItem::Fertile );
          styleItem->setState( QgsDataItem::Populated );
          styleItem->setToolTip( styleName );
          if ( layerItem == this )
          {
            children << styleItem;
          }
          else
          {
            layerItem->addChildItem( styleItem );
          }
        }

        for ( const QgsWmtsTileMatrixSetLink &setLink : qgis::as_const( l.setLinks ) )
        {
          QString linkName = setLink.tileMatrixSet;
          if ( styleItem == layerItem )
            linkName = styleName;  // just one link so no need to display it

          // Ensure link path is unique
          QString linkPathIdentifier { linkName };
          int i = 0;
          while ( linkIdentifiers.contains( linkPathIdentifier ) )
          {
            linkPathIdentifier = QStringLiteral( "%1_%2" ).arg( linkName ).arg( ++i );
          }
          linkIdentifiers.push_back( linkPathIdentifier );


          QgsDataItem *linkItem = l.formats.size() == 1 ? styleItem : new QgsDataCollectionItem( styleItem, linkName, styleItem->path() + '/' + linkPathIdentifier );
          if ( linkItem != styleItem )
          {
            linkItem->setCapabilities( linkItem->capabilities2() & ~QgsDataItem::Fertile );
            linkItem->setState( QgsDataItem::Populated );
            linkItem->setToolTip( linkName );
            if ( styleItem == this )
            {
              children << linkItem;
            }
            else
            {
              styleItem->addChildItem( linkItem );
            }
          }

          for ( const QString &format : qgis::as_const( l.formats ) )
          {
            QString name = format;
            if ( linkItem == styleItem )
              name = linkName;  // just one format so no need to display it

            QgsDataItem *tileLayerItem = new QgsWMTSLayerItem( linkItem, name, linkItem->path() + '/' + name, uri,
                l.identifier, format, style.identifier, setLink.tileMatrixSet, tileMatrixSets[ setLink.tileMatrixSet ].crs, title );
            tileLayerItem->setToolTip( name );
            if ( linkItem == this )
            {
              children << tileLayerItem;
            }
            else
            {
              linkItem->addChildItem( tileLayerItem );
            }
          }
        }
      }
    }
  }

  return children;
}

bool QgsWMSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWMSConnectionItem *o = dynamic_cast<const QgsWMSConnectionItem *>( other );
  if ( !o )
  {
    return false;
  }

  return ( mPath == o->mPath && mName == o->mName );
}

// ---------------------------------------------------------------------------

QgsWMSLayerItem::QgsWMSLayerItem( QgsDataItem *parent, QString name, QString path, const QgsWmsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWmsLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), QgsLayerItem::Raster, QStringLiteral( "wms" ) )
  , mCapabilitiesProperty( capabilitiesProperty )
  , mDataSourceUri( dataSourceUri )
  , mLayerProperty( layerProperty )
{
  mSupportedCRS = mLayerProperty.crs;
  mSupportFormats = mCapabilitiesProperty.capability.request.getMap.format;
  QgsDebugMsgLevel( "uri = " + mDataSourceUri.encodedUri(), 2 );

  mUri = createUri();

  // Populate everything, it costs nothing, all info about layers is collected
  for ( const QgsWmsLayerProperty &layerProperty : qgis::as_const( mLayerProperty.layer ) )
  {
    // Attention, the name may be empty
    QgsDebugMsgLevel( QString::number( layerProperty.orderId ) + ' ' + layerProperty.name + ' ' + layerProperty.title, 2 );
    QString pathName = layerProperty.name.isEmpty() ? QString::number( layerProperty.orderId ) : layerProperty.name;
    QgsWMSLayerItem *layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + '/' + pathName, mCapabilitiesProperty, dataSourceUri, layerProperty );
    //mChildren.append( layer );
    addChildItem( layer );
  }

  mIconName = QStringLiteral( "mIconWms.svg" );

  setState( Populated );
}

QString QgsWMSLayerItem::createUri()
{
  if ( mLayerProperty.name.isEmpty() )
    return QString(); // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( QStringLiteral( "layers" ), mLayerProperty.name );
  QString style = !mLayerProperty.style.isEmpty() ? mLayerProperty.style.at( 0 ).name : QString();
  mDataSourceUri.setParam( QStringLiteral( "styles" ), style );

  QString format;
  // get first supported by qt and server
  QVector<QgsWmsSupportedFormat> formats( QgsWmsProvider::supportedFormats() );
  const auto constFormats = formats;
  for ( const QgsWmsSupportedFormat &f : constFormats )
  {
    if ( mCapabilitiesProperty.capability.request.getMap.format.indexOf( f.format ) >= 0 )
    {
      format = f.format;
      break;
    }
  }
  mDataSourceUri.setParam( QStringLiteral( "format" ), format );

  QString crs;
  // get first known if possible
  QgsCoordinateReferenceSystem testCrs;
  for ( const QString &c : qgis::as_const( mLayerProperty.crs ) )
  {
    testCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( c );
    if ( testCrs.isValid() )
    {
      crs = c;
      break;
    }
  }
  if ( crs.isEmpty() && !mLayerProperty.crs.isEmpty() )
  {
    crs = mLayerProperty.crs[0];
  }
  mDataSourceUri.setParam( QStringLiteral( "crs" ), crs );
  //uri = rasterLayerPath + "|layers=" + layers.join( "," ) + "|styles=" + styles.join( "," ) + "|format=" + format + "|crs=" + crs;

  return mDataSourceUri.encodedUri();
}

// ---------------------------------------------------------------------------

QgsWMTSLayerItem::QgsWMTSLayerItem( QgsDataItem *parent,
                                    const QString &name,
                                    const QString &path,
                                    const QgsDataSourceUri &uri,
                                    const QString &id,
                                    const QString &format,
                                    const QString &style,
                                    const QString &tileMatrixSet,
                                    const QString &crs,
                                    const QString &title )
  : QgsLayerItem( parent, name, path, QString(), QgsLayerItem::Raster, QStringLiteral( "wms" ) )
  , mDataSourceUri( uri )
  , mId( id )
  , mFormat( format )
  , mStyle( style )
  , mTileMatrixSet( tileMatrixSet )
  , mCrs( crs )
  , mTitle( title )
{
  mUri = createUri();
  setState( Populated );
}

QString QgsWMTSLayerItem::createUri()
{
  // TODO dimensions

  QgsDataSourceUri uri( mDataSourceUri );
  uri.setParam( QStringLiteral( "layers" ), mId );
  uri.setParam( QStringLiteral( "styles" ), mStyle );
  uri.setParam( QStringLiteral( "format" ), mFormat );
  uri.setParam( QStringLiteral( "crs" ), mCrs );
  uri.setParam( QStringLiteral( "tileMatrixSet" ), mTileMatrixSet );
  return uri.encodedUri();
}

// ---------------------------------------------------------------------------
QgsWMSRootItem::QgsWMSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconWms.svg" );
  populate();
}

QVector<QgsDataItem *> QgsWMSRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const auto connectionList = QgsWMSConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    QgsWMSConnection connection( connName );
    QgsDataItem *conn = new QgsWMSConnectionItem( this, connName, mPath + '/' + connName, connection.uri().encodedUri() );

    connections.append( conn );
  }
  return connections;
}

// ---------------------------------------------------------------------------


QgsDataItem *QgsWmsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "path = " + path, 2 );
  if ( path.isEmpty() )
  {
    return new QgsWMSRootItem( parentItem, QStringLiteral( "WMS/WMTS" ), QStringLiteral( "wms:" ) );
  }

  // path schema: wms:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wms:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsWMSConnection::connectionList().contains( connectionName ) )
    {
      QgsWMSConnection connection( connectionName );
      return new QgsWMSConnectionItem( parentItem, QStringLiteral( "WMS/WMTS" ), path, connection.uri().encodedUri() );
    }
  }

  return nullptr;
}

// ---------------------------------------------------------------------------


QgsXyzTileRootItem::QgsXyzTileRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconWms.svg" );
  populate();
}

QVector<QgsDataItem *> QgsXyzTileRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsXyzConnectionUtils::connectionList();
  for ( const QString &connName : connectionList )
  {
    QgsXyzConnection connection( QgsXyzConnectionUtils::connection( connName ) );
    QgsDataItem *conn = new QgsXyzLayerItem( this, connName, mPath + '/' + connName, connection.encodedUri() );
    connections.append( conn );
  }
  return connections;
}


// ---------------------------------------------------------------------------


QgsXyzLayerItem::QgsXyzLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri )
  : QgsLayerItem( parent, name, path, encodedUri, QgsLayerItem::Raster, QStringLiteral( "wms" ) )
{
  setState( Populated );
}


// ---------------------------------------------------------------------------


QVector<QgsDataItem *> QgsWmsDataItemProvider::createDataItems( const QString &path, QgsDataItem *parentItem )
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

      const QStringList encodedUris( geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WMS" ) ) );

      if ( !encodedUris.isEmpty() )
      {
        for ( const QString &encodedUri : encodedUris )
        {
          QgsDebugMsgLevel( encodedUri, 3 );
          QgsDataSourceUri uri;
          QgsSettings settings;
          QString key( QgsGeoNodeConnectionUtils::pathGeoNodeConnection() + "/" + connectionName );

          QString dpiMode = settings.value( key + "/wms/dpiMode", "all" ).toString();
          uri.setParam( QStringLiteral( "url" ), encodedUri );
          if ( !dpiMode.isEmpty() )
          {
            uri.setParam( QStringLiteral( "dpiMode" ), dpiMode );
          }

          QgsDebugMsgLevel( QStringLiteral( "WMS full uri: '%1'." ).arg( QString( uri.encodedUri() ) ), 2 );

          QgsDataItem *item = new QgsWMSConnectionItem( parentItem, QStringLiteral( "WMS" ), path, uri.encodedUri() );
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

QString QgsXyzTileDataItemProvider::name()
{
  return QStringLiteral( "XYZ Tiles" );
}

int QgsXyzTileDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsXyzTileDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsXyzTileRootItem( parentItem, QStringLiteral( "XYZ Tiles" ), QStringLiteral( "xyz:" ) );
  return nullptr;
}

QVector<QgsDataItem *> QgsXyzTileDataItemProvider::createDataItems( const QString &path, QgsDataItem *parentItem )
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

      const QgsStringMap urlData( geonodeRequest.fetchServiceUrlDataBlocking( QStringLiteral( "XYZ" ) ) );

      if ( !urlData.isEmpty() )
      {
        auto urlDataIt = urlData.constBegin();
        for ( ; urlDataIt != urlData.constEnd(); ++urlDataIt )
        {
          const QString layerName = urlDataIt.key();
          QgsDebugMsgLevel( urlDataIt.value(), 2 );
          QgsDataSourceUri uri;
          uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
          uri.setParam( QStringLiteral( "url" ), urlDataIt.value() );

          QgsDataItem *item = new QgsXyzLayerItem( parentItem, layerName, path, uri.encodedUri() );
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
