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

#ifndef QGS_GEOMETRYCHECKERUTILS_H
#define QGS_GEOMETRYCHECKERUTILS_H

#include "qgis_analysis.h"
#include "qgsfeature.h"
#include "geometry/qgsabstractgeometry.h"
#include "geometry/qgspoint.h"
#include "qgsgeometrycheckcontext.h"
#include <qmath.h>

class QgsGeometryEngine;
class QgsFeaturePool;
class QgsFeedback;

/**
 * \ingroup analysis
 *
 * \brief Contains utilities required for geometry checks.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryCheckerUtils
{
  public:

    /**
     * \ingroup analysis
     *
     * \brief A layer feature combination to uniquely identify and access a feature in
     * a set of layers.
     *
     * \since QGIS 3.4
     */
    class ANALYSIS_EXPORT LayerFeature
    {
      public:

        /**
         * Create a new layer/feature combination.
         * The layer is defined by \a pool, \a feature needs to be from this layer.
         * If \a useMapCrs is TRUE, geometries will be reprojected to the mapCrs defined
         * in \a context.
         */
        LayerFeature( const QgsFeaturePool *pool, const QgsFeature &feature, const QgsGeometryCheckContext *context, bool useMapCrs );

        /**
         * Returns the feature.
         * The geometry will not be reprojected regardless of useMapCrs.
         */
        QgsFeature feature() const;

        /**
         * The layer.
         */
        QPointer<QgsVectorLayer> layer() const SIP_SKIP;

        /**
         * The layer id.
         */
        QString layerId() const;

        /**
         * Returns the geometry of this feature.
         * If useMapCrs was specified, it will already be reprojected into the
         * CRS specified in the context specified in the constructor.
         */
        QgsGeometry geometry() const;

        /**
         * Returns a combination of the layerId and the feature id.
         */
        QString id() const;
        bool operator==( const QgsGeometryCheckerUtils::LayerFeature &other ) const;
        bool operator!=( const QgsGeometryCheckerUtils::LayerFeature &other ) const;

        /**
         * Returns if the geometry is reprojected to the map CRS or not.
         */
        bool useMapCrs() const;

      private:
        const QgsFeaturePool *mFeaturePool;
        QgsFeature mFeature;
        QgsGeometry mGeometry;
        bool mMapCrs;
    };

    /**
     * \ingroup analysis
     *
     * \brief Contains a set of layers and feature ids in those layers to pass to a geometry check.
     *
     * \since QGIS 3.4
     */
    class ANALYSIS_EXPORT LayerFeatures
    {
      public:
#ifndef SIP_RUN

        /**
         * Creates a new set of layer and features.
         */
        LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
                       const QMap<QString, QgsFeatureIds> &featureIds,
                       const QList<QgsWkbTypes::GeometryType> &geometryTypes,
                       QgsFeedback *feedback,
                       const QgsGeometryCheckContext *context,
                       bool useMapCrs = false );

        /**
         * Creates a new set of layer and features.
         */
        LayerFeatures( const QMap<QString, QgsFeaturePool *> &featurePools,
                       const QList<QString> &layerIds, const QgsRectangle &extent,
                       const QList<QgsWkbTypes::GeometryType> &geometryTypes,
                       const QgsGeometryCheckContext *context );

        /**
         * \ingroup analysis
         *
         * \brief An iterator over all features in a QgsGeometryCheckerUtils::LayerFeatures.
         *
         * \since QGIS 3.4
         */
        class iterator
        {
          public:

            /**
             * Creates a new iterator.
             */
            iterator( const QStringList::const_iterator &layerIt, const LayerFeatures *parent );

            /**
             * Copies the iterator \a rh.
             */
            iterator( const iterator &rh );
            ~iterator();

            /**
             * Increments the item the iterator currently points to by one and
             * returns the new iterator.
             */
            const iterator &operator++();

            /**
             * Increments the item the iterator currently points to by \a n and
             * returns the new iterator.
             */
            iterator operator++( int n );

            /**
             * Dereferences the item at the current iterator location.
             */
            const QgsGeometryCheckerUtils::LayerFeature &operator*() const;
            bool operator!=( const iterator &other ) const;

          private:
            bool nextLayerFeature( bool begin );
            bool nextLayer( bool begin );
            bool nextFeature( bool begin );
            QList<QString>::const_iterator mLayerIt;
            QgsFeatureIds::const_iterator mFeatureIt;
            const LayerFeatures *mParent = nullptr;
            std::unique_ptr<QgsGeometryCheckerUtils::LayerFeature> mCurrentFeature;

            iterator &operator= ( const iterator & ) = delete;
        };

        /**
         * The first feature to start iterating.
         */
        iterator begin() const;

        /**
         * One after the last feature to stop iterating.
         */
        iterator end() const;

