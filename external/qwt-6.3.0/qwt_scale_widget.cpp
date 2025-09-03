/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_scale_widget.h"
#include "qwt_painter.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt_scale_div.h"
#include "qwt_text.h"
#include "qwt_interval.h"
#include "qwt_scale_engine.h"

#include <qpainter.h>
#include <qevent.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qapplication.h>
#include <qmargins.h>

class QwtScaleWidget::PrivateData
{
  public:
    PrivateData()
        : scaleDraw( NULL )
    {
        colorBar.colorMap = NULL;
    }

    ~PrivateData()
    {
        delete scaleDraw;
        delete colorBar.colorMap;
    }

    QwtScaleDraw* scaleDraw;

    int borderDist[2];
    int minBorderDist[2];
    int scaleLength;
    int margin;

    int titleOffset;
    int spacing;
    QwtText title;

    QwtScaleWidget::LayoutFlags layoutFlags;

    struct t_colorBar
    {
        bool isEnabled;
        int width;
        QwtInterval interval;
        QwtColorMap* colorMap;
    } colorBar;
};

/*!
   \brief Create a scale with the position QwtScaleWidget::Left
   \param parent Parent widget
 */
QwtScaleWidget::QwtScaleWidget( QWidget* parent )
    : QWidget( parent )
{
    initScale( QwtScaleDraw::LeftScale );
}

/*!
   \brief Constructor
   \param align Alignment.
   \param parent Parent widget
 */
QwtScaleWidget::QwtScaleWidget( QwtScaleDraw::Alignment align, QWidget* parent )
    : QWidget( parent )
{
    initScale( align );
}

//! Destructor
QwtScaleWidget::~QwtScaleWidget()
{
    delete m_data;
}

//! Initialize the scale
void QwtScaleWidget::initScale( QwtScaleDraw::Alignment align )
{
    m_data = new PrivateData;

    if ( align == QwtScaleDraw::RightScale )
        m_data->layoutFlags |= TitleInverted;

    m_data->borderDist[0] = 0;
    m_data->borderDist[1] = 0;
    m_data->minBorderDist[0] = 0;
    m_data->minBorderDist[1] = 0;
    m_data->margin = 4;
    m_data->titleOffset = 0;
    m_data->spacing = 2;

    m_data->scaleDraw = new QwtScaleDraw;
    m_data->scaleDraw->setAlignment( align );
    m_data->scaleDraw->setLength( 10 );

    m_data->scaleDraw->setScaleDiv(
        QwtLinearScaleEngine().divideScale( 0.0, 100.0, 10, 5 ) );

    m_data->colorBar.colorMap = new QwtLinearColorMap();
    m_data->colorBar.isEnabled = false;
    m_data->colorBar.width = 10;

    const int flags = Qt::AlignHCenter
        | Qt::TextExpandTabs | Qt::TextWordWrap;
    m_data->title.setRenderFlags( flags );
    m_data->title.setFont( font() );

    QSizePolicy policy( QSizePolicy::MinimumExpanding,
        QSizePolicy::Fixed );
    if ( m_data->scaleDraw->orientation() == Qt::Vertical )
        policy.transpose();

    setSizePolicy( policy );

    setAttribute( Qt::WA_WState_OwnSizePolicy, false );
}

/*!
   Toggle an layout flag

   \param flag Layout flag
   \param on true/false

   \sa testLayoutFlag(), LayoutFlag
 */
void QwtScaleWidget::setLayoutFlag( LayoutFlag flag, bool on )
{
    if ( ( ( m_data->layoutFlags & flag ) != 0 ) != on )
    {
        if ( on )
            m_data->layoutFlags |= flag;
        else
            m_data->layoutFlags &= ~flag;

        update();
    }
}

/*!
   Test a layout flag

   \param flag Layout flag
   \return true/false
   \sa setLayoutFlag(), LayoutFlag
 */
bool QwtScaleWidget::testLayoutFlag( LayoutFlag flag ) const
{
    return ( m_data->layoutFlags & flag );
}

/*!
   Give title new text contents

   \param title New title
   \sa title(), setTitle(const QwtText &);
 */
