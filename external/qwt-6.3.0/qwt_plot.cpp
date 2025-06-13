/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot.h"
#include "qwt_plot_dict.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_map.h"
#include "qwt_text_label.h"
#include "qwt_legend.h"
#include "qwt_legend_data.h"
#include "qwt_plot_canvas.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qpointer.h>
#include <qapplication.h>
#include <qcoreevent.h>

static inline void qwtEnableLegendItems( QwtPlot* plot, bool on )
{
    // gcc seems to have problems with const char sig[] in combination with certain options
    const char* sig = SIGNAL(legendDataChanged(QVariant,QList<QwtLegendData>));
    const char* slot = SLOT(updateLegendItems(QVariant,QList<QwtLegendData>));

    if ( on )
        QObject::connect( plot, sig, plot, slot );
    else
        QObject::disconnect( plot, sig, plot, slot );
}

static void qwtSetTabOrder(
    QWidget* first, QWidget* second, bool withChildren )
{
    QList< QWidget* > tabChain;
    tabChain += first;
    tabChain += second;

    if ( withChildren )
    {
        QList< QWidget* > children = second->findChildren< QWidget* >();

        QWidget* w = second->nextInFocusChain();
        while ( children.contains( w ) )
        {
            children.removeAll( w );

            tabChain += w;
            w = w->nextInFocusChain();
        }
    }

    for ( int i = 0; i < tabChain.size() - 1; i++ )
    {
        QWidget* from = tabChain[i];
        QWidget* to = tabChain[i + 1];

        const Qt::FocusPolicy policy1 = from->focusPolicy();
        const Qt::FocusPolicy policy2 = to->focusPolicy();

        QWidget* proxy1 = from->focusProxy();
        QWidget* proxy2 = to->focusProxy();

        from->setFocusPolicy( Qt::TabFocus );
        from->setFocusProxy( NULL);

        to->setFocusPolicy( Qt::TabFocus );
        to->setFocusProxy( NULL);

        QWidget::setTabOrder( from, to );

        from->setFocusPolicy( policy1 );
        from->setFocusProxy( proxy1);

        to->setFocusPolicy( policy2 );
        to->setFocusProxy( proxy2 );
    }
}

class QwtPlot::PrivateData
{
  public:
    QPointer< QwtTextLabel > titleLabel;
    QPointer< QwtTextLabel > footerLabel;
    QPointer< QWidget > canvas;
    QPointer< QwtAbstractLegend > legend;
    QwtPlotLayout* layout;

    bool autoReplot;
};

/*!
   \brief Constructor
   \param parent Parent widget
 */
QwtPlot::QwtPlot( QWidget* parent )
    : QFrame( parent )
{
    initPlot( QwtText() );
}

/*!
   \brief Constructor
   \param title Title text
   \param parent Parent widget
 */
QwtPlot::QwtPlot( const QwtText& title, QWidget* parent )
    : QFrame( parent )
{
    initPlot( title );
}

//! Destructor
QwtPlot::~QwtPlot()
{
    setAutoReplot( false );
    detachItems( QwtPlotItem::Rtti_PlotItem, autoDelete() );

    delete m_data->layout;
    deleteAxesData();
    delete m_data;
}

/*!
   \brief Initializes a QwtPlot instance
   \param title Title text
 */
