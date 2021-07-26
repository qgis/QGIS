/***************************************************************************
  qgsvectorlayer3dpropertieswidget.cpp
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayer3dpropertieswidget.h"

#include "qgsabstractvectorlayer3drenderer.h"

QgsVectorLayer3DPropertiesWidget::QgsVectorLayer3DPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  spinZoomLevelsCount->setClearValue( 3 );

  groupLayerRendering->setCollapsed( true );

  connect( chkShowBoundingBoxes, &QCheckBox::clicked, this, &QgsVectorLayer3DPropertiesWidget::changed );
  connect( spinZoomLevelsCount, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsVectorLayer3DPropertiesWidget::changed );
  connect( mRenderTo3dTerrainCheckBox, &QCheckBox::clicked, this, &QgsVectorLayer3DPropertiesWidget::changed );
}

void QgsVectorLayer3DPropertiesWidget::load( QgsAbstractVectorLayer3DRenderer *renderer )
{
  whileBlocking( spinZoomLevelsCount )->setValue( renderer->tilingSettings().zoomLevelsCount() );
  whileBlocking( chkShowBoundingBoxes )->setChecked( renderer->tilingSettings().showBoundingBoxes() );
}

void QgsVectorLayer3DPropertiesWidget::apply( QgsAbstractVectorLayer3DRenderer *renderer )
{
  QgsVectorLayer3DTilingSettings tilingSettings;
  tilingSettings.setZoomLevelsCount( spinZoomLevelsCount->value() );
  tilingSettings.setShowBoundingBoxes( chkShowBoundingBoxes->isChecked() );
  renderer->setTilingSettings( tilingSettings );
}

bool QgsVectorLayer3DPropertiesWidget::renderLayerTo3dTerrain() const
{
  return mRenderTo3dTerrainCheckBox->isChecked();
}

void QgsVectorLayer3DPropertiesWidget::setRenderLayerTo3dTerrain( bool renderTo3dTerrain )
{
  mRenderTo3dTerrainCheckBox->setChecked( renderTo3dTerrain );
}
