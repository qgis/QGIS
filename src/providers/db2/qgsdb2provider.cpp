#include "qgsdb2provider.h"
#include "qgsdb2dataitems.h"
#include "qgsdb2featureiterator.h"
#include <qgscoordinatereferencesystem.h>
#include <qgsdataitem.h>
#include <qgslogger.h>

static const QString PROVIDER_KEY = "DB2";
static const QString PROVIDER_DESCRIPTION = "DB2 Spatial Extender provider";

int QgsDb2Provider::sConnectionId = 0;

QgsDb2Provider::QgsDb2Provider( QString uri )
    : QgsVectorDataProvider( uri ),
    mWkbType( QGis::WKBUnknown )
{
  QgsDebugMsg( "uri: " + uri );
  QgsDataSourceURI anUri = QgsDataSourceURI( uri );
  if ( !anUri.srid().isEmpty() )
    mSRId = anUri.srid().toInt();
  else
    mSRId = -1;

  mWkbType = anUri.wkbType();

  mValid = true;
  mUseWkb = false;
  mSkipFailures = false;

  mUserName = anUri.username();
  mPassword = anUri.password();
  mService = anUri.service();
  mDatabaseName = anUri.database();
  mHost = anUri.host();
  mPort = anUri.port();
  mDriver = anUri.param( "driver" );
  bool convertIntOk;
  int portNum = mPort.toInt( &convertIntOk, 10 );

  mUseEstimatedMetadata = anUri.useEstimatedMetadata();
  mSqlWhereClause = anUri.sql();
  mDatabase = GetDatabase( mService, mDriver, mHost, portNum, mDatabaseName, mUserName, mPassword );

  if ( !OpenDatabase( mDatabase ) )
  {
    setLastError( mDatabase.lastError().text() );
    QgsDebugMsg( mLastError );
    mValid = false;
    return;
  }

  // Create a query for default connection
  mQuery = QSqlQuery( mDatabase );

  // Database successfully opened; we can now issue SQL commands.
  if ( !anUri.schema().isEmpty() )
    mSchemaName = anUri.schema();
  else
    mSchemaName = mUserName;

  if ( !anUri.table().isEmpty() )
  {
    // the layer name has been specified
    mTableName = anUri.table();
    QStringList sl = mTableName.split( '.' );
    if ( sl.length() == 2 )
    {
      mSchemaName = sl[0];
      mTableName = sl[1];
    }
    mTables = QStringList( mTableName );
  }
  else
  {
    // Get a list of table
    mTables = mDatabase.tables( QSql::Tables );
    if ( mTables.count() > 0 )
      mTableName = mTables[0];
    else
      mValid = false;
  }
  if ( mValid )
  {
    if ( !anUri.keyColumn().isEmpty() )
      mFidColName = anUri.keyColumn();

    if ( !anUri.geometryColumn().isEmpty() )
      mGeometryColName = anUri.geometryColumn();

    if ( mSRId < 0 || mWkbType == QGis::WKBUnknown || mGeometryColName.isEmpty() )
    {
      loadMetadata();
    }
    loadFields();
    UpdateStatistics();

    if ( mGeometryColName.isEmpty() )
    {
      // table contains no geometries
      mWkbType = QGis::WKBNoGeometry;
      mSRId = 0;
    }
  }

  //fill type names into sets
  mNativeTypes
  // integer types
  << QgsVectorDataProvider::NativeType( tr( "8 Bytes integer" ), "bigint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "4 Bytes integer" ), "integer", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "2 Bytes integer" ), "smallint", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), "numeric", QVariant::Double, 1, 31, 0, 31 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), "decimal", QVariant::Double, 1, 31, 0, 31 )

  // floating point
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "real", QVariant::Double )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "double", QVariant::Double )
  //TODO decfloat?

  // string types
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "char", QVariant::String, 1, 254 )
  << QgsVectorDataProvider::NativeType( tr( "Text, variable length (varchar)" ), "varchar", QVariant::String, 1, 32704 )
  << QgsVectorDataProvider::NativeType( tr( "Text, variable length large object (clob)" ), "clob", QVariant::String, 1, 2147483647 )
  //DBCLOB is for 1073741824 double-byte characters, data length should be the same as CLOB (2147483647)?
  << QgsVectorDataProvider::NativeType( tr( "Text, variable length large object (dbclob)" ), "dbclob", QVariant::String, 1, 1073741824 )
  ;
}

