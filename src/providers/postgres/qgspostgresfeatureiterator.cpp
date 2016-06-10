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
    , mOrderByCompiled( false )
    , mLastFetch( false )
    , mFilterRequiresGeometry( false )
{
  if ( !source->mTransactionConnection )
  {
    mConn = QgsPostgresConnPool::instance()->acquireConnection( mSource->mConnInfo );
    mIsTransactionConnection = false;
  }
  else
  {
    mConn = source->mTransactionConnection;
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

  bool limitAtProvider = ( mRequest.limit() >= 0 );

  bool useFallbackWhereClause = false;
  QString fallbackWhereClause;

  if ( !request.filterRect().isNull() && !mSource->mGeometryColumn.isNull() )
  {
    whereClause = whereClauseRect();
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    whereClause = QgsPostgresUtils::andWhereClauses( whereClause, '(' + mSource->mSqlWhereClause + ')' );
  }

  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    QString fidWhereClause = QgsPostgresUtils::whereClause( mRequest.filterFid(), mSource->mFields, mConn, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );

    whereClause = QgsPostgresUtils::andWhereClauses( whereClause, fidWhereClause );
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterFids )
  {
    QString fidsWhereClause = QgsPostgresUtils::whereClause( mRequest.filterFids(), mSource->mFields, mConn, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );

    whereClause = QgsPostgresUtils::andWhereClauses( whereClause, fidsWhereClause );
  }
  else if ( request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      QgsAttributeList attrs = mRequest.subsetOfAttributes();
      Q_FOREACH ( const QString& field, request.filterExpression()->referencedColumns() )
      {
        int attrIdx = mSource->mFields.fieldNameIndex( field );
        if ( !attrs.contains( attrIdx ) )
          attrs << attrIdx;
      }
      mRequest.setSubsetOfAttributes( attrs );
    }
    mFilterRequiresGeometry = request.filterExpression()->needsGeometry();

    if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
    {
      //IMPORTANT - this MUST be the last clause added!
      QgsPostgresExpressionCompiler compiler = QgsPostgresExpressionCompiler( source );

      if ( compiler.compile( request.filterExpression() ) == QgsSqlExpressionCompiler::Complete )
      {
        useFallbackWhereClause = true;
        fallbackWhereClause = whereClause;
        whereClause = QgsPostgresUtils::andWhereClauses( whereClause, compiler.result() );
        mExpressionCompiled = true;
        mCompileStatus = Compiled;
      }
      else
      {
        limitAtProvider = false;
      }
    }
    else
    {
      limitAtProvider = false;
    }
  }

  QStringList orderByParts;

  mOrderByCompiled = true;

  // THIS CODE IS BROKEN - since every retrieved column is cast as text during declareCursor, this method of sorting will always be
  // performed using a text sort.
  // TODO - fix ordering by so that instead of
  //     SELECT my_int_col::text FROM some_table ORDER BY my_int_col
  // we instead use
  //     SELECT my_int_col::text FROM some_table ORDER BY some_table.my_int_col
  // but that's non-trivial
#if 0
  if ( QSettings().value( "/qgis/compileExpressions", true ).toBool() )
  {
    Q_FOREACH ( const QgsFeatureRequest::OrderByClause& clause, request.orderBy() )
    {
      QgsPostgresExpressionCompiler compiler = QgsPostgresExpressionCompiler( source );
      QgsExpression expression = clause.expression();
      if ( compiler.compile( &expression ) == QgsSqlExpressionCompiler::Complete )
      {
        QString part;
        part = compiler.result();
        part += clause.ascending() ? " ASC" : " DESC";
        part += clause.nullsFirst() ? " NULLS FIRST" : " NULLS LAST";
        orderByParts << part;
      }
      else
      {
        // Bail out on first non-complete compilation.
        // Most important clauses at the beginning of the list
        // will still be sent and used to pre-sort so the local
        // CPU can use its cycles for fine-tuning.
        mOrderByCompiled = false;
        break;
      }
    }
  }
  else
