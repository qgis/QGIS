/***************************************************************************
  qgs25drendererwidget.cpp - Qgs25DRendererWidget

 ---------------------
 begin                : 14.1.2016
 copyright            : (C) 2016 by mku
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgs25drendererwidget.h"

#include "qgsmaplayerstylemanager.h"

Qgs25DRendererWidget::Qgs25DRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( nullptr )
{
  if ( !layer )
    return;

  // the renderer only applies to point vector layers
  if ( layer->geometryType() != QGis::Polygon )
  {
    //setup blank dialog
    QGridLayout* layout = new QGridLayout( this );
    QLabel* label = new QLabel( tr( "The 2.5D renderer only can be used with polygon layers. \n"
                                    "'%1' is not a polygon layer and cannot be rendered in 2.5D." )
                                .arg( layer->name() ), this );
    layout->addWidget( label );
    return;
  }

  setupUi( this );

  mWallColorButton->setColorDialogTitle( tr( "Select wall color" ) );
  mWallColorButton->setAllowAlpha( true );
  mWallColorButton->setContext( "symbology" );
  mRoofColorButton->setColorDialogTitle( tr( "Select roof color" ) );
  mRoofColorButton->setAllowAlpha( true );
  mRoofColorButton->setContext( "symbology" );
  mShadowColorButton->setColorDialogTitle( tr( "Select shadow color" ) );
  mShadowColorButton->setAllowAlpha( true );
  mShadowColorButton->setContext( "symbology" );

  if ( renderer )
  {
    mRenderer = Qgs25DRenderer::convertFromRenderer( renderer );
  }

  mHeightWidget->setLayer( layer );

  QgsExpressionContextScope* scope = QgsExpressionContextUtils::layerScope( mLayer );
  QVariant height = scope->variable( "qgis_25d_height" );
  QVariant angle = scope->variable( "qgis_25d_angle" );
  delete scope;

  mHeightWidget->setField( height.isNull() ? "10" : height.toString() );
  mAngleWidget->setValue( angle.isNull() ? 70 : angle.toDouble() );
  mWallColorButton->setColor( mRenderer->wallColor() );
  mRoofColorButton->setColor( mRenderer->roofColor() );
  mShadowColorButton->setColor( mRenderer->shadowColor() );
  mShadowEnabledWidget->setChecked( mRenderer->shadowEnabled() );
  mShadowSizeWidget->setValue( mRenderer->shadowSpread() );
  mWallExpositionShading->setChecked( mRenderer->wallShadingEnabled() );

  connect( mAngleWidget, SIGNAL( valueChanged( int ) ), this, SLOT( updateRenderer() ) );
  connect( mHeightWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( updateRenderer() ) );
  connect( mWallColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( updateRenderer() ) );
  connect( mRoofColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( updateRenderer() ) );
  connect( mShadowColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( updateRenderer() ) );
  connect( mShadowEnabledWidget, SIGNAL( toggled( bool ) ), this, SLOT( updateRenderer() ) );
  connect( mShadowSizeWidget, SIGNAL( valueChanged( double ) ), this, SLOT( updateRenderer() ) );
  connect( mWallExpositionShading, SIGNAL( toggled( bool ) ), this, SLOT( updateRenderer() ) );
}

QgsFeatureRendererV2* Qgs25DRendererWidget::renderer()
{
  return mRenderer;
}

void Qgs25DRendererWidget::updateRenderer()
{
  mRenderer->setRoofColor( mRoofColorButton->color() );
  mRenderer->setWallColor( mWallColorButton->color() );
  mRenderer->setShadowColor( mShadowColorButton->color() );
  mRenderer->setShadowEnabled( mShadowEnabledWidget->isChecked() );
  mRenderer->setShadowSpread( mShadowSizeWidget->value() );
  mRenderer->setWallShadingEnabled( mWallExpositionShading->isChecked() );
  emit widgetChanged();
}

void Qgs25DRendererWidget::apply()
{
  if ( mHeightWidget )
  {
    QgsExpressionContextUtils::setLayerVariable( mLayer, "qgis_25d_height", mHeightWidget->currentText() );
    QgsExpressionContextUtils::setLayerVariable( mLayer, "qgis_25d_angle", mAngleWidget->value() );

    emit layerVariablesChanged();
  }
}

QgsRendererV2Widget* Qgs25DRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new Qgs25DRendererWidget( layer, style, renderer );
}
