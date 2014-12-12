/***************************************************************************
  qgspointlocator.cpp
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

#include "qgspointlocator.h"

#include "qgsgeometry.h"
#include "qgsvectorlayer.h"

#include <spatialindex/SpatialIndex.h>

#include <QLinkedListIterator>

using namespace SpatialIndex;



static void addPointToVertexData( QLinkedList<RTree::Data*>& vertexDataList, id_type id, int vertexIndex, const QgsPoint& pt )
{
  double pLow[2], pHigh[2];
  pLow[0] = pHigh[0] = pt.x();
  pLow[1] = pHigh[1] = pt.y();
  SpatialIndex::Region r( pLow, pHigh, 2 );
  vertexDataList << new RTree::Data( sizeof( int ), reinterpret_cast<byte*>( &vertexIndex ), r, id );
}


static void addVertexData( QLinkedList<RTree::Data*>& vertexDataList, const QgsFeature& f )
{
  QgsGeometry* g = f.geometry();
  if ( !g )
    return;

  id_type id = f.id();
  int vertexIndex = -1;

  switch ( QGis::flatType( g->wkbType() ) )
  {
    case QGis::WKBPoint:
      addPointToVertexData( vertexDataList, id, 0, g->asPoint() );
      break;
    case QGis::WKBLineString:
    {
      foreach ( const QgsPoint& pt, g->asPolyline() )
        addPointToVertexData( vertexDataList, id, ++vertexIndex, pt );
    }
    break;
    case QGis::WKBPolygon:
    {
      foreach ( const QgsPolyline& polyline, g->asPolygon() )
        foreach ( const QgsPoint& pt, polyline )
          addPointToVertexData( vertexDataList, id, ++vertexIndex, pt );
    }
    break;
    case QGis::WKBMultiPoint:
    {
      foreach ( const QgsPoint& pt, g->asMultiPoint() )
        addPointToVertexData( vertexDataList, id, ++vertexIndex, pt );
    }
    break;
    case QGis::WKBMultiLineString:
    {
      foreach ( const QgsPolyline& polyline, g->asMultiPolyline() )
        foreach ( const QgsPoint& pt, polyline )
          addPointToVertexData( vertexDataList, id, ++vertexIndex, pt );
    }
    break;
    case QGis::WKBMultiPolygon:
    {
      foreach ( const QgsPolygon& polygon, g->asMultiPolygon() )
        foreach ( const QgsPolyline& polyline, polygon )
          foreach ( const QgsPoint& pt, polyline )
            addPointToVertexData( vertexDataList, id, ++vertexIndex, pt );
    }
    break;
    default:
      return;
  }

}


static SpatialIndex::Point point2point( const QgsPoint& point )
{
  double plow[2];
  plow[0] = point.x();
  plow[1] = point.y();
  return Point( plow, 2 );
}


static SpatialIndex::Region rect2region( const QgsRectangle& rect )
{
  double pLow[2], pHigh[2];
  pLow[0] = rect.xMinimum();
  pLow[1] = rect.yMinimum();
  pHigh[0] = rect.xMaximum();
  pHigh[1] = rect.yMaximum();
  return SpatialIndex::Region( pLow, pHigh, 2 );
}

static QgsRectangle region2rect( const SpatialIndex::Region& region )
{
  return QgsRectangle( region.m_pLow[0], region.m_pLow[1], region.m_pHigh[0], region.m_pHigh[1] );
}

static void addPolylineToEdgeData( QLinkedList<RTree::Data*>& edgeDataList, const QgsPolyline& pl, id_type id, int& vertexIndex )
{
  if ( pl.count() < 2 )
    return;
  QgsPoint lastPt = pl[0];
  for ( int i = 1; i < pl.count(); ++i )
  {
    vertexIndex++;
    QgsPoint pt = pl[i];

    bool edgeDirection = ( pt.x() < lastPt.x() && pt.y() < lastPt.y() ) || ( pt.x() > lastPt.x() && pt.y() > lastPt.y() );
    bool isReversed = pt.x() < lastPt.x() || ( pt.x() == lastPt.x() && lastPt.y() < pt.y() );
    double pLow[2], pHigh[2];
    pLow[0]  = qMin( pt.x(), lastPt.x() );
    pHigh[0] = qMax( pt.x(), lastPt.x() );
    pLow[1]  = qMin( pt.y(), lastPt.y() );
    pHigh[1] = qMax( pt.y(), lastPt.y() );
    SpatialIndex::Region r( pLow, pHigh, 2 );
    int edgeFlags = edgeDirection | ( isReversed << 1 );
    int extraData[2] = { vertexIndex, edgeFlags };
    edgeDataList << new RTree::Data( sizeof( int )*2, reinterpret_cast<byte*>( extraData ), r, id );

    lastPt = pt;
  }
  vertexIndex++; // for the last vertex
}


static void addEdgeData( QLinkedList<RTree::Data*>& edgeDataList, const QgsFeature& f )
{
  QgsGeometry* g = f.geometry();
  if ( !g )
    return;

  id_type id = f.id();
  int vertexIndex = -1;

  switch ( QGis::flatType( g->wkbType() ) )
  {
    case QGis::WKBLineString:
      addPolylineToEdgeData( edgeDataList, g->asPolyline(), id, vertexIndex );
      break;
    case QGis::WKBPolygon:
    {
      foreach ( const QgsPolyline& polyline, g->asPolygon() )
        addPolylineToEdgeData( edgeDataList, polyline, id, vertexIndex );
    }
    break;
    case QGis::WKBMultiLineString:
    {
      foreach ( const QgsPolyline& polyline, g->asMultiPolyline() )
        addPolylineToEdgeData( edgeDataList, polyline, id, vertexIndex );
    }
    break;
    case QGis::WKBMultiPolygon:
    {
      foreach ( const QgsPolygon& polygon, g->asMultiPolygon() )
        foreach ( const QgsPolyline& polyline, polygon )
          addPolylineToEdgeData( edgeDataList, polyline, id, vertexIndex );
    }
    break;

    case QGis::WKBPoint:
    case QGis::WKBMultiPoint:
    default:
      return;
  }
}


static void addPolygonToAreaData( QLinkedList<RTree::Data*>& areaDataList, QList<GEOSGeometry*>& geomList, const GEOSGeometry* geosGeomOrig, const QgsRectangle& bbox, id_type id )
{
  SpatialIndex::Region r( rect2region( bbox ) );
  GEOSGeometry* geosGeom = GEOSGeom_clone_r( QgsGeometry::getGEOSHandler(), geosGeomOrig );
  geomList << geosGeom;
  areaDataList << new RTree::Data( sizeof( const GEOSGeometry* ), reinterpret_cast<byte*>( &geosGeom ), r, id );
}


static void addAreaData( QLinkedList<RTree::Data*>& areaDataList, QList<GEOSGeometry*>& geomList, const QgsFeature& f )
{
  QgsGeometry* g = f.geometry();
  if ( !g )
    return;

  id_type id = f.id();
  QgsRectangle bbox = g->boundingBox();

  switch ( QGis::flatType( g->wkbType() ) )
  {
    case QGis::WKBPolygon:
    {
      addPolygonToAreaData( areaDataList, geomList, g->asGeos(), bbox, id );
    }
    break;
    case QGis::WKBMultiPolygon:
    {
      GEOSContextHandle_t geos = QgsGeometry::getGEOSHandler();
      const GEOSGeometry* origGeosGeom = g->asGeos();
      for ( int i = 0; i < GEOSGetNumGeometries_r( geos, origGeosGeom ); i++ )
      {
        const GEOSGeometry *origGeosPolygon = GEOSGetGeometryN_r( geos, origGeosGeom, i );
        addPolygonToAreaData( areaDataList, geomList, origGeosPolygon, bbox, id );
      }
    }
    break;
    default:
      break;
  }
}



static void edgeGetEndpoints( const RTree::Data& dd, double&x1, double&y1, double&x2, double&y2 )
{
  x1 = dd.m_region.m_pLow[0];
  x2 = dd.m_region.m_pHigh[0];
  int flags = reinterpret_cast<int*>( dd.m_pData )[1];
  if ( flags & 1 )  // direction bottom-left to top-right
  {
    y1 = dd.m_region.m_pLow[1];
    y2 = dd.m_region.m_pHigh[1];
  }
  else // direction top-left to bottom-right
  {
    y1 = dd.m_region.m_pHigh[1];
    y2 = dd.m_region.m_pLow[1];
  }

  // the other flag tells the direction of the edge: whether it is [x1,y1]->[x2,y2] or reversed
  if ( flags & 2 )
  {
    qSwap( x1, x2 );
    qSwap( y1, y2 );
  }
}


static LineSegment edge2lineSegment( const RTree::Data& dd )
{
  double pStart[2], pEnd[2];
  edgeGetEndpoints( dd, pStart[0], pStart[1], pEnd[0], pEnd[1] );
  return LineSegment( pStart, pEnd, 2 );
}


// for a Data in the edge tree, find out distance from a point
static double edgeMinDist( const RTree::Data& dd, const Point& queryP )
{
  return edge2lineSegment( dd ).getMinimumDistance( queryP );
}


////////////////////////////////////////////////////////////////////////////


/** Helper class for bulk loading of R-trees. */
class QgsPointLocator_Stream : public IDataStream
{
  public:
    QgsPointLocator_Stream( const QLinkedList<RTree::Data*>& dataList ) : mDataList( dataList ), mIt( mDataList ) { }
    ~QgsPointLocator_Stream() { }

