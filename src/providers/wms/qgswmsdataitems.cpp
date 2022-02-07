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

#include "qgsgeonodeconnection.h"
#include "qgsgeonoderequest.h"
#include "qgssettings.h"


// ---------------------------------------------------------------------------
QgsWMSConnectionItem::QgsWMSConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "WMS" ) )
  , mUri( uri )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  mCapabilitiesDownload = new QgsWmsCapabilitiesDownload( false );
}

QgsWMSConnectionItem::~QgsWMSConnectionItem()
{
  delete mCapabilitiesDownload;
}

void QgsWMSConnectionItem::refresh()
{
  mCapabilitiesDownload->setForceRefresh( true );
  QgsDataItem::refresh();
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
    children.append( new QgsErrorItem( this, mCapabilitiesDownload->lastError(), mPath + "/error" ) );
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

    for ( const QgsWmsLayerProperty &layerProperty : std::as_const( capabilityProperty.layers ) )
    {
      // Attention, the name may be empty
      QgsDebugMsgLevel( QString::number( layerProperty.orderId ) + ' ' + layerProperty.name + ' ' + layerProperty.title, 2 );
      QString pathName = layerProperty.name.isEmpty() ? QString::number( layerProperty.orderId ) : layerProperty.name;
      QgsDataItem *layer = nullptr;

      if ( layerProperty.name.isEmpty() || !layerProperty.layer.isEmpty() )
        layer = new QgsWMSLayerCollectionItem( this, layerProperty.title, mPath + '/' + pathName, capabilitiesProperty, uri, layerProperty );
      else
        layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + '/' + pathName, capabilitiesProperty, uri, layerProperty );

      children.append( layer );
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

      QgsDataItem *layerItem = l.styles.size() == 1 ? static_cast<  QgsDataItem * >( this ) : static_cast<  QgsDataItem * >( new QgsWMTSRootItem( this, title, mPath + '/' + l.identifier ) );

      if ( layerItem != this )
      {
        layerItem->setCapabilities( layerItem->capabilities2() & ~ Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile ) );
        layerItem->setState( Qgis::BrowserItemState::Populated );
        layerItem->setToolTip( title );
        children << layerItem;
      }

      for ( const QgsWmtsStyle &style : std::as_const( l.styles ) )
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

        QgsDataItem *styleItem = l.setLinks.size() == 1 ? static_cast<  QgsDataItem * >( layerItem ) : static_cast<  QgsDataItem * >( new QgsWMTSRootItem( layerItem, styleName, layerItem->path() + '/' + stylePathIdentifier ) );

        if ( styleItem != layerItem )
        {
          styleItem->setCapabilities( styleItem->capabilities2() & ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile ) );
          styleItem->setState( Qgis::BrowserItemState::Populated );
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

        for ( const QgsWmtsTileMatrixSetLink &setLink : std::as_const( l.setLinks ) )
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

          QgsDataItem *linkItem = l.formats.size() == 1 ? static_cast<  QgsDataItem * >( styleItem ) : static_cast<  QgsDataItem * >( new QgsWMTSRootItem( styleItem, linkName, styleItem->path() + '/' + linkPathIdentifier ) );

          if ( linkItem != styleItem )
          {
            linkItem->setCapabilities( linkItem->capabilities2() & ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile ) );
            linkItem->setState( Qgis::BrowserItemState::Populated );
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

          for ( const QString &format : std::as_const( l.formats ) )
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
  const QgsWMSConnectionItem *otherConnectionItem = qobject_cast<const QgsWMSConnectionItem *>( other );
  if ( !otherConnectionItem )
  {
    return false;
  }

  bool samePathAndName = ( mPath == otherConnectionItem->mPath && mName == otherConnectionItem->mName );

  if ( samePathAndName )
  {
    // Check if the children are not the same then they are not equal
    if ( mChildren.size() != otherConnectionItem->mChildren.size() )
      return false;

    // compare children content, if the content differs then the parents are not equal
    for ( QgsDataItem *child : mChildren )
    {
      if ( !child )
        continue;
      for ( QgsDataItem *otherChild : otherConnectionItem->mChildren )
      {
        if ( !otherChild )
          continue;
        // In case they have same path, check if they have same content
        if ( child->path() == otherChild->path() )
        {
          if ( !child->equal( otherChild ) )
            return false;
        }
        else
        {
          continue;
        }
      }
    }
  }

  return samePathAndName;
}

