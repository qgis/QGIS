/***************************************************************************
    offline_editing.cpp

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 22-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgsgeometry.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgsmaplayer.h"
#include "qgsofflineediting.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsspatialiteutils.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"
#include "qgsrelationmanager.h"
#include "qgsmapthemecollection.h"
#include "qgslayertree.h"
#include "qgsogrutils.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QDir>
#include <QDomDocument>
#include <QDomNode>
#include <QFile>
#include <QMessageBox>

#include <ogr_srs_api.h>

extern "C"
{
#include <sqlite3.h>
#include <spatialite.h>
}

#define CUSTOM_PROPERTY_IS_OFFLINE_EDITABLE "isOfflineEditable"
#define CUSTOM_PROPERTY_REMOTE_SOURCE "remoteSource"
#define CUSTOM_PROPERTY_REMOTE_PROVIDER "remoteProvider"
#define PROJECT_ENTRY_SCOPE_OFFLINE "OfflineEditingPlugin"
#define PROJECT_ENTRY_KEY_OFFLINE_DB_PATH "/OfflineDbPath"

QgsOfflineEditing::QgsOfflineEditing()
{
  connect( QgsProject::instance(), &QgsProject::layerWasAdded, this, &QgsOfflineEditing::layerAdded );
}

/**
 * convert current project to offline project
 * returns offline project file path
 *
 * Workflow:
 *  - copy layers to SpatiaLite
 *  - create SpatiaLite db at offlineDataPath
 *  - create table for each layer
 *  - add new SpatiaLite layer
 *  - copy features
 *  - save as offline project
 *  - mark offline layers
 *  - remove remote layers
 *  - mark as offline project
 */
bool QgsOfflineEditing::convertToOfflineProject( const QString &offlineDataPath, const QString &offlineDbFile, const QStringList &layerIds, bool onlySelected, ContainerType containerType )
{
  if ( layerIds.isEmpty() )
  {
    return false;
  }
  QString dbPath = QDir( offlineDataPath ).absoluteFilePath( offlineDbFile );
  if ( createOfflineDb( dbPath, containerType ) )
  {
    spatialite_database_unique_ptr database;
    int rc = database.open( dbPath );
    if ( rc != SQLITE_OK )
    {
      showWarning( tr( "Could not open the SpatiaLite database" ) );
    }
    else
    {
      // create logging tables
      createLoggingTables( database.get() );

      emit progressStarted();

      QMap<QString, QgsVectorJoinList > joinInfoBuffer;
      QMap<QString, QgsVectorLayer *> layerIdMapping;

      Q_FOREACH ( const QString &layerId, layerIds )
      {
        QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
        if ( !vl )
          continue;
        QgsVectorJoinList joins = vl->vectorJoins();

        // Layer names will be appended an _offline suffix
        // Join fields are prefixed with the layer name and we do not want the
        // field name to change so we stabilize the field name by defining a
        // custom prefix with the layername without _offline suffix.
        QgsVectorJoinList::iterator joinIt = joins.begin();
        while ( joinIt != joins.end() )
        {
          if ( joinIt->prefix().isNull() )
          {
            QgsVectorLayer *vl = joinIt->joinLayer();

            if ( vl )
              joinIt->setPrefix( vl->name() + '_' );
          }
          ++joinIt;
        }
        joinInfoBuffer.insert( vl->id(), joins );
      }

      // copy selected vector layers to SpatiaLite
      for ( int i = 0; i < layerIds.count(); i++ )
      {
        emit layerProgressUpdated( i + 1, layerIds.count() );

        QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerIds.at( i ) );
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
        if ( vl )
        {
          QString origLayerId = vl->id();
          QgsVectorLayer *newLayer = copyVectorLayer( vl, database.get(), dbPath, onlySelected, containerType );
          if ( newLayer )
          {
            layerIdMapping.insert( origLayerId, newLayer );
            // remove remote layer
            QgsProject::instance()->removeMapLayers(
              QStringList() << origLayerId );
          }
        }
      }

      // restore join info on new SpatiaLite layer
      QMap<QString, QgsVectorJoinList >::ConstIterator it;
      for ( it = joinInfoBuffer.constBegin(); it != joinInfoBuffer.constEnd(); ++it )
      {
        QgsVectorLayer *newLayer = layerIdMapping.value( it.key() );

        if ( newLayer )
        {
          Q_FOREACH ( QgsVectorLayerJoinInfo join, it.value() )
          {
            QgsVectorLayer *newJoinedLayer = layerIdMapping.value( join.joinLayerId() );
            if ( newJoinedLayer )
            {
              // If the layer has been offline'd, update join information
              join.setJoinLayer( newJoinedLayer );
            }
            newLayer->addJoin( join );
          }
        }
      }


      emit progressStopped();

      // save offline project
      QString projectTitle = QgsProject::instance()->title();
      if ( projectTitle.isEmpty() )
      {
        projectTitle = QFileInfo( QgsProject::instance()->fileName() ).fileName();
      }
      projectTitle += QLatin1String( " (offline)" );
      QgsProject::instance()->setTitle( projectTitle );

      QgsProject::instance()->writeEntry( PROJECT_ENTRY_SCOPE_OFFLINE, PROJECT_ENTRY_KEY_OFFLINE_DB_PATH, QgsProject::instance()->writePath( dbPath ) );

      return true;
    }
  }

  return false;
}

bool QgsOfflineEditing::isOfflineProject() const
{
  return !QgsProject::instance()->readEntry( PROJECT_ENTRY_SCOPE_OFFLINE, PROJECT_ENTRY_KEY_OFFLINE_DB_PATH ).isEmpty();
}

