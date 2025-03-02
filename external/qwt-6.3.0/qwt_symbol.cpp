/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_symbol.h"
#include "qwt_painter.h"
#include "qwt_graphic.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qpaintengine.h>
#ifndef QWT_NO_SVG
#include <qsvgrenderer.h>
#endif

namespace QwtTriangle
{
    enum Type
    {
        Left,
        Right,
        Up,
        Down
    };
}

static QwtGraphic qwtPathGraphic( const QPainterPath& path,
    const QPen& pen, const QBrush& brush )
{
    QwtGraphic graphic;
    graphic.setRenderHint( QwtGraphic::RenderPensUnscaled );

    QPainter painter( &graphic );
    painter.setPen( pen );
    painter.setBrush( brush );
    painter.drawPath( path );
    painter.end();

    return graphic;
}

static inline QRectF qwtScaledBoundingRect(
    const QwtGraphic& graphic, const QSizeF size )
{
    QSizeF scaledSize = size;
    if ( scaledSize.isEmpty() )
        scaledSize = graphic.defaultSize();

    const QSizeF sz = graphic.controlPointRect().size();

    double sx = 1.0;
    if ( sz.width() > 0.0 )
        sx = scaledSize.width() / sz.width();

    double sy = 1.0;
    if ( sz.height() > 0.0 )
        sy = scaledSize.height() / sz.height();

    return graphic.scaledBoundingRect( sx, sy );
}

static inline void qwtDrawPixmapSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    QSize size = symbol.size();
    if ( size.isEmpty() )
        size = symbol.pixmap().size();

    const QTransform transform = painter->transform();
    if ( transform.isScaling() )
    {
        const QRect r( 0, 0, size.width(), size.height() );
        size = transform.mapRect( r ).size();
    }

    QPixmap pm = symbol.pixmap();
    if ( pm.size() != size )
        pm = pm.scaled( size );

    QPointF pinPoint( 0.5 * size.width(), 0.5 * size.height() );
    if ( symbol.isPinPointEnabled() )
        pinPoint = symbol.pinPoint();

    painter->resetTransform();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF pos = transform.map( points[i] ) - pinPoint;

        QwtPainter::drawPixmap( painter,
            QRect( pos.toPoint(), pm.size() ), pm );
    }
}

#ifndef QWT_NO_SVG

static inline void qwtDrawSvgSymbols( QPainter* painter,
    const QPointF* points, int numPoints,
    QSvgRenderer* renderer, const QwtSymbol& symbol )
{
    if ( renderer == NULL || !renderer->isValid() )
        return;

    const QRectF viewBox = renderer->viewBoxF();
    if ( viewBox.isEmpty() )
        return;

    QSizeF sz = symbol.size();
    if ( !sz.isValid() )
        sz = viewBox.size();

    const double sx = sz.width() / viewBox.width();
    const double sy = sz.height() / viewBox.height();

    QPointF pinPoint = viewBox.center();
    if ( symbol.isPinPointEnabled() )
        pinPoint = symbol.pinPoint();

    const double dx = sx * ( pinPoint.x() - viewBox.left() );
    const double dy = sy * ( pinPoint.y() - viewBox.top() );

    for ( int i = 0; i < numPoints; i++ )
    {
        const double x = points[i].x() - dx;
        const double y = points[i].y() - dy;

        renderer->render( painter,
            QRectF( x, y, sz.width(), sz.height() ) );
    }
}

#endif

static inline void qwtDrawGraphicSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtGraphic& graphic,
    const QwtSymbol& symbol )
{
    const QRectF pointRect = graphic.controlPointRect();
    if ( pointRect.isEmpty() )
        return;

    double sx = 1.0;
    double sy = 1.0;

    const QSize sz = symbol.size();
    if ( sz.isValid() )
    {
        sx = sz.width() / pointRect.width();
        sy = sz.height() / pointRect.height();
    }

    QPointF pinPoint = pointRect.center();
    if ( symbol.isPinPointEnabled() )
        pinPoint = symbol.pinPoint();

    const QTransform transform = painter->transform();

    for ( int i = 0; i < numPoints; i++ )
    {
        QTransform tr = transform;
        tr.translate( points[i].x(), points[i].y() );
        tr.scale( sx, sy );
        tr.translate( -pinPoint.x(), -pinPoint.y() );

        painter->setTransform( tr );

        graphic.render( painter );
    }

    painter->setTransform( transform );
}

static inline void qwtDrawEllipseSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    painter->setBrush( symbol.brush() );
    painter->setPen( symbol.pen() );

    const QSize size = symbol.size();

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = size.width();
        const int sh = size.height();
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            const int x = qRound( points[i].x() );
            const int y = qRound( points[i].y() );

            const QRectF r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawEllipse( painter, r );
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            const double x = points[i].x();
            const double y = points[i].y();

            const QRectF r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawEllipse( painter, r );
        }
    }
}

