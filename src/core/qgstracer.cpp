/***************************************************************************
  qgstracer.cpp
  --------------------------------------
  Date                 : January 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstracer.h"

#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <queue>
#include <vector>

typedef std::pair<int, double> DijkstraQueueItem; // first = vertex index, second = distance

// utility comparator for queue items based on distance
struct comp
{
  bool operator()( DijkstraQueueItem a, DijkstraQueueItem b )
  {
    return a.second > b.second;
  }
};


// TODO: move to geometry utils
double distance2D( const QgsPolyline& coords )
{
  int np = coords.count();
  if ( np == 0 )
    return 0;

  double x0 = coords[0].x(), y0 = coords[0].y();
  double x1, y1;
  double dist = 0;
  for ( int i = 1; i < np; ++i )
  {
    x1 = coords[i].x();
    y1 = coords[i].y();
    dist += sqrt(( x1 - x0 ) * ( x1 - x0 ) + ( y1 - y0 ) * ( y1 - y0 ) );
    x0 = x1;
    y0 = y1;
  }
  return dist;
}


// TODO: move to geometry utils
double closestSegment( const QgsPolyline& pl, const QgsPoint& pt, int& vertexAfter, double epsilon )
{
  double sqrDist = std::numeric_limits<double>::max();
  const QgsPoint* pldata = pl.constData();
  int plcount = pl.count();
  double prevX = pldata[0].x(), prevY = pldata[0].y();
  double segmentPtX, segmentPtY;
  for ( int i = 1; i < plcount; ++i )
  {
    double currentX = pldata[i].x();
    double currentY = pldata[i].y();
    double testDist = QgsGeometryUtils::sqrDistToLine( pt.x(), pt.y(), prevX, prevY, currentX, currentY, segmentPtX, segmentPtY, epsilon );
    if ( testDist < sqrDist )
    {
      sqrDist = testDist;
      vertexAfter = i;
    }
    prevX = currentX;
    prevY = currentY;
  }
  return sqrDist;
}

/////

/** Simple graph structure for shortest path search */
struct QgsTracerGraph
{
  QgsTracerGraph() : joinedVertices( 0 ) {}

  struct E  // bidirectional edge
  {
    //! vertices that the edge connects
    int v1, v2;
    //! coordinates of the edge (including endpoints)
    QVector<QgsPoint> coords;

    int otherVertex( int v0 ) const { return v1 == v0 ? v2 : v1; }
    double weight() const { return distance2D( coords ); }
  };

  struct V
  {
    //! location of the vertex
    QgsPoint pt;
    //! indices of adjacent edges (used in Dijkstra algorithm)
    QVector<int> edges;
  };

  //! Vertices of the graph
  QVector<V> v;
  //! Edges of the graph
  QVector<E> e;

  //! Temporarily removed edges
  QSet<int> inactiveEdges;
  //! Temporarily added vertices (for each there are two extra edges)
  int joinedVertices;
};


QgsTracerGraph* makeGraph( const QVector<QgsPolyline>& edges )
{
  QgsTracerGraph *g = new QgsTracerGraph();
  g->joinedVertices = 0;
  QHash<QgsPoint, int> point2vertex;

  Q_FOREACH ( const QgsPolyline& line, edges )
  {
    QgsPoint p1( line[0] );
    QgsPoint p2( line[line.count() - 1] );

    int v1 = -1, v2 = -1;
    // get or add vertex 1
    if ( point2vertex.contains( p1 ) )
      v1 = point2vertex.value( p1 );
    else
    {
      v1 = g->v.count();
      QgsTracerGraph::V v;
      v.pt = p1;
      g->v.append( v );
      point2vertex[p1] = v1;
    }

    // get or add vertex 2
    if ( point2vertex.contains( p2 ) )
      v2 = point2vertex.value( p2 );
    else
    {
      v2 = g->v.count();
      QgsTracerGraph::V v;
      v.pt = p2;
      g->v.append( v );
      point2vertex[p2] = v2;
    }

    // add edge
    QgsTracerGraph::E e;
    e.v1 = v1;
    e.v2 = v2;
    e.coords = line;
    g->e.append( e );

    // link edge to vertices
    int eIdx = g->e.count() - 1;
    g->v[v1].edges << eIdx;
    g->v[v2].edges << eIdx;
  }

  return g;
}


