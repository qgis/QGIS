/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_RESCALER_H
#define QWT_PLOT_RESCALER_H

#include "qwt_global.h"
#include "qwt_plot.h"

#include <qobject.h>

class QwtPlot;
class QwtInterval;
class QResizeEvent;

/*!
    \brief QwtPlotRescaler takes care of fixed aspect ratios for plot scales

    QwtPlotRescaler auto adjusts the axes of a QwtPlot according
    to fixed aspect ratios.
 */

class QWT_EXPORT QwtPlotRescaler : public QObject
{
    Q_OBJECT

  public:
    /*!
       The rescale policy defines how to rescale the reference axis and
       their depending axes.

       \sa ExpandingDirection, setIntervalHint()
     */
    enum RescalePolicy
    {
        /*!
           The interval of the reference axis remains unchanged, when the
           geometry of the canvas changes. All other axes
           will be adjusted according to their aspect ratio.
         */
        Fixed,

        /*!
           The interval of the reference axis will be shrunk/expanded,
           when the geometry of the canvas changes. All other axes
           will be adjusted according to their aspect ratio.

           The interval, that is represented by one pixel is fixed.

         */
        Expanding,

        /*!
           The intervals of the axes are calculated, so that all axes include
           their interval hint.
         */
        Fitting
    };

    /*!
       When rescalePolicy() is set to Expanding its direction depends
       on ExpandingDirection
     */
    enum ExpandingDirection
    {
        //! The upper limit of the scale is adjusted
        ExpandUp,

        //! The lower limit of the scale is adjusted
        ExpandDown,

        //! Both limits of the scale are adjusted
        ExpandBoth
    };

    explicit QwtPlotRescaler( QWidget* canvas,
        QwtAxisId referenceAxis = QwtAxis::XBottom,
        RescalePolicy = Expanding );

    virtual ~QwtPlotRescaler();

    void setEnabled( bool );
    bool isEnabled() const;

    void setRescalePolicy( RescalePolicy );
    RescalePolicy rescalePolicy() const;

    void setExpandingDirection( ExpandingDirection );
    void setExpandingDirection( QwtAxisId, ExpandingDirection );
    ExpandingDirection expandingDirection( QwtAxisId ) const;

    void setReferenceAxis( QwtAxisId );
    QwtAxisId referenceAxis() const;

    void setAspectRatio( double ratio );
    void setAspectRatio( QwtAxisId, double ratio );
    double aspectRatio( QwtAxisId ) const;

    void setIntervalHint( QwtAxisId, const QwtInterval& );
    QwtInterval intervalHint( QwtAxisId ) const;

    QWidget* canvas();
    const QWidget* canvas() const;

    QwtPlot* plot();
    const QwtPlot* plot() const;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

    void rescale() const;

  protected:
    virtual void canvasResizeEvent( QResizeEvent* );

    virtual void rescale( const QSize& oldSize, const QSize& newSize ) const;
    virtual QwtInterval expandScale(
        QwtAxisId, const QSize& oldSize, const QSize& newSize ) const;

    virtual QwtInterval syncScale(
        QwtAxisId, const QwtInterval& reference, const QSize& size ) const;

    virtual void updateScales(
        QwtInterval intervals[QwtAxis::AxisPositions] ) const;

    Qt::Orientation orientation( QwtAxisId ) const;
    QwtInterval interval( QwtAxisId ) const;
    QwtInterval expandInterval( const QwtInterval&,
        double width, ExpandingDirection ) const;

  private:
    double pixelDist( QwtAxisId, const QSize& ) const;

    class AxisData;
    class PrivateData;
    PrivateData* m_data;
};

#endif
