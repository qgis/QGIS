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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerutils.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgsfeaturepool.h"
#include "qgspolygon.h"
#include "qgsgeos.h"
#include "qgsgeometrycollection.h"
#include "qgssurface.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheck.h"
#include "qgsfeedback.h"

#include <qmath.h>

QgsGeometryCheckerUtils::LayerFeature::LayerFeature( const QgsFeaturePool *pool,
    const QgsFeature &feature,
    const QgsGeometryCheckContext *context,
    bool useMapCrs )
  : mFeaturePool( pool )
  , mFeature( feature )
  , mGeometry( feature.geometry() )
  , mMapCrs( useMapCrs )
{
  const QgsCoordinateTransform transform( pool->crs(), context->mapCrs, context->transformContext );
  if ( useMapCrs && context->mapCrs.isValid() && !transform.isShortCircuited() )
  {
    try
    {
      mGeometry.transform( transform );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Shrug. What shall we do with a geometry that cannot be converted?" ) );
    }
  }
}

QgsFeature QgsGeometryCheckerUtils::LayerFeature::feature() const
{
  return mFeature;
}

QPointer<QgsVectorLayer> QgsGeometryCheckerUtils::LayerFeature::layer() const
{
  return mFeaturePool->layerPtr();
}

QString QgsGeometryCheckerUtils::LayerFeature::layerId() const
{
  return mFeaturePool->layerId();
}

QgsGeometry QgsGeometryCheckerUtils::LayerFeature::geometry() const
{
  return mGeometry;
}

QString QgsGeometryCheckerUtils::LayerFeature::id() const
{
  return QStringLiteral( "%1:%2" ).arg( mFeaturePool->layerName() ).arg( mFeature.id() );
}

bool QgsGeometryCheckerUtils::LayerFeature::operator==( const LayerFeature &other ) const
{
  return layerId() == other.layerId() && mFeature.id() == other.mFeature.id();
}

bool QgsGeometryCheckerUtils::LayerFeature::operator!=( const LayerFeature &other ) const
{
  return layerId() != other.layerId() || mFeature.id() != other.mFeature.id();
}

/////////////////////////////////////////////////////////////////////////////

QgsGeometryCheckerUtils::LayerFeatures::iterator::iterator( const QStringList::const_iterator &layerIt, const LayerFeatures *parent )
  : mLayerIt( layerIt )
  , mFeatureIt( QgsFeatureIds::const_iterator() )
  , mParent( parent )
{
  nextLayerFeature( true );
}

QgsGeometryCheckerUtils::LayerFeatures::iterator::iterator( const QgsGeometryCheckerUtils::LayerFeatures::iterator &rh )
  : mLayerIt( rh.mLayerIt )
  , mFeatureIt( rh.mFeatureIt )
  , mParent( rh.mParent )
  , mCurrentFeature( std::make_unique<LayerFeature>( *rh.mCurrentFeature.get() ) )
{
}

bool QgsGeometryCheckerUtils::LayerFeature::useMapCrs() const
{
  return mMapCrs;
}
QgsGeometryCheckerUtils::LayerFeatures::iterator::~iterator()
{
}

QgsGeometryCheckerUtils::LayerFeatures::iterator QgsGeometryCheckerUtils::LayerFeatures::iterator::operator++( int )
{
  const iterator tmp( *this );
  ++*this;
  return tmp;
}

const QgsGeometryCheckerUtils::LayerFeature &QgsGeometryCheckerUtils::LayerFeatures::iterator::operator*() const
{
  Q_ASSERT( mCurrentFeature );
  return *mCurrentFeature;
}

bool QgsGeometryCheckerUtils::LayerFeatures::iterator::operator!=( const QgsGeometryCheckerUtils::LayerFeatures::iterator &other ) const
{
  return mLayerIt != other.mLayerIt || mFeatureIt != other.mFeatureIt;
}

const QgsGeometryCheckerUtils::LayerFeatures::iterator &QgsGeometryCheckerUtils::LayerFeatures::iterator::operator++()
{
  nextLayerFeature( false );
  return *this;
}
bool QgsGeometryCheckerUtils::LayerFeatures::iterator::nextLayerFeature( bool begin )
{
  if ( !begin && nextFeature( false ) )
  {
    return true;
  }
  while ( nextLayer( begin ) )
  {
    begin = false;
    if ( nextFeature( true ) )
    {
      return true;
    }
  }
  // End
  mFeatureIt = QgsFeatureIds::const_iterator();
  mCurrentFeature.reset();
  return false;
}

