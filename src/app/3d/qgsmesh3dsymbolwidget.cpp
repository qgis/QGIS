/***************************************************************************
  qgsmesh3dsymbolwidget.cpp
  -------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dsymbolwidget.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsmeshdataprovider.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsmeshdatasetgrouptreeview.h"

QgsMesh3dSymbolWidget::QgsMesh3dSymbolWidget( QgsMeshLayer *meshLayer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  mSpinBoxWireframeLineWidth->setClearValue( 1.0 );
  mSpinBoxVerticaleScale->setClearValue( 1.0 );
  mArrowsSpacingSpinBox->setClearValue( 25.0 );

  mComboBoxTextureType->addItem( tr( "Single Color" ), QgsMesh3DSymbol::SingleColor );
  mComboBoxTextureType->setCurrentIndex( 0 );

  mDatasetGroupListModel = new QgsMeshDatasetGroupListModel( this );
  mComboBoxDatasetVertical->setModel( mDatasetGroupListModel );
  setLayer( meshLayer );

  connect( mChkSmoothTriangles, &QCheckBox::clicked, this, &QgsMesh3dSymbolWidget::changed );
  connect( mGroupBoxWireframe, &QGroupBox::toggled, this, &QgsMesh3dSymbolWidget::changed );
  connect( mColorButtonWireframe, &QgsColorButton::colorChanged, this, &QgsMesh3dSymbolWidget::changed );
  connect( mSpinBoxWireframeLineWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3dSymbolWidget::changed );
  connect( mLodSlider, &QSlider::valueChanged, this, &QgsMesh3dSymbolWidget::changed );

  connect( mColorRampShaderMinMaxReloadButton, &QPushButton::clicked, this, &QgsMesh3dSymbolWidget::reloadColorRampShaderMinMax );
  connect( mColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMesh3dSymbolWidget::changed );

  connect( mColorRampShaderMinEdit, &QLineEdit::editingFinished, this, &QgsMesh3dSymbolWidget::onColorRampShaderMinMaxChanged );
  connect( mColorRampShaderMaxEdit, &QLineEdit::editingFinished, this, &QgsMesh3dSymbolWidget::onColorRampShaderMinMaxChanged );

  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolWidget::onColoringTypeChanged );
  connect( mComboBoxTextureType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolWidget::changed );

  connect( mMeshSingleColorButton, &QgsColorButton::colorChanged, this, &QgsMesh3dSymbolWidget::changed );

  connect( mSpinBoxVerticaleScale, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ),
           this, &QgsMesh3dSymbolWidget::changed );

  connect( mComboBoxDatasetVertical, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsMesh3dSymbolWidget::changed );

  connect( mCheckBoxVerticalMagnitudeRelative, &QCheckBox::clicked, this, &QgsMesh3dSymbolWidget::changed );

  connect( mGroupBoxArrowsSettings, &QGroupBox::toggled, this, &QgsMesh3dSymbolWidget::changed );

  connect( mArrowsSpacingSpinBox, static_cast<void ( QDoubleSpinBox::* )()>( &QDoubleSpinBox::editingFinished ),
           this, &QgsMesh3dSymbolWidget::changed );

  connect( mArrowsFixedSizeCheckBox, &QCheckBox::clicked, this, &QgsMesh3dSymbolWidget::changed );

  connect( mGroupBoxTextureSettings, &QgsCollapsibleGroupBox::collapsedStateChanged, this,  &QgsMesh3dSymbolWidget::onTextureSettingsCollapseStateChanged );
}

void QgsMesh3dSymbolWidget::setSymbol( const QgsMesh3DSymbol *symbol )
{
  mSymbol.reset( symbol->clone() );
  mChkSmoothTriangles->setChecked( symbol->smoothedTriangles() );
  mGroupBoxWireframe->setChecked( symbol->wireframeEnabled() );
  mColorButtonWireframe->setColor( symbol->wireframeLineColor() );
  mSpinBoxWireframeLineWidth->setValue( symbol->wireframeLineWidth() );
  if ( mLayer && mLayer->meshSimplificationSettings().isEnabled() )
    mLodSlider->setValue( mLayer->triangularMeshLevelOfDetailCount() - symbol->levelOfDetailIndex() - 1 );
  else
    mLodSlider->setValue( mLodSlider->maximum() );


  mSpinBoxVerticaleScale->setValue( symbol->verticalScale() );
  mComboBoxTextureType->setCurrentIndex( mComboBoxTextureType->findData( symbol->renderingStyle() ) );
  mMeshSingleColorButton->setColor( symbol->singleMeshColor() );
  mColorRampShaderWidget->setFromShader( symbol->colorRampShader() );
  mColorRampShaderWidget->setMinimumMaximumAndClassify( symbol->colorRampShader().minimumValue(),
      symbol->colorRampShader().maximumValue() );

  setColorRampMinMax( symbol->colorRampShader().minimumValue(), symbol->colorRampShader().maximumValue() );
  mComboBoxDatasetVertical->setCurrentIndex( symbol->verticalDatasetGroupIndex() );
  mCheckBoxVerticalMagnitudeRelative->setChecked( symbol->isVerticalMagnitudeRelative() );

  mGroupBoxArrowsSettings->setChecked( symbol->arrowsEnabled() );
  mArrowsSpacingSpinBox->setValue( symbol->arrowsSpacing() );
  mArrowsFixedSizeCheckBox->setChecked( symbol->arrowsFixedSize() );
}

void QgsMesh3dSymbolWidget::configureForTerrain()
{
  mComboBoxTextureType->addItem( tr( "Color Ramp Shader" ), QgsMesh3DSymbol::ColorRamp );
  enableVerticalSetting( false );
  enableArrowSettings( false );

  onColoringTypeChanged();
}

void QgsMesh3dSymbolWidget::configureForDataset()
{
  mComboBoxTextureType->addItem( tr( "2D Contour Color Ramp Shader" ), QgsMesh3DSymbol::ColorRamp2DRendering );
  mGroupBoxColorRampShader->hide();
  enableVerticalSetting( true );
  enableArrowSettings( true );

  onColoringTypeChanged();
}

void QgsMesh3dSymbolWidget::setLayer( QgsMeshLayer *meshLayer, bool updateSymbol )
{
  mLayer = meshLayer;

  if ( meshLayer && meshLayer->meshSimplificationSettings().isEnabled() )
  {
    mLodSlider->setEnabled( true );
    const int lodCount = meshLayer->triangularMeshLevelOfDetailCount();
    mLodSlider->setTickInterval( 1 );
    mLodSlider->setMaximum( lodCount - 1 );
  }
  else
  {
    mLodSlider->setValue( mLodSlider->maximum() );
    mLodSlider->setEnabled( false );
  }

  if ( !updateSymbol )
    return;

  if ( meshLayer )
  {
    mDatasetGroupListModel->syncToLayer( meshLayer );
    QgsMeshLayer3DRenderer *renderer = static_cast<QgsMeshLayer3DRenderer *>( meshLayer->renderer3D() );
    if ( renderer &&  renderer->type() == QLatin1String( "mesh" ) )
    {
      if ( renderer->symbol() &&  renderer->symbol()->type() == QLatin1String( "mesh" ) )
      {
        setSymbol( static_cast<const QgsMesh3DSymbol *>( renderer->symbol() ) );
        return;
      }
    }
  }

  const std::unique_ptr< QgsMesh3DSymbol > defaultSymbol = std::make_unique< QgsMesh3DSymbol >();
  // set symbol does not take ownership!
  setSymbol( defaultSymbol.get() );

  reloadColorRampShaderMinMax(); //As the symbol is new, the Color ramp shader needs to be initialized with min max value
}

QgsMeshLayer *QgsMesh3dSymbolWidget::meshLayer() const {return mLayer;}

double QgsMesh3dSymbolWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

std::unique_ptr<QgsMesh3DSymbol> QgsMesh3dSymbolWidget::symbol() const
{
  std::unique_ptr< QgsMesh3DSymbol > sym( mSymbol->clone() );

  sym->setSmoothedTriangles( mChkSmoothTriangles->isChecked() );
  sym->setWireframeEnabled( mGroupBoxWireframe->isChecked() );
  sym->setWireframeLineColor( mColorButtonWireframe->color() );
  sym->setWireframeLineWidth( mSpinBoxWireframeLineWidth->value() );
  if ( mLayer )
    sym->setLevelOfDetailIndex( mLayer->triangularMeshLevelOfDetailCount() - mLodSlider->sliderPosition() - 1 );
  else
    sym->setLevelOfDetailIndex( 0 );

  sym->setVerticalScale( mSpinBoxVerticaleScale->value() );
  sym->setRenderingStyle( static_cast<QgsMesh3DSymbol::RenderingStyle>( mComboBoxTextureType->currentData().toInt() ) );
  sym->setSingleMeshColor( mMeshSingleColorButton->color() );
  sym->setVerticalDatasetGroupIndex( mComboBoxDatasetVertical->currentIndex() );
  sym->setIsVerticalMagnitudeRelative( mCheckBoxVerticalMagnitudeRelative->isChecked() );

  if ( sym->renderingStyle() == QgsMesh3DSymbol::ColorRamp )
    sym->setColorRampShader( mColorRampShaderWidget->shader() );

  sym->setArrowsEnabled( mGroupBoxArrowsSettings->isChecked() );
  sym->setArrowsSpacing( mArrowsSpacingSpinBox->value() );
  sym->setArrowsFixedSize( mArrowsFixedSizeCheckBox->isChecked() );

  return sym;
}

void QgsMesh3dSymbolWidget::reloadColorRampShaderMinMax()
{
  if ( !mLayer )
    return;

  QgsTriangularMesh *triangleMesh = mLayer->triangularMesh();
  if ( !triangleMesh )
    return;

  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::lowest();
  for ( int i = 0; i < triangleMesh->vertices().count(); ++i )
  {
    const double zValue = triangleMesh->vertices().at( i ).z();
    if ( zValue > max )
      max = zValue;
    if ( zValue < min )
      min = zValue;
  }
  setColorRampMinMax( min, max );

  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3dSymbolWidget::onColorRampShaderMinMaxChanged()
{
  const double min = lineEditValue( mColorRampShaderMinEdit );
  const double max = lineEditValue( mColorRampShaderMaxEdit );
  mColorRampShaderWidget->setMinimumMaximum( min, max );
  mColorRampShaderWidget->classify();
}

void QgsMesh3dSymbolWidget::onColoringTypeChanged()
{
  mGroupBoxColorRampShader->setVisible( mComboBoxTextureType->currentData() == QgsMesh3DSymbol::ColorRamp );
  mMeshSingleColorWidget->setVisible( mComboBoxTextureType->currentData()  == QgsMesh3DSymbol::SingleColor );
}

void QgsMesh3dSymbolWidget::onTextureSettingsCollapseStateChanged( bool collapsed )
{
  if ( !collapsed )
  {
    onColoringTypeChanged();
  }
}

void QgsMesh3dSymbolWidget::setColorRampMinMax( double min, double max )
{
  whileBlocking( mColorRampShaderMinEdit )->setText( QString::number( min ) );
  whileBlocking( mColorRampShaderMaxEdit )->setText( QString::number( max ) );
}

void QgsMesh3dSymbolWidget::enableVerticalSetting( bool isEnable )
{
  mGroupBoxVerticaleSettings->setVisible( isEnable );
}

void QgsMesh3dSymbolWidget::enableArrowSettings( bool isEnable )
{
  mGroupBoxArrowsSettings->setVisible( isEnable );
}


