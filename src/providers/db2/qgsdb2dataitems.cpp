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
#include "qgsdb2geometrycolumns.h"
#include "qgslogger.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerexporter.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsmessageoutput.h"
#include "qgsapplication.h"

#ifdef HAVE_GUI
#include "qgsdb2newconnection.h"
#include "qgsdb2sourceselect.h"
#endif

#include <QMessageBox>
#include <QProgressDialog>

static const QString PROVIDER_KEY = QStringLiteral( "DB2" );

QgsDb2ConnectionItem::QgsDb2ConnectionItem( QgsDataItem *parent, const QString name, const QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Collapse;
  populate();
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
      QgsDebugMsg( QStringLiteral( "Host, port, driver or database missing" ) );
      errorMsg = QStringLiteral( "Host, port, driver or database missing" );
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
      QgsDebugMsg( QStringLiteral( "Database must be specified" ) );
      errorMsg = QStringLiteral( "Database must be specified" );
      return false;
    }
    connInfo = "service='" + service + "' "
               + "dbname='" + database + "' ";
  }

  if ( !authcfg.isEmpty() )
  {
    connInfo += "authcfg='" + authcfg + "' ";
  }

  if ( !password.isEmpty() )
  {
    // include password if authcfg is empty
    connInfo += "password='" + password + "' ";
  }

  if ( !username.isEmpty() )
  {
    connInfo += "user='" + username + "' ";
  }

  QgsDebugMsg( "connInfo: '" + connInfo + "'" );
  return true;
}

bool QgsDb2ConnectionItem::ConnInfoFromSettings( const QString connName,
    QString &connInfo, QString &errorMsg )
{
  QgsDebugMsg( QStringLiteral( "Get settings for connection '%1'" ).arg( connInfo ) );
  QgsSettings settings;
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
  QVector<QgsDataItem *> items = createChildren();

  // Add new items
  Q_FOREACH ( QgsDataItem *item, items )
  {
    // Is it present in children?
    int index = findItem( mChildren, item );
    if ( index >= 0 )
    {
      ( ( QgsDb2SchemaItem * )mChildren.at( index ) )->addLayers( item );
      delete item;
      continue;
    }
    addChildItem( item, true );
  }
}

