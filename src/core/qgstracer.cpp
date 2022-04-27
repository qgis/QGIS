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


#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsexception.h"
#include "qgsrenderer.h"
#include "qgssettingsregistrycore.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrendercontext.h"

#include <queue>
#include <vector>

typedef std::pair<int, double> DijkstraQueueItem; // first = vertex index, second = distance

// utility comparator for queue items based on distance
struct comp
{
  bool operator()( DijkstraQueueItem a, DijkstraQueueItem b ) const
  {
    return a.second > b.second;
  }
};


// TODO: move to geometry utils
double distance2D( const QgsPolylineXY &coords )
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
    dist += std::sqrt( ( x1 - x0 ) * ( x1 - x0 ) + ( y1 - y0 ) * ( y1 - y0 ) );
    x0 = x1;
    y0 = y1;
  }
  return dist;
}


// TODO: move to geometry utils
double closestSegment( const QgsPolylineXY &pl, const QgsPointXY &pt, int &vertexAfter, double epsilon )
{
  double sqrDist = std::numeric_limits<double>::max();
  const QgsPointXY *pldata = pl.constData();
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

//! Simple graph structure for shortest path search
struct QgsTracerGraph
{
  QgsTracerGraph()  = default;

  struct E  // bidirectional edge
  {
    //! vertices that the edge connects
    int v1, v2;
    //! coordinates of the edge (including endpoints)
    QVector<QgsPointXY> coords;

    int otherVertex( int v0 ) const { return v1 == v0 ? v2 : v1; }
    double weight() const { return distance2D( coords ); }
  };

  struct V
  {
    //! location of the vertex
    QgsPointXY pt;
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
  int joinedVertices{ 0 };
};


QgsTracerGraph *makeGraph( const QVector<QgsPolylineXY> &edges )
{
  QgsTracerGraph *g = new QgsTracerGraph();
  g->joinedVertices = 0;
  QHash<QgsPointXY, int> point2vertex;

  const auto constEdges = edges;
  for ( const QgsPolylineXY &line : constEdges )
  {
    QgsPointXY p1( line[0] );
    QgsPointXY p2( line[line.count() - 1] );

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


QVector<QgsPointXY> shortestPath( const QgsTracerGraph &g, int v1, int v2 )
{
  if ( v1 == -1 || v2 == -1 )
    return QVector<QgsPointXY>(); // invalid input

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

    const QgsTracerGraph::V &vu = g.v[u];
    const int *vuEdges = vu.edges.constData();
    int count = vu.edges.count();
    for ( int i = 0; i < count; ++i )
    {
      const QgsTracerGraph::E &edge = g.e[ vuEdges[i] ];
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
    F[u] = true; // mark the vertex as processed (we know the fastest path to it)
  }

  if ( u != v2 ) // there's no path to the end vertex
    return QVector<QgsPointXY>();

  //qDebug("dist %f", D[u]);

  QVector<QgsPointXY> points;
  QList<int> path;
  while ( S[u] != -1 )
  {
    path << S[u];
    const QgsTracerGraph::E &e = g.e[S[u]];
    QVector<QgsPointXY> edgePoints = e.coords;
    if ( edgePoints[0] != g.v[u].pt )
      std::reverse( edgePoints.begin(), edgePoints.end() );
    if ( !points.isEmpty() )
      points.remove( points.count() - 1 );  // chop last one (will be used from next edge)
    points << edgePoints;
    u = e.otherVertex( u );
  }

  std::reverse( path.begin(), path.end() );
  std::reverse( points.begin(), points.end() );
  return points;
}


int point2vertex( const QgsTracerGraph &g, const QgsPointXY &pt, double epsilon = 1e-6 )
{
  // TODO: use spatial index

  for ( int i = 0; i < g.v.count(); ++i )
  {
    const QgsTracerGraph::V &v = g.v.at( i );
    if ( v.pt == pt || ( std::fabs( v.pt.x() - pt.x() ) < epsilon && std::fabs( v.pt.y() - pt.y() ) < epsilon ) )
      return i;
  }

  return -1;
}


int point2edge( const QgsTracerGraph &g, const QgsPointXY &pt, int &lineVertexAfter, double epsilon = 1e-6 )
{
  for ( int i = 0; i < g.e.count(); ++i )
  {
    if ( g.inactiveEdges.contains( i ) )
      continue;  // ignore temporarily disabled edges

    const QgsTracerGraph::E &e = g.e.at( i );
    int vertexAfter = -1;
    double dist = closestSegment( e.coords, pt, vertexAfter, epsilon );
    if ( dist == 0 )
    {
      lineVertexAfter = vertexAfter;
      return i;
    }
  }
  return -1;
}


void splitLinestring( const QgsPolylineXY &points, const QgsPointXY &pt, int lineVertexAfter, QgsPolylineXY &pts1, QgsPolylineXY &pts2 )
{
  int count1 = lineVertexAfter;
  int count2 = points.count() - lineVertexAfter;

  for ( int i = 0; i < count1; ++i )
    pts1 << points[i];
  if ( points[lineVertexAfter - 1] != pt )
    pts1 << pt;  // repeat if not split exactly at that point

  if ( pt != points[lineVertexAfter] )
    pts2 << pt;  // repeat if not split exactly at that point
  for ( int i = 0; i < count2; ++i )
    pts2 << points[i + lineVertexAfter];
}


int joinVertexToGraph( QgsTracerGraph &g, const QgsPointXY &pt )
{
  // find edge where the point is
  int lineVertexAfter;
  int eIdx = point2edge( g, pt, lineVertexAfter );

  //qDebug("e: %d", eIdx);

  if ( eIdx == -1 )
    return -1;

  const QgsTracerGraph::E &e = g.e[eIdx];
  QgsTracerGraph::V &v1 = g.v[e.v1];
  QgsTracerGraph::V &v2 = g.v[e.v2];

  QgsPolylineXY out1, out2;
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


int pointInGraph( QgsTracerGraph &g, const QgsPointXY &pt )
{
  // try to use existing vertex in the graph
  int v = point2vertex( g, pt );
  if ( v != -1 )
    return v;

  // try to add the vertex to an edge (may fail if point is not on edge)
  return joinVertexToGraph( g, pt );
}


void resetGraph( QgsTracerGraph &g )
{
  // remove extra vertices and edges
  g.v.resize( g.v.count() - g.joinedVertices );
  g.e.resize( g.e.count() - g.joinedVertices * 2 );
  g.joinedVertices = 0;

  // fix vertices of deactivated edges
  for ( int eIdx : std::as_const( g.inactiveEdges ) )
  {
    if ( eIdx >= g.e.count() )
      continue;
    const QgsTracerGraph::E &e = g.e[eIdx];
    QgsTracerGraph::V &v1 = g.v[e.v1];
    for ( int i = 0; i < v1.edges.count(); ++i )
    {
      if ( v1.edges[i] >= g.e.count() )
        v1.edges.remove( i-- );
    }
    v1.edges << eIdx;

    QgsTracerGraph::V &v2 = g.v[e.v2];
    for ( int i = 0; i < v2.edges.count(); ++i )
    {
      if ( v2.edges[i] >= g.e.count() )
        v2.edges.remove( i-- );
    }
    v2.edges << eIdx;
  }

  g.inactiveEdges.clear();
}


void extractLinework( const QgsGeometry &g, QgsMultiPolylineXY &mpl )
{
  QgsGeometry geom = g;
  // segmentize curved geometries - we will use noding algorithm from GEOS
  // to find all intersections a bit later (so we need them segmentized anyway)
  if ( QgsWkbTypes::isCurvedType( g.wkbType() ) )
  {
    QgsAbstractGeometry *segmentizedGeomV2 = g.constGet()->segmentize();
    if ( !segmentizedGeomV2 )
      return;

    geom = QgsGeometry( segmentizedGeomV2 );
  }

  switch ( QgsWkbTypes::flatType( geom.wkbType() ) )
  {
    case QgsWkbTypes::LineString:
      mpl << geom.asPolyline();
      break;

    case QgsWkbTypes::Polygon:
    {
      const auto polygon = geom.asPolygon();
      for ( const QgsPolylineXY &ring : polygon )
        mpl << ring;
    }
    break;

    case QgsWkbTypes::MultiLineString:
    {
      const auto multiPolyline = geom.asMultiPolyline();
      for ( const QgsPolylineXY &linestring : multiPolyline )
        mpl << linestring;
    }
    break;

    case QgsWkbTypes::MultiPolygon:
    {
      const auto multiPolygon = geom.asMultiPolygon();
      for ( const QgsPolygonXY &polygon : multiPolygon )
      {
        for ( const QgsPolylineXY &ring : polygon )
          mpl << ring;
      }
    }
    break;

    default:
      break;  // unknown type - do nothing
  }
}

// -------------


QgsTracer::QgsTracer() = default;

bool QgsTracer::initGraph()
{
  if ( mGraph )
    return true; // already initialized

  mHasTopologyProblem = false;

  QgsFeature f;
  QgsMultiPolylineXY mpl;

  // extract linestrings

  // TODO: use QgsPointLocator as a source for the linework

  QElapsedTimer t1, t2, t2a, t3;

  t1.start();
  int featuresCounted = 0;
  for ( const QgsVectorLayer *vl : std::as_const( mLayers ) )
  {
    QgsFeatureRequest request;
    bool filter = false;
    std::unique_ptr< QgsFeatureRenderer > renderer;
    std::unique_ptr<QgsRenderContext> ctx;

    bool enableInvisibleFeature = QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature.value();
    if ( !enableInvisibleFeature && mRenderContext && vl->renderer() )
    {
      renderer.reset( vl->renderer()->clone() );
      ctx.reset( new QgsRenderContext( *mRenderContext.get() ) );
      ctx->expressionContext() << QgsExpressionContextUtils::layerScope( vl );

      // setup scale for scale dependent visibility (rule based)
      renderer->startRender( *ctx.get(), vl->fields() );
      filter = renderer->capabilities() & QgsFeatureRenderer::Filter;
      request.setSubsetOfAttributes( renderer->usedAttributes( *ctx.get() ), vl->fields() );
    }
    else
    {
      request.setNoAttributes();
    }

    request.setDestinationCrs( mCRS, mTransformContext );
    if ( !mExtent.isEmpty() )
      request.setFilterRect( mExtent );

    QgsFeatureIterator fi = vl->getFeatures( request );
    while ( fi.nextFeature( f ) )
    {
      if ( !f.hasGeometry() )
        continue;

      if ( filter )
      {
        ctx->expressionContext().setFeature( f );
        if ( !renderer->willRenderFeature( f, *ctx.get() ) )
        {
          continue;
        }
      }

      extractLinework( f.geometry(), mpl );

      ++featuresCounted;
      if ( mMaxFeatureCount != 0 && featuresCounted >= mMaxFeatureCount )
        return false;
    }

    if ( renderer )
    {
      renderer->stopRender( *ctx.get() );
    }
  }
  int timeExtract = t1.elapsed();

  // resolve intersections

  t2.start();

  int timeNodingCall = 0;

#if 0
  // without noding - if data are known to be noded beforehand
#else
  QgsGeometry allGeom = QgsGeometry::fromMultiPolylineXY( mpl );

  try
  {
    t2a.start();
    // GEOSNode_r may throw an exception
    geos::unique_ptr allGeomGeos( QgsGeos::asGeos( allGeom ) );
    geos::unique_ptr allNoded( GEOSNode_r( QgsGeos::getGEOSHandler(), allGeomGeos.get() ) );
    timeNodingCall = t2a.elapsed();

    QgsGeometry noded = QgsGeos::geometryFromGeos( allNoded.release() );

    mpl = noded.asMultiPolyline();
  }
  catch ( GEOSException &e )
  {
    // no big deal... we will just not have nicely noded linework, potentially
    // missing some intersections

    mHasTopologyProblem = true;

    QgsDebugMsg( QStringLiteral( "Tracer Noding Exception: %1" ).arg( e.what() ) );
  }
#endif

  int timeNoding = t2.elapsed();

  t3.start();

  mGraph.reset( makeGraph( mpl ) );

  int timeMake = t3.elapsed();

  Q_UNUSED( timeExtract )
  Q_UNUSED( timeNoding )
  Q_UNUSED( timeNodingCall )
  Q_UNUSED( timeMake )
  QgsDebugMsgLevel( QStringLiteral( "tracer extract %1 ms, noding %2 ms (call %3 ms), make %4 ms" )
                    .arg( timeExtract ).arg( timeNoding ).arg( timeNodingCall ).arg( timeMake ), 2 );

  return true;
}

QgsTracer::~QgsTracer()
{
  invalidateGraph();
}

void QgsTracer::setLayers( const QList<QgsVectorLayer *> &layers )
{
  if ( mLayers == layers )
    return;

  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
  {
    disconnect( layer, &QgsVectorLayer::featureAdded, this, &QgsTracer::onFeatureAdded );
    disconnect( layer, &QgsVectorLayer::featureDeleted, this, &QgsTracer::onFeatureDeleted );
    disconnect( layer, &QgsVectorLayer::geometryChanged, this, &QgsTracer::onGeometryChanged );
    disconnect( layer, &QgsVectorLayer::attributeValueChanged, this, &QgsTracer::onAttributeValueChanged );
    disconnect( layer, &QgsVectorLayer::dataChanged, this, &QgsTracer::onDataChanged );
    disconnect( layer, &QgsVectorLayer::styleChanged, this, &QgsTracer::onStyleChanged );
    disconnect( layer, &QObject::destroyed, this, &QgsTracer::onLayerDestroyed );
  }

  mLayers = layers;

  for ( QgsVectorLayer *layer : layers )
  {
    connect( layer, &QgsVectorLayer::featureAdded, this, &QgsTracer::onFeatureAdded );
    connect( layer, &QgsVectorLayer::featureDeleted, this, &QgsTracer::onFeatureDeleted );
    connect( layer, &QgsVectorLayer::geometryChanged, this, &QgsTracer::onGeometryChanged );
    connect( layer, &QgsVectorLayer::attributeValueChanged, this, &QgsTracer::onAttributeValueChanged );
    connect( layer, &QgsVectorLayer::dataChanged, this, &QgsTracer::onDataChanged );
    connect( layer, &QgsVectorLayer::styleChanged, this, &QgsTracer::onStyleChanged );
    connect( layer, &QObject::destroyed, this, &QgsTracer::onLayerDestroyed );
  }

  invalidateGraph();
}

void QgsTracer::setDestinationCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCRS = crs;
  mTransformContext = context;
  invalidateGraph();
}

void QgsTracer::setRenderContext( const QgsRenderContext *renderContext )
{
  mRenderContext.reset( new QgsRenderContext( *renderContext ) );
  invalidateGraph();
}

void QgsTracer::setExtent( const QgsRectangle &extent )
{
  if ( mExtent == extent )
    return;

  mExtent = extent;
  invalidateGraph();
}

void QgsTracer::setOffset( double offset )
{
  mOffset = offset;
}

void QgsTracer::offsetParameters( int &quadSegments, int &joinStyle, double &miterLimit )
{
  quadSegments = mOffsetSegments;
  joinStyle = static_cast< int >( mOffsetJoinStyle );
  miterLimit = mOffsetMiterLimit;
}

void QgsTracer::setOffsetParameters( int quadSegments, int joinStyle, double miterLimit )
{
  mOffsetSegments = quadSegments;
  mOffsetJoinStyle = static_cast< Qgis::JoinStyle >( joinStyle );
  mOffsetMiterLimit = miterLimit;
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
  mGraph.reset( nullptr );
}

void QgsTracer::onFeatureAdded( QgsFeatureId fid )
{
  Q_UNUSED( fid )
  invalidateGraph();
}

void QgsTracer::onFeatureDeleted( QgsFeatureId fid )
{
  Q_UNUSED( fid )
  invalidateGraph();
}

void QgsTracer::onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  Q_UNUSED( fid )
  Q_UNUSED( geom )
  invalidateGraph();
}

void QgsTracer::onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  Q_UNUSED( fid )
  Q_UNUSED( idx )
  Q_UNUSED( value )
  invalidateGraph();
}

void QgsTracer::onDataChanged( )
{
  invalidateGraph();
}

void QgsTracer::onStyleChanged( )
{
  invalidateGraph();
}

void QgsTracer::onLayerDestroyed( QObject *obj )
{
  // remove the layer before it is completely invalid (static_cast should be the safest cast)
  mLayers.removeAll( static_cast<QgsVectorLayer *>( obj ) );
  invalidateGraph();
}

QVector<QgsPointXY> QgsTracer::findShortestPath( const QgsPointXY &p1, const QgsPointXY &p2, PathError *error )
{
  init();  // does nothing if the graph exists already
  if ( !mGraph )
  {
    if ( error ) *error = ErrTooManyFeatures;
    return QVector<QgsPointXY>();
  }

  QElapsedTimer t;
  t.start();
  int v1 = pointInGraph( *mGraph, p1 );
  int v2 = pointInGraph( *mGraph, p2 );
  int tPrep = t.elapsed();

  if ( v1 == -1 )
  {
    if ( error ) *error = ErrPoint1;
    return QVector<QgsPointXY>();
  }
  if ( v2 == -1 )
  {
    if ( error ) *error = ErrPoint2;
    return QVector<QgsPointXY>();
  }

  QElapsedTimer t2;
  t2.start();
  QgsPolylineXY points = shortestPath( *mGraph, v1, v2 );
  int tPath = t2.elapsed();

  Q_UNUSED( tPrep )
  Q_UNUSED( tPath )
  QgsDebugMsgLevel( QStringLiteral( "path timing: prep %1 ms, path %2 ms" ).arg( tPrep ).arg( tPath ), 2 );

  resetGraph( *mGraph );

  if ( !points.isEmpty() && mOffset != 0 )
  {
    QVector<QgsPointXY> pointsInput( points );
    QgsLineString linestring( pointsInput );
    std::unique_ptr<QgsGeometryEngine> linestringEngine( QgsGeometry::createGeometryEngine( &linestring ) );
    std::unique_ptr<QgsAbstractGeometry> linestringOffset( linestringEngine->offsetCurve( mOffset, mOffsetSegments, mOffsetJoinStyle, mOffsetMiterLimit ) );
    if ( QgsLineString *ls2 = qgsgeometry_cast<QgsLineString *>( linestringOffset.get() ) )
    {
      points.clear();
      for ( int i = 0; i < ls2->numPoints(); ++i )
        points << QgsPointXY( ls2->pointN( i ) );

      // sometimes (with negative offset?) the resulting curve is reversed
      if ( points.count() >= 2 )
      {
        QgsPointXY res1 = points.first(), res2 = points.last();
        double diffNormal = res1.distance( p1 ) + res2.distance( p2 );
        double diffReversed = res1.distance( p2 ) + res2.distance( p1 );
        if ( diffReversed < diffNormal )
          std::reverse( points.begin(), points.end() );
      }
    }
  }

  if ( error )
    *error = points.isEmpty() ? ErrNoPath : ErrNone;

  return points;
}

bool QgsTracer::isPointSnapped( const QgsPointXY &pt )
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
