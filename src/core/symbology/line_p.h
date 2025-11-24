/***************************************************************************
    line_p.h
    ---------------------
    begin                : 2025/09/25
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LINE_P_H
#define LINE_P_H

#define SIP_NO_FILE

#include <QPointF>

/////////

///@cond PRIVATE

class Line
{
  public:
    Line( QPointF p1, QPointF p2 )
      : mVertical( false )
      , mIncreasing( false )
      , mT( 0.0 )
      , mLength( 0.0 )
    {
      if ( p1 == p2 )
        return; // invalid

      // tangent and direction
      if ( qgsDoubleNear( p1.x(), p2.x() ) )
      {
        // vertical line - tangent undefined
        mVertical = true;
        mIncreasing = ( p2.y() > p1.y() );
      }
      else
      {
        mVertical = false;
        mT = ( p2.y() - p1.y() ) / ( p2.x() - p1.x() );
        mIncreasing = ( p2.x() > p1.x() );
      }

      // length
      double x = ( p2.x() - p1.x() );
      double y = ( p2.y() - p1.y() );
      mLength = std::sqrt( x * x + y * y );
    }

    // return angle in radians
    double angle()
    {
      double a = ( mVertical ? M_PI_2 : std::atan( mT ) );

      if ( !mIncreasing )
        a += M_PI;
      return a;
    }

    // return difference for x,y when going along the line with specified interval
    QPointF diffForInterval( double interval ) const
    {
      if ( mVertical )
        return ( mIncreasing ? QPointF( 0, interval ) : QPointF( 0, -interval ) );

      double alpha = std::atan( mT );
      double dx = std::cos( alpha ) * interval;
      double dy = std::sin( alpha ) * interval;
      return ( mIncreasing ? QPointF( dx, dy ) : QPointF( -dx, -dy ) );
    }

    double length() const { return mLength; }

  protected:
    bool mVertical;
    bool mIncreasing;
    double mT;
    double mLength;
};

///@endcond

#endif
