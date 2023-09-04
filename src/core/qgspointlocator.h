/***************************************************************************
  qgspointlocator.h
  --------------------------------------
  Date                 : November 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTLOCATOR_H
#define QGSPOINTLOCATOR_H

class QgsPointXY;
class QgsFeatureRenderer;
class QgsRenderContext;
class QgsRectangle;
class QgsVectorLayerFeatureSource;

#include "qgis_core.h"
#include "qgspointxy.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsfeatureid.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgsvectorlayer.h"
#include "qgslinestring.h"
#include "qgspointlocatorinittask.h"
#include <memory>

#include <QPointer>

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for vertices - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorNearestVertex;

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for centroid - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.12
*/
class QgsPointLocator_VisitorNearestCentroid;

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for middle segment - builds a list of matches.
 * \note not available in Python bindings
 * \since QGIS 3.12
*/
class QgsPointLocator_VisitorNearestMiddleOfSegment;

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for edges - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorNearestEdge;

/**
 * \ingroup core
 * \brief Helper class used when traversing the index with areas - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorArea;

/**
 * \ingroup core
 * \brief Helper class used when traversing the index looking for edges - builds a list of matches.
 * \note not available in Python bindings
*/
class QgsPointLocator_VisitorEdgesInRect;

namespace SpatialIndex SIP_SKIP
{
  class IStorageManager;
  class ISpatialIndex;
}

/**
 * \ingroup core
 * \brief The class defines interface for querying point location:
 *
 * - query nearest vertices / edges to a point
 * - query vertices / edges in rectangle
 * - query areas covering a point
 *
 * Works with one layer.
 *
 * \since QGIS 2.8
 */
class CORE_EXPORT QgsPointLocator : public QObject
{
    Q_OBJECT
  public:

    /**
     * Construct point locator for a \a layer.
     *
     * If a valid QgsCoordinateReferenceSystem is passed for \a destinationCrs then the locator will
     * do the searches on data reprojected to the given CRS. For accurate reprojection it is important
     * to set the correct \a transformContext if a \a destinationCrs is specified. This is usually taken
     * from the current QgsProject::transformContext().
     *
     * If \a extent is not NULLPTR, the locator will index only a subset of the layer which falls within that extent.
     */
    explicit QgsPointLocator( QgsVectorLayer *layer, const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem(),
                              const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext(),
                              const QgsRectangle *extent = nullptr );

    ~QgsPointLocator() override;

    /**
     * Gets associated layer
     * \since QGIS 2.14
     */
    QgsVectorLayer *layer() const { return mLayer; }

    /**
     * Gets destination CRS - may be an invalid QgsCoordinateReferenceSystem if not doing OTF reprojection
     * \since QGIS 2.14
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /**
     * Gets extent of the area point locator covers - if NULLPTR then it caches the whole layer
     * \since QGIS 2.14
     */
    const QgsRectangle *extent() const { return mExtent.get(); }

    /**
     * Configure extent - if not NULLPTR, it will index only that area
     * \since QGIS 2.14
     */
    void setExtent( const QgsRectangle *extent );

    /**
     * Configure render context  - if not NULLPTR, it will use to index only visible feature
     * \since QGIS 3.2
     */
    void setRenderContext( const QgsRenderContext *context );

    /**
     * The type of a snap result or the filter type for a snap request.
     */
    enum Type
    {
      Invalid = 0, //!< Invalid
      Vertex  = 1 << 0, //!< Snapped to a vertex. Can be a vertex of the geometry or an intersection.
      Edge    = 1 << 1, //!< Snapped to an edge
      Area    = 1 << 2, //!< Snapped to an area
      Centroid = 1 << 3, //!< Snapped to a centroid
      MiddleOfSegment = 1 << 4, //!< Snapped to the middle of a segment
      LineEndpoint = 1 << 5, //!< Start or end points of lines only (since QGIS 3.20)
      All = Vertex | Edge | Area | Centroid | MiddleOfSegment //!< Combination of all types. Note LineEndpoint is not included as endpoints made redundant by the presence of the Vertex flag.
    };

