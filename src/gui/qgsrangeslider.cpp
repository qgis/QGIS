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

  setFocusPolicy( Qt::FocusPolicy( style()->styleHint( QStyle::SH_Button_FocusPolicy ) ) );
  QSizePolicy sp( QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::Slider );
  if ( mStyleOption.orientation == Qt::Vertical )
    sp.transpose();
  setSizePolicy( sp );
  setAttribute( Qt::WA_WState_OwnSizePolicy, false );

  installEventFilter( this );
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

bool QgsRangeSlider::event( QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
      if ( const QHoverEvent *he = static_cast<const QHoverEvent *>( event ) )
        updateHoverControl( he->pos() );
      break;
    default:
      break;
  }
  return QWidget::event( event );
}

int QgsRangeSlider::pick( const QPoint &pt ) const
{
  return mStyleOption.orientation == Qt::Horizontal ? pt.x() : pt.y();
}

int QgsRangeSlider::pixelPosToRangeValue( int pos ) const
{
  QRect gr = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderGroove, this );
  QRect sr = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
  int sliderMin, sliderMax, sliderLength;
  if ( mStyleOption.orientation == Qt::Horizontal )
  {
    sliderLength = sr.width();
    sliderMin = gr.x();
    sliderMax = gr.right() - sliderLength + 1;
  }
  else
  {
    sliderLength = sr.height();
    sliderMin = gr.y();
    sliderMax = gr.bottom() - sliderLength + 1;
  }

  int value = QStyle::sliderValueFromPosition( mStyleOption.minimum, mStyleOption.maximum, pos - sliderMin,
              sliderMax - sliderMin );
  if ( mInverted )
    value = mStyleOption.maximum - value;
  return value;
}

bool QgsRangeSlider::updateHoverControl( const QPoint &pos )
{
  const QRect lastHoverRect = mHoverRect;
  const bool doesHover = testAttribute( Qt::WA_Hover );
  if ( doesHover && newHoverControl( pos ) )
  {
    update( lastHoverRect );
    update( mHoverRect );
    return true;
  }
  return !doesHover;
}

bool QgsRangeSlider::newHoverControl( const QPoint &pos )
{
  const Control lastHoverControl = mHoverControl;
  const QStyle::SubControl lastHoverSubControl = mHoverSubControl;

  mStyleOption.subControls = QStyle::SC_All;

  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  QRect lowerHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mUpperValue : mUpperValue;
  QRect upperHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );

  QRect grooveRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderGroove, this );
  QRect tickmarksRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderTickmarks, this );
  if ( lowerHandleRect.contains( pos ) )
  {
    mHoverRect = lowerHandleRect;
    mHoverControl = Lower;
    mHoverSubControl = QStyle::SC_SliderHandle;
  }
  else if ( upperHandleRect.contains( pos ) )
  {
    mHoverRect = upperHandleRect;
    mHoverControl = Upper;
    mHoverSubControl = QStyle::SC_SliderHandle;
  }
  else if ( grooveRect.contains( pos ) )
  {
    mHoverRect = grooveRect;
    mHoverControl = None;
    mHoverSubControl = QStyle::SC_SliderGroove;
  }
  else if ( tickmarksRect.contains( pos ) )
  {
    mHoverRect = tickmarksRect;
    mHoverControl = None;
    mHoverSubControl = QStyle::SC_SliderTickmarks;
  }
  else
  {
    mHoverRect = QRect();
    mHoverControl = None;
    mHoverSubControl = QStyle::SC_None;
  }
  return mHoverSubControl != lastHoverSubControl || mHoverControl != lastHoverControl;
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

  mStyleOption.activeSubControls = mHoverSubControl;
  // draw groove
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  QColor color = palette().color( QPalette::Highlight );
  color.setAlpha( 160 );
  painter.setBrush( QBrush( color ) );
  painter.setPen( Qt::NoPen );

  mStyleOption.activeSubControls = mHoverControl == Lower || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  const QRect lowerHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, nullptr );

  mStyleOption.activeSubControls = mHoverControl == Upper || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
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
  mStyleOption.activeSubControls = mHoverControl == Lower || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  if ( mActiveControl == Lower )
    mStyleOption.state |= QStyle::State_Sunken;
  else
    mStyleOption.state &= ~QStyle::State_Sunken;
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  // draw second handle
  mStyleOption.activeSubControls = mHoverControl == Upper || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mUpperValue : mUpperValue;
  if ( mActiveControl == Upper )
    mStyleOption.state |= QStyle::State_Sunken;
  else
    mStyleOption.state &= ~QStyle::State_Sunken;
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );
}