void QwtPlot::initPlot( const QwtText& title )
{
    m_data = new PrivateData;

    m_data->layout = new QwtPlotLayout;
    m_data->autoReplot = false;

    // title
    m_data->titleLabel = new QwtTextLabel( this );
    m_data->titleLabel->setObjectName( "QwtPlotTitle" );
    m_data->titleLabel->setFont( QFont( fontInfo().family(), 14, QFont::Bold ) );

    QwtText text( title );
    text.setRenderFlags( Qt::AlignCenter | Qt::TextWordWrap );
    m_data->titleLabel->setText( text );

    // footer
    m_data->footerLabel = new QwtTextLabel( this );
    m_data->footerLabel->setObjectName( "QwtPlotFooter" );

    QwtText footer;
    footer.setRenderFlags( Qt::AlignCenter | Qt::TextWordWrap );
    m_data->footerLabel->setText( footer );

    // legend
    m_data->legend = NULL;

    // axes
    initAxesData();

    // canvas
    m_data->canvas = new QwtPlotCanvas( this );
    m_data->canvas->setObjectName( "QwtPlotCanvas" );
    m_data->canvas->installEventFilter( this );

    setSizePolicy( QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding );

    resize( 200, 200 );

    using namespace QwtAxis;

    QList< QWidget* > focusChain;
    focusChain << this << m_data->titleLabel << axisWidget( XTop )
               << axisWidget( YLeft ) << m_data->canvas
               << axisWidget( YRight ) << axisWidget( XBottom )
               << m_data->footerLabel;

    for ( int i = 0; i < focusChain.size() - 1; i++ )
        qwtSetTabOrder( focusChain[i], focusChain[i + 1], false );

    qwtEnableLegendItems( this, true );
}

/*!
   \brief Set the drawing canvas of the plot widget

   QwtPlot invokes methods of the canvas as meta methods ( see QMetaObject ).
   In opposite to using conventional C++ techniques like virtual methods
   they allow to use canvas implementations that are derived from
   QWidget or QGLWidget.

   The following meta methods could be implemented:

   - replot()
    When the canvas doesn't offer a replot method, QwtPlot calls
    update() instead.

   - borderPath()
    The border path is necessary to clip the content of the canvas
    When the canvas doesn't have any special border ( f.e rounded corners )
    it is o.k. not to implement this method.

   The default canvas is a QwtPlotCanvas

   \param canvas Canvas Widget
   \sa canvas()
 */
void QwtPlot::setCanvas( QWidget* canvas )
{
    if ( canvas == m_data->canvas )
        return;

    delete m_data->canvas;
    m_data->canvas = canvas;

    if ( canvas )
    {
        canvas->setParent( this );
        canvas->installEventFilter( this );

        if ( isVisible() )
            canvas->show();
    }
}

/*!
   \brief Adds handling of layout requests
   \param event Event

   \return See QFrame::event()
 */
bool QwtPlot::event( QEvent* event )
{
    bool ok = QFrame::event( event );
    switch ( event->type() )
    {
        case QEvent::LayoutRequest:
            updateLayout();
            break;
        case QEvent::PolishRequest:
            replot();
            break;
        default:;
    }
    return ok;
}

/*!
   \brief Event filter

   The plot handles the following events for the canvas:

   - QEvent::Resize
    The canvas margins might depend on its size

   - QEvent::ContentsRectChange
    The layout needs to be recalculated

   \param object Object to be filtered
   \param event Event

   \return See QFrame::eventFilter()

   \sa updateCanvasMargins(), updateLayout()
 */
bool QwtPlot::eventFilter( QObject* object, QEvent* event )
{
    if ( object == m_data->canvas )
    {
        if ( event->type() == QEvent::Resize )
        {
            updateCanvasMargins();
        }
        else if ( event->type() == QEvent::ContentsRectChange )
        {
            updateLayout();
        }
    }

    return QFrame::eventFilter( object, event );
}

//! Replots the plot if autoReplot() is \c true.
void QwtPlot::autoRefresh()
{
    if ( m_data->autoReplot )
        replot();
}

/*!
   \brief Set or reset the autoReplot option

   If the autoReplot option is set, the plot will be
   updated implicitly by manipulating member functions.
   Since this may be time-consuming, it is recommended
   to leave this option switched off and call replot()
   explicitly if necessary.

   The autoReplot option is set to false by default, which
   means that the user has to call replot() in order to make
   changes visible.
   \param tf \c true or \c false. Defaults to \c true.
   \sa replot()
 */
