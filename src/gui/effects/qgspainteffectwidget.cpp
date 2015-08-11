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
#include "qgsstylev2.h"
#include "qgsvectorcolorrampv2.h"

//
// draw source
//

QgsDrawSourceWidget::QgsDrawSourceWidget( QWidget *parent )
    : QgsPaintEffectWidget( parent )
    , mEffect( NULL )
{
  setupUi( this );
  initGui();
}


void QgsDrawSourceWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != "drawSource" )
    return;

  mEffect = static_cast<QgsDrawSourceEffect*>( effect );
  initGui();
}

void QgsDrawSourceWidget::initGui()
{
  if ( !mEffect )
  {
    return;
  }

  blockSignals( true );

  mTransparencySpnBx->setValue( mEffect->transparency() * 100.0 );
  mTransparencySlider->setValue( mEffect->transparency() * 1000.0 );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  blockSignals( false );
}

void QgsDrawSourceWidget::blockSignals( const bool block )
{
  mTransparencySlider->blockSignals( block );
  mTransparencySpnBx->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsDrawSourceWidget::on_mTransparencySpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mTransparencySlider->blockSignals( true );
  mTransparencySlider->setValue( value * 10.0 );
  mTransparencySlider->blockSignals( false );

  mEffect->setTransparency( value / 100.0 );
  emit changed();
}

void QgsDrawSourceWidget::on_mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsDrawSourceWidget::on_mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}

void QgsDrawSourceWidget::on_mTransparencySlider_valueChanged( int value )
{
  mTransparencySpnBx->setValue( value / 10.0 );
}


//
// blur
//

QgsBlurWidget::QgsBlurWidget( QWidget *parent )
    : QgsPaintEffectWidget( parent )
    , mEffect( NULL )
{
  setupUi( this );

  mBlurTypeCombo->addItem( tr( "Stack blur (fast)" ), QgsBlurEffect::StackBlur );
  mBlurTypeCombo->addItem( tr( "Gaussian blur (quality)" ), QgsBlurEffect::GaussianBlur );

  initGui();
}


void QgsBlurWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != "blur" )
    return;

  mEffect = static_cast<QgsBlurEffect*>( effect );
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
  mTransparencySpnBx->setValue( mEffect->transparency() * 100.0 );
  mTransparencySlider->setValue( mEffect->transparency() * 1000.0 );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  blockSignals( false );
}

void QgsBlurWidget::blockSignals( const bool block )
{
  mBlurTypeCombo->blockSignals( block );
  mBlurStrengthSpnBx->blockSignals( block );
  mTransparencySlider->blockSignals( block );
  mTransparencySpnBx->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsBlurWidget::on_mBlurTypeCombo_currentIndexChanged( int index )
{
  if ( !mEffect )
    return;

  QgsBlurEffect::BlurMethod method = ( QgsBlurEffect::BlurMethod ) mBlurTypeCombo->itemData( index ).toInt();
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

void QgsBlurWidget::on_mBlurStrengthSpnBx_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setBlurLevel( value );
  emit changed();
}

void QgsBlurWidget::on_mTransparencySpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mTransparencySlider->blockSignals( true );
  mTransparencySlider->setValue( value * 10.0 );
  mTransparencySlider->blockSignals( false );

  mEffect->setTransparency( value / 100.0 );
  emit changed();
}

void QgsBlurWidget::on_mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsBlurWidget::on_mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}

void QgsBlurWidget::on_mTransparencySlider_valueChanged( int value )
{
  mTransparencySpnBx->setValue( value / 10.0 );
}


//
// Drop Shadow
//

QgsShadowEffectWidget::QgsShadowEffectWidget( QWidget *parent )
    : QgsPaintEffectWidget( parent )
    , mEffect( NULL )
{
  setupUi( this );

  mShadowColorBtn->setAllowAlpha( false );
  mShadowColorBtn->setColorDialogTitle( tr( "Select shadow color" ) );
  mShadowColorBtn->setContext( "symbology" );

  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Pixel << QgsSymbolV2::MapUnit );

  initGui();
}

void QgsShadowEffectWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || ( effect->type() != "dropShadow" && effect->type() != "innerShadow" ) )
    return;

  mEffect = static_cast<QgsShadowEffect*>( effect );
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
  mShadowTranspSpnBx->setValue( mEffect->transparency() * 100.0 );
  mShadowTranspSlider->setValue( mEffect->transparency() * 1000.0 );
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
  mShadowTranspSpnBx->blockSignals( block );
  mShadowColorBtn->blockSignals( block );
  mShadowBlendCmbBx->blockSignals( block );
  mShadowTranspSlider->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsShadowEffectWidget::on_mShadowOffsetAngleSpnBx_valueChanged( int value )
{
  mShadowOffsetAngleDial->blockSignals( true );
  mShadowOffsetAngleDial->setValue( value );
  mShadowOffsetAngleDial->blockSignals( false );

  if ( !mEffect )
    return;

  mEffect->setOffsetAngle( value );
  emit changed();
}

void QgsShadowEffectWidget::on_mShadowOffsetAngleDial_valueChanged( int value )
{
  mShadowOffsetAngleSpnBx->setValue( value );
}

void QgsShadowEffectWidget::on_mShadowOffsetSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setOffsetDistance( value );
  emit changed();
}

void QgsShadowEffectWidget::on_mOffsetUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setOffsetUnit( mOffsetUnitWidget->unit() );
  mEffect->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsShadowEffectWidget::on_mShadowTranspSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mShadowTranspSlider->blockSignals( true );
  mShadowTranspSlider->setValue( value * 10.0 );
  mShadowTranspSlider->blockSignals( false );

  mEffect->setTransparency( value / 100.0 );
  emit changed();
}

void QgsShadowEffectWidget::on_mShadowColorBtn_colorChanged( const QColor &color )
{
  if ( !mEffect )
    return;

  mEffect->setColor( color );
  emit changed();
}

void QgsShadowEffectWidget::on_mShadowRadiuSpnBx_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setBlurLevel( value );
  emit changed();
}

void QgsShadowEffectWidget::on_mShadowTranspSlider_valueChanged( int value )
{
  mShadowTranspSpnBx->setValue( value / 10.0 );
}

void QgsShadowEffectWidget::on_mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsShadowEffectWidget::on_mShadowBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index );

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
    , mEffect( NULL )
{
  setupUi( this );

  mColorBtn->setAllowAlpha( false );
  mColorBtn->setColorDialogTitle( tr( "Select glow color" ) );
  mColorBtn->setContext( "symbology" );

  mSpreadUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Pixel << QgsSymbolV2::MapUnit );

  mRampComboBox->populate( QgsStyleV2::defaultStyle() );
  mRampComboBox->setShowGradientOnly( true );
  connect( mRampComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( applyColorRamp() ) );
  connect( mRampComboBox, SIGNAL( sourceRampEdited() ), this, SLOT( applyColorRamp() ) );
  connect( mButtonEditRamp, SIGNAL( clicked() ), mRampComboBox, SLOT( editSourceRamp() ) );

  connect( radioSingleColor, SIGNAL( toggled( bool ) ), this, SLOT( colorModeChanged() ) );

  initGui();
}

void QgsGlowWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || ( effect->type() != "outerGlow" && effect->type() != "innerGlow" ) )
    return;

  mEffect = static_cast<QgsGlowEffect*>( effect );
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
  mTranspSpnBx->setValue( mEffect->transparency() * 100.0 );
  mTranspSlider->setValue( mEffect->transparency() * 1000.0 );
  mColorBtn->setColor( mEffect->color() );
  mBlendCmbBx->setBlendMode( mEffect->blendMode() );

  if ( mEffect->ramp() )
  {
    mRampComboBox->setSourceColorRamp( mEffect->ramp() );
  }

  radioSingleColor->setChecked( mEffect->colorType() == QgsGlowEffect::SingleColor );
  mColorBtn->setEnabled( mEffect->colorType() == QgsGlowEffect::SingleColor );
  radioColorRamp->setChecked( mEffect->colorType() == QgsGlowEffect::ColorRamp );
  mRampComboBox->setEnabled( mEffect->colorType() == QgsGlowEffect::ColorRamp );
  mButtonEditRamp->setEnabled( mEffect->colorType() == QgsGlowEffect::ColorRamp );
  mInvertCheckBox->setEnabled( mEffect->colorType() == QgsGlowEffect::ColorRamp );
  mDrawModeComboBox->setDrawMode( mEffect->drawMode() );

  blockSignals( false );
}