void QgsRangeSlider::mousePressEvent( QMouseEvent *event )
{
  if ( mStyleOption.maximum == mStyleOption.minimum || ( event->buttons() ^ event->button() ) )
  {
    event->ignore();
    return;
  }

  event->accept();

  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mLowerValue : mLowerValue;
  const bool overLowerControl = style()->hitTestComplexControl( QStyle::CC_Slider, &mStyleOption, event->pos(), this ) == QStyle::SC_SliderHandle;
  const QRect lowerSliderRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
  mStyleOption.sliderPosition = mInverted ? mStyleOption.maximum - mUpperValue : mUpperValue;
  const bool overUpperControl = style()->hitTestComplexControl( QStyle::CC_Slider, &mStyleOption, event->pos(), this ) == QStyle::SC_SliderHandle;
  const QRect upperSliderRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );

  mLowerClickOffset = pick( event->pos() - lowerSliderRect.topLeft() );
  mUpperClickOffset = pick( event->pos() - upperSliderRect.topLeft() );

  if ( overLowerControl && overUpperControl )
  {
    mActiveControl = Both;
    mPreDragLowerValue = mLowerValue;
    mPreDragUpperValue = mUpperValue;
  }
  else if ( overLowerControl )
    mActiveControl = Lower;
  else if ( overUpperControl )
    mActiveControl = Upper;
  else
    mActiveControl = None;

  if ( mActiveControl != None )
  {
    mStartDragPos = pixelPosToRangeValue( pick( event->pos() ) );
  }
}

void QgsRangeSlider::mouseMoveEvent( QMouseEvent *event )
{
  if ( mActiveControl == None )
  {
    event->ignore();
    return;
  }

  event->accept();

  int newPosition = pixelPosToRangeValue( pick( event->pos() ) );

  bool changed = false;
  Control destControl = mActiveControl;
  if ( destControl == Both )
  {
    // if click was over both handles, then the direction of the drag changes which control is affected
    if ( newPosition < mStartDragPos )
    {
      destControl = Lower;
      if ( mUpperValue != mPreDragUpperValue )
      {
        changed = true;
        mUpperValue = mPreDragUpperValue;
      }
    }
    else if ( newPosition > mStartDragPos )
    {
      destControl = Upper;
      if ( mLowerValue != mPreDragLowerValue )
      {
        changed = true;
        mLowerValue = mPreDragLowerValue;
      }
    }
    else
    {
      destControl = None;
      if ( mUpperValue != mPreDragUpperValue )
      {
        changed = true;
        mUpperValue = mPreDragUpperValue;
      }
      if ( mLowerValue != mPreDragLowerValue )
      {
        changed = true;
        mLowerValue = mPreDragLowerValue;
      }
    }
  }

  switch ( destControl )
  {
    case None:
    case Both:
      break;

    case Lower:
    {
      // adjust value to account for lower handle click offset
      newPosition = std::min( mUpperValue, pixelPosToRangeValue( pick( event->pos() ) - mLowerClickOffset ) );
      if ( mLowerValue != newPosition )
      {
        mLowerValue = newPosition;
        changed = true;
      }
      break;
    }

    case Upper:
    {
      // adjust value to account for upper handle click offset
      newPosition = std::max( mLowerValue, pixelPosToRangeValue( pick( event->pos() ) - mUpperClickOffset ) );
      if ( mUpperValue != newPosition )
      {
        mUpperValue = newPosition;
        changed = true;
      }
      break;
    }
  }

  if ( changed )
  {
    update();
    emit rangeChanged( mLowerValue, mUpperValue );
  }
}

void QgsRangeSlider::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mActiveControl == None || event->buttons() )
  {
    event->ignore();
    return;
  }

  event->accept();
  mActiveControl = None;
  update();
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

