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
#include "moc_qgsvectorlayer3dpropertieswidget.cpp"

#include "qgsabstractvectorlayer3drenderer.h"

QgsVectorLayer3DPropertiesWidget::QgsVectorLayer3DPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  constexpr int MAX_CHUNK_FEATURES = 1'000;
  mMaxFeaturesSpinBox->setValue( MAX_CHUNK_FEATURES );
  mMaxFeaturesSpinBox->setClearValue( MAX_CHUNK_FEATURES );
  mMaxFeaturesSpinBox->setToolTip( tr( "This is the maximum number of features that any node will attempt to load.\nFeatures beyond that number will be fetched by child nodes when Maximum Screen Space Error is reached." ) );

  groupLayerRendering->setCollapsed( true );

  connect( chkShowBoundingBoxes, &QCheckBox::clicked, this, &QgsVectorLayer3DPropertiesWidget::changed );
  connect( mMaxFeaturesSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, &QgsVectorLayer3DPropertiesWidget::changed );
}

void QgsVectorLayer3DPropertiesWidget::load( QgsAbstractVectorLayer3DRenderer *renderer )
{
  whileBlocking( chkShowBoundingBoxes )->setChecked( renderer->tilingSettings().showBoundingBoxes() );
  whileBlocking( mMaxFeaturesSpinBox )->setValue( renderer->tilingSettings().maximumChunkFeatures() );
}

void QgsVectorLayer3DPropertiesWidget::apply( QgsAbstractVectorLayer3DRenderer *renderer )
{
  QgsVectorLayer3DTilingSettings tilingSettings;
  tilingSettings.setShowBoundingBoxes( chkShowBoundingBoxes->isChecked() );
  tilingSettings.setMaximumChunkFeatures( mMaxFeaturesSpinBox->value() );
  renderer->setTilingSettings( tilingSettings );
}

void QgsVectorLayer3DPropertiesWidget::reset()
{
  whileBlocking( chkShowBoundingBoxes )->setChecked( false );
  whileBlocking( mMaxFeaturesSpinBox )->clear();
}
