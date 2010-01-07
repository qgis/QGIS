/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_math.h"
#include "qwt_polar_fitter.h"

class QwtPolarFitter::PrivateData
{
public:
    PrivateData():
        stepCount(5)
    {
    }

    int stepCount;
};

/*! 
   Constructor

   \param stepCount Number of points, that will be inserted between 2 points
   \sa setStepCount()
*/
QwtPolarFitter::QwtPolarFitter(int stepCount)
{
    d_data = new PrivateData;
    d_data->stepCount = stepCount;
}

//! Destructor
QwtPolarFitter::~QwtPolarFitter()
{
    delete d_data;
}

/*!
   Assign the number of points, that will be inserted between 2 points
   The default value is 5.

   \param stepCount Number of steps

   \sa stepCount()
*/
void QwtPolarFitter::setStepCount(int stepCount)
{
    d_data->stepCount = qwtMax(stepCount, 0);
}

/*! 
   \return Number of points, that will be inserted between 2 points
   \sa setStepCount()
*/
int QwtPolarFitter::stepCount() const
{
    return d_data->stepCount;
}

/*!
   Insert stepCount() number of additional points between 2 elements
   of points.

   \param points Array of points
   \return Array of points including the additional points
*/
#if QT_VERSION < 0x040000
QwtArray<QwtDoublePoint> QwtPolarFitter::fitCurve(
    const QwtArray<QwtDoublePoint> &points) const
#else
QPolygonF QwtPolarFitter::fitCurve(
    const QPolygonF &points) const
#endif
{
    if ( d_data->stepCount <= 0 || points.size() <= 1 )
        return points;

#if QT_VERSION < 0x040000
    QwtArray<QwtDoublePoint> fittedPoints;
#else
    QPolygonF fittedPoints;
#endif

    int numPoints = points.size() + (points.size() - 1) * d_data->stepCount;

    fittedPoints.resize(numPoints);

    int index = 0;
    fittedPoints[index++] = points[0];
    for ( int i = 1; i < (int)points.size(); i++ )
    {
        const QwtDoublePoint &p1 = points[i-1];
        const QwtDoublePoint &p2 = points[i];

        const double dx = (p2.x() - p1.x()) / d_data->stepCount;
        const double dy = (p2.y() - p1.y()) / d_data->stepCount;
        for ( int j = 1; j <= d_data->stepCount; j++ )
        {
            const double x = p1.x() + j * dx;
            const double y = p1.y() + j * dy;

            fittedPoints[index++] = QwtDoublePoint(x, y);
        }
    }
    fittedPoints.resize(index);

    return fittedPoints;
}
