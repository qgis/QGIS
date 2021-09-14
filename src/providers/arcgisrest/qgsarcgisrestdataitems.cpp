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
#include "qgslogger.h"
#include "qgsowsconnection.h"
#include "qgsarcgisrestutils.h"
#include "qgsarcgisrestquery.h"
#include "qgsarcgisportalutils.h"
#include "qgsdataprovider.h"

#ifdef HAVE_GUI
#include "qgsarcgisrestsourceselect.h"
#endif

#include <QMessageBox>
#include <QCoreApplication>
#include <QSettings>
#include <QUrl>
#include "qgssettings.h"


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

  const QStringList connectionList = QgsOwsConnection::connectionList( QStringLiteral( "ARCGISFEATURESERVER" ) );
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

void addFolderItems( QVector< QgsDataItem * > &items, const QVariantMap &serviceData, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, QgsDataItem *parent,
                     const QString &supportedFormats )
{
  QgsArcGisRestQueryUtils::visitFolderItems( [parent, &baseUrl, &items, headers, authcfg, supportedFormats]( const QString & name, const QString & url )
  {
    std::unique_ptr< QgsArcGisRestFolderItem > folderItem = std::make_unique< QgsArcGisRestFolderItem >( parent, name, url, baseUrl, authcfg, headers );
    folderItem->setSupportedFormats( supportedFormats );
    items.append( folderItem.release() );
  }, serviceData, baseUrl );
}

void addServiceItems( QVector< QgsDataItem * > &items, const QVariantMap &serviceData, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, QgsDataItem *parent,
                      const QString &supportedFormats )
{
  QgsArcGisRestQueryUtils::visitServiceItems(
    [&items, parent, authcfg, headers, supportedFormats]( const QString & name, const QString & url, const QString & service, QgsArcGisRestQueryUtils::ServiceTypeFilter serviceType )
  {
    switch ( serviceType )
    {
      case QgsArcGisRestQueryUtils::Raster:
      {
        std::unique_ptr< QgsArcGisMapServiceItem > serviceItem = std::make_unique< QgsArcGisMapServiceItem >( parent, name, url, url, authcfg, headers, service );
        items.append( serviceItem.release() );
        break;
      }

      case QgsArcGisRestQueryUtils::Vector:
      {
        std::unique_ptr< QgsArcGisFeatureServiceItem > serviceItem = std::make_unique< QgsArcGisFeatureServiceItem >( parent, name, url, url, authcfg, headers );
        serviceItem->setSupportedFormats( supportedFormats );
        items.append( serviceItem.release() );
        break;
      }

      case QgsArcGisRestQueryUtils::AllTypes:
        break;
    }
  }, serviceData, baseUrl );
}

