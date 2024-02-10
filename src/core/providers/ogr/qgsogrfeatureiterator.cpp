/***************************************************************************
    qgsogrfeatureiterator.cpp
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
#include "qgsogrfeatureiterator.h"

#include "qgsogrprovider.h"
#include "qgsogrexpressioncompiler.h"
#include "qgssqliteexpressioncompiler.h"

#include "qgsogrutils.h"
#include "qgscplhttpfetchoverrider.h"
#include "qgsgeometry.h"
#include "qgsexception.h"
#include "qgswkbtypes.h"
#include "qgsogrtransaction.h"
#include "qgssymbol.h"
#include "qgsgeometryengine.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgssetrequestinitiator_p.h"

#include <sqlite3.h>

#include <QTextCodec>
#include <QFile>

// using from provider:
// - setRelevantFields(), mRelevantFieldsForNextFeature
// - ogrLayer
// - mFetchFeaturesWithoutGeom
// - mAttributeFields
// - mEncoding

///@cond PRIVATE


QgsOgrFeatureIterator::QgsOgrFeatureIterator( QgsOgrFeatureSource *source, bool ownSource, const QgsFeatureRequest &request, QgsTransaction *transaction )
  : QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>( source, ownSource, request )
  , mSharedDS( source->mSharedDS )
  , mFirstFieldIsFid( source->mFirstFieldIsFid )
  , mFieldsWithoutFid( source->mFieldsWithoutFid )
  , mAuthCfg( source->mAuthCfg )
  , mSymbolType( QgsSymbol::symbolTypeForGeometryType( QgsWkbTypes::geometryType( source->mWkbType ) ) )
{

  /* When inside a transaction for GPKG/SQLite and fetching fid(s) we might be nested inside an outer fetching loop,
   * (see GH #39178) so we need to skip all calls that might reset the reading (rewind) to avoid an endless loop in the
   * outer fetching iterator that uses the same connection.
   */
  mAllowResetReading = ! transaction ||
                       ( source->mDriverName != QLatin1String( "GPKG" ) && source->mDriverName != QLatin1String( "SQLite" ) ) ||
                       ( mRequest.filterType() != Qgis::FeatureRequestFilterType::Fid
                         && mRequest.filterType() != Qgis::FeatureRequestFilterType::Fids );

  mCplHttpFetchOverrider = std::make_unique< QgsCPLHTTPFetchOverrider >( mAuthCfg );
  QgsSetCPLHTTPFetchOverriderInitiatorClass( *mCplHttpFetchOverrider, QStringLiteral( "QgsOgrFeatureIterator" ) )

  for ( const auto &id :  mRequest.filterFids() )
  {
    mFilterFids.insert( id );
  }
  mFilterFidsIt = mFilterFids.begin();

  // Since connection timeout for OGR connections is problematic and can lead to crashes, disable for now.
  mRequest.setTimeout( -1 );
  if ( mSharedDS )
  {
    mOgrLayer = mSharedDS->getLayerFromNameOrIndex( mSource->mLayerName, mSource->mLayerIndex );
    if ( !mOgrLayer )
    {
      return;
    }
  }
  else
  {
    //QgsDebugMsgLevel( "Feature iterator of " + mSource->mLayerName + ": acquiring connection", 2);
    mConn = QgsOgrConnPool::instance()->acquireConnection( QgsOgrProviderUtils::connectionPoolId( mSource->mDataSource, mSource->mShareSameDatasetAmongLayers ),
            mRequest.timeout(),
            mRequest.requestMayBeNested(),
            mRequest.feedback() );
    if ( !mConn || !mConn->ds )
    {
      iteratorClosed();
      return;
    }

    if ( mSource->mLayerName.isNull() )
    {
      mOgrLayer = GDALDatasetGetLayer( mConn->ds, mSource->mLayerIndex );
    }
    else
    {
      mOgrLayer = GDALDatasetGetLayerByName( mConn->ds, mSource->mLayerName.toUtf8().constData() );
    }
    if ( !mOgrLayer )
    {
      return;
    }

    if ( mAllowResetReading && !mSource->mSubsetString.isEmpty() )
    {
      mOgrLayerOri = mOgrLayer;
      mOgrLayer = QgsOgrProviderUtils::setSubsetString( mOgrLayer, mConn->ds, mSource->mEncoding, mSource->mSubsetString );
      // If the mSubsetString was a full SELECT ...., then mOgrLayer will be a OGR SQL layer != mOgrLayerOri

      if ( !mOgrLayer )
      {
        close();
        return;
      }
    }
  }
  QMutexLocker locker( mSharedDS ? &mSharedDS->mutex() : nullptr );

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

  mFetchGeometry = ( !mFilterRect.isNull() ) ||
                   !( mRequest.flags() & Qgis::FeatureRequestFlag::NoGeometry ) ||
                   ( mSource->mOgrGeometryTypeFilter != wkbUnknown );

  bool filterExpressionAlreadyTakenIntoAccount = false;