    virtual IData* getNext() { return mIt.next(); }
    virtual bool hasNext() { return mIt.hasNext(); }

    virtual uint32_t size() { Q_ASSERT( 0 && "not available" ); return 0; }
    virtual void rewind() { Q_ASSERT( 0 && "not available" ); }

  private:
    QLinkedList<RTree::Data*> mDataList;
    QLinkedListIterator<RTree::Data*> mIt;
};


////////////////////////////////////////////////////////////////////////////


/** Helper class used when traversing the index with vertices or areas - builds a list of matches. */
class QgsPointLocator_VisitorVertexEdge : public IVisitor
{
  public:
    //! constructor for NN queries
    QgsPointLocator_VisitorVertexEdge( QgsVectorLayer* vl, bool vertexTree, const QgsPoint& origPt, QgsPointLocator::MatchList& list )
        : mLayer( vl ), mVertexTree( vertexTree ), mNNQuery( true ), mOrigPt( origPt ), mList( list ) {}

    //! constructor for range queries
    QgsPointLocator_VisitorVertexEdge( QgsVectorLayer* vl, bool vertexTree, const QgsRectangle& origRect, QgsPointLocator::MatchList& list )
        : mLayer( vl ), mVertexTree( vertexTree ), mNNQuery( false ), mOrigRect( origRect ), mList( list ) {}

