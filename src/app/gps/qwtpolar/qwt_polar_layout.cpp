/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qscrollbar.h>
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_legend.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_polar_layout.h"

class QwtPolarLayout::LayoutData
{
  public:
    void init( const QwtPolarPlot *, const QRect &rect );

    struct t_legendData
    {
      int frameWidth;
      int vScrollBarWidth;
      int hScrollBarHeight;
      QSize hint;
    } legend;

    struct t_titleData
    {
      QwtText text;
      int frameWidth;
    } title;

    struct t_canvasData
    {
      int frameWidth;
    } canvas;
};

/*
  Extract all layout relevant data from the plot components
*/

void QwtPolarLayout::LayoutData::init(
  const QwtPolarPlot *plot, const QRect &rect )
{
  // legend

  if ( plot->plotLayout()->legendPosition() != QwtPolarPlot::ExternalLegend
       && plot->legend() )
  {
    legend.frameWidth = plot->legend()->frameWidth();
    legend.vScrollBarWidth =
      plot->legend()->verticalScrollBar()->sizeHint().width();
    legend.hScrollBarHeight =
      plot->legend()->horizontalScrollBar()->sizeHint().height();

    const QSize hint = plot->legend()->sizeHint();

    int w = qwtMin( hint.width(), rect.width() );
    int h = plot->legend()->heightForWidth( w );
    if ( h == 0 )
      h = hint.height();

    if ( h > rect.height() )
      w += legend.vScrollBarWidth;

    legend.hint = QSize( w, h );
  }

  // title

  title.frameWidth = 0;
  title.text = QwtText();

  if ( plot->titleLabel() )
  {
    const QwtTextLabel *label = plot->titleLabel();
    title.text = label->text();
    if ( !( title.text.testPaintAttribute( QwtText::PaintUsingTextFont ) ) )
      title.text.setFont( label->font() );

    title.frameWidth = plot->titleLabel()->frameWidth();
  }

  // canvas

  canvas.frameWidth = plot->canvas()->frameWidth();
}

class QwtPolarLayout::PrivateData
{
  public:
    PrivateData():
        margin( 0 ),
        spacing( 0 )
    {
    }

    QRect titleRect;
    QRect legendRect;
    QRect canvasRect;

    QwtPolarLayout::LayoutData layoutData;

    QwtPolarPlot::LegendPosition legendPos;
    double legendRatio;

    unsigned int margin;
    unsigned int spacing;
};

/*!
  \brief Constructor
 */

QwtPolarLayout::QwtPolarLayout()
{
  d_data = new PrivateData;

  setLegendPosition( QwtPolarPlot::BottomLegend );
  invalidate();
}

//! Destructor
QwtPolarLayout::~QwtPolarLayout()
{
  delete d_data;
}

/*!
  \brief Specify the position of the legend
  \param pos The legend's position.
  \param ratio Ratio between legend and the bounding rect
               of title, canvas and axes. The legend will be shrinked
               if it would need more space than the given ratio.
               The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0
               it will be reset to the default ratio.
               The default vertical/horizontal ratio is 0.33/0.5.

  \sa QwtPolarPlot::setLegendPosition()
*/

void QwtPolarLayout::setLegendPosition( QwtPolarPlot::LegendPosition pos, double ratio )
{
  if ( ratio > 1.0 )
    ratio = 1.0;

  switch ( pos )
  {
    case QwtPolarPlot::TopLegend:
    case QwtPolarPlot::BottomLegend:
      if ( ratio <= 0.0 )
        ratio = 0.33;
      d_data->legendRatio = ratio;
      d_data->legendPos = pos;
      break;
    case QwtPolarPlot::LeftLegend:
    case QwtPolarPlot::RightLegend:
      if ( ratio <= 0.0 )
        ratio = 0.5;
      d_data->legendRatio = ratio;
      d_data->legendPos = pos;
      break;
    case QwtPolarPlot::ExternalLegend:
      d_data->legendRatio = ratio; // meaningless
      d_data->legendPos = pos;
    default:
      break;
  }
}

