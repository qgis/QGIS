/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/
#include <qwt_global.h>

#if defined( QWT_VERSION ) && QWT_VERSION<0x060000

#include <qglobal.h>
#include <qcolor.h>

#include "qwt5_histogram_item.h"

HistogramItem::HistogramItem( const QwtText &title ):
    QwtPlotItem( title )
{
  init();
}

HistogramItem::HistogramItem( const QString &title ):
    QwtPlotItem( QwtText( title ) )
{
  init();
}

HistogramItem::~HistogramItem()
{
  delete d_data;
}

void HistogramItem::init()
{
  d_data = new PrivateData();
  d_data->reference = 0.0;
  d_data->attributes = HistogramItem::Auto;
  d_data->flat = false;
  d_data->spacing = 1;
  d_data->pen = Qt::NoPen;

  setItemAttribute( QwtPlotItem::AutoScale, true );
  setItemAttribute( QwtPlotItem::Legend, true );

  setZ( 20.0 );
}

void HistogramItem::setBaseline( double reference )
{
  if ( d_data->reference != reference )
  {
    d_data->reference = reference;
    itemChanged();
  }
}

double HistogramItem::baseline() const
{
  return d_data->reference;
}

void HistogramItem::setData( const QwtIntervalData &data )
{
  d_data->data = data;
  itemChanged();
}

const QwtIntervalData &HistogramItem::data() const
{
  return d_data->data;
}

void HistogramItem::setColor( const QColor &color )
{
  if ( d_data->color != color )
  {
    d_data->color = color;
    itemChanged();
  }
}

QColor HistogramItem::color() const
{
  return d_data->color;
}

void HistogramItem::setFlat( bool flat )
{
  if ( d_data->flat != flat )
  {
    d_data->flat = flat;
    itemChanged();
  }
}

bool HistogramItem::flat() const
{
  return d_data->flat;
}

void HistogramItem::setSpacing( int spacing )
{
  if ( d_data->spacing != spacing )
  {
    d_data->spacing = spacing;
    itemChanged();
  }
}

int HistogramItem::spacing() const
{
  return d_data->spacing;
}

void HistogramItem::setPen( const QPen &pen )
{
  if ( d_data->pen != pen )
  {
    d_data->pen = pen;
    itemChanged();
  }
}

QPen HistogramItem::pen() const
{
  return d_data->pen;
}

QwtDoubleRect HistogramItem::boundingRect() const
{
  QwtDoubleRect rect = d_data->data.boundingRect();
  if ( !rect.isValid() )
    return rect;

  if ( d_data->attributes & Xfy )
  {
    rect = QwtDoubleRect( rect.y(), rect.x(),
                          rect.height(), rect.width() );

    if ( rect.left() > d_data->reference )
      rect.setLeft( d_data->reference );
    else if ( rect.right() < d_data->reference )
      rect.setRight( d_data->reference );
  }
  else
  {
    if ( rect.bottom() < d_data->reference )
      rect.setBottom( d_data->reference );
    else if ( rect.top() > d_data->reference )
      rect.setTop( d_data->reference );
  }

  return rect;
}


int HistogramItem::rtti() const
{
  return QwtPlotItem::Rtti_PlotHistogram;
}

void HistogramItem::setHistogramAttribute( HistogramAttribute attribute, bool on )
{
  if ( bool( d_data->attributes & attribute ) == on )
    return;

  if ( on )
    d_data->attributes |= attribute;
  else
    d_data->attributes &= ~attribute;

  itemChanged();
}

bool HistogramItem::testHistogramAttribute( HistogramAttribute attribute ) const
{
  return d_data->attributes & attribute;
}

