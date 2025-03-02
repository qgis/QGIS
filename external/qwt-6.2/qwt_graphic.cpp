/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_graphic.h"
#include "qwt_painter_command.h"
#include "qwt_math.h"

#include <qvector.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainterpath.h>

#if QT_VERSION >= 0x050000

#include <qguiapplication.h>

static inline qreal qwtDevicePixelRatio()
{
    return qGuiApp ? qGuiApp->devicePixelRatio() : 1.0;
}

#endif

static bool qwtHasScalablePen( const QPainter* painter )
{
    const QPen pen = painter->pen();

    bool scalablePen = false;

    if ( pen.style() != Qt::NoPen && pen.brush().style() != Qt::NoBrush )
    {
        scalablePen = !pen.isCosmetic();
#if QT_VERSION < 0x050000
        if ( !scalablePen && pen.widthF() == 0.0 )
        {
            const QPainter::RenderHints hints = painter->renderHints();
            if ( hints.testFlag( QPainter::NonCosmeticDefaultPen ) )
                scalablePen = true;
        }
#endif
    }

    return scalablePen;
}

static QRectF qwtStrokedPathRect(
    const QPainter* painter, const QPainterPath& path )
{
    QPainterPathStroker stroker;
    stroker.setWidth( painter->pen().widthF() );
    stroker.setCapStyle( painter->pen().capStyle() );
    stroker.setJoinStyle( painter->pen().joinStyle() );
    stroker.setMiterLimit( painter->pen().miterLimit() );

    QRectF rect;
    if ( qwtHasScalablePen( painter ) )
    {
        QPainterPath stroke = stroker.createStroke( path );
        rect = painter->transform().map( stroke ).boundingRect();
    }
    else
    {
        QPainterPath mappedPath = painter->transform().map( path );
        mappedPath = stroker.createStroke( mappedPath );

        rect = mappedPath.boundingRect();
    }

    return rect;
}

static inline void qwtExecCommand(
    QPainter* painter, const QwtPainterCommand& cmd,
    QwtGraphic::RenderHints renderHints,
    const QTransform& transform,
    const QTransform* initialTransform )
{
    switch( cmd.type() )
    {
        case QwtPainterCommand::Path:
        {
            bool doMap = false;

            if ( painter->transform().isScaling() )
            {
                bool isCosmetic = painter->pen().isCosmetic();
#if QT_VERSION < 0x050000
                if ( isCosmetic && painter->pen().widthF() == 0.0 )
                {
                    QPainter::RenderHints hints = painter->renderHints();
                    if ( hints.testFlag( QPainter::NonCosmeticDefaultPen ) )
                        isCosmetic = false;
                }
#endif

                if ( isCosmetic )
                {
                    // OpenGL2 seems to be buggy for cosmetic pens.
                    // It interpolates curves in too rough steps then

                    doMap = painter->paintEngine()->type() == QPaintEngine::OpenGL2;
                }
                else
                {
                    doMap = renderHints.testFlag( QwtGraphic::RenderPensUnscaled );
                }
            }

            if ( doMap )
            {
                const QTransform tr = painter->transform();

                painter->resetTransform();

                QPainterPath path = tr.map( *cmd.path() );
                if ( initialTransform )
                {
                    painter->setTransform( *initialTransform );
                    path = initialTransform->inverted().map( path );
                }

                painter->drawPath( path );

                painter->setTransform( tr );
            }
            else
            {
                painter->drawPath( *cmd.path() );
            }
            break;
        }
        case QwtPainterCommand::Pixmap:
        {
            const QwtPainterCommand::PixmapData* data = cmd.pixmapData();
            painter->drawPixmap( data->rect, data->pixmap, data->subRect );
            break;
        }
        case QwtPainterCommand::Image:
        {
            const QwtPainterCommand::ImageData* data = cmd.imageData();
            painter->drawImage( data->rect, data->image,
                data->subRect, data->flags );
            break;
        }
        case QwtPainterCommand::State:
        {
            const QwtPainterCommand::StateData* data = cmd.stateData();

            if ( data->flags & QPaintEngine::DirtyPen )
                painter->setPen( data->pen );

            if ( data->flags & QPaintEngine::DirtyBrush )
                painter->setBrush( data->brush );

            if ( data->flags & QPaintEngine::DirtyBrushOrigin )
                painter->setBrushOrigin( data->brushOrigin );

            if ( data->flags & QPaintEngine::DirtyFont )
                painter->setFont( data->font );

            if ( data->flags & QPaintEngine::DirtyBackground )
            {
                painter->setBackgroundMode( data->backgroundMode );
                painter->setBackground( data->backgroundBrush );
            }

            if ( data->flags & QPaintEngine::DirtyTransform )
            {
                painter->setTransform( data->transform * transform );
            }

            if ( data->flags & QPaintEngine::DirtyClipEnabled )
                painter->setClipping( data->isClipEnabled );

            if ( data->flags & QPaintEngine::DirtyClipRegion )
            {
                painter->setClipRegion( data->clipRegion,
                    data->clipOperation );
            }

            if ( data->flags & QPaintEngine::DirtyClipPath )
            {
                painter->setClipPath( data->clipPath, data->clipOperation );
            }

            if ( data->flags & QPaintEngine::DirtyHints )
            {
                for ( int i = 0; i < 8; i++ )
                {
                    const QPainter::RenderHint hint = static_cast< QPainter::RenderHint >( 1 << i );
                    painter->setRenderHint( hint, data->renderHints.testFlag( hint ) );
                }
            }

            if ( data->flags & QPaintEngine::DirtyCompositionMode )
                painter->setCompositionMode( data->compositionMode );

            if ( data->flags & QPaintEngine::DirtyOpacity )
                painter->setOpacity( data->opacity );

            break;
        }
        default:
            break;
    }
}

