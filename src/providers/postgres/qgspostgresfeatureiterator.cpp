/***************************************************************************
    qgspostgresfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgeometry.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresexpressioncompiler.h"
#include "qgspostgresfeatureiterator.h"
#include "qgspostgresprovider.h"
#include "qgspostgrestransaction.h"

#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QObject>
#include <QSettings>


const int QgsPostgresFeatureIterator::sFeatureQueueSize = 2000;


QgsPostgresFeatureIterator::QgsPostgresFeatureIterator( QgsPostgresFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsPostgresFeatureSource>( source, ownSource, request )
    , mFeatureQueueSize( sFeatureQueueSize )
    , mFetched( 0 )
    , mFetchGeometry( false )
    , mExpressionCompiled( false )
{
  if ( !source->mTransactionConnection )
  {
    mConn = QgsPostgresConnPool::instance()->acquireConnection( mSource->mConnInfo );
    mIsTransactionConnection = false;
  }
  else
  {
    mConn = source->mTransactionConnection;
    mConn->lock();
    mIsTransactionConnection = true;
  }

  if ( !mConn )
  {
    mClosed = true;
    iteratorClosed();
    return;
  }

  mCursorName = mConn->uniqueCursorName();
  QString whereClause;

  if ( request.filterType() == QgsFeatureRequest::FilterRect && !mSource->mGeometryColumn.isNull() )
  {
    whereClause = whereClauseRect();
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    whereClause = QgsPostgresUtils::whereClause( mRequest.filterFid(), mSource->mFields, mConn, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFids )
  {
    whereClause = QgsPostgresUtils::whereClause( mRequest.filterFids(), mSource->mFields, mConn, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterExpression
            && QSettings().value( "/qgis/postgres/compileExpressions", false ).toBool() )
  {
    QgsPostgresExpressionCompiler compiler = QgsPostgresExpressionCompiler( source );

    if ( compiler.compile( request.filterExpression() ) == QgsPostgresExpressionCompiler::Complete )
    {
      whereClause = compiler.result();
      mExpressionCompiled = true;
    }
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + mSource->mSqlWhereClause + ")";
  }

  if ( !declareCursor( whereClause ) )
  {
    mClosed = true;
    iteratorClosed();
    return;
  }

  mFetched = 0;
}


QgsPostgresFeatureIterator::~QgsPostgresFeatureIterator()
{
  close();
}


bool QgsPostgresFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mFeatureQueue.empty() )
  {
    QString fetch = QString( "FETCH FORWARD %1 FROM %2" ).arg( mFeatureQueueSize ).arg( mCursorName );
    QgsDebugMsgLevel( QString( "fetching %1 features." ).arg( mFeatureQueueSize ), 4 );
    if ( mConn->PQsendQuery( fetch ) == 0 ) // fetch features asynchronously
    {
      QgsMessageLog::logMessage( QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName ).arg( mConn->PQerrorMessage() ), QObject::tr( "PostGIS" ) );
    }

    QgsPostgresResult queryResult;
    for ( ;; )
    {
      queryResult = mConn->PQgetResult();
      if ( !queryResult.result() )
        break;

      if ( queryResult.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName ).arg( mConn->PQerrorMessage() ), QObject::tr( "PostGIS" ) );
        break;
      }

      int rows = queryResult.PQntuples();
      if ( rows == 0 )
        continue;

      for ( int row = 0; row < rows; row++ )
      {
        mFeatureQueue.enqueue( QgsFeature() );
        getFeature( queryResult, row, mFeatureQueue.back() );
      } // for each row in queue
    }
  }

  if ( mFeatureQueue.empty() )
  {
    QgsDebugMsg( QString( "Finished after %1 features" ).arg( mFetched ) );
    close();

    mSource->mShared->ensureFeaturesCountedAtLeast( mFetched );

    return false;
  }

  // Now return the next feature from the queue
  if ( !mFetchGeometry )
  {
    feature.setGeometryAndOwnership( 0, 0 );
  }
  else
  {
    feature.setGeometry( mFeatureQueue.front().geometryAndOwnership() );
  }
  feature.setFeatureId( mFeatureQueue.front().id() );
  feature.setAttributes( mFeatureQueue.front().attributes() );

  mFeatureQueue.dequeue();
  mFetched++;

  feature.setValid( true );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  return true;
}

bool QgsPostgresFeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsPostgresFeatureIterator::prepareSimplification( const QgsSimplifyMethod& simplifyMethod )
{
  // setup simplification of geometries to fetch
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) &&
       simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification &&
       !simplifyMethod.forceLocalOptimization() )
  {
    QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();

    if ( methodType == QgsSimplifyMethod::OptimizeForRendering || methodType == QgsSimplifyMethod::PreserveTopology )
    {
      return true;
    }
    else
    {
      QgsDebugMsg( QString( "Simplification method type (%1) is not recognised by PostgresFeatureIterator" ).arg( methodType ) );
    }
  }
  return QgsAbstractFeatureIterator::prepareSimplification( simplifyMethod );
}

bool QgsPostgresFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  return methodType == QgsSimplifyMethod::OptimizeForRendering || methodType == QgsSimplifyMethod::PreserveTopology;
}

bool QgsPostgresFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // move cursor to first record
  mConn->PQexecNR( QString( "move absolute 0 in %1" ).arg( mCursorName ) );
  mFeatureQueue.clear();
  mFetched = 0;

  return true;
}

bool QgsPostgresFeatureIterator::close()
{
  if ( mClosed )
    return false;

  mConn->closeCursor( mCursorName );

  if ( !mIsTransactionConnection )
  {
    QgsPostgresConnPool::instance()->releaseConnection( mConn );
  }
  else
  {
    mConn->unlock();
  }
  mConn = 0;

  while ( !mFeatureQueue.empty() )
  {
    mFeatureQueue.dequeue();
  }

  iteratorClosed();

  mClosed = true;
  return true;
}

///////////////

QString QgsPostgresFeatureIterator::whereClauseRect()
{
  QgsRectangle rect = mRequest.filterRect();
  if ( mSource->mSpatialColType == sctGeography )
  {
    rect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 ).intersect( &rect );
  }

  if ( !rect.isFinite() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "PostGIS" ) );
    return "false";
  }

  QString qBox;
  if ( mConn->majorVersion() < 2 )
  {
    qBox = QString( "setsrid('BOX3D(%1)'::box3d,%2)" )
           .arg( rect.asWktCoordinates() )
           .arg( mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );
  }
  else
  {
    qBox = QString( "st_makeenvelope(%1,%2,%3,%4,%5)" )
           .arg( qgsDoubleToString( rect.xMinimum() ) )
           .arg( qgsDoubleToString( rect.yMinimum() ) )
           .arg( qgsDoubleToString( rect.xMaximum() ) )
           .arg( qgsDoubleToString( rect.yMaximum() ) )
           .arg( mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );
  }

  QString whereClause = QString( "%1%2 && %3" )
                        .arg( QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ) )
                        .arg( mSource->mSpatialColType == sctPcPatch ? "::geometry" : "" )
                        .arg( qBox );

  bool castToGeometry = mSource->mSpatialColType == sctGeography ||
                        mSource->mSpatialColType == sctPcPatch;

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    QString curveToLineFn; // in postgis < 1.5 the st_curvetoline function does not exist
    if ( mConn->majorVersion() >= 2 || ( mConn->majorVersion() == 1 && mConn->minorVersion() >= 5 ) )
      curveToLineFn = "st_curvetoline"; // st_ prefix is always used
    whereClause += QString( " AND %1(%2(%3%4),%5)" )
                   .arg( mConn->majorVersion() < 2 ? "intersects" : "st_intersects" )
                   .arg( curveToLineFn )
                   .arg( QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ) )
                   .arg( castToGeometry ? "::geometry" : "" )
                   .arg( qBox );
  }

  if ( !mSource->mRequestedSrid.isEmpty() && ( mSource->mRequestedSrid != mSource->mDetectedSrid || mSource->mRequestedSrid.toInt() == 0 ) )
  {
    whereClause += QString( " AND %1(%2%3)=%4" )
                   .arg( mConn->majorVersion() < 2 ? "srid" : "st_srid" )
                   .arg( QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ) )
                   .arg( castToGeometry ? "::geometry" : "" )
                   .arg( mSource->mRequestedSrid );
  }

  if ( mSource->mRequestedGeomType != QGis::WKBUnknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    whereClause += QString( " AND %1" ).arg( QgsPostgresConn::postgisTypeFilter( mSource->mGeometryColumn, ( QgsWKBTypes::Type )mSource->mRequestedGeomType, castToGeometry ) );
  }

  return whereClause;
}



bool QgsPostgresFeatureIterator::declareCursor( const QString& whereClause )
{
  mFetchGeometry = !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && !mSource->mGeometryColumn.isNull();

#if 0
  // TODO: check that all field indexes exist
  if ( !hasAllFields )
  {
    rewind();
    return false;
  }
#endif

  QString query( "SELECT " ), delim( "" );

  if ( mFetchGeometry )
  {
    QString geom = QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn );

    if ( mSource->mSpatialColType == sctGeography ||
         mSource->mSpatialColType == sctPcPatch )
      geom += "::geometry";

    if ( mSource->mForce2d )
    {
      geom = QString( "%1(%2)" )
             // Force_2D before 2.0
             .arg( mConn->majorVersion() < 2 ? "force_2d"
                   // ST_Force2D since 2.1.0
                   : mConn->majorVersion() > 2 || mConn->minorVersion() > 0 ? "st_force2d"
                   // ST_Force_2D in 2.0.x
                   : "st_force_2d" )
             .arg( geom );
    }

    if ( !mRequest.simplifyMethod().forceLocalOptimization() &&
         mRequest.simplifyMethod().methodType() != QgsSimplifyMethod::NoSimplification &&
         QGis::flatType( QGis::singleType( mSource->mRequestedGeomType != QGis::WKBUnknown
                                           ? mSource->mRequestedGeomType
                                           : mSource->mDetectedGeomType ) ) != QGis::WKBPoint )
    {
      geom = QString( "%1(%2,%3)" )
             .arg( mRequest.simplifyMethod().methodType() == QgsSimplifyMethod::OptimizeForRendering
                   ? ( mConn->majorVersion() < 2 ? "snaptogrid" : "st_snaptogrid" )
                       : ( mConn->majorVersion() < 2 ? "simplifypreservetopology" : "st_simplifypreservetopology" ) )
                 .arg( geom )
                 .arg( mRequest.simplifyMethod().tolerance() * 0.8 ); //-> Default factor for the maximum displacement distance for simplification, similar as GeoServer does
    }

    geom = QString( "%1(%2,'%3')" )
           .arg( mConn->majorVersion() < 2 ? "asbinary" : "st_asbinary" )
           .arg( geom )
           .arg( QgsPostgresProvider::endianString() );

    query += delim + geom;
    delim = ",";
  }

  switch ( mSource->mPrimaryKeyType )
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
      query += delim + QgsPostgresConn::quotedIdentifier( mSource->mFields[ mSource->mPrimaryKeyAttrs[0] ].name() );
      delim = ",";
      break;

    case pktFidMap:
      foreach ( int idx, mSource->mPrimaryKeyAttrs )
      {
        query += delim + mConn->fieldExpression( mSource->mFields[idx] );
        delim = ",";
      }
      break;

    case pktUnknown:
      QgsDebugMsg( "Cannot declare cursor without primary key." );
      return false;
      break;
  }

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  foreach ( int idx, subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList() )
  {
    if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
      continue;

    query += delim + mConn->fieldExpression( mSource->mFields[idx] );
  }

  query += " FROM " + mSource->mQuery;

  if ( !whereClause.isEmpty() )
    query += QString( " WHERE %1" ).arg( whereClause );

  if ( !mConn->openCursor( mCursorName, query ) )
  {

    // reloading the fields might help next time around
    // TODO how to cleanly force reload of fields?  P->loadFields();
    close();
    return false;
  }

  return true;
}


bool QgsPostgresFeatureIterator::getFeature( QgsPostgresResult &queryResult, int row, QgsFeature &feature )
{
  feature.initAttributes( mSource->mFields.count() );

  int col = 0;

  if ( mFetchGeometry )
  {
    int returnedLength = ::PQgetlength( queryResult.result(), row, col );
    if ( returnedLength > 0 )
    {
      unsigned char *featureGeom = new unsigned char[returnedLength + 1];
      memcpy( featureGeom, PQgetvalue( queryResult.result(), row, col ), returnedLength );
      memset( featureGeom + returnedLength, 0, 1 );

      // modify 2.5D WKB types to make them compliant with OGR
      unsigned int wkbType;
      memcpy( &wkbType, featureGeom + 1, sizeof( wkbType ) );
      wkbType = QgsPostgresConn::wkbTypeFromOgcWkbType( wkbType );
      memcpy( featureGeom + 1, &wkbType, sizeof( wkbType ) );

      // change wkb type of inner geometries
      if ( wkbType == QGis::WKBMultiPoint25D ||
           wkbType == QGis::WKBMultiLineString25D ||
           wkbType == QGis::WKBMultiPolygon25D )
      {
        unsigned int numGeoms;
        memcpy( &numGeoms, featureGeom + 5, sizeof( unsigned int ) );
        unsigned char *wkb = featureGeom + 9;
        for ( unsigned int i = 0; i < numGeoms; ++i )
        {
          unsigned int localType;
          memcpy( &localType, wkb + 1, sizeof( localType ) );
          localType = QgsPostgresConn::wkbTypeFromOgcWkbType( localType );
          memcpy( wkb + 1, &localType, sizeof( localType ) );

          // skip endian and type info
          wkb += sizeof( unsigned int ) + 1;

          // skip coordinates
          switch ( wkbType )
          {
            case QGis::WKBMultiPoint25D:
              wkb += sizeof( double ) * 3;
              break;
            case QGis::WKBMultiLineString25D:
            {
              unsigned int nPoints;
              memcpy( &nPoints, wkb, sizeof( int ) );
              wkb += sizeof( int ) + sizeof( double ) * 3 * nPoints;
            }
            break;
            default:
            case QGis::WKBMultiPolygon25D:
            {
              unsigned int nRings;
              memcpy( &nRings, wkb, sizeof( int ) );
              wkb += sizeof( int );
              for ( unsigned int j = 0; j < nRings; ++j )
              {
                unsigned int nPoints;
                memcpy( &nPoints, wkb, sizeof( int ) );
                wkb += sizeof( nPoints ) + sizeof( double ) * 3 * nPoints;
              }
            }
            break;
          }
        }
      }

      feature.setGeometryAndOwnership( featureGeom, returnedLength + 1 );
    }
    else
    {
      feature.setGeometryAndOwnership( 0, 0 );
    }

    col++;
  }

  QgsFeatureId fid = 0;

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  const QgsAttributeList& fetchAttributes = mRequest.subsetOfAttributes();

  switch ( mSource->mPrimaryKeyType )
  {
    case pktOid:
    case pktTid:
    case pktInt:
      fid = mConn->getBinaryInt( queryResult, row, col++ );
      if ( mSource->mPrimaryKeyType == pktInt &&
           ( !subsetOfAttributes || fetchAttributes.contains( mSource->mPrimaryKeyAttrs[0] ) ) )
        feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
      break;

    case pktFidMap:
    {
      QList<QVariant> primaryKeyVals;

      foreach ( int idx, mSource->mPrimaryKeyAttrs )
      {
        const QgsField &fld = mSource->mFields[idx];

        QVariant v = QgsPostgresProvider::convertValue( fld.type(), queryResult.PQgetvalue( row, col ) );
        primaryKeyVals << v;

        if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
          feature.setAttribute( idx, v );

        col++;
      }

      fid = mSource->mShared->lookupFid( QVariant( primaryKeyVals ) );

    }
    break;

    case pktUnknown:
      Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
      return false;
  }

  feature.setFeatureId( fid );
  QgsDebugMsgLevel( QString( "fid=%1" ).arg( fid ), 4 );

  // iterate attributes
  if ( subsetOfAttributes )
  {
    foreach ( int idx, fetchAttributes )
      getFeatureAttribute( idx, queryResult, row, col, feature );
  }
  else
  {
    for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
      getFeatureAttribute( idx, queryResult, row, col, feature );
  }

  return true;
}

void QgsPostgresFeatureIterator::getFeatureAttribute( int idx, QgsPostgresResult& queryResult, int row, int& col, QgsFeature& feature )
{
  if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
    return;

  QVariant v = QgsPostgresProvider::convertValue( mSource->mFields[idx].type(), queryResult.PQgetvalue( row, col ) );
  feature.setAttribute( idx, v );

  col++;
}


//  ------------------

QgsPostgresFeatureSource::QgsPostgresFeatureSource( const QgsPostgresProvider* p )
    : mConnInfo( p->mUri.connectionInfo() )
    , mGeometryColumn( p->mGeometryColumn )
    , mFields( p->mAttributeFields )
    , mSpatialColType( p->mSpatialColType )
    , mRequestedSrid( p->mRequestedSrid )
    , mDetectedSrid( p->mDetectedSrid )
    , mForce2d( p->mForce2d )
    , mRequestedGeomType( p->mRequestedGeomType )
    , mDetectedGeomType( p->mDetectedGeomType )
    , mPrimaryKeyType( p->mPrimaryKeyType )
    , mPrimaryKeyAttrs( p->mPrimaryKeyAttrs )
    , mQuery( p->mQuery )
    , mShared( p->mShared )
{
  mSqlWhereClause = p->filterWhereClause();

  if ( mSqlWhereClause.startsWith( " WHERE " ) )
    mSqlWhereClause = mSqlWhereClause.mid( 7 );

  if ( p->mTransaction )
  {
    mTransactionConnection = p->mTransaction->connection();
    mTransactionConnection->ref();
  }
  else
  {
    mTransactionConnection = 0;
  }
}

QgsPostgresFeatureSource::~QgsPostgresFeatureSource()
{
  if ( mTransactionConnection )
  {
    mTransactionConnection->unref();
  }
}

QgsFeatureIterator QgsPostgresFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsPostgresFeatureIterator( this, false, request ) );
}