#if SQLITE_VERSION_NUMBER >= 3030000L
  // For GeoPackage, try to compile order by expressions as a SQL statement
  // Only SQLite >= 3.30.0 supports NULLS FIRST / NULLS LAST
  // A potential further optimization would be to translate mFilterRect
  // as a JOIN with the GPKG RTree when it exists, instead of having just OGR
  // evaluating it as a post processing.
  const auto constOrderBy = request.orderBy();
  if ( !constOrderBy.isEmpty() &&
       source->mDriverName == QLatin1String( "GPKG" ) &&
       ( request.filterType() == Qgis::FeatureRequestFilterType::NoFilter ||
         request.filterType() == Qgis::FeatureRequestFilterType::Expression ) &&
       ( mSource->mSubsetString.isEmpty() ||
         !mSource->mSubsetString.startsWith( QLatin1String( "SELECT " ), Qt::CaseInsensitive ) ) )
  {
    QByteArray sql = QByteArray( "SELECT " );
    for ( int i = 0; i < source->mFields.size(); ++i )
    {
      if ( i > 0 )
        sql += QByteArray( ", " );
      sql += QgsOgrProviderUtils::quotedIdentifier( source->mFields[i].name().toUtf8(), source->mDriverName );
    }
    if ( strcmp( OGR_L_GetGeometryColumn( mOgrLayer ), "" ) != 0 )
    {
      sql += QByteArray( ", " );
      sql += QgsOgrProviderUtils::quotedIdentifier( OGR_L_GetGeometryColumn( mOgrLayer ), source->mDriverName );
    }
    sql += QByteArray( " FROM " );
    sql += QgsOgrProviderUtils::quotedIdentifier( OGR_L_GetName( mOgrLayer ), source->mDriverName );

    if ( request.filterType() == Qgis::FeatureRequestFilterType::Expression )
    {
      QgsSQLiteExpressionCompiler compiler(
        source->mFields,
        request.flags() & Qgis::FeatureRequestFlag::IgnoreStaticNodesDuringExpressionCompilation );
      QgsSqlExpressionCompiler::Result result = compiler.compile( request.filterExpression() );
      if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
      {
        QString whereClause = compiler.result();
        sql += QByteArray( " WHERE " );
        if ( mSource->mSubsetString.isEmpty() )
        {
          sql += whereClause.toUtf8();
        }
        else
        {
          sql += QByteArray( "(" );
          sql += mSource->mSubsetString.toUtf8();
          sql += QByteArray( ") AND (" );
          sql += whereClause.toUtf8();
          sql += QByteArray( ")" );
        }
        mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
        mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
      }
      else
      {
        sql.clear();
      }
    }
    else if ( !mSource->mSubsetString.isEmpty() )
    {
      sql += QByteArray( " WHERE " );
      sql += mSource->mSubsetString.toUtf8();
    }

    if ( !sql.isEmpty() )
    {
      sql += QByteArray( " ORDER BY " );
      bool firstOrderBy = true;
      for ( const QgsFeatureRequest::OrderByClause &clause : constOrderBy )
      {
        QgsExpression expression = clause.expression();
        QgsSQLiteExpressionCompiler compiler(
          source->mFields, request.flags() & Qgis::FeatureRequestFlag::IgnoreStaticNodesDuringExpressionCompilation );
        QgsSqlExpressionCompiler::Result result = compiler.compile( &expression );
        if ( result == QgsSqlExpressionCompiler::Complete &&
             expression.rootNode()->nodeType() == QgsExpressionNode::ntColumnRef )
        {
          if ( !firstOrderBy )
            sql += QByteArray( ", " );
          sql += compiler.result().toUtf8();
          sql += QByteArray( " COLLATE NOCASE" );
          sql += clause.ascending() ? QByteArray( " ASC" ) : QByteArray( " DESC" );
          if ( clause.nullsFirst() )
            sql += QByteArray( " NULLS FIRST" );
          else
            sql += QByteArray( " NULLS LAST" );
        }
        else
        {
          sql.clear();
          break;
        }
        firstOrderBy = false;
      }
    }

    if ( !sql.isEmpty() )
    {
      mOrderByCompiled = true;

      if ( mOrderByCompiled && request.limit() >= 0 )
      {
        sql += QByteArray( " LIMIT " );
        sql += QString::number( request.limit() ).toUtf8();
      }
      QgsDebugMsgLevel( QStringLiteral( "Using optimized orderBy as: %1" ).arg( QString::fromUtf8( sql ) ), 4 );
      filterExpressionAlreadyTakenIntoAccount = true;
      if ( mOgrLayerOri && mOgrLayer != mOgrLayerOri )
      {
        GDALDatasetReleaseResultSet( mConn->ds, mOgrLayer );
        mOgrLayer = mOgrLayerOri;
      }
      mOgrLayerOri = mOgrLayer;
      mOgrLayer = QgsOgrProviderUtils::setSubsetString( mOgrLayer, mConn->ds, mSource->mEncoding, sql );
      if ( !mOgrLayer )
      {
        close();
        return;
      }
    }
  }
