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
#include "moc_qgsmeshrendererscalarsettingswidget.cpp"

#include "QDialogButtonBox"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmeshvariablestrokewidthwidget.h"
#include "qgsmapcanvas.h"
#include <QPointer>

QgsMeshRendererScalarSettingsWidget::QgsMeshRendererScalarSettingsWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  mScalarMinSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue );
  mScalarMinSpinBox->setSpecialValueText( QString( ) );
  mScalarMaxSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::MinimumValue );
  mScalarMaxSpinBox->setSpecialValueText( QString( ) );

  // add items to data interpolation combo box
  mScalarInterpolationTypeComboBox->addItem( tr( "No Resampling" ), QgsMeshRendererScalarSettings::NoResampling );
  mScalarInterpolationTypeComboBox->addItem( tr( "Neighbour Average" ), QgsMeshRendererScalarSettings::NeighbourAverage );
  mScalarInterpolationTypeComboBox->setCurrentIndex( 0 );

  mMinMaxValueTypeComboBox->addItem( tr( "User Defined" ) );
  mMinMaxValueTypeComboBox->addItem( tr( "Whole Mesh" ) );
  mMinMaxValueTypeComboBox->addItem( tr( "Current Canvas" ) );
  mMinMaxValueTypeComboBox->addItem( tr( "Updated Canvas" ) );
  mMinMaxValueTypeComboBox->setCurrentIndex( 0 );

  mScalarEdgeStrokeWidthUnitSelectionWidget->setUnits(
  {
    Qgis::RenderUnit::Millimeters,
    Qgis::RenderUnit::MetersInMapUnits,
    Qgis::RenderUnit::Pixels,
    Qgis::RenderUnit::Points,
  } );

  // connect
  connect( mScalarRecalculateMinMaxButton, &QPushButton::clicked, this, &QgsMeshRendererScalarSettingsWidget::recalculateMinMaxButtonClicked );
  connect( mScalarMinSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { minMaxChanged(); } );
  connect( mScalarMaxSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { minMaxChanged(); } );
  connect( mScalarEdgeStrokeWidthVariableRadioButton, &QRadioButton::toggled, this, &QgsMeshRendererScalarSettingsWidget::onEdgeStrokeWidthMethodChanged );

  connect( mScalarColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarInterpolationTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );

  connect( mScalarEdgeStrokeWidthUnitSelectionWidget, &QgsUnitSelectionWidget::changed,
           this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarEdgeStrokeWidthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarEdgeStrokeWidthVariableRadioButton, &QCheckBox::toggled, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarEdgeStrokeWidthFixedRadioButton, &QCheckBox::toggled, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
  connect( mScalarEdgeStrokeWidthVariablePushButton, &QgsMeshVariableStrokeWidthButton::widgetChanged, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );

  connect( mMinMaxValueTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererScalarSettingsWidget::minMaxSourceChanged );
  connect( mMinMaxValueTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererScalarSettingsWidget::recalculateMinMax );
  connect( mMinMaxValueTypeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );
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
  settings.setOpacity( mOpacityWidget->opacity() );
  settings.setDataResamplingMethod( dataIntepolationMethod() );

  settings.setClassificationMinimumMaximum( spinBoxValue( mScalarMinSpinBox ), spinBoxValue( mScalarMaxSpinBox ) );
  settings.setMinMaxValueType( minMaxValueType() );

  const bool hasEdges = ( mMeshLayer->contains( QgsMesh::ElementType::Edge ) );
  if ( hasEdges )
  {
    QgsInterpolatedLineWidth edgeStrokeWidth = mScalarEdgeStrokeWidthVariablePushButton->variableStrokeWidth();
    edgeStrokeWidth.setIsVariableWidth( mScalarEdgeStrokeWidthVariableRadioButton->isChecked() );
    edgeStrokeWidth.setFixedStrokeWidth( mScalarEdgeStrokeWidthSpinBox->value() );
    settings.setEdgeStrokeWidth( edgeStrokeWidth );
    settings.setEdgeStrokeWidthUnit( mScalarEdgeStrokeWidthUnitSelectionWidget->unit() );
  }

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

  const double  min = settings.classificationMinimum();
  const double  max = settings.classificationMaximum();

  const int indexMinMaxType = static_cast<int>( settings.minMaxValueType() );
  whileBlocking( mMinMaxValueTypeComboBox )->setCurrentIndex( indexMinMaxType );

  if ( settings.minMaxValueType() == QgsMeshRendererScalarSettings::MinMaxValueType::UserDefined )
  {
    mScalarMinSpinBox->setEnabled( true );
    mScalarMaxSpinBox->setEnabled( true );
    mScalarRecalculateMinMaxButton->setEnabled( true );
  }
  else
  {
    mScalarMinSpinBox->setDisabled( true );
    mScalarMaxSpinBox->setDisabled( true );
    mScalarRecalculateMinMaxButton->setDisabled( true );
  }

  whileBlocking( mScalarMinSpinBox )->setValue( min );
  whileBlocking( mScalarMaxSpinBox )->setValue( max );

  whileBlocking( mScalarColorRampShaderWidget )->setFromShader( shader );
  whileBlocking( mScalarColorRampShaderWidget )->setMinimumMaximum( min, max );
  whileBlocking( mOpacityWidget )->setOpacity( settings.opacity() );
  const int index = mScalarInterpolationTypeComboBox->findData( settings.dataResamplingMethod() );
  whileBlocking( mScalarInterpolationTypeComboBox )->setCurrentIndex( index );

  const bool hasEdges = ( mMeshLayer->contains( QgsMesh::ElementType::Edge ) );
  const bool hasFaces = ( mMeshLayer->contains( QgsMesh::ElementType::Face ) );

  mScalarResamplingWidget->setVisible( hasFaces );

  mEdgeWidthGroupBox->setVisible( hasEdges );

  if ( hasEdges )
  {
    const QgsInterpolatedLineWidth edgeStrokeWidth = settings.edgeStrokeWidth();
    whileBlocking( mScalarEdgeStrokeWidthVariablePushButton )->setVariableStrokeWidth( edgeStrokeWidth );
    whileBlocking( mScalarEdgeStrokeWidthSpinBox )->setValue( edgeStrokeWidth.fixedStrokeWidth() );
    whileBlocking( mScalarEdgeStrokeWidthVariableRadioButton )->setChecked( edgeStrokeWidth.isVariableWidth() );
    whileBlocking( mScalarEdgeStrokeWidthUnitSelectionWidget )->setUnit( settings.edgeStrokeWidthUnit() );
    if ( !hasFaces )
      mOpacityContainerWidget->setVisible( false );

    const QgsMeshDatasetGroupMetadata metadata = mMeshLayer->datasetGroupMetadata( mActiveDatasetGroup );
    const double min = metadata.minimum();
    const double max = metadata.maximum();
    mScalarEdgeStrokeWidthVariablePushButton->setDefaultMinMaxValue( min, max );
  }

  onEdgeStrokeWidthMethodChanged();
}

