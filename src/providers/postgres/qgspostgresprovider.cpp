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

#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgsmessageoutput.h>
#include <qgsmessagelog.h>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>

#include "qgsvectorlayerimport.h"
#include "qgsprovidercountcalcevent.h"
#include "qgsproviderextentcalcevent.h"
#include "qgspostgresprovider.h"
#include "qgspostgresconn.h"
#include "qgspgsourceselect.h"
#include "qgspostgresdataitems.h"
#include "qgslogger.h"

const QString POSTGRES_KEY = "postgres";
const QString POSTGRES_DESCRIPTION = "PostgreSQL/PostGIS data provider";

int QgsPostgresProvider::sProviderIds = 0;
const int QgsPostgresProvider::sFeatureQueueSize = 2000;

QgsPostgresProvider::QgsPostgresProvider( QString const & uri )
    : QgsVectorDataProvider( uri )
    , mFetching( false )
    , mValid( false )
    , mPrimaryKeyType( pktUnknown )
    , mDetectedGeomType( QGis::WKBUnknown )
    , mRequestedGeomType( QGis::WKBUnknown )
    , mFeatureQueueSize( sFeatureQueueSize )
    , mUseEstimatedMetadata( false )
    , mSelectAtIdDisabled( false )
    , mConnectionRO( 0 )
    , mConnectionRW( 0 )
    , mFidCounter( 0 )
{
  mProviderId = sProviderIds++;

  QgsDebugMsg( QString( "URI: %1 " ).arg( uri ) );

  mUri = QgsDataSourceURI( uri );

  // populate members from the uri structure
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  mGeometryColumn = mUri.geometryColumn();
  mSqlWhereClause = mUri.sql();
  mRequestedSrid = mUri.srid();
  mRequestedGeomType = mUri.wkbType();
  mIsGeography = false;

  if ( mSchemaName.isEmpty() &&
       mTableName.startsWith( "(SELECT", Qt::CaseInsensitive ) &&
       mTableName.endsWith( ")" ) )
  {
    mIsQuery = true;
    mQuery = mTableName;
    mTableName = "";
  }
  else
  {
    mIsQuery = false;

    if ( !mSchemaName.isEmpty() )
    {
      mQuery += quotedIdentifier( mSchemaName ) + ".";
    }

    if ( !mTableName.isEmpty() )
    {
      mQuery += quotedIdentifier( mTableName );
    }
  }

  mUseEstimatedMetadata = mUri.useEstimatedMetadata();
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();

  QgsDebugMsg( QString( "Connection info is %1" ).arg( mUri.connectionInfo() ) );
  QgsDebugMsg( QString( "Geometry column is: %1" ).arg( mGeometryColumn ) );
  QgsDebugMsg( QString( "Schema is: %1" ).arg( mSchemaName ) );
  QgsDebugMsg( QString( "Table name is: %1" ).arg( mTableName ) );
  QgsDebugMsg( QString( "Query is: %1" ).arg( mQuery ) );
  QgsDebugMsg( QString( "Where clause is: %1" ).arg( mSqlWhereClause ) );

  // no table/query passed, the provider could be used to get tables
  if ( mQuery.isEmpty() )
  {
    return;
  }

  mConnectionRO = QgsPostgresConn::connectDb( mUri.connectionInfo(), true );
  if ( !mConnectionRO )
  {
    return;
  }

  if ( !hasSufficientPermsAndCapabilities() ) // check permissions and set capabilities
  {
    disconnectDb();
    return;
  }

  if ( !getGeometryDetails() ) // gets srid and geometry type
  {
    // the table is not a geometry table
    QgsMessageLog::logMessage( tr( "invalid PostgreSQL layer" ), tr( "PostGIS" ) );
    disconnectDb();
    return;
  }

  mLayerExtent.setMinimal();
  mFeaturesCounted = -1;

  // set the primary key
  if ( !determinePrimaryKey() )
  {
    mValid = false;
    disconnectDb();
    return;
  }

  // Set the postgresql message level so that we don't get the
  // 'there is no transaction in progress' warning.
#ifndef QGISDEBUG
  mConnectionRO->PQexecNR( "set client_min_messages to error" );
#endif

  //fill type names into sets
  mNativeTypes
  // integer types
  << QgsVectorDataProvider::NativeType( tr( "Whole number (smallint - 16bit)" ), "int2", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 32bit)" ), "int4", QVariant::Int )
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 64bit)" ), "int8", QVariant::LongLong )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), "numeric", QVariant::Double, 1, 20, 0, 20 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), "decimal", QVariant::Double, 1, 20, 0, 20 )

  // floating point
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "real", QVariant::Double )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), "double precision", QVariant::Double )

  // string types
  << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "char", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), "varchar", QVariant::String, 1, 255 )
  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), "text", QVariant::String )
  ;

  QString key;
  switch ( mPrimaryKeyType )
  {
    case pktOid:
      key = "oid";
      break;
    case pktTid:
      key = "tid";
      break;
    case pktInt:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      Q_ASSERT( mAttributeFields.contains( mPrimaryKeyAttrs[0] ) );
      key = mAttributeFields[ mPrimaryKeyAttrs[0] ].name();
      break;
    case pktFidMap:
    {
      QString delim;
      foreach( int idx, mPrimaryKeyAttrs )
      {
        key += delim + mAttributeFields[ idx ].name();
        delim = ",";
      }
    }
    break;
    case pktUnknown:
      mValid = false;
      break;
  }

  if ( mValid )
  {
    mUri.setKeyColumn( key );
    setDataSourceUri( mUri.uri() );
  }
  else
  {
    disconnectDb();
  }
}

QgsPostgresProvider::~QgsPostgresProvider()
{
  disconnectDb();

  QgsDebugMsg( "deconstructing." );
}

void QgsPostgresProvider::disconnectDb()
{
  if ( mFetching )
  {
    mConnectionRO->closeCursor( QString( "qgisf%1" ).arg( mProviderId ) );
    mFetching = false;
  }

  if ( mConnectionRO )
  {
    mConnectionRO->disconnect();
    mConnectionRO = 0;
  }

  if ( mConnectionRW )
  {
    mConnectionRW->disconnect();
    mConnectionRW = 0;
  }
}

QString QgsPostgresProvider::storageType() const
{
  return "PostgreSQL database with PostGIS extension";
}

bool QgsPostgresProvider::declareCursor(
  const QString &cursorName,
  const QgsAttributeList &fetchAttributes,
  bool fetchGeometry,
  QString whereClause )
{
  if ( fetchGeometry && mGeometryColumn.isNull() )
  {
    return false;
  }

  try
  {
    QString query = "SELECT ", delim = "";

    if ( fetchGeometry )
    {
      query += QString( "%1(%2(%3),'%4')" )
               .arg( mConnectionRO->majorVersion() < 2 ? "asbinary" : "st_asbinary" )
               .arg( mConnectionRO->majorVersion() < 2 ? "force_2d" : "st_force_2d" )
               .arg( quotedIdentifier( mGeometryColumn, mIsGeography ) )
               .arg( endianString() );
      delim = ",";
    }

    switch ( mPrimaryKeyType )
    {
      case pktOid:
        query += delim + "oid";
        delim = ",";
        break;

      case pktTid:
        query += delim + "ctid";
        delim = ",";
        break;

      case pktInt:
        query += delim + quotedIdentifier( field( mPrimaryKeyAttrs[0] ).name() );
        delim = ",";
        break;

      case pktFidMap:
        foreach( int idx, mPrimaryKeyAttrs )
        {
          query += delim + mConnectionRO->fieldExpression( field( idx ) );
          delim = ",";
        }
        break;

      case pktUnknown:
        QgsDebugMsg( "Cannot declare cursor without primary key." );
        return false;
        break;
    }

    foreach( int idx, fetchAttributes )
    {
      if ( mPrimaryKeyAttrs.contains( idx ) )
        continue;

      query += delim + mConnectionRO->fieldExpression( field( idx ) );
    }

    query += " FROM " + mQuery;

    if ( !whereClause.isEmpty() )
      query += QString( " WHERE %1" ).arg( whereClause );

    if ( !mConnectionRO->openCursor( cursorName, query ) )
    {
      // reloading the fields might help next time around
      rewind();
      return false;
    }
  }
  catch ( PGFieldNotFound )
  {
    rewind();
    return false;
  }

  return true;
}

static bool operator<( const QVariant &a, const QVariant &b )
{
  if ( a.isNull() || b.isNull() )
    return false;

  if ( a.type() == b.type() )
  {
    switch ( a.type() )
    {
      case QVariant::Int:
      case QVariant::Char:
        return a.toInt() < b.toInt();

      case QVariant::Double:
        return a.toDouble() < b.toDouble();

      case QVariant::LongLong:
        return a.toLongLong() < b.toLongLong();

      case QVariant::List:
      {
        QList<QVariant> al = a.toList();
        QList<QVariant> bl = b.toList();

        int i, n = qMin( al.size(), bl.size() );
        for ( i = 0; i < n && al[i] == bl[i]; i++ )
          ;

        if ( i == n )
          return al.size() < bl.size();
        else
          return al[i] < bl[i];
      }
      break;

      case QVariant::StringList:
      {
        QStringList al = a.toStringList();
        QStringList bl = b.toStringList();

        int i, n = qMin( al.size(), bl.size() );
        for ( i = 0; i < n && al[i] == bl[i]; i++ )
          ;

        if ( i == n )
          return al.size() < bl.size();
        else
          return al[i] < bl[i];
      }
      break;

      case QVariant::Date:
        return a.toDate() < b.toDate();

      case QVariant::Time:
        return a.toTime() < b.toTime();

      case QVariant::DateTime:
        return a.toDateTime() < b.toDateTime();

      case QVariant::Bool:
        return a.toBool() < b.toBool();

      case QVariant::UInt:
        return a.toUInt() < b.toUInt();

      case QVariant::ULongLong:
        return a.toULongLong() < b.toULongLong();

      default:
        break;
    }
  }

  return a.canConvert( QVariant::String ) && b.canConvert( QVariant::String ) && a.toString() < b.toString();
}

QgsFeatureId QgsPostgresProvider::lookupFid( const QVariant &v )
{
  QMap<QVariant, QgsFeatureId>::const_iterator it = mKeyToFid.find( v );

  if ( it != mKeyToFid.constEnd() )
  {
    return it.value();
  }

  mFidToKey.insert( ++mFidCounter, v );
  mKeyToFid.insert( v, mFidCounter );

  return mFidCounter;
}

