/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_item.h"
#include "qwt_text.h"
#include "qwt_plot.h"
#include "qwt_legend_data.h"
#include "qwt_scale_map.h"
#include "qwt_graphic.h"

#include <qpainter.h>

class QwtPlotItem::PrivateData
{
  public:
    PrivateData()
        : plot( NULL )
        , isVisible( true )
        , renderThreadCount( 1 )
        , z( 0.0 )
        , xAxisId( QwtAxis::XBottom )
        , yAxisId( QwtAxis::YLeft )
        , legendIconSize( 8, 8 )
    {
    }

    mutable QwtPlot* plot;

    bool isVisible;

    QwtPlotItem::ItemAttributes attributes;
    QwtPlotItem::ItemInterests interests;

    QwtPlotItem::RenderHints renderHints;
    uint renderThreadCount;

    double z;

    QwtAxisId xAxisId;
    QwtAxisId yAxisId;

    QwtText title;
    QSize legendIconSize;
};

/*!
   Constructor
 */
QwtPlotItem::QwtPlotItem()
{
    m_data = new PrivateData;
}

/*!
   Constructor
   \param title Title of the item
 */
QwtPlotItem::QwtPlotItem( const QString& title )
{
    m_data = new PrivateData;
    m_data->title = title;
}

/*!
   Constructor
   \param title Title of the item
 */
QwtPlotItem::QwtPlotItem( const QwtText& title )
{
    m_data = new PrivateData;
    m_data->title = title;
}

//! Destroy the QwtPlotItem
QwtPlotItem::~QwtPlotItem()
{
    attach( NULL );
    delete m_data;
}

/*!
   \brief Attach the item to a plot.

   This method will attach a QwtPlotItem to the QwtPlot argument. It will first
   detach the QwtPlotItem from any plot from a previous call to attach (if
   necessary). If a NULL argument is passed, it will detach from any QwtPlot it
   was attached to.

   \param plot Plot widget
   \sa detach()
 */
void QwtPlotItem::attach( QwtPlot* plot )
{
    if ( plot == m_data->plot )
        return;

    if ( m_data->plot )
        m_data->plot->attachItem( this, false );

    m_data->plot = plot;

    if ( m_data->plot )
        m_data->plot->attachItem( this, true );
}

/*!
   \brief This method detaches a QwtPlotItem from any
          QwtPlot it has been associated with.

   detach() is equivalent to calling attach( NULL )
   \sa attach()
 */
void QwtPlotItem::detach()
{
    attach( NULL );
}

/*!
   Return rtti for the specific class represented. QwtPlotItem is simply
   a virtual interface class, and base classes will implement this method
   with specific rtti values so a user can differentiate them.

   The rtti value is useful for environments, where the
   runtime type information is disabled and it is not possible
   to do a dynamic_cast<...>.

   \return rtti value
   \sa RttiValues
 */
int QwtPlotItem::rtti() const
{
    return Rtti_PlotItem;
}

//! Return attached plot
QwtPlot* QwtPlotItem::plot() const
{
    return m_data->plot;
}

/*!
   Plot items are painted in increasing z-order.

   \return setZ(), QwtPlotDict::itemList()
 */
double QwtPlotItem::z() const
{
    return m_data->z;
}

/*!
   \brief Set the z value

   Plot items are painted in increasing z-order.

   \param z Z-value
   \sa z(), QwtPlotDict::itemList()
 */
void QwtPlotItem::setZ( double z )
{
    if ( m_data->z != z )
    {
        if ( m_data->plot ) // update the z order
            m_data->plot->attachItem( this, false );

        m_data->z = z;

        if ( m_data->plot )
            m_data->plot->attachItem( this, true );

        itemChanged();
    }
}

/*!
   Set a new title

   \param title Title
   \sa title()
 */
void QwtPlotItem::setTitle( const QString& title )
{
    setTitle( QwtText( title ) );
}

/*!
   Set a new title

   \param title Title
   \sa title()
 */
void QwtPlotItem::setTitle( const QwtText& title )
{
    if ( m_data->title != title )
    {
        m_data->title = title;

        legendChanged();
#if 0
        itemChanged();
#endif
    }
}