/*!
  \brief Specify the position of the legend
  \param pos The legend's position. Valid values are
      \c QwtPolarPlot::LeftLegend, \c QwtPolarPlot::RightLegend,
      \c QwtPolarPlot::TopLegend, \c QwtPolarPlot::BottomLegend.

  \sa QwtPolarPlot::setLegendPosition()
*/
void QwtPolarLayout::setLegendPosition( QwtPolarPlot::LegendPosition pos )
{
  setLegendPosition( pos, 0.0 );
}

/*!
  \return Position of the legend
  \sa setLegendPosition(), QwtPolarPlot::setLegendPosition(),
      QwtPolarPlot::legendPosition()
*/
QwtPolarPlot::LegendPosition QwtPolarLayout::legendPosition() const
{
  return d_data->legendPos;
}

/*!
  Specify the relative size of the legend in the plot
  \param ratio Ratio between legend and the bounding rect
               of title, canvas and axes. The legend will be shrinked
               if it would need more space than the given ratio.
               The ratio is limited to ]0.0 .. 1.0]. In case of <= 0.0
               it will be reset to the default ratio.
               The default vertical/horizontal ratio is 0.33/0.5.
*/
void QwtPolarLayout::setLegendRatio( double ratio )
{
  setLegendPosition( legendPosition(), ratio );
}

/*!
  \return The relative size of the legend in the plot.
  \sa setLegendPosition()
*/
double QwtPolarLayout::legendRatio() const
{
  return d_data->legendRatio;
}

/*!
  \return Geometry for the title
  \sa activate(), invalidate()
*/

const QRect &QwtPolarLayout::titleRect() const
{
  return d_data->titleRect;
}

/*!
  \return Geometry for the legend
  \sa activate(), invalidate()
*/

const QRect &QwtPolarLayout::legendRect() const
{
  return d_data->legendRect;
}

/*!
  \return Geometry for the canvas
  \sa activate(), invalidate()
*/
const QRect &QwtPolarLayout::canvasRect() const
{
  return d_data->canvasRect;
}

/*!
  Invalidate the geometry of all components.
  \sa activate()
*/
void QwtPolarLayout::invalidate()
{
  d_data->titleRect = d_data->legendRect = d_data->canvasRect = QRect();
}

/*!
  \brief Return a minimum size hint
  \sa QwtPolarPlot::minimumSizeHint()
*/
QSize QwtPolarLayout::minimumSizeHint( const QwtPolarPlot * ) const
{
  return QSize();
}

/*!
  Find the geometry for the legend
  \param options Options how to layout the legend
  \param rect Rectangle where to place the legend
  \return Geometry for the legend
*/

QRect QwtPolarLayout::layoutLegend( int options,
                                    const QRect &rect ) const
{
  const QSize hint( d_data->layoutData.legend.hint );

  int dim;
  if ( d_data->legendPos == QwtPolarPlot::LeftLegend
       || d_data->legendPos == QwtPolarPlot::RightLegend )
  {
    // We don't allow vertical legends to take more than
    // half of the available space.

    dim = qwtMin( hint.width(), int( rect.width() * d_data->legendRatio ) );

    if ( !( options & IgnoreScrollbars ) )
    {
      if ( hint.height() > rect.height() )
      {
        // The legend will need additional
        // space for the vertical scrollbar.

        dim += d_data->layoutData.legend.vScrollBarWidth;
      }
    }
  }
  else
  {
    dim = qwtMin( hint.height(), int( rect.height() * d_data->legendRatio ) );
    dim = qwtMax( dim, d_data->layoutData.legend.hScrollBarHeight );
  }

  QRect legendRect = rect;
  switch ( d_data->legendPos )
  {
    case QwtPolarPlot::LeftLegend:
      legendRect.setWidth( dim );
      break;
    case QwtPolarPlot::RightLegend:
      legendRect.setX( rect.right() - dim + 1 );
      legendRect.setWidth( dim );
      break;
    case QwtPolarPlot::TopLegend:
      legendRect.setHeight( dim );
      break;
    case QwtPolarPlot::BottomLegend:
      legendRect.setY( rect.bottom() - dim + 1 );
      legendRect.setHeight( dim );
      break;
    case QwtPolarPlot::ExternalLegend:
      break;
  }

  return legendRect;
}

