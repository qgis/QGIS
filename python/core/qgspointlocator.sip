
class QgsPointLocator : QObject
{
%TypeHeaderCode
#include <qgspointlocator.h>
%End

  public:
    /** Construct point locator for a layer.
     *  @arg destCRS if not null, will do the searches on data reprojected to the given CRS
     *  @arg extent  if not null, will index only a subset of the layer
     */
    explicit QgsPointLocator( QgsVectorLayer* layer, const QgsCoordinateReferenceSystem* destCRS = 0, const QgsRectangle* extent = 0 );

    ~QgsPointLocator();

    enum Type { Invalid, Vertex, Edge, Area, All };

    /** Prepare the index for queries. Does nothing if the index already exists.
     * If the number of features is greater than the value of maxFeaturesToIndex, creation of index is stopped
     * to make sure we do not run out of memory. If maxFeaturesToIndex is -1, no limits are used. Returns
     * false if the creation of index has been prematurely stopped due to the limit of features, otherwise true */
    bool init( int maxFeaturesToIndex = -1 );

    /** Indicate whether the data have been already indexed */
    bool hasIndex() const;

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
      int vertexIndex() const;

      //! reference vector layer
      QgsVectorLayer* layer() const;

      QgsFeatureId featureId() const;

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

    // intersection queries

    //! Find nearest vertex to the specified point - up to distance specified by tolerance
    //! Optional filter may discard unwanted matches.
    Match nearestVertex( const QgsPoint& point, double tolerance, QgsPointLocator::MatchFilter* filter = 0 );
    //! Find nearest edges to the specified point - up to distance specified by tolerance
    //! Optional filter may discard unwanted matches.
    Match nearestEdge( const QgsPoint& point, double tolerance, QgsPointLocator::MatchFilter* filter = 0 );
    //! Find edges within a specified recangle
    //! Optional filter may discard unwanted matches.
    MatchList edgesInRect( const QgsRectangle& rect, QgsPointLocator::MatchFilter* filter = 0 );
    //! Override of edgesInRect that construct rectangle from a center point and tolerance
    MatchList edgesInRect( const QgsPoint& point, double tolerance, QgsPointLocator::MatchFilter* filter = 0 );

    // point-in-polygon query

    // TODO: function to return just the first match?
    //! find out if the point is in any polygons
    MatchList pointInPolygon( const QgsPoint& point );


  protected:
    bool rebuildIndex( int maxFeaturesToIndex = -1 );
    void destroyIndex();
};