void QwtPlot::setAutoReplot( bool tf )
{
    m_data->autoReplot = tf;
}

/*!
   \return true if the autoReplot option is set.
   \sa setAutoReplot()
 */
bool QwtPlot::autoReplot() const
{
    return m_data->autoReplot;
}

/*!
   Change the plot's title
   \param title New title
 */
void QwtPlot::setTitle( const QString& title )
{
    if ( title != m_data->titleLabel->text().text() )
    {
        m_data->titleLabel->setText( title );
        updateLayout();
    }
}

/*!
   Change the plot's title
   \param title New title
 */
void QwtPlot::setTitle( const QwtText& title )
{
    if ( title != m_data->titleLabel->text() )
    {
        m_data->titleLabel->setText( title );
        updateLayout();
    }
}

//! \return Title of the plot
QwtText QwtPlot::title() const
{
    return m_data->titleLabel->text();
}

//! \return Title label widget.
QwtTextLabel* QwtPlot::titleLabel()
{
    return m_data->titleLabel;
}

//! \return Title label widget.
const QwtTextLabel* QwtPlot::titleLabel() const
{
    return m_data->titleLabel;
}

/*!
   Change the text the footer
   \param text New text of the footer
 */
void QwtPlot::setFooter( const QString& text )
{
    if ( text != m_data->footerLabel->text().text() )
    {
        m_data->footerLabel->setText( text );
        updateLayout();
    }
}

/*!
   Change the text the footer
   \param text New text of the footer
 */
void QwtPlot::setFooter( const QwtText& text )
{
    if ( text != m_data->footerLabel->text() )
    {
        m_data->footerLabel->setText( text );
        updateLayout();
    }
}

//! \return Text of the footer
QwtText QwtPlot::footer() const
{
    return m_data->footerLabel->text();
}

//! \return Footer label widget.
QwtTextLabel* QwtPlot::footerLabel()
{
    return m_data->footerLabel;
}

//! \return Footer label widget.
const QwtTextLabel* QwtPlot::footerLabel() const
{
    return m_data->footerLabel;
}

/*!
   \brief Assign a new plot layout

   \param layout Layout()
   \sa plotLayout()
 */
void QwtPlot::setPlotLayout( QwtPlotLayout* layout )
{
    if ( layout != m_data->layout )
    {
        delete m_data->layout;
        m_data->layout = layout;

        updateLayout();
    }
}

//! \return the plot's layout
QwtPlotLayout* QwtPlot::plotLayout()
{
    return m_data->layout;
}

//! \return the plot's layout
const QwtPlotLayout* QwtPlot::plotLayout() const
{
    return m_data->layout;
}

/*!
   \return the plot's legend
   \sa insertLegend()
 */
QwtAbstractLegend* QwtPlot::legend()
{
    return m_data->legend;
}

/*!
   \return the plot's legend
   \sa insertLegend()
 */
const QwtAbstractLegend* QwtPlot::legend() const
{
    return m_data->legend;
}


/*!
   \return the plot's canvas
 */
QWidget* QwtPlot::canvas()
{
    return m_data->canvas;
}

/*!
   \return the plot's canvas
 */
const QWidget* QwtPlot::canvas() const
{
    return m_data->canvas;
}

/*!
   \return Size hint for the plot widget
   \sa minimumSizeHint()
 */
QSize QwtPlot::sizeHint() const
{
    int dw = 0;
    int dh = 0;

    for ( int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++ )
    {
        {
            const QwtAxisId axisId( axisPos );

            if ( isAxisVisible( axisId ) )
            {
                const int niceDist = 40;
                const QwtScaleWidget* scaleWidget = axisWidget( axisId );
                const QwtScaleDiv& scaleDiv = scaleWidget->scaleDraw()->scaleDiv();
                const int majCnt = scaleDiv.ticks( QwtScaleDiv::MajorTick ).count();

                const QSize hint = scaleWidget->minimumSizeHint();

                if ( QwtAxis::isYAxis( axisPos ) )
                {
                    const int hDiff = ( majCnt - 1 ) * niceDist - hint.height();
                    dh = qMax( dh, hDiff );
                }
                else
                {
                    const int wDiff = ( majCnt - 1 ) * niceDist - hint.width();
                    dw = qMax( dw, wDiff );
                }
            }
        }
    }
    return minimumSizeHint() + QSize( dw, dh );
}

