/***************************************************************************
  qgstiledscenelayer3drendererwidget.cpp
  --------------------------------------
  Date                 : August 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenelayer3drendererwidget.h"
#include "moc_qgstiledscenelayer3drendererwidget.cpp"

#include "qgstiledscenelayer.h"
#include "qgstiledscenelayer3drenderer.h"

#include <QBoxLayout>


QgsTiledSceneLayer3DPropertiesWidget::QgsTiledSceneLayer3DPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mMaxErrorSpinBox->setClearValue( 16 );

  connect( mMaxErrorSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsTiledSceneLayer3DPropertiesWidget::widgetChanged );
  connect( mShowBoundingBoxesCheckBox, &QCheckBox::clicked, this, &QgsTiledSceneLayer3DPropertiesWidget::widgetChanged );
}

void QgsTiledSceneLayer3DPropertiesWidget::syncToLayer( QgsTiledSceneLayer *layer )
{
  mLayer = layer;

  QgsAbstract3DRenderer *renderer = layer->renderer3D();
  if ( !renderer || renderer->type() != QLatin1String( "tiledscene" ) )
    return;

  QgsTiledSceneLayer3DRenderer *r = static_cast<QgsTiledSceneLayer3DRenderer *>( renderer );

  whileBlocking( mMaxErrorSpinBox )->setValue( r->maximumScreenError() );
  whileBlocking( mShowBoundingBoxesCheckBox )->setChecked( r->showBoundingBoxes() );
}

void QgsTiledSceneLayer3DPropertiesWidget::apply()
{
  QgsTiledSceneLayer3DRenderer *r = new QgsTiledSceneLayer3DRenderer();
  r->setMaximumScreenError( mMaxErrorSpinBox->value() );
  r->setShowBoundingBoxes( mShowBoundingBoxesCheckBox->isChecked() );
  r->setLayer( mLayer );
  mLayer->setRenderer3D( r );
}

//

QgsTiledSceneLayer3DRendererWidget::QgsTiledSceneLayer3DRendererWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setPanelTitle( tr( "3D View" ) );
  setObjectName( QStringLiteral( "mOptsPage_3DView" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );

  mWidget = new QgsTiledSceneLayer3DPropertiesWidget( this );
  layout->addWidget( mWidget );

  connect( mWidget, &QgsTiledSceneLayer3DPropertiesWidget::widgetChanged, this, &QgsTiledSceneLayer3DRendererWidget::widgetChanged );

  syncToLayer( layer );
}

void QgsTiledSceneLayer3DRendererWidget::syncToLayer( QgsMapLayer *layer )
{
  QgsTiledSceneLayer *tslayer = qobject_cast<QgsTiledSceneLayer *>( layer );
  if ( !tslayer )
  {
    return;
  }
  mLayer = layer;

  mWidget->syncToLayer( tslayer );
}

void QgsTiledSceneLayer3DRendererWidget::apply()
{
  mWidget->apply();
}

//

QgsTiledSceneLayer3DRendererWidgetFactory::QgsTiledSceneLayer3DRendererWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QIcon( ":/images/themes/default/3d.svg" ) );
  setTitle( tr( "3D View" ) );
}

QgsMapLayerConfigWidget *QgsTiledSceneLayer3DRendererWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  Q_UNUSED( dockWidget )
  return new QgsTiledSceneLayer3DRendererWidget( layer, canvas, parent );
}

bool QgsTiledSceneLayer3DRendererWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsTiledSceneLayer3DRendererWidgetFactory::supportsStyleDock() const
{
  return true;
}

bool QgsTiledSceneLayer3DRendererWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::TiledScene;
}

QString QgsTiledSceneLayer3DRendererWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Rendering" );
}
