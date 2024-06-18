/***************************************************************************
           qgsspatialiteprovider.cpp Data provider for SpatiaLite DBMS
begin                : Dec, 2008
copyright            : (C) 2008 Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnpool.h"
#include "qgsspatialitefeatureiterator.h"
#include "qgsfeedback.h"
#include "qgsspatialitedataitems.h"
#include "qgsspatialiteconnection.h"
#include "qgsspatialitetransaction.h"
#include "qgsspatialiteproviderconnection.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"

#include "qgsjsonutils.h"
#include "qgsvectorlayer.h"

#include "qgsprovidermetadata.h"

#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

#include <nlohmann/json.hpp>
using namespace nlohmann;


const QString QgsSpatiaLiteProvider::SPATIALITE_KEY = QStringLiteral( "spatialite" );
const QString QgsSpatiaLiteProvider::SPATIALITE_DESCRIPTION = QStringLiteral( "SpatiaLite data provider" );
QAtomicInt QgsSpatiaLiteProvider::sSavepointId = 0;

bool QgsSpatiaLiteProvider::convertField( QgsField &field )
{
  QString fieldType = QStringLiteral( "TEXT" ); //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QMetaType::Type::LongLong:
      fieldType = QStringLiteral( "BIGINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QMetaType::Type::QDateTime:
      fieldType = QStringLiteral( "TIMESTAMP" );
      fieldSize = -1;
      break;

    case QMetaType::Type::QDate:
      fieldType = QStringLiteral( "DATE" );
      fieldSize = -1;
      break;

    case QMetaType::Type::QTime:
    case QMetaType::Type::QString:
      fieldType = QStringLiteral( "TEXT" );
      fieldPrec = 0;
      break;

    case QMetaType::Type::Int:
      fieldType = QStringLiteral( "INTEGER" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QMetaType::Type::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = QStringLiteral( "REAL" );
        fieldSize = -1;
        fieldPrec = 0;
      }
      else
      {
        fieldType = QStringLiteral( "NUMERIC" );
      }
      break;

    case QMetaType::Type::QVariantList:
    case QMetaType::Type::QStringList:
    {
      QgsField subField = field;
      subField.setType( field.subType() );
      subField.setSubType( QMetaType::Type::UnknownType );
      if ( !convertField( subField ) ) return false;
      fieldType = QgsSpatiaLiteConnection::SPATIALITE_ARRAY_PREFIX + subField.typeName() + QgsSpatiaLiteConnection::SPATIALITE_ARRAY_SUFFIX;
      fieldSize = subField.length();
      fieldPrec = subField.precision();
      break;
    }

    case QMetaType::Type::QByteArray:
      fieldType = QStringLiteral( "BLOB" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}



Qgis::VectorExportResult QgsSpatiaLiteProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    Qgis::WkbType wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options )

  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QgsDebugMsgLevel( "Database is: " + sqlitePath, 2 );
  QgsDebugMsgLevel( "Table name is: " + tableName, 2 );
  QgsDebugMsgLevel( "Geometry column is: " + geometryColumn, 2 );

  // create the table
  {
    char *errMsg = nullptr;
    int toCommit = false;
    QString sql;

    // trying to open the SQLite DB
    QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
    if ( !handle )
    {
      QgsDebugError( QStringLiteral( "Connection to database failed. Import of layer aborted." ) );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Connection to database failed" );
      return Qgis::VectorExportResult::ErrorConnectionFailed;
    }

    sqlite3 *sqliteHandle = handle->handle();

    // get the pk's name and type
    if ( primaryKey.isEmpty() )
    {
      // if no pk name was passed, define the new pk field name
      int index = 0;
      QString pk = primaryKey = QStringLiteral( "pk" );
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        if ( fields.at( fldIdx ).name() == primaryKey )
        {
          // it already exists, try again with a new name
          primaryKey = QStringLiteral( "%1_%2" ).arg( pk ).arg( index++ );
          fldIdx = -1; // it is incremented in the for loop, i.e. restarts at 0
        }
      }
    }
    else
    {
      // search for the passed field
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        if ( fields.at( fldIdx ).name() == primaryKey )
        {
          // found it, get the field type
          QgsField fld = fields.at( fldIdx );
          if ( ( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) || convertField( fld ) )
          {
            primaryKeyType = fld.typeName();
          }
        }
      }
    }

    // if the pk field doesn't exist yet, create an integer pk field
    // as it's autoincremental
    if ( primaryKeyType.isEmpty() )
    {
      primaryKeyType = QStringLiteral( "INTEGER" );
    }
    else
    {
      // if the pk field's type is bigint, use the autoincremental
      // integer type instead
      if ( primaryKeyType == QLatin1String( "BIGINT" ) )
      {
        primaryKeyType = QStringLiteral( "INTEGER" );
      }
    }

    try
    {
      int ret = exec_sql( sqliteHandle,  "BEGIN", uri, errMsg, QGS_QUERY_LOG_ORIGIN );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

      toCommit = true;

      if ( overwrite )
      {
        // delete the table if exists and the related entry in geometry_columns, then re-create it
        sql = QStringLiteral( "DROP TABLE IF EXISTS %1" )
              .arg( QgsSqliteUtils::quotedIdentifier( tableName ) );

        ret = exec_sql( sqliteHandle,  sql.toUtf8().constData(), uri, errMsg, QGS_QUERY_LOG_ORIGIN );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );

        sql = QStringLiteral( "DELETE FROM geometry_columns WHERE upper(f_table_name) = upper(%1)" )
              .arg( QgsSqliteUtils::quotedString( tableName ) );

        ret = exec_sql( sqliteHandle,  sql.toUtf8().constData(), uri, errMsg, QGS_QUERY_LOG_ORIGIN );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );
      }

      sql = QStringLiteral( "CREATE TABLE %1 (%2 %3 PRIMARY KEY%4)" )
            .arg( QgsSqliteUtils::quotedIdentifier( tableName ),
                  QgsSqliteUtils::quotedIdentifier( primaryKey ),
                  primaryKeyType,
                  primaryKeyType == QLatin1String( "INTEGER" ) ? QStringLiteral( " AUTOINCREMENT" ) : QString() );

      ret = exec_sql( sqliteHandle,  sql.toUtf8().constData(), uri, errMsg, QGS_QUERY_LOG_ORIGIN );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

      // get geometry type, dim and srid
      int dim = 2;
      long srid = srs.postgisSrid();

      switch ( wkbType )
      {
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::PointZ:
          dim = 3;
          [[fallthrough]];
        case Qgis::WkbType::Point:
          geometryType = QStringLiteral( "POINT" );
          break;

        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::LineStringZ:
          dim = 3;
          [[fallthrough]];
        case Qgis::WkbType::LineString:
          geometryType = QStringLiteral( "LINESTRING" );
          break;

        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::PolygonZ:
          dim = 3;
          [[fallthrough]];
        case Qgis::WkbType::Polygon:
          geometryType = QStringLiteral( "POLYGON" );
          break;

        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiPointZ:
          dim = 3;
          [[fallthrough]];
        case Qgis::WkbType::MultiPoint:
          geometryType = QStringLiteral( "MULTIPOINT" );
          break;

        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiLineStringZ:
          dim = 3;
          [[fallthrough]];
        case Qgis::WkbType::MultiLineString:
          geometryType = QStringLiteral( "MULTILINESTRING" );
          break;

        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::MultiPolygonZ:
          dim = 3;
          [[fallthrough]];
        case Qgis::WkbType::MultiPolygon:
          geometryType = QStringLiteral( "MULTIPOLYGON" );
          break;

        case Qgis::WkbType::Unknown:
          geometryType = QStringLiteral( "GEOMETRY" );
          break;

        case Qgis::WkbType::NoGeometry:
        default:
          dim = 0;
          break;
      }

      // create geometry column
      if ( !geometryType.isEmpty() )
      {
        sql = QStringLiteral( "SELECT AddGeometryColumn(%1, %2, %3, %4, %5)" )
              .arg( QgsSqliteUtils::quotedString( tableName ),
                    QgsSqliteUtils::quotedString( geometryColumn ) )
              .arg( srid )
              .arg( QgsSqliteUtils::quotedString( geometryType ) )
              .arg( dim );

        ret = exec_sql( sqliteHandle,  sql.toUtf8().constData(), uri, errMsg, QGS_QUERY_LOG_ORIGIN );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );
      }
      else
      {
        geometryColumn = QString();
      }

      ret = exec_sql( sqliteHandle,  "COMMIT", uri, errMsg, QGS_QUERY_LOG_ORIGIN );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

    }
    catch ( SLException &e )
    {
      QgsDebugError( QStringLiteral( "creation of data source %1 failed. %2" )
                     .arg( tableName,
                           e.errorMessage() )
                   );

      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of data source %1 failed. %2" )
                        .arg( tableName,
                              e.errorMessage() );


      if ( toCommit )
      {
        // ROLLBACK after some previous error
        exec_sql( sqliteHandle,  "ROLLBACK", uri, nullptr, QGS_QUERY_LOG_ORIGIN );
      }

      QgsSqliteHandle::closeDb( handle );
      return Qgis::VectorExportResult::ErrorCreatingLayer;
    }

    QgsSqliteHandle::closeDb( handle );
    QgsDebugMsgLevel( "layer " + tableName  + " created.", 2 );
  }

  // use the provider to edit the table
  dsUri.setDataSource( QString(), tableName, geometryColumn, QString(), primaryKey );

  QgsDataProvider::ProviderOptions providerOptions;
  QgsSpatiaLiteProvider *provider = new QgsSpatiaLiteProvider( dsUri.uri(), providerOptions );
  if ( !provider->isValid() )
  {
    QgsDebugError( "The layer " + tableName + " just created is not valid or not supported by the provider." );
    if ( errorMessage )
      *errorMessage = QObject::tr( "loading of the layer %1 failed" )
                      .arg( tableName );

    delete provider;
    return Qgis::VectorExportResult::ErrorInvalidLayer;
  }

  QgsDebugMsgLevel( QStringLiteral( "layer loaded" ), 2 );

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = 1;

    // get the list of fields
    QList<QgsField> flist;
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      QgsField fld = fields.at( fldIdx );
      if ( fld.name() == primaryKey )
        continue;

      if ( fld.name() == geometryColumn )
      {
        QgsDebugError( QStringLiteral( "Found a field with the same name of the geometry column. Skip it!" ) );
        continue;
      }

      if ( !( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) && !convertField( fld ) )
      {
        QgsDebugError( "error creating field " + fld.name() + ": unsupported type" );
        if ( errorMessage )
          *errorMessage = QObject::tr( "unsupported type for field %1" )
                          .arg( fld.name() );

        delete provider;
        return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
      }

      QgsDebugMsgLevel( "creating field #" + QString::number( fldIdx ) +
                        " -> #" + QString::number( offset ) +
                        " name " + fld.name() +
                        " type " + QString( QVariant::typeToName( fld.type() ) ) +
                        " typename " + fld.typeName() +
                        " width " + QString::number( fld.length() ) +
                        " precision " + QString::number( fld.precision() ), 2 );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIdx, offset );

      offset++;
    }

    if ( !provider->addAttributes( flist ) )
    {
      QgsDebugError( QStringLiteral( "error creating fields " ) );
      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of fields failed" );

      delete provider;
      return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
    }

    QgsDebugMsgLevel( QStringLiteral( "Done creating fields" ), 2 );
  }
  return Qgis::VectorExportResult::Success;
}


QgsSpatiaLiteProvider::QgsSpatiaLiteProvider( QString const &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  nDims = GAIA_XY;
  QgsDataSourceUri anUri = QgsDataSourceUri( uri );

  // parsing members from the uri structure
  mTableName = anUri.table();
  mGeometryColumn = anUri.geometryColumn().toLower();
  mSqlitePath = anUri.database();
  mSubsetString = anUri.sql();
  mPrimaryKey = anUri.keyColumn();
  mQuery = mTableName;

  // Retrieve a shared connection
  mHandle = QgsSqliteHandle::openDb( mSqlitePath );

  if ( !mHandle )
  {
    return;
  }

  // Setup handle
  mSqliteHandle = mHandle->handle();
  if ( mSqliteHandle )
  {
    const QStringList pragmaList = anUri.params( QStringLiteral( "pragma" ) );
    for ( const auto &pragma : pragmaList )
    {
      char *errMsg = nullptr;
      int ret = exec_sql( mSqliteHandle, QStringLiteral( "PRAGMA %1" ).arg( pragma ), uri, errMsg );
      if ( ret != SQLITE_OK )
      {
        QgsDebugError( QStringLiteral( "PRAGMA " ) + pragma + QString( " failed : %1" ).arg( errMsg ? errMsg : "" ) );
      }
      sqlite3_free( errMsg );
    }
  }

  bool ret = false;

  gaiaVectorLayersListPtr list = nullptr;
  gaiaVectorLayerPtr lyr = nullptr;

  // Set special cases (views, queries, no geometry specified)
  bool specialCase { mGeometryColumn.isEmpty() ||
                     ( mQuery.startsWith( '(' ) &&mQuery.endsWith( ')' ) ) };

  // Normal case
  if ( ! specialCase )
  {
    // Set pk to ROWID in case the pk passed in the URL is not usable
    if ( mPrimaryKey.isEmpty() || ! tablePrimaryKeys( mTableName ).contains( mPrimaryKey ) )
    {
      mPrimaryKey = QStringLiteral( "ROWID" );
    }
    // using v.4.0 Abstract Interface
    ret = true;
    list = gaiaGetVectorLayersList( mSqliteHandle,
                                    mTableName.toUtf8().constData(),
                                    mGeometryColumn.toUtf8().constData(),
                                    GAIA_VECTORS_LIST_OPTIMISTIC );
    if ( list )
      lyr = list->First;

    ret = lyr && checkLayerTypeAbstractInterface( lyr );
    QgsDebugMsgLevel( QStringLiteral( "Using checkLayerTypeAbstractInterface" ), 2 );
  }
  else  // views, no geometry etc
  {
    ret = checkLayerType();
  }

  if ( !ret )
  {
    // invalid metadata
    mNumberFeatures = 0;

    QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
    closeDb();
    return;
  }

  // TODO: move after pk discovery is definitely done?
  mEnabledCapabilities = mPrimaryKey.isEmpty() ? QgsVectorDataProvider::Capabilities() : ( QgsVectorDataProvider::SelectAtId );
  if ( ( mTableBased || mViewBased ) &&  !mReadOnly )
  {
    // enabling editing only for Tables [excluding Views and VirtualShapes]
    mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::FastTruncate;
    if ( !mGeometryColumn.isEmpty() )
      mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
    mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
    mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
    mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes;
    mEnabledCapabilities |= QgsVectorDataProvider::CreateAttributeIndex;
    mEnabledCapabilities |= QgsVectorDataProvider::TransactionSupport;
  }

  if ( lyr )
  {
    // using the v.4.0 AbstractInterface
    if ( !getGeometryDetailsAbstractInterface( lyr ) )  // gets srid and geometry type
    {
      // the table is not a geometry table
      mNumberFeatures = 0;
      QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
      closeDb();
      gaiaFreeVectorLayersList( list );
      return;
    }

    // if DB has Z geometry we do NOT use the v.4.0 AbstractInterface as it does not retrieve Z extent data
    if ( lyr->GeometryType == GAIA_XY_Z || lyr->GeometryType == GAIA_XY_Z_M )
    {
      if ( !getTableSummary() )     // gets the extent and feature count
      {
        mNumberFeatures = 0;
        QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
        closeDb();
        return;
      }
    }
    else
    {
      if ( !getTableSummaryAbstractInterface( lyr ) )     // gets the extent and feature count
      {
        mNumberFeatures = 0;
        QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
        closeDb();
        gaiaFreeVectorLayersList( list );
        return;
      }
    }
    // load the columns list
    loadFieldsAbstractInterface( lyr );
    gaiaFreeVectorLayersList( list );
  }

  else // no v.4.0 AbstractInterface
  {
    // using the traditional methods
    if ( !getGeometryDetails() )  // gets srid and geometry type
    {
      // the table is not a geometry table
      mNumberFeatures = 0;
      QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
      closeDb();
      return;
    }
    if ( !getTableSummary() )     // gets the extent and feature count
    {
      mNumberFeatures = 0;
      QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
      closeDb();
      return;
    }
    // load the columns list
    loadFields();
  }

  elevationProperties()->setContainsElevationData( nDims == GAIA_XY_Z || nDims == GAIA_XY_Z_M );

  if ( !mSqliteHandle )
  {
    QgsDebugError( QStringLiteral( "Invalid SpatiaLite layer" ) );
    return;
  }

  // Fallback to ROWID is pk is empty or not usable after fields configuration
  if ( mTableBased && hasRowid() && ( mPrimaryKey.isEmpty() || ! tablePrimaryKeys( mTableName ).contains( mPrimaryKey ) ) )
  {
    mPrimaryKey = QStringLiteral( "ROWID" );
  }

  // retrieve version information
  spatialiteVersion();

  //fill type names into sets
  setNativeTypes( QgsSpatiaLiteConnection::nativeTypes() );

  // Update extent and feature count
  if ( ! mSubsetString.isEmpty() )
    getTableSummary();

  mValid = true;
}

QgsSpatiaLiteProvider::~QgsSpatiaLiteProvider()
{
  if ( mTransaction )
  {
    QString errorMessage;
    if ( ! mTransaction->rollback( errorMessage ) )
    {
      QgsMessageLog::logMessage( tr( "Error closing transaction for %1" ).arg( mTableName ), tr( "SpatiaLite" ) );
    }
  }
  closeDb();
  invalidateConnections( mSqlitePath );
}

Qgis::DataProviderFlags QgsSpatiaLiteProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D | Qgis::DataProviderFlag::FastExtent3D;
}

QgsAbstractFeatureSource *QgsSpatiaLiteProvider::featureSource() const
{
  return new QgsSpatiaLiteFeatureSource( this );
}

void QgsSpatiaLiteProvider::updatePrimaryKeyCapabilities()
{
  if ( mPrimaryKey.isEmpty() )
  {
    mEnabledCapabilities &= ~QgsVectorDataProvider::SelectAtId;
  }
  else
  {
    mEnabledCapabilities |= QgsVectorDataProvider::SelectAtId;
  }
}

typedef QPair<QMetaType::Type, QMetaType::Type> TypeSubType;

static TypeSubType getVariantType( const QString &type )
{
  // making some assumptions in order to guess a more realistic type
  if ( type == QLatin1String( "int" ) ||
       type == QLatin1String( "integer" ) ||
       type == QLatin1String( "integer64" ) ||
       type == QLatin1String( "bigint" ) ||
       type == QLatin1String( "smallint" ) ||
       type == QLatin1String( "tinyint" ) ||
       type == QLatin1String( "boolean" ) )
    return TypeSubType( QMetaType::Type::LongLong, QMetaType::Type::UnknownType );
  else if ( type == QLatin1String( "real" ) ||
            type == QLatin1String( "double" ) ||
            type == QLatin1String( "double precision" ) ||
            type == QLatin1String( "float" ) )
    return TypeSubType( QMetaType::Type::Double, QMetaType::Type::UnknownType );
  else if ( type.startsWith( QgsSpatiaLiteConnection::SPATIALITE_ARRAY_PREFIX ) && type.endsWith( QgsSpatiaLiteConnection::SPATIALITE_ARRAY_SUFFIX ) )
  {
    // New versions of OGR convert list types (StringList, IntegerList, Integer64List and RealList)
    // to JSON when it stores a Spatialite table. It sets the column type as JSONSTRINGLIST,
    // JSONINTEGERLIST, JSONINTEGER64LIST or JSONREALLIST
    TypeSubType subType = getVariantType( type.mid( QgsSpatiaLiteConnection::SPATIALITE_ARRAY_PREFIX.length(),
                                          type.length() - QgsSpatiaLiteConnection::SPATIALITE_ARRAY_PREFIX.length() - QgsSpatiaLiteConnection::SPATIALITE_ARRAY_SUFFIX.length() ) );
    return TypeSubType( subType.first == QMetaType::Type::QString ? QMetaType::Type::QStringList : QMetaType::Type::QVariantList, subType.first );
  }
  else if ( type == QLatin1String( "jsonarray" ) )
  {
    return TypeSubType( QMetaType::Type::QVariantList, QMetaType::Type::UnknownType );
  }
  else if ( type == QLatin1String( "blob" ) )
  {
    return TypeSubType( QMetaType::Type::QByteArray, QMetaType::Type::UnknownType );
  }
  else if ( type == QLatin1String( "timestamp" ) ||
            type == QLatin1String( "datetime" ) )
  {
    return  TypeSubType( QMetaType::Type::QDateTime, QMetaType::Type::UnknownType );
  }
  else if ( type == QLatin1String( "date" ) )
  {
    return  TypeSubType( QMetaType::Type::QDate, QMetaType::Type::UnknownType );
  }
  else
    // for sure any SQLite value can be represented as SQLITE_TEXT
    return TypeSubType( QMetaType::Type::QString, QMetaType::Type::UnknownType );
}

void QgsSpatiaLiteProvider::loadFieldsAbstractInterface( gaiaVectorLayerPtr lyr )
{
  if ( !lyr )
    return;

  mAttributeFields.clear();
  mPrimaryKey.clear();
  mPrimaryKeyAttrs.clear();
  mDefaultValues.clear();

  gaiaLayerAttributeFieldPtr fld = lyr->First;
  if ( !fld )
  {
    // defaulting to traditional loadFields()
    loadFields();
    return;
  }

  while ( fld )
  {
    QString name = QString::fromUtf8( fld->AttributeFieldName );
    if ( name.toLower() != mGeometryColumn )
    {
      const char *type = "TEXT";
      QMetaType::Type fieldType = QMetaType::Type::QString; // default: SQLITE_TEXT
      if ( fld->IntegerValuesCount != 0 && fld->DoubleValuesCount == 0 &&
           fld->TextValuesCount == 0 && fld->BlobValuesCount == 0 )
      {
        fieldType = QMetaType::Type::LongLong;
        type = "INTEGER";
      }
      if ( fld->DoubleValuesCount != 0 && fld->TextValuesCount == 0 &&
           fld->BlobValuesCount == 0 )
      {
        fieldType = QMetaType::Type::Double;
        type = "DOUBLE";
      }
      if ( fld->BlobValuesCount != 0 )
      {
        fieldType = QMetaType::Type::QByteArray;
        type = "BLOB";
      }
      mAttributeFields.append( QgsField( name, fieldType, type, 0, 0, QString() ) );
    }
    fld = fld->Next;
  }

  QString sql = QStringLiteral( "PRAGMA table_info(%1)" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret == SQLITE_OK )
  {
    for ( int i = 1; i <= rows; i++ )
    {
      QString name = QString::fromUtf8( results[( i * columns ) + 1] );

      if ( name.compare( mGeometryColumn, Qt::CaseInsensitive ) == 0 )
        continue;

      const QString defaultValue = QString::fromUtf8( results[( i * columns ) + 4] );
      const int realFieldIndex = mAttributeFields.lookupField( name );
      if ( realFieldIndex >= 0 )
      {
        insertDefaultValue( realFieldIndex, defaultValue );
      }

      QString pk = results[( i * columns ) + 5];
      QString type = results[( i * columns ) + 2];
      type = type.toLower();

      const int fieldIndex = mAttributeFields.indexFromName( name );
      if ( fieldIndex >= 0 )
      {
        // set the actual type name, as given by sqlite
        QgsField &field = mAttributeFields[fieldIndex];
        field.setTypeName( type );
        // TODO: column 4 tells us if the field is nullable. Should use that info...
        if ( field.type() == QMetaType::Type::QString )
        {
          // if the type seems unknown, fix it with what we actually have
          TypeSubType typeSubType = getVariantType( type );
          field.setType( typeSubType.first );
          field.setSubType( typeSubType.second );
        }
      }

      if ( pk.toInt() == 0 || ( type.compare( QLatin1String( "integer" ), Qt::CaseSensitivity::CaseInsensitive ) != 0 &&
                                type.compare( QLatin1String( "bigint" ), Qt::CaseSensitivity::CaseInsensitive ) != 0 ) )
        continue;

      if ( mPrimaryKeyAttrs.isEmpty() )
        mPrimaryKey = name;
      else
        mPrimaryKey.clear();
      mPrimaryKeyAttrs << i - 1;
    }
  }

  // check for constraints
  fetchConstraints();

  // for views try to get the primary key from the meta table
  if ( mViewBased && mPrimaryKey.isEmpty() )
  {
    determineViewPrimaryKey();
  }

  updatePrimaryKeyCapabilities();

  sqlite3_free_table( results );
}

QString QgsSpatiaLiteProvider::spatialiteVersion()
{
  if ( mGotSpatialiteVersion )
    return mSpatialiteVersionInfo;

  int ret;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString sql;

  sql = QStringLiteral( "SELECT spatialite_version()" );
  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK || rows != 1 )
  {
    QgsMessageLog::logMessage( tr( "Retrieval of spatialite version failed" ), tr( "SpatiaLite" ) );
    return QString();
  }

  mSpatialiteVersionInfo = QString::fromUtf8( results[( 1 * columns ) + 0] );
  sqlite3_free_table( results );

  QgsDebugMsgLevel( "SpatiaLite version info: " + mSpatialiteVersionInfo, 2 );

  QStringList spatialiteParts = mSpatialiteVersionInfo.split( ' ', Qt::SkipEmptyParts );

  // Get major and minor version
  QStringList spatialiteVersionParts = spatialiteParts[0].split( '.', Qt::SkipEmptyParts );
  if ( spatialiteVersionParts.size() < 2 )
  {
    QgsMessageLog::logMessage( tr( "Could not parse spatialite version string '%1'" ).arg( mSpatialiteVersionInfo ), tr( "SpatiaLite" ) );
    return QString();
  }

  mSpatialiteVersionMajor = spatialiteVersionParts[0].toInt();
  mSpatialiteVersionMinor = spatialiteVersionParts[1].toInt();

  mGotSpatialiteVersion = true;
  return mSpatialiteVersionInfo;
}

bool QgsSpatiaLiteProvider::versionIsAbove( sqlite3 *sqlite_handle, int major, int minor )
{
  char **results = nullptr;
  char *errMsg = nullptr;
  int rows, columns;
  bool above = false;
  int ret = sqlite3_get_table( sqlite_handle, "select spatialite_version()", &results, &rows, &columns, nullptr );
  if ( ret == SQLITE_OK )
  {
    if ( rows == 1 && columns == 1 )
    {
      QString version = QString::fromUtf8( results[1] );
      QStringList parts = version.split( ' ', Qt::SkipEmptyParts );
      if ( !parts.empty() )
      {
        QStringList verparts = parts.at( 0 ).split( '.', Qt::SkipEmptyParts );
        above = verparts.size() >= 2 && ( verparts.at( 0 ).toInt() > major || ( verparts.at( 0 ).toInt() == major && verparts.at( 1 ).toInt() >= minor ) );
      }
    }
    sqlite3_free_table( results );
  }
  else
  {
    QgsLogger::warning( QStringLiteral( "SQLite error querying version: %1" ).arg( errMsg ) );
    sqlite3_free( errMsg );
  }
  return above;
}

QString QgsSpatiaLiteProvider::tableSchemaCondition( const QgsDataSourceUri &dsUri )
{
  return dsUri.schema().isEmpty() ?
         QStringLiteral( "IS NULL" ) :
         QStringLiteral( "= %1" ).arg( QgsSqliteUtils::quotedString( dsUri.schema( ) ) );
}

void QgsSpatiaLiteProvider::fetchConstraints()
{
  char **results = nullptr;
  char *errMsg = nullptr;

  // this is not robust but unfortunately sqlite offers no way to check directly
  // for the presence of constraints on a field (only indexes, but not all constraints are indexes)
  QString sql = QStringLiteral( "SELECT sql FROM sqlite_master WHERE type='table' AND name=%1" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );
  int columns = 0;
  int rows = 0;

  int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return;
  }

  if ( rows < 1 )
    ;
  else
  {
    // Use the same logic implemented in GDAL for GPKG
    QSet<QString> uniqueFieldNames;
    {
      QString errMsg;
      uniqueFieldNames = QgsSqliteUtils::uniqueFields( mSqliteHandle, mTableName, errMsg );
    }

    QString sqlDef = QString::fromUtf8( results[ 1 ] );
    // extract definition
    const thread_local QRegularExpression re( QStringLiteral( R"raw(\((.+)\))raw" ), QRegularExpression::PatternOption::DotMatchesEverythingOption );
    QRegularExpressionMatch match = re.match( sqlDef );
    if ( match.hasMatch() )
    {
      const QString matched = match.captured( 1 );
      for ( auto &field : matched.split( ',' ) )
      {
        field = field.trimmed();
        QString fieldName;
        QString definition;
        const QChar delimiter { field.at( 0 ) };
        if ( delimiter == '"' || delimiter == '`' )
        {
          const int start = field.indexOf( delimiter ) + 1;
          const int end = field.indexOf( delimiter, start );
          fieldName = field.mid( start, end - start );
          definition = field.mid( end + 1 );
        }
        else
        {
          fieldName = field.left( field.indexOf( ' ' ) );
          definition = field.mid( field.indexOf( ' ' ) + 1 );
        }
        int fieldIdx = mAttributeFields.lookupField( fieldName );
        if ( fieldIdx >= 0 )
        {
          QgsFieldConstraints constraints = mAttributeFields.at( fieldIdx ).constraints();
          if ( uniqueFieldNames.contains( fieldName ) || definition.contains( QLatin1String( "primary key" ), Qt::CaseInsensitive ) )
            constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
          if ( definition.contains( QLatin1String( "not null" ), Qt::CaseInsensitive ) || definition.contains( QLatin1String( "primary key" ), Qt::CaseInsensitive ) )
            constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
          mAttributeFields[ fieldIdx ].setConstraints( constraints );
        }
      }
    }

  }
  sqlite3_free_table( results );

  for ( const auto fieldIdx : std::as_const( mPrimaryKeyAttrs ) )
  {
    QgsFieldConstraints constraints = mAttributeFields.at( fieldIdx ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[ fieldIdx ].setConstraints( constraints );

    if ( mAttributeFields[ fieldIdx ].name() == mPrimaryKey )
    {
      QString sql = QStringLiteral( "SELECT sql FROM sqlite_master WHERE type = 'table' AND tbl_name like %1" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );
      int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      if ( ret != SQLITE_OK )
      {
        handleError( sql, errMsg, QString() );
        return;
      }

      if ( rows >= 1 )
      {
        QString tableSql = QString::fromUtf8( results[ 1 ] );
        QRegularExpression rx( QStringLiteral( "[(,]\\s*(?:%1|\"%1\"|`%1`)\\s+INTEGER PRIMARY KEY AUTOINCREMENT" ).arg( mPrimaryKey ), QRegularExpression::CaseInsensitiveOption );
        if ( tableSql.contains( rx ) )
        {
          mPrimaryKeyAutoIncrement = true;
          insertDefaultValue( fieldIdx, tr( "Autogenerate" ) );
        }
      }
      sqlite3_free_table( results );
    }
  }
}

void QgsSpatiaLiteProvider::insertDefaultValue( int fieldIndex, QString defaultVal )
{
  if ( !defaultVal.isEmpty() )
  {
    QVariant defaultVariant = defaultVal;

    if ( mAttributeFields.at( fieldIndex ).name() != mPrimaryKey || !mPrimaryKeyAutoIncrement )
    {
      bool ok;
      switch ( mAttributeFields.at( fieldIndex ).type() )
      {
        case QMetaType::Type::LongLong:
          defaultVariant = defaultVal.toLongLong( &ok );
          break;

        case QMetaType::Type::Double:
          defaultVariant = defaultVal.toDouble( &ok );
          break;

        default:
        {
          // Literal string?
          ok = defaultVal.startsWith( '\'' );
          if ( ok )
            defaultVal = defaultVal.remove( 0, 1 );
          if ( defaultVal.endsWith( '\'' ) )
            defaultVal.chop( 1 );
          defaultVal.replace( QLatin1String( "''" ), QLatin1String( "'" ) );

          defaultVariant = defaultVal;
          break;
        }
      }

      if ( ! ok )  // Must be a SQL clause and not a literal
      {
        mDefaultValueClause.insert( fieldIndex, defaultVal );
      }

    }
    mDefaultValues.insert( fieldIndex, defaultVal );
  }
}

QVariant QgsSpatiaLiteProvider::defaultValue( int fieldId ) const
{
  // TODO: backend-side evaluation
  if ( fieldId < 0 || fieldId >= mAttributeFields.count() )
    return QVariant();

  QString defaultVal = mDefaultValues.value( fieldId, QString() );
  if ( defaultVal.isEmpty() )
    return QVariant();

  QVariant resultVar = defaultVal;
  if ( defaultVal == QLatin1String( "CURRENT_TIMESTAMP" ) )
    resultVar = QDateTime::currentDateTime();
  else if ( defaultVal == QLatin1String( "CURRENT_DATE" ) )
    resultVar = QDate::currentDate();
  else if ( defaultVal == QLatin1String( "CURRENT_TIME" ) )
    resultVar = QTime::currentTime();
  else if ( defaultVal.startsWith( '\'' ) )
  {
    defaultVal = defaultVal.remove( 0, 1 );
    defaultVal.chop( 1 );
    defaultVal.replace( QLatin1String( "''" ), QLatin1String( "'" ) );
    resultVar = defaultVal;
  }

  if ( mTransaction &&
       mAttributeFields.at( fieldId ).name() == mPrimaryKey &&
       mPrimaryKeyAutoIncrement &&
       mDefaultValues.value( fieldId, QString() ) == tr( "Autogenerate" ) &&
       providerProperty( EvaluateDefaultValues, false ).toBool() )
  {
    QString errorMessage;
    QVariant nextVal = QgsSqliteUtils::nextSequenceValue( sqliteHandle(), mTableName, errorMessage );
    if ( errorMessage.isEmpty() && nextVal != -1 )
    {
      resultVar = nextVal;
    }
    else
    {
      QgsMessageLog::logMessage( errorMessage, tr( "SpatiaLite" ) );
    }
  }

  const bool compatible = mAttributeFields.at( fieldId ).convertCompatible( resultVar );
  return compatible && !QgsVariantUtils::isNull( resultVar ) ? resultVar : QVariant();
}

QString QgsSpatiaLiteProvider::defaultValueClause( int fieldIndex ) const
{
  if ( ! mAttributeFields.exists( fieldIndex ) )
  {
    return QString();
  }

  if ( mAttributeFields.at( fieldIndex ).name() == mPrimaryKey && mPrimaryKeyAutoIncrement )
  {
    if ( mTransaction &&
         providerProperty( EvaluateDefaultValues, false ).toBool() )
    {
      return QString();
    }
    else
    {
      return tr( "Autogenerate" );
    }
  }
  return mDefaultValueClause.value( fieldIndex, QString() );
}

void QgsSpatiaLiteProvider::handleError( const QString &sql, char *errorMessage, const QString &savepointId )
{
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errorMessage ? errorMessage : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errorMessage )
  {
    sqlite3_free( errorMessage );
  }

  if ( ! savepointId.isEmpty() )
  {
    // ROLLBACK after some previous error
    ( void )exec_sql( sqliteHandle(), QStringLiteral( "ROLLBACK TRANSACTION TO \"%1\"" ).arg( savepointId ), uri().uri(), nullptr, QGS_QUERY_LOG_ORIGIN );
  }
}

int QgsSpatiaLiteProvider::exec_sql( sqlite3 *handle, const QString &sql, const QString &uri, char *errMsg, const QString &origin )
{
  QgsDatabaseQueryLogWrapper logWrapper( sql, uri, QStringLiteral( "spatialite" ), QStringLiteral( "QgsSpatiaLiteProvider" ), origin );
  // Use transaction's handle (if any)
  return sqlite3_exec( handle,  sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
}

sqlite3 *QgsSpatiaLiteProvider::sqliteHandle() const
{
  return mTransaction && mTransaction->sqliteHandle() ? mTransaction->sqliteHandle() : mSqliteHandle;
}

void QgsSpatiaLiteProvider::loadFields()
{
  int ret;
  int i;
  sqlite3_stmt *stmt = nullptr;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString pkName;
  int pkCount = 0;
  QString sql;

  mAttributeFields.clear();
  mDefaultValues.clear();

  if ( !mIsQuery )
  {
    mPrimaryKey.clear();
    mPrimaryKeyAttrs.clear();

    sql = QStringLiteral( "PRAGMA table_info(%1)" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, QString() );
      return;
    }
    if ( rows < 1 )
      ;
    else
    {
      int realFieldIndex = 0;
      for ( i = 1; i <= rows; i++ )
      {
        QString name = QString::fromUtf8( results[( i * columns ) + 1] );
        if ( name.compare( mGeometryColumn, Qt::CaseInsensitive ) == 0 )
          continue;
        QString type = QString::fromUtf8( results[( i * columns ) + 2] ).toLower();
        QString pk = results[( i * columns ) + 5];
        if ( pk.toInt() != 0 && ( type.compare( QLatin1String( "integer" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 ||
                                  type.compare( QLatin1String( "bigint" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 ) )
        {
          // found a Primary Key column
          pkCount++;
          if ( mPrimaryKeyAttrs.isEmpty() )
            pkName = name;
          else
            pkName.clear();
          mPrimaryKeyAttrs << realFieldIndex;
          QgsDebugMsgLevel( "found primaryKey " + name, 2 );
        }

        const TypeSubType fieldType = getVariantType( type );
        mAttributeFields.append( QgsField( name, fieldType.first, type, 0, 0, QString(), fieldType.second ) );

        insertDefaultValue( realFieldIndex, QString::fromUtf8( results[( i * columns ) + 4] ) );
        realFieldIndex += 1;
      }
    }
    sqlite3_free_table( results );

    if ( pkCount == 1 )
    {
      // setting the Primary Key column name
      mPrimaryKey = pkName;
    }

    // check for constraints
    fetchConstraints();

    // for views try to get the primary key from the meta table
    if ( mViewBased && mPrimaryKey.isEmpty() )
    {
      determineViewPrimaryKey();
    }
  }
  else
  {
    sql = QStringLiteral( "select * from %1 limit 1" ).arg( mQuery );

    if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
    {
      // some error occurred
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ), tr( "SpatiaLite" ) );
      return;
    }

    ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE )
    {
      // there are no rows to fetch
      sqlite3_finalize( stmt );
      return;
    }

    if ( ret == SQLITE_ROW )
    {
      // one valid row has been fetched from the result set
      columns = sqlite3_column_count( stmt );
      for ( i = 0; i < columns; i++ )
      {
        QString name = QString::fromUtf8( sqlite3_column_name( stmt, i ) );
        QString type = QString::fromUtf8( sqlite3_column_decltype( stmt, i ) ).toLower();
        if ( type.isEmpty() )
          type = QStringLiteral( "text" );

        if ( name == mPrimaryKey )
        {
          // Skip if ROWID has been added to the query by the provider
          if ( mRowidInjectedInQuery )
            continue;
          pkCount++;
          if ( mPrimaryKeyAttrs.isEmpty() )
            pkName = name;
          else
            pkName.clear();
          mPrimaryKeyAttrs << i - 1;
          QgsDebugMsgLevel( "found primaryKey " + name, 2 );
        }

        if ( name.toLower() != mGeometryColumn )
        {
          const TypeSubType fieldType = getVariantType( type );
          mAttributeFields.append( QgsField( name, fieldType.first, type, 0, 0, QString(), fieldType.second ) );
        }
      }
    }
    sqlite3_finalize( stmt );

    if ( pkCount == 1 )
    {
      // setting the Primary Key column name
      mPrimaryKey = pkName;
    }
  }

  updatePrimaryKeyCapabilities();
}

void QgsSpatiaLiteProvider::determineViewPrimaryKey()
{
  QString sql = QString( "SELECT view_rowid"
                         " FROM views_geometry_columns"
                         " WHERE upper(view_name) = upper(%1) and upper(view_geometry) = upper(%2)" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                             QgsSqliteUtils::quotedString( mGeometryColumn ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret == SQLITE_OK )
  {
    if ( rows > 0 )
    {
      mPrimaryKey = results[1 * columns];
      int idx = mAttributeFields.lookupField( mPrimaryKey );
      if ( idx != -1 )
        mPrimaryKeyAttrs << idx;
    }
    sqlite3_free_table( results );
  }
}

QStringList QgsSpatiaLiteProvider::tablePrimaryKeys( const QString &tableName ) const
{
  QStringList result;
  const QString sql = QStringLiteral( "PRAGMA table_info(%1)" ).arg( QgsSqliteUtils::quotedIdentifier( tableName ) );
  char **results = nullptr;
  sqlite3_stmt *stmt = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ),
                               tr( "SpatiaLite" ) );
  }
  else
  {
    int ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK )
    {
      for ( int row = 1; row <= rows; ++row )
      {
        QString type = QString::fromUtf8( results[( row * columns ) + 2] ).toLower();
        if ( QString::fromUtf8( results[row * columns + 5] ) == QChar( '1' ) &&
             ( type.compare( QLatin1String( "integer" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 ||
               type.compare( QLatin1String( "bigint" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 ) )
        {
          result << QString::fromUtf8( results[row * columns + 1] );
        }
      }
      sqlite3_free_table( results );
    }
    else
    {
      QgsLogger::warning( QStringLiteral( "SQLite error discovering integer primary keys: %1" ).arg( errMsg ) );
      sqlite3_free( errMsg );
    }
  }
  sqlite3_finalize( stmt );
  return result;
}

bool QgsSpatiaLiteProvider::hasTriggers()
{
  int ret;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString sql;

  sql = QStringLiteral( "SELECT * FROM sqlite_master WHERE type='trigger' AND tbl_name=%1" )
        .arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );

  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  sqlite3_free_table( results );
  return ( ret == SQLITE_OK && rows > 0 );
}

bool QgsSpatiaLiteProvider::hasRowid()
{
  if ( mAttributeFields.lookupField( QStringLiteral( "ROWID" ) ) >= 0 )
    return false;

  // table without rowid column
  QString sql = QStringLiteral( "SELECT rowid FROM %1 WHERE 0" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );
  char *errMsg = nullptr;
  return exec_sql( sqliteHandle( ), sql, uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN ) == SQLITE_OK;
}


QString QgsSpatiaLiteProvider::storageType() const
{
  return QStringLiteral( "SQLite database with SpatiaLite extension" );
}

QgsFeatureIterator QgsSpatiaLiteProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsDebugError( QStringLiteral( "Read attempt on an invalid SpatiaLite data source" ) );
    return QgsFeatureIterator();
  }
  return QgsFeatureIterator( new QgsSpatiaLiteFeatureIterator( new QgsSpatiaLiteFeatureSource( this ), true, request ) );
}


int QgsSpatiaLiteProvider::computeSizeFromGeosWKB2D( const unsigned char *blob,
    int size, Qgis::WkbType type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size )
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob + 5;
  int gsize = 5;

  if ( QgsWkbTypes::isMultiType( type ) )
  {
    gsize += computeSizeFromMultiWKB2D( p_in, nDims, little_endian,
                                        endian_arch );
  }
  else
  {
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      // compunting the required size
      case Qgis::GeometryType::Point:
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
      case Qgis::GeometryType::Line:
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
      case Qgis::GeometryType::Polygon:
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

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        break;
    }
  }

  return gsize;
}

int QgsSpatiaLiteProvider::computeSizeFromMultiWKB2D( const unsigned char *p_in,
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
            p_in += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            p_in += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            p_in += 2 * sizeof( double );
            break;
        }
        break;
      case GAIA_LINESTRING:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            p_in += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            p_in += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            p_in += points * ( 2 * sizeof( double ) );
            break;
        }
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
              p_in += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              p_in += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              p_in += points * ( 2 * sizeof( double ) );
              break;
          }
        }
        break;
    }
  }

  return size;
}

int QgsSpatiaLiteProvider::computeSizeFromGeosWKB3D( const unsigned char *blob,
    int size, Qgis::WkbType type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size )
// calculating the size required to store this WKB
  int rings;
  int points;
  int ib;
  const unsigned char *p_in = blob + 5;
  int gsize = 5;

  if ( QgsWkbTypes::isMultiType( type ) )
  {
    gsize += computeSizeFromMultiWKB3D( p_in, nDims, little_endian,
                                        endian_arch );
  }
  else
  {
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      // compunting the required size
      case Qgis::GeometryType::Point:
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
      case Qgis::GeometryType::Line:
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
      case Qgis::GeometryType::Polygon:
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

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        break;
    }
  }

  return gsize;
}

int QgsSpatiaLiteProvider::computeSizeFromMultiWKB3D( const unsigned char *p_in,
    int nDims,
    int little_endian,
    int endian_arch )
{
// calculating the size required to store this WKB
  int entities;
  Qgis::WkbType type;
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
    type = static_cast< Qgis::WkbType >( gaiaImport32( p_in + 1, little_endian, endian_arch ) );
    p_in += 5;
    size += 5;
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      // compunting the required size
      case Qgis::GeometryType::Point:
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += 4 * sizeof( double );
            p_in += 4 * sizeof( double );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += 3 * sizeof( double );
            p_in += 3 * sizeof( double );
            break;
          default:
            size += 2 * sizeof( double );
            p_in += 2 * sizeof( double );
            break;
        }
        break;
      case Qgis::GeometryType::Line:
        points = gaiaImport32( p_in, little_endian, endian_arch );
        p_in += 4;
        size += 4;
        switch ( nDims )
        {
          case GAIA_XY_Z_M:
            size += points * ( 4 * sizeof( double ) );
            p_in += points * ( 4 * sizeof( double ) );
            break;
          case GAIA_XY_Z:
          case GAIA_XY_M:
            size += points * ( 3 * sizeof( double ) );
            p_in += points * ( 3 * sizeof( double ) );
            break;
          default:
            size += points * ( 2 * sizeof( double ) );
            p_in += points * ( 2 * sizeof( double ) );
            break;
        }
        break;
      case Qgis::GeometryType::Polygon:
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
              p_in += points * ( 4 * sizeof( double ) );
              break;
            case GAIA_XY_Z:
            case GAIA_XY_M:
              size += points * ( 3 * sizeof( double ) );
              p_in += points * ( 3 * sizeof( double ) );
              break;
            default:
              size += points * ( 2 * sizeof( double ) );
              p_in += points * ( 2 * sizeof( double ) );
              break;
          }
        }
        break;

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        break;
    }
  }

  return size;
}

void QgsSpatiaLiteProvider::convertFromGeosWKB( const unsigned char *blob,
    int blob_size,
    unsigned char **wkb,
    int *geom_size,
    int nDims )
{
  // attempting to convert from 2D/3D GEOS own WKB
  Qgis::WkbType type;
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
  type = static_cast< Qgis::WkbType >( gaiaImport32( blob + 1, little_endian, endian_arch ) );
  if ( QgsWkbTypes::hasZ( type ) || QgsWkbTypes::hasM( type ) )
    gDims = 3;
  else if ( type != Qgis::WkbType::Unknown )
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
    gsize = computeSizeFromGeosWKB3D( blob, blob_size, type, nDims,
                                      little_endian, endian_arch );
  else
    gsize = computeSizeFromGeosWKB2D( blob, blob_size, type, nDims,
                                      little_endian, endian_arch );

  unsigned char *wkbGeom = new unsigned char[gsize];
  memset( wkbGeom, '\0', gsize );

  if ( gDims == 3 )
    convertFromGeosWKB3D( blob, blob_size, wkbGeom, gsize, nDims,
                          little_endian, endian_arch );
  else
    convertFromGeosWKB2D( blob, blob_size, wkbGeom, gsize, nDims,
                          little_endian, endian_arch );

  *wkb = wkbGeom;
  *geom_size = gsize;
}

void QgsSpatiaLiteProvider::convertFromGeosWKB2D( const unsigned char *blob,
    int blob_size,
    unsigned char *wkb,
    int geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size )
  Q_UNUSED( geom_size )
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
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
      {
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // M
        p_in += sizeof( double );
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
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // M
          p_in += sizeof( double );
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
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // M
            p_in += sizeof( double );
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
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // M
          p_in += sizeof( double );
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
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // M
            p_in += sizeof( double );
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
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // M
              p_in += sizeof( double );
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
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // M
              p_in += sizeof( double );
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
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // M
                p_in += sizeof( double );
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
                  coord = gaiaImport64( p_in, little_endian, endian_arch );
                  gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                  p_in += sizeof( double );
                  p_out += sizeof( double );
                }
                if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
                {
                  coord = gaiaImport64( p_in, little_endian, endian_arch );
                  gaiaExport64( p_out, coord, 1, endian_arch );  // M
                  p_in += sizeof( double );
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

void QgsSpatiaLiteProvider::convertFromGeosWKB3D( const unsigned char *blob,
    int blob_size,
    unsigned char *wkb,
    int geom_size,
    int nDims,
    int little_endian,
    int endian_arch )
{
  Q_UNUSED( blob_size )
  Q_UNUSED( geom_size )
  // attempting to convert from 3D GEOS own WKB
  Qgis::WkbType type;
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
  type = static_cast< Qgis::WkbType >( gaiaImport32( blob + 1, little_endian, endian_arch ) );
  if ( QgsWkbTypes::geometryType( type ) == Qgis::GeometryType::Point )
  {
    if ( QgsWkbTypes::isSingleType( type ) )
    {
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
    }
    else
    {
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
    }
  }
  else if ( QgsWkbTypes::geometryType( type ) == Qgis::GeometryType::Line )
  {
    if ( QgsWkbTypes::isSingleType( type ) )
    {
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
    }
    else
    {
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
    }
  }
  else if ( QgsWkbTypes::geometryType( type ) == Qgis::GeometryType::Polygon )
  {
    if ( QgsWkbTypes::isSingleType( type ) )
    {
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
    }
    else
    {
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
    }
  }
  else if ( QgsWkbTypes::flatType( type ) == Qgis::WkbType::GeometryCollection )
  {
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
  }
  p_in = blob + 5;
  p_out += 4;
  if ( QgsWkbTypes::isSingleType( type ) )
  {
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      // setting Geometry values
      case Qgis::GeometryType::Point:
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
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
        if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
        {
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // M
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
        break;
      case Qgis::GeometryType::Line:
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
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // M
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
        }
        break;
      case Qgis::GeometryType::Polygon:
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
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // M
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
          }
        }
        break;

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        break;
    }
  }
  else
  {
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      case Qgis::GeometryType::Point:
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
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
          if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
          {
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // M
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
        }
        break;
      case Qgis::GeometryType::Line:
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
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
            {
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // M
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
          }
        }
        break;
      case Qgis::GeometryType::Polygon:
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
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
              if ( nDims == GAIA_XY_M || nDims == GAIA_XY_Z_M )
              {
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // M
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
            }
          }
        }
        break;

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        break;
    }
  }
}

void QgsSpatiaLiteProvider::convertToGeosWKB( const unsigned char *blob,
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
      gsize += 3 * sizeof( double );
      break;
    case GAIA_POINTZM:
      gsize += 4 * sizeof( double );
      break;
    case GAIA_LINESTRINGZ:
    case GAIA_LINESTRINGM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      gsize += points * ( 3 * sizeof( double ) );
      break;
    case GAIA_LINESTRINGZM:
      points = gaiaImport32( p_in, little_endian, endian_arch );
      gsize += 4;
      gsize += points * ( 4 * sizeof( double ) );
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
        gsize += points * ( 4 * sizeof( double ) );
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
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::Point25D ), 1, endian_arch );
      break;
    case GAIA_POINTM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::PointM ), 1, endian_arch );
      break;
    case GAIA_POINTZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::PointZM ), 1, endian_arch );
      break;
    case GAIA_LINESTRINGZ:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::LineString25D ), 1, endian_arch );
      break;
    case GAIA_LINESTRINGM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::LineStringM ), 1, endian_arch );
      break;
    case GAIA_LINESTRINGZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::LineStringZM ), 1, endian_arch );
      break;
    case GAIA_POLYGONZ:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::Polygon25D ), 1, endian_arch );
      break;
    case GAIA_POLYGONM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::PolygonM ), 1, endian_arch );
      break;
    case GAIA_POLYGONZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::PolygonZM ), 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZ:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiPoint25D ), 1, endian_arch );
      break;
    case GAIA_MULTIPOINTM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiPointM ), 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiPointZM ), 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZ:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiLineString25D ), 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiLineStringM ), 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiLineStringZM ), 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZ:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiPolygon25D ), 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiPolygonM ), 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::MultiPolygonZM ), 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZ:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::GeometryCollectionZ ), 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::GeometryCollectionM ), 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZM:
      gaiaExport32( wkbGeom + 1, static_cast< quint32>( Qgis::WkbType::GeometryCollectionZM ), 1, endian_arch );
      break;
  }
  p_in = blob + 5;
  p_out = wkbGeom + 5;
  switch ( type )
  {
    // setting Geometry values
    case GAIA_POINTZ:
    case GAIA_POINTM:
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // X
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Y
      p_in += sizeof( double );
      p_out += sizeof( double );
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
      p_in += sizeof( double );
      p_out += sizeof( double );
      break;
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
      coord = gaiaImport64( p_in, little_endian, endian_arch );
      gaiaExport64( p_out, coord, 1, endian_arch );  // M
      p_in += sizeof( double );
      p_out += sizeof( double );
      break;
    case GAIA_LINESTRINGZ:
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
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
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
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // M
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_POLYGONZ:
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
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
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
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // M
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOINTZ:
    case GAIA_MULTIPOINTM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, static_cast< quint32>( type == GAIA_MULTIPOINTZ ? Qgis::WkbType::Point25D : Qgis::WkbType::PointM ), 1, endian_arch );
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
        gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_MULTIPOINTZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::PointZM ), 1, endian_arch );
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
        coord = gaiaImport64( p_in, little_endian, endian_arch );
        gaiaExport64( p_out, coord, 1, endian_arch );  // M
        p_in += sizeof( double );
        p_out += sizeof( double );
      }
      break;
    case GAIA_MULTILINESTRINGZ:
    case GAIA_MULTILINESTRINGM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, static_cast< quint32>( type == GAIA_MULTILINESTRINGZ ? Qgis::WkbType::LineString25D : Qgis::WkbType::LineStringM ), 1, endian_arch );
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
          gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTILINESTRINGZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::LineStringZM ), 1, endian_arch );
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
          coord = gaiaImport64( p_in, little_endian, endian_arch );
          gaiaExport64( p_out, coord, 1, endian_arch );  // M
          p_in += sizeof( double );
          p_out += sizeof( double );
        }
      }
      break;
    case GAIA_MULTIPOLYGONZ:
    case GAIA_MULTIPOLYGONM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, static_cast< quint32>( type == GAIA_MULTIPOLYGONZ ? Qgis::WkbType::Polygon25D : Qgis::WkbType::PolygonM ), 1, endian_arch );
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
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
            p_in += sizeof( double );
            p_out += sizeof( double );
          }
        }
      }
      break;
    case GAIA_MULTIPOLYGONZM:
      entities = gaiaImport32( p_in, little_endian, endian_arch );
      p_in += 4;
      gaiaExport32( p_out, entities, 1, endian_arch );
      p_out += 4;
      for ( ie = 0; ie < entities; ie++ )
      {
        p_in += 5;
        *p_out++ = 0x01;
        gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::PolygonZM ), 1, endian_arch );
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
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // M
            p_in += sizeof( double );
            p_out += sizeof( double );
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
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::Point25D ), 1, endian_arch );
            break;
          case GAIA_POINTM:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::PointM ), 1, endian_arch );
            break;
          case GAIA_POINTZM:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::PointZM ), 1, endian_arch );
            break;
          case GAIA_LINESTRINGZ:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::LineString25D ), 1, endian_arch );
            break;
          case GAIA_LINESTRINGM:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::LineStringM ), 1, endian_arch );
            break;
          case GAIA_LINESTRINGZM:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::LineStringZM ), 1, endian_arch );
            break;
          case GAIA_POLYGONZ:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::Polygon25D ), 1, endian_arch );
            break;
          case GAIA_POLYGONM:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::PolygonM ), 1, endian_arch );
            break;
          case GAIA_POLYGONZM:
            gaiaExport32( p_out, static_cast< quint32>( Qgis::WkbType::PolygonZM ), 1, endian_arch );
            break;
        }
        p_out += 4;
        switch ( type2 )
        {
          // setting sub-Geometry values
          case GAIA_POINTZ:
          case GAIA_POINTM:
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // X
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Y
            p_in += sizeof( double );
            p_out += sizeof( double );
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
            p_in += sizeof( double );
            p_out += sizeof( double );
            break;
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
            coord = gaiaImport64( p_in, little_endian, endian_arch );
            gaiaExport64( p_out, coord, 1, endian_arch );  // M
            p_in += sizeof( double );
            p_out += sizeof( double );
            break;
          case GAIA_LINESTRINGZ:
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
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            break;
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
              coord = gaiaImport64( p_in, little_endian, endian_arch );
              gaiaExport64( p_out, coord, 1, endian_arch );  // M
              p_in += sizeof( double );
              p_out += sizeof( double );
            }
            break;
          case GAIA_POLYGONZ:
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
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // Z or M
                p_in += sizeof( double );
                p_out += sizeof( double );
              }
            }
            break;
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
                coord = gaiaImport64( p_in, little_endian, endian_arch );
                gaiaExport64( p_out, coord, 1, endian_arch );  // M
                p_in += sizeof( double );
                p_out += sizeof( double );
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

int QgsSpatiaLiteProvider::computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian, int endian_arch )
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
        size += 4 * sizeof( double );
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
        size += points * ( 4 * sizeof( double ) );
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
          size += points * ( 4 * sizeof( double ) );
          p_in += points * ( 4 * sizeof( double ) );
        }
        break;
    }
  }

  return size;
}

QString QgsSpatiaLiteProvider::subsetString() const
{
  return mSubsetString;
}

bool QgsSpatiaLiteProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  if ( theSQL == mSubsetString )
    return true;

  QString prevSubsetString = mSubsetString;
  mSubsetString = theSQL;

  // update URI
  QgsDataSourceUri uri = QgsDataSourceUri( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );

  // update feature count and extents
  if ( updateFeatureCount && getTableSummary() )
  {
    emit dataChanged();
    return true;
  }

  mSubsetString = prevSubsetString;

  // restore URI
  uri = QgsDataSourceUri( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );

  getTableSummary();

  return false;
}


QgsRectangle QgsSpatiaLiteProvider::extent() const
{
  return mLayerExtent.toRectangle();
}

QgsBox3D QgsSpatiaLiteProvider::extent3D() const
{
  return mLayerExtent;
}

void QgsSpatiaLiteProvider::updateExtents()
{
  getTableSummary();
}

size_t QgsSpatiaLiteProvider::layerCount() const
{
  return 1;
}


/**
 * Returns the feature type
 */