#endif

      private:
#ifdef SIP_RUN
        LayerFeatures();
#endif
        QMap<QString, QgsFeaturePool *> mFeaturePools;
        QMap<QString, QgsFeatureIds> mFeatureIds;
        QList<QString> mLayerIds;
        QgsRectangle mExtent;
        QList<QgsWkbTypes::GeometryType> mGeometryTypes;
        QgsFeedback *mFeedback = nullptr;
        const QgsGeometryCheckContext *mContext = nullptr;
        bool mUseMapCrs = true;
    };

#ifndef SIP_RUN

    static std::unique_ptr<QgsGeometryEngine> createGeomEngine( const QgsAbstractGeometry *geometry, double tolerance );

    static QgsAbstractGeometry *getGeomPart( QgsAbstractGeometry *geom, int partIdx );
    static const QgsAbstractGeometry *getGeomPart( const QgsAbstractGeometry *geom, int partIdx );

    static QList <const QgsLineString *> polygonRings( const QgsPolygon *polygon );

    static void filter1DTypes( QgsAbstractGeometry *geom );

    /**
     * Returns the number of points in a polyline, accounting for duplicate start and end point if the polyline is closed
     * \returns The number of distinct points of the polyline
     */
    static inline int polyLineSize( const QgsAbstractGeometry *geom, int iPart, int iRing, bool *isClosed = nullptr )
    {
      if ( !geom->isEmpty() )
      {
        const int nVerts = geom->vertexCount( iPart, iRing );
        const QgsPoint front = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) );
        const QgsPoint back = geom->vertexAt( QgsVertexId( iPart, iRing, nVerts - 1 ) );
        const bool closed = back == front;
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

    static bool pointOnLine( const QgsPoint &p, const QgsLineString *line, double tol, bool excludeExtremities = false );

    static QList<QgsPoint> lineIntersections( const QgsLineString *line1, const QgsLineString *line2, double tol );

    static double sharedEdgeLength( const QgsAbstractGeometry *geom1, const QgsAbstractGeometry *geom2, double tol );

    /**
       * \brief Determine whether two points are equal up to the specified tolerance
       * \param p1 The first point
       * \param p2 The second point
       * \param tol The tolerance
       * \returns Whether the points are equal
       */
    static inline bool pointsFuzzyEqual( const QgsPointXY &p1, const QgsPointXY &p2, double tol )
    {
      double dx = p1.x() - p2.x(), dy = p1.y() - p2.y();
      return ( dx * dx + dy * dy ) < tol * tol;
    }

    static inline bool canDeleteVertex( const QgsAbstractGeometry *geom, int iPart, int iRing )
    {
      const int nVerts = geom->vertexCount( iPart, iRing );
      const QgsPoint front = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) );
      const QgsPoint back = geom->vertexAt( QgsVertexId( iPart, iRing, nVerts - 1 ) );
      const bool closed = back == front;
      return closed ? nVerts > 4 : nVerts > 2;
    }

#endif

}; // QgsGeometryCheckerUtils

#endif // QGS_GEOMETRYCHECKERUTILS_H