bool QgsGeometryCheckerUtils::LayerFeatures::iterator::nextLayer( bool begin )
{
  if ( !begin )
  {
    ++mLayerIt;
  }
  while ( true )
  {
    if ( mLayerIt == mParent->mLayerIds.end() )
    {
      break;
    }
    if ( mParent->mGeometryTypes.contains( mParent->mFeaturePools[*mLayerIt]->geometryType() ) )
    {
      mFeatureIt = mParent->mFeatureIds[*mLayerIt].constBegin();
      return true;
    }
    ++mLayerIt;
  }
  return false;
}

bool QgsGeometryCheckerUtils::LayerFeatures::iterator::nextFeature( bool begin )
{
  QgsFeaturePool *featurePool = mParent->mFeaturePools[*mLayerIt];
  const QgsFeatureIds &featureIds = mParent->mFeatureIds[*mLayerIt];
  if ( !begin )
  {
    ++mFeatureIt;
  }
  while ( true )
  {
    if ( mFeatureIt == featureIds.end() )
    {
      break;
    }
    if ( mParent->mFeedback )
      mParent->mFeedback->setProgress( mParent->mFeedback->progress() + 1.0 );
    QgsFeature feature;
    if ( featurePool->getFeature( *mFeatureIt, feature ) && !feature.geometry().isNull() )
    {
      mCurrentFeature = std::make_unique<LayerFeature>( featurePool, feature, mParent->mContext, mParent->mUseMapCrs );
      return true;
    }
    ++mFeatureIt;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////

QgsGeometryCheckerUtils::LayerFeatures::LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
    const QMap<QString, QgsFeatureIds> &featureIds,
    const QList<QgsWkbTypes::GeometryType> &geometryTypes,
    QgsFeedback *feedback,
    const QgsGeometryCheckContext *context,
    bool useMapCrs )
  : mFeaturePools( featurePools )
  , mFeatureIds( featureIds )
  , mLayerIds( featurePools.keys() )
  , mGeometryTypes( geometryTypes )
  , mFeedback( feedback )
  , mContext( context )
  , mUseMapCrs( useMapCrs )
{}

QgsGeometryCheckerUtils::LayerFeatures::LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
    const QList<QString> &layerIds, const QgsRectangle &extent,
    const QList<QgsWkbTypes::GeometryType> &geometryTypes,
    const QgsGeometryCheckContext *context )
  : mFeaturePools( featurePools )
  , mLayerIds( layerIds )
  , mExtent( extent )
  , mGeometryTypes( geometryTypes )
  , mContext( context )
  , mUseMapCrs( true )
{
  for ( const QString &layerId : layerIds )
  {
    const QgsFeaturePool *featurePool = featurePools[layerId];
    if ( geometryTypes.contains( featurePool->geometryType() ) )
    {
      const QgsCoordinateTransform ct( featurePool->crs(), context->mapCrs, context->transformContext );
      mFeatureIds.insert( layerId, featurePool->getIntersects( ct.transform( extent, Qgis::TransformDirection::Reverse ) ) );
    }
    else
    {
      mFeatureIds.insert( layerId, QgsFeatureIds() );
    }
  }
}

QgsGeometryCheckerUtils::LayerFeatures::iterator QgsGeometryCheckerUtils::LayerFeatures::begin() const
{
  return iterator( mLayerIds.constBegin(), this );
}

QgsGeometryCheckerUtils::LayerFeatures::iterator QgsGeometryCheckerUtils::LayerFeatures::end() const
{
  return iterator( mLayerIds.end(), this );
}

/////////////////////////////////////////////////////////////////////////////

std::unique_ptr<QgsGeometryEngine> QgsGeometryCheckerUtils::createGeomEngine( const QgsAbstractGeometry *geometry, double tolerance )
{
  return std::make_unique<QgsGeos>( geometry, tolerance );
}

QgsAbstractGeometry *QgsGeometryCheckerUtils::getGeomPart( QgsAbstractGeometry *geom, int partIdx )
{
  if ( dynamic_cast<QgsGeometryCollection *>( geom ) )
  {
    return static_cast<QgsGeometryCollection *>( geom )->geometryN( partIdx );
  }
  return geom;
}

const QgsAbstractGeometry *QgsGeometryCheckerUtils::getGeomPart( const QgsAbstractGeometry *geom, int partIdx )
{
  if ( dynamic_cast<const QgsGeometryCollection *>( geom ) )
  {
    return static_cast<const QgsGeometryCollection *>( geom )->geometryN( partIdx );
  }
  return geom;
}

