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

#include "qgsapplication.h"
#include "qgsdoublespinbox.h"
#include "qgselevationutils.h"
#include "qgsguiutils.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmathutils.h"
#include "qgsproject.h"
#include "qgsprojectelevationproperties.h"
#include "qgsrange.h"
#include "qgsrangeslider.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainterPath>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>

#include "moc_qgselevationcontrollerwidget.cpp"

using namespace Qt::StringLiterals;

QgsElevationControllerWidget::QgsElevationControllerWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );

  mConfigureButton = new QToolButton();
  mConfigureButton->setPopupMode( QToolButton::InstantPopup );
  mConfigureButton->setIcon( QgsApplication::getThemeIcon( u"/propertyicons/settings.svg"_s ) );
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );
  hl->addWidget( mConfigureButton );
  hl->addStretch();
  vl->addLayout( hl );
  mMenu = new QMenu( this );
  mConfigureButton->setMenu( mMenu );

  mSettingsAction = new QgsElevationControllerSettingsAction( mMenu );
  mMenu->addAction( mSettingsAction );

  mInvertDirectionAction = new QAction( tr( "Invert Direction" ), this );
  mInvertDirectionAction->setCheckable( true );
  mMenu->addAction( mInvertDirectionAction );

  QMenu *limitsMenu = mSettingsAction->limitsMenu();

  QAction *limitsFromProjectRangeAction = limitsMenu->addAction( tr( "Project Elevation Range" ) );
  connect( limitsFromProjectRangeAction, &QAction::triggered, this, [this] {
    const QgsDoubleRange range = QgsProject::instance()->elevationProperties()->elevationRange();
    if ( range.isInfinite() )
      return;

    // an explicitly configured range is applied as it is
    setRangeLimits( range );
    setRange( range );
  } );

  QAction *limitsFromProjectLayersAction = limitsMenu->addAction( tr( "Project Layers" ) );
  connect( limitsFromProjectLayersAction, &QAction::triggered, this, [this] { setLimitsFromLayers( QgsProject::instance()->mapLayers().values() ); } );

  QAction *limitsFromCurrentLayerAction = limitsMenu->addAction( tr( "Current Layer" ) );
  connect( limitsFromCurrentLayerAction, &QAction::triggered, this, [this] {
    if ( mMapCanvas )
      setLimitsFromLayers( { mMapCanvas->currentLayer() } );
  } );

  // the project and the current layer change at any time, so refresh availability when the menu opens
  connect( limitsMenu, &QMenu::aboutToShow, this, [this, limitsFromProjectRangeAction, limitsFromProjectLayersAction, limitsFromCurrentLayerAction] {
    limitsFromProjectRangeAction->setEnabled( !QgsProject::instance()->elevationProperties()->elevationRange().isInfinite() );

    const QMap<QString, QgsMapLayer *> projectLayers = QgsProject::instance()->mapLayers();
    limitsFromProjectLayersAction->setEnabled( std::any_of( projectLayers.begin(), projectLayers.end(), []( QgsMapLayer *layer ) { return layerHasElevation( layer ); } ) );

    limitsFromCurrentLayerAction->setEnabled( mMapCanvas && layerHasElevation( mMapCanvas->currentLayer() ) );
  } );

  connect( mSettingsAction, &QgsElevationControllerSettingsAction::limitsChanged, this, &QgsElevationControllerWidget::setRangeLimits );
  connect( mSettingsAction, &QgsElevationControllerSettingsAction::fixedRangeSizeChanged, this, &QgsElevationControllerWidget::setFixedRangeSize );
  connect( mSettingsAction, &QgsElevationControllerSettingsAction::fullRangeRequested, this, [this] {
    setRange( rangeLimits() );
    mSettingsAction->updateRangeSize( rangeLimits().upper() - rangeLimits().lower() );
  } );

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
  connect( QgsProject::instance()->elevationProperties(), &QgsProjectElevationProperties::elevationRangeChanged, this, [this]( const QgsDoubleRange &range ) {
    if ( !range.isInfinite() )
      setRangeLimits( range );
  } );

  connect( mSlider, &QgsRangeSlider::rangeChanged, this, [this]( int, int ) {
    if ( mBlockSliderChanges )
      return;

    const QgsDoubleRange snapped = snappedRange( sliderRange() );

    // a drag within one snapping interval leaves the snapped range identical to the current
    // one, but the handles still have to be moved back onto it, hence before the early return
    mBlockSliderChanges++;
    mSlider->setRange( static_cast<int>( std::floor( snapped.lower() * mSliderPrecision ) ), static_cast<int>( std::ceil( snapped.upper() * mSliderPrecision ) ) );
    mBlockSliderChanges--;

    if ( snapped == mCurrentRange )
      return;

    mCurrentRange = snapped;
    emit rangeChanged( mCurrentRange );
    mSliderLabels->setRange( mCurrentRange );
  } );

  connect( mMenu, &QMenu::aboutToShow, this, [this]() {
    mSettingsAction->setLimits( mRangeLimits );
    mSettingsAction->updateRangeSize( range().upper() - range().lower() );
  } );

  connect( mInvertDirectionAction, &QAction::toggled, this, [this]( bool inverted ) {
    mSlider->setFlippedDirection( !inverted );
    mSliderLabels->setInverted( inverted );

    emit invertedChanged( inverted );
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
  return mCurrentRange;
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

QgsMapCanvas *QgsElevationControllerWidget::mapCanvas() const
{
  return mMapCanvas;
}

void QgsElevationControllerWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;

  // a project which defines no elevation range of its own leaves
  // the widget with an estimated range, the canvas layers give a usable one instead
  if ( mMapCanvas && QgsProject::instance()->elevationProperties()->elevationRange().isInfinite() )
    setLimitsFromLayers( mMapCanvas->layers( true ) );
}

bool QgsElevationControllerWidget::layerHasElevation( QgsMapLayer *layer )
{
  return layer && layer->elevationProperties() && layer->elevationProperties()->hasElevation();
}

void QgsElevationControllerWidget::setLimitsFromLayers( const QList<QgsMapLayer *> &layers )
{
  // raster layers gather statistics over their elevation band for this, which is not instant
  const QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );

  setLimitsFromRange( QgsElevationUtils::calculateZRangeForLayers( layers ) );
}

