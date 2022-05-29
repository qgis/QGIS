/***************************************************************************
                          qgsclipper.h  -  a class that clips line
                          segments and polygons
                             -------------------
    begin                : March 2004
    copyright            : (C) 2005 by Gavin Macaulay
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLIPPER_H
#define QGSCLIPPER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"

#include <QVector>
#include <QPolygonF>

#include "qgsbox3d.h"
#include "qgsabstractgeometry.h"

class QgsCurve;
class QgsLineString;

SIP_FEATURE( ARM ) // Some parts are not available in sip bindings on ARM because of qreal double vs. float issues


/**
 * \ingroup core
 * \brief A class to trim lines and polygons to within a rectangular region.
 *
 * The functions in this class are likely to be called from within a
 * render loop and hence need to as CPU efficient as possible.
 * The main purpose of the functions in this class are to trim lines
 *  and polygons to lie within a rectangular region. This is necessary
 *  for drawing items to an X11 display which have a limit on the
 *   magnitude of the screen coordinates (+/- 32768, i.e. 16 bit integer).
 */
class CORE_EXPORT QgsClipper
{
  public:

    // These are the limits for X11 screen coordinates. The actual
    // values are +/-32767, but we allow a little bit of space for
    // rounding errors.

    // You may wonder why the clipping is done to these coordinates
    // rather than the boundaries of the qgis canvas. Reasons include:
    // - making the boundaries static const allows the compiler to
    //   optimise the code that uses these values more than if they changed
    //   for every call to the trim code.
    // - clipping takes quite a bit of CPU effort, and the less that this is
    //   done the better. More stuff would have to be clipped if the
    //   boundaries were the qgis canvas (but this may be offset by
    //   having less to draw).
    //
    // The limit is set to 30,000 instead of 32768 because that things
    // still go wrong.

    //! Maximum X-coordinate of the rectangular box used for clipping.
    static const double MAX_X;
    //! Minimum X-coordinate of the rectangular box used for clipping.
    static const double MIN_X;
    //! Maximum Y-coordinate of the rectangular box used for clipping.
    static const double MAX_Y;
    //! Minimum Y-coordinate of the rectangular box used for clipping.
    static const double MIN_Y;


    //! A handy way to refer to the four boundaries
    enum Boundary
    {
      XMax,
      XMin,
      YMax,
      YMin,
      ZMax, //!< Maximum Z (since QGIS 3.26)
      ZMin, //!< Minimum Z (since QGIS 3.26)
    };

    SIP_IF_FEATURE( !ARM ) // Not available on ARM sip bindings because of qreal issues

    /**
     * Trims the given feature to a rectangular box. Returns the trimmed
     * feature in x and y. The shapeOpen parameter determines whether
     * the function treats the points as a closed shape (polygon), or as
     * an open shape (linestring).
     *
     * \note not available in Python bindings on android
     */
    static void trimFeature( QVector<double> &x,
                             QVector<double> &y,
                             bool shapeOpen );

    SIP_END

    /**
     * Trims the given polygon to a rectangular box, by modifying the given polygon in place.
     *
     * \param pts polygon as 2D coordinates
     * \param clipRect clipping rectangle
     */
    static void trimPolygon( QPolygonF &pts, const QgsRectangle &clipRect );

    /**
     * Trims a polygon consisting of the specified \a x, \a y and \a z values to a pseudo 3D box, by modifying the arrays in place.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static void trimPolygon( QVector< double > &x, QVector< double > &y, QVector< double> &z, const QgsBox3d &clipRect ) SIP_SKIP;

    /**
     * Takes a line with 3D coordinates and clips it to clipExtent.
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static void clipped3dLine( const QVector< double > &xIn, const QVector< double > &yIn, const QVector<double> &zIn, QVector< double > &x, QVector< double > &y, QVector< double > &z, const QgsBox3d &clipExtent ) SIP_SKIP;

    /**
     * Takes a linestring and clips it to clipExtent
     * \param curve the linestring
     * \param clipExtent clipping bounds
     * \returns clipped line coordinates
     */
    static QPolygonF clippedLine( const QgsCurve &curve, const QgsRectangle &clipExtent );

    /**
     * Takes a 2D \a curve and clips it to clipExtent.
     * \param curve the linestring
     * \param clipExtent clipping bounds
     * \returns clipped line coordinates
     * \since QGIS 3.16
     */
    static QPolygonF clippedLine( const QPolygonF &curve, const QgsRectangle &clipExtent );

