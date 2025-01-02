/***************************************************************************
                         qgslinesegment.h
                         -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINESEGMENT_H
#define QGSLINESEGMENT_H

#include "qgis_core.h"
#include "qgspointxy.h"

class QgsLineString;

/**
 * \ingroup core
 * \brief Represents a single 2D line segment, consisting of a 2D start and end vertex only.
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsLineSegment2D
{

  public:

    /**
     * Constructor for a QgsLineSegment2D from the specified \a start point to
     * the \a end point.
     */
    QgsLineSegment2D( const QgsPointXY &start, const QgsPointXY &end ) SIP_HOLDGIL
  : mStart( start )
    , mEnd( end )
    {}

    /**
     * Constructor for a QgsLineSegment2D from the point (\a x1, \a y2) to
     * (\a x2, \a y2).
     */
    QgsLineSegment2D( double x1, double y1, double x2, double y2 ) SIP_HOLDGIL
  : mStart( QgsPointXY( x1, y1 ) )
    , mEnd( QgsPointXY( x2, y2 ) )
    {}

    /**
     * Returns the length of the segment.
     * \see lengthSquared()
     */
    double length() const SIP_HOLDGIL
    {
      return mStart.distance( mEnd );
    }

    /**
     * Returns the squared length of the segment.
     * \see length()
     */
    double lengthSquared() const SIP_HOLDGIL
    {
      return mStart.sqrDist( mEnd );
    }

    /**
     * Returns the segment's start x-coordinate.
     * \see start()
     * \see startY()
     */
    double startX() const SIP_HOLDGIL
    {
      return mStart.x();
    }

    /**
     * Returns the segment's start y-coordinate.
     * \see start()
     * \see startX()
     */
    double startY() const SIP_HOLDGIL
    {
      return mStart.y();
    }

    /**
     * Returns the segment's end x-coordinate.
     * \see end()
     * \see endY()
     */
    double endX() const SIP_HOLDGIL
    {
      return mEnd.x();
    }

    /**
     * Returns the segment's end y-coordinate.
     * \see end()
     * \see endX()
     */
    double endY() const SIP_HOLDGIL
    {
      return mEnd.y();
    }

    /**
     * Returns the segment's start point.
     * \see end()
     * \see startX()
     * \see startY()
     */
    QgsPointXY start() const SIP_HOLDGIL
    {
      return mStart;
    }

    /**
     * Returns the segment's end point.
     * \see start()
     * \see endX()
     * \see endY()
     */
    QgsPointXY end() const SIP_HOLDGIL
    {
      return mEnd;
    }

    /**
     * Sets the segment's start \a x coordinate.
     * \see setEndX()
     * \see setStart()
     * \see setStartY()
     */
    void setStartX( double x ) SIP_HOLDGIL
    {
      mStart.setX( x );
    }

    /**
     * Sets the segment's start \a y coordinate.
     * \see setEndY()
     * \see setStart()
     * \see setStartX()
     */
    void setStartY( double y ) SIP_HOLDGIL
    {
      mStart.setY( y );
    }

    /**
     * Sets the segment's end \a x coordinate.
     * \see setStartX()
     * \see setEnd()
     * \see setEndY()
     */
    void setEndX( double x ) SIP_HOLDGIL
    {
      mEnd.setX( x );
    }

    /**
     * Sets the segment's end \a y coordinate.
     * \see setStartY()
     * \see setEnd()
     * \see setEndX()
     */
    void setEndY( double y ) SIP_HOLDGIL
    {
      mEnd.setY( y );
    }

    /**
     * Sets the segment's \a start point.
     * \see setStartX()
     * \see setStartY()
     * \see setEnd()
     */
    void setStart( const QgsPointXY &start ) SIP_HOLDGIL
    {
      mStart = start;
    }

    /**
     * Sets the segment's \a end point.
     * \see setEndX()
     * \see setEndY()
     * \see setStart()
     */
    void setEnd( const QgsPointXY &end ) SIP_HOLDGIL
    {
      mEnd = end;
    }

    /**
     * Tests if a \a point is to the left of the line segment.
     *
     * Returns -1 if the point falls to the left of the line, or +1 if the point
     * is to the right.
     *
     * If the return value is 0, then the test was unsuccessful (e.g. due to testing a point exactly
     * on the line, or exactly in line with the segment) and the result is undefined.
     *
     * \see QgsGeometryUtils::leftOfLine()
     */
    int pointLeftOfLine( const QgsPointXY &point ) const SIP_HOLDGIL;

    /**
     * Reverses the line segment, so that the start and end points are flipped.
     */
    void reverse() SIP_HOLDGIL
    {
      std::swap( mStart, mEnd );
    }

    // TODO c++20 - replace with = default

    bool operator==( const QgsLineSegment2D &other ) const SIP_HOLDGIL
    {
      return mStart == other.mStart && mEnd == other.mEnd;
    }

    bool operator!=( const QgsLineSegment2D &other ) const SIP_HOLDGIL
    {
      return mStart != other.mStart || mEnd != other.mEnd;
    }

  private:

    QgsPointXY mStart;
    QgsPointXY mEnd;

};

#endif // QGSLINESEGMENT_H
