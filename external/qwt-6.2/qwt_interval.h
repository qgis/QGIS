/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_INTERVAL_H
#define QWT_INTERVAL_H

#include "qwt_global.h"
#include <qmetatype.h>

/*!
   \brief A class representing an interval

   The interval is represented by 2 doubles, the lower and the upper limit.
 */

class QWT_EXPORT QwtInterval
{
  public:
    /*!
       Flag indicating if a border is included or excluded
       \sa setBorderFlags(), borderFlags()
     */
    enum BorderFlag
    {
        //! Min/Max values are inside the interval
        IncludeBorders = 0x00,

        //! Min value is not included in the interval
        ExcludeMinimum = 0x01,

        //! Max value is not included in the interval
        ExcludeMaximum = 0x02,

        //! Min/Max values are not included in the interval
        ExcludeBorders = ExcludeMinimum | ExcludeMaximum
    };

    //! Border flags
    Q_DECLARE_FLAGS( BorderFlags, BorderFlag )

    QwtInterval();
    QwtInterval( double minValue, double maxValue,
        BorderFlags = IncludeBorders );

    void setInterval( double minValue, double maxValue,
        BorderFlags = IncludeBorders );

    QwtInterval normalized() const;
    QwtInterval inverted() const;
    QwtInterval limited( double lowerBound, double upperBound ) const;

    bool operator==( const QwtInterval& ) const;
    bool operator!=( const QwtInterval& ) const;

    void setBorderFlags( BorderFlags );
    BorderFlags borderFlags() const;

    double minValue() const;
    double maxValue() const;

    double width() const;
    long double widthL() const;

    void setMinValue( double );
    void setMaxValue( double );

    bool contains( double value ) const;
    bool contains( const QwtInterval& ) const;

    bool intersects( const QwtInterval& ) const;
    QwtInterval intersect( const QwtInterval& ) const;
    QwtInterval unite( const QwtInterval& ) const;

    QwtInterval operator|( const QwtInterval& ) const;
    QwtInterval operator&( const QwtInterval& ) const;

    QwtInterval& operator|=( const QwtInterval& );
    QwtInterval& operator&=( const QwtInterval& );

    QwtInterval extend( double value ) const;
    QwtInterval operator|( double ) const;
    QwtInterval& operator|=( double );

    bool isValid() const;
    bool isNull() const;
    void invalidate();

    QwtInterval symmetrize( double value ) const;

  private:
    double m_minValue;
    double m_maxValue;
    BorderFlags m_borderFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtInterval::BorderFlags )
Q_DECLARE_METATYPE( QwtInterval )
Q_DECLARE_TYPEINFO( QwtInterval, Q_MOVABLE_TYPE );

/*!
   \brief Default Constructor

   Creates an invalid interval [0.0, -1.0]
   \sa setInterval(), isValid()
 */
inline QwtInterval::QwtInterval()
    : m_minValue( 0.0 )
    , m_maxValue( -1.0 )
    , m_borderFlags( IncludeBorders )
{
}

/*!
   Constructor

   Build an interval with from min/max values

   \param minValue Minimum value
   \param maxValue Maximum value
   \param borderFlags Include/Exclude borders
 */
inline QwtInterval::QwtInterval(
        double minValue, double maxValue, BorderFlags borderFlags )
    : m_minValue( minValue )
    , m_maxValue( maxValue )
    , m_borderFlags( borderFlags )
{
}

/*!
   Assign the limits of the interval

   \param minValue Minimum value
   \param maxValue Maximum value
   \param borderFlags Include/Exclude borders
 */
inline void QwtInterval::setInterval(
    double minValue, double maxValue, BorderFlags borderFlags )
{
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_borderFlags = borderFlags;
}

/*!
   Change the border flags

   \param borderFlags Or'd BorderMode flags
   \sa borderFlags()
 */
inline void QwtInterval::setBorderFlags( BorderFlags borderFlags )
{
    m_borderFlags = borderFlags;
}

/*!
   \return Border flags
   \sa setBorderFlags()
 */
inline QwtInterval::BorderFlags QwtInterval::borderFlags() const
{
    return m_borderFlags;
}

/*!
   Assign the lower limit of the interval

   \param minValue Minimum value
 */
inline void QwtInterval::setMinValue( double minValue )
{
    m_minValue = minValue;
}

/*!
   Assign the upper limit of the interval

   \param maxValue Maximum value
 */
inline void QwtInterval::setMaxValue( double maxValue )
{
    m_maxValue = maxValue;
}

//! \return Lower limit of the interval
inline double QwtInterval::minValue() const
{
    return m_minValue;
}

//! \return Upper limit of the interval
inline double QwtInterval::maxValue() const
{
    return m_maxValue;
}

/*!
   A interval is valid when minValue() <= maxValue().
   In case of QwtInterval::ExcludeBorders it is true
   when minValue() < maxValue()

   \return True, when the interval is valid
 */
inline bool QwtInterval::isValid() const
{
    if ( ( m_borderFlags & ExcludeBorders ) == 0 )
        return m_minValue <= m_maxValue;
    else
        return m_minValue < m_maxValue;
}

/*!
   \brief Return the width of an interval

   The width of invalid intervals is 0.0, otherwise the result is
   maxValue() - minValue().

   \return Interval width
   \sa isValid()
 */
inline double QwtInterval::width() const
{
    return isValid() ? ( m_maxValue - m_minValue ) : 0.0;
}

/*!
   \brief Return the width of an interval as long double

   The width of invalid intervals is 0.0, otherwise the result is
   maxValue() - minValue().

   \return Interval width
   \sa isValid()
 */
inline long double QwtInterval::widthL() const
{
    if ( !isValid() )
        return 0.0;

    return static_cast< long double >( m_maxValue )
           - static_cast< long double >( m_minValue );
}

/*!
   \brief Intersection of two intervals

   \param other Interval to intersect with
   \return Intersection of this and other

   \sa intersect()
 */
inline QwtInterval QwtInterval::operator&(
    const QwtInterval& other ) const
{
    return intersect( other );
}

/*!
   Union of two intervals

   \param other Interval to unite with
   \return Union of this and other

   \sa unite()
 */
inline QwtInterval QwtInterval::operator|(
    const QwtInterval& other ) const
{
    return unite( other );
}

/*!
   \brief Compare two intervals

   \param other Interval to compare with
   \return True, when this and other are equal
 */
inline bool QwtInterval::operator==( const QwtInterval& other ) const
{
    return ( m_minValue == other.m_minValue ) &&
           ( m_maxValue == other.m_maxValue ) &&
           ( m_borderFlags == other.m_borderFlags );
}
/*!
   \brief Compare two intervals

   \param other Interval to compare with
   \return True, when this and other are not equal
 */
inline bool QwtInterval::operator!=( const QwtInterval& other ) const
{
    return ( !( *this == other ) );
}

/*!
   Extend an interval

   \param value Value
   \return Extended interval
   \sa extend()
 */
inline QwtInterval QwtInterval::operator|( double value ) const
{
    return extend( value );
}

//! \return true, if isValid() && (minValue() >= maxValue())
inline bool QwtInterval::isNull() const
{
    return isValid() && m_minValue >= m_maxValue;
}

/*!
   Invalidate the interval

   The limits are set to interval [0.0, -1.0]
   \sa isValid()
 */
inline void QwtInterval::invalidate()
{
    m_minValue = 0.0;
    m_maxValue = -1.0;
}

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<( QDebug, const QwtInterval& );
#endif

#endif