/*!
   \return Title of the item
   \sa setTitle()
 */
const QwtText& QwtPlotItem::title() const
{
    return m_data->title;
}

/*!
   Toggle an item attribute

   \param attribute Attribute type
   \param on true/false

   \sa testItemAttribute(), ItemInterest
 */
void QwtPlotItem::setItemAttribute( ItemAttribute attribute, bool on )
{
    if ( m_data->attributes.testFlag( attribute ) != on )
    {
        if ( on )
            m_data->attributes |= attribute;
        else
            m_data->attributes &= ~attribute;

        if ( attribute == QwtPlotItem::Legend )
        {
            if ( on )
            {
                legendChanged();
            }
            else
            {
                /*
                    In the special case of taking an item from
                    the legend we can't use legendChanged() as
                    it depends on QwtPlotItem::Legend being enabled
                 */
                if ( m_data->plot )
                    m_data->plot->updateLegend( this );
            }
        }

        itemChanged();
    }
}

/*!
   Test an item attribute

   \param attribute Attribute type
   \return true/false
   \sa setItemAttribute(), ItemInterest
 */
bool QwtPlotItem::testItemAttribute( ItemAttribute attribute ) const
{
    return m_data->attributes.testFlag( attribute );
}

/*!
   Toggle an item interest

   \param interest Interest type
   \param on true/false

   \sa testItemInterest(), ItemAttribute
 */
void QwtPlotItem::setItemInterest( ItemInterest interest, bool on )
{
    if ( m_data->interests.testFlag( interest ) != on )
    {
        if ( on )
            m_data->interests |= interest;
        else
            m_data->interests &= ~interest;

        itemChanged();
    }
}

/*!
   Test an item interest

   \param interest Interest type
   \return true/false
   \sa setItemInterest(), ItemAttribute
 */
bool QwtPlotItem::testItemInterest( ItemInterest interest ) const
{
    return m_data->interests.testFlag( interest );
}

/*!
   Toggle an render hint

   \param hint Render hint
   \param on true/false

   \sa testRenderHint(), RenderHint
 */
void QwtPlotItem::setRenderHint( RenderHint hint, bool on )
{
    if ( m_data->renderHints.testFlag( hint ) != on )
    {
        if ( on )
            m_data->renderHints |= hint;
        else
            m_data->renderHints &= ~hint;

        itemChanged();
    }
}

/*!
   Test a render hint

   \param hint Render hint
   \return true/false
   \sa setRenderHint(), RenderHint
 */
bool QwtPlotItem::testRenderHint( RenderHint hint ) const
{
    return m_data->renderHints.testFlag( hint );
}

/*!
   On multi core systems rendering of certain plot item
   ( f.e QwtPlotRasterItem ) can be done in parallel in
   several threads.

   The default setting is set to 1.

   \param numThreads Number of threads to be used for rendering.
                     If numThreads is set to 0, the system specific
                     ideal thread count is used.

   The default thread count is 1 ( = no additional threads )
 */
void QwtPlotItem::setRenderThreadCount( uint numThreads )
{
    m_data->renderThreadCount = numThreads;
}

/*!
   \return Number of threads to be used for rendering.
           If numThreads() is set to 0, the system specific
           ideal thread count is used.
 */
uint QwtPlotItem::renderThreadCount() const
{
    return m_data->renderThreadCount;
}

/*!
   Set the size of the legend icon

   The default setting is 8x8 pixels

   \param size Size
   \sa legendIconSize(), legendIcon()
 */
void QwtPlotItem::setLegendIconSize( const QSize& size )
{
    if ( m_data->legendIconSize != size )
    {
        m_data->legendIconSize = size;
        legendChanged();
    }
}

/*!
   \return Legend icon size
   \sa setLegendIconSize(), legendIcon()
 */
QSize QwtPlotItem::legendIconSize() const
{
    return m_data->legendIconSize;
}

