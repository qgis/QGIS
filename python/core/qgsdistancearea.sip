
class QgsDistanceArea
{
%TypeHeaderCode
#include <qgsdistancearea.h>
%End

  public:

    //! Constructor
    QgsDistanceArea();
    
    //! Destructor
    ~QgsDistanceArea();
    
    //! sets whether coordinates must be projected to ellipsoid before measuring
    void setProjectionsEnabled(bool flag);

    //! returns projections enabled flag
    bool hasCrsTransformEnabled();
    
    //! sets source spatial reference system (by QGIS CRS)
    void setSourceCrs(long srsid);
    
    //! sets source spatial reference system (by EpsgCrsId)
    void setSourceEpsgCrsId(long epsgId);

    //! returns source spatial reference system
    long sourceCrs();
    //! What sort of coordinate system is being used?
    bool geographic();

    //! sets ellipsoid by its acronym
    bool setEllipsoid(const QString& ellipsoid);
    
    //! returns ellipsoid's acronym
    const QString& ellipsoid();
    
    //! returns ellipsoid's semi major axis
    double ellipsoidSemiMajor();
    //! returns ellipsoid's semi minor axis
    double ellipsoidSemiMinor();
    //! returns ellipsoid's inverse flattening
    double ellipsoidInverseFlattening();
    
    //! general measurement (line distance or polygon area)
    double measure(QgsGeometry* geometry);
    
    //! measures line with more segments
    double measureLine(const QList<QgsPoint>& points);
    
    //! measures line with one segment
    double measureLine(const QgsPoint& p1, const QgsPoint& p2);
    
    //! measures polygon area
    double measurePolygon(const QList<QgsPoint>& points);
    
    //! compute bearing - in radians
    double bearing(const QgsPoint& p1, const QgsPoint& p2);

    static QString textUnit(double value, int decimals, QGis::UnitType u, bool isArea);    

};
