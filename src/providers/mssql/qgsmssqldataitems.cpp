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
#include "qgsmssqlconnection.h"

#include "qgsmssqlgeomcolumntypethread.h"
#include "qgslogger.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"
#include "qgsdatasourceuri.h"
#include "qgsmssqlprovider.h"
#include "qgssettings.h"
#include "qgsmessageoutput.h"
#include "qgsmssqlconnection.h"
#include "qgsapplication.h"
#include "qgsproject.h"

#ifdef HAVE_GUI
#include "qgsmssqlsourceselect.h"
#endif

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>

// ---------------------------------------------------------------------------
QgsMssqlConnectionItem::QgsMssqlConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
  , mUseGeometryColumns( false )
  , mUseEstimatedMetadata( false )
  , mAllowGeometrylessTables( true )
{
  mCapabilities |= Fast | Collapse;
  mIconName = QStringLiteral( "mIconConnect.svg" );
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
  if ( settings.value( key + "/saveUsername" ).toString() == QLatin1String( "true" ) )
  {
    mUsername = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == QLatin1String( "true" ) )
  {
    mPassword = settings.value( key + "/password" ).toString();
  }

  mUseGeometryColumns = QgsMssqlConnection::geometryColumnsOnly( mName );
  mUseEstimatedMetadata = QgsMssqlConnection::useEstimatedMetadata( mName );
  mAllowGeometrylessTables = QgsMssqlConnection::allowGeometrylessTables( mName );

  mConnInfo = "dbname='" + mDatabase + "' host='" + mHost + "' user='" + mUsername + "' password='" + mPassword + '\'';
  if ( !mService.isEmpty() )
    mConnInfo += " service='" + mService + '\'';
  if ( mUseEstimatedMetadata )
    mConnInfo += QLatin1String( " estimatedmetadata=true" );
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
  QgsDebugMsg( "mPath = " + mPath );
  stop();

  // read up the schemas and layers from database
  QVector<QgsDataItem *> items = createChildren();

  // Add new items
  const auto constItems = items;
  for ( QgsDataItem *item : constItems )
  {
    // Is it present in children?
    int index = findItem( mChildren, item );
    if ( index >= 0 )
    {
      static_cast< QgsMssqlSchemaItem * >( mChildren.at( index ) )->addLayers( item );
      delete item;
      continue;
    }
    addChildItem( item, true );
  }
}