    Q_DECLARE_FLAGS( Types, Type )

    /**
     * Prepare the index for queries. Does nothing if the index already exists.
     * If the number of features is greater than the value of maxFeaturesToIndex, creation of index is stopped
     * to make sure we do not run out of memory. If maxFeaturesToIndex is -1, no limits are used.
     *
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * in the constructor. if TRUE, index building will be done in another thread and init() method returns
     * immediately. initFinished() signal will be emitted once the initialization is over.
     *
     * Returns FALSE if the creation of index is blocking and has been prematurely stopped due to the limit of features, otherwise TRUE
     *
     * \see QgsPointLocator()
     */
    bool init( int maxFeaturesToIndex = -1, bool relaxed = false );

    //! Indicate whether the data have been already indexed
    bool hasIndex() const;

    struct Match
    {
        //! construct invalid match
        Match() = default;

        Match( QgsPointLocator::Type t, QgsVectorLayer *vl, QgsFeatureId fid, double dist, const QgsPointXY &pt, int vertexIndex = 0, QgsPointXY *edgePoints = nullptr )
          : mType( t )
          , mDist( dist )
          , mPoint( pt )
          , mLayer( vl )
          , mFid( fid )
          , mVertexIndex( vertexIndex )
        {
          if ( edgePoints )
          {
            mEdgePoints[0] = edgePoints[0];
            mEdgePoints[1] = edgePoints[1];
          }
        }

        QgsPointLocator::Type type() const { return mType; }

        bool isValid() const { return mType != Invalid; }
        //! Returns TRUE if the Match is a vertex
        bool hasVertex() const { return mType == Vertex; }
        //! Returns TRUE if the Match is an edge
        bool hasEdge() const { return mType == Edge; }
        //! Returns TRUE if the Match is a centroid
        bool hasCentroid() const { return mType == Centroid; }
        //! Returns TRUE if the Match is an area
        bool hasArea() const { return mType == Area; }
        //! Returns TRUE if the Match is the middle of a segment
        bool hasMiddleSegment() const { return mType == MiddleOfSegment; }

        /**
         * Returns TRUE if the Match is a line endpoint (start or end vertex).
         *
         * \since QGIS 3.20
         */
        bool hasLineEndpoint() const { return mType == LineEndpoint; }

        /**
         * for vertex / edge match
         * units depending on what class returns it (geom.cache: layer units, map canvas snapper: dest crs units)
         */
        double distance() const { return mDist; }

        /**
         * for vertex / edge match
         * coords depending on what class returns it (geom.cache: layer coords, map canvas snapper: dest coords)
         */
        QgsPointXY point() const { return mPoint; }

        //! for vertex / edge match (first vertex of the edge)
        int vertexIndex() const { return mVertexIndex; }

        /**
         * The vector layer where the snap occurred.
         * Will be NULLPTR if the snap happened on an intersection.
         */
        QgsVectorLayer *layer() const { return mLayer; }

        /**
         * The id of the feature to which the snapped geometry belongs.
         */
        QgsFeatureId featureId() const { return mFid; }

        //! Only for a valid edge match - obtain endpoints of the edge
        void edgePoints( QgsPointXY &pt1 SIP_OUT, QgsPointXY &pt2 SIP_OUT ) const
        {
          pt1 = mEdgePoints[0];
          pt2 = mEdgePoints[1];
        }