static inline void qwtDrawRectSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );
    painter->setBrush( symbol.brush() );
    painter->setRenderHint( QPainter::Antialiasing, false );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = size.width();
        const int sh = size.height();
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            const int x = qRound( points[i].x() );
            const int y = qRound( points[i].y() );

            const QRect r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawRect( painter, r );
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            const double x = points[i].x();
            const double y = points[i].y();

            const QRectF r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawRect( painter, r );
        }
    }
}

static inline void qwtDrawDiamondSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );
    painter->setBrush( symbol.brush() );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const int x = qRound( points[i].x() );
            const int y = qRound( points[i].y() );

            const int x1 = x - size.width() / 2;
            const int y1 = y - size.height() / 2;
            const int x2 = x1 + size.width();
            const int y2 = y1 + size.height();

            QPolygonF polygon;
            polygon += QPointF( x, y1 );
            polygon += QPointF( x1, y );
            polygon += QPointF( x, y2 );
            polygon += QPointF( x2, y );

            QwtPainter::drawPolygon( painter, polygon );
        }
    }
    else
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF& pos = points[i];

            const double x1 = pos.x() - 0.5 * size.width();
            const double y1 = pos.y() - 0.5 * size.height();
            const double x2 = x1 + size.width();
            const double y2 = y1 + size.height();

            QPolygonF polygon;
            polygon += QPointF( pos.x(), y1 );
            polygon += QPointF( x2, pos.y() );
            polygon += QPointF( pos.x(), y2 );
            polygon += QPointF( x1, pos.y() );

            QwtPainter::drawPolygon( painter, polygon );
        }
    }
}

static inline void qwtDrawTriangleSymbols(
    QPainter* painter, QwtTriangle::Type type,
    const QPointF* points, int numPoints,
    const QwtSymbol& symbol )
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );

    painter->setBrush( symbol.brush() );

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double sw2 = 0.5 * size.width();
    double sh2 = 0.5 * size.height();

    if ( doAlign )
    {
        sw2 = std::floor( sw2 );
        sh2 = std::floor( sh2 );
    }

    QPolygonF triangle( 3 );
    QPointF* trianglePoints = triangle.data();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF& pos = points[i];

        double x = pos.x();
        double y = pos.y();

        if ( doAlign )
        {
            x = qRound( x );
            y = qRound( y );
        }

        const double x1 = x - sw2;
        const double x2 = x1 + size.width();
        const double y1 = y - sh2;
        const double y2 = y1 + size.height();

        switch ( type )
        {
            case QwtTriangle::Left:
            {
                trianglePoints[0].rx() = x2;
                trianglePoints[0].ry() = y1;

                trianglePoints[1].rx() = x1;
                trianglePoints[1].ry() = y;

                trianglePoints[2].rx() = x2;
                trianglePoints[2].ry() = y2;

                break;
            }
            case QwtTriangle::Right:
            {
                trianglePoints[0].rx() = x1;
                trianglePoints[0].ry() = y1;

                trianglePoints[1].rx() = x2;
                trianglePoints[1].ry() = y;

                trianglePoints[2].rx() = x1;
                trianglePoints[2].ry() = y2;

                break;
            }
            case QwtTriangle::Up:
            {
                trianglePoints[0].rx() = x1;
                trianglePoints[0].ry() = y2;

                trianglePoints[1].rx() = x;
                trianglePoints[1].ry() = y1;

                trianglePoints[2].rx() = x2;
                trianglePoints[2].ry() = y2;

                break;
            }
            case QwtTriangle::Down:
            {
                trianglePoints[0].rx() = x1;
                trianglePoints[0].ry() = y1;

                trianglePoints[1].rx() = x;
                trianglePoints[1].ry() = y2;

                trianglePoints[2].rx() = x2;
                trianglePoints[2].ry() = y1;

                break;
            }
        }
        QwtPainter::drawPolygon( painter, triangle );
    }
}

static inline void qwtDrawLineSymbols(
    QPainter* painter, int orientations,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    const QSize size = symbol.size();

    int off = 0;

    QPen pen = symbol.pen();
    if ( pen.width() > 1 )
    {
        pen.setCapStyle( Qt::FlatCap );
        off = 1;
    }

    painter->setPen( pen );
    painter->setRenderHint( QPainter::Antialiasing, false );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = qwtFloor( size.width() );
        const int sh = qwtFloor( size.height() );
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            if ( orientations & Qt::Horizontal )
            {
                const int x = qRound( points[i].x() ) - sw2;
                const int y = qRound( points[i].y() );

                QwtPainter::drawLine( painter, x, y, x + sw + off, y );
            }
            if ( orientations & Qt::Vertical )
            {
                const int x = qRound( points[i].x() );
                const int y = qRound( points[i].y() ) - sh2;

                QwtPainter::drawLine( painter, x, y, x, y + sh + off );
            }
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            if ( orientations & Qt::Horizontal )
            {
                const double x = points[i].x() - sw2;
                const double y = points[i].y();

                QwtPainter::drawLine( painter, x, y, x + sw, y );
            }
            if ( orientations & Qt::Vertical )
            {
                const double y = points[i].y() - sh2;
                const double x = points[i].x();

                QwtPainter::drawLine( painter, x, y, x, y + sh );
            }
        }
    }
}