void QwtScaleWidget::setTitle( const QString& title )
{
    if ( m_data->title.text() != title )
    {
        m_data->title.setText( title );
        layoutScale();
    }
}

/*!
   Give title new text contents

   \param title New title
   \sa title()
   \warning The title flags are interpreted in
               direction of the label, AlignTop, AlignBottom can't be set
               as the title will always be aligned to the scale.
 */
void QwtScaleWidget::setTitle( const QwtText& title )
{
    QwtText t = title;
    const int flags = title.renderFlags() & ~( Qt::AlignTop | Qt::AlignBottom );
    t.setRenderFlags( flags );

    if ( t != m_data->title )
    {
        m_data->title = t;
        layoutScale();
    }
}

/*!
   Change the alignment

   \param alignment New alignment
   \sa alignment()
 */
void QwtScaleWidget::setAlignment( QwtScaleDraw::Alignment alignment )
{
    if ( m_data->scaleDraw == NULL )
        return;

    m_data->scaleDraw->setAlignment( alignment );

    if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
    {
        QSizePolicy policy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
        if ( m_data->scaleDraw->orientation() == Qt::Vertical )
            policy.transpose();

        setSizePolicy( policy );

        setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    }

    layoutScale();
}


/*!
    \return position
    \sa setPosition()
 */
QwtScaleDraw::Alignment QwtScaleWidget::alignment() const
{
    if ( !scaleDraw() )
        return QwtScaleDraw::LeftScale;

    return scaleDraw()->alignment();
}

/*!
   Specify distances of the scale's endpoints from the
   widget's borders. The actual borders will never be less
   than minimum border distance.
   \param dist1 Left or top Distance
   \param dist2 Right or bottom distance
   \sa borderDist()
 */
void QwtScaleWidget::setBorderDist( int dist1, int dist2 )
{
    if ( dist1 != m_data->borderDist[0] || dist2 != m_data->borderDist[1] )
    {
        m_data->borderDist[0] = dist1;
        m_data->borderDist[1] = dist2;
        layoutScale();
    }
}

/*!
   \brief Specify the margin to the colorBar/base line.
   \param margin Margin
   \sa margin()
 */
void QwtScaleWidget::setMargin( int margin )
{
    margin = qMax( 0, margin );
    if ( margin != m_data->margin )
    {
        m_data->margin = margin;
        layoutScale();
    }
}

/*!
   \brief Specify the distance between color bar, scale and title
   \param spacing Spacing
   \sa spacing()
 */
void QwtScaleWidget::setSpacing( int spacing )
{
    spacing = qMax( 0, spacing );
    if ( spacing != m_data->spacing )
    {
        m_data->spacing = spacing;
        layoutScale();
    }
}

/*!
   \brief Change the alignment for the labels.

   \sa QwtScaleDraw::setLabelAlignment(), setLabelRotation()
 */
void QwtScaleWidget::setLabelAlignment( Qt::Alignment alignment )
{
    m_data->scaleDraw->setLabelAlignment( alignment );
    layoutScale();
}

/*!
   \brief Change the rotation for the labels.
   See QwtScaleDraw::setLabelRotation().

   \param rotation Rotation
   \sa QwtScaleDraw::setLabelRotation(), setLabelFlags()
 */
void QwtScaleWidget::setLabelRotation( double rotation )
{
    m_data->scaleDraw->setLabelRotation( rotation );
    layoutScale();
}

/*!
   Set a scale draw

   scaleDraw has to be created with new and will be deleted in
   ~QwtScaleWidget() or the next call of setScaleDraw().
   scaleDraw will be initialized with the attributes of
   the previous scaleDraw object.

   \param scaleDraw ScaleDraw object
   \sa scaleDraw()
 */
void QwtScaleWidget::setScaleDraw( QwtScaleDraw* scaleDraw )
{
    if ( ( scaleDraw == NULL ) || ( scaleDraw == m_data->scaleDraw ) )
        return;

    const QwtScaleDraw* sd = m_data->scaleDraw;
    if ( sd )
    {
        scaleDraw->setAlignment( sd->alignment() );
        scaleDraw->setScaleDiv( sd->scaleDiv() );

        QwtTransform* transform = NULL;
        if ( sd->scaleMap().transformation() )
            transform = sd->scaleMap().transformation()->copy();

        scaleDraw->setTransformation( transform );
    }

    delete m_data->scaleDraw;
    m_data->scaleDraw = scaleDraw;

    layoutScale();
}