    /**
     * Clips a line segment to a rectangle.
     *
     * An implementation of the 'Fast clipping' algorithm (Sobkow et al. 1987, Computers & Graphics Vol.11, 4, p.459-467).
     *
     * \param left x-coordinate of left side of rectangle
     * \param right x-coordinate of right side of rectangle
     * \param bottom y-coordinate of bottom side of rectangle
     * \param top y-coordinate of top side of rectangle
     * \param x0 x-coordinate of start of line
     * \param y0 y-coordinate of start of line
     * \param x1 x-coordinate of end of line
     * \param y1 y-coordinate of end of line
     * \returns TRUE if line was clipped.
     *
     * \since QGIS 3.26
     */
    static bool clipLineSegment( double left, double right, double bottom, double top, double &x0 SIP_INOUT, double &y0 SIP_INOUT, double &x1 SIP_INOUT, double &y1 SIP_INOUT );

  private:

    // Used when testing for equivalence to 0.0
    static const double SMALL_NUM;

    // Trims the given feature to the given boundary. Returns the
    // trimmed feature in the outX and outY vectors.
    static inline void trimFeatureToBoundary( const QVector<double> &inX,
        const QVector<double> &inY,
        QVector<double> &outX,
        QVector<double> &outY,
        Boundary b,
        bool shapeOpen );

    static inline void trimPolygonToBoundary( const QPolygonF &inPts, QPolygonF &outPts, const QgsRectangle &rect, Boundary b, double boundaryValue );

    static inline void trimPolygonToBoundary( const QVector<double> &inX, const QVector<double> &inY, const QVector< double > &inZ,
        QVector<double> &outX, QVector<double> &outY, QVector<double> &outZ, const QgsBox3d &rect, Boundary boundary, double boundaryValue );

    // Determines if a point is inside or outside the given boundary
    static inline bool inside( double x, double y, Boundary b );

    static inline bool inside( QPointF pt, Boundary b, double val );

    static inline bool inside( double x, double y, double z, Boundary boundary, double boundaryValue );

    // Calculates the intersection point between a line defined by a
    // (x1, y1), and (x2, y2) and the given boundary
    static inline QgsPointXY intersect( double x1, double y1,
                                        double x2, double y2,
                                        Boundary b );

    static inline QPointF intersectRect( QPointF pt1,
                                         QPointF pt2,
                                         Boundary b, const QgsRectangle &rect );

    static inline void intersectRect( double x1, double y1, double z1, double x2, double y2, double z2,
                                      double &xOut, double &yOut, double &zOut,
                                      Boundary boundary, const QgsBox3d &rect );

    static bool clipLineSegment( const QgsBox3d &extent, double &x0, double &y0, double &z0, double &x1, double &y1, double &z1 );

    /**
     * Connects two lines split by the clip (by inserting points on the clip border)
     * \param x0 x-coordinate of the first line end
     * \param y0 y-coordinate of the first line end
     * \param x1 x-coordinate of the second line start
     * \param y1 y-coordinate of the second line start
     * \param clipRect clip rectangle
     * \param pts: in/out array of clipped points
      */
    static void connectSeparatedLines( double x0, double y0, double x1, double y1,
                                       const QgsRectangle &clipRect, QPolygonF &pts );

    /**
     * Connects two 3D lines split by the clip (by inserting points on the clip border)
     * \param x0 x-coordinate of the first line end
     * \param y0 y-coordinate of the first line end
     * \param z0 z-coordinate of the first line end
     * \param x1 x-coordinate of the second line start
     * \param y1 y-coordinate of the second line start
     * \param z1 z-coordinate of the second line start
     * \param clipRect clip box
     * \param ptsX: in/out array of clipped points x
     * \param ptsY: in/out array of clipped points y
     * \param ptsZ: in/out array of clipped points z
      */
    static void connectSeparatedLines( double x0, double y0, double z0, double x1, double y1, double z1,
                                       const QgsBox3d &clipRect, QVector< double > &ptsX, QVector< double > &ptsY, QVector<double> &ptsZ );

    //low level clip methods for fast clip algorithm
    static void clipStartTop( double &x0, double &y0, double x1, double y1, double yMax );
    static void clipStartBottom( double &x0, double &y0, double x1, double y1, double yMin );
    static void clipStartRight( double &x0, double &y0, double x1, double y1, double xMax );
    static void clipStartLeft( double &x0, double &y0, double &x1, double &y1, double xMin );
    static void clipEndTop( double x0, double y0, double &x1, double &y1, double yMax );
    static void clipEndBottom( double x0, double y0, double &x1, double &y1, double yMin );
    static void clipEndRight( double x0, double y0, double &x1, double &y1, double xMax );
    static void clipEndLeft( double x0, double y0, double &x1, double &y1, double xMin );
};