static inline void qwtDrawXCrossSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    const QSize size = symbol.size();
    int off = 0;

    QPen pen = symbol.pen();
    if ( pen.width() > 1 )
    {
        pen.setCapStyle( Qt::FlatCap );
        off = 1;
    }
    painter->setPen( pen );


    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = size.width();
        const int sh = size.height();
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF& pos = points[i];

            const int x = qRound( pos.x() );
            const int y = qRound( pos.y() );

            const int x1 = x - sw2;
            const int x2 = x1 + sw + off;
            const int y1 = y - sh2;
            const int y2 = y1 + sh + off;

            QwtPainter::drawLine( painter, x1, y1, x2, y2 );
            QwtPainter::drawLine( painter, x2, y1, x1, y2 );
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF& pos = points[i];

            const double x1 = pos.x() - sw2;
            const double x2 = x1 + sw;
            const double y1 = pos.y() - sh2;
            const double y2 = y1 + sh;

            QwtPainter::drawLine( painter, x1, y1, x2, y2 );
            QwtPainter::drawLine( painter, x1, y2, x2, y1 );
        }
    }
}

static inline void qwtDrawStar1Symbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    const QSize size = symbol.size();
    painter->setPen( symbol.pen() );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        QRect r( 0, 0, size.width(), size.height() );

        for ( int i = 0; i < numPoints; i++ )
        {
            r.moveCenter( points[i].toPoint() );

            const double sqrt1_2 = 0.70710678118654752440; /* 1/sqrt(2) */

            const double d1 = r.width() / 2.0 * ( 1.0 - sqrt1_2 );

            QwtPainter::drawLine( painter,
                qRound( r.left() + d1 ), qRound( r.top() + d1 ),
                qRound( r.right() - d1 ), qRound( r.bottom() - d1 ) );
            QwtPainter::drawLine( painter,
                qRound( r.left() + d1 ), qRound( r.bottom() - d1 ),
                qRound( r.right() - d1), qRound( r.top() + d1 ) );

            const QPoint c = r.center();

            QwtPainter::drawLine( painter,
                c.x(), r.top(), c.x(), r.bottom() );
            QwtPainter::drawLine( painter,
                r.left(), c.y(), r.right(), c.y() );
        }
    }
    else
    {
        QRectF r( 0, 0, size.width(), size.height() );

        for ( int i = 0; i < numPoints; i++ )
        {
            r.moveCenter( points[i] );

            const double sqrt1_2 = 0.70710678118654752440; /* 1/sqrt(2) */

            const QPointF c = r.center();
            const double d1 = r.width() / 2.0 * ( 1.0 - sqrt1_2 );

            QwtPainter::drawLine( painter,
                r.left() + d1, r.top() + d1,
                r.right() - d1, r.bottom() - d1 );
            QwtPainter::drawLine( painter,
                r.left() + d1, r.bottom() - d1,
                r.right() - d1, r.top() + d1 );
            QwtPainter::drawLine( painter,
                c.x(), r.top(),
                c.x(), r.bottom() );
            QwtPainter::drawLine( painter,
                r.left(), c.y(),
                r.right(), c.y() );
        }
    }
}

static inline void qwtDrawStar2Symbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    QPen pen = symbol.pen();
    if ( pen.width() > 1 )
        pen.setCapStyle( Qt::FlatCap );
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );

    painter->setBrush( symbol.brush() );

    const double cos30 = 0.866025; // cos(30°)

    const double dy = 0.25 * symbol.size().height();
    const double dx = 0.5 * symbol.size().width() * cos30 / 3.0;

    QPolygonF star( 12 );
    QPointF* starPoints = star.data();

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < numPoints; i++ )
    {
        double x = points[i].x();
        double y = points[i].y();
        if ( doAlign )
        {
            x = qRound( x );
            y = qRound( y );
        }

        double x1 = x - 3 * dx;
        double y1 = y - 2 * dy;
        if ( doAlign )
        {
            x1 = qRound( x - 3 * dx );
            y1 = qRound( y - 2 * dy );
        }

        const double x2 = x1 + 1 * dx;
        const double x3 = x1 + 2 * dx;
        const double x4 = x1 + 3 * dx;
        const double x5 = x1 + 4 * dx;
        const double x6 = x1 + 5 * dx;
        const double x7 = x1 + 6 * dx;

        const double y2 = y1 + 1 * dy;
        const double y3 = y1 + 2 * dy;
        const double y4 = y1 + 3 * dy;
        const double y5 = y1 + 4 * dy;

        starPoints[0].rx() = x4;
        starPoints[0].ry() = y1;

        starPoints[1].rx() = x5;
        starPoints[1].ry() = y2;

        starPoints[2].rx() = x7;
        starPoints[2].ry() = y2;

        starPoints[3].rx() = x6;
        starPoints[3].ry() = y3;

        starPoints[4].rx() = x7;
        starPoints[4].ry() = y4;

        starPoints[5].rx() = x5;
        starPoints[5].ry() = y4;

        starPoints[6].rx() = x4;
        starPoints[6].ry() = y5;

        starPoints[7].rx() = x3;
        starPoints[7].ry() = y4;

        starPoints[8].rx() = x1;
        starPoints[8].ry() = y4;

        starPoints[9].rx() = x2;
        starPoints[9].ry() = y3;

        starPoints[10].rx() = x1;
        starPoints[10].ry() = y2;

        starPoints[11].rx() = x3;
        starPoints[11].ry() = y2;

        QwtPainter::drawPolygon( painter, star );
    }
}

