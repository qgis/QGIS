/***************************************************************************
  qgspostgresprovider.cpp  -  QGIS data provider for PostgreSQL/PostGIS layers
                             -------------------
    begin                : 2004/01/07
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* $Id$ */

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <qgis.h>
#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgsmessageoutput.h>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>

#include "qgsprovidercountcalcevent.h"
#include "qgsproviderextentcalcevent.h"

#include "qgspostgresprovider.h"

#include "qgspostgrescountthread.h"
#include "qgspostgresextentthread.h"

#include "qgspostgisbox3d.h"
#include "qgslogger.h"

#include "qgscredentials.h"

const QString POSTGRES_KEY = "postgres";
const QString POSTGRES_DESCRIPTION = "PostgreSQL/PostGIS data provider";

QMap<QString, QgsPostgresProvider::Conn *> QgsPostgresProvider::Conn::connectionsRO;
QMap<QString, QgsPostgresProvider::Conn *> QgsPostgresProvider::Conn::connectionsRW;
QMap<QString, QString> QgsPostgresProvider::Conn::passwordCache;
int QgsPostgresProvider::providerIds = 0;

QgsPostgresProvider::QgsPostgresProvider( QString const & uri )
    : QgsVectorDataProvider( uri ),
    mFetching( false ),
    geomType( QGis::WKBUnknown ),
    mFeatureQueueSize( 200 ),
    mPrimaryKeyDefault( QString::null )
{
  // assume this is a valid layer until we determine otherwise
  valid = true;

  providerId = providerIds++;

  QgsDebugMsg( "Postgresql Layer Creation" );
  QgsDebugMsg( "URI: " + uri );

  mUri = QgsDataSourceURI( uri );

  /* populate members from the uri structure */
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  geometryColumn = mUri.geometryColumn();
  sqlWhereClause = mUri.sql();
  primaryKey = mUri.keyColumn();

  // Keep a schema qualified table name for convenience later on.
  mSchemaTableName = mUri.quotedTablename();

  QgsDebugMsg( "Table name is " + mTableName );
  QgsDebugMsg( "SQL is " + sqlWhereClause );
  QgsDebugMsg( "Connection info is " + mUri.connectionInfo() );

  QgsDebugMsg( "Geometry column is: " + geometryColumn );
  QgsDebugMsg( "Schema is: " + mSchemaName );
  QgsDebugMsg( "Table name is: " + mTableName );

  connectionRW = NULL;
  connectionRO = Conn::connectDb( mUri.connectionInfo(), true );
  if ( connectionRO == NULL )
  {
    valid = false;
    return;
  }

  QgsDebugMsg( "Checking for permissions on the relation" );

  // Check that we can read from the table (i.e., we have
  // select permission).
  QString sql = QString( "select * from %1 limit 1" ).arg( mSchemaTableName );
  Result testAccess = connectionRO->PQexec( sql );
  if ( PQresultStatus( testAccess ) != PGRES_TUPLES_OK )
  {
    showMessageBox( tr( "Unable to access relation" ),
                    tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                    .arg( mSchemaTableName )
                    .arg( QString::fromUtf8( PQresultErrorMessage( testAccess ) ) )
                    .arg( sql ) );
    valid = false;
    disconnectDb();
    return;
  }

  sql = QString( "SELECT "
                 "has_table_privilege(%1,'DELETE'),"
                 "has_table_privilege(%1,'UPDATE'),"
                 "has_table_privilege(%1,'INSERT'),"
                 "current_schema()" )
        .arg( quotedValue( mSchemaTableName ) );

  testAccess = connectionRO->PQexec( sql );
  if ( PQresultStatus( testAccess ) != PGRES_TUPLES_OK )
  {
    showMessageBox( tr( "Unable to access relation" ),
                    tr( "Unable to determine table access privileges for the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                    .arg( mSchemaTableName )
                    .arg( QString::fromUtf8( PQresultErrorMessage( testAccess ) ) )
                    .arg( sql ) );
    valid = false;
    disconnectDb();
    return;
  }

  // postgres has fast access to features at id (thanks to primary key / unique index)
  // the latter flag is here just for compatibility
  enabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;

  if ( QString::fromUtf8( PQgetvalue( testAccess, 0, 0 ) ) == "t" )
  {
    // DELETE
    enabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
  }

  if ( QString::fromUtf8( PQgetvalue( testAccess, 0, 1 ) ) == "t" )
  {
    // UPDATE
    enabledCapabilities |= QgsVectorDataProvider::ChangeGeometries | QgsVectorDataProvider::ChangeAttributeValues;
  }

  if ( QString::fromUtf8( PQgetvalue( testAccess, 0, 2 ) ) == "t" )
  {
    // INSERT
    enabledCapabilities |= QgsVectorDataProvider::AddFeatures;
  }

  mCurrentSchema = QString::fromUtf8( PQgetvalue( testAccess, 0, 3 ) );
  if ( mCurrentSchema == mSchemaName )
  {
    mUri.clearSchema();
  }

  if ( mSchemaName == "" )
    mSchemaName = mCurrentSchema;

  sql = QString( "SELECT 1 FROM pg_class,pg_namespace WHERE "
                 "pg_class.relnamespace=pg_namespace.oid AND "
                 "pg_get_userbyid(relowner)=current_user AND "
                 "relname=%1 AND nspname=%2" )
        .arg( quotedValue( mTableName ) )
        .arg( quotedValue( mSchemaName ) );
  testAccess = connectionRO->PQexec( sql );
  if ( PQresultStatus( testAccess ) == PGRES_TUPLES_OK && PQntuples( testAccess ) == 1 )
  {
    enabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes;
  }

  if ( !getGeometryDetails() ) // gets srid and geometry type
  {
    // the table is not a geometry table
    featuresCounted = 0;
    valid = false;

    QgsDebugMsg( "Invalid Postgres layer" );
    disconnectDb();
    return;
  }

  deduceEndian();
  calculateExtents();
  getFeatureCount();

  // set the primary key
  getPrimaryKey();

  // load the field list
  loadFields();

  // Set the postgresql message level so that we don't get the
  // 'there is no transaction in progress' warning.
#ifndef QGISDEBUG
  connectionRO->PQexecNR( "set client_min_messages to error" );
#endif

  // Kick off the long running threads

#ifdef POSTGRESQL_THREADS
  QgsDebugMsg( "About to touch mExtentThread" );
  mExtentThread.setConnInfo( mUri.connectionInfo );
  mExtentThread.setTableName( mTableName );
  mExtentThread.setSqlWhereClause( sqlWhereClause );
  mExtentThread.setGeometryColumn( geometryColumn );
  mExtentThread.setCallback( this );
  QgsDebugMsg( "About to start mExtentThread" );
  mExtentThread.start();
  QgsDebugMsg( "Main thread just dispatched mExtentThread" );

  QgsDebugMsg( "About to touch mCountThread" );
  mCountThread.setConnInfo( mUri.connectionInfo );
  mCountThread.setTableName( mTableName );
  mCountThread.setSqlWhereClause( sqlWhereClause );
  mCountThread.setGeometryColumn( geometryColumn );
  mCountThread.setCallback( this );
  QgsDebugMsg( "About to start mCountThread" );
  mCountThread.start();
  QgsDebugMsg( "Main thread just dispatched mCountThread" );
#endif

  //fill type names into sets
  mNativeTypes
  // integer types
  << QgsVectorDataProvider::NativeType( tr( "Whole number (smallint - 16bit)" ), "int2", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 32bit)" ), "int4", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 64bit)" ), "int8", QVariant::LongLong )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), "numeric", QVariant::LongLong, 1, 20, 0, 20 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), "decimal", QVariant::LongLong, 1, 20, 0, 20 )

  // floating point
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "real", QVariant::Double )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "double precision", QVariant::Double )

  // string types
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "char", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), "varchar", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), "text", QVariant::String )
  ;

  if ( primaryKey.isEmpty() )
  {
    valid = false;
  }
  else
  {
    mUri.setKeyColumn( primaryKey );
    setDataSourceUri( mUri.uri() );
  }

  // Close the database connection if the layer isn't going to be loaded.
  if ( !valid )
    disconnectDb();
}

QgsPostgresProvider::~QgsPostgresProvider()
{
#ifdef POSTGRESQL_THREADS
  QgsDebugMsg( "About to wait for mExtentThread" );

  mExtentThread.wait();

  QgsDebugMsg( "Finished waiting for mExtentThread" );

  QgsDebugMsg( "About to wait for mCountThread" );

  mCountThread.wait();

  QgsDebugMsg( "Finished waiting for mCountThread" );

  // Make sure all events from threads have been processed
  // (otherwise they will get destroyed prematurely)
  QApplication::sendPostedEvents( this, QGis::ProviderExtentCalcEvent );
  QApplication::sendPostedEvents( this, QGis::ProviderCountCalcEvent );
#endif

  disconnectDb();

  QgsDebugMsg( "deconstructing." );

  //pLog.flush();
}

QgsPostgresProvider::Conn *QgsPostgresProvider::Conn::connectDb( const QString &conninfo, bool readonly )
{
  QMap<QString, QgsPostgresProvider::Conn *> &connections =
    readonly ? QgsPostgresProvider::Conn::connectionsRO : QgsPostgresProvider::Conn::connectionsRW;

  if ( connections.contains( conninfo ) )
  {
    QgsDebugMsg( QString( "Using cached connection for %1" ).arg( conninfo ) );
    connections[conninfo]->ref++;
    return connections[conninfo];
  }

  QgsDebugMsg( QString( "New postgres connection for " ) + conninfo );

  PGconn *pd = PQconnectdb( conninfo.toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus( pd ) != CONNECTION_OK && QString::fromUtf8( PQerrorMessage( pd ) ) == PQnoPasswordSupplied )
  {
    QgsDataSourceURI uri( conninfo );
    QString username = uri.username();
    QString password = uri.password();

    while ( PQstatus( pd ) != CONNECTION_OK )
    {
      bool ok = QgsCredentials::instance()->get( conninfo, username, password, QString::fromUtf8( PQerrorMessage( pd ) ) );
      if ( !ok )
        break;

      ::PQfinish( pd );

      if ( !username.isEmpty() )
        uri.setUsername( username );

      if ( !password.isEmpty() )
        uri.setPassword( password );

      QgsDebugMsg( "Connecting to " + uri.connectionInfo() );
      pd = PQconnectdb( uri.connectionInfo().toLocal8Bit() );
    }

    if ( PQstatus( pd ) == CONNECTION_OK )
      QgsCredentials::instance()->put( conninfo, username, password );
  }

  if ( PQstatus( pd ) != CONNECTION_OK )
  {
    ::PQfinish( pd );
    QgsDebugMsg( "Connection to database failed" );
    return NULL;
  }

  //set client encoding to unicode because QString uses UTF-8 anyway
  QgsDebugMsg( "setting client encoding to UNICODE" );

  int errcode = PQsetClientEncoding( pd, QString( "UNICODE" ).toLocal8Bit() );

  if ( errcode == 0 )
  {
    QgsDebugMsg( "encoding successfully set" );
  }
  else if ( errcode == -1 )
  {
    QgsDebugMsg( "error in setting encoding" );
  }
  else
  {
    QgsDebugMsg( "undefined return value from encoding setting" );
  }

  QgsDebugMsg( "Connection to the database was successful" );

  Conn *conn = new Conn( pd );

  /* Check to see if we have working PostGIS support */
  if ( conn->postgisVersion().isNull() )
  {
    showMessageBox( tr( "No PostGIS Support!" ),
                    tr( "Your database has no working PostGIS support.\n" ) );
    conn->PQfinish();
    delete conn;
    return NULL;
  }

  connections.insert( conninfo, conn );

  /* Check to see if we have GEOS support and if not, warn the user about
     the problems they will see :) */
  QgsDebugMsg( "Checking for GEOS support" );

  if ( !conn->hasGEOS() )
  {
    showMessageBox( tr( "No GEOS Support!" ),
                    tr( "Your PostGIS installation has no GEOS support.\n"
                        "Feature selection and identification will not "
                        "work properly.\nPlease install PostGIS with "
                        "GEOS support (http://geos.refractions.net)" ) );
  }



  return conn;
}

void QgsPostgresProvider::disconnectDb()
{
  if ( mFetching )
  {
    connectionRO->closeCursor( QString( "qgisf%1" ).arg( providerId ) );
    mFetching = false;
  }

  if ( connectionRO )
  {
    Conn::disconnectRO( connectionRO );
  }

  if ( connectionRW )
  {
    Conn::disconnectRW( connectionRW );
  }
}

