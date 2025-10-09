/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_LOCAL_H
#define QWT_SPLINE_LOCAL_H

#include "qwt_global.h"
#include "qwt_spline.h"

/*!
   \brief A spline with C1 continuity

   QwtSplineLocal offers several standard algorithms for interpolating
   a curve with polynomials having C1 continuity at the control points.
   All algorithms are local in a sense, that changing one control point
   only few polynomials.
 */
class QWT_EXPORT QwtSplineLocal : public QwtSplineC1
{
  public:
    /*!
        \brief Spline interpolation type

        All type of spline interpolations are lightweight algorithms
        calculating the slopes at a point by looking 1 or 2 points back
        and ahead.
     */
    enum Type
    {
        /*!
           A cardinal spline

           The cardinal spline interpolation is a very cheap calculation with
           a locality of 1.
         */
        Cardinal,

        /*!
           Parabolic blending is a cheap calculation with a locality of 1. Sometimes
           it is also called Cubic Bessel interpolation.
         */
        ParabolicBlending,

        /*!
           The algorithm of H.Akima is a calculation with a locality of 2.
         */
        Akima,

        /*!
           Piecewise Cubic Hermite Interpolating Polynomial (PCHIP) is an algorithm
           that is popular because of being offered by MATLAB.

           It preserves the shape of the data and respects monotonicity. It has a
           locality of 1.
         */
        PChip
    };

    QwtSplineLocal( Type type );
    virtual ~QwtSplineLocal();

    Type type() const;

    virtual uint locality() const QWT_OVERRIDE;

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const QWT_OVERRIDE;

    // calculating the parametric equations
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< double > slopes( const QPolygonF& ) const QWT_OVERRIDE;

  private:
    const Type m_type;
};

#endif