bool QgsPostgresProvider::getFeature( QgsPostgresResult &queryResult, int row, bool fetchGeometry,
                                      QgsFeature &feature,
                                      const QgsAttributeList &fetchAttributes )
{
  try
  {
    feature.clearAttributeMap();

    int col = 0;

    if ( fetchGeometry )
    {
      int returnedLength = ::PQgetlength( queryResult.result(), row, col );
      if ( returnedLength > 0 )
      {
        unsigned char *featureGeom = new unsigned char[returnedLength + 1];
        memset( featureGeom, 0, returnedLength + 1 );
        memcpy( featureGeom, PQgetvalue( queryResult.result(), row, col ), returnedLength );
        feature.setGeometryAndOwnership( featureGeom, returnedLength + 1 );
      }
      else
      {
        feature.setGeometryAndOwnership( 0, 0 );
        QgsMessageLog::logMessage( tr( "Couldn't get the feature geometry in binary form" ), tr( "PostGIS" ) );
      }

      col++;
    }

    QgsFeatureId fid = 0;

    switch ( mPrimaryKeyType )
    {
      case pktOid:
      case pktTid:
      case pktInt:
        fid = mConnectionRO->getBinaryInt( queryResult, row, col++ );
        if ( mPrimaryKeyType == pktInt && fetchAttributes.contains( mPrimaryKeyAttrs[0] ) )
          feature.addAttribute( mPrimaryKeyAttrs[0], fid );
        break;

      case pktFidMap:
      {
        QList<QVariant> primaryKeyVals;

        foreach( int idx, mPrimaryKeyAttrs )
        {
          const QgsField &fld = field( idx );

          QVariant v = convertValue( fld.type(), queryResult.PQgetvalue( row, col ) );
          primaryKeyVals << v;

          if ( fetchAttributes.contains( idx ) )
            feature.addAttribute( idx, v );

          col++;
        }

        fid = lookupFid( QVariant( primaryKeyVals ) );
      }
      break;

      case pktUnknown:
        Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
        return false;
    }

    feature.setFeatureId( fid );
    QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 4 );

    // iterate attributes
    foreach( int idx, fetchAttributes )
    {
      if ( mPrimaryKeyAttrs.contains( idx ) )
        continue;

      const QgsField &fld = field( idx );

      QVariant v = convertValue( fld.type(), queryResult.PQgetvalue( row, col ) );
      feature.addAttribute( idx, v );

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
  QString cursorName = QString( "qgisf%1" ).arg( mProviderId );

  if ( mFetching )
  {
    mConnectionRO->closeCursor( cursorName );
    mFetching = false;

    while ( !mFeatureQueue.empty() )
    {
      mFeatureQueue.dequeue();
    }
  }

  QString whereClause;

  if ( !rect.isEmpty() && !mGeometryColumn.isNull() )
  {
    if ( mIsGeography )
    {
      rect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 ).intersect( &rect );
      if ( !rect.isFinite() )
        whereClause = "false";
    }

    if ( whereClause.isEmpty() )
    {
      QString qBox;
      if ( mConnectionRO->majorVersion() < 2 )
      {
        qBox = QString( "setsrid('BOX3D(%1)'::box3d,%2)" )
               .arg( rect.asWktCoordinates() )
               .arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
      }
      else
      {
        qBox = QString( "st_makeenvelope(%1,%2,%3,%4,%5)" )
               .arg( rect.xMinimum(), 0, 'f', 16 )
               .arg( rect.yMinimum(), 0, 'f', 16 )
               .arg( rect.xMaximum(), 0, 'f', 16 )
               .arg( rect.yMaximum(), 0, 'f', 16 )
               .arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
      }

      whereClause = QString( "%1 && %2" )
                    .arg( quotedIdentifier( mGeometryColumn ) )
                    .arg( qBox );
      if ( useIntersect )
      {
        whereClause += QString( " AND %1(%2,%3)" )
                       .arg( mConnectionRO->majorVersion() < 2 ? "intersects" : "st_intersects" )
                       .arg( quotedIdentifier( mGeometryColumn, mIsGeography ) )
                       .arg( qBox );
      }
    }

    if ( !mRequestedSrid.isEmpty() && mRequestedSrid != mDetectedSrid )
    {
      whereClause += QString( " AND %1(%2)=%3" )
                     .arg( mConnectionRO->majorVersion() < 2 ? "srid" : "st_srid" )
                     .arg( quotedIdentifier( mGeometryColumn, mIsGeography ) )
                     .arg( mRequestedSrid );
    }

    if ( mRequestedGeomType != QGis::WKBUnknown && mRequestedGeomType != mDetectedGeomType )
    {
      whereClause += QString( " AND %1" ).arg( QgsPostgresConn::postgisTypeFilter( mGeometryColumn, mRequestedGeomType, mIsGeography ) );
    }
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + mSqlWhereClause + ")";
  }

  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  if ( !declareCursor( cursorName, fetchAttributes, fetchGeometry, whereClause ) )
    return;

  mFetching = true;
  mFetched = 0;
}

bool QgsPostgresProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );
  if ( !mValid )
  {
    QgsMessageLog::logMessage( tr( "Read attempt on an invalid postgresql data source" ), tr( "PostGIS" ) );
    return false;
  }

  if ( !mFetching )
  {
    QgsMessageLog::logMessage( tr( "nextFeature() without select()" ), tr( "PostGIS" ) );
    return false;
  }

  QString cursorName = QString( "qgisf%1" ).arg( mProviderId );

  if ( mFeatureQueue.empty() )
  {
    QString fetch = QString( "FETCH FORWARD %1 FROM %2" ).arg( mFeatureQueueSize ).arg( cursorName );
    QgsDebugMsgLevel( QString( "fetching %1 features." ).arg( mFeatureQueueSize ), 3 );
    if ( mConnectionRO->PQsendQuery( fetch ) == 0 ) // fetch features asynchronously
    {
      QgsMessageLog::logMessage( tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( cursorName ).arg( mConnectionRO->PQerrorMessage() ), tr( "PostGIS" ) );
    }

    QgsPostgresResult queryResult;
    for ( ;; )
    {
      queryResult = mConnectionRO->PQgetResult();
      if ( !queryResult.result() )
        break;

      if ( queryResult.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage( tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( cursorName ).arg( mConnectionRO->PQerrorMessage() ), tr( "PostGIS" ) );
        break;
      }

      int rows = queryResult.PQntuples();
      if ( rows == 0 )
        continue;

      for ( int row = 0; row < rows; row++ )
      {
        mFeatureQueue.enqueue( QgsFeature() );
        getFeature( queryResult, row, mFetchGeom, mFeatureQueue.back(), mAttributesToFetch );
      } // for each row in queue
    }
  }

  if ( mFeatureQueue.empty() )
  {
    QgsDebugMsg( QString( "Finished after %1 features" ).arg( mFetched ) );
    mConnectionRO->closeCursor( cursorName );
    mFetching = false;
    if ( mFeaturesCounted < mFetched )
    {
      QgsDebugMsg( QString( "feature count adjusted from %1 to %2" ).arg( mFeaturesCounted ).arg( mFetched ) );
      mFeaturesCounted = mFetched;
    }
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

  mFeatureQueue.dequeue();
  mFetched++;

  feature.setValid( true );
  return true;
}

QString QgsPostgresProvider::pkParamWhereClause( int offset ) const
{
  QString whereClause;

  switch ( mPrimaryKeyType )
  {
    case pktTid:
      whereClause = QString( "ctid=$%1" ).arg( offset );
      break;

    case pktOid:
      whereClause = QString( "oid=$%1" ).arg( offset );
      break;

    case pktInt:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      whereClause = QString( "%1=$%2" ).arg( quotedIdentifier( field( mPrimaryKeyAttrs[0] ).name() ) ).arg( offset );
      break;

    case pktFidMap:
    {
      QString delim = "";
      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        int idx = mPrimaryKeyAttrs[i];
        const QgsField &fld = field( idx );

        whereClause += delim + QString( "%1=$%2" ).arg( mConnectionRO->fieldExpression( fld ) ).arg( offset++ );
        delim = " AND ";
      }
    }
    break;

    case pktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = "NULL";
      break;
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + mSqlWhereClause + ")";
  }

  return whereClause;
}

void QgsPostgresProvider::appendPkParams( QgsFeatureId featureId, QStringList &params ) const
{
  switch ( mPrimaryKeyType )
  {
    case pktOid:
    case pktInt:
      params << QString::number( featureId );
      break;

    case pktTid:
      params << QString( "'(%1,%2)'" ).arg( FID_TO_NUMBER( featureId ) >> 16 ).arg( FID_TO_NUMBER( featureId ) & 0xffff );
      break;

    case pktFidMap:
    {
      QList<QVariant> pkVals;
      QMap<QgsFeatureId, QVariant>::const_iterator it = mFidToKey.find( featureId );
      if ( it != mFidToKey.constEnd() )
      {
        pkVals = it.value().toList();
        Q_ASSERT( pkVals.size() == mPrimaryKeyAttrs.size() );
      }

      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        if ( i < pkVals.size() )
        {
          params << pkVals[i].toString();
        }
        else
        {
          QgsDebugMsg( QString( "FAILURE: Key value %1 for feature %2 not found." ).arg( mPrimaryKeyAttrs[i] ).arg( featureId ) );
          params << "NULL";
        }
      }

      QgsDebugMsg( QString( "keys params: %1" ).arg( params.join( "; " ) ) );
    }
    break;

    case pktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      break;
  }
}

