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
#include "qgsslconnect.h"
#include "qgsspatialiteprovider.h"
#include "qgsspatialiteconnpool.h"
#include "qgsspatialitefeatureiterator.h"
#include "qgsfeedback.h"

#include <qgsjsonutils.h>
#include <qgsvectorlayer.h>

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include "qgsspatialiteutils.h"

//----------------------------------------------------------
//-- Mandatory functions for each Provider
//--> each Provider must be created in an extra library
//----------------------------------------------------------
const QString SPATIALITE_KEY = QStringLiteral( "spatialite" );
const QString SPATIALITE_DESCRIPTION = QStringLiteral( "SpatiaLite data provider" );

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

/** Required key function (used to map the plugin to a data store type)
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
 * Class factory to return a pointer to a newly created
 * QgsSpatiaLiteProvider object
 */
QGISEXTERN QgsSpatiaLiteProvider *classFactory( const QString *uri )
{
  return new QgsSpatiaLiteProvider( *uri );
}
//----------------------------------------------------------
QgsSpatiaLiteProvider::QgsSpatiaLiteProvider( QString const &uri )
  : QgsVectorDataProvider( uri )
  , mIsQuery( false )
  , mTableBased( false )
  , mViewBased( false )
  , mVShapeBased( false )
  , mReadOnly( false )
  , mUriTableName( QString::null )
  , mUriGeometryColumn( QString::null )
  , mUriLayerName( QString::null )
  , mGeometryType( QgsWkbTypes::Unknown )
  , mSpatialIndexRTree( false )
  , mSpatialIndexMbrCache( false )
{
  QgsDataSourceUri anUri = QgsDataSourceUri( uri );
  // parsing members from the uri structure
  mUriTableName = anUri.table();
  mUriGeometryColumn = anUri.geometryColumn().toLower();
  mSqlitePath = anUri.database();
  mSubsetString = anUri.sql(); // \"id_admin\"  < 2
  mUriPrimaryKey = anUri.keyColumn();
  mQuery = mUriTableName;
  // trying to open the SQLite DB
  bool bShared = true;
  bool bLoadLayers = true;
#if 1
  mUriLayerName = mUriTableName;
  if ( !mUriGeometryColumn.isEmpty() )
  {
    mUriLayerName = QString( "%1(%2)" ).arg( mUriTableName ).arg( mUriGeometryColumn );
    bLoadLayers = true;
  }
#endif
  if ( setSqliteHandle( QgsSqliteHandle::openDb( mSqlitePath, bShared, mUriLayerName, bLoadLayers ) ) )
  {
    QStringList pragmaList = anUri.params( QStringLiteral( "pragma" ) );
    Q_FOREACH ( const QString &pragma, pragmaList )
    {
      char *errMsg = nullptr;
      int ret = sqlite3_exec( dbSqliteHandle(), ( "PRAGMA " + pragma ).toUtf8(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
      {
        QgsDebugMsg( QString( "PRAGMA " ) + pragma + QString( " failed : %1" ).arg( errMsg ? errMsg : "" ) );
      }
      sqlite3_free( errMsg );
    }
    mValid = true;
  }
  else
  {
    mValid = false;
  }
}

QgsSpatiaLiteProvider::~QgsSpatiaLiteProvider()
{
  closeDb();
  invalidateConnections( mSqlitePath );
}

bool QgsSpatiaLiteProvider::isValid() const
{
  return isLayerValid();
}

QString QgsSpatiaLiteProvider::name() const
{
  return SPATIALITE_KEY;
}

QString QgsSpatiaLiteProvider::description() const
{
  return SPATIALITE_DESCRIPTION;
}

QgsFields QgsSpatiaLiteProvider::fields() const
{
  return getAttributeFields();
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

void QgsSpatiaLiteProvider::invalidateConnections( const QString &connection )
{
  QgsSpatiaLiteConnPool::instance()->invalidateConnections( connection );
}

bool QgsSpatiaLiteProvider::setSqliteHandle( QgsSqliteHandle *sqliteHandle )
{
  bool bRc = false;
  mHandle = sqliteHandle;
  bool bLoadLayer = true;
  if ( !mHandle )
  {
    return bRc;
  }
  if ( mHandle )
  {
    mSpatialiteDbInfo = mHandle->getSpatialiteDbInfo();
    if ( ( mSpatialiteDbInfo ) && ( isDbValid() )  && ( isDbSpatialite() ) )
    {
      // -- ---------------------------------- --
      // The combination isDbValid() and isDbSpatialite()
      //  - means that the given Layer is supported by the QgsSpatiaLiteProvider
      //  --> i.e. not GeoPackage, MBTiles etc.
      //  - RasterLite1 will return isDbGdalOgr() == 1
      //  -> which renders the RasterLayers with gdal
      //  --> so a check must be done later
      //  ---> that the layer is not a RasterLite1-Layer
      // -- ---------------------------------- --
      QString sLayerName = mUriTableName;
      if ( !mUriGeometryColumn.isEmpty() )
      {
        sLayerName = QString( "%1(%2)" ).arg( mUriTableName ).arg( mUriGeometryColumn );
      }
      bRc = setDbLayer( mSpatialiteDbInfo->getSpatialiteDbLayer( sLayerName, bLoadLayer ) );
    }
  }
  return bRc;
}
bool QgsSpatiaLiteProvider::setDbLayer( SpatialiteDbLayer *dbLayer )
{
  bool bRc = false;
  if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) && ( dbLayer->isLayerSpatialite() ) )
  {
    mDbLayer = dbLayer;
    // mDbLayer->setLayerQuery(mSubsetString);
    mSrid = mDbLayer->getSrid();
    mGeometryType = mDbLayer->getGeometryType();
    if ( mDbLayer->getSpatialIndexType() == GAIA_SPATIAL_INDEX_RTREE )
    {
      mSpatialIndexRTree = true;
    }
    if ( mDbLayer->getSpatialIndexType() == GAIA_SPATIAL_INDEX_MBRCACHE )
    {
      mSpatialIndexMbrCache = true;
    }
    if ( ( mDbLayer->getLayerType() == SpatialiteDbInfo::SpatialTable ) ||
         ( mDbLayer->getLayerType() == SpatialiteDbInfo::TopologyExport ) )
    {
      mTableBased = true;
    }
    if ( mDbLayer->getLayerType() == SpatialiteDbInfo::SpatialView )
    {
      mViewBased = true;
    }
    if ( mDbLayer->getLayerType() == SpatialiteDbInfo::VirtualShape )
    {
      mVShapeBased = true;
    }
    //fill type names into sets
    setNativeTypes( QList<NativeType>()
                    << QgsVectorDataProvider::NativeType( tr( "Binary object (BLOB)" ), QStringLiteral( "BLOB" ), QVariant::ByteArray )
                    << QgsVectorDataProvider::NativeType( tr( "Text" ), QStringLiteral( "TEXT" ), QVariant::String )
                    << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "FLOAT" ), QVariant::Double )
                    << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), QStringLiteral( "INTEGER" ), QVariant::LongLong )

                    // date type
                    << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
                    << QgsVectorDataProvider::NativeType( tr( "Time" ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
                    << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), QStringLiteral( "timestamp without time zone" ), QVariant::DateTime, -1, -1, -1, -1 )

                    << QgsVectorDataProvider::NativeType( tr( "Array of text" ), SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.toUpper() + "TEXT" + SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::StringList, 0, 0, 0, 0, QVariant::String )
                    << QgsVectorDataProvider::NativeType( tr( "Array of decimal numbers (double)" ), SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.toUpper() + "REAL" + SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::Double )
                    << QgsVectorDataProvider::NativeType( tr( "Array of whole numbers (integer)" ), SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX.toUpper() + "INTEGER" + SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX.toUpper(), QVariant::List, 0, 0, 0, 0, QVariant::LongLong )
                  );
    // bRc = checkQuery();
  }
  return bRc;
}

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
      fieldType = SpatialiteDbInfo::SPATIALITE_ARRAY_PREFIX + subField.typeName() + SpatialiteDbInfo::SPATIALITE_ARRAY_SUFFIX;
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
        throw QgsSpatiaLiteUtils::SLException( errMsg );

      toCommit = true;

      if ( overwrite )
      {
        // delete the table if exists and the related entry in geometry_columns, then re-create it
        sql = QStringLiteral( "DROP TABLE IF EXISTS %1" )
              .arg( QgsSpatiaLiteUtils::quotedIdentifier( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw QgsSpatiaLiteUtils::SLException( errMsg );

        sql = QStringLiteral( "DELETE FROM geometry_columns WHERE upper(f_table_name) = upper(%1)" )
              .arg( QgsSpatiaLiteUtils::quotedValue( tableName ) );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw QgsSpatiaLiteUtils::SLException( errMsg );
      }

      sql = QStringLiteral( "CREATE TABLE %1 (%2 %3 PRIMARY KEY)" )
            .arg( QgsSpatiaLiteUtils::quotedIdentifier( tableName ),
                  QgsSpatiaLiteUtils::quotedIdentifier( primaryKey ),
                  primaryKeyType );

      ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw QgsSpatiaLiteUtils::SLException( errMsg );

      // get geometry type, dim and srid
      int dim = 2;
      long srid = srs.postgisSrid();

      switch ( wkbType )
      {
        case QgsWkbTypes::Point25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::Point:
          geometryType = QStringLiteral( "POINT" );
          break;

        case QgsWkbTypes::LineString25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::LineString:
          geometryType = QStringLiteral( "LINESTRING" );
          break;

        case QgsWkbTypes::Polygon25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::Polygon:
          geometryType = QStringLiteral( "POLYGON" );
          break;

        case QgsWkbTypes::MultiPoint25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::MultiPoint:
          geometryType = QStringLiteral( "MULTIPOINT" );
          break;

        case QgsWkbTypes::MultiLineString25D:
          dim = 3;
          FALLTHROUGH;
        case QgsWkbTypes::MultiLineString:
          geometryType = QStringLiteral( "MULTILINESTRING" );
          break;

        case QgsWkbTypes::MultiPolygon25D:
          dim = 3;
          FALLTHROUGH;
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
              .arg( QgsSpatiaLiteUtils::quotedValue( tableName ),
                    QgsSpatiaLiteUtils::quotedValue( geometryColumn ) )
              .arg( srid )
              .arg( QgsSpatiaLiteUtils::quotedValue( geometryType ) )
              .arg( dim );

        ret = sqlite3_exec( sqliteHandle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
        if ( ret != SQLITE_OK )
          throw QgsSpatiaLiteUtils::SLException( errMsg );
      }
      else
      {
        geometryColumn = QString();
      }

      ret = sqlite3_exec( sqliteHandle, "COMMIT", nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
        throw QgsSpatiaLiteUtils::SLException( errMsg );

    }
    catch ( QgsSpatiaLiteUtils::SLException &e )
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
  dsUri.setDataSource( QLatin1String( "" ), tableName, geometryColumn, QString(), primaryKey );
  QgsSpatiaLiteProvider *provider = new QgsSpatiaLiteProvider( dsUri.uri() );
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

QgsAbstractFeatureSource *QgsSpatiaLiteProvider::featureSource() const
{
  return new QgsSpatiaLiteFeatureSource( this );
}

QString QgsSpatiaLiteProvider::storageType() const
{
  return QStringLiteral( "SQLite database with SpatiaLite extension" );
}

QgsFeatureIterator QgsSpatiaLiteProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !isLayerValid() )
  {
    QgsDebugMsg( "Read attempt on an invalid SpatiaLite data source" );
    return QgsFeatureIterator();
  }
  return QgsFeatureIterator( new QgsSpatiaLiteFeatureIterator( new QgsSpatiaLiteFeatureSource( this ), true, request ) );
}

QString QgsSpatiaLiteProvider::subsetString() const
{
  return mSubsetString;
}

bool QgsSpatiaLiteProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  QString prevSubsetString = mSubsetString;
  mSubsetString = theSQL;
  // update URI
  QgsDataSourceUri uri = QgsDataSourceUri( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );
  // update feature count and extents
  if ( updateFeatureCount )
  {
    getLayerExtent( true, true );
    return true;
  }
  mSubsetString = prevSubsetString;
  // restore URI
  uri = QgsDataSourceUri( dataSourceUri() );
  uri.setSql( mSubsetString );
  setDataSourceUri( uri.uri() );
  getLayerExtent( true, true );
  emit dataChanged();
  return false;
}
QgsRectangle QgsSpatiaLiteProvider::extent() const
{
  return getLayerExtent( false, false );
}
void QgsSpatiaLiteProvider::updateExtents()
{
  getLayerExtent( true, true );
}
size_t QgsSpatiaLiteProvider::layerCount() const
{
  return 1;
}