void addLayerItems( QVector< QgsDataItem * > &items, const QVariantMap &serviceData, const QString &parentUrl, const QString &authcfg, const QgsHttpHeaders &headers, QgsDataItem *parent, QgsArcGisRestQueryUtils::ServiceTypeFilter serviceTypeFilter,
                    const QString &supportedFormats )
{
  QMultiMap< QString, QgsDataItem * > layerItems;
  QMap< QString, QString > parents;

  QgsArcGisRestQueryUtils::addLayerItems( [parent, &layerItems, &parents, authcfg, headers, serviceTypeFilter, supportedFormats]( const QString & parentLayerId, QgsArcGisRestQueryUtils::ServiceTypeFilter serviceType, QgsWkbTypes::GeometryType geometryType, const QString & id, const QString & name, const QString & description, const QString & url, bool isParent, const QString & authid, const QString & format )
  {
    Q_UNUSED( description )

    if ( !parentLayerId.isEmpty() )
      parents.insert( id, parentLayerId );

    if ( isParent && serviceType != QgsArcGisRestQueryUtils::Raster )
    {
      if ( !layerItems.value( id ) )
      {
        std::unique_ptr< QgsArcGisRestParentLayerItem > layerItem = std::make_unique< QgsArcGisRestParentLayerItem >( parent, name, url, authcfg, headers );
        layerItems.insert( id, layerItem.release() );
      }
    }
    else
    {
      std::unique_ptr< QgsDataItem > layerItem;
      switch ( serviceTypeFilter == QgsArcGisRestQueryUtils::AllTypes ? serviceType : serviceTypeFilter )
      {
        case QgsArcGisRestQueryUtils::Vector:
          layerItem = std::make_unique< QgsArcGisFeatureServiceLayerItem >( parent, name, url, name, authid, authcfg, headers, geometryType == QgsWkbTypes::PolygonGeometry ? Qgis::BrowserLayerType::Polygon :
                      geometryType == QgsWkbTypes::LineGeometry ? Qgis::BrowserLayerType::Line
                      : geometryType == QgsWkbTypes::PointGeometry ? Qgis::BrowserLayerType::Point : Qgis::BrowserLayerType::Vector );
          break;

        case QgsArcGisRestQueryUtils::Raster:
          layerItem = std::make_unique< QgsArcGisMapServiceLayerItem >( parent, name, url, id, name, authid, format, authcfg, headers );
          static_cast< QgsArcGisMapServiceLayerItem * >( layerItem.get() )->setSupportedFormats( supportedFormats );
          break;

        case QgsArcGisRestQueryUtils::AllTypes:
          break;
      }
      if ( layerItem )
        layerItems.insert( id, layerItem.release() );
    }

  }, serviceData, parentUrl, supportedFormats, serviceTypeFilter );

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

  const QgsSettings settings;
  const QString key = QStringLiteral( "qgis/connections-arcgisfeatureserver/" ) + mConnName;
  mPortalContentEndpoint = settings.value( key + "/content_endpoint" ).toString();
  mPortalCommunityEndpoint = settings.value( key + "/community_endpoint" ).toString();
}

QVector<QgsDataItem *> QgsArcGisRestConnectionItem::createChildren()
{
  const QgsOwsConnection connection( QStringLiteral( "ARCGISFEATURESERVER" ), mConnName );
  const QString url = connection.uri().param( QStringLiteral( "url" ) );
  const QString authcfg = connection.uri().authConfigId();
  const QString referer = connection.uri().param( QStringLiteral( "referer" ) );
  QgsHttpHeaders headers;
  if ( ! referer.isEmpty() )
    headers[ QStringLiteral( "referer" )] = referer;

  QVector<QgsDataItem *> items;
  if ( !mPortalCommunityEndpoint.isEmpty() && !mPortalContentEndpoint.isEmpty() )
  {
    items << new QgsArcGisPortalGroupsItem( this, QStringLiteral( "groups" ), authcfg, headers, mPortalCommunityEndpoint, mPortalContentEndpoint );
    items << new QgsArcGisRestServicesItem( this, url, QStringLiteral( "services" ), authcfg, headers );
  }
  else
  {
    QString errorTitle, errorMessage;
    const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, authcfg, errorTitle, errorMessage, headers );
    if ( serviceData.isEmpty() )
    {
      if ( !errorMessage.isEmpty() )
      {
        std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
        error->setToolTip( errorMessage );
        items.append( error.release() );
        QgsDebugMsg( "Connection failed - " + errorMessage );
      }
      return items;
    }

    addFolderItems( items, serviceData, url, authcfg, headers, this, QString() );
    addServiceItems( items, serviceData, url, authcfg, headers, this, QString() );
    addLayerItems( items, serviceData, url, authcfg, headers, this, QgsArcGisRestQueryUtils::AllTypes, QString() );
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
  const QgsOwsConnection connection( QStringLiteral( "ARCGISFEATURESERVER" ), mConnName );
  return connection.uri().param( QStringLiteral( "url" ) );
}


//
// QgsArcGisPortalGroupsItem
//

