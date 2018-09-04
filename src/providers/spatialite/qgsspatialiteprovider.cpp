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
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayerexporter.h"
#include "qgsspatialiteutils.h"
#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnpool.h"
#include "qgsspatialitefeatureiterator.h"
#include "qgsfeedback.h"

#include "qgsjsonutils.h"
#include "qgsvectorlayer.h"

#ifdef HAVE_GUI
#include "qgssourceselectprovider.h"
#include "qgsspatialitesourceselect.h"
#endif

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>


const QString SPATIALITE_KEY = QStringLiteral( "spatialite" );
const QString SPATIALITE_DESCRIPTION = QStringLiteral( "SpatiaLite data provider" );
static const QString SPATIALITE_ARRAY_PREFIX = QStringLiteral( "json" );
static const QString SPATIALITE_ARRAY_SUFFIX = QStringLiteral( "list" );


bool QgsSpatiaLiteProvider::convertField( QgsField &field )
{
  QString fieldType = QStringLiteral( "TEXT" ); //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "BIGINT" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::String:
      fieldType = QStringLiteral( "TEXT" );
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "INTEGER" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = QStringLiteral( "REAL" );
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = QStringLiteral( "NUMERIC" );
      }
      break;

    case QVariant::List:
    case QVariant::StringList:
    {
      QgsField subField = field;
      subField.setType( field.subType() );
      subField.setSubType( QVariant::Invalid );
      if ( !convertField( subField ) ) return false;
      fieldType = SPATIALITE_ARRAY_PREFIX + subField.typeName() + SPATIALITE_ARRAY_SUFFIX;
      fieldSize = subField.length();
      fieldPrec = subField.precision();
      break;
    }

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}


QgsVectorLayerExporter::ExportError
QgsSpatiaLiteProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options );

  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QgsDebugMsg( "Database is: " + sqlitePath );
  QgsDebugMsg( "Table name is: " + tableName );
  QgsDebugMsg( "Geometry column is: " + geometryColumn );

  // create the table
  {
    char *errMsg = nullptr;
    int toCommit = false;
    QString sql;

    // trying to open the SQLite DB
    QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
    if ( !handle )
    {
      QgsDebugMsg( "Connection to database failed. Import of layer aborted." );
      if ( errorMessage )
        *errorMessage = QObject::tr( "Connection to database failed" );
      return QgsVectorLayerExporter::ErrConnectionFailed;
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
          if ( convertField( fld ) )
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
      int ret = sqlite3_exec( sqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

      toCommit = true;

      if ( overwrite )
      {
        // delete the table if exists and the related entry in geometry_columns, then re-create it
        sql = QStringLiteral( "DROP TABLE IF EXISTS %1" )
              .arg( quotedIdentifier( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );

        sql = QStringLiteral( "DELETE FROM geometry_columns WHERE upper(f_table_name) = upper(%1)" )
              .arg( quotedValue( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );
      }

      sql = QStringLiteral( "CREATE TABLE %1 (%2 %3 PRIMARY KEY%4)" )
            .arg( quotedIdentifier( tableName ),
                  quotedIdentifier( primaryKey ),
                  primaryKeyType,
                  primaryKeyType == QLatin1String( "INTEGER" ) ? QStringLiteral( " AUTOINCREMENT" ) : QString() );

      ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

      // get geometry type, dim and srid
      int dim = 2;
      long srid = srs.postgisSrid();

      switch ( wkbType )
      {
        case QgsWkbTypes::Point25D:
          dim = 3;
          FALLTHROUGH
        case QgsWkbTypes::Point:
          geometryType = QStringLiteral( "POINT" );
          break;

        case QgsWkbTypes::LineString25D:
          dim = 3;
          FALLTHROUGH
        case QgsWkbTypes::LineString:
          geometryType = QStringLiteral( "LINESTRING" );
          break;

        case QgsWkbTypes::Polygon25D:
          dim = 3;
          FALLTHROUGH
        case QgsWkbTypes::Polygon:
          geometryType = QStringLiteral( "POLYGON" );
          break;

        case QgsWkbTypes::MultiPoint25D:
          dim = 3;
          FALLTHROUGH
        case QgsWkbTypes::MultiPoint:
          geometryType = QStringLiteral( "MULTIPOINT" );
          break;

        case QgsWkbTypes::MultiLineString25D:
          dim = 3;
          FALLTHROUGH
        case QgsWkbTypes::MultiLineString:
          geometryType = QStringLiteral( "MULTILINESTRING" );
          break;

        case QgsWkbTypes::MultiPolygon25D:
          dim = 3;
          FALLTHROUGH
        case QgsWkbTypes::MultiPolygon:
          geometryType = QStringLiteral( "MULTIPOLYGON" );
          break;

        case QgsWkbTypes::Unknown:
          geometryType = QStringLiteral( "GEOMETRY" );
          break;

        case QgsWkbTypes::NoGeometry:
        default:
          dim = 0;
          break;
      }

      // create geometry column
      if ( !geometryType.isEmpty() )
      {
        sql = QStringLiteral( "SELECT AddGeometryColumn(%1, %2, %3, %4, %5)" )
              .arg( QgsSpatiaLiteProvider::quotedValue( tableName ),
                    QgsSpatiaLiteProvider::quotedValue( geometryColumn ) )
              .arg( srid )
              .arg( QgsSpatiaLiteProvider::quotedValue( geometryType ) )
              .arg( dim );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw SLException( errMsg );
      }
      else
      {
        geometryColumn = QString();
      }

      ret = sqlite3_exec( sqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw SLException( errMsg );

    }
    catch ( SLException &e )
    {
      QgsDebugMsg( QString( "creation of data source %1 failed. %2" )
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
        sqlite3_exec( sqliteHandle, "ROLLBACK", nullptr, nullptr, nullptr );
      }

      QgsSqliteHandle::closeDb( handle );
      return QgsVectorLayerExporter::ErrCreateLayer;
    }

    QgsSqliteHandle::closeDb( handle );
    QgsDebugMsg( "layer " + tableName  + " created." );
  }

  // use the provider to edit the table
  dsUri.setDataSource( QString(), tableName, geometryColumn, QString(), primaryKey );

  QgsDataProvider::ProviderOptions providerOptions;
  QgsSpatiaLiteProvider *provider = new QgsSpatiaLiteProvider( dsUri.uri(), providerOptions );
  if ( !provider->isValid() )
  {
    QgsDebugMsg( "The layer " + tableName + " just created is not valid or not supported by the provider." );
    if ( errorMessage )
      *errorMessage = QObject::tr( "loading of the layer %1 failed" )
                      .arg( tableName );

    delete provider;
    return QgsVectorLayerExporter::ErrInvalidLayer;
  }

  QgsDebugMsg( "layer loaded" );

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
        QgsDebugMsg( "Found a field with the same name of the geometry column. Skip it!" );
        continue;
      }

      if ( !convertField( fld ) )
      {
        QgsDebugMsg( "error creating field " + fld.name() + ": unsupported type" );
        if ( errorMessage )
          *errorMessage = QObject::tr( "unsupported type for field %1" )
                          .arg( fld.name() );

        delete provider;
        return QgsVectorLayerExporter::ErrAttributeTypeUnsupported;
      }

      QgsDebugMsg( "creating field #" + QString::number( fldIdx ) +
                   " -> #" + QString::number( offset ) +
                   " name " + fld.name() +
                   " type " + QString( QVariant::typeToName( fld.type() ) ) +
                   " typename " + fld.typeName() +
                   " width " + QString::number( fld.length() ) +
                   " precision " + QString::number( fld.precision() ) );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIdx, offset );

      offset++;
    }

    if ( !provider->addAttributes( flist ) )
    {
      QgsDebugMsg( "error creating fields " );
      if ( errorMessage )
        *errorMessage = QObject::tr( "creation of fields failed" );

      delete provider;
      return QgsVectorLayerExporter::ErrAttributeCreationFailed;
    }

    QgsDebugMsg( "Done creating fields" );
  }
  return QgsVectorLayerExporter::NoError;
}


