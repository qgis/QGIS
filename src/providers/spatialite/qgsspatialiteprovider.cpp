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


const QString SPATIALITE_KEY = QStringLiteral( "spatialite" );
const QString SPATIALITE_DESCRIPTION = QStringLiteral( "SpatiaLite data provider" );


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

      sql = QStringLiteral( "CREATE TABLE %1 (%2 %3 PRIMARY KEY)" )
            .arg( quotedIdentifier( tableName ),
                  quotedIdentifier( primaryKey ),
                  primaryKeyType );

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

bool QgsSpatiaLiteProvider::setSqliteHandle( QgsSqliteHandle *sqliteHandle )
{
  bool bRc = false;
  mHandle = sqliteHandle;
  if ( !mHandle )
  {
    return bRc;
  }
  if ( mHandle )
  {
    mSpatialiteDbInfo = mHandle->getSpatialiteDbInfo();
    if ( ( mSpatialiteDbInfo ) && ( isDbValid() )  && ( !isDbGdalOgr() ) )
    {
      // -- ---------------------------------- --
      // The combination isDbValid() and !isDbGdalOgr()
      //  - means that the given Layer is supported by the QgsSpatiaLiteProvider
      //  --> i.e. not GeoPackage, MBTiles etc.
      // -- ---------------------------------- --
      QString sLayerName = mUriTableName;
      if ( !mUriGeometryColumn.isEmpty() )
      {
        sLayerName = QString( "%1(%2)" ).arg( mUriTableName ).arg( mUriGeometryColumn );
      }
      qDebug() << QString( "QgsSpatiaLiteProvider::setSqliteHandle(%1) -z- LayersLoaded[%2] LayersFound[%3]" ).arg( sLayerName ).arg( dbLayersCount() ).arg( dbVectorLayersCount() );
      bRc = setDbLayer( mSpatialiteDbInfo->getSpatialiteDbLayer( sLayerName ) );
    }
  }
  return bRc;
}
bool QgsSpatiaLiteProvider::setDbLayer( SpatialiteDbLayer *dbLayer )
{
  bool bRc = false;
  if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) )
  {
    mDbLayer = dbLayer;
    // mDbLayer->setLayerQuery(mSubsetString);
    mSrid = mDbLayer->getSrid();
    mGeomType = mDbLayer->getGeomType();
    if ( mDbLayer->getSpatialIndexType() == GAIA_SPATIAL_INDEX_RTREE )
    {
      mSpatialIndexRTree = true;
    }
    if ( mDbLayer->getSpatialIndexType() == GAIA_SPATIAL_INDEX_MBRCACHE )
    {
      mSpatialIndexMbrCache = true;
    }
    if ( ( mDbLayer->getLayerType() == SpatialiteDbInfo::SpatialTable ) ||
         ( mDbLayer->getLayerType() == SpatialiteDbInfo::TopopogyLayer ) )
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
    qDebug() << QString( "QgsSpatiaLiteProvider::setDbLayer(%1) -z- DataSourceUri[%2]" ).arg( getLayerName() ).arg( layerConnectionInfo() );
  }
  return bRc;
}
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
  , mGeomType( QgsWkbTypes::Unknown )
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
  // qDebug() << QString( "QgsSpatiaLiteProvider::QgsSpatiaLiteProvider -1- driver[%1] schema[%2] mSubsetString[%3]" ).arg( anUri.driver() ).arg( anUri.schema() ).arg( mSubsetString );
  qDebug() << QString( "QgsSpatiaLiteProvider::QgsSpatiaLiteProvider -0- QgsDataSourceUri[%1] " ).arg( uri );
  // trying to open the SQLite DB
  bool bShared = true;
  bool bLoadLayers = false;
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
      int ret = sqlite3_exec( getSqliteHandle(), ( "PRAGMA " + pragma ).toUtf8(), nullptr, nullptr, &errMsg );
      if ( ret != SQLITE_OK )
      {
        QgsDebugMsg( QString( "PRAGMA " ) + pragma + QString( " failed : %1" ).arg( errMsg ? errMsg : "" ) );
      }
      sqlite3_free( errMsg );
    }
    mValid = true;
    qDebug() << QString( "QgsSpatiaLiteProvider::QgsSpatiaLiteProvider(%1)  -z- DataSourceUri[%2] " ).arg( dbLayersCount() ).arg( dbConnectionInfo() );
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