// ---------------------------------------------------------------------------
QgsWMSItemBase::QgsWMSItemBase( const QgsWmsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWmsLayerProperty &layerProperty )
  : mCapabilitiesProperty( capabilitiesProperty )
  , mDataSourceUri( dataSourceUri )
  , mLayerProperty( layerProperty )
{
}

QString QgsWMSItemBase::createUri( bool withStyle )
{
  if ( mLayerProperty.name.isEmpty() )
    return QString(); // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( QStringLiteral( "layers" ), mLayerProperty.name );
  QString style = !mLayerProperty.style.isEmpty() ? mLayerProperty.style.at( 0 ).name : QString();
  mDataSourceUri.setParam( QStringLiteral( "styles" ), withStyle ? style : QString() );

  // Check for layer dimensions
  for ( const QgsWmsDimensionProperty &dimension : std::as_const( mLayerProperty.dimensions ) )
  {
    // add temporal dimensions only
    if ( dimension.name == QLatin1String( "time" ) || dimension.name == QLatin1String( "reference_time" ) )
    {
      QString name = dimension.name == QLatin1String( "time" ) ? QString( "timeDimensionExtent" ) : QString( "referenceTimeDimensionExtent" );

      if ( !( mDataSourceUri.param( QLatin1String( "type" ) ) == QLatin1String( "wmst" ) ) )
        mDataSourceUri.setParam( QLatin1String( "type" ), QLatin1String( "wmst" ) );
      mDataSourceUri.setParam( name, dimension.extent );
    }
  }

  // WMS-T defaults settings
  if ( mDataSourceUri.param( QLatin1String( "type" ) ) == QLatin1String( "wmst" ) )
  {
    mDataSourceUri.setParam( QLatin1String( "temporalSource" ), QLatin1String( "provider" ) );
    mDataSourceUri.setParam( QLatin1String( "allowTemporalUpdates" ), QLatin1String( "true" ) );
  }

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
  for ( const QString &c : std::as_const( mLayerProperty.crs ) )
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

QgsWMSLayerCollectionItem::QgsWMSLayerCollectionItem( QgsDataItem *parent, QString name, QString path, const QgsWmsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWmsLayerProperty &layerProperty )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "wms" ) )
  , QgsWMSItemBase( capabilitiesProperty, dataSourceUri, layerProperty )
{
  mIconName = QStringLiteral( "mIconWms.svg" );
  // For collection items we want the default style (empty) so let's strip it
  mUri = createUri( /* withStyle */ false );

  // Populate everything, it costs nothing, all info about layers is collected
  for ( const QgsWmsLayerProperty &layerProperty : std::as_const( mLayerProperty.layer ) )
  {
    // Attention, the name may be empty
    QgsDebugMsgLevel( QString::number( layerProperty.orderId ) + ' ' + layerProperty.name + ' ' + layerProperty.title, 2 );
    QString pathName = layerProperty.name.isEmpty() ? QString::number( layerProperty.orderId ) : layerProperty.name;

    QgsDataItem *layer = nullptr;

    if ( layerProperty.name.isEmpty() || !layerProperty.layer.isEmpty() )
      layer = new QgsWMSLayerCollectionItem( this, layerProperty.title, mPath + '/' + pathName, capabilitiesProperty, dataSourceUri, layerProperty );
    else
      layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + '/' + pathName, mCapabilitiesProperty, dataSourceUri, layerProperty );

    addChildItem( layer );
  }

  setState( Qgis::BrowserItemState::Populated );
}