    void visitNode( const INode& n ) { Q_UNUSED( n ); }
    void visitData( std::vector<const IData*>& v ) { Q_UNUSED( v ); }

    void visitData( const IData& d )
    {
      const RTree::Data& dd = static_cast<const RTree::Data&>( d );
      QgsPoint pt;
      double dist;
      QgsPoint edgePoints[2];
      if ( mNNQuery )
      {
        // neirest neighbor query
        if ( mVertexTree )
        {
          pt = QgsPoint( dd.m_region.m_pLow[0], dd.m_region.m_pLow[1] );
          dist = sqrt( pt.sqrDist( mOrigPt ) );
        }
        else
        {
          double x1, x2, y1, y2;
          edgeGetEndpoints( dd, x1, y1, x2, y2 );
          dist = sqrt( mOrigPt.sqrDistToSegment( x1, y1, x2, y2, pt ) );
          edgePoints[0].set( x1, y1 );
          edgePoints[1].set( x2, y2 );
        }
      }
      else
      {
        // range query
        // distance + point do not make sense here... keep them empty
        dist = 0;
        if ( !mVertexTree )
        {
          // need to check if the edge actually intersects the region
          SpatialIndex::Region r( rect2region( mOrigRect ) );
          double pStart[2], pEnd[2];
          edgeGetEndpoints( dd, pStart[0], pStart[1], pEnd[0], pEnd[1] );
          LineSegment ls( pStart, pEnd, 2 );
          if ( !r.intersectsLineSegment( ls ) )
            return;
          edgePoints[0].set( pStart[0], pStart[1] );
          edgePoints[1].set( pEnd[0], pEnd[1] );
        }
      }
      QgsPointLocator::Type t = mVertexTree ? QgsPointLocator::Vertex : QgsPointLocator::Edge;
      int vertexIndex = *reinterpret_cast<int*>( dd.m_pData );
      mList << QgsPointLocator::Match( t, mLayer, d.getIdentifier(), dist, pt, vertexIndex, t == QgsPointLocator::Edge ? edgePoints : 0 );
    }

