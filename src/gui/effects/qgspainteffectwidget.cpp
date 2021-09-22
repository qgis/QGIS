/***************************************************************************
    qgspainteffectwidget.cpp
    ------------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgspainteffectwidget.h"
#include "qgslogger.h"
#include "qgspainteffect.h"
#include "qgsshadoweffect.h"
#include "qgsblureffect.h"
#include "qgsgloweffect.h"
#include "qgstransformeffect.h"
#include "qgscoloreffect.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgscolorrampbutton.h"

//
// draw source
//

QgsDrawSourceWidget::QgsDrawSourceWidget( QWidget *parent )
  : QgsPaintEffectWidget( parent )

{
  setupUi( this );
  connect( mDrawModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDrawSourceWidget::mDrawModeComboBox_currentIndexChanged );
  connect( mBlendCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDrawSourceWidget::mBlendCmbBx_currentIndexChanged );
  initGui();
}


void QgsDrawSourceWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != QLatin1String( "drawSource" ) )
    return;

  mEffect = static_cast<QgsDrawSourceEffect *>( effect );
  initGui();

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsDrawSourceWidget::opacityChanged );
}

void QgsDrawSourceWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mOpacityWidget->setOpacity( mEffect->opacity() );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  blockSignals( false );
}

void QgsDrawSourceWidget::blockSignals( const bool block )
{
  mOpacityWidget->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsDrawSourceWidget::opacityChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOpacity( value );
  emit changed();
}

void QgsDrawSourceWidget::mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsDrawSourceWidget::mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}


//
// blur
//

QgsBlurWidget::QgsBlurWidget( QWidget *parent )
  : QgsPaintEffectWidget( parent )

{
  setupUi( this );
  connect( mBlurTypeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsBlurWidget::mBlurTypeCombo_currentIndexChanged );
  connect( mBlurStrengthSpnBx, static_cast< void ( QDoubleSpinBox::* )( double ) >( &QDoubleSpinBox::valueChanged ), this, &QgsBlurWidget::mBlurStrengthSpnBx_valueChanged );
  connect( mBlurUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsBlurWidget::mBlurUnitWidget_changed );
  connect( mDrawModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsBlurWidget::mDrawModeComboBox_currentIndexChanged );
  connect( mBlendCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsBlurWidget::mBlendCmbBx_currentIndexChanged );

  mBlurTypeCombo->addItem( tr( "Stack Blur (fast, doesn't support high dpi)" ), QgsBlurEffect::StackBlur );
  mBlurTypeCombo->addItem( tr( "Gaussian Blur (quality, supports high dpi)" ), QgsBlurEffect::GaussianBlur );

  mBlurUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits
                             << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  initGui();
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsBlurWidget::opacityChanged );
}


void QgsBlurWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != QLatin1String( "blur" ) )
    return;

  mEffect = static_cast<QgsBlurEffect *>( effect );
  initGui();
}

void QgsBlurWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mBlurTypeCombo->setCurrentIndex( mBlurTypeCombo->findData( mEffect->blurMethod() ) );
  mBlurStrengthSpnBx->setValue( mEffect->blurLevel() );
  mOpacityWidget->setOpacity( mEffect->opacity() );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  mBlurUnitWidget->setUnit( mEffect->blurUnit() );
  mBlurUnitWidget->setMapUnitScale( mEffect->blurMapUnitScale() );

  blockSignals( false );
}

void QgsBlurWidget::blockSignals( const bool block )
{
  mBlurTypeCombo->blockSignals( block );
  mBlurStrengthSpnBx->blockSignals( block );
  mOpacityWidget->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsBlurWidget::mBlurTypeCombo_currentIndexChanged( int index )
{
  if ( !mEffect )
    return;

  const QgsBlurEffect::BlurMethod method = ( QgsBlurEffect::BlurMethod ) mBlurTypeCombo->itemData( index ).toInt();
  mEffect->setBlurMethod( method );

  //also update max radius
  switch ( method )
  {
    case QgsBlurEffect::StackBlur:
      mBlurStrengthSpnBx->setMaximum( 16 );
      break;
    case QgsBlurEffect::GaussianBlur:
      mBlurStrengthSpnBx->setMaximum( 200 );
      break;
  }

  emit changed();
}

void QgsBlurWidget::mBlurStrengthSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setBlurLevel( value );
  emit changed();
}

void QgsBlurWidget::mBlurUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setBlurUnit( mBlurUnitWidget->unit() );
  mEffect->setBlurMapUnitScale( mBlurUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsBlurWidget::opacityChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOpacity( value );
  emit changed();
}

void QgsBlurWidget::mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsBlurWidget::mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}


//
// Drop Shadow
//

QgsShadowEffectWidget::QgsShadowEffectWidget( QWidget *parent )
  : QgsPaintEffectWidget( parent )

{
  setupUi( this );
  connect( mShadowOffsetAngleSpnBx, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &QgsShadowEffectWidget::mShadowOffsetAngleSpnBx_valueChanged );
  connect( mShadowOffsetAngleDial, &QDial::valueChanged, this, &QgsShadowEffectWidget::mShadowOffsetAngleDial_valueChanged );
  connect( mShadowOffsetSpnBx, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsShadowEffectWidget::mShadowOffsetSpnBx_valueChanged );
  connect( mOffsetUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsShadowEffectWidget::mOffsetUnitWidget_changed );
  connect( mShadowColorBtn, &QgsColorButton::colorChanged, this, &QgsShadowEffectWidget::mShadowColorBtn_colorChanged );
  connect( mDrawModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsShadowEffectWidget::mDrawModeComboBox_currentIndexChanged );
  connect( mShadowBlendCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsShadowEffectWidget::mShadowBlendCmbBx_currentIndexChanged );
  connect( mShadowRadiuSpnBx, static_cast< void ( QDoubleSpinBox::* )( double ) >( &QDoubleSpinBox::valueChanged ), this, &QgsShadowEffectWidget::mShadowRadiuSpnBx_valueChanged );
  connect( mBlurUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsShadowEffectWidget::mBlurUnitWidget_changed );

  mShadowColorBtn->setAllowOpacity( false );
  mShadowColorBtn->setColorDialogTitle( tr( "Select Shadow Color" ) );
  mShadowColorBtn->setContext( QStringLiteral( "symbology" ) );
  mShadowOffsetAngleSpnBx->setClearValue( 0 );

  mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mBlurUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits
                             << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  initGui();

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsShadowEffectWidget::opacityChanged );
}

void QgsShadowEffectWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || ( effect->type() != QLatin1String( "dropShadow" ) && effect->type() != QLatin1String( "innerShadow" ) ) )
    return;

  mEffect = static_cast<QgsShadowEffect *>( effect );
  initGui();
}

void QgsShadowEffectWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mShadowOffsetAngleSpnBx->setValue( mEffect->offsetAngle() );
  mShadowOffsetAngleDial->setValue( mEffect->offsetAngle() );
  mShadowOffsetSpnBx->setValue( mEffect->offsetDistance() );
  mOffsetUnitWidget->setUnit( mEffect->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mEffect->offsetMapUnitScale() );
  mShadowRadiuSpnBx->setValue( mEffect->blurLevel() );
  mBlurUnitWidget->setUnit( mEffect->blurUnit() );
  mBlurUnitWidget->setMapUnitScale( mEffect->blurMapUnitScale() );
  mOpacityWidget->setOpacity( mEffect->opacity() );
  mShadowColorBtn->setColor( mEffect->color() );
  mShadowBlendCmbBx->setBlendMode( mEffect->blendMode() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  blockSignals( false );
}

void QgsShadowEffectWidget::blockSignals( const bool block )
{
  mShadowOffsetAngleSpnBx->blockSignals( block );
  mShadowOffsetAngleDial->blockSignals( block );
  mShadowOffsetSpnBx->blockSignals( block );
  mOffsetUnitWidget->blockSignals( block );
  mShadowRadiuSpnBx->blockSignals( block );
  mBlurUnitWidget->blockSignals( block );
  mOpacityWidget->blockSignals( block );
  mShadowColorBtn->blockSignals( block );
  mShadowBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsShadowEffectWidget::mShadowOffsetAngleSpnBx_valueChanged( int value )
{
  mShadowOffsetAngleDial->blockSignals( true );
  mShadowOffsetAngleDial->setValue( value );
  mShadowOffsetAngleDial->blockSignals( false );

  if ( !mEffect )
    return;

  mEffect->setOffsetAngle( value );
  emit changed();
}

void QgsShadowEffectWidget::mShadowOffsetAngleDial_valueChanged( int value )
{
  mShadowOffsetAngleSpnBx->setValue( value );
}

void QgsShadowEffectWidget::mShadowOffsetSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOffsetDistance( value );
  emit changed();
}

void QgsShadowEffectWidget::mOffsetUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setOffsetUnit( mOffsetUnitWidget->unit() );
  mEffect->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsShadowEffectWidget::opacityChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOpacity( value );
  emit changed();
}

void QgsShadowEffectWidget::mShadowColorBtn_colorChanged( const QColor &color )
{
  if ( !mEffect )
    return;

  mEffect->setColor( color );
  emit changed();
}

void QgsShadowEffectWidget::mShadowRadiuSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setBlurLevel( value );
  emit changed();
}

void QgsShadowEffectWidget::mBlurUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setBlurUnit( mBlurUnitWidget->unit() );
  mEffect->setBlurMapUnitScale( mBlurUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsShadowEffectWidget::mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsShadowEffectWidget::mShadowBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mShadowBlendCmbBx->blendMode() );
  emit changed();
}



//
// glow
//

QgsGlowWidget::QgsGlowWidget( QWidget *parent )
  : QgsPaintEffectWidget( parent )

{
  setupUi( this );
  connect( mSpreadSpnBx, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsGlowWidget::mSpreadSpnBx_valueChanged );
  connect( mSpreadUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsGlowWidget::mSpreadUnitWidget_changed );
  connect( mColorBtn, &QgsColorButton::colorChanged, this, &QgsGlowWidget::mColorBtn_colorChanged );
  connect( mBlendCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGlowWidget::mBlendCmbBx_currentIndexChanged );
  connect( mDrawModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGlowWidget::mDrawModeComboBox_currentIndexChanged );
  connect( mBlurRadiusSpnBx, static_cast< void ( QDoubleSpinBox::* )( double ) >( &QDoubleSpinBox::valueChanged ), this, &QgsGlowWidget::mBlurRadiusSpnBx_valueChanged );
  connect( mBlurUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsGlowWidget::mBlurUnitWidget_changed );

  mColorBtn->setAllowOpacity( false );
  mColorBtn->setColorDialogTitle( tr( "Select Glow Color" ) );
  mColorBtn->setContext( QStringLiteral( "symbology" ) );

  mSpreadUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mBlurUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits
                             << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  btnColorRamp->setShowGradientOnly( true );

  initGui();

  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsGlowWidget::applyColorRamp );
  connect( radioSingleColor, &QAbstractButton::toggled, this, &QgsGlowWidget::colorModeChanged );
  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsGlowWidget::opacityChanged );
}

void QgsGlowWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || ( effect->type() != QLatin1String( "outerGlow" ) && effect->type() != QLatin1String( "innerGlow" ) ) )
    return;

  mEffect = static_cast<QgsGlowEffect *>( effect );
  initGui();
}

void QgsGlowWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mSpreadSpnBx->setValue( mEffect->spread() );
  mSpreadUnitWidget->setUnit( mEffect->spreadUnit() );
  mSpreadUnitWidget->setMapUnitScale( mEffect->spreadMapUnitScale() );
  mBlurRadiusSpnBx->setValue( mEffect->blurLevel() );
  mBlurUnitWidget->setUnit( mEffect->blurUnit() );
  mBlurUnitWidget->setMapUnitScale( mEffect->blurMapUnitScale() );
  mOpacityWidget->setOpacity( mEffect->opacity() );
  mColorBtn->setColor( mEffect->color() );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );

  if ( mEffect->ramp() )
  {
    btnColorRamp->setColorRamp( mEffect->ramp() );
  }

  radioSingleColor->setChecked( mEffect->colorType() == QgsGlowEffect::SingleColor );
  mColorBtn->setEnabled( mEffect->colorType() == QgsGlowEffect::SingleColor );
  radioColorRamp->setChecked( mEffect->colorType() == QgsGlowEffect::ColorRamp );
  btnColorRamp->setEnabled( mEffect->colorType() == QgsGlowEffect::ColorRamp );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  blockSignals( false );
}

void QgsGlowWidget::blockSignals( const bool block )
{
  mSpreadSpnBx->blockSignals( block );
  mSpreadUnitWidget->blockSignals( block );
  mBlurRadiusSpnBx->blockSignals( block );
  mOpacityWidget->blockSignals( block );
  mColorBtn->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  btnColorRamp->blockSignals( block );
  radioSingleColor->blockSignals( block );
  radioColorRamp->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsGlowWidget::colorModeChanged()
{
  if ( !mEffect )
  {
    return;
  }

  if ( radioSingleColor->isChecked() )
  {
    mEffect->setColorType( QgsGlowEffect::SingleColor );
  }
  else
  {
    mEffect->setColorType( QgsGlowEffect::ColorRamp );
    mEffect->setRamp( btnColorRamp->colorRamp() );
  }
  emit changed();
}

void QgsGlowWidget::mSpreadSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setSpread( value );
  emit changed();
}

void QgsGlowWidget::mSpreadUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setSpreadUnit( mSpreadUnitWidget->unit() );
  mEffect->setSpreadMapUnitScale( mSpreadUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsGlowWidget::opacityChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOpacity( value );
  emit changed();
}

void QgsGlowWidget::mColorBtn_colorChanged( const QColor &color )
{
  if ( !mEffect )
    return;

  mEffect->setColor( color );
  emit changed();
}

void QgsGlowWidget::mBlurRadiusSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setBlurLevel( value );
  emit changed();
}

void QgsGlowWidget::mBlurUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setBlurUnit( mBlurUnitWidget->unit() );
  mEffect->setBlurMapUnitScale( mBlurUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsGlowWidget::mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}

void QgsGlowWidget::mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsGlowWidget::applyColorRamp()
{
  if ( !mEffect )
  {
    return;
  }

  QgsColorRamp *ramp = btnColorRamp->colorRamp();
  if ( !ramp )
    return;

  mEffect->setRamp( ramp );
  emit changed();
}

//
// transform
//

QgsTransformWidget::QgsTransformWidget( QWidget *parent )
  : QgsPaintEffectWidget( parent )

{
  setupUi( this );
  connect( mDrawModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTransformWidget::mDrawModeComboBox_currentIndexChanged );
  connect( mSpinTranslateX, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mSpinTranslateX_valueChanged );
  connect( mSpinTranslateY, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mSpinTranslateY_valueChanged );
  connect( mTranslateUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTransformWidget::mTranslateUnitWidget_changed );
  connect( mReflectXCheckBox, &QCheckBox::stateChanged, this, &QgsTransformWidget::mReflectXCheckBox_stateChanged );
  connect( mReflectYCheckBox, &QCheckBox::stateChanged, this, &QgsTransformWidget::mReflectYCheckBox_stateChanged );
  connect( mSpinShearX, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mSpinShearX_valueChanged );
  connect( mSpinShearY, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mSpinShearY_valueChanged );
  connect( mSpinScaleX, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mSpinScaleX_valueChanged );
  connect( mSpinScaleY, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mSpinScaleY_valueChanged );
  connect( mRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTransformWidget::mRotationSpinBox_valueChanged );

  mTranslateUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderMapUnits
                                  << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mSpinTranslateX->setClearValue( 0 );
  mSpinTranslateY->setClearValue( 0 );
  mRotationSpinBox->setClearValue( 0 );
  mSpinShearX->setClearValue( 0 );
  mSpinShearY->setClearValue( 0 );
  mSpinScaleX->setClearValue( 100.0 );
  mSpinScaleY->setClearValue( 100.0 );

  initGui();
}


void QgsTransformWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != QLatin1String( "transform" ) )
    return;

  mEffect = static_cast<QgsTransformEffect *>( effect );
  initGui();
}

void QgsTransformWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mReflectXCheckBox->setChecked( mEffect->reflectX() );
  mReflectYCheckBox->setChecked( mEffect->reflectY() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );
  mSpinTranslateX->setValue( mEffect->translateX() );
  mSpinTranslateY->setValue( mEffect->translateY() );
  mTranslateUnitWidget->setUnit( mEffect->translateUnit() );
  mTranslateUnitWidget->setMapUnitScale( mEffect->translateMapUnitScale() );
  mSpinShearX->setValue( mEffect->shearX() );
  mSpinShearY->setValue( mEffect->shearY() );
  mSpinScaleX->setValue( mEffect->scaleX() * 100.0 );
  mSpinScaleY->setValue( mEffect->scaleY() * 100.0 );
  mRotationSpinBox->setValue( mEffect->rotation() );

  blockSignals( false );
}

void QgsTransformWidget::blockSignals( const bool block )
{
  mDrawModeComboBox->blockSignals( block );
  mTranslateUnitWidget->blockSignals( block );
  mSpinTranslateX->blockSignals( block );
  mSpinTranslateY->blockSignals( block );
  mReflectXCheckBox->blockSignals( block );
  mReflectYCheckBox->blockSignals( block );
  mSpinShearX->blockSignals( block );
  mSpinShearY->blockSignals( block );
  mSpinScaleX->blockSignals( block );
  mSpinScaleY->blockSignals( block );
  mRotationSpinBox->blockSignals( block );
}


void QgsTransformWidget::mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsTransformWidget::mSpinTranslateX_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setTranslateX( value );
  emit changed();
}

void QgsTransformWidget::mSpinTranslateY_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setTranslateY( value );
  emit changed();
}

void QgsTransformWidget::mTranslateUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setTranslateUnit( mTranslateUnitWidget->unit() );
  mEffect->setTranslateMapUnitScale( mTranslateUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsTransformWidget::mReflectXCheckBox_stateChanged( int state )
{
  if ( !mEffect )
    return;

  mEffect->setReflectX( state == Qt::Checked );
  emit changed();
}

void QgsTransformWidget::mReflectYCheckBox_stateChanged( int state )
{
  if ( !mEffect )
    return;

  mEffect->setReflectY( state == Qt::Checked );
  emit changed();
}

void QgsTransformWidget::mSpinShearX_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setShearX( value );
  emit changed();
}

void QgsTransformWidget::mSpinShearY_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setShearY( value );
  emit changed();
}

void QgsTransformWidget::mSpinScaleX_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setScaleX( value / 100.0 );
  emit changed();
}

void QgsTransformWidget::mSpinScaleY_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setScaleY( value / 100.0 );
  emit changed();
}

void QgsTransformWidget::mRotationSpinBox_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setRotation( value );
  emit changed();
}


//
// color effect
//

QgsColorEffectWidget::QgsColorEffectWidget( QWidget *parent )
  : QgsPaintEffectWidget( parent )

{
  setupUi( this );
  connect( mBlendCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorEffectWidget::mBlendCmbBx_currentIndexChanged );
  connect( mDrawModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorEffectWidget::mDrawModeComboBox_currentIndexChanged );
  connect( mBrightnessSpinBox, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &QgsColorEffectWidget::mBrightnessSpinBox_valueChanged );
  connect( mContrastSpinBox, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &QgsColorEffectWidget::mContrastSpinBox_valueChanged );
  connect( mSaturationSpinBox, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &QgsColorEffectWidget::mSaturationSpinBox_valueChanged );
  connect( mColorizeStrengthSpinBox, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this, &QgsColorEffectWidget::mColorizeStrengthSpinBox_valueChanged );
  connect( mColorizeCheck, &QCheckBox::stateChanged, this, &QgsColorEffectWidget::mColorizeCheck_stateChanged );
  connect( mColorizeColorButton, &QgsColorButton::colorChanged, this, &QgsColorEffectWidget::mColorizeColorButton_colorChanged );
  connect( mGrayscaleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorEffectWidget::mGrayscaleCombo_currentIndexChanged );

  mBrightnessSpinBox->setClearValue( 0 );
  mContrastSpinBox->setClearValue( 0 );
  mSaturationSpinBox->setClearValue( 0 );
  mColorizeStrengthSpinBox->setClearValue( 100 );
  mColorizeColorButton->setAllowOpacity( false );

  mGrayscaleCombo->addItem( tr( "Off" ), QgsImageOperation::GrayscaleOff );
  mGrayscaleCombo->addItem( tr( "By Lightness" ), QgsImageOperation::GrayscaleLightness );
  mGrayscaleCombo->addItem( tr( "By Luminosity" ), QgsImageOperation::GrayscaleLuminosity );
  mGrayscaleCombo->addItem( tr( "By Average" ), QgsImageOperation::GrayscaleAverage );

  initGui();

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsColorEffectWidget::opacityChanged );
}

void QgsColorEffectWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != QLatin1String( "color" ) )
    return;

  mEffect = static_cast<QgsColorEffect *>( effect );
  initGui();
}

void QgsColorEffectWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mSliderBrightness->setValue( mEffect->brightness() );
  mSliderContrast->setValue( mEffect->contrast() );
  mSliderSaturation->setValue( ( mEffect->saturation() - 1.0 ) * 100.0 );
  mColorizeCheck->setChecked( mEffect->colorizeOn() );
  mSliderColorizeStrength->setValue( mEffect->colorizeStrength() );
  mColorizeColorButton->setColor( mEffect->colorizeColor() );
  const int grayscaleIdx = mGrayscaleCombo->findData( QVariant( ( int ) mEffect->grayscaleMode() ) );
  mGrayscaleCombo->setCurrentIndex( grayscaleIdx == -1 ? 0 : grayscaleIdx );
  mOpacityWidget->setOpacity( mEffect->opacity() );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );
  enableColorizeControls( mEffect->colorizeOn() );

  blockSignals( false );
}

void QgsColorEffectWidget::blockSignals( const bool block )
{
  mBrightnessSpinBox->blockSignals( block );
  mContrastSpinBox->blockSignals( block );
  mSaturationSpinBox->blockSignals( block );
  mColorizeStrengthSpinBox->blockSignals( block );
  mColorizeCheck->blockSignals( block );
  mColorizeColorButton->blockSignals( block );
  mGrayscaleCombo->blockSignals( block );
  mOpacityWidget->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsColorEffectWidget::enableColorizeControls( const bool enable )
{
  mSliderColorizeStrength->setEnabled( enable );
  mColorizeStrengthSpinBox->setEnabled( enable );
  mColorizeColorButton->setEnabled( enable );
}

void QgsColorEffectWidget::opacityChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOpacity( value );
  emit changed();
}

void QgsColorEffectWidget::mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}

void QgsColorEffectWidget::mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsColorEffectWidget::mBrightnessSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setBrightness( value );
  emit changed();
}

void QgsColorEffectWidget::mContrastSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setContrast( value );
  emit changed();
}

void QgsColorEffectWidget::mSaturationSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setSaturation( value / 100.0 + 1 );
  emit changed();
}

void QgsColorEffectWidget::mColorizeStrengthSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setColorizeStrength( value );
  emit changed();
}

void QgsColorEffectWidget::mColorizeCheck_stateChanged( int state )
{
  if ( !mEffect )
    return;

  mEffect->setColorizeOn( state == Qt::Checked );
  enableColorizeControls( state == Qt::Checked );
  emit changed();
}

void QgsColorEffectWidget::mColorizeColorButton_colorChanged( const QColor &color )
{
  if ( !mEffect )
    return;

  mEffect->setColorizeColor( color );
  emit changed();
}

void QgsColorEffectWidget::mGrayscaleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  if ( !mEffect )
    return;

  mEffect->setGrayscaleMode( ( QgsImageOperation::GrayscaleMode ) mGrayscaleCombo->currentData().toInt() );
  emit changed();
}
