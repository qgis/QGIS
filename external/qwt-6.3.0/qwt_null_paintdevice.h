/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_NULL_PAINT_DEVICE_H
#define QWT_NULL_PAINT_DEVICE_H

#include "qwt_global.h"

#include <qpaintdevice.h>
#include <qpaintengine.h>

/*!
   \brief A null paint device doing nothing

   Sometimes important layout/rendering geometries are not
   available or changeable from the public Qt class interface.
   ( f.e hidden in the style implementation ).

   QwtNullPaintDevice can be used to manipulate or filter out
   this information by analyzing the stream of paint primitives.

   F.e. QwtNullPaintDevice is used by QwtPlotCanvas to identify
   styled backgrounds with rounded corners.
 */

class QWT_EXPORT QwtNullPaintDevice : public QPaintDevice
{
  public:
    /*!
       \brief Render mode

       \sa setMode(), mode()
     */
    enum Mode
    {
        /*!
           All vector graphic primitives are painted by
           the corresponding draw methods
         */
        NormalMode,

        /*!
           Vector graphic primitives ( beside polygons ) are mapped to a QPainterPath
           and are painted by drawPath. In PathMode mode
           only a few draw methods are called:

           - drawPath()
           - drawPixmap()
           - drawImage()
           - drawPolygon()
         */
        PolygonPathMode,

        /*!
           Vector graphic primitives are mapped to a QPainterPath
           and are painted by drawPath. In PathMode mode
           only a few draw methods are called:

           - drawPath()
           - drawPixmap()
           - drawImage()
         */
        PathMode
    };

    QwtNullPaintDevice();
    virtual ~QwtNullPaintDevice();

    void setMode( Mode );
    Mode mode() const;

    virtual QPaintEngine* paintEngine() const QWT_OVERRIDE;

    virtual int metric( PaintDeviceMetric ) const QWT_OVERRIDE;

    virtual void drawRects(const QRect*, int );
    virtual void drawRects(const QRectF*, int );

    virtual void drawLines(const QLine*, int );
    virtual void drawLines(const QLineF*, int );

    virtual void drawEllipse(const QRectF&);
    virtual void drawEllipse(const QRect&);

    virtual void drawPath(const QPainterPath&);

    virtual void drawPoints(const QPointF*, int );
    virtual void drawPoints(const QPoint*, int );

    virtual void drawPolygon( const QPointF*, int,
        QPaintEngine::PolygonDrawMode );

    virtual void drawPolygon( const QPoint*, int,
        QPaintEngine::PolygonDrawMode );

    virtual void drawPixmap(const QRectF&,
        const QPixmap&, const QRectF&);

    virtual void drawTextItem(const QPointF&, const QTextItem&);

    virtual void drawTiledPixmap(const QRectF&,
        const QPixmap&, const QPointF& );

    virtual void drawImage(const QRectF&, const QImage&,
        const QRectF&, Qt::ImageConversionFlags );

    virtual void updateState( const QPaintEngineState& );

  protected:
    //! \return Size needed to implement metric()
    virtual QSize sizeMetrics() const = 0;

  private:
    class PaintEngine;
    PaintEngine* m_engine;

    class PrivateData;
    PrivateData* m_data;
};

#endif
