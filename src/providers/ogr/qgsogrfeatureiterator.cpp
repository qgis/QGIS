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
#include "qgsogrgeometrysimplifier.h"

#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QTextCodec>
#include <QFile>

// using from provider:
// - setRelevantFields(), mRelevantFieldsForNextFeature
// - ogrLayer
// - mFetchFeaturesWithoutGeom
// - mAttributeFields
// - mEncoding


QgsOgrFeatureIterator::QgsOgrFeatureIterator( QgsOgrFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>( source, ownSource, request )
    , ogrLayer( 0 )
    , mSubsetStringSet( false )
    , mGeometrySimplifier( NULL )
{
  mFeatureFetched = false;

  mConn = QgsOgrConnPool::instance()->acquireConnection( mSource->mFilePath );

  if ( mSource->mLayerName.isNull() )
  {
    ogrLayer = OGR_DS_GetLayer( mConn->ds, mSource->mLayerIndex );
  }
  else
  {
    ogrLayer = OGR_DS_GetLayerByName( mConn->ds, TO8( mSource->mLayerName ) );
  }

  if ( !mSource->mSubsetString.isEmpty() )
  {
    ogrLayer = QgsOgrUtils::setSubsetString( ogrLayer, mConn->ds, mSource->mEncoding, mSource->mSubsetString );
    mSubsetStringSet = true;
  }

  mFetchGeometry = ( !mRequest.filterRect().isNull() ) || !( mRequest.flags() & QgsFeatureRequest::NoGeometry );
  QgsAttributeList attrs = ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();

  // make sure we fetch just relevant fields
  // unless it's a VRT data source filtered by geometry as we don't know which
  // attributes make up the geometry and OGR won't fetch them to evaluate the
  // filter if we choose to ignore them (fixes #11223)
  if (( mSource->mDriverName != "VRT" && mSource->mDriverName != "OGR_VRT" ) || mRequest.filterRect().isNull() )
  {
    QgsOgrUtils::setRelevantFields( ogrLayer, mSource->mFields.count(), mFetchGeometry, attrs );
  }

  // spatial query to select features
  if ( !mRequest.filterRect().isNull() )
  {
    const QgsRectangle& rect = mRequest.filterRect();

    OGR_L_SetSpatialFilterRect( ogrLayer, rect.xMinimum(), rect.yMinimum(), rect.xMaximum(), rect.yMaximum() );
  }
  else
  {
    OGR_L_SetSpatialFilter( ogrLayer, 0 );
  }

  //start with first feature
  rewind();
}

QgsOgrFeatureIterator::~QgsOgrFeatureIterator()
{
  delete mGeometrySimplifier;
  mGeometrySimplifier = NULL;

  close();
}

bool QgsOgrFeatureIterator::prepareSimplification( const QgsSimplifyMethod& simplifyMethod )
{
  delete mGeometrySimplifier;
  mGeometrySimplifier = NULL;

  // setup simplification of OGR-geometries fetched
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && !simplifyMethod.forceLocalOptimization() )
  {
    QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();
    Q_UNUSED( methodType );

#if defined(GDAL_VERSION_NUM) && defined(GDAL_COMPUTE_VERSION)
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,11,0)
    if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
    {
      int simplifyFlags = QgsMapToPixelSimplifier::SimplifyGeometry | QgsMapToPixelSimplifier::SimplifyEnvelope;
      mGeometrySimplifier = new QgsOgrMapToPixelSimplifier( simplifyFlags, simplifyMethod.tolerance() );
      return true;
    }
#endif
#endif
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1900
    if ( methodType == QgsSimplifyMethod::PreserveTopology )
    {
      mGeometrySimplifier = new QgsOgrTopologyPreservingSimplifier( simplifyMethod.tolerance() );
      return true;
    }
#endif

    QgsDebugMsg( QString( "Simplification method type (%1) is not recognised by OgrFeatureIterator class" ).arg( methodType ) );
  }
  return QgsAbstractFeatureIterator::prepareSimplification( simplifyMethod );
}

bool QgsOgrFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
#if defined(GDAL_VERSION_NUM) && defined(GDAL_COMPUTE_VERSION)
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,11,0)
  if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
  {
    return true;
  }
#endif
#endif
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1900
  if ( methodType == QgsSimplifyMethod::PreserveTopology )
  {
    return true;
  }
#endif

  return false;
}

bool QgsOgrFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    OGRFeatureH fet = OGR_L_GetFeature( ogrLayer, FID_TO_NUMBER( mRequest.filterFid() ) );
    if ( !fet )
    {
      close();
      return false;
    }

    if ( readFeature( fet, feature ) )
      OGR_F_Destroy( fet );

    feature.setValid( true );
    close(); // the feature has been read: we have finished here
    return true;
  }

  OGRFeatureH fet;

  while (( fet = OGR_L_GetNextFeature( ogrLayer ) ) )
  {
    if ( !readFeature( fet, feature ) )
      continue;

    if ( !mRequest.filterRect().isNull() && !feature.constGeometry() )
      continue;

    // we have a feature, end this cycle
    feature.setValid( true );
    OGR_F_Destroy( fet );
    return true;

  } // while

  close();
  return false;
}


