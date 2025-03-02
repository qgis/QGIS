/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PAINTER_H
#define QWT_PAINTER_H

#include "qwt_global.h"

#include <qpalette.h>
#include <qpolygon.h>
#include <qpen.h>

class QwtScaleMap;
class QwtColorMap;
class QwtInterval;

class QPainter;
class QBrush;
class QWidget;
class QImage;
class QPixmap;
class QFontMetrics;
class QFontMetricsF;
class QTextDocument;
class QPainterPath;

/*!
   \brief A collection of QPainter workarounds
 */
class QWT_EXPORT QwtPainter
{
  public:
    static void setPolylineSplitting( bool );
    static bool polylineSplitting();

    static void setRoundingAlignment( bool );
    static bool roundingAlignment();
    static bool roundingAlignment( const QPainter* );

    static void drawText( QPainter*, qreal x, qreal y, const QString& );
    static void drawText( QPainter*, const QPointF&, const QString& );
    static void drawText( QPainter*, qreal x, qreal y, qreal w, qreal h,
        int flags, const QString& );
    static void drawText( QPainter*, const QRectF&,
        int flags, const QString& );

#ifndef QT_NO_RICHTEXT
    static void drawSimpleRichText( QPainter*, const QRectF&,
        int flags, const QTextDocument& );
#endif

    static void drawRect( QPainter*, qreal x, qreal y, qreal w, qreal h );
    static void drawRect( QPainter*, const QRectF& rect );
    static void fillRect( QPainter*, const QRectF&, const QBrush& );

    static void drawEllipse( QPainter*, const QRectF& );
    static void drawPie( QPainter*, const QRectF& r, int a, int alen );

    static void drawLine( QPainter*, qreal x1, qreal y1, qreal x2, qreal y2 );
    static void drawLine( QPainter*, const QPointF& p1, const QPointF& p2 );
    static void drawLine( QPainter*, const QLineF& );

    static void drawPolygon( QPainter*, const QPolygonF& );
    static void drawPolyline( QPainter*, const QPolygonF& );
    static void drawPolyline( QPainter*, const QPointF*, int pointCount );

    static void drawPolygon( QPainter*, const QPolygon& );
    static void drawPolyline( QPainter*, const QPolygon& );
    static void drawPolyline( QPainter*, const QPoint*, int pointCount );

    static void drawPoint( QPainter*, const QPoint& );
    static void drawPoints( QPainter*, const QPolygon& );
    static void drawPoints( QPainter*, const QPoint*, int pointCount );

    static void drawPoint( QPainter*, qreal x, qreal y );
    static void drawPoint( QPainter*, const QPointF& );
    static void drawPoints( QPainter*, const QPolygonF& );
    static void drawPoints( QPainter*, const QPointF*, int pointCount );

    static void drawPath( QPainter*, const QPainterPath& );
    static void drawImage( QPainter*, const QRectF&, const QImage& );
    static void drawPixmap( QPainter*, const QRectF&, const QPixmap& );

    static void drawRoundFrame( QPainter*,
        const QRectF&, const QPalette&, int lineWidth, int frameStyle );

    static void drawRoundedFrame( QPainter*,
        const QRectF&, qreal xRadius, qreal yRadius,
        const QPalette&, int lineWidth, int frameStyle );

    static void drawFrame( QPainter*, const QRectF& rect,
        const QPalette& palette, QPalette::ColorRole foregroundRole,
        int lineWidth, int midLineWidth, int frameStyle );

    static void drawFocusRect( QPainter*, const QWidget* );
    static void drawFocusRect( QPainter*, const QWidget*, const QRect& );

    static void drawColorBar( QPainter*,
        const QwtColorMap&, const QwtInterval&,
        const QwtScaleMap&, Qt::Orientation, const QRectF& );

    static bool isAligning( const QPainter*);
    static bool isX11GraphicsSystem();

    static void fillPixmap( const QWidget*,
        QPixmap&, const QPoint& offset = QPoint() );

    static void drawBackgound( QPainter*,
        const QRectF&, const QWidget* );

    static QPixmap backingStore( QWidget*, const QSize& );
    static qreal devicePixelRatio( const QPaintDevice* );

    static qreal effectivePenWidth( const QPen& );

    static int horizontalAdvance( const QFontMetrics&, const QString& );
    static qreal horizontalAdvance( const QFontMetricsF&, const QString& );

    static int horizontalAdvance( const QFontMetrics&, QChar );
    static qreal horizontalAdvance( const QFontMetricsF&, QChar );

    static QFont scaledFont( const QFont&, const QPaintDevice* = nullptr );

  private:
    static bool m_polylineSplitting;
    static bool m_roundingAlignment;
};

//!  Wrapper for QPainter::drawPoint()
inline void QwtPainter::drawPoint( QPainter* painter, qreal x, qreal y )
{
    QwtPainter::drawPoint( painter, QPointF( x, y ) );
}

//! Wrapper for QPainter::drawPoints()
inline void QwtPainter::drawPoints( QPainter* painter, const QPolygon& polygon )
{
    drawPoints( painter, polygon.data(), polygon.size() );
}

//! Wrapper for QPainter::drawPoints()
inline void QwtPainter::drawPoints( QPainter* painter, const QPolygonF& polygon )
{
    drawPoints( painter, polygon.data(), polygon.size() );
}

//!  Wrapper for QPainter::drawLine()
inline void QwtPainter::drawLine( QPainter* painter,
    qreal x1, qreal y1, qreal x2, qreal y2 )
{
    QwtPainter::drawLine( painter, QPointF( x1, y1 ), QPointF( x2, y2 ) );
}

//!  Wrapper for QPainter::drawLine()
inline void QwtPainter::drawLine( QPainter* painter, const QLineF& line )
{
    QwtPainter::drawLine( painter, line.p1(), line.p2() );
}

/*!
   \return True, when line splitting for the raster paint engine is enabled.
   \sa setPolylineSplitting()
 */
inline bool QwtPainter::polylineSplitting()
{
    return m_polylineSplitting;
}

/*!
   Check whether coordinates should be rounded, before they are painted
   to a paint engine that rounds to integer values. For other paint engines
   ( PDF, SVG ), this flag has no effect.

   \return True, when rounding is enabled
   \sa setRoundingAlignment(), isAligning()
 */
inline bool QwtPainter::roundingAlignment()
{
    return m_roundingAlignment;
}

/*!
   \return roundingAlignment() && isAligning(painter);
   \param painter Painter
 */
inline bool QwtPainter::roundingAlignment( const QPainter* painter )
{
    return m_roundingAlignment && isAligning(painter);
}

/*!
   \return pen.widthF() expanded to at least 1.0
   \param pen Pen
 */
inline qreal QwtPainter::effectivePenWidth( const QPen& pen )
{
    const qreal width = pen.widthF();
    return ( width < 1.0 ) ? 1.0 : width;
}

#endif
