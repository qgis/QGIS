/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_PLEASING_H
#define QWT_SPLINE_PLEASING_H

#include "qwt_spline.h"

/*!
   \brief A spline with G1 continuity

   QwtSplinePleasing is some sort of cardinal spline, with
   non C1 continuous extra rules for narrow angles. It has a locality of 2.

   \note The algorithm is the one offered by a popular office package.
 */
class QWT_EXPORT QwtSplinePleasing : public QwtSplineG1
{
  public:
    QwtSplinePleasing();
    virtual ~QwtSplinePleasing();

    virtual uint locality() const QWT_OVERRIDE;

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const QWT_OVERRIDE;
};

#endif