class QwtGraphic::PathInfo
{
  public:
    PathInfo()
        : m_scalablePen( false )
    {
        // QVector needs a default constructor
    }

    PathInfo( const QRectF& pointRect,
            const QRectF& boundingRect, bool scalablePen )
        : m_pointRect( pointRect )
        , m_boundingRect( boundingRect )
        , m_scalablePen( scalablePen )
    {
    }

    inline QRectF scaledBoundingRect( qreal sx, qreal sy, bool scalePens ) const
    {
        if ( sx == 1.0 && sy == 1.0 )
            return m_boundingRect;

        QTransform transform;
        transform.scale( sx, sy );

        QRectF rect;
        if ( scalePens && m_scalablePen )
        {
            rect = transform.mapRect( m_boundingRect );
        }
        else
        {
            rect = transform.mapRect( m_pointRect );

            const qreal l = qAbs( m_pointRect.left() - m_boundingRect.left() );
            const qreal r = qAbs( m_pointRect.right() - m_boundingRect.right() );
            const qreal t = qAbs( m_pointRect.top() - m_boundingRect.top() );
            const qreal b = qAbs( m_pointRect.bottom() - m_boundingRect.bottom() );

            rect.adjust( -l, -t, r, b );
        }

        return rect;
    }

    inline double scaleFactorX( const QRectF& pathRect,
        const QRectF& targetRect, bool scalePens ) const
    {
        if ( pathRect.width() <= 0.0 )
            return 0.0;

        const QPointF p0 = m_pointRect.center();

        const qreal l = qAbs( pathRect.left() - p0.x() );
        const qreal r = qAbs( pathRect.right() - p0.x() );

        const double w = 2.0 * qwtMinF( l, r )
            * targetRect.width() / pathRect.width();

        double sx;
        if ( scalePens && m_scalablePen )
        {
            sx = w / m_boundingRect.width();
        }
        else
        {
            const qreal pw = qwtMaxF(
                qAbs( m_boundingRect.left() - m_pointRect.left() ),
                qAbs( m_boundingRect.right() - m_pointRect.right() ) );

            sx = ( w - 2 * pw ) / m_pointRect.width();
        }

        return sx;
    }

    inline double scaleFactorY( const QRectF& pathRect,
        const QRectF& targetRect, bool scalePens ) const
    {
        if ( pathRect.height() <= 0.0 )
            return 0.0;

        const QPointF p0 = m_pointRect.center();

        const qreal t = qAbs( pathRect.top() - p0.y() );
        const qreal b = qAbs( pathRect.bottom() - p0.y() );

        const qreal h = 2.0 * qwtMinF( t, b )
            * targetRect.height() / pathRect.height();

        double sy;
        if ( scalePens && m_scalablePen )
        {
            sy = h / m_boundingRect.height();
        }
        else
        {
            const qreal pw = qwtMaxF(
                qAbs( m_boundingRect.top() - m_pointRect.top() ),
                qAbs( m_boundingRect.bottom() - m_pointRect.bottom() ) );

            sy = ( h - 2 * pw ) / m_pointRect.height();
        }

        return sy;
    }

