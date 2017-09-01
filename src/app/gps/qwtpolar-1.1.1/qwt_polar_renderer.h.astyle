/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_RENDERER_H
#define QWT_POLAR_RENDERER_H 1

#include "qwt_polar_global.h"
#include <qobject.h>
#include <qsize.h>

class QwtPolarPlot;
class QRectF;
class QPainter;
class QPrinter;
class QPaintDevice;
#ifndef QWT_NO_POLAR_SVG
#ifdef QT_SVG_LIB
class QSvgGenerator;
#endif
#endif

/*!
  \brief Renderer for exporting a polar plot to a document, a printer
         or anything else, that is supported by QPainter/QPaintDevice
*/
class QWT_POLAR_EXPORT QwtPolarRenderer: public QObject
{
    Q_OBJECT

public:
    explicit QwtPolarRenderer( QObject *parent = NULL );
    virtual ~QwtPolarRenderer();

    void renderDocument( QwtPolarPlot *, const QString &format,
        const QSizeF &sizeMM, int resolution = 85 );

    void renderDocument( QwtPolarPlot *,
        const QString &title, const QString &format,
        const QSizeF &sizeMM, int resolution = 85 );

#ifndef QWT_NO_POLAR_SVG
#ifdef QT_SVG_LIB
#if QT_VERSION >= 0x040500
    void renderTo( QwtPolarPlot *, QSvgGenerator & ) const;
#endif
#endif
#endif
    void renderTo( QwtPolarPlot *, QPrinter & ) const;
    void renderTo( QwtPolarPlot *, QPaintDevice & ) const;

    virtual void render( QwtPolarPlot *,
        QPainter *, const QRectF &rect ) const;

    bool exportTo( QwtPolarPlot *, const QString &documentName,
        const QSizeF &sizeMM = QSizeF( 200, 200 ), int resolution = 85 );

    virtual void renderTitle( QPainter *, const QRectF & ) const;

    virtual void renderLegend(
        const QwtPolarPlot *, QPainter *, const QRectF & ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