bool QgsWMSLayerCollectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWMSLayerCollectionItem *otherCollectionItem = qobject_cast<const QgsWMSLayerCollectionItem *>( other );
  if ( !otherCollectionItem )
  {
    return false;
  }

  // Check if the children are not the same then they are not equal
  if ( mChildren.size() != otherCollectionItem->mChildren.size() )
    return false;

  // compare children content, if the content differs then the parents are not equal
  for ( QgsDataItem *child : mChildren )
  {
    if ( !child )
      continue;
    for ( QgsDataItem *otherChild : otherCollectionItem->mChildren )
    {
      if ( !otherChild )
        continue;
      // In case they have same path, check if they have same content
      if ( child->path() == otherChild->path() )
      {
        if ( !child->equal( otherChild ) )
          return false;
      }
      else
      {
        continue;
      }
    }
  }

  return ( mPath == otherCollectionItem->mPath && mName == otherCollectionItem->mName );
}

bool QgsWMSLayerCollectionItem::hasDragEnabled() const
{
  if ( !mLayerProperty.name.isEmpty() )
    return true;
  return false;
}


QgsMimeDataUtils::UriList QgsWMSLayerCollectionItem::mimeUris() const
{
  QgsMimeDataUtils::Uri u;

  u.layerType = QStringLiteral( "raster" );
  u.providerKey = providerKey();
  u.name = name();
  u.uri = mUri;
  u.supportedCrs = mLayerProperty.crs;
  u.supportedFormats = mCapabilitiesProperty.capability.request.getMap.format;

  return { u };
}

// ---------------------------------------------------------------------------

QgsWMSLayerItem::QgsWMSLayerItem( QgsDataItem *parent, QString name, QString path, const QgsWmsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWmsLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), Qgis::BrowserLayerType::Raster, QStringLiteral( "wms" ) )
  ,  QgsWMSItemBase( capabilitiesProperty, dataSourceUri, layerProperty )
{
  mSupportedCRS = mLayerProperty.crs;
  mSupportFormats = mCapabilitiesProperty.capability.request.getMap.format;
  QgsDebugMsgLevel( "uri = " + mDataSourceUri.encodedUri(), 2 );

  mUri = createUri();
  mIconName = QStringLiteral( "mIconWms.svg" );
  setState( Qgis::BrowserItemState::Populated );
}

bool QgsWMSLayerItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWMSLayerItem *otherLayer = qobject_cast<const QgsWMSLayerItem *>( other );
  if ( !otherLayer )
  {
    return false;
  }

  if ( !mLayerProperty.equal( otherLayer->mLayerProperty ) )
    return false;

  return ( mPath == otherLayer->mPath && mName == otherLayer->mName );
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
  : QgsLayerItem( parent, name, path, QString(), Qgis::BrowserLayerType::Raster, QStringLiteral( "wms" ) )
  , mDataSourceUri( uri )
  , mId( id )
  , mFormat( format )
  , mStyle( style )
  , mTileMatrixSet( tileMatrixSet )
  , mCrs( crs )
  , mTitle( title )
{
  mUri = createUri();
  setState( Qgis::BrowserItemState::Populated );
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
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "WMS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
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

QgsWMTSRootItem::QgsWMTSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "WMS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconWms.svg" );
  populate();

}
// ---------------------------------------------------------------------------


QString QgsWmsDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "wms" );
}

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
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "WMS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconXyz.svg" );
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
  : QgsLayerItem( parent, name, path, encodedUri, Qgis::BrowserLayerType::Raster, QStringLiteral( "wms" ) )
{
  mIconName = QStringLiteral( "mIconXyz.svg" );
  setState( Qgis::BrowserItemState::Populated );
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

QString QgsXyzTileDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "wms" );
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


bool QgsWMSLayerCollectionItem::layerCollection() const
{
  return true;
}
