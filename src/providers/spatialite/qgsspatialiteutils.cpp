/***************************************************************************
     qgsspatialiteutils.cpp
     --------------------------------------
    Date                 : 2017-05-14
    Copyright         : (C) 2017 by Mark Johnson, Berlin Germany
    Email                : mj10777 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QStringList>
#include <QRegularExpression>
#include <QDebug>

#include "qgsspatialiteconnection.h"
#include "qgsslconnect.h"
#include <gdal.h>
#include "qgslogger.h"

//-----------------------------------------------------------
//  SpatialiteDbInfo
//-----------------------------------------------------------
const QString SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX = QStringLiteral( "json" );
const QString SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX = QStringLiteral( "list" );
QStringList SpatialiteDbInfo::mSpatialiteTypes;
SpatialiteDbInfo::~SpatialiteDbInfo()
{
  if ( ( mQSqliteHandle ) && ( isDbValid() ) && ( !isConnectionShared() ) )
  {
    QgsSqliteHandle::closeDb( mQSqliteHandle );
  }
  mSqliteHandle = nullptr;
  mQSqliteHandle = nullptr;
  for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mDbLayers.begin(); it != mDbLayers.end(); ++it )
  {
    SpatialiteDbLayer *dbLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
    delete dbLayer;
  }
};
SpatialiteDbInfo *SpatialiteDbInfo::CreateSpatialiteConnection( const QString sDatabaseFileName, bool bShared, QString sLayerName, bool bLoadLayers, SpatialSniff sniffType )
{
  SpatialiteDbInfo *spatialiteDbInfo = new SpatialiteDbInfo( sDatabaseFileName );
  // Checks if the File exists and is a Sqlite3 container [will not be created]
  if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbSqlite3() ) )
  {
    sqlite3 *sqlite_handle = nullptr;
    int open_flags = bShared ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
    if ( QgsSLConnect::sqlite3_open_v2( sDatabaseFileName.toUtf8().constData(), &sqlite_handle, open_flags, nullptr )  == SQLITE_OK )
    {
      QgsSqliteHandle *handle = new QgsSqliteHandle( sqlite_handle, sDatabaseFileName, bShared );
      if ( spatialiteDbInfo->attachQSqliteHandle( handle ) )
      {
        // Retrieve MetaData and all possible Layers if sLayerName is empty or Null
        if ( !spatialiteDbInfo->GetSpatialiteDbInfo( sLayerName, bLoadLayers, sniffType ) )
        {
          // For 'Drag and Drop' or 'Browser' logic:  Unknown (not supported) Sqlite3-Container (such as Fossil)
          QgsDebugMsg( QString( "Failure while connecting to: %1\n\nThe read Sqlite3 Container is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider. DbValid=%2" ).arg( sDatabaseFileName ).arg( spatialiteDbInfo->isDbValid() ) ); // sDbValid()==false
          // Allow this QgsSqliteHandle to be used for other purposes.
          // QgsSqliteHandle::openDb will return only the QgsSqliteHandle.
          // The Caller must close/delete it with QgsSqliteHandle::closeDb  manually
          return spatialiteDbInfo;
        }
        if ( spatialiteDbInfo->isDbGdalOgr() )
        {
          QgsDebugMsg( QString( "Connection to the database was successful [for QgsOgrProvider or QgsGdalProvider only] MetaType[%1]" ).arg( spatialiteDbInfo->dbSpatialMetadataString() ) );
        }
        else
        {
          QgsDebugMsg( QString( "Connection to the database was successful [for QgsSpatiaLiteProvider] MetaType[%1]" ).arg( spatialiteDbInfo->dbSpatialMetadataString() ) );
        }
        qDebug() << QString( "SpatialiteDbInfo::CreateSpatialiteConnection(%1,%2,%3) Layers-Loaded[%4] Layers-Found[%5] dbPath[%6] " ).arg( handle->getRef() ).arg( bShared ).arg( spatialiteDbInfo->isDbReadOnly() ).arg( spatialiteDbInfo->dbLayersCount() ).arg( spatialiteDbInfo->dbVectorLayersCount() ).arg( sDatabaseFileName );
      }
      else
      {
        QgsDebugMsg( QString( "Connectioncould not be attached to the SpatialiteDbInfo cause[%1]" ).arg( "sqlite3_db_readonly returned a invalid result" ) );
        // Will close the Connection
        delete spatialiteDbInfo;
        spatialiteDbInfo = nullptr;
      }
    }
    else
    {
      // A Sqlite3 file could not be opened with the sqlite3 driver
      QgsDebugMsg( QString( "Failure while connecting to: %1\n%2" ).arg( sDatabaseFileName, QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) ) ) );
    }
  }
  else
  {
    // For 'Drag and Drop' or 'Browser' logic: use another Provider to determineee non-Sqlite3 files
    QgsDebugMsg( QString( "Failure while connecting to: %1\nFile does not exist or is not a Sqlite3 container." ).arg( sDatabaseFileName ) );
  }
  return spatialiteDbInfo;
}
void SpatialiteDbInfo::setSpatialMetadata( int iSpatialMetadata )
{
  mSpatialMetadata = ( SpatialiteDbInfo::SpatialMetadata )iSpatialMetadata;
  if ( mSpatialMetadata == SpatialMetadata::SpatialUnknown )
  {
    if ( checkMBTiles() )
    {
      mSpatialMetadata = SpatialMetadata::SpatialiteMBTiles;
    }
  }
  if ( mSpatialMetadata == SpatialMetadata::SpatialiteFdoOgr )
  {
    if ( checkFdoOgr() )
    {
      // Not yet implemented
    }
  }
  mSpatialMetadataString = SpatialiteDbInfo::SpatialMetadataTypeName( mSpatialMetadata );
  switch ( mSpatialMetadata )
  {
    case SpatialMetadata::SpatialiteFdoOgr:
      mIsValid = false;
      break;
    case SpatialMetadata::SpatialiteGpkg:
    case SpatialMetadata::SpatialiteMBTiles:
      mIsValid = true;
      mIsGdalOgr = true;
      break;
    case SpatialMetadata::SpatialiteLegacy:
      mIsSpatialite = true;
      mIsValid = true;
      break;
    case SpatialMetadata::Spatialite40:
      mIsSpatialite = true;
      mIsSpatialite40 = true;
      mIsValid = true;
      break;
    case SpatialMetadata::Spatialite45:
      mIsSpatialite = true;
      mIsSpatialite40 = true;
      mIsSpatialite45 = true;
      mIsValid = true;
      break;
    case SpatialMetadata::SpatialUnknown:
    default:
      mIsValid = false;
      break;
  }
}
bool SpatialiteDbInfo::attachQSqliteHandle( QgsSqliteHandle *qSqliteHandle )
{
  if ( ( qSqliteHandle ) && ( qSqliteHandle->handle() ) )
  {
    if ( mDatabaseFileName == qSqliteHandle->dbPath() )
    {
      mQSqliteHandle = qSqliteHandle;
      setConnectionShared( mQSqliteHandle->getRef() );
      if ( !mSqliteHandle )
      {
        mSqliteHandle = mQSqliteHandle->handle();
      }
      else
      {
        if ( mSqliteHandle != mQSqliteHandle->handle() )
        {
          //     QString testfilename=QString::fromUtf8(( const char * ) sqlite3_db_filename(mQSqliteHandle->handle(, "main"));
          int i_rc = sqlite3_db_readonly( mQSqliteHandle->handle(), "main" );
          switch ( i_rc )
          {
            case 0:
              mReadOnly = false;
              break;
            case 1:
              mReadOnly = true;
              break;
            case -1:
            default:
              setDatabaseInvalid( QString( "attachQSqliteHandle: sqlite3_db_readonly failed." ) );
              mIsSqlite3 = false;
              break;
          }
          if ( mIsSqlite3 )
          {
            mSqliteHandle = mQSqliteHandle->handle();
          }
        }
      }
    }
    else
    {
      setDatabaseInvalid( QString( "attachQSqliteHandle: Database File mismatch." ) );
    }
  }
  else
  {
    setDatabaseInvalid( QString( "attachQSqliteHandle: other errors." ) );
  }
  if ( mIsSqlite3 )
  {
    mQSqliteHandle->setSpatialiteDbInfo( this );
  }
  return mIsSqlite3;
}
SpatialiteDbInfo::TypeSubType SpatialiteDbInfo::GetVariantType( const QString &type )
{
  // making some assumptions in order to guess a more realistic type
  if ( type == QLatin1String( "int" ) ||
       type == QLatin1String( "integer" ) ||
       type == QLatin1String( "integer64" ) ||
       type == QLatin1String( "bigint" ) ||
       type == QLatin1String( "smallint" ) ||
       type == QLatin1String( "tinyint" ) ||
       type == QLatin1String( "boolean" ) )
    return TypeSubType( QVariant::LongLong, QVariant::Invalid );
  else if ( type == QLatin1String( "real" ) ||
            type == QLatin1String( "double" ) ||
            type == QLatin1String( "double precision" ) ||
            type == QLatin1String( "float" ) )
    return TypeSubType( QVariant::Double, QVariant::Invalid );
  else if ( type == QLatin1String( "date" ) )
    return TypeSubType( QVariant::Date, QVariant::Invalid );
  else if ( type == QLatin1String( "datetime" ) )
    return TypeSubType( QVariant::DateTime, QVariant::Invalid );
  else if ( type.startsWith( SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX ) && type.endsWith( SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX ) )
  {
    // New versions of OGR convert list types (StringList, IntegerList, Integer64List and RealList)
    // to JSON when it stores a Spatialite table. It sets the column type as JSONSTRINGLIST,
    // JSONINTEGERLIST, JSONINTEGER64LIST or JSONREALLIST
    TypeSubType subType = SpatialiteDbInfo::GetVariantType( type.mid( SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.length(),
                          type.length() - SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.length() - SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.length() ) );
    return TypeSubType( subType.first == QVariant::String ? QVariant::StringList : QVariant::List, subType.first );
  }
  else
    // for sure any SQLite value can be represented as SQLITE_TEXT
    return TypeSubType( QVariant::String, QVariant::Invalid );
}
QStringList SpatialiteDbInfo::getSpatialiteTypes()
{
  if ( mSpatialiteTypes.count() == 0 )
  {
    mSpatialiteTypes.append( "SpatialIndex-Data" );
    mSpatialiteTypes.append( "Spatialite-Internal" );
    mSpatialiteTypes.append( "Spatialite-Virtual" );
    mSpatialiteTypes.append( "Spatialite2-Internal" );
    mSpatialiteTypes.append( "Geometries-Internal" );
    mSpatialiteTypes.append( "SpatialTable-Data" );
    mSpatialiteTypes.append( "SpatialTable-Internal" );
    mSpatialiteTypes.append( "SpatialView-Data" );
    mSpatialiteTypes.append( "SpatialView-Internal" );
    mSpatialiteTypes.append( "VirtualShapes-Data" );
    mSpatialiteTypes.append( "VirtualShapes-Internal" );
    mSpatialiteTypes.append( "RasterLite1-Internal" );
    mSpatialiteTypes.append( "RasterLite2-Internal" );
    mSpatialiteTypes.append( "Styles-Internal" );
    mSpatialiteTypes.append( "Topology-Internal" );
    mSpatialiteTypes.append( "Sqlite3-Internal" );
    mSpatialiteTypes.append( "Trigger-Data" );
    mSpatialiteTypes.append( "EPSG-Table" );
    mSpatialiteTypes.append( "EPSG-Internal" );
    mSpatialiteTypes.append( "Table-Data" );
    mSpatialiteTypes.append( "View-Data" );
    mSpatialiteTypes.append( "GeoPackage-Raster-Data" );
    mSpatialiteTypes.append( "GeoPackage-Vector-Data" );
    mSpatialiteTypes.append( "GeoPackage-Internal" );
    mSpatialiteTypes.append( "MBTiles-Raster" );
    mSpatialiteTypes.append( "MBTiles-Index" );
    mSpatialiteTypes.append( "MBTiles-Internal" );
  }
  return mSpatialiteTypes;
}
QIcon SpatialiteDbInfo::NonSpatialTablesTypeIcon( QString typeName )
{
  if ( typeName.startsWith( QLatin1String( "Table" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTable.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "View" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "SpatialTable" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "SpatialView" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNodeTool.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "VirtualShapes" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererPointDisplacementSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Virtual" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererGraduatedSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "SpatialIndex" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionGroupItems.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Geometries" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddWfsLayer.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "RasterLite2" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorSwatches.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "RasterLite1" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconColorBox.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "MBTiles" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "GeoPackage-Raster" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "GeoPackage-Vector" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "EPSG" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Trigger" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionMergeFeatureAttributes.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Sqlite3" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionDataSourceManager.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Topology" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererPointClusterSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Styles" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/rendererRuleBasedSymbol.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Spatialite2" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddLegend.svg" ) );
  }
  else if ( typeName.startsWith( QLatin1String( "Spatialite" ), Qt::CaseInsensitive ) )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) );
  }
  else
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "/multieditChangedValues.svg" ) );
  }
}
QString SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  //----------------------------------------------------------
  switch ( layerType )
  {
    case SpatialiteDbInfo::SpatialTable:
      return QStringLiteral( "SpatialTable" );
    case SpatialiteDbInfo::SpatialView:
      return QStringLiteral( "SpatialView" );
    case SpatialiteDbInfo::VirtualShape:
      return QStringLiteral( "VirtualShape" );
    case SpatialiteDbInfo::RasterLite1:
      return QStringLiteral( "RasterLite1" );
    case SpatialiteDbInfo::RasterLite2:
      return QStringLiteral( "RasterLite2" );
    case SpatialiteDbInfo::SpatialiteTopopogy:
      return QStringLiteral( "SpatialiteTopopogy" );
    case SpatialiteDbInfo::TopopogyExport:
      return QStringLiteral( "TopopogyExport" );
    case SpatialiteDbInfo::VectorStyle:
      return QStringLiteral( "VectorStyle" );
    case SpatialiteDbInfo::RasterStyle:
      return QStringLiteral( "RasterStyle" );
    case SpatialiteDbInfo::GeoPackageVector:
      return QStringLiteral( "GeoPackageVector" );
    case SpatialiteDbInfo::GeoPackageRaster:
      return QStringLiteral( "GeoPackageRaster" );
    case SpatialiteDbInfo::MBTilesTable:
      return QStringLiteral( "MBTilesTable" );
    case SpatialiteDbInfo::MBTilesView:
      return QStringLiteral( "MBTilesView" );
    case SpatialiteDbInfo::Metadata:
      return QStringLiteral( "Metadata" );
    case SpatialiteDbInfo::AllSpatialLayers:
      return QStringLiteral( "AllSpatialLayers" );
    case SpatialiteDbInfo::NonSpatialTables:
      return QStringLiteral( "NonSpatialTables" );
    case SpatialiteDbInfo::AllLayers:
      return QStringLiteral( "AllLayers" );
    case SpatialiteDbInfo::SpatialiteUnknown:
    default:
      return QStringLiteral( "SpatialiteUnknown" );
  }
  //----------------------------------------------------------
}
QIcon SpatialiteDbInfo::SpatialiteLayerTypeIcon( SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  switch ( layerType )
  {
    case SpatialiteDbInfo::AllSpatialLayers:
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::VirtualShape:
    case SpatialiteDbInfo::GdalFdoOgr:
    case SpatialiteDbInfo::GeoPackageVector:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewVectorLayer.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialView:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionNodeTool.svg" ) );
      break;
    case SpatialiteDbInfo::RasterLite1:
    case SpatialiteDbInfo::RasterLite2:
    case SpatialiteDbInfo::GeoPackageRaster:
    case SpatialiteDbInfo::MBTilesTable:
    case SpatialiteDbInfo::MBTilesView:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMap.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialiteTopopogy:
      return QgsApplication::getThemeIcon( QStringLiteral( "/rendererPointClusterSymbol.svg" ) );
      break;
    case SpatialiteDbInfo::TopopogyExport:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTopologicalEditing.svg" ) );
      break;
    case SpatialiteDbInfo::VectorStyle:
    case SpatialiteDbInfo::RasterStyle:
      return QgsApplication::getThemeIcon( QStringLiteral( "/rendererRuleBasedSymbol.svg" ) );
      break;
    case SpatialiteDbInfo::NonSpatialTables:
    case SpatialiteDbInfo::AllLayers:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconListView.png" ) );
      break;
    default:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
      break;
  }
}
QIcon SpatialiteDbInfo::SpatialMetadataTypeIcon( SpatialiteDbInfo::SpatialMetadata spatialMetadata )
{
  switch ( spatialMetadata )
  {
    case SpatialiteDbInfo::SpatialiteLegacy:
    case SpatialiteDbInfo::Spatialite40:
    case SpatialiteDbInfo::Spatialite45:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialiteFdoOgr:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddOgrLayer.svg" ) );
      break;
    case SpatialiteDbInfo::SpatialiteGpkg:
    case SpatialiteDbInfo::SpatialiteMBTiles:
      return QgsApplication::getThemeIcon( QStringLiteral( "/providerGdal.svg" ) );
      break;
    default:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
      break;
  }
}

QIcon SpatialiteDbInfo::SpatialGeometryTypeIcon( QgsWkbTypes::Type geometryType )
{
  switch ( geometryType )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddNodesItem.svg" ) );
      break;
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolyline.svg" ) );
      break;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPolygon.svg" ) );
      break;
    default:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterGroup.svg" ) );
      break;
  }
}
QString SpatialiteDbInfo::SpatialIndexTypeName( SpatialiteDbInfo::SpatialIndexType spatialIndexType )
{
  //----------------------------------------------------------
  switch ( spatialIndexType )
  {
    case SpatialiteDbInfo::SpatialIndexRTree:
      return QStringLiteral( "SpatialIndexRTree" );
    case SpatialiteDbInfo::SpatialIndexMbrCache:
      return QStringLiteral( "SpatialIndexMbrCache" );
    case SpatialiteDbInfo::SpatialIndexNone:
    default:
      return QStringLiteral( "SpatialIndexNone" );
  }
}
SpatialiteDbInfo::SpatialIndexType SpatialiteDbInfo::SpatiaIndexTypeFromName( const QString &typeName )
{
  //----------------------------------------------------------
  if ( QString::compare( typeName, "SpatialIndexRTree", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialIndexRTree;
  }
  else if ( QString::compare( typeName, "SpatialIndexMbrCache", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialIndexMbrCache;
  }
  else
  {
    return SpatialiteDbInfo::SpatialIndexNone;
  }
}
QString SpatialiteDbInfo::SpatialMetadataTypeName( SpatialiteDbInfo::SpatialMetadata spatialMetadataType )
{
  //----------------------------------------------------------
  switch ( spatialMetadataType )
  {
    case SpatialiteDbInfo::SpatialiteLegacy:
      return QStringLiteral( "SpatialiteLegacyr" );
    case SpatialiteDbInfo::SpatialiteFdoOgr:
      return QStringLiteral( "SpatialiteFdoOgr" );
    case SpatialiteDbInfo::Spatialite40:
      return QStringLiteral( "Spatialite40" );
    case SpatialiteDbInfo::SpatialiteGpkg:
      return QStringLiteral( "SpatialiteGpkg" );
    case SpatialiteDbInfo::Spatialite45:
      return QStringLiteral( "Spatialite45" );
    case SpatialiteDbInfo::SpatialiteMBTiles:
      return QStringLiteral( "SpatialiteMBTiles" );
    case SpatialiteDbInfo::SpatialUnknown:
    default:
      return QStringLiteral( "SpatialUnknown" );
  }
}
SpatialiteDbInfo::SpatialMetadata SpatialiteDbInfo::SpatialMetadataTypeFromName( const QString &typeName )
{
  //----------------------------------------------------------
  if ( QString::compare( typeName, "SpatialiteLegacyr", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteLegacy;
  }
  else if ( QString::compare( typeName, "SpatialiteFdoOgr", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteFdoOgr;
  }
  else if ( QString::compare( typeName, "Spatialite40", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::Spatialite40;
  }
  else if ( QString::compare( typeName, "SpatialiteGpkg", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteGpkg;
  }
  else if ( QString::compare( typeName, "Spatialite45", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::Spatialite45;
  }
  else if ( QString::compare( typeName, "SpatialiteMBTiles", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteMBTiles;
  }
  else
  {
    return SpatialiteDbInfo::SpatialUnknown;
  }
}
SpatialiteDbInfo::SpatialiteLayerType SpatialiteDbInfo::SpatialiteLayerTypeFromName( const QString &typeName )
{
  //----------------------------------------------------------
  if ( QString::compare( typeName, "SpatialTable", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialTable;
  }
  else if ( QString::compare( typeName, "SpatialView", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialView;
  }
  else if ( QString::compare( typeName, "VirtualShape", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::VirtualShape;
  }
  else if ( QString::compare( typeName, "RasterLite1", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::RasterLite1;
  }
  else if ( QString::compare( typeName, "RasterLite2", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::RasterLite2;
  }
  else if ( QString::compare( typeName, "SpatialiteTopopogy", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::SpatialiteTopopogy;
  }
  else if ( QString::compare( typeName, "TopopogyExport", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::TopopogyExport;
  }
  else if ( QString::compare( typeName, "VectorStyle", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::VectorStyle;
  }
  else if ( QString::compare( typeName, "RasterStyle", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::RasterStyle;
  }
  else if ( QString::compare( typeName, "GeoPackageVector", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::GeoPackageVector;
  }
  else if ( QString::compare( typeName, "GeoPackageRaster", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::GeoPackageRaster;
  }
  else if ( QString::compare( typeName, "MBTilesTable", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::MBTilesTable;
  }
  else if ( QString::compare( typeName, "MBTilesView", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::MBTilesView;
  }
  else if ( QString::compare( typeName, "NonSpatialTables", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::NonSpatialTables;
  }
  else if ( QString::compare( typeName, "Metadata", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::Metadata;
  }
  else if ( QString::compare( typeName, "AllSpatialLayers", Qt::CaseInsensitive ) == 0 )
  {
    return SpatialiteDbInfo::AllSpatialLayers;
  }
  else
  {
    return SpatialiteDbInfo::SpatialiteUnknown;
  }
  //----------------------------------------------------------
}
int SpatialiteDbInfo::setDatabaseInvalid( QString errCause )
{
  mIsValid = false;
  if ( !errCause.isEmpty() )
  {
    mErrors.append( errCause );
  }
  return errCause.size();
}
bool SpatialiteDbInfo::prepare()
{
  bool bChanged = false;
//----------------------------------------------------------
  // Set (what up to now was the LayerId-counter) to the found valid Layer-Count
  mLayersCount = mDbLayers.size();
  bChanged = prepareDataSourceUris();
  mVectorLayersMissing.clear();
  if ( dbLayersCount() != getDbVectorLayers().count() )
  {
    bChanged = prepareVectorLayersMissing();
  }
//----------------------------------------------------------
  return bChanged;
};
QString SpatialiteDbInfo::getDbLayerUris( QString sLayerName )
{
  QList<QString> sa_list = mDbLayersDataSourceUris.keys( sLayerName );
  if ( sa_list.size() )
  {
    return sa_list.at( 0 );
  }
  return QString();
}
bool SpatialiteDbInfo::prepareVectorLayersMissing()
{
  bool bChanged = false;
  //----------------------------------------------------------
  for ( QMap<QString, QString>::iterator itLayers = mVectorLayers.begin(); itLayers != mVectorLayers.end(); ++itLayers )
  {
    if ( !getSpatialiteDbLayer( itLayers.key(), false ) )
    {
      // 'table_name(geometry_name)';Layer-Type
      mVectorLayersMissing.append( QString( "%1;%2" ).arg( itLayers.key() ).arg( itLayers.value() ) );
    }
  }
  if ( mVectorLayersMissing.size() > 0 )
  {
    bChanged = true;
  }
  //----------------------------------------------------------
  return bChanged;
};
//----------------------------------------------------------
bool SpatialiteDbInfo::prepareDataSourceUris()
{
  bool bChanged = false;
  //----------------------------------------------------------
  mDbLayersDataSourceUris.clear();
  // dbConnectionInfo() will insure that the Path-Name is properly formatted
  QString sDataSourceUriBase = QString( "%1 table=\"%2\"" );
  QString sDataSourceUri;
  for ( QMap<QString, QString>::iterator itLayers = mVectorLayers.begin(); itLayers != mVectorLayers.end(); ++itLayers )
  {
    QString sLayerName  = itLayers.key();
    QString sTableName  = sLayerName;
    QString sGeometryColumn = QString::null;
    if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
    {
      // Extract GeometryName from sent 'table_name(field_name)' from layerName
      QStringList sa_list_name = sTableName.split( "(" );
      sGeometryColumn = sa_list_name[1].replace( ")", "" );
      sTableName = sa_list_name[0];
    }
    sDataSourceUri = QString( sDataSourceUriBase.arg( dbConnectionInfo() ).arg( sTableName ) );
    if ( !sGeometryColumn.isEmpty() )
    {
      sDataSourceUri += QString( " (%1)" ).arg( sGeometryColumn );
    }
    mDbLayersDataSourceUris.insert( sLayerName, sDataSourceUri );
  }
  for ( QMap<QString, QString>::iterator itLayers = mRasterLite2Layers.begin(); itLayers != mRasterLite2Layers.end(); ++itLayers )
  {
    QString sLayerName  = itLayers.key();
    QString sTableName  = sLayerName;
    QString sGeometryColumn = QString::null;
    if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
    {
      // Extract GeometryName from sent 'table_name(field_name)' from layerName
      QStringList sa_list_name = sTableName.split( "(" );
      sGeometryColumn = sa_list_name[1].replace( ")", "" );
      sTableName = sa_list_name[0];
    }
    sDataSourceUri = QString( sDataSourceUriBase.arg( dbConnectionInfo() ).arg( sTableName ) );
    if ( !sGeometryColumn.isEmpty() )
    {
      sDataSourceUri += QString( " (%1)" ).arg( sGeometryColumn );
    }
    mDbLayersDataSourceUris.insert( sLayerName, sDataSourceUri );
  }
  for ( QMap<QString, QString>::iterator itLayers = mTopologyExportLayers.begin(); itLayers != mTopologyExportLayers.end(); ++itLayers )
  {
    QString sLayerName  = itLayers.key();
    QString sTableName  = sLayerName;
    QString sGeometryColumn = QString::null;
    if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
    {
      // Extract GeometryName from sent 'table_name(field_name)' from layerName
      QStringList sa_list_name = sTableName.split( "(" );
      sGeometryColumn = sa_list_name[1].replace( ")", "" );
      sTableName = sa_list_name[0];
    }
    sDataSourceUri = QString( sDataSourceUriBase.arg( dbConnectionInfo() ).arg( sTableName ) );
    if ( !sGeometryColumn.isEmpty() )
    {
      sDataSourceUri += QString( " (%1)" ).arg( sGeometryColumn );
    }
    mDbLayersDataSourceUris.insert( sLayerName, sDataSourceUri );
  }
  for ( QMap<QString, QString>::iterator itLayers = mRasterLite1Layers.begin(); itLayers != mRasterLite1Layers.end(); ++itLayers )
  {
    QString sLayerName  = itLayers.key();
    QString sTableName  = sLayerName;
    QString sGeometryColumn = QString::null;
    // this should be sent to the Gdal-Rasterlite(1)-Driver
    // RASTERLITE:/long/path/to/database/ItalyRail.atlas,table=srtm
    sDataSourceUriBase = QString( "%1,table=%2" );
    if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
    {
      // Extract GeometryName from sent 'table_name(field_name)' from layerName
      QStringList sa_list_name = sTableName.split( "(" );
      sGeometryColumn = sa_list_name[1].replace( ")", "" );
      sTableName = sa_list_name[0];
    }
    sDataSourceUri = QString( sDataSourceUriBase.arg( QString( "%2:%1" ).arg( mDatabaseFileName ).arg( "RASTERLITE" ) ).arg( sTableName ) );
    if ( !sGeometryColumn.isEmpty() )
    {
      sDataSourceUri += QString( " (%1)" ).arg( sGeometryColumn );
    }
    mDbLayersDataSourceUris.insert( sLayerName, sDataSourceUri );
  }
  if ( mDbLayersDataSourceUris.size() > 0 )
  {
    bChanged = true;
    mIsValid = true;
  }
  //----------------------------------------------------------
  return bChanged;
};
QMap<QString, QString> SpatialiteDbInfo::getDbLayersType( SpatialiteLayerType typeLayer )
{
  QMap<QString, QString> mapLayers;
  //----------------------------------------------------------
  switch ( typeLayer )
  {
    case SpatialiteDbInfo::SpatialTable:
    case SpatialiteDbInfo::SpatialView:
    case SpatialiteDbInfo::VirtualShape:
    {
      // Key : LayerType SpatialTable, SpatialView and VirtualShape
      // Value: LayerName formatted as 'table_name(geometry_name)'
      QList<QString> sa_list = mVectorLayersTypes.keys( SpatialiteDbInfo::SpatialiteLayerTypeName( typeLayer ) );
      for ( int i = 0; i < sa_list.size(); ++i )
      {
        // Return a list that only contains the given type
        mapLayers.insert( sa_list.at( i ), mVectorLayers.value( sa_list.at( i ) ) );
      }
    }
    break;
    case SpatialiteDbInfo::RasterLite1:
      mapLayers = getDbRasterLite1Layers();
      break;
    case SpatialiteDbInfo::RasterLite2:
      mapLayers = getDbRasterLite2Layers();
      break;
    case SpatialiteDbInfo::TopopogyExport:
      mapLayers = getDbTopologyExportLayers();
      break;
    case SpatialiteDbInfo::GeoPackageVector:
    case SpatialiteDbInfo::GeoPackageRaster:
      mapLayers = getDbGeoPackageLayers();
      break;
    case SpatialiteDbInfo::MBTilesTable:
    case SpatialiteDbInfo::MBTilesView:
      mapLayers = getDbMBTilesLayers();
      break;
    case SpatialiteDbInfo::NonSpatialTables:
      mapLayers = getDbNonSpatialTables();
      break;
    case SpatialiteDbInfo::AllSpatialLayers:
    case SpatialiteDbInfo::AllLayers:
    {
      mapLayers = getDbVectorLayers();
      if ( dbRasterLite2LayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbRasterLite2Layers().begin(); itLayers != getDbRasterLite2Layers().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbRasterLite1LayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbRasterLite1Layers().begin(); itLayers != getDbRasterLite1Layers().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbTopologyExportLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbTopologyExportLayers().begin(); itLayers != getDbTopologyExportLayers().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbGeoPackageLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbGeoPackageLayers().begin(); itLayers != getDbGeoPackageLayers().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbFdoOgrLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbFdoOgrLayers().begin(); itLayers != getDbFdoOgrLayers().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( dbMBTilesLayersCount() > 0 )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbMBTilesLayers().begin(); itLayers != getDbMBTilesLayers().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
      if ( ( typeLayer == SpatialiteDbInfo::AllLayers ) && ( dbNonSpatialTablesCount() > 0 ) )
      {
        for ( QMap<QString, QString>::iterator itLayers = getDbNonSpatialTables().begin(); itLayers != getDbNonSpatialTables().end(); ++itLayers )
        {
          mapLayers.insert( itLayers.key(), itLayers.value() );
        }
      }
    }
    break;
    default:
      break;
  }
  //----------------------------------------------------------
  return mapLayers;
};
SpatialiteDbLayer *SpatialiteDbInfo::getSpatialiteDbLayer( QString sLayerName, bool loadLayer )
{
  SpatialiteDbLayer *dbLayer = nullptr;
  //----------------------------------------------------------
  if ( ! mDbLayers.contains( sLayerName ) )
  {
    if ( mDbLayersDataSourceUris.contains( sLayerName ) )
    {
      // a DataSourceUris is being sent as LayerName, replace with Layer-Name
      sLayerName = mDbLayersDataSourceUris.value( sLayerName );
    }
  }
  if ( !sLayerName.isEmpty() )
  {
    // Note: for some reason this is being called with the uri ; extranct the LayerName in such cases
    if ( ( sLayerName.contains( "dbname=" ) ) && ( sLayerName.contains( "table=" ) ) )
    {
      QgsDataSourceUri anUri = QgsDataSourceUri( sLayerName );
      sLayerName = anUri.table();
      if ( !anUri.geometryColumn().toLower().isEmpty() )
      {
        sLayerName = QString( "%1(%2)" ).arg( sLayerName ).arg( anUri.geometryColumn().toLower() );
      }
    }
  }
  //----------------------------------------------------------
  if ( mDbLayers.contains( sLayerName ) )
  {
    dbLayer = qobject_cast<SpatialiteDbLayer *>( mDbLayers.value( sLayerName ) );
    if ( dbLayer )
    {
      qDebug() << QString( "-I-> SpatialiteDbInfo::getSpatialiteDbLayer -returning found Layer- [%1] Layer-Name[%2] GeometryType[%3] Features[%4]" ).arg( dbLayer->getLayerTypeString() ).arg( dbLayer->getLayerName() ).arg( dbLayer->getGeometryTypeString() ).arg( dbLayer->getNumberFeatures() );
      return dbLayer;
    }
  }
  //----------------------------------------------------------
  if ( loadLayer )
  {
    if ( GetDbLayersInfo( sLayerName ) )
    {
      dbLayer = getSpatialiteDbLayer( sLayerName, false );
      if ( dbLayer )
      {
        qDebug() << QString( "-I-> SpatialiteDbInfo::getSpatialiteDbLayer -returning found Layer after Loading- [%1] Layer-Name[%2] GeometryType[%3] Features[%4]" ).arg( dbLayer->getLayerTypeString() ).arg( dbLayer->getLayerName() ).arg( dbLayer->getGeometryTypeString() ).arg( dbLayer->getNumberFeatures() );
        return dbLayer;
      }
    }
    // bridge_segments_1938" (soldner_segment)
    // -E->SpatialiteDbLayer::GetLayerSettings -4- IsValid[0] Layer-Type[SpatialView] Layer-Name[street_segments_1884(soldner_segment)] sql[SELECT view_rowid, f_table_name, f_geometry_column FROM views_geometry_columns WHERE ((view_name=\"street_segments_1884\") AND (view_geometry=\"soldner_segment\"))]
    // -E-> SpatialiteDbInfo::GetDbLayersInfo(1) LayerType[SpatialView] Layername[street_segments_1884(soldner_segment)]"
    // -E-> 1): [-E-> SpatialiteDbLayer::setSpatialiteAttributeFields[SpatialView] Layername[street_segments_1884(soldner_segment)] PrimaryKey[id_segment] from 'views_geometry_columns',  was not found.]

    qDebug() << QString( "-E-> SpatialiteDbInfo::getSpatialiteDbLayer -Layer not resolved-  Layer-Name[%1] not found. Layers-Loaded[%2] Layers-Found[%3]" ).arg( sLayerName ).arg( mDbLayers.count() ).arg( dbVectorLayersCount() );
#if 0
    for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mDbLayers.begin(); it != mDbLayers.end(); ++it )
    {
      SpatialiteDbLayer *dbLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
      if ( dbLayer )
      {
        qDebug() << QString( "-W-> SpatialiteDbInfo:: -debug- key[%1] [%2] Layer-Name[%3] GeometryType[%4] Features[%5]" ).arg( it.key() ).arg( dbLayer->getLayerTypeString() ).arg( dbLayer->getLayerName() ).arg( dbLayer->getGeometryTypeString() ).arg( dbLayer->getNumberFeatures() );
      }
    }
#endif
    dbLayer = nullptr;
  }
  //----------------------------------------------------------
  return dbLayer;
};
SpatialiteDbLayer *SpatialiteDbInfo::getSpatialiteDbLayer( int iLayerId )
{
  SpatialiteDbLayer *dbLayer = nullptr;
  //----------------------------------------------------------
  for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mDbLayers.begin(); it != mDbLayers.end(); ++it )
  {
    SpatialiteDbLayer *searchLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
    if ( searchLayer )
    {
      if ( searchLayer->mLayerId == iLayerId )
      {
        return searchLayer;
      }
    }
  }
  //----------------------------------------------------------
  return dbLayer;
};
//----------------------------------------------------------
bool SpatialiteDbInfo::getSniffDatabaseType( )
{
  // -- ---------------------------------- --
  // -- This must be a Sqlite3 Database
  // -- ---------------------------------- --
  if ( mIsSqlite3 )
  {
    sqlite3_stmt *stmt = nullptr;
    int i_rc = sqlite3_db_readonly( mSqliteHandle, "main" );
    switch ( i_rc )
    {
      case 0:
        mReadOnly = false;
        break;
      case 1:
        mReadOnly = true;
        break;
      case -1:
      default:
        setDatabaseInvalid( QString( "GetSpatialiteDbInfo: sqlite3_db_readonly failed." ) );
        break;
    }
    QString sql = QStringLiteral( "SELECT CheckSpatialMetadata(), spatialite_version()" );
    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          // -- ---------------------------------- --
          // Check and set known values
          // -> considered valid
          // - if not known, will check for MBTiles
          // -> if found considered valid since MBTiles is supported by QgsGdalProvider
          // -- ---------------------------------- --
          setSpatialMetadata( sqlite3_column_int( stmt, 0 ) );
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          mSpatialiteVersionInfo = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        }
      }
      sqlite3_finalize( stmt );
      if ( !mIsValid )
      {
        setDatabaseInvalid( QString( "GetSpatialiteDbInfo: Invalid SpatialMetadata[%1]." ).arg( dbSpatialMetadataString() ) );
        return mIsValid;
      }
    }
    QString sVersionInfo = mSpatialiteVersionInfo;
    sVersionInfo.remove( QRegExp( QString::fromUtf8( "[-`~!@#$%^&*()_—+=|:;<>«»,?/{a-zA-Z}\'\"\\[\\]\\\\]" ) ) );
    QStringList spatialiteParts = sVersionInfo.split( ' ', QString::SkipEmptyParts );
    // Get major, minor and revision version
    QStringList spatialiteVersionParts = spatialiteParts[0].split( '.', QString::SkipEmptyParts );
    if ( spatialiteVersionParts.size() == 3 )
    {
      mSpatialiteVersionMajor = spatialiteVersionParts[0].toInt();
      mSpatialiteVersionMinor = spatialiteVersionParts[1].toInt();
      mSpatialiteVersionRevision = spatialiteVersionParts[2].toInt();
    }
    if ( mSpatialiteVersionMajor < 0 )
    {
      setDatabaseInvalid( QString( "GetSpatialiteDbInfo: Invalid spatialite Major version[%1]." ).arg( mSpatialiteVersionMajor ) );
    }
    else
    {
      if ( mSpatialiteVersionMajor >= 4 )
      {
        if ( ( ( mSpatialiteVersionMajor == 4 ) && ( mSpatialiteVersionMinor >= 5 ) ) || ( mSpatialiteVersionMajor > 4 ) )
        {
          mIsVersion45 = true;
        }
      }
    }
  }
  return mIsValid;
}
//----------------------------------------------------------
bool SpatialiteDbInfo::getSniffSniffMinimal( )
{
  //----------------------------------------------------------
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT " );
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master where name = 'raster_coverages') =0) THEN -1 ELSE 0  END)," );
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master where name = 'topologies') =0) THEN -1 ELSE 0  END)," );
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master where name = 'data_licenses') =0) THEN -1 ELSE 5  END)," );
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master where name = 'layer_statistics') =0) THEN -1 ELSE 0  END)," );
    sql += QStringLiteral( "(SELECT CASE WHEN (Exists(SELECT rootpage FROM sqlite_master where name = 'vector_layers') =0) THEN -1 ELSE 0  END)," );
    sql += QStringLiteral( "(SELECT tbl_name FROM sqlite_master WHERE (type = 'table' AND tbl_name='gpkg_contents'))" );
    // SELECT count(tbl_name) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name = 'metadata' OR  tbl_name = 'map' OR tbl_name='images' OR tbl_name='view'))
    // SELECT count(tbl_name) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name IN ('metadata','tiles','images','map')))
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          mHasRasterLite2Tables = sqlite3_column_int( stmt, 0 );
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          mHasTopologyExportTables = sqlite3_column_int( stmt, 1 );
        }
        if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 2 ) == 5 )
          {
            setSpatialMetadata( ( int )Spatialite45 );
          }
        }
        if ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL )
        {
          mHasRasterLite1Tables = sqlite3_column_int( stmt, 3 );
        }
        if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
        {
          mHasVectorLayers = sqlite3_column_int( stmt, 4 );
        }
        if ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL )
        {
          mHasGeoPackageTables = sqlite3_column_int( stmt, 5 );
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( mIsVersion45 )
    {
      sql = QStringLiteral( "SELECT HasTopology(), HasGCP()" ) ;
      i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasTopology = sqlite3_column_int( stmt, 0 );
          }
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            mHasGcp = sqlite3_column_int( stmt, 1 );
          }
        }
        sqlite3_finalize( stmt );
      }
    }
  }
  return mIsValid;
}
//----------------------------------------------------------
bool SpatialiteDbInfo::getSniffLayerMetadata( )
{
  //----------------------------------------------------------
  if ( mIsValid )
  {
    QString sql;
    int i_rc;
    sqlite3_stmt *stmt = nullptr;
    // SELECT count(tbl_name) FROM sqlite_master  WHERE ((type = 'table' OR type = 'view') AND (tbl_name = 'metadata' OR  tbl_name = 'map' OR tbl_name='images' OR tbl_name='view'))
    if ( mHasRasterLite1Tables == 0 )
    {
      sql = QStringLiteral( "SELECT " );
      // truemarble-4km.sqlite
      sql += QStringLiteral( "(SELECT count(raster_layer) FROM 'layer_statistics' WHERE (raster_layer=1))," );
      sql += QStringLiteral( "(SELECT group_concat(table_name,',') FROM 'layer_statistics' WHERE ((raster_layer=0) AND (table_name LIKE '%_metadata')))" );
      int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        QString rl1Tables;
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasRasterLite1Tables = sqlite3_column_int( stmt, 0 );
          }
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            rl1Tables = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          }
        }
        sqlite3_finalize( stmt );
        if ( !rl1Tables.isEmpty() )
        {
          QStringList rl1Parts = rl1Tables.split( ',', QString::SkipEmptyParts );
          for ( int i = 0; i < rl1Parts.size(); i++ )
          {
            // TrueMarble_metadata,srtm_metadata
            sql = QStringLiteral( "SELECT count(id) FROM '%1' WHERE ((id=0) AND (source_name = 'raster metadata'))" ).arg( rl1Parts.at( i ) );
            i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
            if ( i_rc == SQLITE_OK )
            {
              while ( sqlite3_step( stmt ) == SQLITE_ROW )
              {
                if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
                {
                  if ( sqlite3_column_int( stmt, 0 ) == 1 )
                  {
                    mHasRasterLite1Tables++;
                  }
                }
              }
              sqlite3_finalize( stmt );
            }
          }
        }
      }
    }
    if ( mHasVectorLayers == 0 )
    {
      sql = QStringLiteral( "SELECT count(table_name) FROM 'vector_layers'" );
      i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasVectorLayers = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
    }
    if ( mHasRasterLite2Tables == 0 )
    {
      sql = QStringLiteral( "SELECT count(coverage_name) FROM 'raster_coverages'" );
      i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasRasterLite2Tables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
    }
    if ( mHasTopologyExportTables == 0 )
    {
      sql = QStringLiteral( "SELECT count(topology_name) FROM 'topologies'" );
      i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            mHasTopologyExportTables = sqlite3_column_int( stmt, 0 );
          }
        }
        sqlite3_finalize( stmt );
      }
    }
    readNonSpatialTables( );
  }
  return mIsValid;
}
//----------------------------------------------------------
bool SpatialiteDbInfo::getSniffReadLayers( )
{
  //----------------------------------------------------------
  if ( mIsValid )
  {
    if ( mHasVectorLayers > 0 )
    {
      readVectorLayers();
    }
    if ( mHasVectorLayers <= 0 )
    {
      setDatabaseInvalid( QString( "GetSpatialiteDbInfo: No Vector Layers found[%1]." ).arg( mHasVectorLayers ) );
    }
    if ( mHasRasterLite2Tables > 0 )
    {
      readRL2Layers();
    }
    if ( mHasTopologyExportTables > 0 )
    {
      readTopologyLayers();
    }
    if ( mHasRasterLite1Tables > 0 )
    {
      readRL1Layers();
    }
    if ( mHasMBTilesTables > 0 )
    {
      readMBTilesLayers();
    }
    if ( mHasGeoPackageTables > 0 )
    {
      readGeoPackageLayers();
    }
    if ( mHasFdoOgrTables > 0 )
    {
      // Not yet implemented
      readFdoOgrLayers();
    }
  }
  // -- ---------------------------------- --
  // do only on creation, not when (possibly) adding to collection.
  mDbLayers.clear();
  // activating Foreign Key constraints
  ( void )sqlite3_exec( mSqliteHandle, "PRAGMA foreign_keys = 1", nullptr, 0, nullptr );
  // -- ---------------------------------- --
  return mIsValid;
}
//----------------------------------------------------------
bool SpatialiteDbInfo::getSniffLoadLayers( QString sLayerName )
{
  //----------------------------------------------------------
  if ( mIsValid )
  {
    GetDbLayersInfo( sLayerName );
  }
  return mIsValid;
}
//----------------------------------------------------------
bool SpatialiteDbInfo::GetSpatialiteDbInfo( QString sLayerName, bool bLoadLayers, SpatialSniff sniffType )
{
  if ( ( mSqliteHandle ) && ( isDbSqlite3() ) )
  {
    mLoadLayers = bLoadLayers;
    if ( sniffType == SpatialiteDbInfo::SniffUnknown )
    {
      // Parameter was not give, so we will try to guess
      sniffType = SpatialiteDbInfo::SniffDatabaseType;
      if ( bLoadLayers )
      {
        sniffType = SpatialiteDbInfo::SniffLoadLayers;
      }
      else if ( !sLayerName.isEmpty() )
      {
        sniffType = SpatialiteDbInfo::SniffMinimal;
      }
    }
    setSniffType( sniffType );
    if ( !sLayerName.isEmpty() )
    {
      // If a specific Layer is being requested, only that Layer will be loaded into mDbLayers
      mLoadLayers = true;
    }
    // SniffDatabaseType: determinee Sqlite3-Container-Type [SpatialMetadata] and used Spatialite version
    getSniffDatabaseType( );
    if ( sniffType == SpatialiteDbInfo::SniffDatabaseType )
    {
      // -- ---------------------------------- --
      // For cases where only the Sqlite3-Container-Type and the Spatialite Version numbers are needed
      // -- ---------------------------------- --
      return mIsValid;
    }
    // SniffMinimal: Load and store Information about Tables/Layers without details [SpatialiteLayerType]
    if ( getSniffSniffMinimal( ) )
    {
      // Determine the amount of LayerTypeTables that exist
      if ( getSniffLayerMetadata( ) )
      {
        // The amount of Layers for each Layer-Type is determined.
        getSniffReadLayers();
      }
    }
    if ( sniffType == SpatialiteDbInfo::SniffMinimal )
    {
      // -- ---------------------------------- --
      // For where only thelist of what is available is needed
      // - SpatialiteDbInfo::SniffMinimal must run before GetDbLayersInfo can be called.
      // -> SpatialiteDbInfo must be created with either SpatialiteDbInfo::SniffMinimal or SpatialiteDbInfo::SniffLoadLayers
      // --> not with SpatialiteDbInfo::SniffDatabaseType
      // -- ---------------------------------- --
      return mIsValid;
    }
    if ( mIsValid )
    {
      // -- ---------------------------------- --
      // SpatialiteDbInfo::SniffLoadLayers
      // -- ---------------------------------- --
      // Clears mDbLayers [Collection of Loaded-layers]
      // Activating Foreign Key constraints for the Database
      //  Loads Layers being request based on value of sLayerName
      getSniffLoadLayers( sLayerName );
    }
  }
  qDebug() << QString( "SpatialiteDbInfo::GetSpatialiteDbInfo(%1) isValid[%2] Tables[%3] VectorLayers[%4] LayersLoaded[%5]" ).arg( mSpatialiteVersionInfo ).arg( mIsValid ).arg( mNonSpatialTables.size() ).arg( mVectorLayers.size() ).arg( mDbLayers.count() );
  return mIsValid;
}
bool SpatialiteDbInfo::readVectorLayers()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    mVectorLayers.clear();
    mVectorLayersTypes.clear();
    sqlite3_stmt *stmt = nullptr;
    mHasSpatialTables = 0;
    mHasSpatialViews = 0;
    mHasVirtualShapes = 0;
    QString sql = QStringLiteral( "SELECT layer_type,table_name||'('||geometry_column||')' AS layer_name, geometry_type,coord_dimension, srid FROM vector_layers ORDER BY layer_type, table_name,geometry_column" );
    int  i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString sLayerType;
      QString sLayerName;
      QString sGeometryType;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          sLayerType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          SpatialiteDbInfo::SpatialiteLayerType layerType = SpatialiteDbInfo::SpatialiteLayerTypeFromName( sLayerType );
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
            {
              if ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL )
              {
                if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
                {
                  bool bValid = true;
                  sLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
                  sGeometryType = QgsWkbTypes::displayString( SpatialiteDbLayer::GetGeometryType( sqlite3_column_int( stmt, 2 ), sqlite3_column_int( stmt, 3 ) ) );
                  int iSrid = sqlite3_column_int( stmt, 4 );
                  switch ( layerType )
                  {
                    case SpatialiteDbInfo::SpatialTable:
                      mHasSpatialTables++;
                      break;
                    case SpatialiteDbInfo::SpatialView:
                      mHasSpatialViews++;
                      break;
                    case SpatialiteDbInfo::VirtualShape:
                      mHasVirtualShapes++;
                      break;
                    default:
                      // Unexpected value, do not add
                      bValid = false;
                      break;
                  }
                  if ( bValid )
                  {
                    mVectorLayers.insert( sLayerName, QString( "%1;%2" ).arg( sGeometryType ).arg( iSrid ) );
                    mVectorLayersTypes.insert( sLayerName, sLayerType );
                  }
                }
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    mHasVectorLayers = mVectorLayers.count();
    if ( mHasVectorLayers > 0 )
    {
      bRc = true;
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readRL2Layers()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    mRasterLite2Layers.clear();
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT coverage_name,srid FROM 'raster_coverages' ORDER BY coverage_name" );
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite2 );
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            QString sLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            int iSrid = sqlite3_column_int( stmt, 1 );
            mRasterLite2Layers.insert( sLayerName, QString( "%1;%2" ).arg( sLayerType ).arg( iSrid ) );
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    mHasRasterLite2Tables = mRasterLite2Layers.count();
    if ( mHasRasterLite2Tables > 0 )
    {
      bRc = true;
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readRL1Layers()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    mRasterLite1Layers.clear();
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT " );
    sql += QStringLiteral( "(SELECT group_concat(table_name,',') FROM 'layer_statistics' WHERE (raster_layer=1))," );
    sql += QStringLiteral( "(SELECT group_concat(replace(table_name,'_metadata',''),',') FROM 'layer_statistics' WHERE ((raster_layer=0) AND (table_name LIKE '%_metadata')))" );
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString rl1Tables;
      QStringList saRL1Tables;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          // truemarble-4km.sqlite: truemarble-4km.sqlite
          rl1Tables = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( !rl1Tables.isEmpty() )
          {
            saRL1Tables.append( rl1Tables.split( "," ) );
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          // ItalyRail.atlas: TrueMarble_metadata,srtm_metadata
          rl1Tables = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          if ( !rl1Tables.isEmpty() )
          {
            saRL1Tables.append( rl1Tables.split( "," ) );
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( saRL1Tables.size() )
      {
        for ( int i = 0; i < saRL1Tables.size(); i++ )
        {
          QString sRasterLite1Name = saRL1Tables.at( i );
          // TrueMarble_metadata,srtm_metadata
          sql = QStringLiteral( "SELECT " );
          sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE ((id=0) AND (source_name = 'raster metadata'))))," ).arg( QString( "%1_metadata" ).arg( sRasterLite1Name ) );
          sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE (id=1)))" ).arg( QString( "%1_rasters" ).arg( sRasterLite1Name ) );
          sql += QStringLiteral( "(SELECT srid FROM 'geometry_columns' WHERE (f_table_name = '%1_metadata'))" ).arg( sRasterLite1Name );
          // SELECT srid FROM 'geometry_columns' WHERE (f_table_name = 'TrueMarble_metadata')
          i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
          if ( i_rc == SQLITE_OK )
          {
            QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::RasterLite1 );
            while ( sqlite3_step( stmt ) == SQLITE_ROW )
            {
              if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
              {
                if ( ( sqlite3_column_int( stmt, 0 ) == 1 ) && ( sqlite3_column_int( stmt, 1 ) == 1 ) )
                {
                  // The internal-Data needed for RasterLite1 exist and could therefore be valid
                  int iSrid = sqlite3_column_int( stmt, 2 );
                  mRasterLite1Layers.insert( sRasterLite1Name, QString( "%1;%2" ).arg( sLayerType ).arg( iSrid ) );
                }
              }
            }
            sqlite3_finalize( stmt );
          }
        }
      }
    }
    mHasRasterLite1Tables = mRasterLite1Layers.count();
    if ( mHasRasterLite1Tables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "RasterLite" );
      GDALDriverH rl1GdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( rl1GdalDriver )
      {
        mHasGdalRasterLite1Driver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readTopologyLayers()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    mTopologyNames.clear();
    mTopologyExportLayers.clear();
    sqlite3_stmt *stmt = nullptr;
    QString sql = QStringLiteral( "SELECT topology_name,srid FROM 'topologies'" );
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      int iSrid = -1;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            mTopologyNames.append( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) ) );
            iSrid = sqlite3_column_int( stmt, 1 );
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( mTopologyNames.size() > 0 )
      {
        for ( int i = 0; i < mTopologyNames.size(); i++ )
        {
          QString sTopologyName = mTopologyNames.at( i );
          sql = QString( "SELECT topolayer_name FROM '%1'" ).arg( "%1_topolayers" ).arg( sTopologyName );
          i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
          if ( i_rc == SQLITE_OK )
          {
            QString sLayerType = SpatialiteDbInfo::SpatialiteLayerTypeName( SpatialiteDbInfo::SpatialiteTopopogy );
            while ( sqlite3_step( stmt ) == SQLITE_ROW )
            {
              if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
              {
                QString sTopoLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
                QString sLayerName = QString( "%1(%2)" ).arg( sTopologyName ).arg( sTopoLayerName );
                mTopologyExportLayers.insert( sLayerName, QString( "%1;%2" ).arg( sLayerType ).arg( iSrid ) );
              }
            }
            sqlite3_finalize( stmt );
          }
        }
        mHasTopologyExportTables = mTopologyExportLayers.count();
        if ( mHasTopologyExportTables > 0 )
        {
          bRc = true;
        }
      }
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readSqlite3MagicHeaderString( QString sDatabaseFileName )
{
  bool bRc = false;
  //----------------------------------------------------------
  QFile read_file( sDatabaseFileName );
  if ( ( read_file.exists() ) && ( read_file.open( QIODevice::ReadOnly ) ) )
  {
    if ( QString::fromStdString( read_file.peek( 16 ).toStdString() ).contains( QStringLiteral( "SQLite format 3" ) ) )
    {
      bRc = true;
    }
    read_file.close();
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::checkMBTiles()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    //--------------------------------------------------------
    // We are checking if valid, thus the (otherwise not needed) extra WHERE
    QString sql = QStringLiteral( "SELECT count(tbl_name) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name IN ('metadata','tiles','images','map')))" );
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          mHasMBTilesTables = sqlite3_column_int( stmt, 0 ) ;
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( mHasMBTilesTables > 0 )
    {
      bRc = true;
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readMBTilesLayers()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    SpatialiteLayerType type_MBTiles = MBTilesTable;
    if ( mHasMBTilesTables > 2 )
    {
      type_MBTiles = MBTilesView;
    }
    mMBTilesLayers.clear();
    mHasMBTilesTables = 0; // Assume invalid ; 1 is valid (there can only be 1 Raster-Layer)
    //--------------------------------------------------------
    // The 'metadata' table must contain in the column 'name'
    // version: 1.0.0:
    // - name, description, version
    // version: 1.1:
    // - name, description, version,format
    // The 'tiles' table or view must not be empty
    //--------------------------------------------------------
    QString sql = QStringLiteral( "SELECT  " );
    sql += QStringLiteral( "(SELECT count(name) FROM 'metadata' " );
    sql += QStringLiteral( "WHERE (name IN ('name','description','version','format')))," );
    sql += QStringLiteral( "(SELECT count(zoom_level) FROM tiles LIMIT 1)," );
    sql += QStringLiteral( "(SELECT value FROM 'metadata' WHERE (name IN ('name')))" );
    // SELECT (SELECT count(name) FROM 'metadata' WHERE (name IN ('name','description','version','format'))) AS valid, (SELECT count(zoom_level) FROM tiles LIMIT 1) AS count, (SELECT value FROM 'metadata' WHERE (name IN ('name'))) AS name
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
        {
          if ( ( sqlite3_column_int( stmt, 0 ) >= 3 ) && ( sqlite3_column_int( stmt, 1 ) > 0 ) )
          {
            QString sLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
            mMBTilesLayers.insert( sLayerName, QString( "%1;%2" ).arg( SpatialiteDbInfo::SpatialiteLayerTypeName( type_MBTiles ) ).arg( 4326 ) );
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    //--------------------------------------------------------
    mHasMBTilesTables = mMBTilesLayers.count();
    if ( mHasMBTilesTables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "MBTiles" );
      GDALDriverH mbtilesGdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( mbtilesGdalDriver )
      {
        mHasGdalMBTilesDriver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readGeoPackageLayers()
{
  bool bRc = false;
  //----------------------------------------------------------
  if ( mIsValid )
  {
    mHasGeoPackageTables = 0;
    mGeoPackageLayers.clear();
    sqlite3_stmt *stmt = nullptr;
    //--------------------------------------------------------
    //  SELECT table_name, data_type, identifier, srid FROM geopackage_contents WHERE (data_type IN ('features','tiles'))
    //--------------------------------------------------------
    // We are checking if valid, thus the (otherwise not needed) extra WHERE
    QString sql = QStringLiteral( "SELECT table_name, data_type, srid FROM geopackage_contents WHERE (data_type IN ('features','tiles'))" );
    int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
        {
          QString sLayerName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          QString sLayerType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          int i_srid = sqlite3_column_int( stmt, 2 ) ;
          SpatialiteLayerType type_GeoPackage = GeoPackageVector;
          if ( sLayerType == "tiles" )
          {
            type_GeoPackage = GeoPackageRaster;
          }
          mGeoPackageLayers.insert( sLayerName, QString( "%1;%2" ).arg( SpatialiteDbInfo::SpatialiteLayerTypeName( type_GeoPackage ) ).arg( i_srid ) );
        }
      }
      sqlite3_finalize( stmt );
    }
    mHasGeoPackageTables = mGeoPackageLayers.count();
    if ( mHasGeoPackageTables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "GPKG" );
      GDALDriverH gpkgGdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( gpkgGdalDriver )
      {
        mHasGdalGeoPackageDriver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::checkFdoOgr()
{
  bool bRc = false;
  return bRc; // Not yet implemented
  //----------------------------------------------------------
  if ( mIsValid )
  {
    // sqlite3_stmt *stmt = nullptr;
    //--------------------------------------------------------
    //--------------------------------------------------------
    if ( mHasFdoOgrTables > 0 )
    {
      bRc = true;
    }
  }
  //----------------------------------------------------------
  return bRc;
}
bool SpatialiteDbInfo::readFdoOgrLayers()
{
  bool bRc = false;
  return bRc; // Not yet implemented
  //----------------------------------------------------------
  if ( mIsValid )
  {
    mHasFdoOgrTables = 0;
    mFdoOgrLayers.clear();
    QString sql;
    // sqlite3_stmt *stmt = nullptr;
    //--------------------------------------------------------
    mHasFdoOgrTables = mFdoOgrLayers.count();
    if ( mHasFdoOgrTables > 0 )
    {
      bRc = true;
      sql = QStringLiteral( "SQLite" );
      GDALDriverH fdoGdalDriver = GDALGetDriverByName( sql.toLocal8Bit().constData() );
      if ( fdoGdalDriver )
      {
        mHasFdoOgrDriver = true;
        mIsGdalOgr = true;
      }
    }
  }
  //----------------------------------------------------------
  return bRc;
}
int SpatialiteDbInfo::checkLayerSanity( QString sLayerName )
{
  int i_rc = 0;
  // sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  QString sTableName  = sLayerName;
  // if 'sGeometryColumn' remains empty, the row count will be returned
  QString sGeometryColumn = "";
  if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
  {
    // Extract GeometryName from sent 'table_name(field_name)' from layerName
    QStringList sa_list_name = sTableName.split( "(" );
    if ( sa_list_name.size() )
    {
      sGeometryColumn = sa_list_name[1].replace( ")", "" );
      sTableName = sa_list_name[0];
    }
  }
  //----------------------------------------------------------
  QString sql = QStringLiteral( "SELECT count('%2') FROM '%1' LIMIT 1" ).arg( sTableName ).arg( sGeometryColumn );
  i_rc = sqlite3_exec( mSqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( i_rc != SQLITE_OK )
  {
    sql = QString( "%1 rc=%2" ).arg( errMsg ).arg( i_rc );
    // qDebug() << sql;
    if ( sql.contains( "no such table" ) )
    {
      i_rc = 100; // i_rc=1
    }
    else if ( sql.contains( "no such column" ) )
    {
      i_rc = 101; // i_rc=1
    }
    else if ( sql.contains( "is not a function" ) )
    {
      // 'SpatialView': possible faulty Sql-Syntax
      i_rc = 200; // i_rc=1
    }
  }
  //----------------------------------------------------------
  return i_rc;
}
bool SpatialiteDbInfo::GetDbLayersInfo( QString sLayerName )
{
  bool bRc = false;
  int i_debug = 0;
  if ( mIsValid )
  {
    if ( !sLayerName.isEmpty() )
    {
      // Note: for some reason this is being called with the uri ; extranct the LayerName in such cases
      if ( ( sLayerName.contains( "dbname=" ) ) && ( sLayerName.contains( "table=" ) ) )
      {
        QgsDataSourceUri anUri = QgsDataSourceUri( sLayerName );
        sLayerName = anUri.table();
        if ( !anUri.geometryColumn().toLower().isEmpty() )
        {
          sLayerName = QString( "%1(%2)" ).arg( sLayerName ).arg( anUri.geometryColumn().toLower() );
        }
      }
    }
#ifdef SPATIALITE_VERSION_GE_4_0_0

    bool bFoundLayerName = false;
    if ( mHasVectorLayers > 0 )
    {
      // Note: gaiaGetVectorLayersList will deal with legacy metadata style <= v.3.x.x properly
      gaiaVectorLayersListPtr layersList = nullptr;
      gaiaVectorLayerPtr layer = nullptr;
      mLayersCount = 0;
      QStringList saSearchTables;
      if ( !sLayerName.isEmpty() )
      {
        if ( mVectorLayers.contains( sLayerName ) )
        {
          // Searching only for a specific Layer
          saSearchTables.append( sLayerName );
        }
      }
      else
      {
        saSearchTables = mVectorLayers.uniqueKeys();
      }
      if ( saSearchTables.size() )
      {
        for ( int i = 0; i < saSearchTables.size(); i++ )
        {
          QString sSearchLayer = saSearchTables.at( i );
          // Add only if it does not already exist
          if ( !getSpatialiteDbLayer( sSearchLayer, false ) )
          {
            QString sTableName  = sSearchLayer;
            QString sGeometryColumn = QString::null;
            if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
            {
              // Extract GeometryName from sent 'table_name(field_name)' from layerName
              QStringList sa_list_name = sTableName.split( "(" );
              sGeometryColumn = sa_list_name[1].replace( ")", "" );
              sTableName = sa_list_name[0];
            }
            // To retrieve the fields, we must supply at least the table-name:
            // [gaiaGetVectorLayersList: if no "table" is set, we'll never return AttributeField Infos]
            layersList = gaiaGetVectorLayersList( mSqliteHandle, sTableName.toUtf8().constData(), sGeometryColumn.toUtf8().constData(), GAIA_VECTORS_LIST_OPTIMISTIC );
            if ( layersList )
            {
              layer = layersList->First;
              while ( layer )
              {
                sTableName = QString::fromUtf8( layer->TableName );
                sGeometryColumn = QString::fromUtf8( layer->GeometryName );
                if ( !sGeometryColumn.isEmpty() )
                {
                  sTableName = QString( "%1(%2)" ).arg( sTableName ).arg( sGeometryColumn );
                }
                SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
                if ( dbLayer )
                {
                  dbLayer->mLayerId = mLayersCount++;
                  // order will be as found in the vector_layers table
                  switch ( layer->LayerType )
                  {
                    case GAIA_VECTOR_UNKNOWN:
                      break;
                    case GAIA_VECTOR_TABLE:
                    case GAIA_VECTOR_VIEW:
                    case GAIA_VECTOR_VIRTUAL:
                    {
                      dbLayer->setLayerType( ( SpatialiteDbInfo::SpatialiteLayerType )layer->LayerType );
                      dbLayer->mIsValid = true;
                    }
                    break;
                  }
                  dbLayer->mTableName = QString::fromUtf8( layer->TableName );
                  dbLayer->mLayerName = dbLayer->mTableName;
                  dbLayer->mGeometryColumn = QString::fromUtf8( layer->GeometryName );
                  if ( !dbLayer->mGeometryColumn.isEmpty() )
                  {
                    dbLayer->mLayerName = QString( "%1(%2)" ).arg( dbLayer->mTableName ).arg( dbLayer->mGeometryColumn );
                  }
                  dbLayer->setSrid( layer->Srid );
                  if ( dbLayer->mTableName.isEmpty() )
                  {
                    dbLayer->setLayerInvalid( QString( "[%1] Invalid Table-Name[%2]: no result" ).arg( sSearchLayer ).arg( dbLayer->mTableName ) );
                  }
                  if ( dbLayer->mGeometryColumn.isEmpty() )
                  {
                    dbLayer->setLayerInvalid( QString( "[%1] Invalid GeometryColumn-Name[%2]: no result" ).arg( sSearchLayer ).arg( dbLayer->mGeometryColumn ) );
                  }
                  if ( ( dbLayer->mTableName.isEmpty() ) || ( dbLayer->mGeometryColumn.isEmpty() ) || ( dbLayer->mSrid < -1 ) )
                  {
                    dbLayer->setLayerInvalid( QString( "[%1] Invalid Srid[%2]: no result" ).arg( sSearchLayer ).arg( dbLayer->mSrid ) );
                  }
                  dbLayer->setSpatialIndexType( layer->SpatialIndex );
                  if ( dbLayer->mSpatialIndexType == SpatialiteDbInfo::SpatialIndexNone )
                  {
                    dbLayer->setLayerInvalid( QString( "GetDbLayersInfo [%1] Invalid SpatialiIndex[%2]." ).arg( sSearchLayer ).arg( dbLayer->getSpatialIndexString() ) );
                  }
                  dbLayer->setGeomType( SpatialiteDbLayer::GetGeometryType( layer->GeometryType,  layer->Dimensions ) );
                  if ( dbLayer->mGeometryType == QgsWkbTypes::Unknown )
                  {
                    dbLayer->setLayerInvalid( QString( "GetDbLayersInfo [%1] Capabilities: Invalid GeometryrType[%2]." ).arg( sSearchLayer ).arg( dbLayer->getGeometryTypeString() ) );
                  }
                  if ( layer->AuthInfos )
                  {
                    dbLayer->mLayerIsHidden = layer->AuthInfos->IsHidden;
                    if ( dbLayer->mLayerIsHidden )
                    {
                      dbLayer->setLayerInvalid( QString( "GetDbLayersInfo [%1] LayerIsHidden" ).arg( sSearchLayer ) );
                    }
                  }
                  if ( layer->ExtentInfos )
                  {
                    QgsRectangle layerExtent( layer->ExtentInfos->MinX, layer->ExtentInfos->MinY,
                                              layer->ExtentInfos->MaxX, layer->ExtentInfos->MaxY );
                    dbLayer->setLayerExtent( layerExtent );
                    dbLayer->mNumberFeatures = layer->ExtentInfos->Count;
                  }
                  if ( dbLayer->mIsValid )
                  {
                    dbLayer->mCoordDimensions = layer->Dimensions;
                    if ( dbLayer->mLayerType == SpatialiteDbInfo::SpatialTable )
                    {
                      dbLayer->mLayerReadOnly = 0; // default is 1
                    }
                    if ( layer->AuthInfos )
                    {
                      dbLayer->mLayerReadOnly = layer->AuthInfos->IsReadOnly;
                      dbLayer->mLayerIsHidden = layer->AuthInfos->IsHidden;
                      if ( dbLayer->mLayerType == SpatialiteDbInfo::SpatialView )
                      {
#ifdef SPATIALITE_VERSION_GE_4_5_0
                        if ( mIsVersion45 )
                        {
                          dbLayer->mTriggerInsert = layer->AuthInfos->HasTriggerInsert;
                          dbLayer->mTriggerUpdate = layer->AuthInfos->HasTriggerUpdate;
                          dbLayer->mTriggerDelete = layer->AuthInfos->HasTriggerDelete;
                        }
                        else
                        {
                          dbLayer->GetViewTriggers( );
                        }
#else
                        dbLayer->GetViewTriggers( );
#endif
                      }
                    }
                    else
                    {
                      dbLayer->GetViewTriggers( );
                    }
                    //-----------------------------------------
                    dbLayer->mAttributeFields.clear();
                    dbLayer->mPrimaryKey.clear(); // cazzo cazzo cazzo
                    dbLayer->mPrimaryKeyAttrs.clear();
                    dbLayer->mDefaultValues.clear();
                    dbLayer->mTopologyExportLayers.clear();
                    dbLayer->mTopologyExportLayersDataSourceUris.clear();
                    dbLayer->mTopologyExportLayersDataSourceUris.clear();
                    if ( layer->First )
                    {
                      // layer_fields = layer->First; // gaiaLayerAttributeFieldPtr
                      // setSpatialiteAttributeFields() : collect all relevant information
                    }
                  }
                  //-----------------------------------------
                  //---retrieving Table settings ----------
                  //---retrieving common settings --------
                  //---retrieving View settings -----------
                  //---setting EnabledCapabilities---------
                  //-----------------------------------------
                  if ( dbLayer->mIsValid )
                  {
                    if ( dbLayer->GetLayerSettings() )
                    {
                      //-----------------------------------------
                      if ( dbLayer->mIsValid )
                      {
                        switch ( dbLayer->mLayerType )
                        {
                          case SpatialiteDbInfo::SpatialTable:
                            // mHasSpatialTables++;
                            if ( dbLayer->mSpatialIndexType == SpatialiteDbInfo::SpatialIndexRTree )
                            {
                              // Replace type
                              QString sIndexName = QString( "idx_%1_%2" ).arg( dbLayer->mTableName ).arg( dbLayer->mGeometryColumn );
                              mNonSpatialTables[ sIndexName ] = "SpatialIndex-Table";
                              mNonSpatialTables[QString( "%1_node" ).arg( sIndexName ) ] = "SpatialIndex-Node";
                              mNonSpatialTables[QString( "%1_parent" ).arg( sIndexName ) ] = "SpatialIndex-Parent";
                              mNonSpatialTables[QString( "%1_rowid" ).arg( sIndexName ) ] = "SpatialIndex-Rowid";
                            }
                            break;
                          case SpatialiteDbInfo::SpatialView:
                            // mHasSpatialViews++;
                            break;
                          case SpatialiteDbInfo::VirtualShape:
                            //mHasVirtualShapes++;
                            break;
                          default:
                            break;
                        }
                        if ( dbLayer->mLayerName == sLayerName )
                        {
                          // We have found what we are looking for, prevent RL1/2 and Topology search
                          bFoundLayerName = true;
                        }
                        // Remove valid Layer Table-names
                        mNonSpatialTables.remove( dbLayer->mTableName );
                        mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                      }
                    }
                  }
                  if ( !dbLayer->mIsValid )
                  {
                    i_debug = 0;
                    if ( i_debug )
                    {
                      qDebug() << QString( "-E-> SpatialiteDbInfo::GetDbLayersInfo(%1) LayerType[%2] Layername[%3]" ).arg( dbLayer->getErrors().size() ).arg( dbLayer->mLayerTypeString ).arg( dbLayer->mLayerName );
                      for ( int i = 0; i < dbLayer->getErrors().size(); i++ )
                      {
                        qDebug() << QString( "-E-> %1): [%2] " ).arg( ( i + 1 ) ).arg( dbLayer->getErrors().at( i ) );
                      }
                    }
                    mErrors.append( QString( "-W-> SpatialiteDbInfo::GetDbLayersInfo Errors[%1] LayerType[%2] Layername[%3]" ).arg( ( dbLayer->getErrors().size() + 1 ) ).arg( dbLayer->mLayerTypeString ).arg( dbLayer->mLayerName ) );
                    delete dbLayer;
                  }
                }
                layer = layer->Next;
              }
              gaiaFreeVectorLayersList( layersList );
              layer = nullptr;
              layersList = nullptr;
            }
            else
            {
              int i_reason = checkLayerSanity( sSearchLayer );
              switch ( i_reason )
              {
                case 100:
                  mErrors.append( QString( "gaiaGetVectorLayersList: cause: Table not found[%1]" ).arg( sSearchLayer ) );
                  break;
                case 101:
                  mErrors.append( QString( "gaiaGetVectorLayersList: cause: Column not found[%1]" ).arg( sSearchLayer ) );
                  break;
                case 200:
                  mErrors.append( QString( "gaiaGetVectorLayersList: possible cause: Sql-Syntax error when Layer is a SpatialView[%1]" ).arg( sSearchLayer ) );
                  break;
                default:
                  mErrors.append( QString( "gaiaGetVectorLayersList: Unknown error[%1]" ).arg( sSearchLayer ) );
                  break;
              }
            }
          }
          else
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
        }
      }
    }
    if ( !bFoundLayerName )
    {
      bool bRc = false;
      if ( mHasRasterLite2Tables > 0 )
      {
        bRc = GetRL2LayersInfo( sLayerName );
        if ( !sLayerName.isEmpty() )
        {
          bFoundLayerName = bRc;
        }
      }
      if ( mHasTopologyExportTables > 0 )
      {
        bRc = GetTopologyLayersInfo( sLayerName );
        if ( !bFoundLayerName )
        {
          if ( !sLayerName.isEmpty() )
          {
            bFoundLayerName = bRc;
          }
        }
      }
      if ( mHasRasterLite1Tables > 0 )
      {
        bRc = GetRL1LayersInfo( sLayerName );
        if ( !bFoundLayerName )
        {
          if ( !sLayerName.isEmpty() )
          {
            bFoundLayerName = bRc;
          }
        }
      }
      if ( mHasMBTilesTables > 0 )
      {
        bRc = GetMBTilesLayersInfo( sLayerName );
        if ( !bFoundLayerName )
        {
          if ( !sLayerName.isEmpty() )
          {
            bFoundLayerName = bRc;
          }
        }
      }
      if ( mHasGeoPackageTables > 0 )
      {
        bRc = GetGeoPackageLayersInfo( sLayerName );
        if ( !bFoundLayerName )
        {
          if ( !sLayerName.isEmpty() )
          {
            bFoundLayerName = bRc;
          }
        }
      }
      if ( mHasFdoOgrTables > 0 )
      {
        bRc = GetFdoOgrLayersInfo( sLayerName );
        if ( !bFoundLayerName )
        {
          if ( !sLayerName.isEmpty() )
          {
            bFoundLayerName = bRc;
          }
        }
      }
      if ( !bFoundLayerName )
      {
        mNonSpatialTables[ sLayerName ] = "Data-Table";
      }
    }
    prepare();
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = mIsValid;
    }
#else
// this should be depreciated
// get_table_layers_legacy
// get_view_layers_legacy
// get_table_extent_legacy
// get_view_extent_legacy
// get_table_auth_legacy
#endif

  }
  i_debug = 0;
  if ( ( i_debug ) && ( !bRc ) )
  {
    qDebug() << QString( "-I-> SpatialiteDbInfo::GetDbLayersInfo(%1) : bRc[%2]" ).arg( sLayerName ).arg( bRc );
    for ( int i = 0; i < getErrors().size(); i++ )
    {
      qDebug() << QString( "-E-> SpatialiteDbInfo::GetDbLayersInfo Error %1 : [%2]" ).arg( i ).arg( getErrors().at( i ) );
    }
    for ( int i = 0; i < getVectorLayersMissing().size(); i++ )
    {
      qDebug() << QString( "-W-> SpatialiteDbInfo::GetDbLayersInfo Missing %1 : [%2]" ).arg( i ).arg( getVectorLayersMissing().at( i ) );
    }
  }
  return bRc;
}
bool SpatialiteDbInfo::GetRL2LayersInfo( const QString sLayerName )
{
  bool bRc = false;
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    int i_rc = 0;
    bool bFoundLayerName = false;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    if ( !sLayerName.isEmpty() )
    {
      sTableName = sLayerName;
      if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
      {
        // Extract GeometryName from sent 'table_name(field_name)' from layerName
        QStringList sa_list_name = sTableName.split( "(" );
        sGeometryColumn = sa_list_name[1].replace( ")", "" );
        sTableName = sa_list_name[0];
      }
    }
    // SELECT coverage_name, title,abstract,copyright,srid,extent_minx,extent_miny,extent_maxx,extent_maxy FROM raster_coverages
    QString sFields = QString( "coverage_name, title,abstract,copyright,srid,extent_minx,extent_miny,extent_maxx,extent_maxy" );
    QString sql = QStringLiteral( "SELECT %1 FROM raster_coverages" ).arg( sFields );
    if ( !sTableName.isEmpty() )
    {
      sql += QStringLiteral( " WHERE (coverage_name=Lower('%1'))" ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      int i_check_count_rl2 = mHasRasterLite2Tables;
      mHasRasterLite2Tables = 0;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mAttributeFields.clear();
              dbLayer->mPrimaryKey.clear(); // cazzo cazzo cazzo
              dbLayer->mPrimaryKeyAttrs.clear();
              dbLayer->mDefaultValues.clear();
              dbLayer->mTopologyExportLayersDataSourceUris.clear();
              dbLayer->mTopologyExportLayers.clear();
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->setLayerType( SpatialiteDbInfo::RasterLite2 );
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                sFields = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
                if ( sFields.startsWith( "***" ) )
                {
                  sFields = "";
                }
                dbLayer->mTitle = sFields;
                if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
                {
                  sFields = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
                  if ( sFields.startsWith( "***" ) )
                  {
                    sFields = "";
                  }
                  dbLayer->mAbstract = sFields;
                  if ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL )
                  {
                    sFields = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) );
                    if ( sFields.startsWith( "***" ) )
                    {
                      sFields = "";
                    }
                    dbLayer->mCopyright = sFields;
                    if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
                    {
                      if ( ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) &&
                           ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) &&
                           ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL ) &&
                           ( sqlite3_column_type( stmt, 7 ) != SQLITE_NULL ) &&
                           ( sqlite3_column_type( stmt, 8 ) != SQLITE_NULL ) )
                      {
                        // If valid srid, mIsValid will be set to true [srid must be set before LayerExtent]
                        dbLayer->setSrid( sqlite3_column_int( stmt, 4 ) );
                        QgsRectangle layerExtent( sqlite3_column_double( stmt, 5 ), sqlite3_column_double( stmt, 6 ), sqlite3_column_double( stmt, 7 ), sqlite3_column_double( stmt, 8 ) );
                        dbLayer->setLayerExtent( layerExtent );
                      }
                    }
                  }
                }
              }
            }
            if ( dbLayer->mIsValid )
            {
              // Remove (and delete) the RasterLite2 Admin tables, which should not be shown
              SpatialiteDbLayer *removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_sections" ).arg( dbLayer->mTableName ) ) );
              // Replace type
              mNonSpatialTables[QString( "%1_sections" ).arg( dbLayer->mTableName ) ] = "RasterLite2-Sections";
              delete removeLayer;
              removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_tiles" ).arg( dbLayer->mTableName ) ) );
              mNonSpatialTables[QString( "%1_tiles" ).arg( dbLayer->mTableName ) ] = "RasterLite2-Tiles";
              delete removeLayer;
              // mHasSpatialTables -= 2;
              // mHasRasterLite2Tables++;
              // Resolve unset settings
              dbLayer->prepare();
              if ( dbLayer->mLayerName == sLayerName )
              {
                // We have found what we are looking for, prevent RL1/2 and Topology search
                bFoundLayerName = true;
              }
              // Remove valid Layer Table-names
              //mNonSpatialTables.remove( dbLayer->mTableName );
              mDbLayers.insert( dbLayer->mLayerName, dbLayer );
            }
            else
            {
              delete dbLayer;
            }
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( i_check_count_rl2 != mHasRasterLite2Tables )
      {
        // TODO possible warning that something was invalid
      }
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = mIsValid;
    }
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbInfo::GetRL2LayersInfo: called with !mIsValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}

bool SpatialiteDbInfo::GetTopologyLayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    if ( !sLayerName.isEmpty() )
    {
      sTableName = sLayerName;
      if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
      {
        // Extract GeometryName from sent 'table_name(field_name)' from layerName
        QStringList sa_list_name = sTableName.split( "(" );
        sGeometryColumn = sa_list_name[1].replace( ")", "" );
        sTableName = sa_list_name[0];
      }
    }
    // SELECT topology_name, srid FROM topologies
    QString sFields = QString( "topology_name, srid" );
    QString sql = QStringLiteral( "SELECT %1 FROM topologies" ).arg( sFields );
    if ( !sLayerName.isEmpty() )
    {
      sql += QStringLiteral( " WHERE (topology_name=Lower('%1'))" ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      int i_check_count_topology = mHasTopologyExportTables;
      mHasTopologyExportTables = 0;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          QString sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mAttributeFields.clear();
              dbLayer->mPrimaryKey.clear(); // cazzo cazzo cazzo
              dbLayer->mPrimaryKeyAttrs.clear();
              dbLayer->mDefaultValues.clear();
              dbLayer->mTopologyExportLayers.clear();
              dbLayer->mTopologyExportLayersDataSourceUris.clear();
              dbLayer->mNumberFeatures = 0;
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->setLayerType( SpatialiteDbInfo::SpatialiteTopopogy );
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                // If valid srid, mIsValid will be set to true
                dbLayer->setSrid( sqlite3_column_int( stmt, 1 ) );
              }
              if ( dbLayer->mIsValid )
              {
                sFields = QString( "extent_minx,extent_miny,extent_maxx,extent_maxy" );
                sql = QStringLiteral( "SELECT %1 FROM %2" ).arg( sFields ).arg( QString( "%1_face_geoms" ).arg( dbLayer->mTableName ) );
                i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmtSubquery, NULL );
                if ( i_rc == SQLITE_OK )
                {
                  while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                  {
                    if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 2 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmtSubquery, 3 ) != SQLITE_NULL ) )
                    {
                      QgsRectangle layerExtent( sqlite3_column_double( stmtSubquery, 0 ), sqlite3_column_double( stmtSubquery, 1 ), sqlite3_column_double( stmtSubquery, 2 ), sqlite3_column_double( stmtSubquery, 3 ) );
                      dbLayer->setLayerExtent( layerExtent );
                      dbLayer->mIsValid = true;
                    }
                  }
                  sqlite3_finalize( stmtSubquery );
                  if ( dbLayer->mIsValid )
                  {
                    // TODO: valid without layers? could be - ?? just not yet created??
                    sFields = QString( "topolayer_id,topolayer_name" );
                    // "topology_ortsteil_segments_topolayers
                    sql = QStringLiteral( "SELECT %1 FROM %2 ORDER BY name" ).arg( sFields ).arg( QString( "%1_topolayers" ).arg( dbLayer->mTableName ) );
                    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmtSubquery, NULL );
                    if ( i_rc == SQLITE_OK )
                    {
                      while ( sqlite3_step( stmt ) == SQLITE_ROW )
                      {
                        int i_topolayer_id = 0;
                        QString s_topolayer_name = "";
                        if ( ( sqlite3_column_type( stmtSubquery, 0 ) != SQLITE_NULL ) &&
                             ( sqlite3_column_type( stmtSubquery, 1 ) != SQLITE_NULL ) )
                        {
                          // The unique id inside the Topology
                          i_topolayer_id = sqlite3_column_int( stmt, 0 );
                          // The exported SpatialTable [Topology-Features (Metadata) and Geometry]
                          s_topolayer_name = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
                          // The Topology-Features (Metadata)
                          QString featureName = QString( "%1_topofeatures_%2" ).arg( dbLayer->mTableName ).arg( i_topolayer_id );
                          dbLayer->mAttributeFields.append( QgsField( s_topolayer_name, QVariant::UserType, featureName, i_topolayer_id, dbLayer->mLayerId, dbLayer->mTableName ) );
                          // Move the (general) SpatialTable to the Topology Layer
                          SpatialiteDbLayer *moveLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( s_topolayer_name ) );
                          if ( moveLayer )
                          {
                            /*
                             switch ( moveLayer->mLayerType )
                             {
                               case SpatialiteDbInfo::SpatialTable:
                                 mHasSpatialTables--;
                                 break;
                               case SpatialiteDbInfo::SpatialView:
                                 mHasSpatialViews--;
                                 break;
                               case SpatialiteDbInfo::VirtualShape:
                                 mHasVirtualShapes--;
                                 break;
                               default:
                                 break;
                             }
                             */
                            moveLayer->mLayerId = i_topolayer_id;
                            moveLayer->setLayerType( SpatialiteDbInfo::TopopogyExport );
                            dbLayer->mTopologyExportLayers.insert( moveLayer->mLayerName, moveLayer );
                            dbLayer->mTopologyExportLayersDataSourceUris.insert( moveLayer->mLayerName, moveLayer->layerConnectionInfo() );
                            dbLayer->mNumberFeatures++;
                          }
                        }
                      }
                      sqlite3_finalize( stmtSubquery );
                    }
                  }
                }
              }
              if ( dbLayer->mIsValid )
              {
                // Remove (and delete) the Topology Admin tables, which should not be shown
                SpatialiteDbLayer *removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_edge" ).arg( dbLayer->mTableName ) ) );
                // Replace type
                mNonSpatialTables[QString( "%1_edge" ).arg( dbLayer->mTableName ) ] = "Topology-Edge";
                delete removeLayer;
                removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_face" ).arg( dbLayer->mTableName ) ) );
                mNonSpatialTables[QString( "%1_face" ).arg( dbLayer->mTableName ) ] = "Topology-Face";
                delete removeLayer;
                removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_nodes" ).arg( dbLayer->mTableName ) ) );
                mNonSpatialTables[QString( "%1_nodes" ).arg( dbLayer->mTableName ) ] = "Topology-Nodes";
                delete removeLayer;
                removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_seeds" ).arg( dbLayer->mTableName ) ) );
                mNonSpatialTables[QString( "%1_seeds" ).arg( dbLayer->mTableName ) ] = "Topology-Seeds";
                delete removeLayer;
                // mHasSpatialTables -= 4;
                removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_edge_seeds" ).arg( dbLayer->mTableName ) ) );
                mNonSpatialTables[QString( "%1_edge_seeds" ).arg( dbLayer->mTableName ) ] = "Topology-View-Edges_Seeds";
                delete removeLayer;
                removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_face_seeds" ).arg( dbLayer->mTableName ) ) );
                mNonSpatialTables[QString( "%1_face_seeds" ).arg( dbLayer->mTableName ) ] = "Topology-View-Face_Seeds";
                delete removeLayer;
                removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_face_geoms" ).arg( dbLayer->mTableName ) ) );
                mNonSpatialTables[QString( "%1_face_geoms" ).arg( dbLayer->mTableName ) ] = "Topology-View-Face_Geoms";
                delete removeLayer;
                // mHasSpatialViews -= 3;
                // Resolve unset settings
                dbLayer->prepare();
                if ( dbLayer->mLayerName == sLayerName )
                {
                  // We have found what we are looking for, prevent RL1/2 and Topology search
                  bFoundLayerName = true;
                }
                mDbLayers.insert( dbLayer->mLayerName, dbLayer );
                mHasTopologyExportTables++;
              }
              else
              {
                delete dbLayer;
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
      if ( i_check_count_topology != mHasTopologyExportTables )
      {
        // TODO possible warning that something was invalid
      }
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = mIsValid;
    }
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbInfo::GetTopologyLayersInfo: called with !mIsValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
bool SpatialiteDbInfo::GetRL1LayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_rc = 0;
    int i_check_count_rl1 = mHasRasterLite1Tables;
    mHasRasterLite1Tables = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    if ( !sLayerName.isEmpty() )
    {
      sTableName = sLayerName;
      if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
      {
        // Extract GeometryName from sent 'table_name(field_name)' from layerName
        QStringList sa_list_name = sTableName.split( "(" );
        sGeometryColumn = sa_list_name[1].replace( ")", "" );
        sTableName = sa_list_name[0];
      }
    }
    QString sFields = QString( "table_name, geometry_column, row_count, extent_min_x, extent_min_y, extent_max_x, extent_max_y" );
    // truemarble-4km.sqlite
    QString sql = QStringLiteral( "SELECT %1 FROM layer_statistics WHERE ((raster_layer=1) OR ((raster_layer=0) AND (table_name LIKE '%_metadata')))" ).arg( sFields );
    if ( !sTableName.isEmpty() )
    {
      sql = QStringLiteral( "SELECT %1 FROM layer_statistics WHERE (((raster_layer=1) AND (table_name = '%2')) OR ((raster_layer=0) AND (table_name = '%2_metadata')))" ).arg( sFields ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName.contains( "_metadata" ) )
          {
            sTableName.remove( "_metadata" );
          }
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mAttributeFields.clear();
              dbLayer->mPrimaryKey.clear(); // cazzo cazzo cazzo
              dbLayer->mPrimaryKeyAttrs.clear();
              dbLayer->mDefaultValues.clear();
              dbLayer->mTopologyExportLayers.clear();
              dbLayer->mTopologyExportLayersDataSourceUris.clear();
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->setLayerType( SpatialiteDbInfo::RasterLite1 );
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              dbLayer->mCoordDimensions = GAIA_XY;
              QgsRectangle layerExtent;
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                dbLayer->mGeometryColumn = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              }
              if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
              {
                dbLayer->mNumberFeatures = sqlite3_column_int( stmt, 2 );
              }
              if ( ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL ) )
              {
                // Note: set extend only after setting the srid (creation of EWKT)
                layerExtent.set( sqlite3_column_double( stmt, 3 ), sqlite3_column_double( stmt, 4 ), sqlite3_column_double( stmt, 5 ), sqlite3_column_double( stmt, 6 ) );
                // TrueMarble_metadata,srtm_metadata
                sql = QStringLiteral( "SELECT " );
                sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE ((id=0) AND (source_name = 'raster metadata'))))," ).arg( QString( "%1_metadata" ).arg( dbLayer->mTableName ) );
                sql += QStringLiteral( "(SELECT Exists(SELECT id FROM '%1' WHERE (id=1)))" ).arg( QString( "%1_rasters" ).arg( dbLayer->mTableName ) );
                i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmtSubquery, NULL );
                if ( i_rc == SQLITE_OK )
                {
                  while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                  {
                    if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
                    {
                      if ( ( sqlite3_column_int( stmt, 0 ) == 1 ) &&
                           ( sqlite3_column_int( stmt, 1 ) == 1 ) )
                      {
                        dbLayer->mIsValid = true;
                      }
                    }
                  }
                  sqlite3_finalize( stmtSubquery );
                }
              }
              if ( dbLayer->mIsValid )
              {
                QString metadataFields = "f_geometry_column,type,coord_dimension,srid,spatial_index_enabled";
                sql = QStringLiteral( "SELECT %1 FROM 'geometry_columns' WHERE (f_table_name= '%2')" ).arg( metadataFields ).arg( QString( "%1_metadata" ).arg( dbLayer->mTableName ) );
                i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmtSubquery, NULL );
                if ( i_rc == SQLITE_OK )
                {
                  while ( sqlite3_step( stmtSubquery ) == SQLITE_ROW )
                  {
                    if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) )
                    {
                      dbLayer->mGeometryColumn =  QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
                      QString geometryType =  QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
                      QString geometryDimension =  QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
                      dbLayer->setGeomType( SpatialiteDbLayer::GetGeometryTypeLegacy( geometryType, geometryDimension ) );

                    }
                    if ( ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                         ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
                    {
                      // If valid srid, mIsValid will be set to true
                      dbLayer->setSrid( sqlite3_column_int( stmt, 3 ) );
                      dbLayer->setLayerExtent( layerExtent );
                      dbLayer->setSpatialIndexType( sqlite3_column_int( stmt, 4 ) );
                    }
                  }
                  sqlite3_finalize( stmtSubquery );
                }
              }
              if ( dbLayer->mIsValid )
              {
                SpatialiteDbLayer *removeLayer = static_cast<SpatialiteDbLayer *>( mDbLayers.take( QString( "%1_metadata" ).arg( dbLayer->mTableName ) ) );
                // Replace type
                mNonSpatialTables[QString( "%1_metadata" ).arg( dbLayer->mTableName ) ] = "RasterLite1-Metadata";
                mNonSpatialTables[QString( "%1_raster" ).arg( dbLayer->mTableName ) ] = "RasterLite1-Raster";
                if ( removeLayer )
                {
                  mHasSpatialTables--;
                  delete removeLayer;
                }
                mHasRasterLite1Tables++;
                // Resolve unset settings
                dbLayer->prepare();
                if ( dbLayer->mLayerName == sLayerName )
                {
                  // We have found what we are looking for, prevent RL1/2 and Topology search
                  bFoundLayerName = true;
                }
                mDbLayers.insert( dbLayer->mLayerName, dbLayer );
              }
              else
              {
                delete dbLayer;
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_rl1 != mHasRasterLite1Tables )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = mIsValid;
    }
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbInfo::GetRL1LayersInfo: called with !mIsValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
bool SpatialiteDbInfo::GetMBTilesLayersInfo( QString sLayerName )
{
  bool bRc = false;
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    // sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_check_count_mbtiles = mHasMBTilesTables;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    if ( !sLayerName.isEmpty() )
    {
      sTableName = sLayerName;
      if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
      {
        // Extract GeometryName from sent 'table_name(field_name)' from layerName
        QStringList sa_list_name = sTableName.split( "(" );
        sGeometryColumn = sa_list_name[1].replace( ")", "" );
        sTableName = sa_list_name[0];
      }
    }
    QString sFields = QString( "name" );
    // truemarble-4km.sqlite
    QString sql = QStringLiteral( "SELECT " ).arg( sFields );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='name'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='description'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='format'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='minzoom'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='maxzoom'))," );
    sql += QStringLiteral( "(SELECT value FROM metadata WHERE (name='bounds'))," );
    sql += QStringLiteral( "(SELECT count(tbl_name) FROM sqlite_master WHERE ((type = 'table' OR type = 'view') AND (tbl_name IN ('metadata','tiles','images','map'))))" );
    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          QString sTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          if ( sTableName == sLayerName )
          {
            // We have found what we are looking for, prevent RL1/2 and Topology search
            bFoundLayerName = true;
          }
          // Add only if it does not already exist
          if ( !getSpatialiteDbLayer( sTableName, false ) )
          {
            SpatialiteDbLayer *dbLayer = new SpatialiteDbLayer( this );
            if ( dbLayer )
            {
              dbLayer->mAttributeFields.clear();
              dbLayer->mPrimaryKey.clear(); // cazzo cazzo cazzo
              dbLayer->mPrimaryKeyAttrs.clear();
              dbLayer->mDefaultValues.clear();
              dbLayer->mTopologyExportLayers.clear();
              dbLayer->mTopologyExportLayersDataSourceUris.clear();
              dbLayer->mNumberFeatures = 0;
              dbLayer->mLayerId = mLayersCount++;
              dbLayer->mTableName = sTableName;
              dbLayer->mLayerName = dbLayer->mTableName;
              dbLayer->setSrid( 4326 );
              if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
              {
                // If valid srid, mIsValid will be set to true
                dbLayer->mTitle = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
              }
              if ( ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
                   ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL ) )
              {
                dbLayer->mAbstract = QString( "format=%1 ; minzoom=%2 ; maxzoom=%3" ).arg( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) ) ).arg( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 3 ) ) ).arg( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) ) );
              }
              if ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL )
              {
                // Bounds: 11.249999999999993,43.06888777416961,14.062499999999986,44.08758502824516
                QStringList saBounds = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 5 ) ).split( "," );
                if ( saBounds.size() == 4 )
                {
                  QgsRectangle layerExtent( saBounds.at( 0 ).toDouble(), saBounds.at( 1 ).toDouble(), saBounds.at( 2 ).toDouble(), saBounds.at( 3 ).toDouble() );
                  dbLayer->setLayerExtent( layerExtent );
                }
              }
              if ( sqlite3_column_type( stmt, 6 ) != SQLITE_NULL )
              {
                SpatialiteLayerType type_MBTiles = MBTilesTable;
                if ( sqlite3_column_int( stmt, 6 ) > 2 )
                {
                  type_MBTiles = MBTilesView;
                }
                dbLayer->setLayerType( type_MBTiles );
              }
              if ( dbLayer->mIsValid )
              {
                i_check_count_mbtiles++;
                // Resolve unset settings
                dbLayer->prepare();
                if ( dbLayer->mLayerName == sLayerName )
                {
                  // We have found what we are looking for, prevent RL1/2 and Topology search
                  bFoundLayerName = true;
                }
                mDbLayers.insert( dbLayer->mLayerName, dbLayer );
              }
              else
              {
                delete dbLayer;
              }
            }
          }
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_mbtiles != mHasMBTilesTables )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = mIsValid;
    }
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbInfo::GetMBTilesLayersInfo: called with !mIsValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
bool SpatialiteDbInfo::GetGeoPackageLayersInfo( QString sLayerName )
{
  bool bRc = false;
  return bRc; // Not yet implemented
  if ( mIsValid )
  {
    sqlite3_stmt *stmt = nullptr;
    // sqlite3_stmt *stmtSubquery = nullptr;
    bool bFoundLayerName = false;
    int i_check_count_geopackage = mHasGeoPackageTables;
    int i_rc = 0;
    QString sTableName  = QString::null;
    QString sGeometryColumn = QString::null;
    if ( !sLayerName.isEmpty() )
    {
      sTableName = sLayerName;
      if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
      {
        // Extract GeometryName from sent 'table_name(field_name)' from layerName
        QStringList sa_list_name = sTableName.split( "(" );
        sGeometryColumn = sa_list_name[1].replace( ")", "" );
        sTableName = sa_list_name[0];
      }
    }
    QString sFields = QString( "table_name, geometry_column, row_count, extent_min_x, extent_min_y, extent_max_x, extent_max_y" );
    // truemarble-4km.sqlite
    QString sql = QStringLiteral( "SELECT %1 FROM layer_statistics WHERE ((raster_layer=1) OR ((raster_layer=0) AND (table_name LIKE '%_metadata')))" ).arg( sFields );
    if ( !sTableName.isEmpty() )
    {
      sql = QStringLiteral( "SELECT %1 FROM layer_statistics WHERE (((raster_layer=1) AND (table_name = '%2')) OR ((raster_layer=0) AND (table_name = '%2_metadata')))" ).arg( sFields ).arg( sTableName );
    }
    i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      bRc = true;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( i_check_count_geopackage != mHasGeoPackageTables )
    {
      // TODO possible warning that something was invalid
    }
    if ( !sLayerName.isEmpty() )
    {
      if ( bFoundLayerName )
      {
        bRc = bFoundLayerName;
      }
    }
    else
    {
      bRc = mIsValid;
    }
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbInfo::GetGeoPackageLayersInfo: called with !mIsValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
bool SpatialiteDbInfo::GetFdoOgrLayersInfo( QString sLayerName )
{
  bool bRc = false;
  return bRc; // Not yet implemented
  if ( mIsValid )
  {
    // sqlite3_stmt *stmt = nullptr;
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbInfo::GetFdoOgrLayersInfo: called with !mIsValid [%1]" ).arg( sLayerName ) );
  }
  return bRc;
}
bool SpatialiteDbInfo::readNonSpatialTables( )
{
  bool bRc = false;
  mNonSpatialTables.clear();
  sqlite3_stmt *stmt = nullptr;
  QString sWhere = QString( "WHERE ((type IN ('table','view','trigger')) AND (name NOT IN (SELECT table_name FROM vector_layers)))" );
  QString sql = QStringLiteral( "SELECT name,type FROM sqlite_master %1 ORDER BY  type,name;" ).arg( sWhere );
  // SELECT name,type FROM sqlite_master WHERE ((type IN ('table','view','trigger')) AND (name NOT LIKE 'idx_%') AND (name NOT IN (SELECT table_name FROM vector_layers)))
  // SELECT name,type FROM sqlite_master WHERE ((type IN ('table','view','trigger')) AND (name NOT IN (SELECT table_name FROM vector_layers)))
  int i_rc = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
  if ( i_rc == SQLITE_OK )
  {
    bRc = true;
    QMap<QString, QString> spatialiteTypes;
    spatialiteTypes.insert( "SpatialIndex", "Virtual-Internal" );
    spatialiteTypes.insert( "KNN", "Virtual-Internal" );
    spatialiteTypes.insert( "ElementaryGeometries", "Virtual-Internal" );
    spatialiteTypes.insert( "spatialite_history", "Spatialite-Internal" );
    spatialiteTypes.insert( "data_licenses", "Spatialite-Internal" );
    spatialiteTypes.insert( "networks", "Networks-Internal" );
    spatialiteTypes.insert( "geometry_columns", "Geometries-Internal" );
    spatialiteTypes.insert( "sql_statements_log", "Spatialite-Internal" );
    spatialiteTypes.insert( "geom_cols_ref_sys", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_auth", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_field_infos", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_statistics", "Geometries-Internal" );
    spatialiteTypes.insert( "geometry_columns_time", "Geometries-Internal" );
    spatialiteTypes.insert( "vector_layers", "SpatialTable-Internal" );
    spatialiteTypes.insert( "vector_layers_auth", "SpatialTable-Internal" );
    spatialiteTypes.insert( "vector_layers_field_infos", "SpatialTable-Internal" );
    spatialiteTypes.insert( "vector_layers_statistics", "SpatialTable-Internal" );
    spatialiteTypes.insert( "views_geometry_columns", "SpatialView-Internal" );
    spatialiteTypes.insert( "views_geometry_columns_auth", "SpatialView-Internal" );
    spatialiteTypes.insert( "views_geometry_columns_field_infos", "SpatialView-Internal" );
    spatialiteTypes.insert( "views_geometry_columns_statistics", "SpatialView-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns_auth", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns_field_infos", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "virts_geometry_columns_statistics", "VirtualShapes-Internal" );
    spatialiteTypes.insert( "raster_coverages", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages_keyword", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages_ref_sys", "RasterLite2-Internal" );
    spatialiteTypes.insert( "raster_coverages_srid", "RasterLite2-Internal" );
    spatialiteTypes.insert( "topologies", "Topology-Internal" );
    spatialiteTypes.insert( "layer_statistics", "Spatialite2-Internal" );
    spatialiteTypes.insert( "raster_pyramids", "RasterLite1-Internal" );
    spatialiteTypes.insert( "sqlite_sequence", "Sqlite3-Internal" );
    spatialiteTypes.insert( "spatial_ref_sys", "EPSG-Table" );
    spatialiteTypes.insert( "spatial_ref_sys_all", "EPSG-Internal" );
    spatialiteTypes.insert( "spatial_ref_sys_aux", "EPSG-Internal" );
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )  &&
           ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
      {
        QString sName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
        QString sType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        QString masterType = "Unknown-Data";
        if ( !spatialiteTypes.value( sName ).isEmpty() )
        {
          masterType = spatialiteTypes.value( sName );
        }
        else
        {
          if ( sName.startsWith( "SE_" ) )
          {
            masterType = "Styles-Internal";
          }
          else if ( sName.startsWith( "idx_" ) )
          {
            masterType = "SpatialIndex-Data";
          }
          else
          {
            if ( sType == "table" )
            {
              masterType = "Table-Data";
            }
            else if ( sType == "view" )
            {
              masterType = "View-Data";
            }
            else if ( sType == "trigger" )
            {
              masterType = "Trigger-Data";
            }
          }
        }
        if ( mNonSpatialTables.value( sName ).isEmpty() )
        {
          // sName is not contained in the Key-Field (returns value field)
          mNonSpatialTables.insert( sName, masterType );
        }
        // qDebug() << QString( " -I-> readNonSpatialTables -  count[%1] sName[%2] masterType[%3]" ).arg(mNonSpatialTables.count() ).arg( sName ).arg(masterType );
      }
    }
    sqlite3_finalize( stmt );
  }
  return bRc;
}
//-----------------------------------------------------------
//  SpatialiteDbLayer
//-----------------------------------------------------------
SpatialiteDbLayer::~SpatialiteDbLayer()
{
  mDbConnectionInfo = nullptr;
  for ( QMap<QString, SpatialiteDbLayer *>::iterator it = mTopologyExportLayers.begin(); it != mTopologyExportLayers.end(); ++it )
  {
    SpatialiteDbLayer *dbLayer = qobject_cast<SpatialiteDbLayer *>( it.value() );
    delete dbLayer;
  }
};
bool SpatialiteDbInfo::UpdateLayerStatistics( QStringList saLayers )
{
  bool bRc = false;
  if ( ( mIsValid ) && ( isDbSpatialite() ) )
  {
    char *errMsg = nullptr;
    if ( saLayers.size() < 1 )
    {
      // UpdateLayerStatistics for whole Database
      saLayers << QString();
    }
    if ( sqlite3_exec( dbSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg ) == SQLITE_OK )
    {
      bRc = true;
      for ( int i = 0; i < saLayers.size(); i++ )
      {
        QString sql;
        QString sTableName = saLayers.at( i ).trimmed();
        QString sGeometryColumn = QString();
        // Since this can be called from an external class, check if the Database file name was given
        if ( ( !sTableName.isEmpty() ) && ( sTableName.contains( getFileName(), Qt::CaseInsensitive ) ) )
        {
          // Assume that the whole Database is being requested [should never happen, but one never knows ...]
          sTableName = QString();
        }
        if ( !sTableName.isEmpty() )
        {
          // if 'sGeometryColumn' remains empty, all geometries of the TABLE will be done
          if ( ( sTableName.contains( "(" ) ) && ( sTableName.endsWith( QString( ")" ) ) ) )
          {
            // Extract GeometryName from sent 'table_name(field_name)' from layerName
            QStringList sa_list_name = sTableName.split( "(" );
            if ( sa_list_name.size() )
            {
              sGeometryColumn = QString( ",'%1'" ).arg( sa_list_name[1].replace( ")", "" ) );
              sTableName = sa_list_name[0];
            }
          }
          if ( mIsSpatialite40 )
          {
            sql = QStringLiteral( "SELECT InvalidateLayerStatistics('%1'%2),UpdateLayerStatistics('%1'%2)" ).arg( sTableName ).arg( sGeometryColumn );
          }
          else
          {
            sql = QStringLiteral( "SELECT UpdateLayerStatistics('%1'%2)" ).arg( sTableName ).arg( sGeometryColumn );
          }
        }
        else
        {
          if ( mIsSpatialite40 )
          {
            sql = QStringLiteral( "SELECT InvalidateLayerStatistics(),UpdateLayerStatistics()" );
          }
          else
          {
            sql = QStringLiteral( "SELECT UpdateLayerStatistics()" );
          }
        }
        if ( sqlite3_exec( dbSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
        {
          bRc = false;
          qDebug() << QString( "-I-> SpatialiteDbInfo::UpdateLayerStatistics rc=%3[%4] Layer[%1] sql[%2]" ).arg( saLayers.at( i ).trimmed() ).arg( sql ).arg( bRc ).arg( errMsg ? errMsg : "" );
          continue;
        }
        if ( sTableName.isEmpty() )
        {
          // ignore any other commands  since the whole Database has been done [should never happen]
          continue;
        }
      }
      if ( sqlite3_exec( dbSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg ) != SQLITE_OK )
      {
        bRc = false;
      }
    }
  }
  return bRc;
}
bool SpatialiteDbLayer::UpdateLayerStatistics()
{
  if ( ( mIsValid ) && ( mDbConnectionInfo->isDbSpatialite() ) )
  {
    return mDbConnectionInfo->UpdateLayerStatistics( ( QStringList() << mLayerName ) );
  }
  return false;
}
bool SpatialiteDbLayer::prepare()
{
  bool bChanged = false;
  //----------------------------------------------------------
  if ( mEnabledCapabilities == QgsVectorDataProvider::NoCapabilities )
  {
    getCapabilities();
    bChanged = true;
  }
  //----------------------------------------------------------
  return bChanged;
};
int SpatialiteDbLayer::setLayerInvalid( QString errCause )
{
  mIsValid = false;
  if ( !errCause.isEmpty() )
  {
    mErrors.append( errCause );
  }
  return errCause.size();
}
void SpatialiteDbLayer::setSpatialIndexType( int iSpatialIndexType )
{
  mSpatialIndexType = ( SpatialiteDbInfo::SpatialIndexType )iSpatialIndexType;
  mSpatialIndexTypeString = SpatialiteDbInfo::SpatialIndexTypeName( mSpatialIndexType );
}
void SpatialiteDbLayer::setLayerType( SpatialiteDbInfo::SpatialiteLayerType layerType )
{
  mLayerType = layerType;
  mLayerTypeString = SpatialiteDbInfo::SpatialiteLayerTypeName( mLayerType );
}
void SpatialiteDbLayer::setGeomType( int iGeomType )
{
  mGeometryType = ( QgsWkbTypes::Type )iGeomType;
  mGeometryTypeString = QgsWkbTypes::displayString( mGeometryType );
}
QgsVectorDataProvider::Capabilities SpatialiteDbLayer::getCapabilities( bool bUpdate )
{
  //----------------------------------------------------------
  if ( ( mEnabledCapabilities == QgsVectorDataProvider::NoCapabilities ) || ( bUpdate ) )
  {
    if ( bUpdate )
    {
      GetLayerSettings( );
    }
    mEnabledCapabilities = mPrimaryKey.isEmpty() ? QgsVectorDataProvider::Capabilities() : ( QgsVectorDataProvider::SelectAtId );
    switch ( mLayerType )
    {
      case SpatialiteDbInfo::TopopogyExport:
      case SpatialiteDbInfo::SpatialTable:
        mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::FastTruncate;
        if ( !mGeometryColumn.isEmpty() )
          mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
        mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
        mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
        mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes;
        mEnabledCapabilities |= QgsVectorDataProvider::CreateAttributeIndex;
        break;
      case SpatialiteDbInfo::SpatialView:
        if ( !mLayerReadOnly )
        {
          if ( mTriggerDelete )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::FastTruncate;
          }
          if ( mTriggerUpdate )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
            mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
          }
          if ( mTriggerInsert )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
            mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes;
            mEnabledCapabilities |= QgsVectorDataProvider::CreateAttributeIndex;
          }
        }
        break;
      case SpatialiteDbInfo::SpatialiteTopopogy:
      case SpatialiteDbInfo::VectorStyle:
      case SpatialiteDbInfo::RasterStyle:
        mEnabledCapabilities = QgsVectorDataProvider::NoCapabilities;
        break;
      default:
        setLayerInvalid( QString( "[%1] Capabilities: Unexpected Layer-Type[%2]." ).arg( mLayerName ).arg( getLayerTypeString() ) );
        break;
    }
  }
  //----------------------------------------------------------
  return mEnabledCapabilities;
};
bool SpatialiteDbLayer::setSrid( int iSrid )
{
  bool bRc = false;
  if ( mIsValid )
  {
    int i_rc = 0;
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    sql = QStringLiteral( "SELECT auth_name||':'||auth_srid,proj4text FROM spatial_ref_sys WHERE srid=%1" ).arg( iSrid );
    i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) )
        {
          mAuthId = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          mProj4text = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          mSrid = iSrid;
          bRc = true;
        }
      }
      sqlite3_finalize( stmt );
    }
    mIsValid = bRc;
  }
  return bRc;
}
QgsRectangle SpatialiteDbLayer::getLayerExtent( bool bUpdate, bool bUpdateStatistics )
{
  if ( ( mIsValid ) && ( ( bUpdate ) || ( bUpdateStatistics ) ) )
  {
    int i_rc = 0;
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    QString sourceTable;
    QString parmTable;
    QString parmField;
    QString parmGeometryColumn;
    QString sFields;
    switch ( mLayerType )
    {
      case SpatialiteDbInfo::TopopogyExport:
      case SpatialiteDbInfo::SpatialTable:
      case SpatialiteDbInfo::SpatialView:
      case SpatialiteDbInfo::VirtualShape:
        sourceTable = "vector_layers_statistics";
        parmTable = "table_name";
        parmField = "geometry_column";
        parmGeometryColumn = QString( " AND (%1 = '%2')" ).arg( parmField ).arg( mGeometryColumn );
        sFields = QString( "extent_minx,extent_miny,extent_maxx,extent_maxy, row_count" );
        break;
      case SpatialiteDbInfo::RasterLite2:
        sourceTable = "raster_coverages";
        parmTable = "coverage_name";
        parmField = "";
        parmGeometryColumn = "";
        sFields = QString( "extent_minx,extent_miny,extent_maxx,extent_maxy" );
        break;
      default:
        bUpdate = false;
        bUpdateStatistics = false;
        break;
    }
    if ( bUpdateStatistics )
    {
      sql = QStringLiteral( "SELECT InvalidateLayerStatistics('%1','%2'),UpdateLayerStatistics('%1','%2')" ).arg( mTableName ).arg( mGeometryColumn );
      ( void )sqlite3_exec( getSqliteHandle(), sql.toUtf8().constData(), nullptr, 0, nullptr );
    }
    if ( bUpdate )
    {
      sql = QStringLiteral( "SELECT %1 FROM %2 WHERE (%3 = '%4')%5" ).arg( sFields ).arg( sourceTable ).arg( parmTable ).arg( mTableName ).arg( parmGeometryColumn );
      i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
               ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) )
          {
            QgsRectangle layerExtent( sqlite3_column_double( stmt, 0 ), sqlite3_column_double( stmt, 1 ), sqlite3_column_double( stmt, 2 ), sqlite3_column_double( stmt, 3 ) );
            setLayerExtent( layerExtent );
          }
          if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
          {
            mNumberFeatures = sqlite3_column_int64( stmt, 4 );
          }
        }
      }
    }
  }
  return mLayerExtent;
}
long SpatialiteDbLayer::getNumberFeatures( bool bUpdateStatistics )
{
  if ( ( mIsValid ) && ( bUpdateStatistics ) )
  {
    getLayerExtent( true, true );
  }
  return mNumberFeatures;
}
int SpatialiteDbLayer::setSpatialiteAttributeFields()
{
  mAttributeFields.clear();
  mPrimaryKeyAttrs.clear();
  mDefaultValues.clear();
  sqlite3_stmt *stmt = nullptr;
  int pk_cid = -1;
  // Missing entries in 'vector_layers_field_infos' TABLE.
  // Cause could be: when properly registered TABLE or VIEW is empty
  // dbLayer->setLayerInvalid( QString( "GetDbLayersInfo [%1] Missing entries in 'vector_layers_field_infos'. TABLE may be empty." ).arg( sSearchLayer ) );
  QString sql = QStringLiteral( "PRAGMA table_info(\"%1\")" ).arg( mTableName );
  // PRAGMA table_info('pg_grenzkommando_1985')
  int i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
  if ( i_rc == SQLITE_OK )
  {
    int cid = 0;    // 0
    QString sName; // 1
    QString sType;  // 2
    // bool notNull;      // 3
    QString sDefaultValue;  // 4
    // int isPK;  // 5
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      if ( ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) &&
           ( sqlite3_column_type( stmt, 5 ) != SQLITE_NULL ) )
      {
        cid = sqlite3_column_int( stmt, 0 );
        sName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
        sType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
        if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
        {
          // Not an error if NULL (which it often is)
          sDefaultValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
        }
        if ( mLayerType == SpatialiteDbInfo::SpatialView )
        {
          // mPrimaryKey was retrieved from 'views_geometry_columns'
          if ( mPrimaryKey == sName )
          {
            mPrimaryKeyAttrs << mAttributeFields.size();
            // Error if primary Key was not found
            pk_cid = cid;
          }
        }
        if ( sName.toLower() != mGeometryColumn.toLower() )
        {
          QVariant defaultVariant; // Will be set during GetSpatialiteQgsField.
          QgsField column_field = SpatialiteDbLayer::GetSpatialiteQgsField( sName, sType, sDefaultValue, defaultVariant );
          if ( sqlite3_column_int( stmt, 3 ) == 1 )
          {
            // column 4[3] tells us if the field is nullable. Should use that info...
            QgsFieldConstraints constraints = column_field.constraints();
            constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
            column_field.setConstraints( constraints );
          }
          if ( sqlite3_column_int( stmt, 5 ) == 1 )
          {
            if ( mLayerType == SpatialiteDbInfo::SpatialTable )
            {
              // Only if unique pk [SpatialView will almost never have this set here as 1]
              if ( mPrimaryKey.isEmpty() )
              {
                mPrimaryKey = sName;
                pk_cid = cid;
                mPrimaryKeyAttrs << mAttributeFields.size();
              }
              else
              {
                mPrimaryKey.clear();
                mPrimaryKeyAttrs.clear();
              }
            }
          }
          // As done in QgsPostgresProvider::loadFields
          mDefaultValues.insert( mAttributeFields.size(), defaultVariant );
          mAttributeFields.append( column_field );
        }
      }
    }
    sqlite3_finalize( stmt );
  }
  if ( ( mLayerType == SpatialiteDbInfo::SpatialView ) && ( pk_cid < 0 ) )
  {
    mIsValid = false;
    mErrors.append( QString( "-E-> SpatialiteDbLayer::setSpatialiteAttributeFields[%1] Layername[%2] PrimaryKey[%3] from 'views_geometry_columns',  was not found." ).arg( mLayerTypeString ).arg( mLayerName ).arg( mPrimaryKey ) );
    return 0;
  }
  if ( mLayerType == SpatialiteDbInfo::SpatialTable )
  {
    sql = QStringLiteral( "SELECT " );
    sql += QStringLiteral( "(SELECT sql FROM sqlite_master WHERE type='table' AND name=\"%1\"))," ).arg( mTableName );
    sql += QStringLiteral( "(SELECT ROWID FROM \"%1\" LIMIT 1)" ).arg( mTableName );
    i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          QString sqlTable = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
          // extract definition
          QRegularExpression re( QStringLiteral( "\\((.*)\\)" ) );
          QRegularExpressionMatch match = re.match( sqlTable );
          if ( match.hasMatch() )
          {
            QString matched = match.captured( 1 );
            Q_FOREACH ( QString field, matched.split( ',' ) )
            {
              field = field.trimmed();
              QString fieldName = field.left( field.indexOf( ' ' ) );
              QString definition = field.mid( field.indexOf( ' ' ) + 1 );
              int fieldIndex = mAttributeFields.lookupField( fieldName );
              if ( fieldIndex >= 0 )
              {
                QgsFieldConstraints constraints = mAttributeFields.at( fieldIndex ).constraints();
                if ( definition.contains( "unique", Qt::CaseInsensitive ) || definition.contains( "primary key", Qt::CaseInsensitive ) )
                  constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
                if ( definition.contains( "not null", Qt::CaseInsensitive ) || definition.contains( "primary key", Qt::CaseInsensitive ) )
                  constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
                mAttributeFields.at( fieldIndex ).setConstraints( constraints );
              }
            }
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          mPrimaryKey.clear();
          mPrimaryKey = QStringLiteral( "ROWID" );
          mPrimaryKeyAttrs.clear();
        }
      }
      sqlite3_finalize( stmt );
    }
  }
  if ( ( mLayerType == SpatialiteDbInfo::SpatialView ) && ( !mLayerReadOnly ) && ( !mViewTableName.isEmpty() ) )
  {
    // PRAGMA table_info('segments_ortsteile_1938') ; PRAGMA table_info('berlin_admin_segments') ;
    // Notes: PRAGMA table_info will rarely contain useful information for notnull or default-values
    // This information can be retrieved from the underlining TABLE, IF they share the same column names.
    // Since with SpatialViews the underlineing table must be registered, we will use that
    sql = QStringLiteral( "PRAGMA table_info(\"%1\")" ).arg( mViewTableName );
    i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( i_rc == SQLITE_OK )
    {
      QString sName; // 1
      QString sType;  // 2
      QString sDefaultValue;  // 4
      int fieldIndex;
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL ) &&
             ( sqlite3_column_type( stmt, 3 ) != SQLITE_NULL ) )
        {
          sName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
          fieldIndex = mAttributeFields.indexFromName( sName );
          if ( fieldIndex < 0 )
          {
            // Active mGeometryColumn [or unknown field] should be ignored
            continue;
          }
          sType = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
          if ( sqlite3_column_type( stmt, 4 ) != SQLITE_NULL )
            sDefaultValue = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 4 ) );
          QVariant defaultVariant; // Will be set during GetSpatialiteQgsField.
          QgsField column_field = SpatialiteDbLayer::GetSpatialiteQgsField( sName, sType, sDefaultValue, defaultVariant );
          // Replace the (possibly) changed defaut-value
          mDefaultValues.insert( fieldIndex, defaultVariant );
          if ( sqlite3_column_int( stmt, 3 ) == 1 )
          {
            // column 4[3] tells us if the field is nullable. Should use that info...
            QgsFieldConstraints constraints = column_field.constraints();
            constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
            column_field.setConstraints( constraints );
            mAttributeFields.at( fieldIndex ).setConstraints( constraints );
          }
          mAttributeFields.at( fieldIndex ).setType( column_field.type() );
          mAttributeFields.at( fieldIndex ).setTypeName( column_field.typeName() );
          if ( column_field.typeName() == "TEXT" )
          {
            mAttributeFields.at( fieldIndex ).setSubType( column_field.subType() );
          }
          /*
          QString defaultVal = mDefaultValues.value( fieldIndex, QVariant() ).toString();
          qDebug() << QString( "-I-> setSpatialiteAttributeFields -defaults- ViewName[%1] ViewTableName[%2] ColumnName[%3] fieldIndex[%4] name[%5] value[%6,'%7'] expected[%8] comment[%9]" ).arg(mTableName).arg( mViewTableName ).arg( sName ).arg( fieldIndex ).arg( getAttributeFields().at( fieldIndex ).name() ).arg( getAttributeFields().at( fieldIndex ).typeName() ).arg(defaultVal).arg( sDefaultValue ).arg(defaultVariant.toString());
          */
        }
      }
      sqlite3_finalize( stmt );
    }
    if ( !mViewTableName.isEmpty() )
    {
      if ( ( mTriggerInsert ) || ( mTriggerUpdate ) )
      {
        QMap<QString, QString> view_defaults;
        QString sqlInsert;
        QString sqlUpdate;
        QString sql_base = QStringLiteral( "(SELECT sql FROM  sqlite_master WHERE (type = 'trigger' AND tbl_name = '%1' AND (instr(upper(sql),'INSTEAD OF %2') > 0)))" );
        sql = QStringLiteral( "SELECT " );
        if ( mTriggerInsert )
        {
          sql += sql_base.arg( mTableName ).arg( "INSERT" );
          if ( mTriggerUpdate )
          {
            sql += ",";
          }
        }
        if ( mTriggerUpdate )
        {
          sql += sql_base.arg( mTableName ).arg( "UPDATE" );
        }
        i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
        if ( i_rc == SQLITE_OK )
        {
          while ( sqlite3_step( stmt ) == SQLITE_ROW )
          {
            if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
            {
              QString sqlTable = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
              if ( mTriggerInsert )
              {
                sqlInsert = sqlTable;
              }
              else
              {
                sqlUpdate = sqlTable;
              }
            }
            if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
            {
              sqlUpdate = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            }
          }
          sqlite3_finalize( stmt );
          if ( !sqlUpdate.isEmpty() )
          {
            QRegularExpression re( QStringLiteral( "SET (.*) WHERE" ), QRegularExpression::CaseInsensitiveOption );
            // Note: remove line feeds (new line), tabs and carriage returns, plus multiple blanks !
            QString sqlReplaced = sqlUpdate.remove( QRegExp( "[\\n\\t\\r]" ) ).replace( "  ", " " ).replace( "VALUES (", "VALUES(" );
            QRegularExpressionMatch match = re.match( sqlReplaced );
            if ( ( match.hasMatch() ) && ( match.lastCapturedIndex() == 1 ) )
            {
              QStringList sa_fields = match.captured( 1 ).split( "," );
              if ( sa_fields.size() )
              {
                for ( int i = 0 ; i < sa_fields.size(); i++ )
                {
                  QString sTrimmed = sa_fields[i].trimmed();
                  if ( ( !sTrimmed.toUpper().contains( "NEW." ) ) ||
                       ( !sTrimmed.startsWith( "--" ) ) ||
                       ( sTrimmed.toUpper().contains( "CASE WHEN NEW." ) ) )
                  {
                    //  gcp_type = CASE WHEN NEW.gcp_type IS NULL THEN 700 ELSE NEW.gcp_type END,
                    QStringList sa_values = sTrimmed.split( "=" );
                    // Ignore any values that start with a '(' [Sub-Query]
                    if ( ( sa_values.size() ) && ( sa_values.size() > 1 ) &&
                         ( !sa_values[1].trimmed().startsWith( "(" ) ) &&
                         ( !sa_values[1].trimmed().startsWith( "ST_" ) ) &&
                         ( !sa_values[1].trimmed().toUpper().startsWith( "NEW." ) ) )
                    {
                      QString sField = sa_values[0].trimmed(); // Result here: gcp_type
                      sTrimmed = sa_values[1].trimmed();
                      if ( ( sTrimmed.toUpper().startsWith( "CASE WHEN NEW." ) ) &&
                           ( sTrimmed.toUpper().contains( "IS NULL THEN" ) ) &&
                           ( sTrimmed.toUpper().contains( "ELSE NEW." ) ) )
                      {
                        //  CASE WHEN NEW.gcp_type IS NULL THEN 700 ELSE NEW.gcp_type END,
                        QRegularExpression re_case( QStringLiteral( "IS NULL THEN (.*) ELSE NEW." ), QRegularExpression::CaseInsensitiveOption );
                        QRegularExpressionMatch match_case = re_case.match( sTrimmed );
                        if ( ( match_case.hasMatch() ) && ( match_case.lastCapturedIndex() == 1 ) )
                        {
                          sTrimmed = match_case.captured( 1 ); // Result here: 700
                        }
                      }
                      // remove any possible garbage, such as comments etc. at the end of the line
                      sa_values = sTrimmed.split( QRegExp( "\\s+" ) );
                      QString sValue = sa_values[0].trimmed();
                      if ( sField.startsWith( "--" ) )
                      {
                        // remove any possible garbage, such as comments etc. that was before the line
                        sa_values = sField.trimmed().split( QRegExp( "\\s+" ) );
                        sField = sa_values[sa_values.size() - 1];
                      }
                      // The default value of a writable view may be different than that of the underlining table.
                      view_defaults.insert( sField, sValue.remove( "\"" ) );
                      // qDebug() << QString( "SpatialiteDbLayer::GetLayerSettings(%1) -UPDATE -z- Layername[%2] field[%3] value[%4] " ).arg( mLayerTypeString ).arg( mLayerName ).arg( sField ).arg( sValue );
                    }
                  }
                }
              }
            }
            else
            {
              // qDebug() << QString( "-W-> SpatialiteDbLayer::GetLayerSettings(%1) -UPDATE -z- Layername[%2] Expression[%3] match_count[%4] sql[%5] " ).arg( mLayerTypeString ).arg( mLayerName ).arg( "SET (.*) WHERE" ).arg( match.lastCapturedIndex() ).arg( sqlReplaced );
            }
          }
          if ( !sqlInsert.isEmpty() )
          {
            // Warning: the order of the given value MUST match the order of the fields !
            // - this can be tricky, depending on how the sql was written. Thus UPDATE will first be used (when found), since it is easier to parse.
            QRegularExpression re( QStringLiteral( "\\((.*)\\) VALUES\\((.*)\\)" ), QRegularExpression::CaseInsensitiveOption );
            // Note: remove line feeds (new line), tabs and carriage returns, plus multiple blanks !
            QString sqlReplaced = sqlInsert.remove( QRegExp( "[\\n\\t\\r]" ) ).replace( "  ", " " ).replace( "VALUES (", "VALUES(" );
            QRegularExpressionMatch match = re.match( sqlReplaced );
            if ( ( match.hasMatch() ) && ( match.lastCapturedIndex() == 2 ) )
            {
              QStringList sa_fields = match.captured( 1 ).split( "," );
              if ( sa_fields.size() )
              {
                QStringList sa_values = match.captured( 2 ).split( "," );
                if ( sa_fields.size() == sa_values.size() )
                {
                  for ( int i = 0 ; i < sa_values.size(); i++ )
                  {
                    QString sTrimmed = sa_values[i].trimmed();
                    if ( ( !sTrimmed.toUpper().startsWith( "NEW." ) ) &&
                         ( !sTrimmed.startsWith( "--" ) ) &&
                         ( sTrimmed.toUpper().contains( "CASE WHEN NEW." ) ) )
                    {
                      if ( ( sTrimmed.toUpper().startsWith( "CASE WHEN NEW." ) ) &&
                           ( sTrimmed.toUpper().contains( "IS NULL THEN" ) ) &&
                           ( sTrimmed.toUpper().contains( "ELSE NEW." ) ) )
                      {
                        // CASE WHEN NEW.gcp_type IS NULL THEN 700 ELSE NEW.gcp_type END,
                        QRegularExpression re_case( QStringLiteral( "IS NULL THEN (.*) ELSE NEW." ), QRegularExpression::CaseInsensitiveOption );
                        QRegularExpressionMatch match_case = re_case.match( sTrimmed );
                        if ( ( match_case.hasMatch() ) && ( match_case.lastCapturedIndex() == 1 ) )
                        {
                          sTrimmed = match_case.captured( 1 ).trimmed(); // Result here: 700
                        }
                      }
                      QString sField = sa_fields[i].trimmed();
                      QString sValue = sTrimmed.remove( "\"" );
                      // The default value of a writable view may be different than that of the underlining table.
                      // If we already have this value from the UPDATE TRIGGER, then we can ignore this.
                      if ( !view_defaults.value( sField ).isEmpty() )
                      {
                        view_defaults.insert( sField, sValue );
                        // qDebug() << QString( "SpatialiteDbLayer::GetLayerSettings(%1) -INSERT -z- Layername[%2] field[%3]  value[%4] " ).arg( mLayerTypeString ).arg( mLayerName ).arg( sField ).arg( sValue );
                      }
                    }
                  }
                }
              }
            }
            else
            {
              // qDebug() << QString( "-W-> SpatialiteDbLayer::GetLayerSettings(%1) -INSERT -z- Layername[%2] Expression[%3] match_count[%4] sql[%5] " ).arg( mLayerTypeString ).arg( mLayerName ).arg( "\\((.*)\\) VALUES\\((.*)\\)" ).arg( match.lastCapturedIndex() ).arg( sqlReplaced );
            }
          }
        }
        for ( QMap<QString, QString>::iterator itFields = view_defaults.begin(); itFields != view_defaults.end(); ++itFields )
        {
          int fieldIndex = mAttributeFields.lookupField( itFields.key() );
          if ( fieldIndex >= 0 )
          {
            QVariant defaultVariant; // Will be set during GetSpatialiteQgsField.
            QgsField column_field = SpatialiteDbLayer::GetSpatialiteQgsField( itFields.key(), mAttributeFields.at( fieldIndex ).typeName(), itFields.value(), defaultVariant );
            // Replace the (possibly) changed defaut-value
            mDefaultValues.insert( fieldIndex, defaultVariant );
          }
        }
      }
    }
  }
  return mAttributeFields.size();
}
bool SpatialiteDbLayer::GetLayerSettings()
{
  bool bRc = false;
  if ( mIsValid )
  {
    int i_rc = 0;
    sqlite3_stmt *stmt = nullptr;
    QString sSearchTable = mTableName;
    QString sql;
    if ( mLayerType == SpatialiteDbInfo::SpatialView )
    {
      //-----------------------------------------
      //-- Retrieve from View ----------------
      //-- mPrimaryKey
      //-- mViewTableName
      //-- mViewTableGeometryColumn
      //-----------------------------------------
      // SELECT view_rowid, f_table_name, f_geometry_column FROM views_geometry_columns WHERE (view_name="positions_1925")
      sql = QStringLiteral( "SELECT view_rowid, f_table_name, f_geometry_column FROM views_geometry_columns WHERE ((view_name=\"%1\") AND (view_geometry=\"%2\"))" ).arg( mTableName ).arg( mGeometryColumn );
      i_rc = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( i_rc == SQLITE_OK )
      {
        while ( sqlite3_step( stmt ) == SQLITE_ROW )
        {
          if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
          {
            QString viewRowid = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            if ( mPrimaryKey.isEmpty() )
              mPrimaryKey = viewRowid;
          }
          if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
          {
            mViewTableName = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 1 ) );
            sSearchTable = mViewTableName;
          }
          if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
          {
            mViewTableGeometryColumn = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 2 ) );
          }
        }
        sqlite3_finalize( stmt );
      }
      if ( !mViewTableName.isEmpty() )
      {
        mLayerViewTableName = mViewTableName;
        if ( !mViewTableGeometryColumn.isEmpty() )
        {
          mLayerViewTableName = QString( "%1(%2)" ).arg( mLayerViewTableName ).arg( mViewTableGeometryColumn );
        }
      }
    }
    //-----------------------------------------
    //-- Fill ----------------------------------
    //-- mAttributeFields
    //-- mDefaultValues
    //-----------------------------------------
    if ( setSpatialiteAttributeFields() <= 0 )
    {
      mIsValid = false;
      qDebug() << QString( "-E->SpatialiteDbLayer::GetLayerSettings -4- IsValid[%1] Layer-Type[%2] Layer-Name[%3] sql[%4]" ).arg( mIsValid ).arg( mLayerTypeString ).arg( mLayerName ).arg( sql );
    }
    //-----------------------------------------
    //-----------------------------------------
    //---setting EnabledCapabilities---------
    //-----------------------------------------
    prepare();
    bRc = mIsValid;
  }
  else
  {
    mErrors.append( QString( "SpatialiteDbLayer::GetLayerSettings: called with !mIsValid [%1]" ).arg( mLayerName ) );
  }
  return bRc;
}
bool SpatialiteDbLayer::GetViewTriggers()
{
  if ( ( mDbConnectionInfo ) && ( mIsValid ) )
  {
    if ( ( mLayerType == SpatialiteDbInfo::SpatialTable ) || ( mLayerType == SpatialiteDbInfo::VirtualShape ) )
    {
      if ( mLayerType == SpatialiteDbInfo::SpatialTable )
      {
        mLayerReadOnly = 0; // default is 1
      }
      return mLayerReadOnly;
    }
    sqlite3_stmt *stmt = nullptr;
    QString sql;
    QString sql_base = QStringLiteral( "(SELECT Exists(SELECT rootpage FROM  sqlite_master WHERE (type = 'trigger' AND tbl_name = '%1' AND (instr(upper(sql),'INSTEAD OF %2') > 0))))" );
    sql = QStringLiteral( "SELECT " );
    sql += sql_base.arg( mTableName ).arg( "INSERT" ) + ",";
    sql += sql_base.arg( mTableName ).arg( "UPDATE" ) + ",";
    sql += sql_base.arg( mTableName ).arg( "DELETE" );
    int ret = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( ret == SQLITE_OK )
    {
      while ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        if ( sqlite3_column_type( stmt, 0 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 0 ) == 1 )
          {
            mTriggerInsert = true;
            mLayerReadOnly = false;
          }
        }
        if ( sqlite3_column_type( stmt, 1 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 1 ) == 1 )
          {
            mTriggerUpdate = true;
            mLayerReadOnly = false;
          }
        }
        if ( sqlite3_column_type( stmt, 2 ) != SQLITE_NULL )
        {
          if ( sqlite3_column_int( stmt, 2 ) == 1 )
          {
            mTriggerDelete = true;
            mLayerReadOnly = false;
          }
        }
      }
    }
    return mLayerReadOnly;
  }
  return false;
}
QgsField SpatialiteDbLayer::GetSpatialiteQgsField( const QString sName, const QString sType, const QString sDefaultValue, QVariant &defaultVariant )
{
  QgsField column_field;
  column_field.setName( sName );
  QgsWkbTypes::Type geometryType = SpatialiteDbLayer::GetGeometryTypeLegacy( sType );
  if ( geometryType != QgsWkbTypes::Unknown )
  {
    defaultVariant = sDefaultValue.toUtf8().constData();
    column_field.setType( QVariant::ByteArray );
    column_field.setTypeName( sType );
    return column_field;
  }
  else
  {
    QString type = "TEXT";
    QVariant::Type fieldType = QVariant::String; // default: SQLITE_TEXT
    if ( sType == "INTEGER" )
    {
      fieldType = QVariant::LongLong;
      type = "INTEGER";
      defaultVariant = sDefaultValue.toLongLong();
    }
    else if ( sType == "DOUBLE" )
    {
      fieldType = QVariant::Double;
      type = "DOUBLE";
      defaultVariant = sDefaultValue.toDouble();
    }
    else if ( sType == "DATE" )
    {
      fieldType = QVariant::Date;
      type = "DATE";
      QString date_DefaultValue = sDefaultValue;
      // Detailed Description: The minimum value for QDateTimeEdit is 14 September 1752. You can change this by calling setMinimumDate(), taking into account that the minimum value for QDate is 2 January 4713BC.
      // minimumDate: By default, this property contains a date that refers to September 14, 1752. The minimum date must be at least the first day in year 100, otherwise setMinimumDate() has no effect.
      date_DefaultValue.replace( 0x22, "" ).replace( 0x27, "" ); // remove any " or ' - 1752-09-14 minimumDateTime
      defaultVariant = QDate::fromString( date_DefaultValue, Qt::ISODate ); // "YYYY-MM-DD"=Qt::ISODate
    }
    else if ( sType == "DATETIME" )
    {
      fieldType = QVariant::DateTime;
      type = "DATETIME";
      QString date_DefaultValue = sDefaultValue;
      date_DefaultValue.replace( 0x22, "" ).replace( 0x27, "" ); // remove any " or '
      defaultVariant = QDate::fromString( date_DefaultValue, Qt::ISODate ); // "YYYY-MM-DD"=Qt::ISODate
    }
    else if ( sType == "TIME" )
    {
      fieldType = QVariant::Time;
      type = "TIME";
      QString date_DefaultValue = sDefaultValue;
      date_DefaultValue.replace( 0x22, "" ).replace( 0x27, "" ); // remove any " or '
      defaultVariant = QDate::fromString( date_DefaultValue, Qt::ISODate ); // "YYYY-MM-DD"=Qt::ISODate
    }
    else if ( sType == "BLOB" )
    {
      fieldType = QVariant::ByteArray;
      type = "BLOB";
      defaultVariant = sDefaultValue.toUtf8().constData();
    }
    else
    {
      QString defaultString = sDefaultValue;
      if ( defaultString.startsWith( '\'' ) )
        defaultString = defaultString.remove( 0, 1 );
      if ( defaultString.endsWith( '\'' ) )
        defaultString.chop( 1 );
      defaultString.replace( "''", "'" );
      defaultVariant = defaultString;
      // if the type seems unknown, fix it with what we actually have
      SpatialiteDbInfo::TypeSubType typeSubType = SpatialiteDbInfo::GetVariantType( sType );
      column_field.setType( typeSubType.first );
      column_field.setSubType( typeSubType.second );
    }
    if ( type != "TEXT" )
    {
      column_field.setType( fieldType );
    }
    column_field.setTypeName( type );
    // column_field.convertCompatible( defaultVariant );
    return column_field;
  }
}
QgsWkbTypes::Type SpatialiteDbLayer::GetGeometryTypeLegacy( const QString spatialiteGeometryType, const QString spatialiteGeometryDimension )
{
  QgsWkbTypes::Type geometryType = QgsWkbTypes::Unknown;
  int geometryDimension = GAIA_XY;
  QString sGeometryDimension = spatialiteGeometryDimension;
  if ( sGeometryDimension.isEmpty() )
  {
    sGeometryDimension = "XY";
  }
  if ( spatialiteGeometryType == "POINT" )
  {
    geometryType = QgsWkbTypes::Point;
  }
  else if ( spatialiteGeometryType == "POINTZ" )
  {
    sGeometryDimension = "XYZ";
    geometryType = QgsWkbTypes::Point;
  }
  else if ( spatialiteGeometryType == "POINTM" )
  {
    geometryType = QgsWkbTypes::Point;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "POINTZM" )
  {
    geometryType = QgsWkbTypes::Point;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "MULTIPOINT" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
  }
  else if ( spatialiteGeometryType == "MULTIPOINTZ" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "MULTIPOINTM" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "MULTIPOINTZM" )
  {
    geometryType = QgsWkbTypes::MultiPoint;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "LINESTRING" )
  {
    geometryType = QgsWkbTypes::LineString;
  }
  else if ( spatialiteGeometryType == "LINESTRINGZ" )
  {
    geometryType = QgsWkbTypes::LineString;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "LINESTRINGM" )
  {
    geometryType = QgsWkbTypes::LineString;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "LINESTRINGZM" )
  {
    geometryType = QgsWkbTypes::LineString;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "MULTILINESTRING" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
  }
  else if ( spatialiteGeometryType == "MULTILINESTRINGZ" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "MULTILINESTRINGM" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "MULTILINESTRINGZM" )
  {
    geometryType = QgsWkbTypes::MultiLineString;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "POLYGON" )
  {
    geometryType = QgsWkbTypes::Polygon;
  }
  else if ( spatialiteGeometryType == "POLYGONZ" )
  {
    geometryType = QgsWkbTypes::Polygon;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "POLYGONM" )
  {
    geometryType = QgsWkbTypes::Polygon;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "POLYGONZM" )
  {
    geometryType = QgsWkbTypes::Polygon;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGON" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGONZ" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGONM" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "MULTIPOLYGONZM" )
  {
    geometryType = QgsWkbTypes::MultiPolygon;
    sGeometryDimension = "XYZM";
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTION" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTIONZ" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
    sGeometryDimension = "XYZ";
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTIONM" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
    sGeometryDimension = "XYM";
  }
  else if ( spatialiteGeometryType == "GEOMETRYCOLLECTIONZM" )
  {
    geometryType = QgsWkbTypes::GeometryCollection;
    sGeometryDimension = "XYZM";
  }
  if ( sGeometryDimension == "XYZ" )
  {
    geometryDimension = GAIA_XY_Z;
  }
  else if ( sGeometryDimension == "XYM" )
  {
    geometryDimension = GAIA_XY_M;
  }
  else if ( sGeometryDimension == "XYZM" )
  {
    geometryDimension = GAIA_XY_Z_M;
  }
  switch ( geometryDimension )
  {
    case GAIA_XY_Z:
    {
      switch ( geometryType )
      {
        default:
        case QgsWkbTypes::Point:
          geometryType = QgsWkbTypes::PointZ;
          break;
        case QgsWkbTypes::MultiPoint:
          geometryType = QgsWkbTypes::MultiPointZ;
          break;
        case QgsWkbTypes::LineString:
          geometryType = QgsWkbTypes::LineStringZ;
          break;
        case QgsWkbTypes::MultiLineString:
          geometryType = QgsWkbTypes::MultiLineStringZ;
          break;
        case QgsWkbTypes::Polygon:
          geometryType = QgsWkbTypes::PolygonZ;
          break;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QgsWkbTypes::MultiPolygonZ;
          break;
        case QgsWkbTypes::GeometryCollection:
          geometryType = QgsWkbTypes::GeometryCollectionZ;
          break;
      }
    }
    break;
    case GAIA_XY_M:
    {
      switch ( geometryType )
      {
        default:
        case QgsWkbTypes::Point:
          geometryType = QgsWkbTypes::PointM;
          break;
        case QgsWkbTypes::MultiPoint:
          geometryType = QgsWkbTypes::MultiPointM;
          break;
        case QgsWkbTypes::LineString:
          geometryType = QgsWkbTypes::LineStringM;
          break;
        case QgsWkbTypes::MultiLineString:
          geometryType = QgsWkbTypes::MultiLineStringM;
          break;
        case QgsWkbTypes::Polygon:
          geometryType = QgsWkbTypes::PolygonM;
          break;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QgsWkbTypes::MultiPolygonM;
          break;
        case QgsWkbTypes::GeometryCollection:
          geometryType = QgsWkbTypes::GeometryCollectionM;
          break;
      }
    }
    break;
    case GAIA_XY_Z_M:
    {
      switch ( geometryType )
      {
        default:
        case QgsWkbTypes::Point:
          geometryType = QgsWkbTypes::PointZM;
          break;
        case QgsWkbTypes::MultiPoint:
          geometryType = QgsWkbTypes::MultiPointZM;
          break;
        case QgsWkbTypes::LineString:
          geometryType = QgsWkbTypes::LineStringZM;
          break;
        case QgsWkbTypes::MultiLineString:
          geometryType = QgsWkbTypes::MultiLineStringZM;
          break;
        case QgsWkbTypes::Polygon:
          geometryType = QgsWkbTypes::PolygonZM;
          break;
        case QgsWkbTypes::MultiPolygon:
          geometryType = QgsWkbTypes::MultiPolygonZM;
          break;
        case QgsWkbTypes::GeometryCollection:
          geometryType = QgsWkbTypes::GeometryCollectionZM;
          break;
      }
    }
    break;
  }
  return geometryType;
}
QgsWkbTypes::Type SpatialiteDbLayer::GetGeometryType( const int spatialiteGeometryType,   const int spatialiteGeometryDimension )
{
  Q_UNUSED( spatialiteGeometryDimension );
  QgsWkbTypes::Type geometryType = ( QgsWkbTypes::Type )spatialiteGeometryType;
  return geometryType;
}
//-----------------------------------------------------------
//  QgsSpatiaLiteUtils functions
//-----------------------------------------------------------
SpatialiteDbInfo *QgsSpatiaLiteUtils::GetSpatialiteDbInfoWrapper( QString sDatabaseFileName, QString sLayerName, bool bLoadLayers, sqlite3 *sqlite_handle )
{
  SpatialiteDbInfo *spatialiteDbInfo = nullptr;
  if ( ( sqlite_handle ) && ( QFile::exists( sDatabaseFileName ) ) )
  {
    spatialiteDbInfo = new SpatialiteDbInfo( sDatabaseFileName, sqlite_handle );
    if ( spatialiteDbInfo )
    {
      // retrieve all possible Layers if sLayerName is empty or Null
      if ( !spatialiteDbInfo->GetSpatialiteDbInfo( sLayerName, bLoadLayers ) )
      {
        return spatialiteDbInfo;
      }
    }
  }
  return spatialiteDbInfo;
}
//-----------------------------------------------------------
void QgsSpatiaLiteUtils::convertToGeosWKB( const unsigned char *blob,
    int blob_size,
    unsigned char **wkb,
    int *geom_size )
{
// attempting to convert to 2D/3D GEOS own WKB
  int type;
  int dims;
  int little_endian;
  int endian_arch = gaiaEndianArch();
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  int gsize = 6;
  const unsigned char *p_in;
  unsigned char *p_out;
  double coord;

  *wkb = nullptr;
  *geom_size = 0;
  if ( blob_size < 5 )
    return;
  if ( *( blob + 0 ) == 0x01 )
    little_endian = GAIA_LITTLE_ENDIAN;
  else
    little_endian = GAIA_BIG_ENDIAN;
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  if ( type == GAIA_POINTZ || type == GAIA_LINESTRINGZ
       || type == GAIA_POLYGONZ
       || type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
       || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ )
    dims = 3;
  else if ( type == GAIA_POINTM || type == GAIA_LINESTRINGM
            || type == GAIA_POLYGONM || type == GAIA_MULTIPOINTM
            || type == GAIA_MULTILINESTRINGM || type == GAIA_MULTIPOLYGONM
            || type == GAIA_GEOMETRYCOLLECTIONM )
    dims = 3;
  else if ( type == GAIA_POINTZM || type == GAIA_LINESTRINGZM
            || type == GAIA_POLYGONZM || type == GAIA_MULTIPOINTZM
            || type == GAIA_MULTILINESTRINGZM || type == GAIA_MULTIPOLYGONZM
            || type == GAIA_GEOMETRYCOLLECTIONZM )
    dims = 4;
  else if ( type == GAIA_POINT || type == GAIA_LINESTRING
            || type == GAIA_POLYGON || type == GAIA_MULTIPOINT
            || type == GAIA_MULTILINESTRING || type == GAIA_MULTIPOLYGON
            || type == GAIA_GEOMETRYCOLLECTION )
    dims = 2;
  else
    return;

  if ( dims == 2 )
  {
    // already 2D: simply copying is required
    unsigned char *wkbGeom = new unsigned char[blob_size + 1];
    memcpy( wkbGeom, blob, blob_size );
    memset( wkbGeom + blob_size, 0, 1 );
    *wkb = wkbGeom;
    *geom_size = blob_size + 1;
    return;
  }

  // we need creating a 3D GEOS WKB
  p_in = blob + 5;
  switch ( type )
  {
    // compunting the required size
    case GAIA_POINTZ:
    case GAIA_POINTM:
    case GAIA_POINTZM:
      gsize += 3 * sizeof( double );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      gsize += points * ( 3 * sizeof( double ) );
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        gsize += points * ( 3 * sizeof( double ) );
        p_in += points * ( 3 * sizeof( double ) );
      }
      break;
    case GAIA_POLYGONZM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        gsize += points * ( 3 * sizeof( double ) );
        p_in += points * ( 4 * sizeof( double ) );
      }
      break;
    default:
      gsize += computeMultiWKB3Dsize( p_in, little_endian, endian_arch );
      break;
  }

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