#include "qgslinestring.h"
#include "qgspoint.h"

#ifndef SIP_RUN
// The inline functions

// Trim the feature using Sutherland and Hodgman's
// polygon-clipping algorithm. See J. D. Foley, A. van Dam,
// S. K. Feiner, and J. F. Hughes, Computer Graphics, Principles and
// Practice. Addison-Wesley Systems Programming Series,
// Addison-Wesley, 2nd ed., 1991.

// I understand that this is not the most efficient algorithm, but is
// one (the only?) that is guaranteed to always give the correct
// result.

inline void QgsClipper::trimFeature( QVector<double> &x,
                                     QVector<double> &y,
                                     bool shapeOpen )
{
  QVector<double> tmpX;
  QVector<double> tmpY;
  trimFeatureToBoundary( x, y, tmpX, tmpY, XMax, shapeOpen );

  x.clear();
  y.clear();
  trimFeatureToBoundary( tmpX, tmpY, x, y, YMax, shapeOpen );

  tmpX.clear();
  tmpY.clear();
  trimFeatureToBoundary( x, y, tmpX, tmpY, XMin, shapeOpen );

  x.clear();
  y.clear();
  trimFeatureToBoundary( tmpX, tmpY, x, y, YMin, shapeOpen );
}

inline void QgsClipper::trimPolygon( QPolygonF &pts, const QgsRectangle &clipRect )
{
  QPolygonF tmpPts;
  tmpPts.reserve( pts.size() );

  trimPolygonToBoundary( pts, tmpPts, clipRect, XMax, clipRect.xMaximum() );
  pts.resize( 0 );
  trimPolygonToBoundary( tmpPts, pts, clipRect, YMax, clipRect.yMaximum() );
  tmpPts.resize( 0 );
  trimPolygonToBoundary( pts, tmpPts, clipRect, XMin, clipRect.xMinimum() );
  pts.resize( 0 );
  trimPolygonToBoundary( tmpPts, pts, clipRect, YMin, clipRect.yMinimum() );
}

inline void QgsClipper::trimPolygon( QVector<double> &x, QVector<double> &y, QVector<double> &z, const QgsBox3d &clipRect )
{
  QVector< double > tempX;
  QVector< double > tempY;
  QVector< double > tempZ;
  const int size = x.size();
  tempX.reserve( size );
  tempY.reserve( size );
  tempZ.reserve( size );

  trimPolygonToBoundary( x, y, z, tempX, tempY, tempZ, clipRect, XMax, clipRect.xMaximum() );
  x.resize( 0 );
  y.resize( 0 );
  z.resize( 0 );
  trimPolygonToBoundary( tempX, tempY, tempZ, x, y, z, clipRect, YMax, clipRect.yMaximum() );
  tempX.resize( 0 );
  tempY.resize( 0 );
  tempZ.resize( 0 );
  trimPolygonToBoundary( x, y, z, tempX, tempY, tempZ, clipRect, XMin, clipRect.xMinimum() );
  x.resize( 0 );
  y.resize( 0 );
  z.resize( 0 );
  trimPolygonToBoundary( tempX, tempY, tempZ, x, y, z, clipRect, YMin, clipRect.yMinimum() );

  if ( !clipRect.is2d() )
  {
    tempX.resize( 0 );
    tempY.resize( 0 );
    tempZ.resize( 0 );
    trimPolygonToBoundary( x, y, z, tempX, tempY, tempZ, clipRect, ZMax, clipRect.zMaximum() );
    x.resize( 0 );
    y.resize( 0 );
    z.resize( 0 );
    trimPolygonToBoundary( tempX, tempY, tempZ, x, y, z, clipRect, ZMin, clipRect.zMinimum() );
  }
}

// An auxiliary function that is part of the polygon trimming
// code. Will trim the given polygon to the given boundary and return
// the trimmed polygon in the out pointer. Uses Sutherland and
// Hodgman's polygon-clipping algorithm.

