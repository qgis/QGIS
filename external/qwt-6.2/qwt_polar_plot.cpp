/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_polar_layout.h"
#include "qwt_painter.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_text_label.h"
#include "qwt_round_scale_draw.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"

#include <qpointer.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qevent.h>

static inline double qwtDistance(
    const QPointF& p1, const QPointF& p2 )
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return qSqrt( dx * dx + dy * dy );
}

namespace
{
    class ScaleData
    {
      public:
        ScaleData()
            : isValid( false )
            , scaleEngine( NULL )
        {
        }

        ~ScaleData()
        {
            delete scaleEngine;
        }

        bool doAutoScale;

        double minValue;
        double maxValue;
        double stepSize;

        int maxMajor;
        int maxMinor;

        bool isValid;

        QwtScaleDiv scaleDiv;
        QwtScaleEngine* scaleEngine;
    };
}

class QwtPolarPlot::PrivateData
{
  public:
    QBrush canvasBrush;

    bool autoReplot;

    QwtPointPolar zoomPos;
    double zoomFactor;

    ScaleData scaleData[QwtPolar::ScaleCount];
    QPointer< QwtTextLabel > titleLabel;
    QPointer< QwtPolarCanvas > canvas;
    QPointer< QwtAbstractLegend > legend;
    double azimuthOrigin;

    QwtPolarLayout* layout;
};

/*!
   Constructor
   \param parent Parent widget
 */
QwtPolarPlot::QwtPolarPlot( QWidget* parent )
    : QFrame( parent )
{
    initPlot( QwtText() );
}

/*!
   Constructor
   \param title Title text
   \param parent Parent widget
 */
QwtPolarPlot::QwtPolarPlot( const QwtText& title, QWidget* parent )
    : QFrame( parent )
{
    initPlot( title );
}

//! Destructor
QwtPolarPlot::~QwtPolarPlot()
{
    detachItems( QwtPolarItem::Rtti_PolarItem, autoDelete() );

    delete m_data->layout;
    delete m_data;
}

/*!
   Change the plot's title
   \param title New title
 */
void QwtPolarPlot::setTitle( const QString& title )
{
    if ( title != m_data->titleLabel->text().text() )
    {
        m_data->titleLabel->setText( title );
        if ( !title.isEmpty() )
            m_data->titleLabel->show();
        else
            m_data->titleLabel->hide();
    }
}

/*!
   Change the plot's title
   \param title New title
 */
void QwtPolarPlot::setTitle( const QwtText& title )
{
    if ( title != m_data->titleLabel->text() )
    {
        m_data->titleLabel->setText( title );
        if ( !title.isEmpty() )
            m_data->titleLabel->show();
        else
            m_data->titleLabel->hide();
    }
}

//! \return the plot's title
QwtText QwtPolarPlot::title() const
{
    return m_data->titleLabel->text();
}

//! \return the plot's title
QwtTextLabel* QwtPolarPlot::titleLabel()
{
    return m_data->titleLabel;
}

//! \return the plot's title label.
const QwtTextLabel* QwtPolarPlot::titleLabel() const
{
    return m_data->titleLabel;
}

/*!
   \brief Insert a legend

   If the position legend is \c QwtPolarPlot::LeftLegend or \c QwtPolarPlot::RightLegend
   the legend will be organized in one column from top to down.
   Otherwise the legend items will be placed in a table
   with a best fit number of columns from left to right.

   If pos != QwtPolarPlot::ExternalLegend the plot widget will become
   parent of the legend. It will be deleted when the plot is deleted,
   or another legend is set with insertLegend().

   \param legend Legend
   \param pos The legend's position. For top/left position the number
             of columns will be limited to 1, otherwise it will be set to
             unlimited.

   \param ratio Ratio between legend and the bounding rect
               of title, canvas and axes. The legend will be shrunk
               if it would need more space than the given ratio.
               The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0
               it will be reset to the default ratio.
               The default vertical/horizontal ratio is 0.33/0.5.

   \sa legend(), QwtPolarLayout::legendPosition(),
      QwtPolarLayout::setLegendPosition()
 */
