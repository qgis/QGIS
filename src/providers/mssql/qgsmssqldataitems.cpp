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

#include "qgsmssqlsourceselect.h"
#include "qgsmssqlnewconnection.h"
#include "qgslogger.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerimport.h"
#include "qgsdatasourceuri.h"
#include "qgsmssqlprovider.h"

#include <QSettings>
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QProgressDialog>

// ---------------------------------------------------------------------------
QgsMssqlConnectionItem::QgsMssqlConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
    , mUseGeometryColumns( false )
    , mUseEstimatedMetadata( false )
    , mAllowGeometrylessTables( true )
    , mColumnTypeThread( nullptr )
{
  mCapabilities |= Fast;
  mIconName = "mIconConnect.png";
}

QgsMssqlConnectionItem::~QgsMssqlConnectionItem()
{
  stop();
}

void QgsMssqlConnectionItem::readConnectionSettings()
{
  QSettings settings;
  QString key = "/MSSQL/connections/" + mName;
  mService = settings.value( key + "/service" ).toString();
  mHost = settings.value( key + "/host" ).toString();
  mDatabase = settings.value( key + "/database" ).toString();
  if ( settings.value( key + "/saveUsername" ).toString() == "true" )
  {
    mUsername = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == "true" )
  {
    mPassword = settings.value( key + "/password" ).toString();
  }

  mUseGeometryColumns = settings.value( key + "/geometryColumns", false ).toBool();
  mUseEstimatedMetadata = settings.value( key + "/estimatedMetadata", false ).toBool();
  mAllowGeometrylessTables = settings.value( key + "/allowGeometrylessTables", true ).toBool();

  mConnInfo =  "dbname='" + mDatabase + "' host=" + mHost + " user='" + mUsername + "' password='" + mPassword + '\'';
  if ( !mService.isEmpty() )
    mConnInfo += " service='" + mService + '\'';
  if ( mUseEstimatedMetadata )
    mConnInfo += " estimatedmetadata=true";
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
  QVector<QgsDataItem*> items = createChildren();

  // Add new items
  Q_FOREACH ( QgsDataItem *item, items )
  {
    // Is it present in childs?
    int index = findItem( mChildren, item );
    if ( index >= 0 )
    {
      (( QgsMssqlSchemaItem* )mChildren.at( index ) )->addLayers( item );
      delete item;
      continue;
    }
    addChildItem( item, true );
  }
}