/*!
    \return scaleDraw of this scale
    \sa setScaleDraw(), QwtScaleDraw::setScaleDraw()
 */
const QwtScaleDraw* QwtScaleWidget::scaleDraw() const
{
    return m_data->scaleDraw;
}

/*!
    \return scaleDraw of this scale
    \sa QwtScaleDraw::setScaleDraw()
 */
QwtScaleDraw* QwtScaleWidget::scaleDraw()
{
    return m_data->scaleDraw;
}

/*!
    \return title
    \sa setTitle()
 */
QwtText QwtScaleWidget::title() const
{
    return m_data->title;
}

/*!
    \return start border distance
    \sa setBorderDist()
 */
int QwtScaleWidget::startBorderDist() const
{
    return m_data->borderDist[0];
}

/*!
    \return end border distance
    \sa setBorderDist()
 */
int QwtScaleWidget::endBorderDist() const
{
    return m_data->borderDist[1];
}

/*!
    \return margin
    \sa setMargin()
 */
int QwtScaleWidget::margin() const
{
    return m_data->margin;
}

/*!
    \return distance between scale and title
    \sa setMargin()
 */
int QwtScaleWidget::spacing() const
{
    return m_data->spacing;
}

/*!
   \brief paintEvent
 */
void QwtScaleWidget::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    draw( &painter );
}

/*!
   \brief draw the scale
 */
void QwtScaleWidget::draw( QPainter* painter ) const
{
    m_data->scaleDraw->draw( painter, palette() );

    if ( m_data->colorBar.isEnabled && m_data->colorBar.width > 0 &&
        m_data->colorBar.interval.isValid() )
    {
        drawColorBar( painter, colorBarRect( contentsRect() ) );
    }

    QRect r = contentsRect();
    if ( m_data->scaleDraw->orientation() == Qt::Horizontal )
    {
        r.setLeft( r.left() + m_data->borderDist[0] );
        r.setWidth( r.width() - m_data->borderDist[1] );
    }
    else
    {
        r.setTop( r.top() + m_data->borderDist[0] );
        r.setHeight( r.height() - m_data->borderDist[1] );
    }

    if ( !m_data->title.isEmpty() )
        drawTitle( painter, m_data->scaleDraw->alignment(), r );
}

/*!
   Calculate the the rectangle for the color bar

   \param rect Bounding rectangle for all components of the scale
   \return Rectangle for the color bar
 */
QRectF QwtScaleWidget::colorBarRect( const QRectF& rect ) const
{
    QRectF cr = rect;

    if ( m_data->scaleDraw->orientation() == Qt::Horizontal )
    {
        cr.setLeft( cr.left() + m_data->borderDist[0] );
        cr.setWidth( cr.width() - m_data->borderDist[1] + 1 );
    }
    else
    {
        cr.setTop( cr.top() + m_data->borderDist[0] );
        cr.setHeight( cr.height() - m_data->borderDist[1] + 1 );
    }

    switch ( m_data->scaleDraw->alignment() )
    {
        case QwtScaleDraw::LeftScale:
        {
            cr.setLeft( cr.right() - m_data->margin
                - m_data->colorBar.width );
            cr.setWidth( m_data->colorBar.width );
            break;
        }

        case QwtScaleDraw::RightScale:
        {
            cr.setLeft( cr.left() + m_data->margin );
            cr.setWidth( m_data->colorBar.width );
            break;
        }

        case QwtScaleDraw::BottomScale:
        {
            cr.setTop( cr.top() + m_data->margin );
            cr.setHeight( m_data->colorBar.width );
            break;
        }

        case QwtScaleDraw::TopScale:
        {
            cr.setTop( cr.bottom() - m_data->margin
                - m_data->colorBar.width );
            cr.setHeight( m_data->colorBar.width );
            break;
        }
    }

    return cr;
}

/*!
   Change Event handler
   \param event Change event

   Invalidates internal caches if necessary
 */
