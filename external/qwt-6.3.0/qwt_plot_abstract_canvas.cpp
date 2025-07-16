/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_abstract_canvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"
#include "qwt_null_paintdevice.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qstyle.h>
#include <qstyleoption.h>

namespace
{
    class QwtStyleSheetRecorder QWT_FINAL : public QwtNullPaintDevice
    {
      public:
        explicit QwtStyleSheetRecorder( const QSize& size )
            : m_size( size )
        {
        }

        virtual void updateState( const QPaintEngineState& state ) QWT_OVERRIDE
        {
            if ( state.state() & QPaintEngine::DirtyPen )
            {
                m_pen = state.pen();
            }
            if ( state.state() & QPaintEngine::DirtyBrush )
            {
                m_brush = state.brush();
            }
            if ( state.state() & QPaintEngine::DirtyBrushOrigin )
            {
                m_origin = state.brushOrigin();
            }
        }

        virtual void drawRects(const QRectF* rects, int count ) QWT_OVERRIDE
        {
            for ( int i = 0; i < count; i++ )
                border.rectList += rects[i];
        }

        virtual void drawRects(const QRect* rects, int count ) QWT_OVERRIDE
        {
            for ( int i = 0; i < count; i++ )
                border.rectList += rects[i];
        }

        virtual void drawPath( const QPainterPath& path ) QWT_OVERRIDE
        {
            const QRectF rect( QPointF( 0.0, 0.0 ), m_size );
            if ( path.controlPointRect().contains( rect.center() ) )
            {
                setCornerRects( path );
                alignCornerRects( rect );

                background.path = path;
                background.brush = m_brush;
                background.origin = m_origin;
            }
            else
            {
                border.pathList += path;
            }
        }

        void setCornerRects( const QPainterPath& path )
        {
            QPointF pos( 0.0, 0.0 );

            for ( int i = 0; i < path.elementCount(); i++ )
            {
                QPainterPath::Element el = path.elementAt(i);
                switch( el.type )
                {
                    case QPainterPath::MoveToElement:
                    case QPainterPath::LineToElement:
                    {
                        pos.setX( el.x );
                        pos.setY( el.y );
                        break;
                    }
                    case QPainterPath::CurveToElement:
                    {
                        QRectF r( pos, QPointF( el.x, el.y ) );
                        clipRects += r.normalized();

                        pos.setX( el.x );
                        pos.setY( el.y );

                        break;
                    }
                    case QPainterPath::CurveToDataElement:
                    {
                        if ( clipRects.size() > 0 )
                        {
                            QRectF r = clipRects.last();
                            r.setCoords(
                                qwtMinF( r.left(), el.x ),
                                qwtMinF( r.top(), el.y ),
                                qwtMaxF( r.right(), el.x ),
                                qwtMaxF( r.bottom(), el.y )
                                );
                            clipRects.last() = r.normalized();
                        }
                        break;
                    }
                }
            }
        }

      protected:
        virtual QSize sizeMetrics() const QWT_OVERRIDE
        {
            return m_size;
        }

      private:
        void alignCornerRects( const QRectF& rect )
        {
            for ( int i = 0; i < clipRects.size(); i++ )
            {
                QRectF& r = clipRects[i];
                if ( r.center().x() < rect.center().x() )
                    r.setLeft( rect.left() );
                else
                    r.setRight( rect.right() );

                if ( r.center().y() < rect.center().y() )
                    r.setTop( rect.top() );
                else
                    r.setBottom( rect.bottom() );
            }
        }


      public:
        QVector< QRectF > clipRects;

        struct Border
        {
            QList< QPainterPath > pathList;
            QList< QRectF > rectList;
            QRegion clipRegion;
        } border;

        struct Background
        {
            QPainterPath path;
            QBrush brush;
            QPointF origin;
        } background;

      private:
        const QSize m_size;

        QPen m_pen;
        QBrush m_brush;
        QPointF m_origin;
    };
}

static void qwtUpdateContentsRect( int fw, QWidget* canvas )
{
    canvas->setContentsMargins( fw, fw, fw, fw );
}