void QgsPostgresProvider::Conn::disconnectRW( Conn *&connection )
{
  disconnect( connectionsRW, connection );
}

void QgsPostgresProvider::Conn::disconnectRO( Conn *&connection )
{
  disconnect( connectionsRO, connection );
}

void QgsPostgresProvider::Conn::disconnect( QMap<QString, Conn *>& connections, Conn *&conn )
{
  QMap<QString, Conn *>::iterator i;
  for ( i = connections.begin(); i != connections.end() && i.value() != conn; i++ )
    ;

  assert( i.value() == conn );
  assert( i.value()->ref > 0 );

  if ( --i.value()->ref == 0 )
  {
    i.value()->PQfinish();
    delete i.value();
    connections.remove( i.key() );
  }

  conn = NULL;
}

QString QgsPostgresProvider::storageType() const
{
  return "PostgreSQL database with PostGIS extension";
}

QString QgsPostgresProvider::fieldExpression( const QgsField &fld ) const
{
  const QString &type = fld.typeName();
  if ( type == "money" )
  {
    return QString( "cash_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type.startsWith( "_" ) )
  {
    return QString( "array_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "bool" )
  {
    return QString( "boolout(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geometry" )
  {
    return QString( "asewkt(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else
  {
    return quotedIdentifier( fld.name() ) + "::text";
  }
}

bool QgsPostgresProvider::declareCursor(
  const QString &cursorName,
  const QgsAttributeList &fetchAttributes,
  bool fetchGeometry,
  QString whereClause )
{
  try
  {
    QString query = QString( "select %1" ).arg( quotedIdentifier( primaryKey ) );

    if ( fetchGeometry )
    {
      query += QString( ",asbinary(%1,'%2')" )
               .arg( quotedIdentifier( geometryColumn ) )
               .arg( endianString() );
    }

    for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it )
    {
      const QgsField &fld = field( *it );

      if ( fld.name() == primaryKey )
        continue;

      query += "," + fieldExpression( fld );
    }

    query += " from " + mSchemaTableName;

    if ( !whereClause.isEmpty() )
      query += QString( " where %1" ).arg( whereClause );

    return connectionRO->openCursor( cursorName, query );
  }
  catch ( PGFieldNotFound )
  {
    return false;
  }
}

bool QgsPostgresProvider::getFeature( PGresult *queryResult, int row, bool fetchGeometry,
                                      QgsFeature &feature,
                                      const QgsAttributeList &fetchAttributes )
{
  try
  {
    int oid;

    if ( primaryKeyType != "tid" )
    {
      oid = *( int * )PQgetvalue( queryResult, row, 0 );
      if ( swapEndian )
        oid = ntohl( oid ); // convert oid to opposite endian
    }
    else if ( PQgetlength( queryResult, row, 0 ) == 6 )
    {
      char *data = PQgetvalue( queryResult, row, 0 );
      int block = *( int * )data;
      int offset = *( short * )( data + sizeof( int ) );

      if ( swapEndian )
      {
        block = ntohl( block );
        offset = ntohs( offset );
      }

      if ( block > 0xffff )
      {
        QgsDebugMsg( QString( "block number %1 exceeds 16 bit" ).arg( block ) );
        return false;
      }

      oid = ( block << 16 ) + offset;
    }
    else
    {
      QgsDebugMsg( QString( "expecting 6 bytes for tid (found %1 bytes)" ).arg( PQgetlength( queryResult, row, 0 ) ) );
      return false;
    }

    feature.setFeatureId( oid );

    int col;  // first attribute column after geometry

    if ( fetchGeometry )
    {
      int returnedLength = PQgetlength( queryResult, row, 1 );
      if ( returnedLength > 0 )
      {
        unsigned char *featureGeom = new unsigned char[returnedLength + 1];
        memset( featureGeom, '\0', returnedLength + 1 );
        memcpy( featureGeom, PQgetvalue( queryResult, row, 1 ), returnedLength );
        feature.setGeometryAndOwnership( featureGeom, returnedLength + 1 );
      }
      else
      {
        feature.setGeometryAndOwnership( 0, 0 );
        QgsDebugMsg( "Couldn't get the feature geometry in binary form" );
      }

      col = 2;
    }
    else
    {
      col = 1;
    }

    // iterate attributes
    for ( QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); it++ )
    {
      const QgsField &fld = field( *it );

      if ( fld.name() == primaryKey )
      {
        // primary key was already processed
        feature.addAttribute( *it, convertValue( fld.type(), QString::number( oid ) ) );
        continue;
      }

      if ( !PQgetisnull( queryResult, row, col ) )
      {
        feature.addAttribute( *it, convertValue( fld.type(), QString::fromUtf8( PQgetvalue( queryResult, row, col ) ) ) );
      }
      else
      {
        feature.addAttribute( *it, QVariant( QString::null ) );
      }

      col++;
    }

    return true;
  }
  catch ( PGFieldNotFound )
  {
    return false;
  }
}

void QgsPostgresProvider::select( QgsAttributeList fetchAttributes, QgsRectangle rect, bool fetchGeometry, bool useIntersect )
{
  QString cursorName = QString( "qgisf%1" ).arg( providerId );

  if ( mFetching )
  {
    connectionRO->closeCursor( cursorName );
    mFetching = false;

    while ( !mFeatureQueue.empty() )
    {
      mFeatureQueue.pop();
    }
  }

  QString whereClause;

  if ( !rect.isEmpty() )
  {
    if ( useIntersect )
    {
      // Contributed by #qgis irc "creeping"
      // This version actually invokes PostGIS's use of spatial indexes
      whereClause = QString( "%1 && setsrid('BOX3D(%2)'::box3d,%3) and intersects(%1,setsrid('BOX3D(%2)'::box3d,%3))" )
                    .arg( quotedIdentifier( geometryColumn ) )
                    .arg( rect.asWktCoordinates() )
                    .arg( srid );
    }
    else
    {
      whereClause = QString( "%1 && setsrid('BOX3D(%2)'::box3d,%3)" )
                    .arg( quotedIdentifier( geometryColumn ) )
                    .arg( rect.asWktCoordinates() )
                    .arg( srid );
    }
  }

  if ( !sqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " and ";

    whereClause += "(" + sqlWhereClause + ")";
  }

  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  if ( !declareCursor( cursorName, fetchAttributes, fetchGeometry, whereClause ) )
    return;

  mFetching = true;
}

bool QgsPostgresProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );
  QString cursorName = QString( "qgisf%1" ).arg( providerId );

  if ( !valid )
  {
    QgsDebugMsg( "Read attempt on an invalid postgresql data source" );
    return false;
  }

  if ( mFeatureQueue.empty() )
  {
    QString fetch = QString( "fetch forward %1 from %2" ).arg( mFeatureQueueSize ).arg( cursorName );
    if ( connectionRO->PQsendQuery( fetch ) == 0 ) // fetch features asynchronously
    {
      QgsDebugMsg( "PQsendQuery failed (1)" );
    }

    Result queryResult;
    while (( queryResult = connectionRO->PQgetResult() ) )
    {
      int rows = PQntuples( queryResult );
      if ( rows == 0 )
        continue;

      for ( int row = 0; row < rows; row++ )
      {
        mFeatureQueue.push( QgsFeature() );
        getFeature( queryResult, row, mFetchGeom, mFeatureQueue.back(), mAttributesToFetch );
      } // for each row in queue
    }
  }

  if ( mFeatureQueue.empty() )
  {
    QgsDebugMsg( "End of features" );
    connectionRO->closeCursor( cursorName );
    mFetching = false;
    return false;
  }

  // Now return the next feature from the queue
  if ( mFetchGeom )
  {
    QgsGeometry* featureGeom = mFeatureQueue.front().geometryAndOwnership();
    feature.setGeometry( featureGeom );
  }
  else
  {
    feature.setGeometryAndOwnership( 0, 0 );
  }
  feature.setFeatureId( mFeatureQueue.front().id() );
  feature.setAttributeMap( mFeatureQueue.front().attributeMap() );

  mFeatureQueue.pop();

  feature.setValid( true );
  return true;
}

QString QgsPostgresProvider::whereClause( int featureId ) const
{
  QString whereClause;

  if ( primaryKeyType != "tid" )
  {
    whereClause = QString( "%1=%2" ).arg( quotedIdentifier( primaryKey ) ).arg( featureId );
  }
  else
  {
    whereClause = QString( "%1='(%2,%3)'" ).arg( quotedIdentifier( primaryKey ) ).arg( featureId >> 16 ).arg( featureId & 0xffff );
  }

  if ( !sqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " and ";

    whereClause += "(" + sqlWhereClause + ")";
  }

  return whereClause;
}

bool QgsPostgresProvider::featureAtId( int featureId, QgsFeature& feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
  QString cursorName = QString( "qgisfid%1" ).arg( providerId );

  if ( !declareCursor( cursorName, fetchAttributes, fetchGeometry, whereClause( featureId ) ) )
    return false;

  Result queryResult = connectionRO->PQexec( QString( "fetch forward 1 from %1" ).arg( cursorName ) );
  if ( queryResult == 0 )
    return false;

  int rows = PQntuples( queryResult );
  if ( rows == 0 )
  {
    QgsDebugMsg( QString( "feature %1 not found" ).arg( featureId ) );
    connectionRO->closeCursor( cursorName );
    return false;
  }
  else if ( rows != 1 )
  {
    QgsDebugMsg( QString( "found %1 features instead of just one." ).arg( rows ) );
  }

  bool gotit = getFeature( queryResult, 0, fetchGeometry, feature, fetchAttributes );

  connectionRO->closeCursor( cursorName );

  return gotit;
}


QgsDataSourceURI& QgsPostgresProvider::getURI()
{
  return mUri;
}

void QgsPostgresProvider::setExtent( QgsRectangle& newExtent )
{
  layerExtent.setXMaximum( newExtent.xMaximum() );
  layerExtent.setXMinimum( newExtent.xMinimum() );
  layerExtent.setYMaximum( newExtent.yMaximum() );
  layerExtent.setYMinimum( newExtent.yMinimum() );
}

// TODO - make this function return the real extent_
QgsRectangle QgsPostgresProvider::extent()
{
  return layerExtent;      //extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
}

/**
 * Return the feature type
 */
QGis::WkbType QgsPostgresProvider::geometryType() const
{
  return geomType;
}

/**
 * Return the feature type
 */
long QgsPostgresProvider::featureCount() const
{
  return featuresCounted;
}

const QgsField &QgsPostgresProvider::field( int index ) const
{
  QgsFieldMap::const_iterator it = attributeFields.find( index );

  if ( it == attributeFields.constEnd() )
  {
    QgsDebugMsg( "Field " + QString::number( index ) + " not found." );
    throw PGFieldNotFound();
  }

  return it.value();
}

/**
 * Return the number of fields
 */
uint QgsPostgresProvider::fieldCount() const
{
  return attributeFields.size();
}

const QgsFieldMap & QgsPostgresProvider::fields() const
{
  return attributeFields;
}

QString QgsPostgresProvider::dataComment() const
{
  return mDataComment;
}

void QgsPostgresProvider::rewind()
{
  if ( mFetching )
  {
    //move cursor to first record
    connectionRO->PQexecNR( QString( "move 0 in qgisf%1" ).arg( providerId ) );
  }
  mFeatureQueue.empty();
  loadFields();
}

/** @todo XXX Perhaps this should be promoted to QgsDataProvider? */
QString QgsPostgresProvider::endianString()
{
  switch ( QgsApplication::endian() )
  {
    case QgsApplication::NDR:
      return QString( "NDR" );
      break;
    case QgsApplication::XDR:
      return QString( "XDR" );
      break;
    default :
      return QString( "Unknown" );
  }
}

void QgsPostgresProvider::loadFields()
{
  QgsDebugMsg( "Loading fields for table " + mTableName );

  // Get the relation oid for use in later queries
  QString sql = QString( "SELECT regclass(%1)::oid" ).arg( quotedValue( mSchemaTableName ) );
  Result tresult = connectionRO->PQexec( sql );
  QString tableoid = QString::fromUtf8( PQgetvalue( tresult, 0, 0 ) );

  // Get the table description
  sql = QString( "SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=0" ).arg( tableoid );
  tresult = connectionRO->PQexec( sql );
  if ( PQntuples( tresult ) > 0 )
    mDataComment = QString::fromUtf8( PQgetvalue( tresult, 0, 0 ) );

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  sql = QString( "select * from %1 limit 0" ).arg( mSchemaTableName );

  Result result = connectionRO->PQexec( sql );

  // The queries inside this loop could possibly be combined into one
  // single query - this would make the code run faster.

  attributeFields.clear();
  for ( int i = 0; i < PQnfields( result ); i++ )
  {
    QString fieldName = QString::fromUtf8( PQfname( result, i ) );
    if ( fieldName == geometryColumn )
      continue;

    int fldtyp = PQftype( result, i );
    QString typOid = QString().setNum( fldtyp );
    int fieldModifier = PQfmod( result, i );
    QString fieldComment( "" );

    sql = QString( "SELECT typname,typtype,typelem,typlen FROM pg_type WHERE oid=%1" ).arg( typOid );
    // just oid; needs more work to support array type
    //      "oid = (SELECT Distinct typelem FROM pg_type WHERE "  //needs DISTINCT to guard against 2 or more rows on int2
    //      "typelem = " + typOid + " AND typlen = -1)";

    Result oidResult = connectionRO->PQexec( sql );
    QString fieldTypeName = QString::fromUtf8( PQgetvalue( oidResult, 0, 0 ) );
    QString fieldTType = QString::fromUtf8( PQgetvalue( oidResult, 0, 1 ) );
    QString fieldElem = QString::fromUtf8( PQgetvalue( oidResult, 0, 2 ) );
    int fieldSize = QString::fromUtf8( PQgetvalue( oidResult, 0, 3 ) ).toInt();

    sql = QString( "SELECT attnum FROM pg_attribute WHERE attrelid=%1 AND attname=%2" )
          .arg( tableoid ).arg( quotedValue( fieldName ) );

    Result tresult = connectionRO->PQexec( sql );
    QString attnum = QString::fromUtf8( PQgetvalue( tresult, 0, 0 ) );

    sql = QString( "SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=%2" )
          .arg( tableoid ).arg( attnum );

    tresult = connectionRO->PQexec( sql );
    if ( PQntuples( tresult ) > 0 )
      fieldComment = QString::fromUtf8( PQgetvalue( tresult, 0, 0 ) );

    QVariant::Type fieldType;

    if ( fieldTType == "b" )
    {
      bool isArray = fieldTypeName.startsWith( "_" );

      if ( isArray )
        fieldTypeName = fieldTypeName.mid( 1 );

      if ( fieldTypeName == "int8" )
      {
        fieldType = QVariant::LongLong;
        fieldSize = -1;
      }
      else if ( fieldTypeName.startsWith( "int" ) ||
                fieldTypeName == "serial" )
      {
        fieldType = QVariant::Int;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "real" ||
                fieldTypeName == "double precision" ||
                fieldTypeName.startsWith( "float" ) ||
                fieldTypeName == "numeric" )
      {
        fieldType = QVariant::Double;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "text" ||
                fieldTypeName == "bpchar" ||
                fieldTypeName == "varchar" ||
                fieldTypeName == "bool" ||
                fieldTypeName == "geometry" ||
                fieldTypeName == "money" ||
                fieldTypeName.startsWith( "time" ) ||
                fieldTypeName.startsWith( "date" ) )
      {
        fieldType = QVariant::String;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "char" )
      {
        fieldType = QVariant::String;
      }
      else
      {
        QgsDebugMsg( "Field " + fieldName + " ignored, because of unsupported type " + fieldTypeName );
        continue;
      }

      if ( isArray )
      {
        fieldTypeName = "_" + fieldTypeName;
        fieldType = QVariant::String;
        fieldSize = -1;
      }
    }
    else if ( fieldTType == "e" )
    {
      // enum
      fieldType = QVariant::String;
      fieldSize = -1;
    }
    else
    {
      QgsDebugMsg( "Field " + fieldName + " ignored, because of unsupported type type " + fieldTType );
      continue;
    }

    attributeFields.insert( i, QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldModifier, fieldComment ) );
  }
}

QString QgsPostgresProvider::getPrimaryKey()
{
  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql = QString( "select indkey from pg_index where indisunique and indrelid=regclass(%1)::oid and indpred is null" )
                .arg( quotedValue( mSchemaTableName ) );

  QgsDebugMsg( "Getting unique index using '" + sql + "'" );

  Result pk = connectionRO->PQexec( sql );

  QgsDebugMsg( "Got " + QString::number( PQntuples( pk ) ) + " rows." );

  QStringList log;

  // if we got no tuples we ain't got no unique index :)
  if ( PQntuples( pk ) == 0 )
  {
    QgsDebugMsg( "Relation has no unique index -- investigating alternatives" );

    // Two options here. If the relation is a table, see if there is
    // an oid column that can be used instead.
    // If the relation is a view try to find a suitable column to use as
    // the primary key.

    sql = QString( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" )
          .arg( quotedValue( mSchemaTableName ) );
    Result tableType = connectionRO->PQexec( sql );
    QString type = QString::fromUtf8( PQgetvalue( tableType, 0, 0 ) );

    if ( type == "r" ) // the relation is a table
    {
      QgsDebugMsg( "Relation is a table. Checking to see if it has an oid column." );

      primaryKey = "";

      // If there is an oid on the table, use that instead,
      // otherwise give up
      sql = QString( "SELECT attname FROM pg_attribute WHERE attname='oid' AND attrelid=regclass(%1)" )
            .arg( quotedValue( mSchemaTableName ) );

      Result oidCheck = connectionRO->PQexec( sql );

      if ( PQntuples( oidCheck ) != 0 )
      {
        // Could warn the user here that performance will suffer if
        // oid isn't indexed (and that they may want to add a
        // primary key to the table)
        primaryKey = "oid";
        primaryKeyType = "int4";
      }
      else
      {
        sql = QString( "SELECT attname FROM pg_attribute WHERE attname='ctid' AND attrelid=regclass(%1)" )
              .arg( quotedValue( mSchemaTableName ) );

        Result ctidCheck = connectionRO->PQexec( sql );

        if ( PQntuples( ctidCheck ) == 1 )
        {
          sql = QString( "SELECT max(substring(ctid::text from E'\\\\((\\\\d+),\\\\d+\\\\)')::integer) from %1" )
                .arg( mSchemaTableName );

          Result ctidCheck = connectionRO->PQexec( sql );
          if ( PQntuples( ctidCheck ) == 1 )
          {
            int id = QString( PQgetvalue( ctidCheck, 0, 0 ) ).toInt();

            if ( id < 0x10000 )
            {
              // fallback to ctid
              primaryKey = "ctid";
              primaryKeyType = "tid";
            }
          }
        }
      }

      if ( primaryKey.isEmpty() )
      {
        showMessageBox( tr( "No suitable key column in table" ),
                        tr( "The table has no column suitable for use as a key.\n\n"
                            "Qgis requires that the table either has a column of type\n"
                            "int4 with a unique constraint on it (which includes the\n"
                            "primary key), has a PostgreSQL oid column or has a ctid\n"
                            "column with a 16bit block number.\n" ) );
      }
      else
      {
        mPrimaryKeyDefault = defaultValue( primaryKey ).toString();
        if ( mPrimaryKeyDefault.isNull() )
        {
          mPrimaryKeyDefault = QString( "max(%1)+1 from %2.%3" )
                               .arg( quotedIdentifier( primaryKey ) )
                               .arg( quotedIdentifier( mSchemaName ) )
                               .arg( quotedIdentifier( mTableName ) );
        }
      }
    }
    else if ( type == "v" ) // the relation is a view
    {
      if ( !primaryKey.isEmpty() )
      {
        // check last used candidate
        sql = QString( "select pg_type.typname from pg_attribute,pg_type where atttypid=pg_type.oid and attname=%1 and attrelid=regclass(%2)" )
              .arg( quotedValue( primaryKey ) ).arg( quotedValue( mSchemaTableName ) );

        QgsDebugMsg( "checking candidate: " + sql );

        Result result = connectionRO->PQexec( sql );

        QString type;
        if ( PQresultStatus( result ) == PGRES_TUPLES_OK &&
             PQntuples( result ) == 1 )
        {
          type = PQgetvalue( result, 0, 0 );
        }

        // mPrimaryKeyDefault stays null and is retrieved later on demand

        if (( type != "int4" && type != "oid" ) ||
            !uniqueData( mSchemaName, mTableName, primaryKey ) )
        {
          primaryKey = "";
        }
      }

      if ( primaryKey.isEmpty() )
      {
        parseView();
      }
    }
    else
      QgsDebugMsg( "Unexpected relation type of '" + type + "'." );
  }
  else // have some unique indices on the table. Now choose one...
  {
    // choose which (if more than one) unique index to use
    std::vector<std::pair<QString, QString> > suitableKeyColumns;
    for ( int i = 0; i < PQntuples( pk ); ++i )
    {
      QString col = QString::fromUtf8( PQgetvalue( pk, i, 0 ) );
      QStringList columns = col.split( " ", QString::SkipEmptyParts );
      if ( columns.count() == 1 )
      {
        // Get the column name and data type
        sql = QString( "select attname,pg_type.typname from pg_attribute,pg_type where atttypid=pg_type.oid and attnum=%1 and attrelid=regclass(%2)" )
              .arg( col ).arg( quotedValue( mSchemaTableName ) );
        Result types = connectionRO->PQexec( sql );

        if ( PQntuples( types ) > 0 )
        {
          QString columnName = QString::fromUtf8( PQgetvalue( types, 0, 0 ) );
          QString columnType = QString::fromUtf8( PQgetvalue( types, 0, 1 ) );

          if ( columnType != "int4" )
            log.append( tr( "The unique index on column '%1' is unsuitable because Qgis does not currently "
                            "support non-int4 type columns as a key into the table.\n" ).arg( columnName ) );
          else
            suitableKeyColumns.push_back( std::make_pair( columnName, columnType ) );
        }
        else
        {
          //QgsDebugMsg( QString("name and type of %3. column of %1.%2 not found").arg(mSchemaName).arg(mTables).arg(col) );
        }
      }
      else
      {
        sql = QString( "select attname from pg_attribute, pg_type where atttypid=pg_type.oid and attnum in (%1) and attrelid=regclass(%2)::oid" )
              .arg( col.replace( " ", "," ) )
              .arg( quotedValue( mSchemaTableName ) );

        Result types = connectionRO->PQexec( sql );
        QString colNames;
        int numCols = PQntuples( types );
        for ( int j = 0; j < numCols; ++j )
        {
          if ( j == numCols - 1 )
            colNames += tr( "and " );
          colNames += quotedValue( QString::fromUtf8( PQgetvalue( types, j, 0 ) ) );
          if ( j < numCols - 2 )
            colNames += ",";
        }

        log.append( tr( "The unique index based on columns %1 is unsuitable because Qgis does not currently "
                        "support multiple columns as a key into the table.\n" ).arg( colNames ) );
      }
    }

    // suitableKeyColumns now contains the name of columns (and their
    // data type) that
    // are suitable for use as a key into the table. If there is
    // more than one we need to choose one. For the moment, just
    // choose the first in the list.

    if ( suitableKeyColumns.size() > 0 )
    {
      primaryKey = suitableKeyColumns[0].first;
      primaryKeyType = suitableKeyColumns[0].second;
    }
    else
    {
      // If there is an oid on the table, use that instead,
      // otherwise give up
      sql = QString( "select attname from pg_attribute where attname='oid' and attrelid=regclass(%1)::oid" ).arg( quotedValue( mSchemaTableName ) );
      Result oidCheck = connectionRO->PQexec( sql );

      if ( PQntuples( oidCheck ) != 0 )
      {
        primaryKey = "oid";
        primaryKeyType = "int4";
      }
      else
      {
        log.prepend( "There were no columns in the table that were suitable "
                     "as a qgis key into the table (either a column with a "
                     "unique index and type int4 or a PostgreSQL oid column.\n" );
      }
    }

    // Either primaryKey has been set by the above code, or it
    // hasn't. If not, present some info to the user to give them some
    // idea of why not.
    if ( primaryKey.isEmpty() )
    {
      // Give some info to the user about why things didn't work out.
      valid = false;
      showMessageBox( tr( "Unable to find a key column" ), log );
    }
    else
    {
      mPrimaryKeyDefault = defaultValue( primaryKey ).toString();
      if ( mPrimaryKeyDefault.isNull() )
      {
        mPrimaryKeyDefault = QString( "max(%1)+1 from %2.%3" )
                             .arg( quotedIdentifier( primaryKey ) )
                             .arg( quotedIdentifier( mSchemaName ) )
                             .arg( quotedIdentifier( mTableName ) );
      }
    }
  }

  if ( !primaryKey.isNull() )
  {
    QgsDebugMsg( "Qgis row key is " + primaryKey );
  }
  else
  {
    QgsDebugMsg( "Qgis row key was not set." );
  }

  return primaryKey;
}

void QgsPostgresProvider::parseView()
{
  // Have a poke around the view to see if any of the columns
  // could be used as the primary key.
  tableCols cols;

  // Given a schema.view, populate the cols variable with the
  // schema.table.column's that underly the view columns.
  findColumns( cols );

  // pick the primary key, if we don't have one yet
  if ( primaryKey.isEmpty() )
  {
    // From the view columns, choose one for which the underlying
    // column is suitable for use as a key into the view.
    primaryKey = chooseViewColumn( cols );
  }

  tableCols::const_iterator it = cols.find( primaryKey );
  if ( it != cols.end() )
  {
    mPrimaryKeyDefault = defaultValue( it->second.column, it->second.relation, it->second.schema ).toString();
    if ( mPrimaryKeyDefault.isNull() )
    {
      mPrimaryKeyDefault = QString( "max(%1)+1 from %2.%3" )
                           .arg( quotedIdentifier( it->second.column ) )
                           .arg( quotedIdentifier( it->second.schema ) )
                           .arg( quotedIdentifier( it->second.relation ) );
    }
  }
  else
  {
    mPrimaryKeyDefault = QString( "max(%1)+1 from %2.%3" )
                         .arg( quotedIdentifier( primaryKey ) )
                         .arg( quotedIdentifier( mSchemaName ) )
                         .arg( quotedIdentifier( mTableName ) );
  }
}

QString QgsPostgresProvider::primaryKeyDefault()
{
  if ( mPrimaryKeyDefault.isNull() )
    parseView();

  return mPrimaryKeyDefault;
}

// Given the table and column that each column in the view refers to,
// choose one. Prefers column with an index on them, but will
// otherwise choose something suitable.

QString QgsPostgresProvider::chooseViewColumn( const tableCols &cols )
{
  // For each relation name and column name need to see if it
  // has unique constraints on it, or is a primary key (if not,
  // it shouldn't be used). Should then be left with one or more
  // entries in the map which can be used as the key.

  QString sql, key;
  QStringList log;
  tableCols suitable;
  // Cache of relation oid's
  std::map<QString, QString> relOid;

  std::vector<tableCols::const_iterator> oids;
  tableCols::const_iterator iter = cols.begin();
  for ( ; iter != cols.end(); ++iter )
  {
    QString viewCol   = iter->first;
    QString schemaName = iter->second.schema;
    QString tableName = iter->second.relation;
    QString tableCol  = iter->second.column;
    QString colType   = iter->second.type;

    // Get the oid from pg_class for the given schema.relation for use
    // in subsequent queries.
    sql = QString( "select regclass(%1)::oid" ).arg( quotedValue( quotedIdentifier( schemaName ) + "." + quotedIdentifier( tableName ) ) );
    Result result = connectionRO->PQexec( sql );
    QString rel_oid;
    if ( PQntuples( result ) == 1 )
    {
      rel_oid = PQgetvalue( result, 0, 0 );
      // Keep the rel_oid for use later one.
      relOid[viewCol] = rel_oid;
    }
    else
    {
      QgsDebugMsg( "Relation " + schemaName + "." + tableName +
                   " doesn't exist in the pg_class table."
                   "This shouldn't happen and is odd." );
      continue;
    }

    // This sql returns one or more rows if the column 'tableCol' in
    // table 'tableName' and schema 'schemaName' has one or more
    // columns that satisfy the following conditions:
    // 1) the column has data type of int4.
    // 2) the column has a unique constraint or primary key constraint
    //    on it.
    // 3) the constraint applies just to the column of interest (i.e.,
    //    it isn't a constraint over multiple columns.
    sql = QString( "select * from pg_constraint where "
                   "conkey[1]=(select attnum from pg_attribute where attname=%1 and attrelid=%2) "
                   "and conrelid=%2 and (contype='p' or contype='u') "
                   "and array_dims(conkey)='[1:1]'" ).arg( quotedValue( tableCol ) ).arg( rel_oid );

    result = connectionRO->PQexec( sql );
    if ( PQntuples( result ) == 1 && colType == "int4" )
      suitable[viewCol] = iter->second;

    QString details = tr( "'%1' derives from '%2.%3.%4' " ).arg( viewCol ).arg( schemaName ).arg( tableName ).arg( tableCol );

    if ( PQntuples( result ) == 1 && colType == "int4" )
    {
      details += tr( "and is suitable." );
    }
    else
    {
      details += tr( "and is not suitable (type is %1)" ).arg( colType );
      if ( PQntuples( result ) == 1 )
        details += tr( " and has a suitable constraint)" );
      else
        details += tr( " and does not have a suitable constraint)" );
    }

    log << details;

    if ( tableCol == "oid" )
      oids.push_back( iter );
  }

  // 'oid' columns in tables don't have a constraint on them, but
  // they are useful to consider, so add them in if not already
  // here.
  for ( uint i = 0; i < oids.size(); ++i )
  {
    if ( suitable.find( oids[i]->first ) == suitable.end() )
    {
      suitable[oids[i]->first] = oids[i]->second;

      QgsDebugMsg( "Adding column " + oids[i]->first + " as it may be suitable." );
    }
  }

  // Now have a map containing all of the columns in the view that
  // might be suitable for use as the key to the table. Need to choose
  // one thus:
  //
  // If there is more than one suitable column pick one that is
  // indexed, else pick one called 'oid' if it exists, else
  // pick the first one. If there are none we return an empty string.

  // Search for one with an index
  tableCols::const_iterator i = suitable.begin();
  for ( ; i != suitable.end(); ++i )
  {
    // Get the relation oid from our cache.
    QString rel_oid = relOid[i->first];
    // And see if the column has an index
    sql = QString( "select * from pg_index where indrelid=%1 and indkey[0]=(select attnum from pg_attribute where attrelid=%1 and attname=%2)" )
          .arg( rel_oid )
          .arg( quotedValue( i->second.column ) );
    Result result = connectionRO->PQexec( sql );

    if ( PQntuples( result ) > 0 && uniqueData( mSchemaName, mTableName, i->first ) )
    {
      // Got one. Use it.
      key = i->first;
      QgsDebugMsg( "Picked column '" + key + "' because it has an index." );
      break;
    }
  }

  if ( key.isEmpty() )
  {
    // If none have indices, choose one that is called 'oid' (if it
    // exists). This is legacy support and could be removed in
    // future.
    i = suitable.find( "oid" );
    if ( i != suitable.end() && uniqueData( mSchemaName, mTableName, i->first ) )
    {
      key = i->first;

      QgsDebugMsg( "Picked column " + key +
                   " as it is probably the postgresql object id "
                   " column (which contains unique values) and there are no"
                   " columns with indices to choose from." );
    }
    // else choose the first one in the container that has unique data
    else
    {
      tableCols::const_iterator i = suitable.begin();
      for ( ; i != suitable.end(); ++i )
      {
        if ( uniqueData( mSchemaName, mTableName, i->first ) )
        {
          key = i->first;

          QgsDebugMsg( "Picked column " + key +
                       " as it was the first suitable column found"
                       " with unique data and were are no"
                       " columns with indices to choose from" );
          break;
        }
        else
        {
          log << tr( "Note: '%1' initially appeared suitable"
                     " but does not contain unique data, so is not suitable.\n" )
          .arg( i->first );
        }
      }
    }
  }

  if ( key.isEmpty() )
  {
    valid = false;
    log.prepend( tr( "The view '%1.%2' has no column suitable for use as a unique key.\n"
                     "Qgis requires that the view has a column that can be used "
                     "as a unique key. Such a column should be derived from "
                     "a table column of type int4 and be a primary key, "
                     "have a unique constraint on it, or be a PostgreSQL "
                     "oid column. To improve performance the column should also be indexed.\n"
                     "The view you selected has the following columns, none "
                     "of which satisfy the above conditions:" ).arg( mSchemaName ).arg( mTableName ) );
    showMessageBox( tr( "No suitable key column in view" ), log );
  }

  return key;
}

bool QgsPostgresProvider::uniqueData( QString schemaName,
                                      QString tableName, QString colName )
{
  // Check to see if the given column contains unique data

  bool isUnique = false;

  QString sql = QString( "select count(distinct %1)=count(%1) from %2.%3" )
                .arg( quotedIdentifier( colName ) )
                .arg( quotedIdentifier( schemaName ) )
                .arg( quotedIdentifier( tableName ) );

  if ( !sqlWhereClause.isEmpty() )
  {
    sql += " where " + sqlWhereClause;
  }

  Result unique = connectionRO->PQexec( sql );

  if ( PQntuples( unique ) == 1 && QString::fromUtf8( PQgetvalue( unique, 0, 0 ) ).startsWith( "t" ) )
    isUnique = true;

  return isUnique;
}

int QgsPostgresProvider::SRCFromViewColumn( const QString& ns, const QString& relname, const QString& attname_table, const QString& attname_view, const QString& viewDefinition, SRC& result ) const
{
  QString newViewDefSql = QString( "SELECT definition FROM pg_views WHERE schemaname=%1 AND viewname=%2" )
                          .arg( quotedValue( ns ) ).arg( quotedValue( relname ) );
  Result newViewDefResult = connectionRO->PQexec( newViewDefSql );
  int numEntries = PQntuples( newViewDefResult );

  if ( numEntries > 0 ) //relation is a view
  {
    QString newViewDefinition( QString::fromUtf8( PQgetvalue( newViewDefResult, 0, 0 ) ) );

    QString newAttNameView = attname_table;
    QString newAttNameTable = attname_table;

    //find out the attribute name of the underlying table/view
    if ( newViewDefinition.contains( " AS " ) )
    {
      QRegExp s( "(\\w+)" + QString( " AS " ) + QRegExp::escape( attname_table ) );
      if ( s.indexIn( newViewDefinition ) != -1 )
      {
        newAttNameTable = s.cap( 1 );
      }
    }

    QString viewColumnSql =
      QString( "SELECT "
               "table_schema,"
               "table_name,"
               "column_name"
               " FROM "
               "("
               "SELECT DISTINCT "
               "current_database()::information_schema.sql_identifier AS view_catalog,"
               "nv.nspname::information_schema.sql_identifier AS view_schema,"
               "v.relname::information_schema.sql_identifier AS view_name,"
               "current_database()::information_schema.sql_identifier AS table_catalog,"
               "nt.nspname::information_schema.sql_identifier AS table_schema,"
               "t.relname::information_schema.sql_identifier AS table_name,"
               "a.attname::information_schema.sql_identifier AS column_name"
               " FROM "
               "pg_namespace nv,"
               "pg_class v,"
               "pg_depend dv,"
               "pg_depend dt,"
               "pg_class t,"
               "pg_namespace nt,"
               "pg_attribute a"
               " WHERE "
               "nv.oid=v.relnamespace AND "
               "v.relkind='v'::\"char\" AND "
               "v.oid=dv.refobjid AND "
               "dv.refclassid='pg_class'::regclass::oid AND "
               "dv.classid='pg_rewrite'::regclass::oid AND "
               "dv.deptype='i'::\"char\" AND "
               "dv.objid = dt.objid AND "
               "dv.refobjid<>dt.refobjid AND "
               "dt.classid='pg_rewrite'::regclass::oid AND "
               "dt.refclassid='pg_class'::regclass::oid AND "
               "dt.refobjid=t.oid AND "
               "t.relnamespace = nt.oid AND "
               "(t.relkind=ANY (ARRAY['r'::\"char\", 'v'::\"char\"])) AND "
               "t.oid=a.attrelid AND "
               "dt.refobjsubid=a.attnum"
               " ORDER BY "
               "current_database()::information_schema.sql_identifier,"
               "nv.nspname::information_schema.sql_identifier,"
               "v.relname::information_schema.sql_identifier,"
               "current_database()::information_schema.sql_identifier,"
               "nt.nspname::information_schema.sql_identifier,"
               "t.relname::information_schema.sql_identifier,"
               "a.attname::information_schema.sql_identifier"
               ") x"
               " WHERE "
               "view_schema=%1 AND "
               "view_name=%2 AND "
               "column_name=%3" )
      .arg( quotedValue( ns ) )
      .arg( quotedValue( relname ) )
      .arg( quotedValue( newAttNameTable ) );

    Result viewColumnResult = connectionRO->PQexec( viewColumnSql );
    if ( PQntuples( viewColumnResult ) > 0 )
    {
      QString newTableSchema = QString::fromUtf8( PQgetvalue( viewColumnResult, 0, 0 ) );
      QString newTableName = QString::fromUtf8( PQgetvalue( viewColumnResult, 0, 1 ) );
      int retvalue = SRCFromViewColumn( newTableSchema, newTableName, newAttNameTable, newAttNameView, newViewDefinition, result );
      return retvalue;
    }
    else
    {
      return 1;
    }

  }

  //relation is table, we just have to add the type
  QString typeSql = QString( "SELECT "
                             "pg_type.typname"
                             " FROM "
                             "pg_attribute,"
                             "pg_class,"
                             "pg_namespace,"
                             "pg_type"
                             " WHERE "
                             "pg_class.relname=%1 AND "
                             "pg_namespace.nspname=%2 AND "
                             "pg_attribute.attname=%3 AND "
                             "pg_attribute.attrelid=pg_class.oid AND "
                             "pg_class.relnamespace=pg_namespace.oid AND "
                             "pg_attribute.atttypid=pg_type.oid" )
                    .arg( quotedValue( relname ) )
                    .arg( quotedValue( ns ) )
                    .arg( quotedValue( attname_table ) );

  Result typeSqlResult = connectionRO->PQexec( typeSql );
  if ( PQntuples( typeSqlResult ) < 1 )
  {
    return 1;
  }
  QString type = QString::fromUtf8( PQgetvalue( typeSqlResult, 0, 0 ) );

  result.schema = ns;
  result.relation = relname;
  result.column = attname_table;
  result.type = type;
  return 0;
}

// This function will return in the cols variable the
// underlying view and columns for each column in
// mSchemaName.mTableName.

void QgsPostgresProvider::findColumns( tableCols& cols )
{
  QString viewColumnSql =
    QString( "SELECT "
             "table_schema,"
             "table_name,"
             "column_name"
             " FROM "
             "("
             "SELECT DISTINCT "
             "current_database() AS view_catalog,"
             "nv.nspname AS view_schema,"
             "v.relname AS view_name,"
             "current_database() AS table_catalog,"
             "nt.nspname AS table_schema,"
             "t.relname AS table_name,"
             "a.attname AS column_name"
             " FROM "
             "pg_namespace nv,"
             "pg_class v,"
             "pg_depend dv,"
             "pg_depend dt,"
             "pg_class t,"
             "pg_namespace nt,"
             "pg_attribute a"
             " WHERE "
             "nv.oid=v.relnamespace AND "
             "v.relkind='v'::\"char\" AND "
             "v.oid=dv.refobjid AND "
             "dv.refclassid='pg_class'::regclass::oid AND "
             "dv.classid='pg_rewrite'::regclass::oid AND "
             "dv.deptype='i'::\"char\" AND "
             "dv.objid=dt.objid AND "
             "dv.refobjid<>dt.refobjid AND "
             "dt.classid='pg_rewrite'::regclass::oid AND "
             "dt.refclassid='pg_class'::regclass::oid AND "
             "dt.refobjid=t.oid AND "
             "t.relnamespace=nt.oid AND "
             "(t.relkind = ANY (ARRAY['r'::\"char\",'v'::\"char\"])) AND "
             "t.oid=a.attrelid AND "
             "dt.refobjsubid=a.attnum"
             " ORDER BY "
             "current_database(),"
             "nv.nspname,"
             "v.relname,"
             "current_database(),"
             "nt.nspname,"
             "t.relname,"
             "a.attname"
             ") x"
             " WHERE "
             "view_schema=%1 AND view_name=%2" )
    .arg( quotedValue( mSchemaName ) )
    .arg( quotedValue( mTableName ) );

  if ( !primaryKey.isEmpty() )
  {
    viewColumnSql += QString( " AND column_name=%1" ).arg( quotedValue( primaryKey ) );
  }

  Result viewColumnResult = connectionRO->PQexec( viewColumnSql );

  //find out view definition
  QString viewDefSql = QString( "SELECT definition FROM pg_views WHERE schemaname=%1 AND viewname=%2" )
                       .arg( quotedValue( mSchemaName ) )
                       .arg( quotedValue( mTableName ) );
  Result viewDefResult = connectionRO->PQexec( viewDefSql );
  if ( PQntuples( viewDefResult ) < 1 )
  {
    return;
  }

  QString viewDefinition( QString::fromUtf8( PQgetvalue( viewDefResult, 0, 0 ) ) );

  QString ns, relname, attname_table, attname_view;
  SRC columnInformation;

  for ( int i = 0; i < PQntuples( viewColumnResult ); ++i )
  {
    ns = QString::fromUtf8( PQgetvalue( viewColumnResult, i, 0 ) );
    relname = QString::fromUtf8( PQgetvalue( viewColumnResult, i, 1 ) );
    attname_table = QString::fromUtf8( PQgetvalue( viewColumnResult, i, 2 ) );

    //find out original attribute name
    attname_view = attname_table;

    //examine if the column name has been renamed in the view with AS
    if ( viewDefinition.contains( " AS " ) )
    {
      // This regular expression needs more testing. Since the view
      // definition comes from postgresql and has been 'standardised', we
      // don't need to deal with everything that the user could put in a view
      // definition. Does the regexp have to deal with the schema??

      QRegExp s( ".* \"?" + QRegExp::escape( relname ) +
                 "\"?\\.\"?" + QRegExp::escape( attname_table ) +
                 "\"? AS \"?(\\w+)\"?,* .*" );

      QgsDebugMsg( viewDefinition + "\n" + s.pattern() );

      if ( s.indexIn( viewDefinition ) != -1 )
      {
        attname_view = s.cap( 1 );
        QgsDebugMsg( QString( "original view column name was: %1" ).arg( attname_view ) );
      }
    }

    SRCFromViewColumn( ns, relname, attname_table, attname_view, viewDefinition, columnInformation );
    cols.insert( std::make_pair( attname_view, columnInformation ) );
    QgsDebugMsg( "Inserting into cols (for key " + attname_view + " ): " + columnInformation.schema + "." + columnInformation.relation + "." + columnInformation.column + "." + columnInformation.type );
  }
}

// Returns the minimum value of an attribute
QVariant QgsPostgresProvider::minimumValue( int index )
{
  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql;
    if ( sqlWhereClause.isEmpty() )
    {
      sql = QString( "select min(%1) from %2" )
            .arg( quotedIdentifier( fld.name() ) )
            .arg( mSchemaTableName );
    }
    else
    {
      sql = QString( "select min(%1) from %2 where %3" )
            .arg( quotedIdentifier( fld.name() ) )
            .arg( mSchemaTableName )
            .arg( sqlWhereClause );
    }
    Result rmin = connectionRO->PQexec( sql );
    return convertValue( fld.type(), QString::fromUtf8( PQgetvalue( rmin, 0, 0 ) ) );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString::null );
  }
}

// Returns the list of unique values of an attribute
void QgsPostgresProvider::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit )
{
  uniqueValues.clear();

  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql;
    if ( sqlWhereClause.isEmpty() )
    {
      sql = QString( "select distinct %1 from %2 order by %1" )
            .arg( quotedIdentifier( fld.name() ) )
            .arg( mSchemaTableName );
    }
    else
    {
      sql = QString( "select distinct %1 from %2 where %3 order by %1" )
            .arg( quotedIdentifier( fld.name() ) )
            .arg( mSchemaTableName )
            .arg( sqlWhereClause );
    }

    if ( limit >= 0 )
    {
      sql += QString( " LIMIT %1" ).arg( limit );
    }

    Result res = connectionRO->PQexec( sql );
    if ( PQresultStatus( res ) == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < PQntuples( res ); i++ )
        uniqueValues.append( convertValue( fld.type(), QString::fromUtf8( PQgetvalue( res, i, 0 ) ) ) );
    }
  }
  catch ( PGFieldNotFound )
  {
  }
}

