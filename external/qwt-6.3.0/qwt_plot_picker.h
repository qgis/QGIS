/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_PICKER_H
#define QWT_PLOT_PICKER_H

#include "qwt_global.h"
#include "qwt_picker.h"
#include "qwt_axis_id.h"

class QwtPlot;
class QPointF;
class QRectF;

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/*!
   \brief QwtPlotPicker provides selections on a plot canvas

   QwtPlotPicker is a QwtPicker tailored for selections on
   a plot canvas. It is set to a x-Axis and y-Axis and
   translates all pixel coordinates into this coordinate system.
 */

class QWT_EXPORT QwtPlotPicker : public QwtPicker
{
    Q_OBJECT

  public:
    explicit QwtPlotPicker( QWidget* canvas );
    virtual ~QwtPlotPicker();

    explicit QwtPlotPicker( QwtAxisId xAxisId, QwtAxisId yAxisId, QWidget* );

    explicit QwtPlotPicker( QwtAxisId xAxisId, QwtAxisId yAxisId,
        RubberBand rubberBand, DisplayMode trackerMode, QWidget* );

    virtual void setAxes( QwtAxisId xAxisId, QwtAxisId yAxisId );

    QwtAxisId xAxis() const;
    QwtAxisId yAxis() const;

    QwtPlot* plot();
    const QwtPlot* plot() const;

    QWidget* canvas();
    const QWidget* canvas() const;

  Q_SIGNALS:

    /*!
       A signal emitted in case of QwtPickerMachine::PointSelection.
       \param pos Selected point
     */
    void selected( const QPointF& pos );

    /*!
       A signal emitted in case of QwtPickerMachine::RectSelection.
       \param rect Selected rectangle
     */
    void selected( const QRectF& rect );

    /*!
       A signal emitting the selected points,
       at the end of a selection.

       \param pa Selected points
     */
    void selected( const QVector< QPointF >& pa );

    /*!
       A signal emitted when a point has been appended to the selection

       \param pos Position of the appended point.
       \sa append(). moved()
     */
    void appended( const QPointF& pos );

    /*!
       A signal emitted whenever the last appended point of the
       selection has been moved.

       \param pos Position of the moved last point of the selection.
       \sa move(), appended()
     */
    void moved( const QPointF& pos );

  protected:
    QRectF scaleRect() const;

    QRectF invTransform( const QRect& ) const;
    QRect transform( const QRectF& ) const;

    QPointF invTransform( const QPoint& ) const;
    QPoint transform( const QPointF& ) const;

    virtual QwtText trackerText( const QPoint& ) const QWT_OVERRIDE;
    virtual QwtText trackerTextF( const QPointF& ) const;

    virtual void move( const QPoint& ) QWT_OVERRIDE;
    virtual void append( const QPoint& ) QWT_OVERRIDE;
    virtual bool end( bool ok = true ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