void QgsOfflineEditing::synchronize()
{
  // open logging db
  sqlite3_database_unique_ptr database = openLoggingDb();
  if ( !database )
  {
    return;
  }

  emit progressStarted();

  // restore and sync remote layers
  QList<QgsMapLayer *> offlineLayers;
  QMap<QString, QgsMapLayer *> mapLayers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator layer_it = mapLayers.begin() ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsMapLayer *layer = layer_it.value();
    if ( layer->customProperty( CUSTOM_PROPERTY_IS_OFFLINE_EDITABLE, false ).toBool() )
    {
      offlineLayers << layer;
    }
  }

  QgsDebugMsgLevel( QString( "Found %1 offline layers" ).arg( offlineLayers.count() ), 4 );
  for ( int l = 0; l < offlineLayers.count(); l++ )
  {
    QgsMapLayer *layer = offlineLayers.at( l );

    emit layerProgressUpdated( l + 1, offlineLayers.count() );

    QString remoteSource = layer->customProperty( CUSTOM_PROPERTY_REMOTE_SOURCE, "" ).toString();
    QString remoteProvider = layer->customProperty( CUSTOM_PROPERTY_REMOTE_PROVIDER, "" ).toString();
    QString remoteName = layer->name();
    remoteName.remove( QRegExp( " \\(offline\\)$" ) );

    QgsVectorLayer *remoteLayer = new QgsVectorLayer( remoteSource, remoteName, remoteProvider );
    if ( remoteLayer->isValid() )
    {
      // Rebuild WFS cache to get feature id<->GML fid mapping
      if ( remoteLayer->dataProvider()->name().contains( QLatin1String( "WFS" ), Qt::CaseInsensitive ) )
      {
        QgsFeatureIterator fit = remoteLayer->getFeatures();
        QgsFeature f;
        while ( fit.nextFeature( f ) )
        {
        }
      }
      // TODO: only add remote layer if there are log entries?

      QgsVectorLayer *offlineLayer = qobject_cast<QgsVectorLayer *>( layer );

      // register this layer with the central layers registry
      QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << remoteLayer, true );

      // copy style
      copySymbology( offlineLayer, remoteLayer );
      updateRelations( offlineLayer, remoteLayer );
      updateMapThemes( offlineLayer, remoteLayer );
      updateLayerOrder( offlineLayer, remoteLayer );

      // apply layer edit log
      QString qgisLayerId = layer->id();
      QString sql = QStringLiteral( "SELECT \"id\" FROM 'log_layer_ids' WHERE \"qgis_id\" = '%1'" ).arg( qgisLayerId );
      int layerId = sqlQueryInt( database.get(), sql, -1 );
      if ( layerId != -1 )
      {
        remoteLayer->startEditing();

        // TODO: only get commitNos of this layer?
        int commitNo = getCommitNo( database.get() );
        QgsDebugMsgLevel( QString( "Found %1 commits" ).arg( commitNo ), 4 );
        for ( int i = 0; i < commitNo; i++ )
        {
          QgsDebugMsgLevel( "Apply commits chronologically", 4 );
          // apply commits chronologically
          applyAttributesAdded( remoteLayer, database.get(), layerId, i );
          applyAttributeValueChanges( offlineLayer, remoteLayer, database.get(), layerId, i );
          applyGeometryChanges( remoteLayer, database.get(), layerId, i );
        }

        applyFeaturesAdded( offlineLayer, remoteLayer, database.get(), layerId );
        applyFeaturesRemoved( remoteLayer, database.get(), layerId );

        if ( remoteLayer->commitChanges() )
        {
          // update fid lookup
          updateFidLookup( remoteLayer, database.get(), layerId );

          // clear edit log for this layer
          sql = QStringLiteral( "DELETE FROM 'log_added_attrs' WHERE \"layer_id\" = %1" ).arg( layerId );
          sqlExec( database.get(), sql );
          sql = QStringLiteral( "DELETE FROM 'log_added_features' WHERE \"layer_id\" = %1" ).arg( layerId );
          sqlExec( database.get(), sql );
          sql = QStringLiteral( "DELETE FROM 'log_removed_features' WHERE \"layer_id\" = %1" ).arg( layerId );
          sqlExec( database.get(), sql );
          sql = QStringLiteral( "DELETE FROM 'log_feature_updates' WHERE \"layer_id\" = %1" ).arg( layerId );
          sqlExec( database.get(), sql );
          sql = QStringLiteral( "DELETE FROM 'log_geometry_updates' WHERE \"layer_id\" = %1" ).arg( layerId );
          sqlExec( database.get(), sql );
        }
        else
        {
          showWarning( remoteLayer->commitErrors().join( QStringLiteral( "\n" ) ) );
        }
      }
      else
      {
        QgsDebugMsg( "Could not find the layer id in the edit logs!" );
      }
      // Invalidate the connection to force a reload if the project is put offline
      // again with the same path
      offlineLayer->dataProvider()->invalidateConnections( QgsDataSourceUri( offlineLayer->source() ).database() );
      // remove offline layer
      QgsProject::instance()->removeMapLayers( QStringList() << qgisLayerId );


      // disable offline project
      QString projectTitle = QgsProject::instance()->title();
      projectTitle.remove( QRegExp( " \\(offline\\)$" ) );
      QgsProject::instance()->setTitle( projectTitle );
      QgsProject::instance()->removeEntry( PROJECT_ENTRY_SCOPE_OFFLINE, PROJECT_ENTRY_KEY_OFFLINE_DB_PATH );
      remoteLayer->reload(); //update with other changes
    }
    else
    {
      QgsDebugMsg( "Remote layer is not valid!" );
    }
  }

  // reset commitNo
  QString sql = QStringLiteral( "UPDATE 'log_indices' SET 'last_index' = 0 WHERE \"name\" = 'commit_no'" );
  sqlExec( database.get(), sql );

  emit progressStopped();
}

