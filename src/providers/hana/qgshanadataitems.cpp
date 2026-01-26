/***************************************************************************
   qgshanadataitems.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanadataitems.h"

#include <climits>

#include "qgsdatasourceuri.h"
#include "qgsfieldsitem.h"
#include "qgshanaconnection.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmimedatautils.h"

#include <QMessageBox>

#include "moc_qgshanadataitems.cpp"

QgsHanaConnectionItem::QgsHanaConnectionItem(
  QgsDataItem *parent,
  const QString &name,
  const QString &path
)
  : QgsDataCollectionItem( parent, name, path, u"SAP HANA"_s )
{
  mIconName = u"mIconConnect.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;

  updateToolTip( QString(), QString() );
}

QVector<QgsDataItem *> QgsHanaConnectionItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsHanaConnectionRef conn( mName );
  if ( conn.isNull() )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    return items;
  }

  QgsHanaSettings settings( mName, true );

  try
  {
    QString userName = conn->getUserName();
    if ( userName.isEmpty() )
      userName = settings.userName();

    updateToolTip( userName, conn->getDatabaseVersion() );

    const QVector<QgsHanaSchemaProperty> schemas = conn->getSchemas( settings.userTablesOnly() ? userName : QString() );

    if ( schemas.isEmpty() )
    {
      items.append( new QgsErrorItem( this, tr( "No schemas found" ), mPath + "/error" ) );
    }
    else
    {
      for ( const QgsHanaSchemaProperty &schema : schemas )
      {
        QgsHanaSchemaItem *schemaItem = new QgsHanaSchemaItem( this, mName, schema.name, mPath + '/' + schema.name );
        items.append( schemaItem );
      }
    }
  }
  catch ( const QgsHanaException &ex )
  {
    QgsErrorItem *itemError = new QgsErrorItem( this, tr( "Server error occurred" ), mPath + "/error" );
    itemError->setToolTip( ex.what() );
    items.append( itemError );
  }

  return items;
}

bool QgsHanaConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
    return false;

  const QgsHanaConnectionItem *o = qobject_cast<const QgsHanaConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

QgsDataSourceUri QgsHanaConnectionItem::connectionUri() const
{
  return QgsHanaSettings( mName, true ).toDataSourceUri();
}

void QgsHanaConnectionItem::refreshSchema( const QString &schema )
{
  for ( QgsDataItem *child : std::as_const( mChildren ) )
  {
    if ( child->name() == schema || schema.isEmpty() )
      child->refresh();
  }
}

void QgsHanaConnectionItem::updateToolTip( const QString &userName, const QString &dbmsVersion )
{
  QgsHanaSettings settings( mName, true );
  QString tip;
  if ( settings.connectionType() == QgsHanaConnectionType::HostPort )
  {
    if ( !settings.database().isEmpty() )
      tip = tr( "Database: " ) + settings.database();
    if ( !tip.isEmpty() )
      tip += '\n';
    tip += tr( "Host: " ) + settings.host() + u" "_s;
    if ( QgsHanaIdentifierType::fromInt( settings.identifierType() ) == QgsHanaIdentifierType::InstanceNumber )
      tip += settings.identifier();
    else
      tip += settings.port();
    tip += '\n';
  }
  if ( !dbmsVersion.isEmpty() )
    tip += tr( "DB Version: " ) + dbmsVersion + '\n';
  tip += tr( "User: " ) + userName + '\n';
  tip += tr( "Encrypted: " ) + QString( settings.enableSsl() ? tr( "yes" ) : tr( "no" ) );
  setToolTip( tip );
}

// ---------------------------------------------------------------------------
QgsHanaLayerItem::QgsHanaLayerItem(
  QgsDataItem *parent,
  const QString &name,
  const QString &path,
  Qgis::BrowserLayerType layerType,
  const QgsHanaLayerProperty &layerProperty
)
  : QgsLayerItem( parent, name, path, QString(), layerType, u"hana"_s )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;
  mUri = createUri();
  setState( Qgis::BrowserItemState::NotPopulated );
}

QVector<QgsDataItem *> QgsHanaLayerItem::createChildren()
{
  QVector<QgsDataItem *> items;
  items.push_back( new QgsFieldsItem( this, uri() + u"/columns/ "_s, createUri(), providerKey(), mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return items;
}

QString QgsHanaLayerItem::createUri() const
{
  QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugError( "Connection item not found." );
    return QString();
  }

  QgsHanaSettings settings( connItem->name(), true );

  QStringList pkColumns;
  if ( !mLayerProperty.pkCols.isEmpty() )
  {
    const QStringList pkColumnsStored = settings.keyColumns( mLayerProperty.schemaName, mLayerProperty.tableName );
    if ( !pkColumnsStored.empty() )
    {
      // We check whether the primary key columns still exist.
      auto intersection = qgis::listToSet( pkColumnsStored ).intersect( qgis::listToSet( mLayerProperty.pkCols ) );
      if ( intersection.size() == pkColumnsStored.size() )
      {
        for ( const auto &column : pkColumnsStored )
          pkColumns << QgsHanaUtils::quotedIdentifier( column );
      }
    }
  }

  QgsDataSourceUri uri = settings.toDataSourceUri();
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColumns.join( ',' ) );
  uri.setWkbType( mLayerProperty.type );
  if ( uri.wkbType() != Qgis::WkbType::NoGeometry )
    uri.setSrid( QString::number( mLayerProperty.srid ) );
  QgsDebugMsgLevel( u"layer uri: %1"_s.arg( uri.uri( false ) ), 4 );
  return uri.uri( false );
}

QString QgsHanaLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

// ---------------------------------------------------------------------------
QgsHanaSchemaItem::QgsHanaSchemaItem(
  QgsDataItem *parent,
  const QString &connectionName,
  const QString &name,
  const QString &path
)
  : QgsDatabaseSchemaItem( parent, name, path, u"SAP HANA"_s )
  , mConnectionName( connectionName )
{
  mIconName = u"mIconDbSchema.svg"_s;
  mSchemaName = name;
}

QVector<QgsDataItem *> QgsHanaSchemaItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsHanaConnectionRef conn( mConnectionName );
  if ( conn.isNull() )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    return items;
  }

  try
  {
    QgsHanaSettings settings( mConnectionName, true );
    const QVector<QgsHanaLayerProperty> layers = conn->getLayersFull( mSchemaName, settings.allowGeometrylessTables(), settings.userTablesOnly() );

    items.reserve( layers.size() );
    for ( const QgsHanaLayerProperty &layerInfo : layers )
    {
      if ( layerInfo.isValid )
        items.append( createLayer( layerInfo ) );
      else
      {
        QgsErrorItem *itemInvalidLayer = new QgsErrorItem( this, layerInfo.defaultName(), mPath + "/error" );
        itemInvalidLayer->setToolTip( layerInfo.errorMessage );
        items.append( itemInvalidLayer );
      }
    }
  }
  catch ( const QgsHanaException &ex )
  {
    QgsErrorItem *itemError = new QgsErrorItem( this, tr( "Server error occurred" ), mPath + "/error" );
    itemError->setToolTip( ex.what() );
    items.append( itemError );
  }

  setName( mSchemaName );

  return items;
}

QgsHanaLayerItem *QgsHanaSchemaItem::createLayer( const QgsHanaLayerProperty &layerProperty )
{
  QString tip = layerProperty.isView ? u"View"_s : u"Table"_s;

  Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::TableLayer;
  if ( !layerProperty.geometryColName.isEmpty() && layerProperty.isGeometryValid() )
  {
    tip += tr( "\n%1 as %2" ).arg( layerProperty.geometryColName, QgsWkbTypes::displayString( layerProperty.type ) );

    if ( layerProperty.srid >= 0 )
      tip += tr( " (srid %1)" ).arg( layerProperty.srid );
    else
      tip += tr( " (unknown srid)" );

    if ( !layerProperty.tableComment.isEmpty() )
      tip = layerProperty.tableComment + '\n' + tip;

    Qgis::GeometryType geomType = QgsWkbTypes::geometryType( layerProperty.type );
    switch ( geomType )
    {
      case Qgis::GeometryType::Point:
        layerType = Qgis::BrowserLayerType::Point;
        break;
      case Qgis::GeometryType::Line:
        layerType = Qgis::BrowserLayerType::Line;
        break;
      case Qgis::GeometryType::Polygon:
        layerType = Qgis::BrowserLayerType::Polygon;
        break;
      default:
        break;
    }
  }
  else
  {
    tip = tr( "as geometryless table" );
  }

  QgsHanaLayerItem *layerItem = new QgsHanaLayerItem( this, layerProperty.defaultName(), mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

QgsHanaRootItem::QgsHanaRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, u"SAP HANA"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconHana.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsHanaRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList connectionNames = QgsHanaSettings::getConnectionNames();
  connections.reserve( connectionNames.size() );
  for ( const QString &connName : connectionNames )
  {
    connections << new QgsHanaConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

void QgsHanaRootItem::onConnectionsChanged()
{
  refresh();
}

QgsDataItem *QgsHanaDataItemProvider::createDataItem(
  const QString &pathIn, QgsDataItem *parentItem
)
{
  Q_UNUSED( pathIn )
  return new QgsHanaRootItem( parentItem, u"SAP HANA"_s, u"hana:"_s );
}