inline void QgsClipper::trimFeatureToBoundary(
  const QVector<double> &inX,
  const QVector<double> &inY,
  QVector<double> &outX,
  QVector<double> &outY,
  Boundary b, bool shapeOpen )
{
  // The shapeOpen parameter selects whether this function treats the
  // shape as open or closed. False is appropriate for polygons and
  // true for polylines.

  int i1 = inX.size() - 1; // start with last point

  // and compare to the first point initially.
  for ( int i2 = 0; i2 < inX.size() ; ++i2 )
  {
    // look at each edge of the polygon in turn

    //ignore segments with nan or inf coordinates
    if ( std::isnan( inX[i2] ) || std::isnan( inY[i2] ) || std::isinf( inX[i2] ) || std::isinf( inY[i2] )
         || std::isnan( inX[i1] ) || std::isnan( inY[i1] ) || std::isinf( inX[i1] ) || std::isinf( inY[i1] ) )
    {
      i1 = i2;
      continue;
    }


    if ( inside( inX[i2], inY[i2], b ) ) // end point of edge is inside boundary
    {
      if ( inside( inX[i1], inY[i1], b ) )
      {
        outX.push_back( inX[i2] );
        outY.push_back( inY[i2] );
      }
      else
      {
        // edge crosses into the boundary, so trim back to the boundary, and
        // store both ends of the new edge
        if ( !( i2 == 0 && shapeOpen ) )
        {
          const QgsPointXY p = intersect( inX[i1], inY[i1], inX[i2], inY[i2], b );
          outX.push_back( p.x() );
          outY.push_back( p.y() );
        }

        outX.push_back( inX[i2] );
        outY.push_back( inY[i2] );
      }
    }
    else // end point of edge is outside boundary
    {
      // start point is in boundary, so need to trim back
      if ( inside( inX[i1], inY[i1], b ) )
      {
        if ( !( i2 == 0 && shapeOpen ) )
        {
          const QgsPointXY p = intersect( inX[i1], inY[i1], inX[i2], inY[i2], b );
          outX.push_back( p.x() );
          outY.push_back( p.y() );
        }
      }
    }
    i1 = i2;
  }
}

inline void QgsClipper::trimPolygonToBoundary( const QPolygonF &inPts, QPolygonF &outPts, const QgsRectangle &rect, Boundary b, double boundaryValue )
{
  int i1 = inPts.size() - 1; // start with last point

  // and compare to the first point initially.
  for ( int i2 = 0; i2 < inPts.size() ; ++i2 )
  {
    // look at each edge of the polygon in turn
    if ( inside( inPts[i2], b, boundaryValue ) ) // end point of edge is inside boundary
    {
      if ( inside( inPts[i1], b, boundaryValue ) )
      {
        outPts.append( inPts[i2] );
      }
      else
      {
        // edge crosses into the boundary, so trim back to the boundary, and
        // store both ends of the new edge
        outPts.append( intersectRect( inPts[i1], inPts[i2], b, rect ) );
        outPts.append( inPts[i2] );
      }
    }
    else // end point of edge is outside boundary
    {
      // start point is in boundary, so need to trim back
      if ( inside( inPts[i1], b, boundaryValue ) )
      {
        outPts.append( intersectRect( inPts[i1], inPts[i2], b, rect ) );
      }
    }
    i1 = i2;
  }
}

inline void QgsClipper::trimPolygonToBoundary( const QVector<double> &inX, const QVector<double> &inY, const QVector< double > &inZ,
    QVector<double> &outX, QVector<double> &outY, QVector<double> &outZ, const QgsBox3d &rect, Boundary boundary, double boundaryValue )
{
  const double len = inX.length();
  double inI1X = inX.at( len - 1 ); // start with last point
  double inI1Y = inY.at( len - 1 );
  double inI1Z = inZ.at( len - 1 );

  double xTemp;
  double yTemp;
  double zTemp;

  // and compare to the first point initially.
  for ( int i2 = 0; i2 < len; ++i2 )
  {
    double inI2X = inX.at( i2 );
    double inI2Y = inY.at( i2 );
    double inI2Z = inZ.at( i2 );

    // look at each edge of the polygon in turn
    if ( inside( inI2X, inI2Y, inI2Z, boundary, boundaryValue ) ) // end point of edge is inside boundary
    {
      if ( inside( inI1X, inI1Y, inI1Z, boundary, boundaryValue ) )
      {
        outX << inI2X;
        outY << inI2Y;
        outZ << inI2Z;
      }
      else
      {
        // edge crosses into the boundary, so trim back to the boundary, and
        // store both ends of the new edge
        intersectRect( inI1X, inI1Y, inI1Z, inI2X, inI2Y, inI2Z, xTemp, yTemp, zTemp, boundary, rect ) ;
        outX << xTemp;
        outY << yTemp;
        outZ << zTemp;
        outX << inI2X;
        outY << inI2Y;
        outZ << inI2Z;
      }
    }
    else // end point of edge is outside boundary
    {
      // start point is in boundary, so need to trim back
      if ( inside( inI1X, inI1Y, inI1Z, boundary, boundaryValue ) )
      {
        intersectRect( inI1X, inI1Y, inI1Z, inI2X, inI2Y, inI2Z, xTemp, yTemp, zTemp, boundary, rect );
        outX << xTemp;
        outY << yTemp;
        outZ << zTemp;
      }
    }
    inI1X = inI2X;
    inI1Y = inI2Y;
    inI1Z = inI2Z;
  }
}

