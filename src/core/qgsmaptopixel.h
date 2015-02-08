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
    * @deprecated in 2.8, use version with all parameters
    */
    Q_DECL_DEPRECATED QgsMapToPixel( double mapUnitsPerPixel, double height = 0, double ymin = 0, double xmin = 0 );

    /* Constructor
    * @param mapUnitsPerPixel Map units per pixel
    * @param xc X ordinate of map center, in geographical units
    * @param yc Y ordinate of map center, in geographical units
    * @param width Output width, in pixels
    * @param height Output height, in pixels
    * @param degrees clockwise rotation in degrees
    * @note added in 2.8
    */
    QgsMapToPixel( double mapUnitsPerPixel, double xc, double yc, int width, int height, double rotation );

    /*! Constructor
     *
     * Use setParameters to fill
     */
    QgsMapToPixel();

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
    //! The information is only known if setRotation was used
    //! @note added in 2.8
    int mapWidth() const;

    //! Return current map height in pixels
    //! @note added in 2.8
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
    // @deprecated in 2.8, use setParameters
    // @note this really sets the viewport height, not ymax
    Q_DECL_DEPRECATED void setYMaximum( double yMax ) { mHeight = yMax; }

    //! Set minimum y value
    // @deprecated in 2.8, use setParameters
    Q_DECL_DEPRECATED void setYMinimum( double ymin );

    //! set minimum x value
    // @deprecated in 2.8, use setParameters
    Q_DECL_DEPRECATED void setXMinimum( double xmin );

    /*! Set parameters for use in transforming coordinates
    * @param mapUnitsPerPixel Map units per pixel
    * @param xmin Minimum x value
    * @param ymin Minimum y value
    * @param height Map height, in pixels
    * @deprecated in 2.8, use the version with full parameters
    */
    Q_DECL_DEPRECATED void setParameters( double mapUnitsPerPixel, double xmin, double ymin, double height );

    /*! Set parameters for use in transforming coordinates
    * @param mapUnitsPerPixel Map units per pixel
    * @param xc X ordinate of map center, in geographical units
    * @param yc Y ordinate of map center, in geographical units
    * @param width Output width, in pixels
    * @param height Output height, in pixels
    * @param rotation clockwise rotation in degrees
    * @note added in 2.8
    */
    void setParameters( double mapUnitsPerPixel, double xc, double yc, int width, int height, double rotation );

    //! String representation of the parameters used in the transform
    QString showParameters() const;

  private:
    double mMapUnitsPerPixel;
    int mWidth;
    int mHeight;
    double mRotation;
    double xCenter;
    double yCenter;
    double xMin; // @deprecated in 2.8
    double yMin; // @deprecated in 2.8
    QTransform mMatrix;

    void updateMatrix();
};


#endif // QGSMAPTOPIXEL
