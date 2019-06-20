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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QTransform>
#include <vector>
#include "qgsunittypes.h"
#include <cassert>
#include <memory>

class QgsPointXY;
class QPoint;

/**
 * \ingroup core
  * Perform transforms between map coordinates and device coordinates.
  *
  * This class can convert device coordinates to map coordinates and vice versa.
  */
class CORE_EXPORT QgsMapToPixel
{
  public:

    /**
     * Constructor
     * \param mapUnitsPerPixel Map units per pixel
     * \param centerX X coordinate of map center, in geographical units
     * \param centerY Y coordinate of map center, in geographical units
     * \param widthPixels Output width, in pixels
     * \param heightPixels Output height, in pixels
     * \param rotation clockwise rotation in degrees
     * \since QGIS 2.8
     */
    QgsMapToPixel( double mapUnitsPerPixel, double centerX, double centerY, int widthPixels, int heightPixels, double rotation );

    /**
     * Constructor
     * \param mapUnitsPerPixel Map units per pixel
     */
    QgsMapToPixel( double mapUnitsPerPixel );

    /**
     * Returns a new QgsMapToPixel created using a specified \a scale and distance unit.
     * \param scale map scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \param dpi screen DPI
     * \param mapUnits map units
     * \returns matching QgsMapToPixel
     * \since QGIS 3.0
     */
    static QgsMapToPixel fromScale( double scale, QgsUnitTypes::DistanceUnit mapUnits, double dpi = 96 );

    /**
     * Constructor
     *
     * Use setParameters to fill
     */
    QgsMapToPixel();

    /**
     * Transform the point from map (world) coordinates to device coordinates
     * \param p Point to transform
     * \returns QgsPointXY in device coordinates
     */
    QgsPointXY transform( const QgsPointXY &p ) const;

    void transform( QgsPointXY *p ) const;

    /**
     * Transform the point specified by x,y from map (world)
     * coordinates to device coordinates
     * \param x x cordinate o point to transform
     * \param y y coordinate of point to transform
     * \returns QgsPointXY in device coordinates
     */
    QgsPointXY transform( qreal x, qreal y ) const;

    /**
     * Transform device coordinates to map coordinates. Modifies the
     * given coordinates in place. Intended as a fast way to do the
     * transform.
     */
    void transformInPlace( double &x, double &y ) const;

    //! \note not available in Python bindings
    void transformInPlace( float &x, float &y ) const SIP_SKIP;

#ifndef SIP_RUN

    /**
     * Transform device coordinates to map coordinates. Modifies the
     * given coordinates in place. Intended as a fast way to do the
     * transform.
     * \note not available in Python bindings
     */
    template <class T> SIP_SKIP
    void transformInPlace( QVector<T> &x, QVector<T> &y ) const
    {
      assert( x.size() == y.size() );
      for ( int i = 0; i < x.size(); ++i )
        transformInPlace( x[i], y[i] );
    }
#endif

    //! Transform device coordinates to map (world) coordinates
    QgsPointXY toMapCoordinates( int x, int y ) const;

    //! Transform device coordinates to map (world) coordinates
    QgsPointXY toMapCoordinates( double x, double y ) const SIP_PYNAME( toMapCoordinatesF );

    /**
     * Transform device coordinates to map (world) coordinates
     * \param p Point to be converted to map cooordinates
     * \returns QgsPointXY in map coorndiates
     */
    QgsPointXY toMapCoordinates( QPoint p ) const;

    /**
     * Transform device coordinates to map (world) coordinates
     * \deprecated since QGIS 3.4 use toMapCoordinates instead
     */
    Q_DECL_DEPRECATED QgsPointXY toMapPoint( double x, double y ) const SIP_DEPRECATED;

    /**
     * Set map units per pixel
     * \param mapUnitsPerPixel Map units per pixel
     */
    void setMapUnitsPerPixel( double mapUnitsPerPixel );

    //! Returns current map units per pixel
    double mapUnitsPerPixel() const;

    /**
     * Returns current map width in pixels
     * The information is only known if setRotation was used
     * \since QGIS 2.8
     */
    int mapWidth() const;

    /**
     * Returns current map height in pixels
     * \since QGIS 2.8
     */
    int mapHeight() const;

    /**
     * Set map rotation in degrees (clockwise)
     * \param degrees clockwise rotation in degrees
     * \param cx X ordinate of map center in geographical units
     * \param cy Y ordinate of map center in geographical units
     * \since QGIS 2.8
     */
    void setMapRotation( double degrees, double cx, double cy );

    /**
     * Returns current map rotation in degrees
     * \since QGIS 2.8
     */
    double mapRotation() const;

    /**
     * Set parameters for use in transforming coordinates
     * \param mapUnitsPerPixel Map units per pixel
     * \param centerX X coordinate of map center, in geographical units
     * \param centerY Y coordinate of map center, in geographical units
     * \param widthPixels Output width, in pixels
     * \param heightPixels Output height, in pixels
     * \param rotation clockwise rotation in degrees
     * \since QGIS 2.8
     */
    void setParameters( double mapUnitsPerPixel, double centerX, double centerY, int widthPixels, int heightPixels, double rotation );

    //! String representation of the parameters used in the transform
    QString showParameters() const;

    QTransform transform() const;

    /**
     * Returns the center x-coordinate for the transform.
     * \see yCenter()
     * \since QGIS 3.0
     */
    double xCenter() const { return mXCenter; }

    /**
     * Returns the center y-coordinate for the transform.
     * \see xCenter()
     * \since QGIS 3.0
     */
    double yCenter() const { return mYCenter; }

  private:
    double mMapUnitsPerPixel = 1;
    int mWidth = 1;
    int mHeight = 1;
    double mRotation = 0.0;
    double mXCenter = 0.5;
    double mYCenter = 0.5;
    double xMin = 0;
    double yMin = 0;
    QTransform mMatrix;

    bool updateMatrix();
};


#endif // QGSMAPTOPIXEL
