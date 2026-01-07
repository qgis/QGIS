/***************************************************************************
    qgspostgresdataitems.cpp
  --------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgresdataitems.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgsfieldsitem.h"
#include "qgslogger.h"
#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprojectstorage.h"
#include "qgspostgresutils.h"
#include "qgsproject.h"
#include "qgsprojectitem.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"

#include "moc_qgspostgresdataitems.cpp"

// ---------------------------------------------------------------------------
QgsPGConnectionItem::QgsPGConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, u"PostGIS"_s )
{
  mIconName = u"mIconConnect.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsPGConnectionItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( mName );
  // TODO: we need to cancel somehow acquireConnection() if deleteLater() was called on this item to avoid later credential dialog if connection failed
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsPostgresConn::connectionInfo( uri, false ) );
  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugError( "Connection failed - " + QgsPostgresConn::connectionInfo( uri, false ) );
    return items;
  }

  QList<QgsPostgresSchemaProperty> schemas;
  const QString restrictToSchema = QgsPostgresConn::publicSchemaOnly( mName ) ? u"public"_s : QgsPostgresConn::schemaToRestrict( mName );
  const bool ok = conn->getSchemas( schemas, !restrictToSchema.isEmpty() ? QStringList { restrictToSchema } : QStringList {} );

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get schemas" ), mPath + "/error" ) );
    return items;
  }

  for ( const QgsPostgresSchemaProperty &schema : std::as_const( schemas ) )
  {
    QgsPGSchemaItem *schemaItem = new QgsPGSchemaItem( this, mName, schema.name, mPath + '/' + schema.name );
    if ( !schema.description.isEmpty() )
    {
      schemaItem->setToolTip( schema.description );
    }
    items.append( schemaItem );
  }

  return items;
}

bool QgsPGConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsPGConnectionItem *o = qobject_cast<const QgsPGConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

QgsDataSourceUri QgsPGConnectionItem::connectionUri() const
{
  return QgsPostgresConn::connUri( mName );
}

void QgsPGConnectionItem::refreshSchema( const QString &schema )
{
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    if ( child->name() == schema || schema.isEmpty() )
    {
      child->refresh();
    }
  }
}

// ---------------------------------------------------------------------------
QgsPGLayerItem::QgsPGLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsPostgresLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, layerProperty.isRaster ? u"postgresraster"_s : u"postgres"_s )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;
  mUri = createUri();
  // No rasters for now
  setState( layerProperty.isRaster ? Qgis::BrowserItemState::Populated : Qgis::BrowserItemState::NotPopulated );
  Q_ASSERT( mLayerProperty.size() == 1 );
}

QString QgsPGLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

QString QgsPGLayerItem::createUri()
{
  QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugError( u"connection item not found."_s );
    return QString();
  }

  const QString &connName = connItem->name();

  QgsDataSourceUri uri( QgsPostgresConn::connectionInfo( QgsPostgresConn::connUri( connName ), false ) );

  const QgsSettings &settings = QgsSettings();
  QString basekey = u"/PostgreSQL/connections/%1"_s.arg( connName );

  const QStringList defPk( settings.value(
                                     u"%1/keys/%2/%3"_s.arg( basekey, mLayerProperty.schemaName, mLayerProperty.tableName ),
                                     QVariant( !mLayerProperty.pkCols.isEmpty() ? QStringList( mLayerProperty.pkCols.at( 0 ) ) : QStringList() )
  )
                             .toStringList() );

  const bool useEstimatedMetadata = QgsPostgresConn::useEstimatedMetadata( connName );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );

  QStringList cols;
  for ( const QString &col : defPk )
  {
    cols << QgsPostgresConn::quotedIdentifier( col );
  }

  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, cols.join( ',' ) );
  uri.setWkbType( mLayerProperty.types.at( 0 ) );
  if ( uri.wkbType() != Qgis::WkbType::NoGeometry && mLayerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    uri.setSrid( QString::number( mLayerProperty.srids.at( 0 ) ) );

  QgsDebugMsgLevel( u"layer uri: %1"_s.arg( uri.uri( false ) ), 2 );
  return uri.uri( false );
}

// ---------------------------------------------------------------------------
QgsPGSchemaItem::QgsPGSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path )
  : QgsDatabaseSchemaItem( parent, name, path, u"PostGIS"_s )
  , mConnectionName( connectionName )
{
  mIconName = u"mIconDbSchema.svg"_s;
}

QVector<QgsDataItem *> QgsPGSchemaItem::createChildren()
{
  QVector<QgsDataItem *> items;

  QgsDataSourceUri uri = QgsPostgresConn::connUri( mConnectionName );
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsPostgresConn::connectionInfo( uri, false ) );

  if ( !conn )
  {
    items.append( new QgsErrorItem( this, tr( "Connection failed" ), mPath + "/error" ) );
    QgsDebugError( "Connection failed - " + QgsPostgresConn::connectionInfo( uri, false ) );
    return items;
  }

  QVector<QgsPostgresLayerProperty> layerProperties;
  const bool ok = conn->supportedLayers( layerProperties, QgsPostgresConn::geometryColumnsOnly( mConnectionName ), QgsPostgresConn::allowGeometrylessTables( mConnectionName ), QgsPostgresConn::allowRasterOverviewTables( mConnectionName ), mName );

  if ( !ok )
  {
    items.append( new QgsErrorItem( this, tr( "Failed to get layers" ), mPath + "/error" ) );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    return items;
  }

  const bool dontResolveType = QgsPostgresConn::dontResolveType( mConnectionName );
  const bool estimatedMetadata = QgsPostgresConn::useEstimatedMetadata( mConnectionName );
  const bool allowMetadataInDatabase = QgsPostgresConn::allowMetadataInDatabase( mConnectionName );
  // Retrieve metadata for layer items
  QMap<QString, QgsLayerMetadata> layerMetadata;
  if ( allowMetadataInDatabase )
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( "postgres" ) };
    if ( md )
    {
      try
      {
        QgsAbstractDatabaseProviderConnection *dbConn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnectionName ) ) };
        if ( dbConn )
        {
          const QList<QgsLayerMetadataProviderResult> results { dbConn->searchLayerMetadata( QgsMetadataSearchContext() ) };
          for ( const QgsLayerMetadataProviderResult &result : std::as_const( results ) )
          {
            const QgsDataSourceUri resUri { result.uri() };
            layerMetadata.insert( u"%1.%2"_s.arg( resUri.schema(), resUri.table() ), static_cast<QgsLayerMetadata>( result ) );
          }
        }
      }
      catch ( const QgsProviderConnectionException & )
      {
        // ignore
      }
    }
  }

  const auto constLayerProperties = layerProperties;
  for ( QgsPostgresLayerProperty layerProperty : constLayerProperties )
  {
    if ( layerProperty.schemaName != mName )
      continue;

    if ( !layerProperty.geometryColName.isNull() && !layerProperty.isRaster && ( layerProperty.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown || layerProperty.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) )
    {
      if ( dontResolveType )
      {
        QgsDebugMsgLevel( u"skipping column %1.%2 without type constraint"_s.arg( layerProperty.schemaName, layerProperty.tableName ), 2 );
        continue;
      }
      // If the table is empty there is no way we can retrieve layer types, let's make a copy and restore it
      QgsPostgresLayerProperty propertyCopy { layerProperty };
      conn->retrieveLayerTypes( layerProperty, estimatedMetadata );
      if ( layerProperty.size() == 0 )
      {
        layerProperty = propertyCopy;
      }
    }

    for ( int i = 0; i < layerProperty.size(); i++ )
    {
      QgsDataItem *layerItem = nullptr;
      layerItem = createLayer( layerProperty.at( i ) );
      if ( layerItem )
      {
        items.append( layerItem );
        // Attach metadata
        const QString mdKey { u"%1.%2"_s.arg( layerProperty.at( i ).schemaName, layerProperty.at( i ).tableName ) };
        if ( allowMetadataInDatabase && layerMetadata.contains( mdKey ) )
        {
          if ( QgsLayerItem *lItem = static_cast<QgsLayerItem *>( layerItem ) )
          {
            lItem->setLayerMetadata( layerMetadata.value( mdKey ) );
          }
        }
      }
    }
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "postgresql" );
  if ( QgsPostgresConn::allowProjectsInDatabase( mConnectionName ) && storage )
  {
    QgsPostgresProjectUri postUri;
    postUri.connInfo = uri;
    postUri.schemaName = mName;
    QString schemaUri = QgsPostgresProjectStorage::encodeUri( postUri );
    const QStringList projectNames = storage->listProjects( schemaUri );
    for ( const QString &projectName : projectNames )
    {
      QgsPostgresProjectUri projectUri( postUri );
      projectUri.projectName = projectName;
      items.append( new QgsPGProjectItem( this, projectName, projectUri, mConnectionName ) );
    }
  }

  return items;
}


QgsPGLayerItem *QgsPGSchemaItem::createLayer( QgsPostgresLayerProperty layerProperty )
{
  //QgsDebugMsgLevel( "schemaName = " + layerProperty.schemaName + " tableName = " + layerProperty.tableName + " geometryColName = " + layerProperty.geometryColName, 2 );
  QString tip;
  if ( layerProperty.relKind == Qgis::PostgresRelKind::View )
  {
    tip = tr( "View" );
  }
  else if ( layerProperty.relKind == Qgis::PostgresRelKind::MaterializedView )
  {
    tip = tr( "Materialized view" );
  }
  else if ( layerProperty.isRaster )
  {
    tip = tr( "Raster" );
  }
  else if ( layerProperty.relKind == Qgis::PostgresRelKind::ForeignTable )
  {
    tip = tr( "Foreign table" );
  }
  else
  {
    tip = tr( "Table" );
  }

  Qgis::WkbType wkbType = layerProperty.types.at( 0 );
  if ( !layerProperty.isRaster )
  {
    tip += tr( "\n%1 as %2" ).arg( layerProperty.geometryColName, QgsPostgresConn::displayStringForWkbType( wkbType ) );
  }

  if ( layerProperty.srids.at( 0 ) != std::numeric_limits<int>::min() )
    tip += tr( " (srid %1)" ).arg( layerProperty.srids.at( 0 ) );
  else
    tip += tr( " (unknown srid)" );

  if ( !layerProperty.tableComment.isEmpty() )
  {
    tip = layerProperty.tableComment + '\n' + tip;
  }


  Qgis::BrowserLayerType layerType = Qgis::BrowserLayerType::Raster;
  if ( !layerProperty.isRaster )
  {
    Qgis::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
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
        if ( !layerProperty.geometryColName.isEmpty() )
        {
          QgsDebugMsgLevel( u"Adding layer item %1.%2 without type constraint as geometryless table"_s.arg( layerProperty.schemaName, layerProperty.tableName ), 2 );
        }
        layerType = Qgis::BrowserLayerType::TableLayer;
        tip = tr( "as geometryless table" );
    }
  }

  QgsPGLayerItem *layerItem = new QgsPGLayerItem( this, layerProperty.defaultName(), mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  return layerItem;
}

QVector<QgsDataItem *> QgsPGLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, uri() + u"/columns/ "_s, createUri(), providerKey(), mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return children;
}

// ---------------------------------------------------------------------------
QgsPGRootItem::QgsPGRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, u"PostGIS"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconPostgis.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsPGRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsPostgresConn::connectionList();
  for ( const QString &connName : list )
  {
    connections << new QgsPGConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

void QgsPGRootItem::onConnectionsChanged()
{
  refresh();
}

QString QgsPostgresDataItemProvider::name()
{
  return u"PostGIS"_s;
}

QString QgsPostgresDataItemProvider::dataProviderKey() const
{
  return u"postgres"_s;
}

Qgis::DataItemProviderCapabilities QgsPostgresDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::Databases;
}

QgsDataItem *QgsPostgresDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsPGRootItem( parentItem, QObject::tr( "PostgreSQL" ), u"pg:"_s );
}


bool QgsPGSchemaItem::layerCollection() const
{
  return true;
}

QgsPGProjectItem::QgsPGProjectItem( QgsDataItem *parent, const QString name, const QgsPostgresProjectUri &postgresProjectUri, const QString &connectionName )
  : QgsProjectItem( parent, name, QgsPostgresProjectStorage::encodeUri( postgresProjectUri ), u"postgres"_s ), mProjectUri( postgresProjectUri ), mConnectionName( connectionName )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete | Qgis::BrowserItemCapability::Fertile;

  refreshTooltip();
}

QString QgsPGProjectItem::uriWithNewName( const QString &newProjectName )
{
  QgsPostgresProjectUri postgresProjectUri( mProjectUri );
  postgresProjectUri.projectName = newProjectName;
  return QgsPostgresProjectStorage::encodeUri( postgresProjectUri );
}

void QgsPGProjectItem::refresh()
{
  QgsProjectItem::refresh();
  refreshTooltip();
}

void QgsPGProjectItem::refreshTooltip()
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( "postgres" ) };
  if ( md )
  {
    QgsPostgresConn *conn = QgsPostgresConn::connectDb( mProjectUri.connInfo, false );
    if ( conn )
    {
      QString commentColumn;
      if ( QgsPostgresUtils::columnExists( conn, mProjectUri.schemaName, u"qgis_projects"_s, u"comment"_s ) )
      {
        commentColumn = u", comment "_s;
      }

      const QString sql = QStringLiteral( "SELECT (metadata->>'last_modified_time')::timestamp(0)::TEXT AS time, "
                                          "metadata->>'last_modified_user' AS user %1"
                                          "FROM %2.qgis_projects WHERE name = %3" )
                            .arg( commentColumn, QgsPostgresConn::quotedIdentifier( mProjectUri.schemaName ), QgsPostgresConn::quotedValue( mName ) );

      QgsPostgresResult res( conn->PQexec( sql ) );

      if ( res.PQntuples() == 1 )
      {
        const QString tooltip = QStringLiteral( "%1: %2\n"
                                                "%3: %4\n"
                                                "%5: %6" )
                                  .arg( tr( "Last modified time" ), res.PQgetvalue( 0, 0 ), tr( "Last modified user" ), res.PQgetvalue( 0, 1 ), tr( "Comment" ), commentColumn.isEmpty() ? QString() : res.PQgetvalue( 0, 2 ) );
        setToolTip( tooltip );
      }
      conn->unref();
    }
  }
}
