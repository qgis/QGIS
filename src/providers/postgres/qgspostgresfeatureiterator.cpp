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
#include "qgsdbquerylog.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsexception.h"
#include "qgsgeometryengine.h"

#include <QElapsedTimer>
#include <QObject>

QgsPostgresFeatureIterator::QgsPostgresFeatureIterator( QgsPostgresFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsPostgresFeatureSource>( source, ownSource, request )
{
  if ( request.filterType() == QgsFeatureRequest::FilterFids && request.filterFids().isEmpty() )
  {
    mClosed = true;
    iteratorClosed();
    return;
  }

  if ( !source->mTransactionConnection )
  {
    mConn = QgsPostgresConnPool::instance()->acquireConnection( mSource->mConnInfo, request.timeout(), request.requestMayBeNested() );
    mIsTransactionConnection = false;
  }
  else
  {
    mConn = source->mTransactionConnection;
    mIsTransactionConnection = true;
  }

  if ( !mConn || mConn->PQstatus() != CONNECTION_OK )
  {
    mValid = false;
    mClosed = true;
    iteratorClosed();
    return;
  }

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  bool limitAtProvider = ( mRequest.limit() >= 0 );

  mCursorName = mConn->uniqueCursorName();
  QString whereClause;

  bool useFallbackWhereClause = false;
  QString fallbackWhereClause;

  if ( !mFilterRect.isNull() && !mSource->mGeometryColumn.isNull() )
  {
    whereClause = whereClauseRect();
  }

  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      // we only need to test the distance locally if we are transforming features on QGIS side, otherwise
      // we use ST_DWithin on the postgres backend instead
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        if ( !mTransform.isShortCircuited() || mSource->mSpatialColType != SctGeometry )
        {
          mDistanceWithinGeom = mRequest.referenceGeometry();
          mDistanceWithinEngine.reset( QgsGeometry::createGeometryEngine( mDistanceWithinGeom.constGet() ) );
          mDistanceWithinEngine->prepareGeometry();
          limitAtProvider = false;
        }
        else
        {
          // we can safely hand this off to the backend to evaluate, so that it will nicely handle it within the query planner!
          whereClause = QgsPostgresUtils::andWhereClauses( whereClause, QStringLiteral( "ST_DWithin(%1,ST_GeomFromText('%2',%3),%4)" ).arg(
                          QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                          mRequest.referenceGeometry().asWkt(),
                          mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid )
                        .arg( mRequest.distanceWithin() ) );
        }
      }
      break;
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
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += qgis::listToSet( attrs );
      mRequest.setSubsetOfAttributes( qgis::setToList( attributeIndexes ) );
    }
    mFilterRequiresGeometry = request.filterExpression()->needsGeometry();

    //IMPORTANT - this MUST be the last clause added!
    QgsPostgresExpressionCompiler compiler = QgsPostgresExpressionCompiler( source, request.flags() & QgsFeatureRequest::IgnoreStaticNodesDuringExpressionCompilation );

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

  if ( !mClosed )
  {
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
    if ( QgsSettings().value( "qgis/compileExpressions", true ).toBool() )
    {
      const auto constOrderBy = request.orderBy();
      for ( const QgsFeatureRequest::OrderByClause &clause : constOrderBy )
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
      mOrderByCompiled = mRequest.orderBy().isEmpty();
    }

    // ensure that all attributes required for order by are fetched
    if ( !mOrderByCompiled && mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      QgsAttributeList attrs = mRequest.subsetOfAttributes();
      const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
      for ( int attrIndex : usedAttributeIndices )
      {
        if ( !attrs.contains( attrIndex ) )
          attrs << attrIndex;
      }
      mRequest.setSubsetOfAttributes( attrs );
    }

    if ( !mOrderByCompiled )
      limitAtProvider = false;

    bool success = declareCursor( whereClause, limitAtProvider ? mRequest.limit() : -1, false, orderByParts.join( QLatin1Char( ',' ) ) );
    if ( !success && useFallbackWhereClause )
    {
      //try with the fallback where clause, e.g., for cases when using compiled expression failed to prepare
      success = declareCursor( fallbackWhereClause, -1, false, orderByParts.join( QLatin1Char( ',' ) ) );
      if ( success )
      {
        mExpressionCompiled = false;
        mCompileFailed = true;
      }
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
    }
  }

  mFetched = 0;
}


QgsPostgresFeatureIterator::~QgsPostgresFeatureIterator()
{
  close();
}


