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
#include "qgsvariantutils.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <cmath>

QgsSlider::QgsSlider( QWidget *parent ) : QSlider( parent )
{
  setMinimumSize( QSize( 100, 40 ) );
}

QgsSlider::QgsSlider( Qt::Orientation orientation, QWidget *parent ) : QSlider( orientation, parent )
{
  setMinimumSize( QSize( 100, 40 ) );
}

void QgsSlider::paintEvent( QPaintEvent *event )
{
  QSlider::paintEvent( event );
  QPainter painter( this );
  const QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.5, rect.width(), rect.height() ),
                    Qt::AlignHCenter, variantValue().toString(), nullptr );
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
  if ( QgsVariantUtils::isNull( mMin ) || QgsVariantUtils::isNull( mMax ) || QgsVariantUtils::isNull( mStep ) )
    return;

  if ( QgsVariantUtils::isNull( mValue ) )
    mValue = mMin;

  if ( mMin.userType() == QMetaType::Type::Int &&
       mMax.userType() == QMetaType::Type::Int &&
       mStep.userType() == QMetaType::Type::Int &&
       mValue.userType() == QMetaType::Type::Int )
  {
    QSlider::setMinimum( mMin.toInt() );
    QSlider::setMaximum( mMax.toInt() );
    QSlider::setSingleStep( mStep.toInt() );
    QSlider::setValue( mValue.toInt() );
  }
  else
  {
    if ( minimum() != 0 )
      QSlider::setMinimum( 0 );

    const int max = std::ceil( ( mMax.toDouble() - mMin.toDouble() ) / mStep.toDouble() );
    if ( maximum() != max )
      QSlider::setMaximum( max );

    if ( singleStep() != 1 )
      QSlider::setSingleStep( 1 );

    QSlider::setValue( std::ceil( ( mValue.toDouble() - mMin.toDouble() ) / mStep.toDouble() ) );
  }

  connect( this, &QSlider::valueChanged, this, &QgsSlider::onValueChanged );
}

QVariant QgsSlider::variantValue() const
{
  return mValue;
}

void QgsSlider::onValueChanged( int value )
{
  if ( QgsVariantUtils::isNull( mMin ) || QgsVariantUtils::isNull( mMax ) || QgsVariantUtils::isNull( mStep ) )
  {
    mValue = QVariant();
  }
  else if ( mMin.userType() == QMetaType::Type::Int &&
            mMax.userType() == QMetaType::Type::Int &&
            mStep.userType() == QMetaType::Type::Int &&
            mValue.userType() == QMetaType::Type::Int )
  {
    mValue = value;
  }
  else
  {
    mValue = QVariant( mMin.toDouble() + value * mStep.toDouble() );
  }

  emit valueChanged( mValue );
}