#endif
  {
    mOrderByCompiled = false;
  }

  if ( !mOrderByCompiled )
    limitAtProvider = false;

  bool success = declareCursor( whereClause, limitAtProvider ? mRequest.limit() : -1, false, orderByParts.join( "," ) );
  if ( !success && useFallbackWhereClause )
  {
    //try with the fallback where clause, eg for cases when using compiled expression failed to prepare
    success = declareCursor( fallbackWhereClause, -1, false, orderByParts.join( "," ) );
    if ( success )
      mExpressionCompiled = false;
  }

  if ( !success && !orderByParts.isEmpty() )
  {
    //try with no order by clause
    success = declareCursor( whereClause, -1, false );
    if ( success )
      mOrderByCompiled = false;
  }

  if ( !success && useFallbackWhereClause && !orderByParts.isEmpty() )
  {
    //try with no expression compilation AND no order by clause
    success = declareCursor( fallbackWhereClause, -1, false );
    if ( success )
    {
      mExpressionCompiled = false;
      mOrderByCompiled = false;
    }
  }

  if ( !success )
  {
    close();
    mClosed = true;
    iteratorClosed();
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

  if ( mFeatureQueue.empty() && !mLastFetch )
  {
    QString fetch = QString( "FETCH FORWARD %1 FROM %2" ).arg( mFeatureQueueSize ).arg( mCursorName );
    QgsDebugMsgLevel( QString( "fetching %1 features." ).arg( mFeatureQueueSize ), 4 );

    lock();
    if ( mConn->PQsendQuery( fetch ) == 0 ) // fetch features asynchronously
    {
      QgsMessageLog::logMessage( QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName, mConn->PQerrorMessage() ), QObject::tr( "PostGIS" ) );
    }

    QgsPostgresResult queryResult;
    for ( ;; )
    {
      queryResult = mConn->PQgetResult();
      if ( !queryResult.result() )
        break;

      if ( queryResult.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage( QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName, mConn->PQerrorMessage() ), QObject::tr( "PostGIS" ) );
        break;
      }

      int rows = queryResult.PQntuples();
      if ( rows == 0 )
        continue;

      mLastFetch = rows < mFeatureQueueSize;

      for ( int row = 0; row < rows; row++ )
      {
        mFeatureQueue.enqueue( QgsFeature() );
        getFeature( queryResult, row, mFeatureQueue.back() );
      } // for each row in queue
    }
    unlock();
  }

  if ( mFeatureQueue.empty() )
  {
    QgsDebugMsg( QString( "Finished after %1 features" ).arg( mFetched ) );
    close();

    mSource->mShared->ensureFeaturesCountedAtLeast( mFetched );

    return false;
  }

  feature = mFeatureQueue.dequeue();
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

bool QgsPostgresFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

void QgsPostgresFeatureIterator::lock()
{
  if ( mIsTransactionConnection )
    mConn->lock();
}

void QgsPostgresFeatureIterator::unlock()
{
  if ( mIsTransactionConnection )
    mConn->unlock();
}

bool QgsPostgresFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // move cursor to first record

  lock();
  mConn->PQexecNR( QString( "move absolute 0 in %1" ).arg( mCursorName ) );
  unlock();
  mFeatureQueue.clear();
  mFetched = 0;
  mLastFetch = false;

  return true;
}

bool QgsPostgresFeatureIterator::close()
{
  if ( !mConn )
    return false;

  lock();
  mConn->closeCursor( mCursorName );
  unlock();

  if ( !mIsTransactionConnection )
  {
    QgsPostgresConnPool::instance()->releaseConnection( mConn );
  }
  mConn = nullptr;

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
           .arg( rect.asWktCoordinates(),
                 mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );
  }
  else
  {
    qBox = QString( "st_makeenvelope(%1,%2,%3,%4,%5)" )
           .arg( qgsDoubleToString( rect.xMinimum() ),
                 qgsDoubleToString( rect.yMinimum() ),
                 qgsDoubleToString( rect.xMaximum() ),
                 qgsDoubleToString( rect.yMaximum() ),
                 mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );
  }

  bool castToGeometry = mSource->mSpatialColType == sctGeography ||
                        mSource->mSpatialColType == sctPcPatch;

  QString whereClause = QString( "%1%2 && %3" )
                        .arg( QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                              castToGeometry ? "::geometry" : "",
                              qBox );

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    QString curveToLineFn; // in postgis < 1.5 the st_curvetoline function does not exist
    if ( mConn->majorVersion() >= 2 || ( mConn->majorVersion() == 1 && mConn->minorVersion() >= 5 ) )
      curveToLineFn = "st_curvetoline"; // st_ prefix is always used
    whereClause += QString( " AND %1(%2(%3%4),%5)" )
                   .arg( mConn->majorVersion() < 2 ? "intersects" : "st_intersects",
                         curveToLineFn,
                         QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                         castToGeometry ? "::geometry" : "",
                         qBox );
  }

  if ( !mSource->mRequestedSrid.isEmpty() && ( mSource->mRequestedSrid != mSource->mDetectedSrid || mSource->mRequestedSrid.toInt() == 0 ) )
  {
    whereClause += QString( " AND %1(%2%3)=%4" )
                   .arg( mConn->majorVersion() < 2 ? "srid" : "st_srid",
                         QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                         castToGeometry ? "::geometry" : "",
                         mSource->mRequestedSrid );
  }

  if ( mSource->mRequestedGeomType != QGis::WKBUnknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    whereClause += QString( " AND %1" ).arg( QgsPostgresConn::postgisTypeFilter( mSource->mGeometryColumn, ( QgsWKBTypes::Type )mSource->mRequestedGeomType, castToGeometry ) );
  }

  return whereClause;
}



