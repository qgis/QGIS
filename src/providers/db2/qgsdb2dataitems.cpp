/***************************************************************************
  qgsdb2dataitems.cpp - Browser Panel object population
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
  Adapted from MSSQL provider by Tamas Szekeres
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
//#include <qaction.h>
#include "qgsmimedatautils.h"
#include "qgsvectorlayerimport.h"

#include <QSettings>
#include <QMessageBox>
//#include <QtSql/QSqlDatabase>
//#include <QtSql/QSqlError>
#include <QProgressDialog>

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

bool QgsDb2ConnectionItem::ConnInfoFromParameters(
  const QString &service,
  const QString &driver,
  const QString &host,
  const QString &port,
  const QString &database,
  const QString &username,
  const QString &password,
  const QString &authcfg,
  QString &connInfo,
  QString &errorMsg )
{
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
    connInfo += "authcfg='" + authcfg + "' ";
  }
  else  // include user and password if authcfg is empty
  {
    if ( !username.isEmpty() )
    {
      connInfo += "user='" + username + "' ";
    }

    if ( !password.isEmpty() )
    {
      connInfo += "password='" + password + "' ";
    }
  }
  QgsDebugMsg( "connInfo: '" + connInfo + "'" );
  return true;
}

bool QgsDb2ConnectionItem::ConnInfoFromSettings( const QString connName,
    QString &connInfo, QString &errorMsg )
{
  QgsDebugMsg( QString( "Get settings for connection '%1'" ).arg( connInfo ) );
  QSettings settings;
  QString key = "/DB2/connections/" + connName;

  bool rc = QgsDb2ConnectionItem::ConnInfoFromParameters(
              settings.value( key + "/service" ).toString(),
              settings.value( key + "/driver" ).toString(),
              settings.value( key + "/host" ).toString(),
              settings.value( key + "/port" ).toString(),
              settings.value( key + "/database" ).toString(),
              settings.value( key + "/username" ).toString(),
              settings.value( key + "/password" ).toString(),
              settings.value( key + "/authcfg" ).toString(),
              connInfo, errorMsg );

  if ( !rc )
  {
    QgsDebugMsg( "errMsg: " + errorMsg );
    return false;
  }

  QgsDebugMsg( "connInfo: '" + connInfo + "'" );
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
  QVector<QgsDataItem*> children;

  QString connInfo;
  QString errorMsg;
  bool success = QgsDb2ConnectionItem::ConnInfoFromSettings( mName, connInfo, errorMsg );
  if ( !success )
  {
    QgsDebugMsg( "settings error: " + errorMsg );
    children.append( new QgsErrorItem( this, errorMsg, mPath + "/error" ) );
    return children;
  }

  mConnInfo = connInfo;
  QgsDebugMsg( "mConnInfo: '" + mConnInfo + "'" );

  QSqlDatabase db = QgsDb2Provider::getDatabase( connInfo, errorMsg );
  if ( errorMsg.isEmpty() )
  {
    //children.append( new QgsFavouritesItem(this, "connection successful", mPath + "/success"));
    QgsDebugMsg( "DB open successful for connection " + db.connectionName() );
  }
  else
  {
    children.append( new QgsErrorItem( this, errorMsg, mPath + "/error" ) );
    QgsDebugMsg( "DB not open " + errorMsg );
    return children;
  }

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

bool QgsDb2ConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsDb2ConnectionItem *o = qobject_cast<const QgsDb2ConnectionItem *>( other );
  return ( mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsDb2ConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionRefresh = new QAction( tr( "Refresh connection" ), this );
  connect( actionRefresh, SIGNAL( triggered() ), this, SLOT( refreshConnection() ) );
  lst.append( actionRefresh );

  QAction* actionEdit = new QAction( tr( "Edit connection..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete connection" ), this );
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
  QString errMsg;
  QSqlDatabase db = QgsDb2Provider::getDatabase( mConnInfo, errMsg );
  Q_UNUSED( db );
  if ( errMsg.isEmpty() )
  {
    QgsDebugMsg( "successful get db2 connection on refresh" );
  }
  else
  {
    QgsDebugMsg( "failed get db2 connection on refresh " + errMsg + " " + mPath + "/error" );
  }
  refresh();
}


bool QgsDb2ConnectionItem::handleDrop( const QMimeData * data, Qt::DropAction )
{
  return handleDrop( data, QString() );
}

bool QgsDb2ConnectionItem::handleDrop( const QMimeData* data, const QString& toSchema )
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

    QgsDebugMsg( QString( "uri: %1; name: %2; key: %3" ).arg( u.uri, u.name, u.providerKey ) );
    // open the source layer
    QgsVectorLayer* srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey );

    if ( srcLayer->isValid() )
    {
      QString tableName;
      if ( !toSchema.isEmpty() )
      {
        tableName = QString( "%1.%2" ).arg( toSchema, u.name );
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
      err = QgsVectorLayerImport::importLayer( srcLayer, uri, "DB2", &srcLayer->crs(), false, &importError, false, nullptr, progress );
      if ( err == QgsVectorLayerImport::NoError )
      {
        importResults.append( tr( "%1: OK!" ).arg( u.name ) );
        QgsDebugMsg( "import successful" );
      }
      else
      {
        if ( err == QgsVectorLayerImport::ErrUserCancelled )
        {
          cancelled = true;
          QgsDebugMsg( "import cancelled" );
        }
        else
        {
          QString errMsg = QString( "%1: %2" ).arg( u.name, importError );
          QgsDebugMsg( "import failed: " + errMsg );
          importResults.append( errMsg );
          hasError = true;
        }
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
    QMessageBox::information( nullptr, tr( "Import to DB2 database" ), tr( "Import cancelled." ) );
    refresh();
  }
  else if ( hasError )
  {
    QMessageBox::warning( nullptr, tr( "Import to DB2 database" ), tr( "Failed to import some layers!\n\n" ) + importResults.join( "\n" ) );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Import to DB2 database" ), tr( "Import was successful." ) );
  }

  if ( state() == Populated )
    refresh();
  else
    populate();

  return true;
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
  return NULL;
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
  QgsDebugMsg( QString( "new db2 layer created : %1" ).arg( layerType ) );
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
  QgsDebugMsg( "connInfo: '" + connItem->connInfo() + "'" );
  QgsDataSourceURI uri = QgsDataSourceURI( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, mLayerProperty.pkColumnName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QGis::fromOldWkbType( QgsDb2TableModel::wkbTypeFromDb2( mLayerProperty.type ) ) );
  uri.setParam( "extents", mLayerProperty.extents );
  QString uriString = uri.uri( false );
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

bool QgsDb2SchemaItem::handleDrop( const QMimeData* data, Qt::DropAction )
{
  QgsDb2ConnectionItem *conn = qobject_cast<QgsDb2ConnectionItem *>( parent() );
  if ( !conn )
    return 0;

  return conn->handleDrop( data, mName );
}

QgsDb2LayerItem* QgsDb2SchemaItem::addLayer( QgsDb2LayerProperty layerProperty, bool refresh )
{
  QGis::WkbType wkbType = QgsDb2TableModel::wkbTypeFromDb2( layerProperty.type );
  QString tip = tr( "DB2 *** %1 as %2 in %3" ).arg( layerProperty.geometryColName,
                QgsDb2TableModel::displayStringForWkbType( wkbType ),
                layerProperty.srid );
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
