/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_MAGNIFIER_H
#define QWT_POLAR_MAGNIFIER_H

#include "qwt_global.h"
#include "qwt_magnifier.h"

class QwtPolarPlot;
class QwtPolarCanvas;

/*!
   \brief QwtPolarMagnifier provides zooming, by magnifying in steps.

   Using QwtPlotMagnifier a plot can be zoomed in/out in steps using
   keys, the mouse wheel or moving a mouse button in vertical direction.

   Together with QwtPolarPanner it is possible to implement
   an individual navigation of the plot canvas.

   \sa QwtPolarPanner, QwtPolarPlot, QwtPolarCanvas
 */

class QWT_EXPORT QwtPolarMagnifier : public QwtMagnifier
{
    Q_OBJECT

  public:
    explicit QwtPolarMagnifier( QwtPolarCanvas* );
    virtual ~QwtPolarMagnifier();

    void setUnzoomKey( int key, int modifiers );
    void getUnzoomKey( int& key, int& modifiers ) const;

    QwtPolarPlot* plot();
    const QwtPolarPlot* plot() const;

    QwtPolarCanvas* canvas();
    const QwtPolarCanvas* canvas() const;

  public Q_SLOTS:
    virtual void rescale( double factor ) QWT_OVERRIDE;
    void unzoom();

  protected:
    virtual void widgetKeyPressEvent( QKeyEvent* ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
