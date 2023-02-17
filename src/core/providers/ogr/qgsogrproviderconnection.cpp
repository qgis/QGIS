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
#include "qgsogrprovider.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsogrutils.h"
#include "qgsfielddomain.h"
#include "qgsogrproviderutils.h"
#include "qgsdbquerylog.h"
#include "qgsprovidersublayerdetails.h"
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

QgsOgrProviderResultIterator::QgsOgrProviderResultIterator( gdal::ogr_datasource_unique_ptr hDS, OGRLayerH ogrLayer )
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
      // PK
      if ( ! mPrimaryKeyColumnName.isEmpty() )
      {
        row.push_back( OGR_F_GetFID( fet.get() ) );
      }

      if ( ! mFields.isEmpty() )
      {
        QgsFeature f { QgsOgrUtils::readOgrFeature( fet.get(), mFields, QTextCodec::codecForName( "UTF-8" ) ) };
        const QgsAttributes constAttrs  = f.attributes();
        for ( const QVariant &attribute : constAttrs )
        {
          row.push_back( attribute );
        }

        // Geom goes last
        if ( ! mGeometryColumnName.isEmpty( ) )
        {
          row.push_back( f.geometry().asWkt() );
        }

      }
      else // Fallback to strings
      {
        for ( int i = 0; i < OGR_F_GetFieldCount( fet.get() ); i++ )
        {
          row.push_back( QVariant( QString::fromUtf8( OGR_F_GetFieldAsString( fet.get(), i ) ) ) );
        }
      }
    }
    else
    {
      // Release the resources
      GDALDatasetReleaseResultSet( mHDS.get(), mOgrLayer );
      mHDS.release();
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

void QgsOgrProviderResultIterator::setGeometryColumnName( const QString &geometryColumnName )
{
  mGeometryColumnName = geometryColumnName;
}

void QgsOgrProviderResultIterator::setPrimaryKeyColumnName( const QString &primaryKeyColumnName )
{
  mPrimaryKeyColumnName = primaryKeyColumnName;
}

//
// QgsOgrProviderConnection
//

QgsOgrProviderConnection::QgsOgrProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "ogr" );
}

QgsOgrProviderConnection::QgsOgrProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = QStringLiteral( "ogr" );

  // Cleanup the URI in case it contains other information other than the file path
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri );
  if ( !parts.value( QStringLiteral( "path" ) ).toString().isEmpty() && parts.value( QStringLiteral( "path" ) ).toString() != uri )
  {
    setUri( parts.value( QStringLiteral( "path" ) ).toString() );
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
  QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri() );

  if ( !mSingleTableDataset )
    parts.insert( QStringLiteral( "layerName" ), name );

  return QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->encodeUri( parts );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsOgrProviderConnection::tables( const QString &, const TableFlags &flags ) const
{
  QList<QgsAbstractDatabaseProviderConnection::TableProperty> tableInfo;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) );
  const QList< QgsProviderSublayerDetails > subLayers = metadata->querySublayers(
        uri(),
        ( flags & TableFlag::IncludeSystemTables ) ? Qgis::SublayerQueryFlag::IncludeSystemTables : Qgis::SublayerQueryFlags(),
        nullptr );

  tableInfo.reserve( subLayers.size() );
  for ( const QgsProviderSublayerDetails &subLayer : subLayers )
  {
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

QgsAbstractDatabaseProviderConnection::TableProperty QgsOgrProviderConnection::table( const QString &, const QString &table ) const
{
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri() );
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();

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

  if ( geomType.wkbType != QgsWkbTypes::NoGeometry )
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
  QgsProviderMetadata *providerMetadata { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  Q_ASSERT( providerMetadata );
  QMap<QString, QVariant> decoded = providerMetadata->decodeUri( uri() );
  decoded[ QStringLiteral( "subset" ) ] = options.sql;
  return new QgsVectorLayer( providerMetadata->encodeUri( decoded ), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey() );
}

void QgsOgrProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

  GDALDriverH hDriver = GDALIdentifyDriverEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve driver for connection" ) );
  }

  QMap<QString, QVariant> opts { *options };
  opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
  opts[ QStringLiteral( "update" ) ] = true;
  opts[ QStringLiteral( "driverName" ) ] = QString( GDALGetDriverShortName( hDriver ) );
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult errCode = QgsOgrProvider::createEmptyLayer(
                                       uri(),
                                       fields,
                                       wkbType,
                                       srs,
                                       overwrite,
                                       &map,
                                       &errCause,
                                       &opts
                                     );
  if ( errCode != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

void QgsOgrProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
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

  if ( mDriverName == QLatin1String( "OpenFileGDB" ) )
  {
    if ( !name.isEmpty() )
      executeGdalSqlPrivate( QStringLiteral( "REPACK \"%1\"" ).arg( name ) );
    else
      executeGdalSqlPrivate( QStringLiteral( "REPACK" ) );
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
  mSingleTableDataset = GDALGetMetadataItem( hDriver, GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, nullptr ) == nullptr;
#else
  {
    const QVariantMap uriParts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri() );
    const QFileInfo pathInfo( uriParts.value( QStringLiteral( "path" ) ).toString() );
    const QString suffix = uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString().isEmpty()
                           ? pathInfo.suffix().toLower()
                           : QFileInfo( uriParts.value( QStringLiteral( "vsiSuffix" ) ).toString() ).suffix().toLower();
    mSingleTableDataset = !QgsGdalUtils::multiLayerFileExtensions().contains( suffix );
  }
#endif

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  if ( mDriverName == QLatin1String( "OpenFileGDB" ) )
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
    CSLDestroy( papszTokens );
  }
