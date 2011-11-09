/***************************************************************************
                        qgspalgeometry.h
                        ----------------
   begin                : May 2009
   copyright            : (C) 2009 by Marco Hugentobler
   email                : marco dot hugentobler at karto dot baug dot ethz dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPALGEOMETRY_H
#define QGSPALGEOMETRY_H

#include <qglobal.h>
#include "palgeometry.h"

class QgsOverlayObject;


/**A class to make the QGIS geometries known to the PAL library (PAL works on geos geometry). The main purpose of this class is to remove the visibility of PAL and geos classes in the python interface*/
class CORE_EXPORT QgsPALGeometry: public pal::PalGeometry
{
  public:
    /**Constructor that takes the geometry representation as geos object.
    Note that the class does not take ownership*/
    QgsPALGeometry( QgsOverlayObject* op );
    ~QgsPALGeometry();

    //methods inherited from PalGeometry
    GEOSGeometry* getGeosGeometry();
    void releaseGeosGeometry( GEOSGeometry *the_geom ) { Q_UNUSED( the_geom ); }

    /**Returns pointer to the overlay object this geometry referrs to. Don't delete the returned object!*/
    QgsOverlayObject* overlayObjectPtr() const { return mOverlayObjectPtr; }

  private:
    /**Default constructor forbidden*/
    QgsPALGeometry();
    /**Pointer to the related overlay object*/
    QgsOverlayObject* mOverlayObjectPtr;
};

#endif // QGSPALGEOMETRY_H