/*!
   \brief Return a minimum size hint
 */
QSize QwtPlot::minimumSizeHint() const
{
    QSize hint = m_data->layout->minimumSizeHint( this );
    hint += QSize( 2 * frameWidth(), 2 * frameWidth() );

    return hint;
}

/*!
   Resize and update internal layout
   \param e Resize event
 */
void QwtPlot::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );
    updateLayout();
}

/*!
   \brief Redraw the plot

   If the autoReplot option is not set (which is the default)
   or if any curves are attached to raw data, the plot has to
   be refreshed explicitly in order to make changes visible.

   \sa updateAxes(), setAutoReplot()
 */
void QwtPlot::replot()
{
    bool doAutoReplot = autoReplot();
    setAutoReplot( false );

    updateAxes();

    /*
       Maybe the layout needs to be updated, because of changed
       axes labels. We need to process them here before painting
       to avoid that scales and canvas get out of sync.
     */
    QApplication::sendPostedEvents( this, QEvent::LayoutRequest );

    if ( m_data->canvas )
    {
        const bool ok = QMetaObject::invokeMethod(
            m_data->canvas, "replot", Qt::DirectConnection );
        if ( !ok )
        {
            // fallback, when canvas has no a replot method
            m_data->canvas->update( m_data->canvas->contentsRect() );
        }
    }

    setAutoReplot( doAutoReplot );
}

/*!
   \brief Adjust plot content to its current size.
   \sa resizeEvent()
 */
void QwtPlot::updateLayout()
{
    QwtPlotLayout* layout = m_data->layout;
    layout->activate( this, contentsRect() );

    const QRect titleRect = layout->titleRect().toRect();
    const QRect footerRect = layout->footerRect().toRect();
    const QRect legendRect = layout->legendRect().toRect();
    const QRect canvasRect = layout->canvasRect().toRect();

    // resize and show the visible widgets

    if ( !m_data->titleLabel->text().isEmpty() )
    {
        m_data->titleLabel->setGeometry( titleRect );
        if ( !m_data->titleLabel->isVisibleTo( this ) )
            m_data->titleLabel->show();
    }
    else
        m_data->titleLabel->hide();

    if ( !m_data->footerLabel->text().isEmpty() )
    {
        m_data->footerLabel->setGeometry( footerRect );
        if ( !m_data->footerLabel->isVisibleTo( this ) )
            m_data->footerLabel->show();
    }
    else
    {
        m_data->footerLabel->hide();
    }

    for ( int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++ )
    {
        {
            const QwtAxisId axisId( axisPos );

            QwtScaleWidget* scaleWidget = axisWidget( axisId );

            if ( isAxisVisible( axisId ) )
            {
                const QRect scaleRect = layout->scaleRect( axisId ).toRect();

                if ( scaleRect != scaleWidget->geometry() )
                {
                    scaleWidget->setGeometry( scaleRect );

                    int startDist, endDist;
                    scaleWidget->getBorderDistHint( startDist, endDist );
                    scaleWidget->setBorderDist( startDist, endDist );
                }

                if ( !scaleWidget->isVisibleTo( this ) )
                    scaleWidget->show();
            }
            else
            {
                scaleWidget->hide();
            }
        }
    }

    if ( m_data->legend )
    {
        if ( m_data->legend->isEmpty() )
        {
            m_data->legend->hide();
        }
        else
        {
            m_data->legend->setGeometry( legendRect );
            m_data->legend->show();
        }
    }

    m_data->canvas->setGeometry( canvasRect );
}

