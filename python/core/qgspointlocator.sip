/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspointlocator.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsPointLocator : QObject
{
%Docstring
 The class defines interface for querying point location:
  - query nearest vertices / edges to a point
  - query vertices / edges in rectangle
  - query areas covering a point

 Works with one layer.

.. versionadded:: 2.8
%End

%TypeHeaderCode
#include "qgspointlocator.h"
%End
  public:

    explicit QgsPointLocator( QgsVectorLayer *layer, const QgsCoordinateReferenceSystem &destinationCrs = QgsCoordinateReferenceSystem(),
                              const QgsRectangle *extent = 0 );
%Docstring
 Construct point locator for a ``layer``.

  If a valid QgsCoordinateReferenceSystem is passed for ``destinationCrs`` then the locator will
  do the searches on data reprojected to the given CRS.

  If ``extent`` is not null, the locator will index only a subset of the layer which falls within that extent.
%End

    ~QgsPointLocator();

    QgsVectorLayer *layer() const;
%Docstring
 Get associated layer
.. versionadded:: 2.14
 :rtype: QgsVectorLayer
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
 Get destination CRS - may be an invalid QgsCoordinateReferenceSystem if not doing OTF reprojection
.. versionadded:: 2.14
 :rtype: QgsCoordinateReferenceSystem
%End

    const QgsRectangle *extent() const;
%Docstring
 Get extent of the area point locator covers - if null then it caches the whole layer
.. versionadded:: 2.14
 :rtype: QgsRectangle
%End

    void setExtent( const QgsRectangle *extent );
%Docstring
 Configure extent - if not null, it will index only that area
.. versionadded:: 2.14
%End

    enum Type
    {
      Invalid,
      Vertex,
      Edge,
      Area,
      All
    };

    typedef QFlags<QgsPointLocator::Type> Types;


    bool init( int maxFeaturesToIndex = -1 );
%Docstring
 Prepare the index for queries. Does nothing if the index already exists.
 If the number of features is greater than the value of maxFeaturesToIndex, creation of index is stopped
 to make sure we do not run out of memory. If maxFeaturesToIndex is -1, no limits are used. Returns
 false if the creation of index has been prematurely stopped due to the limit of features, otherwise true *
 :rtype: bool
%End

    bool hasIndex() const;
%Docstring
Indicate whether the data have been already indexed
 :rtype: bool
%End

    struct Match
    {
        Match();
%Docstring
construct invalid match
%End

        Match( QgsPointLocator::Type t, QgsVectorLayer *vl, QgsFeatureId fid, double dist, const QgsPointXY &pt, int vertexIndex = 0, QgsPointXY *edgePoints = 0 );

        QgsPointLocator::Type type() const;
%Docstring
 :rtype: QgsPointLocator.Type
%End

        bool isValid() const;
%Docstring
 :rtype: bool
%End
        bool hasVertex() const;
%Docstring
 :rtype: bool
%End
        bool hasEdge() const;
%Docstring
 :rtype: bool
%End
        bool hasArea() const;
%Docstring
 :rtype: bool
%End

        double distance() const;
%Docstring
 for vertex / edge match
 units depending on what class returns it (geom.cache: layer units, map canvas snapper: dest crs units)
 :rtype: float
%End

        QgsPointXY point() const;
%Docstring
 for vertex / edge match
 coords depending on what class returns it (geom.cache: layer coords, map canvas snapper: dest coords)
 :rtype: QgsPointXY
%End

        int vertexIndex() const;
%Docstring
for vertex / edge match (first vertex of the edge)
 :rtype: int
%End

        QgsVectorLayer *layer() const;
%Docstring
 The vector layer where the snap occurred.
 Will be null if the snap happened on an intersection.
 :rtype: QgsVectorLayer
%End

        QgsFeatureId featureId() const;
%Docstring
 The id of the feature to which the snapped geometry belongs.
 :rtype: QgsFeatureId
%End

        void edgePoints( QgsPointXY &pt1 /Out/, QgsPointXY &pt2 /Out/ ) const;
%Docstring
Only for a valid edge match - obtain endpoints of the edge
%End

        bool operator==( const QgsPointLocator::Match &other ) const;

      protected:
    };

    typedef QList<QgsPointLocator::Match> MatchList;

    struct MatchFilter
    {
      virtual ~MatchFilter();
      virtual bool acceptMatch( const QgsPointLocator::Match &match ) = 0;
%Docstring
 :rtype: bool
%End
    };


    Match nearestVertex( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
 Find nearest vertex to the specified point - up to distance specified by tolerance
 Optional filter may discard unwanted matches.
 :rtype: Match
%End

    Match nearestEdge( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
 Find nearest edge to the specified point - up to distance specified by tolerance
 Optional filter may discard unwanted matches.
 :rtype: Match
%End

    MatchList edgesInRect( const QgsRectangle &rect, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
 Find edges within a specified recangle
 Optional filter may discard unwanted matches.
 :rtype: MatchList
%End
    MatchList edgesInRect( const QgsPointXY &point, double tolerance, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
Override of edgesInRect that construct rectangle from a center point and tolerance
 :rtype: MatchList
%End


    MatchList pointInPolygon( const QgsPointXY &point );
%Docstring
find out if the point is in any polygons
 :rtype: MatchList
%End


    int cachedGeometryCount() const;
%Docstring
 Return how many geometries are cached in the index
.. versionadded:: 2.14
 :rtype: int
%End

  protected:
    bool rebuildIndex( int maxFeaturesToIndex = -1 );
%Docstring
 :rtype: bool
%End
  protected slots:
    void destroyIndex();
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspointlocator.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