QString QgsPostgresProvider::whereClause( QgsFeatureId featureId ) const
{
  QString whereClause;

  switch ( mPrimaryKeyType )
  {
    case pktTid:
      whereClause = QString( "ctid='(%1,%2)'" )
                    .arg( FID_TO_NUMBER( featureId ) >> 16 )
                    .arg( FID_TO_NUMBER( featureId ) & 0xffff );
      break;

    case pktOid:
      whereClause = QString( "oid=%1" ).arg( featureId );
      break;

    case pktInt:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      whereClause = QString( "%1=%2" ).arg( quotedIdentifier( field( mPrimaryKeyAttrs[0] ).name() ) ).arg( featureId );
      break;

    case pktFidMap:
    {
      QMap<QgsFeatureId, QVariant>::const_iterator it = mFidToKey.find( featureId );
      if ( it != mFidToKey.constEnd() )
      {
        QList<QVariant> pkVals = it.value().toList();

        Q_ASSERT( pkVals.size() == mPrimaryKeyAttrs.size() );

        QString delim = "";
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs[i];
          const QgsField &fld = field( idx );

          whereClause += delim + QString( "%1=%2" ).arg( mConnectionRO->fieldExpression( fld ) ).arg( quotedValue( pkVals[i].toString() ) );
          delim = " AND ";
        }
      }
      else
      {
        QgsDebugMsg( QString( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        whereClause = "NULL";
      }
    }
    break;

    case pktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = "NULL";
      break;
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + mSqlWhereClause + ")";
  }

  return whereClause;
}

bool QgsPostgresProvider::featureAtId( QgsFeatureId featureId, QgsFeature& feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
  feature.setValid( false );

#if 0
  if ( mFeatureMap.contains( featureId ) )
  {
    QgsFeature * fpointer = &feature;
    *fpointer = mFeatureMap.value( featureId );
    QgsDebugMsg( QString( "retrieve feature %1 from cache" ).arg( featureId ) );

    mPriorityIds.removeAll( featureId );
    mPriorityIds.prepend( featureId );
    return true;
  }
#endif

  QString cursorName = QString( "qgisfid%1" ).arg( mProviderId );

  if ( !declareCursor( cursorName, fetchAttributes, fetchGeometry, whereClause( featureId ) ) )
    return false;

  QgsPostgresResult queryResult = mConnectionRO->PQexec( QString( "FETCH FORWARD 1 FROM %1" ).arg( cursorName ) );

  int rows = queryResult.PQntuples();
  if ( rows == 0 )
  {
    QgsMessageLog::logMessage( tr( "feature %1 not found" ).arg( featureId ), tr( "PostGIS" ) );
    mConnectionRO->closeCursor( cursorName );
    return false;
  }
  else if ( rows != 1 )
  {
    QgsMessageLog::logMessage( tr( "found %1 features instead of just one." ).arg( rows ), tr( "PostGIS" ) );
  }

  bool gotit = getFeature( queryResult, 0, fetchGeometry, feature, fetchAttributes );

  mConnectionRO->closeCursor( cursorName );

  feature.setValid( gotit );

#if 0
  if ( gotit )
  {
    mFeatureMap.insert( featureId, feature );
    mPriorityIds.prepend( featureId );
    if ( mPriorityIds.count() == 20 )
    {
      mFeatureMap.remove( mPriorityIds.takeLast() );
    }
  }
#endif

  return gotit;
}

void QgsPostgresProvider::setExtent( QgsRectangle& newExtent )
{
  mLayerExtent.setXMaximum( newExtent.xMaximum() );
  mLayerExtent.setXMinimum( newExtent.xMinimum() );
  mLayerExtent.setYMaximum( newExtent.yMaximum() );
  mLayerExtent.setYMinimum( newExtent.yMinimum() );
}

/**
 * Return the feature type
 */
QGis::WkbType QgsPostgresProvider::geometryType() const
{
  return mRequestedGeomType != QGis::WKBUnknown ? mRequestedGeomType : mDetectedGeomType;
}

const QgsField &QgsPostgresProvider::field( int index ) const
{
  QgsFieldMap::const_iterator it = mAttributeFields.find( index );

  if ( it == mAttributeFields.constEnd() )
  {
    QgsLogger::warning( QString( "FAILURE: Field %1 not found." ).arg( index ) );
    throw PGFieldNotFound();
  }

  return it.value();
}

/**
 * Return the number of fields
 */
uint QgsPostgresProvider::fieldCount() const
{
  return mAttributeFields.size();
}

const QgsFieldMap & QgsPostgresProvider::fields() const
{
  return mAttributeFields;
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
    mConnectionRO->PQexecNR( QString( "move 0 in qgisf%1" ).arg( mProviderId ) );
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

bool QgsPostgresProvider::loadFields()
{
  if ( !mIsQuery )
  {
    QgsDebugMsg( QString( "Loading fields for table %1" ).arg( mTableName ) );

    // Get the relation oid for use in later queries
    QString sql = QString( "SELECT regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
    QgsPostgresResult tresult = mConnectionRO->PQexec( sql );
    QString tableoid = tresult.PQgetvalue( 0, 0 );

    // Get the table description
    sql = QString( "SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=0" ).arg( tableoid );
    tresult = mConnectionRO->PQexec( sql );
    if ( tresult.PQntuples() > 0 )
      mDataComment = tresult.PQgetvalue( 0, 0 );
  }

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  QString sql = QString( "SELECT * FROM %1 LIMIT 0" ).arg( mQuery );

  QgsPostgresResult result = mConnectionRO->PQexec( sql );

  QSet<QString> fields;

  // The queries inside this loop could possibly be combined into one
  // single query - this would make the code run faster.
  mAttributeFields.clear();
  for ( int i = 0; i < result.PQnfields(); i++ )
  {
    QString fieldName = result.PQfname( i );
    if ( fieldName == mGeometryColumn )
      continue;

    int fldtyp = result.PQftype( i );
    QString typOid = QString().setNum( fldtyp );
    int fieldPrec = -1;
    QString fieldComment( "" );
    int tableoid = result.PQftable( i );

    sql = QString( "SELECT typname,typtype,typelem,typlen FROM pg_type WHERE oid=%1" ).arg( typOid );
    // just oid; needs more work to support array type
    //      "oid = (SELECT Distinct typelem FROM pg_type WHERE "  //needs DISTINCT to guard against 2 or more rows on int2
    //      "typelem = " + typOid + " AND typlen = -1)";

    QgsPostgresResult oidResult = mConnectionRO->PQexec( sql );
    QString fieldTypeName = oidResult.PQgetvalue( 0, 0 );
    QString fieldTType = oidResult.PQgetvalue( 0, 1 );
    QString fieldElem = oidResult.PQgetvalue( 0, 2 );
    int fieldSize = oidResult.PQgetvalue( 0, 3 ).toInt();

    QString formattedFieldType;
    if ( tableoid > 0 )
    {
      sql = QString( "SELECT attnum,pg_catalog.format_type(atttypid,atttypmod) FROM pg_attribute WHERE attrelid=%1 AND attname=%2" )
            .arg( tableoid ).arg( quotedValue( fieldName ) );

      QgsPostgresResult tresult = mConnectionRO->PQexec( sql );
      QString attnum = tresult.PQgetvalue( 0, 0 );
      formattedFieldType = tresult.PQgetvalue( 0, 1 );

      sql = QString( "SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=%2" )
            .arg( tableoid ).arg( attnum );

      tresult = mConnectionRO->PQexec( sql );
      if ( tresult.PQntuples() > 0 )
        fieldComment = tresult.PQgetvalue( 0, 0 );
    }

    QVariant::Type fieldType;

    if ( fieldTType == "b" )
    {
      bool isArray = fieldTypeName.startsWith( "_" );

      if ( isArray )
        fieldTypeName = fieldTypeName.mid( 1 );

      if ( fieldTypeName == "int8" || fieldTypeName == "serial8" )
      {
        fieldType = QVariant::LongLong;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == "int2" || fieldTypeName == "int4" ||
                fieldTypeName == "oid" || fieldTypeName == "serial" )
      {
        fieldType = QVariant::Int;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == "real" || fieldTypeName == "double precision" ||
                fieldTypeName == "float4" || fieldTypeName == "float8" )
      {
        fieldType = QVariant::Double;
        fieldSize = -1;
        fieldPrec = -1;
      }
      else if ( fieldTypeName == "numeric" )
      {
        fieldType = QVariant::Double;

        if ( formattedFieldType == "numeric" )
        {
          fieldSize = -1;
          fieldPrec = -1;
        }
        else
        {
          QRegExp re( "numeric\\((\\d+),(\\d+)\\)" );
          if ( re.exactMatch( formattedFieldType ) )
          {
            fieldSize = re.cap( 1 ).toInt();
            fieldPrec = re.cap( 2 ).toInt();
          }
          else if ( formattedFieldType != "numeric" )
          {
            QgsMessageLog::logMessage( tr( "unexpected formatted field type '%1' for field %2" )
                                       .arg( formattedFieldType )
                                       .arg( fieldName ),
                                       tr( "PostGIS" ) );
            fieldSize = -1;
            fieldPrec = -1;
          }
        }
      }
      else if ( fieldTypeName == "varchar" )
      {
        fieldType = QVariant::String;

        QRegExp re( "character varying\\((\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
        }
        else
        {
          fieldSize = -1;
        }
      }
      else if ( fieldTypeName == "text" ||
                fieldTypeName == "bpchar" ||
                fieldTypeName == "bool" ||
                fieldTypeName == "geometry" ||
                fieldTypeName == "money" ||
                fieldTypeName == "ltree" ||
                fieldTypeName == "uuid" ||
                fieldTypeName.startsWith( "time" ) ||
                fieldTypeName.startsWith( "date" ) )
      {
        fieldType = QVariant::String;
        fieldSize = -1;
      }
      else if ( fieldTypeName == "char" )
      {
        fieldType = QVariant::String;

        QRegExp re( "char\\((\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "unexpected formatted field type '%1' for field %2" )
                                     .arg( formattedFieldType )
                                     .arg( fieldName ) );
          fieldSize = -1;
          fieldPrec = -1;
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName ).arg( fieldTypeName ), tr( "PostGIS" ) );
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
      QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName ).arg( fieldTType ), tr( "PostGIS" ) );
      continue;
    }

    if ( fields.contains( fieldName ) )
    {
      QgsMessageLog::logMessage( tr( "Duplicate field %1 found\n" ).arg( fieldName ), tr( "PostGIS" ) );
      return false;
    }

    fields << fieldName;

    mAttributeFields.insert( i, QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, fieldComment ) );
  }

  return true;
}

