/***************************************************************************
    qgsspatialitedataitems.cpp
    ---------------------
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
#include "qgsspatialitedataitems.h"

#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnection.h"

#ifdef HAVE_GUI
#include "qgsspatialitesourceselect.h"
#endif

#include "qgslogger.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerexporter.h"
#include "qgsmessageoutput.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

QGISEXTERN bool deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause );
static QgsLayerItem::LayerType _layerTypeFromDb( const QString &dbType )
{
  if ( dbType.startsWith( QLatin1String( "POINT" ) ) || dbType.startsWith( QLatin1String( "MULTIPOINT" ) ) )
  {
    return QgsLayerItem::Point;
  }
  else if ( dbType.startsWith( QLatin1String( "LINESTRING" ) ) || dbType.startsWith( QLatin1String( "MULTILINESTRING" ) ) )
  {
    return QgsLayerItem::Line;
  }
  else if ( dbType.startsWith( QLatin1String( "POLYGON" ) ) || dbType.startsWith( QLatin1String( "MULTIPOLYGON" ) ) )
  {
    return QgsLayerItem::Polygon;
  }
  else if ( dbType == QLatin1String( "qgis_table" ) )
  {
    return QgsLayerItem::Table;
  }
  else
  {
    return QgsLayerItem::NoType;
  }
}

QgsSLLayerItem::QgsSLLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, LayerType layerType )
  : QgsLayerItem( parent, name, path, uri, layerType, QStringLiteral( "spatialite" ) ), mSpatialiteDbInfo( nullptr )
{
  setState( Populated ); // no children are expected
}
QgsSLLayerItem::QgsSLLayerItem( QgsDataItem *parent, QString path, QString sLayerName, QString sLayerInfo, SpatialiteDbInfo *spatialiteDbInfo )
  : QgsLayerItem( parent, "", "", "", QgsLayerItem::Point, QStringLiteral( "spatialite" ) ), mSpatialiteDbInfo( spatialiteDbInfo )
{
  setState( Populated ); // no children are expected
  if ( mSpatialiteDbInfo )
  {
    QString sTableName  = sLayerName;
    QString sGeometryColumn = QString::null;
    mUri = mSpatialiteDbInfo->getDbLayerUris( sLayerName );
    if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
    {
      // Extract GeometryName from sent 'table_name(field_name)' from layerName
      QStringList sa_list_name = sTableName.split( "(" );
      sGeometryColumn = sa_list_name[1].replace( ")", "" );
      sTableName = sa_list_name[0];
    }
    //  GeometryType and Srid formatted as 'geometry_type:srid'
    if ( sLayerInfo.contains( ";" ) )
    {
      QStringList sa_list_name = sTableName.split( ";" );
      sLayerInfo = sa_list_name[0];
    }
    mLayerType = _layerTypeFromDb( sLayerInfo );
    mName = sTableName;
    setPath( QString( "%1/%2" ).arg( path ).arg( sTableName ) );
  }

}

#ifdef HAVE_GUI
QList<QAction *> QgsSLLayerItem::actions()
{
  QList<QAction *> lst;

  QAction *actionDeleteLayer = new QAction( tr( "Delete Layer" ), this );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsSLLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );

  return lst;
}

void QgsSLLayerItem::deleteLayer()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Object" ),
                              QObject::tr( "Are you sure you want to delete %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsDataSourceUri uri( mUri );
  QString errCause;
  bool res = ::deleteLayer( uri.database(), uri.table(), errCause );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Delete Layer" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Delete Layer" ), tr( "Layer deleted successfully." ) );
    mParent->refresh();
  }
}
#endif

// ------

QgsSLConnectionItem::QgsSLConnectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mDbPath = QgsSpatiaLiteConnection::connectionPath( name );
  mToolTip = mDbPath;
  mCapabilities |= Collapse;
}

QVector<QgsDataItem *> QgsSLConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QgsSpatiaLiteConnection connectionInfo( mName );
  bool bLoadLayers = false;
  bool bShared = true;
  SpatialiteDbInfo *spatialiteDbInfo = connectionInfo.CreateSpatialiteConnection( QString(), bLoadLayers, bShared );
  QString msgDetails;
  QString msg;
  if ( !spatialiteDbInfo )
  {
    if ( !QFile::exists( connectionInfo.dbPath() ) )
    {
      msg = tr( "SpatiaLite DB Open Error" );
      msgDetails = tr( "Database does not exist: %1" ).arg( connectionInfo.dbPath() );
    }
    else
    {
      msg =  tr( "SpatiaLite DB Open Error" );
      msgDetails = tr( " File is not a Sqlite3 Container: %1" ).arg( connectionInfo.dbPath() );
    }
    children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
    return children;
    delete spatialiteDbInfo;
    spatialiteDbInfo = nullptr;
    return children;
  }
  else
  {
    if ( !spatialiteDbInfo->isDbValid() )
    {
      msg = tr( "SpatiaLite DB Open Error" );
      msgDetails = tr( "The read Sqlite3 Container is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider: %1" ).arg( connectionInfo.dbPath() );
      children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
      return children;
    }
    // populate the table list
    // get the list of suitable tables and columns and populate the UI
    // mTableModel.setSqliteDb( spatialiteDbInfo, cbxAllowGeometrylessTables->isChecked() );
    SpatialiteDbInfo::SpatialiteLayerType typeLayer = SpatialiteDbInfo::AllSpatialLayers; // SpatialiteDbInfo::AllLayers
    QMap<QString, QString> mapLayers = spatialiteDbInfo->getDbLayersType( typeLayer );
    for ( QMap<QString, QString>::iterator itLayers = mapLayers.begin(); itLayers != mapLayers.end(); ++itLayers )
    {
      QgsSLLayerItem *layer = new QgsSLLayerItem( this, mPath, itLayers.key(), itLayers.value(), spatialiteDbInfo );
      children.append( layer );
    }
  }
  return children;
}

bool QgsSLConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsSLConnectionItem *o = dynamic_cast<const QgsSLConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;
}

#ifdef HAVE_GUI
QList<QAction *> QgsSLConnectionItem::actions()
{
  QList<QAction *> lst;

  //QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  //connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  //lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, &QAction::triggered, this, &QgsSLConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsSLConnectionItem::editConnection()
{
}

void QgsSLConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsSpatiaLiteConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}
#endif

bool QgsSLConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc

  QgsDataSourceUri destUri;
  destUri.setDatabase( mDbPath );

  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  Q_FOREACH ( const QgsMimeDataUtils::Uri &u, lst )
  {
    // open the source layer
    bool owner;
    QString error;
    QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
    if ( !srcLayer )
    {
      importResults.append( tr( "%1: %2" ).arg( u.name ).arg( error ) );
      hasError = true;
      continue;
    }

    if ( srcLayer->isValid() )
    {
      destUri.setDataSource( QString(), u.name, srcLayer->geometryType() != QgsWkbTypes::NullGeometry ? QStringLiteral( "geom" ) : QString() );
      QgsDebugMsg( "URI " + destUri.uri() );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( new QgsVectorLayerExporterTask( srcLayer, destUri.uri(), QStringLiteral( "spatialite" ), srcLayer->crs(), QVariantMap(), owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to SpatiaLite database" ), tr( "Import was successful." ) );
        refresh();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [ = ]( int error, const QString & errorMessage )
      {
        if ( error != QgsVectorLayerExporter::ErrUserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to SpatiaLite database" ) );
          output->setMessage( tr( "Failed to import layer!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        refresh();
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
    output->setTitle( tr( "Import to SpatiaLite database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}


// ---------------------------------------------------------------------------

QgsSLRootItem::QgsSLRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconSpatialite.svg" );
  populate();
}

QVector<QgsDataItem *> QgsSLRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  Q_FOREACH ( const QString &connName, QgsSpatiaLiteConnection::connectionList() )
  {
    QgsDataItem *conn = new QgsSLConnectionItem( this, connName, mPath + '/' + connName );
    connections.push_back( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsSLRootItem::actions()
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsSLRootItem::newConnection );
  lst.append( actionNew );

  QAction *actionCreateDatabase = new QAction( tr( "Create Database..." ), this );
  connect( actionCreateDatabase, &QAction::triggered, this, &QgsSLRootItem::createDatabase );
  lst.append( actionCreateDatabase );

  return lst;
}

QWidget *QgsSLRootItem::paramWidget()
{
  QgsSpatiaLiteSourceSelect *select = new QgsSpatiaLiteSourceSelect( nullptr, 0, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsSpatiaLiteSourceSelect::connectionsChanged, this, &QgsSLRootItem::connectionsChanged );
  return select;
}

void QgsSLRootItem::connectionsChanged()
{
  refresh();
}

void QgsSLRootItem::newConnection()
{
  if ( QgsSpatiaLiteSourceSelect::newConnection( nullptr ) )
  {
    refresh();
  }
}
#endif
//-----------------------------------------------------------
// TODO: clean this up
// - not documented:
// used  for 'QgsSLRootItem::createDatabase()' and
// possibly for 'QgsNewSpatialiteLayerDialog::createDb'
// implemented in qgsspatialiteprovider.cpp
//-----------------------------------------------------------
QGISEXTERN bool createDb( const QString &dbPath, QString &errCause );

void QgsSLRootItem::createDatabase()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::homePath() ).toString();

  QString filename = QFileDialog::getSaveFileName( nullptr, tr( "New SpatiaLite Database File" ),
                     lastUsedDir,
                     tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)" );
  if ( filename.isEmpty() )
    return;

  QString errCause;
  if ( ::createDb( filename, errCause ) )
  {
    // add connection
    settings.setValue( "/SpatiaLite/connections/" + QFileInfo( filename ).fileName() + "/sqlitepath", filename );

    refresh();
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Create SpatiaLite database" ), tr( "Failed to create the database:\n" ) + errCause );
  }
}

// ---------------------------------------------------------------------------

#ifdef HAVE_GUI
QGISEXTERN QgsSpatiaLiteSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  // TODO: this should be somewhere else
  return new QgsSpatiaLiteSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  Q_UNUSED( path );
  return new QgsSLRootItem( parentItem, QStringLiteral( "SpatiaLite" ), QStringLiteral( "spatialite:" ) );
}