void QgsOfflineEditing::initializeSpatialMetadata( sqlite3 *sqlite_handle )
{
  // attempting to perform self-initialization for a newly created DB
  if ( !sqlite_handle )
    return;
  // checking if this DB is really empty
  char **results = nullptr;
  int rows, columns;
  int ret = sqlite3_get_table( sqlite_handle, "select count(*) from sqlite_master", &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return;
  int count = 0;
  if ( rows >= 1 )
  {
    for ( int i = 1; i <= rows; i++ )
      count = atoi( results[( i * columns ) + 0] );
  }

  sqlite3_free_table( results );

  if ( count > 0 )
    return;

  bool above41 = false;
  ret = sqlite3_get_table( sqlite_handle, "select spatialite_version()", &results, &rows, &columns, nullptr );
  if ( ret == SQLITE_OK && rows == 1 && columns == 1 )
  {
    QString version = QString::fromUtf8( results[1] );
    QStringList parts = version.split( ' ', QString::SkipEmptyParts );
    if ( !parts.empty() )
    {
      QStringList verparts = parts.at( 0 ).split( '.', QString::SkipEmptyParts );
      above41 = verparts.size() >= 2 && ( verparts.at( 0 ).toInt() > 4 || ( verparts.at( 0 ).toInt() == 4 && verparts.at( 1 ).toInt() >= 1 ) );
    }
  }

  sqlite3_free_table( results );

  // all right, it's empty: proceeding to initialize
  char *errMsg = nullptr;
  ret = sqlite3_exec( sqlite_handle, above41 ? "SELECT InitSpatialMetadata(1)" : "SELECT InitSpatialMetadata()", nullptr, nullptr, &errMsg );

  if ( ret != SQLITE_OK )
  {
    QString errCause = tr( "Unable to initialize SpatialMetadata:\n" );
    errCause += QString::fromUtf8( errMsg );
    showWarning( errCause );
    sqlite3_free( errMsg );
    return;
  }
  spatial_ref_sys_init( sqlite_handle, 0 );
}

bool QgsOfflineEditing::createOfflineDb( const QString &offlineDbPath, ContainerType containerType )
{
  int ret;
  char *errMsg = nullptr;
  QFile newDb( offlineDbPath );
  if ( newDb.exists() )
  {
    QFile::remove( offlineDbPath );
  }

  // see also QgsNewSpatialiteLayerDialog::createDb()

  QFileInfo fullPath = QFileInfo( offlineDbPath );
  QDir path = fullPath.dir();

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath() );

  // creating/opening the new database
  QString dbPath = newDb.fileName();

  // creating geopackage
  switch ( containerType )
  {
    case GPKG:
    {
      OGRSFDriverH hGpkgDriver = OGRGetDriverByName( "GPKG" );
      if ( !hGpkgDriver )
      {
        showWarning( tr( "Creation of database failed. GeoPackage driver not found." ) );
        return false;
      }

      gdal::ogr_datasource_unique_ptr hDS( OGR_Dr_CreateDataSource( hGpkgDriver, dbPath.toUtf8().constData(), nullptr ) );
      if ( !hDS )
      {
        showWarning( tr( "Creation of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        return false;
      }
      break;
    }
    case SpatiaLite:
    {
      break;
    }
  }

  spatialite_database_unique_ptr database;
  ret = database.open_v2( dbPath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( ret )
  {
    // an error occurred
    QString errCause = tr( "Could not create a new database\n" );
    errCause += database.errorMessage();
    showWarning( errCause );
    return false;
  }
  // activating Foreign Key constraints
  ret = sqlite3_exec( database.get(), "PRAGMA foreign_keys = 1", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    showWarning( tr( "Unable to activate FOREIGN_KEY constraints" ) );
    sqlite3_free( errMsg );
    return false;
  }
  initializeSpatialMetadata( database.get() );
  return true;
}

void QgsOfflineEditing::createLoggingTables( sqlite3 *db )
{
  // indices
  QString sql = QStringLiteral( "CREATE TABLE 'log_indices' ('name' TEXT, 'last_index' INTEGER)" );
  sqlExec( db, sql );

  sql = QStringLiteral( "INSERT INTO 'log_indices' VALUES ('commit_no', 0)" );
  sqlExec( db, sql );

  sql = QStringLiteral( "INSERT INTO 'log_indices' VALUES ('layer_id', 0)" );
  sqlExec( db, sql );

  // layername <-> layer id
  sql = QStringLiteral( "CREATE TABLE 'log_layer_ids' ('id' INTEGER, 'qgis_id' TEXT)" );
  sqlExec( db, sql );

  // offline fid <-> remote fid
  sql = QStringLiteral( "CREATE TABLE 'log_fids' ('layer_id' INTEGER, 'offline_fid' INTEGER, 'remote_fid' INTEGER)" );
  sqlExec( db, sql );

  // added attributes
  sql = QStringLiteral( "CREATE TABLE 'log_added_attrs' ('layer_id' INTEGER, 'commit_no' INTEGER, " );
  sql += QLatin1String( "'name' TEXT, 'type' INTEGER, 'length' INTEGER, 'precision' INTEGER, 'comment' TEXT)" );
  sqlExec( db, sql );

  // added features
  sql = QStringLiteral( "CREATE TABLE 'log_added_features' ('layer_id' INTEGER, 'fid' INTEGER)" );
  sqlExec( db, sql );

  // removed features
  sql = QStringLiteral( "CREATE TABLE 'log_removed_features' ('layer_id' INTEGER, 'fid' INTEGER)" );
  sqlExec( db, sql );

  // feature updates
  sql = QStringLiteral( "CREATE TABLE 'log_feature_updates' ('layer_id' INTEGER, 'commit_no' INTEGER, 'fid' INTEGER, 'attr' INTEGER, 'value' TEXT)" );
  sqlExec( db, sql );

  // geometry updates
  sql = QStringLiteral( "CREATE TABLE 'log_geometry_updates' ('layer_id' INTEGER, 'commit_no' INTEGER, 'fid' INTEGER, 'geom_wkt' TEXT)" );
  sqlExec( db, sql );

  /* TODO: other logging tables
    - attr delete (not supported by SpatiaLite provider)
  */
}

QgsVectorLayer *QgsOfflineEditing::copyVectorLayer( QgsVectorLayer *layer, sqlite3 *db, const QString &offlineDbPath, bool onlySelected, ContainerType containerType )
{
  if ( !layer )
    return nullptr;

  QString tableName = layer->id();
  QgsDebugMsgLevel( QString( "Creating offline table %1 ..." ).arg( tableName ), 4 );

  // new layer
  QgsVectorLayer *newLayer = nullptr;

  switch ( containerType )
  {
    case SpatiaLite:
    {
      // create table
      QString sql = QStringLiteral( "CREATE TABLE '%1' (" ).arg( tableName );
      QString delim;
      const QgsFields providerFields = layer->dataProvider()->fields();
      for ( const auto &field : providerFields )
      {
        QString dataType;
        QVariant::Type type = field.type();
        if ( type == QVariant::Int || type == QVariant::LongLong )
        {
          dataType = QStringLiteral( "INTEGER" );
        }
        else if ( type == QVariant::Double )
        {
          dataType = QStringLiteral( "REAL" );
        }
        else if ( type == QVariant::String )
        {
          dataType = QStringLiteral( "TEXT" );
        }
        else
        {
          showWarning( tr( "%1: Unknown data type %2. Not using type affinity for the field." ).arg( field.name(), QVariant::typeToName( type ) ) );
        }

        sql += delim + QStringLiteral( "'%1' %2" ).arg( field.name(), dataType );
        delim = ',';
      }
      sql += ')';

      int rc = sqlExec( db, sql );

      // add geometry column
      if ( layer->isSpatial() )
      {
        const QgsWkbTypes::Type sourceWkbType = layer->wkbType();

        QString geomType;
        switch ( QgsWkbTypes::flatType( sourceWkbType ) )
        {
          case QgsWkbTypes::Point:
            geomType = QStringLiteral( "POINT" );
            break;
          case QgsWkbTypes::MultiPoint:
            geomType = QStringLiteral( "MULTIPOINT" );
            break;
          case QgsWkbTypes::LineString:
            geomType = QStringLiteral( "LINESTRING" );
            break;
          case QgsWkbTypes::MultiLineString:
            geomType = QStringLiteral( "MULTILINESTRING" );
            break;
          case QgsWkbTypes::Polygon:
            geomType = QStringLiteral( "POLYGON" );
            break;
          case QgsWkbTypes::MultiPolygon:
            geomType = QStringLiteral( "MULTIPOLYGON" );
            break;
          default:
            showWarning( tr( "Layer %1 has unsupported geometry type %2." ).arg( layer->name(), QgsWkbTypes::displayString( layer->wkbType() ) ) );
            break;
        };

        QString zmInfo = QStringLiteral( "XY" );

        if ( QgsWkbTypes::hasZ( sourceWkbType ) )
          zmInfo += 'Z';
        if ( QgsWkbTypes::hasM( sourceWkbType ) )
          zmInfo += 'M';

        QString epsgCode;

        if ( layer->crs().authid().startsWith( QLatin1String( "EPSG:" ), Qt::CaseInsensitive ) )
        {
          epsgCode = layer->crs().authid().mid( 5 );
        }
        else
        {
          epsgCode = '0';
          showWarning( tr( "Layer %1 has unsupported Coordinate Reference System (%2)." ).arg( layer->name(), layer->crs().authid() ) );
        }

        QString sqlAddGeom = QStringLiteral( "SELECT AddGeometryColumn('%1', 'Geometry', %2, '%3', '%4')" )
                             .arg( tableName, epsgCode, geomType, zmInfo );

        // create spatial index
        QString sqlCreateIndex = QStringLiteral( "SELECT CreateSpatialIndex('%1', 'Geometry')" ).arg( tableName );

        if ( rc == SQLITE_OK )
        {
          rc = sqlExec( db, sqlAddGeom );
          if ( rc == SQLITE_OK )
          {
            rc = sqlExec( db, sqlCreateIndex );
          }
        }
      }

      if ( rc != SQLITE_OK )
      {
        showWarning( tr( "Filling SpatiaLite for layer %1 failed" ).arg( layer->name() ) );
        return nullptr;
      }

      // add new layer
      QString connectionString = QStringLiteral( "dbname='%1' table='%2'%3 sql=" )
                                 .arg( offlineDbPath,
                                       tableName, layer->isSpatial() ? "(Geometry)" : "" );
      newLayer = new QgsVectorLayer( connectionString,
                                     layer->name() + " (offline)", QStringLiteral( "spatialite" ) );
      break;
    }
    case GPKG:
    {
      // Set options
      char **options = nullptr;

      options = CSLSetNameValue( options, "OVERWRITE", "YES" );
      options = CSLSetNameValue( options, "IDENTIFIER", tr( "%1 (offline)" ).arg( layer->name() ).toUtf8().constData() );
      options = CSLSetNameValue( options, "DESCRIPTION", layer->dataComment().toUtf8().constData() );
#if 0
      options = CSLSetNameValue( options, "FID", featureId.toUtf8().constData() );
#endif

      if ( layer->isSpatial() )
      {
        options = CSLSetNameValue( options, "GEOMETRY_COLUMN", "geom" );
        options = CSLSetNameValue( options, "SPATIAL_INDEX", "YES" );
      }

      OGRSFDriverH hDriver = nullptr;
      OGRSpatialReferenceH hSRS = OSRNewSpatialReference( layer->crs().toWkt().toLocal8Bit().data() );
      gdal::ogr_datasource_unique_ptr hDS( OGROpen( offlineDbPath.toUtf8().constData(), true, &hDriver ) );
      OGRLayerH hLayer = OGR_DS_CreateLayer( hDS.get(), tableName.toUtf8().constData(), hSRS, static_cast<OGRwkbGeometryType>( layer->wkbType() ), options );
      CSLDestroy( options );
      if ( hSRS )
        OSRRelease( hSRS );
      if ( !hLayer )
      {
        showWarning( tr( "Creation of layer failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        return nullptr;
      }

      const QgsFields providerFields = layer->dataProvider()->fields();
      for ( const auto &field : providerFields )
      {
        const QString fieldName( field.name() );
        const QVariant::Type type = field.type();
        OGRFieldType ogrType( OFTString );
        if ( type == QVariant::Int )
          ogrType = OFTInteger;
        else if ( type == QVariant::LongLong )
          ogrType = OFTInteger64;
        else if ( type == QVariant::Double )
          ogrType = OFTReal;
        else if ( type == QVariant::Time )
          ogrType = OFTTime;
        else if ( type == QVariant::Date )
          ogrType = OFTDate;
        else if ( type == QVariant::DateTime )
          ogrType = OFTDateTime;
        else
          ogrType = OFTString;

        int ogrWidth = field.length();

        gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), ogrType ) );
        OGR_Fld_SetWidth( fld.get(), ogrWidth );

        if ( OGR_L_CreateField( hLayer, fld.get(), true ) != OGRERR_NONE )
        {
          showWarning( tr( "Creation of field %1 failed (OGR error: %2)" )
                       .arg( fieldName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
          return nullptr;
        }
      }

      // In GDAL >= 2.0, the driver implements a deferred creation strategy, so
      // issue a command that will force table creation
      CPLErrorReset();
      OGR_L_ResetReading( hLayer );
      if ( CPLGetLastErrorType() != CE_None )
      {
        QString msg( tr( "Creation of layer failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
        showWarning( msg );
        return nullptr;
      }
      hDS.reset();

      QString uri = QStringLiteral( "%1|layername=%2" ).arg( offlineDbPath,  tableName );
      newLayer = new QgsVectorLayer( uri, layer->name() + " (offline)", QStringLiteral( "ogr" ) );
      break;
    }
  }

  if ( newLayer->isValid() )
  {

    // copy features
    newLayer->startEditing();
    QgsFeature f;

    QgsFeatureRequest req;

    if ( onlySelected )
    {
      QgsFeatureIds selectedFids = layer->selectedFeatureIds();
      if ( !selectedFids.isEmpty() )
        req.setFilterFids( selectedFids );
    }

    QgsFeatureIterator fit = layer->dataProvider()->getFeatures( req );

    if ( req.filterType() == QgsFeatureRequest::FilterFids )
    {
      emit progressModeSet( QgsOfflineEditing::CopyFeatures, layer->selectedFeatureIds().size() );
    }
    else
    {
      emit progressModeSet( QgsOfflineEditing::CopyFeatures, layer->dataProvider()->featureCount() );
    }
    int featureCount = 1;

    QList<QgsFeatureId> remoteFeatureIds;
    while ( fit.nextFeature( f ) )
    {
      remoteFeatureIds << f.id();

      // NOTE: SpatiaLite provider ignores position of geometry column
      // fill gap in QgsAttributeMap if geometry column is not last (WORKAROUND)
      QgsAttributes attrs = f.attributes();
      int column = 0;
      int indexOfFid = layer->dataProvider()->fields().lookupField( "fid" );
      if ( containerType == GPKG && ( indexOfFid == -1 || ( layer->dataProvider()->fields().at( indexOfFid ).type() != QVariant::Int
                                      && layer->dataProvider()->fields().at( indexOfFid ).type() != QVariant::LongLong ) ) )
      {
        // newAttrs (1) has an additional attribute (fid) that is (2) of the correct type
        // so we have to add a dummy because otherwise it messes up with the amount of attributes
        column++;
      }
      QgsAttributes newAttrs( attrs.count() + column );
      for ( int it = 0; it < attrs.count(); ++it )
      {
        newAttrs[column++] = attrs.at( it );
      }
      f.setAttributes( newAttrs );

      newLayer->addFeature( f );

      emit progressUpdated( featureCount++ );
    }
    if ( newLayer->commitChanges() )
    {
      emit progressModeSet( QgsOfflineEditing::ProcessFeatures, layer->dataProvider()->featureCount() );
      featureCount = 1;

      // update feature id lookup
      int layerId = getOrCreateLayerId( db, newLayer->id() );
      QList<QgsFeatureId> offlineFeatureIds;

      QgsFeatureIterator fit = newLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( QgsAttributeList() ) );
      while ( fit.nextFeature( f ) )
      {
        offlineFeatureIds << f.id();
      }

      // NOTE: insert fids in this loop, as the db is locked during newLayer->nextFeature()
      sqlExec( db, QStringLiteral( "BEGIN" ) );
      int remoteCount = remoteFeatureIds.size();
      for ( int i = 0; i < remoteCount; i++ )
      {
        // Check if the online feature has been fetched (WFS download aborted for some reason)
        if ( i < offlineFeatureIds.count() )
        {
          addFidLookup( db, layerId, offlineFeatureIds.at( i ), remoteFeatureIds.at( i ) );
        }
        else
        {
          showWarning( tr( "Feature cannot be copied to the offline layer, please check if the online layer '%1' is still accessible." ).arg( layer->name() ) );
          return nullptr;
        }
        emit progressUpdated( featureCount++ );
      }
      sqlExec( db, QStringLiteral( "COMMIT" ) );
    }
    else
    {
      showWarning( newLayer->commitErrors().join( QStringLiteral( "\n" ) ) );
    }

    // mark as offline layer
    newLayer->setCustomProperty( CUSTOM_PROPERTY_IS_OFFLINE_EDITABLE, true );

    // store original layer source
    newLayer->setCustomProperty( CUSTOM_PROPERTY_REMOTE_SOURCE, layer->source() );
    newLayer->setCustomProperty( CUSTOM_PROPERTY_REMOTE_PROVIDER, layer->providerType() );

    // register this layer with the central layers registry
    QgsProject::instance()->addMapLayers(
      QList<QgsMapLayer *>() << newLayer );

    // copy style
    copySymbology( layer, newLayer );

    QgsLayerTreeGroup *layerTreeRoot = QgsProject::instance()->layerTreeRoot();
    // Find the parent group of the original layer
    QgsLayerTreeLayer *layerTreeLayer = layerTreeRoot->findLayer( layer->id() );
    if ( layerTreeLayer )
    {
      QgsLayerTreeGroup *parentTreeGroup = qobject_cast<QgsLayerTreeGroup *>( layerTreeLayer->parent() );
      if ( parentTreeGroup )
      {
        int index = parentTreeGroup->children().indexOf( layerTreeLayer );
        // Move the new layer from the root group to the new group
        QgsLayerTreeLayer *newLayerTreeLayer = layerTreeRoot->findLayer( newLayer->id() );
        if ( newLayerTreeLayer )
        {
          QgsLayerTreeNode *newLayerTreeLayerClone = newLayerTreeLayer->clone();
          QgsLayerTreeGroup *grp = qobject_cast<QgsLayerTreeGroup *>( newLayerTreeLayer->parent() );
          parentTreeGroup->insertChildNode( index, newLayerTreeLayerClone );
          if ( grp )
            grp->removeChildNode( newLayerTreeLayer );
        }
      }
    }

    updateRelations( layer, newLayer );
    updateMapThemes( layer, newLayer );
    updateLayerOrder( layer, newLayer );



  }
  return newLayer;
}

void QgsOfflineEditing::applyAttributesAdded( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId, int commitNo )
{
  QString sql = QStringLiteral( "SELECT \"name\", \"type\", \"length\", \"precision\", \"comment\" FROM 'log_added_attrs' WHERE \"layer_id\" = %1 AND \"commit_no\" = %2" ).arg( layerId ).arg( commitNo );
  QList<QgsField> fields = sqlQueryAttributesAdded( db, sql );

  const QgsVectorDataProvider *provider = remoteLayer->dataProvider();
  QList<QgsVectorDataProvider::NativeType> nativeTypes = provider->nativeTypes();

  // NOTE: uses last matching QVariant::Type of nativeTypes
  QMap < QVariant::Type, QString /*typeName*/ > typeNameLookup;
  for ( int i = 0; i < nativeTypes.size(); i++ )
  {
    QgsVectorDataProvider::NativeType nativeType = nativeTypes.at( i );
    typeNameLookup[ nativeType.mType ] = nativeType.mTypeName;
  }

  emit progressModeSet( QgsOfflineEditing::AddFields, fields.size() );

  for ( int i = 0; i < fields.size(); i++ )
  {
    // lookup typename from layer provider
    QgsField field = fields[i];
    if ( typeNameLookup.contains( field.type() ) )
    {
      QString typeName = typeNameLookup[ field.type()];
      field.setTypeName( typeName );
      remoteLayer->addAttribute( field );
    }
    else
    {
      showWarning( QStringLiteral( "Could not add attribute '%1' of type %2" ).arg( field.name() ).arg( field.type() ) );
    }

    emit progressUpdated( i + 1 );
  }
}

void QgsOfflineEditing::applyFeaturesAdded( QgsVectorLayer *offlineLayer, QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId )
{
  QString sql = QStringLiteral( "SELECT \"fid\" FROM 'log_added_features' WHERE \"layer_id\" = %1" ).arg( layerId );
  QList<int> featureIdInts = sqlQueryInts( db, sql );
  QgsFeatureIds newFeatureIds;
  Q_FOREACH ( int id, featureIdInts )
  {
    newFeatureIds << id;
  }

  QgsExpressionContext context = remoteLayer->createExpressionContext();

  // get new features from offline layer
  QgsFeatureList features;
  QgsFeatureIterator it = offlineLayer->getFeatures( QgsFeatureRequest().setFilterFids( newFeatureIds ) );
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    features << feature;
  }

  // copy features to remote layer
  emit progressModeSet( QgsOfflineEditing::AddFeatures, features.size() );

  int i = 1;
  int newAttrsCount = remoteLayer->fields().count();
  for ( QgsFeatureList::iterator it = features.begin(); it != features.end(); ++it )
  {
    // NOTE: SpatiaLite provider ignores position of geometry column
    // restore gap in QgsAttributeMap if geometry column is not last (WORKAROUND)
    QMap<int, int> attrLookup = attributeLookup( offlineLayer, remoteLayer );
    QgsAttributes newAttrs( newAttrsCount );
    QgsAttributes attrs = it->attributes();
    for ( int it = 0; it < attrs.count(); ++it )
    {
      newAttrs[ attrLookup[ it ] ] = attrs.at( it );
    }

    // respect constraints and provider default values
    QgsFeature f = QgsVectorLayerUtils::createFeature( remoteLayer, it->geometry(), newAttrs.toMap(), &context );
    remoteLayer->addFeature( f );

    emit progressUpdated( i++ );
  }
}

void QgsOfflineEditing::applyFeaturesRemoved( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId )
{
  QString sql = QStringLiteral( "SELECT \"fid\" FROM 'log_removed_features' WHERE \"layer_id\" = %1" ).arg( layerId );
  QgsFeatureIds values = sqlQueryFeaturesRemoved( db, sql );

  emit progressModeSet( QgsOfflineEditing::RemoveFeatures, values.size() );

  int i = 1;
  for ( QgsFeatureIds::const_iterator it = values.constBegin(); it != values.constEnd(); ++it )
  {
    QgsFeatureId fid = remoteFid( db, layerId, *it );
    remoteLayer->deleteFeature( fid );

    emit progressUpdated( i++ );
  }
}

void QgsOfflineEditing::applyAttributeValueChanges( QgsVectorLayer *offlineLayer, QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId, int commitNo )
{
  QString sql = QStringLiteral( "SELECT \"fid\", \"attr\", \"value\" FROM 'log_feature_updates' WHERE \"layer_id\" = %1 AND \"commit_no\" = %2 " ).arg( layerId ).arg( commitNo );
  AttributeValueChanges values = sqlQueryAttributeValueChanges( db, sql );

  emit progressModeSet( QgsOfflineEditing::UpdateFeatures, values.size() );

  QMap<int, int> attrLookup = attributeLookup( offlineLayer, remoteLayer );

  for ( int i = 0; i < values.size(); i++ )
  {
    QgsFeatureId fid = remoteFid( db, layerId, values.at( i ).fid );
    QgsDebugMsgLevel( QString( "Offline changeAttributeValue %1 = %2" ).arg( QString( attrLookup[ values.at( i ).attr ] ), values.at( i ).value ), 4 );
    remoteLayer->changeAttributeValue( fid, attrLookup[ values.at( i ).attr ], values.at( i ).value );

    emit progressUpdated( i + 1 );
  }
}

void QgsOfflineEditing::applyGeometryChanges( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId, int commitNo )
{
  QString sql = QStringLiteral( "SELECT \"fid\", \"geom_wkt\" FROM 'log_geometry_updates' WHERE \"layer_id\" = %1 AND \"commit_no\" = %2" ).arg( layerId ).arg( commitNo );
  GeometryChanges values = sqlQueryGeometryChanges( db, sql );

  emit progressModeSet( QgsOfflineEditing::UpdateGeometries, values.size() );

  for ( int i = 0; i < values.size(); i++ )
  {
    QgsFeatureId fid = remoteFid( db, layerId, values.at( i ).fid );
    QgsGeometry newGeom = QgsGeometry::fromWkt( values.at( i ).geom_wkt );
    remoteLayer->changeGeometry( fid, newGeom );

    emit progressUpdated( i + 1 );
  }
}

void QgsOfflineEditing::updateFidLookup( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId )
{
  // update fid lookup for added features

  // get remote added fids
  // NOTE: use QMap for sorted fids
  QMap < QgsFeatureId, bool /*dummy*/ > newRemoteFids;
  QgsFeature f;

  QgsFeatureIterator fit = remoteLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( QgsAttributeList() ) );

  emit progressModeSet( QgsOfflineEditing::ProcessFeatures, remoteLayer->featureCount() );

  int i = 1;
  while ( fit.nextFeature( f ) )
  {
    if ( offlineFid( db, layerId, f.id() ) == -1 )
    {
      newRemoteFids[ f.id()] = true;
    }

    emit progressUpdated( i++ );
  }

  // get local added fids
  // NOTE: fids are sorted
  QString sql = QStringLiteral( "SELECT \"fid\" FROM 'log_added_features' WHERE \"layer_id\" = %1" ).arg( layerId );
  QList<int> newOfflineFids = sqlQueryInts( db, sql );

  if ( newRemoteFids.size() != newOfflineFids.size() )
  {
    //showWarning( QString( "Different number of new features on offline layer (%1) and remote layer (%2)" ).arg(newOfflineFids.size()).arg(newRemoteFids.size()) );
  }
  else
  {
    // add new fid lookups
    i = 0;
    sqlExec( db, QStringLiteral( "BEGIN" ) );
    for ( QMap<QgsFeatureId, bool>::const_iterator it = newRemoteFids.constBegin(); it != newRemoteFids.constEnd(); ++it )
    {
      addFidLookup( db, layerId, newOfflineFids.at( i++ ), it.key() );
    }
    sqlExec( db, QStringLiteral( "COMMIT" ) );
  }
}

void QgsOfflineEditing::copySymbology( QgsVectorLayer *sourceLayer, QgsVectorLayer *targetLayer )
{
  QString error;
  QDomDocument doc;
  QgsReadWriteContext context;
  QgsMapLayer::StyleCategories categories = static_cast<QgsMapLayer::StyleCategories>( QgsMapLayer::AllStyleCategories ) & ~QgsMapLayer::CustomProperties;
  sourceLayer->exportNamedStyle( doc, error, context, categories );

  if ( error.isEmpty() )
  {
    targetLayer->importNamedStyle( doc, error, categories );
  }
  if ( !error.isEmpty() )
  {
    showWarning( error );
  }
}

void QgsOfflineEditing::updateRelations( QgsVectorLayer *sourceLayer, QgsVectorLayer *targetLayer )
{
  QgsRelationManager *relationManager = QgsProject::instance()->relationManager();
  QList<QgsRelation> relations;
  relations = relationManager->referencedRelations( sourceLayer );

  Q_FOREACH ( QgsRelation relation, relations )
  {
    relationManager->removeRelation( relation );
    relation.setReferencedLayer( targetLayer->id() );
    relationManager->addRelation( relation );
  }

  relations = relationManager->referencingRelations( sourceLayer );

  Q_FOREACH ( QgsRelation relation, relations )
  {
    relationManager->removeRelation( relation );
    relation.setReferencingLayer( targetLayer->id() );
    relationManager->addRelation( relation );
  }
}

void QgsOfflineEditing::updateMapThemes( QgsVectorLayer *sourceLayer, QgsVectorLayer *targetLayer )
{
  QgsMapThemeCollection *mapThemeCollection = QgsProject::instance()->mapThemeCollection();
  QStringList mapThemeNames = mapThemeCollection->mapThemes();

  Q_FOREACH ( const QString &mapThemeName, mapThemeNames )
  {
    QgsMapThemeCollection::MapThemeRecord record = mapThemeCollection->mapThemeState( mapThemeName );

    Q_FOREACH ( QgsMapThemeCollection::MapThemeLayerRecord layerRecord, record.layerRecords() )
    {
      if ( layerRecord.layer() == sourceLayer )
      {
        layerRecord.setLayer( targetLayer );
        record.removeLayerRecord( sourceLayer );
        record.addLayerRecord( layerRecord );
      }
    }

    QgsProject::instance()->mapThemeCollection()->update( mapThemeName, record );
  }
}

void QgsOfflineEditing::updateLayerOrder( QgsVectorLayer *sourceLayer, QgsVectorLayer *targetLayer )
{
  QList<QgsMapLayer *>  layerOrder = QgsProject::instance()->layerTreeRoot()->customLayerOrder();

  auto iterator = layerOrder.begin();

  while ( iterator != layerOrder.end() )
  {
    if ( *iterator == targetLayer )
    {
      iterator = layerOrder.erase( iterator );
      if ( iterator == layerOrder.end() )
        break;
    }

    if ( *iterator == sourceLayer )
    {
      *iterator = targetLayer;
    }

    ++iterator;
  }

  QgsProject::instance()->layerTreeRoot()->setCustomLayerOrder( layerOrder );
}

// NOTE: use this to map column indices in case the remote geometry column is not last
QMap<int, int> QgsOfflineEditing::attributeLookup( QgsVectorLayer *offlineLayer, QgsVectorLayer *remoteLayer )
{
  const QgsAttributeList &offlineAttrs = offlineLayer->attributeList();
  const QgsAttributeList &remoteAttrs = remoteLayer->attributeList();

  QMap < int /*offline attr*/, int /*remote attr*/ > attrLookup;
  // NOTE: use size of remoteAttrs, as offlineAttrs can have new attributes not yet synced
  for ( int i = 0; i < remoteAttrs.size(); i++ )
  {
    attrLookup.insert( offlineAttrs.at( i ), remoteAttrs.at( i ) );
  }

  return attrLookup;
}

void QgsOfflineEditing::showWarning( const QString &message )
{
  emit warning( tr( "Offline Editing Plugin" ), message );
}

sqlite3_database_unique_ptr QgsOfflineEditing::openLoggingDb()
{
  sqlite3_database_unique_ptr database;
  QString dbPath = QgsProject::instance()->readEntry( PROJECT_ENTRY_SCOPE_OFFLINE, PROJECT_ENTRY_KEY_OFFLINE_DB_PATH );
  if ( !dbPath.isEmpty() )
  {
    QString absoluteDbPath = QgsProject::instance()->readPath( dbPath );
    int rc = database.open( absoluteDbPath );
    if ( rc != SQLITE_OK )
    {
      QgsDebugMsg( "Could not open the SpatiaLite logging database" );
      showWarning( tr( "Could not open the SpatiaLite logging database" ) );
    }
  }
  else
  {
    QgsDebugMsg( "dbPath is empty!" );
  }
  return database;
}

int QgsOfflineEditing::getOrCreateLayerId( sqlite3 *db, const QString &qgisLayerId )
{
  QString sql = QStringLiteral( "SELECT \"id\" FROM 'log_layer_ids' WHERE \"qgis_id\" = '%1'" ).arg( qgisLayerId );
  int layerId = sqlQueryInt( db, sql, -1 );
  if ( layerId == -1 )
  {
    // next layer id
    sql = QStringLiteral( "SELECT \"last_index\" FROM 'log_indices' WHERE \"name\" = 'layer_id'" );
    int newLayerId = sqlQueryInt( db, sql, -1 );

    // insert layer
    sql = QStringLiteral( "INSERT INTO 'log_layer_ids' VALUES (%1, '%2')" ).arg( newLayerId ).arg( qgisLayerId );
    sqlExec( db, sql );

    // increase layer_id
    // TODO: use trigger for auto increment?
    sql = QStringLiteral( "UPDATE 'log_indices' SET 'last_index' = %1 WHERE \"name\" = 'layer_id'" ).arg( newLayerId + 1 );
    sqlExec( db, sql );

    layerId = newLayerId;
  }

  return layerId;
}

int QgsOfflineEditing::getCommitNo( sqlite3 *db )
{
  QString sql = QStringLiteral( "SELECT \"last_index\" FROM 'log_indices' WHERE \"name\" = 'commit_no'" );
  return sqlQueryInt( db, sql, -1 );
}

void QgsOfflineEditing::increaseCommitNo( sqlite3 *db )
{
  QString sql = QStringLiteral( "UPDATE 'log_indices' SET 'last_index' = %1 WHERE \"name\" = 'commit_no'" ).arg( getCommitNo( db ) + 1 );
  sqlExec( db, sql );
}

void QgsOfflineEditing::addFidLookup( sqlite3 *db, int layerId, QgsFeatureId offlineFid, QgsFeatureId remoteFid )
{
  QString sql = QStringLiteral( "INSERT INTO 'log_fids' VALUES ( %1, %2, %3 )" ).arg( layerId ).arg( offlineFid ).arg( remoteFid );
  sqlExec( db, sql );
}

QgsFeatureId QgsOfflineEditing::remoteFid( sqlite3 *db, int layerId, QgsFeatureId offlineFid )
{
  QString sql = QStringLiteral( "SELECT \"remote_fid\" FROM 'log_fids' WHERE \"layer_id\" = %1 AND \"offline_fid\" = %2" ).arg( layerId ).arg( offlineFid );
  return sqlQueryInt( db, sql, -1 );
}

QgsFeatureId QgsOfflineEditing::offlineFid( sqlite3 *db, int layerId, QgsFeatureId remoteFid )
{
  QString sql = QStringLiteral( "SELECT \"offline_fid\" FROM 'log_fids' WHERE \"layer_id\" = %1 AND \"remote_fid\" = %2" ).arg( layerId ).arg( remoteFid );
  return sqlQueryInt( db, sql, -1 );
}

bool QgsOfflineEditing::isAddedFeature( sqlite3 *db, int layerId, QgsFeatureId fid )
{
  QString sql = QStringLiteral( "SELECT COUNT(\"fid\") FROM 'log_added_features' WHERE \"layer_id\" = %1 AND \"fid\" = %2" ).arg( layerId ).arg( fid );
  return ( sqlQueryInt( db, sql, 0 ) > 0 );
}

int QgsOfflineEditing::sqlExec( sqlite3 *db, const QString &sql )
{
  char *errmsg = nullptr;
  int rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, &errmsg );
  if ( rc != SQLITE_OK )
  {
    showWarning( errmsg );
  }
  return rc;
}

int QgsOfflineEditing::sqlQueryInt( sqlite3 *db, const QString &sql, int defaultValue )
{
  sqlite3_stmt *stmt = nullptr;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    showWarning( sqlite3_errmsg( db ) );
    return defaultValue;
  }

  int value = defaultValue;
  int ret = sqlite3_step( stmt );
  if ( ret == SQLITE_ROW )
  {
    value = sqlite3_column_int( stmt, 0 );
  }
  sqlite3_finalize( stmt );

  return value;
}

QList<int> QgsOfflineEditing::sqlQueryInts( sqlite3 *db, const QString &sql )
{
  QList<int> values;

  sqlite3_stmt *stmt = nullptr;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    showWarning( sqlite3_errmsg( db ) );
    return values;
  }

  int ret = sqlite3_step( stmt );
  while ( ret == SQLITE_ROW )
  {
    values << sqlite3_column_int( stmt, 0 );

    ret = sqlite3_step( stmt );
  }
  sqlite3_finalize( stmt );

  return values;
}