bool QgsPostgresProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsg( "Checking for permissions on the relation" );

  QgsPostgresResult testAccess;
  if ( !mIsQuery )
  {
    // Check that we can read from the table (i.e., we have select permission).
    QString sql = QString( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );
    QgsPostgresResult testAccess = mConnectionRO->PQexec( sql );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery )
                                 .arg( testAccess.PQresultErrorMessage() )
                                 .arg( sql ), tr( "PostGIS" ) );
      return false;
    }

    bool inRecovery = false;

    if ( mConnectionRO->pgVersion() >= 90000 )
    {
      testAccess = mConnectionRO->PQexec( "SELECT pg_is_in_recovery()" );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK || testAccess.PQgetvalue( 0, 0 ) == "t" )
      {
        QgsMessageLog::logMessage( tr( "PostgreSQL is still in recovery after a database crash\n(or you are connected to a (read-only) slave).\nWrite accesses will be denied." ), tr( "PostGIS" ) );
        inRecovery = true;
      }
    }

    // postgres has fast access to features at id (thanks to primary key / unique index)
    // the latter flag is here just for compatibility
    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
    }

    if ( !inRecovery )
    {
      if ( mConnectionRO->pgVersion() >= 80400 )
      {
        sql = QString( "SELECT "
                       "has_table_privilege(%1,'DELETE'),"
                       "has_any_column_privilege(%1,'UPDATE'),"
                       "%2"
                       "has_table_privilege(%1,'INSERT'),"
                       "current_schema()" )
              .arg( quotedValue( mQuery ) )
              .arg( mGeometryColumn.isNull()
                    ? QString( "'f'," )
                    : QString( "has_column_privilege(%1,%2,'UPDATE')," )
                    .arg( quotedValue( mQuery ) )
                    .arg( quotedValue( mGeometryColumn ) )
                  );
      }
      else
      {
        sql = QString( "SELECT "
                       "has_table_privilege(%1,'DELETE'),"
                       "has_table_privilege(%1,'UPDATE'),"
                       "has_table_privilege(%1,'UPDATE'),"
                       "has_table_privilege(%1,'INSERT'),"
                       "current_schema()" )
              .arg( quotedValue( mQuery ) );
      }

      testAccess = mConnectionRO->PQexec( sql );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage( tr( "Unable to determine table access privileges for the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                   .arg( mQuery )
                                   .arg( testAccess.PQresultErrorMessage() )
                                   .arg( sql ),
                                   tr( "PostGIS" ) );
        return false;
      }


      if ( testAccess.PQgetvalue( 0, 0 ) == "t" )
      {
        // DELETE
        mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
      }

      if ( testAccess.PQgetvalue( 0, 1 ) == "t" )
      {
        // UPDATE
        mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
      }

      if ( testAccess.PQgetvalue( 0, 2 ) == "t" )
      {
        // UPDATE
        mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
      }

      if ( testAccess.PQgetvalue( 0, 3 ) == "t" )
      {
        // INSERT
        mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
      }

      mCurrentSchema = testAccess.PQgetvalue( 0, 4 );
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
      testAccess = mConnectionRO->PQexec( sql );
      if ( testAccess.PQresultStatus() == PGRES_TUPLES_OK && testAccess.PQntuples() == 1 )
      {
        mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes;
      }
    }
  }
  else
  {
    // Check if the sql is a select query
    if ( !mQuery.startsWith( "(SELECT", Qt::CaseInsensitive ) &&
         !mQuery.endsWith( ")" ) )
    {
      QgsMessageLog::logMessage( tr( "The custom query is not a select query." ), tr( "PostGIS" ) );
      return false;
    }

    // get a new alias for the subquery
    int index = 0;
    QString alias;
    QRegExp regex;
    do
    {
      alias = QString( "subQuery_%1" ).arg( QString::number( index++ ) );
      QString pattern = QString( "(\\\"?)%1\\1" ).arg( QRegExp::escape( alias ) );
      regex.setPattern( pattern );
      regex.setCaseSensitivity( Qt::CaseInsensitive );
    }
    while ( mQuery.contains( regex ) );

    // convert the custom query into a subquery
    mQuery = QString( "%1 AS %2" )
             .arg( mQuery )
             .arg( quotedIdentifier( alias ) );

    QString sql = QString( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );

    testAccess = mConnectionRO->PQexec( sql );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( testAccess.PQresultErrorMessage() )
                                 .arg( sql ), tr( "PostGIS" ) );
      return false;
    }

    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
    }
  }

  return true;
}

bool QgsPostgresProvider::determinePrimaryKey()
{
  if ( !loadFields() )
  {
    return false;
  }

  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql;
  if ( !mIsQuery )
  {
    sql = QString( "SELECT indexrelid FROM pg_index WHERE indrelid=%1::regclass AND (indisprimary OR indisunique) ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1" ).arg( quotedValue( mQuery ) );
    QgsDebugMsg( QString( "Retrieving first primary or unique index: %1" ).arg( sql ) );

    QgsPostgresResult res = mConnectionRO->PQexec( sql );
    QgsDebugMsg( QString( "Got %1 rows." ).arg( res.PQntuples() ) );

    QStringList log;

    // no primary or unique indizes found
    if ( res.PQntuples() == 0 )
    {
      QgsDebugMsg( "Relation has no primary key -- investigating alternatives" );

      // Two options here. If the relation is a table, see if there is
      // an oid column that can be used instead.
      // If the relation is a view try to find a suitable column to use as
      // the primary key.

      sql = QString( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
      res = mConnectionRO->PQexec( sql );
      QString type = res.PQgetvalue( 0, 0 );

      if ( type == "r" ) // the relation is a table
      {
        QgsDebugMsg( "Relation is a table. Checking to see if it has an oid column." );

        mPrimaryKeyAttrs.clear();

        // If there is an oid on the table, use that instead,
        sql = QString( "SELECT attname FROM pg_attribute WHERE attname='oid' AND attrelid=regclass(%1)" ).arg( quotedValue( mQuery ) );

        res = mConnectionRO->PQexec( sql );
        if ( res.PQntuples() == 1 )
        {
          // Could warn the user here that performance will suffer if
          // oid isn't indexed (and that they may want to add a
          // primary key to the table)
          mPrimaryKeyType = pktOid;
        }
        else
        {
          sql = QString( "SELECT attname FROM pg_attribute WHERE attname='ctid' AND attrelid=regclass(%1)" ).arg( quotedValue( mQuery ) );

          res = mConnectionRO->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            mPrimaryKeyType = pktTid;
          }
          else
          {
            QgsMessageLog::logMessage( tr( "The table has no column suitable for use as a key. Quantum GIS requires a primary key, a PostgreSQL oid column or a ctid for tables." ), tr( "PostGIS" ) );
          }
        }

      }
      else if ( type == "v" ) // the relation is a view
      {
        QString primaryKey = mUri.keyColumn();
        mPrimaryKeyType = pktUnknown;

        if ( !primaryKey.isEmpty() )
        {
          int idx = fieldNameIndex( primaryKey );

          if ( idx >= 0 )
          {
            if ( mAttributeFields[idx].type() == QVariant::Int || mAttributeFields[idx].type() == QVariant::LongLong )
            {
              if ( mUseEstimatedMetadata || uniqueData( mQuery, primaryKey ) )
              {
                mPrimaryKeyType = pktInt;
                mPrimaryKeyAttrs << idx;
              }
              else
              {
                QgsMessageLog::logMessage( tr( "Primary key field '%1' for view not unique." ).arg( primaryKey ), tr( "PostGIS" ) );
              }
            }
            else
            {
              QgsMessageLog::logMessage( tr( "Type '%1' of primary key field '%2' for view invalid." ).arg( mAttributeFields[idx].typeName() ).arg( primaryKey ), tr( "PostGIS" ) );
            }
          }
          else
          {
            QgsMessageLog::logMessage( tr( "Key field '%1' for view not found." ).arg( primaryKey ), tr( "PostGIS" ) );
          }
        }
        else
        {
          QgsMessageLog::logMessage( tr( "No key field for view given." ), tr( "PostGIS" ) );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unexpected relation type '%1'." ).arg( type ), tr( "PostGIS" ) );
      }
    }
    else
    {
      // have a primary key or unique index
      int indrelid = res.PQgetvalue( 0, 0 ).toInt();
      sql = QString( "SELECT attname FROM pg_index,pg_attribute WHERE indexrelid=%1 AND indrelid=attrelid AND pg_attribute.attnum=any(pg_index.indkey)" ).arg( indrelid );

      QgsDebugMsg( "Retrieving key columns: " + sql );
      res = mConnectionRO->PQexec( sql );
      QgsDebugMsg( QString( "Got %1 rows." ).arg( res.PQntuples() ) );

      bool isInt = true;

      for ( int i = 0; i < res.PQntuples(); i++ )
      {
        QString name = res.PQgetvalue( i, 0 );

        QgsFieldMap::const_iterator it = mAttributeFields.begin();
        while ( it != mAttributeFields.end() && it->name() != name )
          it++;

        if ( it == mAttributeFields.end() )
        {
          QgsDebugMsg( "Skipping " + name );
          continue;
        }

        if ( isInt &&
             it->type() != QVariant::Int &&
             it->type() != QVariant::LongLong )
          isInt = false;

        mPrimaryKeyAttrs << it.key();
      }

      mPrimaryKeyType = ( mPrimaryKeyAttrs.size() == 1 && isInt ) ? pktInt : pktFidMap;
    }
  }
  else
  {
    QString primaryKey = mUri.keyColumn();
    int idx = fieldNameIndex( mUri.keyColumn() );

    if ( idx >= 0 && ( mAttributeFields[idx].type() == QVariant::Int || mAttributeFields[idx].type() == QVariant::LongLong ) )
    {
      if ( mUseEstimatedMetadata || uniqueData( mQuery, primaryKey ) )
      {
        mPrimaryKeyType = pktInt;
        mPrimaryKeyAttrs << idx;
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "No key field for query given." ), tr( "PostGIS" ) );
      mPrimaryKeyType = pktUnknown;
    }
  }

  mValid = mPrimaryKeyType != pktUnknown;

  return mValid;
}

bool QgsPostgresProvider::uniqueData( QString query, QString colName )
{
  Q_UNUSED( query );
  // Check to see if the given column contains unique data
  QString sql = QString( "SELECT count(distinct %1)=count(%1) FROM %2" )
                .arg( quotedIdentifier( colName ) )
                .arg( mQuery );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += " WHERE " + mSqlWhereClause;
  }

  QgsPostgresResult unique = mConnectionRO->PQexec( sql );

  if ( unique.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( unique.PQresultErrorMessage() );
    return false;
  }

  return unique.PQntuples() == 1 && unique.PQgetvalue( 0, 0 ).startsWith( "t" );
}