Qgis::WkbType QgsSpatiaLiteProvider::wkbType() const
{
  return mGeomType;
}

/**
 * Returns the feature type
 */
long long QgsSpatiaLiteProvider::featureCount() const
{
  return mNumberFeatures;
}


QgsCoordinateReferenceSystem QgsSpatiaLiteProvider::crs() const
{
  QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mAuthId );
  if ( !srs.isValid() )
  {
    srs = QgsCoordinateReferenceSystem::fromProj( mProj4text );
  }
  return srs;
}

bool QgsSpatiaLiteProvider::isValid() const
{
  return mValid;
}

Qgis::ProviderStyleStorageCapabilities QgsSpatiaLiteProvider::styleStorageCapabilities() const
{
  Qgis::ProviderStyleStorageCapabilities storageCapabilities;
  if ( isValid() )
  {
    storageCapabilities |= Qgis::ProviderStyleStorageCapability::SaveToDatabase;
    storageCapabilities |= Qgis::ProviderStyleStorageCapability::LoadFromDatabase;
  }
  return storageCapabilities;
}

QString QgsSpatiaLiteProvider::name() const
{
  return SPATIALITE_KEY;
}                               //  QgsSpatiaLiteProvider::name()

QString QgsSpatiaLiteProvider::providerKey()
{
  return SPATIALITE_KEY;
}

