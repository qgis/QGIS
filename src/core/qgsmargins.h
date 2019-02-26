/***************************************************************************
  qgsmargins.h
  ------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMARGINS_H
#define QGSMARGINS_H

#include "qgis_core.h"
#include "qgis.h"
#include <QString>

/**
 * \ingroup core
 * \class QgsMargins
 * \brief The QgsMargins class defines the four margins of a rectangle.
 *
 * QgsMargins defines a set of four margins; left, top, right and bottom, that describe the size of the borders surrounding a rectangle.
 *
 * The isNull() function returns TRUE only if all margins are set to zero.
 * \since QGIS 3.0
 */

//This class was originally based off Qt's QgsMarginsF class
//It was forked in order to always use double values, rather than qreal values.

class CORE_EXPORT QgsMargins
{
  public:

    /**
     * Constructs a margins object with all margins set to 0.
     */
    QgsMargins() = default;

    /**
     * Constructs margins with the given \a left, \a top, \a right, \a bottom
     * \see setLeft()
     * \see setRight()
     * \see setTop()
     * \see setBottom()
     */
    QgsMargins( double left, double top, double right, double bottom )
      : mLeft( left )
      , mTop( top )
      , mRight( right )
      , mBottom( bottom )
    {}

    /**
     * Returns \c TRUE if all margins are is 0; otherwise returns FALSE.
     */
    bool isNull() const
    {
      return qgsDoubleNear( mLeft, 0.0 ) && qgsDoubleNear( mTop, 0.0 ) && qgsDoubleNear( mRight, 0.0 ) && qgsDoubleNear( mBottom, 0.0 );
    }

    /**
     * Returns the left margin.
     * \see setLeft()
     */
    double left() const { return mLeft; }

    /**
     * Returns the top margin.
     * \see setTop()
     */
    double top() const { return mTop; }

    /**
     * Returns the right margin.
     * \see setRight()
     */
    double right() const { return mRight; }

    /**
     * Returns the bottom margin.
     * \see setBottom()
     */
    double bottom() const { return mBottom; }

    /**
     * Sets the left margin to \a left.
     * \see left()
     */
    void setLeft( double left ) { mLeft = left; }

    /**
     * Sets the top margin to \a top.
     * \see top()
     */
    void setTop( double top ) { mTop = top; }

    /**
     * Sets the right margin to \a right.
     * \see right()
     */
    void setRight( double right ) { mRight = right; }

    /**
     * Sets the bottom margin to \a bottom.
     * \see bottom()
     */
    void setBottom( double bottom ) { mBottom = bottom; }

    /**
     * Add each component of \a margins to the respective component of this object
     * and returns a reference to it.
     */
    inline QgsMargins &operator+=( const QgsMargins &margins );

    /**
     * Subtract each component of \a margins from the respective component of this object
     * and returns a reference to it.
     */
    inline QgsMargins &operator-=( const QgsMargins &margins );

    /**
     * Adds the \a addend to each component of this object and returns a reference to it.
     */
    inline QgsMargins &operator+=( double addend );

    /**
     * Subtracts the \a subtrahend from each component of this object
     * and returns a reference to it.
     */
    inline QgsMargins &operator-=( double subtrahend );

    /**
     * Multiplies each component of this object by \a factor
     * and returns a reference to it.
     */
    inline QgsMargins &operator*=( double factor );

    /**
     * Multiplies each component of this object by \a factor
     * and returns a reference to it.
     */
    inline QgsMargins &operator/=( double divisor );

    /**
     * Returns the margins encoded to a string.
     * \see fromString()
     */
    QString toString() const;

    /**
     * Returns a QgsMargins object decoded from a string, or a null QgsMargins
     * if the string could not be interpreted as margins.
     * \see toString()
     */
    static QgsMargins fromString( const QString &string );

  private:
    double mLeft = 0.0;
    double mTop = 0.0;
    double mRight = 0.0;
    double mBottom = 0.0;
};


/**
 * Returns \c TRUE if \a lhs and \a rhs are equal; otherwise returns \c FALSE.
 */
inline bool operator==( const QgsMargins &lhs, const QgsMargins &rhs )
{
  return qgsDoubleNear( lhs.left(), rhs.left() )
         && qgsDoubleNear( lhs.top(), rhs.top() )
         && qgsDoubleNear( lhs.right(), rhs.right() )
         && qgsDoubleNear( lhs.bottom(), rhs.bottom() );
}

/**
 * Returns \c TRUE if \a lhs and \a rhs are different; otherwise returns \c FALSE.
 */
