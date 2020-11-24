/***************************************************************************
    qgsrangeslider.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrangeslider.h"
#include <QPainter>
#include <QMouseEvent>

QgsRangeSlider::QgsRangeSlider( QWidget *parent )
  : QWidget( parent )
{
  mStyleOption.minimum = 0;
  mStyleOption.maximum = 100;
}

int QgsRangeSlider::maximum() const
{
  return mStyleOption.maximum;
}

void QgsRangeSlider::setMaximum( int maximum )
{
  if ( mStyleOption.maximum == maximum )
    return;

  mStyleOption.maximum = maximum;
  emit rangeLimitsChanged( mStyleOption.minimum, mStyleOption.maximum );
  update();
}

int QgsRangeSlider::minimum() const
{
  return mStyleOption.minimum;
}

void QgsRangeSlider::setMinimum( int minimum )
{
  if ( mStyleOption.minimum == minimum )
    return;
  mStyleOption.minimum = minimum;
  emit rangeLimitsChanged( mStyleOption.minimum, mStyleOption.maximum );
  update();
}

void QgsRangeSlider::setRangeLimits( int minimum, int maximum )
{
  if ( mStyleOption.minimum == minimum && mStyleOption.maximum == maximum )
    return;

  mStyleOption.minimum = minimum;
  mStyleOption.maximum = maximum;
  emit rangeLimitsChanged( mStyleOption.minimum, mStyleOption.maximum );
  update();
}

int QgsRangeSlider::lowerValue() const
{
  return mLowerValue;
}

void QgsRangeSlider::setLowerValue( int lowerValue )
{
  if ( lowerValue == mLowerValue )
    return;

  mLowerValue = lowerValue;
  emit rangeChanged( mLowerValue, mUpperValue );
  update();
}

int QgsRangeSlider::upperValue() const
{
  return mUpperValue;
}


void QgsRangeSlider::setUpperValue( int upperValue )
{
  if ( upperValue == mUpperValue )
    return;

  mUpperValue = upperValue;
  emit rangeChanged( mLowerValue, mUpperValue );
  update();
}

void QgsRangeSlider::setRange( int lower, int upper )
{
  if ( lower == mLowerValue && upper == mUpperValue )
    return;

  mLowerValue = lower;
  mUpperValue = upper;
  emit rangeChanged( mLowerValue, mUpperValue );
  update();
}

void QgsRangeSlider::setTickPosition( QSlider::TickPosition position )
{
  mStyleOption.tickPosition = position;
  update();
}

QSlider::TickPosition QgsRangeSlider::tickPosition() const
{
  return mStyleOption.tickPosition;
}

void QgsRangeSlider::setTickInterval( int interval )
{
  mStyleOption.tickInterval = interval;
  update();
}

int QgsRangeSlider::tickInterval() const
{
  return mStyleOption.tickInterval;
}

void QgsRangeSlider::setOrientation( Qt::Orientation orientation )
{
  mStyleOption.orientation = orientation;
  update();
}

Qt::Orientation QgsRangeSlider::orientation() const
{
  return mStyleOption.orientation;
}

bool QgsRangeSlider::invertedAppearance() const
{
  return mInverted;
}

void QgsRangeSlider::setInvertedAppearance( bool inverted )
{
  mInverted = inverted;
  update();
}

void QgsRangeSlider::paintEvent( QPaintEvent * )
{
  QPainter painter( this );

  mStyleOption.initFrom( this );
  mStyleOption.rect = rect();
  mStyleOption.sliderPosition = 0;
  mStyleOption.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;

  // draw groove
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  QColor color = palette().color( QPalette::Highlight );
  color.setAlpha( 160 );
  painter.setBrush( QBrush( color ) );
  painter.setPen( Qt::NoPen );

  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  const QRect lowerHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, nullptr );

  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mUpperValue : mUpperValue;
  const QRect upperHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, nullptr );

  const QRect grooveRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderGroove, nullptr );

  QRect selectionRect;
  switch ( mStyleOption.orientation )
  {
    case Qt::Horizontal:
      selectionRect = mInverted ? QRect( upperHandleRect.left(),
                                         grooveRect.y(),
                                         lowerHandleRect.right() - upperHandleRect.right(),
                                         grooveRect.height()
                                       )
                      : QRect( lowerHandleRect.right(),
                               grooveRect.y(),
                               upperHandleRect.left() - lowerHandleRect.right(),
                               grooveRect.height()
                             );
      break;

    case Qt::Vertical:
      selectionRect = mInverted ? QRect( grooveRect.x(),
                                         lowerHandleRect.bottom(),
                                         grooveRect.width(),
                                         upperHandleRect.top() - lowerHandleRect.top()
                                       )
                      : QRect( grooveRect.x(),
                               upperHandleRect.top(),
                               grooveRect.width(),
                               lowerHandleRect.bottom() - upperHandleRect.top()
                             );
      break;
  }

  painter.drawRect( selectionRect.adjusted( -1, 1, 1, -1 ) );

  // draw first handle
  mStyleOption.subControls = QStyle::SC_SliderHandle;
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  // draw second handle
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mUpperValue : mUpperValue;
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );
}

void QgsRangeSlider::mousePressEvent( QMouseEvent *event )
{
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  mLowerControl = style()->hitTestComplexControl( QStyle::CC_Slider, &mStyleOption, event->pos(), this );

  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mUpperValue : mUpperValue;
  mUpperControl = style()->hitTestComplexControl( QStyle::CC_Slider, &mStyleOption, event->pos(), this );
}

void QgsRangeSlider::mouseMoveEvent( QMouseEvent *event )
{
  const int distance = mStyleOption.maximum - mStyleOption.minimum;

  int pos = style()->sliderValueFromPosition( 0, distance,
            mStyleOption.orientation == Qt::Horizontal ? event->pos().x() : event->pos().y(),
            mStyleOption.orientation == Qt::Horizontal ? rect().width() : rect().height() );

  if ( mInverted )
    pos = mStyleOption.maximum - pos;

  bool changed = false;
  bool overLowerHandle = false;
  if ( mLowerControl == QStyle::SC_SliderHandle )
  {
    if ( pos <= mUpperValue )
    {
      overLowerHandle = true;
      if ( mLowerValue != pos )
      {
        mLowerValue = pos;
        changed = true;
      }
    }
  }

  if ( !overLowerHandle && mUpperControl == QStyle::SC_SliderHandle )
  {
    if ( pos >= mLowerValue )
    {
      if ( mUpperValue != pos )
      {
        mUpperValue = pos;
        changed = true;
      }
    }
  }

  if ( changed )
  {
    update();
    emit rangeChanged( mLowerValue, mUpperValue );
  }
}

QSize QgsRangeSlider::sizeHint() const
{
  static constexpr int sliderLength = 84;
  static constexpr int tickSpace = 5;

  int w = sliderLength;
  int h = style()->pixelMetric( QStyle::PM_SliderThickness, &mStyleOption, this );

  if ( mStyleOption.tickPosition & QSlider::TicksAbove || mStyleOption.tickPosition & QSlider::TicksBelow )
  {
    h += tickSpace;
  }

  return style()->sizeFromContents( QStyle::CT_Slider, &mStyleOption, QSize( w, h ), this );
}

