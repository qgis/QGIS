/***************************************************************************
                             qgsslider.cpp
                             -------------------
    begin                : July 2013
    copyright            : (C) 2013 by Daniel Vaz
    email                : danielvaz at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsslider.h"
#include "qgslogger.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <qmath.h>

QgsSlider::QgsSlider( QWidget * parent ) : QSlider( parent )
{
  setMinimumSize( QSize( 100, 40 ) );
}

QgsSlider::QgsSlider( Qt::Orientation orientation, QWidget * parent ) : QSlider( orientation, parent )
{
  setMinimumSize( QSize( 100, 40 ) );
}

void QgsSlider::paintEvent( QPaintEvent *event )
{
  QSlider::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.5, rect.width(), rect.height() ),
                    Qt::AlignHCenter, variantValue().toString(), 0 );
  painter.end();
}

void QgsSlider::setMinimum( const QVariant &min )
{
  mMin = min;
  update();
}

void QgsSlider::setMaximum( const QVariant &max )
{
  mMax = max;
  update();
}

void QgsSlider::setSingleStep( const QVariant &step )
{
  mStep = step;
  update();
}

void QgsSlider::setValue( const QVariant &value )
{
  mValue = value;
  update();
}

void QgsSlider::update()
{
  if ( mMin.isNull() || mMax.isNull() || mStep.isNull() )
    return;

  if ( mValue.isNull() )
    mValue = mMin;

  if ( mMin.type() == QVariant::Int &&
       mMax.type() == QVariant::Int &&
       mStep.type() == QVariant::Int &&
       mValue.type() == QVariant::Int )
  {
    QSlider::setMinimum( mMin.toInt() );
    QSlider::setMaximum( mMax.toInt() );
    QSlider::setSingleStep( mStep.toInt() );
    QSlider::setValue( mValue.toInt() );
  }

  if ( mMin.type() == QVariant::Double &&
       mMax.type() == QVariant::Double &&
       mStep.type() == QVariant::Double &&
       mValue.type() == QVariant::Double )
  {
    if ( minimum() != 0 )
      QSlider::setMinimum( 0 );

    int max = qCeil(( mMax.toDouble() - mMin.toDouble() ) / mStep.toDouble() );
    if ( maximum() != max )
      QSlider::setMaximum( max );

    if ( singleStep() != 1 )
      QSlider::setSingleStep( 1 );

    QSlider::setValue( qCeil(( mValue.toDouble() - mMin.toDouble() ) / mStep.toDouble() ) );
  }

  connect( this, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
}

QVariant QgsSlider::variantValue() const
{
  return mValue;
}

void QgsSlider::valueChanged( int value )
{
  if ( mMin.isNull() || mMax.isNull() || mStep.isNull() )
  {
    mValue = QVariant();
    return;
  }

  if ( mMin.type() == QVariant::Int &&
       mMax.type() == QVariant::Int &&
       mStep.type() == QVariant::Int &&
       mValue.type() == QVariant::Int )
  {
    mValue = value;
    return;
  }

  if ( mMin.type() == QVariant::Double &&
       mMax.type() == QVariant::Double &&
       mStep.type() == QVariant::Double &&
       mValue.type() == QVariant::Double )
  {
    mValue = QVariant( mMin.toDouble() + value * mStep.toDouble() );
    return;
  }
}
