/***************************************************************************
  qgsdb2dataitems.cpp - Browser Panel object population
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xiaoshir at us.ibm.com, nguyend at us.ibm.com
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2dataitems.h"
#include "qgsdb2newconnection.h"
#include <qgslogger.h>
#include <qaction.h>

static const QString PROVIDER_KEY = "DB2";

QgsDb2ConnectionItem::QgsDb2ConnectionItem( QgsDataItem *parent, const QString name, const QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconConnect.png";
  populate();
}

QgsDb2ConnectionItem::~QgsDb2ConnectionItem()
{
}
void QgsDb2ConnectionItem::readConnectionSettings()
{
  QSettings settings;
  QString key = "/DB2/connections/" + mName;
  mService = settings.value( key + "/service" ).toString();
  mDriver = settings.value( key + "/driver" ).toString();
  mHost = settings.value( key + "/host" ).toString();
  mPort = settings.value( key + "/port" ).toString();
  mDatabase = settings.value( key + "/database" ).toString();
  mUsername = settings.value( key + "/username" ).toString();
  mPassword = settings.value( key + "/password" ).toString();
  mEnvironment = settings.value( key + "/environment" ).toInt();

  //mUseEstimatedMetadata = settings.value( key + "/estimatedMetadata", false ).toBool();
  //mAllowGeometrylessTables = settings.value( key + "/allowGeometrylessTables", true ).toBool();

  mConnInfo =  "dbname='" + mDatabase + "' driver='" + mDriver + "' host=" + mHost + " port=" + mPort + " user='" + mUsername + "' password='" + mPassword + "'";
  if ( !mService.isEmpty() )
    mConnInfo += " service='" + mService + "' user='" + mUsername + "' password='" + mPassword + "'";
  if ( mUseEstimatedMetadata )
    mConnInfo += " estimatedmetadata=true";
  QgsDebugMsg( "mConnInfo: '" + mConnInfo + "'" );
}

void QgsDb2ConnectionItem::refresh()
{
  QgsDebugMsg( "db2 mPath = " + mPath );

  // read up the schemas and layers from database
  QVector<QgsDataItem*> items = createChildren();

  // Add new items
  Q_FOREACH ( QgsDataItem *item, items )
  {
    // Is it present in childs?
    int index = findItem( mChildren, item );
    if ( index >= 0 )
    {
      (( QgsDb2SchemaItem* )mChildren.at( index ) )->addLayers( item );
      delete item;
      continue;
    }
    addChildItem( item, true );
  }
}

QVector<QgsDataItem*> QgsDb2ConnectionItem::createChildren()
{
  QgsDebugMsg( "Entering." );

  QVector<QgsDataItem*> children;
//  QgsDb2GeomColumnTypeThread *columnTypeThread = 0; // TODO - what is this?

  readConnectionSettings();

  bool convertIntOk;
  int portNum = mPort.toInt( &convertIntOk, 10 );
  QSqlDatabase db = QgsDb2Provider::GetDatabase( mService, mDriver, mHost, portNum, mDatabase, mUsername, mPassword );
  if ( db.open() )
  {
    QString connectionName = db.connectionName();
    //children.append( new QgsFavouritesItem(this, "connection successful", mPath + "/success"));
    QgsDebugMsg( "DB open successful for connection " + connectionName );
  }
  else
  {
    children.append( new QgsErrorItem( this, db.lastError().text(), mPath + "/error" ) );
    QgsDebugMsg( "DB not open" + db.lastError().text() );
    return children;
  }
  QString connectionName = db.connectionName();

  // build sql statement, QgsDb2SourceSelect::on_btnConnect_clicked() has identical query
  QString query( "SELECT TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME, TYPE_NAME, SRS_ID FROM DB2GSE.ST_GEOMETRY_COLUMNS" );

  // issue the sql query
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );
  if ( !q.exec( query ) )
  {
    QgsDebugMsg( "ST_Geometry_Columns query failed: " + db.lastError().text() );
    QgsDebugMsg( QString( "SQLCODE: %1" ).arg( q.lastError().number() ) );
    /* Enabling the DB2 Spatial Extender creates the DB2GSE schema and tables,
    so the Extender is either not enabled or set up if SQLCODE -204 is returned. */
    if ( q.lastError().number() == -204 )
    {
      children.append( new QgsErrorItem( this, tr( "DB2 Spatial Extender is not enabled or set up." ), mPath + "/error" ) );
    }
    else
    {
      children.append( new QgsErrorItem( this, db.lastError().text(), mPath + "/error" ) );
    }
    return children;
  }
  else if ( q.isActive() )
  {
    //QVector<QgsDataItem*> newLayers;
    while ( q.next() )
    {
      QgsDb2LayerProperty layer;
      layer.schemaName = q.value( 0 ).toString().trimmed();
      layer.tableName = q.value( 1 ).toString().trimmed();
      layer.geometryColName = q.value( 2 ).toString().trimmed();
      layer.type = q.value( 3 ).toString();
      layer.srid = q.value( 4 ).toString();
      layer.pkCols = QStringList();

      QString type = layer.type;
      QString srid = layer.srid;

      QgsDb2SchemaItem* schemaItem = NULL;
      Q_FOREACH ( QgsDataItem *child, children )
      {
        if ( child->name() == layer.schemaName )
        {
          schemaItem = ( QgsDb2SchemaItem* )child;
          break;
        }
      }

      if ( !schemaItem )
      {
        schemaItem = new QgsDb2SchemaItem( this, layer.schemaName, mPath + "/" + layer.schemaName );
        QgsDebugMsg( "DB2 *** Adding Schema Item : " + layer.schemaName + " " + mPath + "/" + layer.schemaName + " type=" + layer.type
                     + " srid=" + layer.srid + " table=" + layer.tableName + " geoName=" + layer.geometryColName
                   );
        children.append( schemaItem );
      }

      QgsDb2LayerItem* added = schemaItem->addLayer( layer, true );

      if ( added )
      {
        QgsDebugMsg( " DB2 adding layer to schema item: " + added->name() );
        //children.append(added);
      }
      else
      {
        QgsDebugMsg( " DB2 layer not added " );
      }
    }
  }

  return children;
}