/*!
   \brief Calculate the canvas margins

   \param maps QwtAxis::AxisCount maps, mapping between plot and paint device coordinates
   \param canvasRect Bounding rectangle where to paint
   \param left Return parameter for the left margin
   \param top Return parameter for the top margin
   \param right Return parameter for the right margin
   \param bottom Return parameter for the bottom margin

   Plot items might indicate, that they need some extra space
   at the borders of the canvas by the QwtPlotItem::Margins flag.

   updateCanvasMargins(), QwtPlotItem::getCanvasMarginHint()
 */
void QwtPlot::getCanvasMarginsHint(
    const QwtScaleMap maps[], const QRectF& canvasRect,
    double& left, double& top, double& right, double& bottom) const
{
    left = top = right = bottom = -1.0;

    const QwtPlotItemList& itmList = itemList();
    for ( QwtPlotItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        const QwtPlotItem* item = *it;
        if ( item->testItemAttribute( QwtPlotItem::Margins ) )
        {
            using namespace QwtAxis;

            double m[ AxisPositions ];
            item->getCanvasMarginHint(
                maps[ item->xAxis() ], maps[ item->yAxis() ],
                canvasRect, m[YLeft], m[XTop], m[YRight], m[XBottom] );

            left = qwtMaxF( left, m[YLeft] );
            top = qwtMaxF( top, m[XTop] );
            right = qwtMaxF( right, m[YRight] );
            bottom = qwtMaxF( bottom, m[XBottom] );
        }
    }
}

/*!
   \brief Update the canvas margins

   Plot items might indicate, that they need some extra space
   at the borders of the canvas by the QwtPlotItem::Margins flag.

   getCanvasMarginsHint(), QwtPlotItem::getCanvasMarginHint()
 */
void QwtPlot::updateCanvasMargins()
{
    using namespace QwtAxis;

    QwtScaleMap maps[ AxisPositions ];
    for ( int axisId = 0; axisId < AxisPositions; axisId++ )
        maps[axisId] = canvasMap( axisId );

    double margins[AxisPositions];
    getCanvasMarginsHint( maps, canvas()->contentsRect(),
        margins[YLeft], margins[XTop], margins[YRight], margins[XBottom] );

    bool doUpdate = false;
    for ( int axisPos = 0; axisPos < AxisPositions; axisPos++ )
    {
        if ( margins[axisPos] >= 0.0 )
        {
            const int m = qwtCeil( margins[axisPos] );
            plotLayout()->setCanvasMargin( m, axisPos);
            doUpdate = true;
        }
    }

    if ( doUpdate )
        updateLayout();
}

/*!
   Redraw the canvas.
   \param painter Painter used for drawing

   \warning drawCanvas calls drawItems what is also used
           for printing. Applications that like to add individual
           plot items better overload drawItems()
   \sa drawItems()
 */
void QwtPlot::drawCanvas( QPainter* painter )
{
    QwtScaleMap maps[ QwtAxis::AxisPositions ];
    for ( int axisPos = 0; axisPos < QwtAxis::AxisPositions; axisPos++ )
        maps[axisPos] = canvasMap( axisPos );

    drawItems( painter, m_data->canvas->contentsRect(), maps );
}

/*!
   Redraw the canvas items.

   \param painter Painter used for drawing
   \param canvasRect Bounding rectangle where to paint
   \param maps QwtAxis::AxisCount maps, mapping between plot and paint device coordinates

   \note Usually canvasRect is contentsRect() of the plot canvas.
        Due to a bug in Qt this rectangle might be wrong for certain
        frame styles ( f.e QFrame::Box ) and it might be necessary to
        fix the margins manually using QWidget::setContentsMargins()
 */