void QwtScaleWidget::changeEvent( QEvent* event )
{
    if ( event->type() == QEvent::LocaleChange )
    {
        m_data->scaleDraw->invalidateCache();
    }

    QWidget::changeEvent( event );
}

/*!
   Event handler for resize events
   \param event Resize event
 */
void QwtScaleWidget::resizeEvent( QResizeEvent* event )
{
    Q_UNUSED( event );
    layoutScale( false );
}

/*!
   Recalculate the scale's geometry and layout based on
   the current geometry and fonts.

   \param update_geometry Notify the layout system and call update
                         to redraw the scale
 */

void QwtScaleWidget::layoutScale( bool update_geometry )
{
    int bd0, bd1;
    getBorderDistHint( bd0, bd1 );
    if ( m_data->borderDist[0] > bd0 )
        bd0 = m_data->borderDist[0];
    if ( m_data->borderDist[1] > bd1 )
        bd1 = m_data->borderDist[1];

    int colorBarWidth = 0;
    if ( m_data->colorBar.isEnabled && m_data->colorBar.interval.isValid() )
        colorBarWidth = m_data->colorBar.width + m_data->spacing;

    const QRectF r = contentsRect();
    double x, y, length;

    if ( m_data->scaleDraw->orientation() == Qt::Vertical )
    {
        y = r.top() + bd0;
        length = r.height() - ( bd0 + bd1 );

        if ( m_data->scaleDraw->alignment() == QwtScaleDraw::LeftScale )
            x = r.right() - 1.0 - m_data->margin - colorBarWidth;
        else
            x = r.left() + m_data->margin + colorBarWidth;
    }
    else
    {
        x = r.left() + bd0;
        length = r.width() - ( bd0 + bd1 );

        if ( m_data->scaleDraw->alignment() == QwtScaleDraw::BottomScale )
            y = r.top() + m_data->margin + colorBarWidth;
        else
            y = r.bottom() - 1.0 - m_data->margin - colorBarWidth;
    }

    m_data->scaleDraw->move( x, y );
    m_data->scaleDraw->setLength( length );

    const int extent = qwtCeil( m_data->scaleDraw->extent( font() ) );

    m_data->titleOffset =
        m_data->margin + m_data->spacing + colorBarWidth + extent;

    if ( update_geometry )
    {
        updateGeometry();

#if 1
        /*
            for some reason updateGeometry does not send a LayoutRequest event
            when the parent is not visible and has no layout
         */

        if ( QWidget* w = parentWidget() )
        {
            if ( !w->isVisible() && w->layout() == NULL )
            {
                if ( w->testAttribute( Qt::WA_WState_Polished ) )
                    QApplication::postEvent( w, new QEvent( QEvent::LayoutRequest ) );
            }
        }
#endif

        update();
    }
}

/*!
   Draw the color bar of the scale widget

   \param painter Painter
   \param rect Bounding rectangle for the color bar

   \sa setColorBarEnabled()
 */
void QwtScaleWidget::drawColorBar( QPainter* painter, const QRectF& rect ) const
{
    if ( !m_data->colorBar.interval.isValid() )
        return;

    const QwtScaleDraw* sd = m_data->scaleDraw;

    QwtPainter::drawColorBar( painter, *m_data->colorBar.colorMap,
        m_data->colorBar.interval.normalized(), sd->scaleMap(),
        sd->orientation(), rect );
}

/*!
   Rotate and paint a title according to its position into a given rectangle.

   \param painter Painter
   \param align Alignment
   \param rect Bounding rectangle
 */