void QwtPolarPlot::insertLegend( QwtAbstractLegend* legend,
    QwtPolarPlot::LegendPosition pos, double ratio )
{
    m_data->layout->setLegendPosition( pos, ratio );

    if ( legend != m_data->legend )
    {
        if ( m_data->legend && m_data->legend->parent() == this )
            delete m_data->legend;

        m_data->legend = legend;

        if ( m_data->legend )
        {
            connect( this,
                SIGNAL(legendDataChanged(
                    const QVariant&,const QList<QwtLegendData>&)),
                m_data->legend,
                SLOT(updateLegend(
                    const QVariant&,const QList<QwtLegendData>&))
                );

            if ( m_data->legend->parent() != this )
                m_data->legend->setParent( this );

            updateLegend();

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

        }
    }

    updateLayout();
}

/*!
   Emit legendDataChanged() for all plot item

   \sa QwtPlotItem::legendData(), legendDataChanged()
 */
void QwtPolarPlot::updateLegend()
{
    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
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
void QwtPolarPlot::updateLegend( const QwtPolarItem* plotItem )
{
    if ( plotItem == NULL )
        return;

    QList< QwtLegendData > legendData;

    if ( plotItem->testItemAttribute( QwtPolarItem::Legend ) )
        legendData = plotItem->legendData();

    const QVariant itemInfo = itemToInfo( const_cast< QwtPolarItem* >( plotItem ) );
    Q_EMIT legendDataChanged( itemInfo, legendData );
}

/*!
   \return the plot's legend
   \sa insertLegend()
 */
QwtAbstractLegend* QwtPolarPlot::legend()
{
    return m_data->legend;
}

/*!
   \return the plot's legend
   \sa insertLegend()
 */
const QwtAbstractLegend* QwtPolarPlot::legend() const
{
    return m_data->legend;
}

/*!
   \brief Set the background of the plot area

   The plot area is the circle around the pole. It's radius
   is defined by the radial scale.

   \param brush Background Brush
   \sa plotBackground(), plotArea()
 */
void QwtPolarPlot::setPlotBackground( const QBrush& brush )
{
    if ( brush != m_data->canvasBrush )
    {
        m_data->canvasBrush = brush;
        autoRefresh();
    }
}

/*!
   \return plot background brush
   \sa plotBackground(), plotArea()
 */
const QBrush& QwtPolarPlot::plotBackground() const
{
    return m_data->canvasBrush;
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
   \param enable \c true or \c false. Defaults to \c true.
   \sa replot()
 */
void QwtPolarPlot::setAutoReplot( bool enable )
{
    m_data->autoReplot = enable;
}

//! \return true if the autoReplot option is set.
bool QwtPolarPlot::autoReplot() const
{
    return m_data->autoReplot;
}

/*!
   \brief Enable autoscaling

   This member function is used to switch back to autoscaling mode
   after a fixed scale has been set. Autoscaling calculates a useful
   scale division from the bounding interval of all plot items with
   the QwtPolarItem::AutoScale attribute.

   Autoscaling is only supported for the radial scale and enabled as default.

   \param scaleId Scale index

   \sa hasAutoScale(), setScale(), setScaleDiv(),
      QwtPolarItem::boundingInterval()
 */
void QwtPolarPlot::setAutoScale( int scaleId )
{
    if ( scaleId != QwtPolar::ScaleRadius )
        return;

    ScaleData& scaleData = m_data->scaleData[scaleId];
    if ( !scaleData.doAutoScale )
    {
        scaleData.doAutoScale = true;
        autoRefresh();
    }
}

/*!
   \return \c true if autoscaling is enabled
   \param scaleId Scale index
   \sa setAutoScale()
 */
bool QwtPolarPlot::hasAutoScale( int scaleId ) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return false;

    return m_data->scaleData[scaleId].doAutoScale;
}

/*!
   Set the maximum number of major scale intervals for a specified scale

   \param scaleId Scale index
   \param maxMinor maximum number of minor steps
   \sa scaleMaxMajor()
 */
void QwtPolarPlot::setScaleMaxMinor( int scaleId, int maxMinor )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    maxMinor = qBound( 0, maxMinor, 100 );

    ScaleData& scaleData = m_data->scaleData[scaleId];

    if ( maxMinor != scaleData.maxMinor )
    {
        scaleData.maxMinor = maxMinor;
        scaleData.isValid = false;
        autoRefresh();
    }
}