QgsSpatiaLiteProvider::QgsSpatiaLiteProvider( QString const &uri, const ProviderOptions &options )
  : QgsVectorDataProvider( uri, options )
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

  // trying to open the SQLite DB
  mHandle = QgsSqliteHandle::openDb( mSqlitePath );
  if ( !mHandle )
  {
    return;
  }

  mSqliteHandle = mHandle->handle();
  if ( mSqliteHandle )
  {
    QStringList pragmaList = anUri.params( QStringLiteral( "pragma" ) );
    Q_FOREACH ( const QString &pragma, pragmaList )
    {
      char *errMsg = nullptr;
      int ret = sqlite3_exec( mSqliteHandle, ( "PRAGMA " + pragma ).toUtf8(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
      {
        QgsDebugMsg( QString( "PRAGMA " ) + pragma + QString( " failed : %1" ).arg( errMsg ? errMsg : "" ) );
      }
      sqlite3_free( errMsg );
    }
  }

  bool alreadyDone = false;
  bool ret = false;

  gaiaVectorLayersListPtr list = nullptr;
  gaiaVectorLayerPtr lyr = nullptr;
  bool specialCase = false;
  if ( mGeometryColumn.isEmpty() )
    specialCase = true; // non-spatial table
  if ( mQuery.startsWith( '(' ) && mQuery.endsWith( ')' ) )
    specialCase = true;

  if ( !specialCase )
  {
    // using v.4.0 Abstract Interface
    ret = true;
    list = gaiaGetVectorLayersList( mSqliteHandle,
                                    mTableName.toUtf8().constData(),
                                    mGeometryColumn.toUtf8().constData(),
                                    GAIA_VECTORS_LIST_OPTIMISTIC );
    if ( list )
      lyr = list->First;

    ret = lyr && checkLayerTypeAbstractInterface( lyr );
    QgsDebugMsg( "Using checkLayerTypeAbstractInterface" );
    alreadyDone = true;
  }

  if ( !alreadyDone )
  {
    // check if this one Layer is based on a Table, View or VirtualShapefile
    // by using the traditional methods
    ret = checkLayerType();
  }

  if ( !ret )
  {
    // invalid metadata
    mNumberFeatures = 0;

    QgsDebugMsg( "Invalid SpatiaLite layer" );
    closeDb();
    return;
  }
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
  }

  alreadyDone = false;

  if ( lyr )
  {
    // using the v.4.0 AbstractInterface
    if ( !getGeometryDetailsAbstractInterface( lyr ) )  // gets srid and geometry type
    {
      // the table is not a geometry table
      mNumberFeatures = 0;
      QgsDebugMsg( "Invalid SpatiaLite layer" );
      closeDb();
      gaiaFreeVectorLayersList( list );
      return;
    }
    if ( !getTableSummaryAbstractInterface( lyr ) )     // gets the extent and feature count
    {
      mNumberFeatures = 0;
      QgsDebugMsg( "Invalid SpatiaLite layer" );
      closeDb();
      gaiaFreeVectorLayersList( list );
      return;
    }
    // load the columns list
    loadFieldsAbstractInterface( lyr );
    gaiaFreeVectorLayersList( list );
    alreadyDone = true;
  }

  if ( !alreadyDone )
  {
    // using the traditional methods
    if ( !getGeometryDetails() )  // gets srid and geometry type
    {
      // the table is not a geometry table
      mNumberFeatures = 0;
      QgsDebugMsg( "Invalid SpatiaLite layer" );
      closeDb();
      return;
    }
    if ( !getTableSummary() )     // gets the extent and feature count
    {
      mNumberFeatures = 0;
      QgsDebugMsg( "Invalid SpatiaLite layer" );
      closeDb();
      return;
    }
    // load the columns list
    loadFields();
  }
  if ( !mSqliteHandle )
  {
    QgsDebugMsg( "Invalid SpatiaLite layer" );
    return;
  }

  if ( mTableBased && hasRowid() && mPrimaryKey.isEmpty() )
  {
    mPrimaryKey = QStringLiteral( "ROWID" );
  }

  // retrieve version information
  spatialiteVersion();

  //fill type names into sets
  setNativeTypes( QList<NativeType>()
                  << QgsVectorDataProvider::NativeType( tr( "Binary object (BLOB)" ), QStringLiteral( "BLOB" ), QVariant::ByteArray )
                  << QgsVectorDataProvider::NativeType( tr( "Text" ), QStringLiteral( "TEXT" ), QVariant::String )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "FLOAT" ), QVariant::Double )
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), QStringLiteral( "INTEGER" ), QVariant::LongLong )

                  << QgsVectorDataProvider::NativeType( tr( "Array of text" ), SPATIALITE_ARRAY_PREFIX.toUpper() + "TEXT" + SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::StringList, 0, 0, 0, 0, QVariant::String )
                  << QgsVectorDataProvider::NativeType( tr( "Array of decimal numbers (double)" ), SPATIALITE_ARRAY_PREFIX.toUpper() + "REAL" + SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::Double )
                  << QgsVectorDataProvider::NativeType( tr( "Array of whole numbers (integer)" ), SPATIALITE_ARRAY_PREFIX.toUpper() + "INTEGER" + SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::LongLong )
                );

  // Update extent and feature count
  if ( ! mSubsetString.isEmpty() )
    getTableSummary();

  mValid = true;
}

QgsSpatiaLiteProvider::~QgsSpatiaLiteProvider()
{
  closeDb();
  invalidateConnections( mSqlitePath );
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

typedef QPair<QVariant::Type, QVariant::Type> TypeSubType;

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
    return TypeSubType( QVariant::LongLong, QVariant::Invalid );
  else if ( type == QLatin1String( "real" ) ||
            type == QLatin1String( "double" ) ||
            type == QLatin1String( "double precision" ) ||
            type == QLatin1String( "float" ) )
    return TypeSubType( QVariant::Double, QVariant::Invalid );
  else if ( type.startsWith( SPATIALITE_ARRAY_PREFIX ) && type.endsWith( SPATIALITE_ARRAY_SUFFIX ) )
  {
    // New versions of OGR convert list types (StringList, IntegerList, Integer64List and RealList)
    // to JSON when it stores a Spatialite table. It sets the column type as JSONSTRINGLIST,
    // JSONINTEGERLIST, JSONINTEGER64LIST or JSONREALLIST
    TypeSubType subType = getVariantType( type.mid( SPATIALITE_ARRAY_PREFIX.length(),
                                          type.length() - SPATIALITE_ARRAY_PREFIX.length() - SPATIALITE_ARRAY_SUFFIX.length() ) );
    return TypeSubType( subType.first == QVariant::String ? QVariant::StringList : QVariant::List, subType.first );
  }
  else
    // for sure any SQLite value can be represented as SQLITE_TEXT
    return TypeSubType( QVariant::String, QVariant::Invalid );
}