static void qwtFillRegion( QPainter* painter, const QRegion& region )
{
#if QT_VERSION >= 0x050800
    for ( QRegion::const_iterator it = region.cbegin();
        it != region.cend(); ++it )
    {
        painter->drawRect( *it );
    }
#else
    painter->drawRects( region.rects() );
#endif
}

static void qwtDrawBackground( QPainter* painter, QWidget* canvas )
{
    painter->save();

    QPainterPath borderClip;

    ( void )QMetaObject::invokeMethod(
        canvas, "borderPath", Qt::DirectConnection,
        Q_RETURN_ARG( QPainterPath, borderClip ), Q_ARG( QRect, canvas->rect() ) );

    if ( !borderClip.isEmpty() )
        painter->setClipPath( borderClip, Qt::IntersectClip );

    const QBrush& brush = canvas->palette().brush( canvas->backgroundRole() );

    if ( brush.style() == Qt::TexturePattern )
    {
        QPixmap pm( canvas->size() );
        QwtPainter::fillPixmap( canvas, pm );
        painter->drawPixmap( 0, 0, pm );
    }
    else if ( brush.gradient() )
    {
        const bool fillClipRegion =
            brush.gradient()->coordinateMode() != QGradient::ObjectBoundingMode;

        painter->setPen( Qt::NoPen );
        painter->setBrush( brush );

        if ( fillClipRegion )
            qwtFillRegion( painter, painter->clipRegion() );
        else
            painter->drawRect( canvas->rect() );
    }
    else
    {
        painter->setPen( Qt::NoPen );
        painter->setBrush( brush );
        qwtFillRegion( painter, painter->clipRegion() );
    }

    painter->restore();
}

static inline void qwtDrawStyledBackground(
    QWidget* w, QPainter* painter )
{
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w);
}

static QWidget* qwtBackgroundWidget( QWidget* w )
{
    if ( w->parentWidget() == NULL )
        return w;

    if ( w->autoFillBackground() )
    {
        const QBrush brush = w->palette().brush( w->backgroundRole() );
        if ( brush.color().alpha() > 0 )
            return w;
    }

    if ( w->testAttribute( Qt::WA_StyledBackground ) )
    {
        QImage image( 1, 1, QImage::Format_ARGB32 );
        image.fill( Qt::transparent );

        QPainter painter( &image );
        painter.translate( -w->rect().center() );
        qwtDrawStyledBackground( w, &painter );
        painter.end();

        if ( qAlpha( image.pixel( 0, 0 ) ) != 0 )
            return w;
    }

    return qwtBackgroundWidget( w->parentWidget() );
}

static void qwtFillBackground( QPainter* painter,
    QWidget* widget, const QVector< QRectF >& fillRects )
{
    if ( fillRects.isEmpty() )
        return;

    QRegion clipRegion;
    if ( painter->hasClipping() )
        clipRegion = painter->transform().map( painter->clipRegion() );
    else
        clipRegion = widget->contentsRect();

    // Try to find out which widget fills
    // the unfilled areas of the styled background

    QWidget* bgWidget = qwtBackgroundWidget( widget->parentWidget() );

    for ( int i = 0; i < fillRects.size(); i++ )
    {
        const QRect rect = fillRects[i].toAlignedRect();
        if ( clipRegion.intersects( rect ) )
        {
            QPixmap pm( rect.size() );
            QwtPainter::fillPixmap( bgWidget, pm, widget->mapTo( bgWidget, rect.topLeft() ) );
            painter->drawPixmap( rect, pm );
        }
    }
}

static void qwtFillBackground( QPainter* painter, QWidget* canvas )
{
    QVector< QRectF > rects;

    if ( canvas->testAttribute( Qt::WA_StyledBackground ) )
    {
        QwtStyleSheetRecorder recorder( canvas->size() );

        QPainter p( &recorder );
        qwtDrawStyledBackground( canvas, &p );
        p.end();

        if ( recorder.background.brush.isOpaque() )
            rects = recorder.clipRects;
        else
            rects += canvas->rect();
    }
    else
    {
        const double borderRadius = canvas->property( "borderRadius" ).toDouble();
        if ( borderRadius > 0.0 )
        {
            QSizeF sz( borderRadius, borderRadius );

            const QRectF r = canvas->rect();
            rects += QRectF( r.topLeft(), sz );
            rects += QRectF( r.topRight() - QPointF( borderRadius, 0 ), sz );
            rects += QRectF( r.bottomRight() - QPointF( borderRadius, borderRadius ), sz );
            rects += QRectF( r.bottomLeft() - QPointF( 0, borderRadius ), sz );
        }
    }

    qwtFillBackground( painter, canvas, rects);
}

