/***************************************************************************
    qgsrenderermeshpropertieswidget.cpp
    -----------------------------------
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

#include "qgsrenderermeshpropertieswidget.h"

#include "qgis.h"
#include "qgsmapcanvas.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrendererscalarsettingswidget.h"
#include "qgsmeshdatasetgrouptreeview.h"
#include "qgsmeshrendereractivedatasetwidget.h"

QgsRendererMeshPropertiesWidget::QgsRendererMeshPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mMeshLayer( layer )
{
  if ( !mMeshLayer )
    return;

  setupUi( this );

  connect( mMeshLayer,
           &QgsMeshLayer::dataChanged,
           this,
           &QgsRendererMeshPropertiesWidget::syncToLayer );

  mMeshRendererActiveDatasetWidget->setLayer( mMeshLayer );
  mMeshRendererScalarSettingsWidget->setLayer( mMeshLayer );
  mNativeMeshSettingsWidget->setLayer( mMeshLayer, false );
  mTriangularMeshSettingsWidget->setLayer( mMeshLayer, true );
  mMeshRendererVectorSettingsWidget->setLayer( mMeshLayer );
  syncToLayer();

  //blend mode
  mBlendModeComboBox->setBlendMode( mMeshLayer->blendMode() );
  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );

  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::activeScalarGroupChanged,
           this, &QgsRendererMeshPropertiesWidget::onActiveScalarGroupChanged );
  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::activeVectorGroupChanged,
           this, &QgsRendererMeshPropertiesWidget::onActiveVectorGroupChanged );

  connect( mNativeMeshGroup, &QGroupBox::toggled, this, &QgsPanelWidget::widgetChanged );
  connect( mTriangularMeshGroup, &QGroupBox::toggled, this, &QgsPanelWidget::widgetChanged );
  connect( mContoursGroupBox, &QGroupBox::toggled, this, &QgsPanelWidget::widgetChanged );
  connect( mVectorsGroupBox, &QGroupBox::toggled, this, &QgsPanelWidget::widgetChanged );
  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mMeshRendererScalarSettingsWidget, &QgsMeshRendererScalarSettingsWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mMeshRendererVectorSettingsWidget, &QgsMeshRendererVectorSettingsWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mNativeMeshSettingsWidget, &QgsMeshRendererMeshSettingsWidget::widgetChanged,
           this, &QgsPanelWidget::widgetChanged );
  connect( mTriangularMeshSettingsWidget, &QgsMeshRendererMeshSettingsWidget::widgetChanged,
           this, &QgsPanelWidget::widgetChanged );
}

void QgsRendererMeshPropertiesWidget::apply()
{
  if ( !mMeshLayer )
    return;

  // MESH
  bool meshRenderingIsEnabled = mNativeMeshGroup->isChecked();
  QgsMeshRendererMeshSettings meshSettings = mNativeMeshSettingsWidget->settings();
  meshSettings.setEnabled( meshRenderingIsEnabled );

  // TRIANGULAR MESH
  bool triangularMeshRenderingIsEnabled = mTriangularMeshGroup->isChecked();
  QgsMeshRendererMeshSettings triangularMeshSettings = mTriangularMeshSettingsWidget->settings();
  triangularMeshSettings.setEnabled( triangularMeshRenderingIsEnabled );

  // SCALAR
  QgsMeshDatasetIndex activeScalarDatasetIndex = mMeshRendererActiveDatasetWidget->activeScalarDataset();
  if ( !mContoursGroupBox->isChecked() )
    activeScalarDatasetIndex = QgsMeshDatasetIndex();

  // VECTOR
  QgsMeshDatasetIndex activeVectorDatasetIndex = mMeshRendererActiveDatasetWidget->activeVectorDataset();
  if ( !mVectorsGroupBox->isChecked() )
    activeVectorDatasetIndex = QgsMeshDatasetIndex();

  QgsMeshRendererSettings settings = mMeshLayer->rendererSettings();
  settings.setNativeMeshSettings( meshSettings );
  settings.setTriangularMeshSettings( triangularMeshSettings );

  settings.setActiveScalarDataset( activeScalarDatasetIndex );
  if ( activeScalarDatasetIndex.isValid() )
    settings.setScalarSettings( activeScalarDatasetIndex.group(), mMeshRendererScalarSettingsWidget->settings() );

  settings.setActiveVectorDataset( activeVectorDatasetIndex );
  if ( activeVectorDatasetIndex.isValid() )
    settings.setVectorSettings( activeVectorDatasetIndex.group(), mMeshRendererVectorSettingsWidget->settings() );

  //set the blend mode for the layer
  mMeshLayer->setBlendMode( mBlendModeComboBox->blendMode() );

  mMeshLayer->setRendererSettings( settings );
  mMeshLayer->triggerRepaint();
}

void QgsRendererMeshPropertiesWidget::syncToLayer()
{
  mMeshRendererActiveDatasetWidget->syncToLayer();
  mNativeMeshSettingsWidget->syncToLayer();
  mTriangularMeshSettingsWidget->syncToLayer();

  onActiveScalarGroupChanged( mMeshRendererActiveDatasetWidget->activeScalarDatasetGroup() );
  onActiveVectorGroupChanged( mMeshRendererActiveDatasetWidget->activeVectorDatasetGroup() );
}

void QgsRendererMeshPropertiesWidget::onActiveScalarGroupChanged( int groupIndex )
{
  mMeshRendererScalarSettingsWidget->setActiveDatasetGroup( groupIndex );
  mMeshRendererScalarSettingsWidget->syncToLayer();
  mContoursGroupBox->setChecked( groupIndex >= 0 );
  mContoursGroupBox->setEnabled( groupIndex >= 0 );
}

void QgsRendererMeshPropertiesWidget::onActiveVectorGroupChanged( int groupIndex )
{
  if ( groupIndex >= 0 && !mMeshLayer->dataProvider()->datasetGroupMetadata( groupIndex ).isVector() )
    groupIndex = -1;
  mMeshRendererVectorSettingsWidget->setActiveDatasetGroup( groupIndex );
  mMeshRendererVectorSettingsWidget->syncToLayer();
  mVectorsGroupBox->setChecked( groupIndex >= 0 );
  mVectorsGroupBox->setEnabled( groupIndex >= 0 );
}