QVector<QgsPoint> shortestPath( const QgsTracerGraph& g, int v1, int v2 )
{
  if ( v1 == -1 || v2 == -1 )
    return QVector<QgsPoint>(); // invalid input

  // priority queue to drive Dijkstra:
  // first of the pair is vertex index, second is distance
  std::priority_queue< DijkstraQueueItem, std::vector< DijkstraQueueItem >, comp > Q;

  // shortest distances to each vertex
  QVector<double> D( g.v.count(), std::numeric_limits<double>::max() );
  D[v1] = 0;

  // whether vertices have been already processed
  QVector<bool> F( g.v.count() );

  // using which edge there is shortest path to each vertex
  QVector<int> S( g.v.count(), -1 );

  int u = -1;
  Q.push( DijkstraQueueItem( v1, 0 ) );

  while ( !Q.empty() )
  {
    u = Q.top().first; // new vertex to visit
    Q.pop();

    if ( u == v2 )
      break; // we can stop now, there won't be a shorter path

    if ( F[u] )
      continue;  // ignore previously added path which is actually longer

    const QgsTracerGraph::V& vu = g.v[u];
    const int* vuEdges = vu.edges.constData();
    int count = vu.edges.count();
    for ( int i = 0; i < count; ++i )
    {
      const QgsTracerGraph::E& edge = g.e[ vuEdges[i] ];
      int v = edge.otherVertex( u );
      double w = edge.weight();
      if ( !F[v] && D[u] + w < D[v] )
      {
        // found a shorter way to the vertex
        D[v] = D[u] + w;
        S[v] = vuEdges[i];
        Q.push( DijkstraQueueItem( v, D[v] ) );
      }
    }
    F[u] = 1; // mark the vertex as processed (we know the fastest path to it)
  }

  if ( u != v2 ) // there's no path to the end vertex
    return QVector<QgsPoint>();

  //qDebug("dist %f", D[u]);

  QVector<QgsPoint> points;
  QList<int> path;
  while ( S[u] != -1 )
  {
    path << S[u];
    const QgsTracerGraph::E& e = g.e[S[u]];
    QVector<QgsPoint> edgePoints = e.coords;
    if ( edgePoints[0] != g.v[u].pt )
      std::reverse( edgePoints.begin(), edgePoints.end() );
    if ( !points.isEmpty() )
      points.remove( points.count() - 1 );  // chop last one (will be used from next edge)
    points << edgePoints;
    u = e.otherVertex( u );
  }

  std::reverse( path.begin(), path.end() );
  //Q_FOREACH (int x, path)
  //  qDebug("e: %d", x);

  std::reverse( points.begin(), points.end() );
  return points;
}


int point2vertex( const QgsTracerGraph& g, const QgsPoint& pt, double epsilon = 1e-6 )
{
  // TODO: use spatial index

  for ( int i = 0; i < g.v.count(); ++i )
  {
    const QgsTracerGraph::V& v = g.v.at( i );
    if ( v.pt == pt || ( fabs( v.pt.x() - pt.x() ) < epsilon && fabs( v.pt.y() - pt.y() ) < epsilon ) )
      return i;
  }

  return -1;
}


int point2edge( const QgsTracerGraph& g, const QgsPoint& pt, int& lineVertexAfter, double epsilon = 1e-6 )
{
  int vertexAfter;

  for ( int i = 0; i < g.e.count(); ++i )
  {
    if ( g.inactiveEdges.contains( i ) )
      continue;  // ignore temporarily disabled edges

    const QgsTracerGraph::E& e = g.e.at( i );
    double dist = closestSegment( e.coords, pt, vertexAfter, epsilon );
    if ( dist == 0 )
    {
      lineVertexAfter = vertexAfter;
      return i;
    }
  }
  return -1;
}


void splitLinestring( const QgsPolyline& points, const QgsPoint& pt, int lineVertexAfter, QgsPolyline& pts1, QgsPolyline& pts2 )
{
  int count1 = lineVertexAfter;
  int count2 = points.count() - lineVertexAfter;

  for ( int i = 0; i < count1; ++i )
    pts1 << points[i];
  if ( points[lineVertexAfter-1] != pt )
    pts1 << pt;  // repeat if not split exactly at that point

  if ( pt != points[lineVertexAfter] )
    pts2 << pt;  // repeat if not split exactly at that point
  for ( int i = 0; i < count2; ++i )
    pts2 << points[i + lineVertexAfter];
}