/*!
   \return Icon representing the item on the legend

   The default implementation returns an invalid icon

   \param index Index of the legend entry
                ( usually there is only one )
   \param size Icon size

   \sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPlotItem::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index )
    Q_UNUSED( size )

    return QwtGraphic();
}

/*!
   \brief Return a default icon from a brush

   The default icon is a filled rectangle used
   in several derived classes as legendIcon().

   \param brush Fill brush
   \param size Icon size

   \return A filled rectangle
 */
QwtGraphic QwtPlotItem::defaultIcon(
    const QBrush& brush, const QSizeF& size ) const
{
    QwtGraphic icon;
    if ( !size.isEmpty() )
    {
        icon.setDefaultSize( size );

        QRectF r( 0, 0, size.width(), size.height() );

        QPainter painter( &icon );
        painter.fillRect( r, brush );
    }

    return icon;
}

//! Show the item
void QwtPlotItem::show()
{
    setVisible( true );
}

//! Hide the item
void QwtPlotItem::hide()
{
    setVisible( false );
}

/*!
    Show/Hide the item

    \param on Show if true, otherwise hide
    \sa isVisible(), show(), hide()
 */
void QwtPlotItem::setVisible( bool on )
{
    if ( on != m_data->isVisible )
    {
        m_data->isVisible = on;
        itemChanged();
    }
}

/*!
    \return true if visible
    \sa setVisible(), show(), hide()
 */
bool QwtPlotItem::isVisible() const
{
    return m_data->isVisible;
}

/*!
   Update the legend and call QwtPlot::autoRefresh() for the
   parent plot.

   \sa QwtPlot::legendChanged(), QwtPlot::autoRefresh()
 */
void QwtPlotItem::itemChanged()
{
    if ( m_data->plot )
        m_data->plot->autoRefresh();
}

/*!
   Update the legend of the parent plot.
   \sa QwtPlot::updateLegend(), itemChanged()
 */
void QwtPlotItem::legendChanged()
{
    if ( testItemAttribute( QwtPlotItem::Legend ) && m_data->plot )
        m_data->plot->updateLegend( this );
}

/*!
   Set X and Y axis

   The item will painted according to the coordinates of its Axes.

   \param xAxisId X Axis
   \param yAxisId Y Axis

   \sa setXAxis(), setYAxis(), xAxis(), yAxis()
 */
void QwtPlotItem::setAxes( QwtAxisId xAxisId, QwtAxisId yAxisId )
{
    if ( QwtAxis::isXAxis( xAxisId ) )
        m_data->xAxisId = xAxisId;

    if ( QwtAxis::isYAxis( yAxisId ) )
        m_data->yAxisId = yAxisId;

    itemChanged();
}

/*!
   Set the X axis

   The item will painted according to the coordinates its Axes.

   \param axisId X Axis
   \sa setAxes(), setYAxis(), xAxis()
 */
void QwtPlotItem::setXAxis( QwtAxisId axisId )
{
    if ( QwtAxis::isXAxis( axisId ) )
    {
        m_data->xAxisId = axisId;
        itemChanged();
    }
}

/*!
   Set the Y axis

   The item will painted according to the coordinates its Axes.

   \param axisId Y Axis
   \sa setAxes(), setXAxis(), yAxis()
 */
void QwtPlotItem::setYAxis( QwtAxisId axisId )
{
    if ( QwtAxis::isYAxis( axisId ) )
    {
        m_data->yAxisId = axisId;
        itemChanged();
    }
}

//! Return xAxis
QwtAxisId QwtPlotItem::xAxis() const
{
    return m_data->xAxisId;
}

//! Return yAxis
QwtAxisId QwtPlotItem::yAxis() const
{
    return m_data->yAxisId;
}

/*!
   \return An invalid bounding rect: QRectF(1.0, 1.0, -2.0, -2.0)
   \note A width or height < 0.0 is ignored by the autoscaler
 */
QRectF QwtPlotItem::boundingRect() const
{
    return QRectF( 1.0, 1.0, -2.0, -2.0 ); // invalid
}