QgsDb2Provider::~QgsDb2Provider()
{
  if ( mDatabase.isOpen() )
    mDatabase.close();
}

/**
* Returns a QSqlDatabase object that can connect to DB2 for LUW or z/OS.
*
* If service is provided, then username and password is required. If service is not provided, the remaining arguments are required.
*
* @param service The DSN name.
* @param driver The full driver name.
* @param host The host name.
* @param port The port number.
* @param location The database/location name.
* @param username The username.
* @param password The password.
*/
QSqlDatabase QgsDb2Provider::GetDatabase( QString service, QString driver, QString host, int port, QString location, QString username, QString password )
{
  QSqlDatabase db;
  QString connectionName;
  QString connectionString;

  if ( service.isEmpty() )
  {
    if ( driver.isEmpty() || host.isEmpty() || location.isEmpty() || username.isEmpty() || password.isEmpty() )
    {
      QgsDebugMsg( "DB2: service not provided, a required argument is empty." );
      return db;
    }
    connectionName = location + ".";
  }
  else
  {
    if ( username.isEmpty() || password.isEmpty() )
    {
      QgsDebugMsg( "DB2: service provided, a required argument is empty." );
      return db;
    }
    connectionName = service;
  }
  QgsDebugMsg( "connectionName: " + connectionName );
  /* if new database connection */
  QStringList connections = QSqlDatabase::connectionNames();
  for ( int i = 0; i < connections.size(); i++ )
  {
    QString c = connections.at( i );
    QgsDebugMsg( c );
  }
  if ( !QSqlDatabase::contains( connectionName ) )
  {
    QgsDebugMsg( "new DB2 database. create new QODBC mapping" );
    db = QSqlDatabase::addDatabase( "QODBC", connectionName );
  }
  else  /* if existing database connection */
  {
    QgsDebugMsg( "DB2 found existing connection, use the existing one" );
    db = QSqlDatabase::database( connectionName );
  }
  db.setHostName( host );
  db.setPort( port );
  db.setUserName( username );
  db.setPassword( password );

  /* start building connection string */
  if ( service.isEmpty() )
  {
    connectionString = "Driver={%1};Hostname=%2;Port=%3;Protocol=TCPIP;Database=%4;Uid=%5;Pwd=%6;";
    connectionString = connectionString.arg( driver ).arg( host ).arg( db.port() ).arg( location ).arg( username ).arg( password );
  }
  else
  {
    connectionString = service;
  }
  QgsDebugMsg( "DB2: GetDatabase; connection string: " + connectionString );

  db.setDatabaseName( connectionString ); //for QODBC driver, the name can be a DSN or connection string
  return db;
}

bool QgsDb2Provider::OpenDatabase( QSqlDatabase db )
{
  QgsDebugMsg( "OpenDatabase" );
  if ( !db.isOpen() )
  {
    if ( !db.open() )
    {
      QgsDebugMsg( "Database could not be opened." );
      return false;
    }
  }
  return true;
}

void QgsDb2Provider::loadMetadata()
{
  mSRId = 0;
  mWkbType = QGis::WKBUnknown;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( QString( "SELECT COLUMN_NAME, 2, SRS_ID, TYPE_NAME FROM DB2GSE.ST_GEOMETRY_COLUMNS WHERE TABLE_SCHEMA = '%1' AND TABLE_NAME = '%2'" ).arg( mSchemaName, mTableName ) ) )
  {
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }
  // David Adler - not sure if type_name is exactly what is expected for getWkbType
  // assume dimension is always 2
  if ( query.isActive() && query.next() )
  {
    mGeometryColName = query.value( 0 ).toString();
    mGeometryColType = query.value( 3 ).toString();
    mSRId = query.value( 2 ).toInt();
    mWkbType = getWkbType( query.value( 3 ).toString(), query.value( 1 ).toInt() );
    QgsDebugMsg( "table: " + mTableName + " : " + mGeometryColName + " : " + query.value( 2 ).toString() );
  }
}