int joinVertexToGraph( QgsTracerGraph& g, const QgsPoint& pt )
{
  // find edge where the point is
  int lineVertexAfter;
  int eIdx = point2edge( g, pt, lineVertexAfter );

  //qDebug("e: %d", eIdx);

  if ( eIdx == -1 )
    return -1;

  const QgsTracerGraph::E& e = g.e[eIdx];
  QgsTracerGraph::V& v1 = g.v[e.v1];
  QgsTracerGraph::V& v2 = g.v[e.v2];

  QgsPolyline out1, out2;
  splitLinestring( e.coords, pt, lineVertexAfter, out1, out2 );

  int vIdx = g.v.count();
  int e1Idx = g.e.count();
  int e2Idx = e1Idx + 1;

  // prepare new vertex and edges

  QgsTracerGraph::V v;
  v.pt = pt;
  v.edges << e1Idx << e2Idx;

  QgsTracerGraph::E e1;
  e1.v1 = e.v1;
  e1.v2 = vIdx;
  e1.coords = out1;

  QgsTracerGraph::E e2;
  e2.v1 = vIdx;
  e2.v2 = e.v2;
  e2.coords = out2;

  // update edge connectivity of existing vertices
  v1.edges.replace( v1.edges.indexOf( eIdx ), e1Idx );
  v2.edges.replace( v2.edges.indexOf( eIdx ), e2Idx );
  g.inactiveEdges << eIdx;

  // add new vertex and edges to the graph
  g.v.append( v );
  g.e.append( e1 );
  g.e.append( e2 );
  g.joinedVertices++;

  return vIdx;
}


int pointInGraph( QgsTracerGraph& g, const QgsPoint& pt )
{
  // try to use existing vertex in the graph
  int v = point2vertex( g, pt );
  if ( v != -1 )
    return v;

  // try to add the vertex to an edge (may fail if point is not on edge)
  return joinVertexToGraph( g, pt );
}


void resetGraph( QgsTracerGraph& g )
{
  // remove extra vertices and edges
  g.v.resize( g.v.count() - g.joinedVertices );
  g.e.resize( g.e.count() - g.joinedVertices * 2 );
  g.joinedVertices = 0;

  // fix vertices of deactivated edges
  Q_FOREACH ( int eIdx, g.inactiveEdges )
  {
    if ( eIdx >= g.e.count() )
      continue;
    const QgsTracerGraph::E& e = g.e[eIdx];
    QgsTracerGraph::V& v1 = g.v[e.v1];
    for ( int i = 0; i < v1.edges.count(); ++i )
    {
      if ( v1.edges[i] >= g.e.count() )
        v1.edges.remove( i-- );
    }
    v1.edges << eIdx;

    QgsTracerGraph::V& v2 = g.v[e.v2];
    for ( int i = 0; i < v2.edges.count(); ++i )
    {
      if ( v2.edges[i] >= g.e.count() )
        v2.edges.remove( i-- );
    }
    v2.edges << eIdx;
  }

  g.inactiveEdges.clear();
}


void extractLinework( const QgsGeometry* g, QgsMultiPolyline& mpl )
{
  // segmentize curved geometries - we will use noding algorithm from GEOS
  // to find all intersections a bit later (so we need them segmentized anyway)
  QScopedPointer<QgsGeometry> segmentizedGeom;
  if ( QgsWKBTypes::isCurvedType( g->geometry()->wkbType() ) )
  {
    QgsAbstractGeometryV2* segmentizedGeomV2 = g->geometry()->segmentize();
    if ( !segmentizedGeomV2 )
      return;

    // temporarily replace the original geometry by our segmentized one
    segmentizedGeom.reset( new QgsGeometry( segmentizedGeomV2 ) );
    g = segmentizedGeom.data();
  }

  switch ( QgsWKBTypes::flatType( g->geometry()->wkbType() ) )
  {
    case QgsWKBTypes::LineString:
      mpl << g->asPolyline();
      break;

    case QgsWKBTypes::Polygon:
      Q_FOREACH ( const QgsPolyline& ring, g->asPolygon() )
        mpl << ring;
      break;

    case QgsWKBTypes::MultiLineString:
      Q_FOREACH ( const QgsPolyline& linestring, g->asMultiPolyline() )
        mpl << linestring;
      break;

    case QgsWKBTypes::MultiPolygon:
      Q_FOREACH ( const QgsPolygon& polygon, g->asMultiPolygon() )
        Q_FOREACH ( const QgsPolyline& ring, polygon )
          mpl << ring;
      break;

    default:
      break;  // unknown type - do nothing
  }
}

