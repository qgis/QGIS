/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CURVE_FITTER_H
#define QWT_SPLINE_CURVE_FITTER_H

#include "qwt_curve_fitter.h"

class QwtSpline;

/*!
   \brief A curve fitter using a spline interpolation

   The default setting for the spline is a cardinal spline with
   uniform parametrization.

   \sa QwtSpline, QwtSplineLocal
 */
class QWT_EXPORT QwtSplineCurveFitter : public QwtCurveFitter
{
  public:
    QwtSplineCurveFitter();
    virtual ~QwtSplineCurveFitter();

    void setSpline( QwtSpline* );

    const QwtSpline* spline() const;
    QwtSpline* spline();

    virtual QPolygonF fitCurve( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QPainterPath fitCurvePath( const QPolygonF& ) const QWT_OVERRIDE;

  private:
    QwtSpline* m_spline;
};

#endif