QgsAbstractFeatureSource *QgsSpatiaLiteProvider::featureSource() const
{
  return new QgsSpatiaLiteFeatureSource( this );
}

/*
// Note 20170523: not sure if this is still needed.
void QgsSpatiaLiteProvider::updatePrimaryKeyCapabilities()
{
  if ( getPrimaryKey().isEmpty() )
  {
    mEnabledCapabilities &= ~QgsVectorDataProvider::SelectAtId;
  }
  else
  {
    mEnabledCapabilities |= QgsVectorDataProvider::SelectAtId;
  }
}
*/
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
    ( void )sqlite3_exec( getSqliteHandle(), "ROLLBACK", nullptr, nullptr, nullptr );
  }
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
  return getGeomType();
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
}                               //  QgsSpatiaLiteProvider::description()

QgsFields QgsSpatiaLiteProvider::fields() const
{
  return getAttributeFields();
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

    ret = sqlite3_get_table( getSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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

    ret = sqlite3_get_table( getSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
  if ( sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( getSqliteHandle() ) ), tr( "SpatiaLite" ) );
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
          uniqueValues.insert( QVariant( mAttributeFields.at( index ).type() ) );
          break;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( getSqliteHandle() ) ), tr( "SpatiaLite" ) );
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
  if ( sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( getSqliteHandle() ) ), tr( "SpatiaLite" ) );
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
      QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( getSqliteHandle() ) ), tr( "SpatiaLite" ) );
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
  bool hasMultiFunction = dbSpatialiteVersionMajor() > 2 ||
                          ( dbSpatialiteVersionMajor() == 2 && dbSpatialiteVersionMinor() >= 4 );

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

  ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret == SQLITE_OK )
  {
    toCommit = true;

    sql = QStringLiteral( "INSERT INTO %1(" ).arg( quotedIdentifier( getTableName() ) );
    values = QStringLiteral( ") VALUES (" );
    separator = QLatin1String( "" );

    if ( !getGeometryColumn().isEmpty() )
    {
      sql += separator + quotedIdentifier( getGeometryColumn() );
      values += separator + geomParam();
      separator = ',';
    }

    for ( int i = 0; i < attributevec.count(); ++i )
    {
      if ( i >= mAttributeFields.count() )
        continue;

      QString fieldname = mAttributeFields.at( i ).name();
      if ( fieldname.isEmpty() || fieldname == getGeometryColumn() )
        continue;

      sql += separator + quotedIdentifier( fieldname );
      values += separator + '?';
      separator = ',';
    }

    sql += values;
    sql += ')';

    // SQLite prepared statement
    ret = sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr );
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

        if ( !getGeometryColumn().isEmpty() )
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
            QByteArray featureWkb = feature->geometry().exportToWkb();
            // convertFromGeosWKB( reinterpret_cast<const unsigned char *>( featureWkb.constData() ),featureWkb.length(),&wkb, &wkb_size, nDims );
            QgsSpatiaLiteUtils::convertFromGeosWKB( reinterpret_cast<const unsigned char *>( featureWkb.constData() ), featureWkb.length(), &wkb, &wkb_size, getCoordDimensions() );
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
          if ( fieldname.isEmpty() || fieldname == getGeometryColumn() )
            continue;

          QVariant::Type type = mAttributeFields.at( i ).type();

          if ( !v.isValid() )
          {
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
            feature->setId( sqlite3_last_insert_rowid( getSqliteHandle() ) );
          }
          //mNumberFeatures++;
        }
        else
        {
          // some unexpected error occurred
          const char *err = sqlite3_errmsg( getSqliteHandle() );
          errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
          strcpy( errMsg, err );
          break;
        }
      }

      sqlite3_finalize( stmt );

      if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
      {
        ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
      }
    } // prepared statement
  } // BEGIN statement
  getLayerExtent( true, true );
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
      ( void )sqlite3_exec( getSqliteHandle(), "ROLLBACK", nullptr, nullptr, nullptr );
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

  int ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  fieldName = mAttributeFields.at( field ).name();

  sql = QStringLiteral( "CREATE INDEX IF NOT EXISTS %1 ON \"%2\" (%3)" )
        .arg( createIndexName( getTableName(), fieldName ),
              getTableName(),
              quotedIdentifier( fieldName ) );
  ret = sqlite3_exec( getSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
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

  int ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  sql = QStringLiteral( "DELETE FROM %1 WHERE %2=?" ).arg( quotedIdentifier( getTableName() ), quotedIdentifier( getPrimaryKey() ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    pushError( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( getSqliteHandle() ) ) );
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
      // mNumberFeatures--; will be set with getLayerExtent(true, true );
    }
    else
    {
      // some unexpected error occurred
      const char *err = sqlite3_errmsg( getSqliteHandle() );
      errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
      strcpy( errMsg, err );
      handleError( sql, errMsg, true );
      return false;
    }
  }
  sqlite3_finalize( stmt );

  ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  getLayerExtent( true, true );
  return true;
}

