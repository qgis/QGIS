/***************************************************************************
    qgsmeshrendererscalarsettingswidget.cpp
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

#include "qgsmeshrendererscalarsettingswidget.h"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmessagelog.h"

QgsMeshRendererScalarSettingsWidget::QgsMeshRendererScalarSettingsWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  // add items to data interpolation combo box
  mScalarInterpolationTypeComboBox->addItem( tr( "None" ), QgsMeshRendererScalarSettings::None );
  mScalarInterpolationTypeComboBox->addItem( tr( "Neighbour Average" ), QgsMeshRendererScalarSettings::NeighbourAverage );
  mScalarInterpolationTypeComboBox->setCurrentIndex( 0 );

  mScalarEdgeWidthUnitSelectionWidget->setUnits( QgsUnitTypes::RenderUnitList()
      << QgsUnitTypes::RenderMillimeters
      << QgsUnitTypes::RenderMetersInMapUnits
      << QgsUnitTypes::RenderPixels
      << QgsUnitTypes::RenderPoints );

  // connect
  connect( mScalarRecalculateMinMaxButton, &QPushButton::clicked, this, &QgsMeshRendererScalarSettingsWidget::recalculateMinMaxButtonClicked );
  connect( mScalarMinLineEdit, &QLineEdit::textChanged, this, &QgsMeshRendererScalarSettingsWidget::minMaxChanged );
  connect( mScalarMaxLineEdit, &QLineEdit::textChanged, this, &QgsMeshRendererScalarSettingsWidget::minMaxChanged );
  connect( mScalarMinLineEdit, &QLineEdit::textEdited, this, &QgsMeshRendererScalarSettingsWidget::minMaxEdited );
  connect( mScalarMaxLineEdit, &QLineEdit::textEdited, this, &QgsMeshRendererScalarSettingsWidget::minMaxEdited );
  connect( mScalarColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarInterpolationTypeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarEdgeWidthUnitSelectionWidget, &QgsUnitSelectionWidget::changed,
           this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarEdgeWidthSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
}

void QgsMeshRendererScalarSettingsWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
  mScalarInterpolationTypeComboBox->setEnabled( !dataIsDefinedOnEdges() );
}

void QgsMeshRendererScalarSettingsWidget::setActiveDatasetGroup( int groupIndex )
{
  mActiveDatasetGroup = groupIndex;
  mScalarInterpolationTypeComboBox->setEnabled( !dataIsDefinedOnEdges() );
}

QgsMeshRendererScalarSettings QgsMeshRendererScalarSettingsWidget::settings() const
{
  QgsMeshRendererScalarSettings settings;
  settings.setColorRampShader( mScalarColorRampShaderWidget->shader() );
  settings.setClassificationMinimumMaximum( lineEditValue( mScalarMinLineEdit ), lineEditValue( mScalarMaxLineEdit ) );
  settings.setOpacity( mOpacityWidget->opacity() );
  settings.setDataResamplingMethod( dataIntepolationMethod() );
  settings.setEdgeWidth( mScalarEdgeWidthSpinBox->value() );
  settings.setEdgeWidthUnit( mScalarEdgeWidthUnitSelectionWidget->unit() );
  return settings;
}

void QgsMeshRendererScalarSettingsWidget::syncToLayer( )
{
  if ( !mMeshLayer )
    return;

  if ( mActiveDatasetGroup < 0 )
    return;

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
  const QgsMeshRendererScalarSettings settings = rendererSettings.scalarSettings( mActiveDatasetGroup );
  const QgsColorRampShader shader = settings.colorRampShader();
  const double min = settings.classificationMinimum();
  const double max = settings.classificationMaximum();

  whileBlocking( mScalarEdgeWidthSpinBox )->setValue( settings.edgeWidth() );
  whileBlocking( mScalarEdgeWidthUnitSelectionWidget )->setUnit( settings.edgeWidthUnit() );

  whileBlocking( mScalarMinLineEdit )->setText( QString::number( min ) );
  whileBlocking( mScalarMaxLineEdit )->setText( QString::number( max ) );
  whileBlocking( mScalarColorRampShaderWidget )->setFromShader( shader );
  whileBlocking( mScalarColorRampShaderWidget )->setMinimumMaximum( min, max );
  whileBlocking( mOpacityWidget )->setOpacity( settings.opacity() );
  int index = mScalarInterpolationTypeComboBox->findData( settings.dataResamplingMethod() );
  whileBlocking( mScalarInterpolationTypeComboBox )->setCurrentIndex( index );

  bool hasEdges = ( mMeshLayer->dataProvider() &&
                    mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Edge ) );
  mScalarEdgeWidthGroupBox->setVisible( hasEdges );
}

double QgsMeshRendererScalarSettingsWidget::lineEditValue( const QLineEdit *lineEdit ) const
{
  if ( lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return lineEdit->text().toDouble();
}

void QgsMeshRendererScalarSettingsWidget::minMaxChanged()
{
  double min = lineEditValue( mScalarMinLineEdit );
  double max = lineEditValue( mScalarMaxLineEdit );
  mScalarColorRampShaderWidget->setMinimumMaximum( min, max );
}

void QgsMeshRendererScalarSettingsWidget::minMaxEdited()
{
  double min = lineEditValue( mScalarMinLineEdit );
  double max = lineEditValue( mScalarMaxLineEdit );
  mScalarColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
}

void QgsMeshRendererScalarSettingsWidget::recalculateMinMaxButtonClicked()
{
  const QgsMeshDatasetGroupMetadata metadata = mMeshLayer->dataProvider()->datasetGroupMetadata( mActiveDatasetGroup );
  double min = metadata.minimum();
  double max = metadata.maximum();
  whileBlocking( mScalarMinLineEdit )->setText( QString::number( min ) );
  whileBlocking( mScalarMaxLineEdit )->setText( QString::number( max ) );
  mScalarColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
}

QgsMeshRendererScalarSettings::DataResamplingMethod QgsMeshRendererScalarSettingsWidget::dataIntepolationMethod() const
{
  const int data = mScalarInterpolationTypeComboBox->currentData().toInt();
  const QgsMeshRendererScalarSettings::DataResamplingMethod method = static_cast<QgsMeshRendererScalarSettings::DataResamplingMethod>( data );
  return method;
}

bool QgsMeshRendererScalarSettingsWidget::dataIsDefinedOnFaces() const
{
  if ( !mMeshLayer || !mMeshLayer->dataProvider() || !mMeshLayer->dataProvider()->isValid() )
    return false;

  if ( mActiveDatasetGroup < 0 )
    return false;

  QgsMeshDatasetGroupMetadata meta = mMeshLayer->dataProvider()->datasetGroupMetadata( mActiveDatasetGroup );
  const bool onFaces = ( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );
  return onFaces;
}

bool QgsMeshRendererScalarSettingsWidget::dataIsDefinedOnEdges() const
{
  if ( !mMeshLayer || !mMeshLayer->dataProvider() || !mMeshLayer->dataProvider()->isValid() )
    return false;

  if ( mActiveDatasetGroup < 0 )
    return false;

  QgsMeshDatasetGroupMetadata meta = mMeshLayer->dataProvider()->datasetGroupMetadata( mActiveDatasetGroup );
  const bool onEdges = ( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );
  return onEdges;
}


