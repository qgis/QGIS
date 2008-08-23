/***************************************************************************
                          qgsline  -  description
                    A simple line composed of two endpoints
                             -------------------
    begin                : 2004-10-24
    copyright            : (C) 2004 by Gary E.Sherman
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
/* $Id$ */

#include "qgsline.h"

QgsLine::QgsLine()
{
}

// Create a line from two points
QgsLine::QgsLine( QgsPoint &p1, QgsPoint &p2 ) : mBegin( p1 ), mEnd( p2 )
{

}

//! Destructor
QgsLine::~QgsLine()
{
}

/// Sets the begin point of the line
void QgsLine::setBegin( QgsPoint &p1 )
{
  mBegin = p1;
}
/// Sets the end point of the line
void QgsLine::setEnd( QgsPoint &p2 )
{
  mEnd = p2;
}

/// Get the begin point of the line
QgsPoint QgsLine::begin() const
{
  return mBegin;
}

/// Get the end point of the line
QgsPoint QgsLine::end() const
{
  return mEnd;
}

//! String representation of the line
QString QgsLine::toString() const
{
  return QString( "Not implemented" );
}

//! As above but with precision for string representaton of a line
QString QgsLine::toString( int thePrecision ) const
{
  return QString( "Not implemented" );
}

/*! Return the well known text representation for the line.
 * The wkt is created without an SRID.
 * @return Well known text
 */
QString QgsLine::wellKnownText()
{
  return QString( "Not implemented" );
}


//! Inequality operator
bool QgsLine::operator!=( const QgsLine &other )
{
  // Note this function assumes that "flipped" lines are not equal,
  // thus preserving the concept of direction
  if (( mBegin != other.begin() ) || ( mEnd != other.end() ) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

/// Assignment
QgsLine & QgsLine::operator=( const QgsLine & other )
{
  if ( &other != this )
  {
    mBegin = other.begin();
    mEnd = other.end();
  }

  return *this;
}
