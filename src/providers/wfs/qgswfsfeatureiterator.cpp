/***************************************************************************
    qgswfsfeatureiterator.cpp
    ---------------------
    begin                : Januar 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsfeatureiterator.h"
#include "qgsspatialindex.h"
#include "qgswfsprovider.h"
#include "qgsmessagelog.h"
#include "qgsgeometry.h"

QgsWFSFeatureIterator::QgsWFSFeatureIterator( QgsWFSFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsWFSFeatureSource>( source, ownSource, request )
{
  switch ( request.filterType() )
  {
    case QgsFeatureRequest::FilterRect:
      if ( mSource->mSpatialIndex )
      {
        mSelectedFeatures = mSource->mSpatialIndex->intersects( request.filterRect() );
      }
      break;
    case QgsFeatureRequest::FilterFid:
      mSelectedFeatures.push_back( request.filterFid() );
      break;
    case QgsFeatureRequest::FilterNone:
    default:
      mSelectedFeatures = mSource->mFeatures.keys();
  }

  mFeatureIterator = mSelectedFeatures.constBegin();
}

QgsWFSFeatureIterator::~QgsWFSFeatureIterator()
{
  close();
}

bool QgsWFSFeatureIterator::fetchFeature( QgsFeature& f )
{
  if ( mClosed )
    return false;

  if ( mFeatureIterator == mSelectedFeatures.constEnd() )
  {
    return false;
  }

  const QgsFeature *fet = 0;

  for ( ;; )
  {
    if ( mFeatureIterator == mSelectedFeatures.constEnd() )
      return false;

    QgsFeaturePtrMap::const_iterator it = mSource->mFeatures.constFind( *mFeatureIterator );
    if ( it == mSource->mFeatures.constEnd() )
      return false;

    fet = it.value();
    if (( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) == 0 )
      break;

    if ( fet->constGeometry() && fet->constGeometry()->intersects( mRequest.filterRect() ) )
      break;

    ++mFeatureIterator;
  }


  copyFeature( fet, f, !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) );
  ++mFeatureIterator;
  return true;
}

bool QgsWFSFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  mFeatureIterator = mSelectedFeatures.constBegin();

  return true;
}

bool QgsWFSFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  mClosed = true;
  return true;
}



void QgsWFSFeatureIterator::copyFeature( const QgsFeature* f, QgsFeature& feature, bool fetchGeometry )
{
  Q_UNUSED( fetchGeometry );

  if ( !f )
  {
    return;
  }

  //copy the geometry
  const QgsGeometry* geometry = f->constGeometry();
  if ( geometry && fetchGeometry )
  {
    const unsigned char *geom = geometry->asWkb();
    int geomSize = geometry->wkbSize();
    unsigned char* copiedGeom = new unsigned char[geomSize];
    memcpy( copiedGeom, geom, geomSize );
    feature.setGeometryAndOwnership( copiedGeom, geomSize );
  }
  else
  {
    feature.setGeometry( 0 );
  }

  //and the attributes
  feature.initAttributes( mSource->mFields.size() );
  for ( int i = 0; i < mSource->mFields.size(); i++ )
  {
    const QVariant &v = f->attributes().value( i );
    if ( v.type() != mSource->mFields[i].type() )
      feature.setAttribute( i, QgsVectorDataProvider::convertValue( mSource->mFields[i].type(), v.toString() ) );
    else
      feature.setAttribute( i, v );
  }

  //id and valid
  feature.setValid( true );
  feature.setFeatureId( f->id() );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups
}


// -------------------------

QgsWFSFeatureSource::QgsWFSFeatureSource( const QgsWFSProvider* p )
    : QObject(( QgsWFSProvider* ) p )
    , mFields( p->mFields )
    , mFeatures( p->mFeatures )
    , mSpatialIndex( p->mSpatialIndex ? new QgsSpatialIndex( *p->mSpatialIndex ) : 0 )  // just shallow copy
{
}

QgsWFSFeatureSource::~QgsWFSFeatureSource()
{
  delete mSpatialIndex;
}

QgsFeatureIterator QgsWFSFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  if ( request.filterType() == QgsFeatureRequest::FilterRect )
    emit extentRequested( request.filterRect() );
  return QgsFeatureIterator( new QgsWFSFeatureIterator( this, false, request ) );
}
