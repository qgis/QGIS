/***************************************************************************
                             qgsdial.cpp
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

#include "qgsdial.h"
#include "qgslogger.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <qmath.h>

QgsDial::QgsDial( QWidget *parent ) : QDial( parent )
{
  setMinimumSize( QSize( 50, 50 ) );
}

void QgsDial::paintEvent( QPaintEvent *event )
{
  QDial::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.65, rect.width(), rect.height() ),
                    Qt::AlignHCenter, variantValue().toString(), 0 );
  painter.end();
}

void QgsDial::setMinimum( const QVariant &min )
{
  mMin = min;
  update();
}

void QgsDial::setMaximum( const QVariant &max )
{
  mMax = max;
  update();
}

void QgsDial::setSingleStep( const QVariant &step )
{
  mStep = step;
  update();
}

void QgsDial::setValue( const QVariant &value )
{
  mValue = value;
  update();
}

void QgsDial::update()
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
    QDial::setMinimum( mMin.toInt() );
    QDial::setMaximum( mMax.toInt() );
    QDial::setSingleStep( mStep.toInt() );
    QDial::setValue( mValue.toInt() );
  }

  if ( mMin.type() == QVariant::Double &&
       mMax.type() == QVariant::Double &&
       mStep.type() == QVariant::Double &&
       mValue.type() == QVariant::Double )
  {
    if ( minimum() != 0 )
      QDial::setMinimum( 0 );

    int max = qCeil(( mMax.toDouble() - mMin.toDouble() ) / mStep.toDouble() );
    if ( maximum() != max )
      QDial::setMaximum( max );

    if ( singleStep() != 1 )
      QDial::setSingleStep( 1 );

    QDial::setValue( qCeil(( mValue.toDouble() - mMin.toDouble() ) / mStep.toDouble() ) );
  }

  connect( this, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
}

QVariant QgsDial::variantValue() const
{
  return mValue;
}

void QgsDial::valueChanged( int value )
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
