class QgsPALGeometry
{
%TypeHeaderCode
#include <qgspalgeometry.h>
%End
  public:
    /**Constructor that takes the geometry representation as geos object.
    Note that the class does not take ownership*/
    QgsPALGeometry( QgsOverlayObject* op );
    ~QgsPALGeometry();

    //methods inherited from PalGeometry
    // GEOSGeometry* getGeosGeometry();
    // void releaseGeosGeometry( GEOSGeometry *the_geom );

    /**Returns pointer to the overlay object this geometry referrs to. Don't delete the returned object!*/
    QgsOverlayObject* overlayObjectPtr() const;
};