void QwtScaleWidget::drawTitle( QPainter* painter,
    QwtScaleDraw::Alignment align, const QRectF& rect ) const
{
    QRectF r = rect;
    double angle;
    int flags = m_data->title.renderFlags() &
        ~( Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter );

    switch ( align )
    {
        case QwtScaleDraw::LeftScale:
            angle = -90.0;
            flags |= Qt::AlignTop;
            r.setRect( r.left(), r.bottom(),
                r.height(), r.width() - m_data->titleOffset );
            break;

        case QwtScaleDraw::RightScale:
            angle = -90.0;
            flags |= Qt::AlignTop;
            r.setRect( r.left() + m_data->titleOffset, r.bottom(),
                r.height(), r.width() - m_data->titleOffset );
            break;

        case QwtScaleDraw::BottomScale:
            angle = 0.0;
            flags |= Qt::AlignBottom;
            r.setTop( r.top() + m_data->titleOffset );
            break;

        case QwtScaleDraw::TopScale:
        default:
            angle = 0.0;
            flags |= Qt::AlignTop;
            r.setBottom( r.bottom() - m_data->titleOffset );
            break;
    }

    if ( m_data->layoutFlags & TitleInverted )
    {
        if ( align == QwtScaleDraw::LeftScale
            || align == QwtScaleDraw::RightScale )
        {
            angle = -angle;
            r.setRect( r.x() + r.height(), r.y() - r.width(),
                r.width(), r.height() );
        }
    }

    painter->save();
    painter->setFont( font() );
    painter->setPen( palette().color( QPalette::Text ) );

    painter->translate( r.x(), r.y() );
    if ( angle != 0.0 )
        painter->rotate( angle );

    QwtText title = m_data->title;
    title.setRenderFlags( flags );
    title.draw( painter, QRectF( 0.0, 0.0, r.width(), r.height() ) );

    painter->restore();
}

/*!
   \brief Notify a change of the scale

   This virtual function can be overloaded by derived
   classes. The default implementation updates the geometry
   and repaints the widget.
 */

void QwtScaleWidget::scaleChange()
{
    layoutScale();
}

/*!
   \return a size hint
 */
QSize QwtScaleWidget::sizeHint() const
{
    return minimumSizeHint();
}

/*!
   \return a minimum size hint
 */
QSize QwtScaleWidget::minimumSizeHint() const
{
    const Qt::Orientation o = m_data->scaleDraw->orientation();

    // Border Distance cannot be less than the scale borderDistHint
    // Note, the borderDistHint is already included in minHeight/minWidth
    int length = 0;
    int mbd1, mbd2;
    getBorderDistHint( mbd1, mbd2 );
    length += qMax( 0, m_data->borderDist[0] - mbd1 );
    length += qMax( 0, m_data->borderDist[1] - mbd2 );
    length += m_data->scaleDraw->minLength( font() );

    int dim = dimForLength( length, font() );
    if ( length < dim )
    {
        // compensate for long titles
        length = dim;
        dim = dimForLength( length, font() );
    }

    QSize size( length + 2, dim );
    if ( o == Qt::Vertical )
        size.transpose();

    const QMargins m = contentsMargins();
    return size + QSize( m.left() + m.right(), m.top() + m.bottom() );
}

/*!
   \brief Find the height of the title for a given width.
   \param width Width
   \return height Height
 */

int QwtScaleWidget::titleHeightForWidth( int width ) const
{
    return qwtCeil( m_data->title.heightForWidth( width, font() ) );
}

/*!
   \brief Find the minimum dimension for a given length.
         dim is the height, length the width seen in
         direction of the title.
   \param length width for horizontal, height for vertical scales
   \param scaleFont Font of the scale
   \return height for horizontal, width for vertical scales
 */

int QwtScaleWidget::dimForLength( int length, const QFont& scaleFont ) const
{
    const int extent = qwtCeil( m_data->scaleDraw->extent( scaleFont ) );

    int dim = m_data->margin + extent + 1;

    if ( !m_data->title.isEmpty() )
        dim += titleHeightForWidth( length ) + m_data->spacing;

    if ( m_data->colorBar.isEnabled && m_data->colorBar.interval.isValid() )
        dim += m_data->colorBar.width + m_data->spacing;

    return dim;
}

/*!
   \brief Calculate a hint for the border distances.

   This member function calculates the distance
   of the scale's endpoints from the widget borders which
   is required for the mark labels to fit into the widget.
   The maximum of this distance an the minimum border distance
   is returned.

   \param start Return parameter for the border width at
               the beginning of the scale
   \param end Return parameter for the border width at the
             end of the scale

   \warning
   <ul> <li>The minimum border distance depends on the font.</ul>
   \sa setMinBorderDist(), getMinBorderDist(), setBorderDist()
 */