static inline void qwtRevertPath( QPainterPath& path )
{
    if ( path.elementCount() == 4 )
    {
        QPainterPath::Element el0 = path.elementAt(0);
        QPainterPath::Element el3 = path.elementAt(3);

        path.setElementPositionAt( 0, el3.x, el3.y );
        path.setElementPositionAt( 3, el0.x, el0.y );
    }
}

static QPainterPath qwtCombinePathList( const QRectF& rect,
    const QList< QPainterPath >& pathList )
{
    if ( pathList.isEmpty() )
        return QPainterPath();

    QPainterPath ordered[8]; // starting top left

    for ( int i = 0; i < pathList.size(); i++ )
    {
        int index = -1;
        QPainterPath subPath = pathList[i];

        const QRectF br = pathList[i].controlPointRect();
        if ( br.center().x() < rect.center().x() )
        {
            if ( br.center().y() < rect.center().y() )
            {
                if ( qAbs( br.top() - rect.top() ) <
                    qAbs( br.left() - rect.left() ) )
                {
                    index = 1;
                }
                else
                {
                    index = 0;
                }
            }
            else
            {
                if ( qAbs( br.bottom() - rect.bottom() ) <
                    qAbs( br.left() - rect.left() ) )
                {
                    index = 6;
                }
                else
                {
                    index = 7;
                }
            }

            if ( subPath.currentPosition().y() > br.center().y() )
                qwtRevertPath( subPath );
        }
        else
        {
            if ( br.center().y() < rect.center().y() )
            {
                if ( qAbs( br.top() - rect.top() ) <
                    qAbs( br.right() - rect.right() ) )
                {
                    index = 2;
                }
                else
                {
                    index = 3;
                }
            }
            else
            {
                if ( qAbs( br.bottom() - rect.bottom() ) <
                    qAbs( br.right() - rect.right() ) )
                {
                    index = 5;
                }
                else
                {
                    index = 4;
                }
            }
            if ( subPath.currentPosition().y() < br.center().y() )
                qwtRevertPath( subPath );
        }
        ordered[index] = subPath;
    }

    for ( int i = 0; i < 4; i++ )
    {
        if ( ordered[ 2 * i].isEmpty() != ordered[2 * i + 1].isEmpty() )
        {
            // we don't accept incomplete rounded borders
            return QPainterPath();
        }
    }


    const QPolygonF corners( rect );

    QPainterPath path;
    //path.moveTo( rect.topLeft() );

    for ( int i = 0; i < 4; i++ )
    {
        if ( ordered[2 * i].isEmpty() )
        {
            path.lineTo( corners[i] );
        }
        else
        {
            path.connectPath( ordered[2 * i] );
            path.connectPath( ordered[2 * i + 1] );
        }
    }

    path.closeSubpath();

#if 0
    return path.simplified();
#else
    return path;
#endif
}

static QPainterPath qwtBorderPath( const QWidget* canvas, const QRect& rect )
{
    if ( canvas->testAttribute(Qt::WA_StyledBackground ) )
    {
        QwtStyleSheetRecorder recorder( rect.size() );

        QPainter painter( &recorder );

        QStyleOption opt;
        opt.initFrom( canvas );
        opt.rect = rect;
        canvas->style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, canvas );

        painter.end();

        if ( !recorder.background.path.isEmpty() )
            return recorder.background.path;

        if ( !recorder.border.rectList.isEmpty() )
            return qwtCombinePathList( rect, recorder.border.pathList );
    }
    else
    {
        const double borderRadius = canvas->property( "borderRadius" ).toDouble();

        if ( borderRadius > 0.0 )
        {
            double fw2 = canvas->property( "frameWidth" ).toInt() * 0.5;
            QRectF r = QRectF(rect).adjusted( fw2, fw2, -fw2, -fw2 );

            QPainterPath path;
            path.addRoundedRect( r, borderRadius, borderRadius );
            return path;
        }
    }

    return QPainterPath();
}