// An auxiliary function to trimPolygonToBoundarY() that returns
// whether a point is inside or outside the given boundary.

inline bool QgsClipper::inside( const double x, const double y, Boundary b )
{
  switch ( b )
  {
    case XMax: // x < MAX_X is inside
      if ( x < MAX_X )
        return true;
      break;
    case XMin: // x > MIN_X is inside
      if ( x > MIN_X )
        return true;
      break;
    case YMax: // y < MAX_Y is inside
      if ( y < MAX_Y )
        return true;
      break;
    case YMin: // y > MIN_Y is inside
      if ( y > MIN_Y )
        return true;
      break;
    case ZMax: // z < MAX_Z is inside
    case ZMin: // z > MIN_Z is inside
      return false;
  }
  return false;
}

inline bool QgsClipper::inside( QPointF pt, Boundary b, double val )
{
  switch ( b )
  {
    case XMax: // x < MAX_X is inside
      return ( pt.x() < val );
    case XMin: // x > MIN_X is inside
      return ( pt.x() > val );
    case YMax: // y < MAX_Y is inside
      return ( pt.y() < val );
    case YMin: // y > MIN_Y is inside
      return ( pt.y() > val );
    case ZMax: // z < MAX_Z is inside
    case ZMin: // z > MIN_Z is inside
      return false;
  }
  return false;
}

inline bool QgsClipper::inside( double x, double y, double z, Boundary boundary, double boundaryValue )
{
  switch ( boundary )
  {
    case XMax: // x < MAX_X is inside
      return ( x < boundaryValue );
    case XMin: // x > MIN_X is inside
      return ( x > boundaryValue );
    case YMax: // y < MAX_Y is inside
      return ( y < boundaryValue );
    case YMin: // y > MIN_Y is inside
      return ( y > boundaryValue );
    case ZMax: // z < MAX_Z is inside
      return ( z < boundaryValue );
    case ZMin: // z > MIN_Z is inside
      return ( z > boundaryValue );
  }
  return false;
}


// An auxiliary function to trimPolygonToBoundarY() that calculates and
// returns the intersection of the line defined by the given points
// and the given boundary.

inline QgsPointXY QgsClipper::intersect( const double x1, const double y1,
    const double x2, const double y2,
    Boundary b )
{
  // This function assumes that the two given points (x1, y1), and
  // (x2, y2) cross the given boundary. Making this assumption allows
  // some optimisations.

  double r_n = SMALL_NUM, r_d = SMALL_NUM;

  switch ( b )
  {
    case XMax: // x = MAX_X boundary
      r_n = -( x1 - MAX_X ) * ( MAX_Y - MIN_Y );
      r_d = ( x2 - x1 )   * ( MAX_Y - MIN_Y );
      break;
    case XMin: // x = MIN_X boundary
      r_n = -( x1 - MIN_X ) * ( MAX_Y - MIN_Y );
      r_d = ( x2 - x1 )   * ( MAX_Y - MIN_Y );
      break;
    case YMax: // y = MAX_Y boundary
      r_n = ( y1 - MAX_Y ) * ( MAX_X - MIN_X );
      r_d = -( y2 - y1 )   * ( MAX_X - MIN_X );
      break;
    case YMin: // y = MIN_Y boundary
      r_n = ( y1 - MIN_Y ) * ( MAX_X - MIN_X );
      r_d = -( y2 - y1 )   * ( MAX_X - MIN_X );
      break;
    case ZMax: // z < MAX_Z is inside
    case ZMin: // z > MIN_Z is inside
      break;
  }

  QgsPointXY p;

  if ( std::fabs( r_d ) > SMALL_NUM && std::fabs( r_n ) > SMALL_NUM )
  {
    // they cross
    const double r = r_n / r_d;
    p.set( x1 + r * ( x2 - x1 ), y1 + r * ( y2 - y1 ) );
  }
  else
  {
    // Should never get here, but if we do for some reason, cause a
    // clunk because something else is wrong if we do.
    Q_ASSERT( std::fabs( r_d ) > SMALL_NUM && std::fabs( r_n ) > SMALL_NUM );
  }

  return p;
}