QgsArcGisPortalGroupsItem::QgsArcGisPortalGroupsItem( QgsDataItem *parent, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers, const QString &communityEndpoint, const QString &contentEndpoint )
  : QgsDataCollectionItem( parent, tr( "Groups" ), path, QStringLiteral( "AFS" ) )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
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
      std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  for ( const QVariant &group : groups )
  {
    const QVariantMap groupData = group.toMap();
    items << new QgsArcGisPortalGroupItem( this, groupData.value( QStringLiteral( "id" ) ).toString(),
                                           groupData.value( QStringLiteral( "title" ) ).toString(),
                                           mAuthCfg, mHeaders, mPortalCommunityEndpoint, mPortalContentEndpoint );
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
QgsArcGisPortalGroupItem::QgsArcGisPortalGroupItem( QgsDataItem *parent, const QString &groupId, const QString &name, const QString &authcfg, const QgsHttpHeaders &headers, const QString &communityEndpoint, const QString &contentEndpoint )
  : QgsDataCollectionItem( parent, name, groupId, QStringLiteral( "AFS" ) )
  , mId( groupId )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
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
  const QVariantList groupItems = QgsArcGisPortalUtils::retrieveGroupItemsOfType( mPortalContentEndpoint, mId, mAuthCfg, QList<int >() << QgsArcGisPortalUtils::FeatureService
                                  << QgsArcGisPortalUtils::MapService
                                  << QgsArcGisPortalUtils::ImageService,
                                  errorTitle, errorMessage, mHeaders );
  if ( groupItems.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  for ( const QVariant &item : groupItems )
  {
    const QVariantMap itemData = item.toMap();

    if ( itemData.value( QStringLiteral( "type" ) ).toString().compare( QStringLiteral( "Feature Service" ), Qt::CaseInsensitive ) == 0 )
    {
      items << new QgsArcGisFeatureServiceItem( this, itemData.value( QStringLiteral( "title" ) ).toString(),
            itemData.value( QStringLiteral( "url" ) ).toString(),
            itemData.value( QStringLiteral( "url" ) ).toString(), mAuthCfg, mHeaders );
    }
    else
    {
      items << new QgsArcGisMapServiceItem( this, itemData.value( QStringLiteral( "title" ) ).toString(),
                                            itemData.value( QStringLiteral( "url" ) ).toString(),
                                            itemData.value( QStringLiteral( "url" ) ).toString(), mAuthCfg, mHeaders, itemData.value( QStringLiteral( "type" ) ).toString().compare( QStringLiteral( "Map Service" ), Qt::CaseInsensitive ) == 0 ? QStringLiteral( "MapServer" ) : QStringLiteral( "ImageServer" ) );
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

QgsArcGisRestServicesItem::QgsArcGisRestServicesItem( QgsDataItem *parent, const QString &url, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers )
  : QgsDataCollectionItem( parent, tr( "Services" ), path, QStringLiteral( "AFS" ) )
  , mUrl( url )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsArcGisRestServicesItem::createChildren()
{
  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( mUrl, mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mUrl, mAuthCfg, mHeaders, this, QString() );
  addServiceItems( items, serviceData, mUrl, mAuthCfg, mHeaders, this, QString() );
  addLayerItems( items, serviceData, mUrl, mAuthCfg, mHeaders, this, QgsArcGisRestQueryUtils::AllTypes, QString() );
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
QgsArcGisRestFolderItem::QgsArcGisRestFolderItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AFS" ) )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
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
      std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this, mSupportedFormats );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this, mSupportedFormats );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, this, QgsArcGisRestQueryUtils::Vector, mSupportedFormats );
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
QgsArcGisFeatureServiceItem::QgsArcGisFeatureServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AFS" ) )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
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
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this, mSupportedFormats );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this, mSupportedFormats );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, this, QgsArcGisRestQueryUtils::Vector, mSupportedFormats );
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

QgsArcGisMapServiceItem::QgsArcGisMapServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &serviceType )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "AMS" ) )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
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
  const QVariantMap serviceData = QgsArcGisRestQueryUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = std::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  const QString supportedFormats = mServiceType == QLatin1String( "ImageServer" ) ?
                                   QStringLiteral( "JPGPNG,PNG,PNG8,PNG24,JPG,BMP,GIF,TIFF,PNG32,BIP,BSQ,LERC" ) // ImageServer supported formats
                                   : serviceData.value( QStringLiteral( "supportedImageFormatTypes" ) ).toString();

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this, supportedFormats );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this, supportedFormats );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, this, QgsArcGisRestQueryUtils::AllTypes, supportedFormats );
  return items;
}

