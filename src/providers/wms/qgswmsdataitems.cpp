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

#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgswmscapabilities.h"
#include "qgswmsconnection.h"
#include "qgsxyzconnection.h"

#include "moc_qgswmsdataitems.cpp"

// ---------------------------------------------------------------------------
QgsWMSConnectionItem::QgsWMSConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path, u"WMS"_s )
  , mUri( uri )
{
  mIconName = u"mIconConnect.svg"_s;
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

  const QString defaultImageFormat = QgsOwsConnection::settingsDefaultImageFormat->value( { u"wms"_s, name() } );

  // Make sure the format is in the capabilities formats, otherwise use first available
  int imageFormatIndex { 0 };
  const QStringList supportedFormats { caps.supportedImageEncodings() };
  for ( int i = 0; i < supportedFormats.size(); ++i )
  {
    const QString &format = supportedFormats.at( i );
    if ( format.contains( defaultImageFormat, Qt::CaseInsensitive ) )
    {
      imageFormatIndex = i;
      break;
    }
  }

  if ( !supportedFormats.empty() )
    uri.setParam( u"format"_s, supportedFormats.at( imageFormatIndex ) );

  int layerIndex { 0 };

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
      {
        layer = new QgsWMSLayerCollectionItem( this, layerProperty.title, mPath + '/' + pathName, capabilitiesProperty, uri, layerProperty );
      }
      else
      {
        layer = new QgsWMSLayerItem( this, layerProperty.title, mPath + '/' + pathName, capabilitiesProperty, uri, layerProperty );
      }

      layer->setSortKey( layerIndex++ );

      if ( !layerProperty.abstract.isEmpty() )
      {
        layer->setToolTip( layerProperty.abstract );
      }

      children.append( layer );
    }
  }

  QSet<QString> styleIdentifiers;
  QSet<QString> dimensionIdentifiers;
  QSet<QString> dimensionValueIdentifiers;
  QSet<QString> linkIdentifiers;

  const QList<QgsWmtsTileLayer> tileLayers = caps.supportedTileLayers();
  if ( !tileLayers.isEmpty() )
  {
    const QHash<QString, QgsWmtsTileMatrixSet> tileMatrixSets = caps.supportedTileMatrixSets();

    for ( const QgsWmtsTileLayer &l : tileLayers )
    {
      const QString title = l.title.isEmpty() ? l.identifier : l.title;

      QHash<QString, QgsWmtsDimension> dimensions;
      bool hasTimeDimension = false;
      for ( auto it = l.dimensions.constBegin(); it != l.dimensions.constEnd(); ++it )
      {
        if ( it.key().compare( "time"_L1, Qt::CaseInsensitive ) == 0 && !it.value().values.empty() )
        {
          // we will use temporal framework if there's multiple time dimension values, OR if a single time dimension value is itself an interval
          if ( it.value().values.size() > 1 )
          {
            hasTimeDimension = true;
          }
          else
          {
            const thread_local QRegularExpression rxPeriod( u".*/P.*"_s );
            const QRegularExpressionMatch match = rxPeriod.match( it.value().values.constFirst() );
            if ( match.hasMatch() )
            {
              hasTimeDimension = true;
            }
          }

          if ( hasTimeDimension )
            continue; // time dimension gets special handling by temporal framework
        }

        dimensions.insert( it.key(), it.value() );
      }

      QgsDataItem *dimensionItem = ( dimensions.empty() || ( dimensions.size() == 1 && dimensions.constBegin()->values.size() < 2 ) ) ? qobject_cast<QgsDataItem *>( this ) : new QgsWMTSRootItem( this, title, mPath + '/' + l.identifier );
      if ( dimensionItem != this )
      {
        dimensionItem->setCapabilities( dimensionItem->capabilities2() & ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile ) );
        dimensionItem->setState( Qgis::BrowserItemState::Populated );
        dimensionItem->setToolTip( title );
        children << dimensionItem;
      }

      QStringList dimensionIds = dimensions.keys();
      std::sort( dimensionIds.begin(), dimensionIds.end(), []( const QString &a, const QString &b ) -> bool {
        return QString::localeAwareCompare( a, b ) < 0;
      } );

      if ( dimensionIds.empty() )
      {
        // if no dimensions present on service, we add a blank one just to keep the below loops manageable!
        dimensionIds.append( QString() );
      }

      for ( const QString &dimensionId : std::as_const( dimensionIds ) )
      {
        const QgsWmtsDimension dimension = dimensions.value( dimensionId );
        QString dimensionName = dimension.title.isEmpty() ? dimension.identifier : dimension.title;
        if ( dimensionItem == this )
          dimensionName = title; // just one dimension so no need to display it

        // Ensure dimension path is unique
        QString dimensionPathIdentifier { dimension.identifier };
        int i = 0;
        while ( dimensionIdentifiers.contains( dimensionPathIdentifier ) )
        {
          dimensionPathIdentifier = u"%1_%2"_s.arg( dimension.identifier ).arg( ++i );
        }
        dimensionIdentifiers.insert( dimensionPathIdentifier );

        QStringList dimensionValues = dimension.values;

        QgsDataItem *dimensionValueItem = dimensionValues.size() < 2 ? static_cast<QgsDataItem *>( dimensionItem ) : static_cast<QgsDataItem *>( new QgsWMTSRootItem( this, dimensionName, dimensionItem->path() + '/' + dimensionPathIdentifier ) );

        if ( dimensionValueItem != dimensionItem )
        {
          dimensionValueItem->setCapabilities( dimensionItem->capabilities2() & ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile ) );
          dimensionValueItem->setState( Qgis::BrowserItemState::Populated );
          dimensionValueItem->setToolTip( dimensionName );
          if ( dimensionItem == this )
          {
            children << dimensionValueItem;
          }
          else
          {
            dimensionItem->addChildItem( dimensionValueItem );
          }
        }

        if ( dimensionValues.empty() )
        {
          // if no dimension values present on service, we add a blank one just to keep the below loops manageable!
          dimensionValues << QString();
        }

        // iterate through available dimension values
        for ( const QString &dimensionValue : std::as_const( dimensionValues ) )
        {
          QString dimensionValueTitle = dimensionValue;
          if ( dimensionValueItem == this )
            dimensionValueTitle = dimensionName; // just one dimension value so no need to display it

          // Ensure dimension value path is unique
          QString dimensionValuePathIdentifier = u"%1_%2"_s.arg( dimension.identifier, dimensionValue );
          int i = 0;
          while ( dimensionValueIdentifiers.contains( dimensionValuePathIdentifier ) )
          {
            dimensionValuePathIdentifier = u"%1_%2_%3"_s.arg( dimension.identifier, dimensionValue ).arg( ++i );
          }
          dimensionValueIdentifiers.insert( dimensionValuePathIdentifier );

          QgsDataItem *layerItem = l.styles.size() == 1 ? static_cast<QgsDataItem *>( dimensionValueItem ) : static_cast<QgsDataItem *>( new QgsWMTSRootItem( this, dimensionValueTitle, dimensionValueItem->path() + '/' + dimensionValuePathIdentifier ) );

          if ( layerItem != dimensionValueItem )
          {
            layerItem->setCapabilities( layerItem->capabilities2() & ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile ) );
            layerItem->setState( Qgis::BrowserItemState::Populated );
            layerItem->setToolTip( dimensionValueTitle );
            if ( dimensionValueItem == this )
            {
              children << layerItem;
            }
            else
            {
              dimensionValueItem->addChildItem( layerItem );
            }
          }

          for ( const QgsWmtsStyle &style : std::as_const( l.styles ) )
          {
            QString styleName = style.title.isEmpty() ? style.identifier : style.title;
            if ( layerItem == dimensionValueItem )
              styleName = dimensionValueTitle; // just one style so no need to display it

            // Ensure style path is unique
            QString stylePathIdentifier { style.identifier };
            int i = 0;
            while ( styleIdentifiers.contains( stylePathIdentifier ) )
            {
              stylePathIdentifier = u"%1_%2"_s.arg( style.identifier ).arg( ++i );
            }
            styleIdentifiers.insert( stylePathIdentifier );

            QgsDataItem *styleItem = l.setLinks.size() == 1 ? static_cast<QgsDataItem *>( layerItem ) : static_cast<QgsDataItem *>( new QgsWMTSRootItem( layerItem, styleName, layerItem->path() + '/' + stylePathIdentifier ) );

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
                linkName = styleName; // just one link so no need to display it

              // Ensure link path is unique
              QString linkPathIdentifier { linkName };
              int i = 0;
              while ( linkIdentifiers.contains( linkPathIdentifier ) )
              {
                linkPathIdentifier = u"%1_%2"_s.arg( linkName ).arg( ++i );
              }
              linkIdentifiers.insert( linkPathIdentifier );

              QgsDataItem *linkItem = l.formats.size() == 1 ? static_cast<QgsDataItem *>( styleItem ) : static_cast<QgsDataItem *>( new QgsWMTSRootItem( styleItem, linkName, styleItem->path() + '/' + linkPathIdentifier ) );

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
                  name = linkName; // just one format so no need to display it

                QgsDataItem *tileLayerItem = new QgsWMTSLayerItem( linkItem, name, linkItem->path() + '/' + name, uri, l.identifier, dimensionId, dimensionValue, format, style.identifier, setLink.tileMatrixSet, tileMatrixSets[setLink.tileMatrixSet].crs, title );
                tileLayerItem->setToolTip( name );
                tileLayerItem->setSortKey( ++layerIndex );


                if ( hasTimeDimension )
                {
                  tileLayerItem->setIcon( QgsApplication::getThemeIcon( u"/mIconTemporalRaster.svg"_s ) );
                }

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
  mDataSourceUri.setParam( u"layers"_s, mLayerProperty.name );
  QString style = !mLayerProperty.style.isEmpty() ? mLayerProperty.style.at( 0 ).name : QString();
  mDataSourceUri.setParam( u"styles"_s, withStyle ? style : QString() );

  // Check for layer dimensions
  for ( const QgsWmsDimensionProperty &dimension : std::as_const( mLayerProperty.dimensions ) )
  {
    // add temporal dimensions only
    if ( dimension.name == "time"_L1 || dimension.name == "reference_time"_L1 )
    {
      QString name = dimension.name == "time"_L1 ? QString( "timeDimensionExtent" ) : QString( "referenceTimeDimensionExtent" );

      if ( !( mDataSourceUri.param( "type"_L1 ) == "wmst"_L1 ) )
        mDataSourceUri.setParam( "type"_L1, "wmst"_L1 );
      mDataSourceUri.setParam( name, dimension.extent );
    }
  }

  // WMS-T defaults settings
  if ( mDataSourceUri.param( "type"_L1 ) == "wmst"_L1 )
  {
    mDataSourceUri.setParam( "temporalSource"_L1, "provider"_L1 );
    mDataSourceUri.setParam( "allowTemporalUpdates"_L1, "true"_L1 );
  }

  const QString projectCrs = QgsProject::instance()->crs().authid();
  QString crs;
  // if project CRS is supported then use it, otherwise use first available CRS
  if ( !mLayerProperty.crs.isEmpty() )
  {
    QgsCoordinateReferenceSystem testCrs;
    for ( const QString &c : std::as_const( mLayerProperty.crs ) )
    {
      testCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( c );
      if ( testCrs.authid().compare( projectCrs, Qt::CaseInsensitive ) == 0 )
      {
        crs = projectCrs;
        break;
      }
    }

    if ( crs.isEmpty() )
    {
      crs = mLayerProperty.crs[0];
    }
  }

  mDataSourceUri.setParam( u"crs"_s, crs );

  // Set default featureCount to 10, old connections might miss this
  // setting.
  if ( !mDataSourceUri.hasParam( u"featureCount"_s ) )
  {
    mDataSourceUri.setParam( u"featureCount"_s, u"10"_s );
  }

  //uri = rasterLayerPath + "|layers=" + layers.join( "," ) + "|styles=" + styles.join( "," ) + "|format=" + format + "|crs=" + crs;

  return mDataSourceUri.encodedUri();
}


// ---------------------------------------------------------------------------

QgsWMSLayerCollectionItem::QgsWMSLayerCollectionItem( QgsDataItem *parent, QString name, QString path, const QgsWmsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWmsLayerProperty &layerProperty )
  : QgsDataCollectionItem( parent, name, path, u"wms"_s )
  , QgsWMSItemBase( capabilitiesProperty, dataSourceUri, layerProperty )
{
  mIconName = u"mIconWms.svg"_s;
  mUri = createUri( /* withStyle */ false );

  int layerIndex { 0 };

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

    if ( !layerProperty.abstract.isEmpty() )
    {
      layer->setToolTip( layerProperty.abstract );
    }

    layer->setSortKey( layerIndex++ );
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

  u.layerType = u"raster"_s;
  u.providerKey = providerKey();
  u.name = name();
  u.uri = mUri;
  u.supportedCrs = mLayerProperty.crs;
  u.supportedFormats = mCapabilitiesProperty.capability.request.getMap.format;

  return { u };
}

// ---------------------------------------------------------------------------

QgsWMSLayerItem::QgsWMSLayerItem( QgsDataItem *parent, QString name, QString path, const QgsWmsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWmsLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), Qgis::BrowserLayerType::Raster, u"wms"_s )
  , QgsWMSItemBase( capabilitiesProperty, dataSourceUri, layerProperty )
{
  mSupportedCRS = mLayerProperty.crs;
  mSupportFormats = mCapabilitiesProperty.capability.request.getMap.format;
  QgsDebugMsgLevel( "uri = " + mDataSourceUri.encodedUri(), 2 );

  mUri = createUri();

  mIconName = mDataSourceUri.param( "type"_L1 ) == "wmst"_L1 ? u"mIconTemporalRaster.svg"_s : u"mIconRaster.svg"_s;
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

QgsWMTSLayerItem::QgsWMTSLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QgsDataSourceUri &uri, const QString &id, const QString &dimension, const QString &dimensionValue, const QString &format, const QString &style, const QString &tileMatrixSet, const QString &crs, const QString &title )
  : QgsLayerItem( parent, name, path, QString(), Qgis::BrowserLayerType::Raster, u"wms"_s )
  , mDataSourceUri( uri )
  , mId( id )
  , mDimension( dimension )
  , mDimensionValue( dimensionValue )
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
  QgsDataSourceUri uri( mDataSourceUri );
  uri.setParam( u"layers"_s, mId );
  uri.setParam( u"styles"_s, mStyle );
  uri.setParam( u"format"_s, mFormat );
  uri.setParam( u"crs"_s, mCrs );
  uri.setParam( u"tileMatrixSet"_s, mTileMatrixSet );

  if ( !mDimension.isEmpty() && !mDimensionValue.isEmpty() )
    uri.setParam( u"tileDimensions"_s, u"%1=%2"_s.arg( mDimension, mDimensionValue ) );

  return uri.encodedUri();
}

