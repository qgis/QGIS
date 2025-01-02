/***************************************************************************
    qgsmeshrenderervectorsettingswidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderervectorsettingswidget.h"
#include "moc_qgsmeshrenderervectorsettingswidget.cpp"

#include "qgis.h"
#include "qgsmeshlayer.h"

QgsMeshRendererVectorSettingsWidget::QgsMeshRendererVectorSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  QVector<QgsDoubleSpinBox *> widgets;
  widgets << mMinMagSpinBox << mMaxMagSpinBox
          << mHeadWidthSpinBox << mHeadLengthSpinBox
          << mMinimumShaftSpinBox << mMaximumShaftSpinBox
          << mScaleShaftByFactorOfSpinBox << mShaftLengthSpinBox
          << mWindBarbLengthSpinBox << mWindBarbMagnitudeMultiplierSpinBox;

  // Setup defaults and clear values for spin boxes
  for ( const auto &widget : std::as_const( widgets ) )
  {
    widget->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue );
    widget->setSpecialValueText( QString() );
    widget->setValue( widget->minimum() );
  }

  mShaftLengthComboBox->setCurrentIndex( -1 );

  mColoringMethodComboBox->addItem( tr( "Single Color" ), QgsInterpolatedLineColor::SingleColor );
  mColoringMethodComboBox->addItem( tr( "Color Ramp Shader" ), QgsInterpolatedLineColor::ColorRamp );

  mXSpacingSpinBox->setClearValue( 10.0 );
  mYSpacingSpinBox->setClearValue( 10.0 );
  mStreamlinesDensitySpinBox->setClearValue( 15.0 );
  mTracesParticlesCountSpinBox->setClearValue( 1000 );
  mTracesMaxLengthSpinBox->setClearValue( 100.0 );

  mWindBarbLengthSpinBox->setClearValue( 10.0 );
  mWindBarbMagnitudeMultiplierSpinBox->setValue( 1.0 );
  mWindBarbMagnitudeMultiplierSpinBox->setClearValue( 1.0 );

  connect( mColorWidget, &QgsColorButton::colorChanged, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mColoringMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::onColoringMethodChanged );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mColorRampShaderMinimumSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::onColorRampMinMaxChanged );
  connect( mColorRampShaderMaximumSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::onColorRampMinMaxChanged );

  connect( mLineWidthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), mShaftOptionsStackedWidget, &QStackedWidget::setCurrentIndex );

  connect( mDisplayVectorsOnGridGroupBox, &QGroupBox::toggled, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mColorRampShaderLoadButton, &QPushButton::clicked, this, &QgsMeshRendererVectorSettingsWidget::loadColorRampShader );

  onColoringMethodChanged();

  for ( const auto &widget : std::as_const( widgets ) )
  {
    connect( widget, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  }

  connect( mXSpacingSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mYSpacingSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mSymbologyVectorComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::onSymbologyChanged );
  onSymbologyChanged( 0 );

  connect( mSymbologyVectorComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mStreamlinesSeedingMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::onStreamLineSeedingMethodChanged );
  onStreamLineSeedingMethodChanged( 0 );

  connect( mStreamlinesSeedingMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mStreamlinesDensitySpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mTracesMaxLengthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mTracesParticlesCountSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  mTracesTailLengthMapUnitWidget->setUnits(
    { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points
    }
  );

  connect( mTracesTailLengthMapUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  mWindBarbLengthMapUnitWidget->setUnits(
    { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points
    }
  );

  connect( mWindBarbLengthMapUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mWindBarbUnitsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererVectorSettingsWidget::onWindBarbUnitsChanged );
  onWindBarbUnitsChanged( 0 );
}

void QgsMeshRendererVectorSettingsWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

QgsMeshRendererVectorSettings QgsMeshRendererVectorSettingsWidget::settings() const
{
  QgsMeshRendererVectorSettings settings;
  settings.setSymbology(
    static_cast<QgsMeshRendererVectorSettings::Symbology>( mSymbologyVectorComboBox->currentIndex() )
  );

  //Arrow settings
  QgsMeshRendererVectorArrowSettings arrowSettings;

  // basic
  settings.setColor( mColorWidget->color() );
  settings.setLineWidth( mLineWidthSpinBox->value() );
  settings.setColoringMethod( static_cast<QgsInterpolatedLineColor::ColoringMethod>( mColoringMethodComboBox->currentData().toInt() ) );
  settings.setColorRampShader( mColorRampShaderWidget->shader() );

  // filter by magnitude
  double val = filterValue( mMinMagSpinBox, -1 );
  settings.setFilterMin( val );

  val = filterValue( mMaxMagSpinBox, -1 );
  settings.setFilterMax( val );

  // arrow head
  val = filterValue( mHeadWidthSpinBox, arrowSettings.arrowHeadWidthRatio() * 100.0 );
  arrowSettings.setArrowHeadWidthRatio( val / 100.0 );

  val = filterValue( mHeadLengthSpinBox, arrowSettings.arrowHeadLengthRatio() * 100.0 );
  arrowSettings.setArrowHeadLengthRatio( val / 100.0 );

  // user grid
  bool enabled = mDisplayVectorsOnGridGroupBox->isChecked();
  settings.setOnUserDefinedGrid( enabled );
  settings.setUserGridCellWidth( mXSpacingSpinBox->value() );
  settings.setUserGridCellHeight( mYSpacingSpinBox->value() );

  // shaft length
  auto method = static_cast<QgsMeshRendererVectorArrowSettings::ArrowScalingMethod>( mShaftLengthComboBox->currentIndex() );
  arrowSettings.setShaftLengthMethod( method );

  val = filterValue( mMinimumShaftSpinBox, arrowSettings.minShaftLength() );
  arrowSettings.setMinShaftLength( val );

  val = filterValue( mMaximumShaftSpinBox, arrowSettings.maxShaftLength() );
  arrowSettings.setMaxShaftLength( val );

  val = filterValue( mScaleShaftByFactorOfSpinBox, arrowSettings.scaleFactor() );
  arrowSettings.setScaleFactor( val );

  val = filterValue( mShaftLengthSpinBox, arrowSettings.fixedShaftLength() );
  arrowSettings.setFixedShaftLength( val );

  settings.setArrowsSettings( arrowSettings );

  //Streamline setting
  QgsMeshRendererVectorStreamlineSettings streamlineSettings;
  streamlineSettings.setSeedingMethod(
    static_cast<QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod>( mStreamlinesSeedingMethodComboBox->currentIndex() )
  );

  streamlineSettings.setSeedingDensity( mStreamlinesDensitySpinBox->value() / 100 );

  settings.setStreamLinesSettings( streamlineSettings );

  //Traces setting
  QgsMeshRendererVectorTracesSettings tracesSettings;
  tracesSettings.setMaximumTailLength( mTracesMaxLengthSpinBox->value() );
  tracesSettings.setMaximumTailLengthUnit( mTracesTailLengthMapUnitWidget->unit() );
  tracesSettings.setParticlesCount( mTracesParticlesCountSpinBox->value() );
  settings.setTracesSettings( tracesSettings );

  // Wind Barb settings
  QgsMeshRendererVectorWindBarbSettings windBarbSettings;
  windBarbSettings.setShaftLength( mWindBarbLengthSpinBox->value() );
  windBarbSettings.setShaftLengthUnits( mWindBarbLengthMapUnitWidget->unit() );
  windBarbSettings.setMagnitudeUnits(
    static_cast<QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit>( mWindBarbUnitsComboBox->currentIndex() )
  );
  windBarbSettings.setMagnitudeMultiplier( mWindBarbMagnitudeMultiplierSpinBox->value() );
  settings.setWindBarbSettings( windBarbSettings );

  return settings;
}

void QgsMeshRendererVectorSettingsWidget::syncToLayer()
{
  if ( !mMeshLayer || !mMeshLayer->dataProvider() )
    return;

  if ( mActiveDatasetGroup < 0 )
    return;

  bool hasFaces = ( mMeshLayer->dataProvider() && mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Face ) );

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( mActiveDatasetGroup );

  symbologyLabel->setVisible( hasFaces );
  mSymbologyVectorComboBox->setVisible( hasFaces );
  mSymbologyVectorComboBox->setCurrentIndex( hasFaces ? settings.symbology() : 0 );

  // Arrow settings
  const QgsMeshRendererVectorArrowSettings arrowSettings = settings.arrowSettings();

  // basic
  mColorWidget->setColor( settings.color() );
  mLineWidthSpinBox->setValue( settings.lineWidth() );
  mColoringMethodComboBox->setCurrentIndex( mColoringMethodComboBox->findData( settings.coloringMethod() ) );
  mColorRampShaderWidget->setFromShader( settings.colorRampShader() );
  mColorRampShaderMinimumSpinBox->setValue( settings.colorRampShader().minimumValue() );
  mColorRampShaderMaximumSpinBox->setValue( settings.colorRampShader().maximumValue() );

  // filter by magnitude
  if ( settings.filterMin() > 0 )
  {
    mMinMagSpinBox->setValue( settings.filterMin() );
  }
  if ( settings.filterMax() > 0 )
  {
    mMaxMagSpinBox->setValue( settings.filterMax() );
  }

  // arrow head
  mHeadWidthSpinBox->setValue( arrowSettings.arrowHeadWidthRatio() * 100.0 );
  mHeadLengthSpinBox->setValue( arrowSettings.arrowHeadLengthRatio() * 100.0 );

  // user grid
  mDisplayVectorsOnGridGroupBox->setVisible( hasFaces );
  mDisplayVectorsOnGridGroupBox->setChecked( settings.isOnUserDefinedGrid() && hasFaces );
  mXSpacingSpinBox->setValue( settings.userGridCellWidth() );
  mYSpacingSpinBox->setValue( settings.userGridCellHeight() );

  // shaft length
  mShaftLengthComboBox->setCurrentIndex( arrowSettings.shaftLengthMethod() );

  mMinimumShaftSpinBox->setValue( arrowSettings.minShaftLength() );
  mMaximumShaftSpinBox->setValue( arrowSettings.maxShaftLength() );
  mScaleShaftByFactorOfSpinBox->setValue( arrowSettings.scaleFactor() );
  mShaftLengthSpinBox->setValue( arrowSettings.fixedShaftLength() );

  //Streamlines settings
  const QgsMeshRendererVectorStreamlineSettings streamlinesSettings = settings.streamLinesSettings();

  mStreamlinesSeedingMethodComboBox->setCurrentIndex( streamlinesSettings.seedingMethod() );
  mStreamlinesDensitySpinBox->setValue( streamlinesSettings.seedingDensity() * 100 );

  //Traces settings
  const QgsMeshRendererVectorTracesSettings tracesSettings = settings.tracesSettings();

  mTracesMaxLengthSpinBox->setValue( tracesSettings.maximumTailLength() );
  mTracesTailLengthMapUnitWidget->setUnit( tracesSettings.maximumTailLengthUnit() );
  mTracesParticlesCountSpinBox->setValue( tracesSettings.particlesCount() );

  // Wind Barb settings
  const QgsMeshRendererVectorWindBarbSettings windBarbSettings = settings.windBarbSettings();
  mWindBarbLengthSpinBox->setValue( windBarbSettings.shaftLength() );
  mWindBarbUnitsComboBox->setCurrentIndex( static_cast<int>( windBarbSettings.magnitudeUnits() ) );
  if ( windBarbSettings.magnitudeUnits() == QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::OtherUnit )
    mWindBarbMagnitudeMultiplierSpinBox->setValue( windBarbSettings.magnitudeMultiplier() );
}

void QgsMeshRendererVectorSettingsWidget::onSymbologyChanged( int currentIndex )
{
  mStreamlineWidget->setVisible( currentIndex == QgsMeshRendererVectorSettings::Streamlines );
  mArrowLengthGroupBox->setVisible( currentIndex == QgsMeshRendererVectorSettings::Arrows );
  mHeadOptionsGroupBox->setVisible( currentIndex == QgsMeshRendererVectorSettings::Arrows );
  mTracesGroupBox->setVisible( currentIndex == QgsMeshRendererVectorSettings::Traces );
  mWindBarbGroupBox->setVisible( currentIndex == QgsMeshRendererVectorSettings::WindBarbs );

  mDisplayVectorsOnGridGroupBox->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );
  filterByMagnitudeLabel->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );
  minimumMagLabel->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );
  mMinMagSpinBox->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );
  maximumMagLabel->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );
  mMaxMagSpinBox->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );

  mDisplayVectorsOnGridGroupBox->setEnabled(
    currentIndex == QgsMeshRendererVectorSettings::Arrows || currentIndex == QgsMeshRendererVectorSettings::WindBarbs || ( currentIndex == QgsMeshRendererVectorSettings::Streamlines && mStreamlinesSeedingMethodComboBox->currentIndex() == QgsMeshRendererVectorStreamlineSettings::MeshGridded )
  );
}

void QgsMeshRendererVectorSettingsWidget::onStreamLineSeedingMethodChanged( int currentIndex )
{
  bool enabled = currentIndex == QgsMeshRendererVectorStreamlineSettings::Random;
  mStreamlinesDensityLabel->setEnabled( enabled );
  mStreamlinesDensitySpinBox->setEnabled( enabled );

  mDisplayVectorsOnGridGroupBox->setEnabled( !enabled );
}

void QgsMeshRendererVectorSettingsWidget::onWindBarbUnitsChanged( int currentIndex )
{
  const QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit units = static_cast<QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit>( currentIndex );

  mWindBarbMagnitudeMultiplierLabel->setVisible( units == QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::OtherUnit );
  mWindBarbMagnitudeMultiplierSpinBox->setVisible( units == QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::OtherUnit );

  emit widgetChanged();
}

void QgsMeshRendererVectorSettingsWidget::onColoringMethodChanged()
{
  mColorRampShaderGroupBox->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::ColorRamp );
  mColorWidget->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::SingleColor );
  mSingleColorLabel->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::SingleColor );

  if ( mColorRampShaderWidget->shader().colorRampItemList().isEmpty() )
    loadColorRampShader();

  emit widgetChanged();
}

void QgsMeshRendererVectorSettingsWidget::onColorRampMinMaxChanged()
{
  mColorRampShaderWidget->setMinimumMaximumAndClassify(
    filterValue( mColorRampShaderMinimumSpinBox, 0 ),
    filterValue( mColorRampShaderMaximumSpinBox, 0 )
  );
}

void QgsMeshRendererVectorSettingsWidget::loadColorRampShader()
{
  if ( !mMeshLayer )
    return;

  int currentVectorDataSetGroupIndex = mMeshLayer->rendererSettings().activeVectorDatasetGroup();
  if ( currentVectorDataSetGroupIndex < 0 || !mMeshLayer->datasetGroupMetadata( currentVectorDataSetGroupIndex ).isVector() )
    return;

  const QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( currentVectorDataSetGroupIndex );
  double min = meta.minimum();
  double max = meta.maximum();

  mColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
  whileBlocking( mColorRampShaderMinimumSpinBox )->setValue( min );
  whileBlocking( mColorRampShaderMaximumSpinBox )->setValue( max );
}

double QgsMeshRendererVectorSettingsWidget::filterValue( const QgsDoubleSpinBox *spinBox, double errVal ) const
{
  if ( spinBox->value() == spinBox->clearValue() )
    return errVal;

  return spinBox->value();
}