/*!
   \return the maximum number of minor ticks for a specified axis
   \param scaleId Scale index
   \sa setScaleMaxMinor()
 */
int QwtPolarPlot::scaleMaxMinor( int scaleId ) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return 0;

    return m_data->scaleData[scaleId].maxMinor;
}

/*!
   Set the maximum number of major scale intervals for a specified scale

   \param scaleId Scale index
   \param maxMajor maximum number of major steps
   \sa scaleMaxMajor()
 */
void QwtPolarPlot::setScaleMaxMajor( int scaleId, int maxMajor )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    maxMajor = qBound( 1, maxMajor, 10000 );

    ScaleData& scaleData = m_data->scaleData[scaleId];
    if ( maxMajor != scaleData.maxMinor )
    {
        scaleData.maxMajor = maxMajor;
        scaleData.isValid = false;
        autoRefresh();
    }
}

/*!
   \return the maximum number of major ticks for a specified axis
   \param scaleId Scale index

   \sa setScaleMaxMajor()
 */
int QwtPolarPlot::scaleMaxMajor( int scaleId ) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return 0;

    return m_data->scaleData[scaleId].maxMajor;
}

/*!
   Change the scale engine for an axis

   \param scaleId Scale index
   \param scaleEngine Scale engine

   \sa axisScaleEngine()
 */
void QwtPolarPlot::setScaleEngine( int scaleId, QwtScaleEngine* scaleEngine )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData& scaleData = m_data->scaleData[scaleId];
    if ( scaleEngine == NULL || scaleEngine == scaleData.scaleEngine )
        return;

    delete scaleData.scaleEngine;
    scaleData.scaleEngine = scaleEngine;

    scaleData.isValid = false;

    autoRefresh();
}

/*!
   \return Scale engine for a specific scale

   \param scaleId Scale index
   \sa setScaleEngine()
 */
QwtScaleEngine* QwtPolarPlot::scaleEngine( int scaleId )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return m_data->scaleData[scaleId].scaleEngine;
}

/*!
   \return Scale engine for a specific scale

   \param scaleId Scale index
   \sa setScaleEngine()
 */
const QwtScaleEngine* QwtPolarPlot::scaleEngine( int scaleId ) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return m_data->scaleData[scaleId].scaleEngine;
}

/*!
   \brief Disable autoscaling and specify a fixed scale for a selected scale.
   \param scaleId Scale index
   \param min
   \param max minimum and maximum of the scale
   \param stepSize Major step size. If <code>step == 0</code>, the step size is
            calculated automatically using the maxMajor setting.
   \sa setScaleMaxMajor(), setAutoScale()
 */
void QwtPolarPlot::setScale( int scaleId,
    double min, double max, double stepSize )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData& scaleData = m_data->scaleData[scaleId];

    scaleData.isValid = false;

    scaleData.minValue = min;
    scaleData.maxValue = max;
    scaleData.stepSize = stepSize;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/*!
   \brief Disable autoscaling and specify a fixed scale for a selected scale.
   \param scaleId Scale index
   \param scaleDiv Scale division
   \sa setScale(), setAutoScale()
 */
void QwtPolarPlot::setScaleDiv( int scaleId, const QwtScaleDiv& scaleDiv )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData& scaleData = m_data->scaleData[scaleId];

    scaleData.scaleDiv = scaleDiv;
    scaleData.isValid = true;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/*!
   \brief Return the scale division of a specified scale

   scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound()
   are the current limits of the scale.

   \param scaleId Scale index
   \return Scale division

   \sa QwtScaleDiv, setScaleDiv(), setScale()
 */
const QwtScaleDiv* QwtPolarPlot::scaleDiv( int scaleId ) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return &m_data->scaleData[scaleId].scaleDiv;
}

