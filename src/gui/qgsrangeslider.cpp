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
  : QgsRangeSlider( Qt::Horizontal, parent )
{
}

QgsRangeSlider::QgsRangeSlider( Qt::Orientation orientation, QWidget *parent )
  : QWidget( parent )
{
  mStyleOption.minimum = 0;
  mStyleOption.maximum = 100;
  mStyleOption.orientation = orientation;

  setFocusPolicy( Qt::FocusPolicy( style()->styleHint( QStyle::SH_Button_FocusPolicy ) ) );
  QSizePolicy sp( QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::Slider );
  if ( mStyleOption.orientation == Qt::Vertical )
    sp.transpose();
  setSizePolicy( sp );
  setAttribute( Qt::WA_WState_OwnSizePolicy, false );

  setAttribute( Qt::WA_Hover );
  setMouseTracking( true );
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
  mStyleOption.minimum = std::min( maximum, mStyleOption.minimum );
  emit rangeLimitsChanged( mStyleOption.minimum, mStyleOption.maximum );

  if ( mUpperValue > maximum || mLowerValue > maximum )
  {
    mUpperValue = std::min( mUpperValue, maximum );
    mLowerValue = std::min( mLowerValue, maximum );
    emit rangeChanged( mLowerValue, mUpperValue );
  }

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
  mStyleOption.maximum = std::max( minimum, mStyleOption.maximum );
  emit rangeLimitsChanged( mStyleOption.minimum, mStyleOption.maximum );

  if ( mUpperValue < minimum || mLowerValue < minimum )
  {
    mUpperValue = std::max( mUpperValue, minimum );
    mLowerValue = std::max( mLowerValue, minimum );
    emit rangeChanged( mLowerValue, mUpperValue );
  }

  update();
}

void QgsRangeSlider::setRangeLimits( int minimum, int maximum )
{
  if ( maximum < minimum )
    std::swap( minimum, maximum );

  if ( mStyleOption.minimum == minimum && mStyleOption.maximum == maximum )
    return;

  mStyleOption.minimum = minimum;
  mStyleOption.maximum = maximum;
  emit rangeLimitsChanged( mStyleOption.minimum, mStyleOption.maximum );

  if ( mUpperValue < minimum || mLowerValue < minimum || mUpperValue > maximum || mLowerValue > maximum )
  {
    mUpperValue = std::min( maximum, std::max( mUpperValue, minimum ) );
    mLowerValue = std::min( maximum, std::max( mLowerValue, minimum ) );
    emit rangeChanged( mLowerValue, mUpperValue );
  }

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

  mLowerValue = std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, lowerValue ) );
  if ( mFixedRangeSize >= 0 )
  {
    mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
    mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
  }
  else
  {
    mUpperValue = std::max( mLowerValue, mUpperValue );
  }
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

  mUpperValue = std::max( mStyleOption.minimum, std::min( mStyleOption.maximum, upperValue ) );
  if ( mFixedRangeSize >= 0 )
  {
    mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
    mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
  }
  else
  {
    mLowerValue = std::min( mLowerValue, mUpperValue );
  }

  emit rangeChanged( mLowerValue, mUpperValue );
  update();
}