  private:
    QRectF m_pointRect;
    QRectF m_boundingRect;
    bool m_scalablePen;
};

class QwtGraphic::PrivateData
{
  public:
    PrivateData()
        : boundingRect( 0.0, 0.0, -1.0, -1.0 )
        , pointRect( 0.0, 0.0, -1.0, -1.0 )
    {
    }

    QSizeF defaultSize;
    QVector< QwtPainterCommand > commands;
    QVector< QwtGraphic::PathInfo > pathInfos;

    QRectF boundingRect;
    QRectF pointRect;

    QwtGraphic::CommandTypes commandTypes;
    QwtGraphic::RenderHints renderHints;
};

/*!
   \brief Constructor

   Initializes a null graphic
   \sa isNull()
 */
QwtGraphic::QwtGraphic()
{
    setMode( QwtNullPaintDevice::PathMode );
    m_data = new PrivateData;
}

/*!
   \brief Copy constructor

   \param other Source
   \sa operator=()
 */
QwtGraphic::QwtGraphic( const QwtGraphic& other )
{
    setMode( other.mode() );
    m_data = new PrivateData( *other.m_data );
}

//! Destructor
QwtGraphic::~QwtGraphic()
{
    delete m_data;
}

/*!
   \brief Assignment operator

   \param other Source
   \return A reference of this object
 */
QwtGraphic& QwtGraphic::operator=( const QwtGraphic& other )
{
    setMode( other.mode() );
    *m_data = *other.m_data;

    return *this;
}

/*!
   \brief Clear all stored commands
   \sa isNull()
 */
void QwtGraphic::reset()
{
    m_data->commands.clear();
    m_data->pathInfos.clear();

    m_data->commandTypes = CommandTypes();

    m_data->boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    m_data->pointRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    m_data->defaultSize = QSizeF();
}

/*!
   \return True, when no painter commands have been stored
   \sa isEmpty(), commands()
 */
bool QwtGraphic::isNull() const
{
    return m_data->commands.isEmpty();
}

/*!
   \return True, when the bounding rectangle is empty
   \sa boundingRect(), isNull()
 */
bool QwtGraphic::isEmpty() const
{
    return m_data->boundingRect.isEmpty();
}

/*!
   \return Types of painter commands being used
 */
QwtGraphic::CommandTypes QwtGraphic::commandTypes() const
{
    return m_data->commandTypes;
}

/*!
   Toggle an render hint

   \param hint Render hint
   \param on true/false

   \sa testRenderHint(), RenderHint
 */
void QwtGraphic::setRenderHint( RenderHint hint, bool on )
{
    if ( on )
        m_data->renderHints |= hint;
    else
        m_data->renderHints &= ~hint;
}

/*!
   Test a render hint

   \param hint Render hint
   \return true/false
   \sa setRenderHint(), RenderHint
 */
bool QwtGraphic::testRenderHint( RenderHint hint ) const
{
    return m_data->renderHints.testFlag( hint );
}

//! \return Render hints
QwtGraphic::RenderHints QwtGraphic::renderHints() const
{
    return m_data->renderHints;
}

/*!
   The bounding rectangle is the controlPointRect()
   extended by the areas needed for rendering the outlines
   with unscaled pens.

   \return Bounding rectangle of the graphic
   \sa controlPointRect(), scaledBoundingRect()
 */
QRectF QwtGraphic::boundingRect() const
{
    if ( m_data->boundingRect.width() < 0 )
        return QRectF();

    return m_data->boundingRect;
}

/*!
   The control point rectangle is the bounding rectangle
   of all control points of the paths and the target
   rectangles of the images/pixmaps.

   \return Control point rectangle
   \sa boundingRect(), scaledBoundingRect()
 */
QRectF QwtGraphic::controlPointRect() const
{
    if ( m_data->pointRect.width() < 0 )
        return QRectF();

    return m_data->pointRect;
}