bool QgsSpatiaLiteProvider::truncate()
{
  char *errMsg = nullptr;
  QString sql;

  int ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  sql = QStringLiteral( "DELETE FROM %1" ).arg( quotedIdentifier( getTableName() ) );
  ret = sqlite3_exec( getSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }

  ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
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

  int ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    sql = QStringLiteral( "ALTER TABLE \"%1\" ADD COLUMN \"%2\" %3" )
          .arg( getTableName(),
                iter->name(),
                iter->typeName() );
    ret = sqlite3_exec( getSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }

  ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  getNumberFeatures( true );
  // reload columns
  getCapabilities( true );

  return true;
}

bool QgsSpatiaLiteProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  char *errMsg = nullptr;
  QString sql;

  if ( attr_map.isEmpty() )
    return true;

  int ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
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

    QString sql = QStringLiteral( "UPDATE %1 SET " ).arg( quotedIdentifier( getTableName() ) );
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
    sql += QStringLiteral( " WHERE %1=%2" ).arg( quotedIdentifier( getPrimaryKey() ) ).arg( fid );

    ret = sqlite3_exec( getSqliteHandle(), sql.toUtf8().constData(), nullptr, nullptr, &errMsg );
    if ( ret != SQLITE_OK )
    {
      handleError( sql, errMsg, true );
      return false;
    }
  }

  ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
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

  int ret = sqlite3_exec( getSqliteHandle(), "BEGIN", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg );
    return false;
  }

  sql =
    QStringLiteral( "UPDATE %1 SET %2=GeomFromWKB(?, %3) WHERE %4=?" )
    .arg( quotedIdentifier( getTableName() ),
          quotedIdentifier( getGeometryColumn() ) )
    .arg( getSrid() )
    .arg( quotedIdentifier( getPrimaryKey() ) );

  // SQLite prepared statement
  if ( sqlite3_prepare_v2( getSqliteHandle(), sql.toUtf8().constData(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    // some error occurred
    QgsMessageLog::logMessage( tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( getSqliteHandle() ) ), tr( "SpatiaLite" ) );
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
    QByteArray iterWkb = iter->exportToWkb();
    QgsSpatiaLiteUtils::convertFromGeosWKB( reinterpret_cast<const unsigned char *>( iterWkb.constData() ), iterWkb.length(), &wkb, &wkb_size, getCoordDimensions() );
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
      const char *err = sqlite3_errmsg( getSqliteHandle() );
      errMsg = ( char * ) sqlite3_malloc( ( int ) strlen( err ) + 1 );
      strcpy( errMsg, err );
      handleError( sql, errMsg, true );
      return false;
    }
  }
  sqlite3_finalize( stmt );

  ret = sqlite3_exec( getSqliteHandle(), "COMMIT", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    handleError( sql, errMsg, true );
    return false;
  }
  return true;
}

QgsVectorDataProvider::Capabilities QgsSpatiaLiteProvider::capabilities() const
{
  return getCapabilities();
}