inline bool operator!=( const QgsMargins &lhs, const QgsMargins &rhs )
{
  return !operator==( lhs, rhs );
}

/**
 * Returns a QgsMargins object that is the sum of the given margins, \a m1
 * and \a m2; each component is added separately.
 */
inline QgsMargins operator+( const QgsMargins &m1, const QgsMargins &m2 )
{
  return QgsMargins( m1.left() + m2.left(), m1.top() + m2.top(),
                     m1.right() + m2.right(), m1.bottom() + m2.bottom() );
}

/**
 * Returns a QgsMargins object that is formed by subtracting \a m2 from
 * \a m1; each component is subtracted separately.
 */
inline QgsMargins operator-( const QgsMargins &m1, const QgsMargins &m2 )
{
  return QgsMargins( m1.left() - m2.left(), m1.top() - m2.top(),
                     m1.right() - m2.right(), m1.bottom() - m2.bottom() );
}

/**
 * Returns a QgsMargins object that is formed by adding \a rhs to \a lhs.
 */
inline QgsMargins operator+( const QgsMargins &lhs, double rhs )
{
  return QgsMargins( lhs.left() + rhs, lhs.top() + rhs,
                     lhs.right() + rhs, lhs.bottom() + rhs );
}

/**
 * Returns a QgsMargins object that is formed by adding \a lhs to \a rhs.
 */
inline QgsMargins operator+( double lhs, const QgsMargins &rhs )
{
  return QgsMargins( rhs.left() + lhs, rhs.top() + lhs,
                     rhs.right() + lhs, rhs.bottom() + lhs );
}

/**
 * Returns a QgsMargins object that is formed by subtracting \a rhs from \a lhs.
 */
inline QgsMargins operator-( const QgsMargins &lhs, double rhs )
{
  return QgsMargins( lhs.left() - rhs, lhs.top() - rhs,
                     lhs.right() - rhs, lhs.bottom() - rhs );
}

/**
 * Returns a QgsMargins object that is formed by multiplying each component
 * of the given \a margins by \a factor.
 */
inline QgsMargins operator*( const QgsMargins &margins, double factor )
{
  return QgsMargins( margins.left() * factor, margins.top() * factor,
                     margins.right() * factor, margins.bottom() * factor );
}

/**
 * Returns a QgsMargins object that is formed by multiplying each component
 * of the given \a margins by \a factor.
 */
inline QgsMargins operator*( double factor, const QgsMargins &margins )
{
  return QgsMargins( margins.left() * factor, margins.top() * factor,
                     margins.right() * factor, margins.bottom() * factor );
}

/**
 * Returns a QgsMargins object that is formed by dividing the components of
 * the given \a margins by the given \a divisor.
 */
inline QgsMargins operator/( const QgsMargins &margins, double divisor )
{
  return QgsMargins( margins.left() / divisor, margins.top() / divisor,
                     margins.right() / divisor, margins.bottom() / divisor );
}

inline QgsMargins &QgsMargins::operator+=( const QgsMargins &margins ) SIP_SKIP
{
  return *this = *this + margins;
}

inline QgsMargins &QgsMargins::operator-=( const QgsMargins &margins ) SIP_SKIP
{
  return *this = *this - margins;
}

inline QgsMargins &QgsMargins::operator+=( double addend ) SIP_SKIP
{
  mLeft += addend;
  mTop += addend;
  mRight += addend;
  mBottom += addend;
  return *this;
}

inline QgsMargins &QgsMargins::operator-=( double subtrahend ) SIP_SKIP
{
  mLeft -= subtrahend;
  mTop -= subtrahend;
  mRight -= subtrahend;
  mBottom -= subtrahend;
  return *this;
}

inline QgsMargins &QgsMargins::operator*=( double factor ) SIP_SKIP
{
  return *this = *this * factor;
}

inline QgsMargins &QgsMargins::operator/=( double divisor ) SIP_SKIP
{
  return *this = *this / divisor;
}

/**
 * Returns a QgsMargins object that is formed from all components of \a margins.
 */
inline QgsMargins operator+( const QgsMargins &margins )
{
  return margins;
}

/**
 * Returns a QgsMargins object that is formed by negating all components of \a margins.
 */
inline QgsMargins operator-( const QgsMargins &margins )
{
  return QgsMargins( -margins.left(), -margins.top(), -margins.right(), -margins.bottom() );
}

Q_DECLARE_TYPEINFO( QgsMargins, Q_MOVABLE_TYPE );

#endif // QGSMARGINS_H