/*!
   \brief Return the scale division of a specified scale

   scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound()
   are the current limits of the scale.

   \param scaleId Scale index
   \return Scale division

   \sa QwtScaleDiv, setScaleDiv(), setScale()
 */
QwtScaleDiv* QwtPolarPlot::scaleDiv( int scaleId )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return &m_data->scaleData[scaleId].scaleDiv;
}

/*!
   \brief Change the origin of the azimuth scale

   The azimuth origin is the angle where the azimuth scale
   shows the value 0.0.  The default origin is 0.0.

   \param origin New origin
   \sa azimuthOrigin()
 */
void QwtPolarPlot::setAzimuthOrigin( double origin )
{
    origin = ::fmod( origin, 2 * M_PI );
    if ( origin != m_data->azimuthOrigin )
    {
        m_data->azimuthOrigin = origin;
        autoRefresh();
    }
}

/*!
   The azimuth origin is the angle where the azimuth scale
   shows the value 0.0.

   \return Origin of the azimuth scale
   \sa setAzimuthOrigin()
 */
double QwtPolarPlot::azimuthOrigin() const
{
    return m_data->azimuthOrigin;
}

/*!
   \brief Translate and in/decrease the zoom factor

   In zoom mode the zoom position is in the center of the
   canvas. The radius of the circle depends on the size of the plot canvas,
   that is divided by the zoom factor. Thus a factor < 1.0 zoom in.

   Setting an invalid zoom position disables zooming.

   \param zoomPos Center of the translation
   \param zoomFactor Zoom factor

   \sa unzoom(), zoomPos(), zoomFactor()
 */
void QwtPolarPlot::zoom( const QwtPointPolar& zoomPos, double zoomFactor )
{
    zoomFactor = qAbs( zoomFactor );
    if ( zoomPos != m_data->zoomPos ||
        zoomFactor != m_data->zoomFactor )
    {
        m_data->zoomPos = zoomPos;
        m_data->zoomFactor = zoomFactor;
        updateLayout();
        autoRefresh();
    }
}

/*!
   Unzoom the plot
   \sa zoom()
 */
void QwtPolarPlot::unzoom()
{
    if ( m_data->zoomFactor != 1.0 || m_data->zoomPos.isValid() )
    {
        m_data->zoomFactor = 1.0;
        m_data->zoomPos = QwtPointPolar();
        autoRefresh();
    }
}

/*!
   \return Zoom position
   \sa zoom(), zoomFactor()
 */
QwtPointPolar QwtPolarPlot::zoomPos() const
{
    return m_data->zoomPos;
}

/*!
   \return Zoom factor
   \sa zoom(), zoomPos()
 */
double QwtPolarPlot::zoomFactor() const
{
    return m_data->zoomFactor;
}

/*!
   Build a scale map

   The azimuth map translates between the scale values and angles from
   [0.0, 2 * PI[. The radial map translates scale values into the distance
   from the pole. The radial map is calculated from the current geometry
   of the canvas.

   \param scaleId Scale index
   \return Map for the scale on the canvas. With this map pixel coordinates can
          translated to plot coordinates and vice versa.

   \sa QwtScaleMap, transform(), invTransform()
 */
QwtScaleMap QwtPolarPlot::scaleMap( int scaleId ) const
{
    const QRectF pr = plotRect();
    return scaleMap( scaleId, pr.width() / 2.0 );
}

/*!
   Build a scale map

   The azimuth map translates between the scale values and angles from
   [0.0, 2 * PI[. The radial map translates scale values into the distance
   from the pole.

   \param scaleId Scale index
   \param radius Radius of the plot are in pixels
   \return Map for the scale on the canvas. With this map pixel coordinates can
          translated to plot coordinates and vice versa.

   \sa QwtScaleMap, transform(), invTransform()
 */