QVariant QgsSpatiaLiteProvider::defaultValue( int fieldId ) const
{
  return getDefaultValues().value( fieldId, QVariant() );
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
    mQuery = QStringLiteral( "%1 as %2" ).arg( mQuery, quotedIdentifier( alias ) );
    sql = QStringLiteral( "SELECT 0 FROM %1 LIMIT 1" ).arg( mQuery );
    ret = sqlite3_get_table( getSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    mQuery = quotedIdentifier( getTableName() );
  }
  // checking for validity
  return count == 1;
}
bool QgsSpatiaLiteProvider::getQueryGeometryDetails()
{
  int ret;
  int i;
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;

  QString fType( QLatin1String( "" ) );
  QString xSrid( QLatin1String( "" ) );

  // get stuff from the relevant column instead. This may (will?)
  // fail if there is no data in the relevant table.
  QString sql = QStringLiteral( "select srid(%1), geometrytype(%1) from %2" )
                .arg( quotedIdentifier( getGeometryColumn() ),
                      mQuery );

  //it is possible that the where clause restricts the feature type
  if ( !mSubsetString.isEmpty() )
  {
    sql += " WHERE " + mSubsetString;
  }

  sql += QLatin1String( " limit 1" );

  ret = sqlite3_get_table( getSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
            .arg( quotedIdentifier( getGeometryColumn() ),
                  mQuery );

      if ( !mSubsetString.isEmpty() )
        sql += " where " + mSubsetString;

      ret = sqlite3_get_table( getSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
  return true;
}

QgsField QgsSpatiaLiteProvider::field( int index ) const
{
  if ( index < 0 || index >= getAttributeFields().count() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "SpatiaLite" ) );
    throw SLFieldNotFound();
  }

  return getAttributeFields().at( index );
}

void QgsSpatiaLiteProvider::invalidateConnections( const QString &connection )
{
  QgsSpatiaLiteConnPool::instance()->invalidateConnections( connection );
}

/**
 * Class factory to return a pointer to a newly created
 * QgsSpatiaLiteProvider object
 */
QGISEXTERN QgsSpatiaLiteProvider *classFactory( const QString *uri )
{
  return new QgsSpatiaLiteProvider( *uri );
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

QGISEXTERN bool createDb( const QString &dbPath, QString &errCause )
{
  QgsDebugMsg( "creating a new db" );

  QFileInfo fullPath = QFileInfo( dbPath );
  QDir path = fullPath.dir();
  QgsDebugMsg( QString( "making this dir: %1" ).arg( path.absolutePath() ) );

  // Must be sure there is destination directory ~/.qgis
  QDir().mkpath( path.absolutePath() );

  // creating/opening the new database
  sqlite3 *sqlite_handle = nullptr;
  int ret = QgsSLConnect::sqlite3_open_v2( dbPath.toUtf8().constData(), &sqlite_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( ret )
  {
    // an error occurred
    errCause = QObject::tr( "Could not create a new database\n" );
    errCause += QString::fromUtf8( sqlite3_errmsg( sqlite_handle ) );
    QgsSLConnect::sqlite3_close( sqlite_handle );
    return false;
  }
  // activating Foreign Key constraints
  char *errMsg = nullptr;
  ret = sqlite3_exec( sqlite_handle, "PRAGMA foreign_keys = 1", nullptr, nullptr, &errMsg );
  if ( ret != SQLITE_OK )
  {
    errCause = QObject::tr( "Unable to activate FOREIGN_KEY constraints [%1]" ).arg( errMsg );
    sqlite3_free( errMsg );
    QgsSLConnect::sqlite3_close( sqlite_handle );
    return false;
  }
  bool init_res = ::initializeSpatialMetadata( sqlite_handle, errCause );

  // all done: closing the DB connection
  QgsSLConnect::sqlite3_close( sqlite_handle );

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
  QString sql = QString( "DROP TABLE " ) + QgsSpatiaLiteProvider::quotedIdentifier( tableName );
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
        .arg( QgsSpatiaLiteProvider::quotedValue( tableName ) );
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
  const QString sql = QStringLiteral( "PRAGMA foreign_key_list(%1)" ).arg( QgsSpatiaLiteProvider::quotedIdentifier( getTableName() ) );
  char **results = nullptr;
  int rows;
  int columns;
  char *errMsg = nullptr;
  int ret = sqlite3_get_table( getSqliteHandle(), sql.toUtf8().constData(), &results, &rows, &columns, &errMsg );
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
    return QLatin1String( "" );
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