QString QgsSpatiaLiteProvider::description() const
{
  return SPATIALITE_DESCRIPTION;
}                               //  QgsSpatiaLiteProvider::description()

QgsFields QgsSpatiaLiteProvider::fields() const
{
  return mAttributeFields;
}

// Returns the minimum value of an attribute
QVariant QgsSpatiaLiteProvider::minimumValue( int index ) const
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString minValue;
  QString sql;

  try
  {
    // get the field name
    QgsField fld = field( index );

    sql = QStringLiteral( "SELECT Min(%1) FROM %2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), mQuery );

    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
      // unexpected error
      if ( errMsg )
      {
        sqlite3_free( errMsg );
      }
      minValue = QString();
    }
    else
    {
      if ( rows < 1 )
        ;
      else
      {
        for ( i = 1; i <= rows; i++ )
        {
          minValue = results[( i * columns ) + 0];
        }
      }
      sqlite3_free_table( results );

      if ( minValue.isEmpty() )
      {
        // NULL or not found
        minValue = QString();
      }
    }

    return convertValue( fld.type(), minValue );
  }
  catch ( SLFieldNotFound )
  {
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  }
}

// Returns the maximum value of an attribute
QVariant QgsSpatiaLiteProvider::maximumValue( int index ) const
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString maxValue;
  QString sql;

  try
  {
    // get the field name
    QgsField fld = field( index );

    sql = QStringLiteral( "SELECT Max(%1) FROM %2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), mQuery );

    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ? errMsg : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
      // unexpected error
      if ( errMsg )
      {
        sqlite3_free( errMsg );
      }
      maxValue = QString();
    }
    else
    {

      if ( rows < 1 )
        ;
      else
      {
        for ( i = 1; i <= rows; i++ )
        {
          maxValue = results[( i * columns ) + 0];
        }
      }
      sqlite3_free_table( results );

      if ( maxValue.isEmpty() )
      {
        // NULL or not found
        maxValue = QString();
      }
    }

    return convertValue( fld.type(), maxValue );
  }
  catch ( SLFieldNotFound )
  {
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
  }
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsSpatiaLiteProvider::uniqueValues( int index, int limit ) const
{
  sqlite3_stmt *stmt = nullptr;
  QString sql;

  QSet<QVariant> uniqueValues;

  // get the field name
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    return uniqueValues; //invalid field
  }
  QgsField fld = mAttributeFields.at( index );

  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ')';
  }

  sql += QStringLiteral( " ORDER BY %1" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ) );

  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ), tr( "SpatiaLite" ) );
  }
  else
  {
    while ( true )
    {
      // this one is an infinitive loop, intended to fetch any row
      int ret = sqlite3_step( stmt );

      if ( ret == SQLITE_DONE )
      {
        // there are no more rows to fetch - we can stop looping
        break;
      }

      if ( ret == SQLITE_ROW )
      {
        // fetching one column value
        switch ( sqlite3_column_type( stmt, 0 ) )
        {
          case SQLITE_INTEGER:
            uniqueValues.insert( QVariant( sqlite3_column_int64( stmt, 0 ) ) );
            break;
          case SQLITE_FLOAT:
            uniqueValues.insert( QVariant( sqlite3_column_double( stmt, 0 ) ) );
            break;
          case SQLITE_TEXT:
          {
            const QString txt = QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) );
            if ( mAttributeFields.at( index ).type() == QMetaType::Type::QDateTime )
            {
              QDateTime dt = QDateTime::fromString( txt, Qt::ISODate );
              if ( !dt.isValid() )
              {
                // if that fails, try SQLite's default date format
                dt = QDateTime::fromString( txt, QStringLiteral( "yyyy-MM-dd hh:mm:ss" ) );
              }
              uniqueValues.insert( QVariant( dt ) );
            }
            else if ( mAttributeFields.at( index ).type() == QMetaType::Type::QDate )
            {
              uniqueValues.insert( QVariant( QDate::fromString( txt, QStringLiteral( "yyyy-MM-dd" ) ) ) );
            }
            else
            {
              uniqueValues.insert( QVariant( txt ) );
            }
            break;
          }
          default:
            uniqueValues.insert( QgsVariantUtils::createNullVariant( mAttributeFields.at( index ).type() ) );
            break;
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ), tr( "SpatiaLite" ) );
        sqlite3_finalize( stmt );
        return uniqueValues;
      }
    }
  }

  sqlite3_finalize( stmt );
  return uniqueValues;
}