void QgsSpatiaLiteProvider::loadFieldsAbstractInterface( gaiaVectorLayerPtr lyr )
{
  if ( !lyr )
    return;

  mAttributeFields.clear();
  mPrimaryKey.clear(); // cazzo cazzo cazzo
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
      QVariant::Type fieldType = QVariant::String; // default: SQLITE_TEXT
      if ( fld->IntegerValuesCount != 0 && fld->DoubleValuesCount == 0 &&
           fld->TextValuesCount == 0 && fld->BlobValuesCount == 0 )
      {
        fieldType = QVariant::LongLong;
        type = "INTEGER";
      }
      if ( fld->DoubleValuesCount != 0 && fld->TextValuesCount == 0 &&
           fld->BlobValuesCount == 0 )
      {
        fieldType = QVariant::Double;
        type = "DOUBLE";
      }
      mAttributeFields.append( QgsField( name, fieldType, type, 0, 0, QString() ) );
    }
    fld = fld->Next;
  }

  QString sql = QStringLiteral( "PRAGMA table_info(%1)" ).arg( quotedIdentifier( mTableName ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret == SQLITE_OK )
  {
    int realFieldIndex = 0;
    for ( int i = 1; i <= rows; i++ )
    {
      QString name = QString::fromUtf8( results[( i * columns ) + 1] );

      if ( name.compare( mGeometryColumn, Qt::CaseInsensitive ) == 0 )
        continue;

      insertDefaultValue( realFieldIndex, QString::fromUtf8( results[( i * columns ) + 4] ) );

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
        if ( field.type() == QVariant::String )
        {
          // if the type seems unknown, fix it with what we actually have
          TypeSubType typeSubType = getVariantType( type );
          field.setType( typeSubType.first );
          field.setSubType( typeSubType.second );
        }
      }

      if ( pk.toInt() == 0 )
        continue;

      if ( mPrimaryKeyAttrs.isEmpty() )
        mPrimaryKey = name;
      else
        mPrimaryKey.clear();
      mPrimaryKeyAttrs << i - 1;
      realFieldIndex += 1;
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

  QgsDebugMsg( "SpatiaLite version info: " + mSpatialiteVersionInfo );

  QStringList spatialiteParts = mSpatialiteVersionInfo.split( ' ', QString::SkipEmptyParts );

  // Get major and minor version
  QStringList spatialiteVersionParts = spatialiteParts[0].split( '.', QString::SkipEmptyParts );
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

void QgsSpatiaLiteProvider::fetchConstraints()
{
  char **results = nullptr;
  char *errMsg = nullptr;

  // this is not particularly robust but unfortunately sqlite offers no way to check directly
  // for the presence of constraints on a field (only indexes, but not all constraints are indexes)
  QString sql = QStringLiteral( "SELECT sql FROM sqlite_master WHERE type='table' AND name=%1" ).arg( quotedIdentifier( mTableName ) );
  int columns = 0;
  int rows = 0;

  int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return;
  }

  if ( rows < 1 )
    ;
  else
  {
    QString sqlDef = QString::fromUtf8( results[ 1 ] );
    // extract definition
    QRegularExpression re( QStringLiteral( "\\((.*)\\)" ) );
    QRegularExpressionMatch match = re.match( sqlDef );
    if ( match.hasMatch() )
    {
      QString matched = match.captured( 1 );
      Q_FOREACH ( QString field, matched.split( ',' ) )
      {
        field = field.trimmed();
        QString fieldName = field.left( field.indexOf( ' ' ) );
        QString definition = field.mid( field.indexOf( ' ' ) + 1 );
        int fieldIdx = mAttributeFields.lookupField( fieldName );
        if ( fieldIdx >= 0 )
        {
          QgsFieldConstraints constraints = mAttributeFields.at( fieldIdx ).constraints();
          if ( definition.contains( QLatin1String( "unique" ), Qt::CaseInsensitive ) || definition.contains( QLatin1String( "primary key" ), Qt::CaseInsensitive ) )
            constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
          if ( definition.contains( QLatin1String( "not null" ), Qt::CaseInsensitive ) || definition.contains( QLatin1String( "primary key" ), Qt::CaseInsensitive ) )
            constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
          mAttributeFields[ fieldIdx ].setConstraints( constraints );
        }
      }
    }

  }
  sqlite3_free_table( results );

  Q_FOREACH ( int fieldIdx, mPrimaryKeyAttrs )
  {
    QgsFieldConstraints constraints = mAttributeFields.at( fieldIdx ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[ fieldIdx ].setConstraints( constraints );

    if ( mAttributeFields[ fieldIdx ].name() == mPrimaryKey )
    {
      QString sql = QStringLiteral( "SELECT sql FROM sqlite_master WHERE type = 'table' AND tbl_name like %1" ).arg( quotedIdentifier( mTableName ) );
      int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      if ( ret != SQLITE_OK )
      {
        handleError( sql, errMsg );
        return;
      }

      if ( rows >= 1 )
      {
        QString tableSql = QString::fromUtf8( results[ 1 ] );
        QRegularExpression rx( QStringLiteral( "[(,]\\s*(?:%1|\"%1\")\\s+INTEGER PRIMARY KEY AUTOINCREMENT" ).arg( mPrimaryKey ), QRegularExpression::CaseInsensitiveOption );
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

    if ( mAttributeFields.at( fieldIndex ).name() != mPrimaryKey || ( mAttributeFields.at( fieldIndex ).name() == mPrimaryKey && !mPrimaryKeyAutoIncrement ) )
    {
      switch ( mAttributeFields.at( fieldIndex ).type() )
      {
        case QVariant::LongLong:
          defaultVariant = defaultVal.toLongLong();
          break;

        case QVariant::Double:
          defaultVariant = defaultVal.toDouble();
          break;

        default:
        {
          if ( defaultVal.startsWith( '\'' ) )
            defaultVal = defaultVal.remove( 0, 1 );
          if ( defaultVal.endsWith( '\'' ) )
            defaultVal.chop( 1 );
          defaultVal.replace( QLatin1String( "''" ), QLatin1String( "'" ) );

          defaultVariant = defaultVal;
          break;
        }
      }
    }
    mDefaultValues.insert( fieldIndex, defaultVariant );
  }
}

void QgsSpatiaLiteProvider::handleError( const QString &sql, char *errorMessage, bool rollback )
{
  QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, errorMessage ? errorMessage : tr( "unknown cause" ) ), tr( "SpatiaLite" ) );
  // unexpected error
  if ( errorMessage )
  {
    sqlite3_free( errorMessage );
  }

  if ( rollback )
  {
    // ROLLBACK after some previous error
    ( void )sqlite3_exec( mSqliteHandle, "ROLLBACK", nullptr, nullptr, nullptr );
  }
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

    sql = QStringLiteral( "PRAGMA table_info(%1)" ).arg( quotedIdentifier( mTableName ) );

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg );
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
        if ( pk.toInt() != 0 )
        {
          // found a Primary Key column
          pkCount++;
          if ( mPrimaryKeyAttrs.isEmpty() )
            pkName = name;
          else
            pkName.clear();
          mPrimaryKeyAttrs << realFieldIndex;
          QgsDebugMsg( "found primaryKey " + name );
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

    if ( sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
    {
      // some error occurred
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ), tr( "SpatiaLite" ) );
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
          pkCount++;
          if ( mPrimaryKeyAttrs.isEmpty() )
            pkName = name;
          else
            pkName.clear();
          mPrimaryKeyAttrs << i - 1;
          QgsDebugMsg( "found primaryKey " + name );
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
                         " WHERE upper(view_name) = upper(%1) and upper(view_geometry) = upper(%2)" ).arg( quotedValue( mTableName ),
                             quotedValue( mGeometryColumn ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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


bool QgsSpatiaLiteProvider::hasTriggers()
{
  int ret;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  QString sql;

  sql = QStringLiteral( "SELECT * FROM sqlite_master WHERE type='trigger' AND tbl_name=%1" )
        .arg( quotedIdentifier( mTableName ) );

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  sqlite3_free_table( results );
  return ( ret == SQLITE_OK && rows > 0 );
}

bool QgsSpatiaLiteProvider::hasRowid()
{
  if ( mAttributeFields.lookupField( QStringLiteral( "ROWID" ) ) >= 0 )
    return false;

  // table without rowid column
  QString sql = QStringLiteral( "SELECT rowid FROM %1 WHERE 0" ).arg( quotedIdentifier( mTableName ) );
  char *errMsg = nullptr;
  return sqlite3_exec( mSqliteHandle, sql.toUtf8(), nullptr, nullptr, &errMsg ) == SQLITE_OK;
}


QString QgsSpatiaLiteProvider::storageType() const
{
  return QStringLiteral( "SQLite database with SpatiaLite extension" );
}

QgsFeatureIterator QgsSpatiaLiteProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsDebugMsg( "Read attempt on an invalid SpatiaLite data source" );
    return QgsFeatureIterator();
  }
  return QgsFeatureIterator( new QgsSpatiaLiteFeatureIterator( new QgsSpatiaLiteFeatureSource( this ), true, request ) );
}


int QgsSpatiaLiteProvider::computeSizeFromGeosWKB2D( const unsigned char *blob,
    int size, QgsWkbTypes::Type type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
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
      case QgsWkbTypes::PointGeometry:
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
      case QgsWkbTypes::LineGeometry:
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
      case QgsWkbTypes::PolygonGeometry:
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

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
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
    int size, QgsWkbTypes::Type type, int nDims,
    int little_endian, int endian_arch )
{
  Q_UNUSED( size );
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
      case QgsWkbTypes::PointGeometry:
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
      case QgsWkbTypes::LineGeometry:
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
      case QgsWkbTypes::PolygonGeometry:
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

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
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
  QgsWkbTypes::Type type;
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
    type = static_cast< QgsWkbTypes::Type >( gaiaImport32( p_in + 1, little_endian, endian_arch ) );
    p_in += 5;
    size += 5;
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      // compunting the required size
      case QgsWkbTypes::PointGeometry:
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
      case QgsWkbTypes::LineGeometry:
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
      case QgsWkbTypes::PolygonGeometry:
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

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
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
  QgsWkbTypes::Type type;
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
  type = static_cast< QgsWkbTypes::Type >( gaiaImport32( blob + 1, little_endian, endian_arch ) );
  if ( QgsWkbTypes::hasZ( type ) || QgsWkbTypes::hasM( type ) )
    gDims = 3;
  else if ( type != QgsWkbTypes::Unknown )
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
  Q_UNUSED( blob_size );
  Q_UNUSED( geom_size );
// attempting to convert from 3D GEOS own WKB
  QgsWkbTypes::Type type;
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
  type = static_cast< QgsWkbTypes::Type >( gaiaImport32( blob + 1, little_endian, endian_arch ) );
  if ( QgsWkbTypes::geometryType( type ) == QgsWkbTypes::PointGeometry )
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
  else if ( QgsWkbTypes::geometryType( type ) == QgsWkbTypes::LineGeometry )
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
  else if ( QgsWkbTypes::geometryType( type ) == QgsWkbTypes::PolygonGeometry )
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
  else if ( QgsWkbTypes::flatType( type ) == QgsWkbTypes::GeometryCollection )
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
      case QgsWkbTypes::PointGeometry:
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
      case QgsWkbTypes::LineGeometry:
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
      case QgsWkbTypes::PolygonGeometry:
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

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
        break;
    }
  }
  else
  {
    switch ( QgsWkbTypes::geometryType( type ) )
    {
      case QgsWkbTypes::PointGeometry:
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
      case QgsWkbTypes::LineGeometry:
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
      case QgsWkbTypes::PolygonGeometry:
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

      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
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
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::Point25D, 1, endian_arch );
      break;
    case GAIA_POINTM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::PointM, 1, endian_arch );
      break;
    case GAIA_POINTZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::PointZM, 1, endian_arch );
      break;
    case GAIA_LINESTRINGZ:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::LineString25D, 1, endian_arch );
      break;
    case GAIA_LINESTRINGM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::LineStringM, 1, endian_arch );
      break;
    case GAIA_LINESTRINGZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::LineStringZM, 1, endian_arch );
      break;
    case GAIA_POLYGONZ:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::Polygon25D, 1, endian_arch );
      break;
    case GAIA_POLYGONM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::PolygonM, 1, endian_arch );
      break;
    case GAIA_POLYGONZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::PolygonZM, 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZ:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiPoint25D, 1, endian_arch );
      break;
    case GAIA_MULTIPOINTM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiPointM, 1, endian_arch );
      break;
    case GAIA_MULTIPOINTZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiPointZM, 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZ:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiLineString25D, 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiLineStringM, 1, endian_arch );
      break;
    case GAIA_MULTILINESTRINGZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiLineStringZM, 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZ:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiPolygon25D, 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiPolygonM, 1, endian_arch );
      break;
    case GAIA_MULTIPOLYGONZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::MultiPolygonZM, 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZ:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::GeometryCollectionZ, 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::GeometryCollectionM, 1, endian_arch );
      break;
    case GAIA_GEOMETRYCOLLECTIONZM:
      gaiaExport32( wkbGeom + 1, QgsWkbTypes::GeometryCollectionZM, 1, endian_arch );
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
        gaiaExport32( p_out, type == GAIA_MULTIPOINTZ ? QgsWkbTypes::Point25D : QgsWkbTypes::PointM, 1, endian_arch );
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
        gaiaExport32( p_out, QgsWkbTypes::PointZM, 1, endian_arch );
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
        gaiaExport32( p_out, type == GAIA_MULTILINESTRINGZ ? QgsWkbTypes::LineString25D : QgsWkbTypes::LineStringM, 1, endian_arch );
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
        gaiaExport32( p_out, QgsWkbTypes::LineStringZM, 1, endian_arch );
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
        gaiaExport32( p_out, type == GAIA_MULTIPOLYGONZ ? QgsWkbTypes::Polygon25D : QgsWkbTypes::PolygonM, 1, endian_arch );
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
        gaiaExport32( p_out, QgsWkbTypes::PolygonZM, 1, endian_arch );
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
            gaiaExport32( p_out, QgsWkbTypes::Point25D, 1, endian_arch );
            break;
          case GAIA_POINTM:
            gaiaExport32( p_out, QgsWkbTypes::PointM, 1, endian_arch );
            break;
          case GAIA_POINTZM:
            gaiaExport32( p_out, QgsWkbTypes::PointZM, 1, endian_arch );
            break;
          case GAIA_LINESTRINGZ:
            gaiaExport32( p_out, QgsWkbTypes::LineString25D, 1, endian_arch );
            break;
          case GAIA_LINESTRINGM:
            gaiaExport32( p_out, QgsWkbTypes::LineStringM, 1, endian_arch );
            break;
          case GAIA_LINESTRINGZM:
            gaiaExport32( p_out, QgsWkbTypes::LineStringZM, 1, endian_arch );
            break;
          case GAIA_POLYGONZ:
            gaiaExport32( p_out, QgsWkbTypes::Polygon25D, 1, endian_arch );
            break;
          case GAIA_POLYGONM:
            gaiaExport32( p_out, QgsWkbTypes::PolygonM, 1, endian_arch );
            break;
          case GAIA_POLYGONZM:
            gaiaExport32( p_out, QgsWkbTypes::PolygonZM, 1, endian_arch );
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
QgsWkbTypes::Type QgsSpatiaLiteProvider::wkbType() const
{
  return mGeomType;
}