// -------------


QgsTracer::QgsTracer()
    : mGraph( 0 )
    , mReprojectionEnabled( false )
    , mMaxFeatureCount( 0 )
    , mHasTopologyProblem( false )
{
}


bool QgsTracer::initGraph()
{
  if ( mGraph )
    return true; // already initialized

  mHasTopologyProblem = false;

  QgsFeature f;
  QgsMultiPolyline mpl;

  // extract linestrings

  // TODO: use QgsPointLocator as a source for the linework

  QTime t1, t2, t2a, t3;

  t1.start();
  int featuresCounted = 0;
  Q_FOREACH ( QgsVectorLayer* vl, mLayers )
  {
    QgsCoordinateTransform ct( vl->crs(), mCRS );

    QgsFeatureRequest request;
    request.setSubsetOfAttributes( QgsAttributeList() );
    if ( !mExtent.isEmpty() )
      request.setFilterRect( mReprojectionEnabled ? ct.transformBoundingBox( mExtent, QgsCoordinateTransform::ReverseTransform ) : mExtent );

    QgsFeatureIterator fi = vl->getFeatures( request );
    while ( fi.nextFeature( f ) )
    {
      if ( !f.constGeometry() )
        continue;

      if ( mReprojectionEnabled && !ct.isShortCircuited() )
      {
        try
        {
          f.geometry()->transform( ct );
        }
        catch ( QgsCsException& )
        {
          continue; // ignore if the transform failed
        }
      }

      extractLinework( f.constGeometry(), mpl );

      ++featuresCounted;
      if ( mMaxFeatureCount != 0 && featuresCounted >= mMaxFeatureCount )
        return false;
    }
  }
  int timeExtract = t1.elapsed();

  // resolve intersections

  t2.start();

  int timeNodingCall = 0;

#if 0
  // without noding - if data are known to be noded beforehand
#else
  QgsGeometry* allGeom = QgsGeometry::fromMultiPolyline( mpl );

  try
  {
    t2a.start();
    // GEOSNode_r may throw an exception
    GEOSGeometry* allNoded = GEOSNode_r( QgsGeometry::getGEOSHandler(), allGeom->asGeos() );
    timeNodingCall = t2a.elapsed();

    QgsGeometry* noded = new QgsGeometry;
    noded->fromGeos( allNoded );
    delete allGeom;

    mpl = noded->asMultiPolyline();

    delete noded;
  }
  catch ( GEOSException &e )
  {
    // no big deal... we will just not have nicely noded linework, potentially
    // missing some intersections

    mHasTopologyProblem = true;

    QgsDebugMsg( "Tracer Noding Exception: " + e.what() );
  }
#endif

  int timeNoding = t2.elapsed();

  t3.start();

  mGraph = makeGraph( mpl );

  int timeMake = t3.elapsed();

  Q_UNUSED( timeExtract );
  Q_UNUSED( timeNoding );
  Q_UNUSED( timeNodingCall );
  Q_UNUSED( timeMake );
  QgsDebugMsg( QString( "tracer extract %1 ms, noding %2 ms (call %3 ms), make %4 ms" )
               .arg( timeExtract ).arg( timeNoding ).arg( timeNodingCall ).arg( timeMake ) );
  return true;
}

QgsTracer::~QgsTracer()
{
  invalidateGraph();
}