#endif

  QgsAttributeList attrs = ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();

  // ensure that all attributes required for expression filter are being fetched
  if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes && request.filterType() == Qgis::FeatureRequestFilterType::Expression )
  {
    //ensure that all fields required for filter expressions are prepared
    QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
    attributeIndexes += QSet< int >( attrs.constBegin(), attrs.constEnd() );
    attrs = QgsAttributeList( attributeIndexes.constBegin(), attributeIndexes.constEnd() );
    mRequest.setSubsetOfAttributes( attrs );
  }
  // also need attributes required by order by
  if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
  {
    QSet<int> attributeIndexes;
    const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices( mSource->mFields );
    attributeIndexes.reserve( usedAttributeIndices.size() );
    for ( int attrIdx : usedAttributeIndices )
    {
      attributeIndexes << attrIdx;
    }
    attributeIndexes += QSet< int >( attrs.constBegin(), attrs.constEnd() );
    attrs = QgsAttributeList( attributeIndexes.constBegin(), attributeIndexes.constEnd() );
    mRequest.setSubsetOfAttributes( attrs );
  }

  if ( request.filterType() == Qgis::FeatureRequestFilterType::Expression && request.filterExpression()->needsGeometry() )
  {
    mFetchGeometry = true;
  }

  // make sure we fetch just relevant fields
  // unless it's a VRT data source filtered by geometry as we don't know which
  // attributes make up the geometry and OGR won't fetch them to evaluate the
  // filter if we choose to ignore them (fixes #11223)
  if ( ( mSource->mDriverName != QLatin1String( "VRT" ) && mSource->mDriverName != QLatin1String( "OGR_VRT" ) ) || mFilterRect.isNull() )
  {
    QgsOgrProviderUtils::setRelevantFields( mOgrLayer, mSource->mFields.count(), mFetchGeometry, attrs, mSource->mFirstFieldIsFid, mSource->mSubsetString );
    if ( mOgrLayerOri && mOgrLayerOri != mOgrLayer )
      QgsOgrProviderUtils::setRelevantFields( mOgrLayerOri, mSource->mFields.count(), mFetchGeometry, attrs, mSource->mFirstFieldIsFid, mSource->mSubsetString );
  }

  // spatial query to select features
  if ( mAllowResetReading )
  {
    if ( !mFilterRect.isNull() )
    {
      OGR_L_SetSpatialFilterRect( mOgrLayer, mFilterRect.xMinimum(), mFilterRect.yMinimum(), mFilterRect.xMaximum(), mFilterRect.yMaximum() );
      if ( mOgrLayerOri && mOgrLayerOri != mOgrLayer )
        OGR_L_SetSpatialFilterRect( mOgrLayerOri, mFilterRect.xMinimum(), mFilterRect.yMinimum(), mFilterRect.xMaximum(), mFilterRect.yMaximum() );
    }
    else
    {
      OGR_L_SetSpatialFilter( mOgrLayer, nullptr );
      if ( mOgrLayerOri && mOgrLayerOri != mOgrLayer )
        OGR_L_SetSpatialFilter( mOgrLayerOri, nullptr );
    }
  }

  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        mDistanceWithinGeom = mRequest.referenceGeometry();
        mDistanceWithinEngine.reset( QgsGeometry::createGeometryEngine( mDistanceWithinGeom.constGet() ) );
        mDistanceWithinEngine->prepareGeometry();
      }
      break;
  }

  if ( request.filterType() == Qgis::FeatureRequestFilterType::Expression && !filterExpressionAlreadyTakenIntoAccount )
  {
    QgsSqlExpressionCompiler *compiler = nullptr;
    if ( source->mDriverName == QLatin1String( "SQLite" ) || source->mDriverName == QLatin1String( "GPKG" ) )
    {
      compiler = new QgsSQLiteExpressionCompiler( source->mFields, request.flags() & Qgis::FeatureRequestFlag::IgnoreStaticNodesDuringExpressionCompilation );
    }
    else
    {
      compiler = new QgsOgrExpressionCompiler( source, request.flags() & Qgis::FeatureRequestFlag::IgnoreStaticNodesDuringExpressionCompilation );
    }

    QgsSqlExpressionCompiler::Result result = compiler->compile( request.filterExpression() );
    if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
    {
      QString whereClause = compiler->result();
      if ( !mSource->mSubsetString.isEmpty() && mOgrLayer == mOgrLayerOri )
      {
        whereClause = QStringLiteral( "(" ) + mSource->mSubsetString +
                      QStringLiteral( ") AND (" ) + whereClause +
                      QStringLiteral( ")" );
      }

      if ( mAllowResetReading )
      {
        if ( OGR_L_SetAttributeFilter( mOgrLayer, mSource->mEncoding->fromUnicode( whereClause ).constData() ) == OGRERR_NONE )
        {
          //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
          mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
          mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
        }
        else if ( !mSource->mSubsetString.isEmpty() )
        {
          // OGR rejected the compiled expression. Make sure we restore the original subset string if set (and do the filtering on QGIS' side)
          OGR_L_SetAttributeFilter( mOgrLayer, mSource->mEncoding->fromUnicode( mSource->mSubsetString ).constData() );
        }
      }

    }
    else if ( mSource->mSubsetString.isEmpty() && mAllowResetReading )
    {
      OGR_L_SetAttributeFilter( mOgrLayer, nullptr );
    }

    delete compiler;
  }
  else if ( mSource->mSubsetString.isEmpty() && mAllowResetReading )
  {
    OGR_L_SetAttributeFilter( mOgrLayer, nullptr );
  }

  if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes )
  {
    const QgsAttributeList attrs = mRequest.subsetOfAttributes();
    mRequestAttributes = QVector< int >( attrs.begin(), attrs.end() );
    std::sort( mRequestAttributes.begin(), mRequestAttributes.end() );
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  // Install query logger
  // Note: this logger won't track insert/update/delete operations,
  //       in order to do that the callback would need to be installed
  //       in the provider's dataset, but because the provider life-cycle
  //       is significantly longer than this iterator it wouldn't be possible
  //       to install the callback at a later time if the provider was created
  //       when the logger was disabled.
  //       There is currently no API to connect the change of state of the
  //       logger to the data provider.
  if ( QgsApplication::databaseQueryLog()->enabled() && mConn )
  {
    GDALDatasetSetQueryLoggerFunc( mConn->ds, [ ]( const char *pszSQL, const char *pszError, int64_t lNumRecords, int64_t lExecutionTimeMilliseconds, void *pQueryLoggerArg )
    {
      QgsDatabaseQueryLogEntry entry;
      entry.initiatorClass = QStringLiteral( "QgsOgrFeatureIterator" );
      entry.origin = QGS_QUERY_LOG_ORIGIN;
      entry.provider = QStringLiteral( "ogr" );
      entry.uri = *reinterpret_cast<QString *>( pQueryLoggerArg );
      entry.query = QString( pszSQL );
      entry.error = QString( pszError );
      entry.startedTime = QDateTime::currentMSecsSinceEpoch() - lExecutionTimeMilliseconds;
      entry.fetchedRows = lNumRecords;
      QgsApplication::databaseQueryLog()->log( entry );
      QgsApplication::databaseQueryLog()->finished( entry );
    }, reinterpret_cast<void *>( &mConn->path ) );
  }
#endif
  //start with first feature
  rewind();

}