QwtScaleMap QwtPolarPlot::scaleMap( int scaleId, const double radius ) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return QwtScaleMap();

    QwtScaleMap map;
    map.setTransformation( scaleEngine( scaleId )->transformation() );

    const QwtScaleDiv* sd = scaleDiv( scaleId );
    map.setScaleInterval( sd->lowerBound(), sd->upperBound() );

    if ( scaleId == QwtPolar::Azimuth )
    {
        map.setPaintInterval( m_data->azimuthOrigin,
            m_data->azimuthOrigin + 2 * M_PI );
    }
    else
    {
        map.setPaintInterval( 0.0, radius );
    }

    return map;
}

/*!
    \brief Qt event handler

    Handles QEvent::LayoutRequest and QEvent::PolishRequest

    \param e Qt Event
    \return True, when the event was processed
 */
bool QwtPolarPlot::event( QEvent* e )
{
    bool ok = QWidget::event( e );
    switch( e->type() )
    {
        case QEvent::LayoutRequest:
        {
            updateLayout();
            break;
        }
        case QEvent::PolishRequest:
        {
            updateLayout();
            replot();
            break;
        }
        default:;
    }
    return ok;
}

//! Resize and update internal layout
void QwtPolarPlot::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );
    updateLayout();
}

void QwtPolarPlot::initPlot( const QwtText& title )
{
    m_data = new PrivateData;
    m_data->layout = new QwtPolarLayout;

    QwtText text( title );
    text.setRenderFlags( Qt::AlignCenter | Qt::TextWordWrap );

    m_data->titleLabel = new QwtTextLabel( text, this );
    m_data->titleLabel->setFont( QFont( fontInfo().family(), 14, QFont::Bold ) );
    if ( !text.isEmpty() )
        m_data->titleLabel->show();
    else
        m_data->titleLabel->hide();

    m_data->canvas = new QwtPolarCanvas( this );

    m_data->autoReplot = false;
    m_data->canvasBrush = QBrush( Qt::white );

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        ScaleData& scaleData = m_data->scaleData[scaleId];

        if ( scaleId == QwtPolar::Azimuth )
        {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 360.0;
            scaleData.stepSize = 30.0;
        }
        else
        {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 1000.0;
            scaleData.stepSize = 0.0;
        }

        scaleData.doAutoScale = true;

        scaleData.maxMinor = 5;
        scaleData.maxMajor = 8;

        scaleData.isValid = false;

        scaleData.scaleEngine = new QwtLinearScaleEngine;
    }
    m_data->zoomFactor = 1.0;
    m_data->azimuthOrigin = 0.0;

    setSizePolicy( QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding );

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
        updateScale( scaleId );
}

//! Replots the plot if QwtPlot::autoReplot() is \c true.
void QwtPolarPlot::autoRefresh()
{
    if ( m_data->autoReplot )
        replot();
}

//! Rebuild the layout
void QwtPolarPlot::updateLayout()
{
    m_data->layout->activate( this, contentsRect() );

    // resize and show the visible widgets
    if ( m_data->titleLabel )
    {
        if ( !m_data->titleLabel->text().isEmpty() )
        {
            m_data->titleLabel->setGeometry( m_data->layout->titleRect().toRect() );
            if ( !m_data->titleLabel->isVisible() )
                m_data->titleLabel->show();
        }
        else
            m_data->titleLabel->hide();
    }

    if ( m_data->legend )
    {
        if ( m_data->legend->isEmpty() )
        {
            m_data->legend->hide();
        }
        else
        {
            const QRectF legendRect = m_data->layout->legendRect();
            m_data->legend->setGeometry( legendRect.toRect() );
            m_data->legend->show();
        }
    }

    m_data->canvas->setGeometry( m_data->layout->canvasRect().toRect() );
    Q_EMIT layoutChanged();
}

/*!
   \brief Redraw the plot

   If the autoReplot option is not set (which is the default)
   or if any curves are attached to raw data, the plot has to
   be refreshed explicitly in order to make changes visible.

   \sa setAutoReplot()
   \warning Calls canvas()->repaint, take care of infinite recursions
 */
