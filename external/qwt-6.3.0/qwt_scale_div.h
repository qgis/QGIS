/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SCALE_DIV_H
#define QWT_SCALE_DIV_H

#include "qwt_global.h"
#include <qlist.h>

class QwtInterval;

/*!
   \brief A class representing a scale division

   A Qwt scale is defined by its boundaries and 3 list
   for the positions of the major, medium and minor ticks.

   The upperBound() might be smaller than the lowerBound()
   to indicate inverted scales.

   Scale divisions can be calculated from a QwtScaleEngine.

   \sa QwtScaleEngine::divideScale(), QwtPlot::setAxisScaleDiv(),
      QwtAbstractSlider::setScaleDiv()
 */

class QWT_EXPORT QwtScaleDiv
{
  public:
    //! Scale tick types
    enum TickType
    {
        //! No ticks
        NoTick = -1,

        //! Minor ticks
        MinorTick,

        //! Medium ticks
        MediumTick,

        //! Major ticks
        MajorTick,

        //! Number of valid tick types
        NTickTypes
    };

    explicit QwtScaleDiv( double lowerBound = 0.0,
        double upperBound = 0.0 );

    explicit QwtScaleDiv( const QwtInterval&, QList< double >[NTickTypes] );

    explicit QwtScaleDiv( double lowerBound, double upperBound,
        QList< double >[NTickTypes] );

    explicit QwtScaleDiv( double lowerBound, double upperBound,
        const QList< double >& minorTicks, const QList< double >& mediumTicks,
        const QList< double >& majorTicks );

    bool operator==( const QwtScaleDiv& ) const;
    bool operator!=( const QwtScaleDiv& ) const;

    void setInterval( double lowerBound, double upperBound );
    void setInterval( const QwtInterval& );
    QwtInterval interval() const;

    void setLowerBound( double );
    double lowerBound() const;

    void setUpperBound( double );
    double upperBound() const;

    double range() const;

    bool contains( double value ) const;

    void setTicks( int tickType, const QList< double >& );
    QList< double > ticks( int tickType ) const;

    bool isEmpty() const;
    bool isIncreasing() const;

    void invert();
    QwtScaleDiv inverted() const;

    QwtScaleDiv bounded( double lowerBound, double upperBound ) const;

  private:
    double m_lowerBound;
    double m_upperBound;
    QList< double > m_ticks[NTickTypes];
};

Q_DECLARE_TYPEINFO( QwtScaleDiv, Q_MOVABLE_TYPE );

#ifndef QT_NO_DEBUG_STREAM
QWT_EXPORT QDebug operator<<( QDebug, const QwtScaleDiv& );
#endif

#endif
