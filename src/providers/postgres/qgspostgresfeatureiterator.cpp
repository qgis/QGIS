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
#include "qgssettings.h"
#include "qgsexception.h"

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

  if ( !mConn )
  {
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

  mCursorName = mConn->uniqueCursorName();
  QString whereClause;

  bool limitAtProvider = ( mRequest.limit() >= 0 );

  bool useFallbackWhereClause = false;
  QString fallbackWhereClause;

  if ( !mFilterRect.isNull() && !mSource->mGeometryColumn.isNull() )
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
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
      attributeIndexes += attrs.toSet();
      mRequest.setSubsetOfAttributes( attributeIndexes.toList() );
    }
    mFilterRequiresGeometry = request.filterExpression()->needsGeometry();

    if ( QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
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
      Q_FOREACH ( const QgsFeatureRequest::OrderByClause &clause, request.orderBy() )
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
      Q_FOREACH ( const QString &attr, mRequest.orderBy().usedAttributes() )
      {
        int attrIndex = mSource->mFields.lookupField( attr );
        if ( !attrs.contains( attrIndex ) )
          attrs << attrIndex;
      }
      mRequest.setSubsetOfAttributes( attrs );
    }

    if ( !mOrderByCompiled )
      limitAtProvider = false;

    bool success = declareCursor( whereClause, limitAtProvider ? mRequest.limit() : -1, false, orderByParts.join( QStringLiteral( "," ) ) );
    if ( !success && useFallbackWhereClause )
    {
      //try with the fallback where clause, e.g., for cases when using compiled expression failed to prepare
      success = declareCursor( fallbackWhereClause, -1, false, orderByParts.join( QStringLiteral( "," ) ) );
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

  if ( mFeatureQueue.empty() && !mLastFetch )
  {
#if 0 //disabled dynamic queue size
    QElapsedTimer timer;
    timer.start();
#endif

    QString fetch = QStringLiteral( "FETCH FORWARD %1 FROM %2" ).arg( mFeatureQueueSize ).arg( mCursorName );
    QgsDebugMsgLevel( QStringLiteral( "fetching %1 features." ).arg( mFeatureQueueSize ), 4 );

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
    QgsDebugMsg( QStringLiteral( "Finished after %1 features" ).arg( mFetched ) );
    close();

    mSource->mShared->ensureFeaturesCountedAtLeast( mFetched );

    return false;
  }

  feature = mFeatureQueue.dequeue();
  mFetched++;

  feature.setValid( true );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups
  geometryToDestinationCrs( feature, mTransform );

  return true;
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

  mConn->PQexecNR( QStringLiteral( "move absolute 0 in %1" ).arg( mCursorName ) );
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
  if ( mConn->majorVersion() < 2 )
  {
    qBox = QStringLiteral( "setsrid('BOX3D(%1)'::box3d,%2)" )
           .arg( rect.asWktCoordinates(),
                 mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );
  }
  else
  {
    qBox = QStringLiteral( "st_makeenvelope(%1,%2,%3,%4,%5)" )
           .arg( qgsDoubleToString( rect.xMinimum() ),
                 qgsDoubleToString( rect.yMinimum() ),
                 qgsDoubleToString( rect.xMaximum() ),
                 qgsDoubleToString( rect.yMaximum() ),
                 mSource->mRequestedSrid.isEmpty() ? mSource->mDetectedSrid : mSource->mRequestedSrid );
  }

  bool castToGeometry = mSource->mSpatialColType == SctGeography ||
                        mSource->mSpatialColType == SctPcPatch;

  QString whereClause = QStringLiteral( "%1%2 && %3" )
                        .arg( QgsPostgresConn::quotedIdentifier( mSource->mGeometryColumn ),
                              castToGeometry ? "::geometry" : "",
                              qBox );

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
    whereClause += QStringLiteral( " AND %1" ).arg( QgsPostgresConn::postgisTypeFilter( mSource->mGeometryColumn, ( QgsWkbTypes::Type )mSource->mRequestedGeomType, castToGeometry ) );
  }

  return whereClause;
}



bool QgsPostgresFeatureIterator::declareCursor( const QString &whereClause, long limit, bool closeOnFail, const QString &orderBy )
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
         QgsWkbTypes::flatType( QgsWkbTypes::singleType( usedGeomType ) ) != QgsWkbTypes::Point &&
         !QgsWkbTypes::isCurvedType( usedGeomType ) )
    {
      // PostGIS simplification method to use
      QString simplifyPostgisMethod;

      // Simplify again with st_simplify after first simplification ?
      bool postSimplification;
      postSimplification = false; // default to false. Set to true only for PostGIS >= 2.2 when using st_removerepeatedpoints

      if ( mRequest.simplifyMethod().methodType() == QgsSimplifyMethod::OptimizeForRendering )
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
            if ( mRequest.simplifyMethod().threshold() <= 1.0 )
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
      QgsDebugMsg(
        QString( "PostGIS Server side simplification : threshold %1 pixels - method %2" )
        .arg( mRequest.simplifyMethod().threshold() )
        .arg( simplifyPostgisMethod )
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
    case PktUint64:
      query += delim + QgsPostgresConn::quotedIdentifier( mSource->mFields.at( mSource->mPrimaryKeyAttrs.at( 0 ) ).name() );
      delim = ',';
      break;

    case PktFidMap:
      Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
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
  Q_FOREACH ( int idx, subsetOfAttributes ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList() )
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

      if ( ( unsigned int )newType != wkbType )
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
    case PktUint64:
      fid = mConn->getBinaryInt( queryResult, row, col++ );
      if ( !subsetOfAttributes || fetchAttributes.contains( mSource->mPrimaryKeyAttrs.at( 0 ) ) )
      {
        feature.setAttribute( mSource->mPrimaryKeyAttrs[0], fid );
      }
      if ( mSource->mPrimaryKeyType == PktInt )
      {
        // NOTE: this needs be done _after_ the setAttribute call
        // above as we want the attribute value to be 1:1 with
        // database value
        fid = QgsPostgresUtils::int32pk_to_fid( fid );
      }
      break;

    case PktFidMap:
    {
      QVariantList primaryKeyVals;

      Q_FOREACH ( int idx, mSource->mPrimaryKeyAttrs )
      {
        QgsField fld = mSource->mFields.at( idx );

        QVariant v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), queryResult.PQgetvalue( row, col ), fld.typeName() );
        primaryKeyVals << v;

        if ( !subsetOfAttributes || fetchAttributes.contains( idx ) )
          feature.setAttribute( idx, v );

        col++;
      }

      fid = mSource->mShared->lookupFid( primaryKeyVals );

    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: cannot get feature with unknown primary key" );
      return false;
  }

  feature.setId( fid );
  QgsDebugMsgLevel( QStringLiteral( "fid=%1" ).arg( fid ), 4 );

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

void QgsPostgresFeatureIterator::getFeatureAttribute( int idx, QgsPostgresResult &queryResult, int row, int &col, QgsFeature &feature )
{
  if ( mSource->mPrimaryKeyAttrs.contains( idx ) )
    return;

  const QgsField fld = mSource->mFields.at( idx );
  QVariant v = QgsPostgresProvider::convertValue( fld.type(), fld.subType(), queryResult.PQgetvalue( row, col ), fld.typeName() );
  feature.setAttribute( idx, v );

  col++;
}


//  ------------------

QgsPostgresFeatureSource::QgsPostgresFeatureSource( const QgsPostgresProvider *p )
  : mConnInfo( p->mUri.connectionInfo( false ) )
  , mGeometryColumn( p->mGeometryColumn )
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