QgsOgrFeatureIterator::~QgsOgrFeatureIterator()
{
  close();
}

bool QgsOgrFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  // Preparation has already been done in the constructor, so we just communicate the result
  return mOrderByCompiled;
}

bool QgsOgrFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsOgrFeatureIterator::fetchFeatureWithId( QgsFeatureId id, QgsFeature &feature ) const
{
  feature.setValid( false );
  gdal::ogr_feature_unique_ptr fet;

  if ( mAllowResetReading && !mSource->mCanDriverShareSameDatasetAmongLayers )
  {
    OGRLayerH nextFeatureBelongingLayer;
    bool found = false;
    // First pass: try to read from the last feature, in the hope the dataset
    // returns them in increasing feature id number (and as we use a std::set
    // for mFilterFids, we get them in increasing number by the iterator)
    // Second pass: reset before reading
    for ( int passNumber = 0; passNumber < 2; passNumber++ )
    {
      while ( fet.reset( GDALDatasetGetNextFeature(
                           mConn->ds, &nextFeatureBelongingLayer, nullptr, nullptr, nullptr ) ), fet )
      {
        if ( nextFeatureBelongingLayer == mOgrLayer )
        {
          if ( OGR_F_GetFID( fet.get() ) == FID_TO_NUMBER( id ) )
          {
            found = true;
            break;
          }
        }
      }
      if ( found || passNumber == 1 )
      {
        break;
      }
      GDALDatasetResetReading( mConn->ds );
    }

    if ( !found )
    {
      return false;
    }
  }
  else
  {
    fet.reset( OGR_L_GetFeature( mOgrLayer, FID_TO_NUMBER( id ) ) );
  }

  if ( !fet )
  {
    return false;
  }

  if ( !readFeature( std::move( fet ), feature ) )
    return false;

  feature.setValid( true );
  geometryToDestinationCrs( feature, mTransform );
  return true;
}

