/***************************************************************************
                         qgsmssqldataitems.cpp  -  description
                         -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqldataitems.h"

#include "qgsdatasourceuri.h"
#include "qgsfieldsitem.h"
#include "qgslogger.h"
#include "qgsmimedatautils.h"
#include "qgsmssqlconnection.h"
#include "qgsmssqldatabase.h"
#include "qgsmssqlgeomcolumntypethread.h"
#include "qgsmssqlutils.h"
#include "qgsproject.h"
#include "qgssettings.h"

#include "moc_qgsmssqldataitems.cpp"

#ifdef HAVE_GUI
#include "qgsmssqlsourceselect.h"
#endif

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>

// ---------------------------------------------------------------------------
QgsMssqlConnectionItem::QgsMssqlConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path, u"MSSQL"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast | Qgis::BrowserItemCapability::Collapse;
  mIconName = u"mIconConnect.svg"_s;
}

QgsMssqlConnectionItem::~QgsMssqlConnectionItem()
{
  stop();
}

void QgsMssqlConnectionItem::readConnectionSettings()
{
  QgsSettings settings;
  QString key = "/MSSQL/connections/" + mName;
  mService = settings.value( key + "/service" ).toString();
  mHost = settings.value( key + "/host" ).toString();
  mDatabase = settings.value( key + "/database" ).toString();
  if ( settings.value( key + "/saveUsername" ).toString() == "true"_L1 )
  {
    mUsername = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == "true"_L1 )
  {
    mPassword = settings.value( key + "/password" ).toString();
  }

  mSchemaSettings.clear();
  mSchemasFilteringEnabled = settings.value( key + "/schemasFiltering" ).toBool();

  if ( mSchemasFilteringEnabled )
  {
    QVariant schemasSettingsVariant = settings.value( key + "/excludedSchemas" );
    if ( schemasSettingsVariant.isValid() && schemasSettingsVariant.userType() == QMetaType::Type::QVariantMap )
      mSchemaSettings = schemasSettingsVariant.toMap();
  }

  mUseGeometryColumns = QgsMssqlConnection::geometryColumnsOnly( mName );
  mUseEstimatedMetadata = QgsMssqlConnection::useEstimatedMetadata( mName );
  mAllowGeometrylessTables = QgsMssqlConnection::allowGeometrylessTables( mName );

  mConnectionUri = "dbname='" + mDatabase + "' host='" + mHost + "' user='" + mUsername + "' password='" + mPassword + '\'';
  if ( !mService.isEmpty() )
    mConnectionUri += " service='" + mService + '\'';
  if ( mUseEstimatedMetadata )
    mConnectionUri += " estimatedmetadata=true"_L1;
}

void QgsMssqlConnectionItem::stop()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    delete mColumnTypeThread;
    mColumnTypeThread = nullptr;
  }
}

void QgsMssqlConnectionItem::refresh()
{
  QgsDebugMsgLevel( "mPath = " + mPath, 3 );
  stop();

  QgsDataCollectionItem::refresh();
}

QVector<QgsDataItem *> QgsMssqlConnectionItem::createChildren()
{
  stop();

  QVector<QgsDataItem *> children;
  if ( deferredDelete() )
    return children;

  readConnectionSettings();

  QgsDataSourceUri uri;
  uri.setService( mService );
  uri.setHost( mHost );
  uri.setDatabase( mDatabase );
  uri.setUsername( mUsername );
  uri.setPassword( mPassword );
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( uri );

  if ( !db->isValid() )
  {
    children.append( new QgsErrorItem( this, db->errorText(), mPath + "/error" ) );
    setAsPopulated();
    return children;
  }

  // build sql statement
  const QString query = QgsMssqlConnection::buildQueryForTables( mName );

  const bool disableInvalidGeometryHandling = QgsMssqlConnection::isInvalidGeometryHandlingDisabled( mName );

  // issue the sql query
  QSqlQuery q = QSqlQuery( db->db() );
  q.setForwardOnly( true );
  ( void ) q.exec( query );

  QSet<QString> addedSchemas;

  if ( q.isActive() )
  {
    QVector<QgsDataItem *> newLayers;
    while ( q.next() )
    {
      QgsMssqlLayerProperty layer;
      layer.schemaName = q.value( 0 ).toString();
      layer.tableName = q.value( 1 ).toString();
      layer.geometryColName = q.value( 2 ).toString();
      layer.srid = q.value( 3 ).toString();
      layer.type = q.value( 4 ).toString();
      layer.isView = q.value( 5 ).toBool();
      const int dimensions { q.value( 6 ).toInt() };
      if ( dimensions >= 3 )
      {
        layer.type = layer.type.append( 'Z' );
      }
      if ( dimensions == 4 )
      {
        layer.type = layer.type.append( 'M' );
      }
      layer.pkCols = QStringList(); //TODO
      layer.isGeography = false;

      QString type = layer.type;
      QString srid = layer.srid;

      QgsMssqlSchemaItem *schemaItem = nullptr;
      const auto constChildren = children;
      for ( QgsDataItem *child : constChildren )
      {
        if ( child->name() == layer.schemaName )
        {
          schemaItem = static_cast<QgsMssqlSchemaItem *>( child );
          break;
        }
      }

      if ( !schemaItem )
      {
        schemaItem = new QgsMssqlSchemaItem( this, layer.schemaName, mPath + '/' + layer.schemaName );
        schemaItem->setState( Qgis::BrowserItemState::Populating );
        addedSchemas.insert( layer.schemaName );
        children.append( schemaItem );
      }

      if ( !layer.geometryColName.isNull() )
      {
        if ( type == "GEOMETRY"_L1 || type.isNull() || srid.isEmpty() )
        {
          if ( !mColumnTypeThread )
          {
            mColumnTypeThread = new QgsMssqlGeomColumnTypeThread( mService, mHost, mDatabase, mUsername, mPassword, true /* use estimated metadata */, disableInvalidGeometryHandling );

            connect( mColumnTypeThread, &QgsMssqlGeomColumnTypeThread::setLayerType, this, &QgsMssqlConnectionItem::setLayerType );
            connect( this, &QgsMssqlConnectionItem::addGeometryColumn, mColumnTypeThread, &QgsMssqlGeomColumnTypeThread::addGeometryColumn );
          }

          emit addGeometryColumn( layer );
          continue;
        }
      }

      QgsMssqlLayerItem *added = schemaItem->addLayer( layer, false );
      if ( added )
        newLayers.append( added );
    }

    // add missing schemas (i.e., empty schemas)
    const QStringList allSchemas = QgsMssqlConnection::schemas( db, nullptr );
    QStringList excludedSchema = QgsMssqlConnection::excludedSchemasList( mName );
    for ( const QString &schema : allSchemas )
    {
      if ( mSchemasFilteringEnabled && excludedSchema.contains( schema ) )
        continue; // user does not want it to be shown

      if ( addedSchemas.contains( schema ) )
        continue;

      if ( QgsMssqlConnection::isSystemSchema( schema ) )
        continue;

      QgsMssqlSchemaItem *schemaItem = new QgsMssqlSchemaItem( this, schema, mPath + '/' + schema );
      schemaItem->setState( Qgis::BrowserItemState::Populated ); // no tables
      addedSchemas.insert( schema );
      children.append( schemaItem );
    }

    // spawn threads (new layers will be added later on)
    if ( mColumnTypeThread )
    {
      connect( mColumnTypeThread, &QThread::finished, this, &QgsMssqlConnectionItem::setAsPopulated );
      mColumnTypeThread->start();
    }
    else
    {
      //set all as populated -- we also need to do this for newly created items, because they won't yet be children of this item
      for ( QgsDataItem *child : std::as_const( children ) )
      {
        child->setState( Qgis::BrowserItemState::Populated );
      }
      setAsPopulated();
    }
  }
  else
  {
    setAsPopulated();
  }

  return children;
}

