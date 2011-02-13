class QgsOverlayObject
{
%TypeHeaderCode
#include "qgsoverlayobject.h"
%End
 public:
    QgsOverlayObject( int width = 0, int height = 0, double rotation = 0, QgsGeometry* geometry = 0 );
    virtual ~QgsOverlayObject();

    //copy constructor and assignment operator necessary because of mGeometry
    QgsOverlayObject( const QgsOverlayObject& other );


    //this function fill not be wrapped to not depend on geos python bindings
    /**Returns the feature geometry in geos format. The calling function does _not_ take
     ownership of the generated object. The geometry is in map coordinates
    @deprecated Please use geometry() and QgsGeometry::asGeos instead*/
    //GEOSGeometry* getGeosGeometry();
    /**Feature geometry is released when object is destructed so this function is empty.
    @deprecated nop */
    //void releaseGeosGeometry( GEOSGeometry *the_geom )

    //getters
    int width() const;
    int height() const;
    double rotation() const;
    QgsGeometry* geometry();
    //const QgsGeometry* geometry() const;
    QgsPoint position() const;
    QList<QgsPoint> positions() const;

    //setters
    void setHeight( int height );
    void setWidth( int width );
    void setRotation( double rotation );
    /**Set geometry. This class takes ownership of the object*/
    void setGeometry( QgsGeometry* g );
    /**Adds a position in map coordinates*/
    void addPosition( const QgsPoint& position );
};
