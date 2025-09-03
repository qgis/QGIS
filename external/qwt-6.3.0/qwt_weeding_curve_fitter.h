/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_WEEDING_CURVE_FITTER_H
#define QWT_WEEDING_CURVE_FITTER_H

#include "qwt_curve_fitter.h"

/*!
   \brief A curve fitter implementing Douglas and Peucker algorithm

   The purpose of the Douglas and Peucker algorithm is that given a 'curve'
   composed of line segments to find a curve not too dissimilar but that
   has fewer points. The algorithm defines 'too dissimilar' based on the
   maximum distance (tolerance) between the original curve and the
   smoothed curve.

   The runtime of the algorithm increases non linear ( worst case O( n*n ) )
   and might be very slow for huge polygons. To avoid performance issues
   it might be useful to split the polygon ( setChunkSize() ) and to run the algorithm
   for these smaller parts. The disadvantage of having no interpolation
   at the borders is for most use cases irrelevant.

   The smoothed curve consists of a subset of the points that defined the
   original curve.

   In opposite to QwtSplineCurveFitter the Douglas and Peucker algorithm reduces
   the number of points. By adjusting the tolerance parameter according to the
   axis scales QwtSplineCurveFitter can be used to implement different
   level of details to speed up painting of curves of many points.
 */
class QWT_EXPORT QwtWeedingCurveFitter : public QwtCurveFitter
{
  public:
    explicit QwtWeedingCurveFitter( double tolerance = 1.0 );
    virtual ~QwtWeedingCurveFitter();

    void setTolerance( double );
    double tolerance() const;

    void setChunkSize( uint );
    uint chunkSize() const;

    virtual QPolygonF fitCurve( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QPainterPath fitCurvePath( const QPolygonF& ) const QWT_OVERRIDE;

  private:
    virtual QPolygonF simplify( const QPolygonF& ) const;

    class Line;

    class PrivateData;
    PrivateData* m_data;
};

#endif
