/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_polar.h"
#include "qwt_scale_map.h"
#include "qwt_polar_marker.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include "qwt_math.h"

static const int LabelDist = 2;

class QwtPolarMarker::PrivateData
{
  public:
    PrivateData():
        align( Qt::AlignCenter )
    {
      symbol = new QwtSymbol();
    }

    ~PrivateData()
    {
      delete symbol;
    }

    QwtText label;
#if QT_VERSION < 0x040000
    int align;
#else
    Qt::Alignment align;
#endif
    QPen pen;
    QwtSymbol *symbol;

    QwtPolarPoint pos;
};

//! Sets alignment to Qt::AlignCenter, and style to NoLine
QwtPolarMarker::QwtPolarMarker():
    QwtPolarItem( QwtText( "Marker" ) )
{
  d_data = new PrivateData;

  setItemAttribute( QwtPolarItem::AutoScale );
  setZ( 30.0 );
}

//! Destructor
QwtPolarMarker::~QwtPolarMarker()
{
  delete d_data;
}

//! \return QwtPolarItem::Rtti_PlotMarker
int QwtPolarMarker::rtti() const
{
  return QwtPolarItem::Rtti_PolarMarker;
}

//! \return Position of the marker
QwtPolarPoint QwtPolarMarker::position() const
{
  return d_data->pos;
}

//! Change the position of the marker
void QwtPolarMarker::setPosition( const QwtPolarPoint &pos )
{
  if ( d_data->pos != pos )
  {
    d_data->pos = pos;
    itemChanged();
  }
}

/*!
  Draw the marker

  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param radius Radius of the complete plot area in painter coordinates
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPolarMarker::draw( QPainter *painter,
                           const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
                           const QwtDoublePoint &pole, double /* radius */,
                           const QwtDoubleRect & /* canvasRect */ ) const
{
  const double r = radialMap.xTransform( d_data->pos.radius() );
  const double a = azimuthMap.xTransform( d_data->pos.azimuth() );

  const QPoint pos = qwtPolar2Pos( pole, r, a ).toPoint();


  // draw symbol
  QSize sSym( 0, 0 );
  if ( d_data->symbol->style() != QwtSymbol::NoSymbol )
  {
    sSym = d_data->symbol->size();
    d_data->symbol->draw( painter, pos.x(), pos.y() );
  }

  // draw label
  if ( !d_data->label.isEmpty() )
  {
    int xlw = qwtMax( int( d_data->pen.width() ), 1 );
    int ylw = xlw;

    int xlw1 = qwtMax(( xlw + 1 ) / 2, ( sSym.width() + 1 ) / 2 ) + LabelDist;
    xlw = qwtMax( xlw / 2, ( sSym.width() + 1 ) / 2 ) + LabelDist;
    int ylw1 = qwtMax(( ylw + 1 ) / 2, ( sSym.height() + 1 ) / 2 ) + LabelDist;
    ylw = qwtMax( ylw / 2, ( sSym. height() + 1 ) / 2 ) + LabelDist;

    QRect tr( QPoint( 0, 0 ), d_data->label.textSize( painter->font() ) );
    tr.moveCenter( QPoint( 0, 0 ) );

    int dx = pos.x();
    int dy = pos.y();

    if ( d_data->align & ( int ) Qt::AlignTop )
      dy += tr.y() - ylw1;
    else if ( d_data->align & ( int ) Qt::AlignBottom )
      dy -= tr.y() - ylw1;

    if ( d_data->align & ( int ) Qt::AlignLeft )
      dx += tr.x() - xlw1;
    else if ( d_data->align & ( int ) Qt::AlignRight )
      dx -= tr.x() - xlw1;

#if QT_VERSION < 0x040000
    tr.moveBy( dx, dy );
#else
    tr.translate( dx, dy );
#endif
    d_data->label.draw( painter, tr );
  }
}

/*!
  \brief Assign a symbol
  \param s New symbol
  \sa symbol()
*/
void QwtPolarMarker::setSymbol( const QwtSymbol &s )
{
  delete d_data->symbol;
  d_data->symbol = s.clone();
  itemChanged();
}

/*!
  \return the symbol
  \sa setSymbol(), QwtSymbol
*/
const QwtSymbol &QwtPolarMarker::symbol() const
{
  return *d_data->symbol;
}

/*!
  \brief Set the label
  \param label label text
  \sa label()
*/
void QwtPolarMarker::setLabel( const QwtText& label )
{
  if ( label != d_data->label )
  {
    d_data->label = label;
    itemChanged();
  }
}

/*!
  \return the label
  \sa setLabel()
*/
QwtText QwtPolarMarker::label() const
{
  return d_data->label;
}

/*!
  \brief Set the alignment of the label

  The alignment determines where the label is drawn relative to
  the marker's position.

  \param align Alignment. A combination of AlignTop, AlignBottom,
    AlignLeft, AlignRight, AlignCenter, AlgnHCenter,
    AlignVCenter.
  \sa labelAlignment()
*/
#if QT_VERSION < 0x040000
void QwtPolarMarker::setLabelAlignment( int align )
#else
void QwtPolarMarker::setLabelAlignment( Qt::Alignment align )
#endif
{
  if ( align == d_data->align )
    return;

  d_data->align = align;
  itemChanged();
}

/*!
  \return the label alignment
  \sa setLabelAlignment()
*/
#if QT_VERSION < 0x040000
int QwtPolarMarker::labelAlignment() const
#else
Qt::Alignment QwtPolarMarker::labelAlignment() const
#endif
{
  return d_data->align;
}

/*!
   Interval, that is necessary to display the item
   This interval can be useful for operations like clipping or autoscaling

   \param scaleId Scale index
   \return bounding interval ( == position )

   \sa position()
*/
QwtDoubleInterval QwtPolarMarker::boundingInterval( int scaleId ) const
{
  const double v = ( scaleId == QwtPolar::ScaleRadius )
                   ? d_data->pos.radius() : d_data-> pos.azimuth();

  return QwtDoubleInterval( v, v );
}
