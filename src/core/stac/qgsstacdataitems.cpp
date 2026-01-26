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

#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstaccollectionlist.h"
#include "qgsstacconnection.h"
#include "qgsstaccontroller.h"
#include "qgsstacitem.h"
#include "qgsstacitemcollection.h"

#include "moc_qgsstacdataitems.cpp"

constexpr int MAX_DISPLAYED_ITEMS = 20;


//
// QgsStacAssetItem
//

QgsStacAssetItem::QgsStacAssetItem( QgsDataItem *parent, const QString &name, const QgsStacAsset *asset )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name, QString( "%1/%2" ).arg( parent->path(), name ), u"special:Stac"_s ),
    mStacAsset( asset ),
    mName( name )
{
  if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( asset->uri().providerKey ) )
  {
    mIcon = metadata->icon();
  }
  else
  {
    mIconName = u"downloading_svg.svg"_s;
  }
  updateToolTip();
  setState( Qgis::BrowserItemState::Populated );
}

bool QgsStacAssetItem::hasDragEnabled() const
{
  return mStacAsset->isCloudOptimized();
}

QgsStacController *QgsStacAssetItem::stacController() const
{
  const QgsDataItem *item = this;
  while ( item )
  {
    if ( const QgsStacConnectionItem *ci = qobject_cast<const QgsStacConnectionItem *>( item ) )
      return ci->controller();
    item = item->parent();
  }
  Q_ASSERT( false );
  return nullptr;
}

QgsMimeDataUtils::UriList QgsStacAssetItem::mimeUris() const
{
  QgsStacController *controller = stacController();

  const QString authcfg = controller ? controller->authCfg() : QString();

  QgsMimeDataUtils::Uri uri;
  QUrl url( mStacAsset->href() );
  if ( url.isLocalFile() )
  {
    uri.uri = mStacAsset->href();
  }
  else
  {
    uri = mStacAsset->uri( authcfg );
  }

  return { uri };
}

bool QgsStacAssetItem::equal( const QgsDataItem * )
{
  return false;
}

void QgsStacAssetItem::updateToolTip()
{
  QString title = mStacAsset->title();
  if ( title.isNull() || title.isEmpty() )
  {
    title = mName;
  }
  mToolTip = u"STAC Asset:\n%1\n%2"_s.arg( title, mStacAsset->href() );
}

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
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name.isEmpty() ? path : name, path, u"special:Stac"_s )
{
  mIconName = u"mActionPropertiesWidget.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  updateToolTip();
}

QVector<QgsDataItem *> QgsStacItemItem::createChildren()
{
  QgsStacController *controller = stacController();
  QString error;
  setStacItem( controller->fetchStacObject<QgsStacItem>( mPath, &error ) );

  if ( !mStacItem )
    return { new QgsErrorItem( this, error, path() + u"/error"_s ) };

  setState( Qgis::BrowserItemState::Populating );
  QVector<QgsDataItem *> contents;
  contents.reserve( mStacItem->assets().size() );
  const QMap<QString, QgsStacAsset> assets = mStacItem->assets();
  for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
  {
    QgsStacAssetItem *assetItem = new QgsStacAssetItem( this, it.key(), &it.value() );
    contents.append( assetItem );
  }
  return contents;
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

  const QString authcfg = stacController()->authCfg();

  const QMap<QString, QgsStacAsset> assets = mStacItem->assets();
  for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
  {
    QgsMimeDataUtils::Uri uri = it->uri( authcfg );
    if ( uri.isValid() )
    {
      uris.append( uri );
    }
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
    name = mStacItem->properties().value( u"title"_s, mName ).toString();
  }
  mToolTip = u"STAC Item:\n%1\n%2"_s.arg( name, mPath );
}

QgsStacController *QgsStacItemItem::stacController() const
{
  const QgsDataItem *item = this;
  while ( item )
  {
    if ( const QgsStacConnectionItem *ci = qobject_cast<const QgsStacConnectionItem *>( item ) )
      return ci->controller();
    item = item->parent();
  }
  Q_ASSERT( false );
  return nullptr;
}

void QgsStacItemItem::setStacItem( std::unique_ptr<QgsStacItem> item )
{
  mStacItem = std::move( item );
  updateToolTip();
}

QgsStacItem *QgsStacItemItem::stacItem() const
{
  return mStacItem.get();
}

void QgsStacItemItem::itemRequestFinished( int requestId, QString error )
{
  QgsStacController *controller = stacController();
  std::unique_ptr< QgsStacItem > object = controller->takeStacObject< QgsStacItem >( requestId );
  setStacItem( std::move( object ) );
  if ( mStacItem )
  {
    mIconName = u"mActionPropertiesWidget.svg"_s;
    QString name = mStacItem->properties().value( u"title"_s, QString() ).toString();
    if ( name.isEmpty() )
      name = mStacItem->id();
    mName = name;
  }
  else
  {
    mIconName = u"/mIconDelete.svg"_s;
    mName = error;
  }
}


//
// QgsStacCatalogItem
//

QgsStacCatalogItem::QgsStacCatalogItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, u"special:Stac"_s )
{
  mIconName = u"mIconFolder.svg"_s;
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

      const int requestId = stacController()->fetchStacObjectAsync( item->path() );
      item->setProperty( "requestId", requestId );
    }
  }
}