/*!
   \brief Calculate the target rectangle for scaling the graphic

   \param sx Horizontal scaling factor
   \param sy Vertical scaling factor

   \note In case of paths that are painted with a cosmetic pen
        ( see QPen::isCosmetic() ) the target rectangle is different to
        multiplying the bounding rectangle.

   \return Scaled bounding rectangle
   \sa boundingRect(), controlPointRect()
 */
QRectF QwtGraphic::scaledBoundingRect( qreal sx, qreal sy ) const
{
    if ( sx == 1.0 && sy == 1.0 )
        return m_data->boundingRect;

    const bool scalePens = !( m_data->renderHints & RenderPensUnscaled );

    QTransform transform;
    transform.scale( sx, sy );

    QRectF rect = transform.mapRect( m_data->pointRect );

    for ( int i = 0; i < m_data->pathInfos.size(); i++ )
        rect |= m_data->pathInfos[i].scaledBoundingRect( sx, sy, scalePens );

    return rect;
}

//! \return Ceiled defaultSize()
QSize QwtGraphic::sizeMetrics() const
{
    const QSizeF sz = defaultSize();
    return QSize( qwtCeil( sz.width() ), qwtCeil( sz.height() ) );
}

/*!
   \brief Set a default size

   The default size is used in all methods rendering the graphic,
   where no size is explicitly specified. Assigning an empty size
   means, that the default size will be calculated from the bounding
   rectangle.

   The default setting is an empty size.

   \param size Default size

   \sa defaultSize(), boundingRect()
 */
void QwtGraphic::setDefaultSize( const QSizeF& size )
{
    const double w = qwtMaxF( 0.0, size.width() );
    const double h = qwtMaxF( 0.0, size.height() );

    m_data->defaultSize = QSizeF( w, h );
}

/*!
   \brief Default size

   When a non empty size has been assigned by setDefaultSize() this
   size will be returned. Otherwise the default size is the size
   of the bounding rectangle.

   The default size is used in all methods rendering the graphic,
   where no size is explicitly specified.

   \return Default size
   \sa setDefaultSize(), boundingRect()
 */
QSizeF QwtGraphic::defaultSize() const
{
    if ( !m_data->defaultSize.isEmpty() )
        return m_data->defaultSize;

    return boundingRect().size();
}

/*!
   Find the height for a given width

   The height is calculated using the aspect ratio of defaultSize().

   \param width Width

   \return Calculated height
   \sa defaultSize()
 */
qreal QwtGraphic::heightForWidth( qreal width ) const
{
    const QSizeF sz = defaultSize();
    if ( sz.isEmpty() )
        return 0.0;

    return sz.height() * width / sz.width();
}

/*!
   Find the width for a given height

   The width is calculated using the aspect ratio of defaultSize().

   \param height Height

   \return Calculated width
   \sa defaultSize()
 */
qreal QwtGraphic::widthForHeight( qreal height ) const
{
    const QSizeF sz = defaultSize();
    if ( sz.isEmpty() )
        return 0.0;

    return sz.width() * height / sz.height();
}

/*!
   \brief Replay all recorded painter commands
   \param painter Qt painter
 */
void QwtGraphic::render( QPainter* painter ) const
{
    renderGraphic( painter, NULL );
}

void QwtGraphic::renderGraphic( QPainter* painter, QTransform* initialTransform ) const
{
    if ( isNull() )
        return;

    const int numCommands = m_data->commands.size();
    const QwtPainterCommand* commands = m_data->commands.constData();

    const QTransform transform = painter->transform();

    painter->save();

    for ( int i = 0; i < numCommands; i++ )
    {
        qwtExecCommand( painter, commands[i],
            m_data->renderHints, transform, initialTransform );
    }

    painter->restore();
}

/*!
   \brief Replay all recorded painter commands

   The graphic is scaled to fit into the rectangle
   of the given size starting at ( 0, 0 ).

   \param painter Qt painter
   \param size Size for the scaled graphic
   \param aspectRatioMode Mode how to scale - See Qt::AspectRatioMode
 */
void QwtGraphic::render( QPainter* painter, const QSizeF& size,
    Qt::AspectRatioMode aspectRatioMode ) const
{
    const QRectF r( 0.0, 0.0, size.width(), size.height() );
    render( painter, r, aspectRatioMode );
}