bool QgsPostgresFeatureIterator::declareCursor( const QString& whereClause, long limit, bool closeOnFail, const QString& orderBy )
{
  mFetchGeometry = ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) || mFilterRequiresGeometry ) && !mSource->mGeometryColumn.isNull();
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
                   : "st_force_2d",
                   geom );
    }

    QGis::WkbType usedGeomType = mSource->mRequestedGeomType != QGis::WKBUnknown
                                 ? mSource->mRequestedGeomType : mSource->mDetectedGeomType;

    if ( !mRequest.simplifyMethod().forceLocalOptimization() &&
         mRequest.simplifyMethod().methodType() != QgsSimplifyMethod::NoSimplification &&
         QGis::flatType( QGis::singleType( usedGeomType ) ) != QGis::WKBPoint &&
         !QgsWKBTypes::isCurvedType( QGis::fromOldWkbType( usedGeomType ) ) )
    {
      // PostGIS simplification method to use
      QString simplifyPostgisMethod;

      // Simplify again with st_simplify after first simplification ?
      bool postSimplification;
      postSimplification = false; // default to false. Set to true only for postgis >= 2.2 when using st_removerepeatedpoints

      if ( mRequest.simplifyMethod().methodType() == QgsSimplifyMethod::OptimizeForRendering )
      {
        // Optimize simplification for rendering
        if ( mConn->majorVersion() < 2 )
        {
          simplifyPostgisMethod = "snaptogrid";
        }
        else
        {

          // Default to st_snaptogrid
          simplifyPostgisMethod = "st_snaptogrid";

          if (( mConn->majorVersion() == 2 && mConn->minorVersion() >= 2 ) ||
              mConn->majorVersion() > 2 )
          {
            // For postgis >= 2.2 Use ST_RemoveRepeatedPoints instead
            // Do it only if threshold is <= 1 pixel to avoid holes in adjacent polygons
            // We should perhaps use it always for Linestrings, even if threshold > 1 ?
            if ( mRequest.simplifyMethod().threshold() <= 1.0 )
            {
              simplifyPostgisMethod = "st_removerepeatedpoints";
              postSimplification = true; // Ask to apply a post-filtering simplification
            }
          }
        }
      }
      else
      {
        // preserve topology
        if ( mConn->majorVersion() < 2 )
        {
          simplifyPostgisMethod = "simplifypreservetopology";
        }
        else
        {
          simplifyPostgisMethod = "st_simplifypreservetopology";
        }
      }
      QgsDebugMsg(
        QString( "PostGIS Server side simplification : threshold %1 pixels - method %2" )
        .arg( mRequest.simplifyMethod().threshold() )
        .arg( simplifyPostgisMethod )
      );

      geom = QString( "%1(%2,%3)" )
             .arg( simplifyPostgisMethod, geom )
             .arg( mRequest.simplifyMethod().tolerance() * 0.8 ); //-> Default factor for the maximum displacement distance for simplification, similar as GeoServer does

      // Post-simplification
      if ( postSimplification )
      {
        geom = QString( "st_simplify( %1, %2, true )" )
               .arg( geom )
               .arg( mRequest.simplifyMethod().tolerance() * 0.7 ); //-> We use a smaller tolerance than pre-filtering to be on the safe side
      }
    }

    geom = QString( "%1(%2,'%3')" )
           .arg( mConn->majorVersion() < 2 ? "asbinary" : "st_asbinary",
                 geom,
                 QgsPostgresProvider::endianString() );

    query += delim + geom;
    delim = ',';
  }

  switch ( mSource->mPrimaryKeyType )
  {
    case pktOid:
      query += delim + "oid";
      delim = ',';
      break;

    case pktTid:
      query += delim + "ctid";
      delim = ',';
      break;

    case pktInt:
    case pktUint64:
      query += delim + QgsPostgresConn::quotedIdentifier( mSource->mFields.at( mSource->mPrimaryKeyAttrs.at( 0 ) ).name() );
      delim = ',';
      break;

    case pktFidMap:
      Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
      {
        query += delim + mConn->fieldExpression( mSource->mFields.at( idx ) );
        delim = ',';
      }
      break;

    case pktUnknown:
      QgsDebugMsg( "Cannot declare cursor without primary key." );
      return false;
  }

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  Q_FOREACH ( int idx, subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList() )
  {
    if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
      continue;

    query += delim + mConn->fieldExpression( mSource->mFields.at( idx ) );
  }

  query += " FROM " + mSource->mQuery;

  if ( !whereClause.isEmpty() )
    query += QString( " WHERE %1" ).arg( whereClause );

  if ( limit >= 0 )
    query += QString( " LIMIT %1" ).arg( limit );

  if ( !orderBy.isEmpty() )
    query += QString( " ORDER BY %1 " ).arg( orderBy );

  lock();
  if ( !mConn->openCursor( mCursorName, query ) )
  {
    unlock();
    // reloading the fields might help next time around
    // TODO how to cleanly force reload of fields?  P->loadFields();
    if ( closeOnFail )
      close();
    return false;
  }
  unlock();

  mLastFetch = false;
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

      unsigned int wkbType;
      memcpy( &wkbType, featureGeom + 1, sizeof( wkbType ) );
      QgsWKBTypes::Type newType = QgsPostgresConn::wkbTypeFromOgcWkbType( wkbType );

      if (( unsigned int )newType != wkbType )
      {
        // overwrite type
        unsigned int n = newType;
        memcpy( featureGeom + 1, &n, sizeof( n ) );
      }

      // PostGIS stores TIN as a collection of Triangles.
      // Since Triangles are not supported, they have to be converted to Polygons
      const int nDims = 2 + ( QgsWKBTypes::hasZ( newType ) ? 1 : 0 ) + ( QgsWKBTypes::hasM( newType ) ? 1 : 0 );
      if ( wkbType % 1000 == 16 )
      {
        unsigned int numGeoms;
        memcpy( &numGeoms, featureGeom + 5, sizeof( unsigned int ) );
        unsigned char *wkb = featureGeom + 9;
        for ( unsigned int i = 0; i < numGeoms; ++i )
        {
          const unsigned int localType = QgsWKBTypes::singleType( newType ); // polygon(Z|M)
          memcpy( wkb + 1, &localType, sizeof( localType ) );

          // skip endian and type info
          wkb += sizeof( unsigned int ) + 1;

          // skip coordinates
          unsigned int nRings;
          memcpy( &nRings, wkb, sizeof( int ) );
          wkb += sizeof( int );
          for ( unsigned int j = 0; j < nRings; ++j )
          {
            unsigned int nPoints;
            memcpy( &nPoints, wkb, sizeof( int ) );
            wkb += sizeof( nPoints ) + sizeof( double ) * nDims * nPoints;
          }
        }
      }

      QgsGeometry *g = new QgsGeometry();
      g->fromWkb( featureGeom, returnedLength + 1 );
      feature.setGeometry( g );
    }
    else
    {
      feature.setGeometry( nullptr );
    }

    col++;
  }

  QgsFeatureId fid = 0;

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList fetchAttributes = mRequest.subsetOfAttributes();

  switch ( mSource->mPrimaryKeyType )
  {
    case pktOid:
    case pktTid:
      fid = mConn->getBinaryInt( queryResult, row, col++ );
      break;

    case pktInt:
    case pktUint64:
      fid = mConn->getBinaryInt( queryResult, row, col++ );
      if ( !subsetOfAttributes || fetchAttributes.contains( mSource->mPrimaryKeyAttrs.at( 0 ) ) )
      {
        feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
      }
      if ( mSource->mPrimaryKeyType == pktInt )
      {
        // NOTE: this needs be done _after_ the setAttribute call
        // above as we want the attribute value to be 1:1 with
        // database value
        fid = QgsPostgresUtils::int32pk_to_fid( fid );
      }
      break;

    case pktFidMap:
    {
      QList<QVariant> primaryKeyVals;

      Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
      {
        const QgsField &fld = mSource->mFields.at( idx );

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
    Q_FOREACH ( int idx, fetchAttributes )
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

  QVariant v = QgsPostgresProvider::convertValue( mSource->mFields.at( idx ).type(), queryResult.PQgetvalue( row, col ) );
  feature.setAttribute( idx, v );

  col++;
}


//  ------------------

QgsPostgresFeatureSource::QgsPostgresFeatureSource( const QgsPostgresProvider* p )
    : mConnInfo( p->mUri.connectionInfo( false ) )
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
    mTransactionConnection = nullptr;
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