bool QgsArcGisMapServiceItem::equal( const QgsDataItem *other )
{
  const QgsArcGisMapServiceItem *o = qobject_cast<const QgsArcGisMapServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


//
// QgsArcGisFeatureServiceLayerItem
//

QgsArcGisFeatureServiceLayerItem::QgsArcGisFeatureServiceLayerItem( QgsDataItem *parent, const QString &, const QString &url, const QString &title, const QString &authid, const QString &authcfg, const QgsHttpHeaders &headers, Qgis::BrowserLayerType geometryType )
  : QgsLayerItem( parent, title, url, QString(), geometryType, QStringLiteral( "arcgisfeatureserver" ) )
{
  mUri = QStringLiteral( "crs='%1' url='%2'" ).arg( authid, url );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );
  if ( !headers [ QStringLiteral( "referer" ) ].toString().isEmpty() )
    mUri += QStringLiteral( " referer='%1'" ).arg( headers[ QStringLiteral( "referer" ) ].toString() );
  setState( Qgis::BrowserItemState::Populated );
  setToolTip( url );
}

//
// QgsArcGisMapServiceLayerItem
//

QgsArcGisMapServiceLayerItem::QgsArcGisMapServiceLayerItem( QgsDataItem *parent, const QString &, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format, const QString &authcfg, const QgsHttpHeaders &headers )
  : QgsLayerItem( parent, title, url, QString(), Qgis::BrowserLayerType::Raster, QStringLiteral( "arcgismapserver" ) )
{
  const QString trimmedUrl = id.isEmpty() ? url : url.left( url.length() - 1 - id.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
  mUri = QStringLiteral( "crs='%1' format='%2' layer='%3' url='%4'" ).arg( authid, format, id, trimmedUrl );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );
  if ( !headers [ QStringLiteral( "referer" ) ].toString().isEmpty() )
    mUri += QStringLiteral( " referer='%1'" ).arg( headers [ QStringLiteral( "referer" ) ].toString() );
  setState( Qgis::BrowserItemState::Populated );
  setToolTip( mPath );
}

//
// QgsArcGisRestParentLayerItem
//

QgsArcGisRestParentLayerItem::QgsArcGisRestParentLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers )
  : QgsDataItem( Qgis::BrowserItemType::Collection, parent, name, path )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
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
  // migrate legacy map services by moving them to feature server group

  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-arcgismapserver" ) );
  const QStringList legacyServices = settings.childGroups();
  settings.endGroup();
  settings.beginGroup( QStringLiteral( "qgis/connections-arcgisfeatureserver" ) );
  QStringList existingServices = settings.childGroups();
  settings.endGroup();
  for ( const QString &legacyService : legacyServices )
  {
    QString newName = legacyService;
    int i = 1;
    while ( existingServices.contains( newName ) )
    {
      i ++;
      newName = QStringLiteral( "%1 (%2)" ).arg( legacyService ).arg( i );
    }

    settings.beginGroup( QStringLiteral( "qgis/connections-arcgismapserver/%1" ).arg( legacyService ) );
    const QStringList keys = settings.childKeys();
    settings.endGroup();
    for ( const QString &key : keys )
    {
      const QString oldKey = QStringLiteral( "qgis/connections-arcgismapserver/%1/%2" ).arg( legacyService, key );
      const QString newKey = QStringLiteral( "qgis/connections-arcgisfeatureserver/%1/%2" ).arg( newName, key );
      settings.setValue( newKey, settings.value( oldKey ) );
    }

    settings.remove( QStringLiteral( "qgis/connections-arcgismapserver/%1" ).arg( legacyService ) );
    existingServices.append( newName );
  }
}

QString QgsArcGisRestDataItemProvider::name()
{
  return QStringLiteral( "AFS" );
}

int QgsArcGisRestDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
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
    if ( QgsOwsConnection::connectionList( QStringLiteral( "arcgisfeatureserver" ) ).contains( connectionName ) )
    {
      return new QgsArcGisRestConnectionItem( parentItem, QStringLiteral( "ArcGisFeatureServer" ), path, connectionName );
    }
  }

  return nullptr;
}