/**
 * Returns the feature type
 */
long QgsSpatiaLiteProvider::featureCount() const
{
  return mNumberFeatures;
}


QgsCoordinateReferenceSystem QgsSpatiaLiteProvider::crs() const
{
  QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mAuthId );
  if ( !srs.isValid() )
  {
    srs = QgsCoordinateReferenceSystem::fromProj4( mProj4text );
    //TODO: createFromProj4 used to save to the user database any new CRS
    // this behavior was changed in order to separate creation and saving.
    // Not sure if it necessary to save it here, should be checked by someone
    // familiar with the code (should also give a more descriptive name to the generated CRS)
    if ( srs.srsid() == 0 )
    {
      QString myName = QStringLiteral( " * %1 (%2)" )
                       .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                             srs.toProj4() );
      srs.saveAsUserCrs( myName );
    }

  }
  return srs;
}


bool QgsSpatiaLiteProvider::isValid() const
{
  return mValid;
}


QString QgsSpatiaLiteProvider::name() const
{
  return SPATIALITE_KEY;
}                               //  QgsSpatiaLiteProvider::name()


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

    sql = QStringLiteral( "SELECT Min(%1) FROM %2" ).arg( quotedIdentifier( fld.name() ), mQuery );

    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    return QVariant( QVariant::Int );
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

    sql = QStringLiteral( "SELECT Max(%1) FROM %2" ).arg( quotedIdentifier( fld.name() ), mQuery );

    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    return QVariant( QVariant::Int );
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

  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2" ).arg( quotedIdentifier( fld.name() ), mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ')';
  }

  sql += QStringLiteral( " ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) );

  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ), tr( "SpatiaLite" ) );
    return uniqueValues;
  }

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
          uniqueValues.insert( QVariant( sqlite3_column_int( stmt, 0 ) ) );
          break;
        case SQLITE_FLOAT:
          uniqueValues.insert( QVariant( sqlite3_column_double( stmt, 0 ) ) );
          break;
        case SQLITE_TEXT:
          uniqueValues.insert( QVariant( QString::fromUtf8( ( const char * ) sqlite3_column_text( stmt, 0 ) ) ) );
          break;
        default:
          uniqueValues.insert( QVariant( mAttributeFields.at( index ).type() ) );
          break;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return uniqueValues;
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

  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 " ).arg( quotedIdentifier( fld.name() ), mQuery );
  sql += QStringLiteral( " WHERE " ) + quotedIdentifier( fld.name() ) + QStringLiteral( " LIKE '%" ) + substring + QStringLiteral( "%'" );

  if ( !mSubsetString.isEmpty() )
  {
    sql += QStringLiteral( " AND ( " ) + mSubsetString + ')';
  }

  sql += QStringLiteral( " ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) );

  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ), tr( "SpatiaLite" ) );
    return results;
  }

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
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return results;
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
  QString sql;
  QString values;
  QString separator;
  int ia, ret;

  if ( flist.isEmpty() )
    return true;
  QgsAttributes attributevec = flist[0].attributes();

  ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret == SQLITE_OK )
  {
    toCommit = true;

    sql = QStringLiteral( "INSERT INTO %1(" ).arg( quotedIdentifier( mTableName ) );
    values = QStringLiteral( ") VALUES (" );
    separator.clear();

    if ( !mGeometryColumn.isEmpty() )
    {
      sql += separator + quotedIdentifier( mGeometryColumn );
      values += separator + geomParam();
      separator = ',';
    }

    for ( int i = 0; i < attributevec.count(); ++i )
    {
      if ( i >= mAttributeFields.count() )
        continue;

      QString fieldname = mAttributeFields.at( i ).name();
      if ( fieldname.isEmpty() || fieldname == mGeometryColumn )
        continue;

      sql += separator + quotedIdentifier( fieldname );
      values += separator + '?';
      separator = ',';
    }

    sql += values;
    sql += ')';

    // SQLite prepared statement
    ret = sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr );
    if ( ret == SQLITE_OK )
    {
      for ( QgsFeatureList::iterator feature = flist.begin(); feature != flist.end(); ++feature )
      {
        // looping on each feature to insert
        QgsAttributes attributevec = feature->attributes();

        // resetting Prepared Statement and bindings
        sqlite3_reset( stmt );
        sqlite3_clear_bindings( stmt );

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
            QByteArray featureWkb = feature->geometry().asWkb();
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
          QVariant v = attributevec.at( i );

          // binding values for each attribute
          if ( i >= mAttributeFields.count() )
            break;

          QString fieldname = mAttributeFields.at( i ).name();
          if ( fieldname.isEmpty() || fieldname == mGeometryColumn )
            continue;

          QVariant::Type type = mAttributeFields.at( i ).type();

          if ( !v.isValid() )
          {
            ++ia;
          }
          else if ( fieldname == mPrimaryKey && mPrimaryKeyAutoIncrement && v == QVariant( tr( "Autogenerate" ) ) )
          {
            // use auto-generated value if user hasn't specified a unique value
            ++ia;
          }
          else if ( v.isNull() )
          {
            // binding a NULL value
            sqlite3_bind_null( stmt, ++ia );
          }
          else if ( type == QVariant::Int )
          {
            // binding an INTEGER value
            sqlite3_bind_int( stmt, ++ia, v.toInt() );
          }
          else if ( type == QVariant::LongLong )
          {
            // binding a LONGLONG value
            sqlite3_bind_int64( stmt, ++ia, v.toLongLong() );
          }
          else if ( type == QVariant::Double )
          {
            // binding a DOUBLE value
            sqlite3_bind_double( stmt, ++ia, v.toDouble() );
          }
          else if ( type == QVariant::String )
          {
            QString stringVal = v.toString();

            // binding a TEXT value
            QByteArray ba = stringVal.toUtf8();
            sqlite3_bind_text( stmt, ++ia, ba.constData(), ba.size(), SQLITE_TRANSIENT );
          }
          else if ( type == QVariant::StringList || type == QVariant::List )
          {
            const QByteArray ba = QgsJsonUtils::encodeValue( v ).toUtf8();
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

        if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
        {
          // update feature id
          if ( !( flags & QgsFeatureSink::FastInsert ) )
          {
            feature->setId( sqlite3_last_insert_rowid( mSqliteHandle ) );
          }
          mNumberFeatures++;
        }
        else
        {
          // some unexpected error occurred
          const char *err = sqlite3_errmsg( mSqliteHandle );
          errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
          strcpy( errMsg, err );
          break;
        }
      }

      sqlite3_finalize( stmt );

      if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
      {
        ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
      }
    } // prepared statement
  } // BEGIN statement

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
      ( void )sqlite3_exec( mSqliteHandle, "ROLLBACK", nullptr, nullptr, nullptr );
    }
  }

  return ret == SQLITE_OK;
}

