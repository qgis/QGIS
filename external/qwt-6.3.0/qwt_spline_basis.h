/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_BASIS_H
#define QWT_SPLINE_BASIS_H

#include "qwt_global.h"
#include "qwt_spline.h"

/*!
   \brief An approximation using a basis spline

   QwtSplineBasis approximates a set of points by a polynomials with C2 continuity
   ( = first and second derivatives are equal ) at the end points.

   The end points of the spline do not match the original points.
 */
class QWT_EXPORT QwtSplineBasis : public QwtSpline
{
  public:
    QwtSplineBasis();
    virtual ~QwtSplineBasis();

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual uint locality() const QWT_OVERRIDE;
};

#endif