void QwtPlot::drawItems( QPainter* painter, const QRectF& canvasRect,
    const QwtScaleMap maps[ QwtAxis::AxisPositions ] ) const
{
    const QwtPlotItemList& itmList = itemList();
    for ( QwtPlotItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPlotItem* item = *it;
        if ( item && item->isVisible() )
        {
            const QwtAxisId xAxis = item->xAxis();
            const QwtAxisId yAxis = item->yAxis();

            painter->save();

            painter->setRenderHint( QPainter::Antialiasing,
                item->testRenderHint( QwtPlotItem::RenderAntialiased ) );

#if QT_VERSION < 0x050100
            painter->setRenderHint( QPainter::HighQualityAntialiasing,
                item->testRenderHint( QwtPlotItem::RenderAntialiased ) );
#endif

            item->draw( painter, maps[xAxis], maps[yAxis], canvasRect );

            painter->restore();
        }
    }
}

/*!
   \param axisId Axis
   \return Map for the axis on the canvas. With this map pixel coordinates can
          translated to plot coordinates and vice versa.
   \sa QwtScaleMap, transform(), invTransform()
 */
QwtScaleMap QwtPlot::canvasMap( QwtAxisId axisId ) const
{
    QwtScaleMap map;
    if ( !m_data->canvas )
        return map;

    map.setTransformation( axisScaleEngine( axisId )->transformation() );

    const QwtScaleDiv& sd = axisScaleDiv( axisId );
    map.setScaleInterval( sd.lowerBound(), sd.upperBound() );

    if ( isAxisVisible( axisId ) )
    {
        const QwtScaleWidget* s = axisWidget( axisId );
        if ( QwtAxis::isYAxis( axisId ) )
        {
            double y = s->y() + s->startBorderDist() - m_data->canvas->y();
            double h = s->height() - s->startBorderDist() - s->endBorderDist();
            map.setPaintInterval( y + h, y );
        }
        else
        {
            double x = s->x() + s->startBorderDist() - m_data->canvas->x();
            double w = s->width() - s->startBorderDist() - s->endBorderDist();
            map.setPaintInterval( x, x + w );
        }
    }
    else
    {
        using namespace QwtAxis;

        const QRect& canvasRect = m_data->canvas->contentsRect();
        if ( isYAxis( axisId ) )
        {
            int top = 0;
            if ( !plotLayout()->alignCanvasToScale( XTop ) )
                top = plotLayout()->canvasMargin( XTop );

            int bottom = 0;
            if ( !plotLayout()->alignCanvasToScale( XBottom ) )
                bottom = plotLayout()->canvasMargin( XBottom );

            map.setPaintInterval( canvasRect.bottom() - bottom,
                canvasRect.top() + top );
        }
        else
        {
            int left = 0;
            if ( !plotLayout()->alignCanvasToScale( YLeft ) )
                left = plotLayout()->canvasMargin( YLeft );

            int right = 0;
            if ( !plotLayout()->alignCanvasToScale( YRight ) )
                right = plotLayout()->canvasMargin( YRight );

            map.setPaintInterval( canvasRect.left() + left,
                canvasRect.right() - right );
        }
    }

    return map;
}

/*!
   \brief Change the background of the plotting area

   Sets brush to QPalette::Window of all color groups of
   the palette of the canvas. Using canvas()->setPalette()
   is a more powerful way to set these colors.

   \param brush New background brush
   \sa canvasBackground()
 */
void QwtPlot::setCanvasBackground( const QBrush& brush )
{
    QPalette pal = m_data->canvas->palette();
    pal.setBrush( QPalette::Window, brush );

    canvas()->setPalette( pal );
}

/*!
   Nothing else than: canvas()->palette().brush(
        QPalette::Normal, QPalette::Window);

   \return Background brush of the plotting area.
   \sa setCanvasBackground()
 */
QBrush QwtPlot::canvasBackground() const
{
    return canvas()->palette().brush(
        QPalette::Normal, QPalette::Window );
}

