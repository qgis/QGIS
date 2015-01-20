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


#include <spatialindex/SpatialIndex.h>


class QgsCoordinateTransform;
class QgsCoordinateReferenceSystem;

class QgsPointLocator_VisitorNearestVertex;
class QgsPointLocator_VisitorNearestEdge;
class QgsPointLocator_VisitorArea;
class QgsPointLocator_VisitorEdgesInRect;

/**
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
    explicit QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS = 0, const QgsRectangle* extent = 0 );

    ~QgsPointLocator();

    enum Type { Invalid = 0, Vertex = 1, Edge = 2, Area = 4, All = Vertex | Edge | Area };

    /** Prepare the index for queries. Does nothing if the index already exists */
    void init();

    struct Match
    {
      //! consruct invalid match
      Match() : mType( Invalid ), mDist( 0 ), mPoint(), mLayer( 0 ), mFid( 0 ), mVertexIndex( 0 ) {}

      Match( Type t, QgsVectorLayer* vl, QgsFeatureId fid, double dist, const QgsPoint& pt, int vertexIndex = 0, QgsPoint* edgePoints = 0 )
          : mType( t ), mDist( dist ), mPoint( pt ), mLayer( vl ), mFid( fid ), mVertexIndex( vertexIndex )
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

      //! reference vector layer
      QgsVectorLayer* layer() const { return mLayer; }

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
      virtual bool acceptMatch( const Match& match ) = 0;
    };

    // intersection queries

    //! Find nearest vertex to the specified point - up to distance specified by tolerance
    //! Optional filter may discard unwanted matches.
    Match nearestVertex( const QgsPoint& point, double tolerance, MatchFilter* filter = 0 );
    //! Find nearest edges to the specified point - up to distance specified by tolerance
    //! Optional filter may discard unwanted matches.
    Match nearestEdge( const QgsPoint& point, double tolerance, MatchFilter* filter = 0 );
    //! Find edges within a specified recangle
    //! Optional filter may discard unwanted matches.
    MatchList edgesInRect( const QgsRectangle& rect, MatchFilter* filter = 0 );
    //! Override of edgesInRect that construct rectangle from a center point and tolerance
    MatchList edgesInRect( const QgsPoint& point, double tolerance, MatchFilter* filter = 0 );

    // point-in-polygon query

    // TODO: function to return just the first match?
    //! find out if the point is in any polygons
    MatchList pointInPolygon( const QgsPoint& point );


  protected:
    void rebuildIndex();
    void destroyIndex();

  private slots:
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, QgsGeometry& geom );

  private:
    /** storage manager */
    SpatialIndex::IStorageManager* mStorage;

    QHash<QgsFeatureId, QgsGeometry*> mGeoms;
    SpatialIndex::ISpatialIndex* mRTree;

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
