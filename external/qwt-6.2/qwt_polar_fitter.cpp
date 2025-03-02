/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_fitter.h"
#include <qpolygon.h>
#include <qpainterpath.h>

class QwtPolarFitter::PrivateData
{
  public:
    PrivateData()
        : stepCount( 5 )
    {
    }

    int stepCount;
};

/*!
   Constructor

   \param stepCount Number of points, that will be inserted between 2 points
   \sa setStepCount()
 */
QwtPolarFitter::QwtPolarFitter( int stepCount )
    : QwtCurveFitter( QwtPolarFitter::Polygon )
{
    m_data = new PrivateData;
    m_data->stepCount = stepCount;
}

//! Destructor
QwtPolarFitter::~QwtPolarFitter()
{
    delete m_data;
}

/*!
   Assign the number of points, that will be inserted between 2 points
   The default value is 5.

   \param stepCount Number of steps

   \sa stepCount()
 */
void QwtPolarFitter::setStepCount( int stepCount )
{
    m_data->stepCount = qMax( stepCount, 0 );
}

/*!
   \return Number of points, that will be inserted between 2 points
   \sa setStepCount()
 */
int QwtPolarFitter::stepCount() const
{
    return m_data->stepCount;
}

/*!
   Insert stepCount() number of additional points between 2 elements
   of points.

   \param points Array of points
   \return Array of points including the additional points
 */
QPolygonF QwtPolarFitter::fitCurve( const QPolygonF& points ) const
{
    if ( m_data->stepCount <= 0 || points.size() <= 1 )
        return points;

    QPolygonF fittedPoints;

    int numPoints = points.size() + ( points.size() - 1 ) * m_data->stepCount;

    fittedPoints.resize( numPoints );

    int index = 0;
    fittedPoints[index++] = points[0];
    for ( int i = 1; i < points.size(); i++ )
    {
        const QPointF& p1 = points[i - 1];
        const QPointF& p2 = points[i];

        const double dx = ( p2.x() - p1.x() ) / m_data->stepCount;
        const double dy = ( p2.y() - p1.y() ) / m_data->stepCount;
        for ( int j = 1; j <= m_data->stepCount; j++ )
        {
            const double x = p1.x() + j * dx;
            const double y = p1.y() + j * dy;

            fittedPoints[index++] = QPointF( x, y );
        }
    }
    fittedPoints.resize( index );

    return fittedPoints;
}

/*!
   \param points Series of data points
   \return Curve path
   \sa fitCurve()
 */
QPainterPath QwtPolarFitter::fitCurvePath( const QPolygonF& points ) const
{
    QPainterPath path;
    path.addPolygon( fitCurve( points ) );
    return path;
}