bool QgsOgrFeatureIterator::checkFeature( gdal::ogr_feature_unique_ptr &fet, QgsFeature &feature )
{
  if ( !readFeature( std::move( fet ), feature ) )
    return false;

  if ( !mFilterRect.isNull() && ( !feature.hasGeometry() || feature.geometry().isEmpty() ) )
    return false;

  geometryToDestinationCrs( feature, mTransform );

  if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
    return false;

  // we have a feature, end this cycle
  feature.setValid( true );
  return true;
}

void QgsOgrFeatureIterator::setInterruptionChecker( QgsFeedback *interruptionChecker )
{
  mInterruptionChecker = interruptionChecker;
  if ( mCplHttpFetchOverrider && QThread::currentThread() == mCplHttpFetchOverrider->thread() )
  {
    mCplHttpFetchOverrider->setFeedback( interruptionChecker );
  }
}

bool QgsOgrFeatureIterator::fetchFeature( QgsFeature &feature )
{
  QMutexLocker locker( mSharedDS ? &mSharedDS->mutex() : nullptr );

  // if we are on the same thread as the iterator was created in, we don't need to initializer another
  // QgsCPLHTTPFetchOverrider (which is expensive)
  std::unique_ptr< QgsCPLHTTPFetchOverrider > localHttpFetchOverride;
  if ( QThread::currentThread() != mCplHttpFetchOverrider->thread() )
  {
    localHttpFetchOverride = std::make_unique< QgsCPLHTTPFetchOverrider >( mAuthCfg, mInterruptionChecker );
    QgsSetCPLHTTPFetchOverriderInitiatorClass( *localHttpFetchOverride, QStringLiteral( "QgsOgrFeatureIterator" ) )
  }

  feature.setValid( false );

  if ( mClosed || !mOgrLayer )
    return false;

  if ( mRequest.filterType() == Qgis::FeatureRequestFilterType::Fid )
  {
    bool result = fetchFeatureWithId( mRequest.filterFid(), feature );
    close(); // the feature has been read or was not found: we have finished here

    if ( result && mDistanceWithinEngine )
    {
      result = mDistanceWithinEngine->distance( feature.geometry().constGet() ) <= mRequest.distanceWithin();
      feature.setValid( result );
    }

    return result;
  }
  else if ( mRequest.filterType() == Qgis::FeatureRequestFilterType::Fids )
  {
    while ( mFilterFidsIt != mFilterFids.end() )
    {
      QgsFeatureId nextId = *mFilterFidsIt;
      ++mFilterFidsIt;

      if ( fetchFeatureWithId( nextId, feature ) )
      {
        bool result = true;
        if ( mDistanceWithinEngine )
        {
          result = mDistanceWithinEngine->distance( feature.geometry().constGet() ) <= mRequest.distanceWithin();
          feature.setValid( result );
        }

        if ( result )
          return true;
      }
    }
    close();
    return false;
  }

  gdal::ogr_feature_unique_ptr fet;

  // OSM layers (especially large ones) need the GDALDataset::GetNextFeature() call rather than OGRLayer::GetNextFeature()
  // see more details here: https://trac.osgeo.org/gdal/wiki/rfc66_randomlayerreadwrite
  if ( !mSource->mCanDriverShareSameDatasetAmongLayers )
  {
    OGRLayerH nextFeatureBelongingLayer;
    while ( fet.reset( GDALDatasetGetNextFeature( mConn->ds, &nextFeatureBelongingLayer, nullptr, nullptr, nullptr ) ), fet )
    {
      if ( nextFeatureBelongingLayer == mOgrLayer && checkFeature( fet, feature ) )
      {
        return true;
      }
    }
  }
  else
  {

    while ( fet.reset( OGR_L_GetNextFeature( mOgrLayer ) ), fet )
    {
      if ( checkFeature( fet, feature ) )
      {
        return true;
      }
    }
  }

  close();
  return false;
}

