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

QgsEllipseSymbolLayerV2Widget::QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl )
{
  setupUi( this );

  mSymbolWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mSymbolHeightUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOutlineWidthUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );
  mOffsetUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

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
  names << "circle" << "rectangle" << "cross" << "triangle";
  QSize iconSize = mShapeListWidget->iconSize();

  QStringList::const_iterator nameIt = names.constBegin();
  for ( ; nameIt != names.constEnd(); ++nameIt )
  {
    QgsEllipseSymbolLayerV2* lyr = new QgsEllipseSymbolLayerV2();
    lyr->setSymbolName( *nameIt );
    lyr->setOutlineColor( QColor( 0, 0, 0 ) );
    lyr->setFillColor( QColor( 200, 200, 200 ) );
    lyr->setSymbolWidth( 4 );
    lyr->setSymbolHeight( 2 );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, iconSize );
    QListWidgetItem* item = new QListWidgetItem( icon, "", mShapeListWidget );
    item->setToolTip( *nameIt );
    item->setData( Qt::UserRole, *nameIt );
    delete lyr;
  }

  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
}

void QgsEllipseSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "EllipseMarker" )
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
  if ( symbolItemList.size() > 0 )
  {
    mShapeListWidget->setCurrentItem( symbolItemList.at( 0 ) );
  }

  //set combo entries to current values
  blockComboSignals( true );
  if ( mLayer )
  {
    mSymbolWidthUnitWidget->setUnit( mLayer->symbolWidthUnit() );
    mSymbolWidthUnitWidget->setMapUnitScale( mLayer->symbolWidthMapUnitScale() );
    mOutlineWidthUnitWidget->setUnit( mLayer->outlineWidthUnit() );
    mOutlineWidthUnitWidget->setMapUnitScale( mLayer->outlineWidthMapUnitScale() );
    mSymbolHeightUnitWidget->setUnit( mLayer->symbolHeightUnit() );
    mSymbolHeightUnitWidget->setMapUnitScale( mLayer->symbolHeightMapUnitScale() );
    mOffsetUnitWidget->setUnit( mLayer->offsetUnit() );
    mOffsetUnitWidget->setMapUnitScale( mLayer->offsetMapUnitScale() );
  }

  QPointF offsetPt = mLayer->offset();
  spinOffsetX->setValue( offsetPt.x() );
  spinOffsetY->setValue( offsetPt.y() );
  mHorizontalAnchorComboBox->setCurrentIndex( mLayer->horizontalAnchorPoint() );
  mVerticalAnchorComboBox->setCurrentIndex( mLayer->verticalAnchorPoint() );
  blockComboSignals( false );
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
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSymbolWidthUnitWidget->getUnit() );
    mLayer->setSymbolWidthUnit( unit );
    mLayer->setSymbolWidthMapUnitScale( mSymbolWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOutlineWidthUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOutlineWidthUnitWidget->getUnit() );
    mLayer->setOutlineWidthUnit( unit );
    mLayer->setOutlineWidthMapUnitScale( mOutlineWidthUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mSymbolHeightUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSymbolHeightUnitWidget->getUnit() );
    mLayer->setSymbolHeightUnit( unit );
    mLayer->setSymbolHeightMapUnitScale( mSymbolHeightUnitWidget->getMapUnitScale() );
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::on_mOffsetUnitWidget_changed()
{
  if ( mLayer )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mOffsetUnitWidget->getUnit() );
    mLayer->setOffsetUnit( unit );
    mLayer->setOffsetMapUnitScale( mOffsetUnitWidget->getMapUnitScale() );
    emit changed();
  }
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

void QgsEllipseSymbolLayerV2Widget::on_mDataDefinedPropertiesButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QList< QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry > dataDefinedProperties;
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "width", tr( "Symbol width" ), mLayer->dataDefinedPropertyString( "width" ),
      QgsDataDefinedSymbolDialog::doubleHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "height", tr( "Symbol height" ), mLayer->dataDefinedPropertyString( "height" ),
      QgsDataDefinedSymbolDialog::doubleHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "rotation", tr( "Rotation" ), mLayer->dataDefinedPropertyString( "rotation" ),
      QgsDataDefinedSymbolDialog::doubleHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "outline_width", tr( "Outline width" ), mLayer->dataDefinedPropertyString( "outline_width" ),
      QgsDataDefinedSymbolDialog::doubleHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "fill_color", tr( "Fill color" ), mLayer->dataDefinedPropertyString( "fill_color" ),
      QgsDataDefinedSymbolDialog::colorHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "outline_color", tr( "Border color" ), mLayer->dataDefinedPropertyString( "outline_color" ),
      QgsDataDefinedSymbolDialog::colorHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "symbol_name", tr( "Symbol name" ), mLayer->dataDefinedPropertyString( "symbol_name" ),
      "'circle'|'rectangle'|'cross'|'triangle'" );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "offset", tr( "Offset" ), mLayer->dataDefinedPropertyString( "offset" ),
      QgsDataDefinedSymbolDialog::offsetHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "horizontal_anchor_point", tr( "Horizontal anchor point" ), mLayer->dataDefinedPropertyString( "horizontal_anchor_point" ),
      QgsDataDefinedSymbolDialog::horizontalAnchorHelpText() );
  dataDefinedProperties << QgsDataDefinedSymbolDialog::DataDefinedSymbolEntry( "vertical_anchor_point", tr( "Vertical anchor point" ), mLayer->dataDefinedPropertyString( "vertical_anchor_point" ),
      QgsDataDefinedSymbolDialog::verticalAnchorHelpText() );
  QgsDataDefinedSymbolDialog d( dataDefinedProperties, mVectorLayer );
  if ( d.exec() == QDialog::Accepted )
  {
    //empty all existing properties first
    mLayer->removeDataDefinedProperties();

    QMap<QString, QString> properties = d.dataDefinedProperties();
    QMap<QString, QString>::const_iterator it = properties.constBegin();
    for ( ; it != properties.constEnd(); ++it )
    {
      if ( !it.value().isEmpty() )
      {
        mLayer->setDataDefinedProperty( it.key(), it.value() );
      }
    }
    emit changed();
  }
}

void QgsEllipseSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}


