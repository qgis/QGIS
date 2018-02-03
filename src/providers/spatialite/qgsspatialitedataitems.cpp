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
//-----------------------------------------------------------------
// QGISEXTERN deleteLayer [Required Provider function]
//-----------------------------------------------------------------
QGISEXTERN bool deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause );
//-----------------------------------------------------------------
// QGISEXTERN dataCapabilities [Required Provider function]
//-----------------------------------------------------------------
//-Sqlite3 is a file based Container
//-----------------------------------------------------------------
QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::File; //  | QgsDataProvider::Database;
}
//-----------------------------------------------------------------
// dataItem [called from QgsBrowserDockWidget]
//-----------------------------------------------------------------
// Notes:
// 'path' is empty: call QgsSpatialiteRootItem to fill with QgsSpatiaLiteConnection::connectionList
// 'path' is not empty: QgsSpatialiteDbInfo is called in SniffMinimal modus
// - will retrieve basic information about the file
//-- is Sqlite3-Container, contains Layers supported by Providers [bLoadLayers = false]
// --> will end swiftly if false
// --> otherwise will retrieve minimal information to display
// - Connections: 1 per Database
// -- due to Spatialite Max-Connections=64 limit, open with 'bShared = false'
// --> do that the Connection will be closed and freed after reading
// - Layers=1: call QgsSpatialiteLayerItem
// - Layers>1: call QgsSpatialiteCollectionItem
// - Layer-Types not supported by QgsSpatiaLiteProvider or QgsRasterLite2Provider will be  ignored
// --> 'bLoadGdalOgr=false', to avoid double entries in QBrowser
//-----------------------------------------------------------------
// The Ogr and Gdal versions of 'dataItem'
// - should be adapted to prevent the scanning of files suppoted by QgsSpatiaLiteProvider or QgsRasterLite2Provider
// --> to avoid double entries in QBrowser. Code sample:
//-----------------------------------------------------------------
//  CPLPushErrorHandler( CPLQuietErrorHandler );
//  CPLErrorReset();
//  GDALDriverH hDriver = GDALIdentifyDriver( path.toUtf8().constData(), nullptr );
//  CPLPopErrorHandler();
//  QString driverName = GDALGetDriverShortName( hDriver );
//  if ( driverName == QLatin1String( "SQLite" ) )
//  {
//    // SQLite/SpatiaLite' do not load Spatialite formats  [Geometries, RasterLite2]
//    QgsDebugMsgLevel( "Skipping SQLite file because QgsSpatialiteLayerItem will deal with it", 2 );
//    return nullptr;
//  }
//-----------------------------------------------------------------
QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  QgsDataItem *item = nullptr;
  if ( path.isEmpty() )
  {
    return new QgsSpatialiteRootItem( parentItem, QStringLiteral( "SpatiaLite 5.0" ), QStringLiteral( "spatialite:" ) );
    return item;
  }
  bool bLoadLayers = false; // Minimal information. extended information will be retrieved only when needed
  bool bShared = false; // do not retain connection, connection will always be closed after usage
  QgsSpatialiteDbInfo *spatialiteDbInfo = QgsSpatiaLiteUtils::CreateQgsSpatialiteDbInfo( path, bLoadLayers, bShared );
  if ( spatialiteDbInfo )
  {
    // The file exists, check for supported Sqlite3-Container formats
    if ( ( spatialiteDbInfo->isDbSpatialite() ) || ( spatialiteDbInfo->isDbGdalOgr() ) )
    {
      // The read Sqlite3 Container is supported by QgsSpatiaLiteProvider, QRasterLite2Provider,QgsOgrProvider or QgsGdalProvider.
      bool bLoadGdalOgr = true;
      // QgsSettings settings;
      // QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanLoadGdalOgr" ), "basic" ).toString();
      if ( spatialiteDbInfo->isDbGdalOgr() )
      {
#if 1
        // QgsSpatiaLiteSourceSelect/QgsSpatialiteLayerItem can display Gdal/Ogr sqlite3 based Sources (GeoPackage, MbTiles, FDO etc.)
        //  and when selected call the needed QgsOgrProvider or QgsGdalProvider
        // - if not desired (possible User-Setting), then this can be prevented here
        QString sSpatialMetadata = spatialiteDbInfo->dbSpatialMetadataString();
        QgsDebugMsgLevel( QString( "Reading a Gdal/Ogr supported SQLite file. SpatialMetadata[%1]" ).arg( sSpatialMetadata ), 7 );
#else
        bLoadGdalOgr = false;
#endif
      }
      // If a Spatialite/RasterLite2
      // -, or when QgsOgrProvide/QgsGdalProvider should be read
      // and at least 1 valid Layer
      if ( ( ( spatialiteDbInfo->isDbSpatialite() ) || ( bLoadGdalOgr ) ) && ( spatialiteDbInfo->dbLayersCount() > 0 ) )
      {
        // Avoid re-reading the Database for each Layer
        spatialiteDbInfo->setConnectionShared( true );
        if ( spatialiteDbInfo->dbLayersCount() > 1 )
        {
          item = new QgsSpatialiteCollectionItem( parentItem, spatialiteDbInfo->getFileName(), spatialiteDbInfo->getDatabaseFileName(), spatialiteDbInfo );
        }
        else
        {
          item = new QgsSpatialiteLayerItem( parentItem, spatialiteDbInfo->getDatabaseFileName(), QString(), spatialiteDbInfo );
        }
        // Avoid a (possibly) gigantic amount of shared connection that may never be needed
        spatialiteDbInfo->setConnectionShared( false );
      }
    }
    if ( !item )
    {
      if ( !spatialiteDbInfo->checkConnectionNeeded() )
      {
        // Delete only if not being used elsewhere, Connection will be closed
        delete spatialiteDbInfo;
      }
      spatialiteDbInfo = nullptr;
    }
  }
  return item;
}
//-----------------------------------------------------------------
// QgsSpatialiteLayerItem
//-----------------------------------------------------------------
// Will be called from Provider specific 'dataItem'
// - when only 1 Layer exists
// Will be called from 'QgsSpatialiteCollectionItem'
// -> in 'createChildren' for each single Layer
//-----------------------------------------------------------------
// Notes:
// Metadata [QgsBrowserDockWidget::showProperties()]
// - QgsRasterLayer/QgsVectorLayer->htmlMetadata()
//-----------------------------------------------------------------
QgsSpatialiteLayerItem::QgsSpatialiteLayerItem( QgsDataItem *parent, QString filePath, QString sLayerName, QgsSpatialiteDbInfo *spatialiteDbInfo )
  : QgsLayerItem( parent, "", filePath, "", QgsLayerItem::Point, QStringLiteral( "spatialite45" ) ), mSpatialiteDbInfo( spatialiteDbInfo )
{
  mLayerType = QgsLayerItem::NoType;
  if ( path() == QStringLiteral( "spatialite:" ) )
  {
    QgsDebugMsgLevel( QString( "QgsSpatialiteLayerItem::QgsSpatialiteLayerItem [Root Connection] path[%1]" ).arg( path() ), 7 );
  }
  else
  {
    if ( !mSpatialiteDbInfo )
    {
      bool bLoadLayers = false; // Minimal information. extended information will be retrieved only when needed
      bool bShared = false; // do not retain connection, connection will always be closed after usage
      mSpatialiteDbInfo = QgsSpatiaLiteUtils::CreateQgsSpatialiteDbInfo( path(), bLoadLayers, bShared );
      if ( mSpatialiteDbInfo )
      {
        if ( ( mSpatialiteDbInfo->isDbSpatialite() ) || ( mSpatialiteDbInfo->isDbGdalOgr() ) )
        {
          setName( mSpatialiteDbInfo->getFileName() );
          setPath( mSpatialiteDbInfo->getDatabaseFileName() ); // Make sure that the absolute File-Path is being used
        }
      }
      else
      {
        return;
      }
    }
    if ( sLayerName.isEmpty() )
    {
      QList<QString> sa_list = mSpatialiteDbInfo->getDataSourceUris().keys();
      if ( sa_list.size() )
      {
        // this is a Drag and Drop action, load only first layer
        sLayerName = sa_list.at( 0 );
      }
    }
    setName( sLayerName );
    mLayerInfo = mSpatialiteDbInfo->getDbLayerInfo( name() );
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    // Extract TableName/GeometryColumn from sent 'table_name' or 'table_name(field_name)' from LayerName
    QgsSpatialiteDbInfo::parseLayerName( name(), sTableName, sGeometryColumn );
    QString sGeometryType; // For Layers that contain no Geometries, this will be the Layer-Type
    QString sLayerTypeSpatialite;
    int iSpatialIndex = -1;
    int iIsHidden = 0;
    if ( mSpatialiteDbInfo->parseLayerInfo( mLayerInfo, sGeometryType, mLayerSrid, mProviderKey, sLayerTypeSpatialite, iSpatialIndex, iIsHidden ) )
    {
      QString sSpatialMetadata = QString( "%1,%2" ).arg( mSpatialiteDbInfo->dbSpatialMetadataString() ).arg( sLayerTypeSpatialite );
      mUri = mSpatialiteDbInfo->getDbLayerUris( name() );
      mLayerTypeSpatialite = QgsSpatialiteDbInfo::SpatialiteLayerTypeFromName( sLayerTypeSpatialite );
      mLayerTypeIcon = QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( mLayerTypeSpatialite );
      mGeometryType = QgsWkbTypes::parseType( sGeometryType );
      QgsWkbTypes::Type singleType = QgsWkbTypes::singleType( QgsWkbTypes::flatType( mGeometryType ) );
      switch ( singleType )
      {
        case QgsWkbTypes::Point:
          mLayerType = QgsLayerItem::Point;
          mGeometryTypeIcon = QgsSpatialiteDbInfo::SpatialGeometryTypeIcon( mGeometryType );
          break;
        case QgsWkbTypes::LineString:
          mLayerType = QgsLayerItem::Line;
          mGeometryTypeIcon = QgsSpatialiteDbInfo::SpatialGeometryTypeIcon( mGeometryType );
          break;
        case QgsWkbTypes::Polygon:
          mLayerType = QgsLayerItem::Polygon;
          mGeometryTypeIcon = QgsSpatialiteDbInfo::SpatialGeometryTypeIcon( mGeometryType );
          break;
        default:
          // RasterLayers  will not contain a valid Geometry-Type
          mGeometryTypeIcon = mLayerTypeIcon;
          switch ( mLayerTypeSpatialite )
          {
            case QgsSpatialiteDbInfo::RasterLite2Raster:
            case QgsSpatialiteDbInfo::GeoPackageRaster:
            case QgsSpatialiteDbInfo::MBTilesTable:
            case QgsSpatialiteDbInfo::MBTilesView:
            {
              mLayerType = QgsLayerItem::Raster;
              mGeometryTypeIcon =  QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sLayerTypeSpatialite );
            }
            break;
            case QgsSpatialiteDbInfo::SpatialiteTopology:
            case QgsSpatialiteDbInfo::RasterLite2Vector:
            {
              // This should never happen, since they contain a Geometry-Type
              mLayerType = QgsLayerItem::Vector;
            }
            break;
            default:
              mLayerType = QgsLayerItem::NoType;
              break;
          }
          break;
      }
      mIconName = iconName( mLayerType );
      // Either a GeometryType or LayerType icon
      mIcon = mGeometryTypeIcon;
    }
    else
    {
      QgsDebugMsgLevel( QString( "Invalid LayerInfo: name[%1] Metadata[%2] LayerInfo[%3] FileName[%4]" ).arg( name() ).arg( mSpatialiteDbInfo->dbSpatialMetadataString() ).arg( mLayerInfo ).arg( mSpatialiteDbInfo->getFileName() ), 4 );
      return;
    }
    setPath( QString( "%1/%2" ).arg( path() ).arg( name() ) );
  }
  setState( Populated ); // no children are expected
}
#ifdef HAVE_GUI
//-----------------------------------------------------------------
// QgsSpatialiteLayerItem::actions
//-----------------------------------------------------------------
QList<QAction *> QgsSpatialiteLayerItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionDeleteLayer = new QAction( tr( "Delete Layer" ), parent );
  connect( actionDeleteLayer, &QAction::triggered, this, &QgsSpatialiteLayerItem::deleteLayer );
  lst.append( actionDeleteLayer );

  return lst;
}
//-----------------------------------------------------------------
// QgsSpatialiteLayerItem::deleteLayer
//-----------------------------------------------------------------
void QgsSpatialiteLayerItem::deleteLayer()
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
//-----------------------------------------------------------------
//  QgsSpatialiteCollectionItem
//-----------------------------------------------------------------
// Will be called from Provider specific 'dataItem'
// - when more than 1 Layer exists
// -> 'createChildren' will call QgsSpatialiteLayerItem
// --> for each single Layer
//-----------------------------------------------------------------
QgsSpatialiteCollectionItem::QgsSpatialiteCollectionItem( QgsDataItem *parent, const QString name, const QString filePath, QgsSpatialiteDbInfo *spatialiteDbInfo )
  : QgsDataCollectionItem( parent, name, filePath ), mSpatialiteDbInfo( spatialiteDbInfo )
{
  mToolTip = name;
  mCapabilities |= Collapse;
}
//-----------------------------------------------------------------
// QgsSpatialiteCollectionItem::createChildren
//-----------------------------------------------------------------
// Will call QgsSpatialiteLayerItem
// --> for each single Layer
//-----------------------------------------------------------------
QVector<QgsDataItem *> QgsSpatialiteCollectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QString msgDetails;
  QString msg;
  if ( !mSpatialiteDbInfo )
  {
    if ( !QFile::exists( path() ) )
    {
      msg = tr( "SpatiaLite DB Open Error" );
      msgDetails = tr( "Database does not exist: %1" ).arg( path() );
    }
    else
    {
      msg =  tr( "SpatiaLite DB Open Error" );
      msgDetails = tr( " File is not a Sqlite3 Container: %1" ).arg( path() );
    }
    children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
    return children;
    mSpatialiteDbInfo = nullptr;
    return children;
  }
  else
  {
    if ( !mSpatialiteDbInfo->isDbValid() )
    {
      msg = tr( "SpatiaLite DB Open Error" );
      msgDetails = tr( "The read Sqlite3 Container [%2] is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider: %1" ).arg( path() ).arg( mSpatialiteDbInfo->dbSpatialMetadataString() );
      children.append( new QgsErrorItem( this, msg, mPath + "/error" ) );
      return children;
    }
    // populate the table list
    // get the list of suitable tables and columns and populate the UI
    // mTableModel.setSqliteDb( spatialiteDbInfo, cbxAllowGeometrylessTables->isChecked() );
    //               i_removed_count += mVectorLayers.remove( sLayerMetadata );
    //               i_removed_count += mVectorLayersTypes.remove( sLayerMetadata );
    QgsSpatialiteDbInfo::SpatialiteLayerType typeLayer = QgsSpatialiteDbInfo::AllSpatialLayers;
    QMap<QString, QString> mapLayers = mSpatialiteDbInfo->getDbLayersType( typeLayer );
    for ( QMap<QString, QString>::iterator itLayers = mapLayers.begin(); itLayers != mapLayers.end(); ++itLayers )
    {
      QgsSpatialiteLayerItem *layer = new QgsSpatialiteLayerItem( this, path(), itLayers.key(), mSpatialiteDbInfo );
      children.append( layer );
    }
  }
  return children;
}
//-----------------------------------------------------------------
// QgsSpatialiteCollectionItem::equal
//-----------------------------------------------------------------
bool QgsSpatialiteCollectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsSpatialiteCollectionItem *o = dynamic_cast<const QgsSpatialiteCollectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;
}
#ifdef HAVE_GUI
//-----------------------------------------------------------------
// QgsSpatialiteCollectionItem:::actions
//-----------------------------------------------------------------
// At the moment, this does nothing
//-----------------------------------------------------------------
QList<QAction *> QgsSpatialiteCollectionItem::actions( QWidget *parent )
{
  Q_UNUSED( parent );
  QList<QAction *> lst;

  //QAction* actionEdit = new QAction( tr( "Edit..." ), parent );
  //connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  //lst.append( actionEdit );
#if 0
  QAction *actionDelete = new QAction( tr( "Delete" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsSpatialiteCollectionItem::deleteConnection );
  lst.append( actionDelete );
#endif

  return lst;
}
#endif
//-----------------------------------------------------------------
//  QgsSpatialiteConnectionItem
//-----------------------------------------------------------------
// Will be called from QgsSpatialiteRootItem::createChildren
// - for each connection entry found
//-----------------------------------------------------------------
QgsSpatialiteConnectionItem::QgsSpatialiteConnectionItem( QgsDataItem *parent, const QString name, const QString path )
  : QgsDataCollectionItem( parent, name, path )
{

}
#ifdef HAVE_GUI
//-----------------------------------------------------------------
//  QgsSpatialiteConnectionItem::actions
//-----------------------------------------------------------------
QList<QAction *> QgsSpatialiteConnectionItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  //QAction* actionEdit = new QAction( tr( "Edit..." ), parent );
  //connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  //lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), parent );
  connect( actionDelete, &QAction::triggered, this, &QgsSpatialiteConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}
//-----------------------------------------------------------------
//  QgsSpatialiteConnectionItem::equal
//-----------------------------------------------------------------
bool QgsSpatialiteConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsSpatialiteConnectionItem *o = dynamic_cast<const QgsSpatialiteConnectionItem *>( other );
  return o && mPath == o->mPath && mName == o->mName;
}
//-----------------------------------------------------------------
//  QgsSpatialiteConnectionItem::editConnection
//-----------------------------------------------------------------
void QgsSpatialiteConnectionItem::editConnection()
{
}
//-----------------------------------------------------------------
//  QgsSpatialiteConnectionItem::deleteConnection
//-----------------------------------------------------------------
void QgsSpatialiteConnectionItem::deleteConnection()
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( mName ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsSpatiaLiteConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refreshConnections();
}
#endif
//-----------------------------------------------------------------
//  QgsSpatialiteConnectionItem::handleDrop
//-----------------------------------------------------------------
bool QgsSpatialiteConnectionItem::handleDrop( const QMimeData *data, Qt::DropAction )
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
      importResults.append( tr( "%1: %2" ).arg( u.name, error ) );
      hasError = true;
      continue;
    }

    if ( srcLayer->isValid() )
    {
      destUri.setDataSource( QString(), u.name, srcLayer->geometryType() != QgsWkbTypes::NullGeometry ? QStringLiteral( "geom" ) : QString() );
      QgsDebugMsgLevel( QString( "URI %1" ).arg( destUri.uri() ), 3 );

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
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem
//-----------------------------------------------------------------
// Will be called from Provider specific 'dataItem'
// - when 'path' is empty
// Willl call QgsSpatialiteConnectionItem
// --> for each connection entry found
//-----------------------------------------------------------------
QgsSpatialiteRootItem::QgsSpatialiteRootItem( QgsDataItem *parent, const QString &name, const QString &filePath )
  : QgsDataCollectionItem( parent, name, filePath )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconSpatialite.svg" );
  populate();
}
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem::createChildren
//-----------------------------------------------------------------
// Will call QgsSpatiaLiteConnection
// --> for each connection entry found
//-----------------------------------------------------------------
QVector<QgsDataItem *> QgsSpatialiteRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  Q_FOREACH ( const QString &connName, QgsSpatiaLiteConnection::connectionList() )
  {
    QgsDataItem *conn = new QgsSpatialiteConnectionItem( this, connName, mPath + '/' + connName );
    connections.push_back( conn );
  }
  return connections;
}
#ifdef HAVE_GUI
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem::actions
//-----------------------------------------------------------------
QList<QAction *> QgsSpatialiteRootItem::actions( QWidget *parent )
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection..." ), parent );
  connect( actionNew, &QAction::triggered, this, &QgsSpatialiteRootItem::newConnection );
  lst.append( actionNew );

  QAction *actionCreateDatabase = new QAction( tr( "Create Database..." ), parent );
  connect( actionCreateDatabase, &QAction::triggered, this, &QgsSpatialiteRootItem::createDatabase );
  lst.append( actionCreateDatabase );

  return lst;
}
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem::paramWidget
//-----------------------------------------------------------------
QWidget *QgsSpatialiteRootItem::paramWidget()
{
  QgsSpatiaLiteSourceSelect *select = new QgsSpatiaLiteSourceSelect( nullptr, nullptr, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsSpatiaLiteSourceSelect::connectionsChanged, this, &QgsSLRootItem::onConnectionsChanged );
  return select;
}
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem::onConnectionsChanged
//-----------------------------------------------------------------
void QgsSpatialiteRootItem::onConnectionsChanged()
{
  refresh();
}
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem::newConnection
//-----------------------------------------------------------------
void QgsSpatialiteRootItem::newConnection()
{
  if ( QgsSpatiaLiteSourceSelect::newConnection( nullptr ) )
  {
    refreshConnections();
  }
}
#endif
//-----------------------------------------------------------------
//  QGISEXTERN createDb
//-----------------------------------------------------------------
QGISEXTERN bool createDb( const QString &dbPath, QString &errCause );
//-----------------------------------------------------------------
//  QgsSpatialiteRootItem::createDatabase
//-----------------------------------------------------------------
void QgsSpatialiteRootItem::createDatabase()
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

