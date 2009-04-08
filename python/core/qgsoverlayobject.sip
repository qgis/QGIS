class CORE_EXPORT QgsOverlayObject: public pal::PalGeometry
{
%TypeHeaderCode
#include "qgsoverlayobject.h"
%End
  public:
    QgsOverlayObject( int width = 0, int height = 0, double rotation = 0, QgsGeometry* geometry = 0 );
    virtual ~QgsOverlayObject();

    //copy constructor and assignment operator necessary because of mGeometry
    QgsOverlayObject( const QgsOverlayObject& other );
    QgsOverlayObject& operator=( const QgsOverlayObject& other );


    /**Returns the feature geometry in geos format. The calling function does _not_ take
     ownership of the generated object*/
    GEOSGeometry* getGeosGeometry();
    /**Feature geometry is released when object is destructed so this function is empty*/
    void releaseGeosGeometry( GEOSGeometry *the_geom ) {}

    //getters
    int width() const {return mWidth;}
    int height() const {return mHeight;}
    double rotation() const {return mRotation;}
    QgsGeometry* geometry() {return mGeometry;}
    const QgsGeometry* geometry() const {return mGeometry;}
    QgsPoint position() const;
    QList<QgsPoint> positions() const {return mPositions;}

    //setters
    void setHeight( int height ) {mHeight = height;}
    void setWidth( int width ) {mWidth = width;}
    void setRotation( double rotation ) {mRotation = rotation;}
    /**Set geometry. This class takes ownership of the object*/
    void setGeometry( QgsGeometry* g );
    /**Adds a position in map coordinates*/
    void addPosition( const QgsPoint& position );


  private:

    /**Width of the bounding box in pixels*/
    int mWidth;
    /**Height of the bounding box in pixels*/
    int mHeight;
    /**Position of the object in map coordinates. Note that it is possible that an object
    has several positions, e.g. a multiobject or an object that is split into multiple parts
    by the edge of the view extent*/
    QList<QgsPoint> mPositions;
    /**Rotation of the object*/
    double mRotation;
    /**Copy of the feature geometry. A copy is necessary because in QGIS geometries are deleted
    after drawing*/
    QgsGeometry* mGeometry;

};