QStringList QgsSpatiaLiteProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;

  sqlite3_stmt *stmt = nullptr;
  QString sql;

  // get the field name
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    return results; //invalid field
  }
  QgsField fld = mAttributeFields.at( index );

  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 " ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), mQuery );
  sql += QStringLiteral( " WHERE " ) + QgsSqliteUtils::quotedIdentifier( fld.name() ) + QStringLiteral( " LIKE '%" ) + substring + QStringLiteral( "%'" );

  if ( !mSubsetString.isEmpty() )
  {
    sql += QStringLiteral( " AND ( " ) + mSubsetString + ')';
  }

  sql += QStringLiteral( " ORDER BY %1" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ) );

  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ), tr( "SpatiaLite" ) );

  }
  else
  {
    while ( ( limit < 0 || results.size() < limit ) && ( !feedback || !feedback->isCanceled() ) )
    {
      // this one is an infinitive loop, intended to fetch any row
      int ret = sqlite3_step( stmt );

      if ( ret == SQLITE_DONE )
      {
        // there are no more rows to fetch - we can stop looping
        break;
      }

      if ( ret == SQLITE_ROW )
      {
        // fetching one column value
        switch ( sqlite3_column_type( stmt, 0 ) )
        {
          case SQLITE_TEXT:
            results.append( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) ) );
            break;
          default:
            break;
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ), tr( "SpatiaLite" ) );
        sqlite3_finalize( stmt );
        return results;
      }
    }
  }
  sqlite3_finalize( stmt );
  return results;
}

QString QgsSpatiaLiteProvider::geomParam() const
{
  QString geometry;

  bool forceMulti = QgsWkbTypes::isMultiType( wkbType() );

  // ST_Multi function is available from QGIS >= 2.4
  bool hasMultiFunction = mSpatialiteVersionMajor > 2 ||
                          ( mSpatialiteVersionMajor == 2 && mSpatialiteVersionMinor >= 4 );

  if ( forceMulti && hasMultiFunction )
  {
    geometry += QLatin1String( "ST_Multi(" );
  }

  geometry += QStringLiteral( "GeomFromWKB(?, %2)" ).arg( mSrid );

  if ( forceMulti && hasMultiFunction )
  {
    geometry += ')';
  }

  return geometry;
}