class QwtPlotAbstractCanvas::PrivateData
{
  public:
    PrivateData()
        : focusIndicator( NoFocusIndicator )
        , borderRadius( 0 )
    {
        styleSheet.hasBorder = false;
    }

    FocusIndicator focusIndicator;
    double borderRadius;

    struct StyleSheet
    {
        bool hasBorder;
        QPainterPath borderPath;
        QVector< QRectF > cornerRects;

        struct StyleSheetBackground
        {
            QBrush brush;
            QPointF origin;
        } background;

    } styleSheet;

    QWidget* canvasWidget;
};

/*!
   \brief Constructor
   \param canvasWidget plot canvas widget
 */
QwtPlotAbstractCanvas::QwtPlotAbstractCanvas( QWidget* canvasWidget )
{
    m_data = new PrivateData;
    m_data->canvasWidget = canvasWidget;

#ifndef QT_NO_CURSOR
    canvasWidget->setCursor( Qt::CrossCursor );
#endif
    canvasWidget->setAutoFillBackground( true );
}

//! Destructor
QwtPlotAbstractCanvas::~QwtPlotAbstractCanvas()
{
    delete m_data;
}

//! Return parent plot widget
QwtPlot* QwtPlotAbstractCanvas::plot()
{
    return qobject_cast< QwtPlot* >( m_data->canvasWidget->parent() );
}

//! Return parent plot widget
const QwtPlot* QwtPlotAbstractCanvas::plot() const
{
    return qobject_cast< const QwtPlot* >( m_data->canvasWidget->parent() );
}

/*!
   Set the focus indicator

   \sa FocusIndicator, focusIndicator()
 */
void QwtPlotAbstractCanvas::setFocusIndicator( FocusIndicator focusIndicator )
{
    m_data->focusIndicator = focusIndicator;
}

/*!
   \return Focus indicator

   \sa FocusIndicator, setFocusIndicator()
 */
QwtPlotAbstractCanvas::FocusIndicator QwtPlotAbstractCanvas::focusIndicator() const
{
    return m_data->focusIndicator;
}

/*!
   Draw the focus indication
   \param painter Painter
 */
void QwtPlotAbstractCanvas::drawFocusIndicator( QPainter* painter )
{
    const int margin = 1;

    QRect focusRect = m_data->canvasWidget->contentsRect();
    focusRect.setRect( focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin );

    QwtPainter::drawFocusRect( painter, m_data->canvasWidget, focusRect );
}

/*!
   Set the radius for the corners of the border frame

   \param radius Radius of a rounded corner
   \sa borderRadius()
 */
void QwtPlotAbstractCanvas::setBorderRadius( double radius )
{
    m_data->borderRadius = qwtMaxF( 0.0, radius );
}

/*!
   \return Radius for the corners of the border frame
   \sa setBorderRadius()
 */
double QwtPlotAbstractCanvas::borderRadius() const
{
    return m_data->borderRadius;
}

//! \return Path for the canvas border
QPainterPath QwtPlotAbstractCanvas::canvasBorderPath( const QRect& rect ) const
{
    return qwtBorderPath( canvasWidget(), rect );
}

/*!
   Draw the border of the canvas
   \param painter Painter
 */
