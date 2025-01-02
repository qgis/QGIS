/***************************************************************************
  qgsfilebaseddataitemprovider.cpp
  --------------------------------------
  Date                 : July 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilebaseddataitemprovider.h"
#include "moc_qgsfilebaseddataitemprovider.cpp"
#include "qgsdataprovider.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsogrproviderutils.h"
#include "qgsstyle.h"
#include "qgsgeopackagedataitems.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsfieldsitem.h"
#include "qgsfielddomainsitem.h"
#include "qgsrelationshipsitem.h"
#include "qgsproviderutils.h"
#include "qgsprovidermetadata.h"
#include "qgsgdalutils.h"
#include <QUrlQuery>

//
// QgsProviderSublayerItem
//

QgsProviderSublayerItem::QgsProviderSublayerItem( QgsDataItem *parent, const QString &name,
    const QgsProviderSublayerDetails &details, const QString &filePath )
  : QgsLayerItem( parent, name, filePath.isEmpty() ? details.uri() : filePath, details.uri(), layerTypeFromSublayer( details ), details.providerKey() )
  , mDetails( details )
{
  mToolTip = details.uri();

  // no children, except for vector layers, which will show the fields item
  setState( details.type() == Qgis::LayerType::Vector ? Qgis::BrowserItemState::NotPopulated : Qgis::BrowserItemState::Populated );
}

QVector<QgsDataItem *> QgsProviderSublayerItem::createChildren()
{
  QVector<QgsDataItem *> children;

  if ( mDetails.type() == Qgis::LayerType::Vector )
  {
    // sqlite gets special handling because it delegates to the dedicated spatialite provider
    if ( mDetails.driverName() == QLatin1String( "SQLite" ) )
    {
      children.push_back( new QgsFieldsItem( this,
                                             path() + QStringLiteral( "/columns/ " ),
                                             QStringLiteral( R"(dbname="%1")" ).arg( parent()->path().replace( '"', QLatin1String( R"(\")" ) ) ),
                                             QStringLiteral( "spatialite" ), QString(), name() ) );
    }
    else if ( mDetails.providerKey() == QLatin1String( "ogr" ) )
    {
      // otherwise we use the default OGR database connection approach, which is the generic way to handle this
      // for all OGR layer types
      children.push_back( new QgsFieldsItem( this,
                                             path() + QStringLiteral( "/columns/ " ),
                                             path(),
                                             QStringLiteral( "ogr" ), QString(), name() ) );

      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( databaseConnection() );
      if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RetrieveRelationships ) )
      {
        QString relationError;
        QList< QgsWeakRelation > relations;
        try
        {
          relations = conn->relationships( QString(), mDetails.name() );
        }
        catch ( QgsProviderConnectionException &ex )
        {
          relationError = ex.what();
        }

        if ( !relations.empty() || !relationError.isEmpty() )
        {
          std::unique_ptr< QgsRelationshipsItem > relationsItem = std::make_unique< QgsRelationshipsItem >( this, mPath + "/relations", conn->uri(), QStringLiteral( "ogr" ), QString(), mDetails.name() );
          // force this item to appear last by setting a maximum string value for the sort key
          relationsItem->setSortKey( QString( QChar( 0x11FFFF ) ) );
          children.append( relationsItem.release() );
        }
      }
    }
  }
  return children;
}

QgsProviderSublayerDetails QgsProviderSublayerItem::sublayerDetails() const
{
  return mDetails;
}

QgsAbstractDatabaseProviderConnection *QgsProviderSublayerItem::databaseConnection() const
{
  if ( parent() )
  {
    if ( QgsAbstractDatabaseProviderConnection *connection = parent()->databaseConnection() )
      return connection;
  }

  if ( mDetails.providerKey() == QLatin1String( "ogr" ) )
  {
    if ( QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) )
    {
      QVariantMap parts;
      parts.insert( QStringLiteral( "path" ), path() );
      return static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( md->encodeUri( parts ), {} ) );
    }
  }

  return nullptr;
}

Qgis::BrowserLayerType QgsProviderSublayerItem::layerTypeFromSublayer( const QgsProviderSublayerDetails &sublayer )
{
  switch ( sublayer.type() )
  {
    case Qgis::LayerType::Vector:
    {
      switch ( QgsWkbTypes::geometryType( sublayer.wkbType() ) )
      {
        case Qgis::GeometryType::Point:
          return Qgis::BrowserLayerType::Point;

        case Qgis::GeometryType::Line:
          return Qgis::BrowserLayerType::Line;

        case Qgis::GeometryType::Polygon:
          return Qgis::BrowserLayerType::Polygon;

        case Qgis::GeometryType::Null:
          return Qgis::BrowserLayerType::TableLayer;

        case Qgis::GeometryType::Unknown:
          return Qgis::BrowserLayerType::Vector;
      }

      break;
    }
    case Qgis::LayerType::Raster:
      return Qgis::BrowserLayerType::Raster;

    case Qgis::LayerType::Plugin:
      return Qgis::BrowserLayerType::Plugin;

    case Qgis::LayerType::Mesh:
      return Qgis::BrowserLayerType::Mesh;

    case Qgis::LayerType::VectorTile:
      return Qgis::BrowserLayerType::VectorTile;

    case Qgis::LayerType::PointCloud:
      return Qgis::BrowserLayerType::PointCloud;

    case Qgis::LayerType::TiledScene:
      return Qgis::BrowserLayerType::TiledScene;

    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
      break;
  }
  return Qgis::BrowserLayerType::NoType;
}

QString QgsProviderSublayerItem::layerName() const
{
  return mDetails.name();
}

//
// QgsFileDataCollectionGroupItem
//
QgsFileDataCollectionGroupItem::QgsFileDataCollectionGroupItem( QgsDataItem *parent, const QString &groupName, const QString &path )
  : QgsDataCollectionItem( parent, groupName, path )
{
  mCapabilities = Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed;
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
}

void QgsFileDataCollectionGroupItem::appendSublayer( const QgsProviderSublayerDetails &sublayer )
{
  mSublayers.append( sublayer );
}

bool QgsFileDataCollectionGroupItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::UriList QgsFileDataCollectionGroupItem::mimeUris() const
{
  QgsMimeDataUtils::UriList res;
  res.reserve( mSublayers.size() );

  for ( const QgsProviderSublayerDetails &sublayer : mSublayers )
  {
    res << sublayer.toMimeUri();
  }
  return res;
}

//
// QgsFileDataCollectionItem
//

QgsFileDataCollectionItem::QgsFileDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QList<QgsProviderSublayerDetails> &sublayers, const QVariantMap &extraUriParts )
  : QgsDataCollectionItem( parent, name, path )
  , mSublayers( sublayers )
  , mExtraUriParts( extraUriParts )
{
  if ( QgsProviderUtils::sublayerDetailsAreIncomplete( mSublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount ) )
    setCapabilities( Qgis::BrowserItemCapability::Fertile );
  else
    setCapabilities( Qgis::BrowserItemCapability::Fast | Qgis::BrowserItemCapability::Fertile );

  if ( QgsGdalUtils::vsiHandlerType( QgsGdalUtils::vsiPrefixForPath( path ) ) == Qgis::VsiHandlerType::Archive )
  {
    mIconName = QStringLiteral( "/mIconZip.svg" );
  }
}

QVector<QgsDataItem *> QgsFileDataCollectionItem::createChildren()
{
  QList< QgsProviderSublayerDetails> sublayers;
  if ( QgsProviderUtils::sublayerDetailsAreIncomplete( mSublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount ) )
  {
    QSet< QString > providers;
    for ( const QgsProviderSublayerDetails &details : std::as_const( mSublayers ) )
    {
      providers.insert( details.providerKey() );
    }

    for ( const QString &provider : std::as_const( providers ) )
    {
      if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider ) )
      {
        if ( !mExtraUriParts.empty() )
        {
          QVariantMap uriParts = metadata->decodeUri( path() );
          for ( auto it = mExtraUriParts.constBegin(); it != mExtraUriParts.constEnd(); ++it )
          {
            uriParts.insert( it.key(), it.value() );
          }
          const QString updatedUri = metadata->encodeUri( uriParts );

          sublayers.append( metadata->querySublayers( updatedUri.isEmpty() ? path() : updatedUri, Qgis::SublayerQueryFlag::ResolveGeometryType ) );
        }
        else
        {
          sublayers.append( metadata->querySublayers( path(), Qgis::SublayerQueryFlag::ResolveGeometryType ) );
        }
      }
    }
  }
  else if ( mSublayers.empty() )
  {
    sublayers = QgsProviderRegistry::instance()->querySublayers( path(), Qgis::SublayerQueryFlag::ResolveGeometryType );
  }
  else
  {
    sublayers = mSublayers;
  }
  // only ever use the initial sublayers for first population -- after that we requery when asked to create children,
  // or the item won't "refresh" and update its sublayers when the actual file changes
  mSublayers.clear();
  // remove the fast flag -- after the first population we need to requery the dataset
  setCapabilities( capabilities2() & ~static_cast< int >( Qgis::BrowserItemCapability::Fast ) );

  QVector<QgsDataItem *> children;
  children.reserve( sublayers.size() );
  QMap< QStringList, QgsFileDataCollectionGroupItem * > groupItems;
  for ( const QgsProviderSublayerDetails &sublayer : std::as_const( sublayers ) )
  {
    QgsProviderSublayerItem *item = new QgsProviderSublayerItem( nullptr, sublayer.name(), sublayer, QString() );

    if ( !sublayer.path().isEmpty() )
    {
      QStringList currentPath;
      QStringList remainingPaths = sublayer.path();
      QgsFileDataCollectionGroupItem *groupItem = nullptr;

      while ( !remainingPaths.empty() )
      {
        currentPath << remainingPaths.takeAt( 0 );

        auto it = groupItems.constFind( currentPath );
        if ( it == groupItems.constEnd() )
        {
          QgsFileDataCollectionGroupItem *newGroupItem = new QgsFileDataCollectionGroupItem( this, currentPath.constLast(), path() + '/' + currentPath.join( ',' ) );
          newGroupItem->setState( Qgis::BrowserItemState::Populated );
          groupItems.insert( currentPath, newGroupItem );
          if ( groupItem )
            groupItem->addChildItem( newGroupItem );
          else
            children.append( newGroupItem );
          groupItem = newGroupItem;
        }
        else
        {
          groupItem = it.value();
        }

        if ( groupItem )
          groupItem->appendSublayer( sublayer );
      }

      if ( groupItem )
        groupItem->addChildItem( item );
    }
    else
    {
      children.append( item );
    }
  }

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( databaseConnection() );
  if ( conn )
  {
    mCachedCapabilities = conn->capabilities();
    mCachedCapabilities2 = conn->capabilities2();
    mHasCachedCapabilities = true;
  }
  if ( conn && ( mCachedCapabilities & QgsAbstractDatabaseProviderConnection::Capability::ListFieldDomains ) )
  {
    QString domainError;
    QStringList fieldDomains;
    try
    {
      fieldDomains = conn->fieldDomainNames();
    }
    catch ( QgsProviderConnectionException &ex )
    {
      domainError = ex.what();
    }

    if ( !fieldDomains.empty() || !domainError.isEmpty() )
    {
      std::unique_ptr< QgsFieldDomainsItem > domainsItem = std::make_unique< QgsFieldDomainsItem >( this, mPath + "/domains", conn->uri(), QStringLiteral( "ogr" ) );
      // force this item to appear last by setting a maximum string value for the sort key
      domainsItem->setSortKey( QString( QChar( 0x10FFFF ) ) );
      children.append( domainsItem.release() );
    }
  }
  if ( conn && ( mCachedCapabilities & QgsAbstractDatabaseProviderConnection::Capability::RetrieveRelationships ) )
  {
    QString relationError;
    QList< QgsWeakRelation > relations;
    try
    {
      relations = conn->relationships();
    }
    catch ( QgsProviderConnectionException &ex )
    {
      relationError = ex.what();
    }

    if ( !relations.empty() || !relationError.isEmpty() )
    {
      std::unique_ptr< QgsRelationshipsItem > relationsItem = std::make_unique< QgsRelationshipsItem >( this, mPath + "/relations", conn->uri(), QStringLiteral( "ogr" ) );
      // force this item to appear last by setting a maximum string value for the sort key
      relationsItem->setSortKey( QString( QChar( 0x11FFFF ) ) );
      children.append( relationsItem.release() );
    }
  }

  return children;
}

bool QgsFileDataCollectionItem::hasDragEnabled() const
{
  return true;
}

bool QgsFileDataCollectionItem::canAddVectorLayers() const
{
  // if we've previously opened a connection for this item, we can use the previously
  // determined capababilities to return an accurate answer.
  if ( mHasCachedCapabilities )
    return mCachedCapabilities & QgsAbstractDatabaseProviderConnection::Capability::CreateVectorTable;

  if ( mHasCachedDropSupport )
    return mCachedSupportsDrop;

  // otherwise, we are limited to VERY VERY cheap calculations only!!
  // DO NOT UNDER *****ANY***** CIRCUMSTANCES OPEN DATASETS HERE!!!!

  mHasCachedDropSupport = true;
  if ( !QFileInfo( path() ).isWritable() )
  {
    mCachedSupportsDrop = false;
    return mCachedSupportsDrop;
  }

  GDALDriverH hDriver = GDALIdentifyDriverEx( path().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
  {
    mCachedSupportsDrop = false;
    return mCachedSupportsDrop;
  }

  // explicitly blocklist some drivers which we don't want to expose drop support for
  const QString driverName = GDALGetDriverShortName( hDriver );
  if ( driverName == QLatin1String( "PDF" )
       || driverName == QLatin1String( "DXF" ) )
  {
    mCachedSupportsDrop = false;
    return mCachedSupportsDrop;
  }

  // DO NOT UNDER *****ANY***** CIRCUMSTANCES OPEN DATASETS HERE!!!!
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  const bool isSingleTableDriver = !GDALGetMetadataItem( hDriver, GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, nullptr );
#else
  const QFileInfo pathInfo( path() );
  const QString suffix = pathInfo.suffix().toLower();
  const bool isSingleTableDriver = !QgsGdalUtils::multiLayerFileExtensions().contains( suffix );
#endif

  if ( isSingleTableDriver )
  {
    mCachedSupportsDrop = false;
    return mCachedSupportsDrop;
  }

  // DO NOT UNDER *****ANY***** CIRCUMSTANCES OPEN DATASETS HERE!!!!
  mCachedSupportsDrop = true;
  return mCachedSupportsDrop;
}

QgsMimeDataUtils::UriList QgsFileDataCollectionItem::mimeUris() const
{
  QgsMimeDataUtils::Uri collectionUri;
  collectionUri.uri = path();
  collectionUri.layerType = QStringLiteral( "collection" );
  collectionUri.filePath = path();
  return { collectionUri };
}

QgsAbstractDatabaseProviderConnection *QgsFileDataCollectionItem::databaseConnection() const
{
  // test that file is valid with OGR
  if ( OGRGetDriverCount() == 0 )
  {
    OGRRegisterAll();
  }
  // do not print errors, but write to debug
  CPLPushErrorHandler( CPLQuietErrorHandler );
  CPLErrorReset();
  GDALDriverH hDriver = GDALIdentifyDriverEx( path().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  CPLPopErrorHandler();

  if ( ! hDriver )
  {
    QgsDebugMsgLevel( QStringLiteral( "GDALIdentifyDriverEx error # %1 : %2 on %3" ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ).arg( path() ), 2 );
    return nullptr;
  }

  const QString driverName = GDALGetDriverShortName( hDriver );
  if ( driverName == QLatin1String( "PDF" )
       || driverName == QLatin1String( "DXF" ) )
  {
    // unwanted drivers -- it's slow to create connections for these, and we don't really want
    // to expose database capabilities for them (even though they kind of are database formats)
    return nullptr;
  }

  QgsAbstractDatabaseProviderConnection *conn = nullptr;
  if ( driverName == QLatin1String( "SQLite" ) )
  {
    // sqlite gets special handling, as we delegate to the native spatialite provider
    if ( QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) ) )
    {
      QgsDataSourceUri uri;
      uri.setDatabase( path( ) );
      conn = static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri.uri(), {} ) );
    }
  }
  else
  {
    // for all other vector types we use the generic OGR provider
    if ( QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) )
    {
      QVariantMap parts;
      parts.insert( QStringLiteral( "path" ), path() );
      conn = static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( md->encodeUri( parts ), {} ) );
    }
  }

  if ( conn )
  {
    mCachedCapabilities = conn->capabilities();
    mCachedCapabilities2 = conn->capabilities2();
    mHasCachedCapabilities = true;
  }

  return conn;
}

QgsAbstractDatabaseProviderConnection::Capabilities QgsFileDataCollectionItem::databaseConnectionCapabilities() const
{
  if ( mHasCachedCapabilities )
    return mCachedCapabilities;

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( databaseConnection() );
  if ( conn )
  {
    mCachedCapabilities = conn->capabilities();
    mCachedCapabilities2 = conn->capabilities2();
    mHasCachedCapabilities = true;
  }
  return mCachedCapabilities;
}

Qgis::DatabaseProviderConnectionCapabilities2 QgsFileDataCollectionItem::databaseConnectionCapabilities2() const
{
  if ( mHasCachedCapabilities )
    return mCachedCapabilities2;

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( databaseConnection() );
  if ( conn )
  {
    mCachedCapabilities = conn->capabilities();
    mCachedCapabilities2 = conn->capabilities2();
    mHasCachedCapabilities = true;
  }
  return mCachedCapabilities2;
}

QList<QgsProviderSublayerDetails> QgsFileDataCollectionItem::sublayers() const
{
  return mSublayers;
}

//
// QgsFileBasedDataItemProvider
//

QString QgsFileBasedDataItemProvider::name()
{
  return QStringLiteral( "files" );
}

Qgis::DataItemProviderCapabilities QgsFileBasedDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::Files | Qgis::DataItemProviderCapability::Directories;
}

QgsDataItem *QgsFileBasedDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  return createDataItemForPathPrivate( path, parentItem, nullptr, Qgis::SublayerQueryFlags(), QVariantMap() );
}

QgsDataItem *QgsFileBasedDataItemProvider::createLayerItemForPath( const QString &path, QgsDataItem *parentItem, const QStringList &allowedProviders, const QVariantMap &extraUriParts, Qgis::SublayerQueryFlags queryFlags )
{
  return createDataItemForPathPrivate( path, parentItem, &allowedProviders, queryFlags, extraUriParts );
}

QgsDataItem *QgsFileBasedDataItemProvider::createDataItemForPathPrivate( const QString &path, QgsDataItem *parentItem, const QStringList *allowedProviders, Qgis::SublayerQueryFlags queryFlags, const QVariantMap &extraUriParts )
{
  if ( path.isEmpty() )
    return nullptr;

  const QFileInfo info( path );
  QString suffix = info.suffix().toLower();
  const QString name = info.fileName();

  // special handling for some suffixes
  if ( suffix.compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0 )
  {
    // Geopackage is special -- it gets a dedicated collection item type
    QgsGeoPackageCollectionItem *item = new QgsGeoPackageCollectionItem( parentItem, name, path );
    item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
    return item;
  }
  else if ( suffix == QLatin1String( "txt" ) )
  {
    // never ever show .txt files as datasets in browser -- they are only used for geospatial data in extremely rare cases
    // and are predominantly just noise in the browser
    return nullptr;
  }
  // If a .tab exists, then the corresponding .map/.dat is very likely a
  // side-car file of the .tab
  else if ( suffix == QLatin1String( "map" ) || suffix == QLatin1String( "dat" ) )
  {
    if ( QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".tab" ) ) || QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".TAB" ) ) )
      return nullptr;
  }
  // .dbf and .shx should only appear if .shp is not present
  else if ( suffix == QLatin1String( "dbf" ) || suffix == QLatin1String( "shx" ) )
  {
    if ( QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".shp" ) ) || QFile::exists( QDir( info.path() ).filePath( info.baseName() + ".SHP" ) ) )
      return nullptr;
  }
  // skip QGIS style xml files
  else if ( suffix == QLatin1String( "xml" ) && QgsStyle::isXmlStyleFile( path ) )
  {
    return nullptr;
  }
  // GDAL 3.1 Shapefile driver directly handles .shp.zip files
  else if ( path.endsWith( QLatin1String( ".shp.zip" ), Qt::CaseInsensitive ) &&
            GDALIdentifyDriverEx( path.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr ) )
  {
    suffix = QStringLiteral( "shp.zip" );
  }

  // hide blocklisted URIs, such as .aux.xml files
  if ( QgsProviderRegistry::instance()->uriIsBlocklisted( path ) )
    return nullptr;

  QgsSettings settings;

  // should we fast scan only?
  if ( ( settings.value( QStringLiteral( "qgis/scanItemsInBrowser2" ),
                         "extension" ).toString() == QLatin1String( "extension" ) ) ||
       ( parentItem && settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                                       QStringList() ).toStringList().contains( parentItem->path() ) ) )
  {
    queryFlags |= Qgis::SublayerQueryFlag::FastScan;
  }

  QList<QgsProviderSublayerDetails> sublayers;
  if ( !allowedProviders )
  {
    sublayers = QgsProviderRegistry::instance()->querySublayers( path, queryFlags );
  }
  else
  {
    for ( const QString &provider : *allowedProviders )
    {
      if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider ) )
      {
        if ( !extraUriParts.empty() )
        {
          QVariantMap uriParts = metadata->decodeUri( path );
          for ( auto it = extraUriParts.constBegin(); it != extraUriParts.constEnd(); ++it )
          {
            uriParts.insert( it.key(), it.value() );
          }

          sublayers.append( metadata->querySublayers( metadata->encodeUri( uriParts ), queryFlags ) );
        }
        else
        {
          sublayers.append( metadata->querySublayers( path, queryFlags ) );
        }
      }
    }
  }

  if ( sublayers.size() == 1
       && ( ( ( queryFlags & Qgis::SublayerQueryFlag::FastScan ) && !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount | QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownGeometryType ) )
            || ( !( queryFlags & Qgis::SublayerQueryFlag::FastScan ) && !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers, QgsProviderUtils::SublayerCompletenessFlag::IgnoreUnknownFeatureCount ) ) )
     )
  {
    QgsProviderSublayerItem *item = new QgsProviderSublayerItem( parentItem, name, sublayers.at( 0 ), path );
    item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
    return item;
  }
  else if ( !sublayers.empty() )
  {
    QgsFileDataCollectionItem *item = new QgsFileDataCollectionItem( parentItem, name, path, sublayers, extraUriParts );
    item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
    return item;
  }
  else
  {
    return nullptr;
  }
}

bool QgsFileBasedDataItemProvider::handlesDirectoryPath( const QString &path )
{
  QFileInfo info( path );
  QString suffix = info.suffix().toLower();

  QStringList dirExtensions = QgsOgrProviderUtils::directoryExtensions();
  return dirExtensions.contains( suffix );
}
