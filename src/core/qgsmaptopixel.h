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
#include "qgspointxy.h"

#include <cassert>
#include <memory>

class QPoint;

/**
 * \ingroup core
  * \brief Perform transforms between map coordinates and device coordinates.
  *
  * This class can convert device coordinates to map coordinates and vice versa.
  */
class CORE_EXPORT QgsMapToPixel
{
  public:

    /**
     * Constructor for an invalid QgsMapToPixel.
     *
     * A manual call to setParameters() is required to initialize the object.
     */
    QgsMapToPixel();

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
     * Returns TRUE if the object is valid (i.e. it has parameters set), or FALSE if the object is default constructed
     * with no parameters set.
     *
     * \since QGIS 3.22
     */
    bool isValid() const { return mValid; }

    /**
     * Transforms a point \a p from map (world) coordinates to device coordinates.
     * \param p Point to transform
     * \returns QgsPointXY in device coordinates
     */
    QgsPointXY transform( const QgsPointXY &p ) const
    {
      qreal x = p.x();
      qreal y = p.y();
      transformInPlace( x, y );
      return QgsPointXY( x, y );
    }

    /**
     * Transforms a point \a p from map (world) coordinates to device coordinates in place.
     */
    void transform( QgsPointXY *p ) const
    {
      qreal x = p->x();
      qreal y = p->y();
      transformInPlace( x, y );
      p->set( x, y );
    }

    /**
     * Transforms the point specified by x,y from map (world) coordinates to device coordinates.
     *
     * \param x x coordinate of point to transform
     * \param y y coordinate of point to transform
     * \returns QgsPointXY in device coordinates
     */
    QgsPointXY transform( qreal x, qreal y ) const
    {
      transformInPlace( x, y );
      return QgsPointXY( x, y );
    }

    /**
     * Transforms device coordinates to map coordinates.
     *
     * This method modifies the given coordinates in place. It is intended as a fast way to do the
     * transform.
     */
    void transformInPlace( double &x, double &y ) const
    {
      qreal mx, my;
      mMatrix.map( static_cast< qreal >( x ), static_cast< qreal >( y ), &mx, &my );
      x = mx;
      y = my;
    }

    /**
     * Transforms device coordinates to map coordinates.
     *
     * This method modifies the given coordinates in place. It is intended as a fast way to do the
     * transform.
     *
     * \note Not available in Python bindings
     */
    void transformInPlace( float &x, float &y ) const SIP_SKIP
    {
      double mx = x, my = y;
      transformInPlace( mx, my );
      x = mx;
      y = my;
    }

#ifndef SIP_RUN

    /**
     * Transforms device coordinates to map coordinates.
     *
     * This method modifies the given coordinates in place. It is intended as a fast way to do the
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

    /**
     * Transforms device coordinates to map (world) coordinates.
     */
    QgsPointXY toMapCoordinates( int x, int y ) const
    {
      return toMapCoordinates( static_cast<double>( x ), static_cast<double>( y ) );
    }

    /**
     * Transforms device coordinates to map (world) coordinates.
     */
    QgsPointXY toMapCoordinates( double x, double y ) const SIP_PYNAME( toMapCoordinatesF )
    {
      bool invertible;
      const QTransform matrix = mMatrix.inverted( &invertible );
      assert( invertible );
      qreal mx, my;
      matrix.map( static_cast< qreal >( x ), static_cast< qreal >( y ), &mx, &my );
      return QgsPointXY( mx, my );
    }

    /**
     * Transforms device coordinates to map (world) coordinates.
     *
     * \param p Point to be converted to map cooordinates
     * \returns QgsPointXY in map coorndiates
     */
    QgsPointXY toMapCoordinates( QPoint p ) const
    {
      const QgsPointXY mapPt = toMapCoordinates( static_cast<double>( p.x() ), static_cast<double>( p.y() ) );
      return QgsPointXY( mapPt );
    }

    /**
     * Transforms device coordinates to map (world) coordinates.
     *
     * \deprecated since QGIS 3.4 use toMapCoordinates instead
     */
    Q_DECL_DEPRECATED QgsPointXY toMapPoint( double x, double y ) const SIP_DEPRECATED
    {
      return toMapCoordinates( x, y );
    }