bool QgsPostgresFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  while ( true )
  {
    if ( mFeatureQueue.empty() && !mLastFetch )
    {
#if 0 //disabled dynamic queue size
      QElapsedTimer timer;
      timer.start();
#endif

      QString fetch = QStringLiteral( "FETCH FORWARD %1 FROM %2" ).arg( mFeatureQueueSize ).arg( mCursorName );
      QgsDebugMsgLevel( QStringLiteral( "fetching %1 features." ).arg( mFeatureQueueSize ), 4 );

      lock();

      QgsDatabaseQueryLogWrapper logWrapper { fetch, mSource->mConnInfo, QStringLiteral( "postgres" ), QStringLiteral( "QgsPostgresFeatureIterator" ), QGS_QUERY_LOG_ORIGIN };

      if ( mConn->PQsendQuery( fetch ) == 0 ) // fetch features asynchronously
      {
        const QString error { QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" ).arg( mCursorName, mConn->PQerrorMessage() ) };
        QgsMessageLog::logMessage( error, QObject::tr( "PostGIS" ) );
        logWrapper.setError( error );
      }

      QgsPostgresResult queryResult;
      long long fetchedRows { 0 };
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
        else
          fetchedRows += rows;

        mLastFetch = rows < mFeatureQueueSize;

        for ( int row = 0; row < rows; row++ )
        {
          mFeatureQueue.enqueue( QgsFeature() );
          getFeature( queryResult, row, mFeatureQueue.back() );
        } // for each row in queue
      }
      unlock();

      if ( fetchedRows > 0 )
      {
        logWrapper.setFetchedRows( fetchedRows );
      }

#if 0 //disabled dynamic queue size
      if ( timer.elapsed() > 500 && mFeatureQueueSize > 1 )
      {
        mFeatureQueueSize /= 2;
      }
      else if ( timer.elapsed() < 50 && mFeatureQueueSize < 10000 )
      {
        mFeatureQueueSize *= 2;
      }
#endif
    }

    if ( mFeatureQueue.empty() )
    {
      mLastFetch = true;
      break;
    }

    feature = mFeatureQueue.dequeue();
    mFetched++;

    geometryToDestinationCrs( feature, mTransform );
    if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
    {
      continue;
    }

    feature.setValid( true );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    return true;
  }
  QgsDebugMsgLevel( QStringLiteral( "Finished after %1 features" ).arg( mFetched ), 2 );
  close();

  mSource->mShared->ensureFeaturesCountedAtLeast( mFetched );

  return false;
}

bool QgsPostgresFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsPostgresFeatureIterator::prepareSimplification( const QgsSimplifyMethod &simplifyMethod )
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
      QgsDebugMsg( QStringLiteral( "Simplification method type (%1) is not recognised by PostgresFeatureIterator" ).arg( methodType ) );
    }
  }
  return QgsAbstractFeatureIterator::prepareSimplification( simplifyMethod );
}

bool QgsPostgresFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  return methodType == QgsSimplifyMethod::OptimizeForRendering || methodType == QgsSimplifyMethod::PreserveTopology;
}

bool QgsPostgresFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
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

  mConn->LoggedPQexecNR( "QgsPostgresFeatureIterator", QStringLiteral( "move absolute 0 in %1" ).arg( mCursorName ) );
  mFeatureQueue.clear();
  mFetched = 0;
  mLastFetch = false;

  return true;
}

