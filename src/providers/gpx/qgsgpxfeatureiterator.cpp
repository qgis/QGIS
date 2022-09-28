/***************************************************************************
    qgsgpxfeatureiterator.cpp
    ---------------------
    begin                : Dezember 2012
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
#include "qgsgpxfeatureiterator.h"

#include "qgsgpxprovider.h"

#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsexception.h"
#include "qgsgeometryengine.h"

#include <limits>
#include <cstring>


QgsGPXFeatureIterator::QgsGPXFeatureIterator( QgsGPXFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsGPXFeatureSource>( source, ownSource, request )
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
    return;
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

  rewind();
}

QgsGPXFeatureIterator::~QgsGPXFeatureIterator()
{
  close();
}

bool QgsGPXFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mFetchedFid = false;
  }
  else
  {
    if ( mSource->mFeatureType == QgsGPXProvider::WaypointType )
      mWptIter = mSource->mData->waypointsBegin();
    else if ( mSource->mFeatureType == QgsGPXProvider::RouteType )
      mRteIter = mSource->mData->routesBegin();
    else if ( mSource->mFeatureType == QgsGPXProvider::TrackType )
      mTrkIter = mSource->mData->tracksBegin();
  }

  return true;
}

bool QgsGPXFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  mClosed = true;
  return true;
}

bool QgsGPXFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    bool res = readFid( feature );
    close();
    if ( res )
      geometryToDestinationCrs( feature, mTransform );

    if ( res && mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
    {
      res = false;
      feature.setValid( false );
    }

    return res;
  }

  if ( mSource->mFeatureType == QgsGPXProvider::WaypointType )
  {
    // go through the list of waypoints and return the first one that is in
    // the bounds rectangle
    for ( ; mWptIter != mSource->mData->waypointsEnd(); ++mWptIter )
    {
      if ( readWaypoint( *mWptIter, feature ) )
      {
        ++mWptIter;
        geometryToDestinationCrs( feature, mTransform );

        bool res = true;
        if ( res && mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
        {
          res = false;
        }

        if ( !res )
          continue;

        return true;
      }
    }
  }
  else if ( mSource->mFeatureType == QgsGPXProvider::RouteType )
  {
    // go through the routes and return the first one that is in the bounds
    // rectangle
    for ( ; mRteIter != mSource->mData->routesEnd(); ++mRteIter )
    {
      if ( readRoute( *mRteIter, feature ) )
      {
        ++mRteIter;
        geometryToDestinationCrs( feature, mTransform );

        bool res = true;
        if ( res && mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
        {
          res = false;
        }

        if ( !res )
          continue;

        return true;
      }
    }
  }
  else if ( mSource->mFeatureType == QgsGPXProvider::TrackType )
  {
    // go through the tracks and return the first one that is in the bounds
    // rectangle
    for ( ; mTrkIter != mSource->mData->tracksEnd(); ++mTrkIter )
    {
      if ( readTrack( *mTrkIter, feature ) )
      {
        ++mTrkIter;
        geometryToDestinationCrs( feature, mTransform );

        bool res = true;
        if ( res && mDistanceWithinEngine && mDistanceWithinEngine->distance( feature.geometry().constGet() ) > mRequest.distanceWithin() )
        {
          res = false;
        }

        if ( !res )
          continue;

        return true;
      }
    }
  }

  close();
  return false;
}


bool QgsGPXFeatureIterator::readFid( QgsFeature &feature )
{
  if ( mFetchedFid )
    return false;

  mFetchedFid = true;
  const QgsFeatureId fid = mRequest.filterFid();

  if ( mSource->mFeatureType == QgsGPXProvider::WaypointType )
  {
    for ( QgsGpsData::WaypointIterator it = mSource->mData->waypointsBegin() ; it != mSource->mData->waypointsEnd(); ++it )
    {
      if ( it->id == fid )
      {
        readWaypoint( *it, feature );
        return true;
      }
    }
  }
  else if ( mSource->mFeatureType == QgsGPXProvider::RouteType )
  {
    for ( QgsGpsData::RouteIterator it = mSource->mData->routesBegin() ; it != mSource->mData->routesEnd(); ++it )
    {
      if ( it->id == fid )
      {
        readRoute( *it, feature );
        return true;
      }
    }
  }
  else if ( mSource->mFeatureType == QgsGPXProvider::TrackType )
  {
    for ( QgsGpsData::TrackIterator it = mSource->mData->tracksBegin() ; it != mSource->mData->tracksEnd(); ++it )
    {
      if ( it->id == fid )
      {
        readTrack( *it, feature );
        return true;
      }
    }
  }

  return false;
}


bool QgsGPXFeatureIterator::readWaypoint( const QgsWaypoint &wpt, QgsFeature &feature )
{
  if ( !mFilterRect.isNull() )
  {
    if ( ! mFilterRect.contains( wpt.lon, wpt.lat ) )
      return false;
  }

  // some wkb voodoo
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) || !mFilterRect.isNull() )
  {
    QgsGeometry *g = readWaypointGeometry( wpt );
    feature.setGeometry( *g );
    delete g;
  }
  feature.setId( wpt.id );
  feature.setValid( true );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups
  feature.initAttributes( mSource->mFields.count() );

  readAttributes( feature, wpt );

  return true;
}


bool QgsGPXFeatureIterator::readRoute( const QgsRoute &rte, QgsFeature &feature )
{
  if ( rte.points.isEmpty() )
    return false;

  QgsGeometry *geometry = readRouteGeometry( rte );

  if ( !mFilterRect.isNull() )
  {
    if ( ( rte.xMax < mFilterRect.xMinimum() ) || ( rte.xMin > mFilterRect.xMaximum() ) ||
         ( rte.yMax < mFilterRect.yMinimum() ) || ( rte.yMin > mFilterRect.yMaximum() ) )
    {
      delete geometry;
      return false;
    }

    if ( !geometry->intersects( mFilterRect ) ) //use geos for precise intersection test
    {
      delete geometry;
      return false;
    }
  }

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) || !mFilterRect.isNull() )
  {
    feature.setGeometry( *geometry );
    delete geometry;
  }
  else
  {
    delete geometry;
  }
  feature.setId( rte.id );
  feature.setValid( true );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups
  feature.initAttributes( mSource->mFields.count() );

  readAttributes( feature, rte );

  return true;
}


bool QgsGPXFeatureIterator::readTrack( const QgsTrack &trk, QgsFeature &feature )
{
  //QgsDebugMsg( QStringLiteral( "GPX feature track segments: %1" ).arg( trk.segments.size() ) );

  QgsGeometry *geometry = readTrackGeometry( trk );

  if ( !mFilterRect.isNull() )
  {
    if ( ( trk.xMax < mFilterRect.xMinimum() ) || ( trk.xMin > mFilterRect.xMaximum() ) ||
         ( trk.yMax < mFilterRect.yMinimum() ) || ( trk.yMin > mFilterRect.yMaximum() ) )
    {
      delete geometry;
      return false;
    }

    if ( !geometry->intersects( mFilterRect ) ) //use geos for precise intersection test
    {
      delete geometry;
      return false;
    }
  }

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) || !mFilterRect.isNull() )
  {
    feature.setGeometry( *geometry );
    delete geometry;
  }
  else
  {
    delete geometry;
  }
  feature.setId( trk.id );
  feature.setValid( true );
  feature.setFields( mSource->mFields ); // allow name-based attribute lookups
  feature.initAttributes( mSource->mFields.count() );

  readAttributes( feature, trk );

  return true;
}


void QgsGPXFeatureIterator::readAttributes( QgsFeature &feature, const QgsWaypoint &wpt )
{
  // add attributes if they are wanted
  for ( int i = 0; i < mSource->mFields.count(); ++i )
  {
    switch ( mSource->mIndexToAttr.at( i ) )
    {
      case QgsGPXProvider::NameAttr:
        feature.setAttribute( i, QVariant( wpt.name ) );
        break;
      case QgsGPXProvider::EleAttr:
        if ( wpt.ele != -std::numeric_limits<double>::max() )
          feature.setAttribute( i, QVariant( wpt.ele ) );
        break;
      case QgsGPXProvider::SymAttr:
        feature.setAttribute( i, QVariant( wpt.sym ) );
        break;
      case QgsGPXProvider::CmtAttr:
        feature.setAttribute( i, QVariant( wpt.cmt ) );
        break;
      case QgsGPXProvider::DscAttr:
        feature.setAttribute( i, QVariant( wpt.desc ) );
        break;
      case QgsGPXProvider::SrcAttr:
        feature.setAttribute( i, QVariant( wpt.src ) );
        break;
      case QgsGPXProvider::URLAttr:
        feature.setAttribute( i, QVariant( wpt.url ) );
        break;
      case QgsGPXProvider::URLNameAttr:
        feature.setAttribute( i, QVariant( wpt.urlname ) );
        break;
    }
  }
}

void QgsGPXFeatureIterator::readAttributes( QgsFeature &feature, const QgsRoute &rte )
{
  // add attributes if they are wanted
  for ( int i = 0; i < mSource->mFields.count(); ++i )
  {
    switch ( mSource->mIndexToAttr.at( i ) )
    {
      case QgsGPXProvider::NameAttr:
        feature.setAttribute( i, QVariant( rte.name ) );
        break;
      case QgsGPXProvider::NumAttr:
        if ( rte.number != std::numeric_limits<int>::max() )
          feature.setAttribute( i, QVariant( rte.number ) );
        break;
      case QgsGPXProvider::CmtAttr:
        feature.setAttribute( i, QVariant( rte.cmt ) );
        break;
      case QgsGPXProvider::DscAttr:
        feature.setAttribute( i, QVariant( rte.desc ) );
        break;
      case QgsGPXProvider::SrcAttr:
        feature.setAttribute( i, QVariant( rte.src ) );
        break;
      case QgsGPXProvider::URLAttr:
        feature.setAttribute( i, QVariant( rte.url ) );
        break;
      case QgsGPXProvider::URLNameAttr:
        feature.setAttribute( i, QVariant( rte.urlname ) );
        break;
    }
  }
}


void QgsGPXFeatureIterator::readAttributes( QgsFeature &feature, const QgsTrack &trk )
{
  // add attributes if they are wanted
  for ( int i = 0; i < mSource->mFields.count(); ++i )
  {
    switch ( mSource->mIndexToAttr.at( i ) )
    {
      case QgsGPXProvider::NameAttr:
        feature.setAttribute( i, QVariant( trk.name ) );
        break;
      case QgsGPXProvider::NumAttr:
        if ( trk.number != std::numeric_limits<int>::max() )
          feature.setAttribute( i, QVariant( trk.number ) );
        break;
      case QgsGPXProvider::CmtAttr:
        feature.setAttribute( i, QVariant( trk.cmt ) );
        break;
      case QgsGPXProvider::DscAttr:
        feature.setAttribute( i, QVariant( trk.desc ) );
        break;
      case QgsGPXProvider::SrcAttr:
        feature.setAttribute( i, QVariant( trk.src ) );
        break;
      case QgsGPXProvider::URLAttr:
        feature.setAttribute( i, QVariant( trk.url ) );
        break;
      case QgsGPXProvider::URLNameAttr:
        feature.setAttribute( i, QVariant( trk.urlname ) );
        break;
    }
  }
}


QgsGeometry *QgsGPXFeatureIterator::readWaypointGeometry( const QgsWaypoint &wpt )
{
  const int size = 1 + sizeof( int ) + 2 * sizeof( double );
  unsigned char *geo = new unsigned char[size];

  QgsWkbPtr wkbPtr( geo, size );
  wkbPtr << ( char ) QgsApplication::endian() << QgsWkbTypes::Point << wpt.lon << wpt.lat;

  QgsGeometry *g = new QgsGeometry();
  g->fromWkb( geo, size );
  return g;
}


QgsGeometry *QgsGPXFeatureIterator::readRouteGeometry( const QgsRoute &rte )
{
  // some wkb voodoo
  const int size = 1 + 2 * sizeof( int ) + 2 * sizeof( double ) * rte.points.size();
  unsigned char *geo = new unsigned char[size];

  QgsWkbPtr wkbPtr( geo, size );
  wkbPtr << ( char ) QgsApplication::endian() << QgsWkbTypes::LineString << rte.points.size();

  for ( int i = 0; i < rte.points.size(); ++i )
  {
    wkbPtr << rte.points[i].lon << rte.points[i].lat;
  }

  //create QgsGeometry and use it for intersection test
  //if geometry is to be fetched, it is attached to the feature, otherwise we delete it
  QgsGeometry *g = new QgsGeometry();
  g->fromWkb( geo, size );
  return g;
}

QgsGeometry *QgsGPXFeatureIterator::readTrackGeometry( const QgsTrack &trk )
{
  // TODO: support multi line string for segments
  if ( trk.segments.isEmpty() )
    return nullptr;

  // A track consists of several segments. Add all those segments into one.
  int totalPoints = 0;
  for ( int i = 0; i < trk.segments.size(); i ++ )
  {
    totalPoints += trk.segments[i].points.size();
  }

  if ( totalPoints == 0 )
    return nullptr;

  //QgsDebugMsg( "GPX feature track total points: " + QString::number( totalPoints ) );

  // some wkb voodoo
  const int size = 1 + 2 * sizeof( int ) + 2 * sizeof( double ) * totalPoints;
  unsigned char *geo = new unsigned char[size];
  if ( !geo )
  {
    QgsDebugMsg( QStringLiteral( "Track too large!" ) );
    return nullptr;
  }

  QgsWkbPtr wkbPtr( geo, size );
  wkbPtr << ( char ) QgsApplication::endian() << QgsWkbTypes::LineString << totalPoints;

  for ( int k = 0; k < trk.segments.size(); k++ )
  {
    const int nPoints = trk.segments[k].points.size();
    for ( int i = 0; i < nPoints; ++i )
    {
      wkbPtr << trk.segments[k].points[i].lon << trk.segments[k].points[i].lat;
    }
  }

  //create QgsGeometry and use it for intersection test
  //if geometry is to be fetched, it is attached to the feature, otherwise we delete it
  QgsGeometry *g = new QgsGeometry();
  g->fromWkb( geo, size );
  return g;
}


// ------------

QgsGPXFeatureSource::QgsGPXFeatureSource( const QgsGPXProvider *p )
  : mFileName( p->mFileName )
  , mFeatureType( p->mFeatureType )
  , mIndexToAttr( p->mIndexToAttr )
  , mFields( p->mAttributeFields )
  , mCrs( p->crs() )
{
  mData = QgsGpsData::getData( mFileName );
}

QgsGPXFeatureSource::~QgsGPXFeatureSource()
{
  QgsGpsData::releaseData( mFileName );
}

QgsFeatureIterator QgsGPXFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsGPXFeatureIterator( this, false, request ) );
}