void QgsMssqlConnectionItem::setAsPopulated()
{
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    child->setState( Qgis::BrowserItemState::Populated );
  }
  setState( Qgis::BrowserItemState::Populated );
}

void QgsMssqlConnectionItem::setAllowGeometrylessTables( const bool allow )
{
  mAllowGeometrylessTables = allow;
  QgsMssqlConnection::setAllowGeometrylessTables( mName, allow );
  refresh();
}

void QgsMssqlConnectionItem::setLayerType( QgsMssqlLayerProperty layerProperty )
{
  QgsMssqlSchemaItem *schemaItem = nullptr;

  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    if ( child->name() == layerProperty.schemaName )
    {
      schemaItem = static_cast<QgsMssqlSchemaItem *>( child );
      break;
    }
  }

  if ( !schemaItem )
  {
    QgsDebugError( u"schema item for %1 not found."_s.arg( layerProperty.schemaName ) );
    return;
  }

  const auto constChildren = schemaItem->children();
  for ( QgsDataItem *layerItem : constChildren )
  {
    if ( layerItem->name() == layerProperty.tableName )
      return; // already added
  }

  QStringList typeList = layerProperty.type.split( ',', Qt::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ',', Qt::SkipEmptyParts );
  Q_ASSERT( typeList.size() == sridList.size() );

  for ( int i = 0; i < typeList.size(); i++ )
  {
    Qgis::WkbType wkbType = QgsMssqlUtils::wkbTypeFromGeometryType( typeList[i] );
    if ( wkbType == Qgis::WkbType::Unknown )
    {
      QgsDebugError( u"unsupported geometry type:%1"_s.arg( typeList[i] ) );
      continue;
    }

    layerProperty.type = typeList[i];
    layerProperty.srid = sridList[i];
    schemaItem->addLayer( layerProperty, true );
  }

  if ( typeList.isEmpty() )
  {
    // this suggests that retrieval of geometry type and CRS failed if no results were returned
    // for examle due to invalid geometries in the table (WHAAAT?)
    // but we still want to add have such table in the list
    schemaItem->addLayer( layerProperty, true );
  }
}

