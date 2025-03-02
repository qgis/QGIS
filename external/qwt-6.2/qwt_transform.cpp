/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_transform.h"
#include "qwt_math.h"

//! Smallest allowed value for logarithmic scales: 1.0e-150
const double QwtLogTransform::LogMin = 1.0e-150;

//! Largest allowed value for logarithmic scales: 1.0e150
const double QwtLogTransform::LogMax = 1.0e150;

//! Constructor
QwtTransform::QwtTransform()
{
}

//! Destructor
QwtTransform::~QwtTransform()
{
}

/*!
   \param value Value to be bounded
   \return value unmodified
 */
double QwtTransform::bounded( double value ) const
{
    return value;
}

//! Constructor
QwtNullTransform::QwtNullTransform():
    QwtTransform()
{
}

//! Destructor
QwtNullTransform::~QwtNullTransform()
{
}

/*!
   \param value Value to be transformed
   \return value unmodified
 */
double QwtNullTransform::transform( double value ) const
{
    return value;
}

/*!
   \param value Value to be transformed
   \return value unmodified
 */
double QwtNullTransform::invTransform( double value ) const
{
    return value;
}

//! \return Clone of the transformation
QwtTransform* QwtNullTransform::copy() const
{
    return new QwtNullTransform();
}

//! Constructor
QwtLogTransform::QwtLogTransform():
    QwtTransform()
{
}

//! Destructor
QwtLogTransform::~QwtLogTransform()
{
}

/*!
   \param value Value to be transformed
   \return log( value )
 */
double QwtLogTransform::transform( double value ) const
{
    return std::log( value );
}

/*!
   \param value Value to be transformed
   \return exp( value )
 */
double QwtLogTransform::invTransform( double value ) const
{
    return std::exp( value );
}

/*!
   \param value Value to be bounded
   \return qBound( LogMin, value, LogMax )
 */
double QwtLogTransform::bounded( double value ) const
{
    return qBound( LogMin, value, LogMax );
}

//! \return Clone of the transformation
QwtTransform* QwtLogTransform::copy() const
{
    return new QwtLogTransform();
}

/*!
   Constructor
   \param exponent Exponent
 */
QwtPowerTransform::QwtPowerTransform( double exponent ):
    QwtTransform(),
    m_exponent( exponent )
{
}

//! Destructor
QwtPowerTransform::~QwtPowerTransform()
{
}

/*!
   \param value Value to be transformed
   \return Exponentiation preserving the sign
 */
double QwtPowerTransform::transform( double value ) const
{
    if ( value < 0.0 )
        return -std::pow( -value, 1.0 / m_exponent );
    else
        return std::pow( value, 1.0 / m_exponent );

}

/*!
   \param value Value to be transformed
   \return Inverse exponentiation preserving the sign
 */
double QwtPowerTransform::invTransform( double value ) const
{
    if ( value < 0.0 )
        return -std::pow( -value, m_exponent );
    else
        return std::pow( value, m_exponent );
}

//! \return Clone of the transformation
QwtTransform* QwtPowerTransform::copy() const
{
    return new QwtPowerTransform( m_exponent );
}