void QgsOgrFeatureIterator::resetReading()
{
  if ( ! mAllowResetReading )
  {
    return;
  }
  if ( !mSource->mCanDriverShareSameDatasetAmongLayers )
  {
    GDALDatasetResetReading( mConn->ds );
  }
  else
  {
    OGR_L_ResetReading( mOgrLayer );
  }
}


bool QgsOgrFeatureIterator::rewind()
{
  QMutexLocker locker( mSharedDS ? &mSharedDS->mutex() : nullptr );

  if ( mClosed || !mOgrLayer )
    return false;

  resetReading();

  mFilterFidsIt = mFilterFids.begin();

  return true;
}


bool QgsOgrFeatureIterator::close()
{
  // Finally reset the data source filter, in case it was changed by a previous request
  // this fixes https://github.com/qgis/QGIS/issues/51934
  if ( mOgrLayer && ! mSource->mSubsetString.isEmpty() )
  {
    OGR_L_SetAttributeFilter( mOgrLayer,  mSource->mEncoding->fromUnicode( mSource->mSubsetString ).constData() );
  }

  if ( mSharedDS )
  {
    iteratorClosed();

    mOgrLayer = nullptr;
    mSharedDS.reset();
    mClosed = true;
    return true;
  }

  if ( !mConn )
    return false;

  iteratorClosed();

  // Will for example release SQLite3 statements
  if ( mOgrLayer )
  {
    resetReading();
  }

  if ( mOgrLayerOri )
  {
    if ( mOgrLayer != mOgrLayerOri )
    {
      GDALDatasetReleaseResultSet( mConn->ds, mOgrLayer );
    }
    mOgrLayer = nullptr;
    mOgrLayerOri = nullptr;
  }

  if ( mConn )
  {
    //QgsDebugMsgLevel( "Feature iterator of " + mSource->mLayerName + ": releasing connection", 2);
    QgsOgrConnPool::instance()->releaseConnection( mConn );
  }

  mConn = nullptr;
  mOgrLayer = nullptr;

  mClosed = true;
  return true;
}


