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
#include "qgs25drenderer.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"

Qgs25DRendererWidget::Qgs25DRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )

{
  if ( !layer )
    return;

  // the renderer only applies to polygon vector layers
  if ( layer->geometryType() != Qgis::GeometryType::Polygon )
  {
    //setup blank dialog
    QGridLayout *layout = new QGridLayout( this );
    QLabel *label = new QLabel( tr( "The 2.5D renderer only can be used with polygon layers. \n"
                                    "'%1' is not a polygon layer and cannot be rendered in 2.5D." )
                                .arg( layer->name() ), this );
    layout->addWidget( label );
    return;
  }

  setupUi( this );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mAngleWidget->setClearValue( 0 );
  mWallColorButton->setColorDialogTitle( tr( "Select Wall Color" ) );
  mWallColorButton->setAllowOpacity( true );
  mWallColorButton->setContext( QStringLiteral( "symbology" ) );
  mRoofColorButton->setColorDialogTitle( tr( "Select Roof Color" ) );
  mRoofColorButton->setAllowOpacity( true );
  mRoofColorButton->setContext( QStringLiteral( "symbology" ) );
  mShadowColorButton->setColorDialogTitle( tr( "Select Shadow Color" ) );
  mShadowColorButton->setAllowOpacity( true );
  mShadowColorButton->setContext( QStringLiteral( "symbology" ) );

  if ( renderer )
  {
    mRenderer.reset( Qgs25DRenderer::convertFromRenderer( renderer ) );
  }

  mHeightWidget->setLayer( layer );

  QgsExpressionContextScope *scope = QgsExpressionContextUtils::layerScope( mLayer );
  const QVariant height = scope->variable( QStringLiteral( "qgis_25d_height" ) );
  const QVariant angle = scope->variable( QStringLiteral( "qgis_25d_angle" ) );
  delete scope;

  mHeightWidget->setField( QgsVariantUtils::isNull( height ) ? QStringLiteral( "10" ) : height.toString() );
  mAngleWidget->setValue( QgsVariantUtils::isNull( angle ) ? 70 : angle.toDouble() );
  mAngleWidget->setClearValue( 70 );
  mWallColorButton->setColor( mRenderer->wallColor() );
  mRoofColorButton->setColor( mRenderer->roofColor() );
  mShadowColorButton->setColor( mRenderer->shadowColor() );
  mShadowEnabledWidget->setChecked( mRenderer->shadowEnabled() );
  mShadowSizeWidget->setValue( mRenderer->shadowSpread() );
  mShadowSizeWidget->setClearValue( 4 );
  mWallExpositionShading->setChecked( mRenderer->wallShadingEnabled() );

  connect( mAngleWidget, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &Qgs25DRendererWidget::updateRenderer );
  connect( mHeightWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &Qgs25DRendererWidget::updateRenderer );
  connect( mWallColorButton, &QgsColorButton::colorChanged, this, &Qgs25DRendererWidget::updateRenderer );
  connect( mRoofColorButton, &QgsColorButton::colorChanged, this, &Qgs25DRendererWidget::updateRenderer );
  connect( mShadowColorButton, &QgsColorButton::colorChanged, this, &Qgs25DRendererWidget::updateRenderer );
  connect( mShadowEnabledWidget, &QGroupBox::toggled, this, &Qgs25DRendererWidget::updateRenderer );
  connect( mShadowSizeWidget, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &Qgs25DRendererWidget::updateRenderer );
  connect( mWallExpositionShading, &QAbstractButton::toggled, this, &Qgs25DRendererWidget::updateRenderer );
}

Qgs25DRendererWidget::~Qgs25DRendererWidget() = default;

QgsFeatureRenderer *Qgs25DRendererWidget::renderer()
{
  return mRenderer.get();
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
    QgsExpressionContextUtils::setLayerVariable( mLayer, QStringLiteral( "qgis_25d_height" ), mHeightWidget->currentText() );
    QgsExpressionContextUtils::setLayerVariable( mLayer, QStringLiteral( "qgis_25d_angle" ), mAngleWidget->value() );

    emit layerVariablesChanged();
  }
}

QgsRendererWidget *Qgs25DRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new Qgs25DRendererWidget( layer, style, renderer );
}