void QwtScaleWidget::getBorderDistHint( int& start, int& end ) const
{
    m_data->scaleDraw->getBorderDistHint( font(), start, end );

    if ( start < m_data->minBorderDist[0] )
        start = m_data->minBorderDist[0];

    if ( end < m_data->minBorderDist[1] )
        end = m_data->minBorderDist[1];
}

/*!
   Set a minimum value for the distances of the scale's endpoints from
   the widget borders. This is useful to avoid that the scales
   are "jumping", when the tick labels or their positions change
   often.

   \param start Minimum for the start border
   \param end Minimum for the end border
   \sa getMinBorderDist(), getBorderDistHint()
 */
void QwtScaleWidget::setMinBorderDist( int start, int end )
{
    m_data->minBorderDist[0] = start;
    m_data->minBorderDist[1] = end;
}

/*!
   Get the minimum value for the distances of the scale's endpoints from
   the widget borders.

   \param start Return parameter for the border width at
               the beginning of the scale
   \param end Return parameter for the border width at the
             end of the scale

   \sa setMinBorderDist(), getBorderDistHint()
 */
void QwtScaleWidget::getMinBorderDist( int& start, int& end ) const
{
    start = m_data->minBorderDist[0];
    end = m_data->minBorderDist[1];
}

/*!
   \brief Assign a scale division

   The scale division determines where to set the tick marks.

   \param scaleDiv Scale Division
   \sa For more information about scale divisions, see QwtScaleDiv.
 */
void QwtScaleWidget::setScaleDiv( const QwtScaleDiv& scaleDiv )
{
    QwtScaleDraw* sd = m_data->scaleDraw;
    if ( sd->scaleDiv() != scaleDiv )
    {
        sd->setScaleDiv( scaleDiv );
        layoutScale();

        Q_EMIT scaleDivChanged();
    }
}

/*!
   Set the transformation

   \param transformation Transformation
   \sa QwtAbstractScaleDraw::scaleDraw(), QwtScaleMap
 */
void QwtScaleWidget::setTransformation( QwtTransform* transformation )
{
    m_data->scaleDraw->setTransformation( transformation );
    layoutScale();
}

/*!
   En/disable a color bar associated to the scale
   \sa isColorBarEnabled(), setColorBarWidth()
 */
void QwtScaleWidget::setColorBarEnabled( bool on )
{
    if ( on != m_data->colorBar.isEnabled )
    {
        m_data->colorBar.isEnabled = on;
        layoutScale();
    }
}

/*!
   \return true, when the color bar is enabled
   \sa setColorBarEnabled(), setColorBarWidth()
 */
bool QwtScaleWidget::isColorBarEnabled() const
{
    return m_data->colorBar.isEnabled;
}

/*!
   Set the width of the color bar

   \param width Width
   \sa colorBarWidth(), setColorBarEnabled()
 */
void QwtScaleWidget::setColorBarWidth( int width )
{
    if ( width != m_data->colorBar.width )
    {
        m_data->colorBar.width = width;
        if ( isColorBarEnabled() )
            layoutScale();
    }
}

/*!
   \return Width of the color bar
   \sa setColorBarEnabled(), setColorBarEnabled()
 */
int QwtScaleWidget::colorBarWidth() const
{
    return m_data->colorBar.width;
}

/*!
   \return Value interval for the color bar
   \sa setColorMap(), colorMap()
 */
QwtInterval QwtScaleWidget::colorBarInterval() const
{
    return m_data->colorBar.interval;
}

/*!
   Set the color map and value interval, that are used for displaying
   the color bar.

   \param interval Value interval
   \param colorMap Color map

   \sa colorMap(), colorBarInterval()
 */
void QwtScaleWidget::setColorMap(
    const QwtInterval& interval, QwtColorMap* colorMap )
{
    m_data->colorBar.interval = interval;

    if ( colorMap != m_data->colorBar.colorMap )
    {
        delete m_data->colorBar.colorMap;
        m_data->colorBar.colorMap = colorMap;
    }

    if ( isColorBarEnabled() )
        layoutScale();
}

/*!
   \return Color map
   \sa setColorMap(), colorBarInterval()
 */
const QwtColorMap* QwtScaleWidget::colorMap() const
{
    return m_data->colorBar.colorMap;
}

#include "moc_qwt_scale_widget.cpp"