static void deleteWkbBlob( void *wkbBlob )
{
  delete[]( char * )wkbBlob;
}

bool QgsSpatiaLiteProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  bool toCommit = false;
  QString baseValues;
  int ia, ret;
  // SQL for single row
  QString sql;

  if ( flist.isEmpty() )
    return true;

  const QString logUri = uri( ).uri( false );

  QgsAttributes attributevec = flist[0].attributes();

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  ret = exec_sql( sqliteHandle(), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret == SQLITE_OK )
  {
    toCommit = true;

    QString baseSql { QStringLiteral( "INSERT INTO %1(" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) ) };
    baseValues = QStringLiteral( ") VALUES (" );

    QChar baseSeparator { ' ' };

    if ( !mGeometryColumn.isEmpty() )
    {
      baseSql += QgsSqliteUtils::quotedIdentifier( mGeometryColumn );
      baseValues += geomParam();
      baseSeparator = ',';
    }

    for ( QgsFeatureList::iterator feature = flist.begin(); feature != flist.end(); ++feature )
    {

      QChar separator { baseSeparator };
      QString values { baseValues };
      sql = baseSql;

      // looping on each feature to insert
      QgsAttributes attributevec = feature->attributes();

      // Default indexes (to be skipped)
      QList<int> defaultIndexes;

      for ( int i = 0; i < attributevec.count(); ++i )
      {
        if ( mDefaultValues.contains( i ) && (
               mDefaultValues.value( i ) == attributevec.at( i ).toString() ||
               ! attributevec.at( i ).isValid() ) )
        {
          defaultIndexes.push_back( i );
          continue;
        }

        if ( i >= mAttributeFields.count() )
          continue;

        QString fieldname = mAttributeFields.at( i ).name();
        if ( fieldname.isEmpty() || fieldname == mGeometryColumn )
        {
          continue;
        }

        sql += separator + QgsSqliteUtils::quotedIdentifier( fieldname );
        values += separator + '?';
        separator = ',';
      }

      sql += values;
      sql += ')';

      // SQLite prepared statement
      ret = sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr );
      if ( ret == SQLITE_OK )
      {

        // initializing the column counter
        ia = 0;

        if ( !mGeometryColumn.isEmpty() )
        {
          // binding GEOMETRY to Prepared Statement
          if ( !feature->hasGeometry() )
          {
            sqlite3_bind_null( stmt, ++ia );
          }
          else
          {
            unsigned char *wkb = nullptr;
            int wkb_size;
            const QgsGeometry convertedGeom( QgsVectorDataProvider::convertToProviderType( feature->geometry(), wkbType() ) );
            const QByteArray featureWkb{ !convertedGeom.isNull() ? convertedGeom.asWkb() : feature->geometry().asWkb() };
            convertFromGeosWKB( reinterpret_cast<const unsigned char *>( featureWkb.constData() ),
                                featureWkb.length(),
                                &wkb, &wkb_size, nDims );
            if ( !wkb )
              sqlite3_bind_null( stmt, ++ia );
            else
              sqlite3_bind_blob( stmt, ++ia, wkb, wkb_size, deleteWkbBlob );
          }
        }

        for ( int i = 0; i < attributevec.count(); ++i )
        {
          if ( defaultIndexes.contains( i ) )
          {
            continue;
          }

          const QVariant v = attributevec.at( i );

          // binding values for each attribute
          if ( i >= mAttributeFields.count() )
            break;

          QString fieldname = mAttributeFields.at( i ).name();
          if ( fieldname.isEmpty() || fieldname == mGeometryColumn )
            continue;

          QMetaType::Type type = mAttributeFields.at( i ).type();

          if ( !v.isValid() )
          {
            ++ia;
          }
          else if ( fieldname == mPrimaryKey && mPrimaryKeyAutoIncrement && v == QVariant( tr( "Autogenerate" ) ) )
          {
            // use auto-generated value if user hasn't specified a unique value
            ++ia;
          }
          else if ( QgsVariantUtils::isNull( v ) )
          {
            // binding a NULL value
            sqlite3_bind_null( stmt, ++ia );
          }
          else if ( type == QMetaType::Type::Int )
          {
            // binding an INTEGER value
            sqlite3_bind_int( stmt, ++ia, v.toInt() );
          }
          else if ( type == QMetaType::Type::LongLong )
          {
            // binding a LONGLONG value
            sqlite3_bind_int64( stmt, ++ia, v.toLongLong() );
          }
          else if ( type == QMetaType::Type::Double )
          {
            // binding a DOUBLE value
            sqlite3_bind_double( stmt, ++ia, v.toDouble() );
          }
          else if ( type == QMetaType::Type::QString )
          {
            QString stringVal = v.toString();

            // binding a TEXT value
            QByteArray ba = stringVal.toUtf8();
            sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          }
          else if ( type == QMetaType::Type::QByteArray )
          {
            // binding a BLOB value
            const QByteArray ba = v.toByteArray();
            sqlite3_bind_blob( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          }
          else if ( type == QMetaType::Type::QStringList || type == QMetaType::Type::QVariantList )
          {
            const QByteArray ba = QgsJsonUtils::encodeValue( v ).toUtf8();
            sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          }
          else if ( type == QMetaType::Type::QDateTime )
          {
            QDateTime dt = v.toDateTime();
            QByteArray ba = dt.toString( Qt::ISODate ).toUtf8();
            sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          }
          else if ( type == QMetaType::Type::QDate )
          {
            QDate d = v.toDate();
            QByteArray ba = d.toString( QStringLiteral( "yyyy-MM-dd" ) ).toUtf8();
            sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          }
          else
          {
            // Unknown type: bind a NULL value
            sqlite3_bind_null( stmt, ++ia );
          }
        }

        // performing actual row insert
        ret = sqlite3_step( stmt );

        char *expandedSql = sqlite3_expanded_sql( stmt );
        QgsDatabaseQueryLogWrapper logWrapper( QString( expandedSql ), logUri, QStringLiteral( "spatialite" ), QStringLiteral( "QgsSpatiaLiteProvider" ), QStringLiteral( "addFeatures" ) );
        sqlite3_free( expandedSql );

        sqlite3_finalize( stmt );

        if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
        {
          // update feature id
          if ( !( flags & QgsFeatureSink::FastInsert ) )
          {
            feature->setId( sqlite3_last_insert_rowid( sqliteHandle( ) ) );
          }
          mNumberFeatures++;
        }
        else
        {
          // some unexpected error occurred
          const char *err = sqlite3_errmsg( sqliteHandle( ) );
          errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
          strcpy( errMsg, err );
          logWrapper.setError( errMsg );
          break;
        }
      }
    } // prepared statement

    if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
    {
      ret = exec_sql( sqliteHandle(), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
    }

  } // SAVEPOINT statement

  if ( ret != SQLITE_OK )
  {
    pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ? errMsg : tr( "unknown cause" ) ) );
    if ( errMsg )
    {
      sqlite3_free( errMsg );
    }

    if ( toCommit )
    {
      // ROLLBACK after some previous error
      ( void )exec_sql( sqliteHandle(), QStringLiteral( "ROLLBACK TRANSACTION TO SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), nullptr, QGS_QUERY_LOG_ORIGIN );
      // Also release the savepoint or it will remain on the stack.
      ( void ) exec_sql( sqliteHandle(), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
    }
  }
  else
  {
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }

  return ret == SQLITE_OK;
}

QString QgsSpatiaLiteProvider::createIndexName( QString tableName, QString field )
{
  const thread_local QRegularExpression safeExp( QStringLiteral( "[^a-zA-Z0-9]" ) );
  tableName.replace( safeExp, QStringLiteral( "_" ) );
  field.replace( safeExp, QStringLiteral( "_" ) );
  return QStringLiteral( "%1_%2_idx" ).arg( tableName, field );
}

bool QgsSpatiaLiteProvider::createAttributeIndex( int field )
{
  char *errMsg = nullptr;

  if ( field < 0 || field >= mAttributeFields.count() )
    return false;

  QString sql;
  QString fieldName;

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  int ret = exec_sql( sqliteHandle(), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  fieldName = mAttributeFields.at( field ).name();

  sql = QStringLiteral( "CREATE INDEX IF NOT EXISTS %1 ON \"%2\" (%3)" )
        .arg( createIndexName( mTableName, fieldName ),
              mTableName,
              QgsSqliteUtils::quotedIdentifier( fieldName ) );
  ret = exec_sql( sqliteHandle(), sql, uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  ret = exec_sql( sqliteHandle(), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

Qgis::SpatialIndexPresence QgsSpatiaLiteProvider::hasSpatialIndex() const
{
  QgsDataSourceUri u = uri();
  QgsSpatiaLiteProviderConnection conn( u.uri(), QVariantMap() );
  try
  {
    return conn.spatialIndexExists( u.schema(), u.table(), u.geometryColumn() ) ? Qgis::SpatialIndexPresence::Present : Qgis::SpatialIndexPresence::NotPresent;
  }
  catch ( QgsProviderConnectionException & )
  {
    return Qgis::SpatialIndexPresence::Unknown;
  }
}

bool QgsSpatiaLiteProvider::deleteFeatures( const QgsFeatureIds &id )
{
  sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  QString sql;

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  int ret = exec_sql( sqliteHandle(), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  sql = QStringLiteral( "DELETE FROM %1 WHERE %2=?" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ), QgsSqliteUtils::quotedIdentifier( mPrimaryKey ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ) );
    return false;
  }
  else
  {
    for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
    {
      // looping on each feature to be deleted
      // resetting Prepared Statement and bindings
      sqlite3_reset( stmt );
      sqlite3_clear_bindings( stmt );

      qint64 fid = FID_TO_NUMBER( *it );
      sqlite3_bind_int64( stmt, 1, fid );

      // performing actual row deletion
      ret = sqlite3_step( stmt );
      if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
      {
        mNumberFeatures--;
      }
      else
      {
        // some unexpected error occurred
        const char *err = sqlite3_errmsg( sqliteHandle( ) );
        errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
        strcpy( errMsg, err );
        handleError( sql, errMsg, savepointId );
        sqlite3_finalize( stmt );
        return false;
      }
    }
  }

  sqlite3_finalize( stmt );

  ret = exec_sql( sqliteHandle( ), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

bool QgsSpatiaLiteProvider::truncate()
{
  char *errMsg = nullptr;
  QString sql;

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  int ret = exec_sql( sqliteHandle( ), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  sql = QStringLiteral( "DELETE FROM %1" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );
  ret = exec_sql( sqliteHandle( ), sql, uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  ret = exec_sql( sqliteHandle( ), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

bool QgsSpatiaLiteProvider::addAttributes( const QList<QgsField> &attributes )
{
  char *errMsg = nullptr;
  QString sql;

  if ( attributes.isEmpty() )
    return true;

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  int ret = exec_sql( sqliteHandle( ), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    sql = QStringLiteral( "ALTER TABLE \"%1\" ADD COLUMN \"%2\" %3" )
          .arg( mTableName,
                iter->name(),
                iter->typeName() );
    ret = exec_sql( sqliteHandle( ), sql, uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, savepointId );
      return false;
    }
  }

  ret = exec_sql( sqliteHandle( ), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  gaiaStatisticsInvalidate( sqliteHandle( ), mTableName.toUtf8().constData(), mGeometryColumn.toUtf8().constData() );
  update_layer_statistics( sqliteHandle( ), mTableName.toUtf8().constData(), mGeometryColumn.toUtf8().constData() );

  // reload columns
  loadFields();

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  char *errMsg = nullptr;
  QString sql;

  if ( attr_map.isEmpty() )
    return true;

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  int ret = exec_sql( sqliteHandle( ), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    // Loop over all changed features
    //
    // For each changed feature, create an update string like
    // "UPDATE table SET simple_column=23.5, complex_column=? WHERE primary_key=fid"
    // On any update failure, changes to all features will be rolled back

    QgsFeatureId fid = iter.key();

    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    const QgsAttributeMap &attrs = iter.value();
    if ( attrs.isEmpty() )
      continue;

    QString sql = QStringLiteral( "UPDATE %1 SET " ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );
    bool first = true;

    // keep track of map of parameter index to value
    QMap<int, QVariant> bindings;
    int bind_parameter_idx = 1;

    // cycle through the changed attributes of the feature
    for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
    {
      try
      {
        QgsField fld = field( siter.key() );
        const QVariant &val = siter.value();

        if ( !first )
          sql += ',';
        else
          first = false;

        QMetaType::Type type = fld.type();

        if ( QgsVariantUtils::isNull( val ) )
        {
          // binding a NULL value
          sql += QStringLiteral( "%1=NULL" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ) );
        }
        else if ( type == QMetaType::Type::Int || type == QMetaType::Type::LongLong || type == QMetaType::Type::Double )
        {
          // binding a NUMERIC value
          sql += QStringLiteral( "%1=%2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), val.toString() );
        }
        else if ( type == QMetaType::Type::QStringList || type == QMetaType::Type::QVariantList )
        {
          // binding an array value, parse JSON
          QString jRepr;
          try
          {
            const auto jObj = QgsJsonUtils::jsonFromVariant( val );
            if ( ! jObj.is_array() )
            {
              throw json::parse_error::create( 0, 0, tr( "JSON value must be an array" ).toStdString() );
            }
            jRepr = QString::fromStdString( jObj.dump( ) );
            sql += QStringLiteral( "%1=%2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ),  QgsSqliteUtils::quotedString( jRepr ) );
          }
          catch ( json::exception &ex )
          {
            const auto errM { tr( "Field type is JSON but the value cannot be converted to JSON array: %1" ).arg( ex.what() ) };
            auto msgPtr { static_cast<char *>( sqlite3_malloc( errM.length() + 1 ) ) };
            strcpy( static_cast<char *>( msgPtr ), errM.toStdString().c_str() );
            errMsg = msgPtr;
            handleError( jRepr, errMsg, savepointId );
            return false;
          }
        }
        else if ( type == QMetaType::Type::QByteArray )
        {
          // binding a BLOB value
          sql += QStringLiteral( "%1=?" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ) );
          bindings[ bind_parameter_idx++ ] = val;
        }
        else if ( type == QMetaType::Type::QDateTime )
        {
          sql += QStringLiteral( "%1=%2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), QgsSqliteUtils::quotedString( val.toDateTime().toString( Qt::ISODate ) ) );
        }
        else if ( type == QMetaType::Type::QDate )
        {
          sql += QStringLiteral( "%1=%2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), QgsSqliteUtils::quotedString( val.toDateTime().toString( QStringLiteral( "yyyy-MM-dd" ) ) ) );
        }
        else
        {
          // binding a TEXT value
          sql += QStringLiteral( "%1=%2" ).arg( QgsSqliteUtils::quotedIdentifier( fld.name() ), QgsSqliteUtils::quotedString( val.toString() ) );
        }
      }
      catch ( SLFieldNotFound )
      {
        // Field was missing - shouldn't happen
      }
    }
    sql += QStringLiteral( " WHERE %1=%2" ).arg( QgsSqliteUtils::quotedIdentifier( mPrimaryKey ) ).arg( fid );

    // prepare SQLite statement
    sqlite3_stmt *stmt = nullptr;
    ret = sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( ret != SQLITE_OK )
    {
      // some unexpected error occurred during preparation
      const char *err = sqlite3_errmsg( sqliteHandle( ) );
      errMsg = static_cast<char *>( sqlite3_malloc( strlen( err ) + 1 ) );
      strcpy( errMsg, err );
      handleError( sql, errMsg, savepointId );
      return false;
    }

    // bind variables not handled directly in the string
    for ( auto i = bindings.cbegin(); i != bindings.cend(); ++i )
    {
      int parameter_idx = i.key();
      const QVariant val = i.value();
      switch ( val.userType() )
      {
        case QMetaType::Type::QByteArray:
        {
          const QByteArray ba = val.toByteArray();
          sqlite3_bind_blob( stmt, parameter_idx, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          break;
        }

        default:
          // This will only happen if the above code is changed to bind more types,
          // but the programmer has forgotten to handle the type here. Fatal error.
          sqlite3_finalize( stmt );
          Q_ASSERT( false );
      }

      if ( ret != SQLITE_OK )
      {
        // some unexpected error occurred during binding
        const char *err = sqlite3_errmsg( sqliteHandle( ) );
        errMsg = static_cast<char *>( sqlite3_malloc( strlen( err ) + 1 ) );
        strcpy( errMsg, err );
        handleError( sql, errMsg, savepointId );
        sqlite3_finalize( stmt );
        return false;
      }
    }

    ret = sqlite3_step( stmt );
    sqlite3_finalize( stmt );
    if ( ret != SQLITE_DONE )
    {
      // some unexpected error occurred during execution of update query
      const char *err = sqlite3_errmsg( sqliteHandle( ) );
      errMsg = static_cast<char *>( sqlite3_malloc( strlen( err ) + 1 ) );
      strcpy( errMsg, err );
      handleError( sql, errMsg, savepointId );
      return false;
    }
  }

  ret = exec_sql( sqliteHandle(), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

bool QgsSpatiaLiteProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  QString sql;

  const QString savepointId { QStringLiteral( "qgis_spatialite_internal_savepoint_%1" ).arg( ++ sSavepointId ) };

  int ret = exec_sql( sqliteHandle(), QStringLiteral( "SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  sql =
    QStringLiteral( "UPDATE %1 SET %2=GeomFromWKB(?, %3) WHERE %4=?" )
    .arg( QgsSqliteUtils::quotedIdentifier( mTableName ),
          QgsSqliteUtils::quotedIdentifier( mGeometryColumn ) )
    .arg( mSrid )
    .arg( QgsSqliteUtils::quotedIdentifier( mPrimaryKey ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( sqliteHandle( ) ) ), tr( "SpatiaLite" ) );
  }
  else
  {
    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
    {
      // resetting Prepared Statement and bindings
      sqlite3_reset( stmt );
      sqlite3_clear_bindings( stmt );

      // binding GEOMETRY to Prepared Statement
      unsigned char *wkb = nullptr;
      int wkb_size;
      const QgsGeometry convertedGeom( convertToProviderType( *iter ) );
      const QByteArray iterWkb{ !convertedGeom.isNull() ? convertedGeom.asWkb() : iter->asWkb() };
      convertFromGeosWKB( reinterpret_cast<const unsigned char *>( iterWkb.constData() ), iterWkb.length(), &wkb, &wkb_size, nDims );
      if ( !wkb )
        sqlite3_bind_null( stmt, 1 );
      else
        sqlite3_bind_blob( stmt, 1, wkb, wkb_size, deleteWkbBlob );
      sqlite3_bind_int64( stmt, 2, FID_TO_NUMBER( iter.key() ) );

      // performing actual row update
      ret = sqlite3_step( stmt );
      if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
        ;
      else
      {
        // some unexpected error occurred
        const char *err = sqlite3_errmsg( sqliteHandle( ) );
        errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
        strcpy( errMsg, err );
        handleError( sql, errMsg, savepointId );
        sqlite3_finalize( stmt );
        return false;
      }
    }
  }

  sqlite3_finalize( stmt );

  ret = exec_sql( sqliteHandle(), QStringLiteral( "RELEASE SAVEPOINT \"%1\"" ).arg( savepointId ), uri().uri(), errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, savepointId );
    return false;
  }

  if ( mTransaction )
    mTransaction->dirtyLastSavePoint();

  return true;
}

QgsVectorDataProvider::Capabilities QgsSpatiaLiteProvider::capabilities() const
{
  return mEnabledCapabilities;
}

bool QgsSpatiaLiteProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value ) const
{
  Q_UNUSED( constraint )

  // If the field is the primary key, skip in case it's autogenerated / auto-incrementing
  if ( mAttributeFields.at( fieldIndex ).name() == mPrimaryKey && mPrimaryKeyAutoIncrement )
  {
    const QVariant defVal = mDefaultValues.value( fieldIndex );
    return defVal.toInt() == value.toInt();
  }

  return false;
}

void QgsSpatiaLiteProvider::closeDb()
{
// trying to close the SQLite DB
  if ( mHandle )
  {
    QgsSqliteHandle::closeDb( mHandle );
    mHandle = nullptr;
  }
}

bool QgsSpatiaLiteProvider::checkLayerTypeAbstractInterface( gaiaVectorLayerPtr lyr )
{
  if ( !lyr )
    return false;

  mTableBased = false;
  mViewBased = false;
  mVShapeBased = false;
  mIsQuery = false;
  mReadOnly = false;

  switch ( lyr->LayerType )
  {
    case GAIA_VECTOR_TABLE:
      mTableBased = true;
      break;
    case GAIA_VECTOR_VIEW:
      mViewBased = true;
      break;
    case GAIA_VECTOR_VIRTUAL:
      mVShapeBased = true;
      break;
  }

  if ( lyr->AuthInfos )
  {
    if ( lyr->AuthInfos->IsReadOnly )
      mReadOnly = true;
  }
  else if ( mViewBased )
  {
    mReadOnly = !hasTriggers();
  }

  if ( !mIsQuery )
  {
    mQuery = QgsSqliteUtils::quotedIdentifier( mTableName );
  }

  return true;
}

bool QgsSpatiaLiteProvider::checkLayerType()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int count = 0;

  mTableBased = false;
  mViewBased = false;
  mVShapeBased = false;
  mIsQuery = false;

  QString sql;

  if ( mGeometryColumn.isEmpty() && !( mQuery.startsWith( '(' ) && mQuery.endsWith( ')' ) ) )
  {
    // checking if is a non-spatial table
    sql = QString( "SELECT type FROM sqlite_master "
                   "WHERE lower(name) = lower(%1) "
                   "AND type in ('table', 'view') " ).arg( QgsSqliteUtils::quotedString( mTableName ) );

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      QString type = QString( results[ columns + 0 ] );
      if ( type == QLatin1String( "table" ) )
      {
        mTableBased = true;
        mReadOnly = false;
      }
      else if ( type == QLatin1String( "view" ) )
      {
        mViewBased = true;
        mReadOnly = !hasTriggers();
      }
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = nullptr;
    }
    sqlite3_free_table( results );
  }
  else if ( mQuery.startsWith( '(' ) && mQuery.endsWith( ')' ) )
  {
    // get a new alias for the subquery
    int index = 0;
    QString alias;
    QRegularExpression regex;
    do
    {
      alias = QStringLiteral( "subQuery_%1" ).arg( QString::number( index++ ) );
      QString pattern = QStringLiteral( "(\\\"?)%1\\1" ).arg( QRegularExpression::escape( alias ) );
      regex.setPattern( pattern );
      regex.setPatternOptions( QRegularExpression::CaseInsensitiveOption );
    }
    while ( mQuery.contains( regex ) );

    // convert the custom query into a subquery
    mQuery = QStringLiteral( "%1 as %2" )
             .arg( mQuery,
                   QgsSqliteUtils::quotedIdentifier( alias ) );

    sql = QStringLiteral( "SELECT 0, %1 FROM %2 LIMIT 1" ).arg( QgsSqliteUtils::quotedIdentifier( mGeometryColumn ), mQuery );
    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );

    // Try to find a PK or try to use ROWID
    if ( ret == SQLITE_OK && rows == 1 )
    {
      sqlite3_stmt *stmt = nullptr;

      // 1. find the table that provides geometry
      // String containing the name of the table that provides the geometry if the layer data source is based on a query
      QString queryGeomTableName;
      if ( sqlite3_prepare_v2( sqliteHandle( ), sql.toUtf8().constData(), -1, &stmt, nullptr ) == SQLITE_OK )
      {
        queryGeomTableName = sqlite3_column_table_name( stmt, 1 );
      }

      // 3. Find pks
      QList<QString> pks;
      if ( ! queryGeomTableName.isEmpty() )
      {
        pks = tablePrimaryKeys( queryGeomTableName );
      }

      // find table alias if any
      QString tableAlias;
      if ( ! queryGeomTableName.isEmpty() )
      {
        // Try first with single table alias
        // (I couldn't find a sqlite API call to get this information)
        QRegularExpression re { QStringLiteral( R"re("?%1"?\s+AS\s+(\w+))re" ).arg( queryGeomTableName ) };
        re.setPatternOptions( QRegularExpression::PatternOption::MultilineOption |
                              QRegularExpression::PatternOption::CaseInsensitiveOption );
        QRegularExpressionMatch match { re.match( mTableName ) };
        if ( match.hasMatch() )
        {
          tableAlias = match.captured( 1 );
        }
        // Check if the whole sql is aliased i.e. '(SELECT * FROM \\"somedata\\" as my_alias\n)'
        if ( tableAlias.isEmpty() )
        {
          re.setPattern( QStringLiteral( R"re(\s+AS\s+(\w+)\n?\)?$)re" ) );
          match = re.match( mTableName );
          if ( match.hasMatch() )
          {
            tableAlias = match.captured( 1 );
          }
        }
      }

      const QString tableIdentifier { tableAlias.isEmpty() ? queryGeomTableName : tableAlias };
      QRegularExpression injectionRe { QStringLiteral( R"re(SELECT\s([^\(]+?FROM\s+"?%1"?))re" ).arg( tableIdentifier ) };
      injectionRe.setPatternOptions( QRegularExpression::PatternOption::MultilineOption |
                                     QRegularExpression::PatternOption::CaseInsensitiveOption );


      if ( ! pks.isEmpty() )
      {
        if ( pks.length() > 1 )
        {
          QgsMessageLog::logMessage( tr( "SQLite composite keys are not supported in query layer, using the first component only. %1" )
                                     .arg( sql ), tr( "SpatiaLite" ), Qgis::MessageLevel::Warning );
        }

        // Try first without any injection or manipulation
        sql = QStringLiteral( "SELECT %1, %2 FROM %3 LIMIT 1" ).arg( QgsSqliteUtils::quotedIdentifier( pks.first( ) ), QgsSqliteUtils::quotedIdentifier( mGeometryColumn ), mQuery );
        ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
        if ( ret == SQLITE_OK && rows == 1 )
        {
          mPrimaryKey = pks.first( );
        }
        else // if that does not work, try injection with table name/alias
        {
          QString pk { QStringLiteral( "%1.%2" ).arg( QgsSqliteUtils::quotedIdentifier( alias ) ).arg( pks.first() ) };
          QString newSql( mQuery.replace( injectionRe,
                                          QStringLiteral( R"re(SELECT %1.%2, \1)re" )
                                          .arg( QgsSqliteUtils::quotedIdentifier( tableIdentifier ) )
                                          .arg( pks.first() ) ) );
          sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 1" ).arg( pk ).arg( newSql );
          ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
          if ( ret == SQLITE_OK && rows == 1 )
          {
            mQuery = newSql;
            mPrimaryKey = pks.first( );
          }
        }
      }

      // If there is still no primary key, check if we can get use the ROWID from the table that provides the geometry
      if ( mPrimaryKey.isEmpty() )
      {
        // 4. check if the table has a usable ROWID
        if ( ! queryGeomTableName.isEmpty() )
        {
          sql = QStringLiteral( "SELECT ROWID FROM %1 WHERE ROWID IS NOT NULL LIMIT 1" ).arg( QgsSqliteUtils::quotedIdentifier( queryGeomTableName ) );
          ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
          if ( ret != SQLITE_OK || rows != 1 )
          {
            queryGeomTableName = QString();
          }
        }
        // 5. check if ROWID injection works
        if ( ! queryGeomTableName.isEmpty() )
        {
          const QString newSql( mQuery.replace( injectionRe,
                                                QStringLiteral( R"re(SELECT %1.%2, \1)re" )
                                                .arg( QgsSqliteUtils::quotedIdentifier( tableIdentifier ),
                                                    QStringLiteral( "ROWID" ) ) ) );
          sql = QStringLiteral( "SELECT ROWID FROM %1 WHERE ROWID IS NOT NULL LIMIT 1" ).arg( newSql );
          ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
          if ( ret == SQLITE_OK && rows == 1 )
          {
            mQuery = newSql;
            mPrimaryKey = QStringLiteral( "ROWID" );
            mRowidInjectedInQuery = true;
          }
        }
        // 6. if it does not work, simply clear the message and fallback to the original behavior
        if ( errMsg )
        {
          QgsMessageLog::logMessage( tr( "SQLite error while trying to inject ROWID: %2\nSQL: %1" ).arg( sql, errMsg ), tr( "SpatiaLite" ) );
          sqlite3_free( errMsg );
          errMsg = nullptr;
        }
      }
      sqlite3_finalize( stmt );
      mIsQuery = true;
      mReadOnly = true;
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = nullptr;
    }
    sqlite3_free_table( results );
  }
  else
  {
    // checking if this one is a Table-based layer
    sql = QString( "SELECT read_only FROM geometry_columns "
                   "LEFT JOIN geometry_columns_auth "
                   "USING (f_table_name, f_geometry_column) "
                   "WHERE upper(f_table_name) = upper(%1) and upper(f_geometry_column) = upper(%2)" )
          .arg( QgsSqliteUtils::quotedString( mTableName ),
                QgsSqliteUtils::quotedString( mGeometryColumn ) );

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      if ( errMsg && strcmp( errMsg, "no such table: geometry_columns_auth" ) == 0 )
      {
        sqlite3_free( errMsg );
        sql = QStringLiteral( "SELECT 0 FROM geometry_columns WHERE upper(f_table_name) = upper(%1) and upper(f_geometry_column) = upper(%2)" )
              .arg( QgsSqliteUtils::quotedString( mTableName ),
                    QgsSqliteUtils::quotedString( mGeometryColumn ) );
        ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      }
    }
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mTableBased = true;
      mReadOnly = false;
      for ( i = 1; i <= rows; i++ )
      {
        if ( results[( i * columns ) + 0] )
        {
          if ( atoi( results[( i * columns ) + 0] ) != 0 )
            mReadOnly = true;
        }
      }
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = nullptr;
    }
    sqlite3_free_table( results );

    // checking if this one is a View-based layer
    sql = QString( "SELECT view_name, view_geometry FROM views_geometry_columns"
                   " WHERE view_name=%1 and view_geometry=%2" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                       QgsSqliteUtils::quotedString( mGeometryColumn ) );

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mViewBased = true;
      mReadOnly = !hasTriggers();
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = nullptr;
    }
    sqlite3_free_table( results );

    // checking if this one is a VirtualShapefile-based layer
    sql = QString( "SELECT virt_name, virt_geometry FROM virts_geometry_columns"
                   " WHERE virt_name=%1 and virt_geometry=%2" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                       QgsSqliteUtils::quotedString( mGeometryColumn ) );

    ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mVShapeBased = true;
      mReadOnly = true;
      count++;
    }
    if ( errMsg )
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errMsg ), tr( "SpatiaLite" ) );
      sqlite3_free( errMsg );
      errMsg = nullptr;
    }
    sqlite3_free_table( results );
  }

  if ( !mIsQuery )
  {
    mQuery = QgsSqliteUtils::quotedIdentifier( mTableName );
  }

// checking for validity
  return count == 1;
}

bool QgsSpatiaLiteProvider::getGeometryDetailsAbstractInterface( gaiaVectorLayerPtr lyr )
{
  if ( !lyr )
  {
    return false;
  }

  mIndexTable = mTableName;
  mIndexGeometry = mGeometryColumn;

  switch ( lyr->GeometryType )
  {
    case GAIA_VECTOR_POINT:
      mGeomType = Qgis::WkbType::Point;
      break;
    case GAIA_VECTOR_LINESTRING:
      mGeomType = Qgis::WkbType::LineString;
      break;
    case GAIA_VECTOR_POLYGON:
      mGeomType = Qgis::WkbType::Polygon;
      break;
    case GAIA_VECTOR_MULTIPOINT:
      mGeomType = Qgis::WkbType::MultiPoint;
      break;
    case GAIA_VECTOR_MULTILINESTRING:
      mGeomType = Qgis::WkbType::MultiLineString;
      break;
    case GAIA_VECTOR_MULTIPOLYGON:
      mGeomType = Qgis::WkbType::MultiPolygon;
      break;
    default:
      mGeomType = Qgis::WkbType::Unknown;
      break;
  }

  mSrid = lyr->Srid;
  if ( lyr->SpatialIndex == GAIA_SPATIAL_INDEX_RTREE )
  {
    mSpatialIndexRTree = true;
  }
  if ( lyr->SpatialIndex == GAIA_SPATIAL_INDEX_MBRCACHE )
  {
    mSpatialIndexMbrCache = true;
  }
  switch ( lyr->Dimensions )
  {
    case GAIA_XY:
      nDims = GAIA_XY;
      break;
    case GAIA_XY_Z:
      nDims = GAIA_XY_Z;
      mGeomType = QgsWkbTypes::addZ( mGeomType );
      break;
    case GAIA_XY_M:
      nDims = GAIA_XY_M;
      mGeomType = QgsWkbTypes::addM( mGeomType );
      break;
    case GAIA_XY_Z_M:
      nDims = GAIA_XY_Z_M;
      mGeomType = QgsWkbTypes::zmType( mGeomType, true, true );
      break;
  }

  if ( mViewBased && mSpatialIndexRTree )
    getViewSpatialIndexName();

  return getSridDetails();
}

void QgsSpatiaLiteProvider::getViewSpatialIndexName()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  // retrieving the Spatial Index name supporting this View (if any)
  mSpatialIndexRTree = false;

  QString sql = QString( "SELECT f_table_name, f_geometry_column "
                         "FROM views_geometry_columns "
                         "WHERE upper(view_name) = upper(%1) and upper(view_geometry) = upper(%2)" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                             QgsSqliteUtils::quotedString( mGeometryColumn ) );
  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      mIndexTable = QString::fromUtf8( ( const char * ) results[( i * columns ) + 0] );
      mIndexGeometry = QString::fromUtf8( ( const char * ) results[( i * columns ) + 1] );
      mSpatialIndexRTree = true;
    }
  }
  sqlite3_free_table( results );
}

