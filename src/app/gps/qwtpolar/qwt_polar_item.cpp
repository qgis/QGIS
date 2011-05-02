/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_plot.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_polar_item.h"

class QwtPolarItem::PrivateData
{
  public:
    PrivateData():
        plot( NULL ),
        isVisible( true ),
        attributes( 0 ),
#if QT_VERSION >= 0x040000
        renderHints( 0 ),
#endif
        z( 0.0 )
    {
    }

    mutable QwtPolarPlot *plot;

    bool isVisible;
    int attributes;
#if QT_VERSION >= 0x040000
    int renderHints;
#endif
    double z;

    QwtText title;
};

/*!
   Constructor

   \param title Item title, f.e used on a legend

   \sa setTitle()
*/
QwtPolarItem::QwtPolarItem( const QwtText &title )
{
  d_data = new PrivateData;
  d_data->title = title;
}

//! Destroy the QwtPolarItem
QwtPolarItem::~QwtPolarItem()
{
  attach( NULL );
  delete d_data;
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
void QwtPolarItem::attach( QwtPolarPlot *plot )
{
  if ( plot == d_data->plot )
    return;

  // remove the item from the previous plot

  if ( d_data->plot )
  {
    d_data->plot->attachItem( this, false );

    if ( d_data->plot->autoReplot() )
      d_data->plot->update();
  }

  d_data->plot = plot;

  if ( d_data->plot )
  {
    // insert the item into the current plot

    d_data->plot->attachItem( this, true );
    itemChanged();
  }
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

//! Return attached plot
QwtPolarPlot *QwtPolarItem::plot() const
{
  return d_data->plot;
}

/*!
   Plot items are painted in increasing z-order.

   \return setZ(), QwtPolarItemDict::itemList()
*/
double QwtPolarItem::z() const
{
  return d_data->z;
}

/*!
   \brief Set the z value

   Plot items are painted in increasing z-order.

   \param z Z-value
   \sa z(), QwtPolarItemDict::itemList()
*/
void QwtPolarItem::setZ( double z )
{
  if ( d_data->z != z )
  {
    d_data->z = z;
    if ( d_data->plot )
    {
      // update the z order
      d_data->plot->attachItem( this, false );
      d_data->plot->attachItem( this, true );
    }
    itemChanged();
  }
}

/*!
   Set a new title

   \param title Title
   \sa title()
*/
void QwtPolarItem::setTitle( const QString &title )
{
  setTitle( QwtText( title ) );
}

/*!
   Set a new title

   \param title Title
   \sa title()
*/
void QwtPolarItem::setTitle( const QwtText &title )
{
  if ( d_data->title != title )
  {
    d_data->title = title;
    itemChanged();
  }
}

/*!
   \return Title of the item
   \sa setTitle()
*/
const QwtText &QwtPolarItem::title() const
{
  return d_data->title;
}

/*!
   Toggle an item attribute

   \param attribute Attribute type
   \param on true/false

   \sa testItemAttribute(), ItemAttribute
*/
void QwtPolarItem::setItemAttribute( ItemAttribute attribute, bool on )
{
  if ( bool( d_data->attributes & attribute ) != on )
  {
    if ( on )
      d_data->attributes |= attribute;
    else
      d_data->attributes &= ~attribute;

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
  return d_data->attributes & attribute;
}

#if QT_VERSION >= 0x040000

/*!
   Toggle an render hint

   \param hint Render hint
   \param on true/false

   \sa testRenderHint(), RenderHint
*/
void QwtPolarItem::setRenderHint( RenderHint hint, bool on )
{
  if ((( d_data->renderHints & hint ) != 0 ) != on )
  {
    if ( on )
      d_data->renderHints |= hint;
    else
      d_data->renderHints &= ~hint;

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
  return ( d_data->renderHints & hint );
}

#endif

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
  if ( on != d_data->isVisible )
  {
    d_data->isVisible = on;
    itemChanged();
  }
}

/*!
    \return true if visible
    \sa setVisible(), show(), hide()
*/
bool QwtPolarItem::isVisible() const
{
  return d_data->isVisible;
}

/*!
   Update the legend and call QwtPolarPlot::autoRefresh for the
   parent plot.

   \sa updateLegend()
*/
void QwtPolarItem::itemChanged()
{
  if ( d_data->plot )
  {
    if ( d_data->plot->legend() )
      updateLegend( d_data->plot->legend() );

    d_data->plot->autoRefresh();
  }
}

/*!
   Interval, that is necessary to display the item

   This interval can be useful for operations like clipping or autoscaling
   For items ( like the grid ), where a bounding interval makes no
   sense an invalid interval is returned.

   \param scaleId Scale id ( QwtPolar::Scale )
*/
QwtDoubleInterval QwtPolarItem::boundingInterval( int /*scaleId*/ ) const
{
  return QwtDoubleInterval(); // invalid
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
void QwtPolarItem::updateScaleDiv( const QwtScaleDiv & /* azimuthScaleDiv */,
                                   const QwtScaleDiv & /* radialScaleDiv */,
                                   const QwtDoubleInterval & /*interval */ )
{
}

/*!
   \brief Update the widget that represents the item on the legend

   updateLegend() is called from itemChanged() to adopt the widget
   representing the item on the legend to its new configuration.

   The default implementation is made for QwtPolarCurve and updates a
   QwtLegendItem(), but an item could be represented by any type of widget,
   by overloading legendItem() and updateLegend().

   \sa legendItem(), itemChanged(), QwtLegend()
*/
void QwtPolarItem::updateLegend( QwtLegend *legend ) const
{
  if ( !legend )
    return;

  QWidget *lgdItem = legend->find( this );
  if ( testItemAttribute( QwtPolarItem::Legend ) )
  {
    if ( lgdItem == NULL )
    {
      lgdItem = legendItem();
      if ( lgdItem )
      {
        if ( lgdItem->inherits( "QwtLegendItem" ) )
        {
          QwtLegendItem *label = ( QwtLegendItem * )lgdItem;
          label->setItemMode( legend->itemMode() );

          if ( d_data->plot )
          {
            QObject::connect( label, SIGNAL( clicked() ),
                              d_data->plot, SLOT( legendItemClicked() ) );
            QObject::connect( label, SIGNAL( checked( bool ) ),
                              d_data->plot, SLOT( legendItemChecked( bool ) ) );
          }
        }
        legend->insert( this, lgdItem );
      }
    }
    if ( lgdItem && lgdItem->inherits( "QwtLegendItem" ) )
    {
      QwtLegendItem* label = ( QwtLegendItem* )lgdItem;
      if ( label )
        label->setText( d_data->title );
    }
  }
  else
  {
    delete lgdItem;
  }
}

/*!
   \brief Allocate the widget that represents the item on the legend

   The default implementation is made for QwtPolarCurve and returns a
   QwtLegendItem(), but an item could be represented by any type of widget,
   by overloading legendItem() and updateLegend().

   \return QwtLegendItem()
   \sa updateLegend() QwtLegend()
*/
QWidget *QwtPolarItem::legendItem() const
{
#if QWT_VERSION >= 0x050300
  return new QwtLegendCurveItem;
#else
  return new QwtLegendItem;
#endif
}

/*!
   Some items like to display something (f.e. the azimuth axis) outside
   of the area of the interval of the radial scale.
   The default implementation returns 0 pixels
*/
int QwtPolarItem::marginHint() const
{
  return 0;
}
