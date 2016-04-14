/***************************************************************************
 qgsellipsesymbollayerv2widget.cpp
 ---------------------
 begin                : June 2011
 copyright            : (C) 2011 by Marco Hugentobler
 email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsellipsesymbollayerv2widget.h"
#include "qgsdatadefinedsymboldialog.h"
#include "qgsellipsesymbollayerv2.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>

QgsEllipseSymbolLayerV2Widget::QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
    , mLayer( nullptr )
{
  setupUi( this );

  mSymbolWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mSymbolHeightUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOutlineWidthUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );
  mOffsetUnitWidget->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::MapUnit << QgsSymbolV2::Pixel );

  btnChangeColorFill->setAllowAlpha( true );
  btnChangeColorFill->setColorDialogTitle( tr( "Select fill color" ) );
  btnChangeColorFill->setContext( "symbology" );
  btnChangeColorFill->setShowNoColor( true );
  btnChangeColorFill->setNoColorString( tr( "Transparent fill" ) );
  btnChangeColorBorder->setAllowAlpha( true );
  btnChangeColorBorder->setColorDialogTitle( tr( "Select border color" ) );
  btnChangeColorBorder->setContext( "symbology" );
  btnChangeColorBorder->setShowNoColor( true );
  btnChangeColorBorder->setNoColorString( tr( "Transparent border" ) );

  spinOffsetX->setClearValue( 0.0 );
  spinOffsetY->setClearValue( 0.0 );
  mRotationSpinBox->setClearValue( 0.0 );

  QStringList names;
  names << "circle" << "rectangle" << "diamond" << "cross" << "triangle" << "right_half_triangle" << "left_half_triangle" << "semi_circle";
  QSize iconSize = mShapeListWidget->iconSize();

  Q_FOREACH ( const QString& name, names )
  {
    QgsEllipseSymbolLayerV2* lyr = new QgsEllipseSymbolLayerV2();
    lyr->setSymbolName( name );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    lyr->setFillColor( QColor( 200, 200, 200 ) );
    lyr->setSymbolWidth( 4 );
    lyr->setSymbolHeight( 2 );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, iconSize );
    QListWidgetItem* item = new QListWidgetItem( icon, "", mShapeListWidget );
    item->setToolTip( name );
    item->setData( Qt::UserRole, name );
    delete lyr;
  }

  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penJoinStyleChanged() ) );
}

void QgsEllipseSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "EllipseMarker" )
  {
    return;
  }

  mLayer = static_cast<QgsEllipseSymbolLayerV2*>( layer );
  mWidthSpinBox->setValue( mLayer->symbolWidth() );
  mHeightSpinBox->setValue( mLayer->symbolHeight() );
  mRotationSpinBox->setValue( mLayer->angle() );
  mOutlineStyleComboBox->setPenStyle( mLayer->outlineStyle() );
  mOutlineWidthSpinBox->setValue( mLayer->outlineWidth() );
  btnChangeColorBorder->setColor( mLayer->outlineColor() );
  btnChangeColorFill->setColor( mLayer->fillColor() );

  QList<QListWidgetItem *> symbolItemList = mShapeListWidget->findItems( mLayer->symbolName(), Qt::MatchExactly );
  if ( !symbolItemList.isEmpty() )
  {
    mShapeListWidget->setCurrentItem( symbolItemList.at( 0 ) );
  }

  //set combo entries to current values
  blockComboSignals( true );
  mSymbolWidthUnitWidget->setUnit( mLayer->symbolWidthUnit() );
  mSymbolWidthUnitWidget->setMapUnitScale( mLayer->symbolWidthMapUnitScale() );
  mOutlineWidthUnitWidget->setUnit( mLayer->outlineWidthUnit() );
  mOutlineWidthUnitWidget->setMapUnitScale( mLayer->outlineWidthMapUnitScale() );
  mSymbolHeightUnitWidget->setUnit( mLayer->symbolHeightUnit() );
  mSymbolHeightUnitWidget->setMapUnitScale( mLayer->symbolHeightMapUnitScale() );
  mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
  mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  QPointF offsetPt = mLayer->offset();
  spinOffsetX->setValue( offsetPt.x() );
  spinOffsetY->setValue( offsetPt.y() );
  mHorizontalAnchorComboBox->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  mVerticalAnchorComboBox->setCurrentIndex( mLayer->verticalAnchorPoint() );
  cboJoinStyle->setPenJoinStyle( mLayer->penJoinStyle() );
  blockComboSignals( false );

  registerDataDefinedButton( mSymbolWidthDDBtn, "width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mSymbolHeightDDBtn, "height", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mRotationDDBtn, "rotation", QgsDataDefinedButton::Double, QgsDataDefinedButton::double180RotDesc() );
  registerDataDefinedButton( mOutlineWidthDDBtn, "outline_width", QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  registerDataDefinedButton( mFillColorDDBtn, "fill_color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mBorderColorDDBtn, "outline_color", QgsDataDefinedButton::String, QgsDataDefinedButton::colorAlphaDesc() );
  registerDataDefinedButton( mOutlineStyleDDBtn, "outline_style", QgsDataDefinedButton::String, QgsDataDefinedButton::lineStyleDesc() );
  registerDataDefinedButton( mJoinStyleDDBtn, "join_style", QgsDataDefinedButton::String, QgsDataDefinedButton::penJoinStyleDesc() );
  registerDataDefinedButton( mShapeDDBtn, "symbol_name", QgsDataDefinedButton::String, QgsDataDefinedButton::markerStyleDesc() );
  registerDataDefinedButton( mOffsetDDBtn, "offset", QgsDataDefinedButton::String, QgsDataDefinedButton::doubleXYDesc() );
  registerDataDefinedButton( mHorizontalAnchorDDBtn, "horizontal_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::horizontalAnchorDesc() );
  registerDataDefinedButton( mVerticalAnchorDDBtn, "vertical_anchor_point", QgsDataDefinedButton::String, QgsDataDefinedButton::verticalAnchorDesc() );

}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsEllipseSymbolLayerV2Widget::on_mShapeListWidget_itemSelectionChanged()
{
  if ( mLayer )
  {
    QListWidgetItem* item = mShapeListWidget->currentItem();
    if ( item )
    {
      mLayer->setSymbolName( item->data( Qt::UserRole ).toString() );
      emit changed();
    }
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mHeightSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolHeight( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineStyleComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( mLayer )
  {
    mLayer->setOutlineStyle( mOutlineStyleComboBox->penStyle() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorBorder_colorChanged( const QColor& newColor )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setOutlineColor( newColor );
  emit changed();
}

void QgsEllipseSymbolLayerV2Widget::on_btnChangeColorFill_colorChanged( const QColor& newColor )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setFillColor( newColor );
  emit changed();
}

void QgsEllipseSymbolLayerV2Widget::on_mSymbolWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSymbolWidthUnit( mSymbolWidthUnitWidget->unit() );
    mLayer->setSymbolWidthMapUnitScale( mSymbolWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthUnit( mOutlineWidthUnitWidget->unit() );
    mLayer->setOutlineWidthMapUnitScale( mOutlineWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mSymbolHeightUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSymbolHeightUnit( mSymbolHeightUnitWidget->unit() );
    mLayer->setSymbolHeightMapUnitScale( mSymbolHeightUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::penJoinStyleChanged()
{
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  emit changed();
}

void QgsEllipseSymbolLayerV2Widget::blockComboSignals( bool block )
{
  mSymbolWidthUnitWidget->blockSignals( block );
  mOutlineWidthUnitWidget->blockSignals( block );
  mSymbolHeightUnitWidget->blockSignals( block );
  mHorizontalAnchorComboBox->blockSignals( block );
  mVerticalAnchorComboBox->blockSignals( block );
  mSymbolWidthUnitWidget->blockSignals( block );
  mOutlineWidthUnitWidget->blockSignals( block );
  mSymbolHeightUnitWidget->blockSignals( block );
  mOffsetUnitWidget->blockSignals( block );
  cboJoinStyle->blockSignals( block );
}

void QgsEllipseSymbolLayerV2Widget::on_mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint(( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint ) index );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint(( QgsMarkerSymbolLayerV2::VerticalAnchorPoint ) index );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}