inline QPointF QgsClipper::intersectRect( QPointF pt1,
    QPointF pt2,
    Boundary b, const QgsRectangle &rect )
{
  // This function assumes that the two given points (x1, y1), and
  // (x2, y2) cross the given boundary. Making this assumption allows
  // some optimisations.

  double r_n = SMALL_NUM, r_d = SMALL_NUM;
  const double x1 = pt1.x(), x2 = pt2.x();
  const double y1 = pt1.y(), y2 = pt2.y();

  switch ( b )
  {
    case XMax: // x = MAX_X boundary
      r_n = -( x1 - rect.xMaximum() ) * ( rect.yMaximum() - rect.yMinimum() );
      r_d = ( x2 - x1 )   * ( rect.yMaximum() - rect.yMinimum() );
      break;
    case XMin: // x = MIN_X boundary
      r_n = -( x1 - rect.xMinimum() ) * ( rect.yMaximum() - rect.yMinimum() );
      r_d = ( x2 - x1 )   * ( rect.yMaximum() - rect.yMinimum() );
      break;
    case YMax: // y = MAX_Y boundary
      r_n = ( y1 - rect.yMaximum() ) * ( rect.xMaximum() - rect.xMinimum() );
      r_d = -( y2 - y1 )   * ( rect.xMaximum() - rect.xMinimum() );
      break;
    case YMin: // y = MIN_Y boundary
      r_n = ( y1 - rect.yMinimum() ) * ( rect.xMaximum() - rect.xMinimum() );
      r_d = -( y2 - y1 )   * ( rect.xMaximum() - rect.xMinimum() );
      break;
    case ZMax: // z < MAX_Z is inside
    case ZMin: // z > MIN_Z is inside
      break;
  }

  double r = 0;
  if ( !qgsDoubleNear( r_d, 0.0 ) )
  {
    r = r_n / r_d;
  }
  return QPointF( x1 + r * ( x2 - x1 ), y1 + r * ( y2 - y1 ) );
}

inline void QgsClipper::intersectRect( const double x1, const double y1, const double z1,
                                       const double x2, const double y2, const double z2,
                                       double &xOut, double &yOut, double &zOut,
                                       Boundary boundary, const QgsBox3d &rect )
{
  // This function assumes that the two given points (x1, y1, z1), and
  // (x2, y2, z2) cross the given boundary. Making this assumption allows
  // some optimisations.

  double r_n = SMALL_NUM, r_d = SMALL_NUM;

  switch ( boundary )
  {
    case XMax: // x = MAX_X boundary
      r_n = -( x1 - rect.xMaximum() ) * ( rect.yMaximum() - rect.yMinimum() );
      r_d = ( x2 - x1 )   * ( rect.yMaximum() - rect.yMinimum() );
      break;
    case XMin: // x = MIN_X boundary
      r_n = -( x1 - rect.xMinimum() ) * ( rect.yMaximum() - rect.yMinimum() );
      r_d = ( x2 - x1 )   * ( rect.yMaximum() - rect.yMinimum() );
      break;
    case YMax: // y = MAX_Y boundary
      r_n = ( y1 - rect.yMaximum() ) * ( rect.xMaximum() - rect.xMinimum() );
      r_d = -( y2 - y1 )   * ( rect.xMaximum() - rect.xMinimum() );
      break;
    case YMin: // y = MIN_Y boundary
      r_n = ( y1 - rect.yMinimum() ) * ( rect.xMaximum() - rect.xMinimum() );
      r_d = -( y2 - y1 )   * ( rect.xMaximum() - rect.xMinimum() );
      break;
    case ZMax: // z = MAX_Z boundary
      r_n = ( z1 - rect.zMaximum() );
      r_d = -( z2 - z1 );
      break;
    case ZMin: // z = MIN_Z boundary
      r_n = ( z1 - rect.zMinimum() );
      r_d = -( z2 - z1 );
      break;
  }

  double r = 0;
  if ( !qgsDoubleNear( r_d, 0.0 ) )
  {
    r = r_n / r_d;
  }
  xOut = x1 + r * ( x2 - x1 );
  yOut = y1 + r * ( y2 - y1 );
  zOut = z1 + r * ( z2 - z1 );
}