void QwtPlotAbstractCanvas::drawBorder( QPainter* painter )
{
    const QWidget* w = canvasWidget();

    if ( m_data->borderRadius > 0 )
    {
        const int frameWidth = w->property( "frameWidth" ).toInt();
        if ( frameWidth > 0 )
        {
            const int frameShape = w->property( "frameShape" ).toInt();
            const int frameShadow = w->property( "frameShadow" ).toInt();

            const QRectF frameRect = w->property( "frameRect" ).toRect();

            QwtPainter::drawRoundedFrame( painter, frameRect,
                m_data->borderRadius, m_data->borderRadius,
                w->palette(), frameWidth, frameShape | frameShadow );
        }
    }
    else
    {
        const int frameShape = w->property( "frameShape" ).toInt();
        const int frameShadow = w->property( "frameShadow" ).toInt();

#if QT_VERSION < 0x050000
        QStyleOptionFrameV3 opt;
#else
        QStyleOptionFrame opt;
#endif
        opt.initFrom( w );

        opt.frameShape = QFrame::Shape( int( opt.frameShape ) | frameShape );

        switch (frameShape)
        {
            case QFrame::Box:
            case QFrame::HLine:
            case QFrame::VLine:
            case QFrame::StyledPanel:
            case QFrame::Panel:
            {
                opt.lineWidth = w->property( "lineWidth" ).toInt();
                opt.midLineWidth = w->property( "midLineWidth" ).toInt();
                break;
            }
            default:
            {
                opt.lineWidth = w->property( "frameWidth" ).toInt();
                break;
            }
        }

        if ( frameShadow == QFrame::Sunken )
            opt.state |= QStyle::State_Sunken;
        else if ( frameShadow == QFrame::Raised )
            opt.state |= QStyle::State_Raised;

        w->style()->drawControl(QStyle::CE_ShapedFrame, &opt, painter, w );
    }
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::drawBackground( QPainter* painter )
{
    qwtDrawBackground( painter, canvasWidget() );
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::fillBackground( QPainter* painter )
{
    qwtFillBackground( painter, canvasWidget() );
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::drawUnstyled( QPainter* painter )
{
    fillBackground( painter );

    QWidget* w = canvasWidget();

    if ( w->autoFillBackground() )
    {
        const QRect canvasRect = w->rect();

        painter->save();

        painter->setPen( Qt::NoPen );
        painter->setBrush( w->palette().brush( w->backgroundRole() ) );

        const QRect frameRect = w->property( "frameRect" ).toRect();
        if ( borderRadius() > 0.0 && ( canvasRect == frameRect ) )
        {
            const int frameWidth = w->property( "frameWidth" ).toInt();
            if ( frameWidth > 0 )
            {
                painter->setClipPath( canvasBorderPath( canvasRect ) );
                painter->drawRect( canvasRect );
            }
            else
            {
                painter->setRenderHint( QPainter::Antialiasing, true );
                painter->drawPath( canvasBorderPath( canvasRect ) );
            }
        }
        else
        {
            painter->drawRect( canvasRect );
        }

        painter->restore();
    }

    drawCanvas( painter );
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractCanvas::drawStyled( QPainter* painter, bool hackStyledBackground )
{
    fillBackground( painter );

    if ( hackStyledBackground )
    {
        // Antialiasing rounded borders is done by
        // inserting pixels with colors between the
        // border color and the color on the canvas,
        // When the border is painted before the plot items
        // these colors are interpolated for the canvas
        // and the plot items need to be clipped excluding
        // the antialiased pixels. In situations, where
        // the plot items fill the area at the rounded
        // borders this is noticeable.
        // The only way to avoid these annoying "artefacts"
        // is to paint the border on top of the plot items.

        if ( !m_data->styleSheet.hasBorder ||
            m_data->styleSheet.borderPath.isEmpty() )
        {
            // We have no border with at least one rounded corner
            hackStyledBackground = false;
        }
    }

    QWidget* w = canvasWidget();

    if ( hackStyledBackground )
    {
        painter->save();

        // paint background without border
        painter->setPen( Qt::NoPen );
        painter->setBrush( m_data->styleSheet.background.brush );
        painter->setBrushOrigin( m_data->styleSheet.background.origin );
        painter->setClipPath( m_data->styleSheet.borderPath );
        painter->drawRect( w->contentsRect() );

        painter->restore();

        drawCanvas( painter );

        // Now paint the border on top
        QStyleOptionFrame opt;
        opt.initFrom( w );
        w->style()->drawPrimitive( QStyle::PE_Frame, &opt, painter, w);
    }
    else
    {
        QStyleOption opt;
        opt.initFrom( w );
        w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w );

        drawCanvas( painter );
    }
}

//!  \brief Draw the plot to the canvas
void QwtPlotAbstractCanvas::drawCanvas( QPainter* painter )
{
    QWidget* w = canvasWidget();

    painter->save();

    if ( !m_data->styleSheet.borderPath.isEmpty() )
    {
        painter->setClipPath(
            m_data->styleSheet.borderPath, Qt::IntersectClip );
    }
    else
    {
        if ( borderRadius() > 0.0 )
        {
            const QRect frameRect = w->property( "frameRect" ).toRect();
            painter->setClipPath( canvasBorderPath( frameRect ), Qt::IntersectClip );
        }
        else
        {
            painter->setClipRect( w->contentsRect(), Qt::IntersectClip );
        }
    }

    QwtPlot* plot = qobject_cast< QwtPlot* >( w->parent() );
    if ( plot )
        plot->drawCanvas( painter );

    painter->restore();
}

//! Update the cached information about the current style sheet
void QwtPlotAbstractCanvas::updateStyleSheetInfo()
{
    QWidget* w = canvasWidget();

    if ( !w->testAttribute( Qt::WA_StyledBackground ) )
        return;

    QwtStyleSheetRecorder recorder( w->size() );

    QPainter painter( &recorder );

    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, w);

    painter.end();

    m_data->styleSheet.hasBorder = !recorder.border.rectList.isEmpty();
    m_data->styleSheet.cornerRects = recorder.clipRects;

    if ( recorder.background.path.isEmpty() )
    {
        if ( !recorder.border.rectList.isEmpty() )
        {
            m_data->styleSheet.borderPath =
                qwtCombinePathList( w->rect(), recorder.border.pathList );
        }
    }
    else
    {
        m_data->styleSheet.borderPath = recorder.background.path;
        m_data->styleSheet.background.brush = recorder.background.brush;
        m_data->styleSheet.background.origin = recorder.background.origin;
    }
}

//! \return canvas widget
QWidget* QwtPlotAbstractCanvas::canvasWidget()
{
    return m_data->canvasWidget;
}

//! \return canvas widget
const QWidget* QwtPlotAbstractCanvas::canvasWidget() const
{
    return m_data->canvasWidget;
}

class QwtPlotAbstractGLCanvas::PrivateData
{
  public:
    PrivateData():
        frameStyle( QFrame::Panel | QFrame::Sunken),
        lineWidth( 2 ),
        midLineWidth( 0 )
    {
    }

    QwtPlotAbstractGLCanvas::PaintAttributes paintAttributes;

    int frameStyle;
    int lineWidth;
    int midLineWidth;
};

/*!
   \brief Constructor
   \param canvasWidget plot canvas widget
 */
QwtPlotAbstractGLCanvas::QwtPlotAbstractGLCanvas( QWidget* canvasWidget ):
    QwtPlotAbstractCanvas( canvasWidget )
{
    m_data = new PrivateData;

    qwtUpdateContentsRect( frameWidth(), canvasWidget );
    m_data->paintAttributes = QwtPlotAbstractGLCanvas::BackingStore;
}

//! Destructor
QwtPlotAbstractGLCanvas::~QwtPlotAbstractGLCanvas()
{
    delete m_data;
}

/*!
   \brief Changing the paint attributes

   \param attribute Paint attribute
   \param on On/Off

   \sa testPaintAttribute()
 */
void QwtPlotAbstractGLCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( bool( m_data->paintAttributes & attribute ) == on )
        return;

    if ( on )
    {
        m_data->paintAttributes |= attribute;
    }
    else
    {
        m_data->paintAttributes &= ~attribute;

        if ( attribute == BackingStore )
            clearBackingStore();
    }
}

/*!
   Test whether a paint attribute is enabled

   \param attribute Paint attribute
   \return true, when attribute is enabled
   \sa setPaintAttribute()
 */
bool QwtPlotAbstractGLCanvas::testPaintAttribute( PaintAttribute attribute ) const
{
    return m_data->paintAttributes & attribute;
}

/*!
   Set the frame style

   \param style The bitwise OR between a shape and a shadow.

   \sa frameStyle(), QFrame::setFrameStyle(),
      setFrameShadow(), setFrameShape()
 */
void QwtPlotAbstractGLCanvas::setFrameStyle( int style )
{
    if ( style != m_data->frameStyle )
    {
        m_data->frameStyle = style;
        qwtUpdateContentsRect( frameWidth(), canvasWidget() );

        canvasWidget()->update();
    }
}

/*!
   \return The bitwise OR between a frameShape() and a frameShadow()
   \sa setFrameStyle(), QFrame::frameStyle()
 */
int QwtPlotAbstractGLCanvas::frameStyle() const
{
    return m_data->frameStyle;
}

/*!
   Set the frame shadow

   \param shadow Frame shadow
   \sa frameShadow(), setFrameShape(), QFrame::setFrameShadow()
 */
void QwtPlotAbstractGLCanvas::setFrameShadow( QFrame::Shadow shadow )
{
    setFrameStyle( ( m_data->frameStyle & QFrame::Shape_Mask ) | shadow );
}

/*!
   \return Frame shadow
   \sa setFrameShadow(), QFrame::setFrameShadow()
 */
QFrame::Shadow QwtPlotAbstractGLCanvas::frameShadow() const
{
    return (QFrame::Shadow) ( m_data->frameStyle & QFrame::Shadow_Mask );
}

/*!
   Set the frame shape

   \param shape Frame shape
   \sa frameShape(), setFrameShadow(), QFrame::frameShape()
 */
void QwtPlotAbstractGLCanvas::setFrameShape( QFrame::Shape shape )
{
    setFrameStyle( ( m_data->frameStyle & QFrame::Shadow_Mask ) | shape );
}

/*!
   \return Frame shape
   \sa setFrameShape(), QFrame::frameShape()
 */
QFrame::Shape QwtPlotAbstractGLCanvas::frameShape() const
{
    return (QFrame::Shape) ( m_data->frameStyle & QFrame::Shape_Mask );
}

/*!
   Set the frame line width

   The default line width is 2 pixels.

   \param width Line width of the frame
   \sa lineWidth(), setMidLineWidth()
 */
void QwtPlotAbstractGLCanvas::setLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != m_data->lineWidth )
    {
        m_data->lineWidth = qMax( width, 0 );
        qwtUpdateContentsRect( frameWidth(), canvasWidget() );
        canvasWidget()->update();
    }
}

