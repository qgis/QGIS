/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_picker.h"
#include "qwt_plot.h"
#include "qwt_text.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_picker_machine.h"

class QwtPlotPicker::PrivateData
{
  public:
    PrivateData():
        xAxisId( -1 ),
        yAxisId( -1 )
    {
    }

    QwtAxisId xAxisId;
    QwtAxisId yAxisId;
};

/*!
   \brief Create a plot picker

   The picker is set to those x- and y-axis of the plot
   that are enabled. If both or no x-axis are enabled, the picker
   is set to QwtAxis::XBottom. If both or no y-axis are
   enabled, it is set to QwtAxis::YLeft.

   \param canvas Plot canvas to observe, also the parent object

   \sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 */

QwtPlotPicker::QwtPlotPicker( QWidget* canvas )
    : QwtPicker( canvas )
{
    m_data = new PrivateData;

    if ( !canvas )
        return;

    const QwtPlot* plot = QwtPlotPicker::plot();
    // attach axes

    using namespace QwtAxis;

    int xAxis = XBottom;
    if ( !plot->isAxisVisible( XBottom ) && plot->isAxisVisible( XTop ) )
        xAxis = XTop;

    int yAxis = YLeft;
    if ( !plot->isAxisVisible( YLeft ) && plot->isAxisVisible( YRight ) )
        yAxis = YRight;

    setAxes( xAxis, yAxis );
}

/*!
   Create a plot picker

   \param xAxisId X axis of the picker
   \param yAxisId Y axis of the picker
   \param canvas Plot canvas to observe, also the parent object

   \sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 */
QwtPlotPicker::QwtPlotPicker( QwtAxisId xAxisId, QwtAxisId yAxisId, QWidget* canvas )
    : QwtPicker( canvas )
{
    m_data = new PrivateData;
    m_data->xAxisId = xAxisId;
    m_data->yAxisId = yAxisId;
}

/*!
   Create a plot picker

   \param xAxis X axis of the picker
   \param yAxis Y axis of the picker
   \param rubberBand Rubber band style
   \param trackerMode Tracker mode
   \param canvas Plot canvas to observe, also the parent object

   \sa QwtPicker, QwtPicker::setSelectionFlags(), QwtPicker::setRubberBand(),
      QwtPicker::setTrackerMode

   \sa QwtPlot::autoReplot(), QwtPlot::replot(), scaleRect()
 */
QwtPlotPicker::QwtPlotPicker( QwtAxisId xAxisId, QwtAxisId yAxisId,
        RubberBand rubberBand, DisplayMode trackerMode, QWidget* canvas )
    : QwtPicker( rubberBand, trackerMode, canvas )
{
    m_data = new PrivateData;
    m_data->xAxisId = xAxisId;
    m_data->yAxisId = yAxisId;
}

//! Destructor
QwtPlotPicker::~QwtPlotPicker()
{
    delete m_data;
}

//! \return Observed plot canvas
QWidget* QwtPlotPicker::canvas()
{
    return parentWidget();
}

//! \return Observed plot canvas
const QWidget* QwtPlotPicker::canvas() const
{
    return parentWidget();
}

//! \return Plot widget, containing the observed plot canvas
QwtPlot* QwtPlotPicker::plot()
{
    QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >( w );
}

//! \return Plot widget, containing the observed plot canvas
const QwtPlot* QwtPlotPicker::plot() const
{
    const QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >( w );
}

/*!
   \return Normalized bounding rectangle of the axes
   \sa QwtPlot::autoReplot(), QwtPlot::replot().
 */
QRectF QwtPlotPicker::scaleRect() const
{
    QRectF rect;

    if ( plot() )
    {
        const QwtScaleDiv& xs = plot()->axisScaleDiv( xAxis() );
        const QwtScaleDiv& ys = plot()->axisScaleDiv( yAxis() );

        rect = QRectF( xs.lowerBound(), ys.lowerBound(),
            xs.range(), ys.range() );
        rect = rect.normalized();
    }

    return rect;
}

/*!
   Set the x and y axes of the picker

   \param xAxisId X axis
   \param yAxisId Y axis
 */
void QwtPlotPicker::setAxes( QwtAxisId xAxisId, QwtAxisId yAxisId )
{
    const QwtPlot* plt = plot();
    if ( !plt )
        return;

    if ( xAxisId != m_data->xAxisId || yAxisId != m_data->yAxisId )
    {
        m_data->xAxisId = xAxisId;
        m_data->yAxisId = yAxisId;
    }
}

//! Return x axis
QwtAxisId QwtPlotPicker::xAxis() const
{
    return m_data->xAxisId;
}

//! Return y axis
QwtAxisId QwtPlotPicker::yAxis() const
{
    return m_data->yAxisId;
}

/*!
   Translate a pixel position into a position string

   \param pos Position in pixel coordinates
   \return Position string
 */