void QgsPostgresProvider::enumValues( int index, QStringList& enumList )
{
  enumList.clear();

  QString typeName;
  //find out type of index
  QgsFieldMap::const_iterator f_it = attributeFields.find( index );
  if ( f_it != attributeFields.constEnd() )
  {
    typeName = f_it.value().typeName();
  }
  else
  {
    return;
  }

  //is type an enum?
  QString typeSql = QString( "SELECT typtype FROM pg_type where typname = %1" ).arg( quotedValue( typeName ) );
  Result typeRes = connectionRO->PQexec( typeSql );
  if ( PQresultStatus( typeRes ) != PGRES_TUPLES_OK || PQntuples( typeRes ) < 1 )
  {
    return;
  }


  QString typtype = PQgetvalue( typeRes, 0, 0 );
  if ( typtype.compare( "e", Qt::CaseInsensitive ) == 0 )
  {
    //try to read enum_range of attribute
    if ( !parseEnumRange( enumList, f_it->name() ) )
    {
      enumList.clear();
    }
  }
  else
  {
    //is there a domain check constraint for the attribute?
    if ( !parseDomainCheckConstraint( enumList, f_it->name() ) )
    {
      enumList.clear();
    }
  }
}

bool QgsPostgresProvider::parseEnumRange( QStringList& enumValues, const QString& attributeName ) const
{
  enumValues.clear();
  QString enumRangeSql = QString( "SELECT enum_range(%1) from %2 limit 1" ).arg( quotedIdentifier( attributeName ) ).arg( mSchemaTableName );
  Result enumRangeRes = connectionRO->PQexec( enumRangeSql );
  if ( PQresultStatus( enumRangeRes ) == PGRES_TUPLES_OK && PQntuples( enumRangeRes ) > 0 )
  {
    QString enumRangeString = PQgetvalue( enumRangeRes, 0, 0 );
    //strip away the brackets at begin and end
    enumRangeString.chop( 1 );
    enumRangeString.remove( 0, 1 );
    QStringList rangeSplit = enumRangeString.split( "," );
    QStringList::const_iterator range_it = rangeSplit.constBegin();
    for ( ; range_it != rangeSplit.constEnd(); ++range_it )
    {
      QString currentEnumValue = *range_it;
      //remove quotes from begin and end of the value
      if ( currentEnumValue.startsWith( "'" ) || currentEnumValue.startsWith( "\"" ) )
      {
        currentEnumValue.remove( 0, 1 );
      }
      if ( currentEnumValue.endsWith( "'" ) || currentEnumValue.endsWith( "\"" ) )
      {
        currentEnumValue.chop( 1 );
      }
      enumValues << currentEnumValue;
    }
    return true;
  }
  return false;
}