static inline void qwtDrawHexagonSymbols( QPainter* painter,
    const QPointF* points, int numPoints, const QwtSymbol& symbol )
{
    painter->setBrush( symbol.brush() );
    painter->setPen( symbol.pen() );

    const double cos30 = 0.866025; // cos(30°)
    const double dx = 0.5 * ( symbol.size().width() - cos30 );

    const double dy = 0.25 * symbol.size().height();

    QPolygonF hexaPolygon( 6 );
    QPointF* hexaPoints = hexaPolygon.data();

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < numPoints; i++ )
    {
        double x = points[i].x();
        double y = points[i].y();
        if ( doAlign )
        {
            x = qRound( x );
            y = qRound( y );
        }

        double x1 = x - dx;
        double y1 = y - 2 * dy;
        if ( doAlign )
        {
            x1 = std::ceil( x1 );
            y1 = std::ceil( y1 );
        }

        const double x2 = x1 + 1 * dx;
        const double x3 = x1 + 2 * dx;

        const double y2 = y1 + 1 * dy;
        const double y3 = y1 + 3 * dy;
        const double y4 = y1 + 4 * dy;

        hexaPoints[0].rx() = x2;
        hexaPoints[0].ry() = y1;

        hexaPoints[1].rx() = x3;
        hexaPoints[1].ry() = y2;

        hexaPoints[2].rx() = x3;
        hexaPoints[2].ry() = y3;

        hexaPoints[3].rx() = x2;
        hexaPoints[3].ry() = y4;

        hexaPoints[4].rx() = x1;
        hexaPoints[4].ry() = y3;

        hexaPoints[5].rx() = x1;
        hexaPoints[5].ry() = y2;

        QwtPainter::drawPolygon( painter, hexaPolygon );
    }
}

class QwtSymbol::PrivateData
{
  public:
    PrivateData( QwtSymbol::Style st, const QBrush& br,
            const QPen& pn, const QSize& sz )
        : style( st )
        , size( sz )
        , brush( br )
        , pen( pn )
        , isPinPointEnabled( false )
    {
        cache.policy = QwtSymbol::AutoCache;
#ifndef QWT_NO_SVG
        svg.renderer = NULL;
#endif
    }

    ~PrivateData()
    {
#ifndef QWT_NO_SVG
        delete svg.renderer;
#endif
    }

    Style style;
    QSize size;
    QBrush brush;
    QPen pen;

    bool isPinPointEnabled;
    QPointF pinPoint;

    struct Path
    {
        QPainterPath path;
        QwtGraphic graphic;

    } path;

    struct Pixmap
    {
        QPixmap pixmap;

    } pixmap;

    struct Graphic
    {
        QwtGraphic graphic;

    } graphic;

#ifndef QWT_NO_SVG
    struct SVG
    {
        QSvgRenderer* renderer;
    } svg;
#endif

    struct PaintCache
    {
        QwtSymbol::CachePolicy policy;
        QPixmap pixmap;

    } cache;
};

/*!
   Default Constructor
   \param style Symbol Style

   The symbol is constructed with gray interior,
   black outline with zero width, no size and style 'NoSymbol'.
 */
QwtSymbol::QwtSymbol( Style style )
{
    m_data = new PrivateData( style, QBrush( Qt::gray ),
        QPen( Qt::black, 0 ), QSize() );
}

/*!
   \brief Constructor
   \param style Symbol Style
   \param brush brush to fill the interior
   \param pen outline pen
   \param size size

   \sa setStyle(), setBrush(), setPen(), setSize()
 */
QwtSymbol::QwtSymbol( QwtSymbol::Style style, const QBrush& brush,
    const QPen& pen, const QSize& size )
{
    m_data = new PrivateData( style, brush, pen, size );
}

/*!
   \brief Constructor

   The symbol gets initialized by a painter path. The style is
   set to QwtSymbol::Path, the size is set to empty ( the path
   is displayed unscaled ).

   \param path painter path
   \param brush brush to fill the interior
   \param pen outline pen

   \sa setPath(), setBrush(), setPen(), setSize()
 */

QwtSymbol::QwtSymbol( const QPainterPath& path,
    const QBrush& brush, const QPen& pen )
{
    m_data = new PrivateData( QwtSymbol::Path, brush, pen, QSize() );
    setPath( path );
}

//! Destructor
QwtSymbol::~QwtSymbol()
{
    delete m_data;
}

/*!
   Change the cache policy

   The default policy is AutoCache

   \param policy Cache policy
   \sa CachePolicy, cachePolicy()
 */
