/***************************************************************************
    qgsvectorfieldsettingswidget.cpp
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

#include "qgsvectorfieldsettingswidget.h"

#include "qgis.h"

#include "moc_qgsvectorfieldsettingswidget.cpp"

QgsVectorFieldSettingsWidget::QgsVectorFieldSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  QVector<QgsDoubleSpinBox *> widgets;
  widgets
    << mMinMagSpinBox
    << mMaxMagSpinBox
    << mHeadWidthSpinBox
    << mHeadLengthSpinBox
    << mMinimumShaftSpinBox
    << mMaximumShaftSpinBox
    << mScaleShaftByFactorOfSpinBox
    << mShaftLengthSpinBox
    << mWindBarbLengthSpinBox
    << mWindBarbMagnitudeMultiplierSpinBox;

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

  connect( mColorWidget, &QgsColorButton::colorChanged, this, &QgsVectorFieldSettingsWidget::widgetChanged );
  connect( mColoringMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::onColoringMethodChanged );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsVectorFieldSettingsWidget::widgetChanged );
  connect( mColorRampShaderMinimumSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::onColorRampMinMaxChanged );
  connect( mColorRampShaderMaximumSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::onColorRampMinMaxChanged );

  connect( mLineWidthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), mShaftOptionsStackedWidget, &QStackedWidget::setCurrentIndex );

  connect( mDisplayVectorsOnGridGroupBox, &QGroupBox::toggled, this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mColorRampShaderLoadButton, &QPushButton::clicked, this, &QgsVectorFieldSettingsWidget::loadColorRampShader );

  onColoringMethodChanged();

  for ( const auto &widget : std::as_const( widgets ) )
  {
    connect( widget, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );
  }

  connect( mXSpacingSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );
  connect( mYSpacingSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  populateSymbologies();

  connect( mSymbologyVectorComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::onSymbologyChanged );
  onSymbologyChanged();

  connect( mSymbologyVectorComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mStreamlinesSeedingMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::onStreamLineSeedingMethodChanged );
  onStreamLineSeedingMethodChanged( 0 );

  connect( mStreamlinesSeedingMethodComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mStreamlinesDensitySpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mTracesMaxLengthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  connect( mTracesParticlesCountSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsVectorFieldSettingsWidget::widgetChanged );

  mTracesTailLengthMapUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points } );

  connect( mTracesTailLengthMapUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsVectorFieldSettingsWidget::widgetChanged );

  mWindBarbLengthMapUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points } );

  connect( mWindBarbLengthMapUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsVectorFieldSettingsWidget::widgetChanged );
  connect( mWindBarbUnitsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSettingsWidget::onWindBarbUnitsChanged );
  onWindBarbUnitsChanged( 0 );
}

QgsVectorFieldSettings QgsVectorFieldSettingsWidget::settings() const
{
  QgsVectorFieldSettings settings;
  settings.setSymbology( currentSymbology() );

  //Arrow settings
  QgsVectorFieldArrowSettings arrowSettings;

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
  auto method = static_cast<Qgis::VectorFieldArrowScalingMethod>( mShaftLengthComboBox->currentIndex() );
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
  QgsVectorFieldStreamlineSettings streamlineSettings;
  streamlineSettings.setSeedingMethod( static_cast<Qgis::VectorFieldSeedingMethod>( mStreamlinesSeedingMethodComboBox->currentIndex() ) );

  streamlineSettings.setSeedingDensity( mStreamlinesDensitySpinBox->value() / 100 );

  settings.setStreamLinesSettings( streamlineSettings );

  //Traces setting
  QgsVectorFieldTracesSettings tracesSettings;
  tracesSettings.setMaximumTailLength( mTracesMaxLengthSpinBox->value() );
  tracesSettings.setMaximumTailLengthUnit( mTracesTailLengthMapUnitWidget->unit() );
  tracesSettings.setParticlesCount( mTracesParticlesCountSpinBox->value() );
  settings.setTracesSettings( tracesSettings );

  // Wind Barb settings
  QgsVectorFieldWindBarbSettings windBarbSettings;
  windBarbSettings.setShaftLength( mWindBarbLengthSpinBox->value() );
  windBarbSettings.setShaftLengthUnits( mWindBarbLengthMapUnitWidget->unit() );
  windBarbSettings.setMagnitudeUnits( static_cast<Qgis::WindSpeedUnit>( mWindBarbUnitsComboBox->currentIndex() ) );
  windBarbSettings.setMagnitudeMultiplier( mWindBarbMagnitudeMultiplierSpinBox->value() );
  settings.setWindBarbSettings( windBarbSettings );

  return settings;
}

void QgsVectorFieldSettingsWidget::setSupportsInterpolation( bool supported )
{
  mSupportsInterpolation = supported;
  populateSymbologies();
}

void QgsVectorFieldSettingsWidget::setHasMagnitude( bool hasMagnitude )
{
  mHasMagnitude = hasMagnitude;
}

void QgsVectorFieldSettingsWidget::setMagnitudeRange( double minimum, double maximum )
{
  mMagnitudeMinimum = minimum;
  mMagnitudeMaximum = maximum;
  mHasMagnitudeRange = true;
}

void QgsVectorFieldSettingsWidget::setSettings( const QgsVectorFieldSettings &settings )
{
  const int symbologyIndex = mSymbologyVectorComboBox->findData( static_cast< int >( settings.symbology() ) );
  // a symbology the data cannot be drawn with is not offered, fall back to arrows
  mSymbologyVectorComboBox->setCurrentIndex( symbologyIndex >= 0 ? symbologyIndex : mSymbologyVectorComboBox->findData( static_cast< int >( Qgis::VectorFieldSymbology::Arrows ) ) );

  // Arrow settings
  const QgsVectorFieldArrowSettings arrowSettings = settings.arrowSettings();

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
  mDisplayVectorsOnGridGroupBox->setVisible( mSupportsInterpolation );
  mDisplayVectorsOnGridGroupBox->setChecked( settings.isOnUserDefinedGrid() && mSupportsInterpolation );
  mXSpacingSpinBox->setValue( settings.userGridCellWidth() );
  mYSpacingSpinBox->setValue( settings.userGridCellHeight() );

  // shaft length
  mShaftLengthComboBox->setCurrentIndex( static_cast< int >( arrowSettings.shaftLengthMethod() ) );

  mMinimumShaftSpinBox->setValue( arrowSettings.minShaftLength() );
  mMaximumShaftSpinBox->setValue( arrowSettings.maxShaftLength() );
  mScaleShaftByFactorOfSpinBox->setValue( arrowSettings.scaleFactor() );
  mShaftLengthSpinBox->setValue( arrowSettings.fixedShaftLength() );

  //Streamlines settings
  const QgsVectorFieldStreamlineSettings streamlinesSettings = settings.streamLinesSettings();

  mStreamlinesSeedingMethodComboBox->setCurrentIndex( static_cast< int >( streamlinesSettings.seedingMethod() ) );
  mStreamlinesDensitySpinBox->setValue( streamlinesSettings.seedingDensity() * 100 );

  //Traces settings
  const QgsVectorFieldTracesSettings tracesSettings = settings.tracesSettings();

  mTracesMaxLengthSpinBox->setValue( tracesSettings.maximumTailLength() );
  mTracesTailLengthMapUnitWidget->setUnit( tracesSettings.maximumTailLengthUnit() );
  mTracesParticlesCountSpinBox->setValue( tracesSettings.particlesCount() );

  // Wind Barb settings
  const QgsVectorFieldWindBarbSettings windBarbSettings = settings.windBarbSettings();
  mWindBarbLengthSpinBox->setValue( windBarbSettings.shaftLength() );
  mWindBarbUnitsComboBox->setCurrentIndex( static_cast<int>( windBarbSettings.magnitudeUnits() ) );
  if ( windBarbSettings.magnitudeUnits() == Qgis::WindSpeedUnit::OtherUnit )
    mWindBarbMagnitudeMultiplierSpinBox->setValue( windBarbSettings.magnitudeMultiplier() );

  applyMagnitudeSupport();
}

void QgsVectorFieldSettingsWidget::applyMagnitudeSupport()
{
  // a filter on the magnitude of vectors which all have a magnitude of one either passes everything
  // or rejects everything, so it is hidden and cleared rather than left to confuse
  const bool isTraces = currentSymbology() == Qgis::VectorFieldSymbology::Traces;
  const bool showMagnitudeFilter = mHasMagnitude && !isTraces;
  mMagnitudeFilterGroupBox->setVisible( showMagnitudeFilter );

  // the color ramp classifies the magnitude, and only a fixed shaft length can draw vectors which
  // all have the same one
  mColoringMethodLabel->setVisible( mHasMagnitude );
  mColoringMethodComboBox->setVisible( mHasMagnitude );
  mShaftLengthComboBox->setEnabled( mHasMagnitude );

  if ( !mHasMagnitude )
  {
    mMinMagSpinBox->clear();
    mMaxMagSpinBox->clear();
    mColoringMethodComboBox->setCurrentIndex( mColoringMethodComboBox->findData( QgsInterpolatedLineColor::SingleColor ) );
    mShaftLengthComboBox->setCurrentIndex( static_cast< int >( Qgis::VectorFieldArrowScalingMethod::Fixed ) );
  }
}

void QgsVectorFieldSettingsWidget::onSymbologyChanged()
{
  const Qgis::VectorFieldSymbology symbology = currentSymbology();

  mStreamlineWidget->setVisible( symbology == Qgis::VectorFieldSymbology::Streamlines );
  mArrowLengthGroupBox->setVisible( symbology == Qgis::VectorFieldSymbology::Arrows );
  mHeadOptionsGroupBox->setVisible( symbology == Qgis::VectorFieldSymbology::Arrows );
  mTracesGroupBox->setVisible( symbology == Qgis::VectorFieldSymbology::Traces );
  mWindBarbGroupBox->setVisible( symbology == Qgis::VectorFieldSymbology::WindBarbs );

  // the grid places vectors between the data points, which needs a field which can be interpolated
  mDisplayVectorsOnGridGroupBox->setVisible( mSupportsInterpolation && symbology != Qgis::VectorFieldSymbology::Traces );
  applyMagnitudeSupport();

  mDisplayVectorsOnGridGroupBox->setEnabled(
    symbology == Qgis::VectorFieldSymbology::Arrows
    || symbology == Qgis::VectorFieldSymbology::WindBarbs
    || ( symbology == Qgis::VectorFieldSymbology::Streamlines && mStreamlinesSeedingMethodComboBox->currentIndex() == static_cast< int >( Qgis::VectorFieldSeedingMethod::Gridded ) )
  );
}

void QgsVectorFieldSettingsWidget::populateSymbologies()
{
  const QVariant previous = mSymbologyVectorComboBox->currentData();

  const QSignalBlocker blocker( mSymbologyVectorComboBox );
  mSymbologyVectorComboBox->clear();

  // one glyph per data point, which any vector field can place
  mSymbologyVectorComboBox->addItem( tr( "Arrows" ), static_cast< int >( Qgis::VectorFieldSymbology::Arrows ) );
  if ( mSupportsInterpolation )
  {
    // both walk the field between its data points
    mSymbologyVectorComboBox->addItem( tr( "Streamlines" ), static_cast< int >( Qgis::VectorFieldSymbology::Streamlines ) );
    mSymbologyVectorComboBox->addItem( tr( "Traces" ), static_cast< int >( Qgis::VectorFieldSymbology::Traces ) );
  }
  mSymbologyVectorComboBox->addItem( tr( "Wind Barbs" ), static_cast< int >( Qgis::VectorFieldSymbology::WindBarbs ) );

  const int previousIndex = previous.isValid() ? mSymbologyVectorComboBox->findData( previous ) : -1;
  mSymbologyVectorComboBox->setCurrentIndex( previousIndex >= 0 ? previousIndex : 0 );
}

Qgis::VectorFieldSymbology QgsVectorFieldSettingsWidget::currentSymbology() const
{
  return static_cast< Qgis::VectorFieldSymbology >( mSymbologyVectorComboBox->currentData().toInt() );
}

void QgsVectorFieldSettingsWidget::onStreamLineSeedingMethodChanged( int currentIndex )
{
  bool enabled = currentIndex == static_cast< int >( Qgis::VectorFieldSeedingMethod::Random );
  mStreamlinesDensityLabel->setEnabled( enabled );
  mStreamlinesDensitySpinBox->setEnabled( enabled );

  mDisplayVectorsOnGridGroupBox->setEnabled( !enabled );
}

void QgsVectorFieldSettingsWidget::onWindBarbUnitsChanged( int currentIndex )
{
  const Qgis::WindSpeedUnit units = static_cast<Qgis::WindSpeedUnit>( currentIndex );

  mWindBarbMagnitudeMultiplierLabel->setVisible( units == Qgis::WindSpeedUnit::OtherUnit );
  mWindBarbMagnitudeMultiplierSpinBox->setVisible( units == Qgis::WindSpeedUnit::OtherUnit );

  emit widgetChanged();
}

void QgsVectorFieldSettingsWidget::onColoringMethodChanged()
{
  mColorRampShaderGroupBox->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::ColorRamp );
  mColorWidget->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::SingleColor );
  mSingleColorLabel->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::SingleColor );

  if ( mColorRampShaderWidget->shader().colorRampItemList().isEmpty() )
    loadColorRampShader();

  emit widgetChanged();
}

void QgsVectorFieldSettingsWidget::onColorRampMinMaxChanged()
{
  mColorRampShaderWidget->setMinimumMaximumAndClassify( filterValue( mColorRampShaderMinimumSpinBox, 0 ), filterValue( mColorRampShaderMaximumSpinBox, 0 ) );
}

void QgsVectorFieldSettingsWidget::loadColorRampShader()
{
  if ( !mHasMagnitudeRange )
    return;

  mColorRampShaderWidget->setMinimumMaximumAndClassify( mMagnitudeMinimum, mMagnitudeMaximum );
  whileBlocking( mColorRampShaderMinimumSpinBox )->setValue( mMagnitudeMinimum );
  whileBlocking( mColorRampShaderMaximumSpinBox )->setValue( mMagnitudeMaximum );
}

double QgsVectorFieldSettingsWidget::filterValue( const QgsDoubleSpinBox *spinBox, double errVal ) const
{
  if ( spinBox->value() == spinBox->clearValue() )
    return errVal;

  return spinBox->value();
}