        /**
         * Convenient method to return a point on an edge with linear
         * interpolation of the Z value.
         * The parameter \a destinationCrs depends of where the instance of this Match is created (geom.cache: layer CRS, map canvas snapper: dest CRS)
         * \since 3.10
         */
        QgsPoint interpolatedPoint( const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem() ) const
        {
          QgsPoint point;

          if ( mLayer )
          {
            QgsPointXY snappedPoint( mPoint );
            const QgsGeometry geom = mLayer->getGeometry( mFid );
            QgsCoordinateTransform transform( destinationCrs, mLayer->crs(), mLayer->transformContext() );
            if ( transform.isValid() )
            {
              try
              {
                snappedPoint = transform.transform( snappedPoint );
              }
              catch ( QgsCsException & )
              {
                QgsDebugMsg( QStringLiteral( "transformation to layer coordinate failed" ) );
              }
            }

            if ( !( geom.isNull() || geom.isEmpty() ) )
            {
              const QgsLineString line( geom.vertexAt( mVertexIndex ), geom.vertexAt( mVertexIndex + 1 ) );
              point = QgsGeometryUtils::closestPoint( line, QgsPoint( snappedPoint ) );
              if ( QgsWkbTypes::isCurvedType( mLayer->wkbType() ) )
              {
                point.setX( snappedPoint.x() );
                point.setY( snappedPoint.y() );
              }
            }

            if ( transform.isValid() )
            {
              try
              {
                point.transform( transform, Qgis::TransformDirection::Reverse );
              }
              catch ( QgsCsException & )
              {
                QgsDebugMsg( QStringLiteral( "transformation to destination coordinate failed" ) );
              }
            }
          }

          return point;
        }

        // TODO c++20 - replace with = default
        bool operator==( const QgsPointLocator::Match &other ) const
        {
          return mType == other.mType &&
                 mDist == other.mDist &&
                 mPoint == other.mPoint &&
                 mLayer == other.mLayer &&
                 mFid == other.mFid &&
                 mVertexIndex == other.mVertexIndex &&
                 mEdgePoints[0] == other.mEdgePoints[0] &&
                 mEdgePoints[1] == other.mEdgePoints[1] &&
                 mCentroid == other.mCentroid &&
                 mMiddleOfSegment == other.mMiddleOfSegment;
        }

      protected:
        Type mType = Invalid;
        double mDist = 0;
        QgsPointXY mPoint;
        QgsVectorLayer *mLayer = nullptr;
        QgsFeatureId mFid = 0;
        int mVertexIndex = 0; // e.g. vertex index
        QgsPointXY mEdgePoints[2];
        QgsPointXY mCentroid;
        QgsPointXY mMiddleOfSegment;
    };

#ifndef SIP_RUN
    typedef class QList<QgsPointLocator::Match> MatchList;
#else
    typedef QList<QgsPointLocator::Match> MatchList;
#endif

    /**
     * Interface that allows rejection of some matches in intersection queries
     * (e.g. a match can only belong to a particular feature / match must not be a particular point).
     * Implement the interface and pass its instance to QgsPointLocator or QgsSnappingUtils methods.
     */
    struct MatchFilter
    {
      virtual ~MatchFilter() = default;
      virtual bool acceptMatch( const QgsPointLocator::Match &match ) = 0;
    };

    // intersection queries