bool QgsPostgresFeatureIterator::close()
{
  if ( !mConn )
    return false;

  mConn->closeCursor( mCursorName );

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
  QgsRectangle rect = mFilterRect;
  if ( mSource->mSpatialColType == SctGeography )
  {
    rect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 ).intersect( rect );
  }

  if ( !rect.isFinite() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "PostGIS" ) );
    return QStringLiteral( "false" );
  }

  QString qBox;
  const QString bboxSrid = mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid;
  if ( mConn->majorVersion() < 2 )
  {
    qBox = QStringLiteral( "setsrid('BOX3D(%1)'::box3d,%2)" )
           .arg( rect.asWktCoordinates(),
                 bboxSrid );
  }
  else
  {
    qBox = QStringLiteral( "st_makeenvelope(%1,%2,%3,%4,%5)" )
           .arg( qgsDoubleToString( rect.xMinimum() ),
                 qgsDoubleToString( rect.yMinimum() ),
                 qgsDoubleToString( rect.xMaximum() ),
                 qgsDoubleToString( rect.yMaximum() ),
                 bboxSrid );
  }

  bool castToGeometry = mSource->mSpatialColType == SctGeography ||
                        mSource->mSpatialColType == SctPcPatch;

  QString whereClause = QStringLiteral( "%1%2 && %3" )
                        .arg( QgsPostgresConn::quotedIdentifier( mSource->mBoundingBoxColumn ),
                              castToGeometry ? "::geometry" : "",
                              qBox );

  // For geography type, using a && filter with the geography column cast as
  // geometry prevents the use of a spatial index. So for "small" filtering
  // bounding boxes, use the filtering in the geography space. But a bbox in
  // geography uses geodesic arcs, and not rhumb lines, so we must expand a bit the QGIS
  // bounding box (which assumes rhumb lines) to the south in the northern hemisphere
  // See https://trac.osgeo.org/postgis/ticket/2495 for some background
  if ( mConn->majorVersion() >= 2 &&
       mSource->mSpatialColType == SctGeography &&
       bboxSrid == QLatin1String( "4326" ) &&
       std::fabs( rect.yMaximum()  - rect.yMinimum() ) <= 10 &&
       std::fabs( rect.xMaximum()  - rect.xMinimum() ) <= 10 &&
       std::fabs( rect.yMaximum() ) <= 70 )
  {
    /* The following magic constant has been obtained by running :
        #include "geodesic.h"
        #include <math.h>
        #include <stdio.h>

        int main()
        {
            struct geod_geodesic g;
            struct geod_geodesicline l;
            int i;
            geod_init(&g, 6378137, 1/298.257223563);
            double lat1 = 60;
            double lon1 = 0;
            double lat2 = 70;
            double lon2 = 10;
            geod_inverseline(&l, &g, lat1, lon1, lat2, lon2, 0);
            double maxdlat = 0;
            for (i = 0; i <= 100; ++i)
            {
                double lat, lon;
                geod_position(&l, i * l.s13 * 0.01, &lat, &lon, 0);
                double alpha = (lon - lon1) / (lon2 - lon1);
                double lat_rhumb = lat1 + (lat2 - lat1) * alpha;
                double dlat = lat - lat_rhumb;
                if( fabs(dlat) > fabs(maxdlat) )
                    maxdlat = dlat;
                //printf("%f: %f %f delta=%f\n", lon, lat, lat_rhumb, dlat);
            }
            printf("maxdlat = %f\n", maxdlat);
            return 0;
        }
    */
    // And noticing that the difference between the rhumb line and the geodesics
    // increases with higher latitude maximum, and differences of longitude and latitude.
    // We could perhaps get a formula that would give dlat as a function of
    // delta_lon, delta_lat and max(lat), but those maximum values should be good
    // enough for now.
    double dlat = 1.04;

    // For smaller filtering bounding box, use smaller bbox expansion
    if ( std::fabs( rect.yMaximum()  - rect.yMinimum() ) <= 1 &&
         std::fabs( rect.xMaximum()  - rect.xMinimum() ) <= 1 )
    {
      // Value got by changing lat1 to 69 and lon2 to 1 in the above code snippet
      dlat = 0.013;
    }
    // In the northern hemisphere, extends the geog bbox to the south
    const double yminGeog = rect.yMinimum() >= 0 ? std::max( 0.0, rect.yMinimum() - dlat ) : rect.yMinimum();
    const double ymaxGeog = rect.yMaximum() >= 0 ? rect.yMaximum() : std::min( 0.0, rect.yMaximum() + dlat );
    const QString qBoxGeog = QStringLiteral( "st_makeenvelope(%1,%2,%3,%4,%5)" )
                             .arg( qgsDoubleToString( rect.xMinimum() ),
                                   qgsDoubleToString( yminGeog ),
                                   qgsDoubleToString( rect.xMaximum() ),
                                   qgsDoubleToString( ymaxGeog ),
                                   bboxSrid );
    whereClause += QStringLiteral( " AND %1 && %2" )
                   .arg( QgsPostgresConn::quotedIdentifier( mSource->mBoundingBoxColumn ),
                         qBoxGeog );
  }

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    QString curveToLineFn; // in PostGIS < 1.5 the st_curvetoline function does not exist
    if ( mConn->majorVersion() >= 2 || ( mConn->majorVersion() == 1 && mConn->minorVersion() >= 5 ) )
      curveToLineFn = QStringLiteral( "st_curvetoline" ); // st_ prefix is always used
    whereClause += QStringLiteral( " AND %1(%2(%3%4),%5)" )
                   .arg( mConn->majorVersion() < 2 ? "intersects" : "st_intersects",
                         curveToLineFn,
                         QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                         castToGeometry ? "::geometry" : "",
                         qBox );
  }

  if ( !mSource->mRequestedSrid.isEmpty() && ( mSource->mRequestedSrid != mSource->mDetectedSrid || mSource->mRequestedSrid.toInt() == 0 ) )
  {
    whereClause += QStringLiteral( " AND %1(%2%3)=%4" )
                   .arg( mConn->majorVersion() < 2 ? "srid" : "st_srid",
                         QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                         castToGeometry ? "::geometry" : "",
                         mSource->mRequestedSrid );
  }

  if ( mSource->mRequestedGeomType != QgsWkbTypes::Unknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    whereClause += QStringLiteral( " AND %1" ).arg( QgsPostgresConn::postgisTypeFilter( mSource->mGeometryColumn, mSource->mRequestedGeomType, castToGeometry ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "whereClause = %1" ).arg( whereClause ), 4 );
  return whereClause;
}



