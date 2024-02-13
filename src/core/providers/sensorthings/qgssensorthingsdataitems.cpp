/***************************************************************************
    qgssensorthingsdataitems.cpp
    ---------------------
    begin                : December 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssensorthingsdataitems.h"
#include "qgsprovidermetadata.h"
#include "qgssensorthingsconnection.h"
#include "qgssensorthingsprovider.h"
#include "qgssensorthingsutils.h"
#include "qgsdataprovider.h"
#include "qgsproviderregistry.h"

///@cond PRIVATE

//
// QgsSensorThingsRootItem
//

QgsSensorThingsRootItem::QgsSensorThingsRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "sensorthings" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconSensorThings.svg" );
  populate();
}

QVector<QgsDataItem *> QgsSensorThingsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsSensorThingsProviderConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    QgsDataItem *conn = new QgsSensorThingsConnectionItem( this, connName, mPath + '/' + connName );
    connections.append( conn );
  }
  return connections;
}

//
// QgsSensorThingsConnectionItem
//

QgsSensorThingsConnectionItem::QgsSensorThingsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "sensorthings" ) )
  , mConnName( name )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::Fast;
  populate();


}

bool QgsSensorThingsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsSensorThingsConnectionItem *o = qobject_cast<const QgsSensorThingsConnectionItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

QVector<QgsDataItem *> QgsSensorThingsConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QgsSensorThingsProviderConnection::Data connectionData = QgsSensorThingsProviderConnection::connection( mConnName );
  const QString uri = QgsSensorThingsProviderConnection::encodedLayerUri( connectionData );
  const QVariantMap connectionUriParts = QgsProviderRegistry::instance()->decodeUri(
      QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, uri );

  for ( Qgis::SensorThingsEntity entity :
        {
          Qgis::SensorThingsEntity::Thing,
          Qgis::SensorThingsEntity::Location,
          Qgis::SensorThingsEntity::HistoricalLocation,
          Qgis::SensorThingsEntity::Datastream,
          Qgis::SensorThingsEntity::Sensor,
          Qgis::SensorThingsEntity::ObservedProperty,
          Qgis::SensorThingsEntity::Observation,
          Qgis::SensorThingsEntity::FeatureOfInterest,
        } )
  {
    QVariantMap entityUriParts = connectionUriParts;
    entityUriParts.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( entity ) );

    if ( QgsSensorThingsUtils::entityTypeHasGeometry( entity ) )
    {
      children.append( new QgsSensorThingsEntityContainerItem( this,
                       QgsSensorThingsUtils::displayString( entity, true ),
                       mPath + '/' + qgsEnumValueToKey( entity ),
                       QgsProviderRegistry::instance()->encodeUri(
                         QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, entityUriParts ) ) );
    }
    else
    {
      children.append( new QgsSensorThingsLayerEntityItem( this,
                       QgsSensorThingsUtils::displayString( entity, true ),
                       mPath + '/' + qgsEnumValueToKey( entity ),
                       QgsProviderRegistry::instance()->encodeUri(
                         QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, entityUriParts ),
                       QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                       Qgis::BrowserLayerType::TableLayer ) );
    }
  }

  return children;
}


//
// QgsSensorThingsEntityContainerItem
//

QgsSensorThingsEntityContainerItem::QgsSensorThingsEntityContainerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &entityUri )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "sensorthings" ) )
  , mEntityUri( entityUri )
{
  mCapabilities |= Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::Fast;
  populate();
}

bool QgsSensorThingsEntityContainerItem::equal( const QgsDataItem *other )
{
  const QgsSensorThingsEntityContainerItem *o = qobject_cast<const QgsSensorThingsEntityContainerItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName );
}

QVector<QgsDataItem *> QgsSensorThingsEntityContainerItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QVariantMap entityUriParts = QgsProviderRegistry::instance()->decodeUri(
                                       QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, mEntityUri );

  int sortKey = 1;
  for ( const Qgis::WkbType wkbType :
        {
          Qgis::WkbType::Point,
          Qgis::WkbType::MultiPoint,
          Qgis::WkbType::MultiLineString,
          Qgis::WkbType::MultiPolygon
        } )
  {
    QVariantMap geometryUriParts = entityUriParts;
    QString name;
    Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::TableLayer;
    switch ( wkbType )
    {
      case Qgis::WkbType::Point:
        geometryUriParts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "point" ) );
        name = tr( "Points" );
        layerType = Qgis::BrowserLayerType::Point;
        break;
      case Qgis::WkbType::MultiPoint:
        geometryUriParts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "multipoint" ) );
        name = tr( "MultiPoints" );
        layerType = Qgis::BrowserLayerType::Point;
        break;
      case Qgis::WkbType::MultiLineString:
        geometryUriParts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "line" ) );
        name = tr( "Lines" );
        layerType = Qgis::BrowserLayerType::Line;
        break;
      case Qgis::WkbType::MultiPolygon:
        geometryUriParts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "polygon" ) );
        name = tr( "Polygons" );
        layerType = Qgis::BrowserLayerType::Polygon;
        break;
      default:
        break;
    }
    children.append( new QgsSensorThingsLayerEntityItem( this,
                     name,
                     mPath + '/' + name,
                     QgsProviderRegistry::instance()->encodeUri(
                       QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, geometryUriParts ),
                     QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                     layerType ) );
    children.last()->setSortKey( sortKey++ );
  }

  return children;
}

//
// QgsSensorThingsLayerEntityItem
//

QgsSensorThingsLayerEntityItem::QgsSensorThingsLayerEntityItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri, const QString &provider, Qgis::BrowserLayerType type )
  : QgsLayerItem( parent, name, path, encodedUri, type, provider )
{
  setState( Qgis::BrowserItemState::Populated );
}

//
// QgsSensorThingsDataItemProvider
//

QString QgsSensorThingsDataItemProvider::name()
{
  return QStringLiteral( "SensorThings" );
}

QString QgsSensorThingsDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "sensorthings" );
}

Qgis::DataItemProviderCapabilities QgsSensorThingsDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsSensorThingsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsSensorThingsRootItem( parentItem, QObject::tr( "SensorThings" ), QStringLiteral( "sensorthings:" ) );

  return nullptr;
}

///@endcond