QString createIndexName( QString tableName, QString field )
{
  QRegularExpression safeExp( QStringLiteral( "[^a-zA-Z0-9]" ) );
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

  int ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  fieldName = mAttributeFields.at( field ).name();

  sql = QStringLiteral( "CREATE INDEX IF NOT EXISTS %1 ON \"%2\" (%3)" )
        .arg( createIndexName( mTableName, fieldName ),
              mTableName,
              quotedIdentifier( fieldName ) );
  ret = sqlite3_exec( mSqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  return true;
}

bool QgsSpatiaLiteProvider::deleteFeatures( const QgsFeatureIds &id )
{
  sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  QString sql;

  int ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  sql = QStringLiteral( "DELETE FROM %1 WHERE %2=?" ).arg( quotedIdentifier( mTableName ), quotedIdentifier( mPrimaryKey ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ) );
    return false;
  }

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
      const char *err = sqlite3_errmsg( mSqliteHandle );
      errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
      strcpy( errMsg, err );
      handleError( sql, errMsg, true );
      return false;
    }
  }
  sqlite3_finalize( stmt );

  ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  return true;
}

bool QgsSpatiaLiteProvider::truncate()
{
  char *errMsg = nullptr;
  QString sql;

  int ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  sql = QStringLiteral( "DELETE FROM %1" ).arg( quotedIdentifier( mTableName ) );
  ret = sqlite3_exec( mSqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  return true;
}

bool QgsSpatiaLiteProvider::addAttributes( const QList<QgsField> &attributes )
{
  char *errMsg = nullptr;
  QString sql;

  if ( attributes.isEmpty() )
    return true;

  int ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    sql = QStringLiteral( "ALTER TABLE \"%1\" ADD COLUMN \"%2\" %3" )
          .arg( mTableName,
                iter->name(),
                iter->typeName() );
    ret = sqlite3_exec( mSqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }

  ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  gaiaStatisticsInvalidate( mSqliteHandle, mTableName.toUtf8().constData(), mGeometryColumn.toUtf8().constData() );
  update_layer_statistics( mSqliteHandle, mTableName.toUtf8().constData(), mGeometryColumn.toUtf8().constData() );

  // reload columns
  loadFields();

  return true;
}

bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  char *errMsg = nullptr;
  QString sql;

  if ( attr_map.isEmpty() )
    return true;

  int ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
  {
    // Loop over all changed features

    QgsFeatureId fid = iter.key();

    // skip added features
    if ( FID_IS_NEW( fid ) )
      continue;

    const QgsAttributeMap &attrs = iter.value();
    if ( attrs.isEmpty() )
      continue;

    QString sql = QStringLiteral( "UPDATE %1 SET " ).arg( quotedIdentifier( mTableName ) );
    bool first = true;

    // cycle through the changed attributes of the feature
    for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
    {
      // Loop over all changed attributes
      try
      {
        QgsField fld = field( siter.key() );
        const QVariant &val = siter.value();

        if ( !first )
          sql += ',';
        else
          first = false;

        QVariant::Type type = fld.type();

        if ( val.isNull() || !val.isValid() )
        {
          // binding a NULL value
          sql += QStringLiteral( "%1=NULL" ).arg( quotedIdentifier( fld.name() ) );
        }
        else if ( type == QVariant::Int || type == QVariant::LongLong || type == QVariant::Double )
        {
          // binding a NUMERIC value
          sql += QStringLiteral( "%1=%2" ).arg( quotedIdentifier( fld.name() ), val.toString() );
        }
        else if ( type == QVariant::StringList || type == QVariant::List )
        {
          // binding an array value
          sql += QStringLiteral( "%1=%2" ).arg( quotedIdentifier( fld.name() ), quotedValue( QgsJsonUtils::encodeValue( val ) ) );
        }
        else
        {
          // binding a TEXT value
          sql += QStringLiteral( "%1=%2" ).arg( quotedIdentifier( fld.name() ), quotedValue( val.toString() ) );
        }
      }
      catch ( SLFieldNotFound )
      {
        // Field was missing - shouldn't happen
      }
    }
    sql += QStringLiteral( " WHERE %1=%2" ).arg( quotedIdentifier( mPrimaryKey ) ).arg( fid );

    ret = sqlite3_exec( mSqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }

  ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  return true;
}

bool QgsSpatiaLiteProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  sqlite3_stmt *stmt = nullptr;
  char *errMsg = nullptr;
  QString sql;

  int ret = sqlite3_exec( mSqliteHandle, "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  sql =
    QStringLiteral( "UPDATE %1 SET %2=GeomFromWKB(?, %3) WHERE %4=?" )
    .arg( quotedIdentifier( mTableName ),
          quotedIdentifier( mGeometryColumn ) )
    .arg( mSrid )
    .arg( quotedIdentifier( mPrimaryKey ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( mSqliteHandle, sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( mSqliteHandle ) ), tr( "SpatiaLite" ) );
    return false;
  }

  for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
  {
    // resetting Prepared Statement and bindings
    sqlite3_reset( stmt );
    sqlite3_clear_bindings( stmt );

    // binding GEOMETRY to Prepared Statement
    unsigned char *wkb = nullptr;
    int wkb_size;
    QByteArray iterWkb = iter->asWkb();
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
      const char *err = sqlite3_errmsg( mSqliteHandle );
      errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
      strcpy( errMsg, err );
      handleError( sql, errMsg, true );
      return false;
    }
  }
  sqlite3_finalize( stmt );

  ret = sqlite3_exec( mSqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  return true;
}

QgsVectorDataProvider::Capabilities QgsSpatiaLiteProvider::capabilities() const
{
  return mEnabledCapabilities;
}

QVariant QgsSpatiaLiteProvider::defaultValue( int fieldId ) const
{
  return mDefaultValues.value( fieldId, QVariant() );
}

bool QgsSpatiaLiteProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value ) const
{
  Q_UNUSED( constraint );

  // If the field is the primary key, skip in case it's autog-enerated / auto-incrementing
  if ( mAttributeFields.at( fieldIndex ).name() == mPrimaryKey  && mPrimaryKeyAutoIncrement )
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

QString QgsSpatiaLiteProvider::quotedIdentifier( QString id )
{
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsSpatiaLiteProvider::quotedValue( QString value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  value.replace( '\'', QLatin1String( "''" ) );
  return value.prepend( '\'' ).append( '\'' );
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
    mQuery = quotedIdentifier( mTableName );
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
                   "AND type in ('table', 'view') " ).arg( quotedValue( mTableName ) );

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    // checking if this one is a select query

    // get a new alias for the subquery
    int index = 0;
    QString alias;
    QRegExp regex;
    do
    {
      alias = QStringLiteral( "subQuery_%1" ).arg( QString::number( index++ ) );
      QString pattern = QStringLiteral( "(\\\"?)%1\\1" ).arg( QRegExp::escape( alias ) );
      regex.setPattern( pattern );
      regex.setCaseSensitivity( Qt::CaseInsensitive );
    }
    while ( mQuery.contains( regex ) );

    // convert the custom query into a subquery
    mQuery = QStringLiteral( "%1 as %2" )
             .arg( mQuery,
                   quotedIdentifier( alias ) );

    sql = QStringLiteral( "SELECT 0 FROM %1 LIMIT 1" ).arg( mQuery );
    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
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
          .arg( quotedValue( mTableName ),
                quotedValue( mGeometryColumn ) );

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret != SQLITE_OK )
    {
      if ( errMsg && strcmp( errMsg, "no such table: geometry_columns_auth" ) == 0 )
      {
        sqlite3_free( errMsg );
        sql = QStringLiteral( "SELECT 0 FROM geometry_columns WHERE upper(f_table_name) = upper(%1) and upper(f_geometry_column) = upper(%2)" )
              .arg( quotedValue( mTableName ),
                    quotedValue( mGeometryColumn ) );
        ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
                   " WHERE view_name=%1 and view_geometry=%2" ).arg( quotedValue( mTableName ),
                       quotedValue( mGeometryColumn ) );

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
                   " WHERE virt_name=%1 and virt_geometry=%2" ).arg( quotedValue( mTableName ),
                       quotedValue( mGeometryColumn ) );

    ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    mQuery = quotedIdentifier( mTableName );
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
      mGeomType = QgsWkbTypes::Point;
      break;
    case GAIA_VECTOR_LINESTRING:
      mGeomType = QgsWkbTypes::LineString;
      break;
    case GAIA_VECTOR_POLYGON:
      mGeomType = QgsWkbTypes::Polygon;
      break;
    case GAIA_VECTOR_MULTIPOINT:
      mGeomType = QgsWkbTypes::MultiPoint;
      break;
    case GAIA_VECTOR_MULTILINESTRING:
      mGeomType = QgsWkbTypes::MultiLineString;
      break;
    case GAIA_VECTOR_MULTIPOLYGON:
      mGeomType = QgsWkbTypes::MultiPolygon;
      break;
    default:
      mGeomType = QgsWkbTypes::Unknown;
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
                         "WHERE upper(view_name) = upper(%1) and upper(view_geometry) = upper(%2)" ).arg( quotedValue( mTableName ),
                             quotedValue( mGeometryColumn ) );
  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
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
    mGeomType = QgsWkbTypes::NoGeometry;
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

  QString sql = QString( "SELECT type, srid, spatial_index_enabled, coord_dimension FROM geometry_columns"
                         " WHERE upper(f_table_name) = upper(%1) and upper(f_geometry_column) = upper(%2)" ).arg( quotedValue( mTableName ),
                             quotedValue( mGeometryColumn ) );

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
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

      if ( fType == QLatin1String( "POINT" ) )
      {
        mGeomType = QgsWkbTypes::Point;
      }
      else if ( fType == QLatin1String( "MULTIPOINT" ) )
      {
        mGeomType = QgsWkbTypes::MultiPoint;
      }
      else if ( fType == QLatin1String( "LINESTRING" ) )
      {
        mGeomType = QgsWkbTypes::LineString;
      }
      else if ( fType == QLatin1String( "MULTILINESTRING" ) )
      {
        mGeomType = QgsWkbTypes::MultiLineString;
      }
      else if ( fType == QLatin1String( "POLYGON" ) )
      {
        mGeomType = QgsWkbTypes::Polygon;
      }
      else if ( fType == QLatin1String( "MULTIPOLYGON" ) )
      {
        mGeomType = QgsWkbTypes::MultiPolygon;
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
      else if ( dims == QLatin1String( "XYZM" ) )
      {
        nDims = GAIA_XY_Z_M;
        mGeomType = QgsWkbTypes::zmType( mGeomType, true, true );
      }

    }
  }
  sqlite3_free_table( results );

  if ( mGeomType == QgsWkbTypes::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg );
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
                         " WHERE upper(view_name) = upper(%1) and upper(view_geometry) = upper(%2)" ).arg( quotedValue( mTableName ),
                             quotedValue( mGeometryColumn ) );

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
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
        mGeomType = QgsWkbTypes::Point;
      }
      else if ( fType == QLatin1String( "MULTIPOINT" ) )
      {
        mGeomType = QgsWkbTypes::MultiPoint;
      }
      else if ( fType == QLatin1String( "LINESTRING" ) )
      {
        mGeomType = QgsWkbTypes::LineString;
      }
      else if ( fType == QLatin1String( "MULTILINESTRING" ) )
      {
        mGeomType = QgsWkbTypes::MultiLineString;
      }
      else if ( fType == QLatin1String( "POLYGON" ) )
      {
        mGeomType = QgsWkbTypes::Polygon;
      }
      else if ( fType == QLatin1String( "MULTIPOLYGON" ) )
      {
        mGeomType = QgsWkbTypes::MultiPolygon;
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

  if ( mGeomType == QgsWkbTypes::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg );
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
                         " WHERE virt_name=%1 and virt_geometry=%2" ).arg( quotedValue( mTableName ),
                             quotedValue( mGeometryColumn ) );

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
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
        mGeomType = QgsWkbTypes::Point;
      }
      else if ( fType == QLatin1String( "MULTIPOINT" ) )
      {
        mGeomType = QgsWkbTypes::MultiPoint;
      }
      else if ( fType == QLatin1String( "LINESTRING" ) )
      {
        mGeomType = QgsWkbTypes::LineString;
      }
      else if ( fType == QLatin1String( "MULTILINESTRING" ) )
      {
        mGeomType = QgsWkbTypes::MultiLineString;
      }
      else if ( fType == QLatin1String( "POLYGON" ) )
      {
        mGeomType = QgsWkbTypes::Polygon;
      }
      else if ( fType == QLatin1String( "MULTIPOLYGON" ) )
      {
        mGeomType = QgsWkbTypes::MultiPolygon;
      }
      mSrid = xSrid.toInt();

    }
  }
  sqlite3_free_table( results );

  if ( mGeomType == QgsWkbTypes::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg );
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
  QString sql = QStringLiteral( "select srid(%1), geometrytype(%1) from %2" )
                .arg( quotedIdentifier( mGeometryColumn ),
                      mQuery );

  //it is possible that the where clause restricts the feature type
  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mSubsetString;
  }

  sql += QLatin1String( " limit 1" );

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
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
      sql = QString( "select distinct "
                     "case"
                     " when geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
                     " when geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
                     " when geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
                     " end "
                     "from %2" )
            .arg( quotedIdentifier( mGeometryColumn ),
                  mQuery );

      if ( !mSubsetString.isEmpty() )
        sql += " where " + mSubsetString;

      ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
      if ( ret != SQLITE_OK )
      {
        handleError( sql, errMsg );
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

    if ( fType == QLatin1String( "POINT" ) )
    {
      mGeomType = QgsWkbTypes::Point;
    }
    else if ( fType == QLatin1String( "MULTIPOINT" ) )
    {
      mGeomType = QgsWkbTypes::MultiPoint;
    }
    else if ( fType == QLatin1String( "LINESTRING" ) )
    {
      mGeomType = QgsWkbTypes::LineString;
    }
    else if ( fType == QLatin1String( "MULTILINESTRING" ) )
    {
      mGeomType = QgsWkbTypes::MultiLineString;
    }
    else if ( fType == QLatin1String( "POLYGON" ) )
    {
      mGeomType = QgsWkbTypes::Polygon;
    }
    else if ( fType == QLatin1String( "MULTIPOLYGON" ) )
    {
      mGeomType = QgsWkbTypes::MultiPolygon;
    }
    mSrid = xSrid.toInt();
  }

  if ( mGeomType == QgsWkbTypes::Unknown || mSrid < 0 )
  {
    handleError( sql, errMsg );
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

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
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
    mLayerExtent.set( lyr->ExtentInfos->MinX, lyr->ExtentInfos->MinY,
                      lyr->ExtentInfos->MaxX, lyr->ExtentInfos->MaxY );
    mNumberFeatures = lyr->ExtentInfos->Count;
  }
  else
  {
    mLayerExtent.setMinimal();
    mNumberFeatures = 0;
  }

  return true;
}

