/***************************************************************************
      qgsarcgisrestdataitems.cpp
      -----------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsarcgisrestdataitems.h"

#include "qgsarcgisportalutils.h"
#include "qgsarcgisrestquery.h"
#include "qgsarcgisrestutils.h"
#include "qgslogger.h"
#include "qgsmaplayerfactory.h"
#include "qgsowsconnection.h"
#include "qgssettingsentryimpl.h"

#include <QString>

#include "moc_qgsarcgisrestdataitems.cpp"

using namespace Qt::StringLiterals;

#ifdef HAVE_GUI
#include "qgsarcgisrestsourceselect.h"
#endif

#include <QMessageBox>
#include <QCoreApplication>
#include <QSettings>
#include <QUrl>


//
// QgsArcGisRestRootItem
//

QgsArcGisRestRootItem::QgsArcGisRestRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, u"AFS"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconAfs.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsArcGisRestRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList connectionList = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items();
  for ( const QString &connName : connectionList )
  {
    const QString path = u"afs:/"_s + connName;
    connections.append( new QgsArcGisRestConnectionItem( this, connName, path, connName ) );
  }

  return connections;
}

#ifdef HAVE_GUI
QWidget *QgsArcGisRestRootItem::paramWidget()
{
  QgsArcGisRestSourceSelect *select = new QgsArcGisRestSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsArcGisRestSourceSelect::connectionsChanged, this, &QgsArcGisRestRootItem::onConnectionsChanged );
  return select;
}

void QgsArcGisRestRootItem::onConnectionsChanged()
{
  refresh();
}
#endif

///////////////////////////////////////////////////////////////////////////////

void addFolderItems(
  QVector<QgsDataItem *> &items,
  const QVariantMap &serviceData,
  const QString &baseUrl,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString &urlPrefix,
  QgsDataItem *parent,
  const QString &supportedFormats,
  bool forceRefresh
)
{
  QgsArcGisRestQueryUtils::visitFolderItems(
    [parent, &baseUrl, &items, headers, urlPrefix, authcfg, supportedFormats, forceRefresh]( const QString &name, const QString &url ) {
      auto folderItem = std::make_unique<QgsArcGisRestFolderItem>( parent, name, url, baseUrl, authcfg, headers, urlPrefix, forceRefresh );
      folderItem->setSupportedFormats( supportedFormats );
      items.append( folderItem.release() );
    },
    serviceData,
    baseUrl
  );
}

void addServiceItems(
  QVector<QgsDataItem *> &items,
  const QVariantMap &serviceData,
  const QString &baseUrl,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString &urlPrefix,
  QgsDataItem *parent,
  const QString &supportedFormats,
  bool forceRefresh
)
{
  QgsArcGisRestQueryUtils::visitServiceItems(
    [&items, parent, authcfg, headers, urlPrefix, supportedFormats, forceRefresh]( const QString &name, const QString &url, Qgis::ArcGisRestServiceType serviceType ) {
      switch ( serviceType )
      {
        case Qgis::ArcGisRestServiceType::MapServer:
        {
          auto serviceItem = std::make_unique<QgsArcGisMapServiceItem>( parent, name, url, url, authcfg, headers, urlPrefix, forceRefresh );
          items.append( serviceItem.release() );
          break;
        }

        case Qgis::ArcGisRestServiceType::ImageServer:
        {
          auto serviceItem = std::make_unique<QgsArcGisImageServiceItem>( parent, name, url, url, authcfg, headers, urlPrefix, forceRefresh );
          items.append( serviceItem.release() );
          break;
        }

        case Qgis::ArcGisRestServiceType::FeatureServer:
        {
          auto serviceItem = std::make_unique<QgsArcGisFeatureServiceItem>( parent, name, url, url, authcfg, headers, urlPrefix, forceRefresh );
          serviceItem->setSupportedFormats( supportedFormats );
          items.append( serviceItem.release() );
          break;
        }

        case Qgis::ArcGisRestServiceType::SceneServer:
        {
          auto serviceItem = std::make_unique<QgsArcGisSceneServiceItem>( parent, name, url, url, authcfg, headers, urlPrefix, forceRefresh );
          items.append( serviceItem.release() );
          break;
        }

        case Qgis::ArcGisRestServiceType::GlobeServer:
        case Qgis::ArcGisRestServiceType::GPServer:
        case Qgis::ArcGisRestServiceType::GeocodeServer:
        case Qgis::ArcGisRestServiceType::Unknown:
          break; // unsupported
      }
    },
    serviceData,
    baseUrl
  );
}

void addLayerItems(
  QVector<QgsDataItem *> &items,
  const QVariantMap &serviceData,
  const QString &parentUrl,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString urlPrefix,
  QgsDataItem *parent,
  Qgis::ArcGisRestServiceType serviceType,
  const QString &supportedFormats,
  bool forceRefresh
)
{
  QMultiMap<QString, QgsDataItem *> layerItems;
  QMap<QString, QString> parents;
  QMap<QString, QgsMimeDataUtils::Uri> mapServerUrisForAllLayersRender;

  QgsArcGisRestQueryUtils::addLayerItems(
    [parent, &layerItems, &parents, &mapServerUrisForAllLayersRender, authcfg, headers, urlPrefix, serviceType, supportedFormats, forceRefresh]( const QgsArcGisRestQueryUtils::LayerItemDetails &details ) {
      Q_UNUSED( forceRefresh )

      if ( !details.parentLayerId.isEmpty() )
        parents.insert( details.layerId, details.parentLayerId );

      if ( details.isMapServerSpecialAllLayersOption )
      {
        QgsMimeDataUtils::Uri uri;
        uri.layerType = QgsMapLayerFactory::typeToString( Qgis::LayerType::Raster );
        uri.providerKey = u"arcgismapserver"_s;
        uri.name = details.name;
        uri.uri = u"format='%1' layer='' url='%3'"_s.arg( details.format, details.url );
        if ( !authcfg.isEmpty() )
          uri.uri += u" authcfg='%1'"_s.arg( authcfg );
        if ( !urlPrefix.isEmpty() )
          uri.uri += u" urlprefix='%1'"_s.arg( urlPrefix );
        uri.uri += headers.toSpacedString();
        mapServerUrisForAllLayersRender.insert( details.url, uri );
        return;
      }

      const Qgis::BrowserLayerType browserLayerGeometryType = details.geometryType == Qgis::GeometryType::Polygon ? Qgis::BrowserLayerType::Polygon
                                                              : details.geometryType == Qgis::GeometryType::Line  ? Qgis::BrowserLayerType::Line
                                                              : details.geometryType == Qgis::GeometryType::Point ? Qgis::BrowserLayerType::Point
                                                              : details.geometryType == Qgis::GeometryType::Null  ? Qgis::BrowserLayerType::TableLayer
                                                                                                                  : Qgis::BrowserLayerType::Vector;

      if ( details.isParentLayer && details.serviceType != Qgis::ArcGisRestServiceType::MapServer )
      {
        if ( !layerItems.value( details.layerId ) )
        {
          auto layerItem = std::make_unique<QgsArcGisRestParentLayerItem>( parent, details.name, details.url, authcfg, headers, urlPrefix );
          layerItems.insert( details.layerId, layerItem.release() );
        }
      }
      else
      {
        std::unique_ptr<QgsDataItem> layerItem;
        switch ( details.serviceType )
        {
          case Qgis::ArcGisRestServiceType::FeatureServer:
            layerItem = std::make_unique<QgsArcGisFeatureServiceLayerItem>(
              parent, details.url, details.name, details.crs, authcfg, headers, urlPrefix, browserLayerGeometryType, details.isMapServerWithQueryCapability, details.format, details.layerId
            );
            break;

          case Qgis::ArcGisRestServiceType::MapServer:
          {
            {
              layerItem = std::make_unique<
                QgsArcGisMapServiceLayerItem>( parent, details.url, details.layerId, details.name, details.crs, details.format, authcfg, headers, urlPrefix, details.isMapServerWithQueryCapability );
              static_cast<QgsArcGisMapServiceLayerItem *>( layerItem.get() )->setSupportedFormats( supportedFormats );
              break;
            }

            case Qgis::ArcGisRestServiceType::ImageServer:
            {
              layerItem = std::make_unique< QgsArcGisImageServiceLayerItem>( parent, details.url, details.layerId, details.name, details.crs, details.format, authcfg, headers, urlPrefix );
              break;
            }
          }

          case Qgis::ArcGisRestServiceType::SceneServer:
            layerItem = std::make_unique<QgsArcGisSceneServiceLayerItem>( parent, details.url, details.name, details.crs, authcfg, headers, urlPrefix );
            break;


          case Qgis::ArcGisRestServiceType::GlobeServer:
          case Qgis::ArcGisRestServiceType::GPServer:
          case Qgis::ArcGisRestServiceType::GeocodeServer:
          case Qgis::ArcGisRestServiceType::Unknown:
            break;
        }
        if ( layerItem )
          layerItems.insert( details.layerId, layerItem.release() );
      }
    },
    serviceData,
    parentUrl,
    supportedFormats,
    serviceType
  );

  // create groups
  for ( auto it = layerItems.constBegin(); it != layerItems.constEnd(); ++it )
  {
    const QString id = it.key();
    QgsDataItem *item = it.value();
    const QString parentId = parents.value( id );

    if ( QgsDataItem *layerParent = parentId.isEmpty() ? nullptr : layerItems.value( parentId ) )
      layerParent->addChildItem( item );
    else
      items.append( item );
  }

  for ( auto it = mapServerUrisForAllLayersRender.constBegin(); it != mapServerUrisForAllLayersRender.constEnd(); ++it )
  {
    for ( auto layerItemsIt = layerItems.constBegin(); layerItemsIt != layerItems.constEnd(); ++layerItemsIt )
    {
      if ( auto parentLayerItem = qobject_cast< QgsArcGisRestParentLayerItem * >( layerItemsIt.value() ) )
      {
        if ( parentLayerItem->path() == it.key() )
        {
          parentLayerItem->setAllLayersMapServerUri( it.value() );
        }
      }
    }
    if ( auto parentItem = qobject_cast< QgsArcGisRestParentLayerItem * >( parent ) )
    {
      if ( parentItem->path() == it.key() )
      {
        parentItem->setAllLayersMapServerUri( it.value() );
      }
    }
    if ( auto parentItem = qobject_cast< QgsArcGisMapServiceItem * >( parent ) )
    {
      if ( parentItem->path() == it.key() )
      {
        parentItem->setAllLayersMapServerUri( it.value() );
      }
    }
  }
}

//
// QgsArcGisRestConnectionItem
//

QgsArcGisRestConnectionItem::QgsArcGisRestConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName )
  : QgsDataCollectionItem( parent, name, path, u"AFS"_s )
  , mConnName( connectionName )
{
  mIconName = u"mIconConnect.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  mPortalContentEndpoint = QgsArcGisConnectionSettings::settingsContentEndpoint->value( connectionName );
  mPortalCommunityEndpoint = QgsArcGisConnectionSettings::settingsCommunityEndpoint->value( connectionName );
}

void QgsArcGisRestConnectionItem::refresh()
{
  mForceRefresh = true;
  QgsDataItem::refresh();
}

QVector<QgsDataItem *> QgsArcGisRestConnectionItem::createChildren()
{
  const QString url = QgsArcGisConnectionSettings::settingsUrl->value( mConnName );
  const QString authcfg = QgsArcGisConnectionSettings::settingsAuthcfg->value( mConnName );

  QgsHttpHeaders headers( QgsArcGisConnectionSettings::settingsHeaders->value( mConnName ) );
  const QString urlPrefix = QgsArcGisConnectionSettings::settingsUrlPrefix->value( mConnName );

  QVector<QgsDataItem *> items;
  if ( !mPortalCommunityEndpoint.isEmpty() && !mPortalContentEndpoint.isEmpty() )
  {
    items << new QgsArcGisPortalGroupsItem( this, u"groups"_s, authcfg, headers, urlPrefix, mPortalCommunityEndpoint, mPortalContentEndpoint, mForceRefresh );
    items << new QgsArcGisRestServicesItem( this, url, u"services"_s, authcfg, headers, urlPrefix, mForceRefresh );
  }
  else
  {
    QString errorTitle, errorMessage;
    const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, authcfg, errorTitle, errorMessage, headers, urlPrefix, mForceRefresh );
    if ( serviceData.isEmpty() )
    {
      if ( !errorMessage.isEmpty() )
      {
        auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
        error->setToolTip( errorMessage );
        items.append( error.release() );
        QgsDebugError( "Connection failed - " + errorMessage );
      }
      return items;
    }

    Qgis::ArcGisRestServiceType serviceType = QgsArcGisRestQueryUtils::sniffServiceTypeFromUrl( url );
    if ( serviceType == Qgis::ArcGisRestServiceType::Unknown )
    {
      serviceType = QgsArcGisRestQueryUtils::sniffServiceTypeFromJson( serviceData );
    }

    addFolderItems( items, serviceData, url, authcfg, headers, urlPrefix, this, QString(), mForceRefresh );
    addServiceItems( items, serviceData, url, authcfg, headers, urlPrefix, this, QString(), mForceRefresh );
    addLayerItems( items, serviceData, url, authcfg, headers, urlPrefix, this, serviceType, QString(), mForceRefresh );
  }

  return items;
}

bool QgsArcGisRestConnectionItem::equal( const QgsDataItem *other )
{
  const QgsArcGisRestConnectionItem *o = qobject_cast<const QgsArcGisRestConnectionItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

QString QgsArcGisRestConnectionItem::url() const
{
  return QgsArcGisConnectionSettings::settingsUrl->value( mConnName );
}


//
// QgsArcGisPortalGroupsItem
//

QgsArcGisPortalGroupsItem::QgsArcGisPortalGroupsItem(
  QgsDataItem *parent, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, const QString &communityEndpoint, const QString &contentEndpoint, bool forceRefresh
)
  : QgsDataCollectionItem( parent, tr( "Groups" ), path, u"AFS"_s )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mPortalCommunityEndpoint( communityEndpoint )
  , mPortalContentEndpoint( contentEndpoint )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconDbSchema.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}


QVector<QgsDataItem *> QgsArcGisPortalGroupsItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QString errorTitle;
  QString errorMessage;
  const QVariantList groups = QgsArcGisPortalUtils::retrieveUserGroups( mPortalCommunityEndpoint, QString(), mAuthCfg, errorTitle, errorMessage, mHeaders, nullptr, QString(), mForceRefresh );
  if ( groups.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  for ( const QVariant &group : groups )
  {
    const QVariantMap groupData = group.toMap();
    items << new QgsArcGisPortalGroupItem( this, groupData.value( u"id"_s ).toString(), groupData.value( u"title"_s ).toString(), mAuthCfg, mHeaders, mUrlPrefix, mPortalCommunityEndpoint, mPortalContentEndpoint, mForceRefresh );
    items.last()->setToolTip( groupData.value( u"snippet"_s ).toString() );
  }

  return items;
}

bool QgsArcGisPortalGroupsItem::equal( const QgsDataItem *other )
{
  const QgsArcGisPortalGroupsItem *o = qobject_cast<const QgsArcGisPortalGroupsItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisPortalGroupItem
//
QgsArcGisPortalGroupItem::QgsArcGisPortalGroupItem(
  QgsDataItem *parent,
  const QString &groupId,
  const QString &name,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString &urlPrefix,
  const QString &communityEndpoint,
  const QString &contentEndpoint,
  bool forceRefresh
)
  : QgsDataCollectionItem( parent, name, groupId, u"AFS"_s )
  , mId( groupId )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mPortalCommunityEndpoint( communityEndpoint )
  , mPortalContentEndpoint( contentEndpoint )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconDbSchema.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( name );
}

QVector<QgsDataItem *> QgsArcGisPortalGroupItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QString errorTitle;
  QString errorMessage;
  const QVariantList groupItems = QgsArcGisPortalUtils::retrieveGroupItemsOfType(
    mPortalContentEndpoint,
    mId,
    mAuthCfg,
    QList<int>() << static_cast<int>( Qgis::ArcGisRestServiceType::FeatureServer ) << static_cast<int>( Qgis::ArcGisRestServiceType::MapServer ) << static_cast<int>( Qgis::ArcGisRestServiceType::ImageServer ),
    errorTitle,
    errorMessage,
    mHeaders,
    nullptr,
    100,
    QString(),
    mForceRefresh
  );
  if ( groupItems.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  for ( const QVariant &item : groupItems )
  {
    const QVariantMap itemData = item.toMap();

    if ( itemData.value( u"type"_s ).toString().compare( u"Feature Service"_s, Qt::CaseInsensitive ) == 0 )
    {
      items << new QgsArcGisFeatureServiceItem( this, itemData.value( u"title"_s ).toString(), itemData.value( u"url"_s ).toString(), itemData.value( u"url"_s ).toString(), mAuthCfg, mHeaders, mUrlPrefix, mForceRefresh );
    }
    else if ( itemData.value( u"type"_s ).toString().compare( u"Scene Service"_s, Qt::CaseInsensitive ) == 0 )
    {
      items << new QgsArcGisSceneServiceItem( this, itemData.value( u"title"_s ).toString(), itemData.value( u"url"_s ).toString(), itemData.value( u"url"_s ).toString(), mAuthCfg, mHeaders, mUrlPrefix, mForceRefresh );
    }
    else if ( itemData.value( u"type"_s ).toString().compare( u"Map Service"_s, Qt::CaseInsensitive ) == 0 )
    {
      items << new QgsArcGisMapServiceItem( this, itemData.value( u"title"_s ).toString(), itemData.value( u"url"_s ).toString(), itemData.value( u"url"_s ).toString(), mAuthCfg, mHeaders, mUrlPrefix, mForceRefresh );
    }
    else
    {
      items << new QgsArcGisImageServiceItem( this, itemData.value( u"title"_s ).toString(), itemData.value( u"url"_s ).toString(), itemData.value( u"url"_s ).toString(), mAuthCfg, mHeaders, mUrlPrefix, mForceRefresh );
    }
  }

  return items;
}

bool QgsArcGisPortalGroupItem::equal( const QgsDataItem *other )
{
  const QgsArcGisPortalGroupItem *o = qobject_cast<const QgsArcGisPortalGroupItem *>( other );
  return ( type() == other->type() && o && mId == o->mId && mName == o->mName );
}


//
// QgsArcGisRestServicesItem
//

QgsArcGisRestServicesItem::QgsArcGisRestServicesItem(
  QgsDataItem *parent, const QString &url, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers, const QString urlPrefix, bool forceRefresh
)
  : QgsDataCollectionItem( parent, tr( "Services" ), path, u"AFS"_s )
  , mUrl( url )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconDbSchema.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsArcGisRestServicesItem::createChildren()
{
  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( mUrl, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix, mForceRefresh );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  Qgis::ArcGisRestServiceType serviceType = QgsArcGisRestQueryUtils::sniffServiceTypeFromUrl( mUrl );
  if ( serviceType == Qgis::ArcGisRestServiceType::Unknown )
  {
    serviceType = QgsArcGisRestQueryUtils::sniffServiceTypeFromJson( serviceData );
  }

  addFolderItems( items, serviceData, mUrl, mAuthCfg, mHeaders, mUrlPrefix, this, QString(), mForceRefresh );
  addServiceItems( items, serviceData, mUrl, mAuthCfg, mHeaders, mUrlPrefix, this, QString(), mForceRefresh );
  addLayerItems( items, serviceData, mUrl, mAuthCfg, mHeaders, mUrlPrefix, this, serviceType, QString(), mForceRefresh );
  return items;
}

bool QgsArcGisRestServicesItem::equal( const QgsDataItem *other )
{
  const QgsArcGisRestServicesItem *o = qobject_cast<const QgsArcGisRestServicesItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisRestFolderItem
//
QgsArcGisRestFolderItem::QgsArcGisRestFolderItem(
  QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, bool forceRefresh
)
  : QgsDataCollectionItem( parent, name, path, u"AFS"_s )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconDbSchema.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}

void QgsArcGisRestFolderItem::setSupportedFormats( const QString &formats )
{
  mSupportedFormats = formats;
}


QVector<QgsDataItem *> QgsArcGisRestFolderItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, QString(), mForceRefresh );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats, mForceRefresh );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats, mForceRefresh );
  // TO confirm -- is this possible? Can folder items have direct child layers, or do they always need to be placed in services first?
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, Qgis::ArcGisRestServiceType::Unknown, mSupportedFormats, mForceRefresh );
  return items;
}

bool QgsArcGisRestFolderItem::equal( const QgsDataItem *other )
{
  const QgsArcGisRestFolderItem *o = qobject_cast<const QgsArcGisRestFolderItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

//
// QgsArcGisFeatureServiceItem
//
QgsArcGisFeatureServiceItem::QgsArcGisFeatureServiceItem(
  QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, bool forceRefresh
)
  : QgsDataCollectionItem( parent, name, path, u"AFS"_s )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconAfs.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}

void QgsArcGisFeatureServiceItem::setSupportedFormats( const QString &formats )
{
  mSupportedFormats = formats;
}

QVector<QgsDataItem *> QgsArcGisFeatureServiceItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix, mForceRefresh );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats, mForceRefresh );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats, mForceRefresh );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, Qgis::ArcGisRestServiceType::FeatureServer, mSupportedFormats, mForceRefresh );
  return items;
}

bool QgsArcGisFeatureServiceItem::equal( const QgsDataItem *other )
{
  const QgsArcGisFeatureServiceItem *o = qobject_cast<const QgsArcGisFeatureServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisMapServiceItem
//

QgsArcGisMapServiceItem::QgsArcGisMapServiceItem(
  QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, bool forceRefresh
)
  : QgsDataCollectionItem( parent, name, path, u"AMS"_s )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconAms.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}

QVector<QgsDataItem *> QgsArcGisMapServiceItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix, mForceRefresh );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  const QString supportedFormats = serviceData.value( u"supportedImageFormatTypes"_s ).toString();

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, supportedFormats, mForceRefresh );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, supportedFormats, mForceRefresh );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, Qgis::ArcGisRestServiceType::MapServer, supportedFormats, mForceRefresh );
  return items;
}

bool QgsArcGisMapServiceItem::equal( const QgsDataItem *other )
{
  const QgsArcGisMapServiceItem *o = qobject_cast<const QgsArcGisMapServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisImageServiceItem
//

QgsArcGisImageServiceItem::QgsArcGisImageServiceItem(
  QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, bool forceRefresh
)
  : QgsDataCollectionItem( parent, name, path, u"IMS"_s )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconAms.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}

QVector<QgsDataItem *> QgsArcGisImageServiceItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix, mForceRefresh );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  const QString supportedFormats = u"JPGPNG,PNG,PNG8,PNG24,JPG,BMP,GIF,TIFF,PNG32,BIP,BSQ,LERC"_s;
  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, supportedFormats, mForceRefresh );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, supportedFormats, mForceRefresh );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, Qgis::ArcGisRestServiceType::ImageServer, supportedFormats, mForceRefresh );
  return items;
}

bool QgsArcGisImageServiceItem::equal( const QgsDataItem *other )
{
  const QgsArcGisImageServiceItem *o = qobject_cast<const QgsArcGisImageServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisSceneServiceItem
//

QgsArcGisSceneServiceItem::QgsArcGisSceneServiceItem(
  QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, bool forceRefresh
)
  : QgsDataCollectionItem( parent, name, path, u"I3S"_s )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mForceRefresh( forceRefresh )
{
  mIconName = u"mIconEsriI3s.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}

QVector<QgsDataItem *> QgsArcGisSceneServiceItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix, mForceRefresh );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      auto error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats, mForceRefresh );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats, mForceRefresh );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, Qgis::ArcGisRestServiceType::SceneServer, mSupportedFormats, mForceRefresh );
  return items;
}

bool QgsArcGisSceneServiceItem::equal( const QgsDataItem *other )
{
  const QgsArcGisSceneServiceItem *o = qobject_cast<const QgsArcGisSceneServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

//
// QgsArcGisRestLayerItem
//

QgsArcGisRestLayerItem::QgsArcGisRestLayerItem(
  QgsDataItem *parent, const QString &url, const QString &title, const QgsCoordinateReferenceSystem &crs, Qgis::BrowserLayerType layerType, const QString &providerId
)
  : QgsLayerItem( parent, title, url, QString(), layerType, providerId )
  , mCrs( crs )
{}

QgsCoordinateReferenceSystem QgsArcGisRestLayerItem::crs() const
{
  return mCrs;
}


//
// QgsArcGisFeatureServiceLayerItem
//

QgsArcGisFeatureServiceLayerItem::QgsArcGisFeatureServiceLayerItem(
  QgsDataItem *parent,
  const QString &url,
  const QString &title,
  const QgsCoordinateReferenceSystem &crs,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString urlPrefix,
  Qgis::BrowserLayerType geometryType,
  bool isMapServerWithQueryCapability,
  const QString &mapServerFormat,
  const QString &mapServerLayerId
)
  : QgsArcGisRestLayerItem( parent, url, title, crs, geometryType, u"arcgisfeatureserver"_s )
{
  mUri = u"url='%1'"_s.arg( url );
  if ( !authcfg.isEmpty() )
    mUri += u" authcfg='%1'"_s.arg( authcfg );

  if ( !urlPrefix.isEmpty() )
    mUri += u" urlprefix='%1'"_s.arg( urlPrefix );

  mUri += headers.toSpacedString();

  if ( isMapServerWithQueryCapability )
  {
    const QString trimmedUrl = mapServerLayerId.isEmpty() ? url : url.left( url.length() - 1 - mapServerLayerId.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
    mMapServerUri = u"format='%1' layer='%2' url='%3'"_s.arg( mapServerFormat, mapServerLayerId, trimmedUrl );
    if ( !authcfg.isEmpty() )
      mMapServerUri += u" authcfg='%1'"_s.arg( authcfg );

    if ( !urlPrefix.isEmpty() )
      mMapServerUri += u" urlprefix='%1'"_s.arg( urlPrefix );

    mMapServerUri += headers.toSpacedString();

    setIconName( u"/mIconRasterVector.svg"_s );
  }

  setState( Qgis::BrowserItemState::Populated );
  setToolTip( url );
}

QgsMimeDataUtils::Uri QgsArcGisFeatureServiceLayerItem::mapServerMimeUri() const
{
  QgsMimeDataUtils::Uri uri;
  uri.layerType = QgsMapLayerFactory::typeToString( Qgis::LayerType::Raster );
  uri.providerKey = u"arcgismapserver"_s;
  uri.name = layerName();
  uri.uri = mMapServerUri;
  uri.supportedCrs = supportedCrs();

  return uri;
}

QList<QgsLayerItem::LayerUriWithDetails> QgsArcGisFeatureServiceLayerItem::layerUrisWithDetails() const
{
  LayerUriWithDetails vector;
  vector.uri = mimeUris().at( 0 );
  vector.userFriendlyDescription = tr( "Vector Layer (FeatureServer)" );
  if ( mMapServerUri.isEmpty() )
  {
    return { vector };
  }
  LayerUriWithDetails raster;
  raster.uri = mapServerMimeUri();
  raster.userFriendlyDescription = tr( "Raster Layer (MapServer)" );

  return { vector, raster };
}

//
// QgsArcGisMapServiceLayerItem
//

QgsArcGisMapServiceLayerItem::QgsArcGisMapServiceLayerItem(
  QgsDataItem *parent,
  const QString &url,
  const QString &id,
  const QString &title,
  const QgsCoordinateReferenceSystem &crs,
  const QString &format,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString &urlPrefix,
  bool isMapServerWithQueryCapability
)
  : QgsArcGisRestLayerItem( parent, url, title, crs, Qgis::BrowserLayerType::Raster, u"arcgismapserver"_s )
  , mIsMapServerWithQueryCapability( isMapServerWithQueryCapability )
{
  const QString trimmedUrl = id.isEmpty() ? url : url.left( url.length() - 1 - id.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
  mUri = u"format='%1' layer='%2' url='%3'"_s.arg( format, id, trimmedUrl );
  if ( !authcfg.isEmpty() )
    mUri += u" authcfg='%1'"_s.arg( authcfg );

  if ( !urlPrefix.isEmpty() )
    mUri += u" urlprefix='%1'"_s.arg( urlPrefix );

  mUri += headers.toSpacedString();

  setState( Qgis::BrowserItemState::Populated );
  setToolTip( mPath );
}

Qgis::BrowserItemFilterFlags QgsArcGisMapServiceLayerItem::filterFlags() const
{
  Qgis::BrowserItemFilterFlags res;
  if ( mIsMapServerWithQueryCapability )
  {
    res.setFlag( Qgis::BrowserItemFilterFlag::HideWhenNotFilteringByLayerType );
  }
  return res;
}


//
// QgsArcGisImageServiceLayerItem
//

QgsArcGisImageServiceLayerItem::QgsArcGisImageServiceLayerItem(
  QgsDataItem *parent,
  const QString &url,
  const QString &id,
  const QString &title,
  const QgsCoordinateReferenceSystem &crs,
  const QString &format,
  const QString &authcfg,
  const QgsHttpHeaders &headers,
  const QString &urlPrefix
)
  : QgsArcGisRestLayerItem( parent, url, title, crs, Qgis::BrowserLayerType::Raster, u"arcgisimageserver"_s )
{
  const QString trimmedUrl = id.isEmpty() ? url : url.left( url.length() - 1 - id.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
  mUri = u"format='%1' layer='%2' url='%3'"_s.arg( format, id, trimmedUrl );
  if ( !authcfg.isEmpty() )
    mUri += u" authcfg='%1'"_s.arg( authcfg );

  if ( !urlPrefix.isEmpty() )
    mUri += u" urlprefix='%1'"_s.arg( urlPrefix );

  mUri += headers.toSpacedString();

  setState( Qgis::BrowserItemState::Populated );
  setToolTip( mPath );
}

//
// QgsArcGisSceneServiceLayerItem
//

QgsArcGisSceneServiceLayerItem::QgsArcGisSceneServiceLayerItem(
  QgsDataItem *parent, const QString &url, const QString &title, const QgsCoordinateReferenceSystem &crs, const QString &authcfg, const QgsHttpHeaders &headers, const QString urlPrefix
)
  : QgsArcGisRestLayerItem( parent, url, title, crs, Qgis::BrowserLayerType::TiledScene, u"esrii3s"_s )
{
  mUri = u"url='%1'"_s.arg( url );
  if ( !authcfg.isEmpty() )
    mUri += u" authcfg='%1'"_s.arg( authcfg );

  if ( !urlPrefix.isEmpty() )
    mUri += u" urlprefix='%1'"_s.arg( urlPrefix );

  mUri += headers.toSpacedString();

  setState( Qgis::BrowserItemState::Populated );
  setToolTip( url );
}

//
// QgsArcGisRestParentLayerItem
//

QgsArcGisRestParentLayerItem::QgsArcGisRestParentLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix )
  : QgsDataItem( Qgis::BrowserItemType::Collection, parent, name, path )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconDbSchema.svg"_s;
  setToolTip( path );
}

bool QgsArcGisRestParentLayerItem::equal( const QgsDataItem *other )
{
  const QgsArcGisRestParentLayerItem *o = qobject_cast<const QgsArcGisRestParentLayerItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisRestDataItemProvider
//

QgsArcGisRestDataItemProvider::QgsArcGisRestDataItemProvider()
{}

QString QgsArcGisRestDataItemProvider::name()
{
  return u"AFS"_s;
}

Qgis::DataItemProviderCapabilities QgsArcGisRestDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsArcGisRestDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsArcGisRestRootItem( parentItem, QObject::tr( "ArcGIS REST Servers" ), u"arcgisfeatureserver:"_s );
  }

  // path schema: afs:/connection name (used by OWS)
  if ( path.startsWith( "afs:/"_L1 ) )
  {
    const QString connectionName = path.split( '/' ).last();
    if ( QgsArcGisConnectionSettings::sTreeConnectionArcgis->items().contains( connectionName ) )
    {
      return new QgsArcGisRestConnectionItem( parentItem, u"ArcGisFeatureServer"_s, path, connectionName );
    }
  }

  return nullptr;
}