void HistogramItem::draw( QPainter *painter, const QwtScaleMap &xMap,
                          const QwtScaleMap &yMap, const QRect & ) const
{
  const QwtIntervalData &iData = d_data->data;
  const int x0 = xMap.transform( baseline() );
  const int y0 = yMap.transform( baseline() );

  for ( int i = 0; i < ( int )iData.size(); i++ )
  {
    if ( d_data->attributes & HistogramItem::Xfy )
    {
      const int x2 = xMap.transform( iData.value( i ) );
      if ( x2 == x0 )
        continue;

      int y1 = yMap.transform( iData.interval( i ).minValue() );
      int y2 = yMap.transform( iData.interval( i ).maxValue() );
      if ( y1 > y2 )
        qSwap( y1, y2 );

      if ( i < ( int )iData.size() - 2 )
      {
        const int yy1 = yMap.transform( iData.interval( i + 1 ).minValue() );
        const int yy2 = yMap.transform( iData.interval( i + 1 ).maxValue() );

        if ( y2 == qwtMin( yy1, yy2 ) )
        {
          const int xx2 = xMap.transform(
                            iData.interval( i + 1 ).minValue() );
          if ( xx2 != x0 && (( xx2 < x0 && x2 < x0 ) ||
                             ( xx2 > x0 && x2 > x0 ) ) )
          {
            // distance between neighboured bars
            y2 += d_data->spacing;
          }
        }
      }

      drawBar( painter, Qt::Horizontal,
               QRect( x0, y1, x2 - x0, y2 - y1 ) );
    }
    else
    {
      const int y2 = yMap.transform( iData.value( i ) );
      if ( y2 == y0 )
        continue;

      int x1 = xMap.transform( iData.interval( i ).minValue() );
      int x2 = xMap.transform( iData.interval( i ).maxValue() );
      if ( x1 > x2 )
        qSwap( x1, x2 );

      if ( i < ( int )iData.size() - 2 )
      {
        const int xx1 = xMap.transform( iData.interval( i + 1 ).minValue() );
        const int xx2 = xMap.transform( iData.interval( i + 1 ).maxValue() );

        if ( x2 == qwtMin( xx1, xx2 ) )
        {
          const int yy2 = yMap.transform( iData.value( i + 1 ) );
          if ( yy2 != y0 && (( yy2 < y0 && y2 < y0 ) ||
                             ( yy2 > y0 && y2 > y0 ) ) )
          {
            //distance between neighboured bars
            x2 -= d_data->spacing;
          }
        }
      }
      drawBar( painter, Qt::Vertical,
               QRect( x1, y0, x2 - x1, y2 - y0 ) );
    }
  }
}

void HistogramItem::drawBar( QPainter *painter,
                             Qt::Orientation, const QRect& rect ) const
{
  painter->save();

  const QRect r = rect.normalized();
  painter->setBrush( d_data->color );

  if ( d_data->flat )
  {
    painter->setPen( d_data->pen );
    int penWidth = d_data->pen == Qt::NoPen ? 0 :
                   ( d_data->pen.isCosmetic() ? 1 : d_data->pen.width() );
    QwtPainter::drawRect( painter, r.x(), r.y(),
                          r.width(), r.height() - penWidth );
  }
  else
  {
    const int factor = 125;
    const QColor light( d_data->color.light( factor ) );
    const QColor dark( d_data->color.dark( factor ) );

    QwtPainter::drawRect( painter, r.x() + 1, r.y() + 1,
                          r.width() - 2, r.height() - 2 );

    painter->setBrush( Qt::NoBrush );

    painter->setPen( QPen( light, 2 ) );
    QwtPainter::drawLine( painter,
                          r.left() + 1, r.top() + 2, r.right() + 1, r.top() + 2 );

    painter->setPen( QPen( dark, 2 ) );
    QwtPainter::drawLine( painter,
                          r.left() + 1, r.bottom(), r.right() + 1, r.bottom() );

    painter->setPen( QPen( light, 1 ) );

    QwtPainter::drawLine( painter,
                          r.left(), r.top() + 1, r.left(), r.bottom() );
    QwtPainter::drawLine( painter,
                          r.left() + 1, r.top() + 2, r.left() + 1, r.bottom() - 1 );

    painter->setPen( QPen( dark, 1 ) );

    QwtPainter::drawLine( painter,
                          r.right() + 1, r.top() + 1, r.right() + 1, r.bottom() );
    QwtPainter::drawLine( painter,
                          r.right(), r.top() + 2, r.right(), r.bottom() - 1 );
  }

  painter->restore();
}

//!  Update the widget that represents the curve on the legend
// this was adapted from QwtPlotCurve::updateLegend()
void HistogramItem::updateLegend( QwtLegend *legend ) const
{
  if ( !legend )
    return;

  QwtPlotItem::updateLegend( legend );

  QWidget *widget = legend->find( this );
  if ( !widget || !widget->inherits( "QwtLegendItem" ) )
    return;

  QwtLegendItem *legendItem = ( QwtLegendItem * )widget;

  const bool doUpdate = legendItem->updatesEnabled();
  legendItem->setUpdatesEnabled( false );

  const int policy = legend->displayPolicy();

  if ( policy == QwtLegend::FixedIdentifier )
  {
    int mode = legend->identifierMode();

    legendItem->setCurvePen( QPen( color() ) );

    if ( mode & QwtLegendItem::ShowText )
      legendItem->setText( title() );
    else
      legendItem->setText( QwtText() );

    legendItem->setIdentifierMode( mode );
  }
  else if ( policy == QwtLegend::AutoIdentifier )
  {
    int mode = 0;

    legendItem->setCurvePen( QPen( color() ) );
    mode |= QwtLegendItem::ShowLine;
    if ( !title().isEmpty() )
    {
      legendItem->setText( title() );
      mode |= QwtLegendItem::ShowText;
    }
    else
    {
      legendItem->setText( QwtText() );
    }
    legendItem->setIdentifierMode( mode );
  }

  legendItem->setUpdatesEnabled( doUpdate );
  legendItem->update();
}

#endif