  private:
    QgsVectorLayer* mLayer;
    bool mVertexTree;
    bool mNNQuery;
    QgsPoint mOrigPt; // only for NN queries
    QgsRectangle mOrigRect; // only for range queries
    QgsPointLocator::MatchList& mList;
};


////////////////////////////////////////////////////////////////////////////


/** Helper class used when traversing the index with areas - builds a list of matches. */
class QgsPointLocator_VisitorArea : public IVisitor
{
  public:
    //! constructor
    QgsPointLocator_VisitorArea( QgsVectorLayer* vl, const QgsPoint& origPt, QgsPointLocator::MatchList& list )
        : mLayer( vl ), mList( list )
    {
      GEOSContextHandle_t geosH = QgsGeometry::getGEOSHandler();
      GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosH, 1, 2 );
      GEOSCoordSeq_setX_r( geosH, coord, 0, origPt.x() );
      GEOSCoordSeq_setY_r( geosH, coord, 0, origPt.y() );
      geomPt = GEOSGeom_createPoint_r( geosH, coord );
    }

    ~QgsPointLocator_VisitorArea()
    {
      GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), geomPt );
    }

    void visitNode( const INode& n ) { Q_UNUSED( n ); }
    void visitData( std::vector<const IData*>& v ) { Q_UNUSED( v ); }

    void visitData( const IData& d )
    {
      const RTree::Data& dd = static_cast<const RTree::Data&>( d );
      GEOSGeometry* g = *reinterpret_cast<GEOSGeometry**>( dd.m_pData );
      // TODO: handle exceptions
      if ( GEOSIntersects_r( QgsGeometry::getGEOSHandler(), g, geomPt ) )
        mList << QgsPointLocator::Match( QgsPointLocator::Area, mLayer, d.getIdentifier(), 0, QgsPoint() );
    }
  private:
    QgsVectorLayer* mLayer;
    QgsPointLocator::MatchList& mList;
    GEOSGeometry* geomPt;
};


////////////////////////////////////////////////////////////////////////////


/** Helper class for calculation of minimum distance between point and edge (in edge index) */
class EdgeNNComparator : public INearestNeighborComparator
{
  public:
    double getMinimumDistance( const IShape& query, const IShape& entry )
    {
      return query.getMinimumDistance( entry );
    }

    double getMinimumDistance( const IShape& query, const IData& data )
    {
      const RTree::Data& dd = static_cast<const RTree::Data&>( data );
      const Point& queryP = dynamic_cast<const Point&>( query ); // virtual base - cannot use static cast
      return edgeMinDist( dd, queryP );
    }
};


////////////////////////////////////////////////////////////////////////////
#include <QStack>

/** Helper class to dump the R-index nodes and their content */
class QgsPointLocator_DumpTree : public SpatialIndex::IQueryStrategy
{
  private:
    QStack<id_type> ids;

  public:

    void getNextEntry( const IEntry& entry, id_type& nextEntry, bool& hasNext )
    {
      const INode* n = dynamic_cast<const INode*>( &entry );
      qDebug( "NODE: %ld", n->getIdentifier() );
      if ( n->getLevel() > 0 )
      {
        // inner nodes
        for ( uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++ )
        {
          qDebug( "- CH: %ld", n->getChildIdentifier( cChild ) );
          ids.push( n->getChildIdentifier( cChild ) );
        }
      }
      else
      {
        // leaves
        for ( uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++ )
        {
          qDebug( "- L: %ld", n->getChildIdentifier( cChild ) );
        }
      }

      if ( ! ids.empty() )
      {
        nextEntry = ids.back(); ids.pop();
        hasNext = true;
      }
      else
        hasNext = false;
    }
};