void QwtSymbol::setCachePolicy(
    QwtSymbol::CachePolicy policy )
{
    if ( m_data->cache.policy != policy )
    {
        m_data->cache.policy = policy;
        invalidateCache();
    }
}

/*!
   \return Cache policy
   \sa CachePolicy, setCachePolicy()
 */
QwtSymbol::CachePolicy QwtSymbol::cachePolicy() const
{
    return m_data->cache.policy;
}

/*!
   \brief Set a painter path as symbol

   The symbol is represented by a painter path, where the
   origin ( 0, 0 ) of the path coordinate system is mapped to
   the position of the symbol.

   When the symbol has valid size the painter path gets scaled
   to fit into the size. Otherwise the symbol size depends on
   the bounding rectangle of the path.

   \par Example
    The following code defines a symbol drawing an arrow:

    \code
 #include <qwt_symbol.h>

      QwtSymbol *symbol = new QwtSymbol();

      QPen pen( Qt::black, 2 );
      pen.setJoinStyle( Qt::MiterJoin );

      symbol->setPen( pen );
      symbol->setBrush( Qt::red );

      QPainterPath path;
      path.moveTo( 0, 8 );
      path.lineTo( 0, 5 );
      path.lineTo( -3, 5 );
      path.lineTo( 0, 0 );
      path.lineTo( 3, 5 );
      path.lineTo( 0, 5 );

      QTransform transform;
      transform.rotate( -30.0 );
      path = transform.map( path );

      symbol->setPath( path );
      symbol->setPinPoint( QPointF( 0.0, 0.0 ) );

      setSize( 10, 14 );
    \endcode

   \param path Painter path

   \note The style is implicitly set to QwtSymbol::Path.
   \sa path(), setSize()
 */
void QwtSymbol::setPath( const QPainterPath& path )
{
    m_data->style = QwtSymbol::Path;
    m_data->path.path = path;
    m_data->path.graphic.reset();
}

/*!
   \return Painter path for displaying the symbol
   \sa setPath()
 */
const QPainterPath& QwtSymbol::path() const
{
    return m_data->path.path;
}

/*!
   Set a pixmap as symbol

   \param pixmap Pixmap

   \sa pixmap(), setGraphic()

   \note the style() is set to QwtSymbol::Pixmap
   \note brush() and pen() have no effect
 */
void QwtSymbol::setPixmap( const QPixmap& pixmap )
{
    m_data->style = QwtSymbol::Pixmap;
    m_data->pixmap.pixmap = pixmap;
}

/*!
   \return Assigned pixmap
   \sa setPixmap()
 */
const QPixmap& QwtSymbol::pixmap() const
{
    return m_data->pixmap.pixmap;
}

/*!
   Set a graphic as symbol

   \param graphic Graphic

   \sa graphic(), setPixmap()

   \note the style() is set to QwtSymbol::Graphic
   \note brush() and pen() have no effect
 */
void QwtSymbol::setGraphic( const QwtGraphic& graphic )
{
    m_data->style = QwtSymbol::Graphic;
    m_data->graphic.graphic = graphic;
}

/*!
   \return Assigned graphic
   \sa setGraphic()
 */
const QwtGraphic& QwtSymbol::graphic() const
{
    return m_data->graphic.graphic;
}

#ifndef QWT_NO_SVG

/*!
   Set a SVG icon as symbol

   \param svgDocument SVG icon

   \sa setGraphic(), setPixmap()

   \note the style() is set to QwtSymbol::SvgDocument
   \note brush() and pen() have no effect
 */
void QwtSymbol::setSvgDocument( const QByteArray& svgDocument )
{
    m_data->style = QwtSymbol::SvgDocument;
    if ( m_data->svg.renderer == NULL )
        m_data->svg.renderer = new QSvgRenderer();

    m_data->svg.renderer->load( svgDocument );
}

#endif

/*!
   \brief Specify the symbol's size

   If the 'h' parameter is left out or less than 0,
   and the 'w' parameter is greater than or equal to 0,
   the symbol size will be set to (w,w).

   \param width Width
   \param height Height (defaults to -1)

   \sa size()
 */
void QwtSymbol::setSize( int width, int height )
{
    if ( ( width >= 0 ) && ( height < 0 ) )
        height = width;

    setSize( QSize( width, height ) );
}

/*!
   Set the symbol's size
   \param size Size

   \sa size()
 */
void QwtSymbol::setSize( const QSize& size )
{
    if ( size.isValid() && size != m_data->size )
    {
        m_data->size = size;
        invalidateCache();
    }
}

/*!
   \return Size
   \sa setSize()
 */
const QSize& QwtSymbol::size() const
{
    return m_data->size;
}

/*!
   \brief Assign a brush

   The brush is used to draw the interior of the symbol.
   \param brush Brush

   \sa brush()
 */
void QwtSymbol::setBrush( const QBrush& brush )
{
    if ( brush != m_data->brush )
    {
        m_data->brush = brush;
        invalidateCache();

        if ( m_data->style == QwtSymbol::Path )
            m_data->path.graphic.reset();
    }
}

