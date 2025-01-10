/***************************************************************************
    qgsdamengfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
    copyright            : ( C ) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgeometry.h"
#include "qgsdamengconnpool.h"
#include "qgsdamengexpressioncompiler.h"
#include "qgsdamengfeatureiterator.h"
#include "qgsdamengprovider.h"
#include "qgsdamengtransaction.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsexception.h"
#include "qgsgeometryengine.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"

#include <QElapsedTimer>
#include <QObject>

QgsDamengFeatureIterator::QgsDamengFeatureIterator( QgsDamengFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsDamengFeatureSource>( source, ownSource, request )
{
  if ( request.filterType() == Qgis::FeatureRequestFilterType::Fids && request.filterFids().isEmpty() )
  {
    mClosed = true;
    iteratorClosed();
    return;
  }

  if ( !source->mTransactionConnection )
  {
    mConn = QgsDamengConnPool::instance()->acquireConnection( mSource->mConnInfo, request.timeout(), request.requestMayBeNested() );
    mIsTransactionConnection = false;
  }
  else
  {
    mConn = source->mTransactionConnection;
    mIsTransactionConnection = true;
  }

  if ( !mConn || !mConn->DMconnStatus() )
  {
    mValid = false;
    mClosed = true;
    iteratorClosed();
    return;
  }

  mTransform = mRequest.calculateTransform( mSource->mCrs );
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
      // we use ST_DWithin on the dameng backend instead
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
          whereClause = QgsDamengUtils::andWhereClauses( whereClause, QStringLiteral( "DMGEO2.ST_DWithin(%1,DMGEO2.ST_GeomFromText('%2',%3),%4)" ).arg(
                          QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn ),
                          mRequest.referenceGeometry().asWkt(),
                          mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid )
                        .arg( mRequest.distanceWithin() ) );
        }
      }
      break;
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    whereClause = QgsDamengUtils::andWhereClauses( whereClause, '(' + mSource->mSqlWhereClause + ')' );
  }

  if ( request.filterType() == Qgis::FeatureRequestFilterType::Fid )
  {
    QString fidWhereClause = QgsDamengUtils::whereClause( mRequest.filterFid(), mSource->mFields, mConn, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );

    whereClause = QgsDamengUtils::andWhereClauses( whereClause, fidWhereClause );
  }
  else if ( request.filterType() == Qgis::FeatureRequestFilterType::Fids )
  {
    QString fidsWhereClause = QgsDamengUtils::whereClause( mRequest.filterFids(), mSource->mFields, mConn, mSource->mPrimaryKeyType, mSource->mPrimaryKeyAttrs, mSource->mShared );

    whereClause = QgsDamengUtils::andWhereClauses( whereClause, fidsWhereClause );
  }
  else if ( request.filterType() == Qgis::FeatureRequestFilterType::Expression )
  {
    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes )
    {
      QgsAttributeList attrs = mRequest.subsetOfAttributes();
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += qgis::listToSet( attrs );
      mRequest.setSubsetOfAttributes( qgis::setToList( attributeIndexes ) );
    }
    mFilterRequiresGeometry = request.filterExpression()->needsGeometry();

    //IMPORTANT - this MUST be the last clause added!
    QgsDamengExpressionCompiler compiler = QgsDamengExpressionCompiler( source, request.flags() & Qgis::FeatureRequestFlag::IgnoreStaticNodesDuringExpressionCompilation );

    if ( compiler.compile( request.filterExpression() ) == QgsSqlExpressionCompiler::Complete )
    {
      useFallbackWhereClause = true;
      fallbackWhereClause = whereClause;
      whereClause = QgsDamengUtils::andWhereClauses( whereClause, compiler.result() );
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

    mOrderByCompiled = mRequest.orderBy().isEmpty();

    // ensure that all attributes required for order by are fetched
    if ( !mOrderByCompiled && mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes )
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

    bool success = openQuery( whereClause, limitAtProvider ? mRequest.limit() : -1, false, orderByParts.join( QLatin1Char( ',' ) ) );
    if ( !success && useFallbackWhereClause )
    {
      //try with the fallback where clause, e.g., for cases when using compiled expression failed to prepare
      success = openQuery( fallbackWhereClause, -1, false, orderByParts.join( QLatin1Char( ',' ) ) );
      if ( success )
      {
        mExpressionCompiled = false;
        mCompileFailed = true;
      }
    }

    if ( !success && !orderByParts.isEmpty() )
    {
      //try with no order by clause
      success = openQuery( whereClause, -1, false );
      if ( success )
        mOrderByCompiled = false;
    }

    if ( !success && useFallbackWhereClause && !orderByParts.isEmpty() )
    {
      //try with no expression compilation AND no order by clause
      success = openQuery( fallbackWhereClause, -1, false );
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


QgsDamengFeatureIterator::~QgsDamengFeatureIterator()
{
  close();
}


bool QgsDamengFeatureIterator::fetchFeature( QgsFeature &feature )
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

      QString fetch = QStringLiteral( "FETCH FORWARD %1 " ).arg( mFeatureQueueSize );
      QgsDebugMsgLevel( QStringLiteral( "fetching %1 features." ).arg( mFeatureQueueSize ), 4 );

      lock();
      
      QgsDatabaseQueryLogWrapper logWrapper { fetch, mSource->mConnInfo, QStringLiteral( "dameng" ), QStringLiteral( "QgsDamengFeatureIterator" ), QGS_QUERY_LOG_ORIGIN };

      QgsDamengResult queryResult;
      long long fetchedRows { 0 };
      for ( ;; )
      {
        queryResult = mConn->DMgetResult();
        if ( !queryResult.result() )
          break;

        if ( queryResult.result()->getResStatus() != DmResCommandOk )
        {
          QgsMessageLog::logMessage( QObject::tr( "Database error: %2" ).arg( mConn->DMconnErrorMessage() ), QObject::tr( "Dameng" ) );
          break;
        }

        int row = 0;
        while ( queryResult.result()->fetchBinary() && row < mFeatureQueueSize )
        {
          mFeatureQueue.enqueue( QgsFeature() );
          getFeature( queryResult, row, mFeatureQueue.back() );
          row++;
        } // for each row in queue

        if ( row == 0 )
          break;
        else
          fetchedRows += row;

        mLastFetch = row < mFeatureQueueSize;
        break;
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

bool QgsDamengFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsDamengFeatureIterator::prepareSimplification( const QgsSimplifyMethod &simplifyMethod )
{
  // setup simplification of geometries to fetch
  if ( !( mRequest.flags() & Qgis::FeatureRequestFlag::NoGeometry ) &&
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
      QgsDebugError( QStringLiteral( "Simplification method type (%1) is not recognised by DamengFeatureIterator" ).arg( methodType ) );
    }
  }
  return QgsAbstractFeatureIterator::prepareSimplification( simplifyMethod );
}

bool QgsDamengFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  return methodType == QgsSimplifyMethod::OptimizeForRendering || methodType == QgsSimplifyMethod::PreserveTopology;
}

bool QgsDamengFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

void QgsDamengFeatureIterator::lock()
{
  if ( mIsTransactionConnection )
    mConn->lock();
}

void QgsDamengFeatureIterator::unlock()
{
  if ( mIsTransactionConnection )
    mConn->unlock();
}

bool QgsDamengFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  mFeatureQueue.clear();
  mFetched = 0;
  mLastFetch = false;

  return true;
}

bool QgsDamengFeatureIterator::close()
{
  if ( !mConn )
    return false;

  if ( !mIsTransactionConnection )
  {
    QgsDamengConnPool::instance()->releaseConnection( mConn );
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

QString QgsDamengFeatureIterator::whereClauseRect()
{
  QgsRectangle rect = mFilterRect;
  
  // in case coordinates are around 180.0°
  if ( rect.xMinimum() > rect.xMaximum() && mSource->mCrs.isGeographic() )
  {
    rect.setXMaximum( rect.xMaximum() + 360.0 );
  }

  if ( mSource->mSpatialColType == SctGeography )
  {
    rect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 ).intersect( rect );
  }

  if ( !rect.isFinite() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "Dameng" ) );
    return QStringLiteral( "false" );
  }

  QString qBox;
  const QString bboxSrid = mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid;

  qBox = QStringLiteral( "DMGEO2.st_makeenvelope(%1,%2,%3,%4,%5)" )
          .arg( qgsDoubleToString( rect.xMinimum() ),
                qgsDoubleToString( rect.yMinimum() ),
                qgsDoubleToString( rect.xMaximum() ),
                qgsDoubleToString( rect.yMaximum() ),
                bboxSrid );

  QString geom_col;
  switch ( mSource->mSpatialColType )
  {
  case SctGeometry:
    geom_col = QgsDamengConn::quotedIdentifier( mSource->mBoundingBoxColumn );
    break;
  case SctGeography:
    geom_col = QgsDamengConn::quotedIdentifier( mSource->mBoundingBoxColumn ) + "::SYSGEO2.st_geometry";
    break;
  case SctTopoGeometry:
    geom_col = QStringLiteral( "SYSTOPOLOGY.DMTOPOLOGY.Geometry(%1)" )
                .arg( QgsDamengConn::quotedIdentifier( mSource->mBoundingBoxColumn ) );
    break;
  default:
    break;
  }

  QString whereClause = QStringLiteral( "DMGEO2.ST_Box2DIntersects(%1,%2)" ).arg( geom_col, qBox );

  if ( mSource->mSpatialColType == SctGeography &&
       bboxSrid == QLatin1String( "4326" ) &&
       std::fabs( rect.yMaximum()  - rect.yMinimum() ) <= 10 &&
       std::fabs( rect.xMaximum()  - rect.xMinimum() ) <= 10 &&
       std::fabs( rect.yMaximum() ) <= 70 )
  {
    // And noticing that the difference between the rhumb line and the geodesics
    // increases with higher latitude maximum, and differences of longitude and latitude.
    // We could perhaps get a formula that would give dlat as a function of
    // delta_lon, delta_lat and max( lat ), but those maximum values should be good
    // enough for now.
    double dlat = 1.04;

    // For smaller filtering bounding box, use smaller bbox expansion
    if ( std::fabs( rect.yMaximum() - rect.yMinimum() ) <= 1 && std::fabs( rect.xMaximum() - rect.xMinimum() ) <= 1 )
    {
      // Value got by changing lat1 to 69 and lon2 to 1 in the above code snippet
      dlat = 0.013;
    }
    // In the northern hemisphere, extends the geog bbox to the south
    const double yminGeog = rect.yMinimum() >= 0 ? std::max( 0.0, rect.yMinimum() - dlat ) : rect.yMinimum();
    const double ymaxGeog = rect.yMaximum() >= 0 ? rect.yMaximum() : std::min( 0.0, rect.yMaximum() + dlat );
    const QString qBoxGeog = QStringLiteral( "DMGEO2.st_makeenvelope(%1,%2,%3,%4,%5)" )
                             .arg( qgsDoubleToString( rect.xMinimum() ), qgsDoubleToString( yminGeog ), qgsDoubleToString( rect.xMaximum() ), qgsDoubleToString( ymaxGeog ), bboxSrid );
    whereClause += QStringLiteral( " AND DMGEO2.ST_Box2DIntersects(%1::SYSGEO2.st_geometry, %2)" )
                   .arg( QgsDamengConn::quotedIdentifier( mSource->mBoundingBoxColumn ), qBoxGeog );
  }

  switch ( mSource->mSpatialColType )
  {
  case SctGeometry:
    geom_col = QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn );
    break;
  case SctGeography:
    geom_col = QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn ) + "::SYSGEO2.st_geometry";
    break;
  case SctTopoGeometry:
    geom_col = QStringLiteral( "SYSTOPOLOGY.DMTOPOLOGY.Geometry(%1)" )
      .arg( QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn ) );
    break;
  default:
    break;
  }
  if ( mRequest.flags() & Qgis::FeatureRequestFlag::ExactIntersect )
  {
    QString curveToLineFn = QStringLiteral( "DMGEO2.st_curvetoline" ); // st_ prefix is always used
    whereClause += QStringLiteral( " AND %1(%2(%3),%4 )" )
                   .arg( "DMGEO2.st_intersects", curveToLineFn, geom_col, qBox );
  }

  if ( !mSource->mRequestedSrid.isEmpty() && ( mSource->mRequestedSrid != mSource->mDetectedSrid || mSource->mRequestedSrid.toInt() == 0 ) )
  {
    whereClause += QStringLiteral( " AND %1(%2)=%3" )
                   .arg( "DMGEO2.st_srid",geom_col,mSource->mRequestedSrid );
  }

  if ( mSource->mRequestedGeomType != Qgis::WkbType::Unknown && mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    whereClause += QStringLiteral( " AND %1" ).arg( QgsDamengConn::dmSpatialTypeFilter( mSource->mGeometryColumn, mSource->mRequestedGeomType, mSource->mSpatialColType == SctGeography ) );
  }

  QgsDebugMsgLevel( QStringLiteral( "whereClause = %1" ).arg( whereClause ), 4 );
  return whereClause;
}

bool QgsDamengFeatureIterator::openQuery( const QString &whereClause, long limit, bool closeOnFail, const QString &orderBy )
{
  mFetchGeometry = ( !( mRequest.flags() & Qgis::FeatureRequestFlag::NoGeometry )
                     || mFilterRequiresGeometry
                     || ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin && !mTransform.isShortCircuited() ) )
                   && !mSource->mGeometryColumn.isNull();

  QString query( QStringLiteral( "SELECT " ) );
  QString delim;

  if ( mFetchGeometry )
  {
    QString geom;
    switch ( mSource->mSpatialColType )
    {
    case SctGeometry:
      geom = QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn );
      break;
    case SctGeography:
      geom = QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn ) + QLatin1String( "::SYSGEO2.st_geometry" );
      break;
    case SctTopoGeometry:
      geom = QStringLiteral( "SYSTOPOLOGY.DMTOPOLOGY.Geometry(%1)" )
                      .arg( QgsDamengConn::quotedIdentifier( mSource->mGeometryColumn ) );
      break;
    default:
      break;
    }

    Qgis::WkbType usedGeomType = mSource->mRequestedGeomType != Qgis::WkbType::Unknown
                                     ? mSource->mRequestedGeomType : mSource->mDetectedGeomType;

    if ( !mRequest.simplifyMethod().forceLocalOptimization() &&
         mRequest.simplifyMethod().methodType() != QgsSimplifyMethod::NoSimplification &&
      QgsWkbTypes::flatType( QgsWkbTypes::singleType( usedGeomType ) ) != Qgis::WkbType::Point )
    {
      // Dameng simplification method to use
      QString simplifyDAMENGMethod;

      // Simplify again with st_simplify after first simplification ?
      bool postSimplification;
      postSimplification = false; // default to false. Set to true only for Dameng >= 2.2 when using st_removerepeatedpoints

      if ( !QgsWkbTypes::isCurvedType( usedGeomType ) && mRequest.simplifyMethod().methodType() == QgsSimplifyMethod::OptimizeForRendering )
      {
        // Optimize simplification for rendering
        // Default to st_snaptogrid
        simplifyDAMENGMethod = QStringLiteral( "DMGEO2.st_snaptogrid" );

        if ( mRequest.simplifyMethod().threshold() <= 1.0f )
        {
          simplifyDAMENGMethod = QStringLiteral( "DMGEO2.st_removerepeatedpoints" );
          postSimplification = true; // Ask to apply a post-filtering simplification
        }
        
      }
      else
      {
        // preserve topology
        simplifyDAMENGMethod = QStringLiteral( "DMGEO2.st_simplifypreservetopology" );
      }
      QgsDebugMsgLevel(
        QStringLiteral( "Dameng Server side simplification : threshold %1 pixels - method %2" )
        .arg( mRequest.simplifyMethod().threshold() )
        .arg( simplifyDAMENGMethod ),
        3
      );

      geom = QStringLiteral( "%1(%2,%3)" )
             .arg( simplifyDAMENGMethod, geom )
             .arg( mRequest.simplifyMethod().tolerance() * 0.8 ); //-> Default factor for the maximum displacement distance for simplification, similar as GeoServer does

      // Post-simplification
      if ( postSimplification )
      {
        geom = QStringLiteral( "DMGEO2.st_simplify( %1, %2, true )" )
               .arg( geom )
               .arg( mRequest.simplifyMethod().tolerance() * 0.7 ); //-> We use a smaller tolerance than pre-filtering to be on the safe side
      }
    }

    geom = QStringLiteral( "%1(%2,'%3')" )
           .arg( "DMGEO2.st_asbinary",
                 geom,
                 QgsDamengProvider::endianString() );

    query += delim + geom;
    delim = ',';
  }

  switch ( mSource->mPrimaryKeyType )
  {
    case PktRowId:
      query += delim + "cast( rowid as bigint )";
      delim = ',';
      break;

    case PktInt:
    case PktInt64:
    case PktUint64:
      query += delim + QgsDamengConn::quotedIdentifier( mSource->mFields.at( mSource->mPrimaryKeyAttrs.at( 0 ) ).name() );
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
      QgsDebugError( QStringLiteral( "Cannot declare cursor without primary key." ) );
      return false;
  }

  bool subsetOfAttributes = mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes;
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
  else
  {
    if( mConn->getEstimatedMetadata() )
      query += QStringLiteral( " LIMIT %1" ).arg( QgsDamengConn::GEOM_TYPE_SELECT_LIMIT );
  }

  if ( !orderBy.isEmpty() )
    query += QStringLiteral( " ORDER BY %1 " ).arg( orderBy );

  if ( !mConn->DMexecNR( query ) )
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

bool QgsDamengFeatureIterator::getFeature( QgsDamengResult &queryResult, int row, QgsFeature &feature )
{
  feature.initAttributes( mSource->mFields.count() );

  int col = 0;
  QgsDMResult *qresult = queryResult.result();

  if ( mFetchGeometry )
  {
    slength returnedLength = queryResult.DMgetlength( col, DSQL_C_BINARY );
    if ( returnedLength > 0 )
    {
      unsigned char *featureGeom = new unsigned char[returnedLength + 1];
      qresult->getBinarydata( col, featureGeom, returnedLength );
      memset( featureGeom + returnedLength, 0, 1 );

      unsigned int wkbType;
      memcpy( &wkbType, featureGeom + 1, sizeof( wkbType ) );
      Qgis::WkbType newType = QgsDamengConn::wkbTypeFromOgcWkbType( wkbType );

      if ( static_cast<unsigned int>( newType ) != wkbType )
      {
        // overwrite type
        unsigned int n = static_cast<quint32>( newType );
        memcpy( featureGeom + 1, &n, sizeof( n ) );
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

  bool subsetOfAttributes = mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes;
  QgsAttributeList fetchAttributes = mRequest.subsetOfAttributes();

  switch ( mSource->mPrimaryKeyType )
  {
    case PktInt:
      fid = ( *qresult ).value( col++ ).toLongLong();
      if ( !subsetOfAttributes || fetchAttributes.contains( mSource->mPrimaryKeyAttrs.at( 0 ) ) )
      {
        feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
      }
      break;

    case PktRowId:
      fid = ( *qresult ).value( col++ ).toLongLong();
      break;
    case PktUint64:
    case PktInt64:
    {
      QVariantList pkVal;

      int idx = mSource->mPrimaryKeyAttrs.at( 0 );
      QgsField fld = mSource->mFields.at( idx );

      QVariant v = QgsDamengProvider::convertValue( fld.type(), fld.subType(), ( *qresult ).value( col ).toString(), fld.typeName() );
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
          v = QgsDamengProvider::convertValue( fld.type(), fld.subType(), ( *qresult ).value( col ).toString(), fld.typeName() );
        }
        else
        {
          v = QgsDamengProvider::convertValue( fld.type(), fld.subType(), queryResult.DMgetvalue( col ), fld.typeName() );
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
      Q_ASSERT_X( false, "QgsDamengFeatureIterator::getFeature", "FAILURE: cannot get feature with unknown primary key" );
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

void QgsDamengFeatureIterator::getFeatureAttribute( int idx, QgsDamengResult &queryResult, int row, int &col, QgsFeature &feature )
{
  Q_UNUSED( row )
  if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
    return;

  const QgsField fld = mSource->mFields.at( idx );

  QVariant v;
  if( queryResult.DMftype( col ) == DSQL_ROWID )
    v = queryResult.DMgetvalue( col );
  else
    v = QgsDamengProvider::convertValue( fld.type(), fld.subType(), queryResult.DMgetvalue( col ), fld.typeName() );
  feature.setAttribute( idx, v );

  col++;
}


//  ------------------

QgsDamengFeatureSource::QgsDamengFeatureSource( const QgsDamengProvider *p )
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

QgsDamengFeatureSource::~QgsDamengFeatureSource()
{
  if ( mTransactionConnection )
  {
    mTransactionConnection->unref();
  }
}

QgsFeatureIterator QgsDamengFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsDamengFeatureIterator( this, false, request ) );
}
