/***************************************************************************
                         qgsoverlayobject.h  -  description
                         ------------------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
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

#ifndef QGSOVERLAYOBJECT_H
#define QGSOVERLAYOBJECT_H

#include "qgsgeometry.h"
#include "qgspoint.h"
#include <QList>

class QgsGeometry;

/**An object that holds information about the position and bounding box size of
*    an overlay object. It stores a copy of the feature geometry as this information is commonly used
*    to calculate object placement
* \note This class has been added in version 1.1
*/
class CORE_EXPORT QgsOverlayObject
{
  public:
    QgsOverlayObject( int width = 0, int height = 0, double rotation = 0, QgsGeometry* geometry = 0 );
    virtual ~QgsOverlayObject();

    //copy constructor and assignment operator necessary because of mGeometry
    QgsOverlayObject( const QgsOverlayObject& other );
    QgsOverlayObject& operator=( const QgsOverlayObject& other );


    /**Returns the feature geometry in geos format. The calling function does _not_ take
     ownership of the generated object. The geometry is in map coordinates
     @deprecated Please use geometry() and QgsGeometry::asGeos instead*/
    GEOSGeometry* getGeosGeometry();
    /**Feature geometry is released when object is destructed so this function is empty.
     * @deprecated nop
     */
    void releaseGeosGeometry( GEOSGeometry *the_geom ) { Q_UNUSED( the_geom ); }

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
    by the edge of the view extent. It is also possible that there is no position (e.g. geometry too small). In
    that case*/
    QList<QgsPoint> mPositions;
    /**Rotation of the object*/
    double mRotation;
    /**Copy of the feature geometry. A copy is necessary because in QGIS geometries are deleted
    after drawing. The geometry is in map coordinates*/
    QgsGeometry* mGeometry;

};

#endif