bool QgsSpatiaLiteProvider::getTableSummary()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  QString sql = QStringLiteral( "SELECT Count(*)%1 FROM %2" )
                .arg( mGeometryColumn.isEmpty() ? QString() : QStringLiteral( ",Min(MbrMinX(%1)),Min(MbrMinY(%1)),Max(MbrMaxX(%1)),Max(MbrMaxY(%1))" ).arg( quotedIdentifier( mGeometryColumn ) ),
                      mQuery );

  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ')';
  }

  ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString count = results[( i * columns ) + 0];
      mNumberFeatures = count.toLong();

      if ( mGeometryColumn.isEmpty() )
      {
        mLayerExtent.setMinimal();
      }
      else
      {
        QString minX = results[( i * columns ) + 1];
        QString minY = results[( i * columns ) + 2];
        QString maxX = results[( i * columns ) + 3];
        QString maxY = results[( i * columns ) + 4];

        mLayerExtent.set( minX.toDouble(), minY.toDouble(), maxX.toDouble(), maxY.toDouble() );
      }
    }
  }
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

QGISEXTERN QVariantMap decodeUri( const QString &uri )
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "path" ), dsUri.database() );
  components.insert( QStringLiteral( "layerName" ), dsUri.table() );
  return components;
}

/**
 * Class factory to return a pointer to a newly created
 * QgsSpatiaLiteProvider object
 */
