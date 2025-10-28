/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_polar_item.h"
#include "qwt_legend.h"
#include "qwt_scale_div.h"

#include <qpainter.h>

class QwtPolarItem::PrivateData
{
  public:
    PrivateData()
        : plot( NULL )
        , isVisible( true )
        , renderThreadCount( 1 )
        , z( 0.0 )
        , legendIconSize( 8, 8 )
    {
    }

    mutable QwtPolarPlot* plot;

    bool isVisible;
    QwtPolarItem::ItemAttributes attributes;
    QwtPolarItem::RenderHints renderHints;
    uint renderThreadCount;

    double z;

    QwtText title;
    QSize legendIconSize;
};

/*!
   Constructor

   \param title Item title, f.e used on a legend

   \sa setTitle()
 */
QwtPolarItem::QwtPolarItem( const QwtText& title )
{
    m_data = new PrivateData;
    m_data->title = title;
}

//! Destroy the QwtPolarItem
QwtPolarItem::~QwtPolarItem()
{
    attach( NULL );
    delete m_data;
}

/*!
   \brief Attach the item to a plot.

   This method will attach a QwtPolarItem to the QwtPolarPlot argument.
   It will first detach the QwtPolarItem from any plot from a previous
   call to attach (if necessary).
   If a NULL argument is passed, it will detach from any QwtPolarPlot it
   was attached to.

   \param plot Plot widget

   \sa QwtPolarItem::detach()
 */
void QwtPolarItem::attach( QwtPolarPlot* plot )
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
   \brief This method detaches a QwtPolarItem from the QwtPolarPlot it
          has been associated with.

   detach() is equivalent to calling attach( NULL )
   \sa attach()
 */
void QwtPolarItem::detach()
{
    attach( NULL );
}

/*!
   Return rtti for the specific class represented. QwtPolarItem is simply
   a virtual interface class, and base classes will implement this method
   with specific rtti values so a user can differentiate them.

   The rtti value is useful for environments, where the
   runtime type information is disabled and it is not possible
   to do a dynamic_cast<...>.

   \return rtti value
   \sa RttiValues
 */
int QwtPolarItem::rtti() const
{
    return Rtti_PolarItem;
}

//! \return Attached plot
QwtPolarPlot* QwtPolarItem::plot() const
{
    return m_data->plot;
}

/*!
   Plot items are painted in increasing z-order.

   \return Z value
   \sa setZ(), QwtPolarItemDict::itemList()
 */
double QwtPolarItem::z() const
{
    return m_data->z;
}

/*!
   \brief Set the z value

   Plot items are painted in increasing z-order.

   \param z Z-value
   \sa z(), QwtPolarItemDict::itemList()
 */
