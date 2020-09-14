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

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"

QgsMeshRendererVectorSettingsWidget::QgsMeshRendererVectorSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mShaftLengthComboBox->setCurrentIndex( -1 );

  mColoringMethodComboBox->addItem( tr( "Single Color" ), QgsInterpolatedLineColor::SingleColor );
  mColoringMethodComboBox->addItem( tr( "Color Ramp Shader" ), QgsInterpolatedLineColor::ColorRamp );

  connect( mColorWidget, &QgsColorButton::colorChanged, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mColoringMethodComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::onColoringMethodChanged );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged,
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mColorRampShaderMinimumEditLine, &QLineEdit::textEdited,
           this, &QgsMeshRendererVectorSettingsWidget::onColorRampMinMaxChanged );
  connect( mColorRampShaderMaximumEditLine, &QLineEdit::textEdited,
           this, &QgsMeshRendererVectorSettingsWidget::onColorRampMinMaxChanged );

  connect( mLineWidthSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mShaftLengthComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           mShaftOptionsStackedWidget, &QStackedWidget::setCurrentIndex );

  connect( mDisplayVectorsOnGridGroupBox, &QGroupBox::toggled, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mColorRampShaderLoadButton, &QPushButton::clicked, this, &QgsMeshRendererVectorSettingsWidget::loadColorRampShader );

  onColoringMethodChanged();

  QVector<QLineEdit *> widgets;
  widgets << mMinMagLineEdit << mMaxMagLineEdit
          << mHeadWidthLineEdit << mHeadLengthLineEdit
          << mMinimumShaftLineEdit << mMaximumShaftLineEdit
          << mScaleShaftByFactorOfLineEdit << mShaftLengthLineEdit;

  for ( auto widget : widgets )
  {
    connect( widget, &QLineEdit::textChanged, this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  }

  connect( mXSpacingSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
  connect( mYSpacingSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ), this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mSymbologyVectorComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::onSymbologyChanged );
  onSymbologyChanged( 0 );

  connect( mSymbologyVectorComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mStreamlinesSeedingMethodComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::onStreamLineSeedingMethodChanged );
  onStreamLineSeedingMethodChanged( 0 );

  connect( mStreamlinesSeedingMethodComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mStreamlinesDensitySpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mTracesMaxLengthSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  connect( mTracesParticlesCountSpinBox, qgis::overload<int>::of( &QgsSpinBox::valueChanged ),
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );

  mTracesTailLengthMapUnitWidget->setUnits( QgsUnitTypes::RenderUnitList()
      << QgsUnitTypes::RenderMillimeters
      << QgsUnitTypes::RenderMetersInMapUnits
      << QgsUnitTypes::RenderPixels
      << QgsUnitTypes::RenderPoints );

  connect( mTracesTailLengthMapUnitWidget, &QgsUnitSelectionWidget::changed,
           this, &QgsMeshRendererVectorSettingsWidget::widgetChanged );
}

void QgsMeshRendererVectorSettingsWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

QgsMeshRendererVectorSettings QgsMeshRendererVectorSettingsWidget::settings() const
{
  QgsMeshRendererVectorSettings  settings;
  settings.setSymbology(
    static_cast<QgsMeshRendererVectorSettings::Symbology>( mSymbologyVectorComboBox->currentIndex() ) );

  //Arrow settings
  QgsMeshRendererVectorArrowSettings arrowSettings;

  // basic
  settings.setColor( mColorWidget->color() );
  settings.setLineWidth( mLineWidthSpinBox->value() );
  settings.setColoringMethod( static_cast<QgsInterpolatedLineColor::ColoringMethod>
                              ( mColoringMethodComboBox->currentData().toInt() ) );
  settings.setColorRampShader( mColorRampShaderWidget->shader() );

  // filter by magnitude
  double val = filterValue( mMinMagLineEdit->text(), -1 );
  settings.setFilterMin( val );

  val = filterValue( mMaxMagLineEdit->text(), -1 );
  settings.setFilterMax( val );

  // arrow head
  val = filterValue( mHeadWidthLineEdit->text(), arrowSettings.arrowHeadWidthRatio() * 100.0 );
  arrowSettings.setArrowHeadWidthRatio( val / 100.0 );

  val = filterValue( mHeadLengthLineEdit->text(), arrowSettings.arrowHeadLengthRatio() * 100.0 );
  arrowSettings.setArrowHeadLengthRatio( val / 100.0 );

  // user grid
  bool enabled = mDisplayVectorsOnGridGroupBox->isChecked();
  settings.setOnUserDefinedGrid( enabled );
  settings.setUserGridCellWidth( mXSpacingSpinBox->value() );
  settings.setUserGridCellHeight( mYSpacingSpinBox->value() );

  // shaft length
  auto method = static_cast<QgsMeshRendererVectorArrowSettings::ArrowScalingMethod>( mShaftLengthComboBox->currentIndex() );
  arrowSettings.setShaftLengthMethod( method );

  val = filterValue( mMinimumShaftLineEdit->text(), arrowSettings.minShaftLength() );
  arrowSettings.setMinShaftLength( val );

  val = filterValue( mMaximumShaftLineEdit->text(), arrowSettings.maxShaftLength() );
  arrowSettings.setMaxShaftLength( val );

  val = filterValue( mScaleShaftByFactorOfLineEdit->text(), arrowSettings.scaleFactor() );
  arrowSettings.setScaleFactor( val );

  val = filterValue( mShaftLengthLineEdit->text(), arrowSettings.fixedShaftLength() );
  arrowSettings.setFixedShaftLength( val );

  settings.setArrowsSettings( arrowSettings );

  //Streamline setting
  QgsMeshRendererVectorStreamlineSettings streamlineSettings;
  streamlineSettings.setSeedingMethod(
    static_cast<QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod>( mStreamlinesSeedingMethodComboBox->currentIndex() ) );

  streamlineSettings.setSeedingDensity( mStreamlinesDensitySpinBox->value() / 100 );

  settings.setStreamLinesSettings( streamlineSettings );

  //Traces setting
  QgsMeshRendererVectorTracesSettings tracesSettings;
  tracesSettings.setMaximumTailLength( mTracesMaxLengthSpinBox->value() );
  tracesSettings.setMaximumTailLengthUnit( mTracesTailLengthMapUnitWidget->unit() );
  tracesSettings.setParticlesCount( mTracesParticlesCountSpinBox->value() );
  settings.setTracesSettings( tracesSettings );

  return settings;
}

void QgsMeshRendererVectorSettingsWidget::syncToLayer( )
{
  if ( !mMeshLayer && !mMeshLayer->dataProvider() )
    return;

  if ( mActiveDatasetGroup < 0 )
    return;

  bool hasFaces = ( mMeshLayer->dataProvider() &&
                    mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Face ) );

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMeshRendererVectorSettings settings = rendererSettings.vectorSettings( mActiveDatasetGroup );

  mSymbologyGroupBox->setVisible( hasFaces );
  mSymbologyVectorComboBox->setCurrentIndex( hasFaces ? settings.symbology() : 0 );

  // Arrow settings
  const QgsMeshRendererVectorArrowSettings arrowSettings = settings.arrowSettings();

  // basic
  mColorWidget->setColor( settings.color() );
  mLineWidthSpinBox->setValue( settings.lineWidth() );
  mColoringMethodComboBox->setCurrentIndex( mColoringMethodComboBox->findData( settings.coloringMethod() ) );
  mColorRampShaderWidget->setFromShader( settings.colorRampShader() );
  mColorRampShaderMinimumEditLine->setText( QString::number( settings.colorRampShader().minimumValue() ) );
  mColorRampShaderMaximumEditLine->setText( QString::number( settings.colorRampShader().maximumValue() ) );

  // filter by magnitude
  if ( settings.filterMin() > 0 )
  {
    mMinMagLineEdit->setText( QString::number( settings.filterMin() ) );
  }
  if ( settings.filterMax() > 0 )
  {
    mMaxMagLineEdit->setText( QString::number( settings.filterMax() ) );
  }

  // arrow head
  mHeadWidthLineEdit->setText( QString::number( arrowSettings.arrowHeadWidthRatio() * 100.0 ) );
  mHeadLengthLineEdit->setText( QString::number( arrowSettings.arrowHeadLengthRatio() * 100.0 ) );

  // user grid
  mDisplayVectorsOnGridGroupBox->setVisible( hasFaces );
  mDisplayVectorsOnGridGroupBox->setChecked( settings.isOnUserDefinedGrid() && hasFaces );
  mXSpacingSpinBox->setValue( settings.userGridCellWidth() );
  mYSpacingSpinBox->setValue( settings.userGridCellHeight() );

  // shaft length
  mShaftLengthComboBox->setCurrentIndex( arrowSettings.shaftLengthMethod() );

  mMinimumShaftLineEdit->setText( QString::number( arrowSettings.minShaftLength() ) );
  mMaximumShaftLineEdit->setText( QString::number( arrowSettings.maxShaftLength() ) );
  mScaleShaftByFactorOfLineEdit->setText( QString::number( arrowSettings.scaleFactor() ) );
  mShaftLengthLineEdit->setText( QString::number( arrowSettings.fixedShaftLength() ) );

  //Streamlines settings
  const QgsMeshRendererVectorStreamlineSettings streamlinesSettings = settings.streamLinesSettings();

  mStreamlinesSeedingMethodComboBox->setCurrentIndex( streamlinesSettings.seedingMethod() );
  mStreamlinesDensitySpinBox->setValue( streamlinesSettings.seedingDensity() * 100 );

  //Traces settings
  const QgsMeshRendererVectorTracesSettings tracesSettings = settings.tracesSettings();

  mTracesMaxLengthSpinBox->setValue( tracesSettings.maximumTailLength() );
  mTracesTailLengthMapUnitWidget->setUnit( tracesSettings.maximumTailLengthUnit() );
  mTracesParticlesCountSpinBox->setValue( tracesSettings.particlesCount() );

}

