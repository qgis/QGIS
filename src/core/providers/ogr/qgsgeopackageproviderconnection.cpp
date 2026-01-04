/***************************************************************************
  QgsGeoPackageProviderConnection.cpp - QgsGeoPackageProviderConnection

 ---------------------
 begin                : 6.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeopackageproviderconnection.h"

#include <chrono>
#include <sqlite3.h>

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QRegularExpression>
#include <QTextCodec>

///@cond PRIVATE

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &name )
  : QgsOgrProviderConnection( name )
{
  QgsSettings settings;
  settings.beginGroup( u"ogr"_s, QgsSettings::Section::Providers );
  settings.beginGroup( u"GPKG"_s );
  settings.beginGroup( u"connections"_s );
  settings.beginGroup( name );
  setUri( settings.value( u"path"_s ).toString() );
  setDefaultCapabilities();
}

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsOgrProviderConnection( uri, configuration )
{
  setDefaultCapabilities();
}

void QgsGeoPackageProviderConnection::store( const QString &name ) const
{
  QgsSettings settings;
  settings.beginGroup( u"ogr"_s, QgsSettings::Section::Providers );
  settings.beginGroup( u"GPKG"_s );
  settings.beginGroup( u"connections"_s );
  settings.beginGroup( name );
  settings.setValue( u"path"_s, uri() );
}

void QgsGeoPackageProviderConnection::remove( const QString &name ) const
{
  QgsSettings settings;
  settings.beginGroup( u"ogr"_s, QgsSettings::Section::Providers );
  settings.beginGroup( u"GPKG"_s );
  settings.beginGroup( u"connections"_s );
  settings.remove( name );
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsGeoPackageProviderConnection::table( const QString &schema, const QString &name, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tables( schema, TableFlags(), feedback ) };
  for ( const auto &t : constTables )
  {
    if ( t.tableName() == name )
    {
      return t;
    }
  }
  throw QgsProviderConnectionException( QObject::tr( "Table '%1' was not found in schema '%2'" )
                                        .arg( name, schema ) );
}

QString QgsGeoPackageProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  if ( tableInfo.flags().testFlag( QgsAbstractDatabaseProviderConnection::TableFlag::Raster ) )
  {
    return u"GPKG:%1:%2"_s.arg( uri(), name );
  }
  else
  {
    return QgsOgrProviderConnection::tableUri( schema, name );
  }
}

void QgsGeoPackageProviderConnection::dropRasterTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropRasterTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  executeGdalSqlPrivate( u"DROP TABLE %1"_s.arg( name ) );
}

void QgsGeoPackageProviderConnection::renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const
{
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  QString sql( u"ALTER TABLE %1 RENAME TO %2"_s
               .arg( QgsSqliteUtils::quotedIdentifier( name ),
                     QgsSqliteUtils::quotedIdentifier( newName ) ) );
  executeGdalSqlPrivate( sql );
  // This is also done by GDAL (at least by current version)
  sql = u"UPDATE layer_styles SET f_table_name = %2 WHERE f_table_name = %1"_s
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  try
  {
    executeGdalSqlPrivate( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QgsDebugMsgLevel( u"Warning: error while updating the styles, perhaps there are no styles stored in this GPKG: %1"_s.arg( ex.what() ), 4 );
  }
}

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,10,0)
void QgsGeoPackageProviderConnection::renameRasterTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameRasterTable );
  renameTablePrivate( schema, name, newName );
}
#endif

void QgsGeoPackageProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  renameTablePrivate( schema, name, newName );
}

void QgsGeoPackageProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  Q_UNUSED( name );
  checkCapability( Capability::Vacuum );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  executeGdalSqlPrivate( u"VACUUM"_s );
}

void QgsGeoPackageProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QString geometryColumnName { options.geometryColumnName };
  if ( geometryColumnName.isEmpty() )
  {
    // Can we guess it?
    try
    {
      const auto tp { table( schema, name ) };
      geometryColumnName = tp.geometryColumn();
    }
    catch ( QgsProviderConnectionException & )
    {
      // pass
    }
  }

  if ( geometryColumnName.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Geometry column name not specified while creating spatial index" ) );
  }

  executeGdalSqlPrivate( u"SELECT CreateSpatialIndex(%1, %2)"_s.arg( QgsSqliteUtils::quotedString( name ),
                         QgsSqliteUtils::quotedString( ( geometryColumnName ) ) ) );
}

bool QgsGeoPackageProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::CreateSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  const QList<QList<QVariant> > res = executeGdalSqlPrivate( u"SELECT HasSpatialIndex(%1, %2)"_s.arg( QgsSqliteUtils::quotedString( name ),
                                      QgsSqliteUtils::quotedString( geometryColumn ) ) ).rows();
  return !res.isEmpty() && !res.at( 0 ).isEmpty() && res.at( 0 ).at( 0 ).toBool();
}

void QgsGeoPackageProviderConnection::deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::DeleteSpatialIndex );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  executeGdalSqlPrivate( u"SELECT DisableSpatialIndex(%1, %2)"_s.arg( QgsSqliteUtils::quotedString( name ),
                         QgsSqliteUtils::quotedString( geometryColumn ) ) );
}

QList<QgsGeoPackageProviderConnection::TableProperty> QgsGeoPackageProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback *feedback ) const
{

  // List of GPKG quoted system and dummy tables names to be excluded from the tables listing
  static const QStringList excludedTableNames { { u"'ogr_empty_table'"_s } };

  checkCapability( Capability::Tables );

  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by GPKG, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QList<QgsGeoPackageProviderConnection::TableProperty> tableInfo;
  QString errCause;
  QList<QVariantList> results;

  try
  {
    const QString sql = QStringLiteral( "SELECT c.table_name, data_type, description, c.srs_id, g.geometry_type_name, g.column_name "
                                        "FROM gpkg_contents c LEFT JOIN gpkg_geometry_columns g ON (c.table_name = g.table_name) "
                                        "WHERE c.table_name NOT IN (%1)" ).arg( excludedTableNames.join( ',' ) );
    results = executeSql( sql );

    for ( const auto &row : std::as_const( results ) )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      if ( row.size() != 6 )
      {
        throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: wrong number of columns returned by query" ).arg( uri() ) );
      }

      QgsGeoPackageProviderConnection::TableProperty property;
      const QString tableName { row.at( 0 ).toString() };
      property.setTableName( tableName );
      property.setPrimaryKeyColumns( { primaryKeyColumnName( tableName ) } );
      property.setGeometryColumnCount( 0 );
      static const QStringList aspatialTypes = { u"attributes"_s, u"aspatial"_s };
      const QString dataType = row.at( 1 ).toString();

      // Table type
      if ( dataType == "tiles"_L1 || dataType == "2d-gridded-coverage"_L1 )
      {
        property.setFlag( QgsGeoPackageProviderConnection::TableFlag::Raster );
      }
      else if ( dataType == "features"_L1 )
      {
        property.setFlag( QgsGeoPackageProviderConnection::TableFlag::Vector );
        property.setGeometryColumn( row.at( 5 ).toString() );
        property.setGeometryColumnCount( 1 );
      }

      if ( aspatialTypes.contains( dataType ) )
      {
        property.setFlag( QgsGeoPackageProviderConnection::TableFlag::Aspatial );
        property.addGeometryColumnType( Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() );
      }
      else
      {
        bool ok;
        int srid = row.at( 3 ).toInt( &ok );

        if ( !ok )
        {
          throw QgsProviderConnectionException( QObject::tr( "Error fetching srs_id table information: %1" ).arg( row.at( 3 ).toString() ) );
        }

        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromEpsgId( srid );
        property.addGeometryColumnType( QgsWkbTypes::parseType( row.at( 4 ).toString() ),  crs );
      }

      property.setComment( row.at( 2 ).toString() );
      tableInfo.push_back( property );
    }

  }
  catch ( QgsProviderConnectionException &ex )
  {
    errCause = ex.what();
  }

  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( uri(), errCause ) );
  }
  // Filters
  if ( flags )
  {
    tableInfo.erase( std::remove_if( tableInfo.begin(), tableInfo.end(), [ & ]( const QgsAbstractDatabaseProviderConnection::TableProperty & ti )
    {
      return !( ti.flags() & flags );
    } ), tableInfo.end() );
  }
  return tableInfo ;
}

QIcon QgsGeoPackageProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( u"mGeoPackage.svg"_s );
}

void QgsGeoPackageProviderConnection::setDefaultCapabilities()
{
  mCapabilities =
  {
    Capability::Tables,
    Capability::CreateVectorTable,
    Capability::DropVectorTable,
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,10,0)
    Capability::RenameRasterTable,
#endif
    Capability::RenameVectorTable,
    Capability::Vacuum,
    Capability::Spatial,
    Capability::TableExists,
    Capability::ExecuteSql,
    Capability::CreateSpatialIndex,
    Capability::SpatialIndexExists,
    Capability::DeleteSpatialIndex,
    Capability::DeleteField,
    Capability::AddField,
    Capability::RenameField,
    Capability::DropRasterTable,
    Capability::SqlLayers
  };


#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  mCapabilities |= Capability::RetrieveFieldDomain;
  mCapabilities |= Capability::AddFieldDomain;
  mCapabilities |= Capability::SetFieldDomain;
#endif
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
  mCapabilities |= Capability::ListFieldDomains;
#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  mCapabilities |= Capability::RetrieveRelationships;
#endif

  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SingleLineString,
    GeometryColumnCapability::SinglePoint,
    GeometryColumnCapability::SinglePolygon,
    GeometryColumnCapability::Curves,
    GeometryColumnCapability::PolyhedralSurfaces
  };
  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
  };

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  mCapabilities |= Capability::AddRelationship;
  mCapabilities |= Capability::UpdateRelationship;
  mCapabilities |= Capability::DeleteRelationship;
#endif
}

QString QgsGeoPackageProviderConnection::primaryKeyColumnName( const QString &table ) const
{
  QString pkName;

  sqlite3_database_unique_ptr sqliteHandle;
  if ( SQLITE_OK == sqliteHandle.open_v2( uri(), SQLITE_OPEN_READONLY, nullptr ) )
  {
    char *errMsg;

    const QString sql { u"PRAGMA table_info(%1)"_s
                        .arg( QgsSqliteUtils::quotedString( table ) )};

    std::vector<std::string> rows;
    auto cb = [ ](
                void *data /* Data provided in the 4th argument of sqlite3_exec() */,
                int /* The number of columns in row */,
                char **argv /* An array of strings representing fields in the row */,
                char ** /* An array of strings representing column names */ ) -> int
    {
      if ( std::string( argv[5] ).compare( "1" ) == 0 )
        static_cast<std::vector<std::string>*>( data )->push_back( argv[1] );
      return 0;
    };

    // Columns 'cid', 'name', 'type', 'notnull', 'dflt_value', 'pk']
    const int ret = sqlite3_exec( sqliteHandle.get(), sql.toUtf8(), cb, ( void * )&rows, &errMsg );

    if ( errMsg )
    {
      sqlite3_free( errMsg );
    }

    if ( SQLITE_OK == ret && rows.size() > 0 )
    {
      pkName = QString::fromStdString( rows[0] );
    }
  }

  return pkName;
}