void QgsRangeSlider::setRange( int lower, int upper )
{
  if ( lower == mLowerValue && upper == mUpperValue )
    return;

  if ( upper < lower )
    std::swap( lower, upper );

  mLowerValue = std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, lower ) );
  mUpperValue = std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, upper ) );
  if ( mFixedRangeSize >= 0 )
  {
    mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
    mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
  }
  else
  {
    mUpperValue = std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, upper ) );
  }
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
  const QRect gr = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderGroove, this );
  const QRect sr = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
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
  if ( mFlipped )
    value = mStyleOption.maximum + mStyleOption.minimum - value;
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

  mStyleOption.sliderPosition = unFlippedSliderPosition( mLowerValue );
  const QRect lowerHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
  mStyleOption.sliderPosition = unFlippedSliderPosition( mUpperValue );
  const QRect upperHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );

  const QRect grooveRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderGroove, this );
  const QRect tickmarksRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderTickmarks, this );
  if ( lowerHandleRect.contains( pos ) )
  {
    mHoverRect = lowerHandleRect;
    mHoverControl = Lower;
    mHoverSubControl = QStyle::SC_SliderHandle;
    setCursor( Qt::OpenHandCursor );
  }
  else if ( upperHandleRect.contains( pos ) )
  {
    mHoverRect = upperHandleRect;
    mHoverControl = Upper;
    mHoverSubControl = QStyle::SC_SliderHandle;
    setCursor( Qt::OpenHandCursor );
  }
  else if ( grooveRect.contains( pos ) )
  {
    mHoverRect = grooveRect;
    mHoverControl = None;
    mHoverSubControl = QStyle::SC_SliderGroove;

    if ( selectedRangeRect().contains( pos ) )
      setCursor( Qt::OpenHandCursor );
    else
      unsetCursor();
  }
  else if ( tickmarksRect.contains( pos ) )
  {
    mHoverRect = tickmarksRect;
    mHoverControl = None;
    mHoverSubControl = QStyle::SC_SliderTickmarks;
    unsetCursor();
  }
  else
  {
    mHoverRect = QRect();
    mHoverControl = None;
    mHoverSubControl = QStyle::SC_None;
    unsetCursor();
  }
  return mHoverSubControl != lastHoverSubControl || mHoverControl != lastHoverControl;
}

QRect QgsRangeSlider::selectedRangeRect()
{
  QRect selectionRect;

  mStyleOption.activeSubControls = mHoverControl == Lower || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = unFlippedSliderPosition( mLowerValue );
  const QRect lowerHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, nullptr );

  mStyleOption.activeSubControls = mHoverControl == Upper || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = unFlippedSliderPosition( mUpperValue );
  const QRect upperHandleRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, nullptr );

  const QRect grooveRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderGroove, nullptr );

  switch ( mStyleOption.orientation )
  {
    case Qt::Horizontal:
      selectionRect = mFlipped ? QRect( upperHandleRect.right(),
                                        grooveRect.y(),
                                        lowerHandleRect.left() - upperHandleRect.right(),
                                        grooveRect.height()
                                      )
                      : QRect( lowerHandleRect.right(),
                               grooveRect.y(),
                               upperHandleRect.left() - lowerHandleRect.right(),
                               grooveRect.height()
                             );
      break;

    case Qt::Vertical:
      selectionRect = mFlipped ? QRect( grooveRect.x(),
                                        lowerHandleRect.top(),
                                        grooveRect.width(),
                                        upperHandleRect.bottom() - lowerHandleRect.top()
                                      )
                      : QRect( grooveRect.x(),
                               upperHandleRect.top(),
                               grooveRect.width(),
                               lowerHandleRect.bottom() - upperHandleRect.top()
                             );
      break;
  }

  return selectionRect.adjusted( -1, 1, 1, -1 );
}

int QgsRangeSlider::fixedRangeSize() const
{
  return mFixedRangeSize;
}

void QgsRangeSlider::setFixedRangeSize( int size )
{
  if ( size == mFixedRangeSize )
    return;

  mFixedRangeSize = size;

  if ( mFixedRangeSize >= 0 )
    setUpperValue( mLowerValue + mFixedRangeSize );

  emit fixedRangeSizeChanged( mFixedRangeSize );
}

