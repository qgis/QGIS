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
 * Represents a single 2D line segment, consisting of a 2D start and end vertex only.
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsLineSegment2D
{

  public:

    /**
     * Constructor for a QgsLineSegment2D from the specified \a start point to
     * the \a end point.
     */
    QgsLineSegment2D( const QgsPointXY &start, const QgsPointXY &end )
      : mStart( start )
      , mEnd( end )
    {}

    /**
     * Constructor for a QgsLineSegment2D from the point (\a x1, \a y2) to
     * (\a x2, \a y2).
     */
    QgsLineSegment2D( double x1, double y1, double x2, double y2 )
      : mStart( QgsPointXY( x1, y1 ) )
      , mEnd( QgsPointXY( x2, y2 ) )
    {}

    /**
     * Returns the length of the segment.
     * \see lengthSquared()
     */
    double length() const
    {
      return std::sqrt( ( mStart.x() - mEnd.x() ) * ( mStart.x() - mEnd.x() ) + ( mStart.y() - mEnd.y() ) * ( mStart.y() - mEnd.y() ) );
    }

    /**
     * Returns the squared length of the segment.
     * \see length()
     */
    double lengthSquared() const
    {
      return ( mStart.x() - mEnd.x() ) * ( mStart.x() - mEnd.x() ) + ( mStart.y() - mEnd.y() ) * ( mStart.y() - mEnd.y() );
    }

    /**
     * Returns the segment's start x-coordinate.
     * \see start()
     * \see startY()
     */
    double startX() const
    {
      return mStart.x();
    }

    /**
     * Returns the segment's start y-coordinate.
     * \see start()
     * \see startX()
     */
    double startY() const
    {
      return mStart.y();
    }

    /**
     * Returns the segment's end x-coordinate.
     * \see end()
     * \see endY()
     */
    double endX() const
    {
      return mEnd.x();
    }

    /**
     * Returns the segment's end y-coordinate.
     * \see end()
     * \see endX()
     */
    double endY() const
    {
      return mEnd.y();
    }

    /**
     * Returns the segment's start point.
     * \see end()
     * \see startX()
     * \see startY()
     */
    QgsPointXY start() const
    {
      return mStart;
    }

    /**
     * Returns the segment's end point.
     * \see start()
     * \see endX()
     * \see endY()
     */
    QgsPointXY end() const
    {
      return mEnd;
    }

    /**
     * Sets the segment's start \a x coordinate.
     * \see setEndX()
     * \see setStart()
     * \see setStartY()
     */
    void setStartX( double x )
    {
      mStart.setX( x );
    }

    /**
     * Sets the segment's start \a y coordinate.
     * \see setEndY()
     * \see setStart()
     * \see setStartX()
     */
    void setStartY( double y )
    {
      mStart.setY( y );
    }

    /**
     * Sets the segment's end \a x coordinate.
     * \see setStartX()
     * \see setEnd()
     * \see setEndY()
     */
    void setEndX( double x )
    {
      mEnd.setX( x );
    }

    /**
     * Sets the segment's end \a y coordinate.
     * \see setStartY()
     * \see setEnd()
     * \see setEndX()
     */
    void setEndY( double y )
    {
      mEnd.setY( y );
    }

    /**
     * Sets the segment's \a start point.
     * \see setStartX()
     * \see setStartY()
     * \see setEnd()
     */
    void setStart( const QgsPointXY &start )
    {
      mStart = start;
    }

    /**
     * Sets the segment's \a end point.
     * \see setEndX()
     * \see setEndY()
     * \see setStart()
     */
    void setEnd( const QgsPointXY &end )
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
    int pointLeftOfLine( const QgsPointXY &point ) const;

    /**
     * Reverses the line segment, so that the start and end points are flipped.
     */
    void reverse()
    {
      std::swap( mStart, mEnd );
    }

    //! Equality operator
    bool operator==( const QgsLineSegment2D &other ) const
    {
      return mStart == other.mStart && mEnd == other.mEnd;
    }

    //! Inequality operator
    bool operator!=( const QgsLineSegment2D &other ) const
    {
      return mStart != other.mStart || mEnd != other.mEnd;
    }

  private:

    QgsPointXY mStart;
    QgsPointXY mEnd;

};

#endif // QGSLINESEGMENT_H