bool QgsOgrFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  OGR_L_ResetReading( ogrLayer );

  return true;
}


bool QgsOgrFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  if ( mSubsetStringSet )
  {
    OGR_DS_ReleaseResultSet( mConn->ds, ogrLayer );
  }

  QgsOgrConnPool::instance()->releaseConnection( mConn );
  mConn = 0;

  mClosed = true;
  return true;
}


void QgsOgrFeatureIterator::getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex )
{
  OGRFieldDefnH fldDef = OGR_F_GetFieldDefnRef( ogrFet, attindex );

  if ( ! fldDef )
  {
    QgsDebugMsg( "ogrFet->GetFieldDefnRef(attindex) returns NULL" );
    return;
  }

  QVariant value;

  if ( OGR_F_IsFieldSet( ogrFet, attindex ) )
  {
    switch ( mSource->mFields[attindex].type() )
    {
      case QVariant::String: value = QVariant( mSource->mEncoding->toUnicode( OGR_F_GetFieldAsString( ogrFet, attindex ) ) ); break;
      case QVariant::Int: value = QVariant( OGR_F_GetFieldAsInteger( ogrFet, attindex ) ); break;
      case QVariant::Double: value = QVariant( OGR_F_GetFieldAsDouble( ogrFet, attindex ) ); break;
      case QVariant::Date:
      case QVariant::DateTime:
      {
        int year, month, day, hour, minute, second, tzf;

        OGR_F_GetFieldAsDateTime( ogrFet, attindex, &year, &month, &day, &hour, &minute, &second, &tzf );
        if ( mSource->mFields[attindex].type() == QVariant::Date )
          value = QDate( year, month, day );
        else
          value = QDateTime( QDate( year, month, day ), QTime( hour, minute, second ) );
      }
      break;
      default:
        assert( 0 && "unsupported field type" );
    }
  }
  else
  {
    value = QVariant( QString::null );
  }

  f.setAttribute( attindex, value );
}


bool QgsOgrFeatureIterator::readFeature( OGRFeatureH fet, QgsFeature& feature )
{
  feature.setFeatureId( OGR_F_GetFID( fet ) );
  feature.initAttributes( mSource->mFields.count() );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups

  bool useIntersect = mRequest.flags() & QgsFeatureRequest::ExactIntersect;
  bool geometryTypeFilter = mSource->mOgrGeometryTypeFilter != wkbUnknown;
  if ( mFetchGeometry || useIntersect || geometryTypeFilter )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet );

    if ( geom )
    {
      if ( mGeometrySimplifier )
        mGeometrySimplifier->simplifyGeometry( geom );

      // get the wkb representation
      int memorySize = OGR_G_WkbSize( geom );
      unsigned char *wkb = new unsigned char[memorySize];
      OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

      QgsGeometry* geometry = feature.geometry();
      if ( !geometry ) feature.setGeometryAndOwnership( wkb, memorySize ); else geometry->fromWkb( wkb, memorySize );
    }
    else
      feature.setGeometry( 0 );

    if (( useIntersect && ( !feature.constGeometry() || !feature.constGeometry()->intersects( mRequest.filterRect() ) ) )
        || ( geometryTypeFilter && ( !feature.constGeometry() || QgsOgrProvider::ogrWkbSingleFlatten(( OGRwkbGeometryType )feature.constGeometry()->wkbType() ) != mSource->mOgrGeometryTypeFilter ) ) )
    {
      OGR_F_Destroy( fet );
      return false;
    }
  }

  if ( !mFetchGeometry )
  {
    feature.setGeometry( 0 );
  }

  // fetch attributes
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    const QgsAttributeList& attrs = mRequest.subsetOfAttributes();
    for ( QgsAttributeList::const_iterator it = attrs.begin(); it != attrs.end(); ++it )
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


QgsOgrFeatureSource::QgsOgrFeatureSource( const QgsOgrProvider* p )
{
  mFilePath = p->filePath();
  mLayerName = p->layerName();
  mLayerIndex = p->layerIndex();
  mSubsetString = p->mSubsetString;
  mEncoding = p->mEncoding; // no copying - this is a borrowed pointer from Qt
  mFields = p->mAttributeFields;
  mDriverName = p->ogrDriverName;
  mOgrGeometryTypeFilter = wkbFlatten( p->mOgrGeometryTypeFilter );
  QgsOgrConnPool::refS( mFilePath );
}

QgsOgrFeatureSource::~QgsOgrFeatureSource()
{
  QgsOgrConnPool::unrefS( mFilePath );
}

QgsFeatureIterator QgsOgrFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsOgrFeatureIterator( this, false, request ) );
}