QVector<QgsDataItem *> QgsDb2ConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

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
  QString sqlcode = db2GC.open();

  /* Enabling the DB2 Spatial Extender creates the DB2GSE schema and tables,
     so the Extender is either not enabled or set up if SQLCODE -204 is returned. */
  if ( sqlcode == QStringLiteral( "-204" ) )
  {
    children.append( new QgsErrorItem( this, tr( "DB2 Spatial Extender is not enabled or set up." ), mPath + "/error" ) );
    return children;
  }
  else if ( !sqlcode.isEmpty() && sqlcode != QStringLiteral( "0" ) )
  {
    children.append( new QgsErrorItem( this, db.lastError().text(), mPath + "/error" ) );
    return children;
  }
  QgsDb2LayerProperty layer;

  //QVector<QgsDataItem*> newLayers;
  while ( db2GC.populateLayerProperty( layer ) )
  {
    QgsDb2SchemaItem *schemaItem = nullptr;
    Q_FOREACH ( QgsDataItem *child, children )
    {
      if ( child->name() == layer.schemaName )
      {
        schemaItem = ( QgsDb2SchemaItem * )child;
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

    QgsDb2LayerItem *added = schemaItem->addLayer( layer, true );

    if ( added )
    {
      QgsDebugMsg( " DB2 adding layer to schema item: " + added->name() );
      //children.append(added);
    }
    else
    {
      QgsDebugMsg( QStringLiteral( " DB2 layer not added " ) );
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

#ifdef HAVE_GUI
QList<QAction *> QgsDb2ConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionRefresh = new QAction( tr( "Refresh Connection" ), parent );
  connect( actionRefresh, &QAction::triggered, this, &QgsDb2ConnectionItem::refreshConnection );
  lst.append( actionRefresh );

  QAction *actionEdit = new QAction( tr( "Edit Connection…" ), parent );
  connect( actionEdit, &QAction::triggered, this, &QgsDb2ConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete Connection" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsDb2ConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsDb2ConnectionItem::editConnection()
{
  QgsDb2NewConnection nc( nullptr, mName );
  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refreshConnections();
  }
}

void QgsDb2ConnectionItem::deleteConnection()
{
  QString key = "/DB2/connections/" + mName;
  QgsSettings settings;
  settings.remove( key + "/service" );
  settings.remove( key + "/driver" );
  settings.remove( key + "/port" );
  settings.remove( key + "/host" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/environment" );
  settings.remove( key );
  mParent->refreshConnections();
}

void QgsDb2ConnectionItem::refreshConnection()
{
  QString errMsg;
  QSqlDatabase db = QgsDb2Provider::getDatabase( mConnInfo, errMsg );
  Q_UNUSED( db );
  if ( errMsg.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "successful get db2 connection on refresh" ) );
  }
  else
  {
    QgsDebugMsg( "failed get db2 connection on refresh " + errMsg + " " + mPath + "/error" );
  }
  refresh();
}
#endif


bool QgsDb2ConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  return handleDrop( data, QString() );
}

bool QgsDb2ConnectionItem::handleDrop( const QMimeData *data, const QString &toSchema )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc
  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Q_FOREACH ( const QgsMimeDataUtils::Uri &u, lst )
  {
    if ( u.layerType != QLatin1String( "vector" ) )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    QgsDebugMsg( QStringLiteral( "uri: %1; name: %2; key: %3" ).arg( u.uri, u.name, u.providerKey ) );
    // open the source layer
    QgsVectorLayer *srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey );

    if ( srcLayer->isValid() )
    {
      QString tableName;
      if ( !toSchema.isEmpty() )
      {
        tableName = QStringLiteral( "%1.%2" ).arg( toSchema, u.name );
      }
      else
      {
        tableName = u.name;
      }

      QString uri = connInfo() + " table=" + tableName;
      if ( srcLayer->geometryType() != QgsWkbTypes::NullGeometry )
        uri += QLatin1String( " (geom)" );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( QgsVectorLayerExporterTask::withLayerOwnership( srcLayer, uri, QStringLiteral( "DB2" ), srcLayer->crs() ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to DB2 database" ), tr( "Import was successful." ) );
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
          output->setTitle( tr( "Import to DB2 database" ) );
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
    output->setTitle( tr( "Import to DB2 database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

QgsDb2RootItem::QgsDb2RootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconDb2.svg" );
  populate();
}

QVector<QgsDataItem *> QgsDb2RootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/DB2/connections" ) );
  Q_FOREACH ( const QString &connName, settings.childGroups() )
  {
    connections << new QgsDb2ConnectionItem( this, connName, mPath + "/" + connName );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsDb2RootItem::actions( QWidget *parent )
{
  QList<QAction *> actionList;

  QAction *action = new QAction( tr( "New Connection…" ), parent );
  connect( action, &QAction::triggered, this, &QgsDb2RootItem::newConnection );
  actionList.append( action );

  return actionList;
}

QWidget *QgsDb2RootItem::paramWidget()
{
  return nullptr;
}

void QgsDb2RootItem::newConnection()
{
  QgsDebugMsg( QStringLiteral( "DB2: Browser Panel; New Connection dialog requested." ) );
  QgsDb2NewConnection newConnection( nullptr, mName );
  if ( newConnection.exec() )
  {
    refreshConnections();
  }

}
#endif

// ---------------------------------------------------------------------------
QgsDb2LayerItem::QgsDb2LayerItem( QgsDataItem *parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsDb2LayerProperty layerProperty )
  : QgsLayerItem( parent, name, path, QString(), layerType, PROVIDER_KEY )
  , mLayerProperty( layerProperty )
{
  QgsDebugMsg( QStringLiteral( "new db2 layer created : %1" ).arg( layerType ) );
  mUri = createUri();
  setState( Populated );
}

QgsDb2LayerItem *QgsDb2LayerItem::createClone()
{
  return new QgsDb2LayerItem( mParent, mName, mPath, mLayerType, mLayerProperty );
}

QString QgsDb2LayerItem::createUri()
{
  QgsDb2ConnectionItem *connItem = qobject_cast<QgsDb2ConnectionItem *>( parent() ? parent()->parent() : nullptr );

  if ( !connItem )
  {
    QgsDebugMsg( QStringLiteral( "connection item not found." ) );
    return QString();
  }
  QgsDebugMsg( "connInfo: '" + connItem->connInfo() + "'" );
  QgsDataSourceUri uri = QgsDataSourceUri( connItem->connInfo() );
  uri.setDataSource( mLayerProperty.schemaName, mLayerProperty.tableName, mLayerProperty.geometryColName, mLayerProperty.sql, mLayerProperty.pkColumnName );
  uri.setSrid( mLayerProperty.srid );
  uri.setWkbType( QgsDb2TableModel::wkbTypeFromDb2( mLayerProperty.type ) );
  uri.setParam( QStringLiteral( "extents" ), mLayerProperty.extents );
  QString uriString = uri.uri( false );
  QgsDebugMsg( "Layer URI: " + uriString );
  return uriString;
}
// ---------------------------------------------------------------------------
QgsDb2SchemaItem::QgsDb2SchemaItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconDbSchema.svg" );
}

QVector<QgsDataItem *> QgsDb2SchemaItem::createChildren()
{
  QgsDebugMsg( QStringLiteral( "schema this DB2 Entering." ) );

  QVector<QgsDataItem *>items;

  Q_FOREACH ( QgsDataItem *child, this->children() )
  {
    items.append( ( ( QgsDb2LayerItem * )child )->createClone() );
  }
  return items;
}

void QgsDb2SchemaItem::addLayers( QgsDataItem *newLayers )
{
  // Add new items
  Q_FOREACH ( QgsDataItem *child, newLayers->children() )
  {
    // Is it present in children?
    if ( findItem( mChildren, child ) >= 0 )
    {
      continue;
    }
    QgsDb2LayerItem *layer = ( ( QgsDb2LayerItem * )child )->createClone();
    addChildItem( layer, true );
  }
}

bool QgsDb2SchemaItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  QgsDb2ConnectionItem *conn = qobject_cast<QgsDb2ConnectionItem *>( parent() );
  if ( !conn )
    return false;

  return conn->handleDrop( data, mName );
}

QgsDb2LayerItem *QgsDb2SchemaItem::addLayer( QgsDb2LayerProperty layerProperty, bool refresh )
{
  QgsWkbTypes::Type wkbType = QgsDb2TableModel::wkbTypeFromDb2( layerProperty.type );
  QString tip = tr( "DB2 *** %1 as %2 in %3" ).arg( layerProperty.geometryColName,
                QgsWkbTypes::displayString( wkbType ),
                layerProperty.srid );
  QgsDebugMsg( tip );
  QgsLayerItem::LayerType layerType;
  switch ( wkbType )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      layerType = QgsLayerItem::Point;
      break;
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
      layerType = QgsLayerItem::Line;
      break;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      if ( layerProperty.type == QLatin1String( "NONE" ) && layerProperty.geometryColName.isEmpty() )
      {
        layerType = QgsLayerItem::TableLayer;
        tip = tr( "as geometryless table" );
      }
      else
      {
        return nullptr;
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