// Returns the minimum value of an attribute
QVariant QgsPostgresProvider::minimumValue( int index )
{
  try
  {
    // get the field name
    const QgsField &fld = field( index );
    QString sql = QString( "SELECT min(%1) FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    QgsPostgresResult rmin = mConnectionRO->PQexec( sql );
    return convertValue( fld.type(), rmin.PQgetvalue( 0, 0 ) );
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
    QString sql = QString( "SELECT DISTINCT %1 FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql +=  QString( " ORDER BY %1" )
            .arg( quotedIdentifier( fld.name() ) );

    if ( limit >= 0 )
    {
      sql += QString( " LIMIT %1" ).arg( limit );
    }

    QgsPostgresResult res = mConnectionRO->PQexec( sql );
    if ( res.PQresultStatus() == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < res.PQntuples(); i++ )
        uniqueValues.append( convertValue( fld.type(), res.PQgetvalue( i, 0 ) ) );
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
  QgsFieldMap::const_iterator f_it = mAttributeFields.find( index );
  if ( f_it != mAttributeFields.constEnd() )
  {
    typeName = f_it.value().typeName();
  }
  else
  {
    return;
  }

  //is type an enum?
  QString typeSql = QString( "SELECT typtype FROM pg_type WHERE typname=%1" ).arg( quotedValue( typeName ) );
  QgsPostgresResult typeRes = mConnectionRO->PQexec( typeSql );
  if ( typeRes.PQresultStatus() != PGRES_TUPLES_OK || typeRes.PQntuples() < 1 )
  {
    return;
  }


  QString typtype = typeRes.PQgetvalue( 0, 0 );
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

  QString enumRangeSql = QString( "SELECT enumlabel FROM pg_catalog.pg_enum WHERE enumtypid=(SELECT atttypid::regclass FROM pg_attribute WHERE attrelid=%1::regclass AND attname=%2)" )
                         .arg( quotedValue( mQuery ) )
                         .arg( quotedValue( attributeName ) );
  QgsPostgresResult enumRangeRes = mConnectionRO->PQexec( enumRangeSql );

  if ( enumRangeRes.PQresultStatus() != PGRES_TUPLES_OK )
    return false;

  for ( int i = 0; i < enumRangeRes.PQntuples(); i++ )
  {
    enumValues << enumRangeRes.PQgetvalue( i, 0 );
  }

  return true;
}

bool QgsPostgresProvider::parseDomainCheckConstraint( QStringList& enumValues, const QString& attributeName ) const
{
  enumValues.clear();

  //is it a domain type with a check constraint?
  QString domainSql = QString( "SELECT domain_name FROM information_schema.columns WHERE table_name=%1 AND column_name=%2" ).arg( quotedValue( mTableName ) ).arg( quotedValue( attributeName ) );
  QgsPostgresResult domainResult = mConnectionRO->PQexec( domainSql );
  if ( domainResult.PQresultStatus() == PGRES_TUPLES_OK && domainResult.PQntuples() > 0 )
  {
    //a domain type
    QString domainCheckDefinitionSql = QString( "SELECT consrc FROM pg_constraint WHERE conname=(SELECT constraint_name FROM information_schema.domain_constraints WHERE domain_name=%1)" ).arg( quotedValue( domainResult.PQgetvalue( 0, 0 ) ) );
    QgsPostgresResult domainCheckRes = mConnectionRO->PQexec( domainCheckDefinitionSql );
    if ( domainCheckRes.PQresultStatus() == PGRES_TUPLES_OK && domainCheckRes.PQntuples() > 0 )
    {
      QString checkDefinition = domainCheckRes.PQgetvalue( 0, 0 );

      //we assume that the constraint is of the following form:
      //(VALUE = ANY (ARRAY['a'::text, 'b'::text, 'c'::text, 'd'::text]))
      //normally, postgresql creates that if the contstraint has been specified as 'VALUE in ('a', 'b', 'c', 'd')

      int anyPos = checkDefinition.indexOf( QRegExp( "VALUE\\s*=\\s*ANY\\s*\\(\\s*ARRAY\\s*\\[" ) );
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
    QString sql = QString( "SELECT max(%1) FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    QgsPostgresResult rmax = mConnectionRO->PQexec( sql );
    return convertValue( fld.type(), rmax.PQgetvalue( 0, 0 ) );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString::null );
  }
}


bool QgsPostgresProvider::isValid()
{
  return mValid;
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

  QString sql = QString( "SELECT column_default FROM information_schema.columns WHERE column_default IS NOT NULL AND table_schema=%1 AND table_name=%2 AND column_name=%3 " )
                .arg( quotedValue( schemaName ) )
                .arg( quotedValue( tableName ) )
                .arg( quotedValue( fieldName ) );

  QVariant defaultValue( QString::null );

  QgsPostgresResult result = mConnectionRO->PQexec( sql );

  if ( result.PQntuples() == 1 )
    defaultValue = result.PQgetvalue( 0, 0 );

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

QString QgsPostgresProvider::paramValue( QString fieldValue, const QString &defaultValue ) const
{
  if ( fieldValue.isNull() )
    return QString::null;

  if ( fieldValue == defaultValue && !defaultValue.isNull() )
  {
    QgsPostgresResult result = mConnectionRW->PQexec( QString( "SELECT %1" ).arg( defaultValue ) );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    return result.PQgetvalue( 0, 0 );
  }

  return fieldValue;
}

QString QgsPostgresProvider::geomParam( int offset ) const
{
  QString geometry;

  bool forceMulti;
  switch ( geometryType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBLineString:
    case QGis::WKBPolygon:
    case QGis::WKBPoint25D:
    case QGis::WKBLineString25D:
    case QGis::WKBPolygon25D:
    case QGis::WKBUnknown:
    case QGis::WKBNoGeometry:
      forceMulti = false;
      break;

    case QGis::WKBMultiPoint:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiPolygon25D:
      forceMulti = true;
      break;
  }


  if ( forceMulti )
  {
    geometry += mConnectionRO->majorVersion() < 2 ? "multi(" : "st_multi(";
  }

  geometry += QString( "%1($%2%3,%4)" )
              .arg( mConnectionRO->majorVersion() < 2 ? "geomfromwkb" : "st_geomfromwkb" )
              .arg( offset )
              .arg( mConnectionRW->useWkbHex() ? "" : "::bytea" )
              .arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );

  if ( forceMulti )
  {
    geometry += ")";
  }

  return geometry;
}

bool QgsPostgresProvider::addFeatures( QgsFeatureList &flist )
{
  if ( flist.size() == 0 )
    return true;

  if ( mIsQuery )
    return false;

  if ( !connectRW() )
    return false;

  bool returnvalue = true;

  try
  {
    mConnectionRW->PQexecNR( "BEGIN" );

    // Prepare the INSERT statement
    QString insert = QString( "INSERT INTO %1(" ).arg( mQuery );
    QString values = ") VALUES (";
    QString delim = "";
    int offset = 1;

    QStringList defaultValues;
    QList<int> fieldId;

    if ( !mGeometryColumn.isNull() )
    {
      insert += quotedIdentifier( mGeometryColumn );

      values += geomParam( offset++ );

      delim = ",";
    }

    if ( mPrimaryKeyType == pktInt || mPrimaryKeyType == pktFidMap )
    {
      foreach( int idx, mPrimaryKeyAttrs )
      {
        insert += delim + quotedIdentifier( field( idx ).name() );
        values += delim + QString( "$%1" ).arg( defaultValues.size() + offset );
        delim = ",";
        fieldId << idx;
        defaultValues << defaultValue( idx ).toString();
      }
    }

    const QgsAttributeMap &attributevec = flist[0].attributeMap();

    // look for unique attribute values to place in statement instead of passing as parameter
    // e.g. for defaults
    for ( QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++ )
    {
      if ( fieldId.contains( it.key() ) )
        continue;

      QgsFieldMap::const_iterator fit = mAttributeFields.find( it.key() );
      if ( fit == mAttributeFields.end() )
        continue;

      QString fieldname = fit->name();

      QgsDebugMsg( "Checking field against: " + fieldname );

      if ( fieldname.isEmpty() || fieldname == mGeometryColumn )
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

      insert += delim + quotedIdentifier( fieldname );

      QString defVal = defaultValue( it.key() ).toString();

      if ( i == flist.size() )
      {
        if ( *it == defVal )
        {
          if ( defVal.isNull() )
          {
            values += delim + "NULL";
          }
          else
          {
            values += delim + defVal;
          }
        }
        else if ( fit->typeName() == "geometry" )
        {
          values += QString( "%1%2(%3)" )
                    .arg( delim )
                    .arg( mConnectionRO->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt" )
                    .arg( quotedValue( it->toString() ) );
        }
        else if ( fit->typeName() == "geography" )
        {
          values += QString( "%1st_geographyfromewkt(%2)" )
                    .arg( delim )
                    .arg( quotedValue( it->toString() ) );
        }
        else
        {
          values += delim + quotedValue( it->toString() );
        }
      }
      else
      {
        // value is not unique => add parameter
        if ( fit->typeName() == "geometry" )
        {
          values += QString( "%1%2($%3)" )
                    .arg( delim )
                    .arg( mConnectionRO->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt" )
                    .arg( defaultValues.size() + offset );
        }
        else if ( fit->typeName() == "geography" )
        {
          values += QString( "%1st_geographyfromewkt($%2)" )
                    .arg( delim )
                    .arg( defaultValues.size() + offset );
        }
        else
        {
          values += QString( "%1$%2" )
                    .arg( delim )
                    .arg( defaultValues.size() + offset );
        }
        defaultValues.append( defVal );
        fieldId.append( it.key() );
      }

      delim = ",";
    }

    insert += values + ")";

    QgsDebugMsg( QString( "prepare addfeatures: %1" ).arg( insert ) );
    QgsPostgresResult stmt = mConnectionRW->PQprepare( "addfeatures", insert, fieldId.size() + offset - 1, NULL );
    if ( stmt.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( stmt );

    for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); features++ )
    {
      const QgsAttributeMap &attributevec = features->attributeMap();

      QStringList params;
      if ( !mGeometryColumn.isNull() )
      {
        appendGeomParam( features->geometry(), params );
      }

      for ( int i = 0; i < fieldId.size(); i++ )
      {
        QgsAttributeMap::const_iterator attr = attributevec.find( fieldId[i] );

        QString v;
        if ( attr == attributevec.end() )
        {
          const QgsField &fld = field( fieldId[i] );
          v = paramValue( defaultValues[i], defaultValues[i] );
          features->addAttribute( fieldId[i], convertValue( fld.type(), v ) );
        }
        else
        {
          v = paramValue( attr.value().toString(), defaultValues[i] );

          if ( v != attr.value().toString() )
          {
            const QgsField &fld = field( fieldId[i] );
            features->changeAttribute( fieldId[i], convertValue( fld.type(), v ) );
          }
        }

        params << v;
      }

      QgsPostgresResult result = mConnectionRW->PQexecPrepared( "addfeatures", params );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      if ( mPrimaryKeyType == pktOid )
      {
        features->setFeatureId( result.PQoidValue() );
        QgsDebugMsgLevel( QString( "new fid=%1" ).arg( features->id() ), 4 );
      }
    }

    // update feature ids
    if ( mPrimaryKeyType == pktInt || mPrimaryKeyType == pktFidMap )
    {
      for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); features++ )
      {
        const QgsAttributeMap &attributevec = features->attributeMap();

        if ( mPrimaryKeyType == pktInt )
        {
          features->setFeatureId( STRING_TO_FID( attributevec[ mPrimaryKeyAttrs[0] ] ) );
        }
        else
        {
          QList<QVariant> primaryKeyVals;

          foreach( int idx, mPrimaryKeyAttrs )
          {
            primaryKeyVals << attributevec[ idx ];
          }

          features->setFeatureId( lookupFid( QVariant( primaryKeyVals ) ) );
        }
        QgsDebugMsgLevel( QString( "new fid=%1" ).arg( features->id() ), 4 );
      }
    }

    mConnectionRW->PQexecNR( "DEALLOCATE addfeatures" );
    mConnectionRW->PQexecNR( "COMMIT" );

    mFeaturesCounted += flist.size();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while adding features: %1" ).arg( e.errorMessage() ) );
    mConnectionRW->PQexecNR( "ROLLBACK" );
    mConnectionRW->PQexecNR( "DEALLOCATE addfeatures" );
    returnvalue = false;
  }

  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::deleteFeatures( const QgsFeatureIds & id )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( !connectRW() )
    return false;

  try
  {
    mConnectionRW->PQexecNR( "BEGIN" );

    for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
    {
      QString sql = QString( "DELETE FROM %1 WHERE %2" )
                    .arg( mQuery ).arg( whereClause( *it ) );
      QgsDebugMsg( "delete sql: " + sql );

      //send DELETE statement and do error handling
      QgsPostgresResult result = mConnectionRW->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      QVariant v = mFidToKey[ *it ];
      mFidToKey.remove( *it );
      mKeyToFid.remove( v );
    }

    mConnectionRW->PQexecNR( "COMMIT" );

    mFeaturesCounted -= id.size();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while deleting features: %1" ).arg( e.errorMessage() ) );
    mConnectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }
  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( !connectRW() )
    return false;

  try
  {
    mConnectionRW->PQexecNR( "BEGIN" );

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString type = iter->typeName();
      if ( type == "char" || type == "varchar" )
      {
        if ( iter->length() > 0 )
          type = QString( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == "numeric" || type == "decimal" )
      {
        if ( iter->length() > 0 && iter->precision() > 0 )
          type = QString( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
      }

      QString sql = QString( "ALTER TABLE %1 ADD COLUMN %2 %3" )
                    .arg( mQuery )
                    .arg( quotedIdentifier( iter->name() ) )
                    .arg( type );
      QgsDebugMsg( sql );

      //send sql statement and do error handling
      QgsPostgresResult result = mConnectionRW->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      if ( !iter->comment().isEmpty() )
      {
        sql = QString( "COMMENT ON COLUMN %1.%2 IS %3" )
              .arg( mQuery )
              .arg( quotedIdentifier( iter->name() ) )
              .arg( quotedValue( iter->comment() ) );
        result = mConnectionRW->PQexec( sql );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
          throw PGException( result );
      }
    }

    mConnectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while adding attributes: %1" ).arg( e.errorMessage() ) );
    mConnectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }

  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes( const QgsAttributeIds& ids )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( !connectRW() )
    return false;

  try
  {
    mConnectionRW->PQexecNR( "BEGIN" );

    for ( QgsAttributeIds::const_iterator iter = ids.begin(); iter != ids.end(); ++iter )
    {
      QgsFieldMap::const_iterator field_it = mAttributeFields.find( *iter );
      if ( field_it == mAttributeFields.constEnd() )
        continue;

      QString column = field_it->name();
      QString sql = QString( "ALTER TABLE %1 DROP COLUMN %2" )
                    .arg( mQuery )
                    .arg( quotedIdentifier( column ) );

      //send sql statement and do error handling
      QgsPostgresResult result = mConnectionRW->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      //delete the attribute from mAttributeFields
      mAttributeFields.remove( *iter );
    }

    mConnectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while deleting attributes: %1" ).arg( e.errorMessage() ) );
    mConnectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }

  rewind();
  return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( !connectRW() )
    return false;

  try
  {
    mConnectionRW->PQexecNR( "BEGIN" );

    // cycle through the features
    for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
    {
      QgsFeatureId fid = iter.key();

      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      QString sql = QString( "UPDATE %1 SET " ).arg( mQuery );

      const QgsAttributeMap& attrs = iter.value();
      bool pkChanged = false;

      // cycle through the changed attributes of the feature
      QString delim;
      for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          sql += delim + QString( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ",";

          if ( fld.typeName() == "geometry" )
          {
            sql += QString( "%1(%2)" )
                   .arg( mConnectionRO->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt" )
                   .arg( quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == "geography" )
          {
            sql += QString( "st_geographyfromewkt(%1)" )
                   .arg( quotedValue( siter->toString() ) );
          }
          else
          {
            sql += quotedValue( siter->toString() );
          }
        }
        catch ( PGFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      sql += QString( " WHERE %1" ).arg( whereClause( fid ) );

      QgsPostgresResult result = mConnectionRW->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      // update feature id map if key was changed
      if ( pkChanged && mPrimaryKeyType == pktFidMap )
      {
        QVariant v = mFidToKey[ fid ];
        mFidToKey.remove( fid );
        mKeyToFid.remove( v );

        QList<QVariant> k = v.toList();

        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs[i];
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[ idx ];
        }

        mFidToKey.insert( fid, k );
        mKeyToFid.insert( k, fid );
      }
    }

    mConnectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while changing attributes: %1" ).arg( e.errorMessage() ) );
    mConnectionRW->PQexecNR( "ROLLBACK" );
    returnvalue = false;
  }

  rewind();

  return returnvalue;
}

void QgsPostgresProvider::appendGeomParam( QgsGeometry *geom, QStringList &params ) const
{
  QString param;
  unsigned char *buf = geom->asWkb();
  for ( uint i = 0; i < geom->wkbSize(); ++i )
  {
    if ( mConnectionRW->useWkbHex() )
      param += QString( "%1" ).arg(( int ) buf[i], 2, 16, QChar( '0' ) );
    else
      param += QString( "\\%1" ).arg(( int ) buf[i], 3, 8, QChar( '0' ) );
  }
  params << param;
}

bool QgsPostgresProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  QgsDebugMsg( "entering." );

  if ( mIsQuery || mGeometryColumn.isNull() )
    return false;

  if ( !connectRW() )
    return false;

  bool returnvalue = true;

  try
  {
    // Start the PostGIS transaction
    mConnectionRW->PQexecNR( "BEGIN" );

    QString update = QString( "UPDATE %1 SET %2=%3 WHERE %4" )
                     .arg( mQuery )
                     .arg( quotedIdentifier( mGeometryColumn ) )
                     .arg( geomParam( 1 ) )
                     .arg( pkParamWhereClause( 2 ) );

    QgsDebugMsg( "updating: " + update );

    QgsPostgresResult result = mConnectionRW->PQprepare( "updatefeatures", update, 2, NULL );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );

    for ( QgsGeometryMap::iterator iter  = geometry_map.begin();
          iter != geometry_map.end();
          ++iter )
    {
      QgsDebugMsg( "iterating over the map of changed geometries..." );

      if ( !iter->asWkb() )
      {
        QgsDebugMsg( "empty geometry" );
        continue;
      }

      QgsDebugMsg( "iterating over feature id " + FID_TO_STRING( iter.key() ) );

      QStringList params;
      appendGeomParam( &*iter, params );
      appendPkParams( iter.key(), params );

      result = mConnectionRW->PQexecPrepared( "updatefeatures", params );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );
    } // for each feature

    mConnectionRW->PQexecNR( "DEALLOCATE updatefeatures" );
    mConnectionRW->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while changing geometry values: %1" ).arg( e.errorMessage() ) );
    mConnectionRW->PQexecNR( "ROLLBACK" );
    mConnectionRW->PQexecNR( "DEALLOCATE updatefeatures" );
    returnvalue = false;
  }

  rewind();

  QgsDebugMsg( "exiting." );

  return returnvalue;
}

QgsAttributeList QgsPostgresProvider::attributeIndexes()
{
  return mAttributeFields.keys();
}

int QgsPostgresProvider::capabilities() const
{
  return mEnabledCapabilities;
}

bool QgsPostgresProvider::setSubsetString( QString theSQL, bool updateFeatureCount )
{
  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QString( "SELECT * FROM %1" ).arg( mQuery );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
  }

  sql += " LIMIT 0";

  QgsPostgresResult res = mConnectionRO->PQexec( sql );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( res.PQresultErrorMessage() );
    mSqlWhereClause = prevWhere;
    return false;
  }

#if 0
  // FIXME
  if ( mPrimaryKeyType == pktInt && !uniqueData( mQuery, primaryKeyAttr ) )
  {
    sqlWhereClause = prevWhere;
    return false;
  }
#endif

  // Update datasource uri too
  mUri.setSql( theSQL );
  // Update yet another copy of the uri. Why are there 3 copies of the
  // uri? Perhaps this needs some rationalisation.....
  setDataSourceUri( mUri.uri() );

  if ( updateFeatureCount )
  {
    mFeaturesCounted = -1;
  }
  mLayerExtent.setMinimal();

  return true;
}

/**
 * Return the feature count
 */
long QgsPostgresProvider::featureCount() const
{
  if ( mFeaturesCounted >= 0 )
    return mFeaturesCounted;

  // get total number of features
  QString sql;

  // use estimated metadata even when there is a where clause,
  // although we get an incorrect feature count for the subset
  // - but make huge dataset usable.
  if ( !mIsQuery && mUseEstimatedMetadata )
  {
    sql = QString( "SELECT reltuples::int FROM pg_catalog.pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
  }
  else
  {
    sql = QString( "SELECT count(*) FROM %1" ).arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += " WHERE " + mSqlWhereClause;
    }
  }

  QgsPostgresResult result = mConnectionRO->PQexec( sql );

  QgsDebugMsg( "number of features as text: " + result.PQgetvalue( 0, 0 ) );

  mFeaturesCounted = result.PQgetvalue( 0, 0 ).toLong();

  QgsDebugMsg( "number of features: " + QString::number( mFeaturesCounted ) );

  return mFeaturesCounted;
}

QgsRectangle QgsPostgresProvider::extent()
{
  if ( mGeometryColumn.isNull() )
    return QgsRectangle();

  if ( mIsGeography )
    return QgsRectangle( -180.0, -90.0, 180.0, 90.0 );

  if ( mLayerExtent.isEmpty() )
  {
    QString sql;
    QgsPostgresResult result;
    QString ext;

    // get the extents
    if ( !mIsQuery && ( mUseEstimatedMetadata || mSqlWhereClause.isEmpty() ) )
    {
      // do stats exists?
      sql = QString( "SELECT count(*) FROM pg_stats WHERE schemaname=%1 AND tablename=%2 AND attname=%3" )
            .arg( quotedValue( mSchemaName ) )
            .arg( quotedValue( mTableName ) )
            .arg( quotedValue( mGeometryColumn ) );
      result = mConnectionRO->PQexec( sql );
      if ( result.PQresultStatus() == PGRES_TUPLES_OK && result.PQntuples() == 1 )
      {
        if ( result.PQgetvalue( 0, 0 ).toInt() > 0 )
        {
          sql = QString( "SELECT reltuples::int FROM pg_catalog.pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
          result = mConnectionRO->PQexec( sql );
          if ( result.PQresultStatus() == PGRES_TUPLES_OK
               && result.PQntuples() == 1
               && result.PQgetvalue( 0, 0 ).toLong() > 0 )
          {
            sql = QString( "SELECT %1(%2,%3,%4)" )
                  .arg( mConnectionRO->majorVersion() < 2 ? "estimated_extent" : "st_estimated_extent" )
                  .arg( quotedValue( mSchemaName ) )
                  .arg( quotedValue( mTableName ) )
                  .arg( quotedValue( mGeometryColumn ) );
            result = mConnectionRO->PQexec( sql );
            if ( result.PQresultStatus() == PGRES_TUPLES_OK && result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 ) )
            {
              ext = result.PQgetvalue( 0, 0 );

              // fix for what might be a postgis bug: when the extent crosses the
              // dateline extent() returns -180 to 180 (which appears right), but
              // estimated_extent() returns eastern bound of data (>-180) and
              // 180 degrees.
              if ( !ext.startsWith( "-180 " ) && ext.contains( ",180 " ) )
              {
                ext.clear();
              }
            }
          }
          else
          {
            // no features => ignore estimated extent
            ext.clear();
          }
        }
      }
      else
      {
        QgsDebugMsg( QString( "no column statistics for %1.%2.%3" ).arg( mSchemaName ).arg( mTableName ).arg( mGeometryColumn ) );
      }
    }

    if ( ext.isEmpty() )
    {
      sql = QString( "SELECT %1(%2) FROM %3" )
            .arg( mConnectionRO->majorVersion() < 2 ? "extent" : "st_extent" )
            .arg( quotedIdentifier( mGeometryColumn ) )
            .arg( mQuery );

      if ( !mSqlWhereClause.isEmpty() )
        sql += QString( " WHERE %1" ).arg( mSqlWhereClause );

      result = mConnectionRO->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        mConnectionRO->PQexecNR( "ROLLBACK" );
      else if ( result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 ) )
        ext = result.PQgetvalue( 0, 0 );
    }

    if ( !ext.isEmpty() )
    {
      QgsDebugMsg( "Got extents using: " + sql );

      QRegExp rx( "\\((.+) (.+),(.+) (.+)\\)" );
      if ( ext.contains( rx ) )
      {
        QStringList ex = rx.capturedTexts();

        mLayerExtent.setXMinimum( ex[1].toDouble() );
        mLayerExtent.setYMinimum( ex[2].toDouble() );
        mLayerExtent.setXMaximum( ex[3].toDouble() );
        mLayerExtent.setYMaximum( ex[4].toDouble() );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "result of extents query invalid: %1" ).arg( ext ), tr( "PostGIS" ) );
      }
    }

    QgsDebugMsg( "Set extents to: " + mLayerExtent.toString() );
  }

  return mLayerExtent;
}