// ---------------------------------------------------------------------------
QgsWMSRootItem::QgsWMSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, u"WMS"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconWms.svg"_s;
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
  : QgsConnectionsRootItem( parent, name, path, u"WMS"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconWms.svg"_s;
  populate();
}
// ---------------------------------------------------------------------------


QString QgsWmsDataItemProvider::dataProviderKey() const
{
  return u"wms"_s;
}

QgsDataItem *QgsWmsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "path = " + path, 2 );
  if ( path.isEmpty() )
  {
    return new QgsWMSRootItem( parentItem, u"WMS/WMTS"_s, u"wms:"_s );
  }

  // path schema: wms:/connection name (used by OWS)
  if ( path.startsWith( "wms:/"_L1 ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsWMSConnection::connectionList().contains( connectionName ) )
    {
      QgsWMSConnection connection( connectionName );
      return new QgsWMSConnectionItem( parentItem, u"WMS/WMTS"_s, path, connection.uri().encodedUri() );
    }
  }

  return nullptr;
}

// ---------------------------------------------------------------------------


QgsXyzTileRootItem::QgsXyzTileRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, u"WMS"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconXyz.svg"_s;
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
  : QgsLayerItem( parent, name, path, encodedUri, Qgis::BrowserLayerType::Raster, u"wms"_s )
{
  mIconName = u"mIconXyz.svg"_s;
  setState( Qgis::BrowserItemState::Populated );
}


// ---------------------------------------------------------------------------


QString QgsXyzTileDataItemProvider::name()
{
  return u"XYZ Tiles"_s;
}

QString QgsXyzTileDataItemProvider::dataProviderKey() const
{
  return u"wms"_s;
}

Qgis::DataItemProviderCapabilities QgsXyzTileDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsXyzTileDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsXyzTileRootItem( parentItem, QObject::tr( "XYZ Tiles" ), u"xyz:"_s );
  return nullptr;
}

bool QgsWMSLayerCollectionItem::layerCollection() const
{
  return true;
}