QList<QgsField> QgsOfflineEditing::sqlQueryAttributesAdded( sqlite3 *db, const QString &sql )
{
  QList<QgsField> values;

  sqlite3_stmt *stmt = nullptr;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    showWarning( sqlite3_errmsg( db ) );
    return values;
  }

  int ret = sqlite3_step( stmt );
  while ( ret == SQLITE_ROW )
  {
    QgsField field( QString( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 0 ) ) ),
                    static_cast< QVariant::Type >( sqlite3_column_int( stmt, 1 ) ),
                    QString(), // typeName
                    sqlite3_column_int( stmt, 2 ),
                    sqlite3_column_int( stmt, 3 ),
                    QString( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 4 ) ) ) );
    values << field;

    ret = sqlite3_step( stmt );
  }
  sqlite3_finalize( stmt );

  return values;
}

QgsFeatureIds QgsOfflineEditing::sqlQueryFeaturesRemoved( sqlite3 *db, const QString &sql )
{
  QgsFeatureIds values;

  sqlite3_stmt *stmt = nullptr;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    showWarning( sqlite3_errmsg( db ) );
    return values;
  }

  int ret = sqlite3_step( stmt );
  while ( ret == SQLITE_ROW )
  {
    values << sqlite3_column_int( stmt, 0 );

    ret = sqlite3_step( stmt );
  }
  sqlite3_finalize( stmt );

  return values;
}

