/***************************************************************************
 *  qgsgeometrycheckerutils.cpp                                            *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckerutils.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgsfeaturepool.h"
#include <qmath.h>
#include "qgsgeos.h"
#include "qgsgeometrycollection.h"
#include "qgssurface.h"

namespace QgsGeometryCheckerUtils
{
  LayerFeature::LayerFeature( const QgsFeaturePool *pool, const QgsFeature &feature, bool useMapCrs )
    : mLayer( pool->getLayer() )
    , mFeature( feature )
    , mLayerToMapUnits( pool->getLayerToMapUnits() )
    , mLayerToMapTransform( pool->getLayerToMapTransform() )
    , mClonedGeometry( false )
    , mMapCrs( useMapCrs )
  {
    mGeometry = feature.geometry().geometry();
    if ( useMapCrs && !mLayerToMapTransform.isShortCircuited() )
    {
      mClonedGeometry = true;
      mGeometry = mGeometry->clone();
      mGeometry->transform( mLayerToMapTransform );
    }
  }
  LayerFeature::~LayerFeature()
  {
    if ( mClonedGeometry )
    {
      delete mGeometry;
    }
  }

/////////////////////////////////////////////////////////////////////////////

  LayerFeatures::iterator::iterator( const QList<QString>::iterator &layerIt, const QgsFeatureIds::const_iterator &featureIt, const QgsFeature &feature, LayerFeatures *parent )
    : mLayerIt( layerIt )
    , mFeatureIt( featureIt )
    , mFeature( feature )
    , mParent( parent )
    , mCurrentFeature( LayerFeature( parent->mFeaturePools[ * layerIt], feature, parent->mUseMapCrs ) )
  {
  }

  const LayerFeatures::iterator &LayerFeatures::iterator::operator++()
  {
    if ( nextFeature() )
    {
      return *this;
    }
    do
    {
      ++mLayerIt;
      mFeatureIt = mParent->mFeatureIds[*mLayerIt].begin();
      if ( mParent->mGeometryTypes.contains( mParent->mFeaturePools[*mLayerIt]->getLayer()->geometryType() ) && nextFeature() )
      {
        return *this;
      }
    }
    while ( mLayerIt != mParent->mLayerIds.end() );
    return *this;
  }

  bool LayerFeatures::iterator::nextFeature()
  {
    QgsFeaturePool *featurePool = mParent->mFeaturePools[*mLayerIt];
    const QgsFeatureIds &featureIds = mParent->mFeatureIds[*mLayerIt];
    do
    {
      ++mFeatureIt;
      if ( mParent->mProgressCounter )
        mParent->mProgressCounter->fetchAndAddRelaxed( 1 );
      if ( featurePool->get( *mFeatureIt, mFeature ) )
      {
        mCurrentFeature = LayerFeature( mParent->mFeaturePools[*mLayerIt], mFeature, mParent->mUseMapCrs );
        return true;
      }
    }
    while ( mFeatureIt != featureIds.end() );
    return false;
  }

/////////////////////////////////////////////////////////////////////////////

  LayerFeatures::LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
                                const QMap<QString, QgsFeatureIds> &featureIds,
                                const QList<QgsWkbTypes::GeometryType> &geometryTypes,
                                QAtomicInt *progressCounter, bool useMapCrs )
    : mFeaturePools( featurePools )
    , mFeatureIds( featureIds )
    , mLayerIds( featurePools.keys() )
    , mGeometryTypes( geometryTypes )
    , mProgressCounter( progressCounter )
    , mUseMapCrs( useMapCrs )
  {}

  LayerFeatures::LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
                                const QList<QString> &layerIds, const QgsRectangle &extent,
                                const QList<QgsWkbTypes::GeometryType> &geometryTypes )
    : mFeaturePools( featurePools )
    , mLayerIds( layerIds )
    , mExtent( extent )
    , mGeometryTypes( geometryTypes )
    , mUseMapCrs( true )
  {
    for ( const QString &layerId : layerIds )
    {
      const QgsFeaturePool *featurePool = featurePools[layerId];
      if ( geometryTypes.contains( featurePool->getLayer()->geometryType() ) )
      {
        mFeatureIds.insert( layerId, featurePool->getIntersects( featurePool->getLayerToMapTransform().transform( extent, QgsCoordinateTransform::ReverseTransform ) ) );
      }
      else
      {
        mFeatureIds.insert( layerId, QgsFeatureIds() );
      }
    }
  }

  LayerFeatures::iterator LayerFeatures::begin()
  {
    for ( auto layerIt = mLayerIds.begin(), layerItEnd = mLayerIds.end(); layerIt != layerItEnd; ++layerIt )
    {
      if ( !mGeometryTypes.contains( mFeaturePools[*layerIt]->getLayer()->geometryType() ) )
      {
        continue;
      }
      const QgsFeatureIds &featureIds = mFeatureIds[*layerIt];
      for ( auto featureIt = featureIds.begin(), featureItEnd = featureIds.end(); featureIt != featureItEnd; ++featureIt )
      {
        if ( mProgressCounter )
          mProgressCounter->fetchAndAddRelaxed( 1 );
        QgsFeature feature;
        if ( mFeaturePools[*layerIt]->get( *featureIt, feature ) )
        {
          return iterator( layerIt, featureIt, feature, this );
        }
      }
    }
    return end();
  }

  /////////////////////////////////////////////////////////////////////////////

  QSharedPointer<QgsGeometryEngine> createGeomEngine( const QgsAbstractGeometry *geometry, double tolerance )
  {
    return QSharedPointer<QgsGeometryEngine>( new QgsGeos( geometry, tolerance ) );
  }

  QgsAbstractGeometry *getGeomPart( QgsAbstractGeometry *geom, int partIdx )
  {
    if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
    {
      return static_cast<QgsGeometryCollection *>( geom )->geometryN( partIdx );
    }
    return geom;
  }

  const QgsAbstractGeometry *getGeomPart( const QgsAbstractGeometry *geom, int partIdx )
  {
    if ( dynamic_cast<const QgsGeometryCollection *>( geom ) )
    {
      return static_cast<const QgsGeometryCollection *>( geom )->geometryN( partIdx );
    }
    return geom;
  }

  void filter1DTypes( QgsAbstractGeometry *geom )
  {
    if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
    {
      QgsGeometryCollection *geomCollection = static_cast<QgsGeometryCollection *>( geom );
      for ( int nParts = geom->partCount(), iPart = nParts - 1; iPart >= 0; --iPart )
      {
        if ( !dynamic_cast<QgsSurface *>( geomCollection->geometryN( iPart ) ) )
        {
          geomCollection->removeGeometry( iPart );
        }
      }
    }
  }

  static inline double pointLineDist( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &q )
  {
    double nom = std::fabs( ( p2.y() - p1.y() ) * q.x() - ( p2.x() - p1.x() ) * q.y() + p2.x() * p1.y() - p2.y() * p1.x() );
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return nom / std::sqrt( dx * dx + dy * dy );
  }

  bool pointOnLine( const QgsPoint &p, const QgsLineString *line, double tol, bool excludeExtremities )
  {
    int nVerts = line->vertexCount();
    for ( int i = 0 + excludeExtremities; i < nVerts - 1 - excludeExtremities; ++i )
    {
      QgsPoint p1 = line->vertexAt( QgsVertexId( 0, 0, i ) );
      QgsPoint p2 = line->vertexAt( QgsVertexId( 0, 0, i + 1 ) );
      double dist = pointLineDist( p1, p2, p );
      if ( dist < tol )
      {
        return true;
      }
    }
    return false;
  }

  bool linesIntersect( const QgsLineString *line1, const QgsLineString *line2, double tol, QgsPoint &inter )
  {
    for ( int i = 0, n = line1->vertexCount() - 1; i < n; ++i )
    {
      for ( int j = 0, m = line2->vertexCount() - 1; j < m; ++j )
      {
        QgsPoint p1 = line1->vertexAt( QgsVertexId( 0, 0, i ) );
        QgsPoint p2 = line1->vertexAt( QgsVertexId( 0, 0, i + 1 ) );
        QgsPoint q1 = line1->vertexAt( QgsVertexId( 0, 0, j ) );
        QgsPoint q2 = line1->vertexAt( QgsVertexId( 0, 0, j + 1 ) );
        if ( QgsGeometryUtils::segmentIntersection( p1, p2, q1, q2, inter, tol ) )
        {
          return true;
        }
      }
    }
    return false;
  }

  double sharedEdgeLength( const QgsAbstractGeometry *geom1, const QgsAbstractGeometry *geom2, double tol )
  {
    double len = 0;

    // Test every pair of segments for shared edges
    for ( int iPart1 = 0, nParts1 = geom1->partCount(); iPart1 < nParts1; ++iPart1 )
    {
      for ( int iRing1 = 0, nRings1 = geom1->ringCount( iPart1 ); iRing1 < nRings1; ++iRing1 )
      {
        for ( int iVert1 = 0, jVert1 = 1, nVerts1 = geom1->vertexCount( iPart1, iRing1 ); jVert1 < nVerts1; iVert1 = jVert1++ )
        {
          QgsPoint p1 = geom1->vertexAt( QgsVertexId( iPart1, iRing1, iVert1 ) );
          QgsPoint p2 = geom1->vertexAt( QgsVertexId( iPart1, iRing1, jVert1 ) );
          double lambdap1 = 0.;
          double lambdap2 = std::sqrt( QgsGeometryUtils::sqrDistance2D( p1, p2 ) );
          QgsVector d;
          try
          {
            d = QgsVector( p2.x() - p1.x(), p2.y() - p1.y() ).normalized();
          }
          catch ( const QgsException & )
          {
            // Edge has zero length, skip
            continue;
          }

          for ( int iPart2 = 0, nParts2 = geom2->partCount(); iPart2 < nParts2; ++iPart2 )
          {
            for ( int iRing2 = 0, nRings2 = geom2->ringCount( iPart2 ); iRing2 < nRings2; ++iRing2 )
            {
              for ( int iVert2 = 0, jVert2 = 1, nVerts2 = geom2->vertexCount( iPart2, iRing2 ); jVert2 < nVerts2; iVert2 = jVert2++ )
              {
                QgsPoint q1 = geom2->vertexAt( QgsVertexId( iPart2, iRing2, iVert2 ) );
                QgsPoint q2 = geom2->vertexAt( QgsVertexId( iPart2, iRing2, jVert2 ) );

                // Check whether q1 and q2 are on the line p1, p
                if ( pointLineDist( p1, p2, q1 ) <= tol && pointLineDist( p1, p2, q2 ) <= tol )
                {
                  // Get length common edge
                  double lambdaq1 = QgsVector( q1.x() - p1.x(), q1.y() - p1.y() ) * d;
                  double lambdaq2 = QgsVector( q2.x() - p1.x(), q2.y() - p1.y() ) * d;
                  if ( lambdaq1 > lambdaq2 )
                  {
                    std::swap( lambdaq1, lambdaq2 );
                  }
                  double lambda1 = std::max( lambdaq1, lambdap1 );
                  double lambda2 = std::min( lambdaq2, lambdap2 );
                  len += std::max( 0., lambda2 - lambda1 );
                }
              }
            }
          }
        }
      }
    }
    return len;
  }

} // QgsGeometryCheckerUtils
