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
#include "qgsmeshlayerutils.h"

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
  mNativeMeshSettingsWidget->setLayer( mMeshLayer, QgsMeshRendererMeshSettingsWidget::MeshType::Native );
  mTriangularMeshSettingsWidget->setLayer( mMeshLayer, QgsMeshRendererMeshSettingsWidget::MeshType::Triangular );
  mEdgeMeshSettingsWidget->setLayer( mMeshLayer, QgsMeshRendererMeshSettingsWidget::MeshType::Edge );
  mMeshRendererVectorSettingsWidget->setLayer( mMeshLayer );
  m3dAveragingSettingsWidget->setLayer( mMeshLayer );
  syncToLayer();

  //blend mode
  mBlendModeComboBox->setBlendMode( mMeshLayer->blendMode() );
  connect( mBlendModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPanelWidget::widgetChanged );

  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::activeScalarGroupChanged,
           this, &QgsRendererMeshPropertiesWidget::onActiveScalarGroupChanged );
  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::activeVectorGroupChanged,
           this, &QgsRendererMeshPropertiesWidget::onActiveVectorGroupChanged );

  connect( mNativeMeshGroup, &QGroupBox::toggled, this, &QgsPanelWidget::widgetChanged );
  connect( mEdgeMeshGroup, &QGroupBox::toggled, this, &QgsPanelWidget::widgetChanged );
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
  connect( mEdgeMeshSettingsWidget, &QgsMeshRendererMeshSettingsWidget::widgetChanged,
           this, &QgsPanelWidget::widgetChanged );
  connect( m3dAveragingSettingsWidget, &QgsMeshRenderer3dAveragingWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
}

void QgsRendererMeshPropertiesWidget::apply()
{
  if ( !mMeshLayer )
    return;

  // 1D EDGE MESH
  bool edgeMeshRenderingIsEnabled = mEdgeMeshGroup->isChecked();
  QgsMeshRendererMeshSettings edgeMeshSettings = mEdgeMeshSettingsWidget->settings();
  edgeMeshSettings.setEnabled( edgeMeshRenderingIsEnabled );

  // 2D NATIVE MESH
  bool nativeMeshRenderingIsEnabled = mNativeMeshGroup->isChecked();
  QgsMeshRendererMeshSettings nativeMeshSettings = mNativeMeshSettingsWidget->settings();
  nativeMeshSettings.setEnabled( nativeMeshRenderingIsEnabled );

  // 2D TRIANGULAR MESH
  bool triangularMeshRenderingIsEnabled = mTriangularMeshGroup->isChecked();
  QgsMeshRendererMeshSettings triangularMeshSettings = mTriangularMeshSettingsWidget->settings();
  triangularMeshSettings.setEnabled( triangularMeshRenderingIsEnabled );

  // SCALAR
  int activeScalarDatasetGroupIndex = mMeshRendererActiveDatasetWidget->activeScalarDatasetGroup();
  if ( !mContoursGroupBox->isChecked() )
    activeScalarDatasetGroupIndex = -1;

  // VECTOR
  int activeVectorDatasetGroupIndex = mMeshRendererActiveDatasetWidget->activeVectorDatasetGroup();
  if ( !mVectorsGroupBox->isChecked() )
    activeVectorDatasetGroupIndex = -1;

  QgsMeshRendererSettings settings = mMeshLayer->rendererSettings();
  settings.setEdgeMeshSettings( edgeMeshSettings );
  settings.setNativeMeshSettings( nativeMeshSettings );
  settings.setTriangularMeshSettings( triangularMeshSettings );

  settings.setActiveScalarDatasetGroup( activeScalarDatasetGroupIndex );
  if ( activeScalarDatasetGroupIndex > -1 )
    settings.setScalarSettings( activeScalarDatasetGroupIndex, mMeshRendererScalarSettingsWidget->settings() );

  settings.setActiveVectorDatasetGroup( activeVectorDatasetGroupIndex );
  if ( activeVectorDatasetGroupIndex > -1 )
    settings.setVectorSettings( activeVectorDatasetGroupIndex, mMeshRendererVectorSettingsWidget->settings() );

  QgsMeshDatasetIndex staticScalarDatasetIndex( activeScalarDatasetGroupIndex, mMeshLayer->staticScalarDatasetIndex().dataset() );
  QgsMeshDatasetIndex staticVectorDatasetIndex( activeVectorDatasetGroupIndex, mMeshLayer->staticVectorDatasetIndex().dataset() );
  mMeshLayer->setStaticScalarDatasetIndex( staticScalarDatasetIndex );
  mMeshLayer->setStaticVectorDatasetIndex( staticVectorDatasetIndex );

  //set the blend mode for the layer
  mMeshLayer->setBlendMode( mBlendModeComboBox->blendMode() );
  //set the averaging method for the layer
  std::unique_ptr<QgsMesh3dAveragingMethod> averagingMethod( m3dAveragingSettingsWidget->averagingMethod() );
  settings.setAveragingMethod( averagingMethod.get() );
  mMeshLayer->setRendererSettings( settings );
  mMeshLayer->triggerRepaint();

  QgsSettings windowsSettings;
  windowsSettings.setValue( QStringLiteral( "/Windows/RendererMeshProperties/tab" ), mStyleOptionsTab->currentIndex() );
}

void QgsRendererMeshPropertiesWidget::syncToLayer()
{
  mMeshRendererActiveDatasetWidget->syncToLayer();
  mNativeMeshSettingsWidget->syncToLayer();
  mTriangularMeshSettingsWidget->syncToLayer();
  mEdgeMeshSettingsWidget->syncToLayer();
  m3dAveragingSettingsWidget->syncToLayer();

  mNativeMeshGroup->setChecked( mMeshLayer ? mMeshLayer->rendererSettings().nativeMeshSettings().isEnabled() : false );
  mTriangularMeshGroup->setChecked( mMeshLayer ? mMeshLayer->rendererSettings().triangularMeshSettings().isEnabled() : false );
  mEdgeMeshGroup->setChecked( mMeshLayer ? mMeshLayer->rendererSettings().edgeMeshSettings().isEnabled() : false );

  onActiveScalarGroupChanged( mMeshLayer->rendererSettings().activeScalarDatasetGroup() );
  onActiveVectorGroupChanged( mMeshLayer->rendererSettings().activeVectorDatasetGroup() );

  bool hasFaces = ( mMeshLayer->dataProvider() &&
                    mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Face ) );
  mFaceMeshGroupBox->setVisible( hasFaces );

  bool hasEdges = ( mMeshLayer->dataProvider() &&
                    mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Edge ) );
  mEdgeMeshGroupBox->setVisible( hasEdges );

  QgsSettings settings;
  if ( !settings.contains( QStringLiteral( "/Windows/RendererMeshProperties/tab" ) ) )
    settings.setValue( QStringLiteral( "/Windows/RendererMeshProperties/tab" ), 0 );
  else
    mStyleOptionsTab->setCurrentIndex( settings.value( QStringLiteral( "/Windows/RendererMeshProperties/tab" ) ).toInt() );
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
  if ( !mMeshLayer->dataProvider() ||
       ( groupIndex >= 0 && !mMeshLayer->dataProvider()->datasetGroupMetadata( groupIndex ).isVector() ) )
    groupIndex = -1;
  mMeshRendererVectorSettingsWidget->setActiveDatasetGroup( groupIndex );
  mMeshRendererVectorSettingsWidget->syncToLayer();
  mVectorsGroupBox->setChecked( groupIndex >= 0 );
  mVectorsGroupBox->setEnabled( groupIndex >= 0 );
}