void QwtPolarPlot::replot()
{
    bool doAutoReplot = autoReplot();
    setAutoReplot( false );

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
        updateScale( scaleId );

    m_data->canvas->invalidateBackingStore();
    m_data->canvas->repaint();

    setAutoReplot( doAutoReplot );
}

//!  \return the plot's canvas
QwtPolarCanvas* QwtPolarPlot::canvas()
{
    return m_data->canvas;
}

//!  \return the plot's canvas
const QwtPolarCanvas* QwtPolarPlot::canvas() const
{
    return m_data->canvas;
}

/*!
   Redraw the canvas.
   \param painter Painter used for drawing
   \param canvasRect Contents rect of the canvas
 */
void QwtPolarPlot::drawCanvas( QPainter* painter,
    const QRectF& canvasRect ) const
{
    const QRectF cr = canvasRect;
    const QRectF pr = plotRect( cr );

    const double radius = pr.width() / 2.0;

    if ( m_data->canvasBrush.style() != Qt::NoBrush )
    {
        painter->save();
        painter->setPen( Qt::NoPen );
        painter->setBrush( m_data->canvasBrush );

        if ( qwtDistance( pr.center(), cr.topLeft() ) < radius &&
            qwtDistance( pr.center(), cr.topRight() ) < radius &&
            qwtDistance( pr.center(), cr.bottomRight() ) < radius &&
            qwtDistance( pr.center(), cr.bottomLeft() ) < radius )
        {
            QwtPainter::drawRect( painter, cr );
        }
        else
        {
            painter->setRenderHint( QPainter::Antialiasing, true );
            QwtPainter::drawEllipse( painter, pr );
        }
        painter->restore();
    }

    drawItems( painter,
        scaleMap( QwtPolar::Azimuth, radius ),
        scaleMap( QwtPolar::Radius, radius ),
        pr.center(), radius, canvasRect );
}

/*!
   Redraw the canvas items.

   \param painter Painter used for drawing
   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param radius Radius of the complete plot area in painter coordinates
   \param canvasRect Contents rect of the canvas in painter coordinates
 */
void QwtPolarPlot::drawItems( QPainter* painter,
    const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
    const QPointF& pole, double radius,
    const QRectF& canvasRect ) const
{
    const QRectF pr = plotRect( canvasRect );

    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPolarItem* item = *it;
        if ( item && item->isVisible() )
        {
            painter->save();

            // Unfortunately circular clipping slows down
            // painting a lot. So we better try to avoid it.

            bool doClipping = false;
            if ( item->rtti() != QwtPolarItem::Rtti_PolarGrid )
            {
                const QwtInterval intv =
                    item->boundingInterval( QwtPolar::Radius );

                if ( !intv.isValid() )
                    doClipping = true;
                else
                {
                    if ( radialMap.s1() < radialMap.s2() )
                        doClipping = intv.maxValue() > radialMap.s2();
                    else
                        doClipping = intv.minValue() < radialMap.s2();
                }
            }

            if ( doClipping )
            {
                const int margin = item->marginHint();

                const QRectF clipRect = pr.adjusted(
                    -margin, -margin, margin, margin );
                if ( !clipRect.contains( canvasRect ) )
                {
                    QRegion clipRegion( clipRect.toRect(), QRegion::Ellipse );
                    painter->setClipRegion( clipRegion, Qt::IntersectClip );
                }
            }

            painter->setRenderHint( QPainter::Antialiasing,
                item->testRenderHint( QwtPolarItem::RenderAntialiased ) );

            item->draw( painter, azimuthMap, radialMap,
                pole, radius, canvasRect );

            painter->restore();
        }
    }
}

/*!
   Rebuild the scale
   \param scaleId Scale index
 */