/**
 * Return the feature type
 */
QgsWkbTypes::Type QgsSpatiaLiteProvider::wkbType() const
{
  return getGeometryType() ;
}

/**
 * Return the feature type
 */
long QgsSpatiaLiteProvider::featureCount() const
{
  return getNumberFeatures();
}
QgsCoordinateReferenceSystem QgsSpatiaLiteProvider::crs() const
{
  QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( getAuthId() );
  if ( !srs.isValid() )
  {
    srs = QgsCoordinateReferenceSystem::fromProj4( getProj4text() );
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

//-----------------------------------------------------------
//  SpatialiteDbLayer should never store the
// - mQuery and mSubsetString members
// --> since other source may be using the Layer
// Functions using these 'filters' must remain in QgsSpatiaLiteProvider
//-----------------------------------------------------------
// Returns the minimum value of an attribute
// - uses mQuery and mSubsetString
//-----------------------------------------------------------
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
    sql = QStringLiteral( "SELECT Min(%1) FROM %2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }
    ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
  catch ( QgsSpatiaLiteUtils::SLFieldNotFound )
  {
    return QVariant( QVariant::Int );
  }
}
//-----------------------------------------------------------
// Returns the maximum value of an attribute
// - uses mQuery and mSubsetString
//-----------------------------------------------------------
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
    sql = QStringLiteral( "SELECT Max(%1) FROM %2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
    if ( !mSubsetString.isEmpty() )
    {
      sql += " WHERE ( " + mSubsetString + ')';
    }
    ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
  catch ( QgsSpatiaLiteUtils::SLFieldNotFound )
  {
    return QVariant( QVariant::Int );
  }
}
//-----------------------------------------------------------
// Returns the list of unique values of an attribute
// - uses mQuery and mSubsetString
//-----------------------------------------------------------
QSet<QVariant> QgsSpatiaLiteProvider::uniqueValues( int index, int limit ) const
{
  sqlite3_stmt *stmt = nullptr;
  QString sql;
  QSet<QVariant> uniqueValues;
  // get the field name
  if ( index < 0 || index >= getAttributeFields().count() )
  {
    return uniqueValues; //invalid field
  }
  QgsField fld = getAttributeFields().at( index );
  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE ( " + mSubsetString + ')';
  }
  sql += QStringLiteral( " ORDER BY %1" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) );
  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }
  // SQLite prepared statement
  if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
    return uniqueValues;
  }
  while ( 1 )
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
          uniqueValues.insert( QVariant( getAttributeFields().at( index ).type() ) );
          break;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return uniqueValues;
    }
  }
  sqlite3_finalize( stmt );

  return uniqueValues;
}
//-----------------------------------------------------------
// Returns the list of uniqueStringsMatching
// - uses mQuery and mSubsetString
//-----------------------------------------------------------
QStringList QgsSpatiaLiteProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;
  sqlite3_stmt *stmt = nullptr;
  QString sql;
  // get the field name
  if ( index < 0 || index >= getAttributeFields().count() )
  {
    return results; //invalid field
  }
  QgsField fld = getAttributeFields().at( index );
  sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 " ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ), mQuery );
  sql += QStringLiteral( " WHERE " ) + QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) + QStringLiteral( " LIKE '%" ) + substring + QStringLiteral( "%'" );
  if ( !mSubsetString.isEmpty() )
  {
    sql += QStringLiteral( " AND ( " ) + mSubsetString + ')';
  }
  sql += QStringLiteral( " ORDER BY %1" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( fld.name() ) );
  if ( limit >= 0 )
  {
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );
  }
  // SQLite prepared statement
  if ( sqlite3_prepare_v2( dbSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
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
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( dbSqliteHandle() ) ), tr( "SpatiaLite" ) );
      sqlite3_finalize( stmt );
      return results;
    }
  }
  sqlite3_finalize( stmt );
  return results;
}