void QgsPostgresProvider::updateExtents()
{
  mLayerExtent.setMinimal();
}

bool QgsPostgresProvider::getGeometryDetails()
{
  if ( mGeometryColumn.isNull() )
  {
    mDetectedGeomType = QGis::WKBNoGeometry;
    mValid = true;
    return true;
  }

  QgsPostgresResult result;
  QString sql;

  QString schemaName = mSchemaName;
  QString tableName = mTableName;
  QString geomCol = mGeometryColumn;

  if ( mIsQuery )
  {
    sql = QString( "SELECT %1 FROM %2 LIMIT 0" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mQuery );

    QgsDebugMsg( QString( "Getting geometry column: %1" ).arg( sql ) );

    QgsPostgresResult result = mConnectionRO->PQexec( sql );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      Oid tableoid = result.PQftable( 0 );
      int column = result.PQftablecol( 0 );

      result = mConnectionRO->PQexec( sql );
      if ( tableoid > 0 && PGRES_TUPLES_OK == result.PQresultStatus() )
      {
        sql = QString( "SELECT pg_namespace.nspname,pg_class.relname FROM pg_class,pg_namespace WHERE pg_class.relnamespace=pg_namespace.oid AND pg_class.oid=%1" ).arg( tableoid );
        result = mConnectionRO->PQexec( sql );

        if ( PGRES_TUPLES_OK == result.PQresultStatus() && 1 == result.PQntuples() )
        {
          schemaName = result.PQgetvalue( 0, 0 );
          tableName = result.PQgetvalue( 0, 1 );

          sql = QString( "SELECT attname FROM pg_attribute WHERE attrelid=%1 AND attnum=%2" ).arg( tableoid ).arg( column );
          result = mConnectionRO->PQexec( sql );
          if ( PGRES_TUPLES_OK == result.PQresultStatus() && 1 == result.PQntuples() )
          {
            geomCol = result.PQgetvalue( 0, 0 );
          }
          else
          {
            schemaName = mSchemaName;
            tableName = mTableName;
          }
        }
      }
      else
      {
        schemaName = "";
        tableName = mQuery;
      }
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  QString detectedType;
  QString detectedSrid;

  if ( !schemaName.isEmpty() )
  {
    // check geometry columns
    sql = QString( "SELECT upper(type),srid FROM geometry_columns WHERE f_table_name=%1 AND f_geometry_column=%2 AND f_table_schema=%3" )
          .arg( quotedValue( tableName ) )
          .arg( quotedValue( geomCol ) )
          .arg( quotedValue( schemaName ) );

    QgsDebugMsg( QString( "Getting geometry column: %1" ).arg( sql ) );
    result = mConnectionRO->PQexec( sql );
    QgsDebugMsg( QString( "Geometry column query returned %1 rows" ).arg( result.PQntuples() ) );

    if ( result.PQntuples() == 1 )
    {
      detectedType = result.PQgetvalue( 0, 0 );
      detectedSrid = result.PQgetvalue( 0, 1 );
    }
    else
    {
      mConnectionRO->PQexecNR( "COMMIT" );
    }

    if ( detectedType.isEmpty() )
    {
      // check geography columns
      sql = QString( "SELECT upper(type),srid FROM geography_columns WHERE f_table_name=%1 AND f_geography_column=%2 AND f_table_schema=%3" )
            .arg( quotedValue( tableName ) )
            .arg( quotedValue( geomCol ) )
            .arg( quotedValue( schemaName ) );

      QgsDebugMsg( QString( "Getting geography column: %1" ).arg( sql ) );
      result = mConnectionRO->PQexec( sql, false );
      QgsDebugMsg( QString( "Geography column query returned %1" ).arg( result.PQntuples() ) );

      if ( result.PQntuples() == 1 )
      {
        detectedType = result.PQgetvalue( 0, 0 );
        detectedSrid = result.PQgetvalue( 0, 1 );
        mIsGeography = true;
      }
      else
      {
        mConnectionRO->PQexecNR( "COMMIT" );
      }
    }
  }

  if ( QgsPostgresConn::wkbTypeFromPostgis( detectedType ) == QGis::WKBUnknown )
  {
    QgsPostgresLayerProperty layerProperty;
    layerProperty.schemaName = schemaName;
    layerProperty.tableName = tableName;
    layerProperty.geometryColName = mGeometryColumn;
    layerProperty.isGeography = mIsGeography;

    QString delim = "";

    if ( !mSqlWhereClause.isEmpty() )
    {
      layerProperty.sql += delim + "(" + mSqlWhereClause + ")";
      delim = " AND ";
    }

    mConnectionRO->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );

    QStringList typeList = layerProperty.type.split( ",", QString::SkipEmptyParts );
    QStringList sridList = layerProperty.srid.split( ",", QString::SkipEmptyParts );
    Q_ASSERT( typeList.size() == sridList.size() );

    if ( typeList.size() == 0 )
    {
      // no data - so take what's requested
      if ( mRequestedGeomType == QGis::WKBUnknown || mRequestedSrid.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "Geometry type and srid for empty column %1 of %2 undefined." ).arg( mGeometryColumn ).arg( mQuery ) );
      }

      detectedType = "";
      detectedSrid = "";
    }
    else
    {
      int i;
      for ( i = 0; i < typeList.size(); i++ )
      {
        QGis::WkbType wkbType = QgsPostgresConn::wkbTypeFromPostgis( typeList.at( i ) );

        if (( wkbType != QGis::WKBUnknown && ( mRequestedGeomType == QGis::WKBUnknown || mRequestedGeomType == wkbType ) ) &&
            ( mRequestedSrid.isEmpty() || sridList.at( i ) == mRequestedSrid ) )
          break;
      }

      // requested type && srid is available
      if ( i < typeList.size() )
      {
        if ( typeList.size() == 1 )
        {
          // only what we requested is available
          detectedType = typeList.at( 0 );
          detectedSrid = sridList.at( 0 );
        }
        else
        {
          // we need to filter
          detectedType = "";
          detectedSrid = "";
        }
      }
      else
      {
        // geometry type undetermined or not unrequested
        QgsMessageLog::logMessage( tr( "Feature type or srid for %1 of %2 could not be determined or was not requested." ).arg( mGeometryColumn ).arg( mQuery ) );
        detectedType = "";
        detectedSrid = "";
      }
    }
  }

  mDetectedGeomType = QgsPostgresConn::wkbTypeFromPostgis( detectedType );
  mDetectedSrid     = detectedSrid;

  QgsDebugMsg( QString( "Detected SRID is %1" ).arg( mDetectedSrid ) );
  QgsDebugMsg( QString( "Requested SRID is %1" ).arg( mRequestedSrid ) );
  QgsDebugMsg( QString( "Detected type is %1" ).arg( mDetectedGeomType ) );
  QgsDebugMsg( QString( "Requested type is %1" ).arg( mRequestedGeomType ) );

  mValid = ( mDetectedGeomType != QGis::WKBUnknown || mRequestedGeomType != QGis::WKBUnknown )
           && ( !mDetectedSrid.isEmpty() || !mRequestedSrid.isEmpty() );

  if ( !mValid )
    return false;


  // store whether the geometry includes measure value
  if ( detectedType == "POINTM" || detectedType == "MULTIPOINTM" ||
       detectedType == "LINESTRINGM" || detectedType == "MULTILINESTRINGM" ||
       detectedType == "POLYGONM" || detectedType == "MULTIPOLYGONM" )
  {
    // explicitly disable adding new features and editing of geometries
    // as this would lead to corruption of measures
    QgsMessageLog::logMessage( tr( "Editing and adding disabled for 2D+ layer (%1; %2)" ).arg( mGeometryColumn ).arg( mQuery ) );
    mEnabledCapabilities &= ~( QgsVectorDataProvider::ChangeGeometries | QgsVectorDataProvider::AddFeatures );
  }

  QgsDebugMsg( QString( "Feature type name is %1" ).arg( QGis::qgisFeatureTypes[ geometryType()] ) );
  QgsDebugMsg( QString( "Geometry is geography %1" ).arg( mIsGeography ) );

  return mValid;
}

