/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CUBIC_H
#define QWT_SPLINE_CUBIC_H

#include "qwt_global.h"
#include "qwt_spline.h"

/*!
   \brief A cubic spline

   A cubic spline is a spline with C2 continuity at all control points.
   It is a non local spline, what means that all polynomials are changing
   when one control point has changed.

   The implementation is based on the fact, that the continuity condition
   means an equation with 3 unknowns for 3 adjacent points. The equation
   system can be resolved by defining start/end conditions, that allow
   substituting of one of the unknowns for the start/end equations.

   Resolving the equation system is a 2 pass algorithm, requiring more CPU costs
   than all other implemented type of splines.

   \todo The implementation is not numerical stable
 */
class QWT_EXPORT QwtSplineCubic : public QwtSplineC2
{
  public:
    QwtSplineCubic();
    virtual ~QwtSplineCubic();

    virtual uint locality() const QWT_OVERRIDE;

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& points ) const QWT_OVERRIDE;

    // calculating the parametric equations
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< double > slopes( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< double > curvatures( const QPolygonF& ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
