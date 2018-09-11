/***************************************************************************
 *  qgsgeometrycheckerutils.h                                              *
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

#define SIP_NO_FILE

#ifndef QGS_GEOMETRYCHECKERUTILS_H
#define QGS_GEOMETRYCHECKERUTILS_H

#include "qgsfeature.h"
#include "geometry/qgsabstractgeometry.h"
#include "geometry/qgspoint.h"
#include <qmath.h>

class QgsGeometryEngine;
class QgsFeaturePool;
struct QgsGeometryCheckerContext;

namespace QgsGeometryCheckerUtils
{
  class LayerFeature
  {
    public:
      LayerFeature( const QgsFeaturePool *pool, const QgsFeature &feature, QgsGeometryCheckerContext *context, bool useMapCrs );
      ~LayerFeature();
      const QgsFeature &feature() const { return mFeature; }
      QPointer<QgsVectorLayer> layer() const;
      QString layerId() const;
      double layerToMapUnits() const;
      const QgsCoordinateTransform &layerToMapTransform() const;
      const QgsAbstractGeometry *geometry() const { return mGeometry; }
      QString geometryCrs() const { return mMapCrs ? layerToMapTransform().destinationCrs().authid() : layerToMapTransform().sourceCrs().authid(); }
      QString id() const;
      bool operator==( const LayerFeature &other ) const;
      bool operator!=( const LayerFeature &other ) const;

      bool useMapCrs() const;

    private:
      const QgsFeaturePool *mFeaturePool;
      QgsFeature mFeature;
      QgsAbstractGeometry *mGeometry = nullptr;
      bool mMapCrs;
  };

  class LayerFeatures
  {
    public:
      LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
                     const QMap<QString, QgsFeatureIds> &featureIds,
                     const QList<QgsWkbTypes::GeometryType> &geometryTypes,
                     QAtomicInt *progressCounter,
                     QgsGeometryCheckerContext *context,
                     bool useMapCrs = false );

      LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
                     const QList<QString> &layerIds, const QgsRectangle &extent,
                     const QList<QgsWkbTypes::GeometryType> &geometryTypes,
                     QgsGeometryCheckerContext *context );

      class iterator
      {
        public:
          iterator( const QList<QString>::const_iterator &layerIt, const LayerFeatures *parent );
          ~iterator();
          const iterator &operator++();
          iterator operator++( int ) { iterator tmp( *this ); ++*this; return tmp; }
          const LayerFeature &operator*() const { Q_ASSERT( mCurrentFeature ); return *mCurrentFeature; }
          bool operator!=( const iterator &other ) { return mLayerIt != other.mLayerIt || mFeatureIt != other.mFeatureIt; }

        private:
          bool nextLayerFeature( bool begin );
          bool nextLayer( bool begin );
          bool nextFeature( bool begin );
          QList<QString>::const_iterator mLayerIt;
          QgsFeatureIds::const_iterator mFeatureIt;
          const LayerFeatures *mParent;
          const LayerFeature *mCurrentFeature = nullptr;
      };

      iterator begin() const { return iterator( mLayerIds.constBegin(), this ); }
      iterator end() const { return iterator( mLayerIds.end(), this ); }
    private:
      QMap<QString, QgsFeaturePool *> mFeaturePools;
      QMap<QString, QgsFeatureIds> mFeatureIds;
      QList<QString> mLayerIds;
      QgsRectangle mExtent;
      QList<QgsWkbTypes::GeometryType> mGeometryTypes;
      QAtomicInt *mProgressCounter = nullptr;
      QgsGeometryCheckerContext *mContext = nullptr;
      bool mUseMapCrs = true;
  };

  std::unique_ptr<QgsGeometryEngine> createGeomEngine( const QgsAbstractGeometry *geometry, double tolerance );

  QgsAbstractGeometry *getGeomPart( QgsAbstractGeometry *geom, int partIdx );
  const QgsAbstractGeometry *getGeomPart( const QgsAbstractGeometry *geom, int partIdx );

  QList<const QgsLineString *> polygonRings( const QgsPolygon *polygon );

  void filter1DTypes( QgsAbstractGeometry *geom );

  /**
   * Returns the number of points in a polyline, accounting for duplicate start and end point if the polyline is closed
   * \param polyLine The polyline
   * \returns The number of distinct points of the polyline
   */
  inline int polyLineSize( const QgsAbstractGeometry *geom, int iPart, int iRing, bool *isClosed = nullptr )
  {
    if ( !geom->isEmpty() )
    {
      int nVerts = geom->vertexCount( iPart, iRing );
      QgsPoint front = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) );
      QgsPoint back = geom->vertexAt( QgsVertexId( iPart, iRing, nVerts - 1 ) );
      bool closed = back == front;
      if ( isClosed )
        *isClosed = closed;
      return closed ? nVerts - 1 : nVerts;
    }
    else
    {
      if ( isClosed )
        *isClosed = true;
      return 0;
    }
  }

  bool pointOnLine( const QgsPoint &p, const QgsLineString *line, double tol, bool excludeExtremities = false );

  QList<QgsPoint> lineIntersections( const QgsLineString *line1, const QgsLineString *line2, double tol );

  double sharedEdgeLength( const QgsAbstractGeometry *geom1, const QgsAbstractGeometry *geom2, double tol );

  /**
     * \brief Determine whether two points are equal up to the specified tolerance
     * \param p1 The first point
     * \param p2 The second point
     * \param tol The tolerance
     * \returns Whether the points are equal
     */
  inline bool pointsFuzzyEqual( const QgsPointXY &p1, const QgsPointXY &p2, double tol )
  {
    double dx = p1.x() - p2.x(), dy = p1.y() - p2.y();
    return ( dx * dx + dy * dy ) < tol * tol;
  }

  inline bool canDeleteVertex( const QgsAbstractGeometry *geom, int iPart, int iRing )
  {
    int nVerts = geom->vertexCount( iPart, iRing );
    QgsPoint front = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) );
    QgsPoint back = geom->vertexAt( QgsVertexId( iPart, iRing, nVerts - 1 ) );
    bool closed = back == front;
    return closed ? nVerts > 4 : nVerts > 2;
  }
} // QgsGeometryCheckerUtils

#endif // QGS_GEOMETRYCHECKERUTILS_H
