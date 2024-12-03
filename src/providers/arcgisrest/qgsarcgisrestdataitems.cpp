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
#include "moc_qgsarcgisrestdataitems.cpp"
#include "qgslogger.h"
#include "qgsowsconnection.h"
#include "qgsarcgisrestutils.h"
#include "qgsarcgisrestquery.h"
#include "qgsarcgisportalutils.h"
#include "qgssettingsentryimpl.h"

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
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "AFS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconAfs.svg" );
  populate();
}

QVector<QgsDataItem *> QgsArcGisRestRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList connectionList = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items();
  for ( const QString &connName : connectionList )
  {
    const QString path = QStringLiteral( "afs:/" ) + connName;
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

void addFolderItems( QVector<QgsDataItem *> &items, const QVariantMap &serviceData, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, QgsDataItem *parent, const QString &supportedFormats )
{
  QgsArcGisRestQueryUtils::visitFolderItems( [parent, &baseUrl, &items, headers, urlPrefix, authcfg, supportedFormats]( const QString &name, const QString &url ) {
    std::unique_ptr<QgsArcGisRestFolderItem> folderItem = std::make_unique<QgsArcGisRestFolderItem>( parent, name, url, baseUrl, authcfg, headers, urlPrefix );
    folderItem->setSupportedFormats( supportedFormats );
    items.append( folderItem.release() );
  },
                                             serviceData, baseUrl );
}

void addServiceItems( QVector<QgsDataItem *> &items, const QVariantMap &serviceData, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, QgsDataItem *parent, const QString &supportedFormats )
{
  QgsArcGisRestQueryUtils::visitServiceItems(
    [&items, parent, authcfg, headers, urlPrefix, supportedFormats]( const QString &name, const QString &url, Qgis::ArcGisRestServiceType serviceType ) {
      switch ( serviceType )
      {
        case Qgis::ArcGisRestServiceType::MapServer:
        case Qgis::ArcGisRestServiceType::ImageServer:
        {
          std::unique_ptr<QgsArcGisMapServiceItem> serviceItem = std::make_unique<QgsArcGisMapServiceItem>( parent, name, url, url, authcfg, headers, urlPrefix, serviceType );
          items.append( serviceItem.release() );
          break;
        }

        case Qgis::ArcGisRestServiceType::FeatureServer:
        {
          std::unique_ptr<QgsArcGisFeatureServiceItem> serviceItem = std::make_unique<QgsArcGisFeatureServiceItem>( parent, name, url, url, authcfg, headers, urlPrefix );
          serviceItem->setSupportedFormats( supportedFormats );
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
    serviceData, baseUrl
  );
}

void addLayerItems( QVector<QgsDataItem *> &items, const QVariantMap &serviceData, const QString &parentUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString urlPrefix, QgsDataItem *parent, QgsArcGisRestQueryUtils::ServiceTypeFilter serviceTypeFilter, const QString &supportedFormats )
{
  QMultiMap<QString, QgsDataItem *> layerItems;
  QMap<QString, QString> parents;

  QgsArcGisRestQueryUtils::addLayerItems( [parent, &layerItems, &parents, authcfg, headers, urlPrefix, serviceTypeFilter, supportedFormats]( const QString &parentLayerId, QgsArcGisRestQueryUtils::ServiceTypeFilter serviceType, Qgis::GeometryType geometryType, const QString &id, const QString &name, const QString &description, const QString &url, bool isParent, const QgsCoordinateReferenceSystem &crs, const QString &format ) {
    Q_UNUSED( description )

    if ( !parentLayerId.isEmpty() )
      parents.insert( id, parentLayerId );

    if ( isParent && serviceType != QgsArcGisRestQueryUtils::ServiceTypeFilter::Raster )
    {
      if ( !layerItems.value( id ) )
      {
        std::unique_ptr<QgsArcGisRestParentLayerItem> layerItem = std::make_unique<QgsArcGisRestParentLayerItem>( parent, name, url, authcfg, headers, urlPrefix );
        layerItems.insert( id, layerItem.release() );
      }
    }
    else
    {
      std::unique_ptr<QgsDataItem> layerItem;
      switch ( serviceTypeFilter == QgsArcGisRestQueryUtils::ServiceTypeFilter::AllTypes ? serviceType : serviceTypeFilter )
      {
        case QgsArcGisRestQueryUtils::ServiceTypeFilter::Vector:
          layerItem = std::make_unique<QgsArcGisFeatureServiceLayerItem>( parent, url, name, crs, authcfg, headers, urlPrefix, geometryType == Qgis::GeometryType::Polygon ? Qgis::BrowserLayerType::Polygon : geometryType == Qgis::GeometryType::Line  ? Qgis::BrowserLayerType::Line
                                                                                                                                                                                                             : geometryType == Qgis::GeometryType::Point ? Qgis::BrowserLayerType::Point
                                                                                                                                                                                                             : geometryType == Qgis::GeometryType::Null  ? Qgis::BrowserLayerType::TableLayer
                                                                                                                                                                                                                                                         : Qgis::BrowserLayerType::Vector );
          break;

        case QgsArcGisRestQueryUtils::ServiceTypeFilter::Raster:
          layerItem = std::make_unique<QgsArcGisMapServiceLayerItem>( parent, url, id, name, crs, format, authcfg, headers, urlPrefix );
          static_cast<QgsArcGisMapServiceLayerItem *>( layerItem.get() )->setSupportedFormats( supportedFormats );
          break;

        case QgsArcGisRestQueryUtils::ServiceTypeFilter::AllTypes:
          break;
      }
      if ( layerItem )
        layerItems.insert( id, layerItem.release() );
    }
  },
                                          serviceData, parentUrl, supportedFormats, serviceTypeFilter );

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
}

//
// QgsArcGisRestConnectionItem
//

QgsArcGisRestConnectionItem::QgsArcGisRestConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AFS" ) )
  , mConnName( connectionName )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  mPortalContentEndpoint = QgsArcGisConnectionSettings::settingsContentEndpoint->value( connectionName );
  mPortalCommunityEndpoint = QgsArcGisConnectionSettings::settingsCommunityEndpoint->value( connectionName );
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
    items << new QgsArcGisPortalGroupsItem( this, QStringLiteral( "groups" ), authcfg, headers, urlPrefix, mPortalCommunityEndpoint, mPortalContentEndpoint );
    items << new QgsArcGisRestServicesItem( this, url, QStringLiteral( "services" ), authcfg, headers, urlPrefix );
  }
  else
  {
    QString errorTitle, errorMessage;
    const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, authcfg, errorTitle, errorMessage, headers, urlPrefix );
    if ( serviceData.isEmpty() )
    {
      if ( !errorMessage.isEmpty() )
      {
        std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
        error->setToolTip( errorMessage );
        items.append( error.release() );
        QgsDebugError( "Connection failed - " + errorMessage );
      }
      return items;
    }

    addFolderItems( items, serviceData, url, authcfg, headers, urlPrefix, this, QString() );
    addServiceItems( items, serviceData, url, authcfg, headers, urlPrefix, this, QString() );
    addLayerItems( items, serviceData, url, authcfg, headers, urlPrefix, this, QgsArcGisRestQueryUtils::ServiceTypeFilter::AllTypes, QString() );
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

QgsArcGisPortalGroupsItem::QgsArcGisPortalGroupsItem( QgsDataItem *parent, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, const QString &communityEndpoint, const QString &contentEndpoint )
  : QgsDataCollectionItem( parent, tr( "Groups" ), path, QStringLiteral( "AFS" ) )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mPortalCommunityEndpoint( communityEndpoint )
  , mPortalContentEndpoint( contentEndpoint )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}


QVector<QgsDataItem *> QgsArcGisPortalGroupsItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QString errorTitle;
  QString errorMessage;
  const QVariantList groups = QgsArcGisPortalUtils::retrieveUserGroups( mPortalCommunityEndpoint, QString(), mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( groups.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  for ( const QVariant &group : groups )
  {
    const QVariantMap groupData = group.toMap();
    items << new QgsArcGisPortalGroupItem( this, groupData.value( QStringLiteral( "id" ) ).toString(), groupData.value( QStringLiteral( "title" ) ).toString(), mAuthCfg, mHeaders, mUrlPrefix, mPortalCommunityEndpoint, mPortalContentEndpoint );
    items.last()->setToolTip( groupData.value( QStringLiteral( "snippet" ) ).toString() );
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
QgsArcGisPortalGroupItem::QgsArcGisPortalGroupItem( QgsDataItem *parent, const QString &groupId, const QString &name, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, const QString &communityEndpoint, const QString &contentEndpoint )
  : QgsDataCollectionItem( parent, name, groupId, QStringLiteral( "AFS" ) )
  , mId( groupId )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mPortalCommunityEndpoint( communityEndpoint )
  , mPortalContentEndpoint( contentEndpoint )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( name );
}

QVector<QgsDataItem *> QgsArcGisPortalGroupItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QString errorTitle;
  QString errorMessage;
  const QVariantList groupItems = QgsArcGisPortalUtils::retrieveGroupItemsOfType( mPortalContentEndpoint, mId, mAuthCfg, QList<int>() << static_cast<int>( Qgis::ArcGisRestServiceType::FeatureServer ) << static_cast<int>( Qgis::ArcGisRestServiceType::MapServer ) << static_cast<int>( Qgis::ArcGisRestServiceType::ImageServer ), errorTitle, errorMessage, mHeaders );
  if ( groupItems.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  for ( const QVariant &item : groupItems )
  {
    const QVariantMap itemData = item.toMap();

    if ( itemData.value( QStringLiteral( "type" ) ).toString().compare( QStringLiteral( "Feature Service" ), Qt::CaseInsensitive ) == 0 )
    {
      items << new QgsArcGisFeatureServiceItem( this, itemData.value( QStringLiteral( "title" ) ).toString(), itemData.value( QStringLiteral( "url" ) ).toString(), itemData.value( QStringLiteral( "url" ) ).toString(), mAuthCfg, mHeaders, mUrlPrefix );
    }
    else
    {
      items << new QgsArcGisMapServiceItem( this, itemData.value( QStringLiteral( "title" ) ).toString(), itemData.value( QStringLiteral( "url" ) ).toString(), itemData.value( QStringLiteral( "url" ) ).toString(), mAuthCfg, mHeaders, mUrlPrefix, itemData.value( QStringLiteral( "type" ) ).toString().compare( QStringLiteral( "Map Service" ), Qt::CaseInsensitive ) == 0 ? Qgis::ArcGisRestServiceType::MapServer : Qgis::ArcGisRestServiceType::ImageServer );
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

QgsArcGisRestServicesItem::QgsArcGisRestServicesItem( QgsDataItem *parent, const QString &url, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers, const QString urlPrefix )
  : QgsDataCollectionItem( parent, tr( "Services" ), path, QStringLiteral( "AFS" ) )
  , mUrl( url )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsArcGisRestServicesItem::createChildren()
{
  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( mUrl, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mUrl, mAuthCfg, mHeaders, mUrlPrefix, this, QString() );
  addServiceItems( items, serviceData, mUrl, mAuthCfg, mHeaders, mUrlPrefix, this, QString() );
  addLayerItems( items, serviceData, mUrl, mAuthCfg, mHeaders, mUrlPrefix, this, QgsArcGisRestQueryUtils::ServiceTypeFilter::AllTypes, QString() );
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
QgsArcGisRestFolderItem::QgsArcGisRestFolderItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AFS" ) )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
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
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, QgsArcGisRestQueryUtils::ServiceTypeFilter::Vector, mSupportedFormats );
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
QgsArcGisFeatureServiceItem::QgsArcGisFeatureServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AFS" ) )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
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
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, mSupportedFormats );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, QgsArcGisRestQueryUtils::ServiceTypeFilter::Vector, mSupportedFormats );
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

QgsArcGisMapServiceItem::QgsArcGisMapServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix, Qgis::ArcGisRestServiceType serviceType )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AMS" ) )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
  , mUrlPrefix( urlPrefix )
  , mServiceType( serviceType )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
  setToolTip( path );
}

QVector<QgsDataItem *> QgsArcGisMapServiceItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders, mUrlPrefix );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr<QgsErrorItem> error = std::make_unique<QgsErrorItem>( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugError( "Connection failed - " + errorMessage );
    }
    return items;
  }

  const QString supportedFormats = mServiceType == Qgis::ArcGisRestServiceType::ImageServer ? QStringLiteral( "JPGPNG,PNG,PNG8,PNG24,JPG,BMP,GIF,TIFF,PNG32,BIP,BSQ,LERC" ) // ImageServer supported formats
                                                                                            : serviceData.value( QStringLiteral( "supportedImageFormatTypes" ) ).toString();

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, supportedFormats );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, mUrlPrefix, this, supportedFormats );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, mUrlPrefix, this, QgsArcGisRestQueryUtils::ServiceTypeFilter::AllTypes, supportedFormats );
  return items;
}