QgsOfflineEditing::AttributeValueChanges QgsOfflineEditing::sqlQueryAttributeValueChanges( sqlite3 *db, const QString &sql )
{
  AttributeValueChanges values;

  sqlite3_stmt *stmt = nullptr;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    showWarning( sqlite3_errmsg( db ) );
    return values;
  }

  int ret = sqlite3_step( stmt );
  while ( ret == SQLITE_ROW )
  {
    AttributeValueChange change;
    change.fid = sqlite3_column_int( stmt, 0 );
    change.attr = sqlite3_column_int( stmt, 1 );
    change.value = QString( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 2 ) ) );
    values << change;

    ret = sqlite3_step( stmt );
  }
  sqlite3_finalize( stmt );

  return values;
}

QgsOfflineEditing::GeometryChanges QgsOfflineEditing::sqlQueryGeometryChanges( sqlite3 *db, const QString &sql )
{
  GeometryChanges values;

  sqlite3_stmt *stmt = nullptr;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    showWarning( sqlite3_errmsg( db ) );
    return values;
  }

  int ret = sqlite3_step( stmt );
  while ( ret == SQLITE_ROW )
  {
    GeometryChange change;
    change.fid = sqlite3_column_int( stmt, 0 );
    change.geom_wkt = QString( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 1 ) ) );
    values << change;

    ret = sqlite3_step( stmt );
  }
  sqlite3_finalize( stmt );

  return values;
}