void QgsElevationControllerWidget::setLimitsFromRange( const QgsDoubleRange &range )
{
  if ( range.isInfinite() || range.isEmpty() )
    return;

  const QgsDoubleRange rounded = QgsMathUtils::roundedRange( range );
  setRangeLimits( rounded );
  setRange( rounded );
}

void QgsElevationControllerWidget::setRange( const QgsDoubleRange &range )
{
  const QgsDoubleRange newRange = mFixedRangeSize >= 0 ? fixedSizeRangeFrom( range.lower() ) : range;
  if ( newRange == mCurrentRange )
    return;

  mCurrentRange = newRange;
  mBlockSliderChanges = true;
  mSlider->setRange( static_cast<int>( std::floor( mCurrentRange.lower() * mSliderPrecision ) ), static_cast<int>( std::ceil( mCurrentRange.upper() * mSliderPrecision ) ) );
  mBlockSliderChanges = false;
  emit rangeChanged( mCurrentRange );

  mSliderLabels->setRange( mCurrentRange );
}

void QgsElevationControllerWidget::setRangeLimits( const QgsDoubleRange &limits )
{
  if ( limits.isInfinite() || limits.upper() <= limits.lower() )
    return;

  mRangeLimits = limits;
  mSettingsAction->setLimits( mRangeLimits );

  const double limitRange = limits.upper() - limits.lower();

  // pick a reasonable slider precision, given that the slider operates in integer values only
  mSliderPrecision = std::max( 1000, mSlider->height() ) / limitRange;

  // snap the slider to a tenth of the interval used to round elevation ranges, so that dragging
  // stays smooth while still selecting round values
  mSnapInterval = QgsMathUtils::roundingInterval( limitRange, 100 );
  mSnapDecimals = mSnapInterval > 0 ? std::max( 0, -static_cast<int>( std::floor( std::log10( mSnapInterval ) ) ) ) : 0;

  mBlockSliderChanges = true;
  mSlider->setRangeLimits( static_cast<int>( std::floor( limits.lower() * mSliderPrecision ) ), static_cast<int>( std::ceil( limits.upper() * mSliderPrecision ) ) );

  // the slider holds the fixed size in its own integer units, which the new precision changed
  if ( mFixedRangeSize >= 0 )
    mSlider->setFixedRangeSize( static_cast<int>( std::round( mFixedRangeSize * mSliderPrecision ) ) );

  // clip current range to fit limits
  double newCurrentLower = std::max( mCurrentRange.lower(), limits.lower() );
  double newCurrentUpper = std::min( mCurrentRange.upper(), limits.upper() );
  if ( mFixedRangeSize >= 0 )
  {
    // a locked size is kept, the range moves inside the new limits instead of being clipped
    const QgsDoubleRange fitted = fixedSizeRangeFrom( newCurrentLower );
    newCurrentLower = fitted.lower();
    newCurrentUpper = fitted.upper();
  }
  const bool rangeHasChanged = newCurrentLower != mCurrentRange.lower() || newCurrentUpper != mCurrentRange.upper();

  mSlider->setRange( static_cast<int>( std::floor( newCurrentLower * mSliderPrecision ) ), static_cast<int>( std::ceil( newCurrentUpper * mSliderPrecision ) ) );
  mCurrentRange = QgsDoubleRange( newCurrentLower, newCurrentUpper );
  mBlockSliderChanges = false;
  if ( rangeHasChanged )
  {
    emit rangeChanged( mCurrentRange );

    // the selected range was clipped by the new limits, so the range size shown in the menu is stale
    mSettingsAction->updateRangeSize( mCurrentRange.upper() - mCurrentRange.lower() );
  }

  mSliderLabels->setLimits( mRangeLimits );
  mSliderLabels->setRange( mCurrentRange );
}