QList<QAction*> QgsDb2ConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionRefresh = new QAction( tr( "Refresh" ), this );
  connect( actionRefresh, SIGNAL( triggered() ), this, SLOT( refreshConnection() ) );
  lst.append( actionRefresh );

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsDb2ConnectionItem::editConnection()
{
  QgsDb2NewConnection nc( nullptr, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsDb2ConnectionItem::deleteConnection()
{
  QString key = "/DB2/connections/" + mName;
  QSettings settings;
  settings.remove( key + "/service" );
  settings.remove( key + "/driver" );
  settings.remove( key + "/port" );
  settings.remove( key + "/host" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/environment" );
  settings.remove( key );
  mParent->refresh();
}

void QgsDb2ConnectionItem::refreshConnection()
{

  bool convertIntOk;
  int portNum = mPort.toInt( &convertIntOk, 10 );
  QSqlDatabase db = QgsDb2Provider::GetDatabase( mService, mDriver, mHost, portNum, mDatabase, mUsername, mPassword );
  if ( db.open() )
  {
    QString connectionName = db.connectionName();
    QgsDebugMsg( "successful get db2 connection on refresh" );
  }
  else
  {
    QgsDebugMsg( "failed get db2 connection on refresh " + db.lastError().text() + " " + mPath + "/error" );
  }
  refresh();
}

QgsDb2RootItem::QgsDb2RootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconDb2.svg";
  populate();
}

QgsDb2RootItem::~QgsDb2RootItem()
{
}

QVector<QgsDataItem*> QgsDb2RootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  QSettings settings;
  settings.beginGroup( "/DB2/connections" );
  Q_FOREACH ( const QString& connName, settings.childGroups() )
  {
    connections << new QgsDb2ConnectionItem( this, connName, mPath + "/" + connName );
  }
  return connections;
}

QList<QAction*> QgsDb2RootItem::actions()
{
  QList<QAction*> actionList;

  QAction* action = new QAction( tr( "New Connection..." ), this );
  connect( action, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  actionList.append( action );
  QgsDebugMsg( "DB2: Browser Panel; New Connection option added." );

  return actionList;
}

QWidget *QgsDb2RootItem::paramWidget()
{
  return NULL; //TODO?
}

void QgsDb2RootItem::newConnection()
{
  QgsDebugMsg( "DB2: Browser Panel; New Connection dialog requested." );
  QgsDb2NewConnection newConnection( NULL, mName );
  if ( newConnection.exec() )
  {
    refresh();
  }

}

// ---------------------------------------------------------------------------
QgsDb2LayerItem::QgsDb2LayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsDb2LayerProperty layerProperty )
    : QgsLayerItem( parent, name, path, QString(), layerType, PROVIDER_KEY )
    , mLayerProperty( layerProperty )
{
  QgsDebugMsg( "new db2 layer created : " + layerType );
  mUri = createUri();
  setState( Populated );
}

QgsDb2LayerItem::~QgsDb2LayerItem()
{

}

QgsDb2LayerItem* QgsDb2LayerItem::createClone()
{
  return new QgsDb2LayerItem( mParent, mName, mPath, mLayerType, mLayerProperty );
}

QString QgsDb2LayerItem::createUri()
{
  QString pkColName = mLayerProperty.pkCols.size() > 0 ? mLayerProperty.pkCols.at( 0 ) : QString::null;
  QgsDb2ConnectionItem *connItem = qobject_cast<QgsDb2ConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }

  QgsDataSourceURI uri = QgsDataSourceURI( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, pkColName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QgsDb2TableModel::wkbTypeFromDb2( mLayerProperty.type ) );
  QgsDebugMsg( "Layer URI: " + uri.uri() );
  return uri.uri();
}
// ---------------------------------------------------------------------------
QgsDb2SchemaItem::QgsDb2SchemaItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIconName = "mIconDbSchema.png";
}

