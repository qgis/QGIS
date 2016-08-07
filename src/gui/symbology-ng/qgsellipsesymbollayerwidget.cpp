/***************************************************************************
 qgsellipsesymbollayerwidget.cpp
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
#include "qgsellipsesymbollayerwidget.h"
#include "qgsellipsesymbollayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>

QgsEllipseSymbolLayerWidget::QgsEllipseSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerWidget( parent, vl )
    , mLayer( nullptr )
{
  setupUi( this );

  mSymbolWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mSymbolHeightUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mOutlineWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );
  mOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels );

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
    QgsEllipseSymbolLayer* lyr = new QgsEllipseSymbolLayer();
    lyr->setSymbolName( name );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    lyr->setFillColor( QColor( 200, 200, 200 ) );
    lyr->setSymbolWidth( 4 );
    lyr->setSymbolHeight( 2 );
    QIcon icon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( lyr, QgsUnitTypes::RenderMillimeters, iconSize );
    QListWidgetItem* item = new QListWidgetItem( icon, "", mShapeListWidget );
    item->setToolTip( name );
    item->setData( Qt::UserRole, name );
    delete lyr;
  }

  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penJoinStyleChanged() ) );
}

void QgsEllipseSymbolLayerWidget::setSymbolLayer( QgsSymbolLayer* layer )
{
  if ( !layer || layer->layerType() != "EllipseMarker" )
  {
    return;
  }

  mLayer = static_cast<QgsEllipseSymbolLayer*>( layer );
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

QgsSymbolLayer* QgsEllipseSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsEllipseSymbolLayerWidget::on_mShapeListWidget_itemSelectionChanged()
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

void QgsEllipseSymbolLayerWidget::on_mWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mHeightSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSymbolHeight( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mOutlineStyleComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  if ( mLayer )
  {
    mLayer->setOutlineStyle( mOutlineStyleComboBox->penStyle() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_btnChangeColorBorder_colorChanged( const QColor& newColor )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setOutlineColor( newColor );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::on_btnChangeColorFill_colorChanged( const QColor& newColor )
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setFillColor( newColor );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::on_mSymbolWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSymbolWidthUnit( mSymbolWidthUnitWidget->unit() );
    mLayer->setSymbolWidthMapUnitScale( mSymbolWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOutlineWidthUnit( mOutlineWidthUnitWidget->unit() );
    mLayer->setOutlineWidthMapUnitScale( mOutlineWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mSymbolHeightUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setSymbolHeightUnit( mSymbolHeightUnitWidget->unit() );
    mLayer->setSymbolHeightMapUnitScale( mSymbolHeightUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    mLayer->setOffsetUnit( mOffsetUnitWidget->unit() );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::penJoinStyleChanged()
{
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  emit changed();
}

void QgsEllipseSymbolLayerWidget::blockComboSignals( bool block )
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

void QgsEllipseSymbolLayerWidget::on_mHorizontalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setHorizontalAnchorPoint(( QgsMarkerSymbolLayer::HorizontalAnchorPoint ) index );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::on_mVerticalAnchorComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setVerticalAnchorPoint(( QgsMarkerSymbolLayer::VerticalAnchorPoint ) index );
    emit changed();
  }
}

void QgsEllipseSymbolLayerWidget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}