void QgsDb2Provider::loadFields()
{
  mAttributeFields.clear();
  //mDefaultValues.clear();
  QString table = QString( "%1.%2" ).arg( mSchemaName ).arg( mTableName );

  // Use the Qt functionality to get the primary key information
  // to set the FID column.
  // We can only use the primary key if it only has one column and
  // the type is Integer or BigInt.
  QSqlIndex pk = mDatabase.primaryIndex( table );
  if ( pk.count() == 1 )
  {
    QSqlField pkFld = pk.field( 0 );
    QVariant::Type pkType = pkFld.type();
    if (( pkType == QVariant::Int ||  pkType == QVariant::LongLong ) )
    {
      mFidColName = pk.fieldName( 0 );
      QgsDebugMsg( "set FID to " + mFidColName );
    }
  }
  else
  {
    QgsDebugMsg( "Warning: table primary key count is " + QString::number( pk.count() ) );
    mFidColName = QString( "" );
  }

  // Use the Qt functionality to get the fields and their definitions.
  QSqlRecord r = mDatabase.record( table );
  int fieldCount = r.count();

  for ( int i = 0; i < fieldCount; i++ )
  {
    QSqlField f = r.field( i );
    int typeID = f.typeID(); // seems to be DB2 numeric type id (standard?)
    QString sqlTypeName = db2TypeName( typeID );
    QVariant::Type sqlType = f.type();
    QgsDebugMsg( "name: " + f.name() + "; sqlTypeID: " + QString::number( typeID ) + "; sqlTypeName: " + sqlTypeName );
    if ( f.name() == mGeometryColName ) continue; // Got this with loadMeta(), just skip
    if ( sqlType == QVariant::String )
    {
      mAttributeFields.append(
        QgsField(
          f.name(),
          sqlType,
          sqlTypeName,
          f.length()
        ) );
    }
    else if ( sqlType == QVariant::Double )
    {
      mAttributeFields.append(
        QgsField(
          f.name(),
          sqlType,
          sqlTypeName,
          f.length(),
          f.precision()
        ) );
    }
    else
    {
      mAttributeFields.append(
        QgsField(
          f.name(),
          sqlType,
          sqlTypeName
        ) );
    }

    if ( !f.defaultValue().isNull() )
    {
      mDefaultValues.insert( i, f.defaultValue() );
    }
// Hack to get primary key since the primaryIndex function above doesn't work
// on z/OS. Pick first integer column.
    if ( mFidColName.length() == 0 &&
         ( sqlType == QVariant::LongLong || sqlType == QVariant::Int ) )
    {
      mFidColName = f.name();
    }
  }
}

// loadFields() gets the type from the field record
QVariant::Type QgsDb2Provider::DecodeSqlType( int typeId )
{
  QVariant::Type type = QVariant::Invalid;
  switch ( typeId )
  {
    case -5:     //BIGINT
      type = QVariant::LongLong;
      break;

    case -3:     //VARBINARY
      type = QVariant::ByteArray;
      break;

    case 1:     //CHAR
    case 12:    //VARCHAR
      type = QVariant::String;
      break;

    case 4:     //INTEGER
      type = QVariant::Int;
      break;

    case 3:     //NUMERIC and DECIMAL
    case 7:     //REAL
    case 8:     //DOUBLE
      type = QVariant::Double;
      break;

    case 9:    //DATE
      type = QVariant::String; // don't know why it doesn't like QVariant::Date
      break;

    case 10:    //TIME
      type = QVariant::Time;
      break;

    case 11:    //TIMESTAMP
      type = QVariant::String; // don't know why it doesn't like QVariant::DateTime
      break;

    default:
      // Everything else just dumped as a string.
      type = QVariant::String;
  }

  return type;
}

// Return the DB2 type name for the type numeric value
QString QgsDb2Provider::db2TypeName( int typeId )
{
  QString typeName = "";
  switch ( typeId )
  {
    case -3:     //VARBINARY
      typeName = "VARBINARY"; // also for spatial types
      break;

    case 1:     //CHAR
      typeName = "CHAR";
      break;

    case 12:    //VARCHAR
      typeName = "VARCHAR";
      break;

    case 4:     //INTEGER
      typeName = "INTEGER";
      break;

    case -5:     //BIGINT
      typeName = "BIGINT";
      break;

    case 3:     //NUMERIC and DECIMAL
      typeName =  "DECIMAL";
      break;

    case 7:     //REAL
      typeName =  "REAL";
      break;

    case 8:     //DOUBLE
      typeName =  "DOUBLE";
      break;

    case 9:    //DATE
      typeName =  "DATE";
      break;

    case 10:    //TIME
      typeName =  "TIME";
      break;

    case 11:    //TIMESTAMP
      typeName =  "TIMESTAMP";
      break;

    default:
      typeName = "UNKNOWN";
  }

  return typeName;
}