bool QgsSpatiaLiteProvider::getGeometryDetails()
{
  bool ret = false;
  if ( mGeometryColumn.isEmpty() )
  {
    mGeomType = Qgis::WkbType::NoGeometry;
    return true;
  }

  if ( mTableBased )
    ret = getTableGeometryDetails();
  if ( mViewBased )
    ret = getViewGeometryDetails();
  if ( mVShapeBased )
    ret = getVShapeGeometryDetails();
  if ( mIsQuery )
    ret = getQueryGeometryDetails();
  return ret;
}

bool QgsSpatiaLiteProvider::getTableGeometryDetails()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  mIndexTable = mTableName;
  mIndexGeometry = mGeometryColumn;

  QString sql;
  if ( ! versionIsAbove( sqliteHandle( ), 3, 1 ) )
  {
    sql = QString( "SELECT type, srid, spatial_index_enabled, coord_dimension FROM geometry_columns"
                   " WHERE upper(f_table_name) = upper(%1) and upper(f_geometry_column) = upper(%2)" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                       QgsSqliteUtils::quotedString( mGeometryColumn ) );
  }
  else
  {
    sql = QString( "SELECT geometry_type, srid, spatial_index_enabled, coord_dimension FROM geometry_columns"
                   " WHERE upper(f_table_name) = upper(%1) and upper(f_geometry_column) = upper(%2)" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                       QgsSqliteUtils::quotedString( mGeometryColumn ) );
  }

  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString fType = results[( i * columns ) + 0];
      QString xSrid = results[( i * columns ) + 1];
      QString spatialIndex = results[( i * columns ) + 2];
      QString dims = results[( i * columns ) + 3];

      if ( fType == QLatin1String( "POINT" ) || fType == QLatin1String( "1" ) )
      {
        mGeomType = Qgis::WkbType::Point;
      }
      else if ( fType == QLatin1String( "MULTIPOINT" ) || fType == QLatin1String( "4" ) )
      {
        mGeomType = Qgis::WkbType::MultiPoint;
      }
      else if ( fType == QLatin1String( "LINESTRING" ) || fType == QLatin1String( "2" ) )
      {
        mGeomType = Qgis::WkbType::LineString;
      }
      else if ( fType == QLatin1String( "MULTILINESTRING" ) || fType == QLatin1String( "5" ) )
      {
        mGeomType = Qgis::WkbType::MultiLineString;
      }
      else if ( fType == QLatin1String( "POLYGON" )  || fType == QLatin1String( "3" ) )
      {
        mGeomType = Qgis::WkbType::Polygon;
      }
      else if ( fType == QLatin1String( "MULTIPOLYGON" )  || fType == QLatin1String( "6" ) )
      {
        mGeomType = Qgis::WkbType::MultiPolygon;
      }

      mSrid = xSrid.toInt();
      if ( spatialIndex.toInt() == 1 )
      {
        mSpatialIndexRTree = true;
      }
      if ( spatialIndex.toInt() == 2 )
      {
        mSpatialIndexMbrCache = true;
      }
      if ( dims == QLatin1String( "XY" ) || dims == QLatin1String( "2" ) )
      {
        nDims = GAIA_XY;
      }
      else if ( dims == QLatin1String( "XYZ" ) || dims == QLatin1String( "3" ) )
      {
        nDims = GAIA_XY_Z;
        mGeomType = QgsWkbTypes::addZ( mGeomType );
      }
      else if ( dims == QLatin1String( "XYM" ) )
      {
        nDims = GAIA_XY_M;
        mGeomType = QgsWkbTypes::addM( mGeomType );
      }
      else if ( dims == QLatin1String( "XYZM" ) || dims == QLatin1String( "4" ) )
      {
        nDims = GAIA_XY_Z_M;
        mGeomType = QgsWkbTypes::zmType( mGeomType, true, true );
      }

    }
  }
  sqlite3_free_table( results );

  if ( mGeomType == Qgis::WkbType::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  return getSridDetails();
}