void QwtPolarPlot::updateScale( int scaleId )
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData& d = m_data->scaleData[scaleId];

    double minValue = d.minValue;
    double maxValue = d.maxValue;
    double stepSize = d.stepSize;

    if ( scaleId == QwtPolar::ScaleRadius && d.doAutoScale )
    {
        QwtInterval interval;

        const QwtPolarItemList& itmList = itemList();
        for ( QwtPolarItemIterator it = itmList.begin();
            it != itmList.end(); ++it )
        {
            const QwtPolarItem* item = *it;
            if ( item->testItemAttribute( QwtPolarItem::AutoScale ) )
                interval |= item->boundingInterval( scaleId );
        }

        minValue = interval.minValue();
        maxValue = interval.maxValue();

        d.scaleEngine->autoScale( d.maxMajor,
            minValue, maxValue, stepSize );
        d.isValid = false;
    }

    if ( !d.isValid )
    {
        d.scaleDiv = d.scaleEngine->divideScale(
            minValue, maxValue, d.maxMajor, d.maxMinor, stepSize );
        d.isValid = true;
    }

    const QwtInterval interval = visibleInterval();

    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPolarItem* item = *it;
        item->updateScaleDiv( *scaleDiv( QwtPolar::Azimuth ),
            *scaleDiv( QwtPolar::Radius ), interval );
    }
}

/*!
   \return Maximum of all item margin hints.
   \sa QwtPolarItem::marginHint()
 */
int QwtPolarPlot::plotMarginHint() const
{
    int margin = 0;
    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPolarItem* item = *it;
        if ( item && item->isVisible() )
        {
            const int hint = item->marginHint();
            if ( hint > margin )
                margin = hint;
        }
    }
    return margin;
}

/*!
   The plot area depends on the size of the canvas
   and the zoom parameters.

   \return Bounding rect of the plot area

 */
QRectF QwtPolarPlot::plotRect() const
{
    return plotRect( canvas()->contentsRect() );
}

/*!
   \brief Calculate the bounding rect of the plot area

   The plot area depends on the zoom parameters.

   \param canvasRect Rectangle of the canvas
   \return Rectangle for displaying 100% of the plot
 */
QRectF QwtPolarPlot::plotRect( const QRectF& canvasRect ) const
{
    const QwtScaleDiv* sd = scaleDiv( QwtPolar::Radius );
    const QwtScaleEngine* se = scaleEngine( QwtPolar::Radius );

    const int margin = plotMarginHint();
    const QRectF cr = canvasRect;
    const int radius = qMin( cr.width(), cr.height() ) / 2 - margin;

    QwtScaleMap map;
    map.setTransformation( se->transformation() );
    map.setPaintInterval( 0.0, radius / m_data->zoomFactor );
    map.setScaleInterval( sd->lowerBound(), sd->upperBound() );

    double v = map.s1();
    if ( map.s1() <= map.s2() )
        v += m_data->zoomPos.radius();
    else
        v -= m_data->zoomPos.radius();
    v = map.transform( v );

    const QPointF off =
        QwtPointPolar( m_data->zoomPos.azimuth(), v ).toPoint();

    QPointF center( cr.center().x(), cr.top() + margin + radius );
    center -= QPointF( off.x(), -off.y() );

    QRectF rect( 0, 0, 2 * map.p2(), 2 * map.p2() );
    rect.moveCenter( center );

    return rect;
}

/*!
   \return Bounding interval of the radial scale that is
           visible on the canvas.
 */
