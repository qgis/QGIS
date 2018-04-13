/***************************************************************************
                         qgslinesegment.h
                         -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
#ifndef QGSLINESEGMENT_H
#define QGSLINESEGMENT_H

#include "qgis_core.h"
#include "qgspointxy.h"

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