bool QgsSpatiaLiteProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  QString errorMessage;
  if ( !getDbLayer()->addLayerFeatures( flist, flags, errorMessage ) )
  {
    pushError( errorMessage );
    return false;
  }
  return true;
}

bool QgsSpatiaLiteProvider::createAttributeIndex( int field )
{
  return getDbLayer()->createLayerAttributeIndex( field );
}

bool QgsSpatiaLiteProvider::deleteFeatures( const QgsFeatureIds &id )
{
  QString errorMessage;
  if ( !getDbLayer()->deleteLayerFeatures( id, errorMessage ) )
  {
    pushError( errorMessage );
    return false;
  }
  return true;
}

bool QgsSpatiaLiteProvider::truncate()
{
  return getDbLayer()->truncateLayerTableRows();
}

bool QgsSpatiaLiteProvider::addAttributes( const QList<QgsField> &attributes )
{
  return getDbLayer()->addLayerAttributes( attributes );
}

bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  return getDbLayer()->changeLayerAttributeValues( attr_map );
}

bool QgsSpatiaLiteProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  return getDbLayer()->changeLayerGeometryValues( geometry_map );
}

QgsVectorDataProvider::Capabilities QgsSpatiaLiteProvider::capabilities() const
{
  return getCapabilities();
}

