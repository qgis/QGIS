/***************************************************************************
  qgsogrproviderconnection.cpp

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

#include "qgsogrproviderconnection.h"

#include "qgsapplication.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsfeedback.h"
#include "qgsfielddomain.h"
#include "qgsmessagelog.h"
#include "qgsogrprovider.h"
#include "qgsogrproviderutils.h"
#include "qgsogrutils.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgssqlstatement.h"
#include "qgsvectorlayer.h"
#include "qgsweakrelation.h"

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,4,0)
#include "qgsgdalutils.h"
#endif

#include <QTextCodec>
#include <QRegularExpression>

#include <chrono>

///@cond PRIVATE

//
// QgsOgrProviderResultIterator
//

QgsOgrProviderResultIterator::QgsOgrProviderResultIterator( gdal::dataset_unique_ptr hDS, OGRLayerH ogrLayer )
  : mHDS( std::move( hDS ) )
  , mOgrLayer( ogrLayer )
{
  if ( mOgrLayer && OGR_L_TestCapability( mOgrLayer, OLCFastFeatureCount ) )
  {
    // Do not scan the layer!
    mRowCount = OGR_L_GetFeatureCount( mOgrLayer, false );
  }
}

QgsOgrProviderResultIterator::~QgsOgrProviderResultIterator()
{
  if ( mHDS )
  {
    GDALDatasetReleaseResultSet( mHDS.get(), mOgrLayer );
  }
}

void QgsOgrProviderResultIterator::setPrimaryKeyColumnName( const QString &primaryKeyColumnName )
{
  mPrimaryKeyColumnName = primaryKeyColumnName;
}

void QgsOgrProviderResultIterator::setPrimaryKeyColumnIndex( int primaryKeyColumnIndex )
{
  mPrimaryKeyColumnIndex = primaryKeyColumnIndex;
}

QVariantList QgsOgrProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow = mNextRow;
  mNextRow = nextRowInternal();
  return currentRow;
}

QVariantList QgsOgrProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mHDS && mOgrLayer )
  {
    gdal::ogr_feature_unique_ptr fet;
    if ( fet.reset( OGR_L_GetNextFeature( mOgrLayer ) ), fet )
    {
      if ( ! mFields.isEmpty() )
      {
        const QgsFeature f { QgsOgrUtils::readOgrFeature( fet.get(), mFields, QTextCodec::codecForName( "UTF-8" ) ) };
        const QgsAttributes constAttrs  = f.attributes();
        for ( const QVariant &attribute : constAttrs )
        {
          row.push_back( attribute );
        }

        if ( !mGeometryColumns.empty() )
        {
          const int colIndex { mGeometryColumns.cbegin()->first };
          if ( colIndex < 0 || colIndex >= f.fields().count() )
          {
            row.push_back( f.geometry().asWkb() );
          }
          else
          {
            row.insert( colIndex, f.geometry().asWkb() );
          }
        }
      }
      else // Fallback to strings
      {
        for ( int i = 0; i < OGR_F_GetFieldCount( fet.get() ); i++ )
        {
          row.push_back( QVariant( QString::fromUtf8( OGR_F_GetFieldAsString( fet.get(), i ) ) ) );
        }

        // Geometry
        for ( auto &[columnIndex, columnName] : mGeometryColumns )
        {
          const int colOgrIndex = OGR_F_GetGeomFieldIndex( fet.get(), columnName.toUtf8().constData() );
          if ( colOgrIndex < 0 )
          {
            // Emit warning
            QgsMessageLog::logMessage( u"Geometry column '%1' not found in layer."_s.arg( columnName ), u"OGR"_s, Qgis::MessageLevel::Warning );
            continue;
          }

          const OGRGeometryH hGeom { OGR_F_GetGeomFieldRef( fet.get(), colOgrIndex ) };
          if ( hGeom )
          {
            // Get the WKB representation of the geometry
            const int size = OGR_G_WkbSize( hGeom );
            QByteArray wkbBuffer( size, Qt::Initialization::Uninitialized );
            OGR_G_ExportToWkb( hGeom, wkbNDR, reinterpret_cast<unsigned char *>( wkbBuffer.data() ) );
            if ( columnIndex < 0 || columnIndex >= row.count() )
            {
              row.push_back( wkbBuffer );
            }
            else
            {
              row.insert( columnIndex,  wkbBuffer );
            }
          }
          else
          {
            // Emit warning
            QgsMessageLog::logMessage( u"Geometry column '%1' has null geometry."_s.arg( columnName ), u"OGR"_s, Qgis::MessageLevel::Warning );
          }
        }
      }

      // PK
      if ( ! mPrimaryKeyColumnName.isEmpty() )
      {
        if ( mPrimaryKeyColumnIndex < 0 || mPrimaryKeyColumnIndex >= row.count() )
        {
          row.push_back( OGR_F_GetFID( fet.get() ) );
        }
        else
        {
          row.insert( mPrimaryKeyColumnIndex, OGR_F_GetFID( fet.get() ) );
        }
      }
    }
  }
  return row;
}

bool QgsOgrProviderResultIterator::hasNextRowPrivate() const
{
  return !mNextRow.isEmpty();
}

long long QgsOgrProviderResultIterator::rowCountPrivate() const
{
  return mRowCount;
}

void QgsOgrProviderResultIterator::setFields( const QgsFields &fields )
{
  mFields = fields;
}

void QgsOgrProviderResultIterator::addGeometryColumn( const QString &geometryColumnName, int index )
{
  mGeometryColumns[index] = geometryColumnName;
}

//
// QgsOgrProviderConnection
//

QgsOgrProviderConnection::QgsOgrProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = u"ogr"_s;
}

QgsOgrProviderConnection::QgsOgrProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = u"ogr"_s;

  // Cleanup the URI in case it contains other information other than the file path
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( uri );
  if ( !parts.value( u"path"_s ).toString().isEmpty() && parts.value( u"path"_s ).toString() != uri )
  {
    QVariantMap cleanedParts;
    cleanedParts.insert( u"path"_s, parts.value( u"path"_s ).toString() );

    if ( !parts.value( u"vsiPrefix"_s ).toString().isEmpty() )
      cleanedParts.insert( u"vsiPrefix"_s, parts.value( u"vsiPrefix"_s ).toString() );

    const QString cleanedUri = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->encodeUri( cleanedParts );
    setUri( cleanedUri );
  }
  setDefaultCapabilities();
}

void QgsOgrProviderConnection::store( const QString & ) const
{
}

void QgsOgrProviderConnection::remove( const QString & ) const
{
}

QString QgsOgrProviderConnection::tableUri( const QString &, const QString &name ) const
{
  QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( uri() );

  if ( !mSingleTableDataset )
    parts.insert( u"layerName"_s, name );

  return QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->encodeUri( parts );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsOgrProviderConnection::tables( const QString &, const TableFlags &flags, QgsFeedback *feedback ) const
{
  QList<QgsAbstractDatabaseProviderConnection::TableProperty> tableInfo;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s );
  const QList< QgsProviderSublayerDetails > subLayers = metadata->querySublayers(
        uri(),
        ( flags & TableFlag::IncludeSystemTables ) ? Qgis::SublayerQueryFlag::IncludeSystemTables : Qgis::SublayerQueryFlags(),
        feedback );

  tableInfo.reserve( subLayers.size() );
  for ( const QgsProviderSublayerDetails &subLayer : subLayers )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    tableInfo.append( table( QString(), subLayer.name() ) );
  }

  // Filters
  if ( flags )
  {
    tableInfo.erase( std::remove_if( tableInfo.begin(), tableInfo.end(), [ & ]( const QgsAbstractDatabaseProviderConnection::TableProperty & ti )
    {
      return !( ti.flags() & flags );
    } ), tableInfo.end() );
  }

  return tableInfo;
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsOgrProviderConnection::table( const QString &, const QString &table, QgsFeedback * ) const
{
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( uri() );
  const QString path = parts.value( u"path"_s ).toString();

  QgsAbstractDatabaseProviderConnection::TableProperty property;
  if ( !mSingleTableDataset )
    property.setTableName( table );

  QgsOgrLayerUniquePtr userLayer;
  QString errCause;
  if ( !mSingleTableDataset )
  {
    userLayer = QgsOgrProviderUtils::getLayer( path, table, errCause );
  }
  else
  {
    userLayer = QgsOgrProviderUtils::getLayer( path, false, {}, 0, errCause, true );
  }

  if ( !userLayer )
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while retrieving table properties: %1" ).arg( errCause ) );

  QRecursiveMutex *mutex = nullptr;
  OGRLayerH layer = userLayer->getHandleAndMutex( mutex );
  QMutexLocker locker( mutex );

  const QString abstract = GDALGetMetadataItem( layer, "DESCRIPTION", "" );
  if ( !abstract.isEmpty() )
    property.setComment( abstract );

  const QByteArray fidColumn( userLayer->GetFIDColumn() );
  if ( !fidColumn.isEmpty() )
  {
    property.setPrimaryKeyColumns( { QString( fidColumn )} );
  }

  QgsOgrFeatureDefn &fdef = userLayer->GetLayerDefn();
  property.setGeometryColumnCount( fdef.GetGeomFieldCount() );
  if ( property.geometryColumnCount() > 0 )
  {
    OGRGeomFieldDefnH geomH = fdef.GetGeomFieldDefn( 0 );
    property.setGeometryColumn( QString::fromUtf8( OGR_GFld_GetNameRef( geomH ) ) );
  }

  QgsAbstractDatabaseProviderConnection::TableProperty::GeometryColumnType geomType;
  if ( OGRSpatialReferenceH spatialRefSys = userLayer->GetSpatialRef() )
  {
    geomType.crs = QgsOgrUtils::OGRSpatialReferenceToCrs( spatialRefSys );
  }
  geomType.wkbType = QgsOgrUtils::ogrGeometryTypeToQgsWkbType( fdef.GetGeomType() );

  if ( geomType.wkbType != Qgis::WkbType::NoGeometry )
  {
    property.setGeometryColumnTypes( { geomType } );
    property.setFlag( TableFlag::Vector );
  }
  else
  {
    property.setFlag( TableFlag::Aspatial );
  }

  return property;
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOgrProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeGdalSqlPrivate( sql, feedback );
}

QgsVectorLayer *QgsOgrProviderConnection::createSqlVectorLayer( const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions &options ) const
{
  QgsProviderMetadata *providerMetadata { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
  Q_ASSERT( providerMetadata );
  QMap<QString, QVariant> decoded = providerMetadata->decodeUri( uri() );

  QString where;
  QStringList columns;
  QStringList tables;

  QgsAbstractDatabaseProviderConnection::splitSimpleQuery( options.sql, columns, tables, where );

  // We have two options here: if the original SQL is a plain SELECT * FROM table [WHERE ...] statement,
  // we could turn this into a normal layer with a subset filter but this would be a bit inconsistent from a UX
  // perspective because the SQL update menu would not be available, let's keep it a SQL layer always.

  if ( !options.filter.isEmpty() )
  {
    if ( ! where.isEmpty() )
    {
      QString sql {  sanitizeSqlForQueryLayer( options.sql ) };
      const thread_local QRegularExpression whereRegExp( R"sql(\s+WHERE\s+.+$)sql", QRegularExpression::CaseInsensitiveOption );
      sql.remove( whereRegExp );
      decoded[ u"subset"_s ] = QStringLiteral( R"sql(%1 WHERE ( %2 ) AND ( %3 ))sql" ).arg( sql, where, options.filter );
    }
    else
    {
      decoded[ u"subset"_s ] = QStringLiteral( R"sql(%1 WHERE ( %2 ))sql" ).arg( sanitizeSqlForQueryLayer( options.sql ), options.filter );
    }
  }
  else
  {
    decoded[ u"subset"_s ] = sanitizeSqlForQueryLayer( options.sql );
  }

  return new QgsVectorLayer( providerMetadata->encodeUri( decoded ), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey() );
}

void QgsOgrProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    Qgis::WkbType wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  GDALDriverH hDriver = GDALIdentifyDriverEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve driver for connection" ) );
  }

  QMap<QString, QVariant> opts { *options };
  opts[ u"layerName"_s ] = QVariant( name );
  opts[ u"update"_s ] = true;
  opts[ u"driverName"_s ] = QString( GDALGetDriverShortName( hDriver ) );
  QMap<int, int> map;
  QString errCause;
  QString createdLayerName;
  Qgis::VectorExportResult errCode = QgsOgrProvider::createEmptyLayer(
                                       uri(),
                                       fields,
                                       wkbType,
                                       srs,
                                       overwrite,
                                       &map,
                                       createdLayerName,
                                       &errCause,
                                       &opts
                                     );
  // TODO we need some way to hand createdLayerName back to caller, as it may differ from the requested name...
  if ( errCode != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsOgrProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &options, QVariantMap &providerOptions ) const
{
  if ( !options.schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  // OGR provider uses "layerName" from options rather then the table name from the URI
  providerOptions.clear();
  providerOptions.insert( u"layerName"_s, options.layerName );

  return uri();
}

void QgsOgrProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }
  QString errCause;
  const QString layerUri = tableUri( schema, name );
  if ( ! QgsOgrProviderUtils::deleteLayer( layerUri, errCause ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name, errCause ) );
  }
}

void QgsOgrProviderConnection::vacuum( const QString &, const QString &name ) const
{
  Q_UNUSED( name );
  checkCapability( Capability::Vacuum );

  if ( mDriverName == "OpenFileGDB"_L1 )
  {
    if ( !name.isEmpty() )
      executeGdalSqlPrivate( u"REPACK \"%1\""_s.arg( name ) );
    else
      executeGdalSqlPrivate( u"REPACK"_s );
  }
}

void QgsOgrProviderConnection::setDefaultCapabilities()
{
  GDALDriverH hDriver = GDALIdentifyDriverEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
    return;

  mDriverName = GDALGetDriverShortName( hDriver );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  mGeometryColumnCapabilities = GeometryColumnCapability::SinglePoint;

  if ( const char *pszGeometryFlags = GDALGetMetadataItem( hDriver, GDAL_DMD_GEOMETRY_FLAGS, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszGeometryFlags, " ", 0 );
    if ( CSLFindString( papszTokens, "EquatesMultiAndSingleLineStringDuringWrite" ) < 0 )
    {
      mGeometryColumnCapabilities |= GeometryColumnCapability::SingleLineString;
    }
    if ( CSLFindString( papszTokens, "EquatesMultiAndSinglePolygonDuringWrite" ) < 0 )
    {
      mGeometryColumnCapabilities |= GeometryColumnCapability::SinglePolygon;
    }
    CSLDestroy( papszTokens );
  }
  else
  {
    mGeometryColumnCapabilities |= GeometryColumnCapability::SingleLineString;
    mGeometryColumnCapabilities |= GeometryColumnCapability::SinglePolygon;
  }
#else
  mGeometryColumnCapabilities |= GeometryColumnCapability::SinglePoint;
  mGeometryColumnCapabilities |= GeometryColumnCapability::SingleLineString;
  mGeometryColumnCapabilities |= GeometryColumnCapability::SinglePolygon;
#endif

  char **driverMetadata = GDALGetMetadata( hDriver, nullptr );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_Z_GEOMETRIES, false ) )
    mGeometryColumnCapabilities |= GeometryColumnCapability::Z;
#else
  mGeometryColumnCapabilities |= GeometryColumnCapability::Z; // Prior to GDAL 3.6 there was no generic way to test these per driver/dataset
#endif

  mCapabilities = Capability::SqlLayers
                  | Capability::ExecuteSql
                  | Capability::Tables;

  if ( !CSLFetchBoolean( driverMetadata, GDAL_DCAP_NONSPATIAL, false ) && CSLFetchBoolean( driverMetadata, GDAL_DCAP_VECTOR, false ) )
    mCapabilities |= Capability::Spatial;

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  mSingleTableDataset = !GDALGetMetadataItem( hDriver, GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, nullptr );
#else
  {
    const QVariantMap uriParts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( uri() );
    const QFileInfo pathInfo( uriParts.value( u"path"_s ).toString() );
    const QString suffix = uriParts.value( u"vsiSuffix"_s ).toString().isEmpty()
                           ? pathInfo.suffix().toLower()
                           : QFileInfo( uriParts.value( u"vsiSuffix"_s ).toString() ).suffix().toLower();
    mSingleTableDataset = !QgsGdalUtils::multiLayerFileExtensions().contains( suffix );
  }
#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  if ( mDriverName == "OpenFileGDB"_L1 )
  {
    mCapabilities |= Vacuum;
  }
#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE_FIELD, false ) )
    mCapabilities |= AddField;
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_DELETE_FIELD, false ) )
    mCapabilities |= DeleteField;

  if ( const char *pszAlterFieldDefnFlags = GDALGetMetadataItem( hDriver, GDAL_DMD_ALTER_FIELD_DEFN_FLAGS, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszAlterFieldDefnFlags, " ", 0 );
    if ( CSLFindString( papszTokens, "Name" ) >= 0 )
    {
      mCapabilities |= RenameField;
    }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
    // only supported on gdal 3.7 and above
    if ( CSLFindString( papszTokens, "AlternativeName" ) >= 0 )
    {
      mCapabilities2 |= Qgis::DatabaseProviderConnectionCapability2::SetFieldAlias;
    }
    if ( CSLFindString( papszTokens, "Comment" ) >= 0 )
    {
      mCapabilities2 |= Qgis::DatabaseProviderConnectionCapability2::SetFieldComment;
    }
#endif

    CSLDestroy( papszTokens );
  }
#else
  // Prior to GDAL 3.6 there was no generic way to test these per driver/dataset
  mCapabilities |= AddField;
  mCapabilities |= DeleteField;
  mCapabilities |= RenameField;
#endif

  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    // fallback to read only otherwise
    hDS.reset( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  }

  if ( hDS )
  {
    if ( GDALDatasetTestCapability( hDS.get(), ODsCCurveGeometries ) )
      mGeometryColumnCapabilities |= GeometryColumnCapability::Curves;

    if ( GDALDatasetTestCapability( hDS.get(), ODsCMeasuredGeometries ) )
      mGeometryColumnCapabilities |= GeometryColumnCapability::M;

    if ( !mSingleTableDataset )
    {
      if ( GDALDatasetTestCapability( hDS.get(), ODsCCreateLayer ) )
        mCapabilities |= CreateVectorTable;

      if ( GDALDatasetTestCapability( hDS.get(), ODsCDeleteLayer ) )
        mCapabilities |= DropVectorTable;
    }

    if ( GDALDatasetTestCapability( hDS.get(), ODsCUpdateFieldDomain ) )
    {
      mCapabilities2 |= Qgis::DatabaseProviderConnectionCapability2::EditFieldDomain;
    }

    if ( GDALDatasetTestCapability( hDS.get(), ODsCDeleteFieldDomain ) )
    {
      mCapabilities2 |= Qgis::DatabaseProviderConnectionCapability2::DeleteFieldDomain;
    }
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_FIELD_DOMAINS, false ) )
  {
    mCapabilities |= Capability::RetrieveFieldDomain;
    mCapabilities |= Capability::ListFieldDomains;
    mCapabilities |= Capability::SetFieldDomain;
    mCapabilities |= Capability::AddFieldDomain;
  }
#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_RELATIONSHIPS, false ) )
  {
    mCapabilities |= Capability::RetrieveRelationships;
  }
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE_RELATIONSHIP, false ) )
  {
    mCapabilities |= Capability::AddRelationship;
  }
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_UPDATE_RELATIONSHIP, false ) )
  {
    mCapabilities |= Capability::UpdateRelationship;
  }
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_DELETE_RELATIONSHIP, false ) )
  {
    mCapabilities |= Capability::DeleteRelationship;
  }

  if ( const char *pszRelationshipFlags = GDALGetMetadataItem( hDriver, GDAL_DMD_RELATIONSHIP_FLAGS, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszRelationshipFlags, " ", 0 );
    if ( CSLFindString( papszTokens, "OneToOne" ) >= 0 )
      mSupportedRelationshipCardinality.append( Qgis::RelationshipCardinality::OneToOne );

    if ( CSLFindString( papszTokens, "OneToMany" ) >= 0 )
      mSupportedRelationshipCardinality.append( Qgis::RelationshipCardinality::OneToMany );

    if ( CSLFindString( papszTokens, "ManyToOne" ) >= 0 )
      mSupportedRelationshipCardinality.append( Qgis::RelationshipCardinality::ManyToOne );

    if ( CSLFindString( papszTokens, "ManyToMany" ) >= 0 )
      mSupportedRelationshipCardinality.append( Qgis::RelationshipCardinality::ManyToMany );

    if ( CSLFindString( papszTokens, "Composite" ) >= 0 )
      mSupportedRelationshipStrength.append( Qgis::RelationshipStrength::Composition );

    if ( CSLFindString( papszTokens, "Association" ) >= 0 )
      mSupportedRelationshipStrength.append( Qgis::RelationshipStrength::Association );

    if ( CSLFindString( papszTokens, "MultipleFieldKeys" ) >= 0 )
      mRelationshipCapabilities |= Qgis::RelationshipCapability::MultipleFieldKeys;

    if ( CSLFindString( papszTokens, "ForwardPathLabel" ) >= 0 )
      mRelationshipCapabilities |= Qgis::RelationshipCapability::ForwardPathLabel;

    if ( CSLFindString( papszTokens, "BackwardPathLabel" ) >= 0 )
      mRelationshipCapabilities |= Qgis::RelationshipCapability::BackwardPathLabel;

    CSLDestroy( papszTokens );
  }
#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  if ( const char *pszIllegalFieldNames = GDALGetMetadataItem( hDriver, GDAL_DMD_ILLEGAL_FIELD_NAMES, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszIllegalFieldNames, " ", 0 );
    const QStringList illegalFieldNames = QgsOgrUtils::cStringListToQStringList( papszTokens );
    for ( const QString &name : illegalFieldNames )
      mIllegalFieldNames.insert( name );
    CSLDestroy( papszTokens );
  }
#else
  if ( mDriverName == "OpenFileGDB"_L1 || mDriverName == "FileGDB"_L1 )
  {
    mIllegalFieldNames =
    {
      u"ADD"_s,
      u"ALTER"_s,
      u"AND"_s,
      u"BETWEEN"_s,
      u"BY"_s,
      u"COLUMN"_s,
      u"CREATE"_s,
      u"DELETE"_s,
      u"DROP"_s,
      u"EXISTS"_s,
      u"FOR"_s,
      u"FROM"_s,
      u"GROUP"_s,
      u"IN"_s,
      u"INSERT"_s,
      u"INTO"_s,
      u"IS"_s,
      u"LIKE"_s,
      u"NOT"_s,
      u"NULL"_s,
      u"OR"_s,
      u"ORDER"_s,
      u"SELECT"_s,
      u"SET"_s,
      u"TABLE"_s,
      u"UPDATE"_s,
      u"VALUES"_s,
      u"WHERE"_s
    };
  }
#endif

  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
  };

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  if ( const char *pszRelatedTableTypes = GDALGetMetadataItem( hDriver, GDAL_DMD_RELATIONSHIP_RELATED_TABLE_TYPES, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszRelatedTableTypes, " ", 0 );
    mRelatedTableTypes = QgsOgrUtils::cStringListToQStringList( papszTokens );
    CSLDestroy( papszTokens );
  }
#else
  if ( mDriverName == "OpenFileGDB"_L1 )
  {
    mRelatedTableTypes = QStringList
    {
      u"media"_s,
      u"features"_s
    };
  }
  else if ( mDriverName == "GPKG"_L1 )
  {
    mRelatedTableTypes = QStringList
    {
      u"media"_s,
      u"simple_attributes"_s,
      u"features"_s,
      u"attributes"_s,
      u"tiles"_s
    };
  }
#endif
}

QString QgsOgrProviderConnection::databaseQueryLogIdentifier() const
{
  return u"QgsOgrProviderConnection"_s;
}

QString QgsOgrProviderConnection::primaryKeyColumnName( const QString &table ) const
{
  const QgsAbstractDatabaseProviderConnection::TableProperty tableProperty = QgsOgrProviderConnection::table( QString(), table );
  return tableProperty.primaryKeyColumns().value( 0 );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsOgrProviderConnection::executeGdalSqlPrivate( const QString &sql, QgsFeedback *feedback ) const
{
  QgsDatabaseQueryLogWrapper logWrapper( sql, uri(), providerKey(), databaseQueryLogIdentifier(), QGS_QUERY_LOG_ORIGIN );

  if ( feedback && feedback->isCanceled() )
  {
    logWrapper.setCanceled();
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  QString errCause;
  // try first using an editable datasource
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    // fallback to read only otherwise
    hDS.reset( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  }

  if ( hDS )
  {
    if ( feedback && feedback->isCanceled() )
    {
      logWrapper.setCanceled();
      return QgsAbstractDatabaseProviderConnection::QueryResult();
    }

    // Analyze the SQL to determine the properties but first remove LIMIT clause for parsing
    thread_local const QRegularExpression limit { R"(\s+limit\s+\d+\s*;*\s*$)", QRegularExpression::CaseInsensitiveOption };
    QString sqlNoLimit = sql;
    sqlNoLimit.replace( limit, QString() );
    const QgsSQLStatement statement { sqlNoLimit };
    QStringList columnNames;
    QStringList tableNames;

    bool hasAsterisk { false };
    bool hasMultipleGeometries { false };

    // The parser isn't perfect: GDAL may have a chance anyway
    if ( ! statement.hasParserError() )
    {
      const QgsSQLStatement::NodeSelect *nodeSelect = dynamic_cast<const QgsSQLStatement::NodeSelect *>( statement.rootNode() );
      const QList<QgsSQLStatement::NodeSelectedColumn *> columnList { nodeSelect->columns() };
      const QList<QgsSQLStatement::NodeTableDef *> tableList { nodeSelect->tables() };

      for ( const QgsSQLStatement::NodeTableDef *tableDef : std::as_const( tableList ) )
      {
        tableNames.append( tableDef->name() );
      }
      for ( const QgsSQLStatement::NodeSelectedColumn *colNode : std::as_const( columnList ) )
      {
        const QString columnName { colNode->dump() };
        // Determine if the column name contains any ST_ functions
        const thread_local QRegularExpression stFunctionRegex( R"sql(\bST_[A-Za-z_]+\s*\()sql" );
        if ( stFunctionRegex.match( columnName ).hasMatch() )
        {
          hasMultipleGeometries = true;
        }
        if ( columnName.endsWith( ".*"_L1 ) )
        {
          hasAsterisk = true;
          // get table prefix
          const QString tableName { columnName.section( QLatin1Char( '.' ), 0, 0 ) };
          // Get all fields in the table
          const QStringList fieldNames = fields( QString(), tableName ).names();
          columnNames.append( fieldNames );
        }
        else if ( columnName == QLatin1Char( '*' ) )
        {
          hasAsterisk = true;
          if ( tableNames.size() == 1 )
          {
            // Get all fields in the table
            const QStringList fieldNames = fields( QString(), tableNames.first() ).names();
            columnNames.append( fieldNames );
          }
          else
          {
            const QString errMsg = QObject::tr( "Ambiguous use of * in SQL statement %1 with multiple tables" ).arg( sql );
            logWrapper.setError( errMsg );
            throw QgsProviderConnectionException( errMsg );
          }
        }
        else
        {
          const QString alias {colNode->alias()};
          columnNames.append( alias.isEmpty() ? columnName : alias );
        }
      }
    }

    // Execute the query
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS.get(), sql.toUtf8().constData(), nullptr, hasMultipleGeometries ? "INDIRECT_SQLITE" : nullptr ) );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // Read fields
    if ( ogrLayer )
    {
      auto iterator = std::make_shared<QgsOgrProviderResultIterator>( std::move( hDS ), ogrLayer );

      QgsAbstractDatabaseProviderConnection::QueryResult results( iterator );
      results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );

      gdal::ogr_feature_unique_ptr fet;

      if ( fet.reset( OGR_L_GetNextFeature( ogrLayer ) ), fet )
      {

        QString pkColumnName;
        QStringList geomColumnNames;

        if ( ! tableNames.isEmpty() )
        {
          pkColumnName = primaryKeyColumnName( tableNames.first() );
        }

        const OGRFeatureDefnH featureDef = OGR_F_GetDefnRef( fet.get() );

        if ( featureDef )
        {
          const int geomCount { OGR_F_GetGeomFieldCount( fet.get() ) };
          for ( int geomIdx = 0; geomIdx < geomCount; ++geomIdx )
          {
            OGRGeomFieldDefnH geomFldDef { OGR_F_GetGeomFieldDefnRef( fet.get(), geomIdx ) };
            if ( geomFldDef )
            {
              const QString geomColumnName { OGR_GFld_GetNameRef( geomFldDef ) };
              if ( ! geomColumnName.isEmpty() )
              {
                geomColumnNames.append( geomColumnName );
              }
            }
          }
        }

        const QgsFields fields { QgsOgrUtils::readOgrFields( fet.get(), QTextCodec::codecForName( "UTF-8" ) ) };
        iterator->setFields( fields );

        // If SQL had parser errors get columns from the feature
        if ( columnNames.empty() )
        {

          for ( const QgsField &field : std::as_const( fields ) )
          {
            columnNames.append( field.name() );
          }

          // Insert pk at the beginning
          if ( ! pkColumnName.isEmpty() && !columnNames.contains( pkColumnName ) )
          {
            columnNames.insert( 0, pkColumnName );
          }

          // Append geom
          for ( const auto &geomColumnName : std::as_const( geomColumnNames ) )
          {
            if ( ! columnNames.contains( geomColumnName ) )
            {
              columnNames.append( geomColumnName );
            }
          }
        }

        for ( const auto &geomColumnName : std::as_const( geomColumnNames ) )
        {
          iterator->addGeometryColumn( geomColumnName, columnNames.indexOf( geomColumnName ) );
        }

        if ( ! pkColumnName.isEmpty() && ( hasAsterisk || columnNames.contains( pkColumnName ) ) )
        {
          iterator->setPrimaryKeyColumnName( pkColumnName );
          iterator->setPrimaryKeyColumnIndex( static_cast<int>( columnNames.indexOf( pkColumnName ) ) );
        }

        for ( const auto &name : std::as_const( columnNames ) )
        {
          results.appendColumn( name );
        }

      }

      // Check for errors
      if ( CE_Failure == CPLGetLastErrorType() || CE_Fatal == CPLGetLastErrorType() )
      {
        errCause = CPLGetLastErrorMsg( );
      }

      if ( ! errCause.isEmpty() )
      {
        logWrapper.setError( errCause );
        throw QgsProviderConnectionException( QObject::tr( "Error executing SQL statement %1: %2" ).arg( sql, errCause ) );
      }

      OGR_L_ResetReading( ogrLayer );
      iterator->nextRow();

      return results;
    }

    // Check for errors
    if ( CE_Failure == CPLGetLastErrorType() || CE_Fatal == CPLGetLastErrorType() )
    {
      errCause = CPLGetLastErrorMsg( );
    }

  }
  else
  {
    errCause = QObject::tr( "Could not open %1" ).arg( uri() );
  }

  if ( !errCause.isEmpty() )
  {
    logWrapper.setError( errCause );
    throw QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql, errCause ) );
  }

  return QgsAbstractDatabaseProviderConnection::QueryResult();
}

QList<QgsVectorDataProvider::NativeType> QgsOgrProviderConnection::nativeTypes() const
{
  GDALDriverH hDriver = GDALIdentifyDriverEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve driver for connection" ) );
  }

  return QgsOgrUtils::nativeFieldTypesForDriver( hDriver );
}

QStringList QgsOgrProviderConnection::fieldDomainNames() const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    // In some cases (empty geopackage for example), opening in read-only
    // mode fails, so retry in update mode
    hDS.reset( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  }

  if ( hDS )
  {
    QStringList names;
    if ( char **domainNames = GDALDatasetGetFieldDomainNames( hDS.get(), nullptr ) )
    {
      names = QgsOgrUtils::cStringListToQStringList( domainNames );
      CSLDestroy( domainNames );
    }
    return names;
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  throw QgsProviderConnectionException( QObject::tr( "Listing field domains for datasets requires GDAL 3.5 or later" ) );
#endif
}

QList<Qgis::FieldDomainType> QgsOgrProviderConnection::supportedFieldDomainTypes() const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
  GDALDriverH hDriver = GDALIdentifyDriverEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
    return {};

  bool supportsRange = false;
  bool supportsGlob = false;
  bool supportsCoded = false;
  if ( const char *pszDomainTypes = GDALGetMetadataItem( hDriver, GDAL_DMD_CREATION_FIELD_DOMAIN_TYPES, nullptr ) )
  {
    char **papszTokens = CSLTokenizeString2( pszDomainTypes, " ", 0 );
    supportsCoded = CSLFindString( papszTokens, "Coded" ) >= 0;
    supportsRange = CSLFindString( papszTokens, "Range" ) >= 0;
    supportsGlob = CSLFindString( papszTokens, "Glob" ) >= 0;
    CSLDestroy( papszTokens );
  }

  QList<Qgis::FieldDomainType> res;
  if ( supportsCoded )
    res << Qgis::FieldDomainType::Coded;
  if ( supportsRange )
    res << Qgis::FieldDomainType::Range;
  if ( supportsGlob )
    res << Qgis::FieldDomainType::Glob;
  return res;
#else
  return {};
#endif
}

QgsFieldDomain *QgsOgrProviderConnection::fieldDomain( const QString &name ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    // In some cases (empty geopackage for example), opening in read-only
    // mode fails, so retry in update mode
    hDS.reset( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  }

  if ( hDS )
  {
    if ( OGRFieldDomainH domain = GDALDatasetGetFieldDomain( hDS.get(), name.toUtf8().constData() ) )
    {
      std::unique_ptr< QgsFieldDomain > res = QgsOgrUtils::convertFieldDomain( domain );
      if ( res )
      {
        return res.release();
      }
    }
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve field domain %1!" ).arg( name ) );
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  ( void )name;
  throw QgsProviderConnectionException( QObject::tr( "Retrieving field domains for datasets requires GDAL 3.3 or later" ) );
#endif
}

void QgsOgrProviderConnection::setFieldDomainName( const QString &fieldName, const QString &schema, const QString &tableName, const QString &domainName ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( uri(),
                               true,
                               QStringList(),
                               tableName, errCause, true );
  if ( !layer )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset: %1" ).arg( errCause ) );
  }

  //type does not matter, it will not be used
  gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), OFTReal ) );
  OGR_Fld_SetDomainName( fld.get(), domainName.toUtf8().constData() );

  const int fieldIndex = layer->GetLayerDefn().GetFieldIndex( fieldName.toUtf8().constData() );
  if ( fieldIndex < 0 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set field domain for %1 - field does not exist" ).arg( fieldName ) );
  }
  if ( layer->AlterFieldDefn( fieldIndex, fld.get(), ALTER_DOMAIN_FLAG ) != OGRERR_NONE )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set field domain: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
#else
  ( void )fieldName;
  ( void )schema;
  ( void )tableName;
  ( void )domainName;
  throw QgsProviderConnectionException( QObject::tr( "Setting field domains for datasets requires GDAL 3.3 or later" ) );
#endif
}

void QgsOgrProviderConnection::addFieldDomain( const QgsFieldDomain &domain, const QString &schema ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    if ( OGRFieldDomainH ogrDomain = QgsOgrUtils::convertFieldDomain( &domain ) )
    {
      char *failureReason = nullptr;
      if ( !GDALDatasetAddFieldDomain( hDS.get(), ogrDomain, &failureReason ) )
      {
        OGR_FldDomain_Destroy( ogrDomain );
        const QString error( failureReason );
        CPLFree( failureReason );
        throw QgsProviderConnectionException( QObject::tr( "Could not create field domain: %1" ).arg( error ) );
      }
      CPLFree( failureReason );
      OGR_FldDomain_Destroy( ogrDomain );
    }
    else
    {
      throw QgsProviderConnectionException( QObject::tr( "Could not create field domain" ) );
    }
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  ( void )domain;
  ( void )schema;
  throw QgsProviderConnectionException( QObject::tr( "Creating field domains for datasets requires GDAL 3.3 or later" ) );
#endif
}

void QgsOgrProviderConnection::updateFieldDomain( QgsFieldDomain *domain, const QString &schema ) const
{
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }

  if ( GDALDatasetTestCapability( hDS.get(), ODsCUpdateFieldDomain ) )
  {
    OGRFieldDomainH ogrDomain = QgsOgrUtils::convertFieldDomain( domain );
    if ( !ogrDomain )
    {
      throw QgsProviderConnectionException( QObject::tr( "Could not update field domain" ) );
    }

    char *failureReason = nullptr;
    const bool success = GDALDatasetUpdateFieldDomain( hDS.get(), ogrDomain, &failureReason );

    if ( !success )
    {
      const QString error( failureReason );
      CPLFree( failureReason );
      OGR_FldDomain_Destroy( ogrDomain );
      throw QgsProviderConnectionException( QObject::tr( "Could not update field domain: %1" ).arg( error ) );
    }
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Updating field domains is not supported by the current dataset" ) );
  }
}

void QgsOgrProviderConnection::deleteFieldDomain( const QString &name, const QString &schema ) const
{
  if ( !schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }

  if ( GDALDatasetTestCapability( hDS.get(), ODsCDeleteFieldDomain ) )
  {
    char *failureReason = nullptr;
    const bool success = GDALDatasetDeleteFieldDomain( hDS.get(), name.toUtf8().constData(), &failureReason );

    if ( !success )
    {
      const QString error( failureReason );
      CPLFree( failureReason );
      throw QgsProviderConnectionException( QObject::tr( "Could not delete field domain: %1" ).arg( error ) );
    }

    CPLFree( failureReason );
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Deleting field domains is not supported by the current dataset" ) );
  }
}

void QgsOgrProviderConnection::renameField( const QString &schema, const QString &tableName, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameField );

  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( uri(),
                               true,
                               QStringList(),
                               tableName, errCause, true );
  if ( !layer )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset: %1" ).arg( errCause ) );
  }

  QgsOgrFeatureDefn &fdef = layer->GetLayerDefn();
  const int geomFieldCount = fdef.GetGeomFieldCount();
  for ( int i = 0; i < geomFieldCount; ++ i )
  {
    if ( OGRGeomFieldDefnH geomH = fdef.GetGeomFieldDefn( i ) )
    {
      const QString geometryColumn = QString::fromUtf8( OGR_GFld_GetNameRef( geomH ) );
      if ( name == geometryColumn )
      {
        throw QgsProviderConnectionException( QObject::tr( "Cannot rename geometry columns" ) );
      }
    }
  }

  //type does not matter, it will not be used
  gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( newName.toUtf8().constData(), OFTReal ) );

  const int fieldIndex = layer->GetLayerDefn().GetFieldIndex( name.toUtf8().constData() );
  if ( fieldIndex < 0 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not rename %1 - field does not exist" ).arg( name ) );
  }
  if ( layer->AlterFieldDefn( fieldIndex, fld.get(), ALTER_NAME_FLAG ) != OGRERR_NONE )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not rename field: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
}

void QgsOgrProviderConnection::setFieldAlias( const QString &fieldName, const QString &schema, const QString &tableName, const QString &alias ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( uri(),
                               true,
                               QStringList(),
                               tableName, errCause, true );
  if ( !layer )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset: %1" ).arg( errCause ) );
  }

  //type does not matter, it will not be used
  gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), OFTReal ) );
  OGR_Fld_SetAlternativeName( fld.get(), alias.toUtf8().constData() );

  const int fieldIndex = layer->GetLayerDefn().GetFieldIndex( fieldName.toUtf8().constData() );
  if ( fieldIndex < 0 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set alias for %1 - field does not exist" ).arg( fieldName ) );
  }
  if ( layer->AlterFieldDefn( fieldIndex, fld.get(), ALTER_ALTERNATIVE_NAME_FLAG ) != OGRERR_NONE )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set alias: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
#else
  ( void )fieldName;
  ( void )schema;
  ( void )tableName;
  ( void )alias;
  throw QgsProviderConnectionException( QObject::tr( "Setting field aliases for datasets requires GDAL 3.7 or later" ) );
#endif
}

void QgsOgrProviderConnection::setFieldComment( const QString &fieldName, const QString &schema, const QString &tableName, const QString &comment ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( uri(),
                               true,
                               QStringList(),
                               tableName, errCause, true );
  if ( !layer )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset: %1" ).arg( errCause ) );
  }

  //type does not matter, it will not be used
  gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), OFTReal ) );
  OGR_Fld_SetComment( fld.get(), comment.toUtf8().constData() );

  const int fieldIndex = layer->GetLayerDefn().GetFieldIndex( fieldName.toUtf8().constData() );
  if ( fieldIndex < 0 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set comment for %1 - field does not exist" ).arg( fieldName ) );
  }
  if ( layer->AlterFieldDefn( fieldIndex, fld.get(), ALTER_COMMENT_FLAG ) != OGRERR_NONE )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set comment: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
#else
  ( void )fieldName;
  ( void )schema;
  ( void )tableName;
  ( void )comment;
  throw QgsProviderConnectionException( QObject::tr( "Setting field comments for datasets requires GDAL 3.7 or later" ) );
#endif
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsOgrProviderConnection::sqlOptions( const QString &layerSource )
{
  SqlVectorLayerOptions options;
  QgsProviderMetadata *providerMetadata { QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s ) };
  Q_ASSERT( providerMetadata );
  QMap<QString, QVariant> decoded = providerMetadata->decodeUri( layerSource );

  const QString subset { decoded.value( u"subset"_s, QString() ).toString() };
  const QString layerName { decoded.value( u"layerName"_s, QString() ).toString() };

  if ( !subset.isEmpty() && subset.contains( "SELECT"_L1, Qt::CaseSensitivity::CaseInsensitive ) )
  {
    options.sql = subset;
  }
  else
  {
    if ( !layerName.isEmpty() )
    {
      options.sql = u"SELECT * FROM %1"_s.arg( QgsSqliteUtils::quotedIdentifier( layerName ) );
    }
    else if ( mSingleTableDataset && !decoded.value( u"path"_s, QString() ).toString().isEmpty() )
    {
      const QFileInfo fileInfo( decoded[ u"path"_s ].toString() );
      options.sql = u"SELECT * FROM %1"_s.arg( QgsSqliteUtils::quotedIdentifier( fileInfo.baseName() ) );
    }
    options.filter = subset;
  }
  return options;
}

QList<Qgis::RelationshipCardinality> QgsOgrProviderConnection::supportedRelationshipCardinalities() const
{
  return mSupportedRelationshipCardinality;
}

QList<Qgis::RelationshipStrength> QgsOgrProviderConnection::supportedRelationshipStrengths() const
{
  return mSupportedRelationshipStrength;
}

Qgis::RelationshipCapabilities QgsOgrProviderConnection::supportedRelationshipCapabilities() const
{
  return mRelationshipCapabilities;
}

QStringList QgsOgrProviderConnection::relatedTableTypes() const
{
  return mRelatedTableTypes;
}

QList<QgsWeakRelation> QgsOgrProviderConnection::relationships( const QString &schema, const QString &tableName ) const
{
  checkCapability( Capability::RetrieveRelationships );

  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( u"Schema is not supported by OGR, ignoring"_s, u"OGR"_s, Qgis::MessageLevel::Info );
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    // In some cases (empty geopackage for example), opening in read-only
    // mode fails, so retry in update mode
    hDS.reset( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  }

  if ( hDS )
  {
    QList<QgsWeakRelation> output;

    char **relationNames = GDALDatasetGetRelationshipNames( hDS.get(), nullptr );
    if ( !relationNames )
      return {};

    const QStringList names = QgsOgrUtils::cStringListToQStringList( relationNames );
    CSLDestroy( relationNames );

    for ( const QString &name : names )
    {
      GDALRelationshipH relationship = GDALDatasetGetRelationship( hDS.get(), name.toUtf8().constData() );
      if ( !relationship )
        continue;

      const QString leftTableName( GDALRelationshipGetLeftTableName( relationship ) );
      if ( !tableName.isEmpty() && leftTableName != tableName )
        continue;

      const QString rightTableName( GDALRelationshipGetRightTableName( relationship ) );
      if ( rightTableName.isEmpty() )
        continue;

      output.append( QgsOgrUtils::convertRelationship( relationship, uri() ) );
    }
    return output;
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  Q_UNUSED( tableName )
  throw QgsProviderConnectionException( QObject::tr( "Retrieving relationships for datasets requires GDAL 3.6 or later" ) );
#endif
}

void QgsOgrProviderConnection::addRelationship( const QgsWeakRelation &relationship ) const
{
  checkCapability( Capability::AddRelationship );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    const QVariantMap leftParts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( relationship.referencedLayerSource() );
    const QString leftTableName = leftParts.value( u"layerName"_s ).toString();
    if ( leftTableName.isEmpty() )
      throw QgsProviderConnectionException( QObject::tr( "Parent table name was not set" ) );

    const QVariantMap rightParts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( relationship.referencingLayerSource() );
    const QString rightTableName = rightParts.value( u"layerName"_s ).toString();
    if ( rightTableName.isEmpty() )
      throw QgsProviderConnectionException( QObject::tr( "Child table name was not set" ) );

    QString error;
    gdal::relationship_unique_ptr relationH = QgsOgrUtils::convertRelationship( relationship, error );
    if ( !relationH )
    {
      throw QgsProviderConnectionException( error );
    }

    char *failureReason = nullptr;
    if ( !GDALDatasetAddRelationship( hDS.get(), relationH.get(), &failureReason ) )
    {
      const QString error( failureReason );
      CPLFree( failureReason );
      throw QgsProviderConnectionException( QObject::tr( "Could not create relationship: %1" ).arg( error ) );
    }

    CPLFree( failureReason );
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  Q_UNUSED( relationship )
  throw QgsProviderConnectionException( QObject::tr( "Adding relationships for datasets requires GDAL 3.6 or later" ) );
#endif
}

void QgsOgrProviderConnection::updateRelationship( const QgsWeakRelation &relationship ) const
{
  checkCapability( Capability::UpdateRelationship );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    const QVariantMap leftParts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( relationship.referencedLayerSource() );
    const QString leftTableName = leftParts.value( u"layerName"_s ).toString();
    if ( leftTableName.isEmpty() )
      throw QgsProviderConnectionException( QObject::tr( "Parent table name was not set" ) );

    const QVariantMap rightParts = QgsProviderRegistry::instance()->providerMetadata( u"ogr"_s )->decodeUri( relationship.referencingLayerSource() );
    const QString rightTableName = rightParts.value( u"layerName"_s ).toString();
    if ( rightTableName.isEmpty() )
      throw QgsProviderConnectionException( QObject::tr( "Child table name was not set" ) );

    QString error;
    gdal::relationship_unique_ptr relationH = QgsOgrUtils::convertRelationship( relationship, error );
    if ( !relationH )
    {
      throw QgsProviderConnectionException( error );
    }

    char *failureReason = nullptr;
    if ( !GDALDatasetUpdateRelationship( hDS.get(), relationH.get(), &failureReason ) )
    {
      const QString error( failureReason );
      CPLFree( failureReason );
      throw QgsProviderConnectionException( QObject::tr( "Could not update relationship: %1" ).arg( error ) );
    }

    CPLFree( failureReason );
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  Q_UNUSED( relationship )
  throw QgsProviderConnectionException( QObject::tr( "Updating relationships for datasets requires GDAL 3.6 or later" ) );
#endif
}

void QgsOgrProviderConnection::deleteRelationship( const QgsWeakRelation &relationship ) const
{
  checkCapability( Capability::DeleteRelationship );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  gdal::dataset_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    const QString relationshipName = relationship.name();

    char *failureReason = nullptr;
    if ( !GDALDatasetDeleteRelationship( hDS.get(), relationshipName.toLocal8Bit().constData(), &failureReason ) )
    {
      const QString error( failureReason );
      CPLFree( failureReason );
      throw QgsProviderConnectionException( QObject::tr( "Could not delete relationship: %1" ).arg( error ) );
    }

    CPLFree( failureReason );
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  Q_UNUSED( relationship )
  throw QgsProviderConnectionException( QObject::tr( "Deleting relationships for datasets requires GDAL 3.6 or later" ) );
#endif
}

Qgis::DatabaseProviderTableImportCapabilities QgsOgrProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapabilities();
}

///@endcond