QVector<QgsDataItem*> QgsMssqlConnectionItem::createChildren()
{

  setState( Populating );

  stop();

  QVector<QgsDataItem*> children;
  if ( deferredDelete() )
    return children;

  readConnectionSettings();

  QSqlDatabase db = QgsMssqlProvider::GetDatabase( mService, mHost, mDatabase, mUsername, mPassword );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    children.append( new QgsErrorItem( this, db.lastError().text(), mPath + "/error" ) );
    return children;
  }

  QString connectionName = db.connectionName();

  // build sql statement
  QString query( "select " );
  if ( mUseGeometryColumns )
  {
    query += "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type from geometry_columns";
  }
  else
  {
    query += "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY' from sys.columns join sys.types on sys.columns.system_type_id = sys.types.system_type_id and sys.columns.user_type_id = sys.types.user_type_id join sys.objects on sys.objects.object_id = sys.columns.object_id join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and (sys.objects.type = 'U' or sys.objects.type = 'V')";
  }

  if ( mAllowGeometrylessTables )
  {
    query += " union all select sys.schemas.name, sys.objects.name, null, null, 'NONE' from sys.objects join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where not exists (select * from sys.columns sc1 join sys.types on sc1.system_type_id = sys.types.system_type_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and sys.objects.object_id = sc1.object_id) and (sys.objects.type = 'U' or sys.objects.type = 'V')";
  }

  // issue the sql query
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );
  ( void )q.exec( query );

  if ( q.isActive() )
  {
    QVector<QgsDataItem*> newLayers;
    while ( q.next() )
    {
      QgsMssqlLayerProperty layer;
      layer.schemaName = q.value( 0 ).toString();
      layer.tableName = q.value( 1 ).toString();
      layer.geometryColName = q.value( 2 ).toString();
      layer.srid = q.value( 3 ).toString();
      layer.type = q.value( 4 ).toString();
      layer.pkCols = QStringList(); //TODO
      layer.isGeography = false;

      // skip layers which are added already
      bool skip = false;
      Q_FOREACH ( QgsDataItem *child, mChildren )
      {
        if ( child->name() == layer.schemaName )
        {
          Q_FOREACH ( QgsDataItem *child2, child->children() )
          {
            if ( child2->name() == layer.tableName )
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

      QgsMssqlSchemaItem* schemaItem = nullptr;
      Q_FOREACH ( QgsDataItem *child, children )
      {
        if ( child->name() == layer.schemaName )
        {
          schemaItem = ( QgsMssqlSchemaItem* )child;
          break;
        }
      }

      if ( !schemaItem )
      {
        schemaItem = new QgsMssqlSchemaItem( this, layer.schemaName, mPath + '/' + layer.schemaName );
        schemaItem->setState( Populating );
        children.append( schemaItem );
      }

      if ( !layer.geometryColName.isNull() )
      {
        if ( type == "GEOMETRY" || type.isNull() || srid.isEmpty() )
        {
          if ( !mColumnTypeThread )
          {
            mColumnTypeThread = new QgsMssqlGeomColumnTypeThread(
              connectionName, true /* use estimated metadata */ );

            connect( mColumnTypeThread, SIGNAL( setLayerType( QgsMssqlLayerProperty ) ),
                     this, SLOT( setLayerType( QgsMssqlLayerProperty ) ) );
            connect( this, SIGNAL( addGeometryColumn( QgsMssqlLayerProperty ) ),
                     mColumnTypeThread, SLOT( addGeometryColumn( QgsMssqlLayerProperty ) ) );
          }

          emit addGeometryColumn( layer );
          continue;
        }
      }

      QgsMssqlLayerItem* added = schemaItem->addLayer( layer, false );
      if ( added )
        newLayers.append( added );
    }

    // Remove no more present items
    Q_FOREACH ( QgsDataItem *child, mChildren )
    {
      Q_FOREACH ( QgsDataItem *child2, child->children() )
      {
        if ( findItem( newLayers, child2 ) < 0 )
          child->deleteChildItem( child2 );
      }
    }

    // spawn threads (new layers will be added later on)
    if ( mColumnTypeThread )
    {
      connect( mColumnTypeThread, SIGNAL( finished() ), this, SLOT( setAsPopulated() ) );
      mColumnTypeThread->start();
    }
    else
    {
      //set all as populated
      setAsPopulated();
    }
  }

  return children;
}

void QgsMssqlConnectionItem::setAsPopulated()
{
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    child->setState( Populated );
  }
  setState( Populated );
}

void QgsMssqlConnectionItem::setLayerType( QgsMssqlLayerProperty layerProperty )
{
  QgsMssqlSchemaItem *schemaItem = nullptr;

  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    if ( child->name() == layerProperty.schemaName )
    {
      schemaItem = ( QgsMssqlSchemaItem* )child;
      break;
    }
  }

  if ( !schemaItem )
  {
    QgsDebugMsg( QString( "schema item for %1 not found." ).arg( layerProperty.schemaName ) );
    return;
  }

  Q_FOREACH ( QgsDataItem *layerItem, schemaItem->children() )
  {
    if ( layerItem->name() == layerProperty.tableName )
      return; // already added
  }

  QStringList typeList = layerProperty.type.split( ',', QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ',', QString::SkipEmptyParts );
  Q_ASSERT( typeList.size() == sridList.size() );

  for ( int i = 0 ; i < typeList.size(); i++ )
  {
    QGis::WkbType wkbType = QgsMssqlTableModel::wkbTypeFromMssql( typeList[i] );
    if ( wkbType == QGis::WKBUnknown )
    {
      QgsDebugMsg( QString( "unsupported geometry type:%1" ).arg( typeList[i] ) );
      continue;
    }

    layerProperty.type = typeList[i];
    layerProperty.srid = sridList[i];
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

QList<QAction*> QgsMssqlConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionShowNoGeom = new QAction( tr( "Show Non-Spatial Tables" ), this );
  actionShowNoGeom->setCheckable( true );
  actionShowNoGeom->setChecked( mAllowGeometrylessTables );
  connect( actionShowNoGeom, SIGNAL( toggled( bool ) ), this, SLOT( setAllowGeometrylessTables( bool ) ) );
  lst.append( actionShowNoGeom );

  QAction* actionEdit = new QAction( tr( "Edit Connection..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete Connection" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsMssqlConnectionItem::setAllowGeometrylessTables( bool allow )
{
  mAllowGeometrylessTables = allow;
  QString key = "/MSSQL/connections/" + mName;
  QSettings settings;
  settings.setValue( key + "/allowGeometrylessTables", allow );
  refresh();
}

void QgsMssqlConnectionItem::editConnection()
{
  QgsMssqlNewConnection nc( nullptr, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsMssqlConnectionItem::deleteConnection()
{
  QgsMssqlSourceSelect::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}

bool QgsMssqlConnectionItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  return handleDrop( data, QString() );
}

bool QgsMssqlConnectionItem::handleDrop( const QMimeData* data, const QString& toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  qApp->setOverrideCursor( Qt::WaitCursor );

  QProgressDialog *progress = new QProgressDialog( tr( "Copying features..." ), tr( "Abort" ), 0, 0, nullptr );
  progress->setWindowTitle( tr( "Import layer" ) );
  progress->setWindowModality( Qt::WindowModal );
  progress->show();

  QStringList importResults;
  bool hasError = false;
  bool cancelled = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Q_FOREACH ( const QgsMimeDataUtils::Uri& u, lst )
  {
    if ( u.layerType != "vector" )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    // open the source layer
    QgsVectorLayer* srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey );

    if ( srcLayer->isValid() )
    {
      QString tableName;
      if ( !toSchema.isEmpty() )
      {
        tableName = QString( "\"%1\".\"%2\"" ).arg( toSchema, u.name );
      }
      else
      {
        tableName = u.name;
      }

      QString uri = connInfo() + " table=" + tableName;
      if ( srcLayer->geometryType() != QGis::NoGeometry )
        uri += " (geom)";

      QgsVectorLayerImport::ImportError err;
      QString importError;
      err = QgsVectorLayerImport::importLayer( srcLayer, uri, "mssql", &srcLayer->crs(), false, &importError, false, nullptr, progress );
      if ( err == QgsVectorLayerImport::NoError )
        importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      else if ( err == QgsVectorLayerImport::ErrUserCancelled )
        cancelled = true;
      else
      {
        importResults.append( QString( "%1: %2" ).arg( u.name, importError ) );
        hasError = true;
      }
    }
    else
    {
      importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      hasError = true;
    }

    delete srcLayer;
  }

  delete progress;
  qApp->restoreOverrideCursor();

  if ( cancelled )
  {
    QMessageBox::information( nullptr, tr( "Import to MSSQL database" ), tr( "Import cancelled." ) );
    refresh();
  }
  else if ( hasError )
  {
    QMessageBox::warning( nullptr, tr( "Import to MSSQL database" ), tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ) );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Import to MSSQL database" ), tr( "Import was successful." ) );
  }

  if ( state() == Populated )
    refresh();
  else
    populate();

  return true;
}


// ---------------------------------------------------------------------------
QgsMssqlLayerItem::QgsMssqlLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsMssqlLayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, "mssql" )
    , mLayerProperty( layerProperty )
{
  mUri = createUri();
  setState( Populated );
}

QgsMssqlLayerItem::~QgsMssqlLayerItem()
{
}

QgsMssqlLayerItem* QgsMssqlLayerItem::createClone()
{
  return new QgsMssqlLayerItem( mParent, mName, mPath, mLayerType, mLayerProperty );
}

QString QgsMssqlLayerItem::createUri()
{
  QString pkColName = !mLayerProperty.pkCols.isEmpty() ? mLayerProperty.pkCols.at( 0 ) : QString::null;
  QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDataSourceURI uri = QgsDataSourceURI( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QGis::fromOldWkbType( QgsMssqlTableModel::wkbTypeFromMssql( mLayerProperty.type ) ) );
  QgsDebugMsg( QString( "layer uri: %1" ).arg( uri.uri() ) );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsMssqlSchemaItem::QgsMssqlSchemaItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconDbSchema.png";
  //not fertile, since children are created by QgsMssqlConnectionItem
  mCapabilities &= ~( Fertile );
}

QVector<QgsDataItem*> QgsMssqlSchemaItem::createChildren()
{
  QgsDebugMsg( "Entering." );
  return QVector<QgsDataItem*>();
}

QgsMssqlSchemaItem::~QgsMssqlSchemaItem()
{
}

void QgsMssqlSchemaItem::addLayers( QgsDataItem* newLayers )
{
  // Add new items
  Q_FOREACH ( QgsDataItem *child, newLayers->children() )
  {
    // Is it present in childs?
    if ( findItem( mChildren, child ) >= 0 )
    {
      continue;
    }
    QgsMssqlLayerItem* layer = (( QgsMssqlLayerItem* )child )->createClone();
    addChildItem( layer, true );
  }
}

bool QgsMssqlSchemaItem::handleDrop( const QMimeData* data, Qt::DropAction )
{
  QgsMssqlConnectionItem *conn = qobject_cast<QgsMssqlConnectionItem *>( parent() );
  if ( !conn )
    return 0;

  return conn->handleDrop( data, mName );
}

QgsMssqlLayerItem* QgsMssqlSchemaItem::addLayer( const QgsMssqlLayerProperty& layerProperty, bool refresh )
{
  QGis::WkbType wkbType = QgsMssqlTableModel::wkbTypeFromMssql( layerProperty.type );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName, QgsMssqlTableModel::displayStringForWkbType( wkbType ), layerProperty.srid );

  QgsLayerItem::LayerType layerType;
  switch ( wkbType )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      layerType = QgsLayerItem::Point;
      break;
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      layerType = QgsLayerItem::Line;
      break;
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( layerProperty.type == "NONE" && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
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
QgsMssqlRootItem::QgsMssqlRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconMssql.svg";
  populate();
}

QgsMssqlRootItem::~QgsMssqlRootItem()
{
}

QVector<QgsDataItem*> QgsMssqlRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  QSettings settings;
  settings.beginGroup( "/MSSQL/connections" );
  Q_FOREACH ( const QString& connName, settings.childGroups() )
  {
    connections << new QgsMssqlConnectionItem( this, connName, mPath + '/' + connName );
  }
  return connections;
}

QList<QAction*> QgsMssqlRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsMssqlRootItem::paramWidget()
{
  QgsMssqlSourceSelect *select = new QgsMssqlSourceSelect( nullptr, nullptr, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsMssqlRootItem::connectionsChanged()
{
  refresh();
}

void QgsMssqlRootItem::newConnection()
{
  QgsMssqlNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    refresh();
  }
}