QGISEXTERN QgsSpatiaLiteProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsSpatiaLiteProvider( *uri, options );
}

/**
 * Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return SPATIALITE_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return SPATIALITE_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QgsVectorLayerExporter::ExportError createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsSpatiaLiteProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

// -------------

static bool initializeSpatialMetadata( sqlite3 *sqlite_handle, QString &errCause )
{
  // attempting to perform self-initialization for a newly created DB
  if ( !sqlite_handle )
    return false;

  // checking if this DB is really empty
  char **results = nullptr;
  int rows, columns;
  int ret = sqlite3_get_table( sqlite_handle, "select count(*) from sqlite_master", &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
    return false;

  int count = 0;
  if ( rows >= 1 )
  {
    for ( int i = 1; i <= rows; i++ )
      count = atoi( results[( i * columns ) + 0] );
  }

  sqlite3_free_table( results );

  if ( count > 0 )
    return false;

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
    errCause = QObject::tr( "Unable to initialize SpatialMetadata:\n" );
    errCause += QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    return false;
  }
  spatial_ref_sys_init( sqlite_handle, 0 );
  return true;
}

QGISEXTERN bool createDb( const QString &dbPath, QString &errCause )
{
  QgsDebugMsg( "creating a new db" );

  QFileInfo fullPath = QFileInfo( dbPath );
  QDir path = fullPath.dir();
  QgsDebugMsg( QString( "making this dir: %1" ).arg( path.absolutePath() ) );

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath() );

  // creating/opening the new database
  spatialite_database_unique_ptr database;
  int ret = database.open_v2( dbPath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( ret )
  {
    // an error occurred
    errCause = QObject::tr( "Could not create a new database\n" );
    errCause += database.errorMessage();
    return false;
  }
  // activating Foreign Key constraints
  char *errMsg = nullptr;
  ret = sqlite3_exec( database.get(), "PRAGMA foreign_keys = 1", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to activate FOREIGN_KEY constraints [%1]" ).arg( errMsg );
    sqlite3_free( errMsg );
    return false;
  }
  bool init_res = ::initializeSpatialMetadata( database.get(), errCause );

  return init_res;
}

// -------------

QGISEXTERN bool deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause )
{
  QgsDebugMsg( "deleting layer " + tableName );

  QgsSqliteHandle *hndl = QgsSqliteHandle::openDb( dbPath );
  if ( !hndl )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }
  sqlite3 *sqlite_handle = hndl->handle();
  int ret;
  if ( !gaiaDropTable( sqlite_handle, tableName.toUtf8().constData() ) )
  {
    // unexpected error
    errCause = QObject::tr( "Unable to delete table %1\n" ).arg( tableName );
    QgsSqliteHandle::closeDb( hndl );
    return false;
  }

  // TODO: remove spatial indexes?
  // run VACUUM to free unused space and compact the database
  ret = sqlite3_exec( sqlite_handle, "VACUUM", nullptr, nullptr, nullptr );
  if ( ret != SQLITE_OK )
  {
    QgsDebugMsg( "Failed to run VACUUM after deleting table on database " + dbPath );
  }

  QgsSqliteHandle::closeDb( hndl );

  return true;
}

QgsAttributeList QgsSpatiaLiteProvider::pkAttributeIndexes() const
{
  return mPrimaryKeyAttrs;
}

QList<QgsVectorLayer *> QgsSpatiaLiteProvider::searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  Q_FOREACH ( QgsVectorLayer *layer, layers )
  {
    const QgsSpatiaLiteProvider *slProvider = qobject_cast<QgsSpatiaLiteProvider *>( layer->dataProvider() );
    if ( slProvider && slProvider->mSqlitePath == connectionInfo && slProvider->mTableName == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}


QList<QgsRelation> QgsSpatiaLiteProvider::discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const
{
  QList<QgsRelation> output;
  const QString sql = QStringLiteral( "PRAGMA foreign_key_list(%1)" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( mTableName ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( mSqliteHandle, sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
        Q_FOREACH ( const QgsVectorLayer *foundLayer, foundLayers )
        {
          QgsRelation relation;
          relation.setName( name );
          relation.setReferencingLayer( self->id() );
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

QGISEXTERN bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                           const QString &styleName, const QString &styleDescription,
                           const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsg( "Database is: " + sqlitePath );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsg( "Connection to database failed. Save style aborted." );
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  sqlite3 *sqliteHandle = handle->handle();

  // check if layer_styles table already exist
  QString countIfExist = QStringLiteral( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1';" ).arg( QStringLiteral( "layer_styles" ) );

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
    ret = sqlite3_exec( sqliteHandle, createQuery.toUtf8().constData(), nullptr, nullptr, &errMsg );
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
    uiFileValue = QStringLiteral( ",%1" ).arg( QgsSpatiaLiteProvider::quotedValue( uiFileContent ) );
  }

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%11"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10%12"
                         ")" )
                .arg( QgsSpatiaLiteProvider::quotedValue( QString() ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( qmlStyle ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( sldStyle ) )
                .arg( useAsDefault ? "1" : "0" )
                .arg( QgsSpatiaLiteProvider::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.username() ) )
                .arg( uiFileColumn )
                .arg( uiFileValue );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_schema=%1"
                                " AND f_table_name=%2"
                                " AND f_geometry_column=%3"
                                " AND styleName=%4" )
                       .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
                       .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
                       .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) )
                       .arg( QgsSpatiaLiteProvider::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

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
    if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                QObject::tr( "A style named \"%1\" already exists in the database for this layer. Do you want to overwrite it?" )
                                .arg( styleName.isEmpty() ? dsUri.table() : styleName ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
    {
      QgsSqliteHandle::closeDb( handle );
      errCause = QObject::tr( "Operation aborted" );
      return false;
    }

    sql = QString( "UPDATE layer_styles"
                   " SET useAsDefault=%1"
                   ",styleQML=%2"
                   ",styleSLD=%3"
                   ",description=%4"
                   ",owner=%5"
                   " WHERE f_table_schema=%6"
                   " AND f_table_name=%7"
                   " AND f_geometry_column=%8"
                   " AND styleName=%9" )
          .arg( useAsDefault ? "1" : "0" )
          .arg( QgsSpatiaLiteProvider::quotedValue( qmlStyle ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( sldStyle ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.username() ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsSpatiaLiteProvider::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=0"
                                        " WHERE f_table_schema=%1"
                                        " AND f_table_name=%2"
                                        " AND f_geometry_column=%3" )
                               .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
                               .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
                               .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) );
    sql = QStringLiteral( "BEGIN; %1; %2; COMMIT;" ).arg( removeDefaultSql, sql );
  }

  ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
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


QGISEXTERN QString loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsgLevel( "Database is: " + sqlitePath, 5 );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsg( "Connection to database failed. Save style aborted." );
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString selectQmlQuery = QString( "SELECT styleQML"
                                    " FROM layer_styles"
                                    " WHERE f_table_schema=%1"
                                    " AND f_table_name=%2"
                                    " AND f_geometry_column=%3"
                                    " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                                    ",update_time DESC LIMIT 1" )
                           .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
                           .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
                           .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, selectQmlQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Could not load styles from %1 (Query: %2)" ).arg( sqlitePath, selectQmlQuery ) );
    errCause = QObject::tr( "Error executing loading style. The query was logged" );
    return QString();
  }

  QString style = ( rows == 1 ) ? QString::fromUtf8( results[( rows * columns ) + 0 ] ) : QString();
  sqlite3_free_table( results );

  QgsSqliteHandle::closeDb( handle );
  return style;
}

QGISEXTERN int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                           QStringList &descriptions, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsg( "Database is: " + sqlitePath );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsg( "Connection to database failed. Save style aborted." );
    errCause = QObject::tr( "Connection to database failed" );
    return -1;
  }

  sqlite3 *sqliteHandle = handle->handle();

  // check if layer_styles table already exist
  QString countIfExist = QStringLiteral( "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1';" ).arg( QStringLiteral( "layer_styles" ) );

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
    QgsMessageLog::logMessage( QObject::tr( "No styles available on DB" ) );
    errCause = QObject::tr( "No styles available on DB" );
    return false;
  }

  // get them
  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM layer_styles"
                                        " WHERE f_table_schema=%1"
                                        " AND f_table_name=%2"
                                        " AND f_geometry_column=%3"
                                        " ORDER BY useasdefault DESC, update_time DESC" )
                               .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
                               .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
                               .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) );

  ret = sqlite3_get_table( sqliteHandle, selectRelatedQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectRelatedQuery ) );
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

  QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                       " FROM layer_styles"
                                       " WHERE NOT (f_table_schema=%1 AND f_table_name=%2 AND f_geometry_column=%3)"
                                       " ORDER BY update_time DESC" )
                              .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.schema() ) )
                              .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.table() ) )
                              .arg( QgsSpatiaLiteProvider::quotedValue( dsUri.geometryColumn() ) );

  ret = sqlite3_get_table( sqliteHandle, selectOthersQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectOthersQuery ) );
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

QGISEXTERN QString getStyleById( const QString &uri, QString styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString sqlitePath = dsUri.database();
  QgsDebugMsg( "Database is: " + sqlitePath );

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsg( "Connection to database failed. Save style aborted." );
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString style;
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsSpatiaLiteProvider::quotedValue( styleId ) );
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
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" ).arg( QStringLiteral( "layer_styles" ) );
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

QGISEXTERN void cleanupProvider()
{
  QgsSpatiaLiteConnPool::cleanupInstance();
  QgsSqliteHandle::closeAll();
}

#ifdef HAVE_GUI

//! Provider for spatialite source select
class QgsSpatialiteSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "spatialite" ); }
    QString text() const override { return QObject::tr( "SpatiaLite" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 10; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsSpatiaLiteSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsSpatialiteSourceSelectProvider;

  return providers;
}
#endif