/*!
   \brief Replay all recorded painter commands

   The graphic is scaled to fit into the given rectangle

   \param painter Qt painter
   \param rect Rectangle for the scaled graphic
   \param aspectRatioMode Mode how to scale - See Qt::AspectRatioMode
 */
void QwtGraphic::render( QPainter* painter, const QRectF& rect,
    Qt::AspectRatioMode aspectRatioMode ) const
{
    if ( isEmpty() || rect.isEmpty() )
        return;

    double sx = 1.0;
    double sy = 1.0;

    if ( m_data->pointRect.width() > 0.0 )
        sx = rect.width() / m_data->pointRect.width();

    if ( m_data->pointRect.height() > 0.0 )
        sy = rect.height() / m_data->pointRect.height();

    const bool scalePens = !m_data->renderHints.testFlag( RenderPensUnscaled );

    for ( int i = 0; i < m_data->pathInfos.size(); i++ )
    {
        const PathInfo& info = m_data->pathInfos[i];

        const double ssx = info.scaleFactorX(
            m_data->pointRect, rect, scalePens );

        if ( ssx > 0.0 )
            sx = qwtMinF( sx, ssx );

        const double ssy = info.scaleFactorY(
            m_data->pointRect, rect, scalePens );

        if ( ssy > 0.0 )
            sy = qwtMinF( sy, ssy );
    }

    if ( aspectRatioMode == Qt::KeepAspectRatio )
    {
        const qreal s = qwtMinF( sx, sy );
        sx = s;
        sy = s;
    }
    else if ( aspectRatioMode == Qt::KeepAspectRatioByExpanding )
    {
        const qreal s = qwtMaxF( sx, sy );
        sx = s;
        sy = s;
    }

    QTransform tr;
    tr.translate( rect.center().x() - 0.5 * sx * m_data->pointRect.width(),
        rect.center().y() - 0.5 * sy * m_data->pointRect.height() );
    tr.scale( sx, sy );
    tr.translate( -m_data->pointRect.x(), -m_data->pointRect.y() );

    const QTransform transform = painter->transform();

    painter->setTransform( tr, true );

    if ( !scalePens && transform.isScaling() )
    {
        // we don't want to scale pens according to sx/sy,
        // but we want to apply the scaling from the
        // painter transformation later

        QTransform initialTransform;
        initialTransform.scale( transform.m11(), transform.m22() );

        renderGraphic( painter, &initialTransform );
    }
    else
    {
        renderGraphic( painter, NULL );
    }

    painter->setTransform( transform );
}

/*!
   \brief Replay all recorded painter commands

   The graphic is scaled to the defaultSize() and aligned
   to a position.

   \param painter Qt painter
   \param pos Reference point, where to render
   \param alignment Flags how to align the target rectangle to pos.
 */
void QwtGraphic::render( QPainter* painter,
    const QPointF& pos, Qt::Alignment alignment ) const
{
    QRectF r( pos, defaultSize() );

    if ( alignment & Qt::AlignLeft )
    {
        r.moveLeft( pos.x() );
    }
    else if ( alignment & Qt::AlignHCenter )
    {
        r.moveCenter( QPointF( pos.x(), r.center().y() ) );
    }
    else if ( alignment & Qt::AlignRight )
    {
        r.moveRight( pos.x() );
    }

    if ( alignment & Qt::AlignTop )
    {
        r.moveTop( pos.y() );
    }
    else if ( alignment & Qt::AlignVCenter )
    {
        r.moveCenter( QPointF( r.center().x(), pos.y() ) );
    }
    else if ( alignment & Qt::AlignBottom )
    {
        r.moveBottom( pos.y() );
    }

    render( painter, r );
}

/*!
   \brief Convert the graphic to a QPixmap

   All pixels of the pixmap get initialized by Qt::transparent
   before the graphic is scaled and rendered on it.

   The size of the pixmap is the default size ( ceiled to integers )
   of the graphic.

   \param devicePixelRatio Device pixel ratio for the pixmap.
                          If devicePixelRatio <= 0.0 the pixmap
                          is initialized with the system default.

   \return The graphic as pixmap in default size
   \sa defaultSize(), toImage(), render()
 */