bool QgsPostgresProvider::parseDomainCheckConstraint( QStringList& enumValues, const QString& attributeName ) const
{
  enumValues.clear();

  //is it a domain type with a check constraint?
  QString domainSql = QString( "SELECT domain_name from information_schema.columns where table_name = %1 and column_name = %2" ).arg( quotedValue( mTableName ) ).arg( quotedValue( attributeName ) );
  Result domainResult = connectionRO->PQexec( domainSql );
  if ( PQresultStatus( domainResult ) == PGRES_TUPLES_OK && PQntuples( domainResult ) > 0 )
  {
    //a domain type
    QString domainCheckDefinitionSql = QString( "SELECT consrc FROM pg_constraint where conname = (SELECT constraint_name FROM information_schema.domain_constraints WHERE domain_name = %1)" ).arg( quotedValue( PQgetvalue( domainResult, 0, 0 ) ) );
    Result domainCheckRes = connectionRO->PQexec( domainCheckDefinitionSql );
    if ( PQresultStatus( domainCheckRes ) == PGRES_TUPLES_OK && PQntuples( domainCheckRes ) > 0 )
    {
      QString checkDefinition = QString::fromUtf8( PQgetvalue( domainCheckRes, 0, 0 ) );

      //we assume that the constraint is of the following form:
      //(VALUE = ANY (ARRAY['a'::text, 'b'::text, 'c'::text, 'd'::text]))
      //normally, postgresql creates that if the contstraint has been specified as 'VALUE in ('a', 'b', 'c', 'd')

      //todo: ANY must occure before ARRAY
      int anyPos = checkDefinition.indexOf( "VALUE = ANY" );
      int arrayPosition = checkDefinition.lastIndexOf( "ARRAY[" );
      int closingBracketPos = checkDefinition.indexOf( "]", arrayPosition + 6 );

      if ( anyPos == -1 || anyPos >= arrayPosition )
      {
        return false; //constraint has not the required format
      }

      if ( arrayPosition != -1 )
      {
        QString valueList = checkDefinition.mid( arrayPosition + 6, closingBracketPos );
        QStringList commaSeparation = valueList.split( ",", QString::SkipEmptyParts );
        QStringList::const_iterator cIt = commaSeparation.constBegin();
        for ( ; cIt != commaSeparation.constEnd(); ++cIt )
        {
          //get string between ''
          int beginQuotePos = cIt->indexOf( "'" );
          int endQuotePos = cIt->lastIndexOf( "'" );
          if ( beginQuotePos != -1 && ( endQuotePos - beginQuotePos ) > 1 )
          {
            enumValues << cIt->mid( beginQuotePos + 1, endQuotePos - beginQuotePos - 1 );
          }
        }
      }
      return true;
    }
  }
  return false;
}

