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

  if ( renderer )
  {
    mRenderer = Qgs25DRenderer::convertFromRenderer( renderer );
  }

  mHeightWidget->setLayer( layer );
  mHeightWidget->setField( mRenderer->height().expressionOrField() );
  mAngleWidget->setValue( mRenderer->angle() );
  mWallColorButton->setColor( mRenderer->wallColor() );
  mRoofColorButton->setColor( mRenderer->roofColor() );

  connect( mAngleWidget, SIGNAL( valueChanged( int ) ), this, SLOT( updateRenderer() ) );
  connect( mHeightWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( updateRenderer() ) );
  connect( mWallColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( updateRenderer() ) );
  connect( mRoofColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( updateRenderer() ) );
}

QgsFeatureRendererV2* Qgs25DRendererWidget::renderer()
{
  return mRenderer;
}

void Qgs25DRendererWidget::updateRenderer()
{
  mRenderer->setHeight( QgsDataDefined( mHeightWidget->currentText() ) );
  mRenderer->setAngle( mAngleWidget->value() );
  mRenderer->setRoofColor( mRoofColorButton->color() );
  mRenderer->setWallColor( mWallColorButton->color() );
}

QgsRendererV2Widget* Qgs25DRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new Qgs25DRendererWidget( layer, style, renderer );
}
