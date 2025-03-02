/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_MAGNIFIER_H
#define QWT_PLOT_MAGNIFIER_H

#include "qwt_global.h"
#include "qwt_axis_id.h"
#include "qwt_magnifier.h"

class QwtPlot;

/*!
   \brief QwtPlotMagnifier provides zooming, by magnifying in steps.

   Using QwtPlotMagnifier a plot can be zoomed in/out in steps using
   keys, the mouse wheel or moving a mouse button in vertical direction.

   Together with QwtPlotZoomer and QwtPlotPanner it is possible to implement
   individual and powerful navigation of the plot canvas.

   \sa QwtPlotZoomer, QwtPlotPanner, QwtPlot
 */
class QWT_EXPORT QwtPlotMagnifier : public QwtMagnifier
{
    Q_OBJECT

  public:
    explicit QwtPlotMagnifier( QWidget* );
    virtual ~QwtPlotMagnifier();

    void setAxisEnabled( QwtAxisId, bool on );
    bool isAxisEnabled( QwtAxisId ) const;

    QWidget* canvas();
    const QWidget* canvas() const;

    QwtPlot* plot();
    const QwtPlot* plot() const;

  public Q_SLOTS:
    virtual void rescale( double factor ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