/*!
   \brief Insert a legend

   If the position legend is \c QwtPlot::LeftLegend or \c QwtPlot::RightLegend
   the legend will be organized in one column from top to down.
   Otherwise the legend items will be placed in a table
   with a best fit number of columns from left to right.

   insertLegend() will set the plot widget as parent for the legend.
   The legend will be deleted in the destructor of the plot or when
   another legend is inserted.

   Legends, that are not inserted into the layout of the plot widget
   need to connect to the legendDataChanged() signal. Calling updateLegend()
   initiates this signal for an initial update. When the application code
   wants to implement its own layout this also needs to be done for
   rendering plots to a document ( see QwtPlotRenderer ).

   \param legend Legend
   \param pos The legend's position. For top/left position the number
             of columns will be limited to 1, otherwise it will be set to
             unlimited.

   \param ratio Ratio between legend and the bounding rectangle
               of title, canvas and axes. The legend will be shrunk
               if it would need more space than the given ratio.
               The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0
               it will be reset to the default ratio.
               The default vertical/horizontal ratio is 0.33/0.5.

   \sa legend(), QwtPlotLayout::legendPosition(),
      QwtPlotLayout::setLegendPosition()
 */
void QwtPlot::insertLegend( QwtAbstractLegend* legend,
    QwtPlot::LegendPosition pos, double ratio )
{
    m_data->layout->setLegendPosition( pos, ratio );

    if ( legend != m_data->legend )
    {
        if ( m_data->legend && m_data->legend->parent() == this )
            delete m_data->legend;

        m_data->legend = legend;

        if ( m_data->legend )
        {
            connect(
                this, SIGNAL(legendDataChanged(QVariant,QList<QwtLegendData>)),
                m_data->legend, SLOT(updateLegend(QVariant,QList<QwtLegendData>))
                );

            if ( m_data->legend->parent() != this )
                m_data->legend->setParent( this );

            qwtEnableLegendItems( this, false );
            updateLegend();
            qwtEnableLegendItems( this, true );

            QwtLegend* lgd = qobject_cast< QwtLegend* >( legend );
            if ( lgd )
            {
                switch ( m_data->layout->legendPosition() )
                {
                    case LeftLegend:
                    case RightLegend:
                    {
                        if ( lgd->maxColumns() == 0     )
                            lgd->setMaxColumns( 1 ); // 1 column: align vertical
                        break;
                    }
                    case TopLegend:
                    case BottomLegend:
                    {
                        lgd->setMaxColumns( 0 ); // unlimited
                        break;
                    }
                    default:
                        break;
                }
            }

            QWidget* previousInChain = NULL;
            switch ( m_data->layout->legendPosition() )
            {
                case LeftLegend:
                {
                    const QwtAxisId axisId( QwtAxis::XTop );
                    previousInChain = axisWidget( axisId );
                    break;
                }
                case TopLegend:
                {
                    previousInChain = this;
                    break;
                }
                case RightLegend:
                {
                    const QwtAxisId axisId( QwtAxis::YRight );
                    previousInChain = axisWidget( axisId );
                    break;
                }
                case BottomLegend:
                {
                    previousInChain = footerLabel();
                    break;
                }
            }

            if ( previousInChain )
                qwtSetTabOrder( previousInChain, legend, true );
        }
    }

    updateLayout();
}

/*!
   Emit legendDataChanged() for all plot item

   \sa QwtPlotItem::legendData(), legendDataChanged()
 */
void QwtPlot::updateLegend()
{
    const QwtPlotItemList& itmList = itemList();
    for ( QwtPlotItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        updateLegend( *it );
    }
}

/*!
   Emit legendDataChanged() for a plot item

   \param plotItem Plot item
   \sa QwtPlotItem::legendData(), legendDataChanged()
 */
