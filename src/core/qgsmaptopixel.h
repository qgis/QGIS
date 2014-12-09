/***************************************************************************
                          qgsmaptopixel.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPTOPIXEL
#define QGSMAPTOPIXEL

#include "qgspoint.h"
#include <QTransform>
#include <vector>

#include <cassert>

class QgsPoint;
class QPoint;

/** \ingroup core
  * Perform transforms between map coordinates and device coordinates.
  *
  * This class can convert device coordinates to map coordinates and vice versa.
  */
class CORE_EXPORT QgsMapToPixel
{
  public:
    /* Constructor
    * @param mapUnitsPerPixel Map units per pixel
    * @param height Map canvas height, in pixels
    * @param ymin Minimum y value of the map canvas
    * @param xmin Minimum x value of the map canvas
    */
    QgsMapToPixel( double mapUnitsPerPixel = 0, double height = 0, double ymin = 0,
                   double xmin = 0 );
    //! destructor
    ~QgsMapToPixel();
    /*! Transform the point from map (world) coordinates to device coordinates
    * @param p Point to transform
    * @return QgsPoint in device coordinates
    */
    QgsPoint transform( const QgsPoint& p ) const;
    void transform( QgsPoint* p ) const;
    /*! Transform the point specified by x,y from map (world)
     * coordinates to device coordinates
     * @param x x cordinate o point to transform
     * @param y y coordinate of point to transform
     * @return QgsPoint in device coordinates
    */
    QgsPoint transform( double x, double y ) const;
    /*! Transform device coordinates to map (world) coordinates
    * @param x x coordinate of point to be converted to map cooordinates
    * @param y y coordinate of point to be converted to map cooordinates
    * @return QgsPoint in map coordinates
    */

    /* Transform device coordinates to map coordinates. Modifies the
       given coordinates in place. Intended as a fast way to do the
       transform. */
    void transformInPlace( qreal& x, qreal& y ) const;

    /* Transform device coordinates to map coordinates. Modifies the
       given coordinates in place. Intended as a fast way to do the
       transform.
       @note not available in python bindings
     */
    template <class T>
    void transformInPlace( QVector<T>& x, QVector<T>& y ) const
    {
      assert( x.size() == y.size() );
      for ( int i = 0; i < x.size(); ++i )
        transformInPlace( x[i], y[i] );
    }

    QgsPoint toMapCoordinates( int x, int y ) const;

    /*! Transform device coordinates to map (world) coordinates */
    QgsPoint toMapCoordinatesF( double x, double y ) const;

    /*! Transform device coordinates to map (world) coordinates
     * @param p Point to be converted to map cooordinates
     * @return QgsPoint in map coorndiates
     */
    QgsPoint toMapCoordinates( QPoint p ) const;

    QgsPoint toMapPoint( double x, double y ) const;

    /*! Set map units per pixel
    * @param mapUnitsPerPixel Map units per pixel
    */
    void setMapUnitsPerPixel( double mapUnitsPerPixel );

    //! Return current map units per pixel
    double mapUnitsPerPixel() const;

    //! Return current map width in pixels
    int mapWidth() const;

    //! Return current map height in pixels
    int mapHeight() const;

    //! Set map rotation in degrees (clockwise)
    //! @param degrees clockwise rotation in degrees
    //! @param cx X ordinate of map center in geographical units
    //! @param cy Y ordinate of map center in geographical units
    //! @note added in 2.8
    void setMapRotation( double degrees, double cx, double cy );

    //! Return current map rotation in degrees
    //! @note added in 2.8
    double mapRotation() const;

    //! Set maximum y value
    // @deprecated in 2.8, use setViewportHeight
    // @note this really sets the viewport height, not ymax
    void setYMaximum( double yMax ) { setViewportHeight( yMax ); }
    //! Set viewport height
    //! @note added in 2.8
    void setViewportHeight( double height );
    //! Set minimum y value
    void setYMinimum( double ymin );
    //! set minimum x value
    void setXMinimum( double xmin );
    /*! Set parameters for use in transforming coordinates
    * @param mapUnitsPerPixel Map units per pixel
    * @param xmin Minimum x value
    * @param ymin Minimum y value
    * @param height Map height, in pixels
    */
    void setParameters( double mapUnitsPerPixel, double xmin, double ymin, double height );
    //! String representation of the parameters used in the transform
    QString showParameters();

  private:
    double mMapUnitsPerPixel;
    double mHeight;
    double yMin;
    double xMin;
    //! Map rotation around Z axis on map center as clockwise degrees
    //! @note added in 2.8
    double mMapRotation;
    double xCenter;
    double yCenter;

    // Matrix to map from map (geographical) to screen (pixels) units
    QTransform getMatrix() const;
};


#endif // QGSMAPTOPIXEL