// building GEOS 3D WKB
  *wkbGeom = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
    // setting Geometry TYPE
    case GAIA_POINTZ:
    case GAIA_POINTM:
    case GAIA_POINTZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_POINT, 1, endian_arch );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
    case GAIA_LINESTRINGZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_LINESTRING, 1, endian_arch );
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONM:
    case GAIA_POLYGONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_POLYGON, 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTM:
    case GAIA_MULTIPOINTZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTIPOINT, 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGM:
    case GAIA_MULTILINESTRINGZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTILINESTRING, 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONM:
    case GAIA_MULTIPOLYGONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_MULTIPOLYGON, 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZ:
    case GAIA_GEOMETRYCOLLECTIONM:
    case GAIA_GEOMETRYCOLLECTIONZM:
      gaiaExport32( wkbGeom + 1, GEOS_3D_GEOMETRYCOLLECTION, 1, endian_arch );
      break;
  }
  p_in = blob + 5;
  p_out = wkbGeom + 5;
  switch ( type )
  {
    // setting Geometry values
    case GAIA_POINTM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
      p_in += sizeof( double );
      p_out += sizeof( double );
      break;
    case GAIA_POINTZ:
    case GAIA_POINTZM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Z
      p_in += sizeof( double );
      p_out += sizeof( double );
      if ( type == GAIA_POINTZM )
        p_in += sizeof( double );
      break;
    case GAIA_LINESTRINGM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( type == GAIA_LINESTRINGZM )
          p_in += sizeof( double );
      }
      break;
    case GAIA_POLYGONM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_POLYGONZ:
    case GAIA_POLYGONZM:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( type == GAIA_POLYGONZM )
            p_in += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOINTM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( type == GAIA_MULTIPOINTZM )
          p_in += sizeof( double );
      }
      break;
    case GAIA_MULTILINESTRINGM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( type == GAIA_MULTILINESTRINGZM )
            p_in += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOLYGONM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( type == GAIA_MULTIPOLYGONZM )
              p_in += sizeof( double );
          }
        }
      }
      break;
    case GAIA_GEOMETRYCOLLECTIONM:
    case GAIA_GEOMETRYCOLLECTIONZ:
    case GAIA_GEOMETRYCOLLECTIONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GAIA_POINTZ:
          case GAIA_POINTM:
          case GAIA_POINTZM:
            gaiaExport32( p_out, GEOS_3D_POINT, 1, endian_arch );
            break;
          case GAIA_LINESTRINGZ:
          case GAIA_LINESTRINGM:
          case GAIA_LINESTRINGZM:
            gaiaExport32( p_out, GEOS_3D_LINESTRING, 1, endian_arch );
            break;
          case GAIA_POLYGONZ:
          case GAIA_POLYGONM:
          case GAIA_POLYGONZM:
            gaiaExport32( p_out, GEOS_3D_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GAIA_POINTM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            break;
          case GAIA_POINTZ:
          case GAIA_POINTZM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( type2 == GAIA_POINTZM )
              p_in += sizeof( double );
            break;
          case GAIA_LINESTRINGM:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            break;
          case GAIA_LINESTRINGZ:
          case GAIA_LINESTRINGZM:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
              if ( type2 == GAIA_LINESTRINGZM )
                p_in += sizeof( double );
            }
            break;
          case GAIA_POLYGONM:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
            }
            break;
          case GAIA_POLYGONZ:
          case GAIA_POLYGONZM:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
                if ( type2 == GAIA_POLYGONZM )
                  p_in += sizeof( double );
              }
            }
            break;
        }
      }
      break;
  }
  *wkb = wkbGeom;
  *geom_size = gsize;
}
//-----------------------------------------------------------
int QgsSpatiaLiteUtils::computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian, int endian_arch )
{
// computing the required size to store a GEOS 3D MultiXX
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
      // compunting the required size
      case GAIA_POINTZ:
      case GAIA_POINTM:
        size += 3 * sizeof( double );
        p_in += 3 * sizeof( double );
        break;
      case GAIA_POINTZM:
        size += 3 * sizeof( double );
        p_in += 4 * sizeof( double );
        break;
      case GAIA_LINESTRINGZ:
      case GAIA_LINESTRINGM:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        size += points * ( 3 * sizeof( double ) );
        p_in += points * ( 3 * sizeof( double ) );
        break;
      case GAIA_LINESTRINGZM:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        size += points * ( 3 * sizeof( double ) );
        p_in += points * ( 4 * sizeof( double ) );
        break;
      case GAIA_POLYGONZ:
      case GAIA_POLYGONM:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          size += points * ( 3 * sizeof( double ) );
          p_in += points * ( 3 * sizeof( double ) );
        }
        break;
      case GAIA_POLYGONZM:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          size += points * ( 3 * sizeof( double ) );
          p_in += points * ( 4 * sizeof( double ) );
        }
        break;
    }
  }
  return size;
}
//-----------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromMultiWKB2D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
      // compunting the required size
      case GAIA_POINT:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            break;
        }
        p_in += 2 * sizeof( double );
        break;
      case GAIA_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 2 * sizeof( double ) );
        break;
      case GAIA_POLYGON:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          switch ( nDims )
          {
            case GAIA_XY_Z_M:
              size += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              break;
          }
          p_in += points * ( 2 * sizeof( double ) );
        }
        break;
    }
  }
  return size;
}
//-----------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromMultiWKB3D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  int type;
  int rings;
  int points;
  int ie;
  int ib;
  int size = 0;

  entities = gaiaImport32( p_in, little_endian, endian_arch );
  p_in += 4;
  size += 4;
  for ( ie = 0; ie < entities; ie++ )
  {
    type = gaiaImport32( p_in + 1, little_endian, endian_arch );
    p_in += 5;
    size += 5;
    switch ( type )
    {
      // compunting the required size
      case GEOS_3D_POINT:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            break;
        }
        p_in += 3 * sizeof( double );
        break;
      case GEOS_3D_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 3 * sizeof( double ) );
        break;
      case GEOS_3D_POLYGON:
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          size += 4;
          switch ( nDims )
          {
            case GAIA_XY_Z_M:
              size += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              break;
          }
          p_in += points * ( 3 * sizeof( double ) );
        }
        break;
    }
  }
  return size;
}
//-----------------------------------------------------------
void QgsSpatiaLiteUtils::convertFromGeosWKB2D( const unsigned char *blob,
    int blob_size,
    unsigned char *wkb,
    int geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 2D GEOS own WKB
  int type;
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  const unsigned char *p_in;
  unsigned char *p_out = wkb;
  double coord;

// building from GEOS 2D WKB
  *p_out++ = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
    // setting Geometry TYPE
    case GAIA_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
          break;
      }
      break;
    case GAIA_LINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GAIA_POLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTIPOINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOINT, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTILINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTILINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GAIA_MULTIPOLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOLYGON, 1, endian_arch );
          break;
      }
      break;
    case GAIA_GEOMETRYCOLLECTION:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTION, 1, endian_arch );
          break;
      }
      break;
  }
  p_in = blob + 5;
  p_out += 4;
  switch ( type )
  {
    // setting Geometry values
    case GAIA_POINT:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
        p_out += sizeof( double );
      }
      break;
    case GAIA_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOINT:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
            break;
        }
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRING:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
            break;
        }
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGON:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
          }
        }
      }
      break;
    case GAIA_GEOMETRYCOLLECTION:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GAIA_POINT:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
                break;
            }
            break;
          case GAIA_LINESTRING:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
                break;
            }
            break;
          case GAIA_POLYGON:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
                break;
            }
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GAIA_POINT:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
            break;
          case GAIA_LINESTRING:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                p_out += sizeof( double );
              }
            }
            break;
          case GAIA_POLYGON:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // Z
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                  p_out += sizeof( double );
                }
              }
            }
            break;
        }
      }
      break;
  }
}
//-----------------------------------------------------------
void QgsSpatiaLiteUtils::convertFromGeosWKB3D( const unsigned char *blob,
    int blob_size,
    unsigned char *wkb,
    int geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 3D GEOS own WKB
  int type;
  int entities;
  int rings;
  int points;
  int ie;
  int ib;
  int iv;
  const unsigned char *p_in;
  unsigned char *p_out = wkb;
  double coord;

// building from GEOS 3D WKB
  *p_out++ = 0x01;  // little endian byte order
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  switch ( type )
  {
    // setting Geometry TYPE
    case GEOS_3D_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_LINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_POLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTIPOINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOINTZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOINTM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOINT, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTILINESTRING:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTILINESTRINGM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTILINESTRING, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_MULTIPOLYGON:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_MULTIPOLYGONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_MULTIPOLYGON, 1, endian_arch );
          break;
      }
      break;
    case GEOS_3D_GEOMETRYCOLLECTION:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZM, 1, endian_arch );
          break;
        case GAIA_XY_Z:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONZ, 1, endian_arch );
          break;
        case GAIA_XY_M:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTIONM, 1, endian_arch );
          break;
        default:
          gaiaExport32( p_out, GAIA_GEOMETRYCOLLECTION, 1, endian_arch );
          break;
      }
      break;
  }
  p_in = blob + 5;
  p_out += 4;
  switch ( type )
  {
    // setting Geometry values
    case GEOS_3D_POINT:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      p_in += sizeof( double );
      if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
        p_out += sizeof( double );
      }
      break;
    case GEOS_3D_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, points, 1, endian_arch );
      p_out += 4;
      for ( iv = 0; iv < points; iv++ )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        p_in += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GEOS_3D_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, rings, 1, endian_arch );
      p_out += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          p_in += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GEOS_3D_MULTIPOINT:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
            break;
        }
        p_out += 4;
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // X
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Y
        p_in += sizeof( double );
        p_out += sizeof( double );
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        p_in += sizeof( double );
        if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
          p_out += sizeof( double );
        }
      }
      break;
    case GEOS_3D_MULTILINESTRING:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
            break;
        }
        p_out += 4;
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, points, 1, endian_arch );
        p_out += 4;
        for ( iv = 0; iv < points; iv++ )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // X
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Y
          p_in += sizeof( double );
          p_out += sizeof( double );
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          p_in += sizeof( double );
          if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GEOS_3D_MULTIPOLYGON:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
            break;
          case GAIA_XY_Z:
            gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
            break;
          case GAIA_XY_M:
            gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
            break;
          default:
            gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
            break;
        }
        p_out += 4;
        rings = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gaiaExport32( p_out, rings, 1, endian_arch );
        p_out += 4;
        for ( ib = 0; ib < rings; ib++ )
        {
          points = gaiaImport32( p_in, little_endian, endian_arch );
          p_in += 4;
          gaiaExport32( p_out, points, 1, endian_arch );
          p_out += 4;
          for ( iv = 0; iv < points; iv++ )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            p_in += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
          }
        }
      }
      break;
    case GEOS_3D_GEOMETRYCOLLECTION:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        int type2 = gaiaImport32( p_in + 1, little_endian, endian_arch );
        p_in += 5;
        *p_out++ = 0x01;
        switch ( type2 )
        {
          case GEOS_3D_POINT:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POINTZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POINTZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POINTM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POINT, 1, endian_arch );
                break;
            }
            break;
          case GEOS_3D_LINESTRING:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_LINESTRINGZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_LINESTRINGZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_LINESTRINGM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_LINESTRING, 1, endian_arch );
                break;
            }
            break;
          case GEOS_3D_POLYGON:
            switch ( nDims )
            {
              case GAIA_XY_Z_M:
                gaiaExport32( p_out, GAIA_POLYGONZM, 1, endian_arch );
                break;
              case GAIA_XY_Z:
                gaiaExport32( p_out, GAIA_POLYGONZ, 1, endian_arch );
                break;
              case GAIA_XY_M:
                gaiaExport32( p_out, GAIA_POLYGONM, 1, endian_arch );
                break;
              default:
                gaiaExport32( p_out, GAIA_POLYGON, 1, endian_arch );
                break;
            }
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GEOS_3D_POINT:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            p_in += sizeof( double );
            if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
              p_out += sizeof( double );
            }
            break;
          case GEOS_3D_LINESTRING:
            points = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, points, 1, endian_arch );
            p_out += 4;
            for ( iv = 0; iv < points; iv++ )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // X
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Y
              p_in += sizeof( double );
              p_out += sizeof( double );
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              p_in += sizeof( double );
              if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                p_out += sizeof( double );
              }
            }
            break;
          case GEOS_3D_POLYGON:
            rings = gaiaImport32( p_in, little_endian, endian_arch );
            p_in += 4;
            gaiaExport32( p_out, rings, 1, endian_arch );
            p_out += 4;
            for ( ib = 0; ib < rings; ib++ )
            {
              points = gaiaImport32( p_in, little_endian, endian_arch );
              p_in += 4;
              gaiaExport32( p_out, points, 1, endian_arch );
              p_out += 4;
              for ( iv = 0; iv < points; iv++ )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // X
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Y
                p_in += sizeof( double );
                p_out += sizeof( double );
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                p_in += sizeof( double );
                if ( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  gaiaExport64( p_out, 0.0, 1, endian_arch );  // M
                  p_out += sizeof( double );
                }
              }
            }
            break;
        }
      }
      break;
  }
}
//-----------------------------------------------------------
void QgsSpatiaLiteUtils::convertFromGeosWKB( const unsigned char *blob,
    int blob_size,
    unsigned char **wkb,
    int *geom_size,
    int nDims )
{
// attempting to convert from 2D/3D GEOS own WKB
  int type;
  int gDims;
  int gsize;
  int little_endian;
  int endian_arch = gaiaEndianArch();

  *wkb = nullptr;
  *geom_size = 0;
  if ( blob_size < 5 )
    return;
  if ( *( blob + 0 ) == 0x01 )
    little_endian = GAIA_LITTLE_ENDIAN;
  else
    little_endian = GAIA_BIG_ENDIAN;
  type = gaiaImport32( blob + 1, little_endian, endian_arch );
  if ( type == GEOS_3D_POINT || type == GEOS_3D_LINESTRING
       || type == GEOS_3D_POLYGON
       || type == GEOS_3D_MULTIPOINT || type == GEOS_3D_MULTILINESTRING
       || type == GEOS_3D_MULTIPOLYGON || type == GEOS_3D_GEOMETRYCOLLECTION )
    gDims = 3;
  else if ( type == GAIA_POINT || type == GAIA_LINESTRING
            || type == GAIA_POLYGON || type == GAIA_MULTIPOINT
            || type == GAIA_MULTILINESTRING || type == GAIA_MULTIPOLYGON
            || type == GAIA_GEOMETRYCOLLECTION )
    gDims = 2;
  else
    return;

  if ( gDims == 2 && nDims == GAIA_XY )
  {
    // already 2D: simply copying is required
    unsigned char *wkbGeom = new unsigned char[blob_size + 1];
    memcpy( wkbGeom, blob, blob_size );
    memset( wkbGeom + blob_size, 0, 1 );
    *wkb = wkbGeom;
    *geom_size = blob_size + 1;
    return;
  }

// we need creating a GAIA WKB
  if ( gDims == 3 )
    gsize = QgsSpatiaLiteUtils::computeSizeFromGeosWKB3D( blob, blob_size, type, nDims,
            little_endian, endian_arch );
  else
    gsize = QgsSpatiaLiteUtils::computeSizeFromGeosWKB2D( blob, blob_size, type, nDims,
            little_endian, endian_arch );

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

  if ( gDims == 3 )
    QgsSpatiaLiteUtils::convertFromGeosWKB3D( blob, blob_size, wkbGeom, gsize, nDims,
        little_endian, endian_arch );
  else
    QgsSpatiaLiteUtils::convertFromGeosWKB2D( blob, blob_size, wkbGeom, gsize, nDims,
        little_endian, endian_arch );

  *wkb = wkbGeom;
  *geom_size = gsize;
}
//-----------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromGeosWKB3D( const unsigned char *blob,
    int size, int type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob + 5;
  int gsize = 5;

  switch ( type )
  {
    // compunting the required size
    case GEOS_3D_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += 4 * sizeof( double );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += 3 * sizeof( double );
          break;
        default:
          gsize += 2 * sizeof( double );
          break;
      }
      break;
    case GEOS_3D_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += points * ( 4 * sizeof( double ) );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += points * ( 3 * sizeof( double ) );
          break;
        default:
          gsize += points * ( 2 * sizeof( double ) );
          break;
      }
      break;
    case GEOS_3D_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gsize += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_M:
          case GAIA_XY_Z:
            gsize += points * ( 3 * sizeof( double ) );
            break;
          default:
            gsize += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 3 * sizeof( double ) );
      }
      break;
    default:
      gsize += QgsSpatiaLiteUtils::computeSizeFromMultiWKB3D( p_in, nDims, little_endian,
               endian_arch );
      break;
  }
  return gsize;
}
//-----------------------------------------------------------
int QgsSpatiaLiteUtils::computeSizeFromGeosWKB2D( const unsigned char *blob,
    int size, int type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob + 5;
  int gsize = 5;

  switch ( type )
  {
    // compunting the required size
    case GAIA_POINT:
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += 4 * sizeof( double );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += 3 * sizeof( double );
          break;
        default:
          gsize += 2 * sizeof( double );
          break;
      }
      break;
    case GAIA_LINESTRING:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      switch ( nDims )
      {
        case GAIA_XY_Z_M:
          gsize += points * ( 4 * sizeof( double ) );
          break;
        case GAIA_XY_M:
        case GAIA_XY_Z:
          gsize += points * ( 3 * sizeof( double ) );
          break;
        default:
          gsize += points * ( 2 * sizeof( double ) );
          break;
      }
      break;
    case GAIA_POLYGON:
      rings = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gsize += 4;
      for ( ib = 0; ib < rings; ib++ )
      {
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        gsize += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            gsize += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_M:
          case GAIA_XY_Z:
            gsize += points * ( 3 * sizeof( double ) );
            break;
          default:
            gsize += points * ( 2 * sizeof( double ) );
            break;
        }
        p_in += points * ( 2 * sizeof( double ) );
      }
      break;
    default:
      gsize += QgsSpatiaLiteUtils::computeSizeFromMultiWKB2D( p_in, nDims, little_endian,
               endian_arch );
      break;
  }
  return gsize;
}
//-----------------------------------------------------------
