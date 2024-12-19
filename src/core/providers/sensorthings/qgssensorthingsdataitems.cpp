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
          Qgis::SensorThingsEntity::MultiDatastream,
        } )
  {
    QVariantMap entityUriParts = connectionUriParts;
    entityUriParts.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( entity ) );

    if ( QgsSensorThingsUtils::entityTypeHasGeometry( entity ) )
    {
      children.append( new QgsSensorThingsEntityContainerItem( this,
                       QgsSensorThingsUtils::displayString( entity, true ),
                       mPath + '/' + qgsEnumValueToKey( entity ),
                       entityUriParts, entity, mConnName ) );
    }
    else
    {
      children.append( new QgsSensorThingsLayerEntityItem( this,
                       QgsSensorThingsUtils::displayString( entity, true ),
                       mPath + '/' + qgsEnumValueToKey( entity ),
                       entityUriParts,
                       QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                       Qgis::BrowserLayerType::TableLayer, entity, mConnName ) );
    }
  }

  return children;
}


//
// QgsSensorThingsEntityContainerItem
//

QgsSensorThingsEntityContainerItem::QgsSensorThingsEntityContainerItem( QgsDataItem *parent, const QString &name, const QString &path, const QVariantMap &entityUriParts, Qgis::SensorThingsEntity entityType, const QString &connectionName )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "sensorthings" ) )
  , mEntityUriParts( entityUriParts )
  , mEntityType( entityType )
  , mConnectionName( connectionName )
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

  int sortKey = 1;
  QList< Qgis::WkbType > compatibleTypes;
  // we always expose "no geometry" types for these, even though they have a restricted fixed type
  // according to the spec. This is because not all services respect the mandated geometry types!
  switch ( QgsSensorThingsUtils::geometryTypeForEntity( mEntityType ) )
  {
    case Qgis::GeometryType::Point:
      compatibleTypes << Qgis::WkbType::Point << Qgis::WkbType::MultiPoint << Qgis::WkbType::NoGeometry;
      break;
    case Qgis::GeometryType::Line:
      compatibleTypes << Qgis::WkbType::MultiLineString << Qgis::WkbType::NoGeometry;
      break;
    case Qgis::GeometryType::Polygon:
      compatibleTypes << Qgis::WkbType::MultiPolygon << Qgis::WkbType::NoGeometry;
      break;
    case Qgis::GeometryType::Unknown:
      compatibleTypes << Qgis::WkbType::Point << Qgis::WkbType::MultiPoint << Qgis::WkbType::MultiLineString << Qgis::WkbType::MultiPolygon;
      break;
    case Qgis::GeometryType::Null:
      compatibleTypes << Qgis::WkbType::NoGeometry;;
  }

  for ( const Qgis::WkbType wkbType : std::as_const( compatibleTypes ) )
  {
    QVariantMap geometryUriParts = mEntityUriParts;
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
      case Qgis::WkbType::NoGeometry:
        geometryUriParts.remove( QStringLiteral( "geometryType" ) );
        name = tr( "No Geometry" );
        layerType = Qgis::BrowserLayerType::TableLayer;
        break;
      default:
        break;
    }
    children.append( new QgsSensorThingsLayerEntityItem( this,
                     name,
                     mPath + '/' + name,
                     geometryUriParts,
                     QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                     layerType, mEntityType, mConnectionName ) );
    children.last()->setSortKey( sortKey++ );
  }

  return children;
}

//
// QgsSensorThingsLayerEntityItem
//

QgsSensorThingsLayerEntityItem::QgsSensorThingsLayerEntityItem( QgsDataItem *parent, const QString &name, const QString &path,
    const QVariantMap &uriParts, const QString &provider, Qgis::BrowserLayerType type, Qgis::SensorThingsEntity entityType, const QString &connectionName )
  : QgsLayerItem( parent, name, path,
                  QgsProviderRegistry::instance()->encodeUri( QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, uriParts ),
                  type, provider )
  , mUriParts( uriParts )
  , mEntityType( entityType )
  , mConnectionName( connectionName )
{
  setState( Qgis::BrowserItemState::Populated );
}

QString QgsSensorThingsLayerEntityItem::layerName() const
{
  QString baseName;
  if ( QgsSensorThingsUtils::entityTypeHasGeometry( mEntityType ) )
  {
    const QString geometryType = mUriParts.value( QStringLiteral( "geometryType" ) ).toString();
    QString geometryNamePart;
    if ( geometryType.compare( QLatin1String( "point" ), Qt::CaseInsensitive ) == 0 ||
         geometryType.compare( QLatin1String( "multipoint" ), Qt::CaseInsensitive ) == 0 )
    {
      geometryNamePart = tr( "Points" );
    }
    else if ( geometryType.compare( QLatin1String( "line" ), Qt::CaseInsensitive ) == 0 )
    {
      geometryNamePart = tr( "Lines" );
    }
    else if ( geometryType.compare( QLatin1String( "polygon" ), Qt::CaseInsensitive ) == 0 )
    {
      geometryNamePart = tr( "Polygons" );
    }

    if ( !geometryNamePart.isEmpty() )
    {
      baseName = QStringLiteral( "%1 - %2 (%3)" ).arg( mConnectionName,
                 QgsSensorThingsUtils::displayString( mEntityType, true ),
                 geometryNamePart );
    }
    else
    {
      baseName = QStringLiteral( "%1 - %2" ).arg( mConnectionName,
                 QgsSensorThingsUtils::displayString( mEntityType, true ) );
    }
  }
  else
  {
    baseName = QStringLiteral( "%1 - %2" ).arg( mConnectionName,
               QgsSensorThingsUtils::displayString( mEntityType, true ) );
  }

  return baseName;
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

