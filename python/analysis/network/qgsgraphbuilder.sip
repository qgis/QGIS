/**
* \ingroup networkanalysis
* \class QgsGraphBuilder
* \brief This class making the QgsGraph object
*/

class QgsGraphBuilder : QgsGraphBuilderInterface
{
%TypeHeaderCode
#include <qgsgraphbuilder.h>
%End

  public:
    /**
     * default constructor
     */
    QgsGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool otfEnabled = true, double topologyTolerance = 0.0, const QString& ellipsoidID = "WGS84" );

    ~QgsGraphBuilder();

    /*
     * MANDATORY BUILDER PROPERTY DECLARATION
     */
    virtual void addVertex( int id, const QgsPoint& pt );

    virtual void addArc( int pt1id, const QgsPoint& pt1, int pt2id, const QgsPoint& pt2, const QVector< QVariant >& prop );

    /**
     * return QgsGraph result;
     */
    QgsGraph* graph() /Factory/;
};