inline void QgsClipper::clipStartTop( double &x0, double &y0, double x1, double y1, double yMax )
{
  x0 += ( x1 - x0 )  * ( yMax - y0 ) / ( y1 - y0 );
  y0 = yMax;
}

inline void QgsClipper::clipStartBottom( double &x0, double &y0, double x1, double y1, double yMin )
{
  x0 += ( x1 - x0 ) * ( yMin - y0 ) / ( y1 - y0 );
  y0 = yMin;
}

inline void QgsClipper::clipStartRight( double &x0, double &y0, double x1, double y1, double xMax )
{
  y0 += ( y1 - y0 ) * ( xMax - x0 ) / ( x1 - x0 );
  x0 = xMax;
}

inline void QgsClipper::clipStartLeft( double &x0, double &y0, double &x1, double &y1, double xMin )
{
  y0 += ( y1 - y0 ) * ( xMin - x0 ) / ( x1 - x0 );
  x0 = xMin;
}

inline void QgsClipper::clipEndTop( double x0, double y0, double &x1, double &y1, double yMax )
{
  x1 += ( x1 - x0 ) * ( yMax - y1 ) / ( y1 - y0 );
  y1 = yMax;
}

inline void QgsClipper::clipEndBottom( double x0, double y0, double &x1, double &y1, double yMin )
{
  x1 += ( x1 - x0 ) * ( yMin - y1 ) / ( y1 - y0 );
  y1 = yMin;
}

inline void QgsClipper::clipEndRight( double x0, double y0, double &x1, double &y1, double xMax )
{
  y1 += ( y1 - y0 ) * ( xMax - x1 ) / ( x1 - x0 );
  x1 = xMax;
}

inline void QgsClipper::clipEndLeft( double x0, double y0, double &x1, double &y1, double xMin )
{
  y1 += ( y1 - y0 ) * ( xMin - x1 ) / ( x1 - x0 );
  x1 = xMin;
}

inline bool QgsClipper::clipLineSegment( const QgsBox3d &clipExtent, double &x0, double &y0, double &, double &x1, double &y1, double & )
{
  // TODO: while waiting for a valid 3d solution, using the 2d one:
  return clipLineSegment( clipExtent.xMinimum(), clipExtent.xMaximum(), clipExtent.yMinimum(), clipExtent.yMaximum(),
                          x0, y0, x1, y1 );
}