void QgsStacCatalogItem::onControllerFinished( int requestId, const QString &error )
{
  for ( QgsDataItem *child : std::as_const( mChildren ) )
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

  QgsStacController *controller = stacController();
  QString error;
  setStacCatalog( controller->fetchStacObject< QgsStacCatalog >( mPath, &error ) );

  if ( !mStacCatalog )
    return { new QgsErrorItem( this, error, path() + u"/error"_s ) };

  QgsStacCatalog *root = rootCatalog();

  const bool supportsCollections = root && root->conformsTo( u"https://api.stacspec.org/v1.0.0/collections"_s );
  const bool supportsItems = root && root->conformsTo( u"https://api.stacspec.org/v1.0.0/ogcapi-features"_s );

  int itemsCount = 0;
  QVector<QgsDataItem *> contents;
  const QVector< QgsStacLink > links = mStacCatalog->links();

  // treat catalog/collection as static if it does not have a /items endpoint
  bool useItemsEndpoint = false;
  bool useCollectionsEndpoint = false;
  if ( supportsCollections || supportsItems )
  {
    for ( const QgsStacLink &link : links )
    {
      if ( link.relation() == "items"_L1 )
      {
        useItemsEndpoint = true;
      }
      else if ( link.relation() == "data"_L1 &&
                link.href().endsWith( "/collections"_L1 ) )
      {
        useCollectionsEndpoint = true;
      }
      // if we found what we need we can stop looking
      if ( supportsItems == useItemsEndpoint && supportsCollections == useCollectionsEndpoint )
        break;
    }
  }

  for ( const QgsStacLink &link : links )
  {
    // skip hierarchical navigation links
    if ( link.relation() == "self"_L1 ||
         link.relation() == "root"_L1 ||
         link.relation() == "parent"_L1 ||
         link.relation() == "collection"_L1 )
      continue;

    if ( link.relation() == "child"_L1 &&
         !useCollectionsEndpoint )
    {
      // may be either catalog or collection
      QgsStacCatalogItem *c = new QgsStacCatalogItem( this, link.title(), link.href() );
      contents.append( c );
    }
    else if ( link.relation() == "data"_L1 &&
              link.href().endsWith( "/collections"_L1 ) )
    {
      // use /collections api
      QString error;
      std::unique_ptr< QgsStacCollectionList > cols( controller->fetchCollections( link.href(), &error ) );
      if ( cols )
      {
        contents.append( createCollections( cols->takeCollections() ) );
        itemsCount = cols->numberMatched();
      }
      else
      {
        // collection fetching failed
        contents.append( new QgsErrorItem( this, error, path() + u"/error"_s ) );
      }
    }
    else if ( link.relation() == "item"_L1 &&
              !useItemsEndpoint )
    {
      itemsCount++;

      if ( itemsCount > MAX_DISPLAYED_ITEMS )
        continue;

      QgsStacItemItem *i = new QgsStacItemItem( this, link.title(), link.href() );
      contents.append( i );
    }
    else if ( link.relation() == "items"_L1 &&
              useItemsEndpoint )
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
        contents.append( new QgsErrorItem( this, error, path() + u"/error"_s ) );
      }
    }
    else
    {
      // todo: should handle other links?
    }
  }

  if ( QgsStacCollection *collection = dynamic_cast<QgsStacCollection *>( mStacCatalog.get() ) )
  {
    const QMap<QString, QgsStacAsset> assets = collection->assets();
    for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
    {
      QgsStacAssetItem *assetItem = new QgsStacAssetItem( this, it.key(), &it.value() );
      contents.append( assetItem );
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
  mToolTip = u"STAC Catalog:\n%1\n%2"_s.arg( mName, mPath );
  if ( mIsCollection )
  {
    const int pos = mToolTip.indexOf( ':' );
    mToolTip.replace( 0, pos, u"STAC Collection"_s );
  }
}

void QgsStacCatalogItem::setStacCatalog( std::unique_ptr<QgsStacCatalog> catalog )
{
  mStacCatalog = std::move( catalog );
  if ( mStacCatalog )
  {
    if ( mName.isEmpty() && !mStacCatalog->title().isEmpty() )
      setName( mStacCatalog->title() );

    if ( mStacCatalog->type() == Qgis::StacObjectType::Collection )
    {
      mIsCollection = true;
      mIconName = u"mIconFolderOpen.svg"_s;
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

    std::unique_ptr< QgsStacItem > object( item );

    const QString name = item->properties().value( u"title"_s, item->id() ).toString();

    QgsStacItemItem *i = new QgsStacItemItem( this, name, item->url() );
    i->setStacItem( std::move( object ) );
    i->setState( Qgis::BrowserItemState::NotPopulated );
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

    std::unique_ptr< QgsStacCollection > object( col );

    const QString name = col->title().isEmpty() ? col->id() : col->title();

    QgsStacCatalogItem *i = new QgsStacCatalogItem( this, name, col->url() );
    i->setStacCatalog( std::move( object ) );
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
  mIconName = u"mIconConnect.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  const QgsStacConnection::Data data = QgsStacConnection::connection( connectionName );

  mController->setAuthCfg( data.authCfg );

  mPath = data.url;
  mToolTip = u"Connection:\n%1\n%2"_s.arg( connectionName, mPath );
}

QgsStacController *QgsStacConnectionItem::controller() const
{
  return mController.get();
}



//
// QgsStacRootItem
//

QgsStacRootItem::QgsStacRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, u"special:Stac"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconStac.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsStacRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connectionList = QgsStacConnection::connectionList();
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
  return u"STAC"_s;
}

QString QgsStacDataItemProvider::dataProviderKey() const
{
  return u"special:Stac"_s;
}

Qgis::DataItemProviderCapabilities QgsStacDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsStacDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsStacRootItem( parentItem, QObject::tr( "STAC" ), u"stac:"_s );
  return nullptr;
}