////////////////////////////////////////////////////////////////////////////


QgsPointLocator::QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS )
    : mStorage( 0 )
    , mRTreeVertex( 0 )
    , mRTreeEdge( 0 )
    , mRTreeArea( 0 )
    , mTransform( 0 )
    , mLayer( layer )
{
  if ( destCRS )
  {
    mTransform = new QgsCoordinateTransform( layer->crs(), *destCRS );
  }

  mStorage = StorageManager::createNewMemoryStorageManager();

  connect( mLayer, SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( onFeatureAdded( QgsFeatureId ) ) );
  connect( mLayer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( onFeatureDeleted( QgsFeatureId ) ) );
  connect( mLayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), this, SLOT( onGeometryChanged( QgsFeatureId, QgsGeometry& ) ) );
}


QgsPointLocator::~QgsPointLocator()
{
  destroyIndex( All );
  delete mStorage;
  delete mTransform;
}


void QgsPointLocator::init( int types, bool force )
{
  if ( !force )
  {
    // init only indices that do not exist yet
    if (( types & Vertex ) && hasIndex( Vertex ) )
      types &= ~Vertex;
    if (( types & Edge ) && hasIndex( Edge ) )
      types &= ~Edge;
    if (( types & Area ) && hasIndex( Area ) )
      types &= ~Area;
  }

  rebuildIndex( types );
}


bool QgsPointLocator::hasIndex( QgsPointLocator::Type t ) const
{
  switch ( t )
  {
    case Vertex: return mRTreeVertex != 0;
    case Edge:   return mRTreeEdge   != 0;
    case Area:   return mRTreeArea   != 0;
    default:     return 0;
  }
}


void QgsPointLocator::rebuildIndex( int types )
{
  destroyIndex( types );

  QLinkedList<RTree::Data*> vertexDataList, edgeDataList, areaDataList;
  QgsFeature f;
  QGis::GeometryType geomType = mLayer->geometryType();
  if ( geomType == QGis::NoGeometry )
    return; // nothing to index

  bool indexVertex = ( types & Vertex ) && ( geomType == QGis::Polygon || geomType == QGis::Line || geomType == QGis::Point );
  bool indexEdge   = ( types & Edge )   && ( geomType == QGis::Polygon || geomType == QGis::Line );
  bool indexArea   = ( types & Area )   && ( geomType == QGis::Polygon );
  if ( !indexVertex && !indexEdge && !indexArea )
    return; // nothing to index

  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator fi = mLayer->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( !f.geometry() )
      continue;

    if ( mTransform )
      f.geometry()->transform( *mTransform );

    if ( indexVertex )
      addVertexData( vertexDataList, f );

    if ( indexEdge )
      addEdgeData( edgeDataList, f );

    if ( indexArea )
      addAreaData( areaDataList, mAreaGeomList, f );
  }

  // R-Tree parameters
  double fillFactor = 0.7;
  unsigned long indexCapacity = 10;
  unsigned long leafCapacity = 10;
  unsigned long dimension = 2;
  RTree::RTreeVariant variant = RTree::RV_RSTAR;
  SpatialIndex::id_type indexId;

  // create R-tree for vertices
  if ( vertexDataList.count() )
  {
    QgsPointLocator_Stream stream( vertexDataList );
    mRTreeVertex = RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, stream, *mStorage, fillFactor, indexCapacity,
                   leafCapacity, dimension, variant, indexId );
  }

  // create R-tree for vedges
  if ( edgeDataList.count() )
  {
    QgsPointLocator_Stream stream2( edgeDataList );
    mRTreeEdge = RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, stream2, *mStorage, fillFactor, indexCapacity,
                 leafCapacity, dimension, variant, indexId );
  }

  // create R-tree for areas
  if ( areaDataList.count() )
  {
    QgsPointLocator_Stream stream3( areaDataList );
    mRTreeArea = RTree::createAndBulkLoadNewRTree( RTree::BLM_STR, stream3, *mStorage, fillFactor, indexCapacity,
                 leafCapacity, dimension, variant, indexId );
  }
}


