/***************************************************************************
 qgsarrowsymbollayerwidget.cpp
 ---------------------
 begin                : February 2016
 copyright            : (C) 2016 by Hugo Mercier / Oslandia
 email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsarrowsymbollayerwidget.h"
#include "qgsarrowsymbollayer.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>

QgsArrowSymbolLayerWidget::QgsArrowSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent )
  : QgsSymbolLayerWidget( parent, vl )

{
  setupUi( this );
  connect( mArrowWidthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsArrowSymbolLayerWidget::mArrowWidthSpin_valueChanged );
  connect( mArrowWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsArrowSymbolLayerWidget::mArrowWidthUnitWidget_changed );
  connect( mArrowStartWidthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsArrowSymbolLayerWidget::mArrowStartWidthSpin_valueChanged );
  connect( mArrowStartWidthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsArrowSymbolLayerWidget::mArrowStartWidthUnitWidget_changed );
  connect( mHeadLengthSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsArrowSymbolLayerWidget::mHeadLengthSpin_valueChanged );
  connect( mHeadLengthUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsArrowSymbolLayerWidget::mHeadLengthUnitWidget_changed );
  connect( mHeadThicknessSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsArrowSymbolLayerWidget::mHeadThicknessSpin_valueChanged );
  connect( mHeadThicknessUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsArrowSymbolLayerWidget::mHeadThicknessUnitWidget_changed );
  connect( mHeadTypeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsArrowSymbolLayerWidget::mHeadTypeCombo_currentIndexChanged );
  connect( mArrowTypeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsArrowSymbolLayerWidget::mArrowTypeCombo_currentIndexChanged );
  connect( mOffsetSpin, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsArrowSymbolLayerWidget::mOffsetSpin_valueChanged );
  connect( mOffsetUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsArrowSymbolLayerWidget::mOffsetUnitWidget_changed );
  connect( mCurvedArrowChck, &QCheckBox::stateChanged, this, &QgsArrowSymbolLayerWidget::mCurvedArrowChck_stateChanged );
  connect( mRepeatArrowChck, &QCheckBox::stateChanged, this, &QgsArrowSymbolLayerWidget::mRepeatArrowChck_stateChanged );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mArrowWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                   << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mArrowStartWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                        << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mHeadLengthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                   << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mHeadThicknessUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                      << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  mOffsetSpin->setClearValue( 0.0 );
}

void QgsArrowSymbolLayerWidget::setSymbolLayer( QgsSymbolLayer *layer )
{
  if ( !layer || layer->layerType() != QLatin1String( "ArrowLine" ) )
  {
    return;
  }

  mLayer = static_cast<QgsArrowSymbolLayer *>( layer );

  mArrowWidthSpin->setValue( mLayer->arrowWidth() );
  mArrowWidthUnitWidget->setUnit( mLayer->arrowWidthUnit() );
  mArrowWidthUnitWidget->setMapUnitScale( mLayer->arrowWidthUnitScale() );

  mArrowStartWidthSpin->setValue( mLayer->arrowStartWidth() );
  mArrowStartWidthUnitWidget->setUnit( mLayer->arrowStartWidthUnit() );
  mArrowStartWidthUnitWidget->setMapUnitScale( mLayer->arrowStartWidthUnitScale() );

  mHeadLengthSpin->setValue( mLayer->headLength() );
  mHeadLengthUnitWidget->setUnit( mLayer->headLengthUnit() );
  mHeadLengthUnitWidget->setMapUnitScale( mLayer->headLengthUnitScale() );
  mHeadThicknessSpin->setValue( mLayer->headThickness() );
  mHeadThicknessUnitWidget->setUnit( mLayer->headThicknessUnit() );
  mHeadThicknessUnitWidget->setMapUnitScale( mLayer->headThicknessUnitScale() );

  mHeadTypeCombo->setCurrentIndex( mLayer->headType() );
  mArrowTypeCombo->setCurrentIndex( mLayer->arrowType() );

  mOffsetSpin->setValue( mLayer->offset() );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );

  mCurvedArrowChck->setChecked( mLayer->isCurved() );
  mRepeatArrowChck->setChecked( mLayer->isRepeated() );

  registerDataDefinedButton( mArrowWidthDDBtn, QgsSymbolLayer::PropertyArrowWidth );
  registerDataDefinedButton( mArrowStartWidthDDBtn, QgsSymbolLayer::PropertyArrowStartWidth );
  registerDataDefinedButton( mHeadWidthDDBtn, QgsSymbolLayer::PropertyArrowHeadLength );
  registerDataDefinedButton( mHeadHeightDDBtn, QgsSymbolLayer::PropertyArrowHeadThickness );
  registerDataDefinedButton( mHeadTypeDDBtn, QgsSymbolLayer::PropertyArrowHeadType );
  registerDataDefinedButton( mArrowTypeDDBtn, QgsSymbolLayer::PropertyArrowType );
  registerDataDefinedButton( mOffsetDDBtn, QgsSymbolLayer::PropertyOffset );
}


QgsSymbolLayer *QgsArrowSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsArrowSymbolLayerWidget::mArrowWidthSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setArrowWidth( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mArrowStartWidthSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setArrowStartWidth( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mHeadLengthSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setHeadLength( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mHeadThicknessSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setHeadThickness( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mArrowWidthUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setArrowWidthUnit( mArrowWidthUnitWidget->unit() );
  mLayer->setArrowWidthUnitScale( mArrowWidthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mArrowStartWidthUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setArrowStartWidthUnit( mArrowStartWidthUnitWidget->unit() );
  mLayer->setArrowStartWidthUnitScale( mArrowStartWidthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mHeadLengthUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setHeadLengthUnit( mHeadLengthUnitWidget->unit() );
  mLayer->setHeadLengthUnitScale( mHeadLengthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mHeadThicknessUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setHeadThicknessUnit( mHeadThicknessUnitWidget->unit() );
  mLayer->setHeadThicknessUnitScale( mHeadThicknessUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mHeadTypeCombo_currentIndexChanged( int idx )
{
  if ( !mLayer )
    return;

  const QgsArrowSymbolLayer::HeadType t = static_cast<QgsArrowSymbolLayer::HeadType>( idx );
  mLayer->setHeadType( t );
  const bool isSingle = t == QgsArrowSymbolLayer::HeadSingle || t == QgsArrowSymbolLayer::HeadReversed;
  mArrowStartWidthDDBtn->setEnabled( isSingle );
  mArrowStartWidthSpin->setEnabled( isSingle );
  mArrowStartWidthUnitWidget->setEnabled( isSingle );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mArrowTypeCombo_currentIndexChanged( int idx )
{
  if ( !mLayer )
    return;

  const QgsArrowSymbolLayer::ArrowType t = static_cast<QgsArrowSymbolLayer::ArrowType>( idx );
  mLayer->setArrowType( t );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mOffsetSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setOffset( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mOffsetUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
  mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mCurvedArrowChck_stateChanged( int state )
{
  if ( ! mLayer )
    return;

  mLayer->setIsCurved( state == Qt::Checked );
  emit changed();
}

void QgsArrowSymbolLayerWidget::mRepeatArrowChck_stateChanged( int state )
{
  if ( ! mLayer )
    return;

  mLayer->setIsRepeated( state == Qt::Checked );
  emit changed();
}