void QwtPlot::updateLegend( const QwtPlotItem* plotItem )
{
    if ( plotItem == NULL )
        return;

    QList< QwtLegendData > legendData;

    if ( plotItem->testItemAttribute( QwtPlotItem::Legend ) )
        legendData = plotItem->legendData();

    const QVariant itemInfo = itemToInfo( const_cast< QwtPlotItem* >( plotItem ) );
    Q_EMIT legendDataChanged( itemInfo, legendData );
}

/*!
   \brief Update all plot items interested in legend attributes

   Call QwtPlotItem::updateLegend(), when the QwtPlotItem::LegendInterest
   flag is set.

   \param itemInfo Info about the plot item
   \param legendData Entries to be displayed for the plot item ( usually 1 )

   \sa QwtPlotItem::LegendInterest,
      QwtPlotLegendItem, QwtPlotItem::updateLegend()
 */
void QwtPlot::updateLegendItems( const QVariant& itemInfo,
    const QList< QwtLegendData >& legendData )
{
    QwtPlotItem* plotItem = infoToItem( itemInfo );
    if ( plotItem )
    {
        const QwtPlotItemList& itmList = itemList();
        for ( QwtPlotItemIterator it = itmList.begin();
            it != itmList.end(); ++it )
        {
            QwtPlotItem* item = *it;
            if ( item->testItemInterest( QwtPlotItem::LegendInterest ) )
                item->updateLegend( plotItem, legendData );
        }
    }
}

/*!
   \brief Attach/Detach a plot item

   \param plotItem Plot item
   \param on When true attach the item, otherwise detach it
 */
void QwtPlot::attachItem( QwtPlotItem* plotItem, bool on )
{
    if ( plotItem->testItemInterest( QwtPlotItem::LegendInterest ) )
    {
        // plotItem is some sort of legend

        const QwtPlotItemList& itmList = itemList();
        for ( QwtPlotItemIterator it = itmList.begin();
            it != itmList.end(); ++it )
        {
            QwtPlotItem* item = *it;

            QList< QwtLegendData > legendData;
            if ( on && item->testItemAttribute( QwtPlotItem::Legend ) )
            {
                legendData = item->legendData();
                plotItem->updateLegend( item, legendData );
            }
        }
    }

    if ( on )
        insertItem( plotItem );
    else
        removeItem( plotItem );

    Q_EMIT itemAttached( plotItem, on );

    if ( plotItem->testItemAttribute( QwtPlotItem::Legend ) )
    {
        // the item wants to be represented on the legend

        if ( on )
        {
            updateLegend( plotItem );
        }
        else
        {
            const QVariant itemInfo = itemToInfo( plotItem );
            Q_EMIT legendDataChanged( itemInfo, QList< QwtLegendData >() );
        }
    }

    autoRefresh();
}

/*!
   \brief Build an information, that can be used to identify
         a plot item on the legend.

   The default implementation simply wraps the plot item
   into a QVariant object. When overloading itemToInfo()
   usually infoToItem() needs to reimplemeted too.

   \param plotItem Plot item
   \return Plot item embedded in a QVariant
   \sa infoToItem()
 */
QVariant QwtPlot::itemToInfo( QwtPlotItem* plotItem ) const
{
    return QVariant::fromValue( plotItem );
}

/*!
   \brief Identify the plot item according to an item info object,
         that has bee generated from itemToInfo().

   The default implementation simply tries to unwrap a QwtPlotItem
   pointer:

   \code
    if ( itemInfo.canConvert<QwtPlotItem *>() )
        return qvariant_cast<QwtPlotItem *>( itemInfo );
   \endcode
   \param itemInfo Plot item
   \return A plot item, when successful, otherwise a NULL pointer.
   \sa itemToInfo()
 */
QwtPlotItem* QwtPlot::infoToItem( const QVariant& itemInfo ) const
{
    if ( itemInfo.canConvert< QwtPlotItem* >() )
        return qvariant_cast< QwtPlotItem* >( itemInfo );

    return NULL;
}

#include "moc_qwt_plot.cpp"