void QgsGlowWidget::blockSignals( const bool block )
{
  mSpreadSpnBx->blockSignals( block );
  mSpreadUnitWidget->blockSignals( block );
  mBlurRadiusSpnBx->blockSignals( block );
  mTranspSpnBx->blockSignals( block );
  mTranspSlider->blockSignals( block );
  mColorBtn->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mRampComboBox->blockSignals( block );
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
    mEffect->setRamp( mRampComboBox->currentColorRamp() );
  }
  emit changed();
}

void QgsGlowWidget::on_mSpreadSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setSpread( value );
  emit changed();
}

void QgsGlowWidget::on_mSpreadUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setSpreadUnit( mSpreadUnitWidget->unit() );
  mEffect->setSpreadMapUnitScale( mSpreadUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsGlowWidget::on_mTranspSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mTranspSlider->blockSignals( true );
  mTranspSlider->setValue( value * 10.0 );
  mTranspSlider->blockSignals( false );

  mEffect->setTransparency( value / 100.0 );
  emit changed();
}

void QgsGlowWidget::on_mColorBtn_colorChanged( const QColor &color )
{
  if ( !mEffect )
    return;

  mEffect->setColor( color );
  emit changed();
}

void QgsGlowWidget::on_mBlurRadiusSpnBx_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setBlurLevel( value );
  emit changed();
}

void QgsGlowWidget::on_mTranspSlider_valueChanged( int value )
{
  mTranspSpnBx->setValue( value / 10.0 );
}

void QgsGlowWidget::on_mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}

void QgsGlowWidget::on_mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

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

  QgsVectorColorRampV2* ramp = mRampComboBox->currentColorRamp();
  if ( ramp == NULL )
    return;

  mEffect->setRamp( ramp );
  emit changed();
}

//
// transform
//

QgsTransformWidget::QgsTransformWidget( QWidget *parent )
    : QgsPaintEffectWidget( parent )
    , mEffect( NULL )
{
  setupUi( this );

  mTranslateUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Pixel << QgsSymbolV2::MapUnit );
  mSpinTranslateX->setClearValue( 0 );
  mSpinTranslateY->setClearValue( 0 );
  mSpinShearX->setClearValue( 0 );
  mSpinShearY->setClearValue( 0 );
  mSpinScaleX->setClearValue( 100.0 );
  mSpinScaleY->setClearValue( 100.0 );

  initGui();
}


void QgsTransformWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != "transform" )
    return;

  mEffect = static_cast<QgsTransformEffect*>( effect );
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


void QgsTransformWidget::on_mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsTransformWidget::on_mSpinTranslateX_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setTranslateX( value );
  emit changed();
}

void QgsTransformWidget::on_mSpinTranslateY_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setTranslateY( value );
  emit changed();
}

void QgsTransformWidget::on_mTranslateUnitWidget_changed()
{
  if ( !mEffect )
  {
    return;
  }

  mEffect->setTranslateUnit( mTranslateUnitWidget->unit() );
  mEffect->setTranslateMapUnitScale( mTranslateUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsTransformWidget::on_mReflectXCheckBox_stateChanged( int state )
{
  if ( !mEffect )
    return;

  mEffect->setReflectX( state == Qt::Checked );
  emit changed();
}

void QgsTransformWidget::on_mReflectYCheckBox_stateChanged( int state )
{
  if ( !mEffect )
    return;

  mEffect->setReflectY( state == Qt::Checked );
  emit changed();
}

void QgsTransformWidget::on_mSpinShearX_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setShearX( value );
  emit changed();
}

void QgsTransformWidget::on_mSpinShearY_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setShearY( value );
  emit changed();
}

void QgsTransformWidget::on_mSpinScaleX_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setScaleX( value / 100.0 );
  emit changed();
}

void QgsTransformWidget::on_mSpinScaleY_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mEffect->setScaleY( value / 100.0 );
  emit changed();
}