void QgsRangeSlider::applyStep( int step )
{
  switch ( mFocusControl )
  {
    case Lower:
    {
      const int newLowerValue = std::min( mUpperValue, std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, mLowerValue + step ) ) );
      if ( newLowerValue != mLowerValue )
      {
        mLowerValue = newLowerValue;
        if ( mFixedRangeSize >= 0 )
        {
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
        }
        emit rangeChanged( mLowerValue, mUpperValue );
        update();
      }
      break;
    }

    case Upper:
    {
      const int newUpperValue = std::max( mLowerValue, std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, mUpperValue + step ) ) );
      if ( newUpperValue != mUpperValue )
      {
        mUpperValue = newUpperValue;
        if ( mFixedRangeSize >= 0 )
        {
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
        }
        emit rangeChanged( mLowerValue, mUpperValue );
        update();
      }
      break;
    }

    case Range:
    {
      if ( step < 0 )
      {
        const int previousWidth = mUpperValue - mLowerValue;
        const int newLowerValue = std::min( mUpperValue, std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, mLowerValue + step ) ) );
        if ( newLowerValue != mLowerValue )
        {
          mLowerValue = newLowerValue;
          if ( mFixedRangeSize >= 0 )
          {
            mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
            mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
          }
          else
          {
            mUpperValue = std::min( mStyleOption.maximum, mLowerValue + previousWidth );
          }
          emit rangeChanged( mLowerValue, mUpperValue );
          update();
        }
      }
      else
      {
        const int previousWidth = mUpperValue - mLowerValue;
        const int newUpperValue = std::max( mLowerValue, std::min( mStyleOption.maximum, std::max( mStyleOption.minimum, mUpperValue + step ) ) );
        if ( newUpperValue != mUpperValue )
        {
          mUpperValue = newUpperValue;
          if ( mFixedRangeSize >= 0 )
          {
            mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
            mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
          }
          else
          {
            mLowerValue = std::max( mStyleOption.minimum, mUpperValue - previousWidth );
          }
          emit rangeChanged( mLowerValue, mUpperValue );
          update();
        }
      }
      break;
    }

    case None:
    case Both:
      break;
  }
}

int QgsRangeSlider::unFlippedSliderPosition( int value ) const
{
  return mFlipped ? mStyleOption.maximum + mStyleOption.minimum - value : value;
}

int QgsRangeSlider::pageStep() const
{
  return mPageStep;
}

void QgsRangeSlider::setPageStep( int step )
{
  mPageStep = step;
}

int QgsRangeSlider::singleStep() const
{
  return mSingleStep;
}

void QgsRangeSlider::setSingleStep( int step )
{
  mSingleStep = step;
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
  if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
  {
    setSizePolicy( sizePolicy().transposed() );
    setAttribute( Qt::WA_WState_OwnSizePolicy, false );
  }
  update();
  updateGeometry();
}

Qt::Orientation QgsRangeSlider::orientation() const
{
  return mStyleOption.orientation;
}

bool QgsRangeSlider::flippedDirection() const
{
  return mFlipped;
}

void QgsRangeSlider::setFlippedDirection( bool flipped )
{
  mFlipped = flipped;
  update();
}

void QgsRangeSlider::paintEvent( QPaintEvent * )
{
  QPainter painter( this );

  mStyleOption.initFrom( this );
  mStyleOption.rect = rect();
  mStyleOption.sliderPosition = mStyleOption.minimum;
  mStyleOption.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;

  mStyleOption.activeSubControls = mHoverSubControl;
  // draw groove
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  QColor color = palette().color( QPalette::Highlight );
  color.setAlpha( 160 );
  painter.setBrush( QBrush( color ) );
  painter.setPen( Qt::NoPen );
  painter.drawRect( selectedRangeRect() );

  // draw first handle
  mStyleOption.subControls = QStyle::SC_SliderHandle;
  mStyleOption.activeSubControls = mHoverControl == Lower || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = unFlippedSliderPosition( mLowerValue );
  if ( mActiveControl == Lower )
    mStyleOption.state |= QStyle::State_Sunken;
  else
    mStyleOption.state &= ~QStyle::State_Sunken;
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  // draw second handle
  mStyleOption.activeSubControls = mHoverControl == Upper || mActiveControl == Lower ? QStyle::SC_SliderHandle : QStyle::SC_None;
  mStyleOption.sliderPosition = unFlippedSliderPosition( mUpperValue );
  if ( mActiveControl == Upper )
    mStyleOption.state |= QStyle::State_Sunken;
  else
    mStyleOption.state &= ~QStyle::State_Sunken;
  style()->drawComplexControl( QStyle::CC_Slider, &mStyleOption, &painter );

  if ( hasFocus() && mFocusControl != None )
  {
    //draw focus rect
    QStyleOptionFocusRect option;
    option.initFrom( this );
    option.state = QStyle::State_KeyboardFocusChange;
    if ( mFocusControl == Lower )
    {
      mStyleOption.sliderPosition = unFlippedSliderPosition( mLowerValue );
      option.rect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
    }
    else if ( mFocusControl == Upper )
    {
      mStyleOption.sliderPosition = unFlippedSliderPosition( mUpperValue );
      option.rect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
    }
    else if ( mFocusControl == Range )
    {
      option.rect = selectedRangeRect();
      if ( mStyleOption.orientation == Qt::Horizontal )
        option.rect = option.rect.adjusted( 0, -1, 0, 1 );
      else
        option.rect = option.rect.adjusted( -1, 0, 1, 0 );
    }
    style()->drawPrimitive( QStyle::PE_FrameFocusRect, &option, &painter );
  }
}

