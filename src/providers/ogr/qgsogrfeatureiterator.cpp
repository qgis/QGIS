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
#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsexception.h"
#include "qgswkbtypes.h"
#include "qgsogrtransaction.h"

#include <QTextCodec>
#include <QFile>

// using from provider:
// - setRelevantFields(), mRelevantFieldsForNextFeature
// - ogrLayer
// - mFetchFeaturesWithoutGeom
// - mAttributeFields
// - mEncoding


QgsOgrFeatureIterator::QgsOgrFeatureIterator( QgsOgrFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>( source, ownSource, request )
  , mFilterFids( mRequest.filterFids() )
  , mFilterFidsIt( mFilterFids.constBegin() )
  , mSharedDS( source->mSharedDS )
{
  if ( mSharedDS )
  {
    mOgrLayer = mSharedDS->createSQLResultLayer( mSource->mEncoding, mSource->mLayerName, mSource->mLayerIndex );
    if ( !mOgrLayer )
    {
      return;
    }
  }
  else
  {
    //QgsDebugMsg( "Feature iterator of " + mSource->mLayerName + ": acquiring connection");
    mConn = QgsOgrConnPool::instance()->acquireConnection( QgsOgrProviderUtils::connectionPoolId( mSource->mDataSource ), request.connectionTimeout(), request.requestMayBeNested() );
    if ( !mConn || !mConn->ds )
    {
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

    if ( !mSource->mSubsetString.isEmpty() )
    {
      mOgrOrigLayer = mOgrLayer;
      mOgrLayerWithFid = QgsOgrProviderUtils::setSubsetString( mOgrLayer, mConn->ds, mSource->mEncoding, QString(), true, &mOrigFidAdded );
      mOgrLayer = QgsOgrProviderUtils::setSubsetString( mOgrLayer, mConn->ds, mSource->mEncoding, mSource->mSubsetString, true, &mOrigFidAdded );
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
                   !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) ||
                   ( mSource->mOgrGeometryTypeFilter != wkbUnknown );

  QgsAttributeList attrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();
  if ( mOrigFidAdded )
    attrs << mSource->mFields.count();

  // ensure that all attributes required for expression filter are being fetched
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && request.filterType() == QgsFeatureRequest::FilterExpression )
  {
    //ensure that all fields required for filter expressions are prepared
    QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes( mSource->mFields );
    attributeIndexes += attrs.toSet();
    attrs = attributeIndexes.toList();
    mRequest.setSubsetOfAttributes( attrs );
  }
  // also need attributes required by order by
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
  {
    QSet<int> attributeIndexes;
    Q_FOREACH ( const QString &attr, mRequest.orderBy().usedAttributes() )
    {
      attributeIndexes << mSource->mFields.lookupField( attr );
    }
    attributeIndexes += attrs.toSet();
    attrs = attributeIndexes.toList();
    mRequest.setSubsetOfAttributes( attrs );
  }

  if ( request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry() )
  {
    mFetchGeometry = true;
  }

  // make sure we fetch just relevant fields
  // unless it's a VRT data source filtered by geometry as we don't know which
  // attributes make up the geometry and OGR won't fetch them to evaluate the
  // filter if we choose to ignore them (fixes #11223)
  if ( ( mSource->mDriverName != QLatin1String( "VRT" ) && mSource->mDriverName != QLatin1String( "OGR_VRT" ) ) || mFilterRect.isNull() )
  {
    QgsOgrProviderUtils::setRelevantFields( mOgrLayer, mSource->mFields.count() + ( mOrigFidAdded ? 1 : 0 ), mFetchGeometry, attrs, mSource->mFirstFieldIsFid );
    if ( mOgrLayerWithFid )
      QgsOgrProviderUtils::setRelevantFields( mOgrLayerWithFid, mSource->mFields.count() + ( mOrigFidAdded ? 1 : 0 ), mFetchGeometry, attrs, mSource->mFirstFieldIsFid );
  }

  // spatial query to select features
  if ( !mFilterRect.isNull() )
  {
    OGR_L_SetSpatialFilterRect( mOgrLayer, mFilterRect.xMinimum(), mFilterRect.yMinimum(), mFilterRect.xMaximum(), mFilterRect.yMaximum() );
    if ( mOgrLayerWithFid )
      OGR_L_SetSpatialFilterRect( mOgrLayerWithFid, mFilterRect.xMinimum(), mFilterRect.yMinimum(), mFilterRect.xMaximum(), mFilterRect.yMaximum() );
  }
  else
  {
    OGR_L_SetSpatialFilter( mOgrLayer, nullptr );
    if ( mOgrLayerWithFid )
      OGR_L_SetSpatialFilter( mOgrLayerWithFid, nullptr );
  }

  if ( request.filterType() == QgsFeatureRequest::FilterExpression
       && QgsSettings().value( QStringLiteral( "qgis/compileExpressions" ), true ).toBool() )
  {
    QgsSqlExpressionCompiler *compiler = nullptr;
    if ( source->mDriverName == QLatin1String( "SQLite" ) || source->mDriverName == QLatin1String( "GPKG" ) )
    {
      compiler = new QgsSQLiteExpressionCompiler( source->mFields );
    }
    else
    {
      compiler = new QgsOgrExpressionCompiler( source );
    }

    QgsSqlExpressionCompiler::Result result = compiler->compile( request.filterExpression() );
    if ( result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial )
    {
      QString whereClause = compiler->result();
      if ( OGR_L_SetAttributeFilter( mOgrLayer, mSource->mEncoding->fromUnicode( whereClause ).constData() ) == OGRERR_NONE )
      {
        //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
        mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
        mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
      }
    }
    else
    {
      OGR_L_SetAttributeFilter( mOgrLayer, nullptr );
    }

    delete compiler;
  }
  else
  {
    OGR_L_SetAttributeFilter( mOgrLayer, nullptr );
  }


  //start with first feature
  rewind();
}

QgsOgrFeatureIterator::~QgsOgrFeatureIterator()
{
  close();
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
  if ( mOrigFidAdded )
  {
    // if we have been using the original fid as feature IDs (e.g. as a result of a subset
    // string in place), then we need to request features from the original, unfiltered
    // OGR layer. Otherwise the feature IDs we request will correspond to features in the
    // filtered layer, not the original layer!
    OGR_L_ResetReading( mOgrLayerWithFid );
    fet.reset( OGR_L_GetFeature( mOgrLayerWithFid, FID_TO_NUMBER( id ) ) );
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

bool QgsOgrFeatureIterator::fetchFeature( QgsFeature &feature )
{
  QMutexLocker locker( mSharedDS ? &mSharedDS->mutex() : nullptr );

  feature.setValid( false );

  if ( mClosed || !mOgrLayer )
    return false;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    bool result = fetchFeatureWithId( mRequest.filterFid(), feature );
    close(); // the feature has been read or was not found: we have finished here
    return result;
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    while ( mFilterFidsIt != mFilterFids.constEnd() )
    {
      QgsFeatureId nextId = *mFilterFidsIt;
      mFilterFidsIt++;

      if ( fetchFeatureWithId( nextId, feature ) )
        return true;
    }
    close();
    return false;
  }

  gdal::ogr_feature_unique_ptr fet;

  while ( fet.reset( OGR_L_GetNextFeature( mOgrLayer ) ), fet )
  {
    if ( !readFeature( std::move( fet ), feature ) )
      continue;

    if ( !mFilterRect.isNull() && ( !feature.hasGeometry() || feature.geometry().isEmpty() ) )
      continue;

    // we have a feature, end this cycle
    feature.setValid( true );
    geometryToDestinationCrs( feature, mTransform );
    return true;

  } // while

  close();
  return false;
}


bool QgsOgrFeatureIterator::rewind()
{
  QMutexLocker locker( mSharedDS ? &mSharedDS->mutex() : nullptr );

  if ( mClosed || !mOgrLayer )
    return false;

  OGR_L_ResetReading( mOgrLayer );

  mFilterFidsIt = mFilterFids.constBegin();

  return true;
}


bool QgsOgrFeatureIterator::close()
{
  if ( mSharedDS )
  {
    iteratorClosed();

    if ( mOgrLayer )
    {
      mSharedDS->releaseResultSet( mOgrLayer );
      mOgrLayer = nullptr;
    }
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
    OGR_L_ResetReading( mOgrLayer );
  }

  if ( mOgrOrigLayer )
  {
    GDALDatasetReleaseResultSet( mConn->ds, mOgrLayer );
    GDALDatasetReleaseResultSet( mConn->ds, mOgrLayerWithFid );
    mOgrLayer = mOgrOrigLayer;
    mOgrOrigLayer = nullptr;
    mOgrLayerWithFid = nullptr;
  }

  if ( mConn )
  {
    //QgsDebugMsg( "Feature iterator of " + mSource->mLayerName + ": releasing connection");
    QgsOgrConnPool::instance()->releaseConnection( mConn );
  }

  mConn = nullptr;
  mOgrLayer = nullptr;

  mClosed = true;
  return true;
}


void QgsOgrFeatureIterator::getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature &f, int attindex ) const
{
  if ( mSource->mFirstFieldIsFid && attindex == 0 )
  {
    f.setAttribute( 0, static_cast<qint64>( OGR_F_GetFID( ogrFet ) ) );
    return;
  }

  int attindexWithoutFid = ( mSource->mFirstFieldIsFid ) ? attindex - 1 : attindex;
  bool ok = false;
  QVariant value = QgsOgrUtils::getOgrFeatureAttribute( ogrFet, mSource->mFieldsWithoutFid, attindexWithoutFid, mSource->mEncoding, &ok );
  if ( !ok )
    return;

  f.setAttribute( attindex, value );
}

bool QgsOgrFeatureIterator::readFeature( gdal::ogr_feature_unique_ptr fet, QgsFeature &feature ) const
{
  if ( mOrigFidAdded )
  {
    OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( mOgrLayer );
    int lastField = OGR_FD_GetFieldCount( fdef ) - 1;
    if ( lastField >= 0 )
      feature.setId( OGR_F_GetFieldAsInteger64( fet.get(), lastField ) );
    else
      feature.setId( OGR_F_GetFID( fet.get() ) );
  }
  else
  {
    feature.setId( OGR_F_GetFID( fet.get() ) );
  }
  feature.initAttributes( mSource->mFields.count() );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  bool useIntersect = !mRequest.filterRect().isNull();
  bool geometryTypeFilter = mSource->mOgrGeometryTypeFilter != wkbUnknown;
  if ( mFetchGeometry || useIntersect || geometryTypeFilter )
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
    else if ( ( useIntersect && ( !feature.hasGeometry()
                                  || ( mRequest.flags() & QgsFeatureRequest::ExactIntersect && !feature.geometry().intersects( mFilterRect ) )
                                  || ( !( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) && !feature.geometry().boundingBoxIntersects( mFilterRect ) )
                                )
              )
              || ( geometryTypeFilter && ( !feature.hasGeometry() || QgsOgrProvider::ogrWkbSingleFlatten( ( OGRwkbGeometryType )feature.geometry().wkbType() ) != mSource->mOgrGeometryTypeFilter ) ) )
    {
      return false;
    }
  }

  if ( !mFetchGeometry )
  {
    feature.clearGeometry();
  }

  // fetch attributes
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    QgsAttributeList attrs = mRequest.subsetOfAttributes();
    for ( QgsAttributeList::const_iterator it = attrs.constBegin(); it != attrs.constEnd(); ++it )
    {
      getFeatureAttribute( fet.get(), feature, *it );
    }
  }
  else
  {
    // all attributes
    for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
    {
      getFeatureAttribute( fet.get(), feature, idx );
    }
  }

  return true;
}