// Returns the maximum value of an attribute
QVariant QgsPostgresProvider::maximumValue( int index )
{
  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql;
    if ( sqlWhereClause.isEmpty() )
    {
      sql = QString( "select max(%1) from %2" )
            .arg( quotedIdentifier( fld.name() ) )
            .arg( mSchemaTableName );
    }
    else
    {
      sql = QString( "select max(%1) from %2 where %3" )
            .arg( quotedIdentifier( fld.name() ) )
            .arg( mSchemaTableName )
            .arg( sqlWhereClause );
    }
    Result rmax = connectionRO->PQexec( sql );
    return convertValue( fld.type(), QString::fromUtf8( PQgetvalue( rmax, 0, 0 ) ) );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString::null );
  }
}


bool QgsPostgresProvider::isValid()
{
  return valid;
}

QVariant QgsPostgresProvider::defaultValue( QString fieldName, QString tableName, QString schemaName )
{
  if ( schemaName.isNull() )
    schemaName = mSchemaName;
  if ( tableName.isNull() )
    tableName = mTableName;

  // Get the default column value from the Postgres information
  // schema. If there is no default we return an empty string.

  // Maintaining a cache of the results of this query would be quite
  // simple and if this query is called lots, could save some time.

  QString sql( "SELECT column_default FROM"
               " information_schema.columns WHERE"
               " column_default IS NOT NULL"
               " AND table_schema = " + quotedValue( schemaName ) +
               " AND table_name = " + quotedValue( tableName ) +
               " AND column_name = " + quotedValue( fieldName ) );

  QVariant defaultValue( QString::null );

  Result result = connectionRO->PQexec( sql );

  if ( PQntuples( result ) == 1 && !PQgetisnull( result, 0, 0 ) )
    defaultValue = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );

  return defaultValue;
}