QList<QgsLayerMetadataProviderResult> QgsGeoPackageProviderConnection::searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{

  QList<QgsLayerMetadataProviderResult> results;
  if ( ! feedback || ! feedback->isCanceled() )
  {
    try
    {
      // first check if metadata tables/extension exists
      if ( executeSql( u"SELECT name FROM sqlite_master WHERE name='gpkg_metadata' AND type='table'"_s, nullptr ).isEmpty() )
      {
        return results;
      }

      const QString searchQuery { QStringLiteral( R"SQL(
      SELECT
        ref.table_name, md.metadata, gc.geometry_type_name
      FROM
        gpkg_metadata_reference AS ref
      JOIN
        gpkg_metadata AS md ON md.id = ref.md_file_id
      LEFT JOIN
        gpkg_geometry_columns AS gc ON gc.table_name = ref.table_name
      WHERE
        md.md_standard_uri = 'http://mrcc.com/qgis.dtd'
        AND ref.reference_scope = 'table'
        AND md.md_scope = 'dataset'
      )SQL" ) };

      const QList<QVariantList> constMetadataResults { executeSql( searchQuery, feedback ) };
      for ( const QVariantList &mdRow : std::as_const( constMetadataResults ) )
      {

        if ( feedback && feedback->isCanceled() )
        {
          break;
        }

        // Read MD from the XML
        QDomDocument doc;
        doc.setContent( mdRow[1].toString() );
        QgsLayerMetadata layerMetadata;
        if ( layerMetadata.readMetadataXml( doc.documentElement() ) )
        {
          QgsLayerMetadataProviderResult result{ layerMetadata };

          QgsRectangle extents;
          bool extentsValid = false;

          const auto cExtents { layerMetadata.extent().spatialExtents() };
          for ( const auto &ext : std::as_const( cExtents ) )
          {
            QgsRectangle bbox {  ext.bounds.toRectangle()  };
            QgsCoordinateTransform ct { ext.extentCrs, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), searchContext.transformContext };
            try
            {
              ct.transform( bbox );
            }
            catch ( const QgsCsException & )
            {
              QgsDebugError( u"Layer metadata extent failed to reproject to EPSG:4326"_s );
              continue;
            }
            extentsValid = true;
            extents.combineExtentWith( bbox );
          }

          QgsPolygon poly;
          if ( extentsValid )
          {
            poly.fromWkt( extents.asWktPolygon() );
          }

          // Filters
          if ( ! geographicExtent.isEmpty() && ( poly.isEmpty() || ! geographicExtent.intersects( extents ) ) )
          {
            continue;
          }

          if ( ! searchString.trimmed().isEmpty() && ! result.contains( searchString ) )
          {
            continue;
          }

          if ( extentsValid )
          {
            result.setGeographicExtent( poly );
          }
          result.setStandardUri( u"http://mrcc.com/qgis.dtd"_s );
          result.setDataProviderName( u"ogr"_s );
          result.setAuthid( layerMetadata.crs().authid() );
          result.setUri( tableUri( QString(), mdRow[0].toString() ) );
          const QString geomType { mdRow[2].toString().toUpper() };
          if ( geomType.contains( u"POINT"_s, Qt::CaseSensitivity::CaseInsensitive ) )
          {
            result.setGeometryType( Qgis::GeometryType::Point );
          }
          else if ( geomType.contains( u"POLYGON"_s, Qt::CaseSensitivity::CaseInsensitive ) )
          {
            result.setGeometryType( Qgis::GeometryType::Polygon );
          }
          else if ( geomType.contains( u"LINESTRING"_s, Qt::CaseSensitivity::CaseInsensitive ) )
          {
            result.setGeometryType( Qgis::GeometryType::Line );
          }
          else
          {
            result.setGeometryType( Qgis::GeometryType::Unknown );
          }
          result.setLayerType( Qgis::LayerType::Vector );

          results.push_back( result );
        }
        else
        {
          throw QgsProviderConnectionException( u"Error reading XML metdadata from connection %1"_s.arg( uri() ) );
        }
      }
    }
    catch ( const QgsProviderConnectionException &ex )
    {
      throw QgsProviderConnectionException( u"Error fetching metdadata from connection %1: %2"_s.arg( uri(), ex.what() ) );
    }
  }
  return results;
}