QVariant QgsOgrFeatureIterator::getFeatureAttribute( OGRFeatureH ogrFet, int attindex ) const
{
  if ( mFirstFieldIsFid && attindex == 0 )
  {
    return static_cast<qint64>( OGR_F_GetFID( ogrFet ) );
  }

  int attindexWithoutFid = ( mFirstFieldIsFid ) ? attindex - 1 : attindex;
  bool ok = false;
  return QgsOgrUtils::getOgrFeatureAttribute( ogrFet, mFieldsWithoutFid, attindexWithoutFid, mSource->mEncoding, &ok );
}

bool QgsOgrFeatureIterator::readFeature( const gdal::ogr_feature_unique_ptr &fet, QgsFeature &feature ) const
{
  feature.setId( OGR_F_GetFID( fet.get() ) );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  const bool useExactIntersect = mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox && ( mRequest.flags() & Qgis::FeatureRequestFlag::ExactIntersect );
  const bool geometryTypeFilter = mSource->mOgrGeometryTypeFilter != wkbUnknown;
  if ( mFetchGeometry || mRequest.spatialFilterType() != Qgis::SpatialFilterType::NoFilter || geometryTypeFilter )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet.get() );

    if ( geom )
    {
      QgsGeometry g = QgsOgrUtils::ogrGeometryToQgsGeometry( geom );

      // Insure that multipart datasets return multipart geometry
      if ( QgsWkbTypes::isMultiType( mSource->mWkbType ) && !g.isMultipart() )
      {
        g.convertToMultiType();
      }

      feature.setGeometry( g );
    }
    else
      feature.clearGeometry();

    if ( mSource->mOgrGeometryTypeFilter == wkbGeometryCollection &&
         geom && wkbFlatten( OGR_G_GetGeometryType( geom ) ) == wkbGeometryCollection )
    {
      // OK
    }
    else if ( ( geometryTypeFilter && ( !feature.hasGeometry() || QgsOgrProviderUtils::ogrWkbSingleFlattenAndLinear( ( OGRwkbGeometryType )feature.geometry().wkbType() ) != mSource->mOgrGeometryTypeFilter ) )
              || ( !mFilterRect.isNull() &&
                   ( !feature.hasGeometry()
                     || ( useExactIntersect && !feature.geometry().intersects( mFilterRect ) )
                     || ( !useExactIntersect && !feature.geometry().boundingBoxIntersects( mFilterRect ) )
                   )
                 ) )
    {
      return false;
    }
  }

  if ( !mFetchGeometry )
  {
    feature.clearGeometry();
  }

  // fetch attributes
  const int fieldCount = mSource->mFields.count();
  QgsAttributes attributes( fieldCount );
  QVariant *attributeData = attributes.data();
  if ( mRequest.flags() & Qgis::FeatureRequestFlag::SubsetOfAttributes )
  {
    const int requestedAttributeTotal = mRequestAttributes.size();
    if ( requestedAttributeTotal > 0 )
    {
      const int *requestAttribute = mRequestAttributes.constData();
      for ( int i = 0; i < requestedAttributeTotal; ++i )
      {
        const int idx = requestAttribute[i];
        if ( idx >= fieldCount )
          continue;

        attributeData[idx] = getFeatureAttribute( fet.get(), idx );
      }
    }
  }
  else
  {
    // all attributes
    for ( int idx = 0; idx < fieldCount; ++idx )
    {
      *attributeData++ = getFeatureAttribute( fet.get(), idx );
    }
  }
  feature.setAttributes( attributes );

  if ( mRequest.flags() & Qgis::FeatureRequestFlag::EmbeddedSymbols )
  {
    const QString styleString( OGR_F_GetStyleString( fet.get() ) );
    feature.setEmbeddedSymbol( QgsOgrUtils::symbolFromStyleString( styleString, mSymbolType ).release() );
  }

  return true;
}