//'Fast clipping' algorithm (Sobkow et al. 1987, Computers & Graphics Vol.11, 4, p.459-467)
inline bool QgsClipper::clipLineSegment( double xLeft, double xRight, double yBottom, double yTop, double &x0, double &y0, double &x1, double &y1 )
{
  int lineCode = 0;

  if ( y1 < yBottom )
    lineCode |= 4;
  else if ( y1 > yTop )
    lineCode |= 8;

  if ( x1 > xRight )
    lineCode |= 2;
  else if ( x1 < xLeft )
    lineCode |= 1;

  if ( y0 < yBottom )
    lineCode |= 64;
  else if ( y0 > yTop )
    lineCode |= 128;

  if ( x0 > xRight )
    lineCode |= 32;
  else if ( x0 < xLeft )
    lineCode |= 16;

  switch ( lineCode )
  {
    case 0: //completely inside
      return true;

    case 1:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 2:
      clipEndRight( x0, y0, x1, y1, xRight );
      return true;

    case 4:
      clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 5:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 6:
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 8:
      clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 9:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 > yTop )
        clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 10:
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 > yTop )
        clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 16:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 18:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      clipEndRight( x0, y0, x1, y1, xRight );
      return true;

    case 20:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 < yBottom )
        return false;
      clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 22:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 < yBottom )
        return false;
      clipEndBottom( x0, y0, x1, y1, yBottom );
      if ( x1 > xRight )
        clipEndRight( x0, y0, x1, y1, xRight );
      return true;

    case 24:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 > yTop )
        return false;
      clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 26:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 > yTop )
        return false;
      clipEndTop( x0, y0, x1, y1, yTop );
      if ( x1 > xRight )
        clipEndRight( x0, y0, x1, y1, xRight );
      return true;

    case 32:
      clipStartRight( x0, y0, x1, y1, xRight );
      return true;

    case 33:
      clipStartRight( x0, y0, x1, y1, xRight );
      clipEndLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 36:
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 < yBottom )
        return false;
      clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 37:
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 < yBottom )
        return false;
      clipEndBottom( x0, y0, x1, y1, yBottom );
      if ( x1 < xLeft )
        clipEndLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 40:
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 > yTop )
        return false;
      clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 41:
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 > yTop )
        return false;
      clipEndTop( x0, y0, x1, y1, yTop );
      if ( x1 < xLeft )
        clipEndLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 64:
      clipStartBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 65:
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 < xLeft )
        return false;
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 66:
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 > xRight )
        return false;
      clipEndRight( x0, y0, x1, y1, xRight );
      return true;

    case 72:
      clipStartBottom( x0, y0, x1, y1, yBottom );
      clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 73:
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 < xLeft )
        return false;
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 > yTop )
        clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 74:
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 > xRight )
        return false;
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 > yTop )
        clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 80:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 < yBottom )
        clipStartBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 82:
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 < yBottom )
        return false;
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 < xLeft )
        clipStartLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 88:
      clipEndTop( x0, y0, x1, y1, yTop );
      if ( x1 < xLeft )
        return false;
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 < xLeft )
        clipStartLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 90:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 > yTop )
        return false;
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 < yBottom )
        return false;
      if ( y0 < yBottom )
        clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( y1 > yTop )
        clipEndTop( x0, y0, x1, y1, yTop );
      return true;

    case 96:
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 < yBottom )
        clipStartBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 97:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 < yBottom )
        return false;
      clipStartBottom( x0, y0, x1, y1, yBottom );
      if ( x0 > xRight )
        clipStartRight( x0, y0, x1, y1, xRight );
      return true;

    case 104:
      clipEndTop( x0, y0, x1, y1, yTop );
      if ( x1 > xRight )
        return false;
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 < yBottom )
        clipStartBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 105:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 < yBottom )
        return false;
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 > yTop )
        return false;
      if ( y1 > yTop )
        clipEndTop( x0, y0, x1, y1, yTop );
      if ( y0 < yBottom )
        clipStartBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 128:
      clipStartTop( x0, y0, x1, y1, yTop );
      return true;

    case 129:
      clipStartTop( x0, y0, x1, y1, yTop );
      if ( x0 < xLeft )
        return false;
      clipEndLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 130:
      clipStartTop( x0, y0, x1, y1, yTop );
      if ( x0 > xRight )
        return false;
      clipEndRight( x0, y0, x1, y1, xRight );
      return true;

    case 132:
      clipStartTop( x0, y0, x1, y1, yTop );
      clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 133:
      clipStartTop( x0, y0, x1, y1, yTop );
      if ( x0 < xLeft )
        return false;
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 134:
      clipStartTop( x0, y0, x1, y1, yTop );
      if ( x0 > xRight )
        return false;
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 144:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 > yTop )
        clipStartTop( x0, y0, x1, y1, yTop );
      return true;

    case 146:
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 > yTop )
        return false;
      clipStartTop( x0, y0, x1, y1, yTop );
      if ( x0 < xLeft )
        clipStartLeft( x0, y0, x1, y1, xLeft );
      return true;

    case 148:
      clipEndBottom( x0, y0, x1, y1, yBottom );
      if ( x1 < xLeft )
        return false;
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 > yTop )
        clipStartTop( x0, y0, x1, y1, yTop );
      return true;

    case 150:
      clipStartLeft( x0, y0, x1, y1, xLeft );
      if ( y0 < yBottom )
        return false;
      clipEndRight( x0, y0, x1, y1, xRight );
      if ( y1 > yTop )
        return false;
      if ( y0 > yTop )
        clipStartTop( x0, y0, x1, y1, yTop );
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      return true;

    case 160:
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 > yTop )
        clipStartTop( x0, y0, x1, y1, yTop );
      return true;

    case 161:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 > yTop )
        return false;
      clipStartTop( x0, y0, x1, y1, yTop );
      if ( x0 > xRight )
        clipStartRight( x0, y0, x1, y1, xRight );
      return true;

    case 164:
      clipEndBottom( x0, y0, x1, y1, yBottom );
      if ( x1 > xRight )
        return false;
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 > yTop )
        clipStartTop( x0, y0, x1, y1, yTop );
      return true;

    case 165:
      clipEndLeft( x0, y0, x1, y1, xLeft );
      if ( y1 > yTop )
        return false;
      clipStartRight( x0, y0, x1, y1, xRight );
      if ( y0 < yBottom )
        return false;
      if ( y1 < yBottom )
        clipEndBottom( x0, y0, x1, y1, yBottom );
      if ( y0 > yTop )
        clipStartTop( x0, y0, x1, y1, yTop );
      return true;
  }

  return false;

}
#endif // SIP_RUN


#endif