bool QgsSpatiaLiteProvider::getViewGeometryDetails()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  QString sql = QString( "SELECT type, srid, spatial_index_enabled, f_table_name, f_geometry_column "
                         " FROM views_geometry_columns"
                         " JOIN geometry_columns USING (f_table_name, f_geometry_column)"
                         " WHERE upper(view_name) = upper(%1) and upper(view_geometry) = upper(%2)" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                             QgsSqliteUtils::quotedString( mGeometryColumn ) );

  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString fType = results[( i * columns ) + 0];
      QString xSrid = results[( i * columns ) + 1];
      QString spatialIndex = results[( i * columns ) + 2];
      mIndexTable = QString::fromUtf8( ( const char * ) results[( i * columns ) + 3] );
      mIndexGeometry = QString::fromUtf8( ( const char * ) results[( i * columns ) + 4] );

      if ( fType == QLatin1String( "POINT" ) )
      {
        mGeomType = Qgis::WkbType::Point;
      }
      else if ( fType == QLatin1String( "MULTIPOINT" ) )
      {
        mGeomType = Qgis::WkbType::MultiPoint;
      }
      else if ( fType == QLatin1String( "LINESTRING" ) )
      {
        mGeomType = Qgis::WkbType::LineString;
      }
      else if ( fType == QLatin1String( "MULTILINESTRING" ) )
      {
        mGeomType = Qgis::WkbType::MultiLineString;
      }
      else if ( fType == QLatin1String( "POLYGON" ) )
      {
        mGeomType = Qgis::WkbType::Polygon;
      }
      else if ( fType == QLatin1String( "MULTIPOLYGON" ) )
      {
        mGeomType = Qgis::WkbType::MultiPolygon;
      }
      mSrid = xSrid.toInt();
      if ( spatialIndex.toInt() == 1 )
      {
        mSpatialIndexRTree = true;
      }
      if ( spatialIndex.toInt() == 2 )
      {
        mSpatialIndexMbrCache = true;
      }

    }
  }
  sqlite3_free_table( results );

  if ( mGeomType == Qgis::WkbType::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  return getSridDetails();
}

bool QgsSpatiaLiteProvider::getVShapeGeometryDetails()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  QString sql = QString( "SELECT type, srid FROM virts_geometry_columns"
                         " WHERE virt_name=%1 and virt_geometry=%2" ).arg( QgsSqliteUtils::quotedString( mTableName ),
                             QgsSqliteUtils::quotedString( mGeometryColumn ) );

  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString fType = results[( i * columns ) + 0];
      QString xSrid = results[( i * columns ) + 1];

      if ( fType == QLatin1String( "POINT" ) )
      {
        mGeomType = Qgis::WkbType::Point;
      }
      else if ( fType == QLatin1String( "MULTIPOINT" ) )
      {
        mGeomType = Qgis::WkbType::MultiPoint;
      }
      else if ( fType == QLatin1String( "LINESTRING" ) )
      {
        mGeomType = Qgis::WkbType::LineString;
      }
      else if ( fType == QLatin1String( "MULTILINESTRING" ) )
      {
        mGeomType = Qgis::WkbType::MultiLineString;
      }
      else if ( fType == QLatin1String( "POLYGON" ) )
      {
        mGeomType = Qgis::WkbType::Polygon;
      }
      else if ( fType == QLatin1String( "MULTIPOLYGON" ) )
      {
        mGeomType = Qgis::WkbType::MultiPolygon;
      }
      mSrid = xSrid.toInt();

    }
  }
  sqlite3_free_table( results );

  if ( mGeomType == Qgis::WkbType::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  return getSridDetails();
}

bool QgsSpatiaLiteProvider::getQueryGeometryDetails()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  QString fType;
  QString xSrid;

  // get stuff from the relevant column instead. This may (will?)
  // fail if there is no data in the relevant table.
  QString sql = QStringLiteral( "SELECT srid(%1), geometrytype(%1) FROM %2" )
                .arg( QgsSqliteUtils::quotedIdentifier( mGeometryColumn ),
                      mQuery );

  //it is possible that the where clause restricts the feature type
  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mSubsetString;
  }

  sql += QLatin1String( " limit 1" );

  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      xSrid = results[( i * columns ) + 0];
      fType = results[( i * columns ) + 1];
    }
  }
  sqlite3_free_table( results );

  if ( !xSrid.isEmpty() && !fType.isEmpty() )
  {
    if ( fType == QLatin1String( "GEOMETRY" ) )
    {
      // check to see if there is a unique geometry type
      sql = QString( "SELECT DISTINCT "
                     "CASE"
                     " WHEN geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
                     " WHEN geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
                     " WHEN geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
                     " END "
                     "FROM %2" )
            .arg( QgsSqliteUtils::quotedIdentifier( mGeometryColumn ),
                  mQuery );

      if ( !mSubsetString.isEmpty() )
        sql += " where " + mSubsetString;

      ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      if ( ret != SQLITE_OK )
      {
        handleError( sql, errMsg, QString() );
        return false;
      }

      if ( rows != 1 )
        ;
      else
      {
        for ( i = 1; i <= rows; i++ )
        {
          fType = results[( 1 * columns ) + 0];
        }
      }
      sqlite3_free_table( results );
    }

    mGeomType = QgsWkbTypes::parseType( fType );
    mSrid = xSrid.toInt();
  }

  if ( mGeomType == Qgis::WkbType::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  return getSridDetails();
}

bool QgsSpatiaLiteProvider::getSridDetails()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  QString sql = QStringLiteral( "SELECT auth_name||':'||auth_srid,proj4text FROM spatial_ref_sys WHERE srid=%1" ).arg( mSrid );

  ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      mAuthId    = results[( i * columns ) + 0];
      mProj4text = results[( i * columns ) + 1];
    }
  }
  sqlite3_free_table( results );

  return true;
}

bool QgsSpatiaLiteProvider::getTableSummaryAbstractInterface( gaiaVectorLayerPtr lyr )
{
  if ( !lyr )
    return false;

  if ( lyr->ExtentInfos )
  {
    mLayerExtent = QgsBox3D( lyr->ExtentInfos->MinX, lyr->ExtentInfos->MinY, std::numeric_limits<double>::quiet_NaN(),
                             lyr->ExtentInfos->MaxX, lyr->ExtentInfos->MaxY, std::numeric_limits<double>::quiet_NaN() );
    // This can be wrong! see: GH #29264
    // mNumberFeatures = lyr->ExtentInfos->Count;
    // Note: the unique ptr here does not own the handle, it is just used for the convenience
    //       methods available within the class.
    sqlite3_database_unique_ptr slPtr;
    slPtr.reset( sqliteHandle() );
    int resultCode;
    sqlite3_statement_unique_ptr stmt { slPtr.prepare( QStringLiteral( "SELECT COUNT(1) FROM %2" ).arg( mQuery ), resultCode )};
    if ( resultCode == SQLITE_OK )
    {
      stmt.step();
      mNumberFeatures = sqlite3_column_int64( stmt.get(), 0 );
    }
    // Note: the pointer handle is owned by the provider, releasing it
    slPtr.release();
  }
  else
  {
    mLayerExtent.setNull();
    mNumberFeatures = 0;
  }

  return true;
}

bool QgsSpatiaLiteProvider::getTableSummary()
{
  mLayerExtent.setNull();

  QString sql = QStringLiteral( "SELECT Count(1)" );

  if ( ! mGeometryColumn.isEmpty() )
  {
    sql += QStringLiteral(
             ", Min(MbrMinX(%1)), Min(MbrMinY(%1)), Min(ST_MinZ(%1)), Max(MbrMaxX(%1)), Max(MbrMaxY(%1)), Max(ST_MaxZ(%1))"
           ).arg( QgsSqliteUtils::quotedIdentifier( mGeometryColumn ) );
  }

  sql += QStringLiteral( " FROM %1" ) .arg( mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ')';
  }

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  int ret = sqlite3_get_table( sqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, QString() );
    return false;
  }

  if ( rows != 1 )
  {
    QgsMessageLog::logMessage(
      tr( "Spatialite: unexpected number of rows (%1) from aggregate query\nSQL: %2" )
      .arg( rows ) .arg( sql )
    );
    // TODO: should we ROLLBACK ?
    sqlite3_free_table( results );
    return false;
  }

  // the first row of results contains the column names, so we skip that
  QString count = results[columns + 0];
  mNumberFeatures = count.toLongLong();

  if ( mNumberFeatures && ! mGeometryColumn.isEmpty() ) do
    {
      const QString minX = results[columns + 1]; if ( minX.isEmpty() ) break;
      const QString minY = results[columns + 2]; if ( minY.isEmpty() ) break;
      const QString minZ = results[columns + 3];
      const QString maxX = results[columns + 4]; if ( maxX.isEmpty() ) break;
      const QString maxY = results[columns + 5]; if ( maxY.isEmpty() ) break;
      const QString maxZ = results[columns + 6];

      if ( nDims == GAIA_XY || nDims == GAIA_XY_M || minZ.isEmpty() || maxZ.isEmpty() )
      {
        mLayerExtent = QgsBox3D( minX.toDouble(), minY.toDouble(), std::numeric_limits<double>::quiet_NaN(),
                                 maxX.toDouble(), maxY.toDouble(), std::numeric_limits<double>::quiet_NaN() );
      }
      else
      {
        mLayerExtent = QgsBox3D( minX.toDouble(), minY.toDouble(), minZ.toDouble(),
                                 maxX.toDouble(), maxY.toDouble(), maxZ.toDouble() );
      }
    }
    while ( 0 );

  sqlite3_free_table( results );
  return true;
}

QgsField QgsSpatiaLiteProvider::field( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "SpatiaLite" ) );
    throw SLFieldNotFound();
  }

  return mAttributeFields.at( index );
}

void QgsSpatiaLiteProvider::invalidateConnections( const QString &connection )
{
  QgsSpatiaLiteConnPool::instance()->invalidateConnections( connection );
}

QVariantMap QgsSpatiaLiteProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "path" ), dsUri.database() );
  components.insert( QStringLiteral( "layerName" ), dsUri.table() );
  if ( !dsUri.sql().isEmpty() )
    components.insert( QStringLiteral( "subset" ), dsUri.sql() );
  if ( !dsUri.geometryColumn().isEmpty() )
    components.insert( QStringLiteral( "geometryColumn" ), dsUri.geometryColumn() );
  if ( !dsUri.keyColumn().isEmpty() )
    components.insert( QStringLiteral( "keyColumn" ), dsUri.keyColumn() );
  return components;
}

QgsSpatiaLiteProvider *QgsSpatiaLiteProviderMetadata::createProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
{
  return new QgsSpatiaLiteProvider( uri, options, flags );
}

QList<Qgis::LayerType> QgsSpatiaLiteProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}

QString QgsSpatiaLiteProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setDatabase( parts.value( QStringLiteral( "path" ) ).toString() );
  dsUri.setTable( parts.value( QStringLiteral( "layerName" ) ).toString() );
  dsUri.setSql( parts.value( QStringLiteral( "subset" ) ).toString() );
  dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometryColumn" ) ).toString() );
  dsUri.setKeyColumn( parts.value( QStringLiteral( "keyColumn" ) ).toString() );
  return dsUri.uri();
}

QString QgsSpatiaLiteProviderMetadata::absoluteToRelativeUri( const QString &src, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri uri( src );
  QString database = context.pathResolver().writePath( uri.database() );
  uri.setConnection( uri.host(), uri.port(), database, uri.username(), uri.password() );
  return uri.uri();
}

QString QgsSpatiaLiteProviderMetadata::relativeToAbsoluteUri( const QString &src, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri uri( src );
  uri.setDatabase( context.pathResolver().readPath( uri.database() ) );
  return uri.uri();
}

QgsProviderMetadata::ProviderCapabilities QgsSpatiaLiteProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}


Qgis::VectorExportResult QgsSpatiaLiteProviderMetadata::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    Qgis::WkbType wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> &oldToNewAttrIdxMap,
    QString &errorMessage,
    const QMap<QString, QVariant> *options )
{
  return QgsSpatiaLiteProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           &oldToNewAttrIdxMap, &errorMessage, options
         );
}

