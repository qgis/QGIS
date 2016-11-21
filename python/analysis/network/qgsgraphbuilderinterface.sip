%ModuleHeaderCode
#include <qgsgraphbuilder.h>
%End

/**
* \ingroup analysis
* \class QgsGraphBuilderInterface
* \brief Determine interface for creating a graph. Contains the settings of the graph.
* QgsGraphBuilder and QgsGraphDirector both use a "builder" design pattern
*/
class QgsGraphBuilderInterface
{
%TypeHeaderCode
#include <qgsgraphbuilderinterface.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsGraphBuilder* > ( sipCpp ) != NULL )
    sipType = sipType_QgsGraphBuilder;
  else
    sipType = NULL;
%End

  public:
    /**
     * Default constructor
     * @param crs Coordinate reference system for new graph vertex
     * @param ctfEnabled enable coordinate transform from source graph CRS to CRS graph
     * @param topologyTolerance sqrt distance between source point as one graph vertex
     * @param ellipsoidID ellipsoid for edge measurement
     */
    QgsGraphBuilderInterface( const QgsCoordinateReferenceSystem& crs, bool ctfEnabled = true, double topologyTolerance = 0.0, const QString& ellipsoidID = "WGS84" );

    //! Destructor
    virtual ~QgsGraphBuilderInterface();

    //! Returns destinaltion CRS
    QgsCoordinateReferenceSystem destinationCrs() const;

    //! Returns coordinate transformation enabled
    bool coordinateTransformationEnabled();

    //! Returns topology tolerance
    double topologyTolerance();

    //! Returns measurement tool
    QgsDistanceArea* distanceArea();

    /**
     * Add vertex to the graph
     * @param id vertex identifier
     * @param pt vertex coordinates
     * @note id and pt are redundant. You can use pt or id to identify the vertex
     */
    virtual void addVertex( int id, const QgsPoint &pt );

    /**
     * Add edge to the graph
     * @param pt1id first vertex identificator
     * @param pt1   first vertex coordinates
     * @param pt2id second vertex identificator
     * @param pt2   second vertex coordinates
     * @param strategies optimization strategies
     * @note pt1id, pt1 and pt2id, pt2 is a redundant interface. You can use vertex coordinates or their identificators.
     */
    virtual void addEdge( int pt1id, const QgsPoint& pt1, int pt2id, const QgsPoint& pt2, const QVector< QVariant >& strategies );
};
