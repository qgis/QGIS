/***************************************************************************
    qgsdelimitedtextfeatureiterator.cpp
    ---------------------
    begin                : Oktober 2012
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
#include "qgsdelimitedtextfeatureiterator.h"
#include "qgsdelimitedtextprovider.h"
#include "qgsdelimitedtextfile.h"

#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsspatialindex.h"

#include <QtAlgorithms>
#include <QTextStream>

QgsDelimitedTextFeatureIterator::QgsDelimitedTextFeatureIterator( QgsDelimitedTextProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request )
    , P( p )
{
  P->mActiveIterators << this;

  // Determine mode to use based on request...
  QgsDebugMsg( "Setting up QgsDelimitedTextIterator" );

  // Does the layer have geometry - will revise later to determine if we actually need to
  // load it.
  mLoadGeometry = P->mGeomRep != QgsDelimitedTextProvider::GeomNone;

  // Does the layer have an explicit or implicit subset (implicit subset is if we have geometry which can
  // be invalid)

  mTestSubset = P->mSubsetExpression;
  mTestGeometry = false;

  mMode = FileScan;
  if ( request.filterType() == QgsFeatureRequest::FilterFid )
  {
    QgsDebugMsg( "Configuring for returning single id" );
    mFeatureIds.append( request.filterFid() );
    mMode = FeatureIds;
    mTestSubset = false;
  }
  // If have geometry and testing geometry then evaluate options...
  // If we don't have geometry then all records pass geometry filter.
  // CC: 2013-05-09
  // Not sure about intended relationship between filtering on geometry and
  // requesting no geometry? Have preserved current logic of ignoring spatial filter
  // if not requesting geometry.

  else if ( request.filterType() == QgsFeatureRequest::FilterRect && mLoadGeometry
            && !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    QgsDebugMsg( "Configuring for rectangle select" );
    mTestGeometry = true;
    // Exact intersection test only applies for WKT geometries
    mTestGeometryExact = mRequest.flags() & QgsFeatureRequest::ExactIntersect
                         && P->mGeomRep == QgsDelimitedTextProvider::GeomAsWkt;

    QgsRectangle rect = request.filterRect();

    // If request doesn't overlap extents, then nothing to return
    if ( ! rect.intersects( P->extent() ) )
    {
      QgsDebugMsg( "Rectangle outside layer extents - no features to return" );
      mMode = FeatureIds;
    }
    // If the request extents include the entire layer, then revert to
    // a file scan

    else if ( rect.contains( P->extent() ) )
    {
      QgsDebugMsg( "Rectangle contains layer extents - bypass spatial filter" );
      mTestGeometry = false;
    }
    // If we have a spatial index then use it.  The spatial index already accounts
    // for the subset.  Also means we don't have to test geometries unless doing exact
    // intersection

    else if ( P->mUseSpatialIndex )
    {
      mFeatureIds = P->mSpatialIndex->intersects( rect );
      // Sort for efficient sequential retrieval
      qSort( mFeatureIds.begin(), mFeatureIds.end() );
      QgsDebugMsg( QString( "Layer has spatial index - selected %1 features from index" ).arg( mFeatureIds.size() ) );
      mMode = FeatureIds;
      mTestSubset = false;
      mTestGeometry = mTestGeometryExact;
    }
  }

  // If we have a subset index then use it..
  if ( mMode == FileScan && P->mUseSubsetIndex )
  {
    QgsDebugMsg( QString( "Layer has subset index - use %1 items from subset index" ).arg( P->mSubsetIndex.size() ) );
    mTestSubset = false;
    mMode = SubsetIndex;
  }

  // Otherwise just have to scan the file
  if ( mMode == FileScan )
  {
    QgsDebugMsg( "File will be scanned for desired features" );
  }

  // If the request does not require geometry, can we avoid loading it?
  // We need it if we are testing geometry (ie spatial filter), or
  // if testing the subset expression, and it uses geometry.
  if ( mRequest.flags() & QgsFeatureRequest::NoGeometry &&
       ! mTestGeometry &&
       !( mTestSubset && P->mSubsetExpression->needsGeometry() ) )
  {
    QgsDebugMsg( "Feature geometries not required" );
    mLoadGeometry = false;
  }

  QgsDebugMsg( QString( "Iterator is scanning file: " ) + ( scanningFile() ? "Yes" : "No" ) );
  QgsDebugMsg( QString( "Iterator is loading geometries: " ) + ( loadGeometry() ? "Yes" : "No" ) );
  QgsDebugMsg( QString( "Iterator is testing geometries: " ) + ( testGeometry() ? "Yes" : "No" ) );
  QgsDebugMsg( QString( "Iterator is testing subset: " ) + ( testSubset() ? "Yes" : "No" ) );

  rewind();
}

QgsDelimitedTextFeatureIterator::~QgsDelimitedTextFeatureIterator()
{
  close();
}

bool QgsDelimitedTextFeatureIterator::nextFeature( QgsFeature& feature )
{
  // before we do anything else, assume that there's something wrong with
  // the feature
  feature.setValid( false );

  if ( mClosed )
    return false;

  bool gotFeature = false;
  if ( mMode == FileScan )
  {
    gotFeature =  P->nextFeature( feature, P->mFile, this );
  }
  else
  {
    while ( ! gotFeature )
    {
      qint64 fid = -1;
      if ( mMode == FeatureIds )
      {
        if ( mNextId < mFeatureIds.size() )
        {
          fid = mFeatureIds[mNextId];
        }
      }
      else if ( mNextId < P->mSubsetIndex.size() )
      {
        fid = P->mSubsetIndex[mNextId];
      }
      if ( fid < 0 ) break;
      mNextId++;
      gotFeature = ( P->setNextFeatureId( fid ) && P->nextFeature( feature, P->mFile, this ) );
    }
  }

  // CC: 2013-05-08:  What is the intent of rewind/close.  The following
  // line from previous implementation means that we cannot rewind the iterator
  // after reading last record? Is this correct?  This line can be removed if
  // not.

  if ( ! gotFeature ) close();

  return gotFeature;
}

bool QgsDelimitedTextFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  // Skip to first data record
  if ( mMode == FileScan )
  {
    P->resetStream();
  }
  else
  {
    mNextId = 0;
  }
  return true;
}

bool QgsDelimitedTextFeatureIterator::close()
{
  if ( mClosed )
    return false;

  P->mActiveIterators.remove( this );

  mFeatureIds = QList<QgsFeatureId>();
  mClosed = true;
  return true;
}

/**
 * Check to see if the point is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::wantGeometry( const QgsPoint &pt ) const
{
  if ( ! mTestGeometry ) return true;
  return mRequest.filterRect().contains( pt );
}

/**
 * Check to see if the geometry is within the selection rectangle
 */
bool QgsDelimitedTextFeatureIterator::wantGeometry( QgsGeometry *geom ) const
{
  if ( ! mTestGeometry ) return true;

  if ( mTestGeometryExact )
    return geom->intersects( mRequest.filterRect() );
  else
    return geom->boundingBox().intersects( mRequest.filterRect() );
}