bool QgsPostgresProvider::convertField( QgsField &field )
{
  QString fieldType = "varchar"; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = "int8";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::String:
      fieldType = "varchar";
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = "int";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = "float";
        fieldSize = -1;
        fieldPrec = -1;
      }
      else
      {
        fieldType = "decimal";
      }
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}

QgsVectorLayerImport::ImportError QgsPostgresProvider::createEmptyLayer(
  const QString& uri,
  const QgsFieldMap &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  Q_UNUSED( options );

  // populate members from the uri structure
  QgsDataSourceURI dsUri( uri );
  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QString schemaTableName = "";
  if ( !schemaName.isEmpty() )
  {
    schemaTableName += quotedIdentifier( schemaName ) + ".";
  }
  schemaTableName += quotedIdentifier( tableName );

  QgsDebugMsg( QString( "Connection info is: %1" ).arg( dsUri.connectionInfo() ) );
  QgsDebugMsg( QString( "Geometry column is: %1" ).arg( geometryColumn ) );
  QgsDebugMsg( QString( "Schema is: %1" ).arg( schemaName ) );
  QgsDebugMsg( QString( "Table name is: %1" ).arg( tableName ) );

  // create the table
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo(), false );
  if ( !conn )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return QgsVectorLayerImport::ErrConnectionFailed;
  }

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = "id";
    for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      if ( fldIt.value().name() == pk )
      {
        // it already exists, try again with a new name
        primaryKey = QString( "%1_%2" ).arg( pk ).arg( index++ );
        fldIt = fields.begin();
      }
    }
  }
  else
  {
    // search for the passed field
    for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      if ( fldIt.value().name() == primaryKey )
      {
        // found, get the field type
        QgsField fld = fldIt.value();
        if ( convertField( fld ) )
        {
          primaryKeyType = fld.typeName();
        }
      }
    }
  }

  // if the field doesn't not exist yet, create it as a serial field
  if ( primaryKeyType.isEmpty() )
  {
    primaryKeyType = "serial";
#if 0
    // TODO: check the feature count to choose if create a serial8 pk field
    if ( layer->featureCount() > 0xffffffff )
    {
      primaryKeyType = "serial8";
    }
#endif
  }

  try
  {
    conn->PQexecNR( "BEGIN" );

    QString sql = QString( "SELECT 1"
                           " FROM pg_class AS cls JOIN pg_namespace AS nsp"
                           " ON nsp.oid=cls.relnamespace "
                           " WHERE cls.relname=%1 AND nsp.nspname=%2" )
                  .arg( quotedValue( tableName ) )
                  .arg( quotedValue( schemaName ) );

    QgsPostgresResult result = conn->PQexec( sql );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    bool exists = result.PQntuples() > 0;

    if ( exists && overwrite )
    {
      // delete the table if exists, then re-create it
      QString sql = QString( "SELECT DropGeometryTable(%1,%2)"
                             " FROM pg_class AS cls JOIN pg_namespace AS nsp"
                             " ON nsp.oid=cls.relnamespace "
                             " WHERE cls.relname=%2 AND nsp.nspname=%1" )
                    .arg( quotedValue( schemaName ) )
                    .arg( quotedValue( tableName ) );

      result = conn->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );
    }

    sql = QString( "CREATE TABLE %1(%2 %3 PRIMARY KEY)" )
          .arg( schemaTableName )
          .arg( quotedIdentifier( primaryKey ) )
          .arg( primaryKeyType );

    result = conn->PQexec( sql );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );

    // get geometry type, dim and srid
    int dim = 2;
    long srid = srs->postgisSrid();

    QgsPostgresConn::postgisWkbType( wkbType, geometryType, dim );

    // create geometry column
    if ( !geometryType.isEmpty() )
    {
      sql = QString( "SELECT AddGeometryColumn(%1,%2,%3,%4,%5,%6)" )
            .arg( quotedValue( schemaName ) )
            .arg( quotedValue( tableName ) )
            .arg( quotedValue( geometryColumn ) )
            .arg( srid )
            .arg( quotedValue( geometryType ) )
            .arg( dim );

      result = conn->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );
    }
    else
    {
      geometryColumn.clear();
    }

    conn->PQexecNR( "COMMIT" );
  }
  catch ( PGException &e )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Creation of data source %1 failed: \n%2" )
                      .arg( schemaTableName )
                      .arg( e.errorMessage() );

    conn->PQexecNR( "ROLLBACK" );
    conn->disconnect();
    return QgsVectorLayerImport::ErrCreateLayer;
  }
  conn->disconnect();

  QgsDebugMsg( QString( "layer %1 created" ).arg( schemaTableName ) );

  // use the provider to edit the table
  dsUri.setDataSource( schemaName, tableName, geometryColumn, QString(), primaryKey );
  QgsPostgresProvider *provider = new QgsPostgresProvider( dsUri.uri() );
  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( schemaTableName );

    delete provider;
    return QgsVectorLayerImport::ErrInvalidLayer;
  }

  QgsDebugMsg( "layer loaded" );

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = geometryColumn.isEmpty() ? 1 : 2;

    // get the list of fields
    QList<QgsField> flist;
    for ( QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt != fields.end(); ++fldIt )
    {
      QgsField fld = fldIt.value();
      if ( fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( fldIt.key(), 0 );
        continue;
      }

      if ( fld.name() == geometryColumn )
      {
        QgsDebugMsg( "Found a field with the same name of the geometry column. Skip it!" );
        continue;
      }

      if ( !convertField( fld ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        delete provider;
        return QgsVectorLayerImport::ErrAttributeTypeUnsupported;
      }

      QgsDebugMsg( QString( "creating field #%1 -> #%2 name %3 type %4 typename %5 width %6 precision %7" )
                   .arg( fldIt.key() ).arg( offset )
                   .arg( fld.name() ).arg( QVariant::typeToName( fld.type() ) ).arg( fld.typeName() )
                   .arg( fld.length() ).arg( fld.precision() )
                 );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIt.key(), offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      delete provider;
      return QgsVectorLayerImport::ErrAttributeCreationFailed;
    }

    QgsDebugMsg( "Done creating fields" );
  }
  return QgsVectorLayerImport::NoError;
}