/*!
  \brief Recalculate the geometry of all components.

  \param plot Plot to be layout
  \param boundingRect Rect where to place the components
  \param options Options

  \sa invalidate(), titleRect(), legendRect(), canvasRect()
*/
void QwtPolarLayout::activate( const QwtPolarPlot *plot,
                               const QRect &boundingRect, int options )
{
  invalidate();

  QRect rect( boundingRect );  // undistributed rest of the plot rect

  // subtract the margin
  rect.setRect(
    rect.x() + d_data->margin,
    rect.y() + d_data->margin,
    rect.width() - 2 * d_data->margin,
    rect.height() - 2 * d_data->margin
  );

  // We extract all layout relevant data from the widgets
  // and save them to d_data->layoutData.

  d_data->layoutData.init( plot, rect );
  if ( !( options & IgnoreLegend )
       && d_data->legendPos != QwtPolarPlot::ExternalLegend
       && plot->legend() && !plot->legend()->isEmpty() )
  {
    d_data->legendRect = layoutLegend( options, rect );

    // subtract d_data->legendRect from rect

    const QRegion region( rect );
    rect = region.subtract( d_data->legendRect ).boundingRect();

    if ( d_data->layoutData.legend.frameWidth &&
         !( options & IgnoreFrames ) )
    {
      // In case of a frame we have to insert a spacing.
      // Otherwise the leading of the font separates
      // legend and scale/canvas

      switch ( d_data->legendPos )
      {
        case QwtPolarPlot::LeftLegend:
          rect.setLeft( rect.left() + d_data->spacing );
          break;
        case QwtPolarPlot::RightLegend:
          rect.setRight( rect.right() - d_data->spacing );
          break;
        case QwtPolarPlot::TopLegend:
          rect.setTop( rect.top() + d_data->spacing );
          break;
        case QwtPolarPlot::BottomLegend:
          rect.setBottom( rect.bottom() - d_data->spacing );
          break;
        case QwtPolarPlot::ExternalLegend:
          break; // suppress compiler warning
      }
    }
  }

  if ( !( options & IgnoreTitle ) &&
       !d_data->layoutData.title.text.isEmpty() )
  {
    int h = d_data->layoutData.title.text.heightForWidth( rect.width() );
    if ( !( options & IgnoreFrames ) )
      h += 2 * d_data->layoutData.title.frameWidth;

    d_data->titleRect = QRect( rect.x(), rect.y(),
                               rect.width(), h );

    // subtract title
    rect.setTop( rect.top() + h + d_data->spacing );
  }

  if ( plot->zoomPos().radius() > 0.0 || plot->zoomFactor() < 1.0 )
  {
    // In zoomed state we have no idea about the geometry that
    // is best for the plot. So we use the complete rectangle
    // accepting, that there might a lot of space wasted
    // around the plot.

    d_data->canvasRect = rect;
  }
  else
  {
    // In full state we know, that we want
    // to display something circular.

    const int dim = qwtMin( rect.width(), rect.height() );

    d_data->canvasRect.setX( rect.center().x() - dim / 2 );
    d_data->canvasRect.setY( rect.y() );
    d_data->canvasRect.setSize( QSize( dim, dim ) );
  }

  if ( !d_data->legendRect.isEmpty() )
  {
    if ( d_data->legendPos == QwtPolarPlot::LeftLegend
         || d_data->legendPos == QwtPolarPlot::RightLegend )
    {
      // We prefer to align the legend to the canvas - not to
      // the complete plot - if possible.

      if ( d_data->layoutData.legend.hint.height()
           < d_data->canvasRect.height() )
      {
        d_data->legendRect.setY( d_data->canvasRect.y() );
        d_data->legendRect.setHeight( d_data->canvasRect.height() );
      }
    }

    // Shift the legend, so that it is
    // aligned to the canvas

    switch ( d_data->legendPos )
    {
      case QwtPolarPlot::LeftLegend:
      {
        d_data->legendRect.moveRight( d_data->canvasRect.left() - 1 );
        break;
      }
      case QwtPolarPlot::RightLegend:
      {
        d_data->legendRect.moveLeft( d_data->canvasRect.right() + 1 );
        break;
      }
      case QwtPolarPlot::TopLegend:
      {
        d_data->legendRect.moveBottom( d_data->canvasRect.top() - 1 );
        break;
      }
      case QwtPolarPlot::BottomLegend:
      {
        d_data->legendRect.moveTop( d_data->canvasRect.bottom() + 1 );
        break;
      }
      default:;
    }
  }
}
