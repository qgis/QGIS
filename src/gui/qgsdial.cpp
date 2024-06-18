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
#include "qgsvariantutils.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <cmath>

QgsDial::QgsDial( QWidget *parent ) : QDial( parent )
{
  setMinimumSize( QSize( 50, 50 ) );
}

void QgsDial::paintEvent( QPaintEvent *event )
{
  QDial::paintEvent( event );
  QPainter painter( this );
  const QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.65, rect.width(), rect.height() ),
                    Qt::AlignHCenter, variantValue().toString(), nullptr );
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
  if ( QgsVariantUtils::isNull( mMin ) || QgsVariantUtils::isNull( mMax ) || QgsVariantUtils::isNull( mStep ) )
    return;

  if ( QgsVariantUtils::isNull( mValue ) )
    mValue = mMin;

  if ( mMin.userType() == QMetaType::Type::Int &&
       mMax.userType() == QMetaType::Type::Int &&
       mStep.userType() == QMetaType::Type::Int &&
       mValue.userType() == QMetaType::Type::Int )
  {
    QDial::setMinimum( mMin.toInt() );
    QDial::setMaximum( mMax.toInt() );
    QDial::setSingleStep( mStep.toInt() );
    QDial::setValue( mValue.toInt() );
  }

  if ( mMin.userType() == QMetaType::Type::Double &&
       mMax.userType() == QMetaType::Type::Double &&
       mStep.userType() == QMetaType::Type::Double &&
       mValue.userType() == QMetaType::Type::Double )
  {
    if ( minimum() != 0 )
      QDial::setMinimum( 0 );

    const int max = std::ceil( ( mMax.toDouble() - mMin.toDouble() ) / mStep.toDouble() );
    if ( maximum() != max )
      QDial::setMaximum( max );

    if ( singleStep() != 1 )
      QDial::setSingleStep( 1 );

    QDial::setValue( std::ceil( ( mValue.toDouble() - mMin.toDouble() ) / mStep.toDouble() ) );
  }

  connect( this, static_cast < void ( QDial::* )( int ) > ( &QDial::valueChanged ), this, &QgsDial::onValueChanged );
}

QVariant QgsDial::variantValue() const
{
  return mValue;
}

void QgsDial::onValueChanged( int value )
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
  else if ( mMin.userType() == QMetaType::Type::Double &&
            mMax.userType() == QMetaType::Type::Double &&
            mStep.userType() == QMetaType::Type::Double &&
            mValue.userType() == QMetaType::Type::Double )
  {
    mValue = QVariant( mMin.toDouble() + value * mStep.toDouble() );
  }
  emit valueChanged( mValue );
}
