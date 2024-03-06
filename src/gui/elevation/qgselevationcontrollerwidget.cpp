/***************************************************************************
                          qgselevationcontrollerwidget.cpp
                          ---------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationcontrollerwidget.h"
#include "qgsrangeslider.h"
#include "qgsrange.h"
#include "qgsproject.h"
#include "qgsprojectelevationproperties.h"
#include "qgsapplication.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>

QgsElevationControllerWidget::QgsElevationControllerWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );

  mConfigureButton = new QToolButton();
  mConfigureButton->setPopupMode( QToolButton::InstantPopup );
  mConfigureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/settings.svg" ) ) );
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );
  hl->addWidget( mConfigureButton );
  hl->addStretch();
  vl->addLayout( hl );
  mMenu = new QMenu( this );
  mConfigureButton->setMenu( mMenu );

  mSlider = new QgsRangeSlider( Qt::Vertical );
  mSlider->setFlippedDirection( true );
  mSlider->setRangeLimits( 0, 100000 );
  QHBoxLayout *hlSlider = new QHBoxLayout();
  hlSlider->setContentsMargins( 0, 0, 0, 0 );
  hlSlider->addWidget( mSlider );
  hlSlider->addStretch();
  vl->addLayout( hlSlider );

  setCursor( Qt::ArrowCursor );

  setLayout( vl );
  updateWidgetMask();

  const QgsDoubleRange projectRange = QgsProject::instance()->elevationProperties()->elevationRange();
  // if project doesn't have a range, just default to ANY range!
  setRangeLimits( projectRange.isInfinite() ? QgsDoubleRange( 0, 100 ) : projectRange );
  connect( QgsProject::instance()->elevationProperties(), &QgsProjectElevationProperties::elevationRangeChanged, this, [this]( const QgsDoubleRange & range )
  {
    if ( !range.isInfinite() )
      setRangeLimits( range );
  } );

  connect( mSlider, &QgsRangeSlider::rangeChanged, this, [this]( int, int )
  {
    if ( mBlockSliderChanges )
      return;

    emit rangeChanged( range() );
  } );

  // default initial value to full range
  setRange( rangeLimits() );
}

void QgsElevationControllerWidget::resizeEvent( QResizeEvent *event )
{
  QWidget::resizeEvent( event );
  updateWidgetMask();
}

QgsDoubleRange QgsElevationControllerWidget::range() const
{
  // if the current slider range is just the current range, but snapped to the slider precision, then losslessly return the current range
  const int snappedLower = static_cast< int >( std::floor( mCurrentRange.lower() * mSliderPrecision ) );
  const int snappedUpper = static_cast< int >( std::ceil( mCurrentRange.upper() * mSliderPrecision ) );
  if ( snappedLower == mSlider->lowerValue() && snappedUpper == mSlider->upperValue() )
    return mCurrentRange;

  return QgsDoubleRange( mSlider->lowerValue() / mSliderPrecision, mSlider->upperValue() / mSliderPrecision );
}

QgsDoubleRange QgsElevationControllerWidget::rangeLimits() const
{
  return mRangeLimits;
}

QgsRangeSlider *QgsElevationControllerWidget::slider()
{
  return mSlider;
}

QMenu *QgsElevationControllerWidget::menu()
{
  return mMenu;
}

void QgsElevationControllerWidget::setRange( const QgsDoubleRange &range )
{
  if ( range == mCurrentRange )
    return;

  mCurrentRange = range;
  mBlockSliderChanges = true;
  mSlider->setRange( static_cast< int >( std::floor( range.lower() * mSliderPrecision ) ),
                     static_cast< int >( std::ceil( range.upper() * mSliderPrecision ) ) );
  mBlockSliderChanges = false;
  emit rangeChanged( range );
}

void QgsElevationControllerWidget::setRangeLimits( const QgsDoubleRange &limits )
{
  if ( limits.isInfinite() )
    return;

  mRangeLimits = limits;

  const double limitRange = limits.upper() - limits.lower();

  // pick a reasonable slider precision, given that the slider operates in integer values only
  mSliderPrecision = std::max( 1000, mSlider->height() ) / limitRange;

  mBlockSliderChanges = true;
  mSlider->setRangeLimits( static_cast< int >( std::floor( limits.lower() * mSliderPrecision ) ),
                           static_cast< int >( std::ceil( limits.upper() * mSliderPrecision ) ) );

  // clip current range to fit limits
  const double newCurrentLower = std::max( mCurrentRange.lower(), limits.lower() );
  const double newCurrentUpper = std::min( mCurrentRange.upper(), limits.upper() );
  const bool rangeHasChanged = newCurrentLower != mCurrentRange.lower() || newCurrentUpper != mCurrentRange.upper();

  mSlider->setRange( static_cast< int >( std::floor( newCurrentLower * mSliderPrecision ) ),
                     static_cast< int >( std::ceil( newCurrentUpper * mSliderPrecision ) ) );
  mCurrentRange = QgsDoubleRange( newCurrentLower, newCurrentUpper );
  mBlockSliderChanges = false;
  if ( rangeHasChanged )
    emit rangeChanged( mCurrentRange );
}

void QgsElevationControllerWidget::updateWidgetMask()
{
  // we want mouse events from this widgets children to be caught, but events
  // on the widget itself to be ignored and passed to underlying widgets which are NOT THE DIRECT
  // PARENT of this widget.
  // this is definitively *****NOT***** possible with event filters, by overriding mouse events, or
  // with the WA_TransparentForMouseEvents attribute

  QRegion reg( frameGeometry() );
  reg -= QRegion( geometry() );
  reg += childrenRegion();
  setMask( reg );
}
