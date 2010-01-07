/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <math.h>
#include <qevent.h>
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_polar_magnifier.h"

class QwtPolarMagnifier::PrivateData
{
public:
    PrivateData():
        unzoomKey(Qt::Key_Home),
#if QT_VERSION < 0x040000
        unzoomKeyModifiers(Qt::NoButton)
#else
        unzoomKeyModifiers(Qt::NoModifier)
#endif
    {
    }

    int unzoomKey;
    int unzoomKeyModifiers;
};

/*!
   Constructor
   \param canvas Plot canvas to be magnified
*/
QwtPolarMagnifier::QwtPolarMagnifier(QwtPolarCanvas *canvas):
    QwtMagnifier(canvas)
{
    d_data = new PrivateData();
}

//! Destructor
QwtPolarMagnifier::~QwtPolarMagnifier()
{
    delete d_data;
}

/*!
   Assign key and modifiers, that are used for unzooming
   The default combination is Qt::Key_Home + Qt::NoModifier.

   \param key Key code
   \param modifiers Modifiers
   \sa getUnzoomKey(), QwtPolarPlot::unzoom()
*/
void QwtPolarMagnifier::setUnzoomKey(int key, int modifiers)
{
    d_data->unzoomKey = key;
    d_data->unzoomKeyModifiers = modifiers;
}

/*!
   \return Key, and modifiers that are used for unzooming

   \param key Key code
   \param modifiers Modifiers
   \sa setUnzoomKey(), QwtPolarPlot::unzoom()
*/
void QwtPolarMagnifier::getUnzoomKey(int &key, int &modifiers) const
{
    key = d_data->unzoomKey;
    modifiers = d_data->unzoomKeyModifiers;
}

//! Return observed plot canvas
QwtPolarCanvas *QwtPolarMagnifier::canvas()
{
    QWidget *w = parentWidget();
    if ( w && w->inherits("QwtPolarCanvas") )
        return (QwtPolarCanvas *)w;

    return NULL;
}

//! \return observed plot canvas
const QwtPolarCanvas *QwtPolarMagnifier::canvas() const
{
    return ((QwtPolarMagnifier *)this)->canvas();
}

//! Return observed plot
QwtPolarPlot *QwtPolarMagnifier::plot()
{
    QwtPolarCanvas *c = canvas();
    if ( c )
        return c->plot();

    return NULL;
}

//! Return observed plot
const QwtPolarPlot *QwtPolarMagnifier::plot() const
{
    return ((QwtPolarMagnifier *)this)->plot();
}

/*!
  Handle a key press event for the observed widget.

  \param ke Key event
*/
void QwtPolarMagnifier::widgetKeyPressEvent(QKeyEvent *ke)
{
    const int key = ke->key();
#if QT_VERSION < 0x040000
    const int state = ke->state();
#else
    const int state = ke->modifiers();
#endif

    if ( key == d_data->unzoomKey &&
        state == d_data->unzoomKeyModifiers )
    {
        unzoom();
        return;
    }

    QwtMagnifier::widgetKeyPressEvent(ke);
}

/*! 
   Zoom in/out the zoomed area
   \param factor A value < 1.0 zooms in, a value > 1.0 zooms out.
*/
void QwtPolarMagnifier::rescale(double factor)
{
    factor = qwtAbs(factor);
    if ( factor == 1.0 || factor == 0.0 )
        return;

    QwtPolarPlot* plt = plot();
    if ( plt == NULL )
        return;

    QwtPolarPoint zoomPos; 
    double newZoomFactor = plt->zoomFactor() * factor;

    if ( newZoomFactor >= 1.0 )
        newZoomFactor = 1.0;
    else
        zoomPos = plt->zoomPos();
    
    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->zoom(zoomPos, newZoomFactor);

    plt->setAutoReplot(autoReplot);
    plt->replot();
}

//! Unzoom the plot widget
void QwtPolarMagnifier::unzoom()
{
    QwtPolarPlot* plt = plot();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->unzoom();

    plt->setAutoReplot(autoReplot);
    plt->replot();
}
