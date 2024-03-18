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
#include "qgsdoublespinbox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QPainterPath>
#include <QLabel>

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

  QgsElevationControllerSettingsAction *settingsAction = new QgsElevationControllerSettingsAction( mMenu );
  mMenu->addAction( settingsAction );

  settingsAction->sizeSpin()->clear();
  connect( settingsAction->sizeSpin(), qOverload< double >( &QgsDoubleSpinBox::valueChanged ), this, [this]( double size )
  {
    setFixedRangeSize( size < 0 ? -1 : size );
  } );

  mMenu->addSeparator();

  mSlider = new QgsRangeSlider( Qt::Vertical );
  mSlider->setFlippedDirection( true );
  mSlider->setRangeLimits( 0, 100000 );
  mSliderLabels = new QgsElevationControllerLabels();

  QHBoxLayout *hlSlider = new QHBoxLayout();
  hlSlider->setContentsMargins( 0, 0, 0, 0 );
  hlSlider->setSpacing( 2 );
  hlSlider->addWidget( mSlider );
  hlSlider->addWidget( mSliderLabels, 1 );
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
    mSliderLabels->setRange( range() );
  } );

  // default initial value to full range
  setRange( rangeLimits() );
  mSliderLabels->setRange( rangeLimits() );
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

  const QgsDoubleRange sliderRange( mSlider->lowerValue() / mSliderPrecision, mSlider->upperValue() / mSliderPrecision );
  if ( mFixedRangeSize >= 0 )
  {
    // adjust range so that it has exactly the fixed width (given slider int precision the slider range
    // will not have the exact fixed width)
    if ( sliderRange.upper() + mFixedRangeSize <= mRangeLimits.upper() )
      return QgsDoubleRange( sliderRange.lower(), sliderRange.lower() + mFixedRangeSize );
    else
      return QgsDoubleRange( sliderRange.upper() - mFixedRangeSize, sliderRange.upper() );
  }
  else
  {
    return sliderRange;
  }
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

  mSliderLabels->setRange( mCurrentRange );
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

  mSliderLabels->setLimits( mRangeLimits );
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

double QgsElevationControllerWidget::fixedRangeSize() const
{
  return mFixedRangeSize;
}

void QgsElevationControllerWidget::setFixedRangeSize( double size )
{
  if ( size == mFixedRangeSize )
    return;

  mFixedRangeSize = size;
  if ( mFixedRangeSize < 0 )
  {
    mSlider->setFixedRangeSize( -1 );
  }
  else
  {
    mSlider->setFixedRangeSize( static_cast< int >( std::round( mFixedRangeSize * mSliderPrecision ) ) );
  }
}

//
// QgsElevationControllerLabels
//
///@cond PRIVATE
QgsElevationControllerLabels::QgsElevationControllerLabels( QWidget *parent )
  : QWidget( parent )
{
  // Drop the default widget font size by a couple of points
  QFont smallerFont = font();
  int fontSize = smallerFont.pointSize();
#ifdef Q_OS_WIN
  fontSize = std::max( fontSize - 1, 8 ); // bit less on windows, due to poor rendering of small point sizes
#else
  fontSize = std::max( fontSize - 2, 7 );
#endif
  smallerFont.setPointSize( fontSize );
  setFont( smallerFont );

  const QFontMetrics fm( smallerFont );
  setMinimumWidth( fm.horizontalAdvance( '0' ) * 5 );
  setAttribute( Qt::WA_TransparentForMouseEvents );
}