QgsDoubleRange QgsElevationControllerWidget::snappedRange( const QgsDoubleRange &range ) const
{
  const double lower = snapValue( range.lower() );

  // snapping must not alter the locked range size
  if ( mFixedRangeSize >= 0 )
    return fixedSizeRangeFrom( lower );

  return QgsDoubleRange( lower, std::max( snapValue( range.upper() ), lower ) );
}

double QgsElevationControllerWidget::snapValue( double value ) const
{
  if ( mSnapInterval <= 0 )
    return std::clamp( value, mRangeLimits.lower(), mRangeLimits.upper() );

  double snapped = std::round( value / mSnapInterval ) * mSnapInterval;
  if ( mSnapDecimals > 0 )
  {
    // strip the noise the multiplication above leaves in fractional values
    snapped = qgsRound( snapped, mSnapDecimals );
  }

  // an elevation which is significant for the layers is a better snapping target than a round value
  for ( double elevation : mSignificantElevations )
  {
    if ( std::fabs( elevation - value ) < std::fabs( snapped - value ) )
      snapped = elevation;
  }

  return std::clamp( snapped, mRangeLimits.lower(), mRangeLimits.upper() );
}

QgsDoubleRange QgsElevationControllerWidget::sliderRange() const
{
  return QgsDoubleRange( mSlider->lowerValue() / mSliderPrecision, mSlider->upperValue() / mSliderPrecision );
}

QgsDoubleRange QgsElevationControllerWidget::fixedSizeRangeFrom( double lower ) const
{
  double upper = lower + mFixedRangeSize;
  if ( upper > mRangeLimits.upper() )
  {
    upper = mRangeLimits.upper();
    lower = std::max( upper - mFixedRangeSize, mRangeLimits.lower() );
  }

  return QgsDoubleRange( lower, upper );
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
    mSlider->setFixedRangeSize( static_cast<int>( std::round( mFixedRangeSize * mSliderPrecision ) ) );
  }

  mSettingsAction->setFixedRangeSize( mFixedRangeSize );

  emit fixedRangeSizeChanged( mFixedRangeSize );
}

void QgsElevationControllerWidget::setInverted( bool inverted )
{
  mInvertDirectionAction->setChecked( inverted );
}

