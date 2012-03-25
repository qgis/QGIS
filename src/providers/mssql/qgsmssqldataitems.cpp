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

// ---------------------------------------------------------------------------
QgsMssqlConnectionItem::QgsMssqlConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconConnect.png" ) );
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

  mConnInfo =  "dbname='" + mDatabase + "' host=" + mHost + " user='" + mUsername + "' password='" + mPassword + "'";
  if ( !mService.isEmpty() )
    mConnInfo += " service='" + mService + "'";
}

QgsMssqlConnectionItem::~QgsMssqlConnectionItem()
{
}

void QgsMssqlConnectionItem::refresh()
{
  QgsDebugMsg( "mPath = " + mPath );

  // read up the schemas and layers from database
  QVector<QgsDataItem*> items = createChildren( );

  // Add new items
  foreach( QgsDataItem *item, items )
  {
    // Is it present in childs?
    int index = findItem( mChildren, item );
    if ( index >= 0 )
    {
      (( QgsMssqlSchemaItem* )mChildren[index] )->addLayers( item );
      delete item;
      continue;
    }
    addChildItem( item, true );
  }
}

QVector<QgsDataItem*> QgsMssqlConnectionItem::createChildren()
{
  QgsDebugMsg( "Entered" );

  QVector<QgsDataItem*> children;

  QSqlDatabase db = QgsMssqlProvider::GetDatabase( mService,
                    mHost, mDatabase, mUsername, mPassword );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    children.append( new QgsErrorItem( this, db.lastError( ).text( ), mPath + "/error" ) );
    return children;
  }

  QString connectionName;
  if ( mService.isEmpty() )
  {
    if ( mHost.isEmpty() )
    {
      children.append( new QgsErrorItem( this, "QgsMssqlProvider host name not specified", mPath + "/error" ) );
      return children;
    }

    if ( mDatabase.isEmpty() )
    {
      children.append( new QgsErrorItem( this, "QgsMssqlProvider database name not specified", mPath + "/error" ) );
      return children;
    }
    connectionName = mHost + "." + mDatabase;
  }
  else
    connectionName = mService;

  QgsMssqlGeomColumnTypeThread *columnTypeThread = 0;

  // build sql statement
  QString query( "select " );
  if ( mUseGeometryColumns )
  {
    query += "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type from geometry_columns";
  }
  else
  {
    query += "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY' from sys.columns join sys.types on sys.columns.system_type_id = sys.types.system_type_id and sys.columns.user_type_id = sys.types.user_type_id join sys.objects on sys.objects.object_id = sys.columns.object_id join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where sys.types.name = 'geometry' or sys.types.name = 'geography' and sys.objects.type = 'U'";
  }

  // issue the sql query
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );
  q.exec( query );

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

      // skip layers which are added already
      bool skip = false;
      foreach( QgsDataItem *child, mChildren )
      {
        if ( child->name() == layer.schemaName )
        {
          foreach( QgsDataItem *child2, child->children() )
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

      QgsMssqlSchemaItem* schemaItem = NULL;
      foreach( QgsDataItem *child, children )
      {
        if ( child->name() == layer.schemaName )
        {
          schemaItem = ( QgsMssqlSchemaItem* )child;
          break;
        }
      }

      if ( !schemaItem )
      {
        schemaItem = new QgsMssqlSchemaItem( this, layer.schemaName, mPath + "/" + layer.schemaName );
        children.append( schemaItem );
      }

      if ( !layer.geometryColName.isNull() )
      {
        if ( type == "GEOMETRY" || type.isNull() || srid.isEmpty() )
        {
          if ( !columnTypeThread )
          {
            columnTypeThread = new QgsMssqlGeomColumnTypeThread(
              connectionName, true /* use estimated metadata */ );

            connect( columnTypeThread, SIGNAL( setLayerType( QgsMssqlLayerProperty ) ),
                     this, SLOT( setLayerType( QgsMssqlLayerProperty ) ) );
            connect( this, SIGNAL( addGeometryColumn( QgsMssqlLayerProperty ) ),
                     columnTypeThread, SLOT( addGeometryColumn( QgsMssqlLayerProperty ) ) );
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
    foreach( QgsDataItem *child, mChildren )
    {
      foreach( QgsDataItem *child2, child->children() )
      {
        if ( findItem( newLayers, child2 ) < 0 )
          child->deleteChildItem( child2 );
      }
    }

    // spawn threads (new layers will be added later on)
    if ( columnTypeThread )
      columnTypeThread->start();
  }

  return children;
}

void QgsMssqlConnectionItem::setLayerType( QgsMssqlLayerProperty layerProperty )
{
  QgsMssqlSchemaItem *schemaItem = NULL;

  foreach( QgsDataItem *child, mChildren )
  {
    if ( child->name() == layerProperty.schemaName )
    {
      schemaItem = ( QgsMssqlSchemaItem* )child;
      break;
    }
  }

  foreach( QgsDataItem *layerItem, schemaItem->children() )
  {
    if ( layerItem->name() == layerProperty.tableName )
      return; // already added
  }

  if ( !schemaItem )
  {
    QgsDebugMsg( QString( "schema item for %1 not found." ).arg( layerProperty.schemaName ) );
    return;
  }

  QStringList typeList = layerProperty.type.split( ",", QString::SkipEmptyParts );
  QStringList sridList = layerProperty.srid.split( ",", QString::SkipEmptyParts );
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

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsMssqlConnectionItem::editConnection()
{
  QgsMssqlNewConnection nc( NULL, mName );
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
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc

  qApp->setOverrideCursor( Qt::WaitCursor );

  QStringList importResults;
  bool hasError = false;
  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  foreach( const QgsMimeDataUtils::Uri& u, lst )
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
      QString uri = connInfo() + " table=" + u.name + " (qgs_geometry)";

      QgsVectorLayerImport::ImportError err;
      QString importError;
      err = QgsVectorLayerImport::importLayer( srcLayer, uri, "mssql", &srcLayer->crs(), false, &importError );
      if ( err == QgsVectorLayerImport::NoError )
        importResults.append( tr( "%1: OK!" ).arg( u.name ) );
      else
      {
        importResults.append( QString( "%1: %2" ).arg( u.name ).arg( importError ) );
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

  qApp->restoreOverrideCursor();

  if ( hasError )
  {
    QMessageBox::warning( 0, tr( "Import to MSSQL database" ), tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ) );
  }
  else
  {
    QMessageBox::information( 0, tr( "Import to MSSQL database" ), tr( "Import was successful." ) );
  }

  if ( mPopulated )
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
  mPopulated = true;
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
  QString pkColName = mLayerProperty.pkCols.size() > 0 ? mLayerProperty.pkCols.at( 0 ) : QString::null;
  QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDataSourceURI uri = QgsDataSourceURI( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QgsMssqlTableModel::wkbTypeFromMssql( mLayerProperty.type ) );
  QgsDebugMsg( QString( "layer uri: %1" ).arg( uri.uri() ) );
  return uri.uri();
}

// ---------------------------------------------------------------------------
QgsMssqlSchemaItem::QgsMssqlSchemaItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QIcon( getThemePixmap( "mIconDbSchema.png" ) );
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
  foreach( QgsDataItem *child, newLayers->children() )
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

QgsMssqlLayerItem* QgsMssqlSchemaItem::addLayer( QgsMssqlLayerProperty layerProperty, bool refresh )
{
  QGis::WkbType wkbType = QgsMssqlTableModel::wkbTypeFromMssql( layerProperty.type );
  QString tip = tr( "%1 as %2 in %3" ).arg( layerProperty.geometryColName ).arg( QgsMssqlTableModel::displayStringForWkbType( wkbType ) ).arg( layerProperty.srid );

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
      if ( layerProperty.type.isEmpty() && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else
      {
        return NULL;
      }
  }

  QgsMssqlLayerItem *layerItem = new QgsMssqlLayerItem( this, layerProperty.tableName, mPath + "/" + layerProperty.tableName, layerType, layerProperty );
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
  mIcon = QIcon( getThemePixmap( "mIconMssql.png" ) );
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
  foreach( QString connName, settings.childGroups() )
  {
    connections << new QgsMssqlConnectionItem( this, connName, mPath + "/" + connName );
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
  QgsMssqlSourceSelect *select = new QgsMssqlSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsMssqlRootItem::connectionsChanged()
{
  refresh();
}

void QgsMssqlRootItem::newConnection()
{
  QgsMssqlNewConnection nc( NULL );
  if ( nc.exec() )
  {
    refresh();
  }
}