QList<const QgsLineString *> QgsGeometryCheckerUtils::polygonRings( const QgsPolygon *polygon )
{
  QList<const QgsLineString *> rings;
  if ( const QgsLineString *exterior = dynamic_cast<const QgsLineString *>( polygon->exteriorRing() ) )
  {
    rings.append( exterior );
  }
  for ( int iInt = 0, nInt = polygon->numInteriorRings(); iInt < nInt; ++iInt )
  {
    if ( const QgsLineString *interior = dynamic_cast<const QgsLineString *>( polygon->interiorRing( iInt ) ) )
    {
      rings.append( interior );
    }
  }
  return rings;
}

void QgsGeometryCheckerUtils::filter1DTypes( QgsAbstractGeometry *geom )
{
  if ( qgsgeometry_cast<QgsGeometryCollection *>( geom ) )
  {
    QgsGeometryCollection *geomCollection = static_cast<QgsGeometryCollection *>( geom );
    for ( int nParts = geom->partCount(), iPart = nParts - 1; iPart >= 0; --iPart )
    {
      if ( !qgsgeometry_cast<QgsSurface *>( geomCollection->geometryN( iPart ) ) )
      {
        geomCollection->removeGeometry( iPart );
      }
    }
  }
}

double pointLineDist( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &q )
{
  const double nom = std::fabs( ( p2.y() - p1.y() ) * q.x() - ( p2.x() - p1.x() ) * q.y() + p2.x() * p1.y() - p2.y() * p1.x() );
  const double dx = p2.x() - p1.x();
  const double dy = p2.y() - p1.y();
  return nom / std::sqrt( dx * dx + dy * dy );
}

bool QgsGeometryCheckerUtils::pointOnLine( const QgsPoint &p, const QgsLineString *line, double tol, bool excludeExtremities )
{
  const int nVerts = line->vertexCount();
  for ( int i = 0 + excludeExtremities; i < nVerts - 1 - excludeExtremities; ++i )
  {
    const QgsPoint p1 = line->vertexAt( QgsVertexId( 0, 0, i ) );
    const QgsPoint p2 = line->vertexAt( QgsVertexId( 0, 0, i + 1 ) );
    const double dist = pointLineDist( p1, p2, p );
    if ( dist < tol )
    {
      return true;
    }
  }
  return false;
}

QList<QgsPoint> QgsGeometryCheckerUtils::lineIntersections( const QgsLineString *line1, const QgsLineString *line2, double tol )
{
  QList<QgsPoint> intersections;
  QgsPoint inter;
  bool intersection = false;
  for ( int i = 0, n = line1->vertexCount() - 1; i < n; ++i )
  {
    for ( int j = 0, m = line2->vertexCount() - 1; j < m; ++j )
    {
      const QgsPoint p1 = line1->vertexAt( QgsVertexId( 0, 0, i ) );
      const QgsPoint p2 = line1->vertexAt( QgsVertexId( 0, 0, i + 1 ) );
      const QgsPoint q1 = line2->vertexAt( QgsVertexId( 0, 0, j ) );
      const QgsPoint q2 = line2->vertexAt( QgsVertexId( 0, 0, j + 1 ) );
      if ( QgsGeometryUtils::segmentIntersection( p1, p2, q1, q2, inter, intersection, tol ) )
      {
        intersections.append( inter );
      }
    }
  }
  return intersections;
}

double QgsGeometryCheckerUtils::sharedEdgeLength( const QgsAbstractGeometry *geom1, const QgsAbstractGeometry *geom2, double tol )
{
  double len = 0;

  // Test every pair of segments for shared edges
  for ( int iPart1 = 0, nParts1 = geom1->partCount(); iPart1 < nParts1; ++iPart1 )
  {
    for ( int iRing1 = 0, nRings1 = geom1->ringCount( iPart1 ); iRing1 < nRings1; ++iRing1 )
    {
      for ( int iVert1 = 0, jVert1 = 1, nVerts1 = geom1->vertexCount( iPart1, iRing1 ); jVert1 < nVerts1; iVert1 = jVert1++ )
      {
        const QgsPoint p1 = geom1->vertexAt( QgsVertexId( iPart1, iRing1, iVert1 ) );
        const QgsPoint p2 = geom1->vertexAt( QgsVertexId( iPart1, iRing1, jVert1 ) );
        const double lambdap1 = 0.;
        const double lambdap2 = std::sqrt( QgsGeometryUtils::sqrDistance2D( p1, p2 ) );
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
              const QgsPoint q1 = geom2->vertexAt( QgsVertexId( iPart2, iRing2, iVert2 ) );
              const QgsPoint q2 = geom2->vertexAt( QgsVertexId( iPart2, iRing2, jVert2 ) );

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
                const double lambda1 = std::max( lambdaq1, lambdap1 );
                const double lambda2 = std::min( lambdaq2, lambdap2 );
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