void QgsMeshRendererVectorSettingsWidget::onSymbologyChanged( int currentIndex )
{
  mStreamlineWidget->setVisible( currentIndex == QgsMeshRendererVectorSettings::Streamlines );
  mArrowWidget->setVisible( currentIndex == QgsMeshRendererVectorSettings::Arrows );
  mTracesGroupBox->setVisible( currentIndex == QgsMeshRendererVectorSettings::Traces );

  mDisplayVectorsOnGridGroupBox->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );
  mFilterByMagGroupBox->setVisible( currentIndex != QgsMeshRendererVectorSettings::Traces );

  mDisplayVectorsOnGridGroupBox->setEnabled(
    currentIndex == QgsMeshRendererVectorSettings::Arrows ||
    ( currentIndex == QgsMeshRendererVectorSettings::Streamlines &&
      mStreamlinesSeedingMethodComboBox->currentIndex() == QgsMeshRendererVectorStreamlineSettings::MeshGridded ) ) ;
}

void QgsMeshRendererVectorSettingsWidget::onStreamLineSeedingMethodChanged( int currentIndex )
{
  bool enabled = currentIndex == QgsMeshRendererVectorStreamlineSettings::Random;
  mStreamlinesDensityLabel->setEnabled( enabled );
  mStreamlinesDensitySpinBox->setEnabled( enabled );

  mDisplayVectorsOnGridGroupBox->setEnabled( !enabled );
}