QgsAbstractFeatureSource* QgsDb2Provider::featureSource() const
{
  return new QgsDb2FeatureSource( this );
}

QgsFeatureIterator QgsDb2Provider::getFeatures( const QgsFeatureRequest& request )
{
  if ( !mValid )
  {
    QgsDebugMsg( "Read attempt on an invalid db2 data source" );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsDb2FeatureIterator( new QgsDb2FeatureSource( this ), true, request ) );
}

QGis::WkbType QgsDb2Provider::geometryType() const
{
  return mWkbType;
}

long QgsDb2Provider::featureCount() const
{
  // Return the count that we get from the subset.
  if ( !mSqlWhereClause.isEmpty() )
    return mNumberFeatures;

  // On LUW, this could be selected from syscat.tables but I'm not sure if this
  // is actually correct if RUNSTATS hasn't been done.
  // On z/OS, we don't have access to the system catalog.
  // Use count(*) as the easiest approach
  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  QString sql = "SELECT COUNT(*) FROM %1.%2";
  QString statement = QString( sql ).arg( mSchemaName, mTableName );

  if ( query.exec( statement ) && query.next() )
  {
    return query.value( 0 ).toInt();
  }
  else
  {
    return -1;
  }
}

const QgsFields &QgsDb2Provider::fields() const
{
  return mAttributeFields;
}

QgsCoordinateReferenceSystem QgsDb2Provider::crs()
{
  if ( !mCrs.isValid() && mSRId > 0 )
  {
    mCrs.createFromSrid( mSRId );
    if ( mCrs.isValid() )
      return mCrs;

    // try to load crs from the database tables as a fallback
    QSqlQuery query = QSqlQuery( mDatabase );
    query.setForwardOnly( true );
    bool execOk = query.exec( QString( "SELECT DEFINITION FROM DB2GSE.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = %1" ).arg( QString::number( mSRId ) ) );
    if ( execOk && query.isActive() )
    {
      if ( query.next() && mCrs.createFromWkt( query.value( 0 ).toString() ) )
        return mCrs;

      query.finish();
    }
  }
  return mCrs;
}

// update the extent for this layer
void QgsDb2Provider::UpdateStatistics()
{
  // get features to calculate the statistics
  QString statement;

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );

  statement = QString( "SELECT MIN(DB2GSE.ST_MINX(%1)), MIN(DB2GSE.ST_MINY(%1)), MAX(DB2GSE.ST_MAXX(%1)), MAX(DB2GSE.ST_MAXY(%1))" ).arg( mGeometryColName );

  statement += QString( " FROM %1.%2" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    statement += " WHERE (" + mSqlWhereClause + ")";
  }
  QgsDebugMsg( statement );

  if ( !query.exec( statement ) )
  {
    QgsDebugMsg( statement );
    QString msg = query.lastError().text();
    QgsDebugMsg( msg );
  }

  if ( !query.isActive() )
  {
    return;
  }

  if ( query.next() )
  {
    mExtent.setXMinimum( query.value( 0 ).toDouble() );
    mExtent.setYMinimum( query.value( 1 ).toDouble() );
    mExtent.setXMaximum( query.value( 2 ).toDouble() );
    mExtent.setYMaximum( query.value( 3 ).toDouble() );
    QgsDebugMsg( QString( "after setting; mExtent: %1" ).arg( mExtent.toString() ) );
    return;
  }
}

QgsRectangle QgsDb2Provider::extent()
{
  QgsDebugMsg( QString( "entering; mExtent: %1" ).arg( mExtent.toString() ) );
  if ( mExtent.isEmpty() )
    UpdateStatistics();
  return mExtent;
}

bool QgsDb2Provider::isValid()
{
  return true; //DB2 only has valid geometries
}

QString QgsDb2Provider::subsetString()
{
  return mSqlWhereClause;
}

