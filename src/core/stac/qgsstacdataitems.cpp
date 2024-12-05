/***************************************************************************
    qgsstacdataitems.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacdataitems.h"
#include "moc_qgsstacdataitems.cpp"
#include "qgsstacconnection.h"
#include "qgsstaccontroller.h"
#include "qgsstaccatalog.h"
#include "qgsstacitem.h"
#include "qgsstacitemcollection.h"
#include "qgsstaccollection.h"
#include "qgsstaccollections.h"


constexpr int MAX_DISPLAYED_ITEMS = 20;


//
// QgsStacFetchMoreItem
//

QgsStacFetchMoreItem::QgsStacFetchMoreItem( QgsDataItem *parent, const QString &name )
  : QgsDataItem( Qgis::BrowserItemType::Custom,
                 parent,
                 name,
                 QString() )
{
  mState = Qgis::BrowserItemState::Populated;
}

bool QgsStacFetchMoreItem::handleDoubleClick()
{
  if ( QgsStacCatalogItem *catalog = qobject_cast<QgsStacCatalogItem *>( mParent ) )
  {
    catalog->fetchMoreChildren();
    return true;
  }
  else
  {
    return false;
  }
}


//
// QgsStacItemItem
//

QgsStacItemItem::QgsStacItemItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name.isEmpty() ? path : name, path, QStringLiteral( "special:Stac" ) )
{
  mIconName = QStringLiteral( "mActionPropertiesWidget.svg" );
  updateToolTip();
}

QVector<QgsDataItem *> QgsStacItemItem::createChildren()
{
  QgsStacController *controller = stacController();
  QString error;
  QgsStacObject *obj = controller->fetchStacObject( mPath, &error );
  QgsStacItem *item = dynamic_cast<QgsStacItem *>( obj );
  setStacItem( item );

  if ( !mStacItem )
    return { new QgsErrorItem( this, error, path() + QStringLiteral( "/error" ) ) };

  return {};
}

bool QgsStacItemItem::hasDragEnabled() const
{
  if ( !mStacItem )
    return false;

  const QMap<QString, QgsStacAsset> assets = mStacItem->assets();
  for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
  {
    if ( it->isCloudOptimized() )
      return true;
  }
  return false;
}

QgsMimeDataUtils::UriList QgsStacItemItem::mimeUris() const
{
  QgsMimeDataUtils::UriList uris;

  if ( !mStacItem )
    return uris;

  const QMap<QString, QgsStacAsset> assets = mStacItem->assets();
  for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
  {
    QgsMimeDataUtils::Uri uri;
    QUrl url( it->href() );
    if ( url.isLocalFile() )
    {
      uri.uri = it->href();
    }
    else if ( it->mediaType() == QLatin1String( "image/tiff; application=geotiff; profile=cloud-optimized" ) ||
              it->mediaType() == QLatin1String( "image/vnd.stac.geotiff; cloud-optimized=true" ) )
    {
      uri.layerType = QStringLiteral( "raster" );
      uri.providerKey = QStringLiteral( "gdal" );
      if ( it->href().startsWith( QLatin1String( "http" ), Qt::CaseInsensitive ) ||
           it->href().startsWith( QLatin1String( "ftp" ), Qt::CaseInsensitive ) )
      {
        uri.uri = QStringLiteral( "/vsicurl/%1" ).arg( it->href() );
      }
      else if ( it->href().startsWith( QLatin1String( "s3://" ), Qt::CaseInsensitive ) )
      {
        uri.uri = QStringLiteral( "/vsis3/%1" ).arg( it->href().mid( 5 ) );
      }
      else
      {
        uri.uri = it->href();
      }
    }
    else if ( it->mediaType() == QLatin1String( "application/vnd.laszip+copc" ) )
    {
      uri.layerType = QStringLiteral( "pointcloud" );
      uri.providerKey = QStringLiteral( "copc" );
      uri.uri = it->href();
    }
    else if ( it->href().endsWith( QLatin1String( "/ept.json" ) ) )
    {
      uri.layerType = QStringLiteral( "pointcloud" );
      uri.providerKey = QStringLiteral( "ept" );
      uri.uri = it->href();
    }
    uri.name = it->title().isEmpty() ? url.fileName() : it->title();
    uris.append( uri );
  }

  return uris;
}

bool QgsStacItemItem::equal( const QgsDataItem * )
{
  return false;
}

void QgsStacItemItem::updateToolTip()
{
  QString name = mName;
  if ( mStacItem )
  {
    name = mStacItem->properties().value( QStringLiteral( "title" ), mName ).toString();
  }
  mToolTip = QStringLiteral( "STAC Item:\n%1\n%2" ).arg( name, mPath );
}

QgsStacController *QgsStacItemItem::stacController()
{
  QgsDataItem *item = this;
  while ( item )
  {
    if ( QgsStacConnectionItem *ci = qobject_cast<QgsStacConnectionItem *>( item ) )
      return ci->controller();
    item = item->parent();
  }
  Q_ASSERT( false );
  return nullptr;
}

void QgsStacItemItem::setStacItem( QgsStacItem *item )
{
  mStacItem.reset( item );
  updateToolTip();
}

QgsStacItem *QgsStacItemItem::stacItem() const
{
  return mStacItem.get();
}

void QgsStacItemItem::itemRequestFinished( int requestId, QString error )
{
  QgsStacController *controller = stacController();
  QgsStacObject *object = controller->takeStacObject( requestId );
  QgsStacItem *item = dynamic_cast< QgsStacItem * >( object );
  setStacItem( item );
  if ( item )
  {
    mIconName = QStringLiteral( "mActionPropertiesWidget.svg" );
    QString name = item->properties().value( QStringLiteral( "title" ), QString() ).toString();
    if ( name.isEmpty() )
      name = item->id();
    mName = name;
  }
  else
  {
    mIconName = QStringLiteral( "/mIconDelete.svg" );
    mName = error;
  }
  setState( Qgis::BrowserItemState::Populated );
}


//
// QgsStacCatalogItem
//

QgsStacCatalogItem::QgsStacCatalogItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "special:Stac" ) )
{
  mIconName = QStringLiteral( "mIconFolder.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  if ( name.isEmpty() )
  {
    mName = path.mid( path.lastIndexOf( '/' ) + 1 );
  }

  updateToolTip();
}

bool QgsStacCatalogItem::isCatalog() const
{
  return !mIsCollection;
}

bool QgsStacCatalogItem::isCollection() const
{
  return mIsCollection;
}

QgsStacController *QgsStacCatalogItem::stacController() const
{
  const QgsDataItem *item = this;
  while ( item )
  {
    if ( const QgsStacConnectionItem *ci = qobject_cast< const QgsStacConnectionItem *>( item ) )
      return ci->controller();
    item = item->parent();
  }
  Q_ASSERT( false );
  return nullptr;
}

QgsStacCatalog *QgsStacCatalogItem::rootCatalog() const
{
  const QgsDataItem *item = this;
  while ( item )
  {
    if ( const QgsStacConnectionItem *ci = qobject_cast< const QgsStacConnectionItem *>( item ) )
      return ci->mStacCatalog.get();
    item = item->parent();
  }
  Q_ASSERT( false );
  return nullptr;
}

QgsStacFetchMoreItem *QgsStacCatalogItem::fetchMoreItem() const
{
  for ( QgsDataItem *item : mChildren )
  {
    if ( QgsStacFetchMoreItem *moreItem = qobject_cast< QgsStacFetchMoreItem *>( item ) )
    {
      return moreItem;
    }
  }
  return nullptr;
}

void QgsStacCatalogItem::childrenCreated()
{
  QgsDataItem::childrenCreated();

  connect( stacController(), &QgsStacController::finishedStacObjectRequest, this, &QgsStacCatalogItem::onControllerFinished, Qt::UniqueConnection );

  for ( QgsDataItem *child : std::as_const( mChildren ) )
  {
    if ( QgsStacItemItem *item = qobject_cast<QgsStacItemItem *>( child ) )
    {
      if ( item->state() != Qgis::BrowserItemState::NotPopulated )
        continue;

      item->setIconName( QStringLiteral( "mActionIdentify.svg" ) );
      const int requestId = stacController()->fetchStacObjectAsync( item->path() );
      item->setProperty( "requestId", requestId );
    }
  }
}

void QgsStacCatalogItem::onControllerFinished( int requestId, const QString &error )
{
  for ( auto child : std::as_const( mChildren ) )
  {
    if ( child->state() != Qgis::BrowserItemState::NotPopulated )
      continue;

    if ( QgsStacItemItem *item = qobject_cast<QgsStacItemItem *>( child ) )
    {
      if ( item->property( "requestId" ).toInt() == requestId )
      {
        item->itemRequestFinished( requestId, error );
      }
    }
  }
}

QVector<QgsDataItem *> QgsStacCatalogItem::createChildren()
{
  QgsStacCatalog *root = rootCatalog();
  const bool supportsApi = root ? root->supportsStacApi() : false;

  QgsStacController *controller = stacController();
  QString error;
  QgsStacObject *obj = controller->fetchStacObject( mPath, &error );
  QgsStacCatalog *cat = dynamic_cast<QgsStacCatalog *>( obj );
  setStacCatalog( cat );

  if ( !mStacCatalog )
    return { new QgsErrorItem( this, error, path() + QStringLiteral( "/error" ) ) };

  int itemsCount = 0;
  QVector<QgsDataItem *> contents;
  const QVector< QgsStacLink > links = mStacCatalog->links();

  // treat catalog/collection as static if it does not have a /items endpoint
  bool hasItemsEndpoint = false;
  bool hasCollectionsEndpoint = false;
  if ( supportsApi )
  {
    for ( const auto &link : links )
    {
      if ( link.relation() == QLatin1String( "items" ) )
      {
        hasItemsEndpoint = true;
      }
      else if ( link.relation() == QLatin1String( "data" ) &&
                link.href().endsWith( QLatin1String( "/collections" ) ) )
      {
        hasCollectionsEndpoint = true;
      }
      if ( hasItemsEndpoint && hasCollectionsEndpoint )
        break;
    }
  }

  for ( const auto &link : links )
  {
    // skip hierarchical navigation links
    if ( link.relation() == QLatin1String( "self" ) ||
         link.relation() == QLatin1String( "root" ) ||
         link.relation() == QLatin1String( "parent" ) ||
         link.relation() == QLatin1String( "collection" ) )
      continue;

    if ( link.relation() == QLatin1String( "child" ) &&
         !hasCollectionsEndpoint )
    {
      // may be either catalog or collection
      QgsStacCatalogItem *c = new QgsStacCatalogItem( this, link.title(), link.href() );
      contents.append( c );
    }
    else if ( link.relation() == QLatin1String( "data" ) &&
              link.href().endsWith( QLatin1String( "/collections" ) ) )
    {
      // use /collections api
      QString error;
      std::unique_ptr< QgsStacCollections > cols( controller->fetchCollections( link.href(), &error ) );
      if ( cols )
      {
        contents.append( createCollections( cols->takeCollections() ) );
        itemsCount = cols->numberMatched();
      }
      else
      {
        // collection fetching failed
        contents.append( new QgsErrorItem( this, error, path() + QStringLiteral( "/error" ) ) );
      }
    }
    else if ( link.relation() == QLatin1String( "item" ) &&
              !hasItemsEndpoint )
    {
      itemsCount++;

      if ( itemsCount > MAX_DISPLAYED_ITEMS )
        continue;

      QgsStacItemItem *i = new QgsStacItemItem( this, link.title(), link.href() );
      contents.append( i );
    }
    else if ( link.relation() == QLatin1String( "items" ) )
    {
      // stac api items (ogcapi features)
      QString error;
      std::unique_ptr< QgsStacItemCollection > ic( controller->fetchItemCollection( link.href(), &error ) );
      if ( ic )
      {
        contents.append( createItems( ic->takeItems() ) );
        mFetchMoreUrl = ic->nextUrl();
        itemsCount = ic->numberMatched();
      }
      else
      {
        // item collection fetching failed
        contents.append( new QgsErrorItem( this, error, path() + QStringLiteral( "/error" ) ) );
      }
    }
    else
    {
      // todo: should handle other links?
    }
  }

  QgsStacFetchMoreItem *moreItem = nullptr;
  if ( !mFetchMoreUrl.isEmpty() )
  {
    if ( itemsCount >= 0 )
      moreItem = new QgsStacFetchMoreItem( this, tr( "Double-click to fetch more (%L1 total items)..." ).arg( itemsCount ) );
    else
      moreItem = new QgsStacFetchMoreItem( this, tr( "Double-click to fetch more items..." ) );
  }
  else if ( itemsCount > MAX_DISPLAYED_ITEMS )
  {
    moreItem = new QgsStacFetchMoreItem( this, tr( "%1 more items" ).arg( itemsCount - MAX_DISPLAYED_ITEMS ) );
  }

  if ( moreItem )
    contents.append( moreItem );

  return contents;
}

bool QgsStacCatalogItem::equal( const QgsDataItem *other )
{
  Q_UNUSED( other );
  return false;
}

void QgsStacCatalogItem::updateToolTip()
{
  mToolTip = QStringLiteral( "STAC Catalog:\n%1\n%2" ).arg( mName, mPath );
  if ( mIsCollection )
  {
    const int pos = mToolTip.indexOf( ':' );
    mToolTip.replace( 0, pos, QStringLiteral( "STAC Collection" ) );
  }
}

void QgsStacCatalogItem::setStacCatalog( QgsStacCatalog *catalog )
{
  mStacCatalog.reset( catalog );
  if ( mStacCatalog )
  {
    if ( mName.isEmpty() && !mStacCatalog->title().isEmpty() )
      setName( mStacCatalog->title() );

    if ( mStacCatalog->type() == QgsStacObject::Collection )
    {
      mIsCollection = true;
      mIconName = QStringLiteral( "mIconFolderOpen.svg" );
    }
  }
  updateToolTip();
}

QgsStacCatalog *QgsStacCatalogItem::stacCatalog() const
{
  return mStacCatalog.get();
}

QVector< QgsDataItem * > QgsStacCatalogItem::createItems( const QVector<QgsStacItem *> items )
{
  QVector< QgsDataItem * > contents;
  contents.reserve( items.size() );
  for ( QgsStacItem *item : items )
  {
    if ( !item )
      continue;

    const QString name = item->properties().value( QStringLiteral( "title" ), item->id() ).toString();

    QgsStacItemItem *i = new QgsStacItemItem( this, name, item->url() );
    i->setStacItem( item );
    i->setState( Qgis::BrowserItemState::Populated );
    contents.append( i );
  }
  return contents;
}

QVector<QgsDataItem *> QgsStacCatalogItem::createCollections( const QVector<QgsStacCollection *> collections )
{
  QVector< QgsDataItem * > contents;
  contents.reserve( collections.size() );
  for ( QgsStacCollection *col : collections )
  {
    if ( !col )
      continue;

    const QString name = col->title().isEmpty() ? col->id() : col->title();

    QgsStacCatalogItem *i = new QgsStacCatalogItem( this, name, col->url() );
    i->setStacCatalog( col );
    contents.append( i );
  }
  return contents;
}

void QgsStacCatalogItem::fetchMoreChildren()
{
  if ( mFetchMoreUrl.isEmpty() )
    return;

  QgsStacFetchMoreItem *moreItem = fetchMoreItem();

  QgsStacController *c = stacController();
  std::unique_ptr< QgsStacItemCollection > ic( c->fetchItemCollection( mFetchMoreUrl ) );
  if ( ic )
  {
    populate( createItems( ic->takeItems() ) );

    mFetchMoreUrl = ic->nextUrl();
    if ( !ic->nextUrl().isEmpty() && moreItem )
    {

      const int numberMatched = ic->numberMatched();
      if ( numberMatched > -1 )
      {
        moreItem->setName( tr( "Double-click to fetch more (%L1 total items)..." ).arg( numberMatched ) );
      }
    }
    else
    {
      // delete fetch more item
      removeChildItem( moreItem );
    }
  }
}


//
// QgsStacConnectionItem
//

QgsStacConnectionItem::QgsStacConnectionItem( QgsDataItem *parent, const QString &connectionName )
  : QgsStacCatalogItem( parent, connectionName, QString() )
  , mConnName( connectionName )
  , mController( new QgsStacController() )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  const QgsStacConnection::Data data = QgsStacConnection::connection( connectionName );

  mController->setAuthCfg( data.authCfg );

  mPath = data.url;
  mToolTip = QStringLiteral( "Connection:\n%1\n%2" ).arg( connectionName, mPath );
}

QgsStacController *QgsStacConnectionItem::controller() const
{
  return mController.get();
}



//
// QgsStacRootItem
//

QgsStacRootItem::QgsStacRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "special:Stac" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconStac.svg" );
  populate();
}

QVector<QgsDataItem *> QgsStacRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsStacConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    QgsDataItem *conn = new QgsStacConnectionItem( this, connName );
    connections.append( conn );
  }
  return connections;
}

void QgsStacRootItem::onConnectionsChanged()
{
  refresh();
}


//
// QgsStacDataItemProvider
//

QString QgsStacDataItemProvider::name()
{
  return QStringLiteral( "STAC" );
}

QString QgsStacDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "special:Stac" );
}

Qgis::DataItemProviderCapabilities QgsStacDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsStacDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsStacRootItem( parentItem, QObject::tr( "STAC" ), QStringLiteral( "stac:" ) );
  return nullptr;
}