void QwtPolarItem::setZ( double z )
{
    if ( m_data->z != z )
    {
        if ( m_data->plot )
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
void QwtPolarItem::setTitle( const QString& title )
{
    setTitle( QwtText( title ) );
}

/*!
   Set a new title

   \param title Title
   \sa title()
 */
void QwtPolarItem::setTitle( const QwtText& title )
{
    if ( m_data->title != title )
    {
        m_data->title = title;
        itemChanged();
    }
}

/*!
   \return Title of the item
   \sa setTitle()
 */
const QwtText& QwtPolarItem::title() const
{
    return m_data->title;
}

/*!
   Toggle an item attribute

   \param attribute Attribute type
   \param on true/false

   \sa testItemAttribute(), ItemAttribute
 */
void QwtPolarItem::setItemAttribute( ItemAttribute attribute, bool on )
{
    if ( bool( m_data->attributes & attribute ) != on )
    {
        if ( on )
            m_data->attributes |= attribute;
        else
            m_data->attributes &= ~attribute;

        itemChanged();
    }
}

/*!
   Test an item attribute

   \param attribute Attribute type
   \return true/false
   \sa setItemAttribute(), ItemAttribute
 */
bool QwtPolarItem::testItemAttribute( ItemAttribute attribute ) const
{
    return m_data->attributes & attribute;
}

/*!
   Toggle an render hint

   \param hint Render hint
   \param on true/false

   \sa testRenderHint(), RenderHint
 */
void QwtPolarItem::setRenderHint( RenderHint hint, bool on )
{
    if ( ( ( m_data->renderHints & hint ) != 0 ) != on )
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
bool QwtPolarItem::testRenderHint( RenderHint hint ) const
{
    return ( m_data->renderHints & hint );
}

/*!
   On multi core systems rendering of certain plot item
   ( f.e QwtPolarSpectrogram ) can be done in parallel in
   several threads.

   The default setting is set to 1.

   \param numThreads Number of threads to be used for rendering.
                     If numThreads is set to 0, the system specific
                     ideal thread count is used.

   The default thread count is 1 ( = no additional threads )
 */
void QwtPolarItem::setRenderThreadCount( uint numThreads )
{
    m_data->renderThreadCount = numThreads;
}

/*!
   \return Number of threads to be used for rendering.
           If numThreads() is set to 0, the system specific
           ideal thread count is used.
 */
uint QwtPolarItem::renderThreadCount() const
{
    return m_data->renderThreadCount;
}

/*!
   Set the size of the legend icon

   The default setting is 8x8 pixels

   \param size Size
   \sa legendIconSize(), legendIcon()
 */
void QwtPolarItem::setLegendIconSize( const QSize& size )
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
QSize QwtPolarItem::legendIconSize() const
{
    return m_data->legendIconSize;
}

//! Show the item
void QwtPolarItem::show()
{
    setVisible( true );
}

//! Hide the item
void QwtPolarItem::hide()
{
    setVisible( false );
}

/*!
    Show/Hide the item

    \param on Show if true, otherwise hide
    \sa isVisible(), show(), hide()
 */
void QwtPolarItem::setVisible( bool on )
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
bool QwtPolarItem::isVisible() const
{
    return m_data->isVisible;
}

/*!
   Update the legend and call QwtPolarPlot::autoRefresh for the
   parent plot.

   \sa updateLegend()
 */
void QwtPolarItem::itemChanged()
{
    if ( m_data->plot )
        m_data->plot->autoRefresh();
}

/*!
   Update the legend of the parent plot.
   \sa QwtPolarPlot::updateLegend(), itemChanged()
 */
void QwtPolarItem::legendChanged()
{
    if ( testItemAttribute( QwtPolarItem::Legend ) && m_data->plot )
        m_data->plot->updateLegend( this );
}

/*!
   Interval, that is necessary to display the item

   This interval can be useful for operations like clipping or autoscaling
   For items ( like the grid ), where a bounding interval makes no
   sense an invalid interval is returned.

   \param scaleId Scale id ( QwtPolar::Scale )
   \return Bounding interval of the plot item for a specific scale
 */
QwtInterval QwtPolarItem::boundingInterval( int scaleId ) const
{
    Q_UNUSED( scaleId );

    return QwtInterval(); // invalid
}

/*!
   \brief Update the item to changes of the axes scale division

   Update the item, when the axes of plot have changed.
   The default implementation does nothing, but items that depend
   on the scale division (like QwtPolarGrid()) have to reimplement
   updateScaleDiv()

   \param azimuthScaleDiv Scale division of the azimuth-scale
   \param radialScaleDiv Scale division of the radius-axis
   \param interval The interval of the radius-axis, that is
                   visible on the canvas

   \sa QwtPolarPlot::updateAxes()
 */
void QwtPolarItem::updateScaleDiv( const QwtScaleDiv& azimuthScaleDiv,
    const QwtScaleDiv& radialScaleDiv, const QwtInterval& interval )
{
    Q_UNUSED( azimuthScaleDiv );
    Q_UNUSED( radialScaleDiv );
    Q_UNUSED( interval );
}

/*!
   \brief Return all information, that is needed to represent
          the item on the legend

   Most items are represented by one entry on the legend
   showing an icon and a text.

   QwtLegendData is basically a list of QVariants that makes it
   possible to overload and reimplement legendData() to
   return almost any type of information, that is understood
   by the receiver that acts as the legend.

   The default implementation returns one entry with
   the title() of the item and the legendIcon().

   \sa title(), legendIcon(), QwtLegend
 */
QList< QwtLegendData > QwtPolarItem::legendData() const
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
   \return Icon representing the item on the legend

   The default implementation returns an invalid icon

   \param index Index of the legend entry
                ( usually there is only one )
   \param size Icon size

   \sa setLegendIconSize(), legendData()
 */
QwtGraphic QwtPolarItem::legendIcon(
    int index, const QSizeF& size ) const
{
    Q_UNUSED( index )
    Q_UNUSED( size )

    return QwtGraphic();
}

/*!
   Some items like to display something (f.e. the azimuth axis) outside
   of the area of the interval of the radial scale.
   The default implementation returns 0 pixels

   \return Hint for the margin
 */
int QwtPolarItem::marginHint() const
{
    return 0;
}