/*!
   \return Brush
   \sa setBrush()
 */
const QBrush& QwtSymbol::brush() const
{
    return m_data->brush;
}

/*!
   Build and assign a pen

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 )
   what makes it non cosmetic ( see QPen::isCosmetic() ).
   This method has been introduced to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtSymbol::setPen( const QColor& color,
    qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   Assign a pen

   The pen is used to draw the symbol's outline.

   \param pen Pen
   \sa pen(), setBrush()
 */
void QwtSymbol::setPen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;
        invalidateCache();

        if ( m_data->style == QwtSymbol::Path )
            m_data->path.graphic.reset();
    }
}

/*!
   \return Pen
   \sa setPen(), brush()
 */
const QPen& QwtSymbol::pen() const
{
    return m_data->pen;
}

/*!
   \brief Set the color of the symbol

   Change the color of the brush for symbol types with a filled area.
   For all other symbol types the color will be assigned to the pen.

   \param color Color

   \sa setBrush(), setPen(), brush(), pen()
 */
void QwtSymbol::setColor( const QColor& color )
{
    switch ( m_data->style )
    {
        case QwtSymbol::Ellipse:
        case QwtSymbol::Rect:
        case QwtSymbol::Diamond:
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        case QwtSymbol::DTriangle:
        case QwtSymbol::RTriangle:
        case QwtSymbol::LTriangle:
        case QwtSymbol::Star2:
        case QwtSymbol::Hexagon:
        {
            if ( m_data->brush.color() != color )
            {
                m_data->brush.setColor( color );
                invalidateCache();
            }
            break;
        }
        case QwtSymbol::Cross:
        case QwtSymbol::XCross:
        case QwtSymbol::HLine:
        case QwtSymbol::VLine:
        case QwtSymbol::Star1:
        {
            if ( m_data->pen.color() != color )
            {
                m_data->pen.setColor( color );
                invalidateCache();
            }
            break;
        }
        default:
        {
            if ( m_data->brush.color() != color ||
                m_data->pen.color() != color )
            {
                invalidateCache();
            }

            m_data->brush.setColor( color );
            m_data->pen.setColor( color );
        }
    }
}

/*!
   \brief Set and enable a pin point

   The position of a complex symbol is not always aligned to its center
   ( f.e an arrow, where the peak points to a position ). The pin point
   defines the position inside of a Pixmap, Graphic, SvgDocument
   or PainterPath symbol where the represented point has to
   be aligned to.

   \param pos Position
   \param enable En/Disable the pin point alignment

   \sa pinPoint(), setPinPointEnabled()
 */
void QwtSymbol::setPinPoint( const QPointF& pos, bool enable )
{
    if ( m_data->pinPoint != pos )
    {
        m_data->pinPoint = pos;
        if ( m_data->isPinPointEnabled )
        {
            invalidateCache();
        }
    }

    setPinPointEnabled( enable );
}

/*!
   \return Pin point
   \sa setPinPoint(), setPinPointEnabled()
 */
QPointF QwtSymbol::pinPoint() const
{
    return m_data->pinPoint;
}

/*!
   En/Disable the pin point alignment

   \param on Enabled, when on is true
   \sa setPinPoint(), isPinPointEnabled()
 */
void QwtSymbol::setPinPointEnabled( bool on )
{
    if ( m_data->isPinPointEnabled != on )
    {
        m_data->isPinPointEnabled = on;
        invalidateCache();
    }
}

/*!
   \return True, when the pin point translation is enabled
   \sa setPinPoint(), setPinPointEnabled()
 */
bool QwtSymbol::isPinPointEnabled() const
{
    return m_data->isPinPointEnabled;
}

/*!
   Render an array of symbols

   Painting several symbols is more effective than drawing symbols
   one by one, as a couple of layout calculations and setting of pen/brush
   can be done once for the complete array.

   \param painter Painter
   \param points Array of points
   \param numPoints Number of points
 */
