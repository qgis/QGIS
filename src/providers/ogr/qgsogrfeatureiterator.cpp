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
  , mSubsetStringSet( false )
  , mOrigFidAdded( false )
  , mFetchGeometry( false )
  , mExpressionCompiled( false )
  , mFilterFids( mRequest.filterFids() )
  , mFilterFidsIt( mFilterFids.constBegin() )
{
  mConn = QgsOgrConnPool::instance()->acquireConnection( mSource->mDataSource );
  if ( !mConn->ds )
  {
    return;
  }

  if ( mSource->mLayerName.isNull() )
  {
    ogrLayer = GDALDatasetGetLayer( mConn->ds, mSource->mLayerIndex );
  }
  else
  {
    ogrLayer = GDALDatasetGetLayerByName( mConn->ds, mSource->mLayerName.toUtf8().constData() );
  }
  if ( !ogrLayer )
  {
    return;
  }

  if ( !mSource->mSubsetString.isEmpty() )
  {
    ogrLayer = QgsOgrProviderUtils::setSubsetString( ogrLayer, mConn->ds, mSource->mEncoding, mSource->mSubsetString, mOrigFidAdded );
    if ( !ogrLayer )
    {
      return;
    }
    mSubsetStringSet = true;
  }

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    mClosed = true;
    return;
  }

  mFetchGeometry = ( !mFilterRect.isNull() ) || !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  QgsAttributeList attrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();

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
    QgsOgrProviderUtils::setRelevantFields( ogrLayer, mSource->mFields.count(), mFetchGeometry, attrs, mSource->mFirstFieldIsFid );
  }

  // spatial query to select features
  if ( !mFilterRect.isNull() )
  {
    OGR_L_SetSpatialFilterRect( ogrLayer, mFilterRect.xMinimum(), mFilterRect.yMinimum(), mFilterRect.xMaximum(), mFilterRect.yMaximum() );
  }
  else
  {
    OGR_L_SetSpatialFilter( ogrLayer, nullptr );
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
      if ( OGR_L_SetAttributeFilter( ogrLayer, mSource->mEncoding->fromUnicode( whereClause ).constData() ) == OGRERR_NONE )
      {
        //if only partial success when compiling expression, we need to double-check results using QGIS' expressions
        mExpressionCompiled = ( result == QgsSqlExpressionCompiler::Complete );
        mCompileStatus = ( mExpressionCompiled ? Compiled : PartiallyCompiled );
      }
    }
    else
    {
      OGR_L_SetAttributeFilter( ogrLayer, nullptr );
    }

    delete compiler;
  }
  else
  {
    OGR_L_SetAttributeFilter( ogrLayer, nullptr );
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
  OGRFeatureH fet;
  if ( mOrigFidAdded )
  {
    OGR_L_ResetReading( ogrLayer );
    while ( ( fet = OGR_L_GetNextFeature( ogrLayer ) ) )
    {
      if ( OGR_F_GetFieldAsInteger64( fet, 0 ) == id )
      {
        break;
      }
      OGR_F_Destroy( fet );
    }
  }
  else
  {
    fet = OGR_L_GetFeature( ogrLayer, FID_TO_NUMBER( id ) );
  }

  if ( !fet )
  {
    return false;
  }

  if ( readFeature( fet, feature ) )
    OGR_F_Destroy( fet );

  feature.setValid( true );
  geometryToDestinationCrs( feature, mTransform );
  return true;
}

bool QgsOgrFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed || !ogrLayer )
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

  OGRFeatureH fet;

  while ( ( fet = OGR_L_GetNextFeature( ogrLayer ) ) )
  {
    if ( !readFeature( fet, feature ) )
      continue;
    else
      OGR_F_Destroy( fet );

    if ( !mFilterRect.isNull() && !feature.hasGeometry() )
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
  if ( mClosed || !ogrLayer )
    return false;

  OGR_L_ResetReading( ogrLayer );

  mFilterFidsIt = mFilterFids.constBegin();

  return true;
}


bool QgsOgrFeatureIterator::close()
{
  if ( !mConn )
    return false;

  iteratorClosed();

  // Will for example release SQLite3 statements
  if ( ogrLayer )
  {
    OGR_L_ResetReading( ogrLayer );
  }

  if ( mSubsetStringSet )
  {
    GDALDatasetReleaseResultSet( mConn->ds, ogrLayer );
  }

  if ( mConn )
    QgsOgrConnPool::instance()->releaseConnection( mConn );

  mConn = nullptr;
  ogrLayer = nullptr;

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

bool QgsOgrFeatureIterator::readFeature( OGRFeatureH fet, QgsFeature &feature ) const
{
  if ( mOrigFidAdded )
  {
    feature.setId( OGR_F_GetFieldAsInteger64( fet, 0 ) );
  }
  else
  {
    feature.setId( OGR_F_GetFID( fet ) );
  }
  feature.initAttributes( mSource->mFields.count() );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  bool useIntersect = mRequest.flags() & QgsFeatureRequest::ExactIntersect;
  bool geometryTypeFilter = mSource->mOgrGeometryTypeFilter != wkbUnknown;
  if ( mFetchGeometry || useIntersect || geometryTypeFilter )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet );

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
    else if ( ( useIntersect && ( !feature.hasGeometry() || !feature.geometry().intersects( mFilterRect ) ) )
              || ( geometryTypeFilter && ( !feature.hasGeometry() || QgsOgrProvider::ogrWkbSingleFlatten( ( OGRwkbGeometryType )feature.geometry().wkbType() ) != mSource->mOgrGeometryTypeFilter ) ) )
    {
      OGR_F_Destroy( fet );
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
      getFeatureAttribute( fet, feature, *it );
    }
  }
  else
  {
    // all attributes
    for ( int idx = 0; idx < mSource->mFields.count(); ++idx )
    {
      getFeatureAttribute( fet, feature, idx );
    }
  }

  return true;
}


QgsOgrFeatureSource::QgsOgrFeatureSource( const QgsOgrProvider *p )
  : mDataSource( p->dataSourceUri() )
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
{
  for ( int i = ( p->mFirstFieldIsFid ) ? 1 : 0; i < mFields.size(); i++ )
    mFieldsWithoutFid.append( mFields.at( i ) );
  QgsOgrConnPool::instance()->ref( mDataSource );
}

QgsOgrFeatureSource::~QgsOgrFeatureSource()
{
  QgsOgrConnPool::instance()->unref( mDataSource );
}

QgsFeatureIterator QgsOgrFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsOgrFeatureIterator( this, false, request ) );
}