QVariant QgsPostgresProvider::defaultValue( int fieldId )
{
  try
  {
    return defaultValue( field( fieldId ).name() );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString::null );
  }
}

/**
 * Check to see if GEOS is available
 */
bool QgsPostgresProvider::Conn::hasGEOS()
{
  // make sure info is up to date for the current connection
  postgisVersion();
  // get geos capability
  return geosAvailable;
}

/* Functions for determining available features in postGIS */
QString QgsPostgresProvider::Conn::postgisVersion()
{
  Result result = PQexec( "select postgis_version()" );
  if ( PQntuples( result ) != 1 )
  {
    QgsDebugMsg( "Retrieval of postgis version failed" );
    return QString::null;
  }

  postgisVersionInfo = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );

  QgsDebugMsg( "PostGIS version info: " + postgisVersionInfo );

  QStringList postgisParts = postgisVersionInfo.split( " ", QString::SkipEmptyParts );

  // Get major and minor version
  QStringList postgisVersionParts = postgisParts[0].split( ".", QString::SkipEmptyParts );
  if ( postgisVersionParts.size() < 2 )
  {
    QgsDebugMsg( "Could not parse postgis version" );
    return QString::null;
  }

  postgisVersionMajor = postgisVersionParts[0].toInt();
  postgisVersionMinor = postgisVersionParts[1].toInt();

  // assume no capabilities
  geosAvailable = false;
  gistAvailable = false;
  projAvailable = false;

  // parse out the capabilities and store them
  QStringList geos = postgisParts.filter( "GEOS" );
  if ( geos.size() == 1 )
  {
    geosAvailable = ( geos[0].indexOf( "=1" ) > -1 );
  }
  QStringList gist = postgisParts.filter( "STATS" );
  if ( gist.size() == 1 )
  {
    gistAvailable = ( geos[0].indexOf( "=1" ) > -1 );
  }
  QStringList proj = postgisParts.filter( "PROJ" );
  if ( proj.size() == 1 )
  {
    projAvailable = ( proj[0].indexOf( "=1" ) > -1 );
  }

  mUseWkbHex = postgisVersionMajor < 1;

  gotPostgisVersion = true;

  return postgisVersionInfo;
}

QByteArray QgsPostgresProvider::paramValue( QString fieldValue, const QString &defaultValue ) const
{
  if ( fieldValue.isNull() )
    return QByteArray( 0 );  // QByteArray(0).isNull() is true

  if ( fieldValue == defaultValue && !defaultValue.isNull() )
  {
    PGresult *result = connectionRW->PQexec( QString( "select %1" ).arg( defaultValue ) );
    if ( PQresultStatus( result ) == PGRES_FATAL_ERROR )
      throw PGException( result );

    if ( PQgetisnull( result, 0, 0 ) )
    {
      PQclear( result );
      return QByteArray( 0 );  // QByteArray(0).isNull() is true
    }
    else
    {
      QString val = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );
      PQclear( result );
      return val.toUtf8();
    }
  }

  return fieldValue.toUtf8();
}