void QwtSymbol::drawSymbols( QPainter* painter,
    const QPointF* points, int numPoints ) const
{
    if ( numPoints <= 0 )
        return;

    bool useCache = false;

    // Don't use the pixmap, when the paint device
    // could generate scalable vectors

    if ( QwtPainter::roundingAlignment( painter ) &&
        !painter->transform().isScaling() )
    {
        if ( m_data->cache.policy == QwtSymbol::Cache )
        {
            useCache = true;
        }
        else if ( m_data->cache.policy == QwtSymbol::AutoCache )
        {
            switch( painter->paintEngine()->type() )
            {
                case QPaintEngine::OpenGL:
                case QPaintEngine::OpenGL2:
                {
                    // using a FBO as cache ?
                    useCache = false;
                    break;
                }
                case QPaintEngine::OpenVG:
                case QPaintEngine::SVG:
                case QPaintEngine::Pdf:
                case QPaintEngine::Picture:
                {
                    // vector graphics
                    useCache = false;
                    break;
                }
                case QPaintEngine::X11:
                {
                    switch( m_data->style )
                    {
                        case QwtSymbol::XCross:
                        case QwtSymbol::HLine:
                        case QwtSymbol::VLine:
                        case QwtSymbol::Cross:
                        {
                            // for the very simple shapes using vector graphics is
                            // usually faster.

                            useCache = false;
                            break;
                        }

                        case QwtSymbol::Pixmap:
                        {
                            if ( m_data->size.isEmpty() ||
                                m_data->size == m_data->pixmap.pixmap.size() )
                            {
                                // no need to have a pixmap cache for a pixmap
                                // of the same size

                                useCache = false;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
                default:
                {
                    useCache = true;
                }
            }
        }
    }

    if ( useCache )
    {
        const QRect br = boundingRect();

        if ( m_data->cache.pixmap.isNull() )
        {
            m_data->cache.pixmap = QwtPainter::backingStore( NULL, br.size() );
            m_data->cache.pixmap.fill( Qt::transparent );

            QPainter p( &m_data->cache.pixmap );
            p.setRenderHints( painter->renderHints() );
            p.translate( -br.topLeft() );

            const QPointF pos( 0.0, 0.0 );
            renderSymbols( &p, &pos, 1 );
        }

        const int dx = br.left();
        const int dy = br.top();

        for ( int i = 0; i < numPoints; i++ )
        {
            const int left = qRound( points[i].x() ) + dx;
            const int top = qRound( points[i].y() ) + dy;

            painter->drawPixmap( left, top, m_data->cache.pixmap );
        }
    }
    else
    {
        painter->save();
        renderSymbols( painter, points, numPoints );
        painter->restore();
    }
}

/*!
   \brief Draw the symbol into a rectangle

   The symbol is painted centered and scaled into the target rectangle.
   It is always painted uncached and the pin point is ignored.

   This method is primarily intended for drawing a symbol to
   the legend.

   \param painter Painter
   \param rect Target rectangle for the symbol
 */
void QwtSymbol::drawSymbol( QPainter* painter, const QRectF& rect ) const
{
    if ( m_data->style == QwtSymbol::NoSymbol )
        return;

    if ( m_data->style == QwtSymbol::Graphic )
    {
        m_data->graphic.graphic.render(
            painter, rect, Qt::KeepAspectRatio );
    }
    else if ( m_data->style == QwtSymbol::Path )
    {
        if ( m_data->path.graphic.isNull() )
        {
            m_data->path.graphic = qwtPathGraphic(
                m_data->path.path, m_data->pen, m_data->brush );
        }

        m_data->path.graphic.render(
            painter, rect, Qt::KeepAspectRatio );
        return;
    }
    else if ( m_data->style == QwtSymbol::SvgDocument )
    {
#ifndef QWT_NO_SVG
        if ( m_data->svg.renderer )
        {
            QRectF scaledRect;

            QSizeF sz = m_data->svg.renderer->viewBoxF().size();
            if ( !sz.isEmpty() )
            {
                sz.scale( rect.size(), Qt::KeepAspectRatio );
                scaledRect.setSize( sz );
                scaledRect.moveCenter( rect.center() );
            }
            else
            {
                scaledRect = rect;
            }

            m_data->svg.renderer->render(
                painter, scaledRect );
        }
#endif
    }
    else
    {
        const QRect br = boundingRect();

        // scale the symbol size to fit into rect.

        const double ratio = qMin( rect.width() / br.width(),
            rect.height() / br.height() );

        painter->save();

        painter->translate( rect.center() );
        painter->scale( ratio, ratio );

        const bool isPinPointEnabled = m_data->isPinPointEnabled;
        m_data->isPinPointEnabled = false;

        const QPointF pos;
        renderSymbols( painter, &pos, 1 );

        m_data->isPinPointEnabled = isPinPointEnabled;

        painter->restore();
    }
}

/*!
   Render the symbol to series of points

   \param painter Qt painter
   \param points Positions of the symbols
   \param numPoints Number of points
 */
void QwtSymbol::renderSymbols( QPainter* painter,
    const QPointF* points, int numPoints ) const
{
    switch ( m_data->style )
    {
        case QwtSymbol::Ellipse:
        {
            qwtDrawEllipseSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Rect:
        {
            qwtDrawRectSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Diamond:
        {
            qwtDrawDiamondSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Cross:
        {
            qwtDrawLineSymbols( painter, Qt::Horizontal | Qt::Vertical,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::XCross:
        {
            qwtDrawXCrossSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Up,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::DTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Down,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::RTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Right,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::LTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Left,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::HLine:
        {
            qwtDrawLineSymbols( painter, Qt::Horizontal,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::VLine:
        {
            qwtDrawLineSymbols( painter, Qt::Vertical,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::Star1:
        {
            qwtDrawStar1Symbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Star2:
        {
            qwtDrawStar2Symbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Hexagon:
        {
            qwtDrawHexagonSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Path:
        {
            if ( m_data->path.graphic.isNull() )
            {
                m_data->path.graphic = qwtPathGraphic( m_data->path.path,
                    m_data->pen, m_data->brush );
            }

            qwtDrawGraphicSymbols( painter, points, numPoints,
                m_data->path.graphic, *this );
            break;
        }
        case QwtSymbol::Pixmap:
        {
            qwtDrawPixmapSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Graphic:
        {
            qwtDrawGraphicSymbols( painter, points, numPoints,
                m_data->graphic.graphic, *this );
            break;
        }
        case QwtSymbol::SvgDocument:
        {
#ifndef QWT_NO_SVG
            qwtDrawSvgSymbols( painter, points, numPoints,
                m_data->svg.renderer, *this );
#endif
            break;
        }
        default:;
    }
}

/*!
   Calculate the bounding rectangle for a symbol
   at position (0,0).

   \return Bounding rectangle
 */
QRect QwtSymbol::boundingRect() const
{
    QRectF rect;

    bool pinPointTranslation = false;

    switch ( m_data->style )
    {
        case QwtSymbol::Ellipse:
        case QwtSymbol::Rect:
        case QwtSymbol::Hexagon:
        {
            qreal pw = 0.0;
            if ( m_data->pen.style() != Qt::NoPen )
                pw = QwtPainter::effectivePenWidth( m_data->pen );

            rect.setSize( m_data->size + QSizeF( pw, pw ) );
            rect.moveCenter( QPointF( 0.0, 0.0 ) );

            break;
        }
        case QwtSymbol::XCross:
        case QwtSymbol::Diamond:
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        case QwtSymbol::DTriangle:
        case QwtSymbol::RTriangle:
        case QwtSymbol::LTriangle:
        case QwtSymbol::Star1:
        case QwtSymbol::Star2:
        {
            qreal pw = 0.0;
            if ( m_data->pen.style() != Qt::NoPen )
                pw = QwtPainter::effectivePenWidth( m_data->pen );

            rect.setSize( m_data->size + QSizeF( 2 * pw, 2 * pw ) );
            rect.moveCenter( QPointF( 0.0, 0.0 ) );
            break;
        }
        case QwtSymbol::Path:
        {
            if ( m_data->path.graphic.isNull() )
            {
                m_data->path.graphic = qwtPathGraphic(
                    m_data->path.path, m_data->pen, m_data->brush );
            }

            rect = qwtScaledBoundingRect(
                m_data->path.graphic, m_data->size );
            pinPointTranslation = true;

            break;
        }
        case QwtSymbol::Pixmap:
        {
            if ( m_data->size.isEmpty() )
                rect.setSize( m_data->pixmap.pixmap.size() );
            else
                rect.setSize( m_data->size );

            pinPointTranslation = true;

            break;
        }
        case QwtSymbol::Graphic:
        {
            rect = qwtScaledBoundingRect(
                m_data->graphic.graphic, m_data->size );
            pinPointTranslation = true;

            break;
        }
#ifndef QWT_NO_SVG
        case QwtSymbol::SvgDocument:
        {
            if ( m_data->svg.renderer )
                rect = m_data->svg.renderer->viewBoxF();

            if ( m_data->size.isValid() && !rect.isEmpty() )
            {
                QSizeF sz = rect.size();

                const double sx = m_data->size.width() / sz.width();
                const double sy = m_data->size.height() / sz.height();

                QTransform transform;
                transform.scale( sx, sy );

                rect = transform.mapRect( rect );
            }
            pinPointTranslation = true;
            break;
        }
#endif
        default:
        {
            rect.setSize( m_data->size );
            rect.moveCenter( QPointF( 0.0, 0.0 ) );
        }
    }

    if ( pinPointTranslation )
    {
        QPointF pinPoint( 0.0, 0.0 );
        if ( m_data->isPinPointEnabled )
            pinPoint = rect.center() - m_data->pinPoint;

        rect.moveCenter( pinPoint );
    }

    QRect r;
    r.setLeft( qwtFloor( rect.left() ) );
    r.setTop( qwtFloor( rect.top() ) );
    r.setRight( qwtCeil( rect.right() ) );
    r.setBottom( qwtCeil( rect.bottom() ) );

    if ( m_data->style != QwtSymbol::Pixmap )
        r.adjust( -1, -1, 1, 1 ); // for antialiasing

    return r;
}

/*!
   Invalidate the cached symbol pixmap

   The symbol invalidates its cache, whenever an attribute is changed
   that has an effect ob how to display a symbol. In case of derived
   classes with individual styles ( >= QwtSymbol::UserStyle ) it
   might be necessary to call invalidateCache() for attributes
   that are relevant for this style.

   \sa CachePolicy, setCachePolicy(), drawSymbols()
 */
void QwtSymbol::invalidateCache()
{
    if ( !m_data->cache.pixmap.isNull() )
        m_data->cache.pixmap = QPixmap();
}

/*!
   Specify the symbol style

   \param style Style
   \sa style()
 */
void QwtSymbol::setStyle( QwtSymbol::Style style )
{
    if ( m_data->style != style )
    {
        m_data->style = style;
        invalidateCache();
    }
}

/*!
   \return Current symbol style
   \sa setStyle()
 */
QwtSymbol::Style QwtSymbol::style() const
{
    return m_data->style;
}