bool QgsPostgresFeatureIterator::declareCursor( const QString &whereClause, long limit, bool closeOnFail, const QString &orderBy )
{
  mFetchGeometry = ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry )
                     || mFilterRequiresGeometry
                     || ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin && !mTransform.isShortCircuited() ) )
                   && !mSource->mGeometryColumn.isNull();
#if 0
  // TODO: check that all field indexes exist
  if ( !hasAllFields )
  {
    rewind();
    return false;
  }
#endif

  QString query( QStringLiteral( "SELECT " ) );
  QString delim;

  if ( mFetchGeometry )
  {
    QString geom = QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn );

    if ( mSource->mSpatialColType == SctGeography ||
         mSource->mSpatialColType == SctPcPatch )
      geom += QLatin1String( "::geometry" );

    QgsWkbTypes::Type usedGeomType = mSource->mRequestedGeomType != QgsWkbTypes::Unknown
                                     ? mSource->mRequestedGeomType : mSource->mDetectedGeomType;

    if ( !mRequest.simplifyMethod().forceLocalOptimization() &&
         mRequest.simplifyMethod().methodType() != QgsSimplifyMethod::NoSimplification &&
         QgsWkbTypes::flatType( QgsWkbTypes::singleType( usedGeomType ) ) != QgsWkbTypes::Point )
    {
      // PostGIS simplification method to use
      QString simplifyPostgisMethod;

      // Simplify again with st_simplify after first simplification ?
      bool postSimplification;
      postSimplification = false; // default to false. Set to true only for PostGIS >= 2.2 when using st_removerepeatedpoints

      if ( !QgsWkbTypes::isCurvedType( usedGeomType ) && mRequest.simplifyMethod().methodType() == QgsSimplifyMethod::OptimizeForRendering )
      {
        // Optimize simplification for rendering
        if ( mConn->majorVersion() < 2 )
        {
          simplifyPostgisMethod = QStringLiteral( "snaptogrid" );
        }
        else
        {

          // Default to st_snaptogrid
          simplifyPostgisMethod = QStringLiteral( "st_snaptogrid" );

          if ( ( mConn->majorVersion() == 2 && mConn->minorVersion() >= 2 ) ||
               mConn->majorVersion() > 2 )
          {
            // For postgis >= 2.2 Use ST_RemoveRepeatedPoints instead
            // Do it only if threshold is <= 1 pixel to avoid holes in adjacent polygons
            // We should perhaps use it always for Linestrings, even if threshold > 1 ?
            if ( mRequest.simplifyMethod().threshold() <= 1.0f )
            {
              simplifyPostgisMethod = QStringLiteral( "st_removerepeatedpoints" );
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
          simplifyPostgisMethod = QStringLiteral( "simplifypreservetopology" );
        }
        else
        {
          simplifyPostgisMethod = QStringLiteral( "st_simplifypreservetopology" );
        }
      }
      QgsDebugMsgLevel(
        QStringLiteral( "PostGIS Server side simplification : threshold %1 pixels - method %2" )
        .arg( mRequest.simplifyMethod().threshold() )
        .arg( simplifyPostgisMethod ), 3
      );

      geom = QStringLiteral( "%1(%2,%3)" )
             .arg( simplifyPostgisMethod, geom )
             .arg( mRequest.simplifyMethod().tolerance() * 0.8 ); //-> Default factor for the maximum displacement distance for simplification, similar as GeoServer does

      // Post-simplification
      if ( postSimplification )
      {
        geom = QStringLiteral( "st_simplify( %1, %2, true )" )
               .arg( geom )
               .arg( mRequest.simplifyMethod().tolerance() * 0.7 ); //-> We use a smaller tolerance than pre-filtering to be on the safe side
      }
    }

    geom = QStringLiteral( "%1(%2,'%3')" )
           .arg( mConn->majorVersion() < 2 ? "asbinary" : "st_asbinary",
                 geom,
                 QgsPostgresProvider::endianString() );

    query += delim + geom;
    delim = ',';
  }

  switch ( mSource->mPrimaryKeyType )
  {
    case PktOid:
      query += delim + "oid";
      delim = ',';
      break;

    case PktTid:
      query += delim + "ctid";
      delim = ',';
      break;

    case PktInt:
    case PktInt64:
    case PktUint64:
      query += delim + QgsPostgresConn::quotedIdentifier( mSource->mFields.at( mSource->mPrimaryKeyAttrs.at( 0 ) ).name() );
      delim = ',';
      break;

    case PktFidMap:
      for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
      {
        query += delim + mConn->fieldExpression( mSource->mFields.at( idx ) );
        delim = ',';
      }
      break;

    case PktUnknown:
      QgsDebugMsg( QStringLiteral( "Cannot declare cursor without primary key." ) );
      return false;
  }

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  const auto constAllAttributesList = subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();
  for ( int idx : constAllAttributesList )
  {
    if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
      continue;

    query += delim + mConn->fieldExpression( mSource->mFields.at( idx ) );
  }

  query += " FROM " + mSource->mQuery;

  if ( !whereClause.isEmpty() )
    query += QStringLiteral( " WHERE %1" ).arg( whereClause );

  if ( limit >= 0 )
    query += QStringLiteral( " LIMIT %1" ).arg( limit );

  if ( !orderBy.isEmpty() )
    query += QStringLiteral( " ORDER BY %1 " ).arg( orderBy );

  if ( !mConn->openCursor( mCursorName, query ) )
  {
    // reloading the fields might help next time around
    // TODO how to cleanly force reload of fields?  P->loadFields();
    if ( closeOnFail )
      close();
    return false;
  }

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
      QgsWkbTypes::Type newType = QgsPostgresConn::wkbTypeFromOgcWkbType( wkbType );

      if ( static_cast< unsigned int >( newType ) != wkbType )
      {
        // overwrite type
        unsigned int n = newType;
        memcpy( featureGeom + 1, &n, sizeof( n ) );
      }

      // PostGIS stores TIN as a collection of Triangles.
      // Since Triangles are not supported, they have to be converted to Polygons
      const int nDims = 2 + ( QgsWkbTypes::hasZ( newType ) ? 1 : 0 ) + ( QgsWkbTypes::hasM( newType ) ? 1 : 0 );
      if ( wkbType % 1000 == 16 )
      {
        unsigned int numGeoms;
        memcpy( &numGeoms, featureGeom + 5, sizeof( unsigned int ) );
        unsigned char *wkb = featureGeom + 9;
        for ( unsigned int i = 0; i < numGeoms; ++i )
        {
          const unsigned int localType = QgsWkbTypes::singleType( newType ); // polygon(Z|M)
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

      QgsGeometry g;
      g.fromWkb( featureGeom, returnedLength + 1 );
      feature.setGeometry( g );
    }
    else
    {
      feature.clearGeometry();
    }

    col++;
  }

  QgsFeatureId fid = 0;

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList fetchAttributes = mRequest.subsetOfAttributes();

  switch ( mSource->mPrimaryKeyType )
  {
    case PktOid:
    case PktTid:
      fid = mConn->getBinaryInt( queryResult, row, col++ );
      break;

    case PktInt:
      fid = mConn->getBinaryInt( queryResult, row, col++ );
      if ( !subsetOfAttributes || fetchAttributes.contains( mSource->mPrimaryKeyAttrs.at( 0 ) ) )
      {
        feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
      }
      // NOTE: this needs be done _after_ the setAttribute call
      // above as we want the attribute value to be 1:1 with
      // database value
      fid = QgsPostgresUtils::int32pk_to_fid( fid );
      break;

    case PktUint64:
    case PktInt64:
    {
      QVariantList pkVal;

      int idx = mSource->mPrimaryKeyAttrs.at( 0 );
      QgsField fld = mSource->mFields.at( idx );

      QVariant v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), QString::number( mConn->getBinaryInt( queryResult, row, col ) ), fld.typeName(), mConn );
      pkVal << v;

      if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
      {
        feature.setAttribute( idx, v );
      }
      col++;

      fid = mSource->mShared->lookupFid( pkVal );
    }
    break;

    case PktFidMap:
    {
      QVariantList primaryKeyVals;
      primaryKeyVals.reserve( mSource->mPrimaryKeyAttrs.size() );

      for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
      {
        QgsField fld = mSource->mFields.at( idx );
        QVariant v;

        if ( fld.type() == QVariant::LongLong )
        {
          v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), QString::number( mConn->getBinaryInt( queryResult, row, col ) ), fld.typeName(), mConn );
        }
        else
        {
          v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), queryResult.PQgetvalue( row, col ), fld.typeName(), mConn );
        }
        primaryKeyVals << v;

        if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
          feature.setAttribute( idx, v );

        col++;
      }

      fid = mSource->mShared->lookupFid( primaryKeyVals );

    }
    break;

    case PktUnknown:
      Q_ASSERT_X( false, "QgsPostgresFeatureIterator::getFeature", "FAILURE: cannot get feature with unknown primary key" );
      return false;
  }

  feature.setId( fid );
  QgsDebugMsgLevel( QStringLiteral( "fid=%1" ).arg( fid ), 4 );

  // iterate attributes
  if ( subsetOfAttributes )
  {
    const auto constFetchAttributes = fetchAttributes;
    for ( int idx : constFetchAttributes )
      getFeatureAttribute( idx, queryResult, row, col, feature );
  }
  else
  {
    for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
      getFeatureAttribute( idx, queryResult, row, col, feature );
  }

  return true;
}

