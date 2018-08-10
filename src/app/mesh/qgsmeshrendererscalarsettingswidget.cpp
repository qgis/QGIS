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
#include "qgsmessagelog.h"


QgsMeshRendererScalarSettingsWidget::QgsMeshRendererScalarSettingsWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  connect( mScalarRecalculateMinMaxButton, &QPushButton::clicked, this, &QgsMeshRendererScalarSettingsWidget::recalculateMinMaxButtonClicked );
  connect( mScalarMinLineEdit, &QLineEdit::textChanged, this, &QgsMeshRendererScalarSettingsWidget::minMaxChanged );
  connect( mScalarMaxLineEdit, &QLineEdit::textChanged, this, &QgsMeshRendererScalarSettingsWidget::minMaxChanged );
  connect( mScalarMinLineEdit, &QLineEdit::textEdited, this, &QgsMeshRendererScalarSettingsWidget::minMaxEdited );
  connect( mScalarMaxLineEdit, &QLineEdit::textEdited, this, &QgsMeshRendererScalarSettingsWidget::minMaxEdited );
  connect( mScalarColorRampShaderWidget, &QgsColorRampShaderWidget::widgetChanged, this, &QgsMeshRendererScalarSettingsWidget::widgetChanged );

}

void QgsMeshRendererScalarSettingsWidget::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
    syncToLayer();
  }
}

QgsMeshRendererScalarSettings QgsMeshRendererScalarSettingsWidget::settings() const
{
  QgsMeshRendererScalarSettings settings;
  settings.setColorRampShader( mScalarColorRampShaderWidget->shader() );
  return settings;
}

void QgsMeshRendererScalarSettingsWidget::syncToLayer( )
{
  if ( !mMeshLayer )
    return;

  if ( mMeshLayer->rendererScalarSettings().isEnabled() )
  {
    const QgsColorRampShader shader = mMeshLayer->rendererScalarSettings().colorRampShader();
    whileBlocking( mScalarMinLineEdit )->setText( QString::number( shader.minimumValue() ) );
    whileBlocking( mScalarMaxLineEdit )->setText( QString::number( shader.maximumValue() ) );
    whileBlocking( mScalarColorRampShaderWidget )->setFromShader( shader );
  }
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
  double min, max;
  calcMinMax( mActiveDataset, min, max );
  whileBlocking( mScalarMinLineEdit )->setText( QString::number( min ) );
  whileBlocking( mScalarMaxLineEdit )->setText( QString::number( max ) );
  mScalarColorRampShaderWidget->setMinimumMaximumAndClassify( min, max );
}

void QgsMeshRendererScalarSettingsWidget::setActiveDataset( QgsMeshDatasetIndex activeDataset )
{
  mActiveDataset = activeDataset;
}

void QgsMeshRendererScalarSettingsWidget::calcMinMax( QgsMeshDatasetIndex datasetIndex, double &min, double &max ) const
{
  if ( !mMeshLayer )
    return;

  if ( !mMeshLayer->dataProvider() )
    return;

  const QgsMeshDatasetGroupMetadata metadata = mMeshLayer->dataProvider()->datasetGroupMetadata( datasetIndex );
  bool scalarDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
  int count;
  if ( scalarDataOnVertices )
    count = mMeshLayer->dataProvider()->vertexCount();
  else
    count = mMeshLayer->dataProvider()->faceCount();

  bool myFirstIterationFlag = true;
  for ( int i = 0; i < count; ++i )
  {
    double myValue = mMeshLayer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
    if ( std::isnan( myValue ) ) continue; // NULL
    if ( myFirstIterationFlag )
    {
      myFirstIterationFlag = false;
      min = myValue;
      max = myValue;
    }
    else
    {
      if ( myValue < min )
      {
        min = myValue;
      }
      if ( myValue > max )
      {
        max = myValue;
      }
    }
  }
}