void QgsRangeSlider::mousePressEvent( QMouseEvent *event )
{
  if ( mStyleOption.maximum == mStyleOption.minimum || ( event->buttons() ^ event->button() ) )
  {
    event->ignore();
    return;
  }

  event->accept();

  mStyleOption.sliderPosition = unFlippedSliderPosition( mLowerValue );
  const bool overLowerControl = style()->hitTestComplexControl( QStyle::CC_Slider, &mStyleOption, event->pos(), this ) == QStyle::SC_SliderHandle;
  const QRect lowerSliderRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );
  mStyleOption.sliderPosition = unFlippedSliderPosition( mUpperValue );
  const bool overUpperControl = style()->hitTestComplexControl( QStyle::CC_Slider, &mStyleOption, event->pos(), this ) == QStyle::SC_SliderHandle;
  const QRect upperSliderRect = style()->subControlRect( QStyle::CC_Slider, &mStyleOption, QStyle::SC_SliderHandle, this );

  const bool overSelectedRange = selectedRangeRect().contains( event->pos() );

  mLowerClickOffset = pick( event->pos() - lowerSliderRect.topLeft() );
  mUpperClickOffset = pick( event->pos() - upperSliderRect.topLeft() );

  mPreDragLowerValue = mLowerValue;
  mPreDragUpperValue = mUpperValue;
  mRangeDragOffset = 0;

  if ( ( overLowerControl || overUpperControl ) && event->modifiers() & Qt::ShiftModifier )
  {
    mActiveControl = Range; // shift + drag over handle moves the whole range
    mRangeDragOffset = overUpperControl ? mUpperClickOffset : mLowerClickOffset;
    mFocusControl = overUpperControl ? Upper : Lower;
  }
  else if ( overLowerControl && overUpperControl )
    mActiveControl = Both;
  else if ( overLowerControl )
  {
    mActiveControl = Lower;
    mFocusControl = Lower;
  }
  else if ( overUpperControl )
  {
    mActiveControl = Upper;
    mFocusControl = Upper;
  }
  else if ( overSelectedRange )
  {
    mActiveControl = Range;
    mFocusControl = Range;
  }
  else
    mActiveControl = None;

  if ( mActiveControl != None )
  {
    mStartDragPos = pixelPosToRangeValue( pick( event->pos() ) - mRangeDragOffset );
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
      mFocusControl = Lower;
      if ( mUpperValue != mPreDragUpperValue )
      {
        changed = true;
        mUpperValue = mPreDragUpperValue;
        if ( mFixedRangeSize >= 0 )
        {
          // don't permit fixed width drags if it pushes the other value out of range
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
        }
      }
    }
    else if ( newPosition > mStartDragPos )
    {
      destControl = Upper;
      mFocusControl = Upper;
      if ( mLowerValue != mPreDragLowerValue )
      {
        changed = true;
        mLowerValue = mPreDragLowerValue;
        if ( mFixedRangeSize >= 0 )
        {
          // don't permit fixed width drags if it pushes the other value out of range
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
        }
      }
    }
    else
    {
      destControl = None;
      if ( mUpperValue != mPreDragUpperValue )
      {
        changed = true;
        mUpperValue = mPreDragUpperValue;
        if ( mFixedRangeSize >= 0 )
        {
          // don't permit fixed width drags if it pushes the other value out of range
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
        }
      }
      if ( mLowerValue != mPreDragLowerValue )
      {
        changed = true;
        mLowerValue = mPreDragLowerValue;
        if ( mFixedRangeSize >= 0 )
        {
          // don't permit fixed width drags if it pushes the other value out of range
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
        }
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
        if ( mFixedRangeSize >= 0 )
        {
          // don't permit fixed width drags if it pushes the other value out of range
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
        }

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
        if ( mFixedRangeSize >= 0 )
        {
          // don't permit fixed width drags if it pushes the other value out of range
          mLowerValue = std::max( mStyleOption.minimum, mUpperValue - mFixedRangeSize );
          mUpperValue = std::min( mLowerValue + mFixedRangeSize, mStyleOption.maximum );
        }

        changed = true;
      }
      break;
    }

    case Range:
    {
      newPosition = pixelPosToRangeValue( pick( event->pos() ) - mRangeDragOffset ) ;
      int delta = newPosition - mStartDragPos;

      if ( delta > 0 )
      {
        // move range up
        const int maxDelta = mStyleOption.maximum - mPreDragUpperValue;
        delta = std::min( maxDelta, delta );
        mLowerValue = mPreDragLowerValue + delta;
        mUpperValue = mPreDragUpperValue + delta;
        changed = true;
      }
      else if ( delta < 0 )
      {
        // move range down
        delta = -delta;
        const int maxDelta = mPreDragLowerValue - mStyleOption.minimum ;
        delta = std::min( maxDelta, delta );
        mLowerValue = mPreDragLowerValue - delta;
        mUpperValue = mPreDragUpperValue - delta;
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

void QgsRangeSlider::keyPressEvent( QKeyEvent *event )
{
  Control destControl = mFocusControl;
  if ( ( destControl == Lower || destControl == Upper ) && mLowerValue == mUpperValue )
    destControl = Both; //ambiguous destination, because both sliders are on top of each other

  switch ( event->key() )
  {
    case Qt::Key_Left:
    {
      switch ( mStyleOption.orientation )
      {
        case Qt::Horizontal:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Upper : Lower;

          applyStep( mFlipped ? mSingleStep : -mSingleStep );
          break;

        case Qt::Vertical:
          if ( mFlipped )
          {
            switch ( mFocusControl )
            {
              case Lower:
                mFocusControl = Range;
                break;
              case Range:
                mFocusControl = Upper;
                break;
              case Upper:
              case None:
              case Both:
                mFocusControl = Lower;
                break;
            }
          }
          else
          {
            switch ( mFocusControl )
            {
              case Lower:
              case None:
              case Both:
                mFocusControl = Upper;
                break;
              case Range:
                mFocusControl = Lower;
                break;
              case Upper:
                mFocusControl = Range;
                break;
            }
          }
          update();
          break;
      }
      break;
    }

    case Qt::Key_Right:
    {
      switch ( mStyleOption.orientation )
      {
        case Qt::Horizontal:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Lower : Upper;
          applyStep( mFlipped ? -mSingleStep : mSingleStep );
          break;

        case Qt::Vertical:
          if ( mFlipped )
          {
            switch ( mFocusControl )
            {
              case Lower:
              case None:
              case Both:
                mFocusControl = Upper;
                break;
              case Range:
                mFocusControl = Lower;
                break;
              case Upper:
                mFocusControl = Range;
                break;
            }
          }
          else
          {
            switch ( mFocusControl )
            {
              case Lower:
                mFocusControl = Range;
                break;
              case Range:
                mFocusControl = Upper;
                break;
              case Upper:
              case None:
              case Both:
                mFocusControl = Lower;
                break;
            }
          }
          update();
          break;
      }
      break;
    }

    case Qt::Key_Up:
    {
      switch ( mStyleOption.orientation )
      {
        case Qt::Horizontal:
          if ( mFlipped )
          {
            switch ( mFocusControl )
            {
              case Lower:
                mFocusControl = Range;
                break;
              case Range:
                mFocusControl = Upper;
                break;
              case Upper:
              case None:
              case Both:
                mFocusControl = Lower;
                break;
            }
          }
          else
          {
            switch ( mFocusControl )
            {
              case Lower:
                mFocusControl = Upper;
                break;
              case Range:
              case None:
              case Both:
                mFocusControl = Lower;
                break;
              case Upper:
                mFocusControl = Range;
                break;
            }
          }
          update();
          break;

        case Qt::Vertical:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Upper : Lower;

          applyStep( mFlipped ? mSingleStep : -mSingleStep );
          break;
      }
      break;
    }

    case Qt::Key_Down:
    {
      switch ( mStyleOption.orientation )
      {
        case Qt::Horizontal:
          if ( mFlipped )
          {
            switch ( mFocusControl )
            {
              case Lower:
                mFocusControl = Upper;
                break;
              case Range:
              case None:
              case Both:
                mFocusControl = Lower;
                break;
              case Upper:
                mFocusControl = Range;
                break;
            }
          }
          else
          {
            switch ( mFocusControl )
            {
              case Lower:
                mFocusControl = Range;
                break;
              case Range:
                mFocusControl = Upper;
                break;
              case Upper:
              case None:
              case Both:
                mFocusControl = Lower;
                break;
            }
          }
          update();
          break;

        case Qt::Vertical:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Lower : Upper;

          applyStep( mFlipped ? -mSingleStep : mSingleStep );
          break;
      }
      break;
    }

    case Qt::Key_PageUp:
    {
      switch ( mStyleOption.orientation )
      {
        case Qt::Horizontal:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Lower : Upper;

          applyStep( mFlipped ? -mPageStep : mPageStep );
          break;

        case Qt::Vertical:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Upper : Lower;

          applyStep( mFlipped ? mPageStep : -mPageStep );
          break;
      }
      break;
    }

    case Qt::Key_PageDown:
    {
      switch ( mStyleOption.orientation )
      {
        case Qt::Horizontal:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Upper : Lower;

          applyStep( mFlipped ? mPageStep : -mPageStep );
          break;

        case Qt::Vertical:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Lower : Upper;

          applyStep( mFlipped ? -mPageStep : mPageStep );
          break;
      }
      break;
    }

    case Qt::Key_Home:
      switch ( destControl )
      {
        case Lower:
          applyStep( mFlipped ? mUpperValue - mLowerValue : mStyleOption.minimum - mLowerValue );
          break;

        case Upper:
          applyStep( mFlipped ? mStyleOption.maximum - mUpperValue : mLowerValue - mUpperValue );
          break;

        case Range:
          applyStep( mFlipped ? mStyleOption.maximum - mUpperValue : mStyleOption.minimum  - mLowerValue );
          break;

        case Both:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Upper : Lower;

          applyStep( mFlipped ? mStyleOption.maximum - mUpperValue : mStyleOption.minimum - mLowerValue );
          break;

        case None:
          break;
      }

      break;

    case Qt::Key_End:
      switch ( destControl )
      {
        case Lower:
          applyStep( mFlipped ? mStyleOption.minimum - mLowerValue : mUpperValue - mLowerValue );
          break;

        case Upper:
          applyStep( mFlipped ? mLowerValue - mUpperValue : mStyleOption.maximum - mUpperValue );
          break;

        case Range:
          applyStep( mFlipped ? mStyleOption.minimum  - mLowerValue : mStyleOption.maximum - mUpperValue );
          break;

        case Both:
          if ( destControl == Both )
            mFocusControl = mFlipped ? Lower : Upper;

          applyStep( mFlipped ? mStyleOption.minimum - mLowerValue : mStyleOption.maximum - mUpperValue );
          break;

        case None:
          break;
      }
      break;

    default:
      event->ignore();
      break;
  }
}

QSize QgsRangeSlider::sizeHint() const
{
  ensurePolished();

  // these hardcoded magic values look like a hack, but they are taken straight from the Qt QSlider widget code!
  static constexpr int SLIDER_LENGTH = 84;
  static constexpr int TICK_SPACE = 5;

  int thick = style()->pixelMetric( QStyle::PM_SliderThickness, &mStyleOption, this );
  if ( mStyleOption.tickPosition & QSlider::TicksAbove )
    thick += TICK_SPACE;
  if ( mStyleOption.tickPosition & QSlider::TicksBelow )
    thick += TICK_SPACE;
  int w = thick, h = SLIDER_LENGTH;
  if ( mStyleOption.orientation == Qt::Horizontal )
  {
    std::swap( w, h );
  }
  return style()->sizeFromContents( QStyle::CT_Slider, &mStyleOption, QSize( w, h ), this );
}

QSize QgsRangeSlider::minimumSizeHint() const
{
  QSize s = sizeHint();
  const int length = style()->pixelMetric( QStyle::PM_SliderLength, &mStyleOption, this );
  if ( mStyleOption.orientation == Qt::Horizontal )
    s.setWidth( length );
  else
    s.setHeight( length );
  return s;
}