    /**
     * Find nearest vertex to the specified point - up to distance specified by tolerance
     * Optional filter may discard unwanted matches.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     */
    Match nearestVertex( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find nearest centroid to the specified point - up to distance specified by tolerance
     * Optional filter may discard unwanted matches.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * \since 3.12
     */
    Match nearestCentroid( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find nearest middle of segment to the specified point - up to distance specified by tolerance
     * Optional filter may discard unwanted matches.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * \since 3.12
     */
    Match nearestMiddleOfSegment( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find nearest line endpoint (start or end vertex) to the specified point - up to distance specified by tolerance
     * Optional filter may discard unwanted matches.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * \since 3.20
     */
    Match nearestLineEndpoints( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find nearest edge to the specified point - up to distance specified by tolerance
     * Optional filter may discard unwanted matches.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     */
    Match nearestEdge( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find nearest area to the specified point - up to distance specified by tolerance
     * Optional filter may discard unwanted matches.
     * This will first perform a pointInPolygon and return first result.
     * If no match is found and tolerance is not 0, it will return nearestEdge.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * \since QGIS 3.0
     */
    Match nearestArea( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find edges within a specified rectangle
     * Optional filter may discard unwanted matches.
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     */
    MatchList edgesInRect( const QgsRectangle &rect, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Override of edgesInRect that construct rectangle from a center point and tolerance
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     */
    MatchList edgesInRect( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Find vertices within a specified rectangle
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * Optional filter may discard unwanted matches.
     * \since QGIS 3.6
     */
    MatchList verticesInRect( const QgsRectangle &rect, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    /**
     * Override of verticesInRect that construct rectangle from a center point and tolerance
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     * \since QGIS 3.6
     */
    MatchList verticesInRect( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = nullptr, bool relaxed = false );

    // point-in-polygon query

    // TODO: function to return just the first match?

    /**
     * find out if the \a point is in any polygons
     * This method is either blocking or non blocking according to \a relaxed parameter passed
     */
    //!
    MatchList pointInPolygon( const QgsPointXY &point, bool relaxed = false );

    /**
     * Returns how many geometries are cached in the index
     * \since QGIS 2.14
     */
    int cachedGeometryCount() const { return mGeoms.count(); }

    /**
     * Returns TRUE if the point locator is currently indexing the data.
     * This method is useful if constructor parameter \a relaxed is TRUE
     *
     * \see QgsPointLocator()
     */
    bool isIndexing() const { return mIsIndexing; }

    /**
     * If the point locator has been initialized relaxedly and is currently indexing,
     * this methods waits for the indexing to be finished
     */
    void waitForIndexingFinished();

  signals:

    /**
     * Emitted whenever index has been built and initialization is finished
     * \param ok FALSE if the creation of index has been prematurely stopped due to the limit of
     * features, otherwise TRUE
     */
    void initFinished( bool ok );

  protected:
    bool rebuildIndex( int maxFeaturesToIndex = -1 );

  protected slots:
    void destroyIndex();
  private slots:
    void onInitTaskFinished();
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom );
    void onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );

  private:

    /**
     * prepare index if need and returns TRUE if the index is ready to be used
     * \param relaxed TRUE if index build has to be non blocking
     */
    bool prepare( bool relaxed );

    //! Storage manager
    std::unique_ptr< SpatialIndex::IStorageManager > mStorage;

    QHash<QgsFeatureId, QgsGeometry *> mGeoms;
    std::unique_ptr< SpatialIndex::ISpatialIndex > mRTree;

    //! flag whether the layer is currently empty (i.e. mRTree is NULLPTR but it is not necessary to rebuild it)
    bool mIsEmptyLayer = false;


    //! R-tree containing spatial index
    QgsCoordinateTransform mTransform;
    QgsVectorLayer *mLayer = nullptr;
    std::unique_ptr< QgsRectangle > mExtent;

    std::unique_ptr<QgsRenderContext> mContext;
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    int mMaxFeaturesToIndex = -1;
    bool mIsIndexing = false;
    bool mIsDestroying = false;
    QgsFeatureIds mAddedFeatures;
    QgsFeatureIds mDeletedFeatures;
    QPointer<QgsPointLocatorInitTask> mInitTask;

    friend class QgsPointLocator_VisitorNearestVertex;
    friend class QgsPointLocator_VisitorNearestCentroid;
    friend class QgsPointLocator_VisitorNearestMiddleOfSegment;
    friend class QgsPointLocator_VisitorNearestEdge;
    friend class QgsPointLocator_VisitorArea;
    friend class QgsPointLocator_VisitorEdgesInRect;
    friend class QgsPointLocator_VisitorVerticesInRect;
    friend class QgsPointLocatorInitTask;
    friend class TestQgsPointLocator;
    friend class QgsPointLocator_VisitorCentroidsInRect;
    friend class QgsPointLocator_VisitorMiddlesInRect;
    friend class QgsPointLocator_VisitorNearestLineEndpoint;
};


#endif // QGSPOINTLOCATOR_H
