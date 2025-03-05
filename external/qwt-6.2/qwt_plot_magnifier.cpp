/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_plot_magnifier.h"

class QwtPlotMagnifier::PrivateData
{
  public:
    PrivateData()
    {
        for ( int axis = 0; axis < QwtAxis::AxisPositions; axis++ )
            isAxisEnabled[axis] = true;
    }

    bool isAxisEnabled[QwtAxis::AxisPositions];
};

/*!
   Constructor
   \param canvas Plot canvas to be magnified
 */
QwtPlotMagnifier::QwtPlotMagnifier( QWidget* canvas )
    : QwtMagnifier( canvas )
{
    m_data = new PrivateData();
}

//! Destructor
QwtPlotMagnifier::~QwtPlotMagnifier()
{
    delete m_data;
}

/*!
   \brief En/Disable an axis

   Only Axes that are enabled will be zoomed.
   All other axes will remain unchanged.

   \param axisId Axis
   \param on On/Off

   \sa isAxisEnabled()
 */
void QwtPlotMagnifier::setAxisEnabled( QwtAxisId axisId, bool on )
{
    if ( QwtAxis::isValid( axisId ) )
        m_data->isAxisEnabled[axisId] = on;
}

/*!
   Test if an axis is enabled

   \param axisId Axis
   \return True, if the axis is enabled

   \sa setAxisEnabled()
 */
bool QwtPlotMagnifier::isAxisEnabled( QwtAxisId axisId ) const
{
    if ( QwtAxis::isValid( axisId ) )
        return m_data->isAxisEnabled[axisId];

    return true;
}

//! Return observed plot canvas
QWidget* QwtPlotMagnifier::canvas()
{
    return parentWidget();
}

//! Return Observed plot canvas
const QWidget* QwtPlotMagnifier::canvas() const
{
    return parentWidget();
}

//! Return plot widget, containing the observed plot canvas
QwtPlot* QwtPlotMagnifier::plot()
{
    QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >( w );
}

//! Return plot widget, containing the observed plot canvas
const QwtPlot* QwtPlotMagnifier::plot() const
{
    const QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >( w );
}

/*!
   Zoom in/out the axes scales
   \param factor A value < 1.0 zooms in, a value > 1.0 zooms out.
 */
void QwtPlotMagnifier::rescale( double factor )
{
    QwtPlot* plt = plot();
    if ( plt == NULL )
        return;

    factor = qAbs( factor );
    if ( factor == 1.0 || factor == 0.0 )
        return;

    bool doReplot = false;

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot( false );

    for ( int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++ )
    {
        {
            const QwtAxisId axisId( axisPos );

            if ( isAxisEnabled( axisId ) )
            {
                const QwtScaleMap scaleMap = plt->canvasMap( axisId );

                double v1 = scaleMap.s1();
                double v2 = scaleMap.s2();

                if ( scaleMap.transformation() )
                {
                    // the coordinate system of the paint device is always linear

                    v1 = scaleMap.transform( v1 ); // scaleMap.p1()
                    v2 = scaleMap.transform( v2 ); // scaleMap.p2()
                }

                const double center = 0.5 * ( v1 + v2 );
                const double width_2 = 0.5 * ( v2 - v1 ) * factor;

                v1 = center - width_2;
                v2 = center + width_2;

                if ( scaleMap.transformation() )
                {
                    v1 = scaleMap.invTransform( v1 );
                    v2 = scaleMap.invTransform( v2 );
                }

                plt->setAxisScale( axisId, v1, v2 );
                doReplot = true;
            }
        }
    }

    plt->setAutoReplot( autoReplot );

    if ( doReplot )
        plt->replot();
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_plot_magnifier.cpp"
#endif