bool QgsPostgresProvider::addFeatures( QgsFeatureList &flist )
{
  if ( flist.size() == 0 )
    return true;

  if ( !connectRW() )
    return false;

  bool returnvalue = true;

  try
  {
    connectionRW->PQexecNR( "BEGIN" );

    // Prepare the INSERT statement
    QString insert = QString( "INSERT INTO %1(%2" )
                     .arg( mSchemaTableName )
                     .arg( quotedIdentifier( geometryColumn ) ),
                     values = QString( ") VALUES (GeomFromWKB($1%1,%2)" )
                              .arg( connectionRW->useWkbHex() ? "" : "::bytea" )
                              .arg( srid );

    int offset;
    if ( primaryKeyType != "tid" )
    {
      insert += "," + quotedIdentifier( primaryKey );
      values += ",$2";
      offset = 3;
    }
    else
    {
      offset = 2;
    }

    const QgsAttributeMap &attributevec = flist[0].attributeMap();

    QStringList defaultValues;
    QList<int> fieldId;

    // look for unique attribute values to place in statement instead of passing as parameter
    // e.g. for defaults
    for ( QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++ )
    {
      QgsFieldMap::const_iterator fit = attributeFields.find( it.key() );
      if ( fit == attributeFields.end() )
        continue;

      QString fieldname = fit->name();

      QgsDebugMsg( "Checking field against: " + fieldname );

      if ( fieldname.isEmpty() || fieldname == geometryColumn || fieldname == primaryKey )
        continue;

      int i;
      for ( i = 1; i < flist.size(); i++ )
      {
        const QgsAttributeMap &attributevec = flist[i].attributeMap();

        QgsAttributeMap::const_iterator thisit = attributevec.find( it.key() );
        if ( thisit == attributevec.end() )
          break;

        if ( *thisit != *it )
          break;
      }

      insert += "," + quotedIdentifier( fieldname );

      QString defVal = defaultValue( it.key() ).toString();

      if ( i == flist.size() )
      {
        if ( *it == defVal )
        {
          if ( defVal.isNull() )
          {
            values += ",NULL";
          }
          else
          {
            values += "," + defVal;
          }
        }
        else if ( fit->typeName() == "geometry" )
        {
          values += QString( ",geomfromewkt(%1)" ).arg( quotedValue( it->toString() ) );
        }
        else
        {
          values += "," + quotedValue( it->toString() );
        }
      }
      else
      {
        // value is not unique => add parameter
        if ( fit->typeName() == "geometry" )
        {
          values += QString( ",geomfromewkt($%1)" ).arg( defaultValues.size() + offset );
        }
        else
        {
          values += QString( ",$%1" ).arg( defaultValues.size() + offset );
        }
        defaultValues.append( defVal );
        fieldId.append( it.key() );
      }
    }

    insert += values + ")";

    QgsDebugMsg( QString( "prepare addfeatures: %1" ).arg( insert ) );
    PGresult *stmt = connectionRW->PQprepare( "addfeatures", insert, fieldId.size() + offset - 1, NULL );
    if ( stmt == 0 || PQresultStatus( stmt ) == PGRES_FATAL_ERROR )
      throw PGException( stmt );
    PQclear( stmt );

    QList<int> newIds;

    for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); features++ )
    {
      const QgsAttributeMap &attributevec = features->attributeMap();

      QString geomParam;
      appendGeomString( features->geometry(), geomParam );

      QStringList params;
      params << geomParam;

      if ( primaryKeyType != "tid" )
      {
        int id = paramValue( primaryKeyDefault(), primaryKeyDefault() ).toInt();
        params << QString::number( id );
        newIds << id;
      }

      for ( int i = 0; i < fieldId.size(); i++ )
        params << paramValue( attributevec[ fieldId[i] ].toString(), defaultValues[i] );

      PGresult *result = connectionRW->PQexecPrepared( "addfeatures", params );
      if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
        throw PGException( result );
      PQclear( result );
    }

    if ( flist.size() == newIds.size() )
      for ( int i = 0; i < flist.size(); i++ )
        flist[i].setFeatureId( newIds[i] );

    connectionRW->PQexecNR( "DEALLOCATE addfeatures" );
    connectionRW->PQexecNR( "COMMIT" );

    featuresCounted += flist.size();
  }
  catch ( PGException &e )
  {
    e.showErrorMessage( tr( "Error while adding features" ) );
    connectionRW->PQexecNR( "ROLLBACK" );
    connectionRW->PQexecNR( "DEALLOCATE addfeatures" );
    returnvalue = false;
  }

  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::deleteFeatures( const QgsFeatureIds & id )
{
  bool returnvalue = true;

  if ( !connectRW() )
    return false;

  try
  {
    connectionRW->PQexecNR( "BEGIN" );

    for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
    {
      QString sql = QString( "DELETE FROM %1 WHERE %2" )
                    .arg( mSchemaTableName ).arg( whereClause( *it ) );
      QgsDebugMsg( "delete sql: " + sql );

      //send DELETE statement and do error handling
      PGresult *result = connectionRW->PQexec( sql );
      if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
        throw PGException( result );
      PQclear( result );
    }

    connectionRW->PQexecNR( "COMMIT" );

    featuresCounted -= id.size();
  }
  catch ( PGException &e )
  {
    e.showErrorMessage( tr( "Error while deleting features" ) );
    connectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }
  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  if ( !connectRW() )
    return false;

  try
  {
    connectionRW->PQexecNR( "BEGIN" );

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString type = iter->typeName();
      if ( type == "char" || type == "varchar" )
      {
        type = QString( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == "numeric" || type == "decimal" )
      {
        type = QString( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
      }

      QString sql = QString( "ALTER TABLE %1 ADD COLUMN %2 %3" )
                    .arg( mSchemaTableName )
                    .arg( quotedIdentifier( iter->name() ) )
                    .arg( type );
      QgsDebugMsg( sql );

      //send sql statement and do error handling
      PGresult *result = connectionRW->PQexec( sql );
      if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
        throw PGException( result );
      PQclear( result );

      if ( !iter->comment().isEmpty() )
      {
        sql = QString( "COMMENT ON COLUMN %1.%2 IS %3" )
              .arg( mSchemaTableName )
              .arg( quotedIdentifier( iter->name() ) )
              .arg( quotedValue( iter->comment() ) );
        result = connectionRW->PQexec( sql );
        if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
          throw PGException( result );
        PQclear( result );
      }
    }

    connectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    e.showErrorMessage( tr( "Error while adding attributes" ) );
    connectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }

  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes( const QgsAttributeIds& ids )
{
  bool returnvalue = true;

  if ( !connectRW() )
    return false;

  try
  {
    connectionRW->PQexecNR( "BEGIN" );

    for ( QgsAttributeIds::const_iterator iter = ids.begin(); iter != ids.end(); ++iter )
    {
      QgsFieldMap::const_iterator field_it = attributeFields.find( *iter );
      if ( field_it == attributeFields.constEnd() )
        continue;

      QString column = field_it->name();
      QString sql = QString( "ALTER TABLE %1 DROP COLUMN %2" )
                    .arg( mSchemaTableName )
                    .arg( quotedIdentifier( column ) );

      //send sql statement and do error handling
      PGresult *result = connectionRW->PQexec( sql );
      if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
        throw PGException( result );
      PQclear( result );

      //delete the attribute from attributeFields
      attributeFields.remove( *iter );
    }

    connectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    e.showErrorMessage( tr( "Error while deleting attributes" ) );
    connectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }

  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  bool returnvalue = true;

  if ( !connectRW() )
    return false;

  try
  {
    connectionRW->PQexecNR( "BEGIN" );

    // cycle through the features
    for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
    {
      int fid = iter.key();

      // skip added features
      if ( fid < 0 )
        continue;

      QString sql = QString( "UPDATE %1 SET " ).arg( mSchemaTableName );
      bool first = true;

      const QgsAttributeMap& attrs = iter.value();

      // cycle through the changed attributes of the feature
      for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          if ( !first )
            sql += ",";
          else
            first = false;

          sql += QString( fld.typeName() != "geometry" ? "%1=%2" : "%1=geomfromewkt(%2)" )
                 .arg( quotedIdentifier( fld.name() ) )
                 .arg( quotedValue( siter->toString() ) );
        }
        catch ( PGFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      sql += QString( " WHERE %1" ).arg( whereClause( fid ) );

      PGresult *result = connectionRW->PQexec( sql );
      if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
        throw PGException( result );
      PQclear( result );
    }

    connectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    e.showErrorMessage( tr( "Error while changing attributes" ) );
    connectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }

  rewind();

  return returnvalue;
}

void QgsPostgresProvider::appendGeomString( QgsGeometry *geom, QString &geomString ) const
{
  unsigned char *buf = geom->asWkb();
  for ( uint i = 0; i < geom->wkbSize(); ++i )
  {
    if ( connectionRW->useWkbHex() )
      geomString += QString( "%1" ).arg(( int ) buf[i], 2, 16, QChar( '0' ) );
    else
      geomString += QString( "\\%1" ).arg(( int ) buf[i], 3, 8, QChar( '0' ) );
  }
}

bool QgsPostgresProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  QgsDebugMsg( "entering." );

  if ( !connectRW() )
    return false;

  bool returnvalue = true;

  try
  {
    // Start the PostGIS transaction
    connectionRW->PQexecNR( "BEGIN" );

    QString update = QString( "UPDATE %1 SET %2=GeomFromWKB($1%3,%4) WHERE %5=$2" )
                     .arg( mSchemaTableName )
                     .arg( quotedIdentifier( geometryColumn ) )
                     .arg( connectionRW->useWkbHex() ? "" : "::bytea" )
                     .arg( srid )
                     .arg( quotedIdentifier( primaryKey ) );

    PGresult *stmt = connectionRW->PQprepare( "updatefeatures", update, 2, NULL );
    if ( stmt == 0 || PQresultStatus( stmt ) == PGRES_FATAL_ERROR )
      throw PGException( stmt );
    PQclear( stmt );

    for ( QgsGeometryMap::iterator iter  = geometry_map.begin();
          iter != geometry_map.end();
          ++iter )
    {

      QgsDebugMsg( "iterating over the map of changed geometries..." );

      if ( iter->asWkb() )
      {
        QgsDebugMsg( "iterating over feature id " + QString::number( iter.key() ) );

        QString geomParam;
        appendGeomString( &*iter, geomParam );

        QStringList params;
        params << geomParam;
        if ( primaryKeyType != "tid" )
        {
          params << QString( "%1" ).arg( iter.key() );
        }
        else
        {
          params << QString( "(%1,%2)" ).arg( iter.key() >> 16 ).arg( iter.key() & 0xffff );
        }

        PGresult *result = connectionRW->PQexecPrepared( "updatefeatures", params );
        if ( result == 0 || PQresultStatus( result ) == PGRES_FATAL_ERROR )
          throw PGException( result );
        PQclear( result );
      } // if (*iter)

    } // for each feature

    connectionRW->PQexecNR( "DEALLOCATE updatefeatures" );
    connectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    e.showErrorMessage( tr( "Error while changing geometry values" ) );
    connectionRW->PQexecNR( "ROLLBACK" );
    connectionRW->PQexecNR( "DEALLOCATE updatefeatures" );
    returnvalue = false;
  }

  rewind();

  QgsDebugMsg( "exiting." );

  return returnvalue;
}

QgsAttributeList QgsPostgresProvider::attributeIndexes()
{
  QgsAttributeList attributes;
  for ( QgsFieldMap::const_iterator it = attributeFields.constBegin(); it != attributeFields.constEnd(); ++it )
  {
    attributes.push_back( it.key() );
  }
  return attributes;
}


int QgsPostgresProvider::capabilities() const
{
  return enabledCapabilities;
}

bool QgsPostgresProvider::setSubsetString( QString theSQL )
{
  QString prevWhere = sqlWhereClause;

  sqlWhereClause = theSQL;

  if ( !uniqueData( mSchemaName, mTableName, primaryKey ) )
  {
    sqlWhereClause = prevWhere;
    return false;
  }

  // Update datasource uri too
  mUri.setSql( theSQL );
  // Update yet another copy of the uri. Why are there 3 copies of the
  // uri? Perhaps this needs some rationalisation.....
  setDataSourceUri( mUri.uri() );

  // need to recalculate the number of features...
  getFeatureCount();
  calculateExtents();

  return true;
}

long QgsPostgresProvider::getFeatureCount()
{
  // get total number of features

  // First get an approximate count; then delegate to
  // a thread the task of getting the full count.

#ifdef POSTGRESQL_THREADS
  QString sql = QString( "select reltuples from pg_catalog.pg_class where relname=%1" ).arg( quotedValue( tableName ) );
  QgsDebugMsg( "Running SQL: " + sql );
#else
  QString sql = QString( "select count(*) from %1" ).arg( mSchemaTableName );

  if ( sqlWhereClause.length() > 0 )
  {
    sql += " where " + sqlWhereClause;
  }
#endif

  Result result = connectionRO->PQexec( sql );

  QgsDebugMsg( "Approximate Number of features as text: " +
               QString::fromUtf8( PQgetvalue( result, 0, 0 ) ) );

  featuresCounted = QString::fromUtf8( PQgetvalue( result, 0, 0 ) ).toLong();

  QgsDebugMsg( "Approximate Number of features: " + QString::number( featuresCounted ) );

  return featuresCounted;
}

// TODO: use the estimateExtents procedure of PostGIS and PostgreSQL 8
// This tip thanks to #qgis irc nick "creeping"
void QgsPostgresProvider::calculateExtents()
{
#ifdef POSTGRESQL_THREADS
  // get the approximate extent by retrieving the bounding box
  // of the first few items with a geometry

  QString sql = QString( "select box3d(%1) from %2 where " )
                .arg( quotedIdentifier( geometryColumn ) )
                .arg( mSchemaTableName );

  if ( sqlWhereClause.length() > 0 )
  {
    sql += QString( "(%1) and " ).arg( sqlWhereClause );
  }

  sql += QString( "not IsEmpty(%1) limit 5" ).arg( quotedIdentifier( geometryColumn ) );

#if WASTE_TIME
  sql = QString( "select "
                 "xmax(extent(%1)) as xmax,"
                 "xmin(extent(%1)) as xmin,"
                 "ymax(extent(%1)) as ymax,"
                 "ymin(extent(%1)) as ymin"
                 " from %2" ).arg( quotedIdentifier( geometryColumn ) ).arg( mSchemaTableName );
#endif

  QgsDebugMsg( "Getting approximate extent using: '" + sql + "'" );

  Result result = connectionRO->PQexec( sql );

  // TODO: Guard against the result having no rows
  for ( int i = 0; i < PQntuples( result ); i++ )
  {
    QString box3d = PQgetvalue( result, i, 0 );

    if ( 0 == i )
    {
      // create the initial extent
      layerExtent = QgsPostGisBox3d( box3d );
    }
    else
    {
      // extend the initial extent
      QgsPostGisBox3d b = QgsPostGisBox3d( box3d );
      layerExtent.combineExtentWith( &b );
    }

    QgsDebugMsg( QString( "After row %1, extent is %2" ).arg( i ).arg( layerExtent.toString() ) );
  }

#else // non-postgresql threads version
  QString sql;
  Result result;
  QString ext;

  // get the extents
  if ( sqlWhereClause.isEmpty() )
  {
    result = connectionRO->PQexec( QString( "select estimated_extent(%1,%2,%3)" )
                                   .arg( quotedValue( mSchemaName ) )
                                   .arg( quotedValue( mTableName ) )
                                   .arg( quotedValue( geometryColumn ) ) );
    if ( PQntuples( result ) == 1 )
      ext = PQgetvalue( result, 0, 0 );
  }

  if ( ext.isEmpty() )
  {
    sql = QString( "select extent(%1) from %2" )
          .arg( quotedIdentifier( geometryColumn ) )
          .arg( mSchemaTableName );

    if ( !sqlWhereClause.isEmpty() )
      sql += QString( "where %1" ).arg( sqlWhereClause );

    result = connectionRO->PQexec( sql );
    if ( PQntuples( result ) == 1 )
      ext = PQgetvalue( result, 0, 0 );
  }

#if WASTE_TIME
  sql = QString( "select "
                 "xmax(extent(%1)) as xmax,"
                 "xmin(extent(%1)) as xmin,"
                 "ymax(extent(%1)) as ymax,"
                 "ymin(extent(%1)) as ymin"
                 " from %2" ).arg( quotedIdentifier( geometryColumn ) ).arg( mSchemaTableName );
#endif

  QgsDebugMsg( "Getting extents using schema.table: " + sql );

  QRegExp rx( "\\((.+) (.+),(.+) (.+)\\)" );
  if ( ext.contains( rx ) )
  {
    QStringList ex = rx.capturedTexts();

    layerExtent.setXMinimum( ex[1].toDouble() );
    layerExtent.setYMinimum( ex[2].toDouble() );
    layerExtent.setXMaximum( ex[3].toDouble() );
    layerExtent.setYMaximum( ex[4].toDouble() );
  }
  else
  {
    QgsDebugMsg( "extents query failed" );
  }
#endif

#if 0
#ifdef QGISDEBUG
  QString xMsg;
  QTextOStream( &xMsg ).precision( 18 );
  QTextOStream( &xMsg ).width( 18 );
  QTextOStream( &xMsg ) << "QgsPostgresProvider: Set extents to: "
  << layerExtent.xMinimum() << ", "
  << layerExtent.yMinimum() << " "
  << layerExtent.xMaximum() << ", "
  << layerExtent.yMaximum();
  QgsDebugMsg( xMsg );
#endif
#endif
  QgsDebugMsg( "Set extents to: " + layerExtent.toString() );
}

/**
 * Event sink for events from threads
 */
void QgsPostgresProvider::customEvent( QEvent * e )
{
  QgsDebugMsg( "received a custom event " + QString::number( e->type() ) );

  switch (( int ) e->type() )
  {
    case QGis::ProviderExtentCalcEvent:

      QgsDebugMsg( "extent has been calculated" );

      // Collect the new extent from the event and set this layer's
      // extent with it.

      {
        QgsRectangle* r = (( QgsProviderExtentCalcEvent* ) e )->layerExtent();
        setExtent( *r );
      }

      QgsDebugMsg( "new extent has been saved" );

      QgsDebugMsg( "Set extent to: " + layerExtent.toString() );

      QgsDebugMsg( "emitting fullExtentCalculated()" );

      emit fullExtentCalculated();

      // TODO: Only uncomment this when the overview map canvas has been subclassed
      // from the QgsMapCanvas

      //        QgsDebugMsg("emitting repaintRequested()");
      //        emit repaintRequested();

      break;

    case QGis::ProviderCountCalcEvent:

      QgsDebugMsg( "count has been calculated" );

      featuresCounted = (( QgsProviderCountCalcEvent* ) e )->featuresCounted();

      QgsDebugMsg( "count is " + QString::number( featuresCounted ) );

      break;

    default:
      // do nothing
      break;
  }

  QgsDebugMsg( "Finished processing custom event " + QString::number( e->type() ) );

}


bool QgsPostgresProvider::deduceEndian()
{
  // need to store the PostgreSQL endian format used in binary cursors
  // since it appears that starting with
  // version 7.4, binary cursors return data in XDR whereas previous versions
  // return data in the endian of the server

  QString firstOid = QString( "select regclass(%1)::oid" ).arg( quotedValue( mSchemaTableName ) );
  Result oidResult = connectionRO->PQexec( firstOid );
  // get the int value from a "normal" select
  QString oidValue = QString::fromUtf8( PQgetvalue( oidResult, 0, 0 ) );

  QgsDebugMsg( "Creating binary cursor" );

  // get the same value using a binary cursor
  connectionRO->openCursor( "oidcursor", QString( "select regclass(%1)::oid" ).arg( quotedValue( mSchemaTableName ) ) );

  QgsDebugMsg( "Fetching a record and attempting to get check endian-ness" );

  Result fResult = connectionRO->PQexec( "fetch forward 1 from oidcursor" );
  swapEndian = true;
  if ( PQntuples( fResult ) > 0 )
  {
    // get the oid value from the binary cursor
    int oid = *( int * )PQgetvalue( fResult, 0, 0 );

    //--std::cout << "Got oid of " << oid << " from the binary cursor" << std::endl;

    //--std::cout << "First oid is " << oidValue << std::endl;

    // compare the two oid values to determine if we need to do an endian swap
    if ( oid == oidValue.toInt() )
      swapEndian = false;
  }
  connectionRO->closeCursor( "oidcursor" );
  return swapEndian;
}

bool QgsPostgresProvider::getGeometryDetails()
{
  QString fType( "" );
  srid = "";
  valid = false;
  QStringList log;

  QString sql = QString( "select type,srid from geometry_columns"
                         " where f_table_name=%1 and f_geometry_column=%2 and f_table_schema=%3" )
                .arg( quotedValue( mTableName ) )
                .arg( quotedValue( geometryColumn ) )
                .arg( quotedValue( mSchemaName ) );

  QgsDebugMsg( "Getting geometry column: " + sql );

  Result result = connectionRO->PQexec( sql );

  QgsDebugMsg( "geometry column query returned " + QString::number( PQntuples( result ) ) );

  if ( PQntuples( result ) > 0 )
  {
    fType = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );
    srid = QString::fromUtf8( PQgetvalue( result, 0, 1 ) );
  }
  else
  {
    // Didn't find what we need in the geometry_columns table, so
    // get stuff from the relevant column instead. This may (will?)
    // fail if there is no data in the relevant table.
    sql = QString( "select srid(%1),geometrytype(%1) from %2" )
          .arg( quotedIdentifier( geometryColumn ) )
          .arg( mSchemaTableName );

    //it is possible that the where clause restricts the feature type
    if ( !sqlWhereClause.isEmpty() )
    {
      sql += " WHERE " + sqlWhereClause;
    }

    sql += " limit 1";

    result = connectionRO->PQexec( sql );

    if ( PQntuples( result ) > 0 )
    {
      srid = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );
      fType = QString::fromUtf8( PQgetvalue( result, 0, 1 ) );
    }
  }

  if ( !srid.isEmpty() && !fType.isEmpty() )
  {
    valid = true;
    if ( fType == "GEOMETRY" )
    {
      // check to see if there is a unique geometry type
      sql = QString( "select distinct "
                     "case"
                     " when geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
                     " when geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
                     " when geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
                     " end "
                     "from %2" ).arg( quotedIdentifier( geometryColumn ) ).arg( mSchemaTableName );
      if ( mUri.sql() != "" )
        sql += " where " + mUri.sql();

      result = connectionRO->PQexec( sql );

      if ( PQntuples( result ) == 1 )
      {
        fType = QString::fromUtf8( PQgetvalue( result, 0, 0 ) );
      }
    }
    if ( fType == "POINT" || fType == "POINTM" )
    {
      geomType = QGis::WKBPoint;
    }
    else if ( fType == "MULTIPOINT" || fType == "MULTIPOINTM" )
    {
      geomType = QGis::WKBMultiPoint;
    }
    else if ( fType == "LINESTRING" || fType == "LINESTRINGM" )
    {
      geomType = QGis::WKBLineString;
    }
    else if ( fType == "MULTILINESTRING" || fType == "MULTILINESTRINGM" )
    {
      geomType = QGis::WKBMultiLineString;
    }
    else if ( fType == "POLYGON" || fType == "POLYGONM" )
    {
      geomType = QGis::WKBPolygon;
    }
    else if ( fType == "MULTIPOLYGON" || fType == "MULTIPOLYGONM" )
    {
      geomType = QGis::WKBMultiPolygon;
    }
    else
    {
      showMessageBox( tr( "Unknown geometry type" ),
                      tr( "Column %1 in %2 has a geometry type of %3, which Qgis does not currently support." )
                      .arg( geometryColumn ).arg( mSchemaTableName ).arg( fType ) );
      valid = false;
    }
  }
  else // something went wrong...
  {
    log.prepend( tr( "Qgis was unable to determine the type and srid of column %1 in %2. The database communication log was:\n%3" )
                 .arg( geometryColumn )
                 .arg( mSchemaTableName )
                 .arg( QString::fromUtf8( PQresultErrorMessage( result ) ) ) );
    showMessageBox( tr( "Unable to get feature type and srid" ), log );
  }

  // store whether the geometry includes measure value
  if ( fType == "POINTM" || fType == "MULTIPOINTM" ||
       fType == "LINESTRINGM" || fType == "MULTILINESTRINGM" ||
       fType == "POLYGONM"  || fType == "MULTIPOLYGONM" )
  {
    // explicitly disable adding new features and editing of geometries
    // as this would lead to corruption of measures
    enabledCapabilities &= ~( QgsVectorDataProvider::ChangeGeometries | QgsVectorDataProvider::AddFeatures );
  }


  if ( valid )
  {
    QgsDebugMsg( "SRID is " + srid );
    QgsDebugMsg( "type is " + fType );
    QgsDebugMsg( "Feature type is " + QString::number( geomType ) );
    QgsDebugMsg( "Feature type name is " + QString( QGis::qgisFeatureTypes[geomType] ) );
  }
  else
  {
    QgsDebugMsg( "Failed to get geometry details for Postgres layer." );
  }

  return valid;
}