void QgsPostgresFeatureIterator::getFeatureAttribute( int idx, QgsPostgresResult &queryResult, int row, int &col, QgsFeature &feature )
{
  if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
    return;

  const QgsField fld = mSource->mFields.at( idx );

  QVariant v;

  switch ( fld.type() )
  {
    case QVariant::ByteArray:
    {
      //special handling for binary field values
      if ( ::PQgetisnull( queryResult.result(), row, col ) )
      {
        v = QVariant( QVariant::ByteArray );
      }
      else
      {
        size_t returnedLength = 0;
        const char *value = ::PQgetvalue( queryResult.result(), row, col );
        unsigned char *data = ::PQunescapeBytea( reinterpret_cast<const unsigned char *>( value ), &returnedLength );
        if ( returnedLength == 0 )
        {
          v = QVariant( QVariant::ByteArray );
        }
        else
        {
          v = QByteArray( reinterpret_cast<const char *>( data ), int( returnedLength ) );
        }
        ::PQfreemem( data );
      }
      break;
    }
    case QVariant::LongLong:
    {
      if ( ::PQgetisnull( queryResult.result(), row, col ) )
      {
        v = QVariant( QVariant::LongLong );
      }
      else
      {
        v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), QString::number( mConn->getBinaryInt( queryResult, row, col ) ), fld.typeName(), mConn );
      }
      break;
    }
    default:
    {
      v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), queryResult.PQgetvalue( row, col ), fld.typeName(), mConn );
      break;
    }
  }
  feature.setAttribute( idx, v );

  col++;
}


//  ------------------

QgsPostgresFeatureSource::QgsPostgresFeatureSource( const QgsPostgresProvider *p )
  : mConnInfo( p->mUri.connectionInfo( false ) )
  , mGeometryColumn( p->mGeometryColumn )
  , mBoundingBoxColumn( p->mBoundingBoxColumn )
  , mSqlWhereClause( p->filterWhereClause() )
  , mFields( p->mAttributeFields )
  , mSpatialColType( p->mSpatialColType )
  , mRequestedSrid( p->mRequestedSrid )
  , mDetectedSrid( p->mDetectedSrid )
  , mRequestedGeomType( p->mRequestedGeomType )
  , mDetectedGeomType( p->mDetectedGeomType )
  , mPrimaryKeyType( p->mPrimaryKeyType )
  , mPrimaryKeyAttrs( p->mPrimaryKeyAttrs )
  , mQuery( p->mQuery )
  , mCrs( p->crs() )
  , mShared( p->mShared )
{
  if ( mSqlWhereClause.startsWith( QLatin1String( " WHERE " ) ) )
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

QgsFeatureIterator QgsPostgresFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsPostgresFeatureIterator( this, false, request ) );
}