bool QgsArcGisMapServiceItem::equal( const QgsDataItem *other )
{
  const QgsArcGisMapServiceItem *o = qobject_cast<const QgsArcGisMapServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

//
// QgsArcGisRestLayerItem
//

QgsArcGisRestLayerItem::QgsArcGisRestLayerItem( QgsDataItem *parent, const QString &url, const QString &title, const QgsCoordinateReferenceSystem &crs, Qgis::BrowserLayerType layerType, const QString &providerId )
  : QgsLayerItem( parent, title, url, QString(), layerType, providerId )
  , mCrs( crs )
{
}

QgsCoordinateReferenceSystem QgsArcGisRestLayerItem::crs() const
{
  return mCrs;
}


//
// QgsArcGisFeatureServiceLayerItem
//

QgsArcGisFeatureServiceLayerItem::QgsArcGisFeatureServiceLayerItem( QgsDataItem *parent, const QString &url, const QString &title, const QgsCoordinateReferenceSystem &crs, const QString &authcfg, const QgsHttpHeaders &headers, const QString urlPrefix, Qgis::BrowserLayerType geometryType )
  : QgsArcGisRestLayerItem( parent, url, title, crs, geometryType, QStringLiteral( "arcgisfeatureserver" ) )
{
  mUri = QStringLiteral( "url='%1'" ).arg( url );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );

  if ( !urlPrefix.isEmpty() )
    mUri += QStringLiteral( " urlprefix='%1'" ).arg( urlPrefix );

  mUri += headers.toSpacedString();

  setState( Qgis::BrowserItemState::Populated );
  setToolTip( url );
}