QPixmap QwtGraphic::toPixmap( qreal devicePixelRatio ) const
{
    if ( isNull() )
        return QPixmap();

    const QSizeF sz = defaultSize();

    const int w = qwtCeil( sz.width() );
    const int h = qwtCeil( sz.height() );

    QPixmap pixmap( w, h );

#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    pixmap.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
#endif

    pixmap.fill( Qt::transparent );

    const QRectF r( 0.0, 0.0, sz.width(), sz.height() );

    QPainter painter( &pixmap );
    render( &painter, r, Qt::KeepAspectRatio );
    painter.end();

    return pixmap;
}

/*!
   \brief Convert the graphic to a QPixmap

   All pixels of the pixmap get initialized by Qt::transparent
   before the graphic is scaled and rendered on it.

   \param size Size of the image
   \param aspectRatioMode Aspect ratio how to scale the graphic
   \param devicePixelRatio Device pixel ratio for the pixmap.
                          If devicePixelRatio <= 0.0 the pixmap
                          is initialized with the system default.

   \return The graphic as pixmap
   \sa toImage(), render()
 */
QPixmap QwtGraphic::toPixmap( const QSize& size,
    Qt::AspectRatioMode aspectRatioMode, qreal devicePixelRatio ) const
{
    QPixmap pixmap( size );

#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    pixmap.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
#endif
    pixmap.fill( Qt::transparent );

    const QRect r( 0, 0, size.width(), size.height() );

    QPainter painter( &pixmap );
    render( &painter, r, aspectRatioMode );
    painter.end();

    return pixmap;
}

/*!
   \brief Convert the graphic to a QImage

   All pixels of the image get initialized by 0 ( transparent )
   before the graphic is scaled and rendered on it.

   The format of the image is QImage::Format_ARGB32_Premultiplied.

   \param size Size of the image ( will be multiplied by the devicePixelRatio )
   \param aspectRatioMode Aspect ratio how to scale the graphic
   \param devicePixelRatio Device pixel ratio for the image.
                          If devicePixelRatio <= 0.0 the pixmap
                          is initialized with the system default.

   \return The graphic as image
   \sa toPixmap(), render()
 */
QImage QwtGraphic::toImage( const QSize& size,
    Qt::AspectRatioMode aspectRatioMode, qreal devicePixelRatio  ) const
{
#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    QImage image( size* devicePixelRatio, QImage::Format_ARGB32_Premultiplied );
    image.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
    QImage image( size, QImage::Format_ARGB32_Premultiplied );
#endif

    image.fill( 0 );

    const QRect r( 0, 0, size.width(), size.height() );

    QPainter painter( &image );
    render( &painter, r, aspectRatioMode );
    painter.end();

    return image;
}

/*!
   \brief Convert the graphic to a QImage

   All pixels of the image get initialized by 0 ( transparent )
   before the graphic is scaled and rendered on it.

   The format of the image is QImage::Format_ARGB32_Premultiplied.

   The size of the image is the default size ( ceiled to integers )
   of the graphic multiplied by the devicePixelRatio.

   \param devicePixelRatio Device pixel ratio for the image.
                          If devicePixelRatio <= 0.0 the pixmap
                          is initialized with the system default.

   \return The graphic as image in default size
   \sa defaultSize(), toPixmap(), render()
 */
QImage QwtGraphic::toImage( qreal devicePixelRatio ) const
{
    if ( isNull() )
        return QImage();

    const QSizeF sz = defaultSize();

    int w = qwtCeil( sz.width() );
    int h = qwtCeil( sz.height() );

#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    w *= devicePixelRatio;
    h *= devicePixelRatio;

    QImage image( w, h, QImage::Format_ARGB32 );
    image.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
    QImage image( w, h, QImage::Format_ARGB32 );
#endif

    image.fill( 0 );

    const QRect r( 0, 0, sz.width(), sz.height() );

    QPainter painter( &image );
    render( &painter, r, Qt::KeepAspectRatio );
    painter.end();

    return image;
}

/*!
   Store a path command in the command list

   \param path Painter path
   \sa QPaintEngine::drawPath()
 */