bool QgsSpatiaLiteProviderMetadata::createDb( const QString &dbPath, QString &errCause )
{
  return SpatiaLiteUtils::createDb( dbPath, errCause );
}

// -------------

QgsAttributeList QgsSpatiaLiteProvider::pkAttributeIndexes() const
{
  return mPrimaryKeyAttrs;
}

QList<QgsVectorLayer *> QgsSpatiaLiteProvider::searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  for ( auto *layer : layers )
  {
    const QgsSpatiaLiteProvider *slProvider = qobject_cast<QgsSpatiaLiteProvider *>( layer->dataProvider() );
    if ( slProvider && slProvider->mSqlitePath == connectionInfo && slProvider->mTableName == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}

void QgsSpatiaLiteProvider::setTransaction( QgsTransaction *transaction )
{
  QgsDebugMsgLevel( QStringLiteral( "set transaction %1" ).arg( transaction != nullptr ), 2 );
  // static_cast since layers cannot be added to a transaction of a non-matching provider
  mTransaction = static_cast<QgsSpatiaLiteTransaction *>( transaction );
}

Qgis::VectorLayerTypeFlags QgsSpatiaLiteProvider::vectorLayerTypeFlags() const
{
  Qgis::VectorLayerTypeFlags flags;
  if ( mValid && mIsQuery )
  {
    flags.setFlag( Qgis::VectorLayerTypeFlag::SqlQuery );
  }
  return flags;
}

QgsTransaction *QgsSpatiaLiteProvider::transaction( ) const
{
  return static_cast<QgsTransaction *>( mTransaction );
}

QList<QgsRelation> QgsSpatiaLiteProvider::discoverRelations( const QgsVectorLayer *target, const QList<QgsVectorLayer *> &layers ) const
{
  QList<QgsRelation> output;
  const QString sql = QStringLiteral( "PRAGMA foreign_key_list(%1)" ).arg( QgsSqliteUtils::quotedIdentifier( mTableName ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle( ), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret == SQLITE_OK )
  {
    int nbFound = 0;
    for ( int row = 1; row <= rows; ++row )
    {
      const QString name = "fk_" + mTableName + "_" + QString::fromUtf8( results[row * columns + 0] );
      const QString position = QString::fromUtf8( results[row * columns + 1] );
      const QString refTable = QString::fromUtf8( results[row * columns + 2] );
      const QString fkColumn = QString::fromUtf8( results[row * columns + 3] );
      const QString refColumn = QString::fromUtf8( results[row * columns + 4] );
      if ( position == QLatin1String( "0" ) )
      {
        // first reference field => try to find if we have layers for the referenced table
        const QList<QgsVectorLayer *> foundLayers = searchLayers( layers, mSqlitePath, refTable );
        for ( const auto *foundLayer : foundLayers )
        {
          QgsRelation relation;
          relation.setName( name );
          relation.setReferencingLayer( target->id() );
          relation.setReferencedLayer( foundLayer->id() );
          relation.addFieldPair( fkColumn, refColumn );
          relation.generateId();
          if ( relation.isValid() )
          {
            output.append( relation );
            ++nbFound;
          }
          else
          {
            QgsLogger::warning( "Invalid relation for " + name );
          }
        }
      }
      else
      {
        // multi reference field => add the field pair to all the referenced layers found
        for ( int i = 0; i < nbFound; ++i )
        {
          output[output.size() - 1 - i].addFieldPair( fkColumn, refColumn );
        }
      }
    }
    sqlite3_free_table( results );
  }
  else
  {
    QgsLogger::warning( QStringLiteral( "SQLite error discovering relations: %1" ).arg( errMsg ) );
    sqlite3_free( errMsg );
  }
  return output;
}

// ---------------------------------------------------------------------------

bool QgsSpatiaLiteProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  errorCause.clear();
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    errorCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  sqlite3 *sqliteHandle = handle->handle();

  char **results = nullptr;

  // check if layer_styles table exists
  QString countIfExist = QStringLiteral( "SELECT 1 FROM sqlite_master WHERE type='table' AND name='layer_styles';" );

  int rows = 0;
  int columns = 0;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, countIfExist.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( countIfExist ) );
    errorCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }
  if ( rows == 0 )
  {
    // layer_styles table does not exist
    return false;
  }

  const QString checkQuery = QString( "SELECT styleName"
                                      " FROM layer_styles"
                                      " WHERE f_table_schema %1"
                                      " AND f_table_name=%2"
                                      " AND f_geometry_column=%3"
                                      " AND styleName=%4" )
                             .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
                             .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                             .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) )
                             .arg( QgsSqliteUtils::quotedString( styleId.isEmpty() ? dsUri.table() : styleId ) );


  ret = sqlite3_get_table( sqliteHandle, checkQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );

  QString sqlError;
  if ( errMsg )
  {
    sqlError = errMsg;
    sqlite3_free( errMsg );
  }
  QgsSqliteHandle::closeDb( handle );

  if ( SQLITE_OK != ret )
  {
    errorCause = QObject::tr( "Error executing query: %1" ).arg( sqlError );
    return false;
  }
  else
  {
    return rows > 0;
  }
}

bool QgsSpatiaLiteProviderMetadata::saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
    const QString &styleName, const QString &styleDescription,
    const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( "Database is: " + sqlitePath, 2 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugError( QStringLiteral( "Connection to database failed. Save style aborted." ) );
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  sqlite3 *sqliteHandle = handle->handle();

  // check if layer_styles table already exist
  QString countIfExist = QStringLiteral( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1';" ).arg( QLatin1String( "layer_styles" ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, countIfExist.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( countIfExist ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  // if not table exist... create is
  int howMany = 0;
  if ( 1 == rows )
  {
    howMany = atoi( results[( rows * columns ) + 0 ] );
  }
  sqlite3_free_table( results );

  // create table if not exist
  if ( 0 == howMany )
  {

    QString createQuery = QString( "CREATE TABLE layer_styles("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT"
                                   ",f_table_catalog varchar(256)"
                                   ",f_table_schema varchar(256)"
                                   ",f_table_name varchar(256)"
                                   ",f_geometry_column varchar(256)"
                                   ",styleName text"
                                   ",styleQML text"
                                   ",styleSLD text"
                                   ",useAsDefault boolean"
                                   ",description text"
                                   ",owner varchar(30)"
                                   ",ui text"
                                   ",update_time timestamp DEFAULT CURRENT_TIMESTAMP"
                                   ")" );
    ret = QgsSpatiaLiteProvider::exec_sql( sqliteHandle,  createQuery.toUtf8().constData(), uri, errMsg, QGS_QUERY_LOG_ORIGIN );
    if ( SQLITE_OK != ret )
    {
      QgsSqliteHandle::closeDb( handle );
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database." );
      return false;
    }
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = QStringLiteral( ",ui" );
    uiFileValue = QStringLiteral( ",%1" ).arg( QgsSqliteUtils::quotedString( uiFileContent ) );
  }

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%11"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10%12"
                         ")" )
                .arg( QgsSqliteUtils::quotedString( QString() ) )
                .arg( QgsSqliteUtils::quotedString( dsUri.schema() ) )
                .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) )
                .arg( QgsSqliteUtils::quotedString( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( QgsSqliteUtils::quotedString( qmlStyle ) )
                .arg( QgsSqliteUtils::quotedString( sldStyle ) )
                .arg( useAsDefault ? "1" : "0" )
                .arg( QgsSqliteUtils::quotedString( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( QgsSqliteUtils::quotedString( dsUri.username() ) )
                .arg( uiFileColumn )
                .arg( uiFileValue );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_schema %1"
                                " AND f_table_name=%2"
                                " AND f_geometry_column=%3"
                                " AND styleName=%4" )
                       .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
                       .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                       .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) )
                       .arg( QgsSqliteUtils::quotedString( styleName.isEmpty() ? dsUri.table() : styleName ) );

  ret = sqlite3_get_table( sqliteHandle, checkQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( checkQuery ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  if ( 0 != rows )
  {
    sqlite3_free_table( results );
    sql = QString( "UPDATE layer_styles"
                   " SET useAsDefault=%1"
                   ",styleQML=%2"
                   ",styleSLD=%3"
                   ",description=%4"
                   ",owner=%5"
                   " WHERE f_table_schema %6"
                   " AND f_table_name=%7"
                   " AND f_geometry_column=%8"
                   " AND styleName=%9" )
          .arg( useAsDefault ? "1" : "0" )
          .arg( QgsSqliteUtils::quotedString( qmlStyle ) )
          .arg( QgsSqliteUtils::quotedString( sldStyle ) )
          .arg( QgsSqliteUtils::quotedString( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( QgsSqliteUtils::quotedString( dsUri.username() ) )
          .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
          .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
          .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) )
          .arg( QgsSqliteUtils::quotedString( styleName.isEmpty() ? dsUri.table() : styleName ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=0"
                                        " WHERE f_table_schema %1"
                                        " AND f_table_name=%2"
                                        " AND f_geometry_column=%3" )
                               .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
                               .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                               .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) );
    sql = QStringLiteral( "BEGIN; %1; %2; COMMIT;" ).arg( removeDefaultSql, sql );
  }

  ret = QgsSpatiaLiteProvider::exec_sql( sqliteHandle,  sql.toUtf8().constData(), uri, errMsg, QGS_QUERY_LOG_ORIGIN );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( sql ) );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return false;
  }

  if ( errMsg )
    sqlite3_free( errMsg );

  QgsSqliteHandle::closeDb( handle );
  return true;
}

QString QgsSpatiaLiteProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QString styleName;
  return loadStoredStyle( uri, styleName, errCause );
}

QString QgsSpatiaLiteProviderMetadata::loadStoredStyle( const QString &uri, QString &styleName, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( "Database is: " + sqlitePath, 5 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugError( QStringLiteral( "Connection to database failed. Save style aborted." ) );
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString geomColumnExpr;
  if ( dsUri.geometryColumn().isEmpty() )
  {
    geomColumnExpr = QStringLiteral( "IS NULL" );
  }
  else
  {
    geomColumnExpr = QStringLiteral( "=" ) + QgsSqliteUtils::quotedString( dsUri.geometryColumn() );
  }

  QString selectQmlQuery = QString( "SELECT styleName, styleQML"
                                    " FROM layer_styles"
                                    " WHERE f_table_schema %1"
                                    " AND f_table_name=%2"
                                    " AND f_geometry_column %3"
                                    " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                                    ",update_time DESC LIMIT 1" )
                           .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
                           .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                           .arg( geomColumnExpr );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, selectQmlQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    errCause = QObject::tr( "Error executing loading style. The query was logged" );
    return QString();
  }

  styleName = ( rows == 1 ) ? QString::fromUtf8( results[( rows * columns ) + 0 ] ) : QString();
  QString style = ( rows == 1 ) ? QString::fromUtf8( results[( rows * columns ) + 1 ] ) : QString();
  sqlite3_free_table( results );

  QgsSqliteHandle::closeDb( handle );
  return style;
}

int QgsSpatiaLiteProviderMetadata::listStyles( const QString &uri, QStringList &ids, QStringList &names,
    QStringList &descriptions, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( "Database is: " + sqlitePath, 2 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugError( QStringLiteral( "Connection to database failed. Save style aborted." ) );
    errCause = QObject::tr( "Connection to database failed" );
    return -1;
  }

  sqlite3 *sqliteHandle = handle->handle();

  // check if layer_styles table already exist
  QString countIfExist = QStringLiteral( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1';" ).arg( QLatin1String( "layer_styles" ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, countIfExist.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    errCause = QObject::tr( "Error looking for style. The query was logged" );
    return -1;
  }

  int howMany = 0;
  if ( 1 == rows )
  {
    howMany = atoi( results[( rows * columns ) + 0 ] );
  }
  sqlite3_free_table( results );

  if ( 0 == howMany )
  {
    QgsSqliteHandle::closeDb( handle );
    errCause = QObject::tr( "No styles available on DB" );
    return false;
  }

  // get them
  QString selectRelatedQuery = QStringLiteral( "SELECT id,styleName,description"
                               " FROM layer_styles"
                               " WHERE f_table_schema %1"
                               " AND f_table_name=%2"
                               " AND f_geometry_column=%3"
                               " ORDER BY useasdefault DESC, update_time DESC" )
                               .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
                               .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                               .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) );

  ret = sqlite3_get_table( sqliteHandle, selectRelatedQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    errCause = QObject::tr( "Error loading styles. The query was logged" );
    return -1;
  }

  int numberOfRelatedStyles = rows;
  for ( int i = 1; i <= rows; i++ )
  {
    ids.append( results[( i * columns ) + 0 ] );
    names.append( QString::fromUtf8( results[( i * columns ) + 1 ] ) );
    descriptions.append( QString::fromUtf8( results[( i * columns ) + 2 ] ) );
  }
  sqlite3_free_table( results );

  QString selectOthersQuery = QStringLiteral( "SELECT id,styleName,description"
                              " FROM layer_styles"
                              " WHERE NOT (f_table_schema %1 AND f_table_name=%2 AND f_geometry_column=%3)"
                              " ORDER BY update_time DESC" )
                              .arg( QgsSpatiaLiteProvider::tableSchemaCondition( dsUri ) )
                              .arg( QgsSqliteUtils::quotedString( dsUri.table() ) )
                              .arg( QgsSqliteUtils::quotedString( dsUri.geometryColumn() ) );

  ret = sqlite3_get_table( sqliteHandle, selectOthersQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    errCause = QObject::tr( "Error executing the select query for unrelated styles. The query was logged" );
    return -1;
  }

  for ( int i = 1; i <= rows; i++ )
  {
    ids.append( results[( i * columns ) + 0 ] );
    names.append( QString::fromUtf8( results[( i * columns ) + 1 ] ) );
    descriptions.append( QString::fromUtf8( results[( i * columns ) + 2 ] ) );
  }
  sqlite3_free_table( results );

  QgsSqliteHandle::closeDb( handle );
  return numberOfRelatedStyles;
}

QString QgsSpatiaLiteProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( "Database is: " + sqlitePath, 2 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugError( QStringLiteral( "Connection to database failed. Save style aborted." ) );
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString style;
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsSqliteUtils::quotedString( styleId ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, selectQmlQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK == ret )
  {
    if ( 1 == rows )
      style = QString::fromUtf8( results[( rows * columns ) + 0 ] );
    else
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" ).arg( QLatin1String( "layer_styles" ) );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Style with id %1 not found in %2 (Query: %3)" ).arg( styleId, sqlitePath, selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  QgsSqliteHandle::closeDb( handle );
  sqlite3_free_table( results );
  return style;
}

void QgsSpatiaLiteProviderMetadata::cleanupProvider()
{
  QgsSpatiaLiteConnPool::cleanupInstance();
  QgsSqliteHandle::closeAll();
}



QgsSpatiaLiteProviderMetadata::QgsSpatiaLiteProviderMetadata():
  QgsProviderMetadata( QgsSpatiaLiteProvider::SPATIALITE_KEY, QgsSpatiaLiteProvider::SPATIALITE_DESCRIPTION )
{
}

QIcon QgsSpatiaLiteProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconSpatialite.svg" ) );
}

QList< QgsDataItemProvider * > QgsSpatiaLiteProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsSpatiaLiteDataItemProvider;
  return providers;
}

QgsTransaction *QgsSpatiaLiteProviderMetadata::createTransaction( const QString &connString )
{
  const QgsDataSourceUri dsUri{ connString };
  // Cannot use QgsSpatiaLiteConnPool::instance()->acquireConnection( dsUri.database() ) };
  // because it will return a read only connection, use the (cached) connection from the
  // layers instead.
  QgsSqliteHandle *ds { QgsSqliteHandle::openDb( dsUri.database() ) };
  if ( !ds )
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot open transaction on %1, since it is is not currently opened" ).arg( connString ),
                               QObject::tr( "spatialite" ), Qgis::MessageLevel::Critical );
    return nullptr;
  }
  return new QgsSpatiaLiteTransaction( connString, ds );
}

QMap<QString, QgsAbstractProviderConnection *> QgsSpatiaLiteProviderMetadata::connections( bool cached )
{
  return connectionsProtected< QgsSpatiaLiteProviderConnection, QgsSpatiaLiteConnection>( cached );
}

QgsAbstractProviderConnection *QgsSpatiaLiteProviderMetadata::createConnection( const QString &connName )
{
  return new QgsSpatiaLiteProviderConnection( connName );
}

QgsAbstractProviderConnection *QgsSpatiaLiteProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  return new QgsSpatiaLiteProviderConnection( uri, configuration );
}

void QgsSpatiaLiteProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsSpatiaLiteProviderConnection>( name );
}

void QgsSpatiaLiteProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn, const QString &name )
{
  saveConnectionProtected( conn, name );
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsSpatiaLiteProviderMetadata();
}
#endif