/*!
   \return Line width of the frame
   \sa setLineWidth(), midLineWidth()
 */
int QwtPlotAbstractGLCanvas::lineWidth() const
{
    return m_data->lineWidth;
}

/*!
   Set the frame mid line width

   The default midline width is 0 pixels.

   \param width Midline width of the frame
   \sa midLineWidth(), setLineWidth()
 */
void QwtPlotAbstractGLCanvas::setMidLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != m_data->midLineWidth )
    {
        m_data->midLineWidth = width;
        qwtUpdateContentsRect( frameWidth(), canvasWidget() );
        canvasWidget()->update();
    }
}

/*!
   \return Midline width of the frame
   \sa setMidLineWidth(), lineWidth()
 */
int QwtPlotAbstractGLCanvas::midLineWidth() const
{
    return m_data->midLineWidth;
}

/*!
   \return Frame width depending on the style, line width and midline width.
 */
int QwtPlotAbstractGLCanvas::frameWidth() const
{
    return ( frameStyle() != QFrame::NoFrame ) ? m_data->lineWidth : 0;
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
 */
void QwtPlotAbstractGLCanvas::replot()
{
    invalidateBackingStore();

    QWidget* w = canvasWidget();
    if ( testPaintAttribute( QwtPlotAbstractGLCanvas::ImmediatePaint ) )
        w->repaint( w->contentsRect() );
    else
        w->update( w->contentsRect() );
}

//! \return The rectangle where the frame is drawn in.
QRect QwtPlotAbstractGLCanvas::frameRect() const
{
    const int fw = frameWidth();
    return canvasWidget()->contentsRect().adjusted( -fw, -fw, fw, fw );
}

//! Helper function for the derived plot canvas
void QwtPlotAbstractGLCanvas::draw( QPainter* painter )
{
#if FIX_GL_TRANSLATION
    if ( painter->paintEngine()->type() == QPaintEngine::OpenGL2 )
    {
        // work around a translation bug of QPaintEngine::OpenGL2
        painter->translate( 1, 1 );
    }
#endif

    if ( canvasWidget()->testAttribute( Qt::WA_StyledBackground ) )
        drawStyled( painter, true );
    else
        drawUnstyled( painter );

    if ( frameWidth() > 0 )
        drawBorder( painter );
}