/*!
   \brief Calculate a hint for the canvas margin

   When the QwtPlotItem::Margins flag is enabled the plot item
   indicates, that it needs some margins at the borders of the canvas.
   This is f.e. used by bar charts to reserve space for displaying
   the bars.

   The margins are in target device coordinates ( pixels on screen )

   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas in painter coordinates
   \param left Returns the left margin
   \param top Returns the top margin
   \param right Returns the right margin
   \param bottom Returns the bottom margin

   The default implementation returns 0 for all margins

   \sa QwtPlot::getCanvasMarginsHint(), QwtPlot::updateCanvasMargins()
 */
void QwtPlotItem::getCanvasMarginHint( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap, const QRectF& canvasRect,
    double& left, double& top, double& right, double& bottom ) const
{
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );
    Q_UNUSED( canvasRect );

    // use QMargins, when we don't need to support Qt < 4.6 anymore
    left = top = right = bottom = 0.0;
}

/*!
   \brief Return all information, that is needed to represent
          the item on the legend

   Most items are represented by one entry on the legend
   showing an icon and a text, but f.e. QwtPlotMultiBarChart
   displays one entry for each bar.

   QwtLegendData is basically a list of QVariants that makes it
   possible to overload and reimplement legendData() to
   return almost any type of information, that is understood
   by the receiver that acts as the legend.

   The default implementation returns one entry with
   the title() of the item and the legendIcon().

   \return Data, that is needed to represent the item on the legend
   \sa title(), legendIcon(), QwtLegend, QwtPlotLegendItem
 */
QList< QwtLegendData > QwtPlotItem::legendData() const
{
    QwtLegendData data;

    QwtText label = title();
    label.setRenderFlags( label.renderFlags() & Qt::AlignLeft );

    data.setValue( QwtLegendData::TitleRole,
        QVariant::fromValue( label ) );

    const QwtGraphic graphic = legendIcon( 0, legendIconSize() );
    if ( !graphic.isNull() )
    {
        data.setValue( QwtLegendData::IconRole,
            QVariant::fromValue( graphic ) );
    }

    QList< QwtLegendData > list;
    list += data;

    return list;
}

/*!
   \brief Update the item to changes of the axes scale division

   Update the item, when the axes of plot have changed.
   The default implementation does nothing, but items that depend
   on the scale division (like QwtPlotGrid()) have to reimplement
   updateScaleDiv()

   updateScaleDiv() is only called when the ScaleInterest interest
   is enabled. The default implementation does nothing.

   \param xScaleDiv Scale division of the x-axis
   \param yScaleDiv Scale division of the y-axis

   \sa QwtPlot::updateAxes(), ScaleInterest
 */
void QwtPlotItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    Q_UNUSED( xScaleDiv );
    Q_UNUSED( yScaleDiv );
}

/*!
   \brief Update the item to changes of the legend info

   Plot items that want to display a legend ( not those, that want to
   be displayed on a legend ! ) will have to implement updateLegend().

   updateLegend() is only called when the LegendInterest interest
   is enabled. The default implementation does nothing.

   \param item Plot item to be displayed on a legend
   \param data Attributes how to display item on the legend

   \sa QwtPlotLegendItem

   \note Plot items, that want to be displayed on a legend
         need to enable the QwtPlotItem::Legend flag and to implement
         legendData() and legendIcon()
 */
void QwtPlotItem::updateLegend( const QwtPlotItem* item,
    const QList< QwtLegendData >& data )
{
    Q_UNUSED( item );
    Q_UNUSED( data );
}

/*!
   \brief Calculate the bounding scale rectangle of 2 maps

   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.

   \return Bounding scale rect of the scale maps, not normalized
 */
QRectF QwtPlotItem::scaleRect( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap ) const
{
    return QRectF( xMap.s1(), yMap.s1(),
        xMap.sDist(), yMap.sDist() );
}

/*!
   \brief Calculate the bounding paint rectangle of 2 maps

   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.

   \return Bounding paint rectangle of the scale maps, not normalized
 */
QRectF QwtPlotItem::paintRect( const QwtScaleMap& xMap,
    const QwtScaleMap& yMap ) const
{
    const QRectF rect( xMap.p1(), yMap.p1(),
        xMap.pDist(), yMap.pDist() );

    return rect;
}