void QgsTracer::setLayers( const QList<QgsVectorLayer*>& layers )
{
  if ( mLayers == layers )
    return;

  Q_FOREACH ( QgsVectorLayer* layer, mLayers )
  {
    disconnect( layer, SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( onFeatureAdded( QgsFeatureId ) ) );
    disconnect( layer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( onFeatureDeleted( QgsFeatureId ) ) );
    disconnect( layer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), this, SLOT( onGeometryChanged( QgsFeatureId, QgsGeometry& ) ) );
    disconnect( layer, SIGNAL( destroyed( QObject* ) ), this, SLOT( onLayerDestroyed( QObject* ) ) );
  }

  mLayers = layers;

  Q_FOREACH ( QgsVectorLayer* layer, mLayers )
  {
    connect( layer, SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( onFeatureAdded( QgsFeatureId ) ) );
    connect( layer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( onFeatureDeleted( QgsFeatureId ) ) );
    connect( layer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), this, SLOT( onGeometryChanged( QgsFeatureId, QgsGeometry& ) ) );
    connect( layer, SIGNAL( destroyed( QObject* ) ), this, SLOT( onLayerDestroyed( QObject* ) ) );
  }

  invalidateGraph();
}

void QgsTracer::setCrsTransformEnabled( bool enabled )
{
  if ( mReprojectionEnabled == enabled )
    return;

  mReprojectionEnabled = enabled;
  invalidateGraph();
}

void QgsTracer::setDestinationCrs( const QgsCoordinateReferenceSystem& crs )
{
  if ( mCRS == crs )
    return;

  mCRS = crs;
  invalidateGraph();
}

void QgsTracer::setExtent( const QgsRectangle& extent )
{
  if ( mExtent == extent )
    return;

  mExtent = extent;
  invalidateGraph();
}

bool QgsTracer::init()
{
  if ( mGraph )
    return true;

  // configuration from derived class?
  configure();

  return initGraph();
}


void QgsTracer::invalidateGraph()
{
  delete mGraph;
  mGraph = 0;
}

void QgsTracer::onFeatureAdded( QgsFeatureId fid )
{
  Q_UNUSED( fid );
  invalidateGraph();
}

void QgsTracer::onFeatureDeleted( QgsFeatureId fid )
{
  Q_UNUSED( fid );
  invalidateGraph();
}

void QgsTracer::onGeometryChanged( QgsFeatureId fid, QgsGeometry& geom )
{
  Q_UNUSED( fid );
  Q_UNUSED( geom );
  invalidateGraph();
}

void QgsTracer::onLayerDestroyed( QObject* obj )
{
  // remove the layer before it is completely invalid (static_cast should be the safest cast)
  mLayers.removeAll( static_cast<QgsVectorLayer*>( obj ) );
  invalidateGraph();
}

QVector<QgsPoint> QgsTracer::findShortestPath( const QgsPoint& p1, const QgsPoint& p2, PathError* error )
{
  init();  // does nothing if the graph exists already
  if ( !mGraph )
  {
    if ( error ) *error = ErrTooManyFeatures;
    return QVector<QgsPoint>();
  }

  QTime t;
  t.start();
  int v1 = pointInGraph( *mGraph, p1 );
  int v2 = pointInGraph( *mGraph, p2 );
  int tPrep = t.elapsed();

  if ( v1 == -1 )
  {
    if ( error ) *error = ErrPoint1;
    return QVector<QgsPoint>();
  }
  if ( v2 == -1 )
  {
    if ( error ) *error = ErrPoint2;
    return QVector<QgsPoint>();
  }

  QTime t2;
  t2.start();
  QgsPolyline points = shortestPath( *mGraph, v1, v2 );
  int tPath = t2.elapsed();

  Q_UNUSED( tPrep );
  Q_UNUSED( tPath );
  QgsDebugMsg( QString( "path timing: prep %1 ms, path %2 ms" ).arg( tPrep ).arg( tPath ) );

  resetGraph( *mGraph );

  if ( error )
    *error = points.isEmpty() ? ErrNoPath : ErrNone;

  return points;
}

bool QgsTracer::isPointSnapped( const QgsPoint& pt )
{
  init();  // does nothing if the graph exists already
  if ( !mGraph )
    return false;

  if ( point2vertex( *mGraph, pt ) != -1 )
    return true;

  int lineVertexAfter;
  int e = point2edge( *mGraph, pt, lineVertexAfter );
  return e != -1;
}