QwtInterval QwtPolarPlot::visibleInterval() const
{
    const QwtScaleDiv* sd = scaleDiv( QwtPolar::Radius );

    const QRectF cRect = canvas()->contentsRect();
    const QRectF pRect = plotRect( cRect );
    if ( cRect.contains( pRect ) || !cRect.intersects( pRect ) )
    {
        return QwtInterval( sd->lowerBound(), sd->upperBound() );
    }

    const QPointF pole = pRect.center();
    const QRectF scaleRect = pRect & cRect;

    const QwtScaleMap map = scaleMap( QwtPolar::Radius );

    double dmin = 0.0;
    double dmax = 0.0;
    if ( scaleRect.contains( pole ) )
    {
        dmin = 0.0;

        QPointF corners[4];
        corners[0] = scaleRect.bottomRight();
        corners[1] = scaleRect.topRight();
        corners[2] = scaleRect.topLeft();
        corners[3] = scaleRect.bottomLeft();

        dmax = 0.0;
        for ( int i = 0; i < 4; i++ )
        {
            const double dist = qwtDistance( pole, corners[i] );
            if ( dist > dmax )
                dmax = dist;
        }
    }
    else
    {
        if ( pole.x() < scaleRect.left() )
        {
            if ( pole.y() < scaleRect.top() )
            {
                dmin = qwtDistance( pole, scaleRect.topLeft() );
                dmax = qwtDistance( pole, scaleRect.bottomRight() );
            }
            else if ( pole.y() > scaleRect.bottom() )
            {
                dmin = qwtDistance( pole, scaleRect.bottomLeft() );
                dmax = qwtDistance( pole, scaleRect.topRight() );
            }
            else
            {
                dmin = scaleRect.left() - pole.x();
                dmax = qMax( qwtDistance( pole, scaleRect.bottomRight() ),
                    qwtDistance( pole, scaleRect.topRight() ) );
            }
        }
        else if ( pole.x() > scaleRect.right() )
        {
            if ( pole.y() < scaleRect.top() )
            {
                dmin = qwtDistance( pole, scaleRect.topRight() );
                dmax = qwtDistance( pole, scaleRect.bottomLeft() );
            }
            else if ( pole.y() > scaleRect.bottom() )
            {
                dmin = qwtDistance( pole, scaleRect.bottomRight() );
                dmax = qwtDistance( pole, scaleRect.topLeft() );
            }
            else
            {
                dmin = pole.x() - scaleRect.right();
                dmax = qMax( qwtDistance( pole, scaleRect.bottomLeft() ),
                    qwtDistance( pole, scaleRect.topLeft() ) );
            }
        }
        else if ( pole.y() < scaleRect.top() )
        {
            dmin = scaleRect.top() - pole.y();
            dmax = qMax( qwtDistance( pole, scaleRect.bottomLeft() ),
                qwtDistance( pole, scaleRect.bottomRight() ) );
        }
        else if ( pole.y() > scaleRect.bottom() )
        {
            dmin = pole.y() - scaleRect.bottom();
            dmax = qMax( qwtDistance( pole, scaleRect.topLeft() ),
                qwtDistance( pole, scaleRect.topRight() ) );
        }
    }

    const double radius = pRect.width() / 2.0;
    if ( dmax > radius )
        dmax = radius;

    QwtInterval interval;
    interval.setMinValue( map.invTransform( dmin ) );
    interval.setMaxValue( map.invTransform( dmax ) );

    return interval;
}

/*!
   \return Layout, responsible for the geometry of the plot components
 */
QwtPolarLayout* QwtPolarPlot::plotLayout()
{
    return m_data->layout;
}

/*!
   \return Layout, responsible for the geometry of the plot components
 */
const QwtPolarLayout* QwtPolarPlot::plotLayout() const
{
    return m_data->layout;
}

/*!
   \brief Attach/Detach a plot item

   \param plotItem Plot item
   \param on When true attach the item, otherwise detach it
 */
void QwtPolarPlot::attachItem( QwtPolarItem* plotItem, bool on )
{
    if ( on )
        insertItem( plotItem );
    else
        removeItem( plotItem );

    Q_EMIT itemAttached( plotItem, on );

    if ( plotItem->testItemAttribute( QwtPolarItem::Legend ) )
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

    if ( autoReplot() )
        update();
}

/*!
   \brief Build an information, that can be used to identify
         a plot item on the legend.

   The default implementation simply wraps the plot item
   into a QVariant object. When overloading itemToInfo()
   usually infoToItem() needs to reimplemeted too.

   \code
    QVariant itemInfo;
    qVariantSetValue( itemInfo, plotItem );
   \endcode

   \param plotItem Plot item
   \sa infoToItem()
 */
QVariant QwtPolarPlot::itemToInfo( QwtPolarItem* plotItem ) const
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
QwtPolarItem* QwtPolarPlot::infoToItem( const QVariant& itemInfo ) const
{
    if ( itemInfo.canConvert< QwtPolarItem* >() )
        return qvariant_cast< QwtPolarItem* >( itemInfo );

    return NULL;
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_polar_plot.cpp"
#endif