void QgsElevationControllerWidget::setSignificantElevations( const QList<double> &elevations )
{
  mSignificantElevations = elevations;
  mSliderLabels->setSignificantElevations( elevations );
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

  const QRect sliderRect = style()->subControlRect( QStyle::CC_Slider, &styleOption, QStyle::SC_SliderHandle, this );
  const int sliderHeight = sliderRect.height();

  QFont f = font();
  const QFontMetrics fm( f );

  const int left = rect().left() + 2;

  const double limitRange = mLimits.upper() - mLimits.lower();
  const double lowerFraction = ( mRange.lower() - mLimits.lower() ) / limitRange;
  const double upperFraction = ( mRange.upper() - mLimits.lower() ) / limitRange;
  const int lowerY
    = !mInverted
        ? ( std::min( static_cast<int>( std::round( rect().bottom() - sliderHeight * 0.5 - ( rect().height() - sliderHeight ) * lowerFraction + fm.ascent() ) ), rect().bottom() - fm.descent() ) )
        : ( std::max( static_cast<int>( std::round( rect().top() + sliderHeight * 0.5 + ( rect().height() - sliderHeight ) * lowerFraction - fm.descent() ) ), rect().top() + fm.ascent() ) );
  const int upperY
    = !mInverted
        ? ( std::max( static_cast<int>( std::round( rect().bottom() - sliderHeight * 0.5 - ( rect().height() - sliderHeight ) * upperFraction - fm.descent() ) ), rect().top() + fm.ascent() ) )
        : ( std::min( static_cast<int>( std::round( rect().top() + sliderHeight * 0.5 + ( rect().height() - sliderHeight ) * upperFraction + fm.ascent() ) ), rect().bottom() - fm.descent() ) );

  const bool lowerIsCloseToLimit = !mInverted ? ( lowerY + fm.height() > rect().bottom() - fm.descent() ) : ( lowerY - fm.height() < rect().top() + fm.ascent() );
  const bool upperIsCloseToLimit = !mInverted ? ( upperY - fm.height() < rect().top() + fm.ascent() ) : ( upperY + fm.height() > rect().bottom() - fm.descent() );
  const bool lowerIsCloseToUpperLimit = !mInverted ? ( lowerY - fm.height() < rect().top() + fm.ascent() ) : ( lowerY + fm.height() > rect().bottom() - fm.descent() );

  QLocale locale;

  QPainterPath path;

  for ( double value : std::as_const( mSignificantElevations ) )
  {
    const double valueFraction = ( value - mLimits.lower() ) / limitRange;
    const double verticalCenter
      = !mInverted
          ? ( std::min( static_cast<int>( std::round( rect().bottom() - sliderHeight * 0.5 - ( rect().height() - sliderHeight ) * valueFraction + fm.capHeight() * 0.5 ) ), rect().bottom() - fm.descent() ) )
          : ( std::max( static_cast<int>( std::round( rect().top() + sliderHeight * 0.5 + ( rect().height() - sliderHeight ) * valueFraction + fm.capHeight() * 0.5 ) ), rect().top() + fm.ascent() ) );

    const bool valueIsCloseToLower = verticalCenter + fm.height() > lowerY && verticalCenter - fm.height() < lowerY;
    if ( valueIsCloseToLower )
      continue;

    const bool valueIsCloseToUpper = verticalCenter + fm.height() > upperY && verticalCenter - fm.height() < upperY;
    if ( valueIsCloseToUpper )
      continue;

    const bool valueIsCloseToLowerLimit = !mInverted ? ( verticalCenter + fm.height() > rect().bottom() - fm.descent() ) : ( verticalCenter - fm.height() < rect().top() + fm.ascent() );
    if ( valueIsCloseToLowerLimit )
      continue;

    const bool valueIsCloseToUpperLimit = !mInverted ? ( verticalCenter - fm.height() < rect().top() + fm.ascent() ) : ( verticalCenter + fm.height() > rect().bottom() - fm.descent() );
    if ( valueIsCloseToUpperLimit )
      continue;

    path.addText( left, verticalCenter, f, locale.toString( value ) );
  }

  if ( mLimits.lower() > std::numeric_limits<double>::lowest() )
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
      path.addText( left, !mInverted ? ( rect().bottom() - fm.descent() ) : ( rect().top() + fm.ascent() ), f, locale.toString( mLimits.lower() ) );
    }
  }

  if ( mLimits.upper() < std::numeric_limits<double>::max() )
  {
    if ( qgsDoubleNear( mRange.upper(), mRange.lower() ) )
    {
      if ( !lowerIsCloseToUpperLimit )
      {
        f.setBold( false );
        path.addText( left, !mInverted ? ( rect().top() + fm.ascent() ) : ( rect().bottom() - fm.descent() ), f, locale.toString( mLimits.upper() ) );
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
        path.addText( left, !mInverted ? ( rect().top() + fm.ascent() ) : ( rect().bottom() - fm.descent() ), f, locale.toString( mLimits.upper() ) );
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
  const int maxChars = std::max( QLocale().toString( std::floor( limits.lower() ) ).length(), QLocale().toString( std::floor( limits.upper() ) ).length() ) + 3;
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

void QgsElevationControllerLabels::setInverted( bool inverted )
{
  if ( inverted == mInverted )
    return;

  mInverted = inverted;
  update();
}

void QgsElevationControllerLabels::setSignificantElevations( const QList<double> &elevations )
{
  if ( elevations == mSignificantElevations )
    return;

  mSignificantElevations = elevations;
  update();
}

//
// QgsElevationControllerSettingsAction
//

QgsElevationControllerSettingsAction::QgsElevationControllerSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
  , mMenu( qobject_cast<QMenu *>( parent ) )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  QLabel *limitsLabel = new QLabel( tr( "Limits" ) );
  limitsLabel->setToolTip( tr( "Lowest and highest elevations which can be selected with the slider" ) );
  gLayout->addWidget( limitsLabel, 0, 0 );

  mLowerSpin = new QgsDoubleSpinBox();
  mLowerSpin->setDecimals( 4 );
  mLowerSpin->setMinimum( -999999999.0 );
  mLowerSpin->setMaximum( 999999999.0 );
  mLowerSpin->setShowClearButton( false );
  mLowerSpin->setKeyboardTracking( false );
  mLowerSpin->setToolTip( tr( "Lowest elevation which can be selected" ) );

  gLayout->addWidget( mLowerSpin, 0, 1 );

  QLabel *toLabel = new QLabel( tr( "to" ) );
  gLayout->addWidget( toLabel, 0, 2 );

  mUpperSpin = new QgsDoubleSpinBox();
  mUpperSpin->setDecimals( 4 );
  mUpperSpin->setMinimum( -999999999.0 );
  mUpperSpin->setMaximum( 999999999.0 );
  mUpperSpin->setShowClearButton( false );
  mUpperSpin->setKeyboardTracking( false );
  mUpperSpin->setToolTip( tr( "Highest elevation which can be selected" ) );

  gLayout->addWidget( mUpperSpin, 0, 3 );

  mLimitsButton = new QToolButton();
  mLimitsButton->setIcon( QgsApplication::getThemeIcon( u"/mActionRefresh.svg"_s ) );
  mLimitsButton->setPopupMode( QToolButton::InstantPopup );
  mLimitsButton->setToolTip( tr( "Take the elevation limits from the project or the layers" ) );
  mLimitsMenu = new QMenu( mLimitsButton );
  mLimitsButton->setMenu( mLimitsMenu );

  gLayout->addWidget( mLimitsButton, 0, 4 );

  const QString rangeToolTip = tr(
    "Size of the elevation range shown in the map, i.e. the difference between "
    "the upper and the lower handle of the slider. Lock it to keep this size while "
    "moving the slider, or clear it to show the full range."
  );

  QLabel *rangeLabel = new QLabel( tr( "Range" ) );
  rangeLabel->setToolTip( rangeToolTip );
  gLayout->addWidget( rangeLabel, 1, 0 );

  mSizeSpin = new QgsDoubleSpinBox();
  mSizeSpin->setDecimals( 4 );
  mSizeSpin->setMinimum( 0.0 );
  mSizeSpin->setMaximum( 999999999.0 );
  mSizeSpin->setClearValue( 0, tr( "Full range" ) );
  mSizeSpin->setKeyboardTracking( false );
  mSizeSpin->setToolTip( rangeToolTip );

  gLayout->addWidget( mSizeSpin, 1, 1, 1, 3 );

  mLockButton = new QToolButton();
  mLockButton->setIcon( QgsApplication::getThemeIcon( u"/cadtools/lock.svg"_s ) );
  mLockButton->setCheckable( true );
  mLockButton->setToolTip( tr( "Lock the elevation range to a fixed size" ) );

  gLayout->addWidget( mLockButton, 1, 4 );

  // the spin boxes take the extra width, not the labels or the buttons
  gLayout->setColumnStretch( 1, 1 );
  gLayout->setColumnStretch( 3, 1 );

  const auto emitLimits = [this] { emit limitsChanged( QgsDoubleRange( mLowerSpin->value(), mUpperSpin->value() ) ); };
  connect( mLowerSpin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, emitLimits );
  connect( mUpperSpin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, emitLimits );

  connect( mSizeSpin, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [this]( double size ) {
    if ( mSizeSpin->isCleared() )
    {
      emit fixedRangeSizeChanged( -1 );
      emit fullRangeRequested();
    }
    else if ( mLockButton->isChecked() )
    {
      emit fixedRangeSizeChanged( size );
    }
  } );
  connect( mLockButton, &QToolButton::toggled, this, [this]( bool locked ) { emit fixedRangeSizeChanged( locked ? mSizeSpin->value() : -1 ); } );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );

  // watch the whole panel, so that hovering any of its widgets marks this action as the
  // active one and key presses are never handed over to the menu
  w->installEventFilter( this );
  const QList<QWidget *> children = w->findChildren<QWidget *>();
  for ( QWidget *child : children )
    child->installEventFilter( this );

  connect( this, &QAction::hovered, this, &QgsElevationControllerSettingsAction::onHover );

  setDefaultWidget( w );
}

QMenu *QgsElevationControllerSettingsAction::limitsMenu()
{
  return mLimitsMenu;
}

void QgsElevationControllerSettingsAction::setLimits( const QgsDoubleRange &limits )
{
  const QSignalBlocker blockLower( mLowerSpin );
  const QSignalBlocker blockUpper( mUpperSpin );
  // the limits can always be widened, but never crossed over
  mLowerSpin->setMaximum( limits.upper() );
  mUpperSpin->setMinimum( limits.lower() );
  mLowerSpin->setValue( limits.lower() );
  mUpperSpin->setValue( limits.upper() );
}

void QgsElevationControllerSettingsAction::setFixedRangeSize( double size )
{
  {
    const QSignalBlocker blockLock( mLockButton );
    mLockButton->setChecked( size >= 0 );
  }

  if ( size >= 0 )
  {
    const QSignalBlocker blockSpin( mSizeSpin );
    mSizeSpin->setValue( size );
  }
}

void QgsElevationControllerSettingsAction::updateRangeSize( double size )
{
  // a locked size is the one the user asked for, it must not be overwritten
  if ( mLockButton->isChecked() )
    return;

  const QSignalBlocker blockSpin( mSizeSpin );
  mSizeSpin->setValue( size );
}

bool QgsElevationControllerSettingsAction::eventFilter( QObject *watched, QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::Enter:
      onHover();
      break;

    case QEvent::KeyPress:
    {
      const QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
      if ( keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter )
      {
        // spin boxes leave the return key for their parents to handle, which would make the
        // menu activate its highlighted action and close. Apply the typed value ourselves instead.
        QgsDoubleSpinBox *spin = qobject_cast<QgsDoubleSpinBox *>( watched );
        if ( !spin )
          spin = qobject_cast<QgsDoubleSpinBox *>( watched->parent() );

        if ( spin )
        {
          spin->interpretText();
          if ( spin == mSizeSpin && !mSizeSpin->isCleared() )
            mLockButton->setChecked( true );
          return true;
        }
      }
      break;
    }

    default:
      break;
  }
  return QWidgetAction::eventFilter( watched, event );
}

void QgsElevationControllerSettingsAction::onHover()
{
  // see https://bugreports.qt.io/browse/QTBUG-10427
  // the menu keeps highlighting the action the mouse last passed over, and would trigger it
  // when the user hits enter while interacting with this widget
  if ( mSuppressRecurse || !mMenu )
    return;

  mSuppressRecurse = true;
  mMenu->setActiveAction( this );
  mSuppressRecurse = false;
}

///@endcond PRIVATE