    /**
     * Sets the map units per pixel.
     *
     * Calling this method will automatically set the object as valid.
     *
     * \param mapUnitsPerPixel Map units per pixel
     *
     * \see mapUnitsPerPixel()
     */
    void setMapUnitsPerPixel( double mapUnitsPerPixel );

    /**
     * Returns the current map units per pixel.
     *
     * \see setMapUnitsPerPixel()
     */
    double mapUnitsPerPixel() const { return mMapUnitsPerPixel; }

    /**
     * Returns the current map width in pixels.
     *
     * The information is only known if setRotation was used.
     *
     * \see mapHeight()
     * \since QGIS 2.8
     */
    int mapWidth() const { return mWidth; }

    /**
     * Returns current map height in pixels
     *
     * \see mapWidth()
     * \since QGIS 2.8
     */
    int mapHeight() const { return mHeight; }

    /**
     * Sets map rotation in \a degrees (clockwise).
     *
     * Calling this method will automatically set the object as valid.
     *
     * \param degrees clockwise rotation in degrees
     * \param cx X ordinate of map center in geographical units
     * \param cy Y ordinate of map center in geographical units
     *
     * \see mapRotation()
     * \since QGIS 2.8
     */
    void setMapRotation( double degrees, double cx, double cy );

    /**
     * Returns the current map rotation in degrees (clockwise).
     *
     * \see setMapRotation()
     * \since QGIS 2.8
     */
    double mapRotation() const { return mRotation; }

    /**
     * Sets parameters for use in transforming coordinates.
     *
     * Calling this method will automatically set the object as valid.
     *
     * \param mapUnitsPerPixel Map units per pixel
     * \param centerX X coordinate of map center, in geographical units
     * \param centerY Y coordinate of map center, in geographical units
     * \param widthPixels Output width, in pixels
     * \param heightPixels Output height, in pixels
     * \param rotation clockwise rotation in degrees
     *
     * \note if the specified parameters result in an invalid transform then no changes will be applied to the object
     * \since QGIS 2.8
     */
    void setParameters( double mapUnitsPerPixel, double centerX, double centerY, int widthPixels, int heightPixels, double rotation );

    /**
     * Sets parameters for use in transforming coordinates.
     *
     * Calling this method will automatically set the object as valid.
     *
     * \param mapUnitsPerPixel Map units per pixel
     * \param centerX X coordinate of map center, in geographical units
     * \param centerY Y coordinate of map center, in geographical units
     * \param widthPixels Output width, in pixels
     * \param heightPixels Output height, in pixels
     * \param rotation clockwise rotation in degrees
     * \param ok will be set to TRUE if the specified parameters result in a valid transform, otherwise the changes are ignored and ok will be set to FALSE.
     *
     * \since QGIS 3.20
     */
    void setParameters( double mapUnitsPerPixel, double centerX, double centerY, int widthPixels, int heightPixels, double rotation, bool *ok ) SIP_SKIP;

    /**
     * Returns a string representation of the parameters used in the transform.
     */
    QString showParameters() const;

    /**
     * Returns a QTransform encapsulating the map to pixel conversion.
     */
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

    bool operator==( const QgsMapToPixel &other ) const
    {
      return mValid == other.mValid
             && mMapUnitsPerPixel == other.mMapUnitsPerPixel
             && mWidth == other.mWidth
             && mHeight == other.mHeight
             && mRotation == other.mRotation
             && mXCenter == other.mXCenter
             && mYCenter == other.mYCenter
             && mXMin == other.mXMin
             && mYMin == other.mYMin;
    }

    bool operator!=( const QgsMapToPixel &other ) const
    {
      return !( *this == other );
    }

  private:
    bool mValid = false;
    double mMapUnitsPerPixel = 1;
    int mWidth = 1;
    int mHeight = 1;
    double mRotation = 0.0;
    double mXCenter = 0.5;
    double mYCenter = 0.5;
    double mXMin = 0;
    double mYMin = 0;
    QTransform mMatrix;

    bool updateMatrix();
};


#endif // QGSMAPTOPIXEL