QwtText QwtPlotPicker::trackerText( const QPoint& pos ) const
{
    if ( plot() == NULL )
        return QwtText();

    return trackerTextF( invTransform( pos ) );
}

/*!
   \brief Translate a position into a position string

   In case of HLineRubberBand the label is the value of the
   y position, in case of VLineRubberBand the value of the x position.
   Otherwise the label contains x and y position separated by a ',' .

   The format for the double to string conversion is "%.4f".

   \param pos Position
   \return Position string
 */
QwtText QwtPlotPicker::trackerTextF( const QPointF& pos ) const
{
    QString text;

    switch ( rubberBand() )
    {
        case HLineRubberBand:
            text = QString::number( pos.y(), 'f', 4 );
            break;
        case VLineRubberBand:
            text = QString::number( pos.x(), 'f', 4 );
            break;
        default:
            text = QString::number( pos.x(), 'f', 4 )
                + ", " + QString::number( pos.y(), 'f', 4 );
    }
    return QwtText( text );
}

/*!
   Append a point to the selection and update rubber band and tracker.

   \param pos Additional point
   \sa isActive, begin(), end(), move(), appended()

   \note The appended(const QPoint &), appended(const QDoublePoint &)
        signals are emitted.
 */
void QwtPlotPicker::append( const QPoint& pos )
{
    QwtPicker::append( pos );
    Q_EMIT appended( invTransform( pos ) );
}

/*!
   Move the last point of the selection

   \param pos New position
   \sa isActive, begin(), end(), append()

   \note The moved(const QPoint &), moved(const QDoublePoint &)
        signals are emitted.
 */
void QwtPlotPicker::move( const QPoint& pos )
{
    QwtPicker::move( pos );
    Q_EMIT moved( invTransform( pos ) );
}

/*!
   Close a selection setting the state to inactive.

   \param ok If true, complete the selection and emit selected signals
            otherwise discard the selection.
   \return True if the selection has been accepted, false otherwise
 */

bool QwtPlotPicker::end( bool ok )
{
    ok = QwtPicker::end( ok );
    if ( !ok )
        return false;

    QwtPlot* plot = QwtPlotPicker::plot();
    if ( !plot )
        return false;

    const QPolygon points = selection();
    if ( points.count() == 0 )
        return false;

    QwtPickerMachine::SelectionType selectionType =
        QwtPickerMachine::NoSelection;

    if ( stateMachine() )
        selectionType = stateMachine()->selectionType();

    switch ( selectionType )
    {
        case QwtPickerMachine::PointSelection:
        {
            const QPointF pos = invTransform( points.first() );
            Q_EMIT selected( pos );
            break;
        }
        case QwtPickerMachine::RectSelection:
        {
            if ( points.count() >= 2 )
            {
                const QPoint p1 = points.first();
                const QPoint p2 = points.last();

                const QRect rect = QRect( p1, p2 ).normalized();
                Q_EMIT selected( invTransform( rect ) );
            }
            break;
        }
        case QwtPickerMachine::PolygonSelection:
        {
            QVector< QPointF > dpa( points.count() );
            for ( int i = 0; i < points.count(); i++ )
                dpa[i] = invTransform( points[i] );

            Q_EMIT selected( dpa );
        }
        default:
            break;
    }

    return true;
}

/*!
    Translate a rectangle from pixel into plot coordinates

    \return Rectangle in plot coordinates
    \sa transform()
 */
QRectF QwtPlotPicker::invTransform( const QRect& rect ) const
{
    const QwtScaleMap xMap = plot()->canvasMap( xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( yAxis() );

    return QwtScaleMap::invTransform( xMap, yMap, rect );
}

/*!
    Translate a rectangle from plot into pixel coordinates
    \return Rectangle in pixel coordinates
    \sa invTransform()
 */
QRect QwtPlotPicker::transform( const QRectF& rect ) const
{
    const QwtScaleMap xMap = plot()->canvasMap( xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( yAxis() );

    return QwtScaleMap::transform( xMap, yMap, rect ).toRect();
}

/*!
    Translate a point from pixel into plot coordinates
    \return Point in plot coordinates
    \sa transform()
 */
QPointF QwtPlotPicker::invTransform( const QPoint& pos ) const
{
    const QwtScaleMap xMap = plot()->canvasMap( xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( yAxis() );

    return QPointF(
        xMap.invTransform( pos.x() ),
        yMap.invTransform( pos.y() )
        );
}

/*!
    Translate a point from plot into pixel coordinates
    \return Point in pixel coordinates
    \sa invTransform()
 */
QPoint QwtPlotPicker::transform( const QPointF& pos ) const
{
    const QwtScaleMap xMap = plot()->canvasMap( xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( yAxis() );

    const QPointF p( xMap.transform( pos.x() ), yMap.transform( pos.y() ) );

    return p.toPoint();
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_plot_picker.cpp"
#endif