void QgsOfflineEditing::committedAttributesAdded( const QString &qgisLayerId, const QList<QgsField> &addedAttributes )
{
  sqlite3_database_unique_ptr database = openLoggingDb();
  if ( !database )
    return;

  // insert log
  int layerId = getOrCreateLayerId( database.get(), qgisLayerId );
  int commitNo = getCommitNo( database.get() );

  for ( QList<QgsField>::const_iterator it = addedAttributes.begin(); it != addedAttributes.end(); ++it )
  {
    QgsField field = *it;
    QString sql = QStringLiteral( "INSERT INTO 'log_added_attrs' VALUES ( %1, %2, '%3', %4, %5, %6, '%7' )" )
                  .arg( layerId )
                  .arg( commitNo )
                  .arg( field.name() )
                  .arg( field.type() )
                  .arg( field.length() )
                  .arg( field.precision() )
                  .arg( field.comment() );
    sqlExec( database.get(), sql );
  }

  increaseCommitNo( database.get() );
}

void QgsOfflineEditing::committedFeaturesAdded( const QString &qgisLayerId, const QgsFeatureList &addedFeatures )
{
  sqlite3_database_unique_ptr database = openLoggingDb();
  if ( !database )
    return;

  // insert log
  int layerId = getOrCreateLayerId( database.get(), qgisLayerId );

  // get new feature ids from db
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( qgisLayerId );
  QgsDataSourceUri uri = QgsDataSourceUri( layer->source() );

  QString offlinePath = QgsProject::instance()->readPath( QgsProject::instance()->readEntry( PROJECT_ENTRY_SCOPE_OFFLINE, PROJECT_ENTRY_KEY_OFFLINE_DB_PATH ) );
  QString tableName;

  if ( !offlinePath.contains( ".gpkg" ) )
  {
    tableName = uri.table();
  }
  else
  {
    tableName = uri.param( offlinePath + "|layername" );
  }

  // only store feature ids
  QString sql = QStringLiteral( "SELECT ROWID FROM '%1' ORDER BY ROWID DESC LIMIT %2" ).arg( tableName ).arg( addedFeatures.size() );
  QList<int> newFeatureIds = sqlQueryInts( database.get(), sql );
  for ( int i = newFeatureIds.size() - 1; i >= 0; i-- )
  {
    QString sql = QStringLiteral( "INSERT INTO 'log_added_features' VALUES ( %1, %2 )" )
                  .arg( layerId )
                  .arg( newFeatureIds.at( i ) );
    sqlExec( database.get(), sql );
  }
}

