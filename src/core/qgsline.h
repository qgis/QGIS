/***************************************************************************
                          qgsline.h  -  description
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

#ifndef QGSLINE_H
#define QGSLINE_H

#include <iostream>

#include "qgspoint.h"
/** \ingroup core
 * A simple line object composed of a begin and end point.
 *
 * Comparisons (==, !=) honor the direction of the line. This means that
 * flipped lines with the same coordinates are not considered to be equal.
 *
 * For example, (1,1 2,2) != (2,2 1,1)
 */
class CORE_EXPORT QgsLine
{
  public:
    //! Default constructor
    QgsLine();

    /*! Create a line from two points
     * @param p1 begin point
     * @param p2 end point
     */
    QgsLine( QgsPoint &p1, QgsPoint &p2 );

    //! Destructor
    ~QgsLine();

    /*! Sets the begin point of the line
     * @param p1 Beginning point of the line
     */
    void setBegin( QgsPoint &p1 );

    /*! Sets the end point of the line
     * @param p1 End point of the line
     */
    void setEnd( QgsPoint &p2 );

    /*! Get the begin point of the line
     * @return Begin point of the line
     */
    QgsPoint begin() const;

    /*! Get the end point of the line
     * @return End point of the line
     */
    QgsPoint end() const;

    //! String representation of the line
    QString toString() const;

    //! As above but with precision for string representation of a line
    QString toString( int thePrecision ) const;

    /*! Return the well known text representation for the line.
     * The wkt is created without an SRID.
     * @return Well known text
     */
    QString wellKnownText();

    //! Equality operator
    bool operator==( const QgsLine &other );

    //! Inequality operator
    bool operator!=( const QgsLine &other );

    /// Assignment
    QgsLine & operator=( const QgsLine &other );

  private:

    //! Begin point
    QgsPoint mBegin;

    //! End point
    QgsPoint mEnd;

}; // class QgsLine


inline bool operator==( const QgsLine &l1, const QgsLine &l2 )
{
  // Note this function assumes that "flipped" lines are not equal,
  // thus preserving the concept of direction
  if (( l1.begin() == l2.begin() ) && ( l1.end() == l2.end() ) )
  {
    return true;
  }
  else
  {
    return false;
  }
}
//! Stream operator for writing the line
inline std::ostream& operator << ( std::ostream& os, const QgsLine &l )
{
  os << l.toString().toLocal8Bit().data();
  return os;
}


#endif //QGSLINE_H

