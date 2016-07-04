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

class QgsPoint;
class QgsVectorLayer;

#include "qgsfeature.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

class QgsCoordinateTransform;
class QgsCoordinateReferenceSystem;

class QgsPointLocator_VisitorNearestVertex;
class QgsPointLocator_VisitorNearestEdge;
class QgsPointLocator_VisitorArea;
class QgsPointLocator_VisitorEdgesInRect;

namespace SpatialIndex
{
  class IStorageManager;
  class ISpatialIndex;
}

/** \ingroup core
 * @brief The class defines interface for querying point location:
 *  - query nearest vertices / edges to a point
 *  - query vertices / edges in rectangle
 *  - query areas covering a point
 *
 * Works with one layer.
 *
 * @note added in 2.8
 */
class CORE_EXPORT QgsPointLocator : public QObject
{
    Q_OBJECT
  public:
    /** Construct point locator for a layer.
     *  @arg destCRS if not null, will do the searches on data reprojected to the given CRS
     *  @arg extent  if not null, will index only a subset of the layer
     */
    explicit QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS = nullptr, const QgsRectangle* extent = nullptr );

    ~QgsPointLocator();

    //! Get associated layer
    //! @note added in QGIS 2.14
    QgsVectorLayer* layer() const { return mLayer; }
    //! Get destination CRS - may be null if not doing OTF reprojection
    //! @note added in QGIS 2.14
    const QgsCoordinateReferenceSystem* destCRS() const;
    //! Get extent of the area point locator covers - if null then it caches the whole layer
    //! @note added in QGIS 2.14
    const QgsRectangle* extent() const { return mExtent; }
    //! Configure extent - if not null, it will index only that area
    //! @note added in QGIS 2.14
    void setExtent( const QgsRectangle* extent );

    /**
     * The type of a snap result or the filter type for a snap request.
     */
    enum Type
    {
      Invalid = 0, //!< Invalid
      Vertex  = 1, //!< Snapped to a vertex. Can be a vertex of the geometry or an intersection.
      Edge    = 2, //!< Snapped to an edge
      Area    = 4, //!< Snapped to an area
      All = Vertex | Edge | Area //!< Combination of vertex, edge and area
    };

    Q_DECLARE_FLAGS( Types, Type )

    /** Prepare the index for queries. Does nothing if the index already exists.
     * If the number of features is greater than the value of maxFeaturesToIndex, creation of index is stopped
     * to make sure we do not run out of memory. If maxFeaturesToIndex is -1, no limits are used. Returns
     * false if the creation of index has been prematurely stopped due to the limit of features, otherwise true */
    bool init( int maxFeaturesToIndex = -1 );

    /** Indicate whether the data have been already indexed */
    bool hasIndex() const;

    struct Match
    {
      //! construct invalid match
      Match()
          : mType( Invalid )
          , mDist( 0 )
          , mPoint()
          , mLayer( nullptr )
          , mFid( 0 )
          , mVertexIndex( 0 )
      {}

      Match( Type t, QgsVectorLayer* vl, QgsFeatureId fid, double dist, const QgsPoint& pt, int vertexIndex = 0, QgsPoint* edgePoints = nullptr )
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

      Type type() const { return mType; }

      bool isValid() const { return mType != Invalid; }
      bool hasVertex() const { return mType == Vertex; }
      bool hasEdge() const { return mType == Edge; }
      bool hasArea() const { return mType == Area; }

      //! for vertex / edge match
      //! units depending on what class returns it (geom.cache: layer units, map canvas snapper: dest crs units)
      double distance() const { return mDist; }

      //! for vertex / edge match
      //! coords depending on what class returns it (geom.cache: layer coords, map canvas snapper: dest coords)
      QgsPoint point() const { return mPoint; }

      //! for vertex / edge match (first vertex of the edge)
      int vertexIndex() const { return mVertexIndex; }

      /**
       * The vector layer where the snap occurred.
       * Will be null if the snap happened on an intersection.
       */
      QgsVectorLayer* layer() const { return mLayer; }

      /**
       * The id of the feature to which the snapped geometry belongs.
       */
      QgsFeatureId featureId() const { return mFid; }

      //! Only for a valid edge match - obtain endpoints of the edge
      void edgePoints( QgsPoint& pt1, QgsPoint& pt2 ) const
      {
        pt1 = mEdgePoints[0];
        pt2 = mEdgePoints[1];
      }

    protected:
      Type mType;
      double mDist;
      QgsPoint mPoint;
      QgsVectorLayer* mLayer;
      QgsFeatureId mFid;
      int mVertexIndex; // e.g. vertex index
      QgsPoint mEdgePoints[2];
    };

    typedef class QList<Match> MatchList;

    //! Interface that allows rejection of some matches in intersection queries
    //! (e.g. a match can only belong to a particular feature / match must not be a particular point).
    //! Implement the interface and pass its instance to QgsPointLocator or QgsSnappingUtils methods.
    struct MatchFilter
    {
      virtual ~MatchFilter() {}
      virtual bool acceptMatch( const Match& match ) = 0;
    };

    // intersection queries

    //! Find nearest vertex to the specified point - up to distance specified by tolerance
    //! Optional filter may discard unwanted matches.
    Match nearestVertex( const QgsPoint& point, double tolerance, MatchFilter* filter = nullptr );
    //! Find nearest edges to the specified point - up to distance specified by tolerance
    //! Optional filter may discard unwanted matches.
    Match nearestEdge( const QgsPoint& point, double tolerance, MatchFilter* filter = nullptr );
    //! Find edges within a specified recangle
    //! Optional filter may discard unwanted matches.
    MatchList edgesInRect( const QgsRectangle& rect, MatchFilter* filter = nullptr );
    //! Override of edgesInRect that construct rectangle from a center point and tolerance
    MatchList edgesInRect( const QgsPoint& point, double tolerance, MatchFilter* filter = nullptr );

    // point-in-polygon query

    // TODO: function to return just the first match?
    //! find out if the point is in any polygons
    MatchList pointInPolygon( const QgsPoint& point );

    //

    //! Return how many geometries are cached in the index
    //! @note added in QGIS 2.14
    int cachedGeometryCount() const { return mGeoms.count(); }

  protected:
    bool rebuildIndex( int maxFeaturesToIndex = -1 );
    void destroyIndex();

  private slots:
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, QgsGeometry& geom );

  private:
    /** Storage manager */
    SpatialIndex::IStorageManager* mStorage;

    QHash<QgsFeatureId, QgsGeometry*> mGeoms;
    SpatialIndex::ISpatialIndex* mRTree;

    //! flag whether the layer is currently empty (i.e. mRTree is null but it is not necessary to rebuild it)
    bool mIsEmptyLayer;

    /** R-tree containing spatial index */
    QgsCoordinateTransform* mTransform;
    QgsVectorLayer* mLayer;
    QgsRectangle* mExtent;

    friend class QgsPointLocator_VisitorNearestVertex;
    friend class QgsPointLocator_VisitorNearestEdge;
    friend class QgsPointLocator_VisitorArea;
    friend class QgsPointLocator_VisitorEdgesInRect;
};


#endif // QGSPOINTLOCATOR_H
