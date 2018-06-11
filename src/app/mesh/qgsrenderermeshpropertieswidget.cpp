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

  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::activeScalarDatasetChanged,
           mMeshRendererScalarSettingsWidget, &QgsMeshRendererScalarSettingsWidget::setActiveDataset );
  connect( mMeshRendererActiveDatasetWidget, &QgsMeshRendererActiveDatasetWidget::activeVectorDatasetChanged,
           mMeshRendererVectorSettingsWidget, &QgsMeshRendererVectorSettingsWidget::setActiveDataset );

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
  bool meshRenderingIsEnabled = mMeshRendererActiveDatasetWidget->isNativeMeshEnabled();
  QgsMeshRendererMeshSettings meshSettings = mNativeMeshSettingsWidget->settings();
  meshSettings.setEnabled( meshRenderingIsEnabled );
  whileBlocking( mMeshLayer )->setRendererNativeMeshSettings( meshSettings );

  // TRIANGULAR MESH
  bool triangularMeshRenderingIsEnabled = mMeshRendererActiveDatasetWidget->isTriangularMeshEnabled();
  QgsMeshRendererMeshSettings triangularMeshSettings = mTriangularMeshSettingsWidget->settings();
  triangularMeshSettings.setEnabled( triangularMeshRenderingIsEnabled );
  whileBlocking( mMeshLayer )->setRendererTriangularMeshSettings( triangularMeshSettings );

  // SCALAR
  int activeScalarDatasetIndex = mMeshRendererActiveDatasetWidget->activeScalarDataset();
  whileBlocking( mMeshLayer )->setActiveScalarDataset( activeScalarDatasetIndex );
  if ( activeScalarDatasetIndex != -1 )
  {
    const  QgsMeshRendererScalarSettings settings = mMeshRendererScalarSettingsWidget->settings();
    whileBlocking( mMeshLayer )->setRendererScalarSettings( settings );
  }

  // VECTOR
  int activeVectorDatasetIndex = mMeshRendererActiveDatasetWidget->activeVectorDataset();
  whileBlocking( mMeshLayer )->setActiveVectorDataset( activeVectorDatasetIndex );
  if ( activeVectorDatasetIndex != -1 )
  {
    const QgsMeshRendererVectorSettings settings = mMeshRendererVectorSettingsWidget->settings();
    whileBlocking( mMeshLayer )->setRendererVectorSettings( settings );
  }

  mMeshLayer->triggerRepaint();
}

void QgsRendererMeshPropertiesWidget::syncToLayer()
{
  mMeshRendererActiveDatasetWidget->syncToLayer();
  mMeshRendererScalarSettingsWidget->syncToLayer();
  mNativeMeshSettingsWidget->syncToLayer();
  mTriangularMeshSettingsWidget->syncToLayer();
  mMeshRendererVectorSettingsWidget->syncToLayer();
}