QVariant QgsSpatiaLiteProvider::defaultValue( int fieldId ) const
{
  return getDefaultValues().value( fieldId, QVariant() );
}

//-----------------------------------------------------------
// Check the validaty of a possible sugstring query
// - not documented, so I am guessing
//-----------------------------------------------------------
bool QgsSpatiaLiteProvider::checkQuery()
{
  int count = 0;
  mIsQuery = false;
  if ( mQuery.startsWith( '(' ) && mQuery.endsWith( ')' ) )
  {
    QString sql;
    // checking if this one is a select query
    int ret;
    char **results = nullptr;
    char *errMsg = nullptr;
    int rows, columns;
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
    mQuery = QStringLiteral( "%1 as %2" ).arg( mQuery, QgsSpatiaLiteUtils::quotedIdentifier( alias ) );
    sql = QStringLiteral( "SELECT 0 FROM %1 LIMIT 1" ).arg( mQuery );
    ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
    if ( ret == SQLITE_OK && rows == 1 )
    {
      mIsQuery = true;
      mReadOnly = true;
      count++;
    }
    sqlite3_free_table( results );
  }
  if ( !mIsQuery )
  {
    mQuery = QgsSpatiaLiteUtils::quotedIdentifier( getTableName() );
  }
  // checking for validity
  return count == 1;
}
QgsField QgsSpatiaLiteProvider::field( int index ) const
{
  return getDbLayer()->getAttributeField( index );
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

#if 0
//-----------------------------------------------------------
// TODO: determine if these functions should be moved to QgsSpatiaLiteUtils
// - not documented: used only for  'QGISEXTERN bool createDb'
//-----------------------------------------------------------
// this functionality is now done: QgsSpatiaLiteUtils::createSpatialDatabase
//-----------------------------------------------------------
static bool initializeSpatialMetadata( sqlite3 *sqlite_handle, QString &errCause )
{
  // attempting to perform self-initialization for a newly created DB
  if ( !sqlite_handle )
  {
    return false;
  }
  // checking if this DB is really empty
  char **results = nullptr;
  int rows, columns;
  int ret = sqlite3_get_table( sqlite_handle, "select count(*) from sqlite_master", &results, &rows, &columns, nullptr );
  if ( ret != SQLITE_OK )
  {
    return false;
  }
  int count = 0;
  if ( rows >= 1 )
  {
    for ( int i = 1; i <= rows; i++ )
    {
      count = atoi( results[( i * columns ) + 0] );
    }
  }
  sqlite3_free_table( results );
  if ( count > 0 )
  {
    return false;
  }
  bool above41 = false;
  ret = sqlite3_get_table( sqlite_handle, "select spatialite_version()", &results, &rows, &columns, nullptr );
  if ( ret == SQLITE_OK && rows == 1 && columns == 1 )
  {
    QString version = QString::fromUtf8( results[1] );
    QStringList parts = version.split( ' ', QString::SkipEmptyParts );
    if ( parts.size() >= 1 )
    {
      QStringList verparts = parts[0].split( '.', QString::SkipEmptyParts );
      above41 = verparts.size() >= 2 && ( verparts[0].toInt() > 4 || ( verparts[0].toInt() == 4 && verparts[1].toInt() >= 1 ) );
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
#endif
//-----------------------------------------------------------
// TODO: determine if these functions should be moved to QgsSpatiaLiteUtils
// - not documented:
// used  for 'QgsSLRootItem::createDatabase()' and
// and for 'QgsNewSpatialiteLayerDialog::createDb'
//-----------------------------------------------------------
// this function now calls: QgsSpatiaLiteUtils::createSpatialDatabase
//-----------------------------------------------------------
QGISEXTERN bool createDb( const QString &dbPath, QString &errCause )
{
  bool bRc = false;
  QgsDebugMsg( "creating a new db" );

  QFileInfo fullPath = QFileInfo( dbPath );
  QDir path = fullPath.dir();
  QgsDebugMsg( QString( "making this dir: %1" ).arg( path.absolutePath() ) );

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath() );
  QString sDatabaseFileName = dbPath;
  SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::Spatialite45;
  bRc = QgsSpatiaLiteUtils::createSpatialDatabase( sDatabaseFileName, errCause, dbCreateOption );

  return bRc;
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
#ifdef SPATIALITE_VERSION_GE_4_0_0
  // only if libspatialite version is >= 4.0.0
  // if libspatialite is v.4.0 (or higher) using the internal library
  // method is highly recommended
  if ( !gaiaDropTable( sqlite_handle, tableName.toUtf8().constData() ) )
  {
    // unexpected error
    errCause = QObject::tr( "Unable to delete table %1\n" ).arg( tableName );
    QgsSqliteHandle::closeDb( hndl );
    return false;
  }
#else
  // drop the table
  QString sql = QString( "DROP TABLE " ) + QgsSpatiaLiteUtils::quotedIdentifier( tableName );
  QgsDebugMsg( sql );
  char *errMsg = nullptr;
  ret = sqlite3_exec( sqlite_handle, sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to delete table %1:\n" ).arg( tableName );
    errCause += QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    QgsSqliteHandle::closeDb( hndl );
    return false;
  }

  // remove table from geometry columns
  sql = QString( "DELETE FROM geometry_columns WHERE upper(f_table_name) = upper(%1)" )
        .arg( QgsSpatiaLiteUtils::quotedValue( tableName ) );
  ret = sqlite3_exec( sqlite_handle, sql.toUtf8().constData(), nullptr, nullptr, nullptr );
  if ( ret != SQLITE_OK )
  {
    QgsDebugMsg( "sqlite error: " + QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) ) );
  }
#endif

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
  return getPrimaryKeyAttrs();
}

QList<QgsVectorLayer *> QgsSpatiaLiteProvider::searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  Q_FOREACH ( QgsVectorLayer *layer, layers )
  {
    const QgsSpatiaLiteProvider *slProvider = qobject_cast<QgsSpatiaLiteProvider *>( layer->dataProvider() );
    if ( slProvider && slProvider->mSqlitePath == connectionInfo && slProvider->getTableName() == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}


QList<QgsRelation> QgsSpatiaLiteProvider::discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const
{
  QList<QgsRelation> output;
  const QString sql = QStringLiteral( "PRAGMA foreign_key_list(%1)" ).arg( QgsSpatiaLiteUtils::quotedIdentifier( getTableName() ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( dbSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( ret == SQLITE_OK )
  {
    int nbFound = 0;
    for ( int row = 1; row <= rows; ++row )
    {
      const QString name = "fk_" + getTableName() + "_" + QString::fromUtf8( results[row * columns + 0] );
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
  // Avoid sql-statement errors
  if ( dsUri.schema().isEmpty() )
    return false;

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
    uiFileValue = QStringLiteral( ",%1" ).arg( QgsSpatiaLiteUtils::quotedValue( uiFileContent ) );
  }

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%11"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10%12"
                         ")" )
                .arg( QgsSpatiaLiteUtils::quotedValue( QString() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( qmlStyle ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( sldStyle ) )
                .arg( useAsDefault ? "1" : "0" )
                .arg( QgsSpatiaLiteUtils::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.username() ) )
                .arg( uiFileColumn )
                .arg( uiFileValue );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_schema=%1"
                                " AND f_table_name=%2"
                                " AND f_geometry_column=%3"
                                " AND styleName=%4" )
                       .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                       .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                       .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) )
                       .arg( QgsSpatiaLiteUtils::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

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
          .arg( QgsSpatiaLiteUtils::quotedValue( qmlStyle ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( sldStyle ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.username() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsSpatiaLiteUtils::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=0"
                                        " WHERE f_table_schema=%1"
                                        " AND f_table_name=%2"
                                        " AND f_geometry_column=%3" )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );
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
  QgsDebugMsg( "Database is: " + sqlitePath );
  // Avoid sql-statement errors
  if ( dsUri.schema().isEmpty() )
  {
    QgsDebugMsg( "Retrieving style failed. Load style aborted." );
    errCause = QObject::tr( "Missing table_schema " );
    return QString();
  }

  // trying to open the SQLite DB
  QgsSqliteHandle *handle = QgsSqliteHandle::openDb( sqlitePath );
  if ( !handle )
  {
    QgsDebugMsg( "Connection to database failed. Load style aborted." );
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
                           .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                           .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                           .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );

  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( sqliteHandle, selectQmlQuery.toUtf8().constData(), &results, &rows, &columns, &errMsg );
  if ( SQLITE_OK != ret )
  {
    QgsSqliteHandle::closeDb( handle );
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing loading style. The query was logged" );
    return QLatin1String( "" );
  }

  QString style = ( rows == 1 ) ? QString::fromUtf8( results[( rows * columns ) + 0 ] ) : QLatin1String( "" );
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
                                        " AND f_geometry_column=%3" )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                               .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );

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
                              .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.schema() ) )
                              .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.table() ) )
                              .arg( QgsSpatiaLiteUtils::quotedValue( dsUri.geometryColumn() ) );

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
    return QLatin1String( "" );
  }

  sqlite3 *sqliteHandle = handle->handle();

  QString style;
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsSpatiaLiteUtils::quotedValue( styleId ) );
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
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  QgsSqliteHandle::closeDb( handle );
  sqlite3_free_table( results );
  return style;
}

QGISEXTERN void cleanupProvider()
{
  QgsSqliteHandle::closeAll();
}