void QgsElevationControllerLabels::paintEvent( QPaintEvent * )
{
  QStyleOptionSlider styleOption;
  styleOption.initFrom( this );

  const QRect sliderRect = style()->subControlRect( QStyle::CC_Slider, &styleOption,  QStyle::SC_SliderHandle, this );
  const int sliderHeight = sliderRect.height();

  QFont f = font();
  const QFontMetrics fm( f );

  const int left = rect().left() + 2;

  const double limitRange = mLimits.upper() - mLimits.lower();
  const double lowerFraction = ( mRange.lower() - mLimits.lower() ) / limitRange;
  const double upperFraction = ( mRange.upper() - mLimits.lower() ) / limitRange;
  const int lowerY = std::min( static_cast< int >( std::round( rect().bottom() - sliderHeight * 0.5 - ( rect().height() - sliderHeight ) * lowerFraction + fm.ascent() ) ),
                               rect().bottom() - fm.descent() );
  const int upperY = std::max( static_cast< int >( std::round( rect().bottom() - sliderHeight * 0.5 - ( rect().height() - sliderHeight ) * upperFraction - fm.descent() ) ),
                               rect().top() + fm.ascent() );

  const bool lowerIsCloseToLimit = lowerY + fm.height() > rect().bottom() - fm.descent();
  const bool upperIsCloseToLimit = upperY - fm.height() < rect().top() + fm.ascent();
  const bool lowerIsCloseToUpperLimit = lowerY - fm.height() < rect().top() + fm.ascent();

  QLocale locale;

  QPainterPath path;
  if ( mLimits.lower() > std::numeric_limits< double >::lowest() )
  {
    if ( lowerIsCloseToLimit )
    {
      f.setBold( true );
      path.addText( left, lowerY, f, locale.toString( mRange.lower() ) );
    }
    else
    {
      f.setBold( true );
      path.addText( left, lowerY, f, locale.toString( mRange.lower() ) );
      f.setBold( false );
      path.addText( left, rect().bottom() - fm.descent(), f, locale.toString( mLimits.lower() ) );
    }
  }

  if ( mLimits.upper() < std::numeric_limits< double >::max() )
  {
    if ( qgsDoubleNear( mRange.upper(), mRange.lower() ) )
    {
      if ( !lowerIsCloseToUpperLimit )
      {
        f.setBold( false );
        path.addText( left, rect().top() + fm.ascent(), f, locale.toString( mLimits.upper() ) );
      }
    }
    else
    {
      if ( upperIsCloseToLimit )
      {
        f.setBold( true );
        path.addText( left, upperY, f, locale.toString( mRange.upper() ) );
      }
      else
      {
        f.setBold( true );
        path.addText( left, upperY, f, locale.toString( mRange.upper() ) );
        f.setBold( false );
        path.addText( left, rect().top() + fm.ascent(), f, locale.toString( mLimits.upper() ) );
      }
    }
  }

  QPainter p( this );
  p.setRenderHint( QPainter::Antialiasing, true );
  const QColor bufferColor = palette().color( QPalette::Window );
  const QColor textColor = palette().color( QPalette::WindowText );
  QPen pen( bufferColor );
  pen.setJoinStyle( Qt::RoundJoin );
  pen.setCapStyle( Qt::RoundCap );
  pen.setWidthF( 4 );
  p.setPen( pen );
  p.setBrush( Qt::NoBrush );
  p.drawPath( path );
  p.setPen( Qt::NoPen );
  p.setBrush( QBrush( textColor ) );
  p.drawPath( path );
  p.end();
}

void QgsElevationControllerLabels::setLimits( const QgsDoubleRange &limits )
{
  if ( limits == mLimits )
    return;

  const QFontMetrics fm( font() );
  const int maxChars = std::max( QLocale().toString( std::floor( limits.lower() ) ).length(),
                                 QLocale().toString( std::floor( limits.upper() ) ).length() ) + 3;
  setMinimumWidth( fm.horizontalAdvance( '0' ) * maxChars );

  mLimits = limits;
  update();
}

void QgsElevationControllerLabels::setRange( const QgsDoubleRange &range )
{
  if ( range == mRange )
    return;

  mRange = range;
  update();
}

//
// QgsElevationControllerSettingsAction
//

QgsElevationControllerSettingsAction::QgsElevationControllerSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  QLabel *label = new QLabel( tr( "Fixed Range Size" ) );
  gLayout->addWidget( label, 0, 0 );

  mSizeSpin = new QgsDoubleSpinBox();
  mSizeSpin->setDecimals( 4 );
  mSizeSpin->setMinimum( -1.0 );
  mSizeSpin->setMaximum( 999999999.0 );
  mSizeSpin->setClearValue( -1, tr( "Not set" ) );
  mSizeSpin->setKeyboardTracking( false );
  mSizeSpin->setToolTip( tr( "Limit elevation range to a fixed size" ) );

  gLayout->addWidget( mSizeSpin, 0, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}

QgsDoubleSpinBox *QgsElevationControllerSettingsAction::sizeSpin()
{
  return mSizeSpin;
}

///@endcond PRIVATE