QgsOgrFeatureSource::QgsOgrFeatureSource( const QgsOgrProvider *p )
  : mDataSource( p->dataSourceUri( true ) )
  , mAuthCfg( p->authCfg() )
  , mShareSameDatasetAmongLayers( p->mShareSameDatasetAmongLayers )
  , mLayerName( p->layerName() )
  , mLayerIndex( p->layerIndex() )
  , mSubsetString( QgsOgrProviderUtils::cleanSubsetString( p->mSubsetString ) )
  , mEncoding( p->textEncoding() ) // no copying - this is a borrowed pointer from Qt
  , mFields( p->mAttributeFields )
  , mFirstFieldIsFid( p->mFirstFieldIsFid )
  , mOgrGeometryTypeFilter( p->mUniqueGeometryType ? wkbUnknown : QgsOgrProviderUtils::ogrWkbSingleFlattenAndLinear( p->mOgrGeometryTypeFilter ) )
  , mDriverName( p->mGDALDriverName )
  , mCrs( p->crs() )
  , mWkbType( p->wkbType() )
  , mSharedDS( nullptr )
{
  if ( p->mTransaction )
  {
    mTransaction = p->mTransaction;
    mSharedDS = p->mTransaction->sharedDS();
  }
  for ( int i = ( p->mFirstFieldIsFid ) ? 1 : 0; i < mFields.size(); i++ )
    mFieldsWithoutFid.append( mFields.at( i ) );
  QgsOgrConnPool::instance()->ref( QgsOgrProviderUtils::connectionPoolId( mDataSource, mShareSameDatasetAmongLayers ) );

  mCanDriverShareSameDatasetAmongLayers = QgsOgrProviderUtils::canDriverShareSameDatasetAmongLayers( mDriverName );
}

QgsOgrFeatureSource::~QgsOgrFeatureSource()
{
  QgsOgrConnPool::instance()->unref( QgsOgrProviderUtils::connectionPoolId( mDataSource, mShareSameDatasetAmongLayers ) );
}

QgsFeatureIterator QgsOgrFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsOgrFeatureIterator( this, false, request, mTransaction ) );
}

///@endcond