QVector<QgsDataItem*> QgsDb2SchemaItem::createChildren()
{
  QgsDebugMsg( "schema this DB2 Entering." );

  QVector<QgsDataItem*>items;

  Q_FOREACH ( QgsDataItem *child, this->children() )
  {
    items.append((( QgsDb2LayerItem* )child )->createClone() );
  }
  return items;
}

QgsDb2SchemaItem::~QgsDb2SchemaItem()
{
}

void QgsDb2SchemaItem::addLayers( QgsDataItem* newLayers )
{
  // Add new items
  Q_FOREACH ( QgsDataItem *child, newLayers->children() )
  {
    // Is it present in childs?
    if ( findItem( mChildren, child ) >= 0 )
    {
      continue;
    }
    QgsDb2LayerItem* layer = (( QgsDb2LayerItem* )child )->createClone();
    addChildItem( layer, true );
  }
}

QgsDb2LayerItem* QgsDb2SchemaItem::addLayer( QgsDb2LayerProperty layerProperty, bool refresh )
{
  QGis::WkbType wkbType = QgsDb2TableModel::wkbTypeFromDb2( layerProperty.type );
  QString tip = tr( "DB2 *** %1 as %2 in %3" ).arg( layerProperty.geometryColName ).arg( QgsDb2TableModel::displayStringForWkbType( wkbType ) ).arg( layerProperty.srid );
  QgsDebugMsg( tip );
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
        return NULL;
      }
  }

  QgsDb2LayerItem *layerItem = new QgsDb2LayerItem( this, layerProperty.tableName, mPath + "/" + layerProperty.tableName, layerType, layerProperty );
  layerItem->setToolTip( tip );
  if ( refresh )
    addChildItem( layerItem, true );
  else
    addChild( layerItem );

  return layerItem;
}