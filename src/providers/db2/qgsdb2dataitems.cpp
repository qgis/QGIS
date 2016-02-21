/***************************************************************************
  qgsdb2dataitems.cpp - Browser Panel object population
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2dataitems.h"
#include "qgsdb2newconnection.h"
#include "qgsdb2geometrycolumns.h"
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

bool QgsDb2ConnectionItem::ConnInfoFromSettings(const QString connName,
                                            QString &connInfo, QString &errorMsg)
{
  QSettings settings;
  QString key = "/DB2/connections/" + connName;

  QString service = settings.value( key + "/service" ).toString();
  QString driver = settings.value( key + "/driver" ).toString();
  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  QString database = settings.value( key + "/database" ).toString();
  QString username = settings.value( key + "/username" ).toString();
  QString password = settings.value( key + "/password" ).toString();
  QString authcfg = settings.value( key + "/authcfg" ).toString(); 
   
  if ( service.isEmpty() )
  {
    if ( driver.isEmpty() || host.isEmpty() || database.isEmpty() || port.isEmpty() )
    {
      QgsDebugMsg( "Host, port, driver or database missing" );
      errorMsg = "Host, port, driver or database missing";
      return false;
    }
    connInfo = "driver='" + driver + "' "
             + "host='" + host + "' "
             + "dbname='" + database + "' "
             + "port='" + port + "' ";
  }
  else
  {
    if ( database.isEmpty() )
    {
      QgsDebugMsg( "Database must be specified" );
      errorMsg = "Database must be specified";      
      return false;
    }
    connInfo = "service='" + service + "' "
             + "dbname='" + database + "' ";
  }

  if ( !authcfg.isEmpty() )
  {
    username.clear();
    password.clear();
    connInfo += "authcfg='" + authcfg + "' ";
  }   
  
  if ( !username.isEmpty() )  // will be empty if !authcfg.isEmpty()
  {
    connInfo += "user='" + username + "' ";
  }
  
  
  if ( !password.isEmpty() )
  {
    connInfo += "password='" + password + "' ";
  }
  QgsDebugMsg("connInfo: '" + connInfo + "'");    
  return true;  
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
  
  QString connInfo;
  QString errorMsg;
  bool success = QgsDb2ConnectionItem::ConnInfoFromSettings(mName, connInfo, errorMsg);
  if ( !success ) 
  {
    QgsDebugMsg("settings error: " + errorMsg);
  }
  QgsDebugMsg("connInfo: " + connInfo);
  mConnInfo = connInfo;
    QgsDebugMsg("mConnInfo: '" + mConnInfo + "'");
  QSqlDatabase db = QgsDb2Provider::GetDatabase( connInfo );
  QgsDebugMsg( "back from GetDatabase" );
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
  QgsDb2GeometryColumns db2GC = QgsDb2GeometryColumns( db );
  int sqlcode = db2GC.open();

  /* Enabling the DB2 Spatial Extender creates the DB2GSE schema and tables,
     so the Extender is either not enabled or set up if SQLCODE -204 is returned. */
  if ( sqlcode == -204 )
  {
    children.append( new QgsErrorItem( this, tr( "DB2 Spatial Extender is not enabled or set up." ), mPath + "/error" ) );
    return children;
  }
  else if ( sqlcode != 0 )
  {
    children.append( new QgsErrorItem( this, db.lastError().text(), mPath + "/error" ) );
    return children;
  }
  QgsDb2LayerProperty layer;

  //QVector<QgsDataItem*> newLayers;
  while ( db2GC.populateLayerProperty( layer ) )
  {
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
      QgsDebugMsg( "Adding Schema Item : " + layer.schemaName + " " + mPath + "/" + layer.schemaName + " type=" + layer.type
                   + " srid=" + layer.srid + " table=" + layer.tableName + "(" + layer.geometryColName + ")"
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
  QSqlDatabase db = QgsDb2Provider::GetDatabase( mConnInfo );  
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
  QgsDb2ConnectionItem *connItem = qobject_cast<QgsDb2ConnectionItem *>( parent() ? parent()->parent() : 0 );

  if ( !connItem )
  {
    QgsDebugMsg( "connection item not found." );
    return QString::null;
  }
  QgsDebugMsg("connInfo: '" + connItem->connInfo() + "'");
  QgsDataSourceURI uri = QgsDataSourceURI( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, mLayerProperty.pkColumnName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QgsDb2TableModel::wkbTypeFromDb2( mLayerProperty.type ) );
  uri.setParam( "extents", mLayerProperty.extents );
  QString uriString = uri.uri(false);
    QgsDebugMsg( "Layer URI: " + uriString );
  return uriString;
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