bool QgsDb2Provider::setSubsetString( const QString& theSQL, bool )
{
  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QString( "SELECT COUNT(*) FROM " );

  sql += QString( "%1.%2" ).arg( mSchemaName, mTableName );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
  }

  if ( !OpenDatabase( mDatabase ) )
  {
    return false;
  }

  QSqlQuery query = QSqlQuery( mDatabase );
  query.setForwardOnly( true );
  if ( !query.exec( sql ) )
  {
    QgsDebugMsg( sql );
    pushError( query.lastError().text() );
    mSqlWhereClause = prevWhere;
    return false;
  }

  if ( query.isActive() && query.next() )
    mNumberFeatures = query.value( 0 ).toInt();

  QgsDataSourceURI anUri = QgsDataSourceURI( dataSourceUri() );
  anUri.setSql( mSqlWhereClause );

  setDataSourceUri( anUri.uri() );

  mExtent.setMinimal();

  emit dataChanged();

  return true;
}

void QgsDb2Provider::db2WkbTypeAndDimension( QGis::WkbType wkbType, QString &geometryType, int &dim )
{
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      dim = 3;
    case QGis::WKBPoint:
      geometryType = "ST_POINT";
      break;

    case QGis::WKBLineString25D:
      dim = 3;
    case QGis::WKBLineString:
      geometryType = "ST_LINESTRING";
      break;

    case QGis::WKBPolygon25D:
      dim = 3;
    case QGis::WKBPolygon:
      geometryType = "ST_POLYGON";
      break;

    case QGis::WKBMultiPoint25D:
      dim = 3;
    case QGis::WKBMultiPoint:
      geometryType = "ST_MULTIPOINT";
      break;

    case QGis::WKBMultiLineString25D:
      dim = 3;
    case QGis::WKBMultiLineString:
      geometryType = "ST_MULTILINESTRING";
      break;

    case QGis::WKBMultiPolygon25D:
      dim = 3;
    case QGis::WKBMultiPolygon:
      geometryType = "ST_MULTIPOLYGON";
      break;

    case QGis::WKBUnknown:
      geometryType = "ST_GEOMETRY";
      break;

    case QGis::WKBNoGeometry:
    default:
      dim = 0;
      break;
  }
}

QGis::WkbType QgsDb2Provider::getWkbType( const QString& geometryType, int dim )
{
  if ( dim == 3 )
  {
    if ( geometryType == "ST_POINT" )
      return QGis::WKBPoint25D;
    if ( geometryType == "ST_LINESTRING" )
      return QGis::WKBLineString25D;
    if ( geometryType == "ST_POLYGON" )
      return QGis::WKBPolygon25D;
    if ( geometryType == "ST_MULTIPOINT" )
      return QGis::WKBMultiPoint25D;
    if ( geometryType == "ST_MULTILINESTRING" )
      return QGis::WKBMultiLineString25D;
    if ( geometryType == "ST_MULTIPOLYGON" )
      return QGis::WKBMultiPolygon25D;
    else
      return QGis::WKBUnknown;
  }
  else
  {
    if ( geometryType == "ST_POINT" )
      return QGis::WKBPoint;
    if ( geometryType == "ST_LINESTRING" )
      return QGis::WKBLineString;
    if ( geometryType == "ST_POLYGON" )
      return QGis::WKBPolygon;
    if ( geometryType == "ST_MULTIPOINT" )
      return QGis::WKBMultiPoint;
    if ( geometryType == "ST_MULTILINESTRING" )
      return QGis::WKBMultiLineString;
    if ( geometryType == "ST_MULTIPOLYGON" )
      return QGis::WKBMultiPolygon;
    else
      return QGis::WKBUnknown;
  }
}

QString QgsDb2Provider::name() const
{
  return PROVIDER_KEY;
}

QString QgsDb2Provider::description() const
{
  return PROVIDER_DESCRIPTION;
}

QGISEXTERN QgsDb2Provider *classFactory( const QString *uri )
{
  return new QgsDb2Provider( *uri );
}

QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}

QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN void *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsDb2SourceSelect( parent, fl );
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  QgsDebugMsg( "DB2: Browser Panel; data item detected." );
  return new QgsDb2RootItem( parentItem, PROVIDER_KEY, "db2:" );
}