QgsOgrFeatureSource::QgsOgrFeatureSource( const QgsOgrProvider *p )
  : mDataSource( p->dataSourceUri( true ) )
  , mLayerName( p->layerName() )
  , mLayerIndex( p->layerIndex() )
  , mSubsetString( p->mSubsetString )
  , mEncoding( p->textEncoding() ) // no copying - this is a borrowed pointer from Qt
  , mFields( p->mAttributeFields )
  , mFirstFieldIsFid( p->mFirstFieldIsFid )
  , mOgrGeometryTypeFilter( QgsOgrProvider::ogrWkbSingleFlatten( p->mOgrGeometryTypeFilter ) )
  , mDriverName( p->mGDALDriverName )
  , mCrs( p->crs() )
  , mWkbType( p->wkbType() )
  , mSharedDS( nullptr )
{
  if ( p->mTransaction )
  {
    mSharedDS = p->mTransaction->sharedDS();
  }
  for ( int i = ( p->mFirstFieldIsFid ) ? 1 : 0; i < mFields.size(); i++ )
    mFieldsWithoutFid.append( mFields.at( i ) );
  QgsOgrConnPool::instance()->ref( QgsOgrProviderUtils::connectionPoolId( mDataSource ) );
}

QgsOgrFeatureSource::~QgsOgrFeatureSource()
{
  QgsOgrConnPool::instance()->unref( QgsOgrProviderUtils::connectionPoolId( mDataSource ) );
}

QgsFeatureIterator QgsOgrFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsOgrFeatureIterator( this, false, request ) );
}
