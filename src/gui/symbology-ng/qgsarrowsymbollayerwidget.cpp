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

QgsArrowSymbolLayerWidget::QgsArrowSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
    , mLayer( nullptr )
{
  setupUi( this );

  mArrowWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mArrowStartWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mHeadLengthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mHeadThicknessUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

  mOffsetSpin->setClearValue( 0.0 );
}

void QgsArrowSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "ArrowLine" )
  {
    return;
  }

  mLayer = static_cast<QgsArrowSymbolLayer*>( layer );

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

  registerDataDefinedButton( mArrowWidthDDBtn, "arrow_width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mArrowStartWidthDDBtn, "arrow_start_width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mHeadWidthDDBtn, "head_length", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mHeadHeightDDBtn, "head_thickness", QgsDataDefinedButton::Double, QgsDataDefinedButton::doubleDesc() );
  registerDataDefinedButton( mHeadTypeDDBtn, "head_type", QgsDataDefinedButton::Int, QgsDataDefinedButton::intDesc() );
  registerDataDefinedButton( mArrowTypeDDBtn, "arrow_type", QgsDataDefinedButton::Int, QgsDataDefinedButton::intDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::String, QgsDataDefinedButton::doubleDesc() );
}


QgsSymbolLayerV2* QgsArrowSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsArrowSymbolLayerWidget::on_mArrowWidthSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setArrowWidth( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mArrowStartWidthSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setArrowStartWidth( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mHeadLengthSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setHeadLength( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mHeadThicknessSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setHeadThickness( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mArrowWidthUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setArrowWidthUnit( mArrowWidthUnitWidget->unit() );
  mLayer->setArrowWidthUnitScale( mArrowWidthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mArrowStartWidthUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setArrowStartWidthUnit( mArrowStartWidthUnitWidget->unit() );
  mLayer->setArrowStartWidthUnitScale( mArrowStartWidthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mHeadLengthUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setHeadLengthUnit( mHeadLengthUnitWidget->unit() );
  mLayer->setHeadLengthUnitScale( mHeadLengthUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mHeadThicknessUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setHeadThicknessUnit( mHeadThicknessUnitWidget->unit() );
  mLayer->setHeadThicknessUnitScale( mHeadThicknessUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mHeadTypeCombo_currentIndexChanged( int idx )
{
  if ( !mLayer )
    return;

  QgsArrowSymbolLayer::HeadType t = static_cast<QgsArrowSymbolLayer::HeadType>( idx );
  mLayer->setHeadType( t );
  bool isSingle = t == QgsArrowSymbolLayer::HeadSingle || t == QgsArrowSymbolLayer::HeadReversed;
  mArrowStartWidthDDBtn->setEnabled( isSingle );
  mArrowStartWidthSpin->setEnabled( isSingle );
  mArrowStartWidthUnitWidget->setEnabled( isSingle );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mArrowTypeCombo_currentIndexChanged( int idx )
{
  if ( !mLayer )
    return;

  QgsArrowSymbolLayer::ArrowType t = static_cast<QgsArrowSymbolLayer::ArrowType>( idx );
  mLayer->setArrowType( t );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mOffsetSpin_valueChanged( double d )
{
  if ( !mLayer )
    return;

  mLayer->setOffset( d );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mOffsetUnitWidget_changed()
{
  if ( !mLayer )
    return;

  mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
  mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mCurvedArrowChck_stateChanged( int state )
{
  if ( ! mLayer )
    return;

  mLayer->setIsCurved( state == Qt::Checked );
  emit changed();
}

void QgsArrowSymbolLayerWidget::on_mRepeatArrowChck_stateChanged( int state )
{
  if ( ! mLayer )
    return;

  mLayer->setIsRepeated( state == Qt::Checked );
  emit changed();
}