void QgsPointLocator::destroyIndex( int types )
{
  if ( types & Vertex )
  {
    delete mRTreeVertex;
    mRTreeVertex = 0;
  }

  if ( types & Edge )
  {
    delete mRTreeEdge;
    mRTreeEdge = 0;
  }

  if ( types & Area )
  {
    delete mRTreeArea;
    mRTreeArea = 0;

    foreach ( GEOSGeometry* g, mAreaGeomList )
      GEOSGeom_destroy_r( QgsGeometry::getGEOSHandler(), g );
    mAreaGeomList.clear();
  }
}

void QgsPointLocator::onFeatureAdded( QgsFeatureId fid )
{
  Q_UNUSED( fid );
  destroyIndex( All );
}

void QgsPointLocator::onFeatureDeleted( QgsFeatureId fid )
{
  Q_UNUSED( fid );
  destroyIndex( All );
}

void QgsPointLocator::onGeometryChanged( QgsFeatureId fid, QgsGeometry& geom )
{
  Q_UNUSED( fid );
  Q_UNUSED( geom );
  destroyIndex( All );
}


QgsPointLocator::MatchList QgsPointLocator::nearestVertices( const QgsPoint& point, int maxMatches )
{
  if ( !mRTreeVertex )
  {
    init( Vertex );
    if ( !mRTreeVertex ) // still invalid?
      return MatchList();
  }

  double plow[2];
  plow[0] = point.x();
  plow[1] = point.y();
  Point query( plow, 2 );
  MatchList lst;
  QgsPointLocator_VisitorVertexEdge visitor( mLayer, true, point, lst );
  mRTreeVertex->nearestNeighborQuery( maxMatches, query, visitor );
  return lst;
}


QgsPointLocator::MatchList QgsPointLocator::nearestEdges( const QgsPoint& point, int maxMatches )
{
  if ( !mRTreeEdge )
  {
    init( Edge );
    if ( !mRTreeEdge ) // still invalid?
      return MatchList();
  }

  MatchList lst;
  QgsPointLocator_VisitorVertexEdge visitor( mLayer, false, point, lst );
  EdgeNNComparator edgeNNC;
  mRTreeEdge->nearestNeighborQuery( maxMatches, point2point( point ), visitor, edgeNNC );
  return lst;
}


QgsPointLocator::MatchList QgsPointLocator::verticesInRect( const QgsRectangle& rect )
{
  if ( !mRTreeVertex )
  {
    init( Vertex );
    if ( !mRTreeVertex ) // still invalid?
      return MatchList();
  }

  MatchList lst;
  QgsPointLocator_VisitorVertexEdge visitor( mLayer, true, rect, lst );
  mRTreeVertex->intersectsWithQuery( rect2region( rect ), visitor );
  return lst;
}


QgsPointLocator::MatchList QgsPointLocator::edgesInRect( const QgsRectangle& rect )
{
  if ( !mRTreeEdge )
  {
    init( Edge );
    if ( !mRTreeEdge ) // still invalid?
      return MatchList();
  }

  MatchList lst;
  QgsPointLocator_VisitorVertexEdge visitor( mLayer, false, rect, lst );
  mRTreeEdge->intersectsWithQuery( rect2region( rect ), visitor );
  return lst;
}


QgsPointLocator::MatchList QgsPointLocator::pointInPolygon( const QgsPoint& point )
{
  if ( !mRTreeArea )
  {
    init( Area );
    if ( !mRTreeArea ) // still invalid?
      return MatchList();
  }

  MatchList lst;
  QgsPointLocator_VisitorArea visitor( mLayer, point, lst );
  mRTreeArea->intersectsWithQuery( point2point( point ), visitor );
  return lst;
}