//
// QgsArcGisMapServiceLayerItem
//

QgsArcGisMapServiceLayerItem::QgsArcGisMapServiceLayerItem( QgsDataItem *parent, const QString &url, const QString &id, const QString &title, const QgsCoordinateReferenceSystem &crs, const QString &format, const QString &authcfg, const QgsHttpHeaders &headers, const QString &urlPrefix )
  : QgsArcGisRestLayerItem( parent, url, title, crs, Qgis::BrowserLayerType::Raster, QStringLiteral( "arcgismapserver" ) )
{
  const QString trimmedUrl = id.isEmpty() ? url : url.left( url.length() - 1 - id.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
  mUri = QStringLiteral( "format='%1' layer='%2' url='%3'" ).arg( format, id, trimmedUrl );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );

  if ( !urlPrefix.isEmpty() )
    mUri += QStringLiteral( " urlprefix='%1'" ).arg( urlPrefix );

  mUri += headers.toSpacedString();

  setState( Qgis::BrowserItemState::Populated );
  setToolTip( mPath );
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
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
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
{
}

QString QgsArcGisRestDataItemProvider::name()
{
  return QStringLiteral( "AFS" );
}

Qgis::DataItemProviderCapabilities QgsArcGisRestDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsArcGisRestDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsArcGisRestRootItem( parentItem, QObject::tr( "ArcGIS REST Servers" ), QStringLiteral( "arcgisfeatureserver:" ) );
  }

  // path schema: afs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "afs:/" ) ) )
  {
    const QString connectionName = path.split( '/' ).last();
    if ( QgsArcGisConnectionSettings::sTreeConnectionArcgis->items().contains( connectionName ) )
    {
      return new QgsArcGisRestConnectionItem( parentItem, QStringLiteral( "ArcGisFeatureServer" ), path, connectionName );
    }
  }

  return nullptr;
}