void QgsOfflineEditing::committedFeaturesRemoved( const QString &qgisLayerId, const QgsFeatureIds &deletedFeatureIds )
{
  sqlite3_database_unique_ptr database = openLoggingDb();
  if ( !database )
    return;

  // insert log
  int layerId = getOrCreateLayerId( database.get(), qgisLayerId );

  for ( QgsFeatureIds::const_iterator it = deletedFeatureIds.begin(); it != deletedFeatureIds.end(); ++it )
  {
    if ( isAddedFeature( database.get(), layerId, *it ) )
    {
      // remove from added features log
      QString sql = QStringLiteral( "DELETE FROM 'log_added_features' WHERE \"layer_id\" = %1 AND \"fid\" = %2" ).arg( layerId ).arg( *it );
      sqlExec( database.get(), sql );
    }
    else
    {
      QString sql = QStringLiteral( "INSERT INTO 'log_removed_features' VALUES ( %1, %2)" )
                    .arg( layerId )
                    .arg( *it );
      sqlExec( database.get(), sql );
    }
  }
}

void QgsOfflineEditing::committedAttributeValuesChanges( const QString &qgisLayerId, const QgsChangedAttributesMap &changedAttrsMap )
{
  sqlite3_database_unique_ptr database = openLoggingDb();
  if ( !database )
    return;

  // insert log
  int layerId = getOrCreateLayerId( database.get(), qgisLayerId );
  int commitNo = getCommitNo( database.get() );

  for ( QgsChangedAttributesMap::const_iterator cit = changedAttrsMap.begin(); cit != changedAttrsMap.end(); ++cit )
  {
    QgsFeatureId fid = cit.key();
    if ( isAddedFeature( database.get(), layerId, fid ) )
    {
      // skip added features
      continue;
    }
    QgsAttributeMap attrMap = cit.value();
    for ( QgsAttributeMap::const_iterator it = attrMap.constBegin(); it != attrMap.constEnd(); ++it )
    {
      QString sql = QStringLiteral( "INSERT INTO 'log_feature_updates' VALUES ( %1, %2, %3, %4, '%5' )" )
                    .arg( layerId )
                    .arg( commitNo )
                    .arg( fid )
                    .arg( it.key() ) // attr
                    .arg( it.value().toString() ); // value
      sqlExec( database.get(), sql );
    }
  }

  increaseCommitNo( database.get() );
}

