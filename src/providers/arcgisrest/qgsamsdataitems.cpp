/***************************************************************************
      qgsamsdataitems.cpp
      -------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsamsdataitems.h"
#include "qgsarcgisrestutils.h"
#include "qgsowsconnection.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"

#ifdef HAVE_GUI
#include "qgsamssourceselect.h"
#endif

#include <QImageReader>

QgsAmsRootItem::QgsAmsRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconAms.svg" );
  populate();
}

QVector<QgsDataItem *> QgsAmsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList connectionList = QgsOwsConnection::connectionList( QStringLiteral( "ARCGISMAPSERVER" ) );
  for ( const QString &connName : connectionList )
  {
    QString path = "ams:/" + connName;
    connections.append( new QgsAmsConnectionItem( this, connName, path, connName ) );
  }
  return connections;
}

#ifdef HAVE_GUI
QWidget *QgsAmsRootItem::paramWidget()
{
  QgsAmsSourceSelect *select = new QgsAmsSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsArcGisServiceSourceSelect::connectionsChanged, this, &QgsAmsRootItem::onConnectionsChanged );
  return select;
}

void QgsAmsRootItem::onConnectionsChanged()
{
  refresh();
}
#endif

///////////////////////////////////////////////////////////////////////////////


void addFolderItems( QVector< QgsDataItem * > &items, const QVariantMap &serviceData, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers, QgsDataItem *parent )
{
  QgsArcGisRestUtils::visitFolderItems( [parent, &baseUrl, &items, headers, authcfg]( const QString & name, const QString & url )
  {
    std::unique_ptr< QgsAmsFolderItem > folderItem = qgis::make_unique< QgsAmsFolderItem >( parent, name, url, baseUrl, authcfg, headers );
    items.append( folderItem.release() );
  }, serviceData, baseUrl );
}

void addServiceItems( QVector< QgsDataItem * > &items, const QVariantMap &serviceData, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers, QgsDataItem *parent )
{
  QgsArcGisRestUtils::visitServiceItems(
    [&items, parent, authcfg, headers]( const QString & name, const QString & url )
  {
    std::unique_ptr< QgsAmsServiceItem > serviceItem = qgis::make_unique< QgsAmsServiceItem >( parent, name, url, url, authcfg, headers );
    items.append( serviceItem.release() );
  }, serviceData, baseUrl, QgsArcGisRestUtils::Raster );
}

void addLayerItems( QVector< QgsDataItem * > &items, const QVariantMap &serviceData, const QString &parentUrl, const QString &authcfg, const QgsStringMap &headers, QgsDataItem *parent )
{
  QMap< QString, QgsDataItem * > layerItems;
  QMap< QString, QString > parents;

  QgsArcGisRestUtils::addLayerItems( [parent, &layerItems, &parents, authcfg, headers]( const QString & parentLayerId, const QString & id, const QString & name, const QString & description, const QString & url, bool, const QString & authid, const QString & format )
  {
    Q_UNUSED( description )

    if ( !parentLayerId.isEmpty() )
      parents.insert( id, parentLayerId );

    std::unique_ptr< QgsAmsLayerItem > layerItem = qgis::make_unique< QgsAmsLayerItem >( parent, name, url, id, name, authid, format, authcfg, headers );
    layerItems.insert( id, layerItem.release() );

  }, serviceData, parentUrl, QgsArcGisRestUtils::Raster );

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


QgsAmsConnectionItem::QgsAmsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName )
  : QgsDataCollectionItem( parent, name, path )
  , mConnName( connectionName )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsAmsConnectionItem::createChildren()
{
  const QgsOwsConnection connection( QStringLiteral( "ARCGISMAPSERVER" ), mConnName );
  const QString url = connection.uri().param( QStringLiteral( "url" ) );
  const QString authcfg = connection.uri().param( QStringLiteral( "authcfg" ) );
  const QString referer = connection.uri().param( QStringLiteral( "referer" ) );
  QgsStringMap headers;
  if ( ! referer.isEmpty() )
    headers[ QStringLiteral( "Referer" )] = referer;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;

  QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( url, authcfg, errorTitle,  errorMessage, headers );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = qgis::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, url, authcfg, headers, this );
  addServiceItems( items, serviceData, url, authcfg, headers, this );
  addLayerItems( items, serviceData, url, authcfg, headers, this );

  return items;
}

bool QgsAmsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsAmsConnectionItem *o = qobject_cast<const QgsAmsConnectionItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

QString QgsAmsConnectionItem::url() const
{
  const QgsOwsConnection connection( QStringLiteral( "ARCGISMAPSERVER" ), mConnName );
  return connection.uri().param( QStringLiteral( "url" ) );
}


QgsAmsFolderItem::QgsAmsFolderItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers )
  : QgsDataCollectionItem( parent, name, path )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Collapse;
  setToolTip( path );
}


QVector<QgsDataItem *> QgsAmsFolderItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = qgis::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, this );
  return items;
}

bool QgsAmsFolderItem::equal( const QgsDataItem *other )
{
  const QgsAmsFolderItem *o = qobject_cast<const QgsAmsFolderItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


QgsAmsServiceItem::QgsAmsServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers )
  : QgsDataCollectionItem( parent, name, path )
  , mBaseUrl( baseUrl )
  , mAuthCfg( authcfg )
  , mHeaders( headers )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  mCapabilities |= Collapse;
  setToolTip( path );
}

QVector<QgsDataItem *> QgsAmsServiceItem::createChildren()
{
  const QString url = mPath;

  QVector<QgsDataItem *> items;
  QString errorTitle, errorMessage;
  const QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( url, mAuthCfg, errorTitle, errorMessage, mHeaders );
  if ( serviceData.isEmpty() )
  {
    if ( !errorMessage.isEmpty() )
    {
      std::unique_ptr< QgsErrorItem > error = qgis::make_unique< QgsErrorItem >( this, tr( "Connection failed: %1" ).arg( errorTitle ), mPath + "/error" );
      error->setToolTip( errorMessage );
      items.append( error.release() );
      QgsDebugMsg( "Connection failed - " + errorMessage );
    }
    return items;
  }

  addFolderItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this );
  addServiceItems( items, serviceData, mBaseUrl, mAuthCfg, mHeaders, this );
  addLayerItems( items, serviceData, mPath, mAuthCfg, mHeaders, this );
  return items;
}

bool QgsAmsServiceItem::equal( const QgsDataItem *other )
{
  const QgsAmsServiceItem *o = qobject_cast<const QgsAmsServiceItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}


///////////////////////////////////////////////////////////////////////////////

QgsAmsLayerItem::QgsAmsLayerItem( QgsDataItem *parent, const QString &, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format, const QString &authcfg, const QgsStringMap &headers )
  : QgsLayerItem( parent, title, url, QString(), QgsLayerItem::Raster, QStringLiteral( "arcgismapserver" ) )
{
  QString trimmedUrl = id.isEmpty() ? url : url.left( url.length() - 1 - id.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
  mUri = QStringLiteral( "crs='%1' format='%2' layer='%3' url='%4'" ).arg( authid, format, id, trimmedUrl );
  if ( !authcfg.isEmpty() )
    mUri += QStringLiteral( " authcfg='%1'" ).arg( authcfg );
  if ( !headers.value( QStringLiteral( "Referer" ) ).isEmpty() )
    mUri += QStringLiteral( " referer='%1'" ).arg( headers.value( QStringLiteral( "Referer" ) ) );
  setState( Populated );
  mIconName = QStringLiteral( "mIconAms.svg" );
  setToolTip( mPath );
}

//
// QgsAmsDataItemProvider
//

QString QgsAmsDataItemProvider::name()
{
  return QStringLiteral( "AMS" );
}

int QgsAmsDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsAmsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsAmsRootItem( parentItem, QStringLiteral( "ArcGisMapServer" ), QStringLiteral( "arcgismapserver:" ) );
  }

  // path schema: ams:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "ams:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "arcgismapserver" ) ).contains( connectionName ) )
    {
      return new QgsAmsConnectionItem( parentItem, QStringLiteral( "ArcGisMapServer" ), path, connectionName );
    }
  }

  return nullptr;
}