QgsCoordinateReferenceSystem QgsPostgresProvider::crs()
{
  QgsCoordinateReferenceSystem srs;
  srs.createFromSrid( mRequestedSrid.isEmpty() ? mDetectedSrid.toInt() : mRequestedSrid.toInt() );
  return srs;
}

QString QgsPostgresProvider::subsetString()
{
  return mSqlWhereClause;
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

QGISEXTERN QgsPgSourceSelect *selectWidget( QWidget *parent, Qt::WFlags fl )
{
  return new QgsPgSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString thePath, QgsDataItem *parentItem )
{
  Q_UNUSED( thePath );
  return new QgsPGRootItem( parentItem, "PostGIS", "pg:" );
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsVectorLayerImport::ImportError createEmptyLayer(
  const QString& uri,
  const QgsFieldMap &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsPostgresProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

QGISEXTERN bool deleteLayer( const QString& uri, QString& errCause )
{
  QgsDebugMsg( "deleting layer " + uri );

  QgsDataSourceURI dsUri( uri );
  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + ".";
  }
  schemaTableName += QgsPostgresConn::quotedIdentifier( tableName );

  QgsPostgresConn* conn = QgsPostgresConn::connectDb( dsUri.connectionInfo(), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // check the geometry column count
  QString sql = QString( "SELECT count(*) "
                         "FROM geometry_columns, pg_class, pg_namespace "
                         "WHERE f_table_name=relname AND f_table_schema=nspname "
                         "AND pg_class.relnamespace=pg_namespace.oid "
                         "AND f_table_schema=%1 AND f_table_name=%2" )
                .arg( QgsPostgresConn::quotedValue( schemaName ) )
                .arg( QgsPostgresConn::quotedValue( tableName ) );
  QgsPostgresResult result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName )
               .arg( result.PQresultErrorMessage() );
    conn->disconnect();
    return false;
  }

  int count = result.PQgetvalue( 0, 0 ).toInt();

  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    sql = QString( "SELECT DropGeometryColumn(%1,%2,%3)" )
          .arg( QgsPostgresConn::quotedValue( schemaName ) )
          .arg( QgsPostgresConn::quotedValue( tableName ) )
          .arg( QgsPostgresConn::quotedValue( geometryCol ) );
  }
  else
  {
    // drop the table
    sql = QString( "SELECT DropGeometryTable(%1,%2)" )
          .arg( QgsPostgresConn::quotedValue( schemaName ) )
          .arg( QgsPostgresConn::quotedValue( tableName ) );
  }

  result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName )
               .arg( result.PQresultErrorMessage() );
    conn->disconnect();
    return false;
  }

  conn->disconnect();
  return true;
}