void QwtGraphic::drawPath( const QPainterPath& path )
{
    const QPainter* painter = paintEngine()->painter();
    if ( painter == NULL )
        return;

    m_data->commands += QwtPainterCommand( path );
    m_data->commandTypes |= QwtGraphic::VectorData;

    if ( !path.isEmpty() )
    {
        const QPainterPath scaledPath = painter->transform().map( path );

        QRectF pointRect = scaledPath.boundingRect();
        QRectF boundingRect = pointRect;

        if ( painter->pen().style() != Qt::NoPen
            && painter->pen().brush().style() != Qt::NoBrush )
        {
            boundingRect = qwtStrokedPathRect( painter, path );
        }

        updateControlPointRect( pointRect );
        updateBoundingRect( boundingRect );

        m_data->pathInfos += PathInfo( pointRect,
            boundingRect, qwtHasScalablePen( painter ) );
    }
}

/*!
   \brief Store a pixmap command in the command list

   \param rect target rectangle
   \param pixmap Pixmap to be painted
   \param subRect Reactangle of the pixmap to be painted

   \sa QPaintEngine::drawPixmap()
 */
void QwtGraphic::drawPixmap( const QRectF& rect,
    const QPixmap& pixmap, const QRectF& subRect )
{
    const QPainter* painter = paintEngine()->painter();
    if ( painter == NULL )
        return;

    m_data->commands += QwtPainterCommand( rect, pixmap, subRect );
    m_data->commandTypes |= QwtGraphic::RasterData;

    const QRectF r = painter->transform().mapRect( rect );
    updateControlPointRect( r );
    updateBoundingRect( r );
}

/*!
   \brief Store a image command in the command list

   \param rect target rectangle
   \param image Image to be painted
   \param subRect Reactangle of the pixmap to be painted
   \param flags Image conversion flags

   \sa QPaintEngine::drawImage()
 */
void QwtGraphic::drawImage( const QRectF& rect, const QImage& image,
    const QRectF& subRect, Qt::ImageConversionFlags flags )
{
    const QPainter* painter = paintEngine()->painter();
    if ( painter == NULL )
        return;

    m_data->commands += QwtPainterCommand( rect, image, subRect, flags );
    m_data->commandTypes |= QwtGraphic::RasterData;

    const QRectF r = painter->transform().mapRect( rect );

    updateControlPointRect( r );
    updateBoundingRect( r );
}

/*!
   \brief Store a state command in the command list

   \param state State to be stored
   \sa QPaintEngine::updateState()
 */
void QwtGraphic::updateState( const QPaintEngineState& state )
{
    m_data->commands += QwtPainterCommand( state );

    if ( state.state() & QPaintEngine::DirtyTransform )
    {
        if ( !( m_data->commandTypes & QwtGraphic::Transformation ) )
        {
            /*
                QTransform::isScaling() returns true for all type
                of transformations beside simple translations
                even if it is f.e a rotation
             */
            if ( state.transform().isScaling() )
                m_data->commandTypes |= QwtGraphic::Transformation;
        }
    }
}

void QwtGraphic::updateBoundingRect( const QRectF& rect )
{
    QRectF br = rect;

    const QPainter* painter = paintEngine()->painter();
    if ( painter && painter->hasClipping() )
    {
        QRectF cr = painter->clipRegion().boundingRect();
        cr = painter->transform().mapRect( cr );

        br &= cr;
    }

    if ( m_data->boundingRect.width() < 0 )
        m_data->boundingRect = br;
    else
        m_data->boundingRect |= br;
}

void QwtGraphic::updateControlPointRect( const QRectF& rect )
{
    if ( m_data->pointRect.width() < 0.0 )
        m_data->pointRect = rect;
    else
        m_data->pointRect |= rect;
}

/*!
   \return List of recorded paint commands
   \sa setCommands()
 */
const QVector< QwtPainterCommand >& QwtGraphic::commands() const
{
    return m_data->commands;
}

/*!
   \brief Append paint commands

   \param commands Paint commands
   \sa commands()
 */
void QwtGraphic::setCommands( const QVector< QwtPainterCommand >& commands )
{
    reset();

    const int numCommands = commands.size();
    if ( numCommands <= 0 )
        return;

    // to calculate a proper bounding rectangle we don't simply copy
    // the commands.

    const QwtPainterCommand* cmds = commands.constData();

    const QTransform noTransform;
    const RenderHints noRenderHints;

    QPainter painter( this );
    for ( int i = 0; i < numCommands; i++ )
        qwtExecCommand( &painter, cmds[i], noRenderHints, noTransform, NULL );

    painter.end();
}
