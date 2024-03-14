/***************************************************************************
    qgsrasterelevationpropertieswidget.cpp
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterelevationpropertieswidget.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

QgsRasterElevationPropertiesWidget::QgsRasterElevationPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mModeComboBox->addItem( tr( "Disabled" ) );
  mModeComboBox->addItem( tr( "Represents Elevation Surface" ), QVariant::fromValue( Qgis::RasterElevationMode::RepresentsElevationSurface ) );
  mModeComboBox->addItem( tr( "Fixed Elevation Range" ), QVariant::fromValue( Qgis::RasterElevationMode::FixedElevationRange ) );

  mLimitsComboBox->addItem( tr( "Include Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeBoth ) );
  mLimitsComboBox->addItem( tr( "Include Lower, Exclude Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeLowerExcludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower, Include Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeLowerIncludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeBoth ) );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );
  mSymbologyStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mFixedLowerSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedUpperSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedLowerSpinBox->clear();
  mFixedUpperSpinBox->clear();

  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillBelow ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillAbove.svg" ) ), tr( "Fill Above" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillAbove ) );
  mElevationLimitSpinBox->setClearValue( mElevationLimitSpinBox->minimum(), tr( "Not set" ) );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mElevationLimitSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterElevationPropertiesWidget::modeChanged );
  connect( mLimitsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mStyleComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    switch ( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) )
    {
      case Qgis::ProfileSurfaceSymbology::Line:
        mSymbologyStackedWidget->setCurrentWidget( mPageLine );
        break;
      case Qgis::ProfileSurfaceSymbology::FillBelow:
      case Qgis::ProfileSurfaceSymbology::FillAbove:
        mSymbologyStackedWidget->setCurrentWidget( mPageFill );
        break;
    }

    onChanged();
  } );

  setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#elevation-properties" ) );
}

void QgsRasterElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  const QgsRasterLayerElevationProperties *props = qgis::down_cast< const QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );
  if ( !props->isEnabled() )
  {
    mModeComboBox->setCurrentIndex( 0 );
    mStackedWidget->setCurrentWidget( mPageDisabled );
    mProfileChartGroupBox->hide();
  }
  else
  {
    mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( props->mode() ) ) );
    switch ( props->mode() )
    {
      case Qgis::RasterElevationMode::FixedElevationRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::RasterElevationMode::RepresentsElevationSurface:
        mStackedWidget->setCurrentWidget( mPageSurface );
        break;
    }
    mProfileChartGroupBox->show();
  }

  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  if ( std::isnan( props->elevationLimit() ) )
    mElevationLimitSpinBox->clear();
  else
    mElevationLimitSpinBox->setValue( props->elevationLimit() );
  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );
  mBandComboBox->setLayer( mLayer );
  mBandComboBox->setBand( props->bandNumber() );

  if ( props->fixedRange().lower() != std::numeric_limits< double >::lowest() )
    mFixedLowerSpinBox->setValue( props->fixedRange().lower() );
  else
    mFixedLowerSpinBox->clear();
  if ( props->fixedRange().upper() != std::numeric_limits< double >::max() )
    mFixedUpperSpinBox->setValue( props->fixedRange().upper() );
  else
    mFixedUpperSpinBox->clear();
  if ( props->fixedRange().includeLower() && props->fixedRange().includeUpper() )
    mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( Qgis::RangeLimits::IncludeBoth ) ) );
  else if ( props->fixedRange().includeLower() )
    mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( Qgis::RangeLimits::IncludeLowerExcludeUpper ) ) );
  else if ( props->fixedRange().includeUpper() )
    mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( Qgis::RangeLimits::ExcludeLowerIncludeUpper ) ) );
  else
    mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( Qgis::RangeLimits::ExcludeBoth ) ) );

  mStyleComboBox->setCurrentIndex( mStyleComboBox->findData( static_cast <int >( props->profileSymbology() ) ) );
  switch ( props->profileSymbology() )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mSymbologyStackedWidget->setCurrentWidget( mPageLine );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
    case Qgis::ProfileSurfaceSymbology::FillAbove:
      mSymbologyStackedWidget->setCurrentWidget( mPageFill );
      break;
  }

  mBlockUpdates = false;
}

void QgsRasterElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsRasterLayerElevationProperties *props = qgis::down_cast< QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );

  if ( !mModeComboBox->currentData().isValid() )
  {
    props->setEnabled( false );
  }
  else
  {
    props->setEnabled( true );
    props->setMode( mModeComboBox->currentData().value< Qgis::RasterElevationMode >() );
  }

  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  if ( mElevationLimitSpinBox->value() != mElevationLimitSpinBox->clearValue() )
    props->setElevationLimit( mElevationLimitSpinBox->value() );
  else
    props->setElevationLimit( std::numeric_limits< double >::quiet_NaN() );
  props->setProfileLineSymbol( mLineStyleButton->clonedSymbol< QgsLineSymbol >() );
  props->setProfileFillSymbol( mFillStyleButton->clonedSymbol< QgsFillSymbol >() );
  props->setProfileSymbology( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) );
  props->setBandNumber( mBandComboBox->currentBand() );

  double fixedLower = std::numeric_limits< double >::lowest();
  double fixedUpper = std::numeric_limits< double >::max();
  if ( mFixedLowerSpinBox->value() != mFixedLowerSpinBox->clearValue() )
    fixedLower = mFixedLowerSpinBox->value();
  if ( mFixedUpperSpinBox->value() != mFixedUpperSpinBox->clearValue() )
    fixedUpper = mFixedUpperSpinBox->value();

  bool includeLower = true;
  bool includeUpper = true;
  switch ( mLimitsComboBox->currentData().value< Qgis::RangeLimits >() )
  {
    case Qgis::RangeLimits::IncludeBoth:
      break;
    case Qgis::RangeLimits::IncludeLowerExcludeUpper:
      includeUpper = false;
      break;
    case Qgis::RangeLimits::ExcludeLowerIncludeUpper:
      includeLower = false;
      break;
    case Qgis::RangeLimits::ExcludeBoth:
      includeLower = false;
      includeUpper = false;
      break;
  }
  props->setFixedRange( QgsDoubleRange( fixedLower, fixedUpper, includeLower, includeUpper ) );

  mLayer->trigger3DUpdate();
}

void QgsRasterElevationPropertiesWidget::modeChanged()
{
  if ( mModeComboBox->currentData().isValid() )
  {
    switch ( mModeComboBox->currentData().value< Qgis::RasterElevationMode >() )
    {
      case Qgis::RasterElevationMode::FixedElevationRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::RasterElevationMode::RepresentsElevationSurface:
        mStackedWidget->setCurrentWidget( mPageSurface );
        break;
    }
    mProfileChartGroupBox->show();
  }
  else
  {
    mStackedWidget->setCurrentWidget( mPageDisabled );
    mProfileChartGroupBox->hide();
  }

  onChanged();
}

void QgsRasterElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}


//
// QgsRasterElevationPropertiesWidgetFactory
//

QgsRasterElevationPropertiesWidgetFactory::QgsRasterElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsRasterElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsRasterElevationPropertiesWidget( qobject_cast< QgsRasterLayer * >( layer ), canvas, parent );
}

bool QgsRasterElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Raster;
}

QString QgsRasterElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

