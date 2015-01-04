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

typedef struct GEOSGeom_t GEOSGeometry;

class QgsCoordinateTransform;
class QgsCoordinateReferenceSystem;

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
class QgsPointLocator : public QObject
{
    Q_OBJECT
  public:
    explicit QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS = 0 );

    ~QgsPointLocator();

    enum Type { Invalid = 0, Vertex = 1, Edge = 2, Area = 4, All = Vertex | Edge | Area };

    /** Prepare the indexes for given or-ed combination of query types (Vertex, Edge, Area).
     *  If not initialized explicitly, index of particular type will be inited when first such query is issued.
     */
    void init( int types = All, bool force = false );

    //! check whether index for given query type exists
    bool hasIndex( Type t ) const;

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

      void replaceIfBetter( const Match& m, double maxDistance )
      {
        // is other match relevant?
        if ( !m.isValid() || m.mDist > maxDistance )
          return;

        // is other match actually better?
        if ( isValid() && mDist - 10e-6 < m.mDist )
          return;

        // prefer vertex matches to edge matches (even if they are closer)
        if ( type() == Vertex && m.type() == Edge )
          return;

        *this = m; // the other match is better!
      }

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

    typedef struct QList<Match> MatchList;

    //! Interface that allows rejection of some matches in intersection queries
    //! (e.g. a match can only belong to a particular feature / match must not be a particular point).
    //! Implement the interface and pass its instance to QgsPointLocator or QgsSnappingUtils methods.
    struct MatchFilter
    {
      virtual bool acceptMatch( const Match& match ) = 0;
    };

    // 1-NN queries

    //! find nearest vertex to the specified point
    inline Match nearestVertex( const QgsPoint& point )
    {
      MatchList lst = nearestVertices( point, 1 );
      return lst.count() ? lst[0] : Match();
    }

    //! find nearest edge to the specified point
    inline Match nearestEdge( const QgsPoint& point )
    {
      MatchList lst = nearestEdges( point, 1 );
      return lst.count() ? lst[0] : Match();
    }

    // k-NN queries

    //! find nearest vertices to the specified point - sorted by distance
    //! will return up to maxMatches matches
    MatchList nearestVertices( const QgsPoint& point, int maxMatches );
    //! find nearest edges to the specified point - sorted by distance
    MatchList nearestEdges( const QgsPoint& point, int maxMatches );

    // intersection queries

    //! Find nearest vertices to the specified point - sorted by distance.
    //! Will return matches up to distance given by tolerance.
    //! Optional filter may discard unwanted matches.
    MatchList verticesInTolerance( const QgsPoint& point, double tolerance, MatchFilter* filter = 0 );
    //! Find nearest edges to the specified point - sorted by distance.
    //! Will return matches up to distance given by tolerance.
    //! Optional filter may discard unwanted matches.
    MatchList edgesInTolerance( const QgsPoint& point, double tolerance, MatchFilter* filter = 0 );

    //! Find vertices within given rectangle.
    //! If distToPoint is given, the matches will be sorted by distance to that point.
    //! Optional filter may discard unwanted matches.
    MatchList verticesInRect( const QgsRectangle& rect, const QgsPoint* distToPoint = 0, MatchFilter* filter = 0 );
    //! Find edges within given rectangle.
    //! If distToPoint is given, the matches will be sorted by distance to that point.
    //! Optional filter may discard unwanted matches.
    MatchList edgesInRect( const QgsRectangle& rect, const QgsPoint* distToPoint = 0, MatchFilter* filter = 0 );

    // point-in-polygon query

    // TODO: function to return just the first match?
    //! find out if the point is in any polygons
    MatchList pointInPolygon( const QgsPoint& point );


  protected:
    void rebuildIndex( int types );
    void destroyIndex( int types );

  private slots:
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, QgsGeometry& geom );

  private:
    /** storage manager */
    SpatialIndex::IStorageManager* mStorage;

    /** R-tree containing spatial index */
    SpatialIndex::ISpatialIndex* mRTreeVertex;
    SpatialIndex::ISpatialIndex* mRTreeEdge;
    SpatialIndex::ISpatialIndex* mRTreeArea;
    QList<GEOSGeometry*> mAreaGeomList; // owns the geometries - only for area R-tree
    QgsCoordinateTransform* mTransform;
    int mQueryTypes;
    QgsVectorLayer* mLayer;
};


#endif // QGSPOINTLOCATOR_H