double QgsMeshRendererScalarSettingsWidget::spinBoxValue( const QgsDoubleSpinBox *spinBox ) const
{
  if ( spinBox->value() == spinBox->clearValue() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return spinBox->value();
}

void QgsMeshRendererScalarSettingsWidget::minMaxChanged()
{
  const double min = spinBoxValue( mScalarMinSpinBox );
  const double max = spinBoxValue( mScalarMaxSpinBox );
  mScalarColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
}

void QgsMeshRendererScalarSettingsWidget::recalculateMinMaxButtonClicked()
{
  const QgsMeshDatasetGroupMetadata metadata = mMeshLayer->datasetGroupMetadata( mActiveDatasetGroup );
  const double min = metadata.minimum();
  const double max = metadata.maximum();
  whileBlocking( mScalarMinSpinBox )->setValue( min );
  whileBlocking( mScalarMaxSpinBox )->setValue( max );
  mScalarColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
}

void QgsMeshRendererScalarSettingsWidget::onEdgeStrokeWidthMethodChanged()
{
  const bool variableWidth = mScalarEdgeStrokeWidthVariableRadioButton->isChecked();
  mScalarEdgeStrokeWidthVariablePushButton->setVisible( variableWidth );
  mScalarEdgeStrokeWidthSpinBox->setVisible( !variableWidth );
}

QgsMeshRendererScalarSettings::DataResamplingMethod QgsMeshRendererScalarSettingsWidget::dataIntepolationMethod() const
{
  const int data = mScalarInterpolationTypeComboBox->currentData().toInt();
  const QgsMeshRendererScalarSettings::DataResamplingMethod method = static_cast<QgsMeshRendererScalarSettings::DataResamplingMethod>( data );
  return method;
}

QgsMeshRendererScalarSettings::MinMaxValueType QgsMeshRendererScalarSettingsWidget::minMaxValueType() const
{
  const QgsMeshRendererScalarSettings::MinMaxValueType type = static_cast<QgsMeshRendererScalarSettings::MinMaxValueType>( mMinMaxValueTypeComboBox->currentIndex() );
  return type;
}

bool QgsMeshRendererScalarSettingsWidget::dataIsDefinedOnFaces() const
{
  if ( !mMeshLayer )
    return false;

  if ( mActiveDatasetGroup < 0 )
    return false;

  const QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( mActiveDatasetGroup );
  const bool onFaces = ( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );
  return onFaces;
}

bool QgsMeshRendererScalarSettingsWidget::dataIsDefinedOnEdges() const
{
  if ( !mMeshLayer )
    return false;

  if ( mActiveDatasetGroup < 0 )
    return false;

  const QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( mActiveDatasetGroup );
  const bool onEdges = ( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );
  return onEdges;
}

void QgsMeshRendererScalarSettingsWidget::minMaxSourceChanged()
{
  if ( mMinMaxValueTypeComboBox->currentIndex() == 0 )
  {
    mScalarMinSpinBox->setEnabled( true );
    mScalarMaxSpinBox->setEnabled( true );
    mScalarRecalculateMinMaxButton->setEnabled( true );
  }
  else
  {
    mScalarMinSpinBox->setEnabled( false );
    mScalarMaxSpinBox->setEnabled( false );
    mScalarRecalculateMinMaxButton->setEnabled( false );
  }
}

void QgsMeshRendererScalarSettingsWidget::setCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

void QgsMeshRendererScalarSettingsWidget::recalculateMinMax()
{
  QgsRectangle searchExtent;

  switch ( minMaxValueType() )
  {
    case QgsMeshRendererScalarSettings::MinMaxValueType::WholeMesh:
    {
      searchExtent = mMeshLayer->extent();
      break;
    }
    case QgsMeshRendererScalarSettings::MinMaxValueType::FixedCanvas:
    case QgsMeshRendererScalarSettings::MinMaxValueType::InteractiveFromCanvas:
    {
      QgsCoordinateTransform ct = QgsCoordinateTransform( mCanvas->mapSettings().destinationCrs(), mMeshLayer->crs(), QgsProject::instance() );
      searchExtent = mCanvas->extent();
      searchExtent = ct.transform( searchExtent );
      break;
    }
    default:
      break;
  }

  if ( !searchExtent.isEmpty() )
  {
    QgsMeshDatasetIndex datasetIndex = mMeshLayer->activeScalarDatasetAtTime( mCanvas->temporalRange(), mActiveDatasetGroup );
    double min, max;
    bool found;

    found = mMeshLayer->minimumMaximumActiveScalarDataset( searchExtent, datasetIndex, min, max );
    if ( found )
    {
      whileBlocking( mScalarMinSpinBox )->setValue( min );
      whileBlocking( mScalarMaxSpinBox )->setValue( max );
      minMaxChanged();
    }
  }
}
