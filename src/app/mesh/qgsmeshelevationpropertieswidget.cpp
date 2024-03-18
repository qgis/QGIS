/***************************************************************************
    qgsmeshelevationpropertieswidget.cpp
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

#include "qgsmeshelevationpropertieswidget.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

QgsMeshElevationPropertiesWidget::QgsMeshElevationPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mModeComboBox->addItem( tr( "From Vertices" ), QVariant::fromValue( Qgis::MeshElevationMode::FromVertices ) );
  mModeComboBox->addItem( tr( "Fixed Elevation Range" ), QVariant::fromValue( Qgis::MeshElevationMode::FixedElevationRange ) );

  mLimitsComboBox->addItem( tr( "Include Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeBoth ) );
  mLimitsComboBox->addItem( tr( "Include Lower, Exclude Upper" ), QVariant::fromValue( Qgis::RangeLimits::IncludeLowerExcludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower, Include Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeLowerIncludeUpper ) );
  mLimitsComboBox->addItem( tr( "Exclude Lower and Upper" ), QVariant::fromValue( Qgis::RangeLimits::ExcludeBoth ) );

  mStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );
  mSymbologyStackedWidget->setSizeMode( QgsStackedWidget::SizeMode::CurrentPageOnly );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillBelow ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillAbove.svg" ) ), tr( "Fill Above" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillAbove ) );
  mElevationLimitSpinBox->setClearValue( mElevationLimitSpinBox->minimum(), tr( "Not set" ) );

  mFixedLowerSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedUpperSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue, tr( "Not set" ) );
  mFixedLowerSpinBox->clear();
  mFixedUpperSpinBox->clear();

  syncToLayer( layer );

  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshElevationPropertiesWidget::modeChanged );
  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mElevationLimitSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsMeshElevationPropertiesWidget::onChanged );
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

  setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#elevation-properties" ) );
}

void QgsMeshElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsMeshLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  const QgsMeshLayerElevationProperties *props = qgis::down_cast< const QgsMeshLayerElevationProperties * >( mLayer->elevationProperties() );

  mModeComboBox->setCurrentIndex( mModeComboBox->findData( QVariant::fromValue( props->mode() ) ) );
  switch ( props->mode() )
  {
    case Qgis::MeshElevationMode::FixedElevationRange:
      mStackedWidget->setCurrentWidget( mPageFixedRange );
      break;
    case Qgis::MeshElevationMode::FromVertices:
      mStackedWidget->setCurrentWidget( mPageFromVertices );
      break;
  }

  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  if ( std::isnan( props->elevationLimit() ) )
    mElevationLimitSpinBox->clear();
  else
    mElevationLimitSpinBox->setValue( props->elevationLimit() );

  if ( props->fixedRange().lower() != std::numeric_limits< double >::lowest() )
    mFixedLowerSpinBox->setValue( props->fixedRange().lower() );
  else
    mFixedLowerSpinBox->clear();
  if ( props->fixedRange().upper() != std::numeric_limits< double >::max() )
    mFixedUpperSpinBox->setValue( props->fixedRange().upper() );
  else
    mFixedUpperSpinBox->clear();
  mLimitsComboBox->setCurrentIndex( mLimitsComboBox->findData( QVariant::fromValue( props->fixedRange().rangeLimits() ) ) );

  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );

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

void QgsMeshElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsMeshLayerElevationProperties *props = qgis::down_cast< QgsMeshLayerElevationProperties * >( mLayer->elevationProperties() );
  props->setMode( mModeComboBox->currentData().value< Qgis::MeshElevationMode >() );

  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  if ( mElevationLimitSpinBox->value() != mElevationLimitSpinBox->clearValue() )
    props->setElevationLimit( mElevationLimitSpinBox->value() );
  else
    props->setElevationLimit( std::numeric_limits< double >::quiet_NaN() );

  double fixedLower = std::numeric_limits< double >::lowest();
  double fixedUpper = std::numeric_limits< double >::max();
  if ( mFixedLowerSpinBox->value() != mFixedLowerSpinBox->clearValue() )
    fixedLower = mFixedLowerSpinBox->value();
  if ( mFixedUpperSpinBox->value() != mFixedUpperSpinBox->clearValue() )
    fixedUpper = mFixedUpperSpinBox->value();

  props->setFixedRange( QgsDoubleRange( fixedLower, fixedUpper, mLimitsComboBox->currentData().value< Qgis::RangeLimits >() ) );

  props->setProfileLineSymbol( mLineStyleButton->clonedSymbol< QgsLineSymbol >() );
  props->setProfileFillSymbol( mFillStyleButton->clonedSymbol< QgsFillSymbol >() );
  props->setProfileSymbology( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) );
  mLayer->trigger3DUpdate();
}

void QgsMeshElevationPropertiesWidget::modeChanged()
{
  if ( mModeComboBox->currentData().isValid() )
  {
    switch ( mModeComboBox->currentData().value< Qgis::MeshElevationMode >() )
    {
      case Qgis::MeshElevationMode::FixedElevationRange:
        mStackedWidget->setCurrentWidget( mPageFixedRange );
        break;
      case Qgis::MeshElevationMode::FromVertices:
        mStackedWidget->setCurrentWidget( mPageFromVertices );
        break;
    }
  }

  onChanged();
}

void QgsMeshElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}


//
// QgsMeshElevationPropertiesWidgetFactory
//

QgsMeshElevationPropertiesWidgetFactory::QgsMeshElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsMeshElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsMeshElevationPropertiesWidget( qobject_cast< QgsMeshLayer * >( layer ), canvas, parent );
}

bool QgsMeshElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsMeshElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsMeshElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Mesh;
}

QString QgsMeshElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