Qgis::DatabaseProviderTableImportCapabilities QgsGeoPackageProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapabilities();
}

QgsFields QgsGeoPackageProviderConnection::fields( const QString &schema, const QString &table, QgsFeedback * ) const
{
  Q_UNUSED( schema )

  // Get fields from layer
  QgsFields fieldList;

  const QString pkname { primaryKeyColumnName( table ) };

  if ( ! pkname.isEmpty() )
  {
    fieldList.append( QgsField{ pkname, QMetaType::Type::LongLong } );
  }

  QgsVectorLayer::LayerOptions options { false, true };
  options.skipCrsValidation = true;
  QgsVectorLayer vl { tableUri( schema, table ), u"temp_layer"_s, mProviderKey, options };
  if ( vl.isValid() )
  {
    const auto parentFields { vl.fields() };
    for ( const auto &pField : std::as_const( parentFields ) )
    {
      fieldList.append( pField );
    }
    // Append name of the geometry column, the data provider does not expose this information so we need an extra query:/
    const QString sql = QStringLiteral( "SELECT g.column_name "
                                        "FROM gpkg_contents c CROSS JOIN gpkg_geometry_columns g ON (c.table_name = g.table_name) "
                                        "WHERE c.table_name = %1" ).arg( QgsSqliteUtils::quotedString( table ) );
    try
    {
      const auto results = executeSql( sql );
      if ( ! results.isEmpty() )
      {
        fieldList.append( QgsField{ results.first().first().toString(), QMetaType::Type::QString, u"geometry"_s } );
      }
    }
    catch ( QgsProviderConnectionException &ex )
    {
      throw QgsProviderConnectionException( QObject::tr( "Error retrieving fields information for uri %1: %2" ).arg( vl.publicSource(), ex.what() ) );
    }

  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving fields information for uri: %1" ).arg( vl.publicSource() ) );
  }

  return fieldList;
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsGeoPackageProviderConnection::sqlDictionary()
{
  /*
   * last_insert_rowid + list from: http://www.gaia-gis.it/gaia-sins/spatialite-sql-4.2.0.html
   *
   import requests, re
   result = requests.get('http://www.gaia-gis.it/gaia-sins/spatialite-sql-4.2.0.html')
   functions = {}
   for r in result.content.decode('utf8').split('\n'):
       if 'name="' in r:
           section = (re.findall(r'.*name="[^"]+">([^<]+)<.*', r)[0])
           functions[section] = []
           m = re.match(r'\t\t\t\t<td>(.*?) :.*', r)
           if m:
               functions[section].append(m.group(1))
   for title, f in functions.items():
       print(f"// {title}")
       for _f in f:
           print( f"QStringLiteral( \"{_f}\" ),")
   */

  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
  {
    {
      Qgis::SqlKeywordCategory::Math, {
        // SQL math functions
        u"Abs( x [Double precision] )"_s,
        u"Acos( x [Double precision] )"_s,
        u"Asin( x [Double precision] )"_s,
        u"Atan( x [Double precision] )"_s,
        u"Ceil( x [Double precision] )"_s,
        u"Cos( x [Double precision] )"_s,
        u"Cot( x [Double precision] )"_s,
        u"Degrees( x [Double precision] )"_s,
        u"Exp( x [Double precision] )"_s,
        u"Floor( x [Double precision] )"_s,
        u"Ln( x [Double precision] )"_s,
        u"Log( b [Double precision] , x [Double precision] )"_s,
        u"Log2( x [Double precision] )"_s,
        u"Log10( x [Double precision] )"_s,
        u"PI( void )"_s,
        u"Pow( x [Double precision] , y [Double precision] )"_s,
        u"Radians( x [Double precision] )"_s,
        u"Sign( x [Double precision] )"_s,
        u"Sin( x [Double precision] )"_s,
        u"Sqrt( x [Double precision] )"_s,
        u"Stddev_pop( x [Double precision] )"_s,
        u"Stddev_samp( x [Double precision] )"_s,
        u"Tan( x [Double precision] )"_s,
        u"Var_pop( x [Double precision] )"_s,
        u"Var_samp( x [Double precision] )"_s
      }
    },
    {
      Qgis::SqlKeywordCategory::Function, {

        // Specific
        u"last_insert_rowid"_s,

        // SQL Version Info [and build options testing] functions
        u"spatialite_version( void )"_s,
        u"spatialite_target_cpu( void )"_s,
        u"proj4_version( void )"_s,
        u"geos_version( void )"_s,
        u"lwgeom_version( void )"_s,
        u"libxml2_version( void )"_s,
        u"HasIconv( void )"_s,
        u"HasMathSQL( void )"_s,
        u"HasGeoCallbacks( void )"_s,
        u"HasProj( void )"_s,
        u"HasGeos( void )"_s,
        u"HasGeosAdvanced( void )"_s,
        u"HasGeosTrunk( void )"_s,
        u"HasLwGeom( void )"_s,
        u"HasLibXML2( void )"_s,
        u"HasEpsg( void )"_s,
        u"HasFreeXL( void )"_s,
        u"HasGeoPackage( void )"_s,

        // Generic SQL functions
        u"CastToInteger( value [Generic] )"_s,
        u"CastToDouble( value [Generic] )"_s,
        u"CastToText( value [Generic] )"_s,
        u"CastToBlob( value [Generic] )"_s,
        u"ForceAsNull( val1 [Generic] , val2 [Generic])"_s,
        u"CreateUUID( void )"_s,
        u"MD5Checksum( BLOB | TEXT )"_s,
        u"MD5TotalChecksum( BLOB | TEXT )"_s,

        // SQL utility functions for BLOB objects
        u"IsZipBlob( content [BLOB] )"_s,
        u"IsPdfBlob( content [BLOB] )"_s,
        u"IsGifBlob( image [BLOB] )"_s,
        u"IsPngBlob( image [BLOB] )"_s,
        u"IsTiffBlob( image [BLOB] )"_s,
        u"IsJpegBlob( image [BLOB] )"_s,
        u"IsExifBlob( image [BLOB] )"_s,
        u"IsExifGpsBlob( image [BLOB] )"_s,
        u"IsWebpBlob( image [BLOB] )"_s,
        u"GetMimeType( payload [BLOB] )"_s,
        u"BlobFromFile( filepath [String] )"_s,
        u"BlobToFile( payload [BLOB] , filepath [String] )"_s,
        u"CountUnsafeTriggers( )"_s,

        // SQL functions supporting XmlBLOB
        u"XB_Create(  xmlPayload [BLOB] )"_s,
        u"XB_GetPayload( xmlObject [XmlBLOB] [ , indent [Integer] ] )"_s,
        u"XB_GetDocument( xmlObject [XmlBLOB] [ , indent [Integer] ] )"_s,
        u"XB_SchemaValidate(  xmlObject [XmlBLOB] , schemaURI [Text] [ , compressed [Boolean] ] )"_s,
        u"XB_Compress( xmlObject [XmlBLOB] )"_s,
        u"XB_Uncompress( xmlObject [XmlBLOB] )"_s,
        u"XB_IsValid( xmlObject [XmlBLOB] )"_s,
        u"XB_IsCompressed( xmlObject [XmlBLOB] )"_s,
        u"XB_IsSchemaValidated( xmlObject [XmlBLOB] )"_s,
        u"XB_IsIsoMetadata( xmlObject [XmlBLOB] )"_s,
        u"XB_IsSldSeVectorStyle( xmlObject [XmlBLOB] )"_s,
        u"XB_IsSldSeRasterStyle( xmlObject [XmlBLOB] )"_s,
        u"XB_IsSvg( xmlObject [XmlBLOB] )"_s,
        u"XB_GetDocumentSize( xmlObject [XmlBLOB] )"_s,
        u"XB_GetEncoding( xmlObject [XmlBLOB] )"_s,
        u"XB_GetSchemaURI( xmlObject [XmlBLOB] )"_s,
        u"XB_GetInternalSchemaURI( xmlPayload [BLOB] )"_s,
        u"XB_GetFileId( xmlObject [XmlBLOB] )"_s,
        u"XB_SetFileId( xmlObject [XmlBLOB] , fileId [String] )"_s,
        u"XB_AddFileId( xmlObject [XmlBLOB] , fileId [String] , IdNameSpacePrefix [String] , IdNameSpaceURI [String] , CsNameSpacePrefix [String] , CsNameSpaceURI [String] )"_s,
        u"XB_GetParentId( xmlObject [XmlBLOB] )"_s,
        u"XB_SetParentId( xmlObject [XmlBLOB] , parentId [String] )"_s,
        u"XB_AddParentId( xmlObject [XmlBLOB] , parentId [String] , IdNameSpacePrefix [String] , IdNameSpaceURI [String] , CsNameSpacePrefix [String] , CsNameSpaceURI [String] )"_s,
        u"XB_GetTitle( xmlObject [XmlBLOB] )"_s,
        u"XB_GetAbstract( xmlObject [XmlBLOB] )"_s,
        u"XB_GetGeometry( xmlObject [XmlBLOB] )"_s,
        u"XB_GetLastParseError( [void] )"_s,
        u"XB_GetLastValidateError( [void] )"_s,
        u"XB_IsValidXPathExpression( expr [Text] )"_s,
        u"XB_GetLastXPathError( [void] )"_s,
        u"XB_CacheFlush( [void] )"_s,
        u"XB_LoadXML( filepath-or-URL [String] )"_s,
        u"XB_StoreXML( XmlObject [XmlBLOB] , filepath [String] )"_s,

      }
    },
    {
      Qgis::SqlKeywordCategory::Geospatial, {
        // SQL functions reporting GEOS / LWGEOM errors and warnings
        u"GEOS_GetLastWarningMsg( [void] )"_s,
        u"GEOS_GetLastErrorMsg( [void] )"_s,
        u"GEOS_GetLastAuxErrorMsg( [void] )"_s,
        u"GEOS_GetCriticalPointFromMsg( [void] )"_s,
        u"LWGEOM_GetLastWarningMsg( [void] )"_s,
        u"LWGEOM_GetLastErrorMsg( [void] )"_s,

        // SQL length/distance unit-conversion functions
        u"CvtToKm( x [Double precision] )"_s,
        u"CvtToDm( x [Double precision] )"_s,
        u"CvtToCm( x [Double precision] )"_s,
        u"CvtToMm( x [Double precision] )"_s,
        u"CvtToKmi( x [Double precision] )"_s,
        u"CvtToIn( x [Double precision] )"_s,
        u"CvtToFt( x [Double precision] )"_s,
        u"CvtToYd( x [Double precision] )"_s,
        u"CvtToMi( x [Double precision] )"_s,
        u"CvtToFath( x [Double precision] )"_s,
        u"CvtToCh( x [Double precision] )"_s,
        u"CvtToLink( x [Double precision] )"_s,
        u"CvtToUsIn( x [Double precision] )"_s,
        u"CvtToUsFt( x [Double precision] )"_s,
        u"CvtToUsYd( x [Double precision] )"_s,
        u"CvtToUsMi( x [Double precision] )"_s,
        u"CvtToUsCh( x [Double precision] )"_s,
        u"CvtToIndFt( x [Double precision] )"_s,
        u"CvtToIndYd( x [Double precision] )"_s,
        u"CvtToIndCh( x [Double precision] )"_s,

        // SQL conversion functions from DD/DMS notations (longitude/latitude)
        u"LongLatToDMS( longitude [Double precision] , latitude [Double precision] )"_s,
        u"LongitudeFromDMS( dms_expression [Sting] )"_s,

        // SQL utility functions [
        u"GeomFromExifGpsBlob( image [BLOB] )"_s,
        u"ST_Point( x [Double precision] , y [Double precision]  )"_s,
        u"MakeLine( pt1 [PointGeometry] , pt2 [PointGeometry] )"_s,
        u"MakeLine( geom [PointGeometry] )"_s,
        u"MakeLine( geom [MultiPointGeometry] , direction [Boolean] )"_s,
        u"SquareGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )"_s,
        u"TriangularGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )"_s,
        u"HexagonalGrid( geom [ArealGeometry] , size [Double precision] [ , edges_only [Boolean] , [ origing [PointGeometry] ] ] )"_s,
        u"Extent( geom [Geometry] )"_s,
        u"ToGARS( geom [Geometry] )"_s,
        u"GARSMbr( code [String] )"_s,
        u"MbrMinX( geom [Geometry])"_s,
        u"MbrMinY( geom [Geometry])"_s,
        u"MbrMaxX( geom [Geometry])"_s,
        u"MbrMaxY( geom [Geometry])"_s,
        u"ST_MinZ( geom [Geometry])"_s,
        u"ST_MaxZ( geom [Geometry])"_s,
        u"ST_MinM( geom [Geometry])"_s,
        u"ST_MaxM( geom [Geometry])"_s,

        // SQL functions for constructing a geometric object given its Well-known Text Representation
        u"GeomFromText( wkt [String] [ , SRID [Integer]] )"_s,
        u"ST_WKTToSQL( wkt [String] )"_s,
        u"PointFromText( wktPoint [String] [ , SRID [Integer]] )"_s,
        u"LineFromText( wktLineString [String] [ , SRID [Integer]] )"_s,
        u"PolyFromText( wktPolygon [String] [ , SRID [Integer]] )"_s,
        u"MPointFromText( wktMultiPoint [String] [ , SRID [Integer]] )"_s,
        u"MLineFromText( wktMultiLineString [String] [ , SRID [Integer]] )"_s,
        u"MPolyFromText( wktMultiPolygon [String] [ , SRID [Integer]] )"_s,
        u"GeomCollFromText( wktGeometryCollection [String] [ , SRID [Integer]] )"_s,
        u"BdPolyFromText( wktMultilinestring [String] [ , SRID [Integer]] )"_s,
        u"BdMPolyFromText( wktMultilinestring [String] [ , SRID [Integer]] )"_s,

        // SQL functions for constructing a geometric object given its Well-known Binary Representation
        u"GeomFromWKB( wkbGeometry [Binary] [ , SRID [Integer]] )"_s,
        u"ST_WKBToSQL( wkbGeometry [Binary] )"_s,
        u"PointFromWKB( wkbPoint [Binary] [ , SRID [Integer]] )"_s,
        u"LineFromWKB( wkbLineString [Binary] [ , SRID [Integer]] )"_s,
        u"PolyFromWKB( wkbPolygon [Binary] [ , SRID [Integer]] )"_s,
        u"MPointFromWKB( wkbMultiPoint [Binary] [ , SRID [Integer]] )"_s,
        u"MLineFromWKB( wkbMultiLineString [Binary] [ , SRID [Integer]] )"_s,
        u"MPolyFromWKB( wkbMultiPolygon [Binary] [ , SRID [Integer]] )"_s,
        u"GeomCollFromWKB( wkbGeometryCollection [Binary] [ , SRID [Integer]] )"_s,
        u"BdPolyFromWKB( wkbMultilinestring [Binary] [ , SRID [Integer]] )"_s,
        u"BdMPolyFromWKB( wkbMultilinestring [Binary] [ , SRID [Integer]] )"_s,

        // SQL functions for obtaining the Well-known Text / Well-known Binary Representation of a geometric object
        u"AsText( geom [Geometry] )"_s,
        u"AsWKT( geom [Geometry] [ , precision [Integer] ] )"_s,
        u"AsBinary( geom [Geometry] )"_s,

        // SQL functions supporting exotic geometric formats
        u"AsSVG( geom [Geometry] [ , relative [Integer] [ , precision [Integer] ] ] )"_s,
        u"AsKml( geom [Geometry] [ , precision [Integer] ] )"_s,
        u"GeomFromKml( KmlGeometry [String] )"_s,
        u"AsGml( geom [Geometry] [ , precision [Integer] ] )"_s,
        u"GeomFromGML( gmlGeometry [String] )"_s,
        u"AsGeoJSON( geom [Geometry] [ , precision [Integer] [ , options [Integer] ] ] )"_s,
        u"GeomFromGeoJSON( geoJSONGeometry [String] )"_s,
        u"AsEWKB( geom [Geometry] )"_s,
        u"GeomFromEWKB( ewkbGeometry [String] )"_s,
        u"AsEWKT( geom [Geometry] )"_s,
        u"GeomFromEWKT( ewktGeometry [String] )"_s,
        u"AsFGF( geom [Geometry] )"_s,
        u"GeomFromFGF( fgfGeometry [Binary] [ , SRID [Integer]] )"_s,

        // SQL functions on type Geometry
        u"Dimension( geom [Geometry] )"_s,
        u"CoordDimension( geom [Geometry] )"_s,
        u"ST_NDims( geom [Geometry] )"_s,
        u"ST_Is3D( geom [Geometry] )"_s,
        u"ST_IsMeasured( geom [Geometry] )"_s,
        u"GeometryType( geom [Geometry] )"_s,
        u"SRID( geom [Geometry] )"_s,
        u"SetSRID( geom [Geometry] , SRID [Integer] )"_s,
        u"IsEmpty( geom [Geometry] )"_s,
        u"IsSimple( geom [Geometry] )"_s,
        u"IsValid( geom [Geometry] )"_s,
        u"IsValidReason( geom [Geometry] )"_s,
        u"IsValidDetail( geom [Geometry] )"_s,
        u"Boundary( geom [Geometry] )"_s,
        u"Envelope( geom [Geometry] )"_s,
        u"ST_Expand( geom [Geometry] , amount [Double precision] )"_s,
        u"ST_NPoints( geom [Geometry] )"_s,
        u"ST_NRings( geom [Geometry] )"_s,
        u"ST_Reverse( geom [Geometry] )"_s,
        u"ST_ForceLHR( geom [Geometry] )"_s,

        // SQL functions attempting to repair malformed Geometries
        u"SanitizeGeometry( geom [Geometry] )"_s,

        // SQL Geometry-compression functions
        u"CompressGeometry( geom [Geometry] )"_s,
        u"UncompressGeometry( geom [Geometry] )"_s,

        // SQL Geometry-type casting functions
        u"CastToPoint( geom [Geometry] )"_s,
        u"CastToLinestring( geom [Geometry] )"_s,
        u"CastToPolygon( geom [Geometry] )"_s,
        u"CastToMultiPoint( geom [Geometry] )"_s,
        u"CastToMultiLinestring( geom [Geometry] )"_s,
        u"CastToMultiPolygon( geom [Geometry] )"_s,
        u"CastToGeometryCollection( geom [Geometry] )"_s,
        u"CastToMulti( geom [Geometry] )"_s,
        u"CastToSingle( geom [Geometry] )"_s,

        // SQL Space-dimensions casting functions
        u"CastToXY( geom [Geometry] )"_s,
        u"CastToXYZ( geom [Geometry] )"_s,
        u"CastToXYM( geom [Geometry] )"_s,
        u"CastToXYZM( geom [Geometry] )"_s,

        // SQL functions on type Point
        u"X( pt [Point] )"_s,
        u"Y( pt [Point] )"_s,
        u"Z( pt [Point] )"_s,
        u"M( pt [Point] )"_s,

        // SQL functions on type Curve [Linestring or Ring]
        u"StartPoint( c [Curve] )"_s,
        u"EndPoint( c [Curve] )"_s,
        u"GLength( c [Curve] )"_s,
        u"Perimeter( s [Surface] )"_s,
        u"GeodesicLength( c [Curve] )"_s,
        u"GreatCircleLength( c [Curve] )"_s,
        u"IsClosed( c [Curve] )"_s,
        u"IsRing( c [Curve] )"_s,
        u"PointOnSurface( s [Surface/Curve] )"_s,
        u"Simplify( c [Curve] , tolerance [Double precision] )"_s,
        u"SimplifyPreserveTopology( c [Curve] , tolerance [Double precision] )"_s,

        // SQL functions on type LineString
        u"NumPoints( line [LineString] )"_s,
        u"PointN( line [LineString] , n [Integer] )"_s,
        u"AddPoint( line [LineString] , point [Point] [ , position [Integer] ] )"_s,
        u"SetPoint( line [LineString] , position [Integer] , point [Point] )"_s,
        u"RemovePoint( line [LineString] , position [Integer] )"_s,

        // SQL functions on type Surface [Polygon or Ring]
        u"Centroid( s [Surface] )"_s,
        u"Area( s [Surface] )"_s,

        // SQL functions on type Polygon
        u"ExteriorRing( polyg [Polygon] )"_s,
        u"NumInteriorRing( polyg [Polygon] )"_s,
        u"InteriorRingN( polyg [Polygon] , n [Integer] )"_s,

        // SQL functions on type GeomCollection
        u"NumGeometries( geom [GeomCollection] )"_s,
        u"GeometryN( geom [GeomCollection] , n [Integer] )"_s,

        // SQL functions that test approximate spatial relationships via MBRs
        u"MbrEqual( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"MbrDisjoint( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"MbrTouches( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"MbrWithin( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"MbrOverlaps( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"MbrIntersects( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"ST_EnvIntersects( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"MbrContains( geom1 [Geometry] , geom2 [Geometry] )"_s,

        // SQL functions that test spatial relationships
        u"Equals( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Disjoint( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Touches( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Within( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Overlaps( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Crosses( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Intersects( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Contains( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Covers( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"CoveredBy( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"Relate( geom1 [Geometry] , geom2 [Geometry] , patternMatrix [String] )"_s,

        // SQL functions for distance relationships
        u"Distance( geom1 [Geometry] , geom2 [Geometry] )"_s,

        // SQL functions that implement spatial operators
        u"MakeValid( geom [Geometry] )"_s,
        u"MakeValidDiscarded( geom [Geometry] )"_s,
        u"Segmentize( geom [Geometry], dist [Double precision]  )"_s,
        u"Split( geom [Geometry], blade [Geometry]  )"_s,
        u"SplitLeft( geom [Geometry], blade [Geometry]  )"_s,
        u"SplitRight( geom [Geometry], blade [Geometry]  )"_s,
        u"Azimuth( pt1 [Geometry], pt2 [Geometry]  )"_s,
        u"Project( start_point [Geometry], distance [Double precision], azimuth [Double precision]  )"_s,
        u"SnapToGrid( geom [Geometry] , size [Double precision]  )"_s,
        u"GeoHash( geom [Geometry] )"_s,
        u"AsX3D( geom [Geometry] )"_s,
        u"MaxDistance( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"ST_3DDistance( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"ST_3DMaxDistance( geom1 [Geometry] , geom2 [Geometry] )"_s,
        u"ST_Node( geom [Geometry] )"_s,
        u"SelfIntersections( geom [Geometry] )"_s,

        // SQL functions for coordinate transformations
        u"Transform( geom [Geometry] , newSRID [Integer] )"_s,
        u"SridFromAuthCRS( auth_name [String] , auth_SRID [Integer] )"_s,
        u"ShiftCoords( geom [Geometry] , shiftX [Double precision] , shiftY [Double precision] )"_s,
        u"ST_Translate( geom [Geometry] , shiftX [Double precision] , shiftY [Double precision] , shiftZ [Double precision] )"_s,
        u"ST_Shift_Longitude( geom [Geometry] )"_s,
        u"NormalizeLonLat( geom [Geometry] )"_s,
        u"ScaleCoords( geom [Geometry] , scaleX [Double precision] [ , scaleY [Double precision] ] )"_s,
        u"RotateCoords( geom [Geometry] , angleInDegrees [Double precision] )"_s,
        u"ReflectCoords( geom [Geometry] , xAxis [Integer] , yAxis [Integer] )"_s,
        u"SwapCoords( geom [Geometry] )"_s,

        // SQL functions for Spatial-MetaData and Spatial-Index handling
        u"InitSpatialMetaData( void )"_s,
        u"InsertEpsgSrid( srid [Integer] )"_s,
        u"DiscardGeometryColumn( table [String] , column [String] )"_s,
        u"RegisterVirtualGeometry( table [String] )"_s,
        u"DropVirtualGeometry( table [String] )"_s,
        u"CreateSpatialIndex( table [String] , column [String] )"_s,
        u"CreateMbrCache( table [String] , column [String] )"_s,
        u"DisableSpatialIndex( table [String] , column [String] )"_s,
        u"CheckShadowedRowid( table [String] )"_s,
        u"CheckWithoutRowid( table [String] )"_s,
        u"CheckSpatialIndex( void )"_s,
        u"RecoverSpatialIndex( [ no_check"_s,
        u"InvalidateLayerStatistics( [ void )"_s,
        u"UpdateLayerStatistics( [ void )"_s,
        u"GetLayerExtent( table [String] [ , column [String] [ , mode [Boolean]] ] )"_s,
        u"CreateTopologyTables( SRID [Integer] , dims"_s,
        u"CreateRasterCoveragesTable( [void] )"_s,

        // SQL functions supporting the MetaCatalog and related Statistics
        u"CreateMetaCatalogTables( transaction [Integer] )"_s,
        u"UpdateMetaCatalogStatistics( transaction [Integer] , table_name [String] , column_name [String] )"_s,

        // SQL functions supporting SLD/SE Styled Layers
        u"CreateStylingTables()"_s,
        u"RegisterExternalGraphic( xlink_href [String] , resource [BLOB] )"_s,
        u"RegisterVectorStyledLayer( f_table_name [String] , f_geometry_column [String] , style [BLOB] )"_s,
        u"RegisterRasterStyledLayer( coverage_name [String] , style [BLOB] )"_s,
        u"RegisterStyledGroup( group_name [String] , f_table_name [String] , f_geometry_column [String] [ , paint_order [Integer] ] )"_s,
        u"SetStyledGroupInfos( group_name [String] , title [String] , abstract [String] )"_s,
        u"RegisterGroupStyle( group_name [String] , style [BLOB] )"_s,

        // SQL functions supporting ISO Metadata
        u"CreateIsoMetadataTables()"_s,
        u"RegisterIsoMetadata( scope [String] , metadata [BLOB] )"_s,
        u"GetIsoMetadataId( fileIdentifier [String] )"_s,

        // SQL functions implementing FDO/OGR compatibility
        u"CheckSpatialMetaData( void )"_s,
        u"AutoFDOStart( void )"_s,
        u"AutoFDOStop( void )"_s,
        u"InitFDOSpatialMetaData( void )"_s,
        u"DiscardFDOGeometryColumn( table [String] , column [String] )"_s,

        // SQL functions implementing OGC GeoPackage compatibility
        u"CheckGeoPackageMetaData( void )"_s,
        u"AutoGPKGStart( void )"_s,
        u"AutoGPKGStop( void )"_s,
        u"gpkgCreateBaseTables( void )"_s,
        u"gpkgInsertEpsgSRID( srid [Integer] )"_s,
        u"gpkgAddTileTriggers( tile_table_name [String] )"_s,
        u"gpkgGetNormalZoom( tile_table_name [String] , inverted_zoom_level [Integer] )"_s,
        u"gpkgGetNormalRow( tile_table_name [String] , normal_zoom_level [Integer] , inverted_row_number [Integer] )"_s,
        u"gpkgGetImageType( image [Blob] )"_s,
        u"gpkgAddGeometryTriggers( table_name [String] , geometry_column_name [String] )"_s,
        u"gpkgAddSpatialIndex( table_name [String] , geometry_column_name [String] )"_s,
        u"gpkgMakePoint (x [Double precision] , y [Double precision] )"_s,
        u"gpkgMakePointZ (x [Double precision] , y [Double precision] , z [Double precision] )"_s,
        u"gpkgMakePointM (x [Double precision] , y [Double precision] , m [Double precision] )"_s,
        u"gpkgMakePointZM (x [Double precision] , y [Double precision] , z [Double precision] , m [Double precision] )"_s,
        u"IsValidGPB( geom [Blob] )"_s,
        u"AsGPB( geom [BLOB encoded geometry] )"_s,
        u"GeomFromGPB( geom [GPKG Blob Geometry] )"_s,
        u"CastAutomagic( geom [Blob] )"_s,
        u"GPKG_IsAssignable( expected_type_name [String] , actual_type_name [String] )"_s,

      }
    }
  } );
}

QList<Qgis::FieldDomainType> QgsGeoPackageProviderConnection::supportedFieldDomainTypes() const
{
  return
  {
    Qgis::FieldDomainType::Coded,
    Qgis::FieldDomainType::Glob,
    Qgis::FieldDomainType::Range
  };
}

QString QgsGeoPackageProviderConnection::databaseQueryLogIdentifier() const
{
  return u"QgsGeoPackageProviderConnection"_s;
}


///@endcond