void QgsOfflineEditing::committedGeometriesChanges( const QString &qgisLayerId, const QgsGeometryMap &changedGeometries )
{
  sqlite3_database_unique_ptr database = openLoggingDb();
  if ( !database )
    return;

  // insert log
  int layerId = getOrCreateLayerId( database.get(), qgisLayerId );
  int commitNo = getCommitNo( database.get() );

  for ( QgsGeometryMap::const_iterator it = changedGeometries.begin(); it != changedGeometries.end(); ++it )
  {
    QgsFeatureId fid = it.key();
    if ( isAddedFeature( database.get(), layerId, fid ) )
    {
      // skip added features
      continue;
    }
    QgsGeometry geom = it.value();
    QString sql = QStringLiteral( "INSERT INTO 'log_geometry_updates' VALUES ( %1, %2, %3, '%4' )" )
                  .arg( layerId )
                  .arg( commitNo )
                  .arg( fid )
                  .arg( geom.asWkt() );
    sqlExec( database.get(), sql );

    // TODO: use WKB instead of WKT?
  }

  increaseCommitNo( database.get() );
}

void QgsOfflineEditing::startListenFeatureChanges()
{
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( sender() );
  // enable logging, check if editBuffer is not null
  if ( vLayer->editBuffer() )
  {
    QgsVectorLayerEditBuffer *editBuffer = vLayer->editBuffer();
    connect( editBuffer, &QgsVectorLayerEditBuffer::committedAttributesAdded,
             this, &QgsOfflineEditing::committedAttributesAdded );
    connect( editBuffer, &QgsVectorLayerEditBuffer::committedAttributeValuesChanges,
             this, &QgsOfflineEditing::committedAttributeValuesChanges );
    connect( editBuffer, &QgsVectorLayerEditBuffer::committedGeometriesChanges,
             this, &QgsOfflineEditing::committedGeometriesChanges );
  }
  connect( vLayer, &QgsVectorLayer::committedFeaturesAdded,
           this, &QgsOfflineEditing::committedFeaturesAdded );
  connect( vLayer, &QgsVectorLayer::committedFeaturesRemoved,
           this, &QgsOfflineEditing::committedFeaturesRemoved );
}

void QgsOfflineEditing::stopListenFeatureChanges()
{
  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( sender() );
  // disable logging, check if editBuffer is not null
  if ( vLayer->editBuffer() )
  {
    QgsVectorLayerEditBuffer *editBuffer = vLayer->editBuffer();
    disconnect( editBuffer, &QgsVectorLayerEditBuffer::committedAttributesAdded,
                this, &QgsOfflineEditing::committedAttributesAdded );
    disconnect( editBuffer, &QgsVectorLayerEditBuffer::committedAttributeValuesChanges,
                this, &QgsOfflineEditing::committedAttributeValuesChanges );
    disconnect( editBuffer, &QgsVectorLayerEditBuffer::committedGeometriesChanges,
                this, &QgsOfflineEditing::committedGeometriesChanges );
  }
  disconnect( vLayer, &QgsVectorLayer::committedFeaturesAdded,
              this, &QgsOfflineEditing::committedFeaturesAdded );
  disconnect( vLayer, &QgsVectorLayer::committedFeaturesRemoved,
              this, &QgsOfflineEditing::committedFeaturesRemoved );
}

void QgsOfflineEditing::layerAdded( QgsMapLayer *layer )
{
  // detect offline layer
  if ( layer->customProperty( CUSTOM_PROPERTY_IS_OFFLINE_EDITABLE, false ).toBool() )
  {
    QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );
    connect( vLayer, &QgsVectorLayer::editingStarted, this, &QgsOfflineEditing::startListenFeatureChanges );
    connect( vLayer, &QgsVectorLayer::editingStopped, this, &QgsOfflineEditing::stopListenFeatureChanges );
  }
}


