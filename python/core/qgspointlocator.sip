
class QgsPointLocator : QObject
{
%TypeHeaderCode
#include <qgspointlocator.h>
%End

  public:
    explicit QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS = 0 );

    ~QgsPointLocator();

    enum Type { Invalid, Vertex, Edge, Area, All };

    /** Prepare the indexes for given or-ed combination of query types (Vertex, Edge, Area).
     *  If not initialized explicitly, index of particular type will be inited when first such query is issued.
     */
    void init( int types = All, bool force = false );

    //! check whether index for given query type exists
    bool hasIndex( Type t ) const;

    struct Match
    {
      //! consruct invalid match
      Match();

      Match( QgsPointLocator::Type t, QgsVectorLayer* vl, QgsFeatureId fid, double dist, const QgsPoint& pt, int vertexIndex = 0 );

      QgsPointLocator::Type type() const;

      bool isValid() const;
      bool hasVertex() const;
      bool hasEdge() const;
      bool hasArea() const;

      //! for vertex / edge match
      //! units depending on what class returns it (geom.cache: layer units, map canvas snapper: dest crs units)
      double distance() const;

      //! for vertex / edge match
      //! coords depending on what class returns it (geom.cache: layer coords, map canvas snapper: dest coords)
      QgsPoint point() const;

      //! for vertex / edge match (first vertex of the edge)
      int vertexIndex();

      //! reference vector layer
      QgsVectorLayer* layer();

      QgsFeatureId featureId();

      void replaceIfBetter( const QgsPointLocator::Match& m, double maxDistance );

      //! Only for a valid edge match - obtain endpoints of the edge
      void edgePoints( QgsPoint& pt1 /Out/, QgsPoint& pt2 /Out/ ) const;
    };

    typedef QList<QgsPointLocator::Match> MatchList;

    //! Interface that allows rejection of some matches in intersection queries
    //! (e.g. a match can only belong to a particular feature / match must not be a particular point).
    //! Implement the interface and pass its instance to QgsPointLocator or QgsSnappingUtils methods.
    struct MatchFilter
    {
      virtual bool acceptMatch( const QgsPointLocator::Match& match ) = 0;
    };

    // 1-NN queries

    //! find nearest vertex to the specified point
    QgsPointLocator::Match nearestVertex( const QgsPoint& point );

    //! find nearest edge to the specified point
    QgsPointLocator::Match nearestEdge( const QgsPoint& point );

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
    MatchList verticesInTolerance( const QgsPoint& point, double tolerance, QgsPointLocator::MatchFilter* filter = 0 );
    //! Find nearest edges to the specified point - sorted by distance.
    //! Will return matches up to distance given by tolerance.
    //! Optional filter may discard unwanted matches.
    MatchList edgesInTolerance( const QgsPoint& point, double tolerance, QgsPointLocator::MatchFilter* filter = 0 );

    //! Find vertices within given rectangle.
    //! If distToPoint is given, the matches will be sorted by distance to that point.
    //! Optional filter may discard unwanted matches.
    MatchList verticesInRect( const QgsRectangle& rect, const QgsPoint* distToPoint = 0, QgsPointLocator::MatchFilter* filter = 0 );
    //! Find edges within given rectangle.
    //! If distToPoint is given, the matches will be sorted by distance to that point.
    //! Optional filter may discard unwanted matches.
    MatchList edgesInRect( const QgsRectangle& rect, const QgsPoint* distToPoint = 0, QgsPointLocator::MatchFilter* filter = 0 );

    // point-in-polygon query

    // TODO: function to return just the first match?
    //! find out if the point is in any polygons
    MatchList pointInPolygon( const QgsPoint& point );


};
