%ModuleHeaderCode
#include <qgsgraphbuilder.h>
%End

/**
* \ingroup networkanalysis
* \class QgsGraphBuilderInterface
* \brief Determine interface for creating a graph. Contains the settings of the graph. QgsGraphBuilder and QgsGraphDirector is a Builder pattern
*/
class QgsGraphBuilderInterface
{
%TypeHeaderCode
#include <qgsgraphbuilderintr.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast< QgsGraphBuilder* > ( sipCpp ) != NULL )
    sipClass = sipClass_QgsGraphBuilder;
  else
    sipClass = NULL;
%End

  public:
    /**
     * QgsGraphBuilderInterface constructor
     * @param crs Coordinate reference system for new graph vertex
     * @param ctfEnabled enable coordinate transform from source graph CRS to CRS graph 
     * @param topologyTolerance sqrt distance between source point as one graph vertex
     * @param ellipsoidID ellipsoid for edge measurement
     */  
    QgsGraphBuilderInterface( const QgsCoordinateReferenceSystem& crs, bool ctfEnabled = true, double topologyTolerance = 0.0, const QString& ellipsoidID = "WGS84" );

    QgsCoordinateReferenceSystem& destinationCrs();
   
    //! get coordinate transformation enabled
    bool coordinateTransformationEnabled();

    //! get topology tolerance
    double topologyTolerance();
    
    //! get measurement tool
    QgsDistanceArea* distanceArea();

    /**
     * add vertex
     * @param id vertex identyficator
     * @param pt vertex coordinate
     * @note id and pt is a redundant interface. You can use coordinates or id for vertex identyfy
     */
    virtual void addVertex( int id, const QgsPoint& pt );

    /**
     * add arc
     * @param pt1id first vertex identificator
     * @param pt1   first vertex coordinate
     * @param pt2id second vertex identificator
     * @param pt2   second vertex coordinate
     * @param properties arc properties
     * @note pt1id, pt1 and pt2id, pt2 is a redundant interface. You can use vertex coordinates or their identificators.
     */
    virtual void addArc( int pt1id, const QgsPoint& pt1, int pt2id, const QgsPoint& pt2, const QVector< QVariant >& properties );
  
};

