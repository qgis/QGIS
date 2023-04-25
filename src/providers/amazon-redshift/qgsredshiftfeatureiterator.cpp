/***************************************************************************
   qgsredshiftfeatureiterator.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftfeatureiterator.h"

#include <utility>

#include <QElapsedTimer>
#include <QObject>

#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsredshiftconnpool.h"
#include "qgsredshiftexpressioncompiler.h"
#include "qgsredshiftprovider.h"
#include "qgssettings.h"

QgsRedshiftFeatureIterator::QgsRedshiftFeatureIterator( QgsRedshiftFeatureSource *source, bool ownSource,
    const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsRedshiftFeatureSource>( source, ownSource, request )
{
  if ( request.filterType() == QgsFeatureRequest::FilterFids && request.filterFids().isEmpty() )
  {
    mClosed = true;
    iteratorClosed();
    return;
  }

  mConn = QgsRedshiftConnPool::instance()->acquireConnection( mSource->mConnInfo, request.timeout(),
          request.requestMayBeNested() );

  if ( !mConn || mConn->PQstatus() != CONNECTION_OK )
  {
    mValid = false;
    mClosed = true;
    iteratorClosed();
    return;
  }

  initCursor();
}

QgsRedshiftFeatureIterator::~QgsRedshiftFeatureIterator()
{
  close();
}

bool QgsRedshiftFeatureIterator::initCursor()
{
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
    return false;
  }

  mCursorName = mConn->uniqueCursorName();
  QString whereClause;

  bool limitAtProvider = ( mRequest.limit() >= 0 );

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
          whereClause =
            QgsRedshiftUtils::andWhereClauses( whereClause,
                                               QStringLiteral( "ST_DWithin(%1,ST_GeomFromText('%2',%3),%4)" )
                                               .arg( QgsRedshiftConn::quotedIdentifier(
                                                   mSource->mGeometryColumn ),
                                                   mRequest.referenceGeometry().asWkt(),
                                                   mSource->mRequestedSrid.isEmpty() ?
                                                   mSource->mDetectedSrid :
                                                   mSource->mRequestedSrid )
                                               .arg( mRequest.distanceWithin() ) );
        }
      }
      break;
  }

  if ( !mSource->mSqlWhereClause.isEmpty() )
  {
    whereClause = QgsRedshiftUtils::andWhereClauses( whereClause, '(' + mSource->mSqlWhereClause + ')' );
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    QString fidWhereClause = QgsRedshiftUtils::whereClause( mRequest.filterFid(), mSource->mFields, mConn,
                             mSource->mPrimaryKeyAttrs, mSource->mShared );

    whereClause = QgsRedshiftUtils::andWhereClauses( whereClause, fidWhereClause );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    QString fidsWhereClause = QgsRedshiftUtils::whereClause( mRequest.filterFids(), mSource->mFields, mConn,
                              mSource->mPrimaryKeyAttrs, mSource->mShared );

    whereClause = QgsRedshiftUtils::andWhereClauses( whereClause, fidsWhereClause );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
  {
    // ensure that all attributes required for expression filter are being
    // fetched
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      QgsAttributeList attrs = mRequest.subsetOfAttributes();
      // ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = mRequest.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += qgis::listToSet( attrs );
      mRequest.setSubsetOfAttributes( qgis::setToList( attributeIndexes ) );
    }
    mFilterRequiresGeometry = mRequest.filterExpression()->needsGeometry();

    if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
    {
      // IMPORTANT - this MUST be the last clause added!
      QgsRedshiftExpressionCompiler compiler = QgsRedshiftExpressionCompiler( mSource );

      if ( compiler.compile( mRequest.filterExpression() ) == QgsSqlExpressionCompiler::Complete )
      {
        useFallbackWhereClause = true;
        fallbackWhereClause = whereClause;
        whereClause = QgsRedshiftUtils::andWhereClauses( whereClause, compiler.result() );
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

  bool success = false;
  if ( !mClosed )
  {
    QStringList orderByParts;

    mOrderByCompiled = mRequest.orderBy().isEmpty();

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

    success = declareCursor( whereClause, limitAtProvider ? mRequest.limit() : -1, false,
                             orderByParts.join( QStringLiteral( "," ) ) );
    if ( !success && useFallbackWhereClause )
    {
      // try with the fallback where clause, e.g., for cases when using compiled
      // expression failed to prepare
      success = declareCursor( fallbackWhereClause, -1, false, orderByParts.join( QStringLiteral( "," ) ) );
      if ( success )
      {
        mExpressionCompiled = false;
        mCompileFailed = true;
      }
    }

    if ( !success && !orderByParts.isEmpty() )
    {
      // try with no order by clause
      success = declareCursor( whereClause, -1, false );
      if ( success )
        mOrderByCompiled = false;
    }

    if ( !success && useFallbackWhereClause && !orderByParts.isEmpty() )
    {
      // try with no expression compilation AND no order by clause
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

  return success;
}

static bool FetchFeaturesAsync( QgsRedshiftConn *mConn, int count,
                                QString &cursorName )
{
  QString fetch = QStringLiteral( "FETCH FORWARD %1 FROM %2" )
                  .arg( count ).arg( cursorName );
  QgsDebugMsgLevel( QStringLiteral( "fetching %1 features." )
                    .arg( count ), 4 );
  // 1 is returned if the command is successfuly dispatched
  // https://www.postgresql.org/docs/8.1/libpq-async.html
  return mConn->PQsendQuery( fetch ) == 1; // fetch features asynchronously
}

bool QgsRedshiftFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mFeatureQueue.empty() && !mLastFetch )
  {
    const int featureFetchCount = mConn->getFetchLimit();
    if ( !FetchFeaturesAsync( mConn, featureFetchCount, mCursorName ) )
    {
      QgsMessageLog::logMessage(
        QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" )
        .arg( mCursorName, mConn->PQerrorMessage() ),
        QObject::tr( "Redshift Spatial" ) );
      return false;
    }
    QgsRedshiftResult queryResult;
    for ( ;; )
    {
      queryResult = mConn->PQgetResult();
      if ( !queryResult.result() )
        break;

      if ( queryResult.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage(
          QObject::tr( "Fetching from cursor %1 failed\nDatabase error: %2" )
          .arg( mCursorName, mConn->PQerrorMessage() ),
          QObject::tr( "Redshift Spatial" ) );
        break;
      }

      int rows = queryResult.PQntuples();
      if ( rows == 0 )
        continue;

      mLastFetch = rows < featureFetchCount;

      for ( int row = 0; row < rows; row++ )
      {
        mFeatureQueue.enqueue( QgsFeature() );
        getFeature( queryResult, row, mFeatureQueue.back() );
      } // for each row in queue
    }
  }

  while ( !mFeatureQueue.empty() )
  {
    feature = mFeatureQueue.dequeue();
    mFetched++;
    geometryToDestinationCrs( feature, mTransform );

    if ( mDistanceWithinEngine )
    {
      QgsDebugMsg( QStringLiteral( "Feature %1 distance %2, the distance %3, id %4" )
                   .arg( feature.geometry().asWkt() )
                   .arg( mDistanceWithinEngine->distance(
                           feature.geometry().constGet() ) )
                   .arg( mRequest.distanceWithin() )
                   .arg( feature.id() ) );
      if ( mDistanceWithinEngine->distance(
             feature.geometry().constGet() ) > mRequest.distanceWithin() )
      {
        continue;
      }
    }
    feature.setValid( true );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    return true;
  }

  mLastFetch = true;
  QgsDebugMsgLevel( QStringLiteral( "Finished after %1 features" ).arg( mFetched ), 2 );
  close();

  mSource->mShared->ensureFeaturesCountedAtLeast( mFetched );

  return false;
}

bool QgsRedshiftFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsRedshiftFeatureIterator::prepareSimplification( const QgsSimplifyMethod &simplifyMethod )
{
  // setup simplification of geometries to fetch
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) &&
       simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && !simplifyMethod.forceLocalOptimization() )
  {
    QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();

    if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
    {
      return true;
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Simplification method type (%1) is not "
                                   "recognised by RedshiftFeatureIterator" )
                   .arg( methodType ) );
    }
  }
  return QgsAbstractFeatureIterator::prepareSimplification( simplifyMethod );
}

bool QgsRedshiftFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  return methodType == QgsSimplifyMethod::OptimizeForRendering;
}

bool QgsRedshiftFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just
  // communicate the result
  return mOrderByCompiled;
}

// TODO(marcel): no way to rewind cursor in Redshift, for now use workaround
// where we close current cursor and open a new one
bool QgsRedshiftFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // close current cursor
  mConn->closeCursor( mCursorName );
  mFeatureQueue.clear();
  mFetched = 0;
  mLastFetch = false;

  // init new cursor
  bool ok = initCursor();
  return ok;
}

bool QgsRedshiftFeatureIterator::close()
{
  if ( !mConn )
    return false;

  mConn->closeCursor( mCursorName );

  QgsRedshiftConnPool::instance()->releaseConnection( mConn );
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

QString QgsRedshiftFeatureIterator::whereClauseRect()
{
  QgsRectangle rect = mFilterRect;
  if ( mSource->mSpatialColType == SctGeography )
  {
    rect = QgsRectangle( -180.0, -90.0, 180.0, 90.0 ).intersect( rect );
  }

  if ( !rect.isFinite() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Infinite filter rectangle specified" ), QObject::tr( "Redshift Spatial" ) );
    return QStringLiteral( "false" );
  }

  QString qBox;

  qBox = QStringLiteral( "st_makeenvelope(%1,%2,%3,%4,%5)" )
         .arg( qgsDoubleToString( rect.xMinimum() ), qgsDoubleToString( rect.yMinimum() ),
               qgsDoubleToString( rect.xMaximum() ), qgsDoubleToString( rect.yMaximum() ),
               mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );

  bool castToGeometry = mSource->mSpatialColType == SctGeography;

  // TODO(marcel): change to bounding box intersection here
  QString whereClause = QStringLiteral( "st_intersects(st_envelope(%1%2) , %3)" )
                        .arg( QgsRedshiftConn::quotedIdentifier( mSource->mBoundingBoxColumn ),
                              castToGeometry ? "::geometry" : "", qBox );

  if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    whereClause += QStringLiteral( " AND st_intersects(%1%2,%3)" )
                   .arg( QgsRedshiftConn::quotedIdentifier( mSource->mGeometryColumn ),
                         castToGeometry ? "::geometry" : "", qBox );
  }

  if ( !mSource->mRequestedSrid.isEmpty() &&
       ( mSource->mRequestedSrid != mSource->mDetectedSrid || mSource->mRequestedSrid.toInt() == 0 ) )
  {
    whereClause += QStringLiteral( " AND st_srid(%1%2)=%3" )
                   .arg( QgsRedshiftConn::quotedIdentifier( mSource->mGeometryColumn ),
                         castToGeometry ? "::geometry" : "", mSource->mRequestedSrid );
  }

  if ( mSource->mRequestedGeomType != Qgis::WkbType::Unknown &&
       mSource->mRequestedGeomType != mSource->mDetectedGeomType )
  {
    whereClause += QStringLiteral( " AND %1" ).arg( QgsRedshiftConn::spatialTypeFilter(
                     mSource->mGeometryColumn, ( Qgis::WkbType )mSource->mRequestedGeomType, castToGeometry ) );
  }

  return whereClause;
}

bool QgsRedshiftFeatureIterator::declareCursor( const QString &whereClause, long limit, bool closeOnFail,
    const QString &orderBy )
{
  mFetchGeometry = ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) || mFilterRequiresGeometry ) &&
                   !mSource->mGeometryColumn.isNull();

  QString query( QStringLiteral( "SELECT " ) );
  QString delim;

  if ( mFetchGeometry )
  {
    QString geom = QgsRedshiftConn::quotedIdentifier( mSource->mGeometryColumn );

    if ( mSource->mSpatialColType == SctGeography )
      geom += QLatin1String( "::geometry" );

    Qgis::WkbType usedGeomType = mSource->mRequestedGeomType != Qgis::WkbType::Unknown
                                 ? mSource->mRequestedGeomType
                                 : mSource->mDetectedGeomType;

    if ( !mRequest.simplifyMethod().forceLocalOptimization() &&
         mRequest.simplifyMethod().methodType() != QgsSimplifyMethod::NoSimplification &&
         QgsWkbTypes::flatType( QgsWkbTypes::singleType( usedGeomType ) ) != Qgis::WkbType::Point )
    {
      if ( mRequest.simplifyMethod().methodType() == QgsSimplifyMethod::OptimizeForRendering )
      {
        QString simplificationMethod = "st_simplify";

        QgsDebugMsg( QString( "Redshift Server side simplification : threshold "
                              "%1 pixels - method %2" )
                     .arg( mRequest.simplifyMethod().threshold() )
                     .arg( simplificationMethod ) );

        geom = QStringLiteral( "%1(%2,%3)" )
               .arg( simplificationMethod, geom )
               .arg( mRequest.simplifyMethod().tolerance() *
                     0.8 ); //-> Default factor for the maximum displacement
      }
    }
    // TODO(reflectored): we must zero out SRID here for the encoding
    geom = QStringLiteral( "%1(%2 , 0)" ).arg( "st_setsrid", geom );

    query += delim + geom;
    delim = ',';
  }

  for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
  {
    query += delim + mConn->fieldExpression( mSource->mFields.at( idx ) );
    delim = ',';
  }

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  const auto constAllAttributesList =
    subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();
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

  if ( !mConn->openCursor( mCursorName, query, mSource->mIsExternalDatabase ) )
  {
    if ( closeOnFail )
      close();
    return false;
  }

  mLastFetch = false;
  return true;
}

bool QgsRedshiftFeatureIterator::getFeature( QgsRedshiftResult &queryResult, int row, QgsFeature &feature )
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

      // TODO(marcel): fix wkb type, as Redshift returns EKWB instead of OGC WKB
      // when calling st_asbinary
      uint32_t wkb_type;
      memcpy( &wkb_type, featureGeom + 1, sizeof( wkb_type ) );
      if ( wkb_type & 0x40000000 )
      {
        wkb_type &= ~0x40000000;
        wkb_type += 2000;
      }
      if ( wkb_type & 0x80000000 )
      {
        wkb_type &= ~0x80000000;
        wkb_type += 1000;
      }

      memcpy( featureGeom + 1, &wkb_type, sizeof( wkb_type ) );

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

  QVariantList primaryKeyVals;

  for ( int idx : std::as_const( mSource->mPrimaryKeyAttrs ) )
  {
    QgsField fld = mSource->mFields.at( idx );
    QVariant v;

    v = QgsRedshiftProvider::convertValue( fld.type(), queryResult.PQgetvalue( row, col ), fld.typeName() );

    primaryKeyVals << v;

    if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
      feature.setAttribute( idx, v );

    col++;
  }

  fid = mSource->mShared->lookupFid( primaryKeyVals );

  feature.setId( fid );

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

void QgsRedshiftFeatureIterator::getFeatureAttribute( int idx, QgsRedshiftResult &queryResult, int row, int &col,
    QgsFeature &feature )
{
  if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
    return;

  const QgsField fld = mSource->mFields.at( idx );

  QVariant v;

  v = QgsRedshiftProvider::convertValue( fld.type(), queryResult.PQgetvalue( row, col ), fld.typeName() );

  feature.setAttribute( idx, v );

  col++;
}

//  ------------------

QgsRedshiftFeatureSource::QgsRedshiftFeatureSource( const QgsRedshiftProvider *p )
  : mConnInfo( p->mUri.connectionInfo( false ) ), mGeometryColumn( p->mGeometryColumn ),
    mBoundingBoxColumn( p->mBoundingBoxColumn ), mSqlWhereClause( p->filterWhereClause() ), mFields( p->mAttributeFields ),
    mSpatialColType( p->mSpatialColType ), mRequestedSrid( p->mRequestedSrid ), mDetectedSrid( p->mDetectedSrid ),
    mRequestedGeomType( p->mRequestedGeomType ), mDetectedGeomType( p->mDetectedGeomType ),
    mPrimaryKeyAttrs( p->mPrimaryKeyAttrs ), mQuery( p->mQuery ),
    mCrs( p->crs() ),
    mIsExternalDatabase( !p->mUri.externalDatabase().isEmpty() ),
    mShared( p->mShared )
{
  if ( mSqlWhereClause.startsWith( QLatin1String( " WHERE " ) ) )
    mSqlWhereClause = mSqlWhereClause.mid( 7 );
}

QgsFeatureIterator QgsRedshiftFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsRedshiftFeatureIterator( this, false, request ) );
}