void QgsMeshRendererVectorSettingsWidget::onColoringMethodChanged()
{
  mColorRampShaderGroupBox->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::ColorRamp );
  mColorWidget->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::SingleColor );
  mSimgleColorLabel->setVisible( mColoringMethodComboBox->currentData() == QgsInterpolatedLineColor::SingleColor );

  if ( mColorRampShaderWidget->shader().colorRampItemList().isEmpty() )
    loadColorRampShader();

  emit widgetChanged();
}

void QgsMeshRendererVectorSettingsWidget::onColorRampMinMaxChanged()
{
  mColorRampShaderWidget->setMinimumMaximumAndClassify(
    filterValue( mColorRampShaderMinimumEditLine->text(), 0 ),
    filterValue( mColorRampShaderMaximumEditLine->text(), 0 ) );
}

void QgsMeshRendererVectorSettingsWidget::loadColorRampShader()
{
  if ( !mMeshLayer )
    return;

  int currentVectorDataSetGroupIndex = mMeshLayer->rendererSettings().activeVectorDatasetGroup();
  if ( currentVectorDataSetGroupIndex < 0 ||
       !mMeshLayer->datasetGroupMetadata( currentVectorDataSetGroupIndex ).isVector() )
    return;

  const QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( currentVectorDataSetGroupIndex );
  double min = meta.minimum();
  double max = meta.maximum();

  mColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
  whileBlocking( mColorRampShaderMinimumEditLine )->setText( QString::number( min ) );
  whileBlocking( mColorRampShaderMaximumEditLine )->setText( QString::number( max ) );
}

double QgsMeshRendererVectorSettingsWidget::filterValue( const QString &text, double errVal ) const
{
  if ( text.isEmpty() )
    return errVal;

  bool ok;
  double val = text.toDouble( &ok );
  if ( !ok )
    return errVal;

  if ( val < 0 )
    return errVal;

  return val;
}