#else
  // Prior to GDAL 3.6 there was no generic way to test these per driver/dataset
  mCapabilities |= AddField;
  mCapabilities |= DeleteField;
  mCapabilities |= RenameField;
#endif

  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( !hDS )
  {
    // fallback to read only otherwise
    hDS.reset( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  }

  if ( hDS )
  {
    if ( OGR_DS_TestCapability( hDS.get(), ODsCCurveGeometries ) )
      mGeometryColumnCapabilities |= GeometryColumnCapability::Curves;

    if ( OGR_DS_TestCapability( hDS.get(), ODsCMeasuredGeometries ) )
      mGeometryColumnCapabilities |= GeometryColumnCapability::M;

    if ( !mSingleTableDataset )
    {
      if ( OGR_DS_TestCapability( hDS.get(), ODsCCreateLayer ) )
        mCapabilities |= CreateVectorTable;

      if ( OGR_DS_TestCapability( hDS.get(), ODsCDeleteLayer ) )
        mCapabilities |= DropVectorTable;
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
  if ( mDriverName == QLatin1String( "OpenFileGDB" ) || mDriverName == QLatin1String( "FileGDB" ) )
  {
    mIllegalFieldNames =
    {
      QStringLiteral( "ADD" ),
      QStringLiteral( "ALTER" ),
      QStringLiteral( "AND" ),
      QStringLiteral( "BETWEEN" ),
      QStringLiteral( "BY" ),
      QStringLiteral( "COLUMN" ),
      QStringLiteral( "CREATE" ),
      QStringLiteral( "DELETE" ),
      QStringLiteral( "DROP" ),
      QStringLiteral( "EXISTS" ),
      QStringLiteral( "FOR" ),
      QStringLiteral( "FROM" ),
      QStringLiteral( "GROUP" ),
      QStringLiteral( "IN" ),
      QStringLiteral( "INSERT" ),
      QStringLiteral( "INTO" ),
      QStringLiteral( "IS" ),
      QStringLiteral( "LIKE" ),
      QStringLiteral( "NOT" ),
      QStringLiteral( "NULL" ),
      QStringLiteral( "OR" ),
      QStringLiteral( "ORDER" ),
      QStringLiteral( "SELECT" ),
      QStringLiteral( "SET" ),
      QStringLiteral( "TABLE" ),
      QStringLiteral( "UPDATE" ),
      QStringLiteral( "VALUES" ),
      QStringLiteral( "WHERE" )
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
  if ( mDriverName == QLatin1String( "OpenFileGDB" ) )
  {
    mRelatedTableTypes = QStringList
    {
      QStringLiteral( "media" ),
      QStringLiteral( "features" )
    };
  }
  else if ( mDriverName == QLatin1String( "GPKG" ) )
  {
    mRelatedTableTypes = QStringList
    {
      QStringLiteral( "media" ),
      QStringLiteral( "simple_attributes" ),
      QStringLiteral( "features" ),
      QStringLiteral( "attributes" ),
      QStringLiteral( "tiles" )
    };
  }
#endif
}

QString QgsOgrProviderConnection::databaseQueryLogIdentifier() const
{
  return QStringLiteral( "QgsOgrProviderConnection" );
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
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
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

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS.get(), sql.toUtf8().constData(), nullptr, nullptr ) );
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
        // pk column name
        QString pkColumnName;

        QgsFields fields { QgsOgrUtils::readOgrFields( fet.get(), QTextCodec::codecForName( "UTF-8" ) ) };

        // We try to guess the table name from the FROM clause
        thread_local const QRegularExpression tableNameRegexp { QStringLiteral( R"re((?<=from|join)\s+(\w+)|"([^"]+)")re" ), QRegularExpression::PatternOption::CaseInsensitiveOption };
        const auto match { tableNameRegexp.match( sql ) };
        if ( match.hasMatch() )
        {
          pkColumnName = primaryKeyColumnName( match.captured( match.lastCapturedIndex() ) );
        }

        // fallback to "fid"
        if ( pkColumnName.isEmpty() )
        {
          pkColumnName = QStringLiteral( "fid" );
        }

        // geom column name
        QString geomColumnName;

        OGRFeatureDefnH featureDef = OGR_F_GetDefnRef( fet.get() );

        if ( featureDef )
        {
          if ( OGR_F_GetGeomFieldCount( fet.get() ) > 0 )
          {
            OGRGeomFieldDefnH geomFldDef { OGR_F_GetGeomFieldDefnRef( fet.get(), 0 ) };
            if ( geomFldDef )
            {
              geomColumnName = OGR_GFld_GetNameRef( geomFldDef );
            }
          }
        }

        // May need to prepend PK and append geometry to the columns
        if ( ! pkColumnName.isEmpty() )
        {
          const QRegularExpression pkRegExp { QStringLiteral( R"(^select\s+(\*|%1)[,\s+](.*)from)" ).arg( pkColumnName ),  QRegularExpression::PatternOption::CaseInsensitiveOption };
          if ( pkRegExp.match( sql.trimmed() ).hasMatch() )
          {
            iterator->setPrimaryKeyColumnName( pkColumnName );
            results.appendColumn( pkColumnName );
          }
        }

        // Add other fields
        for ( const auto &f : std::as_const( fields ) )
        {
          results.appendColumn( f.name() );
        }

        // Append geom
        if ( ! geomColumnName.isEmpty() )
        {
          results.appendColumn( geomColumnName );
          iterator->setGeometryColumnName( geomColumnName );
        }

        iterator->setFields( fields );
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
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
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
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
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
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
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
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
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

void QgsOgrProviderConnection::renameField( const QString &schema, const QString &tableName, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameField );

  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
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

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsOgrProviderConnection::sqlOptions( const QString &layerSource )
{
  SqlVectorLayerOptions options;
  QgsProviderMetadata *providerMetadata { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) };
  Q_ASSERT( providerMetadata );
  QMap<QString, QVariant> decoded = providerMetadata->decodeUri( layerSource );
  if ( decoded.contains( QStringLiteral( "subset" ) ) )
  {
    options.sql = decoded[ QStringLiteral( "subset" ) ].toString();
  }
  else if ( decoded.contains( QStringLiteral( "layerName" ) ) )
  {
    options.sql = QStringLiteral( "SELECT * FROM %1" ).arg( QgsSqliteUtils::quotedIdentifier( decoded[ QStringLiteral( "layerName" ) ].toString() ) );
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
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
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
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    const QVariantMap leftParts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( relationship.referencedLayerSource() );
    const QString leftTableName = leftParts.value( QStringLiteral( "layerName" ) ).toString();
    if ( leftTableName.isEmpty() )
      throw QgsProviderConnectionException( QObject::tr( "Parent table name was not set" ) );

    const QVariantMap rightParts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( relationship.referencingLayerSource() );
    const QString rightTableName = rightParts.value( QStringLiteral( "layerName" ) ).toString();
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
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    const QVariantMap leftParts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( relationship.referencedLayerSource() );
    const QString leftTableName = leftParts.value( QStringLiteral( "layerName" ) ).toString();
    if ( leftTableName.isEmpty() )
      throw QgsProviderConnectionException( QObject::tr( "Parent table name was not set" ) );

    const QVariantMap rightParts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( relationship.referencingLayerSource() );
    const QString rightTableName = rightParts.value( QStringLiteral( "layerName" ) ).toString();
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
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_UPDATE | GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
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

///@endcond