QVector<QgsDataItem *> QgsMssqlConnectionItem::createChildren()
{

  setState( Populating );

  stop();

  QVector<QgsDataItem *> children;
  if ( deferredDelete() )
    return children;

  readConnectionSettings();

  QSqlDatabase db = QgsMssqlConnection::getDatabase( mService, mHost, mDatabase, mUsername, mPassword );

  if ( !QgsMssqlConnection::openDatabase( db ) )
  {
    children.append( new QgsErrorItem( this, db.lastError().text(), mPath + "/error" ) );
    return children;
  }

  // build sql statement
  QString query( QStringLiteral( "select " ) );
  if ( mUseGeometryColumns )
  {
    query += QLatin1String( "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0 from geometry_columns" );
  }
  else
  {
    query += QLatin1String( "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY', case when sys.objects.type = 'V' then 1 else 0 end from sys.columns join sys.types on sys.columns.system_type_id = sys.types.system_type_id and sys.columns.user_type_id = sys.types.user_type_id join sys.objects on sys.objects.object_id = sys.columns.object_id join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and (sys.objects.type = 'U' or sys.objects.type = 'V')" );
  }

  if ( mAllowGeometrylessTables )
  {
    query += QLatin1String( " union all select sys.schemas.name, sys.objects.name, null, null, 'NONE', case when sys.objects.type = 'V' then 1 else 0 end from sys.objects join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where not exists (select * from sys.columns sc1 join sys.types on sc1.system_type_id = sys.types.system_type_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and sys.objects.object_id = sc1.object_id) and (sys.objects.type = 'U' or sys.objects.type = 'V')" );
  }

  const bool disableInvalidGeometryHandling = QgsMssqlConnection::isInvalidGeometryHandlingDisabled( mName );

  // issue the sql query
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );
  ( void )q.exec( query );

  QSet< QString > addedSchemas;

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
      layer.pkCols = QStringList(); //TODO
      layer.isGeography = false;

      // skip layers which are added already
      bool skip = false;
      const auto constMChildren = mChildren;
      for ( QgsDataItem *child : constMChildren )
      {
        if ( child->name() == layer.schemaName )
        {
          const auto constChildren = child->children();
          for ( QgsDataItem *child2 : constChildren )
          {
            QgsMssqlLayerItem *layerItem = qobject_cast< QgsMssqlLayerItem *>( child2 );
            if ( child2->name() == layer.tableName && layerItem && layerItem->disableInvalidGeometryHandling() == disableInvalidGeometryHandling )
            {
              newLayers.append( child2 );
              skip = true; // already added
              break;
            }
          }
        }
      }

      if ( skip )
        continue;

      QString type = layer.type;
      QString srid = layer.srid;

      QgsMssqlSchemaItem *schemaItem = nullptr;
      const auto constChildren = children;
      for ( QgsDataItem *child : constChildren )
      {
        if ( child->name() == layer.schemaName )
        {
          schemaItem = static_cast< QgsMssqlSchemaItem * >( child );
          break;
        }
      }

      if ( !schemaItem )
      {
        schemaItem = new QgsMssqlSchemaItem( this, layer.schemaName, mPath + '/' + layer.schemaName );
        schemaItem->setState( Populating );
        addedSchemas.insert( layer.schemaName );
        children.append( schemaItem );
      }

      if ( !layer.geometryColName.isNull() )
      {
        if ( type == QLatin1String( "GEOMETRY" ) || type.isNull() || srid.isEmpty() )
        {
          if ( !mColumnTypeThread )
          {
            mColumnTypeThread = new QgsMssqlGeomColumnTypeThread( mService, mHost, mDatabase, mUsername, mPassword, true /* use estimated metadata */ );

            connect( mColumnTypeThread, &QgsMssqlGeomColumnTypeThread::setLayerType,
                     this, &QgsMssqlConnectionItem::setLayerType );
            connect( this, &QgsMssqlConnectionItem::addGeometryColumn,
                     mColumnTypeThread, &QgsMssqlGeomColumnTypeThread::addGeometryColumn );
          }

          emit addGeometryColumn( layer );
          continue;
        }
      }

      QgsMssqlLayerItem *added = schemaItem->addLayer( layer, false );
      if ( added )
        newLayers.append( added );
    }

    // Remove no more present items
    const auto constMChildren = mChildren;
    for ( QgsDataItem *child : constMChildren )
    {
      const auto constChildren = child->children();
      for ( QgsDataItem *child2 : constChildren )
      {
        if ( findItem( newLayers, child2 ) < 0 )
          child->deleteChildItem( child2 );
      }
    }

    // add missing schemas (i.e., empty schemas)
    const QString uri = connInfo();
    const QStringList allSchemas = QgsMssqlConnection::schemas( uri, nullptr );
    for ( const QString &schema : allSchemas )
    {
      if ( addedSchemas.contains( schema ) )
        continue;

      if ( QgsMssqlConnection::isSystemSchema( schema ) )
        continue;

      QgsMssqlSchemaItem *schemaItem = new QgsMssqlSchemaItem( this, schema, mPath + '/' + schema );
      schemaItem->setState( Populated ); // no tables
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
      for ( QgsDataItem *child : qgis::as_const( children ) )
      {
        child->setState( Populated );
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
    child->setState( Populated );
  }
  setState( Populated );
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
      schemaItem = static_cast< QgsMssqlSchemaItem * >( child );
      break;
    }
  }

  if ( !schemaItem )
  {
    QgsDebugMsg( QStringLiteral( "schema item for %1 not found." ).arg( layerProperty.schemaName ) );
    return;
  }

  const auto constChildren = schemaItem->children();
  for ( QgsDataItem *layerItem : constChildren )
  {
    if ( layerItem->name() == layerProperty.tableName )
      return; // already added
  }

  QStringList typeList = layerProperty.type.split( ',', QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ',', QString::SkipEmptyParts );
  Q_ASSERT( typeList.size() == sridList.size() );

  for ( int i = 0; i < typeList.size(); i++ )
  {
    QgsWkbTypes::Type wkbType = QgsMssqlTableModel::wkbTypeFromMssql( typeList[i] );
    if ( wkbType == QgsWkbTypes::Unknown )
    {
      QgsDebugMsg( QStringLiteral( "unsupported geometry type:%1" ).arg( typeList[i] ) );
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

bool QgsMssqlConnectionItem::handleDrop( const QMimeData *data, const QString &toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  const auto constLst = lst;
  for ( const QgsMimeDataUtils::Uri &u : constLst )
  {
    if ( u.layerType != QLatin1String( "vector" ) )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    // open the source layer
    const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
    QgsVectorLayer *srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey, options );

    if ( srcLayer->isValid() )
    {
      QString tableName;
      if ( !toSchema.isEmpty() )
      {
        tableName = QStringLiteral( "\"%1\".\"%2\"" ).arg( toSchema, u.name );
      }
      else
      {
        tableName = u.name;
      }

      QString uri = connInfo() + " table=" + tableName;
      if ( srcLayer->geometryType() != QgsWkbTypes::NullGeometry )
        uri += QLatin1String( " (geom)" );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( QgsVectorLayerExporterTask::withLayerOwnership( srcLayer, uri, QStringLiteral( "mssql" ), srcLayer->crs() ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to MSSQL database" ), tr( "Import was successful." ) );
        if ( state() == Populated )
          refresh();
        else
          populate();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
      {
        if ( error != QgsVectorLayerExporter::ErrUserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to MSSQL database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        if ( state() == Populated )
          refresh();
        else
          populate();
      } );

      QgsApplication::taskManager()->addTask( exportTask.release() );
    }
    else
    {
      importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
      hasError = true;
    }
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to MSSQL database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}


// ---------------------------------------------------------------------------
QgsMssqlLayerItem::QgsMssqlLayerItem( QgsDataItem *parent, const QString &name, const QString &path, QgsLayerItem::LayerType layerType, const QgsMssqlLayerProperty &layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, QStringLiteral( "mssql" ) )
  , mLayerProperty( layerProperty )
{
  mCapabilities |= Delete;
  mUri = createUri();
  setState( Populated );
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
    QgsDebugMsg( QStringLiteral( "connection item not found." ) );
    return QString();
  }

  QgsDataSourceUri uri = QgsDataSourceUri( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QgsMssqlTableModel::wkbTypeFromMssql( mLayerProperty.type ) );
  uri.setUseEstimatedMetadata( QgsMssqlConnection::useEstimatedMetadata( connItem->name() ) );
  mDisableInvalidGeometryHandling = QgsMssqlConnection::isInvalidGeometryHandlingDisabled( connItem->name() );
  uri.setParam( QStringLiteral( "disableInvalidGeometryHandling" ), mDisableInvalidGeometryHandling ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  QgsDebugMsg( QStringLiteral( "layer uri: %1" ).arg( uri.uri() ) );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsMssqlSchemaItem::QgsMssqlSchemaItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
  //not fertile, since children are created by QgsMssqlConnectionItem
  mCapabilities &= ~( Fertile );
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
    QgsMssqlLayerItem *layer = static_cast< QgsMssqlLayerItem * >( child )->createClone();
    addChildItem( layer, true );
  }
}

QgsMssqlLayerItem *QgsMssqlSchemaItem::addLayer( const QgsMssqlLayerProperty &layerProperty, bool refresh )
{
  QgsWkbTypes::Type wkbType = QgsMssqlTableModel::wkbTypeFromMssql( layerProperty.type );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName, QgsWkbTypes::displayString( wkbType ), layerProperty.srid );

  QgsLayerItem::LayerType layerType;
  QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( wkbType );
  switch ( flatType )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::MultiPoint:
      layerType = QgsLayerItem::Point;
      break;
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::MultiLineString:
      layerType = QgsLayerItem::Line;
      break;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::MultiPolygon:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( layerProperty.type == QLatin1String( "NONE" ) && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else if ( !layerProperty.geometryColName.isEmpty() && layerProperty.type.isEmpty() )
      {
        // geometry column is there but we failed to determine geometry type (e.g. due to invalid geometries)
        layerType = QgsLayerItem::Vector;
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
    addChild( layerItem );

  return layerItem;
}

// ---------------------------------------------------------------------------
QgsMssqlRootItem::QgsMssqlRootItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconMssql.svg" );
  populate();
}

QVector<QgsDataItem *> QgsMssqlRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/MSSQL/connections" ) );
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
  QgsMssqlSourceSelect *select = new QgsMssqlSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
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
  return QStringLiteral( "MSSQL" );
}

int QgsMssqlDataItemProvider::capabilities() const
{
  return QgsDataProvider::Database;
}

QgsDataItem *QgsMssqlDataItemProvider::createDataItem( const QString &pathIn, QgsDataItem *parentItem )
{
  Q_UNUSED( pathIn )
  return new QgsMssqlRootItem( parentItem, QStringLiteral( "MSSQL" ), QStringLiteral( "mssql:" ) );
}
