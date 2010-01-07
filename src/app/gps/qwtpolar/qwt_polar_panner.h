/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_PANNER_H
#define QWT_POLAR_PANNER_H 1

#include "qwt_polar_global.h"
#include "qwt_panner.h"

class QwtPolarPlot;
class QwtPolarCanvas;

/*!
  \brief QwtPolarPanner provides panning of a polar plot canvas

  QwtPolarPanner is a panner for a QwtPolarCanvas, that
  adjusts the visible area after dropping
  the canvas on its new position.

  Together with QwtPolarMagnifier individual ways
  of navigating on a QwtPolarPlot widget can be implemented easily.

  \sa QwtPolarMagnifier
*/

class QWT_POLAR_EXPORT QwtPolarPanner: public QwtPanner
{
    Q_OBJECT

  public:
    explicit QwtPolarPanner( QwtPolarCanvas * );
    virtual ~QwtPolarPanner();

    QwtPolarPlot *plot();
    const QwtPolarPlot *plot() const;

    QwtPolarCanvas *canvas();
    const QwtPolarCanvas *canvas() const;

  protected slots:
    virtual void movePlot( int dx, int dy );

  protected:
    virtual void widgetMousePressEvent( QMouseEvent * );
};

#endif