void QgsTransformWidget::on_mRotationSpinBox_valueChanged( double value )
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
    , mEffect( NULL )
{
  setupUi( this );

  mBrightnessSpinBox->setClearValue( 0 );
  mContrastSpinBox->setClearValue( 0 );
  mSaturationSpinBox->setClearValue( 0 );
  mColorizeColorButton->setAllowAlpha( false );

  mGrayscaleCombo->addItem( tr( "Off" ), QgsImageOperation::GrayscaleOff );
  mGrayscaleCombo->addItem( tr( "By lightness" ), QgsImageOperation::GrayscaleLightness );
  mGrayscaleCombo->addItem( tr( "By luminosity" ), QgsImageOperation::GrayscaleLuminosity );
  mGrayscaleCombo->addItem( tr( "By average" ), QgsImageOperation::GrayscaleAverage );

  initGui();
}

void QgsColorEffectWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect || effect->type() != "color" )
    return;

  mEffect = static_cast<QgsColorEffect*>( effect );
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
  mSliderSaturation->setValue(( mEffect->saturation() - 1.0 ) * 100.0 );
  mColorizeCheck->setChecked( mEffect->colorizeOn() );
  mSliderColorizeStrength->setValue( mEffect->colorizeStrength() );
  mColorizeColorButton->setColor( mEffect->colorizeColor() );
  int grayscaleIdx = mGrayscaleCombo->findData( QVariant(( int ) mEffect->grayscaleMode() ) );
  mGrayscaleCombo->setCurrentIndex( grayscaleIdx == -1 ? 0 : grayscaleIdx );
  mTranspSpnBx->setValue( mEffect->transparency() * 100.0 );
  mTranspSlider->setValue( mEffect->transparency() * 1000.0 );
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
  mTranspSpnBx->blockSignals( block );
  mTranspSlider->blockSignals( block );
  mBlendCmbBx->blockSignals( block );
  mDrawModeComboBox->blockSignals( block );
}

void QgsColorEffectWidget::enableColorizeControls( const bool enable )
{
  mSliderColorizeStrength->setEnabled( enable );
  mColorizeStrengthSpinBox->setEnabled( enable );
  mColorizeColorButton->setEnabled( enable );
}

void QgsColorEffectWidget::on_mTranspSpnBx_valueChanged( double value )
{
  if ( !mEffect )
    return;

  mTranspSlider->blockSignals( true );
  mTranspSlider->setValue( value * 10.0 );
  mTranspSlider->blockSignals( false );

  mEffect->setTransparency( value / 100.0 );
  emit changed();
}

void QgsColorEffectWidget::on_mBlendCmbBx_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setBlendMode( mBlendCmbBx->blendMode() );
  emit changed();
}

void QgsColorEffectWidget::on_mDrawModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setDrawMode( mDrawModeComboBox->drawMode() );
  emit changed();
}

void QgsColorEffectWidget::on_mTranspSlider_valueChanged( int value )
{
  mTranspSpnBx->setValue( value / 10.0 );
}

void QgsColorEffectWidget::on_mBrightnessSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setBrightness( value );
  emit changed();
}

void QgsColorEffectWidget::on_mContrastSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setContrast( value );
  emit changed();
}

void QgsColorEffectWidget::on_mSaturationSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setSaturation( value / 100.0 + 1 );
  emit changed();
}

void QgsColorEffectWidget::on_mColorizeStrengthSpinBox_valueChanged( int value )
{
  if ( !mEffect )
    return;

  mEffect->setColorizeStrength( value );
  emit changed();
}

void QgsColorEffectWidget::on_mColorizeCheck_stateChanged( int state )
{
  if ( !mEffect )
    return;

  mEffect->setColorizeOn( state == Qt::Checked );
  enableColorizeControls( state == Qt::Checked );
  emit changed();
}

void QgsColorEffectWidget::on_mColorizeColorButton_colorChanged( const QColor &color )
{
  if ( !mEffect )
    return;

  mEffect->setColorizeColor( color );
  emit changed();
}

void QgsColorEffectWidget::on_mGrayscaleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( !mEffect )
    return;

  mEffect->setGrayscaleMode(( QgsImageOperation::GrayscaleMode ) mGrayscaleCombo->itemData( mGrayscaleCombo->currentIndex() ).toInt() );
  emit changed();
}