bool QgsMssqlConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsMssqlConnectionItem *o = qobject_cast<const QgsMssqlConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

// ---------------------------------------------------------------------------
QgsMssqlLayerItem::QgsMssqlLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsMssqlLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, u"mssql"_s )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Qgis::BrowserItemCapability::Delete;
  mUri = createUri();
  setState( Qgis::BrowserItemState::NotPopulated );
}


QgsMssqlLayerItem *QgsMssqlLayerItem::createClone()
{
  return new QgsMssqlLayerItem( mParent, mName, mPath, mLayerType, mLayerProperty );
}

bool QgsMssqlLayerItem::disableInvalidGeometryHandling() const
{
  return mDisableInvalidGeometryHandling;
}

QString QgsMssqlLayerItem::createUri()
{
  QString pkColName = !mLayerProperty.pkCols.isEmpty() ? mLayerProperty.pkCols.at( 0 ) : QString();
  QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugError( u"connection item not found."_s );
    return QString();
  }

  QgsDataSourceUri uri = QgsDataSourceUri( connItem->connectionUri() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QgsMssqlUtils::wkbTypeFromGeometryType( mLayerProperty.type ) );
  uri.setUseEstimatedMetadata( QgsMssqlConnection::useEstimatedMetadata( connItem->name() ) );
  mDisableInvalidGeometryHandling = QgsMssqlConnection::isInvalidGeometryHandlingDisabled( connItem->name() );
  uri.setParam( u"disableInvalidGeometryHandling"_s, mDisableInvalidGeometryHandling ? u"1"_s : u"0"_s );
  if ( QgsMssqlConnection::geometryColumnsOnly( connItem->name() ) )
  {
    uri.setParam( u"extentInGeometryColumns"_s, QgsMssqlConnection::extentInGeometryColumns( connItem->name() ) ? u"1"_s : u"0"_s );
  }
  if ( mLayerProperty.isView )
    uri.setParam( u"primaryKeyInGeometryColumns"_s, QgsMssqlConnection::primaryKeyInGeometryColumns( connItem->name() ) ? u"1"_s : u"0"_s );

  QgsDebugMsgLevel( u"layer uri: %1"_s.arg( uri.uri() ), 3 );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsMssqlSchemaItem::QgsMssqlSchemaItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDatabaseSchemaItem( parent, name, path, u"MSSQL"_s )
{
  mIconName = u"mIconDbSchema.svg"_s;
  //not fertile, since children are created by QgsMssqlConnectionItem
  mCapabilities &= ~Qgis::BrowserItemCapabilities( Qgis::BrowserItemCapability::Fertile );
}