QString QgsPostgresProvider::quotedIdentifier( QString ident ) const
{
  ident.replace( '"', "\"\"" );
  return ident.prepend( "\"" ).append( "\"" );
}

QString QgsPostgresProvider::quotedValue( QString value ) const
{
  if ( value.isNull() )
    return "NULL";

  // FIXME: use PQescapeStringConn
  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
}

PGresult *QgsPostgresProvider::Conn::PQexec( QString query )
{
  QgsDebugMsgLevel( QString( "Executing SQL: %1" ).arg( query ), 3 );
  PGresult *res = ::PQexec( conn, query.toUtf8() );

#ifdef QGISDEBUG
  if ( res )
  {
    int errorStatus = PQresultStatus( res );
    if ( errorStatus != PGRES_COMMAND_OK && errorStatus != PGRES_TUPLES_OK )
    {
      QString err = QString( "Errornous query: %1 returned %2 [%3]" )
                    .arg( query )
                    .arg( errorStatus )
                    .arg( PQresultErrorMessage( res ) );
      QgsDebugMsgLevel( err, 3 );
    }
  }
#endif

  return res;
}

bool QgsPostgresProvider::Conn::openCursor( QString cursorName, QString sql )
{
  if ( openCursors++ == 0 )
  {
    QgsDebugMsg( "Starting read-only transaction" );
    PQexecNR( "BEGIN READ ONLY" );
  }
  QgsDebugMsgLevel( QString( "Binary cursor %1 for %2" ).arg( cursorName ).arg( sql ), 3 );
  return PQexecNR( QString( "declare %1 binary cursor for %2" ).arg( cursorName ).arg( sql ) );
}

bool QgsPostgresProvider::Conn::closeCursor( QString cursorName )
{
  bool res = PQexecNR( QString( "CLOSE %1" ).arg( cursorName ) );

  if ( --openCursors == 0 )
  {
    QgsDebugMsg( "Committing read-only transaction" );
    PQexecNR( "COMMIT" );
  }

  return res;
}

bool QgsPostgresProvider::Conn::PQexecNR( QString query )
{
  Result res = ::PQexec( conn, query.toUtf8() );
  if ( res )
  {
    int errorStatus = PQresultStatus( res );
    if ( errorStatus != PGRES_COMMAND_OK )
    {
#ifdef QGISDEBUG
      QString err = QString( "Query: %1 returned %2 [%3]" )
                    .arg( query )
                    .arg( errorStatus )
                    .arg( PQresultErrorMessage( res ) );
      QgsDebugMsgLevel( err, 3 );
#endif
      if ( openCursors )
      {
        PQexecNR( "ROLLBACK" );
        QgsDebugMsg( QString( "Re-starting read-only transaction after errornous statement - state of %1 cursors lost" ).arg( openCursors ) );
        PQexecNR( "BEGIN READ ONLY" );
      }
    }
    return errorStatus == PGRES_COMMAND_OK;
  }
  else
  {
    QgsDebugMsgLevel( QString( "Query: %1 returned no result buffer" ).arg( query ), 3 );
  }
  return false;
}

PGresult *QgsPostgresProvider::Conn::PQgetResult()
{
  return ::PQgetResult( conn );
}

PGresult *QgsPostgresProvider::Conn::PQprepare( QString stmtName, QString query, int nParams, const Oid *paramTypes )
{
  return ::PQprepare( conn, stmtName.toUtf8(), query.toUtf8(), nParams, paramTypes );
}

PGresult *QgsPostgresProvider::Conn::PQexecPrepared( QString stmtName, const QStringList &params )
{
  const char **param = new const char *[ params.size()];
  QList<QByteArray> qparam;

  for ( int i = 0; i < params.size(); i++ )
  {
    qparam << params[i].toUtf8();

    if ( params[i].isNull() )
      param[i] = 0;
    else
      param[i] = qparam[i];
  }

  PGresult *res = ::PQexecPrepared( conn, stmtName.toUtf8(), params.size(), param, NULL, NULL, 0 );

  delete [] param;

  return res;
}

void QgsPostgresProvider::Conn::PQfinish()
{
  ::PQfinish( conn );
}

int QgsPostgresProvider::Conn::PQsendQuery( QString query )
{
  return ::PQsendQuery( conn, query.toUtf8() );
}

void QgsPostgresProvider::showMessageBox( const QString& title, const QString& text )
{
  QgsMessageOutput* message = QgsMessageOutput::createMessageOutput();
  message->setTitle( title );
  message->setMessage( text, QgsMessageOutput::MessageText );
  message->showMessage();
}

void QgsPostgresProvider::showMessageBox( const QString& title, const QStringList& text )
{
  showMessageBox( title, text.join( "\n" ) );
}


QgsCoordinateReferenceSystem QgsPostgresProvider::crs()
{
  QgsCoordinateReferenceSystem srs;
  srs.createFromSrid( srid.toInt() );
  return srs;
}

QString QgsPostgresProvider::subsetString()
{
  return sqlWhereClause;
}

PGconn * QgsPostgresProvider::pgConnection()
{
  connectRW();
  return connectionRW->pgConnection();
}

QString QgsPostgresProvider::getTableName()
{
  return mTableName;
}


size_t QgsPostgresProvider::layerCount() const
{
  return 1;                   // XXX need to return actual number of layers
} // QgsPostgresProvider::layerCount()



QString  QgsPostgresProvider::name() const
{
  return POSTGRES_KEY;
} //  QgsPostgresProvider::name()



QString  QgsPostgresProvider::description() const
{
  return POSTGRES_DESCRIPTION;
} //  QgsPostgresProvider::description()

/**
 * Class factory to return a pointer to a newly created
 * QgsPostgresProvider object
 */
QGISEXTERN QgsPostgresProvider * classFactory( const QString *uri )
{
  return new QgsPostgresProvider( *uri );
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return  POSTGRES_KEY;
}
/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return POSTGRES_DESCRIPTION;
}
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