QVector<QgsDataItem *> QgsMssqlSchemaItem::createChildren()
{
  return QVector<QgsDataItem *>();
}


void QgsMssqlSchemaItem::addLayers( QgsDataItem *newLayers )
{
  // Add new items
  const auto constChildren = newLayers->children();
  for ( QgsDataItem *child : constChildren )
  {
    // Is it present in children?
    if ( findItem( mChildren, child ) >= 0 )
    {
      continue;
    }
    QgsMssqlLayerItem *layer = static_cast<QgsMssqlLayerItem *>( child )->createClone();
    addChildItem( layer, true );
  }
}

QgsMssqlLayerItem *QgsMssqlSchemaItem::addLayer( const QgsMssqlLayerProperty &layerProperty, bool refresh )
{
  Qgis::WkbType wkbType = QgsMssqlUtils::wkbTypeFromGeometryType( layerProperty.type );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName, QgsWkbTypes::displayString( wkbType ), layerProperty.srid );

  Qgis::BrowserLayerType layerType;
  const Qgis::GeometryType geomType = QgsWkbTypes::geometryType( wkbType );
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
      if ( layerProperty.type == "NONE"_L1 && layerProperty.geometryColName.isEmpty() )
      {
        layerType = Qgis::BrowserLayerType::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else if ( !layerProperty.geometryColName.isEmpty() && layerProperty.type.isEmpty() )
      {
        // geometry column is there but we failed to determine geometry type (e.g. due to invalid geometries)
        layerType = Qgis::BrowserLayerType::Vector;
      }
      else
      {
        return nullptr;
      }
  }

  QgsMssqlLayerItem *layerItem = new QgsMssqlLayerItem( this, layerProperty.tableName, mPath + '/' + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  if ( refresh )
    addChildItem( layerItem, true );
  else
  {
    addChild( layerItem );
    layerItem->setParent( this );
  }

  return layerItem;
}

void QgsMssqlSchemaItem::refresh()
{
  if ( auto *lParent = parent() )
    lParent->refresh();
}


QVector<QgsDataItem *> QgsMssqlLayerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  children.push_back( new QgsFieldsItem( this, uri() + u"/columns/ "_s, createUri(), providerKey(), mLayerProperty.schemaName, mLayerProperty.tableName ) );
  return children;
}

// ---------------------------------------------------------------------------
QgsMssqlRootItem::QgsMssqlRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsConnectionsRootItem( parent, name, path, u"MSSQL"_s )
{
  mIconName = u"mIconMssql.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsMssqlRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  QgsSettings settings;
  settings.beginGroup( u"/MSSQL/connections"_s );
  const auto constChildGroups = settings.childGroups();
  for ( const QString &connName : constChildGroups )
  {
    connections << new QgsMssqlConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

#ifdef HAVE_GUI
QWidget *QgsMssqlRootItem::paramWidget()
{
  QgsMssqlSourceSelect *select = new QgsMssqlSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsMssqlSourceSelect::connectionsChanged, this, &QgsMssqlRootItem::onConnectionsChanged );
  return select;
}

void QgsMssqlRootItem::onConnectionsChanged()
{
  refresh();
}
#endif

QString QgsMssqlDataItemProvider::name()
{
  return u"MSSQL"_s;
}

QString QgsMssqlDataItemProvider::dataProviderKey() const
{
  return u"mssql"_s;
}

Qgis::DataItemProviderCapabilities QgsMssqlDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::Databases;
}

QgsDataItem *QgsMssqlDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsMssqlRootItem( parentItem, QObject::tr( "MS SQL Server" ), u"mssql:"_s );
}


bool QgsMssqlSchemaItem::layerCollection() const
{
  return true;
